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


static unsigned short crossbar_ioaddr = 0;
static unsigned short gpio_ioaddr = 0;
void pinMode(uint8_t pin, uint8_t mode) { 
	static bool pinmode_init = false;
	int i, crossbar_bit;
	if(pin >= PINS) return;
	
	if(pinmode_init == false)
	{
		// check crossbar registers
		crossbar_ioaddr = sb_Read16(SB_CROSSBASE) & 0xfffe;
		if(crossbar_ioaddr == 0 || crossbar_ioaddr == 0xfffe)
		{
			sb_Write16(SB_CROSSBASE, CROSSBARBASE | 0x01);
			crossbar_ioaddr = CROSSBARBASE;
		}
		
		// check gpio registers
		gpio_ioaddr = sb_Read16(SB_GPIOBASE) & 0xfffe;
		if(gpio_ioaddr == 0 || gpio_ioaddr == 0xfffe)
		{
			sb_Write16(SB_GPIOBASE, GPIOCTRLBASE | 0x01);
			gpio_ioaddr = GPIOCTRLBASE;
		}
		
		io_outpdw(gpio_ioaddr, 0x00ff);
		for(i=0;i<8;i++)
			io_outpdw(gpio_ioaddr + (i+1)*4,((GPIODIRBASE + i*4)<<16) + GPIODATABASE + i*4);
		pinmode_init = true;
	} 
	
	crossbar_bit = pinMap[pin];
	
	io_DisableINT();
	if (mode == INPUT)
	{
	    io_outpb(crossbar_ioaddr + 0x30 + pinMap[pin], TRI_STATE);
	    io_outpb(GPIODIRBASE + 4*(crossbar_bit/8), io_inpb(GPIODIRBASE + 4*(crossbar_bit/8))&~(1<<(crossbar_bit%8)));   
	}
	else if(mode == INPUT_PULLDOWN)
	{
		io_outpb(crossbar_ioaddr + 0x30 + pinMap[pin], PULL_DOWN);
	    io_outpb(GPIODIRBASE + 4*(crossbar_bit/8), io_inpb(GPIODIRBASE + 4*(crossbar_bit/8))&~(1<<(crossbar_bit%8)));   
	}
	else if (mode == INPUT_PULLUP)
	{ 
	    io_outpb(crossbar_ioaddr + 0x30 + pinMap[pin], PULL_UP);
	    io_outpb(GPIODIRBASE + 4*(crossbar_bit/8), io_inpb(GPIODIRBASE + 4*(crossbar_bit/8))&~(1<<(crossbar_bit%8)));
	}     
	else
	    io_outpb(GPIODIRBASE + 4*(crossbar_bit/8), io_inpb(GPIODIRBASE + 4*(crossbar_bit/8))|(1<<(crossbar_bit%8)));      
	io_RestoreINT();
}

void digitalWrite(uint8_t pin, uint8_t val) {
    unsigned int port;
    unsigned int value;
    int crossbar_bit;
	if(pin >= PINS) return;
    if(crossbar_ioaddr == 0 || gpio_ioaddr == 0) return;// You should use pinMode() first
	
	if(pin == 32) timer1_pin32_isUsed = true;
	
	crossbar_bit = pinMap[pin];
    port = GPIODATABASE + 4*(crossbar_bit/8);
    value = 1<<(crossbar_bit%8);
    
	if(crossbar_bit > 31)
		io_outpb(crossbar_ioaddr + 0x80 + (crossbar_bit/8), 0x01);
	else if(crossbar_bit <= 31 && (crossbar_ioaddr + 0x90 + crossbar_bit) != 0x01)
	{
		io_outpb(crossbar_ioaddr + 0x90 + crossbar_bit, 0x01);
		Close_Pwm(pin);
    }
    
    io_DisableINT();
    if (val == LOW) 
        io_outpb(port, io_inpb(port)&(~value));
    else
        io_outpb(port, io_inpb(port)|value);
    io_RestoreINT();
}

int digitalRead(uint8_t pin) {
	int crossbar_bit;
	if(pin >= PINS) return 0xffff;
	if(crossbar_ioaddr == 0 || gpio_ioaddr == 0) return 0xffff;// You should use pinMode() first
	
	if(pin == 32) timer1_pin32_isUsed = true;
	
	crossbar_bit = pinMap[pin];
	
	if(crossbar_bit > 31)
		io_outpb(crossbar_ioaddr + 0x80 + (crossbar_bit/8), 0x01);
	else if(crossbar_bit <= 31 && (crossbar_ioaddr + 0x90 + crossbar_bit) != 0x01)
	{
		io_outpb(crossbar_ioaddr + 0x90 + crossbar_bit, 0x01);
		Close_Pwm(pin);
	}  
	
	if(io_inpb(GPIODATABASE + 4*(crossbar_bit/8))&(1<<(crossbar_bit%8))) return HIGH;
	return LOW;
}
