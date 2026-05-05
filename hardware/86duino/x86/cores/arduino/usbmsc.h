/*
  usbmsc.h - DM&P Vortex86 USB Device Mass Storage Clase Library
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
#ifndef __USB_MASS_STORAGE_H
#define __USB_MASS_STORAGE_H

#include <stdlib.h>
#include "usbdev.h"

#ifdef DMP_DOS_DJGPP
    #ifdef HANDLE_PRAGMA_PACK_PUSH_POP
        #undef HANDLE_PRAGMA_PACK_PUSH_POP
    #endif
    #define HANDLE_PRAGMA_PACK_PUSH_POP 1   // enable "#pragma pack" in DJGPP
#endif

#if defined(DMP_DOS_DJGPP) || defined(DMP_DOS_WATCOM)
    #pragma pack(push, 1)                   // avoid to optimize memory alignment of "struct" members
#endif

typedef struct command_block_wrapper {
    
    unsigned long dCBWSignature;
    unsigned long dCBWTag;
    unsigned long dCBWDataTransferLength;
    unsigned char bmCBWFlags;
    unsigned char bCBWLUN;                  // Bit4 to Bit7: Reserved(0)
    unsigned char bCBWCBLength;             // Bit5 to Bit7: Reserved(0)
    unsigned char CBWCB[16];
    
} __attribute((packed)) USBMSC_CBW;

typedef struct command_status_wrapper {
    
    unsigned long dCSWSignature;
    unsigned long dCSWTag;
    unsigned long dCSWDataResidue;
    unsigned char bCSWStatus;
    
} __attribute((packed)) USBMSC_CSW;

#if defined(DMP_DOS_DJGPP) || defined(DMP_DOS_WATCOM)
#pragma pack(pop)
#endif

typedef struct usb_mass_storage USBMSC;

USBMSC *USBMSC_Init             (USBDev *dev, void (*suspend)(void));
void    USBMSC_Close            (USBMSC *msc);

bool    USBMSC_CheckReady       (USBMSC *msc);
bool    USBMSC_ReadData         (USBMSC *msc, unsigned char *buf, int len);
bool    USBMSC_ReadComplete     (USBMSC *msc);
bool    USBMSC_WriteData        (USBMSC *msc, unsigned char *buf, int len);
bool    USBMSC_WriteCSW         (USBMSC *msc, USBMSC_CSW *csw);
bool    USBMSC_WriteComplete    (USBMSC *msc);
void    USBMSC_DataInStall      (USBMSC *msc);
void    USBMSC_DataOutStall     (USBMSC *msc);
bool    USBMSC_ReadCBWTrigger   (USBMSC *msc);
size_t  USBMSC_GetCBWSize       (USBMSC *msc);
bool    USBMSC_ReadCBW          (USBMSC *msc, USBMSC_CBW *cbw);

#define MSC_CBW_SIGNATURE               (0x43425355)
#define MSC_CSW_SIGNATURE               (0x53425355)
#define MSC_CSW_CMD_PASSED              (0)
#define MSC_CSW_CMD_FAILED              (1)
#define MSC_CSW_PHASE_ERROR             (2)

#endif
