/*
  EEPROM.h - EEPROM library
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
  
  Modified 25 November 2013 by Vic Cheng
*/

#ifndef EEPROM_h
#define EEPROM_h

#include "RBlock.h"

#define EEPROM_200B    (0)
#define EEPROM_16K     (1)

class EEPROMClass
{
  public:
	EEPROMClass(void);
	~EEPROMClass(void);
    unsigned char read(unsigned short int in_addr);
    void write(unsigned short int in_addr, unsigned char in_data);
    void set_bank(bool);
	void setBank(int);
	
  private:
    EEPROMBlock **_data;
	int _bank;
};

extern EEPROMClass EEPROM;

#endif

