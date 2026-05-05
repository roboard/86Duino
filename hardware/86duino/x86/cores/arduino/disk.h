/*
  disk.h - DM&P Vortex86 Disk Control for USB Device Mass Storage Class
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
#ifndef __DISK_H
#define __DISK_H

typedef struct disk_ctrl {
    
    char drive;
    int  disk;
    
    int media;
    int ramdisk;
    
    int SPT;
    int HPC;
    unsigned long max_nsects;
    
    unsigned char *buffer;
    unsigned long long sector;
    unsigned long nsects;
    
    unsigned short block_size;
    unsigned long long block_count;
    
    int dma_handle;
    unsigned long phyaddr;
    
    void (*read) (struct disk_ctrl *);
    void (*write)(struct disk_ctrl *);
    
    unsigned char *memory_disk;
    
} DISK_CTRL;

/* Parameter 'drive' 0 for Memory Disk.
                     1 for 1st Hard Disk.
                     2 for 2nd Hard Disk.
                     3 for 3rd Hard Disk.
                     ...
                     A for Drive A.
                     B for Drive B.
                     C for Drive C.
                     D for Drive D.
                     ...
                     Z for Drive Z. */
DISK_CTRL      *DC_Init         (char drive);
void            DC_Close        (DISK_CTRL *dc);
unsigned long   DC_GetBlockSize (DISK_CTRL *dc);
unsigned long   DC_GetBlockCount(DISK_CTRL *dc);
unsigned char  *DC_ReadSector   (DISK_CTRL *dc, unsigned long long sector, unsigned long nsects, unsigned long *ns);
unsigned char  *DC_WriteSector  (DISK_CTRL *dc, unsigned long long sector, unsigned long nsects, unsigned long *ns);
void            DC_ReadComplete (DISK_CTRL *dc);
void            DC_WriteComplete(DISK_CTRL *dc);

#endif
