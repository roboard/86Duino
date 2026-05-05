/*
  d1060.h - DM&P Vortex86EX USB Device
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
#ifndef __D1060_H
#define __D1060_H

#define D1060_CONNECT_PORT              (7)
#define D1060_CONNECT_PIN               (1)

#define DAR                             (0x00)
#define CFR                             (0x02)
#define FNR                             (0x06)
#define IER                             (0x08)
#define ISR                             (0x0C)
#define TMR                             (0x68)
            
#define EP0CtrlTR                       (0x10)
#define EP1OutTR                        (0x12)
#define EP1InTR                         (0x14)
#define EP2OutTR                        (0x16)
#define EP2InTR                         (0x18)
#define EP3OutTR                        (0x1A)
#define EP3InTR                         (0x1C)
#define EP0SetupDLR                     (0x20)
#define EP0OutDLR                       (0x24)
#define EP0InDLR                        (0x28)
#define EP1OutDLR                       (0x2C)
#define EP1InDLR                        (0x30)
#define EP2OutDLR                       (0x34)
#define EP2InDLR                        (0x38)
#define EP3OutDLR                       (0x3C)
#define EP3InDLR                        (0x40)
#define EP0SetupDSR                     (0x44)
#define EP0OutDSR                       (0x48)
#define EP0InDSR                        (0x4C)
#define EP1OutDSR                       (0x50)
#define EP1InDSR                        (0x54)
#define EP2OutDSR                       (0x58)
#define EP2InDSR                        (0x5C)
#define EP3OutDSR                       (0x60)
#define EP3InDSR                        (0x64)
            
#define IBRST                           (0x00000001L)
#define ISOF                            (0x00000002L)
#define ISUSP                           (0x00000004L)
#define IRESM                           (0x00000008L)
#define SYSERR                          (0x00000010L)
#define IEP0SETUP                       (0x00000100L)
#define IEP0RX                          (0x00000200L)
#define IEP0TX                          (0x00000400L)
#define IEP1RX                          (0x00000800L)
#define IEP1TX                          (0x00001000L)
#define IEP2RX                          (0x00002000L)
#define IEP2TX                          (0x00004000L)
#define IEP3RX                          (0x00008000L)
#define IEP3TX                          (0x00010000L)

#define D1060_INTR_MASK1_EP1            (IEP1RX | IEP1TX)
#define D1060_INTR_MASK1_EP2            (IEP2RX | IEP2TX)
#define D1060_INTR_MASK1_EP3            (IEP3RX | IEP3TX)

#define D1060_INTSTS_SYSERR             (SYSERR)
#define D1060_INTSTS_BUSRST             (IBRST)
#define D1060_INTSTS_SUSPEND            (ISUSP)
#define D1060_INTSTS_RESUME             (IRESM)
#define D1060_INTSTS_SOF                (ISOF)
#define D1060_INTSTS_EP0SETUP           (IEP0SETUP)
#define D1060_INTSTS_EP0INDMA           (IEP0TX)
#define D1060_INTSTS_EP0OUTDMA          (IEP0RX)
#define D1060_INTSTS_EP1INDMA           (IEP1TX)
#define D1060_INTSTS_EP1OUTDMA          (IEP1RX)
#define D1060_INTSTS_EP2INDMA           (IEP2TX)
#define D1060_INTSTS_EP2OUTDMA          (IEP2RX)
#define D1060_INTSTS_EP3INDMA           (IEP3TX)
#define D1060_INTSTS_EP3OUTDMA          (IEP3RX)
    
#define D1060_INTR_MASK_CTRL            (ISOF + IBRST + ISUSP + IRESM + SYSERR + IEP0SETUP + IEP0RX + IEP0TX)

#define D1060_DMA_CTL_DATA_LENGTH_MASK  (0x00001FFFUL)
#define D1060_DMA_STS_HALT              (0x00010000UL)

#define D1060_EP_NUM                    (8)
#define D1060_EP0_IN                    (0)
#define D1060_EP0_OUT                   (1)
#define D1060_EP1_IN                    (2)
#define D1060_EP1_OUT                   (3)
#define D1060_EP2_IN                    (4)
#define D1060_EP2_OUT                   (5)
#define D1060_EP3_IN                    (6)
#define D1060_EP3_OUT                   (7)

#define D1060_EPTYPE_CTRL               (0)
#define D1060_EPTYPE_ISOC               (1)
#define D1060_EPTYPE_BULK               (2)
#define D1060_EPTYPE_INTR               (3)

#define D1060_EPIDX_EP0                 (0)
#define D1060_EPIDX_EP1                 (1)
#define D1060_EPIDX_EP2                 (2)
#define D1060_EPIDX_EP3                 (3)

#define D1060_EPDIR_IN                  (0)
#define D1060_EPDIR_OUT                 (1)

#define D1060_EP0SETUP_SIZE             (8)
#define D1060_EP0IN_MAXPACKETSIZE       (64)
#define D1060_EP0OUT_MAXPACKETSIZE      (64)

#define GPIO_CONFIG_ADDR                (0xF100)
#define GPIO_BASE_ADDR                  (0xF200)
static unsigned short gpio_config_addr = 0x0000;
static unsigned short usb_on_off_data  = 0x0000;
static unsigned short usb_on_off_dir   = 0x0000;
static char usb_on_off_pin = 0;
static int usb_connect_inited = 0;

void set_gpio_config_addr(void)
{
    unsigned long value;
    
    value = sb_Read16(0x62) & 0xFFFE;
    if (!value)
        gpio_config_addr = GPIO_CONFIG_ADDR;
    else
        gpio_config_addr = value;
    value = gpio_config_addr | 0x01;
    sb_Write16(0x62, value);
}

void set_pin_out(int port, int pin)
{
    io_outpdw(gpio_config_addr + 0x00, io_inpdw(gpio_config_addr + 0x00) | 0x01L << port);

    // set data address
    if ((usb_on_off_data = io_inpw(gpio_config_addr + (4 + 4*port))) == 0x0000)
        io_outpw(gpio_config_addr + (4 + 4*port), (usb_on_off_data = (GPIO_BASE_ADDR + port*4)));
    // set direction address
    if ((usb_on_off_dir  = io_inpw(gpio_config_addr + (6 + 4*port))) == 0x0000)
        io_outpw(gpio_config_addr + (6 + 4*port), (usb_on_off_dir = (GPIO_BASE_ADDR + port*4 + 2)));

    // set USB-ONOFF pin -> OUT
    io_outpb(usb_on_off_dir, io_inpb(usb_on_off_dir)  | (1 << pin));
    io_outpb(usb_on_off_data, io_inpb(usb_on_off_data) | (1 << pin));
    usb_on_off_pin = pin;
}

void USB_Connect(void)
{
    io_outpb(usb_on_off_data, io_inpb(usb_on_off_data) & ~(1 << usb_on_off_pin));
}

void USB_Disconnect(void)
{
    io_outpb(usb_on_off_data, io_inpb(usb_on_off_data) | (1 << usb_on_off_pin));
}

static inline int d1060_get_frame(struct usb_device *dev)
{
    return io_In16(dev->io, FNR) & 0x7FF;
}

static inline void d1060_rmt_wakeup(struct usb_device *dev)
{
    
}

static inline void d1060_set_disconnect(struct usb_device *dev)
{
    if (usb_connect_inited)
        USB_Disconnect();
}

static inline void d1060_clear_disconnect(struct usb_device *dev)
{
    if (!usb_connect_inited) {
        set_gpio_config_addr();
        set_pin_out(D1060_CONNECT_PORT, D1060_CONNECT_PIN);
        usb_connect_inited = 1;
    }
    if (usb_connect_inited)
        USB_Connect();
}

static inline void d1060_write_interrupt_enable(struct usb_device *dev, unsigned long mask)
{
    io_Out32(dev->io, IER, mask);
}

static inline unsigned long d1060_read_interrupt_enable(struct usb_device *dev)
{
    return io_In32(dev->io, IER);
}

static inline void d1060_reinit(struct usb_device *dev)
{
    io_Out8(dev->io, DAR, io_In8(dev->io, DAR) | 0x80);
    io_Out8(dev->io, CFR, io_In8(dev->io, CFR) | 0x01);
}

static inline unsigned long d1060_read_interrupt_status(struct usb_device *dev)
{
    return io_In32(dev->io, ISR);
}

static inline void d1060_write_interrupt_status(struct usb_device *dev, unsigned long isr)
{
    io_Out32(dev->io, ISR, isr);
}

static void d1060_ep_enable_interrupt(struct usb_ep *ep)
{
    unsigned long value;

    value = d1060_read_interrupt_enable(ep->dev);
    if (ep->num == D1060_EPIDX_EP1 &&  ep->in) value |= IEP1TX;
    if (ep->num == D1060_EPIDX_EP1 && !ep->in) value |= IEP1RX;
    if (ep->num == D1060_EPIDX_EP2 &&  ep->in) value |= IEP2TX;
    if (ep->num == D1060_EPIDX_EP2 && !ep->in) value |= IEP2RX;
    if (ep->num == D1060_EPIDX_EP3 &&  ep->in) value |= IEP3TX;
    if (ep->num == D1060_EPIDX_EP3 && !ep->in) value |= IEP3RX;
    d1060_write_interrupt_enable(ep->dev, value);
}

static void d1060_ep_disable_interrupt(struct usb_ep *ep)
{
    unsigned long value;
    
    value = d1060_read_interrupt_enable(ep->dev);
    if (ep->num == D1060_EPIDX_EP1 &&  ep->in) value &= ~IEP1TX;
    if (ep->num == D1060_EPIDX_EP1 && !ep->in) value &= ~IEP1RX;
    if (ep->num == D1060_EPIDX_EP2 &&  ep->in) value &= ~IEP2TX;
    if (ep->num == D1060_EPIDX_EP2 && !ep->in) value &= ~IEP2RX;
    if (ep->num == D1060_EPIDX_EP3 &&  ep->in) value &= ~IEP3TX;
    if (ep->num == D1060_EPIDX_EP3 && !ep->in) value &= ~IEP3RX;
    d1060_write_interrupt_enable(ep->dev, value);
}

static void d1060_ep_enable(struct usb_ep *ep, struct usb_endpoint_descriptor *desc)
{
    unsigned long value;
    unsigned long offset;
    
    offset = 0x12 + (ep->num - 1) * 4 + ep->in * 2;
    value = desc->wMaxPacketSize & 0x7FFUL;
    value |= (desc->bmAttributes & 0x03) << 11;
    value |= 0x2000;
    io_Out16(ep->dev->io, offset, value);
    d1060_ep_enable_interrupt(ep);
}

static void d1060_ep_disable(struct usb_ep *ep)
{
    unsigned long offset;
    
    d1060_ep_disable_interrupt(ep);
    offset = 0x12 + (ep->num - 1) * 4 + ep->in * 2;
    io_Out16(ep->dev->io, offset, io_In16(ep->dev->io, offset) & ~0x2000UL);
}

static inline void d1060_setup_ep0(struct usb_device *dev)
{
    unsigned short value;
    value = (dev->ep[0].in) ? D1060_EP0IN_MAXPACKETSIZE : D1060_EP0OUT_MAXPACKETSIZE;
    value |= 0x2000UL;
    io_Out32(dev->io, EP0SetupDSR, dev->dma_setup);
    io_Out16(dev->io, EP0CtrlTR, value);
    io_Out32(dev->io, EP0SetupDLR, 0x80000000UL);
}

static void d1060_ep_set_stall(struct usb_ep *ep)
{
    unsigned long offset = 0x24 + ep->num * 8 + (ep->in ? 4 : 0);
    
    d1060_ep_disable_interrupt(ep);
    io_Out32(ep->dev->io, offset, io_In32(ep->dev->io, offset) | 0x40000000UL);
}

static void d1060_ep_clr_stall(struct usb_ep *ep)
{
    unsigned long offset = 0x24 + ep->num * 8 + (ep->in ? 4 : 0);
    
    io_Out32(ep->dev->io, offset, io_In32(ep->dev->io, offset) & ~0x40000000UL);
    d1060_ep_disable(ep);
    d1060_ep_enable(ep, ep->desc);
}

static void d1060_set_ack(struct usb_ep *ep)
{
    unsigned long offset = 0x24 + ep->num * 8 + (ep->in ? 4 : 0);
    io_Out32(ep->dev->io, offset, io_In32(ep->dev->io, offset) | 0xA0000000UL);
}

static void d1060_software_reset(struct usb_device *dev)
{
    io_Out8(dev->io, CFR, 0x02);
	while (io_In8(dev->io, CFR) & 0x02);
}

static void d1060_hardware_reset(struct usb_device *dev)
{
    d1060_software_reset(dev);
}

static void d1060_reconnect(struct usb_device *dev)
{
    d1060_clear_disconnect(dev);
}

static bool d1060_ep_is_busy(struct usb_ep *ep)
{
    unsigned long offset = 0x24 + ep->num * 8 + (ep->in ? 4 : 0);
    return (io_In32(ep->dev->io, offset) & 0x80000000UL) ? true : false;
}

static void d1060_set_address(struct usb_device *dev, int address)
{
    unsigned long config;
    
    config = io_In8(dev->io, DAR) & ~0x7F;
    config = config | dev->address;
    io_Out8(dev->io, DAR, config);
}

static unsigned int d1060_ep_done(struct usb_ep *ep, unsigned long *pstatus)
{
    unsigned long offset = 0x24 + ep->num * 8 + (ep->in ? 4 : 0);
    unsigned int size;
    unsigned long status;
    
    status = io_In32(ep->dev->io, offset);
    
    size = status & D1060_DMA_CTL_DATA_LENGTH_MASK;
    
    if (size > 0)
        dosmemget(ep->dma_buf, size, ep->buf);
    
    *pstatus = status & 0x1F0000UL;
    
    return size;
}

static void d1060_ep_init(struct usb_ep *ep, unsigned int size)
{
    struct usb_device *dev;
    unsigned long offset;
    
    dev = ep->dev;
    
    if (ep->in) {
        
        dosmemput(ep->buf, size, ep->dma_buf);
        
        offset = 0x48 + ep->num * 8 + 4;
        io_Out32(dev->io, offset, ep->dma_buf);
        offset = 0x24 + ep->num * 8 + 4;
        io_Out32(dev->io, offset, size | 0x80000000UL);
        
    } else {
        
        offset = 0x48 + ep->num * 8;
        io_Out32(dev->io, offset, ep->dma_buf);
        offset = 0x24 + ep->num * 8;
        io_Out32(dev->io, offset, size | 0x80000000UL);
        
    }
}

static inline void d1060_bus_reset(struct usb_device *dev)
{
    d1060_software_reset(dev);
    d1060_reinit(dev);
    d1060_setup_ep0(dev);
    d1060_write_interrupt_enable(dev, D1060_INTR_MASK_CTRL);
}

static inline void d1060_suspend(struct usb_device *dev)
{
    struct class_register *reg;
    
    if (dev->reset)
        dev->reset = true;
    
    
    reg = dev->class_reg;
    while (reg) {
        
        reg->suspend(reg->data);
        reg = reg->next;
        
    }
}

static inline void d1060_sof(struct usb_device *dev)
{
    unsigned long intr;
    
    if (dev->address) {
        
        if (dev->sof_interrupt == 0) {
            intr = d1060_read_interrupt_enable(dev) & ~0x02;
            d1060_write_interrupt_enable(dev, intr);
        }
        
        d1060_set_address(dev, dev->address);
        dev->address = 0;
        
    }
    
    if (dev->start_of_frame)
        dev->start_of_frame();
}

static inline void d1060_setup_stage(struct usb_device *dev)
{
    struct usb_ctrlrequest *setup = &dev->setup;
    struct usb_ep *ep = dev->in;
    struct class_register *reg;
    
    dosmemget(dev->dma_setup, 8, setup);

    switch (setup->bRequest) {
    case USB_REQ_SET_CONFIGURATION:
        if (setup->bRequestType == USB_RECIP_DEVICE) {
            
            d1060_set_ack(ep);
            reg = dev->class_reg;
            while (reg) {
                
                reg->classenable(reg->data);
                reg = reg->next;
                
            }
            
        }
        break;

    case USB_REQ_SET_INTERFACE:
        if (setup->bRequestType == USB_RECIP_INTERFACE)
            d1060_set_ack(ep);
        break;

    case USB_REQ_SET_ADDRESS:
        if (setup->bRequestType == USB_RECIP_DEVICE) {
            dev->address = setup->wValue & 0x7F;
            d1060_set_ack(ep);
        }
        break;
        
    case USB_REQ_CLEAR_FEATURE:
        if (setup->bRequestType == USB_RECIP_ENDPOINT &&
            setup->wValue       == USB_ENDPOINT_HALT  &&
            setup->wLength      == 0) {
            int idx = ((setup->wIndex & 0x7f) << 1) + ((setup->wIndex & 0x80) ? 0 : 1);
            d1060_set_ack(ep);
            if (dev->ep[idx].stall) {
                
                d1060_ep_clr_stall(&dev->ep[idx]);
                dev->ep[idx].stall = false;
                
            }
        } else if (setup->bRequestType == (USB_DIR_OUT | USB_RECIP_INTERFACE))
            d1060_set_ack(ep);
        break;
        
    case USB_REQ_SET_FEATURE:
        if (setup->bRequestType == USB_RECIP_ENDPOINT &&
            setup->wValue       == USB_ENDPOINT_HALT  &&
            setup->wLength      == 0) {
            int idx = (setup->wIndex & 0x7f) << 1;
            d1060_set_ack(ep);
            d1060_ep_set_stall(&dev->ep[idx]);
        } else if (setup->bRequestType == (USB_DIR_OUT | USB_RECIP_INTERFACE))
            d1060_set_ack(ep);
        break;

    default:
        if (setup->bRequestType & USB_DIR_IN) {
            if (setup->wLength == 0)
                d1060_set_ack(dev->out);
        } else {
            if (setup->wLength == 0)
                d1060_set_ack(ep);
        }
        break;
    }
    
    usb_control_request(dev, setup);
    d1060_setup_ep0(dev);
}

static inline void d1060_data_in(struct usb_ep *ep)
{
    int i, len;
    
    if (d1060_ep_is_busy(ep))
        return;

    i = 0;
    while (i < ep->maxpacket && (ep->dataptr - ep->databuf) < ep->datalen) {
        
        ep->buf[i++] = ep->dataptr[0];
        ep->dataptr++;
        
    }
    
    if (i > 0) {
        
        d1060_ep_init(ep, i);
        if (ep->dev->data_in)
            ep->dev->data_in();
        
    } else if ((len = ep->dataptr - ep->databuf) >= ep->datalen) {
        
        if (ep->complete)
            ep->complete(ep->data, len);
        
    }
}

static inline void d1060_data_out(struct usb_ep *ep)
{
    unsigned long size, status;
    int i, len;

    size = d1060_ep_done(ep, &status);
    
    if (status & D1060_DMA_STS_HALT) {
        
        d1060_ep_set_stall(ep);
        return;
        
    }
    
    i = 0;
    while (i < size && (ep->dataptr - ep->databuf) < ep->datalen) {
        
        ep->dataptr[0] = ep->buf[i++];
        ep->dataptr++;
    
    }
    
    if ((len = ep->dataptr - ep->databuf) >= ep->datalen || size < ep->maxpacket) {
        
        if (ep->num == 0)
            d1060_set_ack(ep->dev->in);
        if (ep->complete)
            ep->complete(ep->data, len);
        if (ep->dev->data_out)
            ep->dev->data_out();
        
    }

}

static inline void d1060_ep_init_transfer(struct usb_ep *ep)
{
    int size = ep->datalen;
    
    if (size > ep->maxpacket)
        size = ep->maxpacket;
    
    if (ep->num == 0 && ep->in)
        d1060_set_ack(ep->dev->out);
    
    if (ep->in)
        d1060_data_in(ep);
    else
        d1060_ep_init(ep, size);
}

static inline bool d1060_initialize(struct usb_device *dev)
{
    int i;
    
    for (i = 0; i < USB_EP_NUM; i++) {
        
        dev->ep[i].dev = dev;
        dev->ep[i].num = i / 2;
        dev->ep[i].in = ~i & 1;
        
        if (dev->ep[i].num == 0 && dev->ep[i].in) {
            
            dev->in = USBDev_InitEndpoint(dev, &control_in, NULL, NULL);
            USBDev_EnableEndpoint(dev->in);
            
        } else if (dev->ep[i].num == 0) {
        
            dev->out = USBDev_InitEndpoint(dev, &control_out, NULL, NULL);
            USBDev_EnableEndpoint(dev->out);
        
        }
        
    }
    
    if (!dev->in || !dev->out) {
        
        if (dev->in) {
            USBDev_DisableEndpoint(dev->in);
            USBDev_DeinitEndpoint(dev->in);
        }
        if (dev->out) {
            USBDev_DisableEndpoint(dev->out);
            USBDev_DeinitEndpoint(dev->out);
        }
        
        return false;
        
    }
    
    dev->dma_setup_handle = dma_Alloc(sizeof(struct usb_ctrlrequest), &dev->dma_setup);
    if (dev->dma_setup_handle == DMA_FAIL) {
        
        printf("ERROR: %s() alloc DMA buffer failed.\n", __FUNCTION__);
        return false;
        
    }
    
    d1060_hardware_reset(dev);
    d1060_reinit(dev);
    d1060_setup_ep0(dev);
    d1060_set_disconnect(dev);
    
    return true;
}

static inline void d1060_deinitialize(struct usb_device *dev)
{
    d1060_hardware_reset(dev);
    dma_Free(dev->dma_setup_handle);
    USBDev_DisableEndpoint(dev->in);
    USBDev_DisableEndpoint(dev->out);
    USBDev_DeinitEndpoint(dev->in);
    USBDev_DeinitEndpoint(dev->out);
}

static inline void d1060_enable_sof_int(struct usb_device *dev)
{
    unsigned long value;
    
    io_DisableINT();
    value = d1060_read_interrupt_enable(dev);
    value |= ISOF;
    d1060_write_interrupt_enable(dev, value);
    io_RestoreINT();
}

static inline void d1060_disable_sof_int(struct usb_device *dev)
{
    unsigned long value;
    
    io_DisableINT();
    value = d1060_read_interrupt_enable(dev);
    value &= ~ISOF;
    d1060_write_interrupt_enable(dev, value);
    io_RestoreINT();
}

static int d1060_isr(struct usb_device *dev)
{
    int status = ISR_NONE;
    unsigned long isr;
    
    isr = d1060_read_interrupt_status(dev);
    d1060_write_interrupt_status(dev, isr);
    
    if (isr & D1060_INTSTS_SYSERR) {
        
        status = ISR_HANDLED;
        
    }
    
    if (isr & D1060_INTSTS_BUSRST) {
        
        d1060_bus_reset(dev);
        status = ISR_HANDLED;
        
    }
    
    if (isr & D1060_INTSTS_SUSPEND) {
        
        d1060_suspend(dev);
        status = ISR_HANDLED;
        
    }
    
    if (isr & D1060_INTSTS_RESUME) {
        
        status = ISR_HANDLED;
        
    }
    
    if (isr & D1060_INTSTS_SOF) {
        
        d1060_sof(dev);
        status = ISR_HANDLED;
        
    }
    
    if (isr & D1060_INTSTS_EP0INDMA) {
        
        d1060_data_in(dev->in);
        status = ISR_HANDLED;
        
    }
    
    if (isr & D1060_INTSTS_EP0OUTDMA) {
        
        d1060_data_out(dev->out);
        status = ISR_HANDLED;
        
    }
    
    if (isr & D1060_INTSTS_EP0SETUP) {
        
        d1060_setup_stage(dev);
        status = ISR_HANDLED;
        
    }
    
    if (isr & D1060_INTSTS_EP1INDMA) {
        
        d1060_data_in(&dev->ep[D1060_EP1_IN]);
        status = ISR_HANDLED;
        
    }
    
    if (isr & D1060_INTSTS_EP1OUTDMA) {
        
        d1060_data_out(&dev->ep[D1060_EP1_OUT]);
        status = ISR_HANDLED;
        
    }
    
    if (isr & D1060_INTSTS_EP2INDMA) {
        
        d1060_data_in(&dev->ep[D1060_EP2_IN]);
        status = ISR_HANDLED;
        
    }
    
    if (isr & D1060_INTSTS_EP2OUTDMA) {
        
        d1060_data_out(&dev->ep[D1060_EP2_OUT]);
        status = ISR_HANDLED;
        
    }
    
    if (isr & D1060_INTSTS_EP3INDMA) {
        
        d1060_data_in(&dev->ep[D1060_EP3_IN]);
        status = ISR_HANDLED;
        
    }
    
    if (isr & D1060_INTSTS_EP3OUTDMA) {
        
        d1060_data_out(&dev->ep[D1060_EP3_OUT]);
        status = ISR_HANDLED;
        
    }
    
    return status;
}

bool d1060_start(USBDev *dev)
{
    d1060_setup_ep0(dev);
    
    io_DisableINT();
    d1060_write_interrupt_enable(dev, D1060_INTR_MASK_CTRL);
    io_RestoreINT();
    
    d1060_clear_disconnect(dev);
    
    return true;
}

void d1060_stop(USBDev *dev)
{
    d1060_set_disconnect(dev);
    
    io_DisableINT();
    d1060_write_interrupt_enable(dev, 0);
    io_RestoreINT();
}

bool d1060_ep_dma_alloc(struct usb_ep *ep)
{
    ep->dma_handle = dma_Alloc(ep->maxpacket, &ep->dma_buf);
    if (ep->dma_handle == DMA_FAIL) {
        
        printf("ERROR: %s() alloc DMA buffer failed.\n", __FUNCTION__);
        return false;
        
    }
    
    return true;
}

struct usb_devive_ops d1060_ops = {
    
    d1060_isr,
    d1060_start,
    d1060_stop,
    d1060_initialize,
    d1060_deinitialize,
    d1060_enable_sof_int,
    d1060_disable_sof_int,
    
    d1060_ep_dma_alloc,
    d1060_ep_enable,
    d1060_ep_disable,
    d1060_ep_init_transfer,
    d1060_ep_set_stall,
    
};

#endif
