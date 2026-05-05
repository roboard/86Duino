/*
  usbdesc.cpp - DM&P Vortex86 USB Device Library
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
#include "usbdesc.h"

#define USB_VER_11      (0x0110)
#define USB_VER_20      (0x0200)
#define VENDOR_ID       (0x0525)
#define PRODUCT_ID      (0xa4a5)
#define DEVICE_BCB      (0x0001)

// -----------------------------------
//      USB Device Descriptors
// -----------------------------------
struct usb_device_descriptor device_desc = 
{
	0x12,                                	    // bLength
	USB_DT_DEVICE,                              // bDescriptorType
	USB_VER_11,                                 // bcdUSB
	0x00,                                	    // bDeviceClass
	0x00,                                	    // bDeviceSubClass
	0x00,                                	    // bDeviceProtocol
	0x40,                	                    // bMaxPacketSize0
	VENDOR_ID,                                  // idVendor
	PRODUCT_ID,                                 // idProduct
	DEVICE_BCB,                                 // bcdDevice
	0x01,                                	    // iManufacturer
	0x02,                                	    // iProduct
	0x03,                                	    // iSerialNumber
	0x01,                                	    // bNumConfigurations
};

// -----------------------------------
//      USB Device Qualifier Descriptors
// -----------------------------------
struct usb_qualifier_descriptor qualifier_desc = 
{
    0x0A,                                       // bLength;
    USB_DT_DEVICE_QUALIFIER,                    // bDescriptorType;
    USB_VER_11,                                 // bcdUSB;
    0x00,                                       // bDeviceClass;
    0x00,                                       // bDeviceSubClass;
    0x00,                                       // bDeviceProtocol;
    0x40,                                       // bMaxPacketSize0;
    0x01,                                       // bNumConfigurations;
    0x00,                                       // bRESERVED;
};

// -----------------------------------
//         String Descriptors
// -----------------------------------
//
struct usb_string_descriptor string_descs[] =
{
	// iLanguageStr -> Index 0
	{
        0x04,                                   // bLength
        USB_DT_STRING,                          // bDescriptorType
        0x09,                                   // 0x09 for Language - English
        0x04                                    // 0x04 for Sub Language - U.S.
	},
	
	// iManufacturerStr -> Index 1
	{
        42,	                                    // bLength
        USB_DT_STRING,                          // bDescriptorType
        'D', '\0',                              // bString
        'M', '\0',
        'P', '\0',
        ' ', '\0',
        'E', '\0',
        'l', '\0',
        'e', '\0',
        'c', '\0',
        't', '\0',
        'r', '\0',
        'o', '\0',
        'n', '\0',
        'i', '\0',
        'c', '\0',
        's', '\0',
        ' ', '\0',
        'I', '\0',
        'N', '\0',
        'C', '\0',
        '.', '\0',
	},
	
	// iProductStr -> Index 2
	{
        50,                                     // bLength
        USB_DT_STRING,                          // bDescriptorType
        'D', '\0',                              // bString
        'M', '\0',
        '&', '\0',
        'P', '\0',
        ' ', '\0',
        'V', '\0',
        'o', '\0',
        'r', '\0',
        't', '\0',
        'e', '\0',
        'x', '\0',
        '8', '\0',
        '6', '\0',
        ' ', '\0',
        'U', '\0',
        'S', '\0',
        'B', '\0',
        ' ', '\0',
        'D', '\0',
        'e', '\0',
        'v', '\0',
        'i', '\0',
        'c', '\0',
        'e', '\0'
	},
	
	// iSerialNumberStr -> Index 3
	{
        12,                                     // bLength
        USB_DT_STRING,                          // bDescriptorType
        '1', '\0',                              // bString
        '.', '\0',
        '0', '\0',
        '.', '\0',
        '0', '\0',
	}
};

// -----------------------------------
//      USB Configuration Descriptors
// -----------------------------------
// Configuration Descriptor
struct usb_config_descriptor config_desc = {
    
    0x09,                                       // bLength
    USB_DT_CONFIG,                	            // bDescriptorType
    sizeof(struct usb_config_descriptor),       // wTotalLength
    0x00,                                       // bNumInterfaces
    0x01,                                       // bConfigurationValue
    0x00,                                       // iConfiguration
    USB_CONFIG_ATT_ONE,                         // bmAttributes
    DSC_MAXPOWER(500),                	        // bMaxPower
    
};

struct usb_acm_interface acm_interface = {
    
    // CDC-ACM: Interface Association Descriptor
    {
        0x08,                                   // bLength;
        USB_DT_INTERFACE_ASSOCIATION,           // bDescriptorType;
        0x00,                                   // bFirstInterface;
        0x02,                                   // bInterfaceCount;
        0x02,                                   // bFunctionClass;
        0x02,                                   // bFuntionSubClass;
        0x01,                                   // bFunctionProtocol;
        0x00,                                   // iFunction;
    },
    
    // CDC-ACM: Interface Descriptor
    {
        0x09,                                   // bLength
        USB_DT_INTERFACE,                	    // bDescriptorType
        0x00,                                   // bInterfaceNumber
        0x00,                                   // bAlternateSetting
        0x01,                                   // bNumEndpoints
        0x02,                                   // bInterfaceClass
        0x02,                                   // bInterfaceSubClass
        0x01,                                   // bInterfaceProtocol
        0x00,                                   // iInterface
    },
    
    // CDC-ACM: Header Functional Descriptor
    {
		0x05,                                   // bLength
		USB_DT_CS_INTERFACE,                    // bDescriptorType
		0x00,                                   // bDescriptorSubtype
		0x0110,                                 // bcdCDC
    },
	
    // CDC-ACM: Call Management Functional Descriptor
    {
        0x05,                                   // bLength
        USB_DT_CS_INTERFACE,                    // bDescriptorType
        0x01,                                   // bDescriptorSubtype
        0x00,                                   // bmCapabilities
        0x01,                                   // bDataInterface
    },
	
    // CDC-ACM: Abstract Control Management Functional Descriptor
    {
        0x04,                                   // bLength
        USB_DT_CS_INTERFACE,                    // bDescriptorType
        0x02,                                   // bDescriptorSubtype
        0x06,                                   // bmCapabilities
    },
	
    // CDC-ACM: Union Functional Descriptor
    {
        0x05,                                   // bLength
        USB_DT_CS_INTERFACE,                    // bDescriptorType
        0x06,                                   // bDescriptorSubtype
        0x00,                                   // bMasterInterface
        0x01,                                   // bSlaveInterface0
    },
	
	// CDC-ACM: Endpoint Descriptor (Interrupt, IN)
	{
		0x07, 								    // bLength
		USB_DT_ENDPOINT, 					    // bDescriptorType
		EP1_IN, 							    // bEndpointAddress
		USB_ENDPOINT_XFER_INT,                  // bmAttributes
		0x0A,                                   // wMaxPacketSize
		0x20,                                   // bInterval
	},
    
	// CDC-ACM: Interface Descriptor
	{
        0x09,                                   // bLength
        USB_DT_INTERFACE,                	    // bDescriptorType
        0x01,                                   // bInterfaceNumber
        0x00,                                   // bAlternateSetting
        0x02,                                   // bNumEndpoints
        0x0A,                                   // bInterfaceClass
        0x00,                                   // bInterfaceSubClass
        0x00,                                   // bInterfaceProtocol
        0x00,                                   // iInterface
	},
	
	// CDC-ACM: Endpoint Descriptor (Bulk, IN)
	{
        0x07,                                   // bLength
        USB_DT_ENDPOINT,                 	    // bDescriptorType
        EP2_IN,                         	    // bEndpointAddress
        USB_ENDPOINT_XFER_BULK,                 // bmAttributes
        0x40,                                   // wMaxPacketSize
        0x00,                                   // bInterval
	},
	
	// CDC-ACM: Endpoint Descriptor (Bulk, OUT)
	{
        0x07,                                   // bLength
        USB_DT_ENDPOINT,                 	    // bDescriptorType
        EP2_OUT,                         	    // bEndpointAddress
        USB_ENDPOINT_XFER_BULK,                 // bmAttributes
        0x40,                                   // wMaxPacketSize
        0x00,                                   // bInterval
	},
    
};

struct usb_msc_interface msc_interface = {
    
	// MSC: Interface Descriptor
	{
        0x09,                                   // bLength
        USB_DT_INTERFACE,                	    // bDescriptorType
        0x00,                                   // bInterfaceNumber
        0x00,                                   // bAlternateSetting
        0x02,                                   // bNumEndpoints
        0x08,                                   // bInterfaceClass
        0x06,                                   // bInterfaceSubClass
        0x50,                                   // bInterfaceProtocol
        0x00,                                   // iInterface
	},
	
	// MSC: Endpoint Descriptor (Bulk, IN)
	{
        0x07,                                   // bLength
        USB_DT_ENDPOINT,                 	    // bDescriptorType
        EP1_IN,                         	    // bEndpointAddress
        USB_ENDPOINT_XFER_BULK,                 // bmAttributes
        0x40,                                   // wMaxPacketSize
        0x00,                                   // bInterval
	},
	
	// MSC: Endpoint Descriptor (Bulk, OUT)
	{
        0x07,                                   // bLength
        USB_DT_ENDPOINT,                 	    // bDescriptorType
        EP1_OUT,                         	    // bEndpointAddress
        USB_ENDPOINT_XFER_BULK,                 // bmAttributes
        0x40,                                   // wMaxPacketSize
        0x00,                                   // bInterval
	},
    
};

struct usb_hid_interface hid_interface = {
    
	// HID: Interface Descriptor
	{
        0x09,                                   // bLength
        USB_DT_INTERFACE,                	    // bDescriptorType
        0x00,                                   // bInterfaceNumber
        0x00,                                   // bAlternateSetting
        0x01,                                   // bNumEndpoints
        0x03,                                   // bInterfaceClass
        0x00,                                   // bInterfaceSubClass
        0x00,                                   // bInterfaceProtocol
        0x00,                                   // iInterface
	},
    
    // HID: HID Descriptor
    {
        0x09,                                   // bLength
        0x21,                                   // bDescriptorType
        0x0111,                                 // bcdHID
        0x00,                                   // bCountryCode
        0x01,                                   // bNumDescriptors
        0x22,                                   // bSubDescriptorType
        0x00,                                   // wDescriptorLength
    },
	
	// HID: Endpoint Descriptor (Interrupt, IN)
	{
        0x07,                                   // bLength
        USB_DT_ENDPOINT,                 	    // bDescriptorType
        EP1_IN,                         	    // bEndpointAddress
        USB_ENDPOINT_XFER_INT,                  // bmAttributes
        0x40,                                   // wMaxPacketSize
        0x0A,                                   // bInterval
	},
    
};

unsigned char *hid_report_desc = NULL;
unsigned short hid_report_desc_size = 0;

unsigned char *config_descs = NULL;

void usb_desc_hid_report_descriptor(unsigned char *report_desc, unsigned short size)
{
    hid_report_desc = report_desc;
    hid_report_desc_size = size;
}

bool usb_desc_initialize(unsigned char class_reg)
{
    int size, epnum = 0;
    unsigned char *p;
    
    size = sizeof(config_desc);
    if (class_reg & USB_CDC)
        size += sizeof(acm_interface);
    if (class_reg & USB_MSC)
        size += sizeof(msc_interface);
    if (class_reg & USB_HID)
        size += sizeof(hid_interface);
    
    config_descs = (unsigned char *)malloc(size);
    if (!config_descs)
        return false;
    
    config_desc.wTotalLength = size;
    
    if (class_reg & USB_CDC) {
    
        acm_interface.acm_control_interface.bInterfaceNumber += config_desc.bNumInterfaces;
        acm_interface.acm_data_interface.bInterfaceNumber    += config_desc.bNumInterfaces;
        config_desc.bNumInterfaces += 2;
        
        acm_interface.acm_notify.bEndpointAddress   += epnum;
        acm_interface.acm_data_in.bEndpointAddress  += epnum;
        acm_interface.acm_data_out.bEndpointAddress += epnum;
        epnum += 2;
        
        if (class_reg & USB_HIGH_SPEED) {
            
            acm_interface.acm_data_in.wMaxPacketSize  = 0x200;
            acm_interface.acm_data_out.wMaxPacketSize = 0x200;
            
        }
    
    }
    
    if (class_reg & USB_MSC) {
    
        msc_interface.msc_data_interface.bInterfaceNumber += config_desc.bNumInterfaces;
        config_desc.bNumInterfaces += 1;
        
        msc_interface.msc_data_in.bEndpointAddress  += epnum;
        msc_interface.msc_data_out.bEndpointAddress += epnum;
        epnum += 1;
        
        if (class_reg & USB_HIGH_SPEED) {
            
            msc_interface.msc_data_in.wMaxPacketSize  = 0x200;
            msc_interface.msc_data_out.wMaxPacketSize = 0x200;
            
        }
    
    }
    
    if (class_reg & USB_HID) {
    
        hid_interface.hid_data_interface.bInterfaceNumber += config_desc.bNumInterfaces;
        config_desc.bNumInterfaces += 1;
        
        hid_interface.hid_data_in.bEndpointAddress += epnum;
        epnum += 1;
        
        hid_interface.hid_data_desc.wDescriptorLength = hid_report_desc_size;
        
        if (class_reg & USB_HIGH_SPEED) {
            
            hid_interface.hid_data_in.wMaxPacketSize  = 0x400;
            
        }
    
    }
    
    p = config_descs;
    memcpy(p, (unsigned char *)&config_desc, sizeof(config_desc));
    p += sizeof(config_desc);
    if (class_reg & USB_CDC) {
        
        memcpy(p, (unsigned char *)&acm_interface, sizeof(acm_interface));
        p += sizeof(acm_interface);
        
    }
    if (class_reg & USB_MSC) {
        
        memcpy(p, (unsigned char *)&msc_interface, sizeof(msc_interface));
        p += sizeof(msc_interface);
        
    }
    if (class_reg & USB_HID) {
        
        memcpy(p, (unsigned char *)&hid_interface, sizeof(hid_interface));
        p += sizeof(hid_interface);
        
    }
    
    if (class_reg & USB_HIGH_SPEED) {
        
        device_desc.bcdUSB    = USB_VER_20;
        qualifier_desc.bcdUSB = USB_VER_20;
        
    }
    
    if (epnum > 3) {
        
        usb_desc_uninitialize();
        return false;
        
    }
    
    return true;
}

void usb_desc_uninitialize()
{
    if (config_descs) {
        
        acm_interface.acm_control_interface.bInterfaceNumber = 0;
        acm_interface.acm_data_interface.bInterfaceNumber    = 1;
        msc_interface.msc_data_interface.bInterfaceNumber    = 0;
        hid_interface.hid_data_interface.bInterfaceNumber    = 0;
        
        acm_interface.acm_notify.bEndpointAddress   = EP1_IN;
        acm_interface.acm_data_in.bEndpointAddress  = EP2_IN;
        acm_interface.acm_data_out.bEndpointAddress = EP2_OUT;
        msc_interface.msc_data_in.bEndpointAddress  = EP1_IN;
        msc_interface.msc_data_out.bEndpointAddress = EP1_OUT;
        hid_interface.hid_data_in.bEndpointAddress  = EP1_IN;
        
        acm_interface.acm_data_in.wMaxPacketSize  = 0x40;
        acm_interface.acm_data_out.wMaxPacketSize = 0x40;
        msc_interface.msc_data_in.wMaxPacketSize  = 0x40;
        msc_interface.msc_data_out.wMaxPacketSize = 0x40;
        hid_interface.hid_data_in.wMaxPacketSize  = 0x40;
        
        hid_interface.hid_data_desc.wDescriptorLength = 0x00;
        
        config_desc.bNumInterfaces = 0;
            
            
        free(config_descs);
        config_descs = NULL;
        
    }
}
