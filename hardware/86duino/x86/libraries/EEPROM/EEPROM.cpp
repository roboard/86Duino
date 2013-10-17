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
#define E2END        (1000)
FILE* create_newfile(void) {
	FILE* fp;
	int i;
	char ch[DATA_SIZE+1];
	if((fp = fopen("eeprom.txt", "w")) == NULL) {printf("EEPROM init fail\n"); return NULL;}
	
	for(i=0; i<DATA_SIZE; i++) ch[i] = '0';
	ch[DATA_SIZE] = '\n';
	
	for(i=0; i<E2END; i++)
	{
		if(fseek(fp, i*(DATA_SIZE+2), SEEK_SET) != 0) {printf("fseek() fail\n"); fclose(fp); return NULL;}
		if(fwrite(ch, sizeof(char), DATA_SIZE+1, fp) != (DATA_SIZE+1)) {printf("fwrite() fail\n"); fclose(fp); return NULL;}
	}
	printf("EEPROM init ok\n");
	return fp;
}

static uint8_t eeprom_read_byte(int addr) {
    FILE* fp;
	uint8_t ch[DATA_SIZE+1], val = 0;
	int i;

	if(addr >= E2END) return 0;
	fp = fopen("eeprom.txt", "r");
	if(fp == NULL)
		if((fp = create_newfile()) == NULL) return 0;
	
	fseek(fp, addr*(DATA_SIZE+2), SEEK_SET);
	fread(ch, sizeof(char), DATA_SIZE+1, fp);
	for(i=0; i<DATA_SIZE; i++)
		val = (val<<1)+((ch[i] == '0') ? 0 : 1);
	fclose(fp);
	return val;
}

static void eeprom_write_byte(int addr, uint8_t val) {
    FILE* fp;
	uint8_t ch[DATA_SIZE+1];
	uint8_t* a;
	int i;

	if(addr >= E2END) return;
	fp = fopen("eeprom.txt", "r+");
	if(fp == NULL)
		if((fp = create_newfile()) == NULL) return;
	
	for(i=DATA_SIZE-1, a=ch; i>=0; i--, a++)
		*a = ((val & (0x01<<i)) == 0) ? '0' : '1';
	*a = '\n';
	fseek(fp, addr*(DATA_SIZE+2), SEEK_SET);
	fwrite(ch, sizeof(char), DATA_SIZE+1, fp);
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
