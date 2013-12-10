#ifndef _WSPI_H_INCLUDED
#define _WSPI_H_INCLUDED

#include <stdio.h>
#include <Arduino.h>

#define WIFI_SPI_CLOCK_DIV25   (25) 
#define WIFI_SPI_CLOCK_DIV100  (100)    
#define WIFI_SPI_CLOCK_DIV400  (400)    
#define WIFI_SPI_CLOCK_DIV800  (800)    
#define WIFI_SPI_CLOCK_DIV13   (13)     
#define WIFI_SPI_CLOCK_DIV50   (50)     
#define WIFI_SPI_CLOCK_DIV200  (200)

#define WIFI_SPI_MODE0 0x00
#define WIFI_SPI_MODE1 0x02
#define WIFI_SPI_MODE2 0x04
#define WIFI_SPI_MODE3 0x06

class WIFI_SPIClass {
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

extern WIFI_SPIClass wSPI;

#endif
