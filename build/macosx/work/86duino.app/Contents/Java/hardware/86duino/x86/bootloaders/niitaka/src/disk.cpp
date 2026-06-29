/*
  disk.cpp - DM&P Vortex86 Disk Control for USB Device Mass Storage Class
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
#include "disk.h"

#include <dos.h>
#include <bios.h>
#include <ctype.h>
#include <string.h>
#include <go32.h>
#include <dpmi.h>

#include <stdio.h>

#include "mem_disk.h"

#define MEMORY_DISK_MEGABYTES   (25)

#pragma pack(1)

struct DCB {

    unsigned long sector;
    unsigned short number;
    unsigned char *buffer;

} __attribute__ ((packed));

#define SECTOR_BUFFER_SIZE      (64 * 1024 * 1024)
#define DMA_SECTOR_NUM          (64)
#define DMA_BUFFER_SIZE         (DMA_SECTOR_NUM * 512 + sizeof(struct DCB))

static void lock_logical_volume(DISK_CTRL *dc)
{
    __dpmi_regs regs;
    
    regs.h.bl = dc->disk;
    regs.h.bh = 0;
    regs.x.ax = 0x440D;
    regs.x.cx = 0x084A;
    regs.x.dx = 0x0001;
    __dpmi_int(0x21, &regs);
}

static void unlock_logical_volume(DISK_CTRL *dc)
{
    __dpmi_regs regs;
    
    regs.h.bl = dc->disk;
    regs.x.ax = 0x440D;
    regs.x.cx = 0x086A;
    regs.x.dx = 0x0001;
    __dpmi_int(0x21, &regs);
}

static void absolute_disk_read(DISK_CTRL *dc)
{
    __dpmi_regs regs;
    
    regs.x.ax = dc->disk - 1;
    regs.x.dx = dc->sector;
    regs.x.cx = dc->nsects;
    regs.x.ds = dc->phyaddr / 16;
    regs.x.bx = dc->phyaddr & 15;
    __dpmi_int(0x25, &regs);
    
    dosmemget(dc->phyaddr, dc->nsects * dc->block_size, (void *)dc->buffer);
}

static void absolute_disk_write(DISK_CTRL *dc)
{
    __dpmi_regs regs;
    
    dosmemput((void *)dc->buffer, dc->nsects * dc->block_size, dc->phyaddr);

    regs.x.ax = dc->disk - 1;
    regs.x.dx = dc->sector;
    regs.x.cx = dc->nsects;
    regs.x.ds = dc->phyaddr / 16;
    regs.x.bx = dc->phyaddr & 15;
    __dpmi_int(0x26, &regs);
}

static void absolute_disk_read32(DISK_CTRL *dc)
{
    __dpmi_regs regs;
    struct DCB dcb;
    
    dcb.sector = dc->sector;
    dcb.number = dc->nsects;
    dcb.buffer = (unsigned char *)(((dc->phyaddr / 16) << 16) + sizeof(struct DCB));
    dosmemput((void *)&dcb, sizeof(struct DCB), dc->phyaddr);

    regs.x.ax = dc->disk - 1;
    regs.x.cx = 0xFFFF;
    regs.x.ds = dc->phyaddr / 16;
    regs.x.bx = dc->phyaddr & 15;

    __dpmi_int(0x25, &regs);
    
    dosmemget(dc->phyaddr + sizeof(struct DCB), dc->nsects * dc->block_size, (void *)dc->buffer);
}

static void absolute_disk_write32(DISK_CTRL *dc)
{
    __dpmi_regs regs;
    struct DCB dcb;
    
    regs.x.ax = dc->disk - 1;
    regs.x.cx = 0xffff;
    dcb.sector = dc->sector;
    dcb.number = dc->nsects;
    dcb.buffer = (unsigned char *)(((dc->phyaddr / 16) << 16) + sizeof(struct DCB));
    dosmemput((void *)&dcb, sizeof(struct DCB), dc->phyaddr);
    dosmemput((void *)dc->buffer, dc->nsects * dc->block_size, dc->phyaddr + sizeof(struct DCB));

    regs.x.ds = dc->phyaddr / 16;
    regs.x.bx = dc->phyaddr & 15;
    __dpmi_int(0x26, &regs);
}

static void extended_absolute_disk_read(DISK_CTRL *dc)
{
    __dpmi_regs regs;
    struct DCB dcb;
    
    dcb.sector = dc->sector;
    dcb.number = dc->nsects;
    dcb.buffer = (unsigned char *)(((dc->phyaddr / 16) << 16) + sizeof(struct DCB));
    dosmemput((void *)&dcb, sizeof(struct DCB), dc->phyaddr);

    regs.h.dl = dc->disk;
    regs.x.ax = 0x7305;
    regs.x.cx = 0xFFFF;
    regs.x.si = 0x0000;
    regs.x.ds = dc->phyaddr / 16;
    regs.x.bx = dc->phyaddr & 15;

    __dpmi_int(0x21, &regs);
    
    dosmemget(dc->phyaddr + sizeof(struct DCB), dc->nsects * dc->block_size, (void *)dc->buffer);
}

static void extended_absolute_disk_write(DISK_CTRL *dc)
{
    __dpmi_regs regs;
    struct DCB dcb;
    
    regs.h.dl = dc->disk;
    regs.x.ax = 0x7305;
    regs.x.cx = 0xFFFF;
    regs.x.si = 0x0001;
    dcb.sector = dc->sector;
    dcb.number = dc->nsects;
    dcb.buffer = (unsigned char *)(((dc->phyaddr / 16) << 16) + sizeof(struct DCB));
    dosmemput((void *)&dcb, sizeof(struct DCB), dc->phyaddr);
    dosmemput((void *)dc->buffer, dc->nsects * dc->block_size, dc->phyaddr + sizeof(struct DCB));

    regs.x.ds = dc->phyaddr / 16;
    regs.x.bx = dc->phyaddr & 15;
    __dpmi_int(0x21, &regs);
}

static void read_sector_from_drive(DISK_CTRL *dc)
{
    int sector, head, track;
    
    if (dc->disk < 3)
        return;
    
    sector = (dc->sector % 0x3F) + 1;
    head   = (dc->sector / 0x3F) % 0xFF;
    track  = (dc->sector / 0x3F) / 0xFF;
    
    biosdisk(2, 0x80 + dc->disk - 3, head, track, sector, dc->nsects, dc->buffer);
}

static void write_sector_from_drive(DISK_CTRL *dc)
{
    int sector, head, track;
    
    if (dc->disk < 3)
        return;
    
    sector = (dc->sector % 0x3F) + 1;
    head   = (dc->sector / 0x3F) % 0xFF;
    track  = (dc->sector / 0x3F) / 0xFF;
    
    biosdisk(3, 0x80 + dc->disk - 3, head, track, sector, dc->nsects, dc->buffer);
}

DISK_CTRL *DC_Init(char drive)
{
    char temp;
    int tmp;
    DISK_CTRL *dc;
    
    temp = toupper(drive);
    
    dc = (DISK_CTRL *)malloc(sizeof(DISK_CTRL));
    if (dc == NULL)
        return NULL;
    
    memset(dc, 0, sizeof(DISK_CTRL));
        
    dc->drive = temp;
    
    if (dc->drive < 'A' || dc->drive > 'Z') {
        
        if (memory_disk_initialize(dc, MEMORY_DISK_MEGABYTES) == false) {
            
            free(dc);
            return NULL;
            
        }
        
    } else {
        
        dc->disk  = dc->drive - 'A' + 1;
        
        if (_media_type(dc->disk) <= 0)
            return NULL;
        
        dc->buffer = (unsigned char *)malloc(SECTOR_BUFFER_SIZE);
        if (dc->buffer == NULL) {
        
            free(dc);
            return NULL;
        
        }
        
        tmp = __dpmi_allocate_dos_memory((DMA_BUFFER_SIZE + 15) >> 4, &dc->dma_handle);
        if (tmp < 0) {
            
            free(dc->buffer);
            free(dc);
            return NULL;
            
        }
        dc->phyaddr = (unsigned long)tmp << 4;
        
        
        dc->block_size = 512;
        dc->sector     = 0;
        dc->nsects     = 1;
        extended_absolute_disk_read(dc);
        
        dc->block_size = (dc->buffer[12] << 8) | dc->buffer[11];
        dc->block_count  = (dc->buffer[20] << 8) | dc->buffer[19];
        if (dc->block_count == 0) {
        
            dc->block_count = (dc->buffer[35] << 24) | (dc->buffer[34] << 16) | (dc->buffer[33] << 8) | dc->buffer[32];
        
        }
        
        if (_get_fat_size(dc->disk) == 32) {
        
            dc->read  = extended_absolute_disk_read;
            dc->write = extended_absolute_disk_write;
            dc->fat32 = true;
        
        } else if (_is_ram_drive(dc->disk) && (((unsigned long long)dc->block_count * dc->block_size) / 1024 / 1024) >= 32) {
         
            dc->read  = absolute_disk_read32;
            dc->write = absolute_disk_write32;
            dc->fat32 = false;
        
        } else if (_is_ram_drive(dc->disk)) {
        
            dc->read  = absolute_disk_read;
            dc->write = absolute_disk_write;
            dc->fat32 = false;
        
        } else {
        
            __dpmi_free_dos_memory(dc->dma_handle);
            free(dc->buffer);
            free(dc);
            return NULL;
        
        }
        lock_logical_volume(dc);
        
    }
    
    return dc;
}

void DC_Close(DISK_CTRL *dc)
{
    if (dc->memory_disk) {
        
        memory_disk_uninitialize(dc);
        
    } else {
        
        unlock_logical_volume(dc);
        __dpmi_free_dos_memory(dc->dma_handle);
        free(dc->buffer);
        free(dc);
        
    }
}

unsigned long DC_GetBlockSize(DISK_CTRL *dc)
{
    return dc->block_size;
}

unsigned long DC_GetBlockCount(DISK_CTRL *dc)
{
    return dc->block_count;
}

unsigned char *DC_ReadSector(DISK_CTRL *dc, unsigned long sector, unsigned long nsects, unsigned long *ns)
{
    if (dc->memory_disk) {
        
        dc->buffer = dc->memory_disk + sector * 512;
        *ns = nsects;
        
    } else {
    
        if (nsects > DMA_SECTOR_NUM)
            nsects = DMA_SECTOR_NUM;
        *ns = nsects;
        
        dc->sector = sector;
        dc->nsects = nsects;
        dc->read(dc);
        
        if (sector == 0 && dc->buffer[0] == 0 && dc->buffer[1] == 0 && dc->buffer[2] == 0) {
            
            if (dc->fat32) {
            
                dc->buffer[0] = 0xEB;
                dc->buffer[1] = 0x58;
                dc->buffer[2] = 0x90;
            
            } else {
            
                dc->buffer[0] = 0xEB;
                dc->buffer[1] = 0x3C;
                dc->buffer[2] = 0x90;
            
            }
            
        }
    
    }
    
    return dc->buffer;
}

unsigned char *DC_WriteSector(DISK_CTRL *dc, unsigned long sector, unsigned long nsects, unsigned long *ns)
{
    if (dc->memory_disk) {
        
        dc->buffer = dc->memory_disk + sector * 512;
        *ns = nsects;
        
    } else {
    
        if (nsects > DMA_SECTOR_NUM)
            nsects = DMA_SECTOR_NUM;
        *ns = nsects;
        
        dc->sector = sector;
        dc->nsects = nsects;
    
    }
    
    return dc->buffer;
}

void DC_ReadComplete(DISK_CTRL *dc)
{
    
}

void DC_WriteComplete(DISK_CTRL *dc)
{
    if (dc->memory_disk == NULL)
        dc->write(dc);
}
