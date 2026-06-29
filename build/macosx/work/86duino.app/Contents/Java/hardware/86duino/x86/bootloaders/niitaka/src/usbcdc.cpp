/*
  usbcdc.cpp - DM&P Vortex86 USB Device Communication Device Class Library
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
#include "usbcdc.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "io.h"
#include "usb.h"
#include "usbdesc.h"

#define USB_CDC_INTRIN_BUFSIZE      (1 *  256)
#define USB_CDC_DATAIN_BUFSIZE      (4 * 1024)
#define USB_CDC_DATAOUT_BUFSIZE     (4 * 1024)

struct usb_serial_ep {
    
    USBEP *ep;
    
    bool done;
    
    unsigned char *reg_buf;
    unsigned int   reg_size;
    
    unsigned char          *buf;
    volatile unsigned int   head;
    volatile unsigned int   tail;
    
};

struct usb_serial {
    
    bool active;
    
    USBDev *dev;
    
    struct usb_serial_ep intr_in;
    struct usb_serial_ep data_in;
    struct usb_serial_ep data_out;
    
    void (*suspend)(void);
    void (*control_line_state)(void);
    
    struct usb_cdc_line_coding  port_line_coding;
    unsigned short              port_line_state;
    int                         port_break;
    
    struct usb_cdc_notification notification;
    unsigned short              serial_state;
    
};

static void USBCDC_ControlRequest(void *data, struct usb_ctrlrequest *setup)
{
    struct usb_serial *cdc = (struct usb_serial *)data;
    
    switch (setup->bRequest) {

    case USB_CDC_REQ_SET_LINE_CODING:
        if (setup->bRequestType != (USB_DIR_OUT | USB_TYPE_CLASS | USB_RECIP_INTERFACE))
            break;
        if (setup->wIndex != acm_interface.acm_control_interface.bInterfaceNumber || setup->wLength != sizeof(struct usb_cdc_line_coding))
            break;
        USBDev_StartTransfer(cdc->dev->out, (unsigned char *)&cdc->port_line_coding, sizeof(struct usb_cdc_line_coding));
        break;
    
    case USB_CDC_REQ_GET_LINE_CODING:
        if (setup->bRequestType != (USB_DIR_IN | USB_TYPE_CLASS | USB_RECIP_INTERFACE))
            break;
        if (setup->wIndex != acm_interface.acm_control_interface.bInterfaceNumber)
            break;
        USBDev_StartTransfer(cdc->dev->in, (unsigned char *)&cdc->port_line_coding, sizeof(struct usb_cdc_line_coding));
        break;
        
    case USB_CDC_REQ_SET_CONTROL_LINE_STATE:
        if (setup->bRequestType != (USB_DIR_OUT | USB_TYPE_CLASS | USB_RECIP_INTERFACE))
            break;
        if (setup->wIndex != acm_interface.acm_control_interface.bInterfaceNumber)
            break;
        cdc->port_line_state = setup->wValue;
        if (cdc->control_line_state)
            cdc->control_line_state();
        break;
    
    case USB_CDC_REQ_SEND_BREAK:
        if (setup->bRequestType != (USB_DIR_OUT | USB_TYPE_CLASS | USB_RECIP_INTERFACE))
            break;
        if (setup->wIndex != acm_interface.acm_control_interface.bInterfaceNumber)
            break;
        cdc->port_break = setup->wValue;
        break;
        
    }
}

static void USBCDC_Enable(void *data)
{
    struct usb_serial *cdc = (struct usb_serial *)data;
    
    USBDev_EnableEndpoint(cdc->intr_in.ep);
    USBDev_EnableEndpoint(cdc->data_in.ep);
    USBDev_EnableEndpoint(cdc->data_out.ep);
    USBDev_StartTransfer(cdc->data_out.ep, cdc->data_out.reg_buf, cdc->data_out.reg_size);
    
    cdc->active = true;
}

static void USBCDC_Suspend(void *data)
{
    struct usb_serial *cdc = (struct usb_serial *)data;
    
    USBDev_DisableEndpoint(cdc->data_out.ep);
    USBDev_DisableEndpoint(cdc->data_in.ep);
    USBDev_DisableEndpoint(cdc->intr_in.ep);
    
    cdc->active = false;
    
    
    if (cdc->suspend) {
        
        io_DisableINT();
        cdc->suspend();
        io_RestoreINT();
        
    }
}

static void USBCDC_IntrInComplete(void *data, int len)
{
    struct usb_serial *cdc = (struct usb_serial *)data;
    unsigned int h, t, size, i = 0;
    
    io_DisableINT();
    
    cdc->intr_in.done = true;
    
    h = cdc->intr_in.head;
    t = cdc->intr_in.tail;
    size = (h <= t) ? (t - h) : (t + USB_CDC_INTRIN_BUFSIZE - h);
    
    if (size >= sizeof(cdc->serial_state)) {
        
        size = sizeof(cdc->serial_state);
        
        cdc->notification.bmRequestType = USB_DIR_IN | USB_TYPE_CLASS | USB_RECIP_INTERFACE;
        cdc->notification.bNotificationType = USB_CDC_NOTIFY_SERIAL_STATE;
        cdc->notification.wValue = 0;
        cdc->notification.wIndex = 0;
        cdc->notification.wLength = size;
        
        memcpy(cdc->intr_in.reg_buf, &cdc->notification, sizeof(struct usb_cdc_notification));
        while (i < size) {
    
            cdc->intr_in.reg_buf[sizeof(struct usb_cdc_notification) + i++] = cdc->intr_in.buf[cdc->intr_in.head];
            cdc->intr_in.head = (cdc->intr_in.head + 1) & (USB_CDC_INTRIN_BUFSIZE - 1);
    
        }
        
        cdc->intr_in.done = false;
        USBDev_StartTransfer(cdc->intr_in.ep, cdc->intr_in.reg_buf, sizeof(struct usb_cdc_notification) + i);
        
    }
    
    io_RestoreINT();
    
}

static void USBCDC_DataInComplete(void *data, int len)
{
    struct usb_serial *cdc = (struct usb_serial *)data;
    unsigned int i = 0;
    
    io_DisableINT();
    
    cdc->data_in.done = true;
    
    while (cdc->data_in.head != cdc->data_in.tail && i < cdc->data_in.reg_size) {
    
        cdc->data_in.reg_buf[i++] = cdc->data_in.buf[cdc->data_in.head];
        cdc->data_in.head = (cdc->data_in.head + 1) & (USB_CDC_DATAIN_BUFSIZE - 1);
    
    }
    
    if (i > 0) {
    
        cdc->data_in.done = false;
        USBDev_StartTransfer(cdc->data_in.ep, cdc->data_in.reg_buf, i);

    }
    
    io_RestoreINT();
}

static void USBCDC_DataOutComplete(void *data, int len)
{
    struct usb_serial *cdc = (struct usb_serial *)data;
    unsigned int h, t, size, i = 0;
    
    io_DisableINT();
    
    cdc->data_out.done = true;
    
    while (((cdc->data_out.tail + 1) & (USB_CDC_DATAOUT_BUFSIZE - 1)) != cdc->data_out.head && i < len) {
    
        cdc->data_out.buf[cdc->data_out.tail] = cdc->data_out.reg_buf[i++];
        cdc->data_out.tail = (cdc->data_out.tail + 1) & (USB_CDC_DATAOUT_BUFSIZE - 1);
    
    }
     
    USBDev_StartTransfer(cdc->data_out.ep, cdc->data_out.reg_buf, cdc->data_out.reg_size);
    
    cdc->data_out.done = false;
    
    io_RestoreINT();
}

static bool USBCDC_SendSerialState(USBCDC *cdc, unsigned short state)
{
    unsigned int h, t, size, cur = 0;
    unsigned char *buf = (unsigned char *)&state;
    bool done;
    
    io_DisableINT();
    h = cdc->intr_in.head;
    t = cdc->intr_in.tail;
    io_RestoreINT();
    size = (h <= t) ? (USB_CDC_INTRIN_BUFSIZE - 1 - t + h) : (h - t - 1);
    
    if (size < sizeof(cdc->serial_state))
        return false;
    
    size = sizeof(cdc->serial_state);
    
    while (size > 0) {
    
        io_DisableINT();
        cdc->intr_in.buf[cdc->intr_in.tail] = buf[cur++];
        cdc->intr_in.tail = (cdc->intr_in.tail + 1) & (USB_CDC_INTRIN_BUFSIZE - 1);
        io_RestoreINT();
        
        size--;
    
    }
    
    if (cur > 0) {
    
        io_DisableINT();
        done = cdc->intr_in.done;
        io_RestoreINT();
        
        if (done) {
            
            USBCDC_IntrInComplete((void *)cdc, 0);
            
        }
    
    }
    
    return true;
}

USBCDC *USBCDC_Init(USBDev *dev, void (*suspend)(void))
{
    USBCDC *cdc;
    
    cdc = (struct usb_serial *)malloc(sizeof(struct usb_serial));
    if (cdc == NULL) {
    
        printf("ERROR: Alloc struct usb_serial size fail.\n");
        goto ALLOC_CDC_STRUCT_FAIL;
    
    }
    memset(cdc, 0, sizeof(struct usb_serial));
    cdc->active = false;
    cdc->suspend = suspend;
    cdc->port_line_coding.dwDTERate   = 115200;
    cdc->port_line_coding.bCharFormat = USB_CDC_1_STOP_BITS;
    cdc->port_line_coding.bParityType = USB_CDC_NO_PARITY;
    cdc->port_line_coding.bDataBits   = 8;
    cdc->port_line_state = 0;
    cdc->port_break = -1;
    cdc->serial_state = 0;
    
    // Notify Endpoint Buffer Initialize
    cdc->intr_in.done = true;
    cdc->intr_in.reg_size = acm_interface.acm_notify.wMaxPacketSize;
    cdc->intr_in.reg_buf = (unsigned char *)malloc(cdc->intr_in.reg_size);
    if (cdc->intr_in.reg_buf == NULL) {
        
        printf("ERROR: Alloc notify registed buffer fail.\n");
        goto ALLOC_INTRIN_REG_BUF_FAIL;
        
    }
    cdc->intr_in.buf = (unsigned char *)malloc(USB_CDC_INTRIN_BUFSIZE);
    if (cdc->intr_in.buf == NULL) {
        
        printf("ERROR: Alloc notify buffer fail.\n");
        goto ALLOC_INTRIN_BUF_FAIL;
        
    }
    
    // Data IN Endpoint Buffer Initialize
    cdc->data_in.done = true;
    cdc->data_in.reg_size = acm_interface.acm_data_in.wMaxPacketSize;
    cdc->data_in.reg_buf = (unsigned char *)malloc(cdc->data_in.reg_size);
    if (cdc->data_in.reg_buf == NULL) {
        
        printf("ERROR: Alloc Data IN registed buffer fail.\n");
        goto ALLOC_DATAIN_REG_BUF_FAIL;
        
    }
    cdc->data_in.buf = (unsigned char *)malloc(USB_CDC_DATAIN_BUFSIZE);
    if (cdc->data_in.buf == NULL) {
        
        printf("ERROR: Alloc Data IN buffer fail.\n");
        goto ALLOC_DATAIN_BUF_FAIL;
        
    }
    
    // Data OUT Endpoint Buffer Initialize
    cdc->data_out.done = true;
    cdc->data_out.reg_size = acm_interface.acm_data_out.wMaxPacketSize;
    cdc->data_out.reg_buf = (unsigned char *)malloc(cdc->data_out.reg_size);
    if (cdc->data_out.reg_buf == NULL) {
        
        printf("ERROR: Alloc Data IN registed buffer fail.\n");
        goto ALLOC_DATAOUT_REG_BUF_FAIL;
        
    }
    cdc->data_out.buf = (unsigned char *)malloc(USB_CDC_DATAOUT_BUFSIZE);
    if (cdc->data_out.buf == NULL) {
        
        printf("ERROR: Alloc Data IN buffer fail.\n");
        goto ALLOC_DATAOUT_BUF_FAIL;
        
    }
    
    cdc->dev = dev;
    
    if (!USBDev_ClassRegister(cdc->dev, cdc, USBCDC_ControlRequest, USBCDC_Enable, USBCDC_Suspend)) {
        
        printf("ERROR: USBDev_ClassRegister(CDC) fail.\n");
        goto REGISTER_CLASS_FAIL;
        
    }
    
    cdc->intr_in.ep = USBDev_InitEndpoint(cdc->dev, &acm_interface.acm_notify, cdc, USBCDC_IntrInComplete);
    if (cdc->intr_in.ep == NULL) {
    
        printf("ERROR: USBDev_InitEndpoint(CDC->IntrIN) fail.\n");
        goto INIT_INTR_IN_EP_FAIL;
    
    }
    
    cdc->data_in.ep = USBDev_InitEndpoint(cdc->dev, &acm_interface.acm_data_in, cdc, USBCDC_DataInComplete);
    if (cdc->data_in.ep == NULL) {
    
        printf("ERROR: USBDev_InitEndpoint(CDC->DataIN) fail.\n");
        goto INIT_DATA_IN_EP_FAIL;
    
    }
    
    cdc->data_out.ep = USBDev_InitEndpoint(cdc->dev, &acm_interface.acm_data_out, cdc, USBCDC_DataOutComplete);
    if (cdc->data_out.ep == NULL) {
    
        printf("ERROR: USBDev_InitEndpoint(CDC->DataOUT) fail.\n");
        goto INIT_DATA_OUT_EP_FAIL;
    
    }
    
    return cdc;

INIT_DATA_OUT_EP_FAIL:
    USBDev_DeinitEndpoint(cdc->data_in.ep);
INIT_DATA_IN_EP_FAIL:
    USBDev_DeinitEndpoint(cdc->intr_in.ep);
INIT_INTR_IN_EP_FAIL:
    USBDev_ClassUnRegister(cdc->dev, cdc);
REGISTER_CLASS_FAIL:
    free(cdc->data_out.buf);
ALLOC_DATAOUT_BUF_FAIL:
    free(cdc->data_out.reg_buf);
ALLOC_DATAOUT_REG_BUF_FAIL:
    free(cdc->data_in.buf);
ALLOC_DATAIN_BUF_FAIL:
    free(cdc->data_in.reg_buf);
ALLOC_DATAIN_REG_BUF_FAIL:
    free(cdc->intr_in.buf);
ALLOC_INTRIN_BUF_FAIL:
    free(cdc->intr_in.reg_buf);
ALLOC_INTRIN_REG_BUF_FAIL:
    free(cdc);
ALLOC_CDC_STRUCT_FAIL:
    return NULL;
}

void USBCDC_Close(USBCDC *cdc)
{
    if (cdc->data_out.ep) {
        USBDev_DisableEndpoint(cdc->data_out.ep);
        USBDev_DeinitEndpoint(cdc->data_out.ep);
    }
    if (cdc->data_in.ep) {
        USBDev_DisableEndpoint(cdc->data_in.ep);
        USBDev_DeinitEndpoint(cdc->data_in.ep);
    }
    if (cdc->intr_in.ep) {
        USBDev_DisableEndpoint(cdc->intr_in.ep);
        USBDev_DeinitEndpoint(cdc->intr_in.ep);
    }
    USBDev_ClassUnRegister(cdc->dev, cdc);
    free(cdc->data_out.buf);
    free(cdc->data_out.reg_buf);
    free(cdc->data_in.buf);
    free(cdc->data_in.reg_buf);
    free(cdc->intr_in.buf);
    free(cdc->intr_in.reg_buf);
    free(cdc);
}

unsigned int USBCDC_ReadData(USBCDC *cdc, unsigned char *buf, int len)
{
    unsigned int h, t, free, size, cur = 0;
    bool done;
    
    while ((free = len - cur) > 0) {
        
        io_DisableINT();
        h = cdc->data_out.head;
        t = cdc->data_out.tail;
        io_RestoreINT();
        size = (h <= t) ? (t - h) : (t + USB_CDC_DATAOUT_BUFSIZE - h);
        
        if (size > free)
            size = free;
        
        if (size <= 0)
            break;
        
        while (size > 0) {
        
            io_DisableINT();
            buf[cur++] = cdc->data_out.buf[cdc->data_out.head];
            cdc->data_out.head = (cdc->data_out.head + 1) & (USB_CDC_DATAOUT_BUFSIZE - 1);
            io_RestoreINT();
            
            size--;
        
        }
        
    }
    
    return cur;
}

unsigned int USBCDC_WriteData(USBCDC *cdc, unsigned char *buf, int len)
{
    unsigned int h, t, free, size, cur = 0;
    bool done;
    
    while ((free = len - cur) > 0) {
        
        io_DisableINT();
        h = cdc->data_in.head;
        t = cdc->data_in.tail;
        io_RestoreINT();
        size = (h <= t) ? (USB_CDC_DATAIN_BUFSIZE - 1 - t + h) : (h - t - 1);
        
        if (size > free)
            size = free;
        
        if (size <= 0)
            break;
        
        while (size > 0) {
        
            io_DisableINT();
            cdc->data_in.buf[cdc->data_in.tail] = buf[cur++];
            cdc->data_in.tail = (cdc->data_in.tail + 1) & (USB_CDC_DATAIN_BUFSIZE - 1);
            io_RestoreINT();
            
            size--;
        
        }
        
    }
    
    if (cur > 0) {
    
        io_DisableINT();
        done = cdc->data_in.done;
        io_RestoreINT();
        
        if (done) {
            
            USBCDC_DataInComplete((void *)cdc, 0);
            
        }
    
    }
    
    return cur;
}

unsigned int USBCDC_RFIFOSize(USBCDC *cdc)
{
    unsigned int h, t;
    
    io_DisableINT();
    h = cdc->data_out.head;
    t = cdc->data_out.tail;
    io_RestoreINT();
    
    return (h <= t) ? (t - h) : (t + USB_CDC_DATAOUT_BUFSIZE - h);
}

unsigned int USBCDC_WFIFOSize(USBCDC *cdc)
{
    unsigned int h, t;
    
    io_DisableINT();
    h = cdc->data_in.head;
    t = cdc->data_in.tail;
    io_RestoreINT();
    
    return (h <= t) ? (t - h) : (t + USB_CDC_DATAIN_BUFSIZE - h);
}

unsigned int USBCDC_RFIFOFree(USBCDC *cdc)
{
    unsigned int h, t;
    
    io_DisableINT();
    h = cdc->data_out.head;
    t = cdc->data_out.tail;
    io_RestoreINT();
    
    return (h <= t) ? (USB_CDC_DATAOUT_BUFSIZE - 1 - t + h) : (h - t - 1);
}

unsigned int USBCDC_WFIFOFree(USBCDC *cdc)
{
    unsigned int h, t;
    
    io_DisableINT();
    h = cdc->data_in.head;
    t = cdc->data_in.tail;
    io_RestoreINT();
    
    return (h <= t) ? (USB_CDC_DATAIN_BUFSIZE - 1 - t + h) : (h - t - 1);
}

void USBCDC_RFIFOFlush(USBCDC *cdc)
{
    io_DisableINT();
    cdc->data_out.head = cdc->data_out.tail;
    io_RestoreINT();
}

void USBCDC_WFIFOFlush(USBCDC *cdc)
{
    io_DisableINT();
    cdc->data_in.head = cdc->data_in.tail;
    io_RestoreINT();
}

void USBCDC_WFIFOWaitFlush(USBCDC *cdc)
{
    bool done;
    
    while (USBCDC_WFIFOSize(cdc) > 0);
    
    do {
        
        io_DisableINT();
        done = cdc->data_in.done;
        io_RestoreINT();
        
    } while (!done);
}

bool USBCDC_CheckReady(USBCDC *cdc)
{
    bool ret;
    
    io_DisableINT();
    ret = cdc->active;
    io_RestoreINT();
    
    return ret;
}

unsigned long USBCDC_GetBaud(USBCDC *cdc)
{
    unsigned long baud;
    
    io_DisableINT();
    baud = cdc->port_line_coding.dwDTERate;
    io_RestoreINT();
    
    return baud;
}

unsigned char USBCDC_GetStopBits(USBCDC *cdc)
{
    unsigned char stopbits;
    
    io_DisableINT();
    stopbits = cdc->port_line_coding.bCharFormat;
    io_RestoreINT();
    
    return stopbits;
}

unsigned char USBCDC_GetParityType(USBCDC *cdc)
{
    unsigned char parity;
    
    io_DisableINT();
    parity = cdc->port_line_coding.bParityType;
    io_RestoreINT();
    
    return parity;
}

unsigned char USBCDC_GetDataBits(USBCDC *cdc)
{
    unsigned char databits;
    
    io_DisableINT();
    databits = cdc->port_line_coding.bDataBits;
    io_RestoreINT();
    
    return databits;
}

bool USBCDC_GetDTR(USBCDC *cdc)
{
    unsigned char ret;
    
    io_DisableINT();
    ret = (cdc->port_line_state & 0x01) ? true : false;
    io_RestoreINT();
    
    return ret;
}

bool USBCDC_GetRTS(USBCDC *cdc)
{
    unsigned char ret;
    
    io_DisableINT();
    ret = (cdc->port_line_state & 0x02) ? true : false;
    io_RestoreINT();
    
    return ret;
}

int USBCDC_ReadBreak(USBCDC *cdc)
{
    int ret;
    
    io_DisableINT();
    ret = cdc->port_break;
    cdc->port_break = -1;
    io_RestoreINT();
    
    return ret;
}

bool USBCDC_Connect(USBCDC *cdc)
{
    cdc->serial_state |= ACM_CTRL_DSR | ACM_CTRL_DCD;
    return USBCDC_SendSerialState(cdc, cdc->serial_state);
}

bool USBCDC_Disconnect(USBCDC *cdc)
{
    cdc->serial_state &= ~(ACM_CTRL_DSR | ACM_CTRL_DCD);
    return USBCDC_SendSerialState(cdc, cdc->serial_state);
}

bool USBCDC_SendBreak(USBCDC *cdc, int duration)
{
    if (duration)
        cdc->serial_state |= ACM_CTRL_BRK;
    else
        cdc->serial_state &= ~ACM_CTRL_BRK;
    
    return USBCDC_SendSerialState(cdc, cdc->serial_state);
}

void USBCDC_SetControlLineStateCB(USBCDC *cdc, void (*func)(void))
{
    cdc->control_line_state = func;
}
