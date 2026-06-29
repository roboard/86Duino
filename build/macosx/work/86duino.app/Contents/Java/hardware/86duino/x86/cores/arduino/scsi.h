/*
  scsi.h - DM&P Vortex86 SCSI Library for USB Device Mass Storage Class
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
#ifndef __SCSI_H
#define __SCSI_H

#include "disk.h"
#include "usbmsc.h"

typedef struct scsi_msc {
    
    DISK_CTRL *dc;
    
    USBMSC *msc;
    USBMSC_CBW cbw;
    USBMSC_CSW csw;
    
    bool disconnect;
    
    unsigned char TxBuf[4096];
    unsigned char RxBuf[4096];
    
    int parse_state;
    int write_state;
    int read_state;
    void (*poll_state)(struct scsi_msc *);
    
    unsigned long long sector;
    unsigned long nsects;
    
    
} SCSI_MSC;

SCSI_MSC   *SCSI_Init      (USBMSC *msc, DISK_CTRL *dc);
void        SCSI_Close     (SCSI_MSC *scsi);
void        SCSI_Suspend   (SCSI_MSC *scsi);
void        SCSI_Poll      (SCSI_MSC *scsi);

#endif
