/*
  usb_desc.cpp - Part of DM&P Vortex86 USB Device library
  Copyright (c) 2013 DY Hung <Dyhung@dmp.com.tw>. All right reserved.
  2014/06 Modified by Android Lin <acen@dmp.com.tw>.

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

#define __DMP_USB_DESC_LIB

#include "USBDesc.h"

#define USB_VER						(0x0110)
#define VENDOR_ID					(0x0525)
#define PRODUCT_ID					(0x8036)

/*
#if defined __86DUINO_EDUCAKE
	#define PRODUCT_ID					(0x8038)
#elif defined __86DUINO_ONE
	#define PRODUCT_ID					(0x8037)
#elif defined __86DUINO_ZERO
	#define PRODUCT_ID					(0x8036)
#endif
*/

#define DEVICE_BCB					(0x0001)

// -----------------------------------
//      USB Standard Descriptors
// -----------------------------------
Device_Descriptor desc_Device = 
{
	0x12,									// bLength
	DSC_TYPE_DEVICE,						// bDescriptorType
	USB_VER,								// bcdUSB
	0x00,									// bDeviceClass // 2014.06.12 modified: 0x02 -> 0x00
	0x00,									// bDeviceSubClass
	0x00,									// bDeviceProtocol
	EP0_MAX_PACKET_SIZE,					// bMaxPacketSize0
	VENDOR_ID,								// idVendor
	PRODUCT_ID,								// idProduct
	DEVICE_BCB,								// bcdDevice
	0x01,									// iManufacturer
	0x02,									// iProduct
	0x03,									// iSerialNumber
	0x01,									// bNumConfigurations
};



// -----------------------------------
//      USB CDC ACM Descriptors
// -----------------------------------
Configuration_Desc_Set desc_Config_Set =
{
	// Configuration Descriptor
	{
		0x09,								// bLength
		DSC_TYPE_CONFIG,					// bDescriptorType
		0x0064,								// wTotalLength   // 2014.06.11 modified: 0x43 -> 0x64
		0x03,								// bNumInterfaces // 2014.06.11 modified: 0x02 -> 0x03
		0x01,								// bConfigurationValue
		0x04,								// iConfiguration
		DSC_CONFIG_ATR_BASE,				// bmAttributes
		DSC_MAXPOWER(500),					// bMaxPower // 2014.06.11 modified: 100 -> 500
	},
	
	// Interface Association Descriptor // 2014.06.11 modified: new
	{
		0x08,								// bLength
	    DSC_TYPE_INTERFACE_ASSOCIATION,		// bDescriptorType
	    0x00,                               // bFirstInterface
	    0x02,                               // bInterfaceCount
	    0x02,                               // bFunctionClass (CDC_COMMUNICATION_INTERFACE_CLASS)
	    0x02,                               // bFuntionSubClass (CDC_ABSTRACT_CONTROL_MODEL)
	    0x01,                               // bFunctionProtocol
	    0x00,                               // iFunction
	},
	
	// Communication Interface Class Descriptor
	{
		0x09,								// bLength
		DSC_TYPE_INTERFACE,					// bDescriptorType
		0x00,								// bInterfaceNumber
		0x00,								// bAlternateSetting
		0x01,								// bNumEndpoints
		0x02,								// bInterfaceClass
		0x02,								// bInterfaceSubClass
		0x01,								// bInterfaceProtocol (AT command)
		0x05,								// iInterface
	},
	
	// Header Functional Descriptor
	{
		0x05,								// bLength
		DSC_TYPE_CS_INTERFACE,				// bDescriptorType
		DSC_SUBTYPE_CS_HEADER,				// bDescriptorSubtype
		0x0110,								// bcdCDC
	},
	
	// Call Management Functional Descriptor
	{
		0x05,								// bLength
		DSC_TYPE_CS_INTERFACE,				// bDescriptorType
		DSC_SUBTYPE_CS_CALL_MANAGE,			// bDescriptorSubtype
		0x00,								// bmCapabilities
		0x01,								// bDataInterface
	},
	
	// Abstract Control Management Functional Descriptor
	{
		0x04,								// bLength
		DSC_TYPE_CS_INTERFACE,				// bDescriptorType
		DSC_SUBTYPE_CS_ABST_CONTROL,		// bDescriptorSubtype
		0x06,								// bmCapabilities
	},
	
	// Union Functional Descriptor
	{
		0x05,								// bLength
		DSC_TYPE_CS_INTERFACE,				// bDescriptorType
		DSC_SUBTYPE_CS_UNION,				// bDescriptorSubtype
		0x00,								// bMasterInterface
		0x01,								// bSlaveInterface0
	},
	
	// Endpoint Descriptor (Interrupt, EP1_IN)
	{
		0x07, 								// bLength
		DSC_TYPE_ENDPOINT, 					// bDescriptorType
		EP1_IN, 							// bEndpointAddress
		DSC_EP_INTERRUPT, 					// bmAttributes
		EP1_MAX_PACKET_SIZE_IN, 			// wMaxPacketSize
		0xFF, 								// bInterval
	},
	
	// Data Interface Class Descriptor
	{
		0x09,								// bLength
		DSC_TYPE_INTERFACE,					// bDescriptorType
		0x01,								// bInterfaceNumber
		0x00,								// bAlternateSetting
		0x02,								// bNumEndpoints
		0x0A,								// bInterfaceClass
		0x00,								// bInterfaceSubClass
		0x00,								// bInterfaceProtocol
		0x06,								// iInterface
	},
	
	// Endpoint Descriptor (Bulk, EP2_IN)
	{
		0x07, 								// bLength
		DSC_TYPE_ENDPOINT, 					// bDescriptorType
		EP2_IN, 							// bEndpointAddress
		DSC_EP_BULK, 						// bmAttributes
		EP2_MAX_PACKET_SIZE_IN, 			// wMaxPacketSize
		0x00, 								// bInterval
	},
	
	// Endpoint Descriptor (Interrupt, EP2_OUT)
	{
		0x07, 								// bLength
		DSC_TYPE_ENDPOINT, 					// bDescriptorType
		EP2_OUT, 							// bEndpointAddress
		DSC_EP_BULK, 						// bmAttributes
		EP2_MAX_PACKET_SIZE_OUT, 			// wMaxPacketSize
		0x00, 								// bInterval
	},
	
	// HID Interface Class Descriptor
	{
		0x09,								// bLength
		DSC_TYPE_INTERFACE,					// bDescriptorType
		0x02,                               // bInterfaceNumber
		0x00,								// bAlternateSetting
		0x01,								// bNumEndpoints
		0x03,								// bInterfaceClass
		0x00,								// bInterfaceSubClass
		0x00,								// bInterfaceProtocol
		0x00,								// iInterface
	},
	
	// HID Descriptor
	{
		0x09,								// bLength
		0x21,                               // bDescriptorType (HID)
		0x0101,                             // bcdHID (1.01 version)
		0x21,                               // bCountryCode (USA)
		0x01,                               // bNumDescriptors
		0x22,                               // bDescriptorType
		0x0065,                             // bDescriptorLength
	},
	
	// Endpoint Descriptor (Interrupt, EP3_IN)
	{
		0x07, 								// bLength
		DSC_TYPE_ENDPOINT, 					// bDescriptorType
		EP3_IN, 							// bEndpointAddress
		DSC_EP_INTERRUPT, 					// bmAttributes
		EP3_MAX_PACKET_SIZE_OUT, 			// wMaxPacketSize
		0x01, 								// bInterval (1ms)
	},
	
};

// -----------------------------------
//         String Descriptors
// -----------------------------------
#define DSTRLEN(str) (sizeof(str)*2)
//
String_Descriptor StringDescTable[] =
{
	// iLanguageStr -> Index 0
	{
		0x04,								// bLength
		DSC_TYPE_STRING,					// bDescriptorType
		0x0409								// 0x09 for Language - English
											// 0x04 for Sub Language - U.S.
	},
	
	// iManufacturerStr -> Index 1
	{
		DSTRLEN("DMP Electronics INC."),	// bLength
		DSC_TYPE_STRING,					// bDescriptorType
		'D',								// bString
		'M',
		'P',
		' ',
		'E',
		'l',
		'e',
		'c',
		't',
		'r',
		'o',
		'n',
		'i',
		'c',
		's',
		' ',
		'I',
		'N',
		'C',
		'.',
	},
	
	// iProductStr -> Index 2
	{
		DSTRLEN("Vortex86EX USB Device"),	// bLength
		DSC_TYPE_STRING,					// bDescriptorType
		'V',								// bString
		'o',
		'r',
		't',
		'e',
		'x',
		'8',
		'6',
		'E',
		'X',
		' ',
		'U',
		'S',
		'B',
		' ',
		'D',
		'e',
		'v',
		'i',
		'c',
		'e'
	},
	
	// iSerialNumberStr -> Index 3
	{
		DSTRLEN("1.0.0"),					// bLength
		DSC_TYPE_STRING,					// bDescriptorType
		'1', 								// bString
		'.',
		'0',
		'.',
		'0'
	},
	
	// iConfigurationStr -> Index 4
	{
		DSTRLEN("EX Configuration"),		// bLength
		DSC_TYPE_STRING,					// bDescriptorType
		'E',								// bString
		'X',
		' ',
		'c',
		'o',
		'n',
		'f',
		'i',
		'g',
		'u',
		'r',
		'a',
		't',
		'i',
		'o',
		'n',
	},
	
	// iCommInterfaceStr -> Index 5
	{
		DSTRLEN("EX Interface 0"),			// bLength
		DSC_TYPE_STRING,					// bDescriptorType
		'E',								// bString
		'X',
		' ',
		'I',
		'n',
		't',
		'e',
		'r',
		'f',
		'a',
		'c',
		'e',
		' ',
		'0',
	},
	
	// iDataInterfaceStr -> Index 6
	{
		DSTRLEN("EX Interface 1"),			// bLength
		DSC_TYPE_STRING,					// bDescriptorType
		'E',								// bString
		'X',
		' ',
		'I',
		'n',
		't',
		'e',
		'r',
		'f',
		'a',
		'c',
		'e',
		' ',
		'1',
	}
};

// -----------------------------------
//       HID Report Descriptors
// -----------------------------------
const char hidReportDescriptor[101] = {
	
	//	Mouse
    0x05, 0x01,                    // USAGE_PAGE (Generic Desktop)	// 54
    0x09, 0x02,                    // USAGE (Mouse)
    0xa1, 0x01,                    // COLLECTION (Application)
    0x09, 0x01,                    //   USAGE (Pointer)
    0xa1, 0x00,                    //   COLLECTION (Physical)
    0x85, 0x01,                    //     REPORT_ID (1)
    0x05, 0x09,                    //     USAGE_PAGE (Button)
    0x19, 0x01,                    //     USAGE_MINIMUM (Button 1)
    0x29, 0x03,                    //     USAGE_MAXIMUM (Button 3)
    0x15, 0x00,                    //     LOGICAL_MINIMUM (0)
    0x25, 0x01,                    //     LOGICAL_MAXIMUM (1)
    0x95, 0x03,                    //     REPORT_COUNT (3)
    0x75, 0x01,                    //     REPORT_SIZE (1)
    0x81, 0x02,                    //     INPUT (Data,Var,Abs)
    0x95, 0x01,                    //     REPORT_COUNT (1)
    0x75, 0x05,                    //     REPORT_SIZE (5)
    0x81, 0x03,                    //     INPUT (Cnst,Var,Abs)
    0x05, 0x01,                    //     USAGE_PAGE (Generic Desktop)
    0x09, 0x30,                    //     USAGE (X)
    0x09, 0x31,                    //     USAGE (Y)
    0x09, 0x38,                    //     USAGE (Wheel)
    0x15, 0x81,                    //     LOGICAL_MINIMUM (-127)
    0x25, 0x7f,                    //     LOGICAL_MAXIMUM (127)
    0x75, 0x08,                    //     REPORT_SIZE (8)
    0x95, 0x03,                    //     REPORT_COUNT (3)
    0x81, 0x06,                    //     INPUT (Data,Var,Rel)
    0xc0,                          //   END_COLLECTION
    0xc0,                          // END_COLLECTION
	
	//	Keyboard
    0x05, 0x01,                    // USAGE_PAGE (Generic Desktop)	// 47
    0x09, 0x06,                    // USAGE (Keyboard)
    0xa1, 0x01,                    // COLLECTION (Application)
    0x85, 0x02,                    //   REPORT_ID (2)
    0x05, 0x07,                    //   USAGE_PAGE (Keyboard)
   
	0x19, 0xe0,                    //   USAGE_MINIMUM (Keyboard LeftControl)
    0x29, 0xe7,                    //   USAGE_MAXIMUM (Keyboard Right GUI)
    0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
    0x25, 0x01,                    //   LOGICAL_MAXIMUM (1)
    0x75, 0x01,                    //   REPORT_SIZE (1)
    
	0x95, 0x08,                    //   REPORT_COUNT (8)
    0x81, 0x02,                    //   INPUT (Data,Var,Abs)
    0x95, 0x01,                    //   REPORT_COUNT (1)
    0x75, 0x08,                    //   REPORT_SIZE (8)
    0x81, 0x03,                    //   INPUT (Cnst,Var,Abs)
    
	0x95, 0x06,                    //   REPORT_COUNT (6)
    0x75, 0x08,                    //   REPORT_SIZE (8)
    0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
    0x25, 0x65,                    //   LOGICAL_MAXIMUM (101)
    0x05, 0x07,                    //   USAGE_PAGE (Keyboard)
    
	0x19, 0x00,                    //   USAGE_MINIMUM (Reserved (no event indicated))
    0x29, 0x65,                    //   USAGE_MAXIMUM (Keyboard Application)
    0x81, 0x00,                    //   INPUT (Data,Ary,Abs)
    0xc0,                          // END_COLLECTION
};
