/*
  uart.h - DM&P Vortex86 UART library
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

#ifndef __DMP_UART_H
#define __DMP_UART_H

#include "dmpcfg.h"
#include "vortex86.h"
#include "queue.h"

#if COM_LIB_TIMEOUT_DEBUG != 0
#define UART_TIMEOUT_DEBUG  (1)
#else
#define UART_TIMEOUT_DEBUG  (0)
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef volatile struct serial_port
{
	unsigned short addr;				// comport base address
	int            com; 				// comport number
	int            nIRQ;				// comport IRQ number
	
	unsigned long  nBuad;				// baud rate
	int            nData;				// number of data bit
	int            nStop;				// number of stop bit
	int            nParity;				// parity
	int            control;				// flow control selection
	
	int InUse;
	int INT_InUse;
	
	int rts;							// for hardware flow control
	int cts;							//
	
	int xonxoff_rcvd;                   // for software flow control
	int xonxoff_xmit;                   //
	int xon;                            //
	int xoff;                           //
																			 
	unsigned char old_lsb;				// record the configuration of comport
	unsigned char old_msb;				//
	unsigned char old_ier;				//
	unsigned char old_lcr;				//
	unsigned char old_mcr;				//
	unsigned long old_RxTimeOut;		//
	unsigned long old_TxTimeOut;		//

	unsigned char lsb;					// the configuration of comport at now time
	unsigned char msb;					//
	unsigned char ier;					//
	unsigned char lcr;					//
	unsigned char mcr;					//
	unsigned char fcr;					//
	unsigned char lsr;					//
	unsigned char msr;					//
	unsigned long RxTimeOut;			//
	unsigned long TxTimeOut;			//
	
	int RFIFO_Size;						// comport FIFO size
	int WFIFO_Size;						//

	Queue *rcvd;						// RX queue
	Queue *xmit;						// TX queue
	
	// LSR & MSR interrupt handler
	void (*lsr_handler)(volatile struct serial_port *port);
	void (*msr_handler)(volatile struct serial_port *port);
	
	// -------------------------------- //
	// comport registers's base address //
	// -------------------------------- //
	unsigned short TXDB;				// DLAB = 0, Transmit Data Buffer
	unsigned short RXDB;				// DLAB = 0, Receive Data Buffer
	unsigned short DLLSB;				// DLAB = 1, LSB of Baud Rate generator Divisor Latches
	unsigned short DLMSB;				// DLAB = 1, MSB of Baud Rate generator Divisor Latches
	unsigned short IER;					// DLAB = 0, Interrupt Enable Register
	unsigned short IIR;					// DLAB = 0, Interrupt Identification Register
	unsigned short FCR;					// DLAB = 1, FIFO Control Register
	unsigned short LCR;					// Line Control Register
	unsigned short MCR;					// Modem Control Register
	unsigned short LSR;					// Line Status Register
	unsigned short MSR;					// Modem Status Register
	unsigned short SCR;					// Scratchpad Register
	
	// -------------------------------- //
	//         Debug Serial Mode        //
	// -------------------------------- //
	int (*recv)(volatile struct serial_port *port, unsigned char* buf, int bsize);
	int (*send)(volatile struct serial_port *port, unsigned char* buf, int bsize);
	 
} SerialPort;

DMPAPI(void *) CreateUART(int com);
DMPAPI(bool) uart_Init(void *vport);
DMPAPI(void) uart_Close(void *vport);
// com
#define UART_COM1       (0)
#define UART_COM2       (1)
#define UART_COM3       (2)
#define UART_COM4       (3)
#define UART_COM5       (4)
#define UART_COM6       (5)
#define UART_COM7       (6)
#define UART_COM8       (7)
#define UART_COM9       (8)
#define UART_COM10      (9)
// mode
// #define MAX_BAUD_6000000			(1)
// #define MAX_BAUD_3000000			(2)
// #define MAX_BAUD_750000 			(3)
// #define MAX_BAUD_115200A 		(4)
// #define MAX_BAUD_115200B			(5)

DMPAPI(bool) uart_SetBaud(void *vport, unsigned long baud);
#define UARTBAUD_6000000BPS         (0xC001FFFFUL)
#define UARTBAUD_3000000BPS         (0xC002FFFFUL)
#define UARTBAUD_2000000BPS         (0xC003FFFFUL)
#define UARTBAUD_1500000BPS         (0xC004FFFFUL)
#define UARTBAUD_1000000BPS         (0xC006FFFFUL)
#define UARTBAUD_750000BPS          (0xC0088002UL)
#define UARTBAUD_500000BPS          (0xC00C8003UL)
#define UARTBAUD_461538BPS          (0xC00DFFFFUL)
#define UARTBAUD_333333BPS          (0xC012FFFFUL)
#define UARTBAUD_300000BPS          (0xC0148005UL)
#define UARTBAUD_250000BPS          (0x800C8006UL)
#define UARTBAUD_200000BPS          (0x800FFFFFUL)
#define UARTBAUD_150000BPS          (0x8014800AUL)
#define UARTBAUD_125000BPS          (0x8018800CUL)
#define UARTBAUD_115200BPS          (0x00010001UL)
#define UARTBAUD_57600BPS           (0x00020002UL)
#define UARTBAUD_38400BPS           (0x00030003UL)
#define UARTBAUD_28800BPS           (0x00040004UL)
#define UARTBAUD_19200BPS           (0x00060006UL)
#define UARTBAUD_14400BPS           (0x00080008UL)
#define UARTBAUD_9600BPS            (0x000C000CUL)
#define UARTBAUD_4800BPS            (0x00180018UL)
#define UARTBAUD_2400BPS            (0x00300030UL)
#define UARTBAUD_1200BPS            (0x00600060UL)
#define UARTBAUD_600BPS             (0x00C000C0UL)
#define UARTBAUD_300BPS             (0x01800180UL)
#define UARTBAUD_50BPS              (0x09000900UL)
DMPAPI(void) uart_SetFormat(void *vport, unsigned char format);
#define BYTESIZE5          (0x00)
#define BYTESIZE6          (0x01)
#define BYTESIZE7          (0x02)
#define BYTESIZE8          (0x03)
#define STOPBIT1           (0x00)
#define STOPBIT2           (0x04)
#define NOPARITY           (0x00)
#define ODDPARITY          (0x08)
#define EVENPARITY         (0x18)
DMPAPI(void) uart_SetFlowControl(void *vport, int control);
#define NO_CONTROL         (0)
#define RTS_CTS            (1)
#define XON_XOFF           (2)
DMPAPI(void) uart_SetTimeOut(void *vport, unsigned long rx_timeout, unsigned long tx_timeout);
#define UART_NO_TIMEOUT         (-1)
DMPAPI(unsigned char) uart_GetLSR(void *vport);
DMPAPI(unsigned char) uart_GetMSR(void *vport);

DMPAPI(void) uart_SetHWFIFO(void *vport, int fifo);
#define DISABLE_HW_FIFO		(1)
#define ENABLE_HW_FIFO_16	(16)
#define ENABLE_HW_FIFO_32	(32)
#define ENABLE_HW_FIFO_128	(128)
DMPAPI(bool) uart_SetWFIFOSize(void *vport, int size); // setup after enable/disable fifo

DMPAPI(void) uart_ClearRFIFO(void *vport);
DMPAPI(void) uart_ClearWFIFO(void *vport);
DMPAPI(void) uart_SetRFIFOTrig(void *vport, int cap);
#define RFIFO_TL_1         (0x00)
#define RFIFO_TL_4         (0x40)
#define RFIFO_TL_8         (0x80)
#define RFIFO_TL_14        (0xc0)

DMPAPI(int) uart_QueryRxQueue(void *vport);
DMPAPI(bool) uart_RxQueueFull(void *vport);
DMPAPI(bool) uart_RxQueueEmpty(void *vport);
DMPAPI(unsigned int) uart_Read(void *vport);
DMPAPI(int) uart_Receive(void *vport, unsigned char* buf, int bsize);
DMPAPI(void) uart_FlushRxQueue(void *vport);

DMPAPI(int) uart_QueryTxQueue(void *vport);
DMPAPI(bool) uart_TxQueueFull(void *vport);
DMPAPI(bool) uart_TxQueueEmpty(void *vport);
DMPAPI(bool) uart_TxReady(void *vport);
DMPAPI(int)  uart_Send(void *vport, unsigned char* buf, int bsize);
DMPAPI(bool) uart_Write(void *vport, unsigned char val);
DMPAPI(void) uart_FlushTxQueue(void *vport);
DMPAPI(void) uart_FlushWFIFO(void *vport);

DMPAPI(void) uart_IntEnable(void *vport, int cap);
DMPAPI(void) uart_IntDisable(void *vport);
// cap
#define IERALL             (0x0f)
#define MSI                (0x08)
#define RLSI               (0x04)
#define THREI              (0x02)
#define RDAI               (0x01)

DMPAPI(void) uart_SetLSRHandler(void *vport, void (*func)(SerialPort *));
DMPAPI(void) uart_SetMSRHandler(void *vport, void (*func)(SerialPort *));

DMPAPI(void) uart_EnableHalfDuplex(void *vport);
DMPAPI(void) uart_EnableFullDuplex(void *vport);

DMPAPI(void) uart_EnableDebugMode(void *vport);
DMPAPI(void) uart_DisableDebugMode(void *vport);

#ifdef __cplusplus
}
#endif

#endif
