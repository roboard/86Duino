/*
  uart.cpp - DM&P Vortex86 UART library
  Copyright (c) 2018 Johnson Hung <Dyhung@dmp.com.tw>. All right reserved.

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
////////////////////////////////////////////////////////////////////////////////
//    note that most of functions in this lib assume no paging issue when 
//    using them in ISR; so to use this lib in ISR in DJGPP, it is suggested 
//    to employ PMODE/DJ or HDPMI instead of CWSDPMI.
////////////////////////////////////////////////////////////////////////////////

// #define _VORTEX86EXC_UART_WORKAROUND

#include "uart.h"

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
#define TX_QUEUE_SIZE    		(1024*4)
#define RX_QUEUE_SIZE    		(1024*1024*4)

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

#define MSIE    (1 << 3)
#define RLSIE   (1 << 2)
#define THREIE  (1 << 1)
#define RDAIE   (1 << 0)

#define FIFOE   (3 << 6)
#define FIFOM   (1 << 3)
#define PT      (3 << 1)
#define IP      (1 << 0)

#define TL      (3 << 6)
#define CTF     (1 << 2)
#define CRF     (1 << 1)
#define FE      (1 << 0)

#define DLAB    (1 << 7)
#define BC      (1 << 6)
#define SP      (1 << 5)
#define EOP     (1 << 4)
#define PE      (1 << 3)
#define NSB     (1 << 2)
#define SCN     (3 << 0)

#define LBF     (1 << 4)
#define INTE    (1 << 3)
#define RTS     (1 << 1)
#define DTR     (1 << 0)

#define EB      (1 << 7)
#define TEMT    (1 << 6)
#define THRE    (1 << 5)
#define BI      (1 << 4)
#define FES     (1 << 3)
#define PES     (1 << 2)
#define OE      (1 << 1)
#define DR      (1 << 0)

#define DCD     (1 << 7)
#define RI      (1 << 6)
#define DSR     (1 << 5)
#define CTS     (1 << 4)
#define DDCD    (1 << 3)
#define TERI    (1 << 2)
#define DDSR    (1 << 1)
#define DCTS    (1 << 0)


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

static int _recv_poll(SerialPort *port, unsigned char* buf, int bsize)
{
	int cur = 0;
	
	while (cur < bsize) {
		if (io_inpb(port->LSR) & 0x01) {
			buf[cur] = io_inpb(port->RXDB);
			cur++;
		} else {
			break;
		}
	}
	
	return cur;
}

static int _send_poll(SerialPort *port, unsigned char* buf, int bsize)
{
	int cur = 0;
	
	while (cur < bsize) {
		if ((io_inpb(port->LSR) & 0x60) == 0x60) {
			io_outpb(port->TXDB, buf[cur]);
			cur++;
		}
	}
	
	return cur;
}

static int _recv_int(SerialPort *port, unsigned char* buf, int bsize)
{
    int i;
    unsigned int value;
	unsigned long nowtime, pretime;
    bool doTimeout = false;
	
	for (i = 0; i < bsize; ) {
        io_DisableINT();
        if ((value = PopQueue(port->rcvd)) != 0xffff) {
            buf[i++] = (unsigned char)value;
            doTimeout = false;
		
			switch (port->control)
			{
				case NO_CONTROL: break;
				
				case RTS_CTS:
				{
					if (QueueSize(port->rcvd) < (RX_QUEUE_SIZE - NEAR_FULL_SIZE) && port->rts == 0) {
						io_outpb(port->MCR, io_inpb(port->MCR) | 0x02);
						port->rts = 1;
					}
				}
				break;
				
				case XON_XOFF:
				{
					if (QueueSize(port->rcvd) < (RX_QUEUE_SIZE - NEAR_FULL_SIZE) && port->xonxoff_xmit != XON_XMIT) {
						port->xon = 1;
						io_outpb(port->IER, port->ier |= 0x02);
					}
				}
				break;
				
				default: break;
			};
            io_RestoreINT();
		} else if (port->RxTimeOut == 0) {
            io_RestoreINT();
            break;
        } else if (port->RxTimeOut > 0 && doTimeout == true) {
            io_RestoreINT();
            nowtime = timer_NowTime();
            if (nowtime - pretime > port->RxTimeOut) {
				if (UART_TIMEOUT_DEBUG) {
					err_print((char*)"%s: COM%d receive timeout.\n", __FUNCTION__, port->com + 1);
                }
                break;
            }
        } else if (port->RxTimeOut > 0 && doTimeout == false) {
            io_RestoreINT();
            pretime = timer_NowTime();
            doTimeout = true;
        } else {
            io_RestoreINT();
        }
    }
    
    return i;
}

static int _send_int(SerialPort *port, unsigned char* buf, int bsize)
{
	int i;
	unsigned long nowtime, pretime;
    bool doTimeout = false;
	
	for (i = 0; i < bsize; ) {
        io_DisableINT();
        if (PushQueue(port->xmit, buf[i]) == true) {
            i++;
            doTimeout = false;
			
			if (!(port->ier & THREI) && ((i >= bsize) || QueueFull(port->xmit))) {
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
            io_RestoreINT();
        } else if (port->TxTimeOut == 0) {
            io_RestoreINT();
            break;
        } else if (port->TxTimeOut > 0 && doTimeout == true) {
            io_RestoreINT();
            nowtime = timer_NowTime();
            if (nowtime - pretime > port->TxTimeOut) {
				if (UART_TIMEOUT_DEBUG) {
					err_print((char*)"%s: COM%d transmit timeout.\n", __FUNCTION__, port->com + 1);
                }
                io_DisableINT();
				if (!(port->ier & THREI)) io_outpb(port->IER, port->ier |= THREI);
                io_RestoreINT();
                break;
            }
        } else if (port->TxTimeOut > 0 && doTimeout == false) {
            io_RestoreINT();
            pretime = timer_NowTime();
            doTimeout = true;
        } else {
            io_RestoreINT();
        }
    }
    
    return i;
}

static bool native_Rquery(SerialPort *port)
{
    return (io_inpb(port->LSR) & 0x01) != 0;
}

static int native_read(SerialPort *port, volatile unsigned char *buf)
{
    buf[0] = io_inpb(port->RXDB);
    
    return 1;
}

static bool fastRquery0w(SerialPort *port)
{
    return vx86_GetUartRxFIFORcvd(port->com, port->fabase) >= 1;
}

static int fastread0w(SerialPort *port, volatile unsigned char *buf)
{
    buf[0] = (vx86_UartFastMode0Read16(port->com, port->fabase) >> 8) & 0xFF;
    
    return 1;
}

static bool fastRquery0d(SerialPort *port)
{
    return vx86_GetUartRxFIFORcvd(port->com, port->fabase) >= 2;
}

static int fastread0d(SerialPort *port, volatile unsigned char *buf)
{
    unsigned long value = vx86_UartFastMode0Read(port->com, port->fabase);
    
    buf[0] = (value >>  8) & 0xFF;
    buf[1] = (value >> 24) & 0xFF;
    
    return 2;
}

static bool fastRquery1b(SerialPort *port)
{
    return vx86_GetUartRxFIFORcvd(port->com, port->fabase) >= 1;
}

static int fastread1b(SerialPort *port, volatile unsigned char *buf)
{
    buf[0] = vx86_UartFastMode1Read8(port->com, port->fabase);
    
    return 1;
}

static bool fastRquery1w(SerialPort *port)
{
    return vx86_GetUartRxFIFORcvd(port->com, port->fabase) >= 2;
}

static int fastread1w(SerialPort *port, volatile unsigned char *buf)
{
    unsigned short value = vx86_UartFastMode1Read16(port->com, port->fabase);
    
    buf[0] = value & 0xFF;
    buf[1] = (value >> 8) & 0xFF;
    
    return 2;
}

static bool fastRquery1d(SerialPort *port)
{
    return vx86_GetUartRxFIFORcvd(port->com, port->fabase) >= 4;
}

static int fastread1d(SerialPort *port, volatile unsigned char *buf)
{
    unsigned long value = vx86_UartFastMode1Read(port->com, port->fabase);
    
    buf[0] = value & 0xFF;
    buf[1] = (value >>  8) & 0xFF;
    buf[2] = (value >> 16) & 0xFF;
    buf[3] = (value >> 24) & 0xFF;
    
    return 4;
}

static bool auto_Rquery(SerialPort *port)
{
    int rcvd = vx86_GetUartRxFIFORcvd(port->com, port->fabase);
    
    if (rcvd >= 4) {
        
        port->intread = fastread1d;
        
    } else if (rcvd >= 2) {
        
        port->intread = fastread1w;
        
    } else {
        
        port->intread = fastread1b;
        
    }
    
    return rcvd > 0;
}

static bool native_Tquery(SerialPort *port)
{
    return !QueueEmpty(port->xmit);
}

static int native_write(SerialPort *port)
{
    io_outpb(port->TXDB, (unsigned char)PopQueue(port->xmit));
    
    return 1;
}

static bool fastTqueryb(SerialPort *port)
{
    return (QueueSize(port->xmit) >= 1) && (vx86_GetUartTxFIFOFree(port->com, port->fabase) >= 1);
}

static int fastwriteb(SerialPort *port)
{
    vx86_UartFastWrite8(port->com, port->fabase, (unsigned char)PopQueue(port->xmit));
    
    return 1;
}

static bool fastTqueryw(SerialPort *port)
{
    return (QueueSize(port->xmit) >= 2) && (vx86_GetUartTxFIFOFree(port->com, port->fabase) >= 2);
}

static int fastwritew(SerialPort *port)
{
    unsigned short value;
    
    value = PopQueue(port->xmit) & 0xFF;
    value |= ((unsigned short)(PopQueue(port->xmit) & 0xFF) << 8);
    
    vx86_UartFastWrite16(port->com, port->fabase, value);
    
    return 2;
}

static bool fastTqueryd(SerialPort *port)
{
    return (QueueSize(port->xmit) >= 4) && (vx86_GetUartTxFIFOFree(port->com, port->fabase) >= 4);
}

static int fastwrited(SerialPort *port)
{
    unsigned long value;
    
    value = PopQueue(port->xmit) & 0xFF;
    value |= ((unsigned long)(PopQueue(port->xmit) & 0xFF) <<  8);
    value |= ((unsigned long)(PopQueue(port->xmit) & 0xFF) << 16);
    value |= ((unsigned long)(PopQueue(port->xmit) & 0xFF) << 24);
    
    vx86_UartFastWrite(port->com, port->fabase, value);
    
    return 4;
}

static bool auto_Tquery(SerialPort *port)
{
    unsigned long size;
    int free;
    
    free = vx86_GetUartTxFIFOFree(port->com, port->fabase);
    size = QueueSize(port->xmit);
    
    if (free >= 4 && size >= 4) {
        
        port->intwrite = fastwrited;
        return true;
        
    } else if (free >= 2 && size >= 2) {
        
        port->intwrite = fastwritew;
        return true;
        
    } else if (free >= 1 && size >= 1) {
        
        port->intwrite = fastwriteb;
        return true;
        
    }
    
    return false;
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
    port->fabase = vx86_GetUartFastAccessBase();
    port->gibase = 0xFFFFFFFFUL;
	
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
																			 
	port->old_lsb        = port->lsb        = 0;
	port->old_msb        = port->msb        = 0;
	port->old_ier        = port->ier        = 0;
	port->old_lcr        = port->lcr        = 0;
	port->old_mcr        = port->mcr        = 0;
	port->old_RxTimeOut  = port->RxTimeOut  = UART_NO_TIMEOUT;
	port->old_TxTimeOut  = port->TxTimeOut  = UART_NO_TIMEOUT;
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
	port->old_lsb       = port->lsb       = _16550_DLAB_In(port, port->DLLSB);
	port->old_msb       = port->msb       = _16550_DLAB_In(port, port->DLMSB);
	port->old_ier       = port->ier       = io_inpb(port->IER); 
	port->old_lcr       = port->lcr       = io_inpb(port->LCR); 
	port->old_mcr       = port->mcr       = io_inpb(port->MCR);
	port->old_RxTimeOut = port->RxTimeOut = UART_NO_TIMEOUT;
	port->old_TxTimeOut = port->TxTimeOut = UART_NO_TIMEOUT;

	port->recv = _recv_int;
	port->send = _send_int;
    
	port->intread = native_read;
	port->intwrite = native_write;
	port->intRxQuery = native_Rquery;
	port->intTxQuery = native_Tquery;
    uart_SetFastReadMode(vport, FAST_READ_AUTO);
    uart_SetFastWriteMode(vport, FAST_WRITE_AUTO);
	
	// UART initial
	uart_SetBaud(vport, UARTBAUD_115200BPS);
	uart_SetFormat(vport, BYTESIZE8 + STOPBIT1 + NOPARITY);
	uart_SetTimeOut(vport, UART_NO_TIMEOUT, UART_NO_TIMEOUT);

	// flush rx & tx queue
	uart_FlushRxQueue(vport);
	uart_FlushTxQueue(vport);

	// clear Hardware RX & TX FIFO
	uart_ClearRFIFO(vport); 
	uart_ClearWFIFO(vport); 

	// enable FIFO mode
	uart_SetHWFIFO(vport, ENABLE_HW_TURBO_FIFO); 
	
	// set flow control
	uart_SetFlowControl(vport, NO_CONTROL);
	
	// enable receive data interrupt
	uart_IntEnable(vport, IERALL);

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
		uart_SetTimeOut(vport, port->old_RxTimeOut, port->old_TxTimeOut);
		
		vx86_uart_Close(port->com);
		if (io_Close() == false) err_print((char*)"Close IO lib error!!\n");
	}
	uart_Free(port);
}

DMPAPI(bool) uart_SetBaud(void *vport, unsigned long baud)
{
	unsigned char lcr;
	unsigned long divisor;
	SerialPort *port = (SerialPort *)vport;
	
	if (port == NULL) { err_print((char*)"%s: port is null.\n", __FUNCTION__); return false; }

	divisor = vx86_GetUartDiv(port->com, baud);
	if (divisor == 0) {
		err_print((char*)"%s: Unsupported baud rate.\n", __FUNCTION__);
		return false;
	}
	
	port->lsb = (unsigned char)(divisor & 0x00FFUL);
	port->msb = (unsigned char)((divisor & 0xFF00UL) >> 8);

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

DMPAPI(void) uart_SetTimeOut(void *vport, unsigned long rx_timeout, unsigned long tx_timeout)
{
	SerialPort *port = (SerialPort *)vport;
	if (port == NULL) { err_print((char*)"%s: port is null.\n", __FUNCTION__); return; }
	
	port->RxTimeOut = rx_timeout;
	port->TxTimeOut = tx_timeout;
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
		
		case ENABLE_HW_NORMAL_FIFO:
		{
			vx86_uart_DisableTurboFIFO(port->com);
			_16550_DLAB_Out(port, port->FCR, port->fcr |= 0x01);
			port->RFIFO_Size = FIFO_SIZE_16;
			port->WFIFO_Size = FIFO_SIZE_16;
		}
		break;
		
		case ENABLE_HW_TURBO_FIFO:
		{
			vx86_uart_EnableTurboFIFO(port->com);
			_16550_DLAB_Out(port, port->FCR, port->fcr |= 0x01);
			port->RFIFO_Size = vx86_uart_TruboFIFOSize(port->com);
			port->WFIFO_Size = vx86_uart_TruboFIFOSize(port->com);
		}
		break;
	};
}

DMPAPI(void) uart_ClearRFIFO(void *vport)   
{
	SerialPort *port = (SerialPort *)vport;
	if (port == NULL) { err_print((char*)"%s: port is null.\n", __FUNCTION__); return; }
	
	_16550_DLAB_Out(port, port->FCR, port->fcr | 0x02);
}

DMPAPI(void) uart_ClearWFIFO(void *vport)
{
	SerialPort *port = (SerialPort *)vport;
	if (port == NULL) { err_print((char*)"%s: port is null.\n", __FUNCTION__); return; }
	
	_16550_DLAB_Out(port, port->FCR, port->fcr | 0x04);
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
	return QueueSize(port->rcvd);
}
DMPAPI(bool) uart_RxQueueFull(void *vport)
{
	SerialPort *port = (SerialPort *)vport;
	return QueueFull(port->rcvd);
}
DMPAPI(bool) uart_RxQueueEmpty(void *vport)
{
	SerialPort *port = (SerialPort *)vport;
	return QueueEmpty(port->rcvd);
}

DMPAPI(int) uart_Receive(void *vport, unsigned char* buf, int bsize)
{
	SerialPort *port = (SerialPort *)vport;
	return (port == NULL) ? (0) : (port->recv(port, buf, bsize));
}

DMPAPI(unsigned int) uart_Read(void *vport)
{
	unsigned char val;
	
	if (uart_Receive(vport, &val, 1) <= 0)
		return 0xffff;
	
	return (unsigned int)val;
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
	return QueueSize(port->xmit);
}

DMPAPI(bool) uart_TxQueueFull(void *vport)
{
	SerialPort *port = (SerialPort *)vport;
	return QueueFull(port->xmit);
}

DMPAPI(bool) uart_TxQueueEmpty(void *vport)
{
	SerialPort *port = (SerialPort *)vport;
	return QueueEmpty(port->xmit);
}

DMPAPI(bool) uart_TxReady(void *vport)
{
	return (uart_TxQueueFull(vport) == false) ? (true) : (false);
}

DMPAPI(int) uart_Send(void *vport, unsigned char* buf, int bsize)
{
	SerialPort *port = (SerialPort *)vport;
	return (port == NULL) ? (0) : (port->send(port, buf, bsize));
}

DMPAPI(bool) uart_Write(void *vport, unsigned char val)
{
	if (uart_Send(vport, &val, 1) <= 0)
		return false;
	
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
	while (QueueEmpty(port->xmit) == false);
	while ((io_inpb(port->LSR) & 0x60) != 0x60);
}

static int UART_ISR(int irq, void *data)
{
	volatile int i, s, t;
	unsigned char iir;
	SerialPort *port;
	
	int ISR_Status = ISR_NONE;

	port  = (SerialPort *)data;
    
    if (port->gibase != 0xFFFFFFFFUL) {
    
        if (!(io_inpdw(port->gibase) & (1UL << port->com))) {
            
            return ISR_Status;
            
        }
    
    }

	while (!((iir = io_inpb(port->IIR)) & 0x01)) {
        
        ISR_Status = ISR_HANDLED;
        
        port->lsr = io_inpb(port->LSR);
        if (port->lsr & (DR | BI)) {
            
            t = 0;
            while (port->intRxQuery(port) && t < 64) {
                
                s = port->intread(port, port->RxHwBuf);
                
                for (i = 0; i < s; i++) {
                    
                    PushQueue(port->rcvd, port->RxHwBuf[i]);
                    
                }
                
                t += s;
            
            }
            
        }
        
        if (!(iir & 0x0F)) {
            
            port->msr = io_inpb(port->MSR);
            
        }
        
        if ((port->ier & THREIE) && (port->lsr & THRE)) {
            
            if (port->intTxQuery(port)) {

                i = 0;
                do {
                    
                    i += port->intwrite(port);
                    
                } while (port->intTxQuery(port) && i < port->WFIFO_Size);
                
            } else {
                
                io_DisableINT();
                io_outpb(port->IER, port->ier &= 0xfd);
                io_RestoreINT();
                
            }
            
        }
    }

	return ISR_Status;
}

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

DMPAPI(void) uart_EnableDebugMode(void *vport)
{
	SerialPort *port = (SerialPort *)vport;
	if (port == NULL) { err_print((char*)"%s: port is null.\n", __FUNCTION__); return; }
	
	io_outpb(port->MCR, io_inpb(port->MCR) & ~(0x08));
	port->recv = _recv_poll;
	port->send = _send_poll;
}

DMPAPI(void) uart_DisableDebugMode(void *vport)
{
	SerialPort *port = (SerialPort *)vport;
	if (port == NULL) { err_print((char*)"%s: port is null.\n", __FUNCTION__); return; }
	
	port->recv = _recv_int;
	port->send = _send_int;
	io_outpb(port->MCR, io_inpb(port->MCR) | 0x08);
}

DMPAPI(void) uart_SetFastReadMode(void *vport, int mode)
{
	SerialPort *port = (SerialPort *)vport;
	if (port == NULL) { err_print((char*)"%s: port is null.\n", __FUNCTION__); return; }
    
    
    io_DisableINT();
    
    if (port->fabase != 0xFFFFFFFFUL) {
        
        switch (mode) {
        case FAST_READ_AUTO:
            port->intRxQuery = auto_Rquery;
            port->intread = native_read;
            break;
        case NATIVE_READ:
            port->intRxQuery = native_Rquery;
            port->intread = native_read;
            break;
        case FAST_READ_MODE0W:
            port->intRxQuery = fastRquery0w;
            port->intread = fastread0w;
            break;
        case FAST_READ_MODE0D:
            port->intRxQuery = fastRquery0d;
            port->intread = fastread0d;
            break;
        case FAST_READ_MODE1B:
            port->intRxQuery = fastRquery1b;
            port->intread = fastread1b;
            break;
        case FAST_READ_MODE1W:
            port->intRxQuery = fastRquery1w;
            port->intread = fastread1w;
            break;
        case FAST_READ_MODE1D:
            port->intRxQuery = fastRquery1d;
            port->intread = fastread1d;
            break;
        default:
            break;
        };
        
    }
    
    io_RestoreINT();
}

DMPAPI(void) uart_SetFastWriteMode(void *vport, int mode)
{
	SerialPort *port = (SerialPort *)vport;
	if (port == NULL) { err_print((char*)"%s: port is null.\n", __FUNCTION__); return; }
    
    io_DisableINT();
    
    if (port->fabase != 0xFFFFFFFFUL) {
        
        switch (mode) {
        case FAST_WRITE_AUTO:
            port->intTxQuery = auto_Tquery;
            port->intwrite = native_write;
            break;
        case NATIVE_WRITE:
            port->intTxQuery = native_Tquery;
            port->intwrite = native_write;
            break;
        case FAST_WRITE_B:
            port->intTxQuery = fastTqueryb;
            port->intwrite = fastwriteb;
            break;
        case FAST_WRITE_W:
            port->intTxQuery = fastTqueryw;
            port->intwrite = fastwritew;
            break;
        case FAST_WRITE_D:
            port->intTxQuery = fastTqueryd;
            port->intwrite = fastwrited;
            break;
        default:
            break;
        };
    
    }
    
    io_RestoreINT();
}

DMPAPI(void) uart_CheckGlobalInterruptEnable(void *vport)
{
	SerialPort *port = (SerialPort *)vport;
	if (port == NULL) { err_print((char*)"%s: port is null.\n", __FUNCTION__); return; }
    
    io_DisableINT();
    if (port->fabase != 0xFFFFFFFFUL) {
        
        port->gibase = 0xF400;
        io_outpw(port->fabase + 0x28, port->gibase | 0x0001);
        
    }
    io_RestoreINT();
    
    printf("Enable UART Global Interrupt Check.\n");
}

DMPAPI(void) uart_CheckGlobalInterruptDisable(void *vport)
{
	SerialPort *port = (SerialPort *)vport;
	if (port == NULL) { err_print((char*)"%s: port is null.\n", __FUNCTION__); return; }
    
    io_DisableINT();
    if (port->fabase != 0xFFFFFFFFUL) {
        
        port->gibase = 0xFFFFFFFFUL;
        io_outpw(port->fabase + 0x28, 0x00);
        
    }
    io_RestoreINT();
    
    printf("Disable UART Global Interrupt Check.\n");
}
