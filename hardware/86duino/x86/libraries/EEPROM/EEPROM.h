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

#include "v86board.h"
#include "Block.h"

#define EEPROM_200B    (255)
#if defined (__86DUINO_DUO)
    #define EEPROM_16K     (0)
    #define EEPROM_16K_1   (1)
    #define EEPROM_16K_2   (2)
    #define EEPROM_15K     (3)
    #define EEPROM_16K_3   (4)
    #define EEPROM_16K_4   (5)
    #define EEPROM_95K     (6) // 7 Bank
#elif defined (__86DUINO_QEC) || defined (__86DUINO_QEC_M2) || defined (__86DUINO_QEC_7) || defined (__86DUINO_QEC_9) || defined (__86DUINO_QEC_15) || defined (__86DUINO_QEC_M02) || defined (__86DUINO_QEC_PPC104) || defined (__86DUINO_QEC_PPC9) || defined (__86DUINO_PPC9S) || defined (__86DUINO_PPC15S)
    #define EEPROM_16K     (0)
    #define EEPROM_16K_1   (1) // reserve for private, not for user
#else
    #define EEPROM_16K     (1)
#endif

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

