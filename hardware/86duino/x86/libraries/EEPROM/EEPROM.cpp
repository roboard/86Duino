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
#include "avr/eeprom.h"

#include "EEPROM.h"
#include <stdio.h>
#include <stdlib.h>

/******************************************************************************
 * Definitions
 ******************************************************************************/
 
#define ADDRESS 16
#define DATASIZE 1024
#define EEPROMSIZE (ADDRESS * DATASIZE)
#define EEPROMSIZE_BANK0 200

//PCI device
/*
typedef struct
{
  //PCI device address.
  int bus;
  int dev;
  int func;
} pci_dev;
*/ 
/******************************************************************************
 * Constructors
 ******************************************************************************/
EEPROMClass::EEPROMClass(void)
{
  io_Init();
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
 
/**** remove read_cmos() and read_cmos() and put them into eeprom.cpp in cores/avr directory **/
/*
 static unsigned char read_cmos(unsigned char address)
{
  if(address >= EEPROMSIZE_BANK0) // 0~199
	return 0;
  io_DisableINT();
  void *pciDev = NULL;
  // south bridge register C0H bit 3 controls CMOS page select
  unsigned long int reg;
  unsigned char result;
  
  pciDev = pci_Alloc(0x00, 0x07, 0x00);
  if(pciDev == NULL)
  {
#if (defined(DMP_DOS_BC) || defined(DMP_DOS_DJGPP) || defined(DMP_DOS_WATCOM))
    Serial.print("CMOS device doesn't exist\n");
#elif (defined(DMP_LINUX))
    printf("CMOS device doesn't exist\n");
#endif
    return 0;
  }
  
  reg = pci_In32(pciDev, 0xc0);

  if(address == 20 || address == 22)//special case
  {
  	//Set bit 3 to access high 128 bytes RTC SRAM
	pci_Out32(pciDev, 0xc0, reg | 0x00000008);
	address = (address == 20)? 26:27;
	io_outpb(0x70, address);
  }
  else if(address < 100) // 0~99 low page 
  {
	//clear bit 3 to access low 128 bytes RTC SRAM
	pci_Out32(pciDev, 0xc0, reg & 0xfffffff7);
	io_outpb(0x70, address + 28);
  }
  else// 100~199 high page
  {
	//Set bit 3 to access high 128 bytes RTC SRAM
	pci_Out32(pciDev, 0xc0, reg | 0x00000008);
	address -= 100;
	io_outpb(0x70, address + 28);
  }
  result = io_inpb(0x71);
  
  // Restore old register value
  pci_Out32(pciDev, 0xc0, reg);
  
  pci_Free(pciDev);

  io_RestoreINT();
  return result;
}

static void write_cmos(unsigned char address, unsigned char buf)
{
  if(address >= EEPROMSIZE_BANK0) // 0~199
	return ;
  io_DisableINT();
  void *pciDev = NULL;
  pciDev = pci_Alloc(0x00, 0x07, 0x00);
  if(pciDev == NULL)
  {
#if (defined(DMP_DOS_BC) || defined(DMP_DOS_DJGPP) || defined(DMP_DOS_WATCOM))
    Serial.print("CMOS device doesn't exist\n");
#elif (defined(DMP_LINUX))
    printf("CMOS device doesn't exist\n");
#endif
    return;
  }

  unsigned long int reg;
  reg = pci_In32(pciDev, 0xc0);
  if(address == 20 || address == 22) //special case
  {
    //Set bit 3 to access high 128 bytes RTC SRAM
	pci_Out32(pciDev, 0xc0, reg | 0x00000008);
        address = (address == 20)? 26:27;
        io_outpb(0x70, address);
  }
  else if(address < 100) // 0~99 low page 
  {
	//clear bit 3 to access low 128 bytes RTC SRAM
	pci_Out32(pciDev, 0xc0, reg & 0xfffffff7);
	io_outpb(0x70, address + 28);
  }
  else// 100~199 high page
  {
	//Set bit 3 to access high 128 bytes RTC SRAM
	pci_Out32(pciDev, 0xc0, reg | 0x00000008);
	address -= 100;
	io_outpb(0x70, address + 28);
  }
  io_outpb(0x71, buf);
  // Restore old register value
  pci_Out32(pciDev, 0xc0, reg);
  pci_Free(pciDev);
  io_RestoreINT();

}
*/

unsigned char EEPROMClass::read(unsigned short int in_addr)
{
  if(((in_addr >= EEPROMSIZE) && (_bank == EEPROM_16K)) || ((in_addr >= EEPROMSIZE_BANK0) && (_bank == EEPROM_200B)))//bank == true 0~16384, false 0~199
  {
#if (defined(DMP_DOS_BC) || defined(DMP_DOS_DJGPP) || defined(DMP_DOS_WATCOM))
    Serial.print("EEPROMClass read error(bank = ");
	Serial.print(_bank);
	Serial.print("): in_addr = ");
	Serial.print(in_addr);
	Serial.print(" >= ");
	Serial.println(EEPROMSIZE);
#elif (defined(DMP_LINUX))
	printf("EEPROMClass read error (bank = %d): in_addr = %x >= %x\n", _bank, in_addr, EEPROMSIZE);
#endif
	return 0;
  }
  
  if(_bank == EEPROM_200B)
    return eeprom_read_byte((uint8_t*)in_addr);
  else
	return (_data[in_addr / DATASIZE])->read(in_addr - (DATASIZE * (in_addr / DATASIZE)));
}

void EEPROMClass::write(unsigned short int in_addr, unsigned char in_data)
{
  if(((in_addr >= EEPROMSIZE) && (_bank == EEPROM_16K)) || ((in_addr >= EEPROMSIZE_BANK0) && (_bank == EEPROM_200B)))//bank == true 0~16384, false 0~199
  {
#if (defined(DMP_DOS_BC) || defined(DMP_DOS_DJGPP) || defined(DMP_DOS_WATCOM))
    	Serial.print("EEPROMClass write error(bank = ");
	Serial.print(_bank);
	Serial.print("): in_addr = ");
	Serial.print(in_addr);
	Serial.print(" >= ");
	Serial.println(EEPROMSIZE);
#elif (defined(DMP_LINUX))
	printf("EEPROMClass write error(bank = %d): in_addr = %x >= %x\n", _bank, in_addr, EEPROMSIZE);
#endif
	return;
  }
  if(_bank == EEPROM_200B)
    eeprom_write_byte((uint8_t*)in_addr, in_data);
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

