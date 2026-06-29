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

#define DPC_MODE  (0)
#define RT_MODE   (1)
#define HT_MODE   (2)
static _HelperTask* _task = NULL;

static int mcint_offset[2] = {0, 24};
static void clear_INTSTATUS(int mc, int md, unsigned long used_int) {
    mc_outp(mc, 0x04, used_int << mcint_offset[md]);
}

static void disable_MCINT(int mc, int md, unsigned long used_int) {
    mc_outp(mc, 0x00, mc_inp(mc, 0x00) & ~(used_int << mcint_offset[md]));  // disable mc interrupt
    mc_outp(MC_GENERAL, 0x38, mc_inp(MC_GENERAL, 0x38, mc/4) | (1L << (mc%4)), mc/4);
}

static void enable_MCINT(int mc, int md, unsigned long used_int) {
	mc_outp(MC_GENERAL, 0x38, mc_inp(MC_GENERAL, 0x38, mc/4) & ~(1L << (mc%4)), mc/4);
	mc_outp(mc, 0x00, mc_inp(mc, 0x00) | (used_int << mcint_offset[md]));
}


static volatile bool mcm_init[EXTERNAL_NUM_INTERRUPTS];
static void (*sifIntMode[3])(int, int, unsigned long) = {mcpfau_SetCapMode1, mcpfau_SetCapMode2, mcpfau_SetCapMode3};
static void (*sifSetPol[3])(int, int, unsigned long) = {mcpfau_SetPolarity1, mcpfau_SetPolarity2, mcpfau_SetPolarity3};
static void (*sifSetRelease[3])(int, int, unsigned long) = {mcpfau_SetFAU1RELS, mcpfau_SetFAU2RELS, mcpfau_SetFAU3RELS};
static void (*sifClearStat[3])(int, int) = {mcpfau_ClearFAU1STAT, mcpfau_ClearFAU2STAT, mcpfau_ClearFAU3STAT};
static void (*sifSetMask[3])(int, int, unsigned long) = {mcpfau_SetMask1, mcpfau_SetMask2, mcpfau_SetMask3};
static unsigned long (*readCapStat[3])(int, int) = {mcpfau_ReadCAPSTAT1, mcpfau_ReadCAPSTAT2, mcpfau_ReadCAPSTAT3};
static unsigned long (*readCapFIFO[3])(int, int, unsigned long*) = {mcpfau_ReadCAPFIFO1, mcpfau_ReadCAPFIFO2, mcpfau_ReadCAPFIFO3};
static volatile unsigned long _usedMode[EXTERNAL_NUM_INTERRUPTS][3];
static void* DPCh = NULL; // DPC hadles
static unsigned long dpcTiggerTimes[EXTERNAL_NUM_INTERRUPTS] = {0};
static volatile int rt[EXTERNAL_NUM_INTERRUPTS] = {0}; // DPC_MODE
static volatile bool UT_flag[EXTERNAL_NUM_INTERRUPTS] = {false};
static int user_int(int irq, void* data) {
	int i, m, n, evt = 0;
	unsigned long capdata;
    
	// detect interrupt pin
	for (i=0; i<EXTERNAL_NUM_INTERRUPTS; i++)
	{
		m = PIN86[INTPINSMAP[i]].ENCMC;
		n = i%3; // offset (capture pin number 1/2/3)
		if (mcm_init[m] == false) continue;
		if ((mc_inp(m, 0x04) & ((0x20000000L)<<n)) != 0L) // detect input edge-trigger, !! only for MODULEB
		{
			mc_outp(m, 0x04, (0x20000000L)<<n);
			break;
		}

		if ((mc_inp(m, 0x04) & ((0x04000000L)<<n)) != 0L) // detect input level-trigger, !! only for MODULEB
		{
			mc_outp(m, 0x04, (0x04000000L)<<n);
			break;
		}
	}

	// execute user function for the pin
	if (i < EXTERNAL_NUM_INTERRUPTS)
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
          		if (readCapFIFO[n](m, MCSIF_MODULEB, &capdata) != MCPFAU_CAP_CAPCNT_OVERFLOW) evt++;
			break;
		case MCPFAU_CAP_1TO0:
			while(readCapStat[n](m, MCSIF_MODULEB)!= MCENC_CAPFIFO_EMPTY)
          		if (readCapFIFO[n](m, MCSIF_MODULEB, &capdata) == MCPFAU_CAP_1TO0EDGE) evt++;
          	break;
		case MCPFAU_CAP_0TO1:
			while(readCapStat[n](m, MCSIF_MODULEB)!= MCENC_CAPFIFO_EMPTY)
          		if (readCapFIFO[n](m, MCSIF_MODULEB, &capdata) == MCPFAU_CAP_0TO1EDGE) evt++;
          	break;
		}

		// do user's function
        if (rt[i] == RT_MODE)
        {
            for (; evt > 0; evt--)
                PIN86[INTPINSMAP[i]].userfunc();
        }
        else if (rt[i] == DPC_MODE)
        {
            if ((evt > 0) && (DPCh != NULL)) irq_TriggerDPC(DPCh);
            dpcTiggerTimes[i] += evt;
        }
        else if (rt[i] == HT_MODE)
        {
            if (evt > 0) UT_flag[i] = true;
        }

		// if select level-trigger, switch the MASK to "NONE" after user's function is complete.
		switch(_usedMode[m][n])
		{
		case MCPFAU_CAP_LEVEL0: case MCPFAU_CAP_LEVEL1:
			sifSetMask[n](m, MCSIF_MODULEB, MCPFAU_MASK_NONE);
			break;
		default: break;
		}
	}
	if (i > 0 && i == EXTERNAL_NUM_INTERRUPTS) return ISR_NONE;
	return ISR_HANDLED;
}

static uint8_t used_irq = 0xff;
static const char* name = "attachInt";
static bool interrupt_init(int mc) {
	if (used_irq != 0xff) return true;

    if (irq_InstallISR(GetMCIRQ(mc/4), user_int, (void*)name) == false)
	{
	    printf("irq_install fail\n"); return false;
	}

	used_irq = GetMCIRQ(mc/4);
    Set_MCIRQ(used_irq, mc/4);
	return true;
}


static void mcmsif_init(int mc, int md) {
    if (mcm_init[mc] == true) return;

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

static void mcmsif_close(int mc, int md) {
	io_DisableINT();
    mcsif_Disable(mc, md);
	mcm_init[mc] = false;
	io_RestoreINT();
}

static void dpcCallBackFunc(void* data) {
    int i;
    unsigned long t[EXTERNAL_NUM_INTERRUPTS];
    
    io_DisableINT();
    for (i=0; i<EXTERNAL_NUM_INTERRUPTS; i++) {
        if (dpcTiggerTimes[i] > 0) {
            t[i] = dpcTiggerTimes[i];
            dpcTiggerTimes[i] = 0;
        } else {
            t[i] = 0;
        }
    }
    io_RestoreINT();
    
    for (i=0; i<EXTERNAL_NUM_INTERRUPTS; i++) {
        while (t[i] > 0) {
            PIN86[INTPINSMAP[i]].userfunc();
            t[i]--;
        }
    }
}

static void UT_LOOP(void) {
    int i;
    for (i=0; i<EXTERNAL_NUM_INTERRUPTS; i++) {
        if (UT_flag[i]) {
            PIN86[INTPINSMAP[i]].userfunc();
            UT_flag[i] = false;
            helperYield();
        }
    }
}

void internalAttachInterrupt(uint8_t interruptNum, void (*userCallBackFunc)(void), int mode, int _rt) {
	int mc, md;
    
    if (interruptNum >= EXTERNAL_NUM_INTERRUPTS)
	{
		printf("This interrupt%d has no one pin to use\n", interruptNum);
		return;
	}
	
	mc = PIN86[INTPINSMAP[interruptNum]].ENCMC;
    md = PIN86[INTPINSMAP[interruptNum]].ENCMD;
    
    if (mc == NOENC || md == NOENC) return;
    
    io_DisableINT();
    if (PIN86[INTPINSMAP[interruptNum]].userfunc != NULL) {io_RestoreINT(); return;}
	if (interrupt_init(mc) == false) {io_RestoreINT(); return;}
    
	mcmsif_init(mc, md);
    clear_INTSTATUS(mc, md, 0xfc);
	enable_MCINT(mc, md, 0xfc); // SIFB FAULT INT3/2/1 + STAT3/2/1 = 6 bits
	
	mcsif_Disable(mc, md);

    PIN86[INTPINSMAP[interruptNum]].userfunc = userCallBackFunc;
    rt[interruptNum] = _rt;
    if (_rt == DPC_MODE) {
        if (DPCh == NULL) {
            DPCh = irq_AllocDPC(dpcCallBackFunc, NULL, IRQ_DPC_USEFPU);
            if (DPCh == NULL)
                printf("We get NULL retrun from irq_AllocDPC()\n");
        } // We just alloc DPC once
    } else if (_rt == HT_MODE) {
        _task = myHelper.startLoop(10 * 1024);
        if (_task == NULL) {
            printf("WInterrupt: myHelper init fail\n");
            return;
        }
        _task->registerFunc(UT_LOOP);
        _task->start();
    }
    
	switch (mode)
	{
	case LOW:
		sifSetPol[interruptNum%3](mc, md, MCPFAU_POL_INVERSE);
		sifSetMask[interruptNum%3](mc, md, MCPFAU_MASK_INACTIVE);
		sifSetRelease[interruptNum%3](mc, md, MCPFAU_FAURELS_FSTAT0);
		sifClearStat[interruptNum%3](mc, md);
		_usedMode[mc][interruptNum%3] = MCPFAU_CAP_LEVEL0;
		clear_INTSTATUS(mc, md, 0xfc);
		break;
	case HIGH:
		sifSetMask[interruptNum%3](mc, md, MCPFAU_MASK_INACTIVE);
		sifSetRelease[interruptNum%3](mc, md, MCPFAU_FAURELS_FSTAT0);
		sifClearStat[interruptNum%3](mc, md);
		_usedMode[mc][interruptNum%3] = MCPFAU_CAP_LEVEL1;
		clear_INTSTATUS(mc, md, 0xfc);
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
        io_RestoreINT();
		return;
	}

	// switch crossbar to MCM_SIF_PIN
    #if defined (__86DUINO_EX)
		io_outpb(CROSSBARBASE + 0x90 + PIN86[INTPINSMAP[interruptNum]].gpN, 0x08);//RICH IO
	#elif defined (__86DUINO_EX2)
		if (PIN86[INTPINSMAP[interruptNum]].gpN < 48)
            io_outpb(CROSSBARBASE + 0x100 + PIN86[INTPINSMAP[interruptNum]].gpN, 0x01);//RICH IO
        else
            io_outpb(CROSSBARBASE + 0xF0 + PIN86[INTPINSMAP[interruptNum]].gpN/8, 0x01);//RICH IO
	#endif

	mcsif_Enable(mc, md);

	// If select level-trigger, switch the MASK to "NONE" after sif is enabled.
	switch (mode)
	{
	case LOW: case HIGH:
		sifSetMask[interruptNum%3](mc, md, MCPFAU_MASK_NONE);
		break;
	default: break;
	}
    
    io_RestoreINT();
}

void attachInterrupt(uint8_t interruptNum, void (*userCallBackFunc)(void), int mode) {
    internalAttachInterrupt(interruptNum, userCallBackFunc, mode, DPC_MODE);
}

void attachRTInterrupt(uint8_t interruptNum, void (*userCallBackFunc)(void), int mode) {
    internalAttachInterrupt(interruptNum, userCallBackFunc, mode, RT_MODE);
}

void attachHTInterrupt(uint8_t interruptNum, void (*userCallBackFunc)(void), int mode) {
    internalAttachInterrupt(interruptNum, userCallBackFunc, mode, HT_MODE);
}

void detachInterrupt(uint8_t interruptNum) {
	int i, mc, md;
    
    if (interruptNum >= EXTERNAL_NUM_INTERRUPTS) return;
    
	mc = PIN86[INTPINSMAP[interruptNum]].ENCMC;
    md = PIN86[INTPINSMAP[interruptNum]].ENCMD;

    if (mc == NOENC || md == NOENC) return;
    
    io_DisableINT();
	if (PIN86[INTPINSMAP[interruptNum]].userfunc == NULL) {io_RestoreINT(); return;}

	mcsif_Disable(mc, md);
	sifIntMode[interruptNum%3](mc, md, MCPFAU_CAP_DISABLE);

	PIN86[INTPINSMAP[interruptNum]].userfunc = NULL;
    
	for (i=0; i<3; i++)
		if (PIN86[INTPINSMAP[mc*3+i]].userfunc != NULL) break;
	if (i == 3) {
        disable_MCINT(mc, md, 0xfc);
        mcmsif_close(mc, md);
    } else
        mcsif_Enable(mc, md);

	for (i=0; i<EXTERNAL_NUM_INTERRUPTS; i++)
		if (PIN86[INTPINSMAP[i]].userfunc != NULL) break;
	if (i == EXTERNAL_NUM_INTERRUPTS)
	{
        if (DPCh != NULL) {
            if (irq_FreeDPC(DPCh) == false)
                printf("We free the DPC handle fail\n");
            else
                DPCh = NULL;
        } else if (_task != NULL) {
            _task->kill();
            _task = NULL;
        }
        
        for (i=0; i<EXTERNAL_NUM_INTERRUPTS; i++) {
            rt[i] = DPC_MODE;
            dpcTiggerTimes[i] = 0;
        }
    
		if (irq_UninstallISR(used_irq, (void*)name) == false)
		    printf("irq_install fail\n");
		else
			used_irq = 0xff;
	}
    io_RestoreINT();
}

