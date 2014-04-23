#ifndef _WSPI_H_INCLUDED
#define _WSPI_H_INCLUDED

#include <stdio.h>
#include <Arduino.h>

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

#define WIFI_SPI_MODE0 0x00
#define WIFI_SPI_MODE1 0x02
#define WIFI_SPI_MODE2 0x04
#define WIFI_SPI_MODE3 0x06

extern unsigned WIFI_SPI_IOaddr;

class WIFI_SPIClass {
public:
	//static unsigned SPI_IOaddr;
	inline static byte transfer(byte);
	inline static void attachInterrupt();
	inline static void detachInterrupt(); // Default
	
	static void begin(); // Default
	static void end();
	
	static void setBitOrder(uint8_t);
	static void setDataMode(uint8_t);
	static void setClockDivider(uint16_t);
	static void SPICS(uint8_t data);
	static void setSS(uint8_t data);
};

extern WIFI_SPIClass wSPI;

byte WIFI_SPIClass::transfer(byte data) {
	if(WIFI_SPI_IOaddr == 0) return 0;
	io_outpb(WIFI_SPI_IOaddr, data);
	while((io_inpb(WIFI_SPI_IOaddr + 3) & 0x08) == 0);
	while((io_inpb(WIFI_SPI_IOaddr + 3) & 0x20) == 0);
	return io_inpb(WIFI_SPI_IOaddr + 1);
}

void WIFI_SPIClass::attachInterrupt() {
	if(WIFI_SPI_IOaddr == 0) return;
	io_outpb(WIFI_SPI_IOaddr + 8, io_inpb(WIFI_SPI_IOaddr + 8) | 0x01);
}

void WIFI_SPIClass::detachInterrupt() {
	if(WIFI_SPI_IOaddr == 0) return;
	io_outpb(WIFI_SPI_IOaddr + 8, io_inpb(WIFI_SPI_IOaddr + 8) & 0xFE);
}

#endif
