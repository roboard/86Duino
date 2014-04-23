/*
  wSPI.cpp - Wifi SPI library for 86Duino
  Copyright (c) 2013 Android Lin <acen@dmp.com.tw>. All right reserved.

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

#include "Arduino.h"
#include "wSPI.h"
// #define WIFI_DEBUG_MODE    (1)

WIFI_SPIClass wSPI;

unsigned WIFI_SPI_IOaddr = 0;

static void WIFI_useFIFO(void) {
	if(WIFI_SPI_IOaddr == 0) return;
	io_outpb(WIFI_SPI_IOaddr + 2, io_inpb(WIFI_SPI_IOaddr + 2) | 0x10);
}

// we must use internal FIFO, so only view "FIFO enable" option description in the CPU datasheet
#define WIFI_SPI_BUSY          (0x80)
#define WIFI_SPI_FIFOFULL      (0x40)
#define WIFI_SPI_DATAREADY     (0x20)
#define WIFI_SPI_SENDOK        (0x10)
#define WIFI_SPI_OFIFOEMPTY    (0x08)
#define WIFI_SPI_IFIFOFULL     (0x04)
uint8_t WIFI_ReadStateR(void) {
	if(WIFI_SPI_IOaddr == 0) return 0;
	return io_inpb(WIFI_SPI_IOaddr + 3);
}

void WIFI_SPIClass::setSS(uint8_t data) {
	SPICS(data);
}

void WIFI_SPIClass::SPICS(uint8_t data) {
	if(WIFI_SPI_IOaddr == 0) return;
	if(data != 0)
		io_outpb(WIFI_SPI_IOaddr + 4, 0x01);
	else
		io_outpb(WIFI_SPI_IOaddr + 4, 0x00);
}

// skip WIFI_SPI_IOaddr + 5
void WIFI_WriteCLKDIVR(uint8_t data) {
	if(WIFI_SPI_IOaddr == 0) return;
	io_outpb(WIFI_SPI_IOaddr + 6, data);
}

#define WIFI_FULLDUPEX    (0x80) // 0x7F is half-dupex
#define WIFI_LSBSHIFT     (0x20) // 0xDF is MSB shift
#define WIFI_SMOD         (0x08) // 0xF7 is disable it
#define WIFI_CPOL         (0x04) // 0xFB is disable it
#define WIFI_CPHA         (0x02) // 0xFD is disable it
#define WIFI_RESET        (0x01) // it will be clear when out of reset state
void WIFI_Reset(void) {
    if(WIFI_SPI_IOaddr == 0) return;
	io_outpb(WIFI_SPI_IOaddr + 7, 0x01);
    while((io_inpb(WIFI_SPI_IOaddr + 7)&0x01) != 0); // wait SPI reset for complete
}

void WIFI_WriteCTRR(uint8_t data) {
	if(WIFI_SPI_IOaddr == 0) return;
	io_outpb(WIFI_SPI_IOaddr + 7, io_inpb(WIFI_SPI_IOaddr + 7) | data);
}

// skip WIFI_SPI_IOaddr + 8&9, mean that we don't use interrupt

void WIFI_SPIClass::begin() {
	void *pciDev = NULL;
	
	if(io_Init() == false)
		return;
	
	// Get SPI device base address	
	pciDev = pci_Alloc(0x00, 0x10, 0x01); // PCI SPI configuration space
	if(pciDev == NULL) {printf("WiFi-SPI device don't exist\n"); return;}
	WIFI_SPI_IOaddr = (unsigned)(pci_In16(pciDev, 0x10) & 0xFFFFFFF0L); // get SPI base address
	#if defined WIFI_DEBUG_MODE
	printf("WiFi-SPI base address = %04X\n", WIFI_SPI_IOaddr);
	#endif
	pci_Free(pciDev);
  
    WIFI_WriteCTRR(WIFI_FULLDUPEX + WIFI_SPI_MODE0 + WIFI_RESET);
    
	io_outpb(WIFI_SPI_IOaddr + 7, WIFI_FULLDUPEX); // full-dupex
	io_outpb(WIFI_SPI_IOaddr + 7, io_inpb(WIFI_SPI_IOaddr + 7) & 0xF1 | WIFI_SPI_MODE0); // set mode
	io_outpb(WIFI_SPI_IOaddr + 0x0b, 0x08); // delay clk between two transfers
    //SOURCE clock/(2 * SPI_CLOCK_DIV)
	setClockDivider(25); // 2MHz
	WIFI_useFIFO();
	detachInterrupt();
    
	io_outpb(WIFI_SPI_IOaddr + 4, 0x01); // set CS = high
  
	// Set SS to high so a connected chip will be "deselected" by default
	digitalWrite(SS, HIGH);
	
	// When the SS pin is set as OUTPUT, it can be used as
	// a general purpose output port (it doesn't influence
	// SPI operations).
	//pinMode(SS, OUTPUT);
	
	// Warning: if the SS pin ever becomes a LOW INPUT then SPI
	// automatically switches to Slave, so the data direction of
	// the SS pin MUST be kept as OUTPUT.
	//SPCR |= _BV(MSTR);
	//SPCR |= _BV(SPE);
	
	// Set direction register for SCK and MOSI pin.
	// MISO pin automatically overrides to INPUT.
	// By doing this AFTER enabling SPI, we avoid accidentally
	// clocking in a single bit since the lines go directly
	// from "input" to SPI control.  
	// http://code.google.com/p/arduino/issues/detail?id=888
	//pinMode(SCK, OUTPUT);
	//pinMode(MOSI, OUTPUT);
}


void WIFI_SPIClass::end() {
	//SPCR &= ~_BV(SPE);
}

void WIFI_SPIClass::setBitOrder(uint8_t bitOrder)
{
	if(WIFI_SPI_IOaddr == 0) return;
	if(bitOrder == LSBFIRST)
		io_outpb(WIFI_SPI_IOaddr + 7, io_inpb(WIFI_SPI_IOaddr + 7) | WIFI_LSBSHIFT);
	else
		io_outpb(WIFI_SPI_IOaddr + 7, io_inpb(WIFI_SPI_IOaddr + 7) & ~WIFI_LSBSHIFT);
}

/*
Arduino Atmega328p
#define WIFI_SPI_MODE0 0x00 : WIFI_CPOL = 0, WIFI_CPHA = 0
#define WIFI_SPI_MODE1 0x04 : WIFI_CPOL = 0, WIFI_CPHA = 1
#define WIFI_SPI_MODE2 0x08 : WIFI_CPOL = 1, WIFI_CPHA = 0
#define WIFI_SPI_MODE3 0x0C : WIFI_CPOL = 1, WIFI_CPHA = 1

86Duino Vertox86EX
#define WIFI_SPI_MODE0 0x00 : WIFI_CPOL = 0, WIFI_CPHA = 0, WIFI_SMOD alawys is zero
#define WIFI_SPI_MODE1 0x02 : WIFI_CPOL = 0, WIFI_CPHA = 1
#define WIFI_SPI_MODE2 0x04 : WIFI_CPOL = 1, WIFI_CPHA = 0
#define WIFI_SPI_MODE3 0x06 : WIFI_CPOL = 1, WIFI_CPHA = 1
*/
void WIFI_SPIClass::setDataMode(uint8_t mode)
{
	if(WIFI_SPI_IOaddr == 0) return;
	io_outpb(WIFI_SPI_IOaddr + 7, (io_inpb(WIFI_SPI_IOaddr + 7) & 0xF1) | mode);
}

// Vertex86EX has 3 ClockDivider: High, Mid, Low
// Base + 0x06: High and Mid
// Base + 0x02: Low
// PCI clock/(2 * SPI clock divided): here PCI clock is 100MHz

// Arduino SPI
/*
#define SPI_CLOCK_DIV4 0x00     : 4MHz
#define SPI_CLOCK_DIV16 0x01    : 1MHz
#define SPI_CLOCK_DIV64 0x02    : 250kHz
#define SPI_CLOCK_DIV128 0x03   : 125kHz
#define SPI_CLOCK_DIV2 0x04     : 8MHz
#define SPI_CLOCK_DIV8 0x05     : 2MHz
#define SPI_CLOCK_DIV32 0x06    : 500MHz
it's result is 16MHz/above value, so that the lowest speed is 0.125MHz

#define SPI_CLOCK_DIV25   (25)     : 4MHz
#define SPI_CLOCK_DIV100  (100)    : 1MHz
#define SPI_CLOCK_DIV400  (400)    : 250kHz
#define SPI_CLOCK_DIV800  (800)    : 125kHz
#define SPI_CLOCK_DIV13   (13)     : 8MHz (it is 12.5)
#define SPI_CLOCK_DIV50   (50)     : 2MHz
#define SPI_CLOCK_DIV200  (200)    : 500kHz
*/
void WIFI_SPIClass::setClockDivider(uint16_t rate)
{
	if(rate == 0 && rate > 4095) return;
	if(WIFI_SPI_IOaddr == 0) return;
	if(rate > 15)
		io_outpb(WIFI_SPI_IOaddr + 6, (rate&0x0ff0)>>4);
	
	io_outpb(WIFI_SPI_IOaddr + 2, (io_inpb(WIFI_SPI_IOaddr + 2) & 0xF0) | (rate%16));
}

