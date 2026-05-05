/*
  usbdesc.h - DM&P Vortex86 USB Device Library
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
#ifndef __USB_DESC_H
#define __USB_DESC_H

#include "usb.h"

#ifdef DMP_DOS_DJGPP
    #ifdef HANDLE_PRAGMA_PACK_PUSH_POP
        #undef HANDLE_PRAGMA_PACK_PUSH_POP
    #endif
    #define HANDLE_PRAGMA_PACK_PUSH_POP 1   // enable "#pragma pack" in DJGPP
#endif

#if defined(DMP_DOS_DJGPP) || defined(DMP_DOS_WATCOM)
    #pragma pack(push, 1)                   // avoid to optimize memory alignment of "struct" members
#endif

// CDC-ACM
struct usb_acm_interface {
    
    struct usb_interface_assoc_descriptor   acm_iad;
    struct usb_interface_descriptor         acm_control_interface;
    struct usb_cdc_header_desc              acm_header;
    struct usb_cdc_call_mgmt_descriptor     acm_call_mgmt;
    struct usb_cdc_acm_descriptor           acm;
    struct usb_cdc_union_desc               acm_union;
    struct usb_endpoint_descriptor          acm_notify;
    struct usb_interface_descriptor         acm_data_interface;
    struct usb_endpoint_descriptor          acm_data_in;
    struct usb_endpoint_descriptor          acm_data_out;
    
} __attribute__ ((packed));

// MSC
struct usb_msc_interface {
    
    struct usb_interface_descriptor         msc_data_interface;
    struct usb_endpoint_descriptor          msc_data_in;
    struct usb_endpoint_descriptor          msc_data_out;
    
} __attribute__ ((packed));

// HID
struct usb_hid_interface {
    
    struct usb_interface_descriptor         hid_data_interface;
    struct hid_descriptor                   hid_data_desc;
    struct usb_endpoint_descriptor          hid_data_in;
    
} __attribute__ ((packed));

#if defined(DMP_DOS_DJGPP) || defined(DMP_DOS_WATCOM)
#pragma pack(pop)
#endif

extern struct usb_device_descriptor     device_desc;
extern struct usb_qualifier_descriptor  qualifier_desc;
extern struct usb_string_descriptor     string_descs[];

extern struct usb_config_descriptor     config_desc;
extern struct usb_acm_interface         acm_interface;
extern struct usb_msc_interface         msc_interface;
extern struct usb_hid_interface         hid_interface;
extern unsigned char *config_descs;
extern unsigned char *hid_report_desc;

void usb_desc_hid_report_descriptor(unsigned char *report_desc, unsigned short size);

#define USB_FULL_SPEED  (0x00)
#define USB_HIGH_SPEED  (0x80)
#define USB_CDC         (0x01)
#define USB_MSC         (0x02)
#define USB_HID         (0x04)
bool usb_desc_initialize(unsigned char class_reg);
void usb_desc_uninitialize();

#endif
