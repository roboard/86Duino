/*
  EEPROM.cpp - EEPROM library
  Copyright (c) 2006 David A. Mellis.  All right reserved.

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
*/

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "Arduino.h"
#include "EEPROM.h"

/******************************************************************************
 * Definitions
 ******************************************************************************/

/******************************************************************************
 * Constructors
 ******************************************************************************/

/******************************************************************************
 * User API
 ******************************************************************************/
// assume eeprom.txt exist in the disk
#define DATA_SIZE    (8)
#define E2END        (1024)
FILE* create_newfile(void) {
	FILE* fp;
	int i;
	if((fp = fopen("eeprom.bin", "wb")) == NULL) 
		return NULL;
	
	char ch = '\0';
	if(fwrite(&ch, 1, E2END, fp) != E2END)
	{
		fclose(fp);
		return NULL;
	}
	return fp;
}

static uint32_t get_size(FILE *_file) {
  if (_file == NULL) 
	return 0;
  
  long pos = ftell(_file);
  long len = 0;
  fseek(_file, 0L, SEEK_END);
  len = ftell(_file);
  fseek(_file, pos, SEEK_SET);
  return len;
}

static uint8_t eeprom_read_byte(int addr) {
    FILE* fp;
	uint8_t ch = 0;
	int size;

	if(addr >= E2END) return 0;
	fp = fopen("eeprom.bin", "rb");
	if(fp == NULL)
		if((fp = create_newfile()) == NULL) return 0;
	
	size = get_size(fp);
	if(addr > size)
		return 0;
	
	fseek(fp, addr, SEEK_SET);
	fread(&ch, sizeof(char), 1, fp);
	fclose(fp);
	return ch;
}

static void eeprom_write_byte(int addr, uint8_t val) {
    FILE* fp;
	int size;

	if(addr >= E2END) return;
	fp = fopen("eeprom.bin", "rb+");
	if(fp == NULL)
		if((fp = create_newfile()) == NULL) return;
	
	size = get_size(fp);
	if(addr > size)
		return;
	
	fseek(fp, addr, SEEK_SET);
	fwrite(&val, sizeof(char), 1, fp);
	fclose(fp);
}

uint8_t EEPROMClass::read(int address)
{
	return eeprom_read_byte(address);
}

void EEPROMClass::write(int address, uint8_t value)
{
	eeprom_write_byte(address, value);
}

EEPROMClass EEPROM;
