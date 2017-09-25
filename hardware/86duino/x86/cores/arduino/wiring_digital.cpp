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

DMPAPI(void) pinMode(uint8_t pin, uint8_t mode) {
	int crossbar_bit;
	if(pin >= PINS || PIN86[pin].gpN == NOUSED) return;
	
	crossbar_bit = PIN86[pin].gpN;
	
	io_DisableINT();
	if (mode == INPUT)
	{
	    io_outpb(CROSSBARBASE + 0x30 + crossbar_bit, TRI_STATE);
	    io_outpb(GPIODIRBASE + 4*(crossbar_bit/8), io_inpb(GPIODIRBASE + 4*(crossbar_bit/8))&~(1<<(crossbar_bit%8)));   
	}
	else if(mode == INPUT_PULLDOWN)
	{
		io_outpb(CROSSBARBASE + 0x30 + crossbar_bit, PULL_DOWN);
	    io_outpb(GPIODIRBASE + 4*(crossbar_bit/8), io_inpb(GPIODIRBASE + 4*(crossbar_bit/8))&~(1<<(crossbar_bit%8)));   
	}
	else if (mode == INPUT_PULLUP)
	{ 
	    io_outpb(CROSSBARBASE + 0x30 + crossbar_bit, PULL_UP);
	    io_outpb(GPIODIRBASE + 4*(crossbar_bit/8), io_inpb(GPIODIRBASE + 4*(crossbar_bit/8))&~(1<<(crossbar_bit%8)));
	}
    else if (mode == INPUT_NOCHANGE)
	{
        // do nothing
    }
	else
	    io_outpb(GPIODIRBASE + 4*(crossbar_bit/8), io_inpb(GPIODIRBASE + 4*(crossbar_bit/8))|(1<<(crossbar_bit%8)));      

    io_RestoreINT();
}

DMPAPI(void) digitalWrite(uint8_t pin, uint8_t val) {
    unsigned int port;
    unsigned int value;
    int crossbar_bit;
	if(pin >= PINS || PIN86[pin].gpN == NOUSED) return;

    #if defined (DMP_DOS_BC) || defined (DMP_DOS_DJGPP) || defined (DMP_DOS_WATCOM)
		if(pin == 32) timer1_pin32_isUsed = true;
	#endif

	crossbar_bit = PIN86[pin].gpN;
    port = GPIODATABASE + 4*(crossbar_bit/8);
    value = 1<<(crossbar_bit%8);

    io_DisableINT();
    
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

    io_RestoreINT();
}

DMPAPI(int) digitalRead(uint8_t pin) {
	int crossbar_bit, val;
	if(pin >= PINS || PIN86[pin].gpN == NOUSED) return LOW;
	
	crossbar_bit = PIN86[pin].gpN;
	
    #if defined (DMP_DOS_BC) || defined (DMP_DOS_DJGPP) || defined (DMP_DOS_WATCOM)
		if(pin == 32) timer1_pin32_isUsed = true;
	#endif

    io_DisableINT();
    
	if(crossbar_bit > 31)
		io_outpb(CROSSBARBASE + 0x80 + (crossbar_bit/8), 0x01);
	else if(crossbar_bit <= 31 && io_inpb(CROSSBARBASE + 0x90 + crossbar_bit) != 0x01)
	{
		io_outpb(CROSSBARBASE + 0x90 + crossbar_bit, 0x01);
		Close_Pwm(pin);
	}  

	val = io_inpb(GPIODATABASE + 4*(crossbar_bit/8))&(1<<(crossbar_bit%8));

    io_RestoreINT();

	if(val != 0) return HIGH;
	return LOW;
}

void setPinStatus(void) {
	int i;
	for(i=0; i<PINS; i++)
	{
		if(PIN86[i].gpMode != NOSTATUS) pinMode(i, PIN86[i].gpMode);
		if(PIN86[i].gpOutput != NOSTATUS) digitalWrite(i, PIN86[i].gpOutput);
	}
}
