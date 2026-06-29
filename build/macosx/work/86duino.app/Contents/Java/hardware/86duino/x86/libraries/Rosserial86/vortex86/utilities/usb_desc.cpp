/*
  usb_desc.cpp - Part of DM&P Vortex86 USB Device library
  Copyright (c) 2013 DY Hung <Dyhung@dmp.com.tw>. All right reserved.

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
////////////////////////////////////////////////////////////////////////////////
//    note that most of functions in this lib assume no paging issue when 
//    using them in ISR; so to use this lib in ISR in DJGPP, it is suggested 
//    to employ PMODE/DJ or HDPMI instead of CWSDPMI.
////////////////////////////////////////////////////////////////////////////////

#include "USB_Desc.h"

#define USB_VER							(0x0110)
#define VENDOR_ID						(0x0525)
#define PRODUCT_ID						(0xA4A5)
#define DEVICE_BCB						(0x0001)

// -----------------------------------
//      USB Standard Descriptors
// -----------------------------------
Device_Descriptor desc_Device = 
{
	0x12,									// bLength
	DSC_TYPE_DEVICE,						// bDescriptorType
	USB_VER,								// bcdUSB
	0x02,									// bDeviceClass
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
		0x0043,								// wTotalLength
		0x02,								// bNumInterfaces.
		0x01,								// bConfigurationValue
		0x04,								// iConfiguration
		DSC_CONFIG_ATR_BASE,				// bmAttributes
		DSC_MAXPOWER(100),					// bMaxPower
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
		0x01,								// bInterfaceProtocol
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

