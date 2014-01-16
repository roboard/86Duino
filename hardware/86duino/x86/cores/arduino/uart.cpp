/*
  uart.cpp - DM&P Vortex86 UART library
  Copyright (c) 2013 DY Hung <Dyhung@dmp.com.tw>. All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

  (If you need a commercial license, please contact soc@dmp.com.tw 
   to get more information.)
*/

#define __DMP_COM_LIB

#define _VORTEX86EXC_UART_WORKAROUND

#include "uart.h"
#define  USE_COMMON
#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#if defined DMP_DOS_DJGPP
	#include <time.h>
#elif defined DMP_DOS_WATCOM
    #include <time.h>
#elif defined DMP_DOS_BC
    #include <dos.h>
#endif

#include "io.h"
#include "irq.h"
#include "err.h"

//
// ------------------------------------------------- //
//                 Macro Definition                  //
// ------------------------------------------------- //
//
// TX & RX Queue size macro
#define TX_QUEUE_SIZE    		(4096)
#define RX_QUEUE_SIZE    		(4096)

#define NEAR_FULL_SIZE          (1024)

//
// Hardware flow control macro
#define CTS_ON  				(1)
#define CTS_OFF 				(0)

//
// Software flow control macro
#define XON_ASCII   			(0x11)
#define XOFF_ASCII  			(0x13)
#define XON_RCVD 				(1)
#define XOFF_RCVD 				(0)
#define XON_XMIT  				(1)
#define XOFF_XMIT 				(0)

//
// FIFO size
#define FIFO_SIZE_NO        (1)
#define FIFO_SIZE_16        (16)
#define FIFO_SIZE_32        (32)
// ------------------------------------------------- //

//
// only for DLLSB, DLMSB
DMP_INLINE(unsigned char) _16550_DLAB_In(SerialPort *port, unsigned short addr)
{
	unsigned char data, lcr;
	
	io_DisableINT();
	{
		lcr = io_inpb(port->LCR);  
		io_outpb(port->LCR, 0x80); 
		data = io_inpb(addr);      
		io_outpb(port->LCR, lcr);  
	}
	io_RestoreINT();
	
	return data;
}
//
// only for DLLSB, DLMSB, FCR
DMP_INLINE(void) _16550_DLAB_Out(SerialPort *port, unsigned short addr, unsigned char data)
{
	unsigned char lcr;
	
	io_DisableINT();
	{
		lcr = io_inpb(port->LCR);  
		io_outpb(port->LCR, 0x80); 
		io_outpb(addr, data);      
		io_outpb(port->LCR, lcr);  
	}
	io_RestoreINT();
}

//
// ------------------------------------------------- //
//                  Global Variable                  //
// ------------------------------------------------- //
//
// pre-definition of 10 UART's base address and IRQ
// static unsigned short uart_addr[10] = {0x3f8, 0x2f8, 0x3e8, 0x2e8, 0x360, 0x260, 0x368, 0x268, 0x3e0, 0x2e0};
// static int            uart_irq[10]  = {0x04, 0x04, 0x04, 0x04, 0x03, 0x03, 0x03, 0x03, 0x09, 0x09};

DMPAPI(void *) CreateUART(int com)
{
	SerialPort *port;
	
	if (io_Init() == false)
	{
		err_print((char*)"%s: Init IO lib error!!\n", __FUNCTION__);
		return NULL;
	}
	
	if ((port = (SerialPort *)ker_Malloc(sizeof(SerialPort))) == NULL)
	{
		io_Close();
		return NULL;
	}

	port->com  = com;
	port->addr = vx86_uart_GetBaseAddr(com);
	port->nIRQ = vx86_uart_GetIRQ(com);
	
	if (port->addr == 0x00 || port->nIRQ == 0)
	{
		err_print((char*)"%s: COM%d is null. [Base address: 0x%04x, IRQ: %d]\n", __FUNCTION__, com+1, port->addr, port->nIRQ);
		ker_Mfree((void*)port);
		io_Close();
		return NULL;
	}
	
	port->nBuad        = 115200L;
	port->nData        = BYTESIZE8;
	port->nStop        = STOPBIT1;
	port->nParity      = NOPARITY;
	port->control      = NO_CONTROL;
	
	port->InUse        = 0;
	port->INT_InUse    = 0;
	
	port->rts          = 0;
	port->cts          = CTS_OFF;
	
	port->xonxoff_rcvd = XOFF_RCVD;
	port->xonxoff_xmit = XOFF_XMIT;
	port->xon          = 0;
	port->xoff         = 0;
																			 
	port->old_lsb      = port->lsb      = 0;
	port->old_msb      = port->msb      = 0;
	port->old_ier      = port->ier      = 0;
	port->old_lcr      = port->lcr      = 0;
	port->old_mcr      = port->mcr      = 0;
	port->old_TimeOut  = port->TimeOut  = UART_NO_TIMEOUT;
	port->fcr = 0;
	
	port->RFIFO_Size   = 0;
	port->WFIFO_Size   = 0;
	
	if ((port->rcvd = CreateQueue(RX_QUEUE_SIZE)) == NULL) goto CREATE_RX_QUEUE_FAIL;
	if ((port->xmit = CreateQueue(TX_QUEUE_SIZE)) == NULL) goto CREATE_TX_QUEUE_FAIL;
	
	port->msr_handler = NULL;
	port->lsr_handler = NULL;
	
	port->TXDB  = port->addr + 0;
	port->RXDB  = port->addr + 0;
	port->DLLSB = port->addr + 0;
	port->DLMSB = port->addr + 1;
	port->IER   = port->addr + 1;
	port->IIR   = port->addr + 2;
	port->FCR   = port->addr + 2;
	port->LCR   = port->addr + 3;
	port->MCR   = port->addr + 4;
	port->LSR   = port->addr + 5;
	port->MSR   = port->addr + 6;
	port->SCR   = port->addr + 7;
	
	return (void *)port;
	
CREATE_TX_QUEUE_FAIL:
	DestoryQueue(port->rcvd);
CREATE_RX_QUEUE_FAIL:
	ker_Mfree((void*)port);
	io_Close();
	return NULL;
}

DMP_INLINE(void) uart_Free(SerialPort *port)
{
	if (port == NULL) { err_print((char*)"%s: port is null.\n", __FUNCTION__); return; }
	DestoryQueue(port->rcvd);
	DestoryQueue(port->xmit);
	ker_Mfree((void*)port);
}

DMP_INLINE(bool) uart_IsOK(SerialPort *port)
{
	unsigned char temp;
	if (port == NULL) { err_print((char*)"%s: port is null.\n", __FUNCTION__); return false; }
	
	if ((io_inpb(port->IIR) & 0x30) != 0x00) return false;
	
	temp = io_inpb(port->SCR);
	io_outpb(port->SCR, 0x55);
	if (io_inpb(port->SCR) != 0x55) return false;
	io_outpb(port->SCR, 0xaa);
	if (io_inpb(port->SCR) != 0xaa) return false;
	io_outpb(port->SCR, temp);
	
	return true;
}

DMPAPI(bool) uart_Init(void *vport)
{
	SerialPort *port = (SerialPort *)vport;

	if (vx86_uart_Init(port->com) == false) {
		err_print((char*)"%s: Vertex86 COM%d init fail!!\n", __FUNCTION__, port->com + 1);
		goto VX86_UART_INIT_FAIL;
	}

	if (uart_IsOK(port) == false) {
		err_print((char*)"%s: COM%d doesn't exist.\n", __FUNCTION__, port->com + 1);
		goto CHECK_EXIST_FAIL;
	}

	if (irq_Init() == false) {
        err_print((char*)"%s: IRQ init fail.\n", __FUNCTION__);
        goto IRQ_INIT_FAIL;
    }

	// save old UART config
	port->old_lsb     = port->lsb     = _16550_DLAB_In(port, port->DLLSB);
	port->old_msb     = port->msb     = _16550_DLAB_In(port, port->DLMSB);
	port->old_ier     = port->ier     = io_inpb(port->IER); 
	port->old_lcr     = port->lcr     = io_inpb(port->LCR); 
	port->old_mcr     = port->mcr     = io_inpb(port->MCR); 
	port->old_TimeOut = port->TimeOut = UART_NO_TIMEOUT;
	
	// UART initial
	uart_SetBaud(vport, 115200L);
	uart_SetFormat(vport, BYTESIZE8 + STOPBIT1 + NOPARITY);
	uart_SetTimeOut(vport, UART_NO_TIMEOUT);

	// flush rx & tx queue
	uart_FlushRxQueue(vport);
	uart_FlushTxQueue(vport);

	// clear Hardware RX & TX FIFO
	uart_ClearRFIFO(vport); 
	uart_ClearWFIFO(vport); 

	// enable FIFO mode
	uart_SetHWFIFO(vport, ENABLE_HW_FIFO_16); 

	// enable receive data interrupt
	uart_IntEnable(vport, IERALL);
	
	// set flow control
	uart_SetFlowControl(vport, NO_CONTROL);

	port->InUse = 1;
	
	return true;

IRQ_INIT_FAIL:
CHECK_EXIST_FAIL:
	vx86_uart_Close(port->com);
VX86_UART_INIT_FAIL:
	if (io_Close() == false) err_print((char*)"Close IO lib error!!\n");
	return false;
}

DMPAPI(void) uart_Close(void *vport)
{
	unsigned char lcr;
	SerialPort *port = (SerialPort *)vport;
	if (port == NULL) { err_print((char*)"%s: port is null.\n", __FUNCTION__); return; }
	
	if (port->InUse != 0)
	{
		// restore old IER & MCR
		uart_IntDisable(vport);
		io_DisableINT();
		{
			io_outpb(port->MCR, port->old_mcr); 
		}
		io_RestoreINT();
		irq_Close();
		
		// restore old LSB & MSB
#ifdef _VORTEX86EXC_UART_WORKAROUND
	/* To avoid Vortex86EX(D) write sync. issue. */
	io_DisableINT();
	{
		lcr = io_inpb(port->LCR);  
		io_outpb(port->LCR, 0x80); 
		
		do {
			io_outpb(port->DLLSB, port->old_lsb);
		} while (io_inpb(port->DLLSB) != port->old_lsb);
		
		do {
			io_outpb(port->DLMSB, port->old_msb);
		} while (io_inpb(port->DLMSB) != port->old_msb);
		
		io_inpb(0x80); // do IO delay
		io_outpb(port->LCR, lcr);  
	}
	io_RestoreINT();
#else
		_16550_DLAB_Out(port, port->DLLSB, port->old_lsb);
		_16550_DLAB_Out(port, port->DLMSB, port->old_msb);
#endif
		
		// restore old LCR & timeout
		uart_SetFormat(vport, port->old_lcr);
		uart_SetTimeOut(vport, port->old_TimeOut);
		
		vx86_uart_Close(port->com);
		if (io_Close() == false) err_print((char*)"Close IO lib error!!\n");
	}
	uart_Free(port);
}

DMPAPI(bool) uart_SetBaud(void *vport, unsigned long baud)
{
	unsigned char lcr;
	unsigned short divisor;
	SerialPort *port = (SerialPort *)vport;
	
	if (port == NULL) { err_print((char*)"%s: port is null.\n", __FUNCTION__); return false; }
	
	if (vx86_uart_GetSBCLK() != 0)
		divisor = (unsigned short)(baud >> 16);
	else
		divisor = (unsigned short)(baud);
	
	if (divisor == 0xFFFF) {
		err_print((char*)"%s: Unsupported baud rate.\n", __FUNCTION__);
		return false;
	}
	
	// set speed mode
	vx86_uart_SetCS(port->com, (divisor & 0x8000) >> 15);
	vx86_uart_SetHCS(port->com, (divisor & 0x4000) >> 14);
	
	divisor &= 0x3FFF;
	port->lsb = (unsigned char)(divisor & 0x00FF);
	port->msb = (unsigned char)((divisor & 0xFF00) >> 8);

#ifdef _VORTEX86EXC_UART_WORKAROUND
	/* To avoid Vortex86EX(D) write sync. issue. */
	io_DisableINT();
	{
		lcr = io_inpb(port->LCR);  
		io_outpb(port->LCR, 0x80); 
		
		do {
			io_outpb(port->DLLSB, port->lsb);
		} while (io_inpb(port->DLLSB) != port->lsb);
		
		do {
			io_outpb(port->DLMSB, port->msb);
		} while (io_inpb(port->DLMSB) != port->msb);
		
		io_inpb(0x80); // do IO delay
		io_outpb(port->LCR, lcr);  
	}
	io_RestoreINT();
#else
	_16550_DLAB_Out(port, port->DLLSB, port->lsb);
	_16550_DLAB_Out(port, port->DLMSB, port->msb);
#endif
	
	return true;
}

DMPAPI(void) uart_SetFormat(void *vport, unsigned char format)
{
	SerialPort *port = (SerialPort *)vport;
	if (port == NULL) { err_print((char*)"%s: port is null.\n", __FUNCTION__); return; }
	
	io_DisableINT();
	{
		io_outpb(port->LCR, format); 
	}
	io_RestoreINT();
}

DMPAPI(void) uart_SetFlowControl(void *vport, int control)
{
	SerialPort *port = (SerialPort *)vport;
	if (port == NULL) { err_print((char*)"%s: port is null.\n", __FUNCTION__); return; }
	
	port->control = control;
	
	switch (port->control) {
	
		case NO_CONTROL: break;
		
		case RTS_CTS:
		{
			io_outpb(port->MCR, io_inpb(port->MCR) | 0x03);
			port->rts = 1;
			port->cts = (io_inpb(port->MSR) & 0x10) ? CTS_ON : CTS_OFF;
		}
		break;
		
		case XON_XOFF:
		{
			port->xonxoff_rcvd = XON_RCVD;
			port->xonxoff_xmit = XON_XMIT;
		}
		break;
		
		default: break;
	};
}

DMPAPI(void) uart_SetTimeOut(void *vport, unsigned long timeout)
{
	SerialPort *port = (SerialPort *)vport;
	if (port == NULL) { err_print((char*)"%s: port is null.\n", __FUNCTION__); return; }
	
	port->TimeOut = timeout;
}

DMPAPI(unsigned char) uart_GetLSR(void *vport)
{
	SerialPort *port = (SerialPort *)vport;
	if (port == NULL) { err_print((char*)"%s: port is null.\n", __FUNCTION__); return 0x00; }
	
	return io_inpb(port->LSR);
}

DMPAPI(unsigned char) uart_GetMSR(void *vport)
{
	SerialPort *port = (SerialPort *)vport;
	if (port == NULL) { err_print((char*)"%s: port is null.\n", __FUNCTION__); return 0x00; }
	
	return io_inpb(port->MSR);
}

DMPAPI(void) uart_SetHWFIFO(void *vport, int fifo)
{
	SerialPort *port = (SerialPort *)vport;
	if (port == NULL) { err_print((char*)"%s: port is null.\n", __FUNCTION__); return; }
	
	switch (fifo)
	{
		case DISABLE_HW_FIFO:
		{
			_16550_DLAB_Out(port, port->FCR, port->fcr &= 0xfe);
			port->RFIFO_Size = FIFO_SIZE_NO;
			port->WFIFO_Size = FIFO_SIZE_NO;
		}
		break;
		
		case ENABLE_HW_FIFO_16:
		{
			vx86_uart_SetFIFO16(port->com);
			_16550_DLAB_Out(port, port->FCR, port->fcr |= 0x01);
			port->RFIFO_Size = FIFO_SIZE_16;
			port->WFIFO_Size = FIFO_SIZE_16;
		}
		break;
		
		case ENABLE_HW_FIFO_32:
		case ENABLE_HW_FIFO_128:
		{
			vx86_uart_SetFIFO32(port->com);
			_16550_DLAB_Out(port, port->FCR, port->fcr |= 0x01);
			port->RFIFO_Size = FIFO_SIZE_32;
			port->WFIFO_Size = FIFO_SIZE_32;
		}
		break;
	};
}

DMPAPI(void) uart_ClearRFIFO(void *vport)   
{
	SerialPort *port = (SerialPort *)vport;
	if (port == NULL) { err_print((char*)"%s: port is null.\n", __FUNCTION__); return; }
	
	_16550_DLAB_Out(port, port->FCR, port->fcr |= 0x02);
}

DMPAPI(void) uart_ClearWFIFO(void *vport)
{
	SerialPort *port = (SerialPort *)vport;
	if (port == NULL) { err_print((char*)"%s: port is null.\n", __FUNCTION__); return; }
	
	_16550_DLAB_Out(port, port->FCR, port->fcr |= 0x04);
}

DMPAPI(void) uart_SetRFIFOTrig(void *vport, int cap)
{
	SerialPort *port = (SerialPort *)vport;
	if (port == NULL) { err_print((char*)"%s: port is null.\n", __FUNCTION__); return; }
	
	_16550_DLAB_Out(port, port->FCR, port->fcr = ((port->fcr & 0x3f) | (unsigned char)cap));
}

DMPAPI(bool) uart_SetWFIFOSize(void *vport, int size) // setup after enable/disable fifo
{
	SerialPort *port = (SerialPort *)vport;
	if (port == NULL) { err_print((char*)"%s: port is null.\n", __FUNCTION__); return false; }
	
	if (size > port->RFIFO_Size) return false; // check fifo size by receive fifo
	port->WFIFO_Size = size;
	return true;
}

DMPAPI(int) uart_QueryRxQueue(void *vport)
{
	SerialPort *port = (SerialPort *)vport;
	return port->rcvd->count;
}
DMPAPI(bool) uart_RxQueueFull(void *vport)
{
	SerialPort *port = (SerialPort *)vport;
	return (port->rcvd->count >= port->rcvd->size) ? (true) : (false);
}
DMPAPI(bool) uart_RxQueueEmpty(void *vport)
{
	SerialPort *port = (SerialPort *)vport;
	return (port->rcvd->count <= 0) ? (true) : (false);
}

DMPAPI(unsigned int) uart_Read(void *vport)
{
	unsigned char val;
	unsigned long pretime;
	SerialPort *port = (SerialPort *)vport;
	
	if (port == NULL) { err_print((char*)"%s: port is null.\n", __FUNCTION__); return 0xffff; }
	
	if (port->TimeOut != UART_NO_TIMEOUT) {
		pretime = timer_nowtime();
		while (port->rcvd->count <= 0 && (timer_nowtime() - pretime) < port->TimeOut); 
		
		if (port->rcvd->count <= 0) {
			if (UART_TIMEOUT_DEBUG)
				err_print((char*)"%s: COM%d receive timeout.\n", __FUNCTION__, port->com + 1);
			return (unsigned int)0xffff;
		}
	}
	else while (port->rcvd->count <= 0);
	
	io_DisableINT();
	{
		val = PopQueue(port->rcvd);
		
		switch (port->control)
		{
			case NO_CONTROL: break;
			
			case RTS_CTS:
			{
				if (port->rcvd->count < (port->rcvd->size - NEAR_FULL_SIZE) && port->rts == 0) {
					io_outpb(port->MCR, io_inpb(port->MCR) | 0x02);
					port->rts = 1;
				}
			}
			break;
			
			case XON_XOFF:
			{
				if (port->rcvd->count < (port->rcvd->size - NEAR_FULL_SIZE) && port->xonxoff_xmit != XON_XMIT) {
					port->xon = 1;
					io_outpb(port->IER, port->ier |= 0x02);
				}
			}
			break;
			
			default: break;
		};
	}
	io_RestoreINT();

    return (unsigned int)val;
}

DMPAPI(int) uart_Receive(void *vport, unsigned char* buf, int bsize)
{
	int i;
	unsigned int val;
	
	for (i = 0; i < bsize; ) {
		val = uart_Read(vport);
		if (val != 0xffff) buf[i++] = (unsigned char)val;
		else break;
	}
	
	return i;
}

DMPAPI(void) uart_FlushRxQueue(void *vport)
{
	SerialPort *port = (SerialPort *)vport;
	io_DisableINT();
	{
		ClearQueue(port->rcvd);
		
		switch (port->control)
		{
			case NO_CONTROL: break;
			
			case RTS_CTS:
			{
				if (port->rts == 0) {
					io_outpb(port->MCR, io_inpb(port->MCR) | 0x02);
					port->rts = 1;
				}
			}
			break;
			
			case XON_XOFF:
			{
				if (port->xonxoff_xmit != XON_XMIT) {
					port->xon = 1;
					io_outpb(port->IER, port->ier |= 0x02);
				}
			}
			break;
			
			default: break;
		};
	}
	io_RestoreINT();
}

DMPAPI(int) uart_QueryTxQueue(void *vport)
{
	SerialPort *port = (SerialPort *)vport;
	return port->xmit->count;
}

DMPAPI(bool) uart_TxQueueFull(void *vport)
{
	SerialPort *port = (SerialPort *)vport;
	return (port->xmit->count >= port->xmit->size) ? (true) : (false);
}

DMPAPI(bool) uart_TxQueueEmpty(void *vport)
{
	SerialPort *port = (SerialPort *)vport;
	return (port->xmit->count <= 0) ? (true) : (false);
}

DMPAPI(bool) uart_TxReady(void *vport)
{
	SerialPort *port = (SerialPort *)vport;
	return (port->xmit->count < port->xmit->size) ? (true) : (false);
}

DMPAPI(int) uart_Send(void *vport, unsigned char* buf, int bsize)
{
	int i;
	unsigned long pretime;
	SerialPort *port = (SerialPort *)vport;
	
	if (port == NULL) { err_print((char*)"%s: port is null.\n", __FUNCTION__); return 0; }
	
	for (i = 0; i < bsize; i++)
	{
		if (port->TimeOut != UART_NO_TIMEOUT) {
			pretime = timer_nowtime();
			while (port->xmit->count >= port->xmit->size && (timer_nowtime() - pretime) < port->TimeOut); 
			
			if (port->xmit->count >= port->xmit->size) {
				if (UART_TIMEOUT_DEBUG)
					err_print((char*)"%s: COM%d transmit timeout.\n", __FUNCTION__, port->com + 1);
				if (!(port->ier & THREI)) io_outpb(port->IER, port->ier |= THREI);
				return i;
			}
		}
		else while (port->xmit->count >= port->xmit->size);
		
		io_DisableINT();
		{
			PushQueue(port->xmit, buf[i]);
			
			if (!(port->ier & THREI)
			&& (i == (bsize - 1) || port->xmit->count >= port->xmit->size))
			{	
				switch (port->control)
				{
					case NO_CONTROL:
					{
						io_outpb(port->IER, port->ier |= THREI);
					}
					break;
					
					case RTS_CTS:
					{
						if (port->cts == CTS_ON) {
							io_outpb(port->IER, port->ier |= THREI);
						}
					}
					break;
					
					case XON_XOFF:
					{
						if (port->xonxoff_rcvd != XOFF_RCVD) {
							io_outpb(port->IER, port->ier |= THREI);
						}
					}
					break;
					
					default: break;
				};
			}
		}
		io_RestoreINT();
	}
	
	return i;
}

DMPAPI(bool) uart_Write(void *vport, unsigned char val)
{
	if (uart_Send(vport, &val, 1) != 1) return false;
	return true;
}

DMPAPI(void) uart_FlushTxQueue(void *vport)
{
	SerialPort *port = (SerialPort *)vport;
	io_DisableINT();
	{
		ClearQueue(port->xmit);
	}
	io_RestoreINT();
}

DMPAPI(void) uart_FlushWFIFO(void *vport)
{
	SerialPort *port = (SerialPort *)vport;
	while (port->xmit->count > 0);
	while ((io_inpb(port->LSR) & 0x60) != 0x60);
}

static int UART_ISR(int irq, void *data)
{
	int i;
	unsigned char c, iir;
	SerialPort *port;
	
	int ISR_Status = ISR_NONE;

	port  = (SerialPort *)data;
	
	while (!((iir = io_inpb(port->IIR)) & 0x01))
	{
		ISR_Status = ISR_HANDLED;
		
		switch (iir & 0x07)
		{
			// timeout & receive data ready interrupt
			case 0x0c: case 0x04:
			{
				do {
					c = io_inpb(port->RXDB);
					
					switch (port->control)
					{
						case NO_CONTROL:
						{
							PushQueue(port->rcvd, c);
						}
						break;
						
						case RTS_CTS:
						{
							PushQueue(port->rcvd, c);
							if (port->rcvd->count >= (port->rcvd->size - NEAR_FULL_SIZE))
							{
								io_outpb(port->MCR, io_inpb(port->MCR) & 0xfd);
								port->rts = 0;
							}
						}
						break;
						
						case XON_XOFF:
						{
							if (c == XON_ASCII) {
								port->xonxoff_rcvd = XON_RCVD;
								io_outpb(port->IER, port->ier |= 0x02);
								break;
							}
							if (c == XOFF_ASCII) {
								port->xonxoff_rcvd = XOFF_RCVD;
								io_outpb(port->IER, port->ier &= 0xfd);
								break;
							}
							PushQueue(port->rcvd, c);
							if (port->rcvd->count >= (port->rcvd->size - NEAR_FULL_SIZE) && port->xonxoff_xmit != XOFF_XMIT)
							{
								port->xoff = 1;
								io_outpb(port->IER, port->ier |= 0x02);
							}
						}
						break;
						
						default: break;
					};
					
				} while (io_inpb(port->LSR) & 0x01);
			}
			break;
			
			// transmitter holding register empty interrupt
			case 0x02:
			{
				switch (port->control)
				{
					case NO_CONTROL:
					{
						if (port->xmit->count <= 0)
							io_outpb(port->IER, port->ier &= 0xfd);
						else
						{
							for (i = 0; i < port->WFIFO_Size && port->xmit->count > 0; i++)
								io_outpb(port->TXDB, (unsigned char)PopQueue(port->xmit));
						}
					}
					break;
					
					case RTS_CTS:
					{
						if (port->xmit->count <= 0)
							io_outpb(port->IER, port->ier &= 0xfd);
						else
						{
							for (i = 0; i < port->WFIFO_Size && port->xmit->count > 0; i++)
							{
								if (port->cts == CTS_OFF)
								{
									io_outpb(port->IER, port->ier &= 0xfd);
									break;
								}
								io_outpb(port->TXDB, (unsigned char)PopQueue(port->xmit));
							}
						}
					}
					break;
					
					case XON_XOFF:
					{
						if (port->xoff == 1) {
							io_outpb(port->TXDB, XOFF_ASCII);
							port->xoff = 0;
							port->xonxoff_xmit = XOFF_XMIT;
						}
						else if (port->xon == 1) {
							io_outpb(port->TXDB, XON_ASCII);
							port->xon = 0;
							port->xonxoff_xmit = XON_XMIT;
						}
						if (port->xmit->count <= 0 || port->xonxoff_rcvd == XOFF_RCVD)
							io_outpb(port->IER, port->ier &= 0xfd);
						else
						{
							for (i = 0; i < port->WFIFO_Size && port->xmit->count > 0; i++)
							{
								if (port->xonxoff_rcvd == XOFF_RCVD)
								{
									io_outpb(port->IER, port->ier &= 0xfd);
									break;
								}
								io_outpb(port->TXDB, (unsigned char)PopQueue(port->xmit));
							}
						}
					}
					break;
					
					default: break;
				};
			}
			break;
			
			// received line status interrupt
			case 0x06:
			{
				port->lsr = io_inpb(port->LSR);
				if (port->lsr_handler != NULL) port->lsr_handler(port);
			}
			break;
			
			// modem status interrupt
			case 0x00:
			{
				port->msr = io_inpb(port->MSR);
				if (port->control == RTS_CTS) {
					if (port->cts == CTS_OFF && (port->msr & 0x10)) {
						port->cts = CTS_ON;
						if (port->xmit->count > 0) io_outpb(port->IER, port->ier |= 0x02);
					}
					else if (port->cts == CTS_ON && !(port->msr & 0x10)){
						port->cts = CTS_OFF;
						io_outpb(port->IER, port->ier &= 0xfd);
					}
				}
				if (port->msr_handler != NULL) port->msr_handler(port);
			}
			break;
			
			default: break;
			
		};
		
		if (port->rcvd->count >= (port->rcvd->count - NEAR_FULL_SIZE)) break;
	}

	return ISR_Status;
}
#if defined DMP_DOS_DJGPP
DPMI_END_OF_LOCKED_STATIC_FUNC(UART_ISR)
#endif

DMPAPI(void) uart_IntEnable(void *vport, int cap)
{
	SerialPort *port = (SerialPort *)vport;
	if (port == NULL) { err_print((char*)"%s: port is null.\n", __FUNCTION__); return; }
	
	if (port->INT_InUse != 0) return;
	if (port->nIRQ <= 0 || port->nIRQ > 15)
	{
		err_print((char*)"%s: There is no IRQ%d.\n", __FUNCTION__, port->nIRQ);
		return;
	}
	
	#if defined DMP_DOS_DJGPP
	DPMI_LOCK_FUNC(UART_ISR);
	#endif
	
	io_DisableINT();
	{
		irq_Setting(port->nIRQ, IRQ_LEVEL_TRIGGER);
		irq_InstallISR(port->nIRQ, UART_ISR, (void *)port);
		
		io_outpb(port->IER, port->ier |= ((unsigned char)cap)); 
		io_outpb(port->MCR, io_inpb(port->MCR) | 0x08);         
	}
	io_RestoreINT();
	
	port->INT_InUse++;
}
 
DMPAPI(void) uart_IntDisable(void *vport)
{
	SerialPort *port = (SerialPort *)vport;
	if (port == NULL) { err_print((char*)"%s: port is null.\n", __FUNCTION__); return; }
	
	if (port->INT_InUse == 0) return;
	
	io_DisableINT();
	{
		io_outpb(port->MCR, io_inpb(port->MCR) & 0xf7); 
		io_outpb(port->IER, port->old_ier);             
		
		irq_UninstallISR(port->nIRQ, (void *)port);
	}
	io_RestoreINT();
	
	port->INT_InUse = 0;
}

DMPAPI(void) uart_SetLSRHandler(void *vport, void (*func)(SerialPort *))
{
	SerialPort *port = (SerialPort *)vport;
	if (port == NULL) { err_print((char*)"%s: port is null.\n", __FUNCTION__); return; }
	
	port->lsr_handler = func;
}

DMPAPI(void) uart_SetMSRHandler(void *vport, void (*func)(SerialPort *))
{
	SerialPort *port = (SerialPort *)vport;
	if (port == NULL) { err_print((char*)"%s: port is null.\n", __FUNCTION__); return; }
	
	port->msr_handler = func;
}

DMPAPI(void) uart_EnableHalfDuplex(void *vport)
{
	SerialPort *port = (SerialPort *)vport;
	if (port == NULL) { err_print((char*)"%s: port is null.\n", __FUNCTION__); return; }
	
	vx86_uart_EnableHalfDuplex(port->com);
}

DMPAPI(void) uart_EnableFullDuplex(void *vport)
{
	SerialPort *port = (SerialPort *)vport;
	if (port == NULL) { err_print((char*)"%s: port is null.\n", __FUNCTION__); return; }
	
	vx86_uart_EnableFullDuplex(port->com);
}
