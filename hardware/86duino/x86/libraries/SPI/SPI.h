/*
 * Copyright (c) 2010 by Cristian Maglie <c.maglie@bug.st>
 * SPI Master library for arduino.
 *
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of either the GNU General Public License version 2
 * or the GNU Lesser General Public License version 2.1, both as
 * published by the Free Software Foundation.
 */

#ifndef _SPI_H_INCLUDED
#define _SPI_H_INCLUDED

#include <stdio.h>
#include <Arduino.h>

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

class SPIClass {
public:
  static void begin(); // Default
  static void end();

  static void setBitOrder(uint8_t);
  static void setDataMode(uint8_t);
  static void setClockDivider(uint16_t);
  static void SPICS(uint8_t);
  static uint8_t transfer(uint8_t);
  static void attachInterrupt();
  static void detachInterrupt(); // Default
};

extern SPIClass SPI;

#endif
