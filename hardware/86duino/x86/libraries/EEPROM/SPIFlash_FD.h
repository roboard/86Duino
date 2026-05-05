/*
  SPIFlash.cpp - Part of DM&P Vortex86 EEPROM library
  Copyright (c) 2013 Vic Chen <vic@dmp.com.tw>. All right reserved.

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

#include "SPIFlash.h"

#include <io.h>
#include <unistd.h>

#define SECTORSIZE 4096

#include <string.h>
static unsigned char reg_sb_c4 = 0;
static unsigned char spi_ctrl_reg = 0;
static unsigned long spi_base = 0;
static unsigned long flash_size = 0;
static unsigned long EEPROM_offset = 0;

static unsigned char TX_DATA = 0;

static unsigned char exchange_spi_byte(unsigned short iobase, unsigned char n)
{
    io_outpb(iobase, n);
    while((io_inpb(iobase + 8) & 0x10) == 0);
    
    return io_inpb(iobase + 4);
}

static void enable_cs(unsigned short iobase)
{
  io_outpb(iobase + 0x0A, 0);
}

static void disable_cs(unsigned short iobase)
{
  io_outpb(iobase + 0x0A, 1);
}

void write_spi_24bit_addr(unsigned short iobase, unsigned long addr)
{
  if(flash_size <= 16L * 1024 * 1024)
  {
    exchange_spi_byte(iobase, (unsigned char)((addr >> 16) & 0xff));
    exchange_spi_byte(iobase, (unsigned char)((addr >> 8) & 0xff));
    exchange_spi_byte(iobase, (unsigned char)((addr) & 0xff));
  }
  else
  {
    exchange_spi_byte(iobase, (unsigned char)((addr >> 24) & 0xff));
    exchange_spi_byte(iobase, (unsigned char)((addr >> 16) & 0xff));
    exchange_spi_byte(iobase, (unsigned char)((addr >> 8) & 0xff));
    exchange_spi_byte(iobase, (unsigned char)((addr) & 0xff));  
  }
}



struct flash_info
{
  unsigned char id[3];
  const char *name;
  unsigned long size;
  unsigned long sectorbias;
};

static struct flash_info* f_info;

static const struct flash_info flash_info_table[] =
{
  {{0xc2, 0x20, 0x12}, "MX25L2005",         256L * 1024, 0x7F000},
  {{0xc2, 0x20, 0x15}, "MX25L1605",    2L * 1024 * 1024, 0x7F000},
  {{0xc2, 0x20, 0x16}, "MX25L3206E",   4L * 1024 * 1024, 0x3FF000},
  {{0xc2, 0x20, 0x17}, "MX25L6406E",   8L * 1024 * 1024, 0x7FF000},
  {{0xc2, 0x20, 0x19}, "MX25L25735E", 32L * 1024 * 1024, 0x1FFF000},
  {{0xc2, 0x20, 0x1A}, "MX25L51245E", 64L * 1024 * 1024, 0x3FFF000},
  {{0xc2, 0x25, 0x37}, "MX25U6435F",   8L * 1024 * 1024, 0x7F000},
  {{0xc2, 0x25, 0x38}, "MX25U12835F", 16L * 1024 * 1024, 0x7F000},
  {{0x1c, 0x38, 0x13}, "EN25S40",           512L * 1024, 0x7F000},
  {{0x1c, 0x38, 0x17}, "EN25S64",          8192L * 1024, 0x7FF000},
  {{0x1c, 0x30, 0x14}, "EN25F16",             2L * 1024, 0x1FF000}
};

static const int flash_info_table_size = sizeof(flash_info_table) / sizeof(flash_info_table[0]);

static const flash_info* get_flash_info(unsigned char id[3])
{
  int i;
  for(i = 0; i < flash_info_table_size; i++)
  {
    if(memcmp(id, flash_info_table[i].id, 3) == 0)
	{
		return flash_info_table + i;
	}
  }
  return NULL;
}

static void read_flash_device_id(unsigned char *p)
{
  enable_cs(spi_base);
  exchange_spi_byte(spi_base, 0x9f);// RDID command
  //disable_cs(spi_base);
  
  //read device ID
  //enable_cs(spi_base);
  p[0] = exchange_spi_byte(spi_base, 0x9f);
  p[1] = exchange_spi_byte(spi_base, 0x9f);
  p[2] = exchange_spi_byte(spi_base, 0x9f);
  disable_cs(spi_base);
}

#define SPI_MODE0 0x00 // CPOL = 0, CPHA = 0
#define SPI_MODE1 0x02 // CPOL = 0, CPHA = 1
#define SPI_MODE2 0x04 // CPOL = 1, CPHA = 0
#define SPI_MODE3 0x06 // CPOL = 1, CPHA = 1
static void set_flash_writable()
{
  //io_DisableINT();

  reg_sb_c4 = sb_Read8(0xc4);
  
  //set sb register 0xC4
  sb_Write8(0xc4, reg_sb_c4 | 1);

  //set afdis bit for EX2
  io_outpb(spi_base + 0x13, 0x07); // Reset_Spi_Module
  while(io_inpb(spi_base + 0x13) != 0); // wait for FIFO reset OK
  io_outpw(spi_base + 0x10, 0x0c); // Set_Spi_Clock_Divided
  // disable_cs(spi_base); // Disable_Spi_CS1
  io_outpb(spi_base + 0x1A, 0x00); // disable Auto-fetch: very important!
}

static void set_flash_unwritable()
{
  sb_Write8(0xc4, reg_sb_c4);
  //io_RestoreINT();
}


SPIFlash::SPIFlash(unsigned int in_index, unsigned int start_addr)
{
  unsigned int _4K_Strat_Section = 0;
  
  if (spi_base == 0)
  {
    spi_base = nb_Read(0x40) & 0x0000fff0;
  }
  
  if((flash_size == 0) || (EEPROM_offset == 0)) //init variables
  {
	set_flash_writable();
    unsigned char id[3];
	read_flash_device_id(id);
    set_flash_unwritable();
    const flash_info *pfi = get_flash_info(id);
    flash_size = pfi->size;
    #if defined (__86DUINO_DUO)
        int offset = in_index / 4;
        if (offset < 32) { // 32 block = 16 block + 16 block
            EEPROM_offset = 7864320; // 7.5 * 1024 * 1024;
        } else if (offset < 63) { // 32 + 31 = 63 block
            EEPROM_offset = 16252928; // 15.5 * 1024 * 1024;
			_4K_Strat_Section = 128; // 32 * 4 = 128; start from 128 of 4K sections
        } else if (offset < 95) { // 32 + 31 + 32 = 95 block
            EEPROM_offset = 24641536; // 23.5 * 1024 * 1024;
			_4K_Strat_Section = 252; // 63 * 4 = 252; start from 252 of 4K sections
		}
    #elif defined (__86DUINO_QEC) || defined (__86DUINO_QEC_M2) || defined (__86DUINO_QEC_7) || defined (__86DUINO_QEC_9) || defined (__86DUINO_QEC_15) || defined (__86DUINO_QEC_M02) || defined (__86DUINO_QEC_PPC104) || defined (__86DUINO_QEC_PPC9) || defined (__86DUINO_PPC9S) || defined (__86DUINO_PPC15S)
        EEPROM_offset = (16 * 1024 * 1024) - (512L * 1024); // for 32 blocks
    #else
        EEPROM_offset = flash_size - (512L * 1024);
    #endif
  }
  
  if (start_addr == 0)
    _sector_offset = EEPROM_offset + ((in_index - _4K_Strat_Section) * SECTORSIZE);
  else
    _sector_offset = start_addr;
}

SPIFlash::~SPIFlash()
{
}

unsigned long SPIFlash::sectorBias()
{
    return f_info->sectorbias;
}

void SPIFlash::write(unsigned short int in_addr, unsigned char in_value)
{
  if(in_addr >= SECTORSIZE)
    return;
	
  set_flash_writable();
  
  //mxic write enable
  enable_cs(spi_base);
  exchange_spi_byte(spi_base, 6);
  disable_cs(spi_base);
  
  //write data
  enable_cs(spi_base);
  exchange_spi_byte(spi_base, 0x02); //PAGE PROGRAM
  write_spi_24bit_addr(spi_base, _sector_offset + in_addr);//address
  exchange_spi_byte(spi_base, in_value);
  disable_cs(spi_base);
  
  //wait wip
  unsigned char s;
  int wip_cnt = 0;
  enable_cs(spi_base);
  exchange_spi_byte(spi_base, 5); //RDSR
  while(1)
  {
    s = exchange_spi_byte(spi_base, 5);
	if((s&1) == 0) //WIP == 0
	{
	  wip_cnt++;
	  if(wip_cnt >= 3)
      {
	    break;
      }
	}
	else
	{
	  wip_cnt = 0;
	}
  }
  disable_cs(spi_base);
  
  //mxic write disable
  enable_cs(spi_base);
  exchange_spi_byte(spi_base, 4);
  disable_cs(spi_base);
  
  set_flash_unwritable();
}

unsigned char SPIFlash::read(unsigned short int in_addr)
{
  unsigned char value = 0;
  if(in_addr >= SECTORSIZE)
    return 0;
  set_flash_writable();
  enable_cs(spi_base);
  exchange_spi_byte(spi_base, 0x03); //READ
  write_spi_24bit_addr(spi_base, _sector_offset + in_addr); //address
  value = exchange_spi_byte(spi_base, value);
  disable_cs(spi_base);
  set_flash_unwritable();
  return value;
}

//reset 4k sector to ff
void SPIFlash::reset()
{
  set_flash_writable();
  
  //mxic write enable
  enable_cs(spi_base);
  exchange_spi_byte(spi_base, 6);
  disable_cs(spi_base);
  
  //reset command and address
  enable_cs(spi_base);
  exchange_spi_byte(spi_base, 0x20);
  write_spi_24bit_addr(spi_base, _sector_offset);
  disable_cs(spi_base);
  
  //wait wip
  unsigned char s;
  int wip_cnt = 0;
  enable_cs(spi_base);
  exchange_spi_byte(spi_base, 5); //RDSR
  while(1)
  {
    s = exchange_spi_byte(spi_base, 5);
	if((s&1) == 0) //WIP == 0
	{
	  wip_cnt ++;
	  if(wip_cnt >= 3)
	    break;
	}
	else
	{
	  wip_cnt = 0;
	}
  }
  disable_cs(spi_base);
  
  //mxic write disable
  enable_cs(spi_base);
  exchange_spi_byte(spi_base, 4);
  disable_cs(spi_base);
  
  
  set_flash_unwritable();
}
