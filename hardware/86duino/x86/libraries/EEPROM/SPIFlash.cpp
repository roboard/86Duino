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


static void write_spi_byte(unsigned short iobase, unsigned char n)
{
  io_outpb(iobase, n);
  //waiting for ODC bit set
  while((io_inpb(iobase + 3) & 0x10) == 0);
}

static unsigned char read_spi_byte(unsigned short iobase)
{
  io_outpb(iobase + 1, 0);
  //waiting for IDR bit set
  while((io_inpb(iobase + 3) & 0x20) == 0);
  return io_inpb(iobase + 1);
}

static void enable_cs(unsigned short iobase)
{
  io_outpb(iobase + 4, 0);
}

static void disable_cs(unsigned short iobase)
{
  io_outpb(iobase + 4, 1);
}

void write_spi_24bit_addr(unsigned short iobase, unsigned long addr)
{
  write_spi_byte(iobase, (unsigned char)((addr >> 16) & 0xff));
  write_spi_byte(iobase, (unsigned char)((addr >> 8) & 0xff));
  write_spi_byte(iobase, (unsigned char)((addr) & 0xff));
}

static unsigned char reg_sb_c4 = 0;
static unsigned long reg_nb_40 = 0;
static unsigned char spi_ctrl_reg = 0;
static unsigned long spi_base = 0;
static unsigned long flash_size = 0;
static unsigned long EEPROM_offset = 0;

struct flash_info
{
  unsigned char id[3];
  const char *name;
  unsigned long size;
};

static struct flash_info* f_info;

static const struct flash_info flash_info_table[] =
{
  {{0xc2, 0x20, 0x12}, "MX25L2005", 256L * 1024},
  {{0xc2, 0x20, 0x15}, "MX25L1605", 2L * 1024 * 1024},
  {{0xc2, 0x25, 0x37}, "MX25U6435F", 8L * 1024 * 1024},
  {{0xc2, 0x25, 0x38}, "MX25U12835F", 16L * 1024 * 1024},
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
  write_spi_byte(spi_base, 0x9f);// RDID command
  //disable_cs(spi_base);
  
  //read device ID
  //enable_cs(spi_base);
  p[0] = read_spi_byte(spi_base);
  p[1] = read_spi_byte(spi_base);
  p[2] = read_spi_byte(spi_base);
  disable_cs(spi_base);
}

static void set_flash_writable()
{
  io_DisableINT();

  reg_sb_c4 = sb_Read8(0xc4);  
  reg_nb_40 = nb_Read(0x40);
  spi_base = reg_nb_40 & 0x0000fff0;
  spi_ctrl_reg = io_inpb(spi_base + 2);
  
  //set sb register 0xC4
  sb_Write8(0xc4, reg_sb_c4 | 1);
  
  //set nb register 0x40
  nb_Write(0x40, reg_nb_40 | 1);
  
  //set_spi_ckdiv and spi_afdis_bit 0x0c for EX
  io_outpdw(spi_base + 2, ((spi_ctrl_reg | 0x20) & 0xf0) | 0x0c);
}

static void set_flash_unwritable()
{
  io_outpdw(spi_base + 2, spi_ctrl_reg);
  nb_Write(0x40, reg_nb_40);
  sb_Write8(0xc4, reg_sb_c4);
  io_RestoreINT();
}


SPIFlash::SPIFlash(unsigned int in_index)
{
  if((flash_size == 0) || (EEPROM_offset == 0)) //init variables
  {
	set_flash_writable();
	unsigned char id[3];
	read_flash_device_id(id);
	set_flash_unwritable();
	
	const flash_info *pfi = get_flash_info(id);
	flash_size = pfi->size;
	EEPROM_offset = flash_size - (512L * 1024); 
  }
  
  _sector_offset = EEPROM_offset + (in_index * SECTORSIZE);
}

SPIFlash::~SPIFlash()
{
}

void SPIFlash::write(unsigned short int in_addr, unsigned char in_value)
{
  if(in_addr >= SECTORSIZE)
    return;
	
  set_flash_writable();
  
  //mxic write enable
  enable_cs(spi_base);
  write_spi_byte(spi_base, 6);
  disable_cs(spi_base);
  
  //write data
  enable_cs(spi_base);
  write_spi_byte(spi_base, 0x02); //PAGE PROGRAM
  write_spi_24bit_addr(spi_base, _sector_offset + in_addr);//address
  write_spi_byte(spi_base, in_value);
  disable_cs(spi_base);
  
  //wait wip
  unsigned char s;
  int wip_cnt = 0;
  enable_cs(spi_base);
  write_spi_byte(spi_base, 5); //RDSR
  while(1)
  {
    s = read_spi_byte(spi_base);
	if((s&1) == 0) //WIP == 0
	{
	  wip_cnt++;
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
  write_spi_byte(spi_base, 4);
  disable_cs(spi_base);
  
  set_flash_unwritable();
}

unsigned char SPIFlash::read(unsigned short int in_addr)
{
  unsigned char value;
  if(in_addr >= SECTORSIZE)
    return 0;
  set_flash_writable();
  enable_cs(spi_base);
  write_spi_byte(spi_base, 0x03); //READ
  write_spi_24bit_addr(spi_base, _sector_offset + in_addr); //address
  value = read_spi_byte(spi_base);
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
  write_spi_byte(spi_base, 6);
  disable_cs(spi_base);
  
  //reset command and address
  enable_cs(spi_base);
  write_spi_byte(spi_base, 0x20);
  write_spi_24bit_addr(spi_base, _sector_offset);
  disable_cs(spi_base);
  
  //wait wip
  unsigned char s;
  int wip_cnt = 0;
  enable_cs(spi_base);
  write_spi_byte(spi_base, 5); //RDSR
  while(1)
  {
    s = read_spi_byte(spi_base);
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
  write_spi_byte(spi_base, 4);
  disable_cs(spi_base);
  
  
  set_flash_unwritable();
}


