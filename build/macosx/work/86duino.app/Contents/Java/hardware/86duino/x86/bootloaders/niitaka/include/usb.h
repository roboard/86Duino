/*
  usb.h - DM&P Vortex86 USB Device Library
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
#ifndef __USB_H
#define __USB_H

#ifdef DMP_DOS_DJGPP
    #ifdef HANDLE_PRAGMA_PACK_PUSH_POP
        #undef HANDLE_PRAGMA_PACK_PUSH_POP
    #endif
    #define HANDLE_PRAGMA_PACK_PUSH_POP 1   // enable "#pragma pack" in DJGPP
#endif

#if defined(DMP_DOS_DJGPP) || defined(DMP_DOS_WATCOM)
    #pragma pack(push, 1)                   // avoid to optimize memory alignment of "struct" members
#endif

#define USB_DIR_OUT                         (0X00)
#define USB_DIR_IN                          (0X80)

#define USB_TYPE_MASK                       (0X03 << 5)
#define USB_TYPE_STANDARD                   (0X00 << 5)
#define USB_TYPE_CLASS                      (0X01 << 5)
#define USB_TYPE_VENDOR                     (0X02 << 5)
#define USB_TYPE_RESERVED                   (0X03 << 5)

#define USB_ENDPOINT_HALT                   (0)	

#define USB_RECIP_MASK                      (0X1F)
#define USB_RECIP_DEVICE                    (0X00)
#define USB_RECIP_INTERFACE                 (0X01)
#define USB_RECIP_ENDPOINT                  (0X02)
#define USB_RECIP_OTHER                     (0X03)

#define USB_REQ_GET_STATUS                  (0X00)
#define USB_REQ_CLEAR_FEATURE               (0X01)
#define USB_REQ_SET_FEATURE                 (0X03)
#define USB_REQ_SET_ADDRESS                 (0X05)
#define USB_REQ_GET_DESCRIPTOR              (0X06)
#define USB_REQ_SET_DESCRIPTOR              (0X07)
#define USB_REQ_GET_CONFIGURATION           (0X08)
#define USB_REQ_SET_CONFIGURATION           (0X09)
#define USB_REQ_GET_INTERFACE               (0X0A)
#define USB_REQ_SET_INTERFACE               (0X0B)
#define USB_REQ_SYNCH_FRAME                 (0X0C)
#define USB_REQ_SET_SEL                     (0X30)
#define USB_REQ_SET_ISOCH_DELAY             (0X31)

#define USB_REQ_SET_ENCRYPTION              (0X0D)
#define USB_REQ_GET_ENCRYPTION              (0X0E)
#define USB_REQ_RPIPE_ABORT                 (0X0E)
#define USB_REQ_SET_HANDSHAKE               (0X0F)
#define USB_REQ_RPIPE_RESET                 (0X0F)
#define USB_REQ_GET_HANDSHAKE               (0X10)
#define USB_REQ_SET_CONNECTION              (0X11)
#define USB_REQ_SET_SECURITY_DATA           (0X12)
#define USB_REQ_GET_SECURITY_DATA           (0X13)
#define USB_REQ_SET_WUSB_DATA               (0X14)
#define USB_REQ_LOOPBACK_DATA_WRITE         (0X15)
#define USB_REQ_LOOPBACK_DATA_READ          (0X16)
#define USB_REQ_SET_INTERFACE_DS            (0X17)

#define USB_BULK_RESET_REQUEST              (0xFF)
#define USB_BULK_GET_MAX_LUN_REQUEST        (0xFE)

#define USB_CDC_SEND_ENCAPSULATED_COMMAND   (0x00)
#define USB_CDC_GET_ENCAPSULATED_RESPONSE   (0x01)
#define USB_CDC_REQ_SET_LINE_CODING         (0x20)
#define USB_CDC_REQ_GET_LINE_CODING         (0x21)
#define USB_CDC_REQ_SET_CONTROL_LINE_STATE  (0x22)
#define USB_CDC_REQ_SEND_BREAK              (0x23)

#define USB_REQ_GET_REPORT                  (0x01)
#define USB_REQ_GET_IDLE                    (0x02)
#define USB_REQ_GET_PROTOCOL                (0x03)
#define USB_REQ_SET_REPORT                  (0x09)
#define USB_REQ_SET_IDLE                    (0x0A)
#define USB_REQ_SET_PROTOCOL                (0x0B)

#define USB_DT_DEVICE                       (0x01)
#define USB_DT_CONFIG                       (0x02)
#define USB_DT_STRING                       (0x03)
#define USB_DT_INTERFACE                    (0x04)
#define USB_DT_ENDPOINT                     (0x05)
#define USB_DT_DEVICE_QUALIFIER             (0x06)
#define USB_DT_OTHER_SPEED_CONFIG           (0x07)
#define USB_DT_INTERFACE_POWER              (0x08)
#define USB_DT_INTERFACE_ASSOCIATION        (0x0B)

#define USB_DT_CS_DEVICE                    (USB_TYPE_CLASS | USB_DT_DEVICE)
#define USB_DT_CS_CONFIG                    (USB_TYPE_CLASS | USB_DT_CONFIG)
#define USB_DT_CS_STRING                    (USB_TYPE_CLASS | USB_DT_STRING)
#define USB_DT_CS_INTERFACE                 (USB_TYPE_CLASS | USB_DT_INTERFACE)
#define USB_DT_CS_ENDPOINT                  (USB_TYPE_CLASS | USB_DT_ENDPOINT)

struct usb_ctrlrequest {
    
	unsigned char  bRequestType;
	unsigned char  bRequest;
	unsigned short wValue;
	unsigned short wIndex;
	unsigned short wLength;
    
} __attribute__ ((packed));

// -----------------------------------
//      USB Standard Descriptors
// -----------------------------------
//
// Device Descriptor
struct usb_device_descriptor {
    
    unsigned char  bLength;                     // Size of this Descriptor in Bytes
    unsigned char  bDescriptorType;             // Descriptor Type (=1)
    unsigned short bcdUSB;                      // USB Spec Release Number in BCD
    unsigned char  bDeviceClass;                // Device Class Code
    unsigned char  bDeviceSubClass;             // Device Subclass Code
    unsigned char  bDeviceProtocol;             // Device Protocol Code
    unsigned char  bMaxPacketSize0;             // Maximum Packet Size for EP0
    unsigned short idVendor;                    // Vendor ID
    unsigned short idProduct;                   // Product ID
    unsigned short bcdDevice;                   // Device Release Number in BCD
    unsigned char  iManufacturer;               // Index of String Desc for Manufacturer
    unsigned char  iProduct;                    // Index of String Desc for Product
    unsigned char  iSerialNumber;               // Index of String Desc for SerNo
    unsigned char  bNumConfigurations;          // Number of possible Configurations
    
} __attribute__ ((packed));
//
// Device Qualifier Descriptor
struct usb_qualifier_descriptor {
    
    unsigned char  bLength;
    unsigned char  bDescriptorType;
    unsigned short bcdUSB;
    unsigned char  bDeviceClass;
    unsigned char  bDeviceSubClass;
    unsigned char  bDeviceProtocol;
    unsigned char  bMaxPacketSize0;
    unsigned char  bNumConfigurations;
    unsigned char  bRESERVED;
    
} __attribute__ ((packed));
//
// Configuration Descriptor
struct usb_config_descriptor {
    
    unsigned char  bLength;                     // Size of this Descriptor in Bytes
    unsigned char  bDescriptorType;             // Descriptor Type (=2)
    unsigned short wTotalLength;                // Total Length of Data for this Conf
    unsigned char  bNumInterfaces;              // number of Interfaces supported by Conf
    unsigned char  bConfigurationValue;         // Designator Value for *this* Conf
    unsigned char  iConfiguration;              // Index of String Desc for this Conf
    unsigned char  bmAttributes;                // Configuration Characteristics
    unsigned char  bMaxPower;                   // Max. Power Consumption in Conf (*2mA)
    
} __attribute__ ((packed));
//
// Interface Descriptor
struct usb_interface_descriptor {
    
    unsigned char  bLength;                     // Size of this Descriptor in Bytes
    unsigned char  bDescriptorType;             // Descriptor Type (=4)
    unsigned char  bInterfaceNumber;            // Number of *this* Interface (0..)
    unsigned char  bAlternateSetting;           // Alternative for this Interface
    unsigned char  bNumEndpoints;               // No of EPs used by this IF (excl. EP0)
    unsigned char  bInterfaceClass;             // Interface Class Code
    unsigned char  bInterfaceSubClass;          // Interface Subclass Code
    unsigned char  bInterfaceProtocol;          // Interface Protocol Code
    unsigned char  iInterface;                  // Index of String Desc for Interface
    
} __attribute__ ((packed));
//
// Endpoint Descriptor
struct usb_endpoint_descriptor {
    
    unsigned char  bLength;                     // Size of this Descriptor in Bytes
    unsigned char  bDescriptorType;             // Descriptor Type (=5)
    unsigned char  bEndpointAddress;            // Endpoint Address (Number + Direction)
    unsigned char  bmAttributes;                // Endpoint Attributes (Transfer Type)
    unsigned short wMaxPacketSize;              // Max. Endpoint Packet Size
    unsigned char  bInterval;                   // Polling Interval (Interrupt) ms
    
} __attribute__ ((packed));

// String Descriptor
struct usb_string_descriptor {
    
    unsigned char bLength;
    unsigned char bDescriptorType;
    char bString[256];
    
} __attribute__ ((packed));

struct usb_interface_assoc_descriptor {
    
    unsigned char  bLength;
    unsigned char  bDescriptorType;
    unsigned char  bFirstInterface;
    unsigned char  bInterfaceCount;
    unsigned char  bFunctionClass;
    unsigned char  bFuntionSubClass;
    unsigned char  bFunctionProtocol;
    unsigned char  iFunction;
    
} __attribute__ ((packed));

struct usb_cdc_header_desc {
    
    unsigned char  bLength;
    unsigned char  bDescriptorType;
    unsigned char  bDescriptorSubtype;
    unsigned short bcdCDC;
    
} __attribute__ ((packed));

struct usb_cdc_call_mgmt_descriptor {
    
    unsigned char  bLength;
    unsigned char  bDescriptorType;
    unsigned char  bDescriptorSubtype;
    unsigned char  bmCapabilities;
    unsigned char  bDataInterface;
    
} __attribute__ ((packed));

struct usb_cdc_acm_descriptor {
    
    unsigned char  bLength;
    unsigned char  bDescriptorType;
    unsigned char  bDescriptorSubtype;
    unsigned char  bmCapabilities;
    
} __attribute__ ((packed));

struct usb_cdc_union_desc {
    
    unsigned char  bLength;
    unsigned char  bDescriptorType;
    unsigned char  bDescriptorSubtype;
    unsigned char  bMasterInterface;
    unsigned char  bSlaveInterface0;
    
} __attribute__ ((packed));

struct usb_cdc_line_coding {
    
    unsigned long dwDTERate;
    unsigned char bCharFormat;
    #define USB_CDC_1_STOP_BITS     (0)
    #define USB_CDC_1_5_STOP_BITS   (1)
    #define USB_CDC_2_STOP_BITS     (2)

    unsigned char bParityType;
    #define USB_CDC_NO_PARITY       (0)
    #define USB_CDC_ODD_PARITY      (1)
    #define USB_CDC_EVEN_PARITY     (2)
    #define USB_CDC_MARK_PARITY     (3)
    #define USB_CDC_SPACE_PARITY    (4)

    unsigned char bDataBits;
    
} __attribute__ ((packed));

#define USB_CDC_NOTIFY_NETWORK_CONNECTION   (0x00)
#define USB_CDC_NOTIFY_RESPONSE_AVAILABLE   (0x01)
#define USB_CDC_NOTIFY_SERIAL_STATE         (0x20)
#define USB_CDC_NOTIFY_SPEED_CHANGE         (0x2A)

struct usb_cdc_notification {
    
    unsigned char  bmRequestType;
    unsigned char  bNotificationType;
    unsigned short wValue;
    unsigned short wIndex;
    unsigned short wLength;
    
} __attribute__ ((packed));

#define ACM_CTRL_DCD                        (0x01)
#define ACM_CTRL_DSR                        (0x02)
#define ACM_CTRL_BRK                        (0x04)
#define ACM_CTRL_RI                         (0x08)
#define ACM_CTRL_FRAMING                    (0x10)
#define ACM_CTRL_PARITY                     (0x20)
#define ACM_CTRL_OVERRUN                    (0x40)

struct hid_descriptor {
    
    unsigned char  bLength;
    unsigned char  bDescriptorType;
    unsigned short bcdHID;
    unsigned char  bCountryCode;
    unsigned char  bNumDescriptors;
    
    unsigned char  bSubDescriptorType;
    unsigned short wDescriptorLength;

} __attribute__ ((packed));

#if defined(DMP_DOS_DJGPP) || defined(DMP_DOS_WATCOM)
#pragma pack(pop)
#endif

//---------------------------------------------
//  Definition of Standard Descriptor Fields
//---------------------------------------------
//
// Configuration Descriptor -> bmAttributes
#define USB_CONFIG_ATT_ONE              (1 << 7)
#define USB_CONFIG_ATT_SELFPOWER        (1 << 6)
#define USB_CONFIG_ATT_WAKEUP           (1 << 5)
#define USB_CONFIG_ATT_BATTERY          (1 << 4)
//
// Configuration Descriptor -> bMaxPower
#define DSC_MAXPOWER(mA)                ((mA + 1) >> 1)
//
// Endpoint Descriptor -> bEndpointAddress
#define EP1_OUT                        	(0x01)
#define EP2_OUT                        	(0x02)
#define EP3_OUT                        	(0x03)
#define EP1_IN                        	(0x81)
#define EP2_IN                        	(0x82)
#define EP3_IN                        	(0x83)
//
// Endpoint Descriptor -> bmAttributes
#define USB_ENDPOINT_XFER_CONTROL       (0x00)
#define USB_ENDPOINT_XFER_ISOC          (0x01)
#define USB_ENDPOINT_XFER_BULK          (0x02)
#define USB_ENDPOINT_XFER_INT           (0x03)

#endif
