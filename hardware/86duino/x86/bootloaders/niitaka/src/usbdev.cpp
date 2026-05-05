/*
  usbdev.cpp - DM&P Vortex86 USB Device Library
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
#include "usbdev.h"

#include <stdlib.h>
#include <string.h>
#include <dpmi.h>
#include "io.h"
#include "irq.h"
#include "usb.h"
#include "usbdesc.h"

#define D1060_EP_NUM            (8)
#define USBDEV_TXBUFFER_SIZE    (1024*4)
#define USBDEV_RXBUFFER_SIZE    (1024*4)

/***************************************************************************
*                                                                          *
*                DJGPP DOS DMA Alloc and Free function.                    *
*                                                                          *
****************************************************************************/
typedef int DMA_HANDLE_t;
#define DMA_FAIL (-1)

static DMA_HANDLE_t dma_Alloc(unsigned long size, unsigned long* phyaddr)
{
    DMA_HANDLE_t dma_handle;
    int tmp = __dpmi_allocate_dos_memory((size + 15) >> 4, &dma_handle);
    
    if (tmp == -1)
        return DMA_FAIL;
    else
        *phyaddr = (unsigned long)tmp << 4;

    return dma_handle;
}

static bool dma_Free(DMA_HANDLE_t dma_handle)
{
    if (__dpmi_free_dos_memory(dma_handle) == -1) return false; else return true;
}
/***************************************************************************/
static struct usb_endpoint_descriptor control_in = {

        0x07,                               // bLength
        USB_DT_ENDPOINT,                 	// bDescriptorType
        USB_DIR_IN | 0x00,                  // bEndpointAddress
        USB_ENDPOINT_XFER_CONTROL,          // bmAttributes
        0x40,                               // wMaxPacketSize
        0xFF,                               // bInterval
        
};

static struct usb_endpoint_descriptor control_out = {

        0x07,                               // bLength
        USB_DT_ENDPOINT,                 	// bDescriptorType
        0x00,                               // bEndpointAddress
        USB_ENDPOINT_XFER_CONTROL,          // bmAttributes
        0x40,                               // wMaxPacketSize
        0xFF,                               // bInterval
        
};

static inline void usb_control_request(struct usb_device *dev, struct usb_ctrlrequest *setup)
{
	int value, min;
    struct class_register *reg;

    switch (setup->bRequest) {

    case USB_REQ_GET_DESCRIPTOR:
        if (setup->bRequestType != (USB_DIR_IN | USB_TYPE_STANDARD | USB_RECIP_DEVICE))
            break;
        switch (setup->wValue >> 8) {
        case USB_DT_DEVICE:
            min = device_desc.bLength;
            if (min > setup->wLength)
                min = setup->wLength;
            USBDev_StartTransfer(dev->in, (unsigned char *)&device_desc, min);
            break;
        case USB_DT_DEVICE_QUALIFIER:
            min = qualifier_desc.bLength;
            if (min > setup->wLength)
                min = setup->wLength;
            USBDev_StartTransfer(dev->in, (unsigned char *)&qualifier_desc, min);
            break;
        case USB_DT_CONFIG:
            min = config_desc.wTotalLength;
            if (min > setup->wLength)
                min = setup->wLength;
            USBDev_StartTransfer(dev->in, config_descs, min);
            break;
        case USB_DT_STRING:
            value = setup->wValue & 0xff;
            min = string_descs[value].bLength;
            if (min > setup->wLength)
                min = setup->wLength;
            USBDev_StartTransfer(dev->in, (unsigned char *)&string_descs[value], min);
            break;
        }
        break;
        
    }
    
    reg = dev->class_reg;
    while (reg) {
        
        reg->ctrlrequest(reg->data, setup);
        reg = reg->next;
        
    }
}

#include "d1060.h"
#include "d1061.h"

static int USBDev_ISR(int irq, void *data)
{
    struct usb_device *dev = (struct usb_device *)data;
    
    return dev->ops->usb_dev_isr(dev);
}

struct pci_device_id {
    
    unsigned short VID;
    unsigned short PID;
    
    unsigned int space;
    unsigned int len;
    
    struct usb_devive_ops *ops;
    
} pci_dev[] = {
    { 0x17F3, 0x1060, IO_USE_PORTIO, 0x100, &d1060_ops},
    { 0x17F3, 0x1061, IO_USE_MMIO  , 0x100, &d1061_ops}
};

USBDev *USBDev_Init()
{
    int pci_i, size;
    void *pci;
    struct usb_device *dev;
    
    if (io_Init() == false) {
        
        printf("ERROR: io_Init() fail.\n");
        return NULL;
        
    }
    
    dev = (struct usb_device *)malloc(sizeof(struct usb_device));
    if (!dev) {
        
        printf("ERROR: Alloc struct usb_device size fail.\n");
        io_Close();
        return NULL;
        
    }
    memset(dev, 0, sizeof(struct usb_device));
    
    size = sizeof(pci_dev)/sizeof(pci_dev[0]);
    for (pci_i = 0; pci_i < size; pci_i++) {
        
        pci = pci_Find(pci_dev[pci_i].VID, pci_dev[pci_i].PID);
        if (pci) {
            
            dev->base = pci_In32(pci, 0x10) & 0xFFFFFFFEUL;
            dev->irq = pci_In8(pci, 0x3C);
            pci_Free(pci);
            break;
            
        } else if((pci_i + 1) == size) {
            
            printf("ERROR: No Vortex86EX-Series USB Device.\n");
            free(dev);
            io_Close();
            return NULL;
            
        }
        
    }
    
    dev->ops = pci_dev[pci_i].ops;
    
    dev->io = io_Alloc(pci_dev[pci_i].space, dev->base, pci_dev[pci_i].len);
    if (dev->io == NULL) {
    
        printf("ERROR: io_Alloc(0x%x, 0x%x) fail.\n", dev->base, pci_dev[pci_i].len);
        free(dev);
        io_Close();
        return NULL;
    
    }
    dev->reset = true;
    
    if (irq_Init() == false) {
    
        printf("ERROR: irq_Init() fail.\n");
        io_Free(dev->io);
        free(dev);
        io_Close();
        return NULL;
    
    }
    
    irq_Setting(dev->irq, IRQ_LEVEL_TRIGGER);
    irq_InstallISR(dev->irq, USBDev_ISR, dev);
    
    
    if (dev->ops->usb_dev_initialize(dev) == false) {
    
        printf("ERROR: USBDev_Initialize() fail.\n");
        irq_Close();
        io_Free(dev->io);
        free(dev);
        io_Close();
        return NULL;
    
    }
    
    return dev;
}

void USBDev_Close(USBDev *dev)
{
    dev->ops->usb_dev_deinitialize(dev);
    irq_UninstallISR(dev->irq, dev);
    irq_Close();
    io_Free(dev->io);
    free(dev);
    io_Close();
}

bool USBDev_ClassRegister(USBDev *dev, void *data, void (*ctrlrequest)(void *, struct usb_ctrlrequest *),
                                                   void (*classenable)(void *),
                                                   void (*suspend)(void *))
{
    struct class_register *class_reg, *reg;
    
    reg = dev->class_reg;
    while (reg) {
        
        if (reg->data == data) {
            
            printf("ERROR: This USB class has been registered.\n");
            return false;
            
        }
        
        reg = reg->next;
        
    }
    
    class_reg = (struct class_register *)malloc(sizeof(struct class_register));
    if (class_reg == NULL) {
    
        printf("ERROR: Alloc struct class_register size fail.\n");
        return false;
    
    }
    class_reg->data         = data;
    class_reg->ctrlrequest  = ctrlrequest;
    class_reg->classenable  = classenable;
    class_reg->suspend      = suspend;
    class_reg->next         = NULL;
    
    if (dev->class_reg) {
        
        reg = dev->class_reg;
        while (reg->next)
            reg = reg->next;
        reg->next = class_reg;
        
    } else {
        
        dev->class_reg = class_reg;
        
    }
    
    return true;
}

void USBDev_ClassUnRegister(USBDev *dev, void *data)
{
    struct class_register *pre, *now;

    pre = NULL;
    now = dev->class_reg;
    while (now) {
        
        if (now->data == data) {
            
            if (pre == NULL) {
                
                dev->class_reg = NULL;
                
            } else {
                
                pre->next = now->next;
                
            }
            free(now);
            break;
            
        }
        
        pre = now;
        now = now->next;
        
    }
}

bool USBDev_Start(USBDev *dev)
{
    return dev->ops->usb_dev_start(dev);
}

void USBDev_Stop(USBDev *dev)
{
    dev->ops->usb_dev_stop(dev);
}

void USBDev_SetDataInCB(USBDev *dev, void (*func)(void))
{
    dev->data_in = func;
}

void USBDev_SetDataOutCB(USBDev *dev, void (*func)(void))
{
    dev->data_out = func;
}

void USBDev_SetSOFCB(USBDev *dev, void (*func)(void))
{
    dev->start_of_frame = func;
}

void USBDev_EnableSOFInterrupt(USBDev *dev)
{
    dev->sof_interrupt = 1;
    dev->ops->usb_dev_enable_sof_int(dev);
}

void USBDev_DisableSOFInterrupt(USBDev *dev)
{
    dev->sof_interrupt = 0;
    dev->ops->usb_dev_disable_sof_int(dev);
}

USBEP *USBDev_InitEndpoint(USBDev *dev, struct usb_endpoint_descriptor *desc, void *data, void (*complete)(void *, int))
{
    int index, in, num;
    USBEP *ep;
    
    num = desc->bEndpointAddress & 0x03;
    in = (desc->bEndpointAddress & 0x80) ? 1 : 0; 
    index = (num << 1) + (in ? 0 : 1);
    
    ep = &dev->ep[index];
    
    ep->data = data;
    ep->desc = desc;
    ep->num = num;
    ep->in = in;
    ep->type = desc->bmAttributes;
    ep->maxpacket = desc->wMaxPacketSize;
    ep->complete = complete;
    
    ep->buf = (unsigned char *)malloc(ep->maxpacket);
    if (!ep->buf) {
    
        printf("ERROR: %s() alloc buffer failed.\n", __FUNCTION__);
        return NULL;
    
    }
    
    if (!dev->ops->usb_ep_dma_alloc(ep)) {
        
        printf("ERROR: %s() alloc DMA buffer failed.\n", __FUNCTION__);
        return NULL;
        
    }
    
    ep->databuf = NULL;
    ep->dataptr = NULL;
    ep->datalen = 0;
    
    return ep;
}

void USBDev_DeinitEndpoint(USBEP *ep)
{
    dma_Free(ep->dma_handle);
    ep->dma_desc = 0;
    ep->dma_buf = 0;
    ep->dma_handle = DMA_FAIL;
    
    free(ep->buf);
    ep->buf = NULL;
    ep->desc = NULL;
}

void USBDev_EnableEndpoint(USBEP *ep)
{
    ep->databuf = NULL;
    ep->dataptr = NULL;
    ep->datalen = 0;
    ep->active = true;
    ep->stall = false;
    
    ep->dev->ops->usb_ep_enable(ep, ep->desc);
}

void USBDev_DisableEndpoint(USBEP *ep)
{
    ep->dev->ops->usb_ep_disable(ep);
    
    ep->stall = false;
    ep->active = false;
    ep->databuf = NULL;
    ep->dataptr = NULL;
    ep->datalen = 0;
}

bool USBDev_StartTransfer(USBEP *ep, unsigned char *buf, int len)
{
    if (!ep || !ep->active || ep->stall)
        return false;
    
    io_DisableINT();
    ep->databuf = buf;
    ep->dataptr = buf;
    ep->datalen = len;
    if (ep->num == 0 && ep->in && ep->dev->reset) {
        
        if (ep->datalen > ep->maxpacket)
            ep->datalen = ep->maxpacket;
        ep->dev->reset = 0;
        
    }
    ep->dev->ops->usb_ep_init_transfer(ep);
    io_RestoreINT();
    
    return true;
}

void USBDev_EndpointStall(USBEP *ep)
{
    ep->stall = true;
    ep->dev->ops->usb_ep_set_stall(ep);
}
