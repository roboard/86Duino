/*
  usbcom.h - DM&P Vortex86 USB Device Library
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
#ifndef __USB_COM_H
#define __USB_COM_H

#include "dmpcfg.h"
#include "usb.h"
#include "usbdev.h"
#include "usbcdc.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct usb_cdc_line_coding Line_Coding;
typedef struct usb_dev {
    
    USBDev *dev;
    USBCDC *cdc;

    bool (*reinitialize)(struct usb_dev *);
    void (*uninitialize)(struct usb_dev *);
    void (*suspend)(void);
    void (*connect)(void);
    void (*disconnect)(void);
    
    unsigned long RxTimeOut;
    unsigned long TxTimeOut;

} USB_Device;

DMPAPI(void *) CreateUSBDevice(bool (*reinitialize)(struct usb_dev *), void (*uninitialize)(struct usb_dev *), void (*suspend)(void), void (*connect)(void), void (*disconnect)(void));
DMPAPI(void) DestoryUSBDevice(void *vusb);
DMPAPI(bool) usb_Init(void *vusb);
DMPAPI(void) usb_Close(void *vusb);
DMPAPI(bool) usb_Ready(void *vusb);

DMPAPI(void) usb_SetTimeOut(void *vusb, unsigned long rx_timeout, unsigned long tx_timeout);
#define USB_NO_TIMEOUT  (~0UL)
DMPAPI(int) usb_ReadBreak(void *vusb);
DMPAPI(unsigned long) usb_GetBaud(void *vusb);
DMPAPI(unsigned char) usb_GetStopBits(void *vusb);
DMPAPI(unsigned char) usb_GetParityType(void *vusb);
DMPAPI(unsigned char) usb_GetDataBits(void *vusb);
DMPAPI(bool) usb_GetDTR(void *vusb);
DMPAPI(bool) usb_GetRTS(void *vusb);
DMPAPI(bool) usb_Connect(void *vusb);
DMPAPI(bool) usb_Disconnect(void *vusb);
DMPAPI(bool) usb_SendBreak(void *vusb, int duration);

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

DMPAPI(void) usb_SetControlLineStateCallback(void *vusb, void (*func)(void));
DMPAPI(void) usb_SetDataInCallback(void *vusb, void (*func)(void));
DMPAPI(void) usb_SetDataOutCallback(void *vusb, void (*func)(void));
DMPAPI(void) usb_SetSOFCallback(void *vusb, void (*func)(void));
DMPAPI(void) usb_EnableSOFInterrupt(void *vusb);
DMPAPI(void) usb_DisableSOFInterrupt(void *vusb);

#ifdef __cplusplus
}
#endif

#endif
