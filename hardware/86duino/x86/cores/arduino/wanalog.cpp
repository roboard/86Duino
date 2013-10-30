#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include <string.h>


#include "Arduino.h"
#include "io.h"
#define USE_COMMON
#include "common.h"
#include "dmpcfg.h"
#include "mcm.h"

#define BaseAddress (0xfe00)
#define TimeOut		(5000)
#define MCM_MC      (0)
#define MCM_MD      (1)
static int mc_md_inuse[PINS];

#define ADC_RESOLUTION    (11) // for 86Duino
#define PWM_RESOLUTION    (13) // for 86duino						
static int mc = MC_MODULE0, md = MCPWM_MODULEA;
//uint8_t analog_reference = DEFAULT;

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

/*
void Close_Pwm(uint8_t pin){			
	if(pin >= PINS) return;
	if(mc_md_inuse[pin] == 0) return;
	
	mc = arduino_to_mc_md[MCM_MC][pin];
	md = arduino_to_mc_md[MCM_MD][pin];
	
	if(mc == NOPWM || md == NOPWM) return;	
	
	mcpwm_Disable(mc, md); 
	mc_md_inuse[pin] = 0;   
}
*/
int analogRead(uint8_t pin) {
	unsigned long d;
	unsigned long time;
	int i;
	
	if(pin > A6) return 0xffff;
	
	while((io_inpb(BaseAddress + 2) & 0x01) != 0)
		io_inpb(BaseAddress + 4);
		
	io_outpb(BaseAddress + 1, 0x08); // enable ADC_PD & ADC_ST
	io_outpb(BaseAddress + 0, (0x01<<pin)); // enable channel 0
	io_outpb(BaseAddress + 1, 0x01); // enable ADC_PD & ADC_ST
	time = timer_nowtime();
	
	while(1)
	{
		if(timer_nowtime() - time > TimeOut)
		{
			printf("timeout!!\n");
			return 0xffff;
		}	
		
		if((io_inpb(BaseAddress + 2) & 0x01) != 0) //data_Rdy
		{
			d = io_inpw(BaseAddress + 4);// read AUX_NO & ADC_Data
			break;
		}
	}
	
	d = mapResolution((d&0x7ffL), ADC_RESOLUTION, _readResolution);
	
	return d;
}

#define MAX_RESOLUTION    (100000L)
static unsigned short crossbar_ioaddr = 0;
void analogWrite(uint8_t pin, unsigned long val) {
	float uint;
	
	if(pin >= PINS) return;
	
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
	    if(mc_md_inuse[pin] == 0)
		{
			mcpwm_SetOutMask(mc, md, MCPWM_HMASK_NONE + MCPWM_LMASK_NONE);
			mcpwm_SetOutPolarity(mc, md, MCPWM_HPOL_NORMAL + MCPWM_LPOL_NORMAL);
			mcpwm_SetDeadband(mc, md, 0L);
			mcpwm_ReloadOUT_Unsafe(mc, md, MCPWM_RELOAD_NOW);
			
			mcpwm_SetWaveform(mc, md, MCPWM_EDGE_A0I1);
			mcpwm_SetSamplCycle(mc, md, 1999L);   // sample cycle: 20ms
			
			mcpwm_SetOutPolarity(mc, md, MCPWM_HPOL_NORMAL + MCPWM_LPOL_NORMAL);
			mcpwm_SetDeadband(mc, md, 0L);
			mcpwm_ReloadOUT_Unsafe(mc, md, MCPWM_RELOAD_NOW);
			
			crossbar_ioaddr = sb_Read16(0x64)&0xfffe;
			if (pin <= 9)
				io_outpb(crossbar_ioaddr + 2, 0x01); // GPIO port2: 0A, 0B, 0C, 3A
			else if (pin > 28)
		    	io_outpb(crossbar_ioaddr, 0x03); // GPIO port0: 2A, 2B, 2C, 3C
			else
				io_outpb(crossbar_ioaddr + 3, 0x02); // GPIO port3: 1A, 1B, 1C, 3B
	    }
	    
	    io_outpb(crossbar_ioaddr + 0x90 + pinMap[pin], 0x08);
	    val = mapResolution(val, _writeResolution, PWM_RESOLUTION);
	    uint = ((float)MAX_RESOLUTION)/((float)(0x00000001L<<PWM_RESOLUTION));
	    mcpwm_SetWidth(mc, md, 1000L*SYSCLK, ((float)val)*uint);
	    mcpwm_ReloadPWM(mc, md, MCPWM_RELOAD_PEREND);
	    
	    if(mc_md_inuse[pin] == 0)
		{
			mcpwm_Enable(mc, md);
			mc_md_inuse[pin] = 1;
	    }  
	}
}

