/*
  usbcom.cpp - DM&P Vortex86 USB Device Library
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
#include <stdlib.h>
#include <string.h>

#include "usbcom.h"

#include "usbdev.h"
#include "usbdesc.h"
#include "usbmsc.h"
#include "scsi.h"
#include "disk.h"
#include "usbcdc.h"
#include "io.h"

DMPAPI(void *) CreateUSBDevice(bool (*reinitialize)(struct usb_dev *), void (*uninitialize)(struct usb_dev *), void (*suspend)(void))
{
    USB_Device *usb;
    
    usb = (USB_Device *)malloc(sizeof(USB_Device));
    if (usb) {
        
        memset(usb, 0, sizeof(USB_Device));
        
        usb->RxTimeOut = USB_NO_TIMEOUT;
        usb->TxTimeOut = USB_NO_TIMEOUT;  
        
        usb->reinitialize = reinitialize;
        usb->uninitialize = uninitialize;
        usb->suspend = suspend;
        
    }

    return usb;
}

DMPAPI(void) DestoryUSBDevice(void *vusb)
{
    USB_Device *usb = (USB_Device *)vusb;
    
    free(vusb);
}

DMPAPI(bool) usb_Init(void *vusb)
{
    USB_Device *usb = (USB_Device *)vusb;
    int usb_class;
    
    if (io_Init() == false) {
    
        printf("ERROR: io_Init() failed.\n");
        return false;
    
    }
    
    usb_class = USB_FULL_SPEED | USB_CDC;
    if (vx86_CpuID() == CPU_VORTEX86EX2M || vx86_CpuID() == CPU_VORTEX86EX2S)
        usb_class |= USB_HIGH_SPEED;
    
    if (usb_desc_initialize(usb_class) == false) {
        
        printf("ERROR: usb_desc_initialize() failed.\n");
        io_Close();
        return false;
        
    }
    
    usb->dev = USBDev_Init();
    if (usb->dev == NULL) {
    
        printf("ERROR: USBDev_Init() failed.\n");
        usb_desc_uninitialize();
        io_Close();
        return false;
    
    }
    
    if (usb->reinitialize && usb->reinitialize(usb) == false) {
        
        printf("ERROR: usb->reinitialize() failed.\n");
        USBDev_Close(usb->dev);
        usb_desc_uninitialize();
        io_Close();
        return false;
        
    }
    
    usb->cdc = USBCDC_Init(usb->dev, usb->suspend);
    if (usb->cdc == NULL) {
    
        printf("ERROR: USBCDC_Init() failed.\n");
        if (usb->uninitialize)
            usb->uninitialize(usb);
        USBDev_Close(usb->dev);
        usb_desc_uninitialize();
        io_Close();
        return false;
    
    }
    
    return USBDev_Start(usb->dev);
}

DMPAPI(void) usb_Close(void *vusb)
{
    USB_Device *usb = (USB_Device *)vusb;
    
    USBDev_Stop(usb->dev);
    timer_Delay(1);
    USBCDC_Close(usb->cdc);
    
    if (usb->uninitialize)
        usb->uninitialize(usb);
    
    USBDev_Close(usb->dev);
    usb_desc_uninitialize();
    io_Close();
}

DMPAPI(bool) usb_Ready(void *vusb)
{
    USB_Device *usb = (USB_Device *)vusb;
    
    return USBCDC_CheckReady(usb->cdc);
}

DMPAPI(void) usb_SetTimeOut(void *vusb, unsigned long rx_timeout, unsigned long tx_timeout)
{
    USB_Device *usb = (USB_Device *)vusb;
    
    usb->RxTimeOut = rx_timeout;
    usb->TxTimeOut = tx_timeout;
}

DMPAPI(int) usb_ReadBreak(void *vusb)
{
    USB_Device *usb = (USB_Device *)vusb;
    
    return USBCDC_ReadBreak(usb->cdc);
}

DMPAPI(unsigned long) usb_GetBaud(void *vusb)
{
    USB_Device *usb = (USB_Device *)vusb;
    
    return USBCDC_GetBaud(usb->cdc);
}

DMPAPI(unsigned char) usb_GetStopBits(void *vusb)
{
    USB_Device *usb = (USB_Device *)vusb;
    
    return USBCDC_GetStopBits(usb->cdc);
}

DMPAPI(unsigned char) usb_GetParityType(void *vusb)
{
    USB_Device *usb = (USB_Device *)vusb;
    
    return USBCDC_GetParityType(usb->cdc);
}

DMPAPI(unsigned char) usb_GetDataBits(void *vusb)
{
    USB_Device *usb = (USB_Device *)vusb;
    
    return USBCDC_GetDataBits(usb->cdc);
}

DMPAPI(bool) usb_GetDTR(void *vusb)
{
    USB_Device *usb = (USB_Device *)vusb;
    
    return USBCDC_GetDTR(usb->cdc);
}

DMPAPI(bool) usb_GetRTS(void *vusb)
{
    USB_Device *usb = (USB_Device *)vusb;
    
    return USBCDC_GetRTS(usb->cdc);
}

DMPAPI(bool) usb_Connect(void *vusb)
{
    USB_Device *usb = (USB_Device *)vusb;
    
    return USBCDC_Connect(usb->cdc);
}

DMPAPI(bool) usb_Disconnect(void *vusb)
{
    USB_Device *usb = (USB_Device *)vusb;
    
    return USBCDC_Disconnect(usb->cdc);
}

DMPAPI(bool) usb_SendBreak(void *vusb, int duration)
{
    USB_Device *usb = (USB_Device *)vusb;
    
    return USBCDC_SendBreak(usb->cdc, duration);
}

DMPAPI(int) usb_QueryRxQueue(void *vusb)
{
    USB_Device *usb = (USB_Device *)vusb;
    
    return USBCDC_RFIFOSize(usb->cdc);
}

DMPAPI(bool) usb_RxQueueFull(void *vusb)
{
    USB_Device *usb = (USB_Device *)vusb;
    
    return USBCDC_RFIFOFree(usb->cdc) <= 0;
}

DMPAPI(bool) usb_RxQueueEmpty(void *vusb)
{
    USB_Device *usb = (USB_Device *)vusb;
    
    return USBCDC_RFIFOSize(usb->cdc) <= 0;
}

DMPAPI(int) usb_Receive(void *vusb, unsigned char* buf, int len)
{
    USB_Device *usb = (USB_Device *)vusb;
    int rc, cur = 0;
    unsigned long pre, now;
    
    if (usb->RxTimeOut != USB_NO_TIMEOUT)
        pre = timer_NowTime();
    
    while (cur < len) {
        
        rc = USBCDC_ReadData(usb->cdc, &buf[cur], len - cur);
        cur += rc;
        
        if (usb->RxTimeOut == USB_NO_TIMEOUT)
            continue;
        
        now = timer_NowTime();
        if (rc)
            pre = now;
        
        if (now - pre > usb->RxTimeOut)
            break;
        
    }
    
    return cur;
}

DMPAPI(unsigned int) usb_Read(void *vusb)
{
    unsigned char val;
    
    if (usb_Receive(vusb, &val, 1) <= 0)
        return 0xFFFF;
        
    return val;
}

DMPAPI(void) usb_FlushRxQueue(void *vusb)
{
    USB_Device *usb = (USB_Device *)vusb;
    
    USBCDC_RFIFOFlush(usb->cdc);
}

DMPAPI(int) usb_QueryTxQueue(void *vusb)
{
    USB_Device *usb = (USB_Device *)vusb;
    
    return USBCDC_WFIFOSize(usb->cdc);
}

DMPAPI(bool) usb_TxQueueFull(void *vusb)
{
    USB_Device *usb = (USB_Device *)vusb;
    
    return USBCDC_WFIFOFree(usb->cdc) <= 0;
}

DMPAPI(bool) usb_TxQueueEmpty(void *vusb)
{
    USB_Device *usb = (USB_Device *)vusb;
    
    return USBCDC_WFIFOSize(usb->cdc) <= 0;
}

DMPAPI(bool) usb_TxReady(void *vusb)
{
    return !usb_TxQueueFull(vusb);
}

DMPAPI(int) usb_Send(void *vusb, unsigned char* buf, int len)
{
    USB_Device *usb = (USB_Device *)vusb;
    int rc, cur = 0;
    unsigned long pre, now;
    
    if (usb->TxTimeOut != USB_NO_TIMEOUT)
        pre = timer_NowTime();
    
    while (cur < len) {
        
        rc = USBCDC_WriteData(usb->cdc, &buf[cur], len - cur);
        cur += rc;
        
        if (usb->TxTimeOut == USB_NO_TIMEOUT)
            continue;
        
        now = timer_NowTime();
        if (rc)
            pre = now;
        
        if (now - pre > usb->TxTimeOut)
            break;
        
    }
    
    return cur;
}

DMPAPI(bool) usb_Write(void *vusb, unsigned char val)
{
    if (usb_Send(vusb, &val, 1) <= 0)
        return false;
        
    return true;
}

DMPAPI(void) usb_FlushTxQueue(void *vusb)
{
    USB_Device *usb = (USB_Device *)vusb;
    
    USBCDC_WFIFOFlush(usb->cdc);
}

DMPAPI(void) usb_FlushWFIFO(void *vusb)
{
    USB_Device *usb = (USB_Device *)vusb;
    
    USBCDC_WFIFOWaitFlush(usb->cdc);
}

DMPAPI(void) usb_SetControlLineStateCallback(void *vusb, void (*func)(void))
{
    USB_Device *usb = (USB_Device *)vusb;
    
    USBCDC_SetControlLineStateCB(usb->cdc, func);
}

DMPAPI(void) usb_SetDataInCallback(void *vusb, void (*func)(void))
{
    USB_Device *usb = (USB_Device *)vusb;
    
    USBDev_SetDataInCB(usb->dev, func);
}

DMPAPI(void) usb_SetDataOutCallback(void *vusb, void (*func)(void))
{
    USB_Device *usb = (USB_Device *)vusb;
    
    USBDev_SetDataOutCB(usb->dev, func);
}

DMPAPI(void) usb_SetSOFCallback(void *vusb, void (*func)(void))
{
    USB_Device *usb = (USB_Device *)vusb;
    
    USBDev_SetSOFCB(usb->dev, func);
}

DMPAPI(void) usb_EnableSOFInterrupt(void *vusb)
{
    USB_Device *usb = (USB_Device *)vusb;
    
    USBDev_EnableSOFInterrupt(usb->dev);
}

DMPAPI(void) usb_DisableSOFInterrupt(void *vusb)
{
    USB_Device *usb = (USB_Device *)vusb;
    
    USBDev_DisableSOFInterrupt(usb->dev);
}

