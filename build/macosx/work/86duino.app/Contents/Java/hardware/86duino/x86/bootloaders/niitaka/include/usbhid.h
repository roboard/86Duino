/*
  usbhid.h - DM&P Vortex86 USB Device HID Class Library
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
#ifndef __USB_HID_H
#define __USB_HID_H

#include "usbdev.h"

typedef struct usb_hid USBHID;

USBHID         *USBHID_Init             (USBDev *dev, void (*suspend)(void), unsigned int input_byte, unsigned int output_byte);
void            USBHID_Close            (USBHID *hid);

bool            USBHID_CheckReady       (USBHID *hid);

unsigned int    USBHID_ReadOutputReport (USBHID *hid, unsigned char *buf, int len);
unsigned int    USBHID_WriteInputReport (USBHID *hid, unsigned char *buf, int len);
unsigned int    USBHID_WFIFOSize        (USBHID *hid);
unsigned int    USBHID_WFIFOFree        (USBHID *hid);
void            USBHID_WFIFOFlush       (USBHID *hid);
void            USBHID_WFIFOWaitFlush   (USBHID *hid);

#endif
