/*
  Hserial.cpp - Hardware serial library for Wiring
  Copyright (c) 2006 Nicholas Zambetti.  All right reserved.

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
  
  Modified 23 November 2006 by David A. Mellis
  Modified 28 September 2010 by Mark Sproul
  Modified 14 August 2012 by Alarus
  Modified 01 November 2013 by Android Lin
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "Arduino.h"

#include "Hserial.h"

#define  COM1_TX    (0x9A)
#define  COM1_RX    (0x9B)
#define  COM2_TX    (0x9E)
#define  COM2_RX    (0x9F)
#define  COM3_TX    (0x9C)
#define  COM3_RX    (0x9D)

// Public Methods //////////////////////////////////////////////////////////////
HardwareSerial::HardwareSerial(int com_port, unsigned long com_baudrate, unsigned char com_format, unsigned long com_timeout) {
	port        = com_port;
	baudrate    = com_baudrate;
	format      = com_format;
	timeout     = com_timeout;
	peek_stored = false;
	handle      = NULL;
}


void HardwareSerial::begin(unsigned long baud) {
	begin(baud, format, COM_FullDuplex);
}

void HardwareSerial::begin(unsigned long baud, int comtype) {
	begin(baud, format, comtype);
}

void HardwareSerial::begin(unsigned long baud, uint8_t config) {
    begin(baud, config, COM_FullDuplex);
}

void HardwareSerial::begin(unsigned long baud, uint8_t config, int comtype) {
    unsigned short crossbar_ioaddr = 0;
	if(handle != NULL) return;
	if ((handle = com_Init(port)) == NULL) 
	{
		printf("COM init fail!!\n");
		return;
	}
	
	com_SetBPS(handle, baud);
	com_SetFormat(handle, config);  
	com_FlushTxQueue(handle);
	com_FlushRxQueue(handle);
	com_SetTimeOut(handle, timeout);
	crossbar_ioaddr = sb_Read16(0x64)&0xfffe;
	if(port == COM1)
	{
		if(comtype == COM_HalfDuplex) com_EnableHalfDuplex(handle);
		io_outpb(crossbar_ioaddr + COM1_TX, 0x08);
		io_outpb(crossbar_ioaddr + COM1_RX, 0x08);
	}
	else if(port == COM2)
	{
		if(comtype == COM_HalfDuplex) com_EnableHalfDuplex(handle);
		io_outpb(crossbar_ioaddr + COM2_TX, 0x08);
		io_outpb(crossbar_ioaddr + COM2_RX, 0x08);
	}
	else if(port == COM3)
	{
		if(comtype == COM_HalfDuplex) com_EnableHalfDuplex(handle);
		io_outpb(crossbar_ioaddr + COM3_TX, 0x08);
		io_outpb(crossbar_ioaddr + COM3_RX, 0x08);
	}
}

void HardwareSerial::end() {
	if(handle == NULL) return;
	com_FlushWFIFO(handle);
	com_Close(handle);
}

int HardwareSerial::available(void) {
    if(handle == NULL) return 0;
	return com_QueryRxQueue(handle);
} 

int HardwareSerial::peek(void) {
  	if(handle == NULL) return -1;
	if(peek_stored == true)
  		return peek_val;
  	else
  	{
  		if((peek_val = com_Read(handle)) == 0xFFFF)
  			peek_val = -1;
  		peek_stored = true;
		return peek_val;
	}
}

int HardwareSerial::read(void) {
	int c;
	if(handle == NULL) return -1;
	if(peek_stored == true)
	{
		peek_stored = false;
		return peek_val;
	}
	else
	{
		c = com_Read(handle);
		return (c == 0xFFFF) ? -1 : c;
	}
}

void HardwareSerial::flush() {
    if(handle == NULL) return;
	com_FlushTxQueue(handle);
}

size_t HardwareSerial::write(uint8_t c) {
	if(handle == NULL) return 0;
	return (com_Write(handle, c) == true) ? 1 : 0;
} 

HardwareSerial::operator bool() {
	return true;
} 

void serialEvent() __attribute__((weak));
void serialEvent1() __attribute__((weak));
void serialEvent2() __attribute__((weak));
void serialEvent3() __attribute__((weak));
void serialEvent485() __attribute__((weak));
void serialEvent4() __attribute__((weak));
void serialEvent() {}
void serialEvent1() {}
void serialEvent2() {}
void serialEvent3() {}
void serialEvent485() {}
void serialEvent4() {}
void serialEventRun(void)
{
	if(USBDEV != NULL && Serial.available() > 0) serialEvent();
	if(Serial1.handle != NULL && Serial1.available() > 0) serialEvent1();
	if(Serial2.handle != NULL && Serial2.available() > 0) serialEvent2();
	if(Serial3.handle != NULL && Serial3.available() > 0) serialEvent3();
	if(Serial485.handle != NULL && Serial485.available() > 0) serialEvent485();
	if(Serial4.handle != NULL && Serial4.available() > 0) serialEvent4();
}

HardwareSerial Serial1(COM1, 115200L, BYTESIZE8|NOPARITY|STOPBIT1, 500L);
HardwareSerial Serial2(COM2, 115200L, BYTESIZE8|NOPARITY|STOPBIT1, 500L);
HardwareSerial Serial3(COM3, 115200L, BYTESIZE8|NOPARITY|STOPBIT1, 500L);
HardwareSerial Serial485(COM4, 115200L, BYTESIZE8|NOPARITY|STOPBIT1, 500L);
HardwareSerial Serial4(COM6, 115200L, BYTESIZE8|NOPARITY|STOPBIT1, 500L);
// Preinstantiate Objects //////////////////////////////////////////////////////
