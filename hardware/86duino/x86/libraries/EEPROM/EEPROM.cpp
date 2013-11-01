/*
  EEPROM.cpp - EEPROM library for 86Duino
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

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "Arduino.h"
#include "EEPROM.h"

#include <unistd.h>
#include <pc.h>
#include <io.h>
/******************************************************************************
 * Definitions
 ******************************************************************************/
 
 // PCI device.
typedef struct
{
  // PCI device address.
  int bus;
  int dev;
  int func;
  
} pci_dev;

static FILE* create_newfile();
static uint32_t get_size(FILE *fp);

/******************************************************************************
 * Constructors
 ******************************************************************************/
EEPROMClass::EEPROMClass(void)
{
	bank = false;//default use bank 0
	_file = NULL;
	E2END = 1024;//default size for bank 1
}

EEPROMClass::~EEPROMClass(void)
{
	if(_file != NULL)
		fclose(_file);
	bank = false;
}
 
/******************************************************************************
 * User API
 ******************************************************************************/
FILE* EEPROMClass::create_newfile() {
	if(!bank) //only bank1 needs create new file.
		return NULL;
		
	FILE *fp;
	int i;
	
	//If there's a eeprom file already.
	if((fp = fopen("_eeprom.bin", "rb+")) != NULL)
	{
		E2END = get_size(fp);
		return fp;
	}	
	
	//If not, create a new one.
	if((fp = fopen("_eeprom.bin", "wb+")) == NULL) 
		return NULL;
	
	char ch = '\0';
	for(i = 0; i < E2END; i++) //set default value
	{
		if(fwrite(&ch, sizeof(char), 1, fp) != 1)
		{
			fclose(fp);
			return NULL;
		}
		fflush(fp);
		fsync(fileno(fp));
	}
	return fp;
}

uint32_t EEPROMClass::get_size(FILE *fp) 
{

  if (!bank) //bank 0  only support 200 bytes
	return 200;
	
  if (fp == NULL) 
	return 0;
	
  long pos = ftell(fp);
  long len = 0;
  fseek(fp, 0L, SEEK_END);
  len = ftell(fp);
  fseek(fp, pos, SEEK_SET);
  return len;
}

static unsigned long pci_dev_read_dw(pci_dev *pdev, unsigned int index)
{
  unsigned long tmp;
  unsigned long pci_config_addr32 = 0x80000000L |
                       (((unsigned long)pdev->bus) << 16) |
                       (((unsigned long)pdev->dev) << 11) |
                       (((unsigned long)pdev->func) << 8) |
                       index;
  outportl(0x0cf8, pci_config_addr32);				   
  tmp = inportl(0x0cfc);
  
  return tmp;
}

static void pci_dev_write_dw(pci_dev *pdev, unsigned int index, unsigned long int value)
{
  unsigned long pci_config_addr32 = 0x80000000L |
                       (((unsigned long)pdev->bus) << 16) |
                       (((unsigned long)pdev->dev) << 11) |
                       (((unsigned long)pdev->func) << 8) |
                       index; 
  outportl(0xcf8, pci_config_addr32);
  outportl(0xcfc, value);
}

static unsigned char read_cmos(unsigned char address)
{
  if(address >= 200) // 0~199
	return 0;
  pci_dev pcidev;
  // south bridge register C0H bit 3 controls CMOS page select
  unsigned long int reg;
  unsigned char result;
  
  io_DisableINT();
  
  pcidev.bus = 0;
  pcidev.dev = 7;
  pcidev.func = 0;

  reg = pci_dev_read_dw(&pcidev, 0xc0);
  
  if(address < 100) // 0~99 low page 
  {
	//clear bit 3 to access low 128 bytes RTC SRAM
	pci_dev_write_dw(&pcidev, 0xc0, reg & 0xfffffff7);
  }
  else// 100~199 high page
  {
	//Set bit 3 to access high 128 bytes RTC SRAM
	pci_dev_write_dw(&pcidev, 0xc0, reg | 0x00000008);
	address -= 100;
  }
  outp(0x70, address + 28);
  result = inp(0x71);
  
  // Restore old register value
  pci_dev_write_dw(&pcidev, 0xc0, reg);
  
  io_RestoreINT();
  return result;
}

static void write_cmos(unsigned char address, unsigned char buf)
{
  if(address >= 200) // 0~199
	return ;

  pci_dev pcidev;
  unsigned char reg;
  io_DisableINT();
  
  pcidev.bus = 0;
  pcidev.dev = 7;
  pcidev.func = 0;
  
  reg = pci_dev_read_dw(&pcidev, 0xc0);
  
  if(address < 100) // 0~99 low page 
  {
	//clear bit 3 to access low 128 bytes RTC SRAM
	pci_dev_write_dw(&pcidev, 0xc0, reg & 0xfffffff7);
  }
  else// 100~199 high page
  {
	//Set bit 3 to access high 128 bytes RTC SRAM
	pci_dev_write_dw(&pcidev, 0xc0, reg | 0x00000008);
	address -= 100;
  }
  outp(0x70, address + 28);
  outp(0x71, buf);

  // Restore old register value
  pci_dev_write_dw(&pcidev, 0xc0, reg);
  io_RestoreINT();
}

uint8_t EEPROMClass::eeprom_read_byte(FILE *fp, int addr) {
	uint8_t ch = 0;
	
	//if bank 0 read from cmos
	if(!bank)
	{
		ch = read_cmos(addr);
		return ch;
	}
	
	if(fp == NULL)
		return 0;
	fseek(fp, addr, SEEK_SET);
	fread(&ch, sizeof(char), 1, fp);
	return ch;
}

void EEPROMClass::eeprom_write_byte(FILE *fp, int addr, uint8_t val) 
{
	//if bank 0 write to cmos
	if(!bank)
	{
		write_cmos(addr, val);
		return;
	}
	
	if(fp == NULL)
		return ;
	fseek(fp, addr, SEEK_SET);
	fwrite(&val, sizeof(char), 1, fp);
	
}

uint8_t EEPROMClass::read(int address)
{
	//bank 0 only support 200 bytes (0~199)
	if(!bank && address >= 200)
		return 0;
	else if(!bank)
		return eeprom_read_byte(_file, address);
		
	
	if(bank && address >= E2END) 
		return 0;
	
	if(_file == NULL)
	{
		//if _file == NULL create_newfile first
		_file = create_newfile();
		if(_file == NULL)
			return 0;
	}
	
	return eeprom_read_byte(_file, address);
}

void EEPROMClass::write(int address, uint8_t value)
{
	if(!bank && address >= 200)
		return;
	else if(!bank)
		eeprom_write_byte(_file, address, value);
		
	if(bank && address >= E2END) 
		return;
	
	if(_file == NULL)
	{
		_file = create_newfile();
		if(_file == NULL)
			return;
	}
	eeprom_write_byte(_file, address, value);
}

void EEPROMClass::set_bank(bool input)
{
	bank = false;
	return;
	bank = input;
	if(!bank && _file != NULL)
		fclose(_file);
}

void EEPROMClass::EEPROMflush()
{
	if(!bank || _file == NULL)
		return;
		
	fflush(_file);	
	fsync(fileno(_file));
}

EEPROMClass EEPROM;
