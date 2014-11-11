/*
  Block.cpp - Part of DM&P Vortex86 EEPROM library
  Copyright (c) 2013 Vic Cheng <vic@dmp.com.tw>. All right reserved.

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

#include "Block.h"
#include "Arduino.h"

#include <string.h>
#include <pc.h>
#include <stdio.h>
#include <stdlib.h>

#define ADDRESS 16
#define SECTORSIZE 4096
#define DATASIZE 1024
#define EEPROMSIZE (ADDRESS * DATASIZE)
#define BYTESIZE 8
#define EEPROM_OFFSET_CMOS 145
#define CMOSSIZE (ADDRESS/BYTESIZE)

//PCI device
typedef struct
{
  //PCI device address.
  int bus;
  int dev;
  int func;
} pci_dev;

static unsigned long pci_dev_read_dw(pci_dev *pdev, unsigned int index)
{
  unsigned long tmp;
  unsigned long pci_config_addr32 = 0x80000000L |
                       (((unsigned long)pdev->bus) << 16) |
					   (((unsigned long)pdev->dev) << 11) |
					   (((unsigned long)pdev->func)<<  8) |
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
					   (((unsigned long)pdev->func)<<  8) |
					   index;
  outportl(0xcf8, pci_config_addr32);
  outportl(0xcfc, value);
}


static unsigned char read_cmos(unsigned char address)
{
  //every 1K EEPROM space needs 1bit CMOS to control.
  if(address < EEPROM_OFFSET_CMOS || address >= (EEPROM_OFFSET_CMOS + CMOSSIZE))
    return 0;
  pci_dev pcidev;
  
  unsigned long int reg;
  unsigned char result;
 
  io_DisableINT();
  
  pcidev.bus = 0;
  pcidev.dev = 7;
  pcidev.func = 0;
  
  reg = pci_dev_read_dw(&pcidev, 0xc0);
  pci_dev_write_dw(&pcidev, 0xc0, reg | 0x00000008);
  
  
  outp(0x70, address - 128);
  result = inp(0x71);
  
  pci_dev_write_dw(&pcidev, 0xc0, reg);
  
  io_RestoreINT();
  return result;
  
}

static void write_cmos(unsigned char address, unsigned char buf)
{
  if(address < EEPROM_OFFSET_CMOS || address >= (EEPROM_OFFSET_CMOS + CMOSSIZE))
    return;
  pci_dev pcidev;
  unsigned char reg;
  io_DisableINT();
  
  pcidev.bus = 0;
  pcidev.dev = 7;
  pcidev.func = 0;
  
  reg = pci_dev_read_dw(&pcidev, 0xc0);
  pci_dev_write_dw(&pcidev, 0xc0, reg | 0x00000008);
  
  outp(0x70, address - 128);
  outp(0x71, buf);
  
  pci_dev_write_dw(&pcidev, 0xc0, reg);
  io_RestoreINT();
}


EEPROMBlock::EEPROMBlock(unsigned short in_addr)
:_data(NULL), _data_redundancy(NULL), _counter(NULL),
 _data_buffer(NULL),
 _data_position(0xffff), _counter_position(0xffff),
 _redundancy(false), _addr(0x0000)
{
  if(in_addr >= ADDRESS)
  {
	return;
  }
  _addr = in_addr;
  
  //create three SPIFlash spaces.
  _data = new SPIFlash(_addr * 3 + 0);
  _data_redundancy = new SPIFlash(_addr * 3 + 1);
  _counter = new SPIFlash(_addr * 3 + 2);

  _data_buffer = new unsigned char[SECTORSIZE];
  if(_data_buffer == NULL)
  {
	return;
  }
  
  CheckCounterResetState();
  SetRedundancyFlag(); //set _redundancy flag

  //load data
  SPIFlash *temp = (_redundancy)?_data_redundancy : _data;
  
  for(int i = 0; i < SECTORSIZE; ++i)
  {
    _data_buffer[i] = temp->read(i); //get data
  }
  CalculateRealData();
}

EEPROMBlock::~EEPROMBlock()
{
  delete[] _data_buffer;
  delete _data;
  delete _data_redundancy;
  delete _counter;
}

void EEPROMBlock::write(unsigned short int in_addr, unsigned char in_data)
{ 
  if(in_addr >= DATASIZE)
  {
    Serial.print("EEPROMBlock write failed: in_addr = ");
	Serial.print(in_addr);
    Serial.print(" >= ");
	Serial.println(DATASIZE);
	return;
  }
  SPIFlash *temp;
  
  //1. check if _data_buffer is full or not
  if(_data_position >= SECTORSIZE) //full
  {
    temp = (_redundancy)?_data:_data_redundancy; 
    //a. reset another data space
	temp->reset(); 
	//b. write current real data 
	for(int i = 0; i < DATASIZE; ++i)
	{
	  temp->write(i, _data_buffer[i]);
	}
	//c. modify counter
	WriteCounter();
	//d. cleaer position
	_data_position = DATASIZE;
  }
  //2. modify _data_buffer
  _data_buffer[_data_position] = (in_addr >> 8) & 0x03; //get first two bits
  _data_buffer[_data_position + 1] = (unsigned char)(in_addr & 0x00ff); //get last byte
  _data_buffer[_data_position + 2] = in_data;
  
  _data_buffer[in_addr] = in_data;
  
  //3. write to SPIFlash
  //a. set temp to writable space
  temp = (_redundancy)?_data_redundancy:_data;
  //b. write data
  
  temp->write(_data_position + 2, _data_buffer[_data_position + 2]);
  temp->write(_data_position + 1, _data_buffer[_data_position + 1]);
  temp->write(_data_position, _data_buffer[_data_position]);
  
  _data_position += 3;
}

unsigned char EEPROMBlock::read(unsigned short int in_addr)
{
  
  if(in_addr >= DATASIZE)
  {
    Serial.print("EEPROMBlock read failed: in_addr = ");
	Serial.print(in_addr);
    Serial.print(" >= ");
	Serial.println(DATASIZE);
	return 0;
  }
  return _data_buffer[in_addr];
}

void EEPROMBlock::CalculateRealData()
{
  for(int i = DATASIZE; i < SECTORSIZE; i+=3) //0~1023 = data 1024~4095 = modify
  {
    //check valid bit
	if(_data_buffer[i] & 0x80) //valid bit not cleared
	{
	  if((_data_buffer[i] != 0xff) || (_data_buffer[i+1] != 0xff) || (_data_buffer[i+2] != 0xff)) //data was modified already but valid bit not cleared
	    continue; //abandon
	  else 
	  {
	    _data_position = i;//all data behind this byte should be 0xff
	    return; //claculate OK
	  }
	}
	
	//valid bit was set
	unsigned short addr = (((unsigned short)(_data_buffer[i] & 0x03)) << 8) | _data_buffer[i+1];//10 bits for 0~1023

	if(addr >= DATASIZE)
	  continue;
	_data_position = i;
	
	_data_buffer[addr] = _data_buffer[i+2];
  }
  _data_position = SECTORSIZE;
}

//make sure counter is in a right status.
void EEPROMBlock::CheckCounterResetState()
{
  //read CMOS to check need reset or not
  unsigned char data;
  
  data = read_cmos(EEPROM_OFFSET_CMOS + _addr / BYTESIZE);
  if((data >> (_addr % BYTESIZE)) & 0x01) //check bit is 1 or 0, if 1 reset counter
  {
    _counter->reset();
	data = data & (0xff ^ (0x01 << (_addr % BYTESIZE))); //cleaer bit to zero
	write_cmos(EEPROM_OFFSET_CMOS + _addr / BYTESIZE, data);
  }
}

//check counter to decide using _data or _data_redundancy
void EEPROMBlock::SetRedundancyFlag()
{
  unsigned char data;
  for(int i = 0; i < SECTORSIZE; ++i)
  {
    data = _counter->read(i);
	if(data == 0xff) //not cleared yet
	{
	  //all data behind this byte should be 0xff
	  _counter_position = i;
	  _redundancy = false;
	  break;
	}
	else if( data == 0x00) //cleared
	{
	  continue;
	}
	else
	{
	  _counter_position = i; //all data behind this byte should be 0xff
	  unsigned char number_of_ones = 0;
	  
	  for(int j = 0; j < BYTESIZE; ++j)
	  {
	    if((data >> j) & 0x01)
		{
		  number_of_ones++;
		}
	  }
	  //Set flag
	  _redundancy = number_of_ones % 2; //1 = use redundancy
	  break;
	}
  }
}


void EEPROMBlock::WriteCounter()
{
  //1. check if counter is full or not
  if(_counter_position >= SECTORSIZE) //full
  {
    //a.write cmos to 1
	unsigned char data;
	data = read_cmos(EEPROM_OFFSET_CMOS + _addr / BYTESIZE);
	data = data | (0x01 << (_addr % BYTESIZE));//set bit to 1.
	write_cmos(EEPROM_OFFSET_CMOS + _addr / BYTESIZE, data);
	//b. reset counter
	_counter->reset();
	_counter_position = 0;
	
	//c. write cmos to 0
	data = data & (0xff ^ (0x01 << (_addr % BYTESIZE)));//clear bit to 0
	write_cmos(EEPROM_OFFSET_CMOS + _addr / BYTESIZE, data);	
  }
  //2. write counter bit
  unsigned char data;
  data = _counter->read(_counter_position);
  //find leading one
  for(int i = BYTESIZE - 1; i >= 0; --i)
  {
    if((data >> i) & 0x01) // if not cleared
	{
	  data = data ^ (0x01 << i); //clear bit
	  _counter->write(_counter_position, data);
	  if(i == 0) //byte is all cleared
	    _counter_position++; //move to next byte
	  break;
	}
  }
  //3. set redundancy flag
  _redundancy = !_redundancy;
}

