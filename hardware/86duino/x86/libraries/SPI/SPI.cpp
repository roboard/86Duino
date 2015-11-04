/*
 * Copyright (c) 2010 by Cristian Maglie <c.maglie@arduino.cc>
 * Copyright (c) 2014 by Paul Stoffregen <paul@pjrc.com> (Transaction API)
 * Copyright (c) 2014 by Matthijs Kooijman <matthijs@stdin.nl> (SPISettings AVR)
 * Copyright (c) 2014 by Andrew J. Kroll <xxxajk@gmail.com> (atomicity fixes)
 * Modified by Android Lin <acen@dmp.com.tw> to support 86Duino boards
 * SPI Master library for arduino.
 *
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of either the GNU General Public License version 2
 * or the GNU Lesser General Public License version 2.1, both as
 * published by the Free Software Foundation.
 */

#include "SPI.h"

SPIClass SPI;

uint8_t SPIClass::initialized = 0;
uint8_t SPIClass::interruptMode = 0;
uint8_t SPIClass::interruptMask = 0;
uint8_t SPIClass::interruptSave = 0;
unsigned long long SPIClass::interrupt_flag = 0LL;
#ifdef SPI_TRANSACTION_MISMATCH_LED
uint8_t SPIClass::inTransactionFlag = 0;
#endif

//#define DEBUG_MODE

unsigned SPI_IOaddr = 0;
int __clockDiv = 64, __bitOrder = MSBFIRST, __dataMode = SPI_MODE0;

// we must use internal FIFO, so only view "FIFO enable" option description in the CPU datasheet
#define SPI_BUSY          (0x80)
#define SPI_FIFOFULL      (0x40)
#define SPI_DATAREADY     (0x20)
#define SPI_SENDOK        (0x10)
#define SPI_OFIFOEMPTY    (0x08)
#define SPI_IFIFOFULL     (0x04)

static void useFIFO(void) {
	if(SPI_IOaddr == 0) return;
	io_outpb(SPI_IOaddr + 2, io_inpb(SPI_IOaddr + 2) | 0x10);
}

uint8_t ReadStateR(void) {
	if(SPI_IOaddr == 0) return 0;
	return io_inpb(SPI_IOaddr + 3);
}

void SPIClass::setSS(uint8_t data) {
	SPICS(data);
}

void SPIClass::SPICS(uint8_t data) {
	if(SPI_IOaddr == 0) return;
	if(data != 0)
		io_outpb(SPI_IOaddr + 4, 0x01);
	else
		io_outpb(SPI_IOaddr + 4, 0x00);
}

// skip SPI_IOaddr + 5
void WriteCLKDIVR(uint8_t data) {
	if(SPI_IOaddr == 0) return;
	io_outpb(SPI_IOaddr + 6, data);
}

void Reset(void) {
    if(SPI_IOaddr == 0) return;
	io_outpb(SPI_IOaddr + 7, 0x01);
    while((io_inpb(SPI_IOaddr + 7)&0x01) != 0); // wait SPI reset for complete
}

void WriteCTRR(uint8_t data) {
	if(SPI_IOaddr == 0) return;
	io_outpb(SPI_IOaddr + 7, io_inpb(SPI_IOaddr + 7) | data);
}

void SPIClass::begin()
{
  if (!initialized) {
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
	setClockDivider(13); // 100/(2*13) ~= 4MHz
	useFIFO();
	detachInterrupt();

	io_outpb(SPI_IOaddr + 4, 0x01); // set CS = high

	// Set SS to high so a connected chip will be "deselected" by default
	pinMode(SS, OUTPUT);
	digitalWrite(SS, HIGH);
  }
  initialized++; // reference count
}

void SPIClass::end() {
  if (initialized) initialized--;

  if (!initialized) {
    interruptMode = 0;
    #ifdef SPI_TRANSACTION_MISMATCH_LED
    inTransactionFlag = 0;
    #endif
  }
}

// mapping of interrupt numbers to bits within SPI_AVR_EIMSK

/*
#if defined(__AVR_ATmega32U4__)
  #define SPI_INT0_MASK  (1<<INT0)
  #define SPI_INT1_MASK  (1<<INT1)
  #define SPI_INT2_MASK  (1<<INT2)
  #define SPI_INT3_MASK  (1<<INT3)
  #define SPI_INT4_MASK  (1<<INT6)
#elif defined(__AVR_AT90USB646__) || defined(__AVR_AT90USB1286__)
  #define SPI_INT0_MASK  (1<<INT0)
  #define SPI_INT1_MASK  (1<<INT1)
  #define SPI_INT2_MASK  (1<<INT2)
  #define SPI_INT3_MASK  (1<<INT3)
  #define SPI_INT4_MASK  (1<<INT4)
  #define SPI_INT5_MASK  (1<<INT5)
  #define SPI_INT6_MASK  (1<<INT6)
  #define SPI_INT7_MASK  (1<<INT7)
#elif defined(EICRA) && defined(EICRB) && defined(EIMSK)
  #define SPI_INT0_MASK  (1<<INT4)
  #define SPI_INT1_MASK  (1<<INT5)
  #define SPI_INT2_MASK  (1<<INT0)
  #define SPI_INT3_MASK  (1<<INT1)
  #define SPI_INT4_MASK  (1<<INT2)
  #define SPI_INT5_MASK  (1<<INT3)
  #define SPI_INT6_MASK  (1<<INT6)
  #define SPI_INT7_MASK  (1<<INT7)
#else
  #ifdef INT0
  #define SPI_INT0_MASK  (1<<INT0)
  #endif
  #ifdef INT1
  #define SPI_INT1_MASK  (1<<INT1)
  #endif
  #ifdef INT2
  #define SPI_INT2_MASK  (1<<INT2)
  #endif
#endif
*/

void SPIClass::usingInterrupt(uint8_t interruptNumber)
{
  if(interruptNumber > 63) return;
  interrupt_flag |= (1LL<<interruptNumber);
}

void SPIClass::notUsingInterrupt(uint8_t interruptNumber)
{
  if(interruptNumber > 63) return;
  interrupt_flag &= ~(1LL<<interruptNumber);
}

void SPIClass::setBitOrder(uint8_t bitOrder)
{
	if(SPI_IOaddr == 0) return;
	if(bitOrder == LSBFIRST)
		io_outpb(SPI_IOaddr + 7, io_inpb(SPI_IOaddr + 7) | LSBSHIFT);
	else
	  	io_outpb(SPI_IOaddr + 7, io_inpb(SPI_IOaddr + 7) & ~LSBSHIFT);
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
void SPIClass::setDataMode(uint8_t mode) {
	if(SPI_IOaddr == 0) return;
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
it's result is 16MHz/above value, so that the lowest speed is 0.125MHz

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
	if(rate == 0 || rate > 4095) return;
	if(SPI_IOaddr == 0) return;
	if(rate > 15)
		io_outpb(SPI_IOaddr + 6, (rate&0x0ff0)>>4);

	io_outpb(SPI_IOaddr + 2, (io_inpb(SPI_IOaddr + 2) & 0xF0) | (rate%16));
}

