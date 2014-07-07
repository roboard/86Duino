/*
  WInter.cpp - attachInterrupt() for 86Duino
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
#include "Arduino.h"
#include "mcm.h"
#include "irq.h"

#define MCPFAU_CAP_LEVEL0    (0x08L << 16)
#define MCPFAU_CAP_LEVEL1    (0x09L << 16)

static int mc = 0, md = 1;
static int mcint_offset[2] = {0, 24};
static void clear_INTSTATUS(void) {
    mc_outp(mc, 0x04, 0xffL << mcint_offset[md]); //for EX
}

static void disable_MCINT(void) {
    mc_outp(mc, 0x00, mc_inp(mc, 0x00) & ~(0xffL << mcint_offset[md]));  // disable mc interrupt
    mc_outp(MC_GENERAL, 0x38, mc_inp(MC_GENERAL, 0x38) | (1L << mc));
}

static void enable_MCINT(unsigned long used_int) {
	mc_outp(MC_GENERAL, 0x38, mc_inp(MC_GENERAL, 0x38) & ~(1L << mc));
	mc_outp(mc, 0x00, (mc_inp(mc, 0x00) & ~(0xffL<<mcint_offset[md])) | (used_int << mcint_offset[md]));
}

static void (*sifIntMode[3])(int, int, unsigned long) = {mcpfau_SetCapMode1, mcpfau_SetCapMode2, mcpfau_SetCapMode3}; 
static void (*sifSetPol[3])(int, int, unsigned long) = {mcpfau_SetPolarity1, mcpfau_SetPolarity2, mcpfau_SetPolarity3}; 
static void (*sifSetRelease[3])(int, int, unsigned long) = {mcpfau_SetFAU1RELS, mcpfau_SetFAU2RELS, mcpfau_SetFAU3RELS}; 
static void (*sifClearStat[3])(int, int) = {mcpfau_ClearFAU1STAT, mcpfau_ClearFAU2STAT, mcpfau_ClearFAU3STAT}; 
static void (*sifSetMask[3])(int, int, unsigned long) = {mcpfau_SetMask1, mcpfau_SetMask2, mcpfau_SetMask3}; 
static unsigned long (*readCapStat[3])(int, int) = {mcpfau_ReadCAPSTAT1, mcpfau_ReadCAPSTAT2, mcpfau_ReadCAPSTAT3};
static unsigned long (*readCapFIFO[3])(int, int, unsigned long*) = {mcpfau_ReadCAPFIFO1, mcpfau_ReadCAPFIFO2, mcpfau_ReadCAPFIFO3};
static volatile bool mcm_init[4] = {false, false, false, false};
static unsigned long _usedMode[4][3];
static int user_int(int irq, void* data) {
	int i, m, n, evt = 0;
	unsigned long capdata;
	
	// detect interrupt pin
	for(i=0; i<EXTERNAL_NUM_INTERRUPTS; i++)
	{
		m = i/3; // sif mc
		n = i%3; // offset (capture pin number 1/2/3)
		if(mcm_init[m] == false) {i += 2; continue;}
		if((mc_inp(m, 0x04) & ((0x20000000L)<<n)) != 0L) // detect input edge-trigger
		{
			mc_outp(m, 0x04, (0x20000000L)<<n);
			break;
		}
		
		if((mc_inp(m, 0x04) & ((0x04000000L)<<n)) != 0L) // detect input level-trigger
		{
			mc_outp(m, 0x04, (0x04000000L)<<n);
			break;
		}
	}
	
	// execute user function for the pin
	if(i < EXTERNAL_NUM_INTERRUPTS)
	{
		switch(_usedMode[m][n])
		{
		case MCPFAU_CAP_LEVEL0: case MCPFAU_CAP_LEVEL1:
			sifSetMask[n](m, MCSIF_MODULEB, MCPFAU_MASK_INACTIVE);
			sifClearStat[n](m, MCSIF_MODULEB);
			evt++;
			break;
		case MCPFAU_CAP_BOTH:
	    	while(readCapStat[n](m, MCSIF_MODULEB)!= MCENC_CAPFIFO_EMPTY)
          		if(readCapFIFO[n](m, MCSIF_MODULEB, &capdata) != MCPFAU_CAP_CAPCNT_OVERFLOW) evt++;
			break;
		case MCPFAU_CAP_1TO0:
			while(readCapStat[n](m, MCSIF_MODULEB)!= MCENC_CAPFIFO_EMPTY)
          		if(readCapFIFO[n](m, MCSIF_MODULEB, &capdata) == MCPFAU_CAP_1TO0EDGE) evt++;
          	break;
		case MCPFAU_CAP_0TO1:
			while(readCapStat[n](m, MCSIF_MODULEB)!= MCENC_CAPFIFO_EMPTY)
          		if(readCapFIFO[n](m, MCSIF_MODULEB, &capdata) == MCPFAU_CAP_0TO1EDGE) evt++;
          	break;
		}
		
		// do user's function
		for(; evt > 0; evt--)
			_userfunc[i]();
		
		// if select level-trigger, switch the MASK to "NONE" after user's function is complete.
		switch(_usedMode[m][n])
		{
		case MCPFAU_CAP_LEVEL0: case MCPFAU_CAP_LEVEL1:
			sifSetMask[n](m, MCSIF_MODULEB, MCPFAU_MASK_NONE);
			break;
		default: break;
		}
	}
	if(i == EXTERNAL_NUM_INTERRUPTS) return ISR_NONE;
	return ISR_HANDLED;
}

static uint8_t used_irq = 0xff;
static char* name = "attachInt";
static bool interrupt_init(void) {
	if(used_irq != 0xff) return true;
	
	/* this part is moved to wiring.cpp
	if(irq_Init() == false) 
    {
        printf("irq_init fail\n"); return false;
    }
    
    if(irq_Setting(GetMCIRQ(), IRQ_LEVEL_TRIGGER + IRQ_DISABLE_INTR) == false)
    {
        printf("%s\n", __FUNCTION__); return false;
    }
    */
    
    if(irq_InstallISR(GetMCIRQ(), user_int, (void*)name) == false)
	{
	    printf("irq_install fail\n"); return false;
	}
	//printf("BaseAddr = %08lxh irq = %d\n\n", mc_setbaseaddr(), GetMCIRQ());
	//Master_DX2();
	
	used_irq = GetMCIRQ();
	// Set_MCIRQ(used_irq); // moved to wiring.cpp
	return true;
}


static void mcmsif_init(void) {
    if(mcm_init[mc] == true) return;
    
    mcsif_SetInputFilter(mc, md, 20L);
    mcsif_SetSWDeadband(mc, md, 0L);
    mcsif_SetSWPOL(mc, md, MCSIF_SWPOL_REMAIN);
    mcsif_SetSamplWin(mc, md, MCSIF_SWSTART_DISABLE + MCSIF_SWEND_NOW);  // stop the current sampling window first
    mcsif_SetSamplWin(mc, md, MCSIF_SWSTART_NOW + MCSIF_SWEND_DISABLE);

	mcsif_SetMode(mc, md, MCSIF_PFAU);               // enable MC2 SIFA
    
    mcpfau_SetCapMode1(mc, md, MCPFAU_CAP_DISABLE);
    mcpfau_SetCapInterval1(mc, md, 1L);              // pin1 for FAUTRIG
    mcpfau_SetCap1INT(mc, md, 1L);
    mcpfau_SetPolarity1(mc, md, MCPFAU_POL_NORMAL);
    mcpfau_SetMask1(mc, md, MCPFAU_MASK_NONE);
    mcpfau_SetRLDTRIG1(mc, md, MCPFAU_RLDTRIG_DISABLE);
    mcpfau_SetFAU1TRIG(mc, md, MCPFAU_FAUTRIG_INPUT1);
    mcpfau_SetFAU1RELS(mc, md, MCPFAU_FAURELS_INPUT0);
	
    mcpfau_SetCapMode2(mc, md, MCPFAU_CAP_DISABLE);     // pin2 for RLDTRIG
    mcpfau_SetCapInterval2(mc, md, 1L);
    mcpfau_SetCap2INT(mc, md, 1L);
    mcpfau_SetPolarity2(mc, md, MCPFAU_POL_NORMAL);
    mcpfau_SetMask2(mc, md, MCPFAU_MASK_NONE);
    mcpfau_SetRLDTRIG2(mc, md, MCPFAU_RLDTRIG_DISABLE);
    mcpfau_SetFAU2TRIG(mc, md, MCPFAU_FAUTRIG_INPUT1);
    mcpfau_SetFAU2RELS(mc, md, MCPFAU_FAURELS_INPUT0);

    mcpfau_SetCapMode3(mc, md, MCPFAU_CAP_DISABLE);
    mcpfau_SetCapInterval3(mc, md, 1L);              // pin3 for FAUTRIG 
    mcpfau_SetCap3INT(mc, md, 1L);
    mcpfau_SetPolarity3(mc, md, MCPFAU_POL_NORMAL);
    mcpfau_SetMask3(mc, md, MCPFAU_MASK_NONE);
    mcpfau_SetRLDTRIG3(mc, md, MCPFAU_RLDTRIG_DISABLE);          
	mcpfau_SetFAU3TRIG(mc, md, MCPFAU_FAUTRIG_INPUT1);
	mcpfau_SetFAU3RELS(mc, md, MCPFAU_FAURELS_INPUT0);
	
	io_DisableINT();
	mcm_init[mc] = true;
	io_RestoreINT();
}

static void mcmsif_close(void) {
	mcsif_Disable(mc, md);
	io_DisableINT();
	mcm_init[mc] = false;
	io_RestoreINT();
}

void attachInterrupt(uint8_t interruptNum, void (*userFunc)(void), int mode) {
	int i;
	unsigned short crossbar_ioaddr;
	
	if(interruptNum >= EXTERNAL_NUM_INTERRUPTS)
	{
		printf("This interrupt%d has no one pin to use\n", interruptNum);
		return;
	}
    mc = interruptNum/3;
    md = MCSIF_MODULEB;
    
    if(_userfunc[interruptNum] != NULL) return;
	if(interrupt_init() == false) return;
	mcmsif_init();
	
    clear_INTSTATUS();
	enable_MCINT(0xfc); // SIFB FAULT INT3/2/1 + STAT3/2/1 = 6 bits
	
	crossbar_ioaddr = sb_Read16(0x64)&0xfffe;
    if (mc == 0)
		io_outpb(crossbar_ioaddr + 2, 0x01); // GPIO port2: 0A, 0B, 0C, 3A
	else if (mc == 1)
    	io_outpb(crossbar_ioaddr + 3, 0x02); // GPIO port3: 1A, 1B, 1C, 3B
	else if(mc == 2)
		io_outpb(crossbar_ioaddr, 0x03); // GPIO port0: 2A, 2B, 2C, 3C
	else if(mc == 3)
	{
		io_outpb(crossbar_ioaddr + 2, 0x01);
		io_outpb(crossbar_ioaddr + 3, 0x02);
		io_outpb(crossbar_ioaddr, 0x03);
	}
	
	mcsif_Disable(mc, md);

	io_DisableINT();
	_userfunc[interruptNum] = userFunc;
	io_RestoreINT();
	
	switch (mode) 
	{
	case LOW:
		sifSetPol[interruptNum%3](mc, md, MCPFAU_POL_INVERSE);
		sifSetMask[interruptNum%3](mc, md, MCPFAU_MASK_INACTIVE);
		sifSetRelease[interruptNum%3](mc, md, MCPFAU_FAURELS_FSTAT0);
		sifClearStat[interruptNum%3](mc, md);
		_usedMode[mc][interruptNum%3] = MCPFAU_CAP_LEVEL0;
		clear_INTSTATUS();
		break;
	case HIGH:
		sifSetMask[interruptNum%3](mc, md, MCPFAU_MASK_INACTIVE);
		sifSetRelease[interruptNum%3](mc, md, MCPFAU_FAURELS_FSTAT0);
		sifClearStat[interruptNum%3](mc, md);
		_usedMode[mc][interruptNum%3] = MCPFAU_CAP_LEVEL1;
		clear_INTSTATUS();
		break;
	case CHANGE:
		sifIntMode[interruptNum%3](mc, md, MCPFAU_CAP_BOTH);
		_usedMode[mc][interruptNum%3] = MCPFAU_CAP_BOTH;
		break;
	case FALLING:
		sifIntMode[interruptNum%3](mc, md, MCPFAU_CAP_1TO0);
		_usedMode[mc][interruptNum%3] = MCPFAU_CAP_1TO0;
		break;	
	case RISING:
	    sifIntMode[interruptNum%3](mc, md, MCPFAU_CAP_0TO1);
	    _usedMode[mc][interruptNum%3] = MCPFAU_CAP_0TO1;
	    break;
	default:
		printf("No support this mode\n");
		return;
	}
	
	// switch crossbar to MCM_SIF_PIN
	io_outpb(crossbar_ioaddr + 0x90 + pin_offset[interruptNum], 0x08);//RICH IO
	mcsif_Enable(mc, md);
	
	// If select level-trigger, switch the MASK to "NONE" after sif is enabled.
	switch (mode)
	{
	case LOW: case HIGH:
		sifSetMask[interruptNum%3](mc, md, MCPFAU_MASK_NONE);
		break;
	default: break;
	}
}

void detachInterrupt(uint8_t interruptNum) {
	int i;
	mc = interruptNum/3;
	
	if(interruptNum >= EXTERNAL_NUM_INTERRUPTS) return;
	if(_userfunc[interruptNum] == NULL) return;
	
	mcsif_Disable(mc, md);
	sifIntMode[interruptNum%3](mc, md, MCPFAU_CAP_DISABLE);
	
	io_DisableINT();
	_userfunc[interruptNum] = NULL;
	io_RestoreINT();
	
	for(i=0; i<3; i++)
		if(_userfunc[mc*3+i] != NULL) break;
	if(i == 3) mcmsif_close(); else mcsif_Enable(mc, md);
	
	for(i=0; i<EXTERNAL_NUM_INTERRUPTS; i++)
		if(_userfunc[i] != NULL) break;
	if(i == EXTERNAL_NUM_INTERRUPTS)
	{
		if(irq_UninstallISR(used_irq, (void*)name) == false)
		    printf("irq_install fail\n");
		else
			used_irq = 0xff;
	}
}

