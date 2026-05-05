/*
  mem_disk.h - DM&P Vortex86 Memory Disk for USB Device Mass Storage Class
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
#define BS_JMPBOOT()                                    \
        do {                                            \
            dc->memory_disk[0x00] = 0xEB;               \
            dc->memory_disk[0x01] = 0x3C;               \
            dc->memory_disk[0x02] = 0x90;               \
        } while (0)

#define BS_OEMNAME()                                    \
        do {                                            \
            dc->memory_disk[0x03] = 0x00;               \
            dc->memory_disk[0x04] = 0x00;               \
            dc->memory_disk[0x05] = 0x00;               \
            dc->memory_disk[0x06] = 0x00;               \
            dc->memory_disk[0x07] = 0x00;               \
            dc->memory_disk[0x08] = 0x00;               \
            dc->memory_disk[0x09] = 0x00;               \
            dc->memory_disk[0x0A] = 0x00;               \
        } while (0)

#define BPB_BYTSPERSEC(n)                               \
        do {                                            \
            dc->memory_disk[0x0B] = n & 0xFF;           \
            dc->memory_disk[0x0C] = (n >> 8) & 0xFF;    \
        } while (0)

#define BPB_SECPERCLUSTER(n)                            \
        do {                                            \
            dc->memory_disk[0x0D] = n & 0xFF;           \
        } while (0)

#define BPB_RSVDSECCNT(n)                               \
        do {                                            \
            dc->memory_disk[0x0E] = n & 0xFF;           \
            dc->memory_disk[0x0F] = (n >> 8) & 0xFF;    \
        } while (0)

#define BPB_NUMFATS(n)                                  \
        do {                                            \
            dc->memory_disk[0x10] = n & 0xFF;           \
        } while (0)

#define BPB_ROOTENTCNT(n)                               \
        do {                                            \
            dc->memory_disk[0x11] = n & 0xFF;           \
            dc->memory_disk[0x12] = (n >> 8) & 0xFF;    \
        } while (0)

#define BPB_TOTSEC16(n)                                 \
        do {                                            \
            dc->memory_disk[0x13] = n & 0xFF;           \
            dc->memory_disk[0x14] = (n >> 8) & 0xFF;    \
        } while (0)

#define BPB_TOTSEC32(n)                                 \
        do {                                            \
            dc->memory_disk[0x20] = n & 0xFF;           \
            dc->memory_disk[0x21] = (n >> 8) & 0xFF;    \
            dc->memory_disk[0x22] = (n >> 16) & 0xFF;   \
            dc->memory_disk[0x23] = (n >> 24) & 0xFF;   \
        } while (0)

#define BPB_MEDIA()                                     \
        do {                                            \
            dc->memory_disk[0x15] = 0xF8;               \
        } while (0)

#define BPB_FATSZ16(n)                                  \
        do {                                            \
            dc->memory_disk[0x16] = n & 0xFF;           \
            dc->memory_disk[0x17] = (n >> 8) & 0xFF;    \
        } while (0)

#define BPB_SECPERTRK(n)                                \
        do {                                            \
            dc->memory_disk[0x18] = n & 0xFF;           \
            dc->memory_disk[0x19] = (n >> 8) & 0xFF;    \
        } while (0)

#define BPB_NUMHEADS(n)                                 \
        do {                                            \
            dc->memory_disk[0x1A] = n & 0xFF;           \
            dc->memory_disk[0x1B] = (n >> 8) & 0xFF;    \
        } while (0)

#define BPB_HIDDSEC(n)                                  \
        do {                                            \
            dc->memory_disk[0x1C] = n & 0xFF;           \
            dc->memory_disk[0x1D] = (n >> 8) & 0xFF;    \
            dc->memory_disk[0x1E] = (n >> 16) & 0xFF;   \
            dc->memory_disk[0x1F] = (n >> 24) & 0xFF;   \
        } while (0)

#define BS_DRVNUM(n)                                    \
        do {                                            \
            dc->memory_disk[0x24] = n & 0xFF;           \
        } while (0)

#define BS_RESERVED1()                                  \
        do {                                            \
            dc->memory_disk[0x25] = 0x00;               \
        } while (0)

#define BS_BOOTSIG(n)                                   \
        do {                                            \
            dc->memory_disk[0x26] = n & 0xFF;           \
        } while (0)

#define BS_VOLID(n)                                     \
        do {                                            \
            dc->memory_disk[0x27] = n & 0xFF;           \
            dc->memory_disk[0x28] = (n >> 8) & 0xFF;    \
            dc->memory_disk[0x29] = (n >> 16) & 0xFF;   \
            dc->memory_disk[0x2A] = (n >> 24) & 0xFF;   \
        } while (0)

#define BS_VOLLAB(s)                                    \
        do {                                            \
            int x, y;                                   \
            x = 0x2B;                                   \
            for (y = 0; y < 11; y++)                    \
                dc->memory_disk[x + y] = s[y];          \
        } while (0)

#define BS_FILSYSTYPE(s)                                \
        do {                                            \
            int x, y;                                   \
            x = 0x36;                                   \
            for (y = 0; y < 8; y++)                     \
                dc->memory_disk[x + y] = s[y];          \
        } while (0)

#define BS_END()                                        \
        do {                                            \
            dc->memory_disk[0x1FE] = 0x55;              \
            dc->memory_disk[0x1FF] = 0xAA;              \
        } while (0)

#define FDT_VOLLAB(n,s)                                 \
        do {                                            \
            int x, y;                                   \
            x = (n) * (dc->block_size);                 \
            for (y = 0; y < 11; y++)                    \
                dc->memory_disk[x + y] = s[y];          \
            dc->memory_disk[x + y] = 0x08;              \
        } while (0)

#define FAT_INIT(n)                                     \
        do {                                            \
            dc->memory_disk[0x200] = n & 0xFF;          \
            dc->memory_disk[0x201] = (n >> 8) & 0xFF;   \
            dc->memory_disk[0x202] = (n >> 16) & 0xFF;  \
            dc->memory_disk[0x203] = (n >> 24) & 0xFF;  \
        } while (0)

bool memory_disk_initialize(DISK_CTRL *dc, unsigned long Megabytes)
{
    unsigned char cluster;
    unsigned long nbytes, temp;
    unsigned short fatsect;
    
    if (Megabytes == 0)
        return false;
    
    dc->block_size = 512;
    dc->block_count = Megabytes << 11;
    nbytes = dc->block_count * dc->block_size;
    
    cluster = 1;
    temp = 32 << 11;
    while (dc->block_count >= temp) {
        
        temp <<= 1;
        cluster <<= 1;
        
    }
    fatsect = (((dc->block_count / cluster) + 255) >> 8) + 8;
    
    dc->memory_disk = (unsigned char *)malloc(nbytes);
    if (dc->memory_disk == NULL)
        return false;
    
    memset(dc->memory_disk, 0, nbytes);
    
    BS_JMPBOOT();
    BS_OEMNAME();
    BPB_BYTSPERSEC(dc->block_size);
    BPB_SECPERCLUSTER(cluster);
    BPB_RSVDSECCNT(1);
    BPB_NUMFATS(1);
    BPB_ROOTENTCNT(512);
    
    if (dc->block_count <= 0xFFFF)
        BPB_TOTSEC16(dc->block_count);
    else
        BPB_TOTSEC32(dc->block_count);
    
    BPB_MEDIA();
    BPB_FATSZ16(fatsect);
    BPB_SECPERTRK(63);
    BPB_NUMHEADS(255);
    BPB_HIDDSEC(0);
    BS_DRVNUM(0xF0);
    BS_RESERVED1();
    BS_BOOTSIG(0x29);
    BS_VOLID(0x00000000);
    BS_VOLLAB("DMP U-Disk ");
    BS_FILSYSTYPE("FAT16   ");
    BS_END();
    
    FAT_INIT(0xFFFFFFF8);
    
    FDT_VOLLAB(fatsect + 1, "DMP U-Disk ");
    
    return true;
}

void memory_disk_uninitialize(DISK_CTRL *dc)
{
    free(dc->memory_disk);
    dc->memory_disk = NULL;
}
