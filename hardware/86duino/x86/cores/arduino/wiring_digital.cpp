/*
  wdigital.cpp - digital input and output functions for 86Duino
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
*/

#include "Arduino.h"
#include "io.h"

#define TRI_STATE     (0x00)
#define PULL_UP       (0x01)
#define PULL_DOWN     (0x02)

void pinMode(uint8_t pin, uint8_t mode) {
	int i, crossbar_bit;
	if(pin >= PINS) return;
	
	crossbar_bit = pinMap[pin];
	
	#if defined (DMP_LINUX)
        lockGPIO(crossbar_bit/8);
    #elif defined (DMP_DOS_BC) || defined (DMP_DOS_DJGPP) || defined (DMP_DOS_WATCOM)
        io_DisableINT();
    #endif
    
	if (mode == INPUT)
	{
	    io_outpb(CROSSBARBASE + 0x30 + pinMap[pin], TRI_STATE);
	    io_outpb(GPIODIRBASE + 4*(crossbar_bit/8), io_inpb(GPIODIRBASE + 4*(crossbar_bit/8))&~(1<<(crossbar_bit%8)));   
	}
	else if(mode == INPUT_PULLDOWN)
	{
		io_outpb(CROSSBARBASE + 0x30 + pinMap[pin], PULL_DOWN);
	    io_outpb(GPIODIRBASE + 4*(crossbar_bit/8), io_inpb(GPIODIRBASE + 4*(crossbar_bit/8))&~(1<<(crossbar_bit%8)));   
	}
	else if (mode == INPUT_PULLUP)
	{ 
	    io_outpb(CROSSBARBASE + 0x30 + pinMap[pin], PULL_UP);
	    io_outpb(GPIODIRBASE + 4*(crossbar_bit/8), io_inpb(GPIODIRBASE + 4*(crossbar_bit/8))&~(1<<(crossbar_bit%8)));
	}     
	else
	    io_outpb(GPIODIRBASE + 4*(crossbar_bit/8), io_inpb(GPIODIRBASE + 4*(crossbar_bit/8))|(1<<(crossbar_bit%8)));      
	
    #if defined (DMP_LINUX)
        unLockGPIO(crossbar_bit/8);
    #elif defined (DMP_DOS_BC) || defined (DMP_DOS_DJGPP) || defined (DMP_DOS_WATCOM)
        io_RestoreINT();
    #endif    
}

void digitalWrite(uint8_t pin, uint8_t val) {
    unsigned int port;
    unsigned int value;
    int crossbar_bit;
	if(pin >= PINS) return;
	
    #if defined (DMP_DOS_BC) || defined (DMP_DOS_DJGPP) || defined (DMP_DOS_WATCOM)
		if(pin == 32) timer1_pin32_isUsed = true;
	#endif
	
	crossbar_bit = pinMap[pin];
    port = GPIODATABASE + 4*(crossbar_bit/8);
    value = 1<<(crossbar_bit%8);
    
    #if defined (DMP_LINUX)
        lockGPIO(crossbar_bit/8);
    #elif defined (DMP_DOS_BC) || defined (DMP_DOS_DJGPP) || defined (DMP_DOS_WATCOM)
        io_DisableINT();
    #endif
    
	if(crossbar_bit > 31)
		io_outpb(CROSSBARBASE + 0x80 + (crossbar_bit/8), 0x01);
	else if(crossbar_bit <= 31 && io_inpb(CROSSBARBASE + 0x90 + crossbar_bit) != 0x01)
	{
		io_outpb(CROSSBARBASE + 0x90 + crossbar_bit, 0x01);
		Close_Pwm(pin);
    }

    if (val == LOW) 
        io_outpb(port, io_inpb(port)&(~value));
    else
        io_outpb(port, io_inpb(port)|value);
    
    #if defined (DMP_LINUX)
        unLockGPIO(crossbar_bit/8);
    #elif defined (DMP_DOS_BC) || defined (DMP_DOS_DJGPP) || defined (DMP_DOS_WATCOM)
        io_RestoreINT();
    #endif    
}

int digitalRead(uint8_t pin) {
	int crossbar_bit, val;
	if(pin >= PINS) return LOW;
	
    #if defined (DMP_DOS_BC) || defined (DMP_DOS_DJGPP) || defined (DMP_DOS_WATCOM)
		if(pin == 32) timer1_pin32_isUsed = true;
	#endif
	
	crossbar_bit = pinMap[pin];

    #if defined (DMP_LINUX)
        lockGPIO(crossbar_bit/8);
    #elif defined (DMP_DOS_BC) || defined (DMP_DOS_DJGPP) || defined (DMP_DOS_WATCOM)
        io_DisableINT();
    #endif
    
	if(crossbar_bit > 31)
		io_outpb(CROSSBARBASE + 0x80 + (crossbar_bit/8), 0x01);
	else if(crossbar_bit <= 31 && io_inpb(CROSSBARBASE + 0x90 + crossbar_bit) != 0x01)
	{
		io_outpb(CROSSBARBASE + 0x90 + crossbar_bit, 0x01);
		Close_Pwm(pin);
	}  

	val = io_inpb(GPIODATABASE + 4*(crossbar_bit/8))&(1<<(crossbar_bit%8));
    
    #if defined (DMP_LINUX)
        unLockGPIO(crossbar_bit/8);
    #elif defined (DMP_DOS_BC) || defined (DMP_DOS_DJGPP) || defined (DMP_DOS_WATCOM)
        io_RestoreINT();
    #endif    

	if(val != 0) return HIGH;
	return LOW;
}
