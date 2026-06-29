/*
  MsTimer2.cpp - DM&P Vortex86 MsTimer2 library
  Copyright (c) 2014 Android Lin <acen@dmp.com.tw>. All right reserved.

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

#include <MsTimer2.h>
#include <stdio.h>
#include "io.h"
#include "irq.h"
#include "mcm.h"
#include "Arduino.h"

#define DPC_MODE  (0)
#define RT_MODE   (1)
#define HT_MODE   (2)
static _HelperTask* _task = NULL;

void (*MsTimer2::func)();

#if defined (__86DUINO_ZERO) || defined (__86DUINO_ONE) || defined (__86DUINO_EDUCAKE) || defined (__86DUINO_AI)
	#define TIMER_PIN    (32) // It is a PWM pin (like analogWrite), but it is used as a timer here.
    static int mc = 3;
	static int md = 1;
#elif defined (__86DUINO_DUO)
	static int mc = 7;
	static int md = 1;
#elif defined (__86DUINO_QEC) || defined (__86DUINO_QEC_M2) || defined (__86DUINO_QEC_M02) || defined (__86DUINO_QEC_7) || defined (__86DUINO_QEC9) || defined (__86DUINO_QEC_15)
    static int mc = 11;
	static int md = 1;
#endif
/*****************/
/*   Interrupt   */                                                                       
/*****************/
static int mcint_offset[3] = {0, 8, 16};
static void clear_INTSTATUS(unsigned long used_int) {
    mc_outp(mc, 0x04, used_int << mcint_offset[md]);
}

static void disable_MCINT(unsigned long used_int) {
    mc_outp(mc, 0x00, mc_inp(mc, 0x00) & ~(used_int << mcint_offset[md]));  // disable mc interrupt
    mc_outp(MC_GENERAL, 0x38, mc_inp(MC_GENERAL, 0x38, mc/4) | (1L << (mc%4)), mc/4);
}

static void enable_MCINT(unsigned long used_int) {
	mc_outp(MC_GENERAL, 0x38, mc_inp(MC_GENERAL, 0x38, mc/4) & ~(1L << (mc%4)), mc/4);
	mc_outp(mc, 0x00, mc_inp(mc, 0x00) | (used_int << mcint_offset[md]));
}


// TimerOne ISR
static char* isrname_one = "TimerTwo";
static void* DPCh = NULL; // DPC hadles
static volatile int rt = DPC_MODE;
static volatile bool UT_flag = false;
static int timer2_isr_handler(int irq, void* data) {
    if((mc_inp(mc, 0x04) & (PULSE_END_INT << mcint_offset[md])) == 0) return ISR_NONE;
    
	mc_outp(mc, 0x04, (PULSE_END_INT << mcint_offset[md]));
    
    if (rt == RT_MODE) {
        if (MsTimer2::func != NULL) MsTimer2::func();
    } else if (rt == DPC_MODE){
        if (DPCh != NULL) irq_TriggerDPC(DPCh);
    } else if (rt == HT_MODE) {
        UT_flag = true;
    }

    return ISR_HANDLED;
}

static void dpcCallBackFunc(void* data) {
    if (MsTimer2::func != NULL) MsTimer2::func();
}

static bool init_mc_irq(void) {
	static bool timerTwoInitInt = false;
	if(timerTwoInitInt == false)
	{	
		if(irq_InstallISR(GetMCIRQ(mc/4), timer2_isr_handler, isrname_one) == false)
		{
		    printf("irq_install fail\n"); return false;
		}
		timerTwoInitInt = true;
	}
#if defined (__86DUINO_ZERO) || defined (__86DUINO_ONE) || defined (__86DUINO_EDUCAKE) || defined (__86DUINO_AI)
	if(timer1_pin32_isUsed == false)
	{
		pinMode(TIMER_PIN, INPUT);
		digitalWrite(TIMER_PIN, LOW);
	}
#endif
    return true;
}

static void pwmInit(void) {
	mcpwm_ReloadPWM(mc, md, MCPWM_RELOAD_CANCEL);
	mcpwm_SetOutMask(mc, md, MCPWM_HMASK_NONE + MCPWM_LMASK_NONE);
	mcpwm_SetOutPolarity(mc, md, MCPWM_HPOL_NORMAL + MCPWM_LPOL_NORMAL);
	mcpwm_SetDeadband(mc, md, 0L);
	mcpwm_ReloadOUT_Unsafe(mc, md, MCPWM_RELOAD_NOW);
	
	mcpwm_SetWaveform(mc, md, MCPWM_EDGE_A0I1);
	mcpwm_SetSamplCycle(mc, md, 1999L); // sample cycle: 20ms
}

static void UT_LOOP(void) {
    if (UT_flag) {
        if (MsTimer2::func != NULL) MsTimer2::func();
        UT_flag = false;
        helperYield();
    }
}

void MsTimer2::set(unsigned long ms, void (*isr)(void), int _rt) {
	int i;
	
	if(isr == NULL) return;
	mcpwm_Disable(mc, md);
	
	pwmInit();
	disable_MCINT(PULSE_END_INT);
    
	clear_INTSTATUS(PULSE_END_INT);
	if(init_mc_irq() == false)
	{
		printf("Init MC IRQ fail\n");
		return;
    }
    //enable_MCINT(PULSE_END_INT);
    
	io_DisableINT();
	func = isr; // register the user's callback with the real ISR
    rt = _rt;
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
	io_RestoreINT();
	
	if (ms == 0L)
		mcpwm_SetWidth(mc, md, (1000L*SYSCLK)-1L, 1L);
	else
		mcpwm_SetWidth(mc, md, (ms*1000L*SYSCLK)-1L, 1L);
	mcpwm_ReloadPWM(mc, md, MCPWM_RELOAD_PEREND);	
	mcpwm_Enable(mc, md);
}

void MsTimer2::set(unsigned long ms, void (*isr)(void)) {
    set(ms, isr, DPC_MODE);
}

void MsTimer2::RTSet(unsigned long ms, void (*isr)(void)) {
    set(ms, isr, RT_MODE);
}

void MsTimer2::HTSet(unsigned long ms, void (*isr)(void)) {
    set(ms, isr, HT_MODE);
}

// enable interrupt
void MsTimer2::start() {
	clear_INTSTATUS(PULSE_END_INT);
	enable_MCINT(PULSE_END_INT);
}

// disable interrupt
void MsTimer2::stop() {
    disable_MCINT(PULSE_END_INT);
}

