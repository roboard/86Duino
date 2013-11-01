/*
  EEPROM.h - EEPROM library for 86Duino
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
*/

#ifndef EEPROM_h
#define EEPROM_h

#include <inttypes.h>
#include <stdio.h>

class EEPROMClass
{
  public:
	EEPROMClass(void);
	~EEPROMClass(void);
    uint8_t read(int);
    void write(int, uint8_t);
	void EEPROMflush();
	void set_bank(bool);
	

  private:
    unsigned long int E2END;
	FILE *_file;
	bool bank;
	
	FILE* create_newfile();
	uint32_t get_size(FILE *fp);
	uint8_t eeprom_read_byte(FILE *fp, int addr);
	void eeprom_write_byte(FILE *fp, int addr, uint8_t val);
};

extern EEPROMClass EEPROM;

#endif

