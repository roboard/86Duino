/*
  wanalog.cpp - analog input and output for 86duino
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

#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include <string.h>


#include "Arduino.h"
#include "io.h"
#include "dmpcfg.h"
#include "mcm.h"

#define BaseAddress (0xfe00)
#define TimeOut		(1000)
#define MCM_MC      (0)
#define MCM_MD      (1)
static int mc_md_inuse[PINS];

#define ADC_RESOLUTION    (11) // for 86Duino
#define PWM_RESOLUTION    (13) // for 86duino

void analogReference(uint8_t mode) {
	return;	
}

static int _readResolution = 10; // Arduino compatibility
static int _writeResolution = 8; // Arduino compatibility
void analogReadResolution(int res) {
	_readResolution = res;
}

void analogWriteResolution(int res) {
	_writeResolution = res;
}

static unsigned long mapResolution(unsigned long value, unsigned long from, unsigned long to) {
	if (from == to)
		return value;
	if (from > to)
		return value >> (from-to);
	else
		return value << (to-from);
}


void Close_Pwm(uint8_t pin) {
	int mc, md;
	if(pin >= PINS) return;
	
	mc = arduino_to_mc_md[MCM_MC][pin];
	md = arduino_to_mc_md[MCM_MD][pin];
	
	io_DisableINT();
	if(mc_md_inuse[pin] == 1)
	{	
		mcpwm_Disable(mc, md); 
		mc_md_inuse[pin] = 0;
	}
	io_RestoreINT();   
}

int analogRead(uint8_t pin) {
	unsigned long d;
	unsigned long time;
	
	if((pin > 6 && pin < 45) || pin > 51) return 0xffff;
	
	#if defined(__86DUINO_EDUCAKE) || defined(__86DUINO_ONE) || defined(__86DUINO_ZERO)
		if(pin >= 45) pin -= 45;
	#endif
	
	while((io_inpb(BaseAddress + 2) & 0x01) != 0)
		io_inpb(BaseAddress + 4);
	
	io_DisableINT();	
	io_outpb(BaseAddress + 1, 0x08); // disable ADC
	io_outpb(BaseAddress + 0, (0x01<<pin));
	io_outpb(BaseAddress + 1, 0x01); // enable ADC_ST
	
	for(time = timer_NowTime(); (io_inpb(BaseAddress + 2) & 0x01) == 0;)
	{
		if((timer_NowTime() - time) > TimeOut)
		{
			io_RestoreINT();
			return 0xffff;
		}
	}
	
    d = io_inpw(BaseAddress + 4);
	io_RestoreINT();
	
	d = mapResolution((d&0x7ffL), ADC_RESOLUTION, _readResolution);
	
	return d;
}

#define MAX_RESOLUTION    (100000L)
static unsigned short crossbar_ioaddr = 0;
void analogWrite(uint8_t pin, unsigned long val) {
	float unit;
	int mc, md;
		
	if(pin >= PINS) return;
	pinMode(pin, OUTPUT);
	
	if (val == 0L)
		digitalWrite(pin, LOW);
	else if (val >= ((0x00000001L<<_writeResolution)-1))
		digitalWrite(pin, HIGH);
	else
	{
	    mc = arduino_to_mc_md[MCM_MC][pin];
	    md = arduino_to_mc_md[MCM_MD][pin];
	    
	    if(mc == NOPWM || md == NOPWM)
		{
			if (val < (0x00000001L<<(_writeResolution-1)))
				digitalWrite(pin, LOW);
			else
				digitalWrite(pin, HIGH);
			return;				
	    }
		  
	    // Init H/W PWM
	    io_DisableINT();
	    if(mc_md_inuse[pin] == 0)
		{
			mcpwm_ReloadPWM(mc, md, MCPWM_RELOAD_CANCEL);
			mcpwm_SetOutMask(mc, md, MCPWM_HMASK_NONE + MCPWM_LMASK_NONE);
			mcpwm_SetOutPolarity(mc, md, MCPWM_HPOL_NORMAL + MCPWM_LPOL_NORMAL);
			mcpwm_SetDeadband(mc, md, 0L);
			mcpwm_ReloadOUT_Unsafe(mc, md, MCPWM_RELOAD_NOW);
			
			mcpwm_SetWaveform(mc, md, MCPWM_EDGE_A0I1);
			mcpwm_SetSamplCycle(mc, md, 1999L);   // sample cycle: 20ms
			
			crossbar_ioaddr = sb_Read16(0x64)&0xfffe;
			if (pin <= 9)
				io_outpb(crossbar_ioaddr + 2, 0x01); // GPIO port2: 0A, 0B, 0C, 3A
			else if (pin > 28)
		    	io_outpb(crossbar_ioaddr, 0x03); // GPIO port0: 2A, 2B, 2C, 3C
			else
				io_outpb(crossbar_ioaddr + 3, 0x02); // GPIO port3: 1A, 1B, 1C, 3B
	    }
	    io_RestoreINT();
	    
	    val = mapResolution(val, _writeResolution, PWM_RESOLUTION);
	    unit = ((float)MAX_RESOLUTION)/((float)(0x00000001L<<PWM_RESOLUTION));
	    
	    io_DisableINT();
		mcpwm_SetWidth(mc, md, 1000L*SYSCLK, ((float)val)*unit);
	    mcpwm_ReloadPWM(mc, md, MCPWM_RELOAD_PEREND);
	    if(mc_md_inuse[pin] == 0)
		{
			mcpwm_Enable(mc, md);
			io_outpb(crossbar_ioaddr + 0x90 + pinMap[pin], 0x08);
			mc_md_inuse[pin] = 1;
	    }
		io_RestoreINT();  
	}
}

double cpuTemperature(void) {
    return cpuTemperature(DEGREE_C);
}

double cpuTemperature(uint8_t unit) {
	unsigned long d;
	unsigned long time;
	double temperature;
	
	while((io_inpb(BaseAddress + 2) & 0x01) != 0)
		io_inpb(BaseAddress + 4);

	io_DisableINT();
	sb1_Write(0xe0, sb1_Read(0xe0) | (0x01L<<21));
	io_outpb(BaseAddress + 1, 0x08); // disable ADC
	io_outpb(BaseAddress + 0, 0x80); // ex: any pin is temperature pin
	io_outpb(BaseAddress + 1, 0x01); // enable ADC_ST

	for(time = timer_NowTime(); (io_inpb(BaseAddress + 2) & 0x01) == 0; )
	{
		if(timer_NowTime() - time > TimeOut)
			return 0xffff;
	}

    d = io_inpw(BaseAddress + 4);
	sb1_Write(0xe0, sb1_Read(0xe0) & ~(0x01L<<21));
	io_RestoreINT();

	temperature = 0.2173913*(d&0x7ffL)-66.0;
	
	if(unit == DEGREE_F) temperature = (temperature * 1.8) + 32.0;

	return temperature;
}

