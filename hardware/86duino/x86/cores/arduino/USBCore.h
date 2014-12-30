/*
  usb.h - DM&P Vortex86 USB Device library
  Copyright (c) 2013 DY Hung <Dyhung@dmp.com.tw>. All right reserved.
  2014/06 Modified by Android Lin <acen@dmp.com.tw>.
  
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

#include "USBDesc.h"
#include "queue.h"

#define DMP_86DUINO_MODE
#if COM_LIB_TIMEOUT_DEBUG != 0
#define USB_TIMEOUT_DEBUG  (1)
#else
#define USB_TIMEOUT_DEBUG  (0)
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned char  BYTE;
typedef unsigned short WORD;
typedef unsigned long  DWORD;

#ifdef DMP_DOS_WATCOM
#pragma pack(push, 1)
#endif

#ifdef DMP_DOS_DJGPP
typedef struct __attribute__((packed)) {
#else
typedef struct {
#endif
	DWORD dwDTERate;
	BYTE  bCharFormat;
	BYTE  bParityType;
	BYTE  bDataBits;
} Line_Coding;

#ifdef DMP_DOS_WATCOM
#pragma pack(pop)
#endif

typedef struct
{
	WORD  CtrlTR;                          // Control endpoint type register address
	WORD  InTR;                            // IN endpoint type register address
	WORD  OutTR;                           // OUT endpoint type register address
	
	WORD  SetupDLR;                        // Setup endpoint data buffer register address
	WORD  InDLR;                           // IN endpoint data length register address
	WORD  OutDLR;                          // OUT endpoint data length register address
	
	WORD  SetupDSR;                        // Setup endpoint data buffer register address
	WORD  InDSR;                           // IN endpoint data buffer register address
	WORD  OutDSR;                          // OUT endpoint data buffer register address
	
	BYTE  *SetupBuf;			           // Setup endpoint data buffer
	BYTE  *InBuf;				           // IN endpoint data buffer
	BYTE  *OutBuf;				           // OUT endpoint data buffer
	
	DWORD SetupPhysical;		           // Setup endpoint data buffer physical memory address
	DWORD InPhysical;			           // IN endpoint data buffer physical memory address
	DWORD OutPhysical;			           // OUT endpoint data buffer physical memory address
	
} Endpoint;

typedef union
{
	WORD Value;
	BYTE Val[2];
	
} UNION;

typedef struct
{
	BYTE  bmRequestType;
	BYTE  bRequest;
	UNION wValue;
	UNION wIndex;
	WORD  wLength;
	
} SetupPacket;

typedef volatile struct usb_device
{
	WORD  addr;							   // USB device IO base address
	BYTE  DevAddr;                         // USB device address
	int   nIRQ;							   // USB device IRQ number
	
	bool  ReadySetAddr;
	
	int	  state;
	bool  stall;
	
	int   InUse;
	int   IsSet;
	bool  setup_in_handled;
	bool  setup_out_handled;
	
	bool  bulk_in_transmitting;
	bool  interrupt_in_transmitting;
	
	DWORD RxTimeOut;
	DWORD TxTimeOut;

	BYTE  *InDataPtr;
	BYTE  *OutDataPtr;
	WORD  InDataSize;
	WORD  OutDataSize;

	Queue *rcvd;						   // RX queue
	Queue *xmit;						   // TX queue
	Queue *hidxmit;						   // HID TX queue 2014/06/17 modified: new
	
	SetupPacket Setup;                     // Setup packet

	Line_Coding ling_coding;			   // Line Coding Structure
	
	WORD  control_line_state;			   // Control Line State (host to device)
	WORD  serial_state;					   // Serial State (device to host)
	WORD  break_time;
	
	// ----------------------------------- //
	// USB device registers's base address //
	// ----------------------------------- //
	WORD DAR;							   // USB device address register
	WORD CFR;							   // Control function register
	WORD FNR;							   // Frame number register
	WORD IER;							   // Interrupt enable register
	WORD ISR;							   // Interrupt status register
	WORD TMR;							   // Test mode register
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

DMPAPI(void) usb_SetSerialState(void *vusb, WORD serial);
DMPAPI(void) usb_SetTimeOut(void *vusb, DWORD rx_timeout, DWORD tx_timeout);
#define USB_NO_TIMEOUT         (-1)
DMPAPI(Line_Coding *) usb_GetLineCoding(void *vusb);
DMPAPI(WORD) usb_GetControlLineState(void *vusb);

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
DMPAPI(int)  hid_Send(void *vusb, unsigned char* buf, int bsize); //2014/06/17 modified: new
DMPAPI(bool) usb_Write(void *vusb, unsigned char val);
DMPAPI(void) usb_FlushTxQueue(void *vusb);
DMPAPI(void) usb_FlushWFIFO(void *vusb);

#ifdef __cplusplus
}
#endif

#endif
