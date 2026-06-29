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

#define DATASIZE 1024
#define EEPROMSIZE_BANK0 200

#if defined (__86DUINO_DUO)
    #define ADDRESS 95 // 32 + 31 + 32 = 95
    #define BANKNUM 7
    int bank2BlockArea[BANKNUM][2] = {{0, 15}, {16, 31}, {32, 47}, {48, 62}, {63, 78}, {79, 94}, {0, 94}}; // 7 Bank to 7 kind Block area
#elif defined (__86DUINO_QEC) || defined (__86DUINO_QEC_M2) || defined (__86DUINO_QEC_7) || defined (__86DUINO_QEC_9) || defined (__86DUINO_QEC_15) || defined (__86DUINO_QEC_M02) || defined (__86DUINO_QEC_PPC104) || defined (__86DUINO_QEC_PPC9)
    #define ADDRESS 16
    #define BANKNUM 2
    int bank2BlockArea[BANKNUM][2] = {{0, 15}, {16, 31}};
#else
    #define BANKNUM 2
    #define ADDRESS 16
    #define EEPROMSIZE (ADDRESS * DATASIZE)
#endif

#if defined (EEPROM_REMOTE)
#include "ipcnet.h"

#define MAX_SOCKS   (4)
static int fd, rc;
static unsigned char rxbuf[4096];
static unsigned char txbuf[4096];
static unsigned long val;

// port 86: serial port
// port 87: SD port

// serial command: 
// 1 : read
// 2 : write
// 3 : set_bank
// 4 : setBank
EEPROMClass::EEPROMClass(void)
{
    val = 1;
    fd = ipcnet_socket();
    ipcnet_ioctl(fd, IPCNET_FIONBIO, (unsigned long)&val);

    do {
        rc = ipcnet_connect(fd, 87); // 87 : SD port
        if (rc == 0)
            break;
        // if (rc < 0 && ipcnet_errno(fd) != ipcnet_EWOULDBLOCK) {
            // printf("begining...fail!\r");
            // printf("\n\n %d\n", ipcnet_errno(fd));
            // return;
        // }
        printf("begining...\r");
    } while (1);
    printf("\n begin() OK!\n");
}

EEPROMClass::~EEPROMClass(void)
{
  ipcnet_close(fd);
  io_Close();
  // need add new command to close EEPROM class
}

static void send_command_para(uint8_t c, uint8_t* p, int p_size) {
    int i = 0, cur = 0, j;
    
    // command
    txbuf[i++] = c;
    
    // other parameters
    if (p != NULL) {
        for (j=0; j<p_size; j++)
            txbuf[i++] = *(p++);
    }
    
    do {
        rc = ipcnet_send(fd, &txbuf[cur], i - cur);
        if (rc > 0) cur += rc;
    } while (cur != i);
    
}

// -1 : continue
//  0 : complite
static int recv_command_result(uint8_t* mybuf, int num) {
    int i, cur = 0;
    
    do {
        rc = ipcnet_recv(fd, &rxbuf[cur], num - cur);
        if (rc < 0 && ipcnet_errno(fd) == IPCNET_ECONNABORTED)
            return -1;
        cur += rc;
    } while (cur != num);
    
    for (i=0; i<num; i++)
        mybuf[i] = rxbuf[i];
    
    return 0;
}

 unsigned char EEPROMClass::read(unsigned short int in_addr)
{
    int r;
    uint8_t _r[1];
    uint8_t d[2];

    d[0] = (uint8_t) ((in_addr >> 8) & 0xFF);
    d[1] = (uint8_t) (in_addr & 0xff);

    send_command_para(1, d, 2); // 1 : "read" command
    r = recv_command_result(_r, 1);
    if (r == 0) {
        return _r[0];
    } else {
        printf("read Error\n");
        return 0;
    }
  
}

void EEPROMClass::write(unsigned short int in_addr, unsigned char in_data)
{
    int r;
    uint8_t _r[1];
    uint8_t d[3];
    
    d[0] = (uint8_t) ((in_addr >> 8) & 0xFF);
    d[1] = (uint8_t) (in_addr & 0xff);
    d[2] = in_data;
    
    send_command_para(2, d, 3); // 2 : "write" command
    r = recv_command_result(_r, 1);
    if (r != 0) {
        printf("write Error\n");
    }
}

void EEPROMClass::set_bank(bool input)
{
    int r;
    uint8_t _r[1];
    uint8_t d[1];
    
    d[0] = (input == true) ? 1 : 0;
    
    send_command_para(3, d, 1); // 3 : "set_bank" command
    r = recv_command_result(_r, 1);
    if (r == 0) {
        if (_r[0] == 0)
            printf("set_bank OK!\n");
    } else {
        printf("set_bank Error\n");
    }
}

void EEPROMClass::setBank(int input)
{
    int r;
    uint8_t _r[1];
    uint8_t d[1];
    
    d[0] = (input == true) ? 1 : 0;
    
    send_command_para(4, d, 1); // 4 : "setBank" command
    r = recv_command_result(_r, 1);
    if (r == 0) {
        if (_r[0] == 0)
            printf("setBank OK!\n");
    } else {
        printf("setBank Error\n");
    }
}

#else

EEPROMClass::EEPROMClass(void)
{
  io_Init();

  _bank = EEPROM_16K; //default use bank 0;
  
  _data = new EEPROMBlock*[ADDRESS];
  
  // init 16K EEPROM 

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

unsigned char EEPROMClass::read(unsigned short int in_addr)
{
#if defined (__86DUINO_DUO) || defined (__86DUINO_QEC) || defined (__86DUINO_QEC_M2) || defined (__86DUINO_QEC_7) || defined (__86DUINO_QEC_9) || defined (__86DUINO_QEC_15) || defined (__86DUINO_QEC_M02) || defined (__86DUINO_QEC_PPC104) || defined (__86DUINO_QEC_PPC9)
  int blockOffset = bank2BlockArea[_bank][0];
  int EEPROMSIZE = (bank2BlockArea[_bank][1] - bank2BlockArea[_bank][0] + 1) * DATASIZE;
#else
  int blockOffset = 0;
#endif

  if(((in_addr >= EEPROMSIZE) && (_bank != EEPROM_200B)) || ((in_addr >= EEPROMSIZE_BANK0) && (_bank == EEPROM_200B)))//bank == true 0~16384, false 0~199
  {
	printf("EEPROMClass read error (bank = %d): in_addr = %x >= %x\n", _bank, in_addr, EEPROMSIZE);
	return 0;
  }

  if(_bank == EEPROM_200B)
    return eeprom_read_byte((uint8_t*)in_addr);
  else
	return (_data[blockOffset + (in_addr / DATASIZE)])->read(in_addr - (DATASIZE * (in_addr / DATASIZE)));
}

void EEPROMClass::write(unsigned short int in_addr, unsigned char in_data)
{
#if defined (__86DUINO_DUO) || defined (__86DUINO_QEC) || defined (__86DUINO_QEC_M2) || defined (__86DUINO_QEC_7) || defined (__86DUINO_QEC_9) || defined (__86DUINO_QEC_15) || defined (__86DUINO_QEC_M02) || defined (__86DUINO_QEC_PPC104) || defined (__86DUINO_QEC_PPC9)
  int blockOffset = bank2BlockArea[_bank][0];
  int EEPROMSIZE = (bank2BlockArea[_bank][1] - bank2BlockArea[_bank][0] + 1) * DATASIZE;
#else
  int blockOffset = 0;
#endif

  if(((in_addr >= EEPROMSIZE) && (_bank != EEPROM_200B)) || ((in_addr >= EEPROMSIZE_BANK0) && (_bank == EEPROM_200B)))//bank == true 0~16384, false 0~199
  {
	printf("EEPROMClass write error(bank = %d): in_addr = %x >= %x\n", _bank, in_addr, EEPROMSIZE);
	return;
  }
  if(_bank == EEPROM_200B)
    eeprom_write_byte((uint8_t*)in_addr, in_data);
  else
    (_data[blockOffset + (in_addr / DATASIZE)])->write(in_addr - (DATASIZE * (in_addr / DATASIZE)) , in_data);
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
  if(input != EEPROM_200B && input >= BANKNUM)
  {
  	_bank = EEPROM_16K;
  	return;
  }	
  
  _bank = input;
}
#endif

EEPROMClass EEPROM;

