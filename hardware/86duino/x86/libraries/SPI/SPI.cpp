/*
 * Copyright (c) 2010 by Cristian Maglie <c.maglie@bug.st>
 * SPI Master library for arduino.
 *
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of either the GNU General Public License version 2
 * or the GNU Lesser General Public License version 2.1, both as
 * published by the Free Software Foundation.
 */

#include "Arduino.h"
#include "SPI.h"
#define DEBUG_MODE    (1)

SPIClass SPI;
static unsigned SPI_IOaddr = 0;

void WriteData(uint8_t data) {
	io_outpb(SPI_IOaddr, data);
	while((io_inpb(SPI_IOaddr + 3) & 0x08) == 0);
}

uint8_t ReadData(void) {
	while((io_inpb(SPI_IOaddr + 3) & 0x20) == 0);
	return io_inpb(SPI_IOaddr + 1);
}

uint8_t SPIClass::transfer(uint8_t _data) {
	uint8_t tmp;
	WriteData(_data);
	tmp = ReadData();
	return tmp;
}

static void useFIFO(void) {
	io_outpb(SPI_IOaddr + 2, io_inpb(SPI_IOaddr + 2) | 0x10);
}

// we must use internal FIFO, so only view "FIFO enable" option description in the CPU datasheet
#define SPI_BUSY          (0x80)
#define SPI_FIFOFULL      (0x40)
#define SPI_DATAREADY     (0x20)
#define SPI_SENDOK        (0x10)
#define SPI_OFIFOEMPTY    (0x08)
#define SPI_IFIFOFULL     (0x04)
uint8_t ReadStateR(void) {
	return io_inpb(SPI_IOaddr + 3);
}

void SPIClass::SPICS(uint8_t data) {
	if(data != 0)
		io_outpb(SPI_IOaddr + 4, 0x01);
	else
		io_outpb(SPI_IOaddr + 4, 0x00);
}

// skip SPI_IOaddr + 5

void WriteCLKDIVR(uint8_t data) {
	io_outpb(SPI_IOaddr + 6, data);
}

#define FULLDUPEX    (0x80) // 0x7F is half-dupex
#define LSBSHIFT     (0x20) // 0xDF is MSB shift
#define SMOD         (0x08) // 0xF7 is disable it
#define CPOL         (0x04) // 0xFB is disable it
#define CPHA         (0x02) // 0xFD is disable it
#define RESET        (0x01) // it will be clear when out of reset state
void Reset(void) {
    io_outpb(SPI_IOaddr + 7, 0x01);
    while((io_inpb(SPI_IOaddr + 7)&0x01) != 0); // wait SPI reset for complete
}

void WriteCTRR(uint8_t data) {
	io_outpb(SPI_IOaddr + 7, io_inpb(SPI_IOaddr + 7) | data);
}

// skip SPI_IOaddr + 8&9, mean that we don't use interrupt

void SPIClass::begin() {
	void *pciDev = NULL;
	
	// Get SPI device base address	
	pciDev = pci_Alloc(0x00, 0x10, 0x01); // PCI SPI configuration space
	if(pciDev == NULL) {printf("SPI device don't exist\n"); return;}
	SPI_IOaddr = (unsigned)(pci_In16(pciDev, 0x10) & 0xFFFFFFF0L); // get SPI base address
	#if defined DEBUG_MODE
	printf("SPI base address = %04X\n", SPI_IOaddr);
	#endif
	pci_Free(pciDev);
  
    WriteCTRR(FULLDUPEX + SPI_MODE0 + RESET);
    
	io_outpb(SPI_IOaddr + 7, FULLDUPEX); // full-dupex
	io_outpb(SPI_IOaddr + 7, io_inpb(SPI_IOaddr + 7) & 0xF1 | SPI_MODE0); // set mode
	io_outpb(SPI_IOaddr + 0x0b, 0x08); // delay clk between two transfers
    //SOURCE clock/(2 * SPI_CLOCK_DIV)
	setClockDivider(SPI_CLOCK_DIV800); // 125k Hz
	useFIFO();
	detachInterrupt();
    
	io_outpb(SPI_IOaddr + 4, 0x01); // set CS = high
  
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


void SPIClass::end() {
	//SPCR &= ~_BV(SPE);
}

void SPIClass::setBitOrder(uint8_t bitOrder)
{
	if(bitOrder == LSBFIRST) {
	  io_outpb(SPI_IOaddr + 7, io_inpb(SPI_IOaddr + 7) | LSBSHIFT);
	} else {
	  io_outpb(SPI_IOaddr + 7, io_inpb(SPI_IOaddr + 7) & ~LSBSHIFT);
	}
}

/*
Arduino Atmega328p
#define SPI_MODE0 0x00 : CPOL = 0, CPHA = 0
#define SPI_MODE1 0x04 : CPOL = 0, CPHA = 1
#define SPI_MODE2 0x08 : CPOL = 1, CPHA = 0
#define SPI_MODE3 0x0C : CPOL = 1, CPHA = 1

86Duino Vertox86EX
#define SPI_MODE0 0x00 : CPOL = 0, CPHA = 0, SMOD alawys is zero
#define SPI_MODE1 0x02 : CPOL = 0, CPHA = 1
#define SPI_MODE2 0x04 : CPOL = 1, CPHA = 0
#define SPI_MODE3 0x06 : CPOL = 1, CPHA = 1
*/
void SPIClass::setDataMode(uint8_t mode)
{
  io_outpb(SPI_IOaddr + 7, (io_inpb(SPI_IOaddr + 7) & 0xF1) | mode);
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
it's result is 16MHz/above value, so that the lowest speed is 1.25MHz

#define SPI_CLOCK_DIV25   (25)     : 4MHz
#define SPI_CLOCK_DIV100  (100)    : 1MHz
#define SPI_CLOCK_DIV400  (400)    : 250kHz
#define SPI_CLOCK_DIV800  (800)    : 125kHz
#define SPI_CLOCK_DIV13   (13)     : 8MHz (it is 12.5)
#define SPI_CLOCK_DIV50   (50)     : 2MHz
#define SPI_CLOCK_DIV200  (200)    : 500kHz
*/
void SPIClass::setClockDivider(uint16_t rate)
{
	if(rate == 0 && rate > 4095) return;
	if(rate > 255)
	{
		io_outpb(SPI_IOaddr + 6, rate/256);
	}
	io_outpb(SPI_IOaddr + 2, (io_inpb(SPI_IOaddr + 2) & 0xF0) | (rate%256));
}

// bit0: Transfer data complete
void SPIClass::attachInterrupt() {
	io_outpb(SPI_IOaddr + 8, io_inpb(SPI_IOaddr + 8) | 0x01);
}

void SPIClass::detachInterrupt() {
	io_outpb(SPI_IOaddr + 8, io_inpb(SPI_IOaddr + 8) & 0xFE);
}

