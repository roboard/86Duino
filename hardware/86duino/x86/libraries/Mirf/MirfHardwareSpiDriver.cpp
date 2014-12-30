#include "MirfHardwareSpiDriver.h"
uint8_t MirfHardwareSpiDriver::transfer(uint8_t data){
	return SPI.transfer(data);
}

void MirfHardwareSpiDriver::begin(){
	SPI.begin();
	SPI.setDataMode(SPI_MODE0);
	SPI.setClockDivider(SPI_CLOCK_DIV8);
}

void MirfHardwareSpiDriver::end(){
}

MirfHardwareSpiDriver MirfHardwareSpi;
