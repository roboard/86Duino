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

#ifndef _SPI_H_INCLUDED
#define _SPI_H_INCLUDED

#include <Arduino.h>

#ifndef LSBFIRST
#define LSBFIRST 0
#endif
#ifndef MSBFIRST
#define MSBFIRST 1
#endif

// For Arduino compatibility (it is 16MHz ex: ATmega328p)
#define SPI_CLOCK_DIV2    (7)   // 8M   ~= 100/(2*7)
#define SPI_CLOCK_DIV4    (13)  // 4M   ~= 100/(2*13)
#define SPI_CLOCK_DIV8    (25)  // 2M   ~= 100/(2*25)
#define SPI_CLOCK_DIV16   (50)  // 1M   ~= 100/(2*50)
#define SPI_CLOCK_DIV32   (100) // 500k ~= 100/(2*100)
#define SPI_CLOCK_DIV64   (200) // 250k ~= 100/(2*200)
#define SPI_CLOCK_DIV128  (400) // 125k ~= 100/(2*400)

// Only for 86Duino Coding 102 or before, they are not used on Coding 103
#define SPI_CLOCK_DIV25   (25)
#define SPI_CLOCK_DIV100  (100)
#define SPI_CLOCK_DIV400  (400)
#define SPI_CLOCK_DIV800  (800)
#define SPI_CLOCK_DIV13   (13)
#define SPI_CLOCK_DIV50   (50)
#define SPI_CLOCK_DIV200  (200)

#define SPI_MODE0 0x00
#define SPI_MODE1 0x02
#define SPI_MODE2 0x04
#define SPI_MODE3 0x06

#define PCI_CLOCK (100000000L) // 100MHz

#define FULLDUPEX    (0x80) // 0x7F is half-dupex
#define LSBSHIFT     (0x20) // 0xDF is MSB shift
#define SMOD         (0x08) // 0xF7 is disable it
#define CPOL         (0x04) // 0xFB is disable it
#define CPHA         (0x02) // 0xFD is disable it
#define RESET        (0x01) // it will be clear when out of reset state

extern unsigned SPI_IOaddr;
extern int __clockDiv, __bitOrder, __dataMode;

class SPISettings {
public:
  SPISettings(uint32_t clock, uint8_t bitOrder, uint8_t dataMode) {
    if (__builtin_constant_p(clock)) {
      init_AlwaysInline(clock, bitOrder, dataMode);
    } else {
      init_MightInline(clock, bitOrder, dataMode);
    }
  }
  SPISettings() {
    init_AlwaysInline(4000000, MSBFIRST, SPI_MODE0);
  }
private:
  void init_MightInline(uint32_t clock, uint8_t bitOrder, uint8_t dataMode) {
    init_AlwaysInline(clock, bitOrder, dataMode);
  }
  void init_AlwaysInline(uint32_t clock, uint8_t bitOrder, uint8_t dataMode)
    __attribute__((__always_inline__)) {

    uint8_t clockDiv;

    if (__builtin_constant_p(clock)) {
      if (clock >= PCI_CLOCK / 2) {
        clockDiv = 1;
      } else if (clock >= PCI_CLOCK / 4) {
        clockDiv = 2;
      } else if (clock >= PCI_CLOCK / 8) {
        clockDiv = 4;
      } else if (clock >= PCI_CLOCK / 16) {
        clockDiv = 8;
      } else if (clock >= PCI_CLOCK / 32) {
        clockDiv = 16;
      } else if (clock >= PCI_CLOCK / 64) {
        clockDiv = 32;
      } else {
        clockDiv = 64;
      }
    } else {
      uint32_t clockSetting = PCI_CLOCK / 2;
      clockDiv = 1;
      while (clockDiv < 128 && clock < clockSetting) {
        clockSetting /= 2;
        clockDiv *= 2;
      }
    }

	__clockDiv = clockDiv;
	__bitOrder = bitOrder;
	__dataMode = dataMode;
  }
  friend class SPIClass;
};


class SPIClass {
public:
  // Initialize the SPI library
  static void begin();

  // If SPI is used from within an interrupt, this function registers
  // that interrupt with the SPI library, so beginTransaction() can
  // prevent conflicts.  The input interruptNumber is the number used
  // with attachInterrupt.  If SPI is used from a different interrupt
  // (eg, a timer), interruptNumber should be 255.
  static void usingInterrupt(uint8_t interruptNumber);
  // And this does the opposite.
  static void notUsingInterrupt(uint8_t interruptNumber);
  // Note: the usingInterrupt and notUsingInterrupt functions should
  // not to be called from ISR context or inside a transaction.
  // For details see:
  // https://github.com/arduino/Arduino/pull/2381
  // https://github.com/arduino/Arduino/pull/2449

  // Before using SPI.transfer() or asserting chip select pins,
  // this function is used to gain exclusive access to the SPI bus
  // and configure the correct settings.
  inline static void beginTransaction(SPISettings settings) {
    if (interrupt_flag > 0) {
	  io_DisableINT();
    }

    setBitOrder(__bitOrder);
	setDataMode(__dataMode);
    setClockDivider(__clockDiv);

    #ifdef SPI_TRANSACTION_MISMATCH_LED
    if (inTransactionFlag) {
      pinMode(SPI_TRANSACTION_MISMATCH_LED, OUTPUT);
      digitalWrite(SPI_TRANSACTION_MISMATCH_LED, HIGH);
    }
    inTransactionFlag = 1;
    #endif
  }

  // Write to the SPI bus (MOSI pin) and also receive (MISO pin)
  inline static uint8_t transfer(uint8_t data) {
    if(SPI_IOaddr == 0) return 0;
	io_outpb(SPI_IOaddr, data);
	while((io_inpb(SPI_IOaddr + 3) & 0x08) == 0);
	while((io_inpb(SPI_IOaddr + 3) & 0x20) == 0);
	return io_inpb(SPI_IOaddr + 1);
  }
  inline static uint16_t transfer16(uint16_t data) {
    union { uint16_t val; struct { uint8_t lsb; uint8_t msb; }; } in, out;
    if(SPI_IOaddr == 0) return 0;
    in.val = data;

    if (!(io_inpb(SPI_IOaddr + 7) & 0x20)) {
      io_outpb(SPI_IOaddr, in.msb);
      while((io_inpb(SPI_IOaddr + 3) & 0x08) == 0);
	  while((io_inpb(SPI_IOaddr + 3) & 0x20) == 0);
      out.msb = io_inpb(SPI_IOaddr + 1);
      io_outpb(SPI_IOaddr, in.lsb);
      while((io_inpb(SPI_IOaddr + 3) & 0x08) == 0);
	  while((io_inpb(SPI_IOaddr + 3) & 0x20) == 0);
      out.lsb = io_inpb(SPI_IOaddr + 1);
    } else {
      io_outpb(SPI_IOaddr, in.lsb);
      while((io_inpb(SPI_IOaddr + 3) & 0x08) == 0);
	  while((io_inpb(SPI_IOaddr + 3) & 0x20) == 0);
      out.lsb = io_inpb(SPI_IOaddr + 1);
      io_outpb(SPI_IOaddr, in.msb);
      while((io_inpb(SPI_IOaddr + 3) & 0x08) == 0);
	  while((io_inpb(SPI_IOaddr + 3) & 0x20) == 0);
      out.msb = io_inpb(SPI_IOaddr + 1);
    }
    return out.val;
  }
  inline static void transfer(void *buf, size_t count) {
    if(SPI_IOaddr == 0 || count == 0) return;

    uint8_t *p = (uint8_t *)buf;
    io_outpb(SPI_IOaddr, *p);
    while (--count > 0) {
      uint8_t out = *(p + 1);
      while((io_inpb(SPI_IOaddr + 3) & 0x08) == 0);
	  while((io_inpb(SPI_IOaddr + 3) & 0x20) == 0);
      uint8_t in = io_inpb(SPI_IOaddr + 1);
      io_outpb(SPI_IOaddr, out);
      *p++ = in;
    }
    while((io_inpb(SPI_IOaddr + 3) & 0x08) == 0);
	while((io_inpb(SPI_IOaddr + 3) & 0x20) == 0);
    *p = io_inpb(SPI_IOaddr + 1);
  }

  // After performing a group of transfers and releasing the chip select
  // signal, this function allows others to access the SPI bus
  inline static void endTransaction(void) {
    #ifdef SPI_TRANSACTION_MISMATCH_LED
    if (!inTransactionFlag) {
      pinMode(SPI_TRANSACTION_MISMATCH_LED, OUTPUT);
      digitalWrite(SPI_TRANSACTION_MISMATCH_LED, HIGH);
    }
    inTransactionFlag = 0;
    #endif

    if (interrupt_flag > 0) {
	  io_RestoreINT();
    }
  }

  // Disable the SPI bus
  static void end();

  // These undocumented functions should not be used.  SPI.transfer()
  // polls the hardware flag which is automatically cleared as the
  // AVR responds to SPI's interrupt
  inline static void attachInterrupt();
  inline static void detachInterrupt(); // Default

  static void setBitOrder(uint8_t);
  static void setDataMode(uint8_t);
  static void setClockDivider(uint16_t);
  static void SPICS(uint8_t data);
  static void setSS(uint8_t data);

private:
  static uint8_t initialized;
  static uint8_t interruptMode; // 0=none, 1=mask, 2=global
  static uint8_t interruptMask; // which interrupts to mask
  static uint8_t interruptSave; // temp storage, to restore state
  static unsigned long long interrupt_flag;
  #ifdef SPI_TRANSACTION_MISMATCH_LED
  static uint8_t inTransactionFlag;
  #endif
};

extern SPIClass SPI;

void SPIClass::attachInterrupt() {
	if(SPI_IOaddr == 0) return;
	io_outpb(SPI_IOaddr + 8, io_inpb(SPI_IOaddr + 8) | 0x01);
}

void SPIClass::detachInterrupt() {
	if(SPI_IOaddr == 0) return;
	io_outpb(SPI_IOaddr + 8, io_inpb(SPI_IOaddr + 8) & 0xFE);
}

#endif
