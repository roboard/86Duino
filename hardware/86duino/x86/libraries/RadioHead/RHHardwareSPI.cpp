// RHHardwareSPI.h
// Author: Mike McCauley (mikem@airspayce.com)
// Copyright (C) 2011 Mike McCauley
// Contributed by Joanna Rutkowska
// $Id: RHHardwareSPI.cpp,v 1.11 2014/08/12 00:54:52 mikem Exp $

#include <RHHardwareSPI.h>

// Declare a single default instance of the hardware SPI interface class
RHHardwareSPI hardware_spi;

/*
#ifdef RH_HAVE_HARDWARE_SPI
	HardwareSPI SPI(1);
#endif
*/

RHHardwareSPI::RHHardwareSPI(Frequency frequency, BitOrder bitOrder, DataMode dataMode)
    :
    RHGenericSPI(frequency, bitOrder, dataMode)
{
}

uint8_t RHHardwareSPI::transfer(uint8_t data) 
{
    return SPI.transfer(data);
}

void RHHardwareSPI::attachInterrupt() 
{
    SPI.attachInterrupt();
}

void RHHardwareSPI::detachInterrupt() 
{
    SPI.detachInterrupt();
}

void RHHardwareSPI::begin() 
{
	SPI.begin();
	
    // Sigh: there are no common symbols for some of these SPI options across all platforms
    uint8_t dataMode;
    if (_dataMode == DataMode0)
		dataMode = SPI_MODE0;
    else if (_dataMode == DataMode1)
		dataMode = SPI_MODE1;
    else if (_dataMode == DataMode2)
		dataMode = SPI_MODE2;
    else if (_dataMode == DataMode3)
		dataMode = SPI_MODE3;
    else
		dataMode = SPI_MODE0;

	SPI.setDataMode(dataMode);

    uint8_t bitOrder;
    if (_bitOrder == BitOrderLSBFirst)
		bitOrder = LSBFIRST;
    else
		bitOrder = MSBFIRST;
    SPI.setBitOrder(bitOrder);

    uint8_t divider;
    switch (_frequency)
    {
	case Frequency1MHz:
	    divider = 50;
	    break;
	case Frequency2MHz:
	    divider = 25;
	    break;
	case Frequency4MHz:
	    divider = 13;
	    break;
	case Frequency8MHz:
	    divider = 6;
	    break;
	case Frequency16MHz:
	    divider = 3;
	    break;
	default:
		divider = 13;
		break;
    }
    
	SPI.setClockDivider(divider);
}

void RHHardwareSPI::end() 
{
    return SPI.end();
}

