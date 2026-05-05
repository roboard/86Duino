/*
  usbhid.cpp - DM&P Vortex86 USB Device HID Class Library
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
#include "usbhid.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "io.h"
#include "usb.h"
#include "usbdesc.h"

#define USB_HID_REPORT_BUFSIZE     (256)

struct hid_report {

    unsigned char *buffer;
    unsigned int   length;

};

struct usb_hid_ep {
    
    USBEP *ep;
    
    bool done;
    
    unsigned char *reg_buf;
    unsigned int   reg_size;
    
};

struct usb_hid {
        
    bool active;
    
    USBDev *dev;
    
    struct usb_hid_ep intr_in;
    
    void (*suspend)(void);
    
    unsigned char  protocol;
    unsigned char  idle;
    
    struct hid_report      *input;
    unsigned int            input_byte;
    volatile unsigned int   input_head;
    volatile unsigned int   input_tail;
    
    unsigned char          *output_report;
    unsigned int            output_byte;
    
};

static void USBHID_ControlRequest(void *data, struct usb_ctrlrequest *setup)
{ 
    struct usb_hid *hid = (struct usb_hid *)data;
    int min;
    
    switch (setup->bRequest) {
        
    case USB_REQ_GET_DESCRIPTOR:
        if (setup->bRequestType != (USB_DIR_IN | USB_TYPE_STANDARD | USB_RECIP_INTERFACE))
            break;
        if ((setup->wValue >> 8) == 0x22) {
            min = hid_interface.hid_data_desc.wDescriptorLength;
            if (min > setup->wLength)
                min = setup->wLength;
            USBDev_StartTransfer(hid->dev->in, hid_report_desc, min);
        }
        break;
    
    case USB_REQ_GET_REPORT:
        break;
        
    case USB_REQ_SET_REPORT:
        if (setup->bRequestType != (USB_DIR_OUT | USB_TYPE_CLASS | USB_RECIP_INTERFACE))
            break;
        if (setup->wIndex != hid_interface.hid_data_interface.bInterfaceNumber)
            break;
        USBDev_StartTransfer(hid->dev->out, hid->output_report, hid->output_byte);
        break;
        
    case USB_REQ_GET_PROTOCOL:
        if (setup->bRequestType != (USB_DIR_IN | USB_TYPE_CLASS | USB_RECIP_INTERFACE))
            break;
        if (setup->wIndex != hid_interface.hid_data_interface.bInterfaceNumber)
            break;
        USBDev_StartTransfer(hid->dev->in, &hid->protocol, 1);
        break;
        
    case USB_REQ_SET_PROTOCOL:
        if (setup->bRequestType != (USB_DIR_OUT | USB_TYPE_CLASS | USB_RECIP_INTERFACE))
            break;
        if (setup->wIndex != hid_interface.hid_data_interface.bInterfaceNumber)
            break;
        hid->protocol = setup->wValue;
        break;
        
    case USB_REQ_GET_IDLE:
        if (setup->bRequestType != (USB_DIR_IN | USB_TYPE_CLASS | USB_RECIP_INTERFACE))
            break;
        if (setup->wIndex != hid_interface.hid_data_interface.bInterfaceNumber)
            break;
        USBDev_StartTransfer(hid->dev->in, &hid->idle, 1);
        break;
        
    case USB_REQ_SET_IDLE:
        if (setup->bRequestType != (USB_DIR_OUT | USB_TYPE_CLASS | USB_RECIP_INTERFACE))
            break;
        if (setup->wIndex != hid_interface.hid_data_interface.bInterfaceNumber)
            break;
        hid->idle = setup->wValue;
        break;
        
    }
}

static void USBHID_Enable(void *data)
{
    struct usb_hid *hid = (struct usb_hid *)data;
    
    USBDev_EnableEndpoint(hid->intr_in.ep);
    
    hid->active = true;
}

static void USBHID_Suspend(void *data)
{
    struct usb_hid *hid = (struct usb_hid *)data;
    
    USBDev_DisableEndpoint(hid->intr_in.ep);
    
    hid->active = false;
    
    if (hid->suspend) {
        
        io_DisableINT();
        hid->suspend();
        io_RestoreINT();
        
    }
}

static void USBHID_IntrInComplete(void *data, int len)
{
    struct usb_hid *hid = (struct usb_hid *)data;
    unsigned int  min;
    
    io_DisableINT();
    
    hid->intr_in.done = true;
    
    if (USBHID_WFIFOSize(hid) > 0) {
        
        min = hid->input[hid->input_head].length;
        if (min > hid->intr_in.reg_size)
            min = hid->intr_in.reg_size;
        
        memcpy(hid->intr_in.reg_buf, hid->input[hid->input_head].buffer, min);
        hid->input_head = (hid->input_head + 1) & (USB_HID_REPORT_BUFSIZE - 1);
        
        hid->intr_in.done = false;
        USBDev_StartTransfer(hid->intr_in.ep, hid->intr_in.reg_buf, min);
        
    }
    
    io_RestoreINT();
}

USBHID *USBHID_Init(USBDev *dev, void (*suspend)(void), unsigned int input_byte, unsigned int output_byte)
{
    USBHID *hid;
    int i, size;
    
    hid = (struct usb_hid *)malloc(sizeof(struct usb_hid));
    if (hid == NULL) {
    
        printf("ERROR: Alloc struct usb_hid size fail.\n");
        goto ALLOC_HID_STRUCT_FAIL;
    
    }
    memset(hid, 0, sizeof(struct usb_hid));
    hid->active = false;
    hid->suspend = suspend;
    
    hid->output_byte = output_byte;
    hid->output_report = (unsigned char *)malloc(hid->output_byte);
    if (hid->output_report == NULL) {
    
        printf("ERROR: Alloc Output Report buffer fail.\n");
        goto ALLOC_OUT_REPORT_BUF_FAIL;
    
    }
    memset(hid->output_report, 0, hid->output_byte);
    
    // Interrupt IN Endpoint Buffer Initialize
    hid->input_byte = input_byte;
    hid->intr_in.done = true;
    hid->intr_in.reg_size = hid->input_byte;
    hid->intr_in.reg_buf = (unsigned char *)malloc(hid->intr_in.reg_size);
    if (hid->intr_in.reg_buf == NULL) {
        
        printf("ERROR: Alloc Interrupt IN registed buffer fail.\n");
        goto ALLOC_INTRIN_REG_BUF_FAIL;
        
    }
    
    size = (sizeof(struct hid_report) + hid->intr_in.reg_size) * USB_HID_REPORT_BUFSIZE;
    hid->input = (struct hid_report *)malloc(size);
    if (hid->input == NULL) {
        
        printf("ERROR: Alloc Interrupt IN buffer fail.\n");
        goto ALLOC_INTRIN_BUF_FAIL;
        
    }
    memset(hid->input, 0, size);
    
    size = sizeof(struct hid_report) * USB_HID_REPORT_BUFSIZE;
    for (i = 0; i < USB_HID_REPORT_BUFSIZE; i++) {
    
        hid->input[i].buffer = (unsigned char *)hid->input + size + i * hid->intr_in.reg_size;
    
    }
    
    hid->dev = dev;
    
    if (!USBDev_ClassRegister(hid->dev, hid, USBHID_ControlRequest, USBHID_Enable, USBHID_Suspend)) {
        
        printf("ERROR: USBDev_ClassRegister(HID) fail.\n");
        goto REGISTER_CLASS_FAIL;
        
    }
    
    hid->intr_in.ep = USBDev_InitEndpoint(hid->dev, &hid_interface.hid_data_in, hid, USBHID_IntrInComplete);
    if (hid->intr_in.ep == NULL) {
    
        printf("ERROR: USBDev_InitEndpoint(HID->IntrIN) fail.\n");
        goto INIT_INTR_IN_EP_FAIL;
    
    }
    
    return hid;

INIT_INTR_IN_EP_FAIL:
    USBDev_ClassUnRegister(hid->dev, hid);
REGISTER_CLASS_FAIL:
    free(hid->input);
ALLOC_INTRIN_BUF_FAIL:
    free(hid->intr_in.reg_buf);
ALLOC_INTRIN_REG_BUF_FAIL:
    free(hid->output_report);
ALLOC_OUT_REPORT_BUF_FAIL:
    free(hid);
ALLOC_HID_STRUCT_FAIL:
    return NULL;
}

void USBHID_Close(USBHID *hid)
{
    if (hid->intr_in.ep) {
        USBDev_DisableEndpoint(hid->intr_in.ep);
        USBDev_DeinitEndpoint(hid->intr_in.ep);
    }
    USBDev_ClassUnRegister(hid->dev, hid);
    free(hid->input);
    free(hid->intr_in.reg_buf);
    free(hid->output_report);
    free(hid);
}

bool USBHID_CheckReady(USBHID *hid)
{
    bool ret;
    
    io_DisableINT();
    ret = hid->active;
    io_RestoreINT();
    
    return ret;
}

unsigned int USBHID_WriteInputReport(USBHID *hid, unsigned char *buf, int len)
{
    unsigned int min;
    bool done;
    
    if (USBHID_WFIFOFree(hid) <= 0)
        return 0;
    
    min = hid->input_byte;
    if (min > len)
        min = len;
    
    io_DisableINT();
    hid->input[hid->input_tail].length = min;
    memcpy(hid->input[hid->input_tail].buffer, buf, min);
    hid->input_tail = (hid->input_tail + 1) & (USB_HID_REPORT_BUFSIZE - 1);
    
    done = hid->intr_in.done;
    io_RestoreINT();
    
    if (done) {
        
        USBHID_IntrInComplete((void *)hid, 0);
        
    }
    
    return min;
}

unsigned int USBHID_WFIFOSize(USBHID *hid)
{
    unsigned int h, t;
    
    io_DisableINT();
    h = hid->input_head;
    t = hid->input_tail;
    io_RestoreINT();
    
    return (h <= t) ? (t - h) : (t + USB_HID_REPORT_BUFSIZE - h);
}

unsigned int USBHID_WFIFOFree(USBHID *hid)
{
    unsigned int h, t;
    
    io_DisableINT();
    h = hid->input_head;
    t = hid->input_tail;
    io_RestoreINT();
    
    return (h <= t) ? (USB_HID_REPORT_BUFSIZE - 1 - t + h) : (h - t - 1);
}

void USBHID_WFIFOFlush(USBHID *hid)
{
    io_DisableINT();
    hid->input_head = hid->input_tail;
    io_RestoreINT();
}

void USBHID_WFIFOWaitFlush(USBHID *hid)
{
    bool done;
    
    while (USBHID_WFIFOSize(hid) > 0);
    
    do {
        
        io_DisableINT();
        done = hid->intr_in.done;
        io_RestoreINT();
        
    } while (!done);
}

unsigned int USBHID_ReadOutputReport(USBHID *hid, unsigned char *buf, int len)
{
    unsigned int min;
    
    min = hid->output_byte;
    if (min > len)
        min = len;
    
    io_DisableINT();
    memcpy(buf, hid->output_report, min);
    io_RestoreINT();
    
    return min;
}
