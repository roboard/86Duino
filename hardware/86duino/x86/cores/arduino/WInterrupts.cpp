/*
  WInter.cpp - attachInterrupt() for 86Duino
  Copyright (c) 2013 Android Lin <acen@dmp.com.tw>. All right reserved.
  Modify by Sayter <sayter@dmp.com.tw> 1 January 2016.

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

#include "dmpcfg.h"
#include "Arduino.h"
#include "mcm.h"

#if defined (DMP_DOS_DJGPP)
#include "irq.h"
#include <stdio.h>
#endif

#define MCPFAU_CAP_LEVEL0 (0x08L << 16)
#define MCPFAU_CAP_LEVEL1 (0x09L << 16)

#if defined (DMP_LINUX)

#define INTERRUPTS 13
#if defined(__86DUINO_ZERO)
#define MAX_INTR_NUM 2
#elif defined(__86DUINO_EDUCAKE)
#define MAX_INTR_NUM 5
#else
#define MAX_INTR_NUM 11
#endif

struct interrupt {
    bool used;
    void (*callback)(void);
    uint32_t mode;
    uint32_t start;
    uint32_t timeout;
};

struct interrupt_desc {
    pthread_t thread;
    OSSPIN spinlock;
	struct interrupt intr[INTERRUPTS];
};

static interrupt_desc idc;
bool do_callback[INTERRUPTS] = {0};

#elif defined(DMP_DOS_DJGPP)

static int mc = 0, md = 1;
static int mcint_offset[2] = {0, 24};
static uint8_t used_irq = 0xff;
static char* name = "attachInt";
DMP_INLINE(void) clear_INTSTATUS(void) {
    mc_outp(mc, 0x04, 0xffL << mcint_offset[md]); //for EX
}

DMP_INLINE(void) disable_MCINT(void) {
    mc_outp(mc, 0x00, mc_inp(mc, 0x00) & ~(0xffL << mcint_offset[md]));  // disable mc interrupt
    mc_outp(MC_GENERAL, 0x38, mc_inp(MC_GENERAL, 0x38) | (1L << mc));
}

DMP_INLINE(void) enable_MCINT(unsigned long used_int) {
	mc_outp(MC_GENERAL, 0x38, mc_inp(MC_GENERAL, 0x38) & ~(1L << mc));
	mc_outp(mc, 0x00, (mc_inp(mc, 0x00) & ~(0xffL<<mcint_offset[md])) | (used_int << mcint_offset[md]));
}

#endif

static void (*sifIntMode[3])(int, int, unsigned long) = {mcpfau_SetCapMode1, mcpfau_SetCapMode2, mcpfau_SetCapMode3};
static void (*sifSetPol[3])(int, int, unsigned long) = {mcpfau_SetPolarity1, mcpfau_SetPolarity2, mcpfau_SetPolarity3};
static void (*sifSetRelease[3])(int, int, unsigned long) = {mcpfau_SetFAU1RELS, mcpfau_SetFAU2RELS, mcpfau_SetFAU3RELS};
static void (*sifClearStat[3])(int, int) = {mcpfau_ClearFAU1STAT, mcpfau_ClearFAU2STAT, mcpfau_ClearFAU3STAT};
static void (*sifSetMask[3])(int, int, unsigned long) = {mcpfau_SetMask1, mcpfau_SetMask2, mcpfau_SetMask3};
static unsigned long (*readCapStat[3])(int, int) = {mcpfau_ReadCAPSTAT1, mcpfau_ReadCAPSTAT2, mcpfau_ReadCAPSTAT3};
static unsigned long (*readCapFIFO[3])(int, int, unsigned long*) = {mcpfau_ReadCAPFIFO1, mcpfau_ReadCAPFIFO2, mcpfau_ReadCAPFIFO3};
static volatile bool mcm_init[4] = {false, false, false, false};
static unsigned long _usedMode[4][3];

#if defined (DMP_LINUX)
void *intrMain(void* pargs)
{
	unsigned long capdata;
	int32_t m, n;
    while(true)
    {
        OSSPINLOCK(idc.spinlock);
        for(int i = 0; i < INTERRUPTS; i++)
        {
			if(idc.intr[i].used == false)
				continue;
            if(i <= MAX_INTR_NUM)
            {
				m = i/3;
				n = i%3;
				lockMCMSIF();
                switch(idc.intr[i].mode)
                {
                case LOW:
                    if(digitalRead(pin_interrupt[i]) == LOW)
                        do_callback[i] = true;
                    break;
                case HIGH:
                    if(digitalRead(pin_interrupt[i]) == HIGH)
						do_callback[i] = true;
                    break;
                case CHANGE:
					if(readCapStat[n](m, MCSIF_MODULEB) != MCENC_CAPFIFO_EMPTY)
						if(readCapFIFO[n](m, MCSIF_MODULEB, &capdata) != MCPFAU_CAP_CAPCNT_OVERFLOW)
							do_callback[i] = true;
                    break;
                case FALLING:
					if(readCapStat[n](m, MCSIF_MODULEB)!= MCENC_CAPFIFO_EMPTY)
          				if(readCapFIFO[n](m, MCSIF_MODULEB, &capdata) == MCPFAU_CAP_1TO0EDGE)
                       		do_callback[i] = true;
                    break;
                case RISING:
					if(readCapStat[n](m, MCSIF_MODULEB)!= MCENC_CAPFIFO_EMPTY)
          				if(readCapFIFO[n](m, MCSIF_MODULEB, &capdata) == MCPFAU_CAP_0TO1EDGE)
                        	do_callback[i] = true;
                    break;
                default:
                    break;
                }
				unLockMCMSIF();
            }
            else
            {
                if(micros() - idc.intr[i].start > idc.intr[i].timeout)
				{
                    do_callback[i] = true;
					idc.intr[i].start = micros();
				}
            }
        }
        OSSPINUNLOCK(idc.spinlock);
		OSSPINLOCK(idc.spinlock);
		for(int i = 0; i < INTERRUPTS; i++)
		{
			if(do_callback[i] && idc.intr[i].used)
				idc.intr[i].callback();
			do_callback[i] = false;
		}
		OSSPINUNLOCK(idc.spinlock);
	}
    pthread_exit(NULL);
}
#endif

#if defined (DMP_DOS_DJGPP)
DMP_INLINE(int) user_int(int irq, void* data) {
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
#endif

DMP_INLINE(void) mcmsif_init(int32_t mc)
{
    if(mcm_init[mc] == true) return;
	mcsif_SetInputFilter(mc, MCSIF_MODULEB, 20L);
    mcsif_SetSWDeadband(mc, MCSIF_MODULEB, 0L);
    mcsif_SetSWPOL(mc, MCSIF_MODULEB, MCSIF_SWPOL_REMAIN);
    mcsif_SetSamplWin(mc, MCSIF_MODULEB, MCSIF_SWSTART_DISABLE + MCSIF_SWEND_NOW);
    mcsif_SetSamplWin(mc, MCSIF_MODULEB, MCSIF_SWSTART_NOW + MCSIF_SWEND_DISABLE);

	mcsif_SetMode(mc, MCSIF_MODULEB, MCSIF_PFAU);               // enable MC2 SIFA

    mcpfau_SetCapMode1(mc, MCSIF_MODULEB, MCPFAU_CAP_DISABLE);
    mcpfau_SetCapInterval1(mc, MCSIF_MODULEB, 1L);              // pin1 for FAUTRIG
    mcpfau_SetCap1INT(mc, MCSIF_MODULEB, 1L);
    mcpfau_SetPolarity1(mc, MCSIF_MODULEB, MCPFAU_POL_NORMAL);
    mcpfau_SetMask1(mc, MCSIF_MODULEB, MCPFAU_MASK_NONE);
    mcpfau_SetRLDTRIG1(mc, MCSIF_MODULEB, MCPFAU_RLDTRIG_DISABLE);
    mcpfau_SetFAU1TRIG(mc, MCSIF_MODULEB, MCPFAU_FAUTRIG_INPUT1);
    mcpfau_SetFAU1RELS(mc, MCSIF_MODULEB, MCPFAU_FAURELS_INPUT0);

    mcpfau_SetCapMode2(mc, MCSIF_MODULEB, MCPFAU_CAP_DISABLE);     // pin2 for RLDTRIG
    mcpfau_SetCapInterval2(mc, MCSIF_MODULEB, 1L);
    mcpfau_SetCap2INT(mc, MCSIF_MODULEB, 1L);
    mcpfau_SetPolarity2(mc, MCSIF_MODULEB, MCPFAU_POL_NORMAL);
    mcpfau_SetMask2(mc, MCSIF_MODULEB, MCPFAU_MASK_NONE);
    mcpfau_SetRLDTRIG2(mc, MCSIF_MODULEB, MCPFAU_RLDTRIG_DISABLE);
    mcpfau_SetFAU2TRIG(mc, MCSIF_MODULEB, MCPFAU_FAUTRIG_INPUT1);
    mcpfau_SetFAU2RELS(mc, MCSIF_MODULEB, MCPFAU_FAURELS_INPUT0);

    mcpfau_SetCapMode3(mc, MCSIF_MODULEB, MCPFAU_CAP_DISABLE);
    mcpfau_SetCapInterval3(mc, MCSIF_MODULEB, 1L);              // pin3 for FAUTRIG
    mcpfau_SetCap3INT(mc, MCSIF_MODULEB, 1L);
    mcpfau_SetPolarity3(mc, MCSIF_MODULEB, MCPFAU_POL_NORMAL);
    mcpfau_SetMask3(mc, MCSIF_MODULEB, MCPFAU_MASK_NONE);
    mcpfau_SetRLDTRIG3(mc, MCSIF_MODULEB, MCPFAU_RLDTRIG_DISABLE);
	mcpfau_SetFAU3TRIG(mc, MCSIF_MODULEB, MCPFAU_FAUTRIG_INPUT1);
	mcpfau_SetFAU3RELS(mc, MCSIF_MODULEB, MCPFAU_FAURELS_INPUT0);
	#if defined (DMP_DOS_DJGPP)
	io_DisableINT();
	#endif
	mcm_init[mc] = true;
	#if defined (DMP_DOS_DJGPP)
	io_RestoreINT();
	#endif
}

#if defined (DMP_LINUX)
DMP_INLINE(int) addIRQEntry(uint8_t interruptNum, void (*callback)(void), int mode, uint32_t timeout)
{
	OSSPINLOCK(idc.spinlock);

	idc.intr[interruptNum].used = true;
    idc.intr[interruptNum].callback = callback;
    idc.intr[interruptNum].mode = mode;
    idc.intr[interruptNum].start = micros();
    idc.intr[interruptNum].timeout = timeout;
	pinMode(pin_interrupt[interruptNum], INPUT);

	if(mode != LOW && mode != HIGH && timeout == 0)
	{
		uint16_t crossbar_ioaddr;
		int32_t mc = interruptNum/3;
		int32_t md = MCSIF_MODULEB;
		lockMCMSIF();
		if(mcm_init[mc] == false)
			mcmsif_init(mc);
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

		switch (mode)
		{
		case CHANGE:
			sifIntMode[interruptNum%3](mc, md, MCPFAU_CAP_BOTH);
			break;
		case FALLING:
			sifIntMode[interruptNum%3](mc, md, MCPFAU_CAP_1TO0);
			break;
		case RISING:
			sifIntMode[interruptNum%3](mc, md, MCPFAU_CAP_0TO1);
			break;
		default:
			break;
		}
		// switch crossbar to MCM_SIF_PIN
		io_outpb(crossbar_ioaddr + 0x90 + pin_offset[interruptNum], 0x08);//RICH IO
		mcsif_Enable(mc, md);
		unLockMCMSIF();
	}

	OSSPINUNLOCK(idc.spinlock);
}
#endif

DMPAPI(int) interrupt_init(void)
{
	#if defined (DMP_LINUX)
    OSSPININIT(idc.spinlock);
    for(int i = 0; i < INTERRUPTS; i++)
		idc.intr[i].used = false;
    int err = pthread_create(&idc.thread, NULL, intrMain, NULL);
	if(err != 0)
		printf("failed to create the thread\n");
	return 0;
	#elif defined (DMP_DOS_DJGPP)
	if(used_irq != 0xff) return 0;

    if(irq_InstallISR(GetMCIRQ(), user_int, (void*)name) == false)
	{
	    printf("irq_install fail\n"); return -1;
	}

	used_irq = GetMCIRQ();
	return 0;
	#endif
}

DMPAPI(void) attachInterrupt(uint8_t interruptNum, void (*userFunc)(void), int mode)
{
	#if defined (DMP_LINUX)
    if( mode < 0 || mode > 4 )
    {
        printf("Not support this mode\n");
        return;
    }

    if( interruptNum > MAX_INTR_NUM )
    {
        printf("Not support this interruptNum\n");
        return;
    }

	if( idc.intr[interruptNum].used == true )
	{
		printf("This pin was attached before\n");
		return;
	}

    addIRQEntry(interruptNum, userFunc, mode, 0);
	#elif defined (DMP_DOS_DJGPP)
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
	if(interrupt_init() == -1) return;
	mcmsif_init(mc);

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
	#endif
}

DMP_INLINE(void) mcmsif_close(int32_t mc)
{
	mcsif_Disable(mc, MCSIF_MODULEB);
	#if defined (DMP_DOS_DJGPP)
	io_DisableINT();
	#endif
	mcm_init[mc] = false;
	#if defined (DMP_DOS_DJGPP)
	io_RestoreINT();
	#endif
}

DMPAPI(void) detachInterrupt(uint8_t interruptNum)
{
	#if defined (DMP_LINUX)
	if(interruptNum > MAX_INTR_NUM + 1)
		return;

	OSSPINLOCK(idc.spinlock);
	idc.intr[interruptNum].used = false;
	uint8_t mc = interruptNum/3;
	lockMCMSIF();
	if(mc < 4 && !(idc.intr[mc*3].used) && !(idc.intr[mc*3 + 1].used) && !(idc.intr[mc*3 + 2].used))
		mcmsif_close(mc);
	unLockMCMSIF();
	OSSPINUNLOCK(idc.spinlock);
	#elif defined (DMP_DOS_DJGPP)
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
	if(i == 3) mcmsif_close(mc); else mcsif_Enable(mc, md);

	for(i=0; i<EXTERNAL_NUM_INTERRUPTS; i++)
		if(_userfunc[i] != NULL) break;
	if(i == EXTERNAL_NUM_INTERRUPTS)
	{
		if(irq_UninstallISR(used_irq, (void*)name) == false)
		    printf("irq_install fail\n");
		else
			used_irq = 0xff;
	}
	#endif
}

#if defined (DMP_LINUX)
DMPAPI(void) attachTimerInterrupt(uint8_t interruptNum, void (*callback)(void), uint32_t microseconds)
{
    if(interruptNum != 12)
        return;
    addIRQEntry(interruptNum, callback, 0, microseconds);
}

DMPAPI(void) detachTimerInterrupt(void)
{
	detachInterrupt(12);
}
#endif
