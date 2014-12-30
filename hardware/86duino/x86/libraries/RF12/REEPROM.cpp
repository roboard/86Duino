/*
  EEPROM.cpp - DM&P Vortex86 EEPROM library
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
#include "REEPROM.h"
#include <stdio.h>
#include <stdlib.h>
#include <pc.h>

/******************************************************************************
 * Definitions
 ******************************************************************************/
 
#define ADDRESS 16
#define DATASIZE 1024
#define EEPROMSIZE (ADDRESS * DATASIZE)
#define EEPROMSIZE_BANK0 200

//PCI device
typedef struct
{
  //PCI device address.
  int bus;
  int dev;
  int func;
} pci_dev;
 
/******************************************************************************
 * Constructors
 ******************************************************************************/
EEPROMClass::EEPROMClass(void)
{
  _bank = EEPROM_16K; //default use bank 0;
  
  _data = new EEPROMBlock*[ADDRESS];
  
  //init 16K EEPROM 

  for(int i = 0; i < ADDRESS; ++i)
  {
    _data[i] = new EEPROMBlock(i);
  }
  
}

EEPROMClass::~EEPROMClass(void)
{

  for(int i = 0; i < ADDRESS; ++i)
    delete _data[i];

  delete [](_data);
}
 
/******************************************************************************
 * User API
 ******************************************************************************/
 
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
  if(address >= EEPROMSIZE_BANK0) // 0~199
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
  if(address == 20 || address == 22)//special case
  {
  	//Set bit 3 to access high 128 bytes RTC SRAM
	pci_dev_write_dw(&pcidev, 0xc0, reg | 0x00000008);
	address = (address == 20)? 26:27;
	outp(0x70, address);
  }
  else if(address < 100) // 0~99 low page 
  {
	//clear bit 3 to access low 128 bytes RTC SRAM
	pci_dev_write_dw(&pcidev, 0xc0, reg & 0xfffffff7);
	outp(0x70, address + 28);
  }
  else// 100~199 high page
  {
	//Set bit 3 to access high 128 bytes RTC SRAM
	pci_dev_write_dw(&pcidev, 0xc0, reg | 0x00000008);
	address -= 100;
	outp(0x70, address + 28);
  }
  result = inp(0x71);
  
  // Restore old register value
  pci_dev_write_dw(&pcidev, 0xc0, reg);
  
  io_RestoreINT();
  return result;
}

static void write_cmos(unsigned char address, unsigned char buf)
{
  if(address >= EEPROMSIZE_BANK0) // 0~199
	return ;

  pci_dev pcidev;
  unsigned long int reg;
  io_DisableINT();
  
  pcidev.bus = 0;
  pcidev.dev = 7;
  pcidev.func = 0;
  
  reg = pci_dev_read_dw(&pcidev, 0xc0);
  if(address == 20 || address == 22) //special case
  {
    //Set bit 3 to access high 128 bytes RTC SRAM
	pci_dev_write_dw(&pcidev, 0xc0, reg | 0x00000008);
	address = (address == 20)? 26:27;
	outp(0x70, address);
  }
  else if(address < 100) // 0~99 low page 
  {
	//clear bit 3 to access low 128 bytes RTC SRAM
	pci_dev_write_dw(&pcidev, 0xc0, reg & 0xfffffff7);
	outp(0x70, address + 28);
  }
  else// 100~199 high page
  {
	//Set bit 3 to access high 128 bytes RTC SRAM
	pci_dev_write_dw(&pcidev, 0xc0, reg | 0x00000008);
	address -= 100;
	outp(0x70, address + 28);
  }
  
  outp(0x71, buf);

  // Restore old register value
  pci_dev_write_dw(&pcidev, 0xc0, reg);
  io_RestoreINT();
}
 
unsigned char EEPROMClass::read(unsigned short int in_addr)
{
  if(((in_addr >= EEPROMSIZE) && (_bank == EEPROM_16K)) || ((in_addr >= EEPROMSIZE_BANK0) && (_bank == EEPROM_200B)))//bank == true 0~16384, false 0~199
  {
    Serial.print("EEPROMClass read error(bank = ");
	Serial.print(_bank);
	Serial.print("): in_addr = ");
	Serial.print(in_addr);
	Serial.print(" >= ");
	Serial.println(EEPROMSIZE);
	return 0;
  }
  
  if(_bank == EEPROM_200B)
    return read_cmos(in_addr);
  else
	return (_data[in_addr / DATASIZE])->read(in_addr - (DATASIZE * (in_addr / DATASIZE)));
}

void EEPROMClass::write(unsigned short int in_addr, unsigned char in_data)
{
  if(((in_addr >= EEPROMSIZE) && (_bank == EEPROM_16K)) || ((in_addr >= EEPROMSIZE_BANK0) && (_bank == EEPROM_200B)))//bank == true 0~16384, false 0~199
  {
    Serial.print("EEPROMClass write error(bank = ");
	Serial.print(_bank);
	Serial.print("): in_addr = ");
	Serial.print(in_addr);
	Serial.print(" >= ");
	Serial.println(EEPROMSIZE);
	return;
  }
  if(_bank == EEPROM_200B)
    write_cmos(in_addr, in_data);
  else
    (_data[in_addr / DATASIZE])->write(in_addr - (DATASIZE * (in_addr / DATASIZE)) , in_data);
}

void EEPROMClass::set_bank(bool input)
{
  if(input == false)
  	_bank = EEPROM_200B;
  else
  	_bank = EEPROM_16K;
}

void EEPROMClass::setBank(int input)
{
  if(input != EEPROM_200B && input != EEPROM_16K)
  {
  	_bank = EEPROM_16K;
  	return;
  }	
  
  _bank = input;
}

EEPROMClass EEPROM;

