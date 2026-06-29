/*
  usbdev.h - DM&P Vortex86 USB Device Library
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
#ifndef __USB_DEVICE_H
#define __USB_DEVICE_H

#include "usb.h"

#define USB_EP_NUM  (8)

struct usb_device;
struct usb_ep;

struct usb_devive_ops {

    int  (*usb_dev_isr)             (struct usb_device *);
    bool (*usb_dev_start)           (struct usb_device *);
    void (*usb_dev_stop)            (struct usb_device *);
    bool (*usb_dev_initialize)      (struct usb_device *);
    void (*usb_dev_deinitialize)    (struct usb_device *);
    void (*usb_dev_enable_sof_int)  (struct usb_device *);
    void (*usb_dev_disable_sof_int) (struct usb_device *);
    
    bool (*usb_ep_dma_alloc)        (struct usb_ep *);
    void (*usb_ep_enable)           (struct usb_ep *, struct usb_endpoint_descriptor *);
    void (*usb_ep_disable)          (struct usb_ep *);
    void (*usb_ep_init_transfer)    (struct usb_ep *);
    void (*usb_ep_set_stall)        (struct usb_ep *);
    void (*usb_ep_clear_stall)      (struct usb_ep *);
    void (*usb_ep_set_ack)          (struct usb_ep *);
    
};

struct class_register {

    void *data;
    bool (*ctrlrequest)(void *, struct usb_ctrlrequest *);
    void (*classenable)(void *);
    void (*suspend)    (void *);
    
    struct class_register *next;

};

typedef struct usb_ep {
    
    struct usb_device *dev;
    struct usb_endpoint_descriptor *desc;
    void *data;
    
    int type;
    int num;
    int in;
    int maxpacket;
    
    bool active;
    unsigned short status;
    
    unsigned char *dataptr;
    unsigned char *databuf;
    int datalen;
    
    unsigned char *buf;
    unsigned long dma_buf;
    unsigned long dma_desc;
    int dma_handle;
    void (*complete)(void *, int);
    
} USBEP;

typedef struct usb_device {
    
    struct usb_ep ep[USB_EP_NUM];

    void *pci;
    void *io;
    unsigned long base;
    int irq;
    
    struct usb_ep *in;
    struct usb_ep *out;
    
    struct usb_ctrlrequest setup;
    unsigned long dma_setup;
    int dma_setup_handle;
    
    struct usb_devive_ops *ops;
    struct class_register *class_reg;
    unsigned char configuration;
    
    bool reset;
    int address;
    
    int sof_interrupt;
    
    void (*start_of_frame)(void);
    void (*data_in)(void);
    void (*data_out)(void);
    
    unsigned short device_status;
    unsigned short interface_status;
    unsigned char interface;
    
} USBDev;

USBDev *USBDev_Init                 (void);
void    USBDev_Close                (USBDev *dev);
bool    USBDev_ClassRegister        (USBDev *dev, void *data, bool (*ctrlrequest)(void *, struct usb_ctrlrequest *),
                                                              void (*classenable)(void *),
                                                              void (*suspend)(void *));
void    USBDev_ClassUnRegister      (USBDev *dev, void *data);
bool    USBDev_Start                (USBDev *dev);
void    USBDev_Stop                 (USBDev *dev);
void    USBDev_SetDataInCB          (USBDev *dev, void (*func)(void));
void    USBDev_SetDataOutCB         (USBDev *dev, void (*func)(void));
void    USBDev_SetSOFCB             (USBDev *dev, void (*func)(void));
void    USBDev_EnableSOFInterrupt   (USBDev *dev);
void    USBDev_DisableSOFInterrupt  (USBDev *dev);

USBEP  *USBDev_InitEndpoint         (USBDev *dev, struct usb_endpoint_descriptor *desc, void *data, void (*complete)(void *, int));
void    USBDev_DeinitEndpoint       (USBEP *ep);
void    USBDev_EnableEndpoint       (USBEP *ep);
void    USBDev_DisableEndpoint      (USBEP *ep);
bool    USBDev_StartTransfer        (USBEP *ep, unsigned char *buf, int len);
void    USBDev_EndpointStall        (USBEP *ep);
void    USBDev_EndpointStallClear   (USBEP *ep);

#endif
