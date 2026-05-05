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

#if defined (__86DUINO_EX)
    #ifndef LSBFIRST
		#define LSBFIRST 0
	#endif
	#ifndef MSBFIRST
		#define MSBFIRST 1
	#endif
    extern unsigned short SPI_IOaddr;
#elif defined (__86DUINO_EX2)
	#include "SPIreg.h"
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
    
    if (clock == 0) clock = 1;
    clockDiv = PCI_CLOCK / (2 * clock);

	__clockDiv = clockDiv;
	__bitOrder = bitOrder;
	__dataMode = dataMode;
  }
  int __clockDiv;
  int __bitOrder;
  int __dataMode;
  friend class SPIClass;
};


class SPIClass {
public:
  // Initialize the SPI library
  SPIClass(uint8_t dev);
  
  void begin();
  uint8_t transfer(uint8_t data);
  uint16_t transfer16(uint16_t data);
  void transfer(void *buf, size_t count);

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
  inline void beginTransaction(SPISettings settings) {
    if (interrupt_flag > 0) {
	  io_DisableINT();
    }

    setBitOrder(settings.__bitOrder);
	setDataMode(settings.__dataMode);
    setClockDivider(settings.__clockDiv);

    #ifdef SPI_TRANSACTION_MISMATCH_LED
    if (inTransactionFlag) {
      pinMode(SPI_TRANSACTION_MISMATCH_LED, OUTPUT);
      digitalWrite(SPI_TRANSACTION_MISMATCH_LED, HIGH);
    }
    inTransactionFlag = 1;
    #endif
  }

  // After performing a group of transfers and releasing the chip select
  // signal, this function allows others to access the SPI bus
  inline void endTransaction(void) {
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
  void end();

  // These undocumented functions should not be used.  SPI.transfer()
  // polls the hardware flag which is automatically cleared as the
  // AVR responds to SPI's interrupt
  void attachInterrupt();
  void detachInterrupt(); // Default

  void setBitOrder(uint8_t);
  void setDataMode(uint8_t);
  void setClockDivider(uint16_t);
  void SPICS(uint8_t data);
  void setSS(uint8_t data);
  void SPICS1(uint8_t data);
  void setSS1(uint8_t data);
private:
  uint8_t spi_dev;
  unsigned SPI_IOaddr;
  
  uint8_t initialized;
  static uint8_t interruptMode; // 0=none, 1=mask, 2=global
  static uint8_t interruptMask; // which interrupts to mask
  static uint8_t interruptSave; // temp storage, to restore state
  static unsigned long long interrupt_flag;
  #ifdef SPI_TRANSACTION_MISMATCH_LED
  static uint8_t inTransactionFlag;
  #endif
};

extern SPIClass SPI;
#if defined (__86DUINO_EX2)
extern SPIClass SPI1;
#endif

#endif
