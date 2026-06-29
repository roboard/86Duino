/*
  usbmsc.cpp - DM&P Vortex86 USB Device Mass Storage Clase Library
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
#include "usbmsc.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "io.h"
#include "usb.h"
#include "usbdesc.h"

struct usb_mass_storage {
    
    bool active;
    
    USBDev *dev;
    USBEP *in;
    USBEP *out;
    
    bool in_done;
    bool out_done;
    
    void (*suspend)(void);
    
    unsigned char nluns;
    
};

static void USBMSC_ControlRequest(void *data, struct usb_ctrlrequest *setup)
{
    struct usb_mass_storage *msc = (struct usb_mass_storage *)data;
    
    switch (setup->bRequest) {

    case USB_BULK_RESET_REQUEST:
        if (setup->bRequestType != (USB_DIR_OUT | USB_TYPE_CLASS | USB_RECIP_INTERFACE))
            break;
        if (setup->wValue != 0 || setup->wIndex != msc_interface.msc_data_interface.bInterfaceNumber || setup->wLength != 0)
            break;
        break;

    case USB_BULK_GET_MAX_LUN_REQUEST:
        if (setup->bRequestType != (USB_DIR_IN | USB_TYPE_CLASS | USB_RECIP_INTERFACE))
            break;
        if (setup->wValue != 0 || setup->wIndex != msc_interface.msc_data_interface.bInterfaceNumber || setup->wLength != 1)
            break;
        USBDev_StartTransfer(msc->dev->in, &msc->nluns, 1);
        break;
    }
}

static void USBMSC_Enable(void *data)
{
    struct usb_mass_storage *msc = (struct usb_mass_storage *)data;
    
    USBDev_EnableEndpoint(msc->in);
    USBDev_EnableEndpoint(msc->out);
    
    msc->active = true;
}

static void USBMSC_Suspend(void *data)
{
    struct usb_mass_storage *msc = (struct usb_mass_storage *)data;
    
    USBDev_DisableEndpoint(msc->in);
    USBDev_DisableEndpoint(msc->out);
    
    msc->active = false;
    
    if (msc->suspend) {
        
        io_DisableINT();
        msc->suspend();
        io_RestoreINT();
        
    }
}

static void USBDev_DataInComplete(void *data, int len)
{
    struct usb_mass_storage *msc = (struct usb_mass_storage *)data;
    
    msc->in_done = true;
}

static void USBDev_DataOutComplete(void *data, int len)
{
    struct usb_mass_storage *msc = (struct usb_mass_storage *)data;
    
    msc->out_done = true;
}

USBMSC *USBMSC_Init(USBDev *dev, void (*suspend)(void))
{
    USBMSC *msc;
    
    msc = (struct usb_mass_storage *)malloc(sizeof(struct usb_mass_storage));
    if (msc == NULL) {
    
        printf("ERROR: Alloc struct usb_mass_storage size fail.\n");
        return NULL;
    
    }
    memset(msc, 0, sizeof(struct usb_mass_storage));
    msc->nluns = 0;
    msc->active = false;
    msc->in_done = true;
    msc->out_done = true;
    msc->suspend = suspend;
    
    msc->dev = dev;
    
    if (!USBDev_ClassRegister(msc->dev, msc, USBMSC_ControlRequest, USBMSC_Enable, USBMSC_Suspend)) {
        
        printf("ERROR: USBDev_ClassRegister(MSC) fail.\n");
        free(msc);
        return NULL;
        
    }
    
    msc->in = USBDev_InitEndpoint(msc->dev, &msc_interface.msc_data_in, msc, USBDev_DataInComplete);
    if (msc->in == NULL) {
    
        printf("ERROR: USBDev_InitEndpoint(MSC->IN) fail.\n");
        USBDev_ClassUnRegister(msc->dev, msc);
        free(msc);
        return NULL;
    
    }
    
    msc->out = USBDev_InitEndpoint(msc->dev, &msc_interface.msc_data_out, msc, USBDev_DataOutComplete);
    if (msc->out == NULL) {
    
        printf("ERROR: USBDev_InitEndpoint(MSC->OUT) fail.\n");
        USBDev_DeinitEndpoint(msc->in);
        USBDev_ClassUnRegister(msc->dev, msc);
        free(msc);
        return NULL;
    
    }
    
    return msc;
}

void USBMSC_Close(USBMSC *msc)
{
    if (msc->out) {
        USBDev_DisableEndpoint(msc->out);
        USBDev_DeinitEndpoint(msc->out);
    }
    if (msc->in) {
        USBDev_DisableEndpoint(msc->in);
        USBDev_DeinitEndpoint(msc->in);
    }
    USBDev_ClassUnRegister(msc->dev, msc);
    free(msc);
}

bool USBMSC_CheckReady(USBMSC *msc)
{
    bool active;
    
    io_DisableINT();
    active = msc->active;
    io_RestoreINT();
    
    return active;
}

bool USBMSC_ReadData(USBMSC *msc, unsigned char *buf, int len)
{
    bool ret;
    
    io_DisableINT();
    if (ret = USBDev_StartTransfer(msc->out, buf, len))
         msc->out_done = false;
    io_RestoreINT();
    
    return ret;
}

bool USBMSC_ReadCBW(USBMSC *msc, USBMSC_CBW *cbw)
{
    return USBMSC_ReadData(msc, (unsigned char *)cbw, sizeof(USBMSC_CBW));
}

bool USBMSC_ReadComplete(USBMSC *msc)
{
    bool done;
    
    io_DisableINT();
    done = msc->out_done;
    io_RestoreINT();
    
    return done;
}

bool USBMSC_WriteData(USBMSC *msc, unsigned char *buf, int len)
{
    bool ret;
    
    io_DisableINT();
    if (ret = USBDev_StartTransfer(msc->in, buf, len))
         msc->in_done = false;
    io_RestoreINT();
    
    return ret;
}

bool USBMSC_WriteCSW(USBMSC *msc, USBMSC_CSW *csw)
{
    return USBMSC_WriteData(msc, (unsigned char *)csw, sizeof(USBMSC_CSW));
}

bool USBMSC_WriteComplete(USBMSC *msc)
{
    bool done;
    
    io_DisableINT();
    done = msc->in_done;
    io_RestoreINT();
    
    return done;
}

void USBMSC_DataInStall(USBMSC *msc)
{
    USBDev_EndpointStall(msc->in);
}

void USBMSC_DataOutStall(USBMSC *msc)
{
    USBDev_EndpointStall(msc->out);
}
