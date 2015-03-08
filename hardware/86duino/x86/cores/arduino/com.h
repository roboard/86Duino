/*
  com.h - DM&P Vortex86 Universal Serial library
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

#ifndef __DMP_COM_H
#define __DMP_COM_H

#define COM_LIB_TIMEOUT_DEBUG    (0)

#include "dmpcfg.h"
#include "uart.h"
#include "USBCore.h"
#include "can.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
	int   com;
	void *func;
	
	DMPAPI(void) (*Close)(void *);
	DMPAPI(bool) (*SetBPS)(void *, unsigned long); // only for UART & CAN
	DMPAPI(void) (*SetTimeOut)(void *, unsigned long, unsigned long);
	
	DMPAPI(unsigned int) (*Read)(void *);
	DMPAPI(int)  (*Receive)(void *, unsigned char *, int);
	DMPAPI(int)  (*QueryRxQueue)(void *);
	DMPAPI(bool) (*RxQueueFull)(void *);
	DMPAPI(bool) (*RxQueueEmpty)(void *);
	DMPAPI(void) (*FlushRxQueue)(void *);

	DMPAPI(bool) (*Write)(void *, unsigned char);
	DMPAPI(int)  (*Send)(void *, unsigned char *, int);
	DMPAPI(int)  (*QueryTxQueue)(void *);
	DMPAPI(bool) (*TxQueueFull)(void *);
	DMPAPI(bool) (*TxQueueEmpty)(void *);
	DMPAPI(void) (*FlushTxQueue)(void *);
	DMPAPI(bool) (*TxReady)(void *);
	DMPAPI(void) (*FlushWFIFO)(void *);
	
	/* only for UART */
	DMPAPI(void) (*SetFormat)(void *, unsigned char);
	DMPAPI(void) (*SetFlowControl)(void *, int);
	DMPAPI(void) (*EnableFIFO)(void *, int);
	DMPAPI(bool) (*SetWFIFOSize)(void *, int);
	DMPAPI(void) (*ClearRFIFO)(void *);
	DMPAPI(void) (*ClearWFIFO)(void *);
	DMPAPI(void) (*SetLSRHandler)(void *, void (*)(SerialPort *));
	DMPAPI(void) (*SetMSRHandler)(void *, void (*)(SerialPort *));
	DMPAPI(unsigned char) (*GetLSR)(void *);
	DMPAPI(unsigned char) (*GetMSR)(void *);
	DMPAPI(void) (*EnableHalfDuplex)(void *);
	DMPAPI(void) (*EnableFullDuplex)(void *);
	DMPAPI(void) (*EnableDebugMode)(void *);
	DMPAPI(void) (*DisableDebugMode)(void *);
	
	/* only for USB device */
	DMPAPI(bool) (*Ready)(void *);
	DMPAPI(void) (*SetSerialState)(void *, unsigned short);
	DMPAPI(Line_Coding *) (*GetLineCoding)(void *);
	DMPAPI(unsigned short) (*GetControlLineState)(void *);
	
	/* only for CAN bus */
	DMPAPI(void) (*Reset)(void *);
	DMPAPI(bool) (*AddIDFilter)(void *, int, int, unsigned long, unsigned long);
	DMPAPI(bool) (*GetIDFilter)(void *, int, int *, unsigned long *, unsigned long *);
	DMPAPI(bool) (*DelIDFilter)(void *, int);
	DMPAPI(void) (*ClearIDList)(void *);
	DMPAPI(void) (*EnableBypass)(void *);
	DMPAPI(void) (*DisableBypass)(void *);
	DMPAPI(bool) (*SetEWLimit)(void *, int);
	DMPAPI(int)  (*GetEWLimit)(void *);
	DMPAPI(int)  (*GetTxErrorCount)(void *);
	DMPAPI(int)  (*GetRxErrorCount)(void *);
	DMPAPI(void) (*EnableStoreError)(void *);
	DMPAPI(void) (*DisableStoreError)(void *);
	DMPAPI(void) (*SetCANBusOffHandler)(void *, void (*)(CAN_Bus *));
	DMPAPI(unsigned char)  (*GetNowState)(void *);
	DMPAPI(unsigned char) (*PopError)(void *);
	DMPAPI(unsigned char) (*GetLastError)(void *);
	DMPAPI(bool) (*ReadCAN)(void *, CANFrame*);
	DMPAPI(bool) (*WriteCAN)(void *, CANFrame*);
	
} COMPort;

// Note that set USB-Device DETECT & ONOFF pins before init USB_COM
DMPAPI(void) com_SetUSBPins(char port1, char pin1, char port2, char pin2);

#define SIZE_OF_COM			(12) // (COM1 ~ COM10) + USB_COM + CAN_BUS

#define COM1			   	(0x00)
#define COM2			   	(0x01)
#define COM3			   	(0x02)
#define COM4			   	(0x03)
#define COM5			   	(0x04)
#define COM6			   	(0x05)
#define COM7			   	(0x06)
#define COM8			   	(0x07)
#define COM9			   	(0x08)
#define COM10				(0x09)
#define USB_COM			   	(0x0A)
#define CAN_BUS				(0x0B)
DMPAPI(COMPort *) com_Init(int com);
DMPAPI(void) com_Close(COMPort *port);

DMPAPI(bool) com_SetBPS(COMPort *port, unsigned long bps);
// for UART
#define COM_UARTBAUD_6000000BPS         (0xC001FFFFUL)
#define COM_UARTBAUD_3000000BPS         (0xC002FFFFUL)
#define COM_UARTBAUD_2000000BPS         (0xC003FFFFUL)
#define COM_UARTBAUD_1500000BPS         (0xC004FFFFUL)
#define COM_UARTBAUD_1000000BPS         (0xC006FFFFUL)
#define COM_UARTBAUD_750000BPS          (0xC0088002UL)
#define COM_UARTBAUD_500000BPS          (0xC00C8003UL)
#define COM_UARTBAUD_461538BPS          (0xC00DFFFFUL)
#define COM_UARTBAUD_333333BPS          (0xC012FFFFUL)
#define COM_UARTBAUD_300000BPS          (0xC0148005UL)
#define COM_UARTBAUD_250000BPS          (0x800C8006UL)
#define COM_UARTBAUD_200000BPS          (0x800FFFFFUL)
#define COM_UARTBAUD_150000BPS          (0x8014800AUL)
#define COM_UARTBAUD_125000BPS          (0x8018800CUL)
#define COM_UARTBAUD_115200BPS          (0x00010001UL)
#define COM_UARTBAUD_57600BPS           (0x00020002UL)
#define COM_UARTBAUD_38400BPS           (0x00030003UL)
#define COM_UARTBAUD_28800BPS           (0x00040004UL)
#define COM_UARTBAUD_19200BPS           (0x00060006UL)
#define COM_UARTBAUD_14400BPS           (0x00080008UL)
#define COM_UARTBAUD_9600BPS            (0x000C000CUL)
#define COM_UARTBAUD_4800BPS            (0x00180018UL)
#define COM_UARTBAUD_2400BPS            (0x00300030UL)
#define COM_UARTBAUD_1200BPS            (0x00600060UL)
#define COM_UARTBAUD_600BPS             (0x00C000C0UL)
#define COM_UARTBAUD_300BPS             (0x01800180UL)
#define COM_UARTBAUD_50BPS              (0x09000900UL)
// for CAN bus
#define COM_CAN_BPS_1000K               (CAN_BPS_1000K)
#define COM_CAN_BPS_833K                (CAN_BPS_833K)
#define COM_CAN_BPS_500K                (CAN_BPS_500K)
#define COM_CAN_BPS_250K                (CAN_BPS_250K)
#define COM_CAN_BPS_125K                (CAN_BPS_125K)
#define COM_CAN_BPS_100K                (CAN_BPS_100K)
#define COM_CAN_BPS_83K3                (CAN_BPS_83K3)
#define COM_CAN_BPS_50K                 (CAN_BPS_50K)
#define COM_CAN_BPS_20K                 (CAN_BPS_20K)
#define COM_CAN_BPS_10K                 (CAN_BPS_10K)
DMPAPI(void) com_SetTimeOut(COMPort *port, unsigned long rx_timeout, unsigned long tx_timeout);
#define NO_TIMEOUT         (-1)

DMPAPI(unsigned int) com_Read(COMPort *port); // not for CAN bus
DMPAPI(int)  com_Receive(COMPort *port, unsigned char* buf, int bsize);
DMPAPI(int)  com_QueryRxQueue(COMPort *port);
DMPAPI(bool) com_RxQueueFull(COMPort *port);
DMPAPI(bool) com_RxQueueEmpty(COMPort *port);
DMPAPI(void) com_FlushRxQueue(COMPort *port);

DMPAPI(bool) com_Write(COMPort *port, unsigned char val); // not for CAN bus
DMPAPI(int)  com_Send(COMPort *port, unsigned char* buf, int bsize);
DMPAPI(int)  com_QueryTxQueue(COMPort *port);
DMPAPI(bool) com_TxQueueFull(COMPort *port);
DMPAPI(bool) com_TxQueueEmpty(COMPort *port);
DMPAPI(void) com_FlushTxQueue(COMPort *port);
DMPAPI(bool) com_TxReady(COMPort *port);
DMPAPI(void) com_FlushWFIFO(COMPort *port);


/* only for UART */
DMPAPI(void) com_SetFormat(COMPort *port, unsigned char format);
#define BYTESIZE5          (0x00)
#define BYTESIZE6          (0x01)
#define BYTESIZE7          (0x02)
#define BYTESIZE8          (0x03)
#define STOPBIT1           (0x00)
#define STOPBIT2           (0x04)
#define NOPARITY           (0x00)
#define ODDPARITY          (0x08)
#define EVENPARITY         (0x18)
DMPAPI(void) com_SetFlowControl(COMPort *port, int control);
#define NO_CONTROL         (0)
#define RTS_CTS            (1)
#define XON_XOFF           (2)
DMPAPI(void) com_EnableFIFO(COMPort *port, int fifo);
#define FIFO_001		   (1)
#define FIFO_016		   (16)
#define FIFO_032		   (32)
#define FIFO_128		   (128)
DMPAPI(bool) com_SetWFIFOSize(COMPort *port, int size); // setup after enable/disable fifo
DMPAPI(void) com_ClearRFIFO(COMPort *port);
DMPAPI(void) com_ClearWFIFO(COMPort *port);
DMPAPI(void) com_SetLSRHandler(COMPort *port, void (*func)(SerialPort *port));
DMPAPI(void) com_SetMSRHandler(COMPort *port, void (*func)(SerialPort *port));
DMPAPI(unsigned char) com_GetLSR(COMPort *port);
#define LSR_EB             (0x80)
#define LSR_TEMT           (0x40)
#define LSR_THRE           (0x20)
#define LSR_BI             (0x10)
#define LSR_FE             (0x08)
#define LSR_PE             (0x04)
#define LSR_OE             (0x02)
#define LSR_DR             (0x01)
DMPAPI(unsigned char) com_GetMSR(COMPort *port);
#define MSR_DCD            (0x80)
#define MSR_RI             (0x40)
#define MSR_DSR            (0x20)
#define MSR_CTS            (0x10)
#define MSR_DDCD           (0x08)
#define MSR_TERI           (0x04)
#define MSR_DDSR           (0x02)
#define MSR_DCTS           (0x01)
DMPAPI(void) com_EnableHalfDuplex(COMPort *port);
DMPAPI(void) com_EnableFullDuplex(COMPort *port);
DMPAPI(void) com_EnableDebugMode(COMPort *port);
DMPAPI(void) com_DisableDebugMode(COMPort *port);

/* only for USB device */
DMPAPI(bool) com_Ready(COMPort *port);
DMPAPI(Line_Coding *) com_GetLineCoding(COMPort *port);
	// typedef struct {
	//	   DWORD dwDTERate;
	//	   BYTE  bCharFormat;
	//	   BYTE  bParityType;
	//	   BYTE  bDataBits;
	// } Line_Coding;
DMPAPI(void) com_SetSerialState(COMPort *port, unsigned short state);
	//  Serial State Bitmap
	//    7 - 15: reserved
	//    6:  bOverRun    overrun error
	//    5:  bParity     parity error
	//    4:  bFraming    framing error
	//    3:  bRingSignal RI
	//    2:  bBreak      break reception
	//    1:  bTxCarrier  DSR
	//    0:  bRxCarrier  DCD
DMPAPI(unsigned short) com_GetControlLineState(COMPort *port);

/* only for CAN bus */
DMPAPI(void) com_Reset(COMPort *port);
DMPAPI(bool) com_AddIDFilter(COMPort *port, int index, int ext_id, unsigned long filter, unsigned long mask);
DMPAPI(bool) com_GetIDFilter(COMPort *port, int index, int *ext_id, unsigned long *filter, unsigned long *mask);
DMPAPI(bool) com_DelIDFilter(COMPort *port, int index);
DMPAPI(void) com_ClearIDList(COMPort *port);
DMPAPI(void) com_EnableBypass(COMPort *port);
DMPAPI(void) com_DisableBypass(COMPort *port);
DMPAPI(bool) com_SetEWLimit(COMPort *port, int ewl);
DMPAPI(int)  com_GetEWLimit(COMPort *port);
DMPAPI(int)  com_GetTxErrorCount(COMPort *port);
DMPAPI(int)  com_GetRxErrorCount(COMPort *port);
DMPAPI(void) com_EnableStoreError(COMPort *port);
DMPAPI(void) com_DisableStoreError(COMPort *port);
DMPAPI(void) com_SetCANBusOffHandler(COMPort *port, void (*func)(CAN_Bus *));
DMPAPI(unsigned char)  com_GetNowState(COMPort *port);
DMPAPI(unsigned char) com_PopError(COMPort *port);
DMPAPI(unsigned char) com_GetLastError(COMPort *port);
DMPAPI(bool) com_ReadCAN(COMPort *port, CANFrame*);
DMPAPI(bool) com_WriteCAN(COMPort *port, CANFrame*);

#ifdef __cplusplus
}
#endif

#endif
