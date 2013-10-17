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
#define TimeOut		  (5000)

static int mc = MC_MODULE2, md = MCPWM_MODULEA;
unsigned long pulseIn(uint8_t pin, uint8_t state, unsigned long timeout) {
	uint8_t stateMask;
	unsigned long width = 0; // keep initialization out of time critical area
	
	if(pin >= PINS) return 0x00;
	                                                                       
	mc_setbaseaddr();
	//Master_DX2();
	if (mc_SetMode(mc, MCMODE_PWM) == false)
	{
	    printf("ERROR: fail to change MC mode!\n");
	    //Master_DX2();
	}
	
	mcpwm_SetOutMask(mc, md, MCPWM_HMASK_NONE + MCPWM_LMASK_NONE);
	mcpwm_SetOutPolarity(mc, md, MCPWM_HPOL_NORMAL + MCPWM_LPOL_NORMAL);
	mcpwm_SetDeadband(mc, md, 0L);
	mcpwm_ReloadOUT_Unsafe(mc, md, MCPWM_RELOAD_NOW);
	
	mcpwm_SetWaveform(mc, md, MCPWM_EDGE_A0I1);
	mcpwm_SetWidth(mc, md, 100L , 50L);  // period: 20ms, duty: 2us
	mcpwm_SetSamplCycle(mc, md, 9999L);   // sample cycle: 20ms
	mcpwm_SetOutPolarity(mc, md, MCPWM_HPOL_INVERSE + MCPWM_LPOL_INVERSE);
	mcpwm_SetDeadband(mc, md, 3L);
	mcpwm_ReloadOUT_Unsafe(mc, md, MCPWM_RELOAD_PEREND);  
	stateMask = digitalRead(pin);
	while(digitalRead(pin) == state);
	while(digitalRead(pin) != state);
	mcpwm_Enable(mc, md);
	while(digitalRead(pin) == state);
	width=mcpwm_ReadSTATREG1(mc, md);
	mcpwm_Disable(mc, md);
	printf("%ld \n",10003L-width+((10002L-width)/100));
  
	return (10019L-width); 
}
