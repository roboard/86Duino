/*
  usb.h - DM&P Vortex86 USB Device library
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

#ifndef __DMP_USB_H
#define __DMP_USB_H

#include "dmpcfg.h"

#include "usb_desc.h"
#include "queue.h"

#if COM_LIB_TIMEOUT_DEBUG != 0
#define USB_TIMEOUT_DEBUG  (1)
#else
#define USB_TIMEOUT_DEBUG  (0)
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifdef DMP_DOS_WATCOM
#pragma pack(push, 1)
#endif

#ifdef DMP_DOS_DJGPP
typedef struct __attribute__((packed)) {
#else
typedef struct {
#endif
	unsigned long dwDTERate;
	unsigned char bCharFormat;
	unsigned char bParityType;
	unsigned char bDataBits;
} Line_Coding;

#ifdef DMP_DOS_WATCOM
#pragma pack(pop)
#endif

typedef struct
{
	unsigned short CtrlTR;                          // Control endpoint type register address
	unsigned short InTR;                            // IN endpoint type register address
	unsigned short OutTR;                           // OUT endpoint type register address
	
	unsigned short SetupDLR;                        // Setup endpoint data buffer register address
	unsigned short InDLR;                           // IN endpoint data length register address
	unsigned short OutDLR;                          // OUT endpoint data length register address
	
	unsigned short SetupDSR;                        // Setup endpoint data buffer register address
	unsigned short InDSR;                           // IN endpoint data buffer register address
	unsigned short OutDSR;                          // OUT endpoint data buffer register address
	
	unsigned char *SetupBuf;			            // Setup endpoint data buffer
	unsigned char *InBuf;				            // IN endpoint data buffer
	unsigned char *OutBuf;				            // OUT endpoint data buffer
	
	unsigned long  SetupPhysical;		            // Setup endpoint data buffer physical memory address
	unsigned long  InPhysical;			            // IN endpoint data buffer physical memory address
	unsigned long  OutPhysical;			            // OUT endpoint data buffer physical memory address
	
} Endpoint;

typedef union
{
	unsigned short Value;
	unsigned char  Val[2];
	
} UNION;

typedef struct
{
	unsigned char bmRequestType;
	unsigned char bRequest;
	UNION wValue;
	UNION wIndex;
	unsigned short wLength;
	
} SetupPacket;

typedef volatile struct usb_device
{
	unsigned short addr;							   // USB device IO base address
	unsigned char DevAddr;                         // USB device address
	int  nIRQ;							   // USB device IRQ number
	
	bool ReadySetAddr;
	
	int	 state;
	bool stall;
	
	int  InUse;
	int  IsSet;
	bool setup_in_handled;
	bool setup_out_handled;
	
	bool bulk_in_transmitting;
	
	unsigned long RxTimeOut;
	unsigned long TxTimeOut;

	unsigned char *InDataPtr;
	unsigned char *OutDataPtr;
	unsigned short InDataSize;
	unsigned short OutDataSize;

	Queue *rcvd;						   // RX queue
	Queue *xmit;						   // TX queue
	
	SetupPacket Setup;                     // Setup packet

	Line_Coding ling_coding;			   // Line Coding Structure
	
	unsigned short control_line_state;			   // Control Line State (host to device)
	unsigned short serial_state;					   // Serial State (device to host)
	unsigned short break_time;
	
	// ----------------------------------- //
	// USB device registers's base address //
	// ----------------------------------- //
	unsigned short DAR;							   // USB device address register
	unsigned short CFR;							   // Control function register
	unsigned short FNR;							   // Frame number register
	unsigned short IER;							   // Interrupt enable register
	unsigned short ISR;							   // Interrupt status register
	unsigned short TMR;							   // Test mode register
	Endpoint EP[4];
	 
} USB_Device;

DMPAPI(void *) CreateUSBDevice(void);
DMPAPI(bool) usb_Init(void *vusb);
DMPAPI(void) usb_Close(void *vusb);
DMPAPI(int) usb_State(void *vusb);
#define USB_DEV_NOT_ATTACHED (0x00)
#define USB_DEV_ATTACHED     (0x01)
#define USB_DEV_POWERED      (0x02)
#define USB_DEV_DEFAULT      (0x03)
#define USB_DEV_ADDRESS      (0x04)
#define USB_DEV_CONFIGURED   (0x05)
#define USB_DEV_SUSPENDED    (0x06)
#define USB_DEV_CDC_CONNECT  (0x07)
DMPAPI(bool) usb_Ready(void *vusb);
DMPAPI(bool) usb_SetUSBPins(void *vusb, char port1, char pin1, char port2, char pin2);

DMPAPI(void) usb_SetSerialState(void *vusb, unsigned short serial);
DMPAPI(void) usb_SetTimeOut(void *vusb, unsigned long rx_timeout, unsigned long tx_timeout);
#define USB_NO_TIMEOUT         (-1)
DMPAPI(Line_Coding *) usb_GetLineCoding(void *vusb);
DMPAPI(unsigned short) usb_GetControlLineState(void *vusb);

DMPAPI(int) usb_QueryRxQueue(void *vusb);
DMPAPI(bool) usb_RxQueueFull(void *vusb);
DMPAPI(bool) usb_RxQueueEmpty(void *vusb);
DMPAPI(unsigned int) usb_Read(void *vusb);
DMPAPI(int) usb_Receive(void *vusb, unsigned char* buf, int bsize);
DMPAPI(void) usb_FlushRxQueue(void *vusb);

DMPAPI(int) usb_QueryTxQueue(void *vusb);
DMPAPI(bool) usb_TxQueueFull(void *vusb);
DMPAPI(bool) usb_TxQueueEmpty(void *vusb);
DMPAPI(bool) usb_TxReady(void *vusb);
DMPAPI(int)  usb_Send(void *vusb, unsigned char* buf, int bsize);
DMPAPI(bool) usb_Write(void *vusb, unsigned char val);
DMPAPI(void) usb_FlushTxQueue(void *vusb);
DMPAPI(void) usb_FlushWFIFO(void *vusb);

#ifdef __cplusplus
}
#endif

#endif
