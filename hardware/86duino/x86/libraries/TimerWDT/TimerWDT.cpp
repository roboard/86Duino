/*
  TimerWDT.cpp - DM&P Vortex86 TimerWDT library
  Copyright (c) 2015 Android Lin <acen@dmp.com.tw>. All right reserved.

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

  (If you need a commercial license, please contact soc@dmp.com.tw 
   to get more information.)
*/

#ifndef TIMERWDT_cpp
#define TIMERWDT_cpp

#include <stdio.h>
#include "io.h"
#include "irq.h"
#include "Arduino.h"
#include "TimerWDT.h"
#include "global.h"

#define SYSTEM_RESET    (0)
#define INTERRUPT       (1)

#define DPC_MODE  (0)
#define RT_MODE   (1)
#define HT_MODE   (2)
static _HelperTask* _task = NULL;
static bool usingFPU_INT = false;
static bool timerWDTEnable[2] = {false, false};

static struct wdt_status {
	unsigned char _ctrl;   
    unsigned char _signal; 
    unsigned char _count0; 
    unsigned char _count1; 
    unsigned char _count2; 
    unsigned char _stat;   

    unsigned char _reload; 
} WDT_t[2];

static struct wdt_registers {
	unsigned char ctrl_reg;   
    unsigned char signal_reg; 
    unsigned char count0_reg; 
    unsigned char count1_reg; 
    unsigned char count2_reg; 
    unsigned char stat_reg;   

    unsigned char reload_reg; 
} WDT_r[2];

void unlockWDT0(void) {
    io_outpb(0x22, 0x13);
    io_outpb(0x23, 0xC5);
}

void lockWDT0(void) {
    io_outpb(0x22, 0x13);
    io_outpb(0x23, 0x00);
}

unsigned char readWDT0(unsigned char index) {
    io_outpb(0x22, index);
    return io_inpb(0x23);
}

void writeWDT0(unsigned char index, unsigned char data) {
    io_outpb(0x22, index);
    io_outpb(0x23, data);
}

void wdt_settimer(int wdtn, unsigned long usec) {
	unsigned long ival;
	double dval;
	dval = ((double)usec) / 30.5;
	ival = (unsigned long)dval;

	if (ival > 0x00ffffffL) ival = 0x00ffffffL;

    if (wdtn == 1) 
    {
        io_outpb(WDT_r[1].count2_reg, (ival >> 16) & 0x00ff);
        io_outpb(WDT_r[1].count1_reg, (ival >> 8) & 0x00ff);
        io_outpb(WDT_r[1].count0_reg, ival & 0x00ff);
    }
    else
    {
        unlockWDT0();
        writeWDT0(WDT_r[0].count2_reg, (ival >> 16) & 0x00ff);
        writeWDT0(WDT_r[0].count1_reg, (ival >> 8) & 0x00ff);
        writeWDT0(WDT_r[0].count0_reg, ival & 0x00ff);
        lockWDT0();
    }
}

void _wdt_enable(int wdtn) {
    unsigned char val;
    if (timerWDTEnable[wdtn] == true) return;
    if (wdtn == 1)
    {
        val = io_inpb(WDT_r[1].ctrl_reg);
        io_outpb(WDT_r[1].ctrl_reg, val | 0x40);  // set bit 6 to enable WDT1
	}
    else
    {
        unlockWDT0();
        val = readWDT0(WDT_r[0].ctrl_reg);
        writeWDT0(WDT_r[0].ctrl_reg, val | 0x40);  // set bit 6 to enable WDT0
        lockWDT0();
    }
    timerWDTEnable[wdtn] = true;
}

void _wdt_disable(int wdtn) {
    unsigned char val;
    if (timerWDTEnable[wdtn] == false) return;
    if (wdtn == 1)
    {
        val = io_inpb(WDT_r[1].ctrl_reg);
        io_outpb(WDT_r[1].ctrl_reg, val & (~0x40));  // reset bit 6 to disable WDT1
    }
    else
    {
        unlockWDT0();
        val = readWDT0(WDT_r[0].ctrl_reg);
        writeWDT0(WDT_r[0].ctrl_reg, val & (~0x40));  // reset bit 6 to disable WDT0
        lockWDT0();
    }
	timerWDTEnable[wdtn] = false;
}



void TimerWatchdogTimer::save_WDT_registers() {
    if (selectedWDT == 1)
    {
        WDT_t[1]._ctrl   = io_inpb(WDT_r[1].ctrl_reg);
        WDT_t[1]._signal = io_inpb(WDT_r[1].signal_reg);
        WDT_t[1]._count0 = io_inpb(WDT_r[1].count0_reg);
        WDT_t[1]._count1 = io_inpb(WDT_r[1].count1_reg);
        WDT_t[1]._count2 = io_inpb(WDT_r[1].count2_reg);
        WDT_t[1]._stat   = io_inpb(WDT_r[1].stat_reg);
        WDT_t[1]._reload = io_inpb(WDT_r[1].reload_reg);
    }
    else
    {
        unlockWDT0();
        WDT_t[0]._ctrl   = readWDT0(WDT_r[0].ctrl_reg);
        WDT_t[0]._signal = readWDT0(WDT_r[0].signal_reg);
        WDT_t[0]._count0 = readWDT0(WDT_r[0].count0_reg);
        WDT_t[0]._count1 = readWDT0(WDT_r[0].count1_reg);
        WDT_t[0]._count2 = readWDT0(WDT_r[0].count2_reg);
        WDT_t[0]._stat   = readWDT0(WDT_r[0].stat_reg);
        WDT_t[0]._reload = readWDT0(WDT_r[0].reload_reg);
        lockWDT0();
    }
}

void TimerWatchdogTimer::restore_WDT_registers() {
    if (selectedWDT == 1)
    {
        io_outpb(WDT_r[1].ctrl_reg,   WDT_t[1]._ctrl);
        io_outpb(WDT_r[1].signal_reg, WDT_t[1]._signal);
        io_outpb(WDT_r[1].count0_reg, WDT_t[1]._count0);
        io_outpb(WDT_r[1].count1_reg, WDT_t[1]._count1);
        io_outpb(WDT_r[1].count2_reg, WDT_t[1]._count2);
        io_outpb(WDT_r[1].stat_reg,   WDT_t[1]._stat);
        io_outpb(WDT_r[1].reload_reg, WDT_t[1]._reload);
    }
    else
    {
        unlockWDT0();
        writeWDT0(WDT_r[0].ctrl_reg,   WDT_t[0]._ctrl);
        writeWDT0(WDT_r[0].signal_reg, WDT_t[0]._signal);
        writeWDT0(WDT_r[0].count0_reg, WDT_t[0]._count0);
        writeWDT0(WDT_r[0].count1_reg, WDT_t[0]._count1);
        writeWDT0(WDT_r[0].count2_reg, WDT_t[0]._count2);
        writeWDT0(WDT_r[0].stat_reg,   WDT_t[0]._stat);
        writeWDT0(WDT_r[0].reload_reg, WDT_t[0]._reload);
        lockWDT0();
    }
}

#define WDTIRQ    (7)
TimerWatchdogTimer::TimerWatchdogTimer(int n) {
    selectedWDT = n;
    if (selectedWDT == 0)
    {
        WDT_r[0].ctrl_reg   = 0x37;
        WDT_r[0].signal_reg = 0x38;
        WDT_r[0].count0_reg = 0x39;
        WDT_r[0].count1_reg = 0x3A;
        WDT_r[0].count2_reg = 0x3B;
        WDT_r[0].stat_reg   = 0x3C;
        WDT_r[0].reload_reg = 0x65;
    }
    else // watchdog 1
    {
        WDT_r[1].ctrl_reg   = 0xA8;
        WDT_r[1].signal_reg = 0xA9;
        WDT_r[1].count0_reg = 0xAA;
        WDT_r[1].count1_reg = 0xAB;
        WDT_r[1].count2_reg = 0xAC;
        WDT_r[1].stat_reg   = 0xAD;
        WDT_r[1].reload_reg = 0xAE;
    }
    
	isrCallback = NULL;
	timerWDTEnable[selectedWDT] = false;
    timerWDTInit = false;
}

TimerWatchdogTimer::~TimerWatchdogTimer(void) {
	if (timerWDTInit == true && timerWDTIntEnable == true)
	    irq_UninstallISR(WDTIRQ, (void*)&selectedWDT);

	if (timerWDTInit == true)
	{
        _wdt_disable(selectedWDT);

		restore_WDT_registers();
	}
}

static void* DPCh = NULL; // DPC hadles
static volatile int rt[2] = {DPC_MODE, DPC_MODE};
static volatile bool UT_flag = false;
static int timerwdt_isr_handler(int irq, void* data) {
	unsigned char val;
    int wdtn = *(int*) data;
    
	if ((wdtn == 1) && ((io_inpb(WDT_r[1].stat_reg) & 0x80) == 0)) return ISR_NONE;
    if (wdtn == 0) {
        unlockWDT0();
        if ((readWDT0(WDT_r[0].stat_reg) & 0x80) == 0) {
            lockWDT0();
            return ISR_NONE;
        }
        lockWDT0();
    }
    
	_wdt_disable(wdtn);
    if (rt[wdtn] == RT_MODE) {
        #if defined (__86DUINO_QEC_M2) || defined (__86DUINO_QEC_M02) || defined (__86DUINO_QEC) || defined (__86DUINO_QEC_7) || defined (__86DUINO_QEC_9) || defined (__86DUINO_QEC_15)
        if (TimerWDT1.isrCallback != NULL) TimerWDT1.isrCallback();
        #endif
        if (TimerWDT.isrCallback != NULL) TimerWDT.isrCallback();
    } else if (rt[wdtn] == DPC_MODE){
        if (DPCh != NULL) irq_TriggerDPC(DPCh);
    } else if (rt[wdtn] == HT_MODE) {
        UT_flag = true;
    }
    _wdt_enable(wdtn);

    return ISR_HANDLED;
}

static void dpcCallBackFunc(void* data) {
    #if defined (__86DUINO_QEC_M2) || defined (__86DUINO_QEC_M02) || defined (__86DUINO_QEC) || defined (__86DUINO_QEC_7) || defined (__86DUINO_QEC_9) || defined (__86DUINO_QEC_15)
    if (TimerWDT1.isrCallback != NULL) TimerWDT1.isrCallback();
    #endif
    if (TimerWDT.isrCallback != NULL) TimerWDT.isrCallback();
}

void TimerWatchdogTimer::initialize(long microseconds, bool type) {
	int n = selectedWDT;
    if (selectedWDT == 0 && _G_WDT0 == NOTHING) return;
    if (selectedWDT == 1 && _G_WDT1 == NOTHING) return;
    if (timerWDTInit == true) return;
    
    // save current WDT1 register
    save_WDT_registers();
    
    if (WDT_t[n]._ctrl == 0xFF && WDT_t[n]._signal == 0xFF && WDT_t[n]._count0 == 0xFF &&
        WDT_t[n]._count1 == 0xFF && WDT_t[n]._count2 == 0xFF && WDT_t[n]._stat == 0xFF && WDT_t[n]._reload == 0xFF)
    {
        // This WDT1 device is not available.
        return;
    }

    if (selectedWDT == 1)
        io_outpb(WDT_r[n].stat_reg, 0x80); // clear timeout event bit
    else
    {
        unlockWDT0();
        writeWDT0(WDT_r[n].stat_reg, 0x80);
        lockWDT0();
    }
    
	// set reset signal mode
	if (type == true)
	{
        if (selectedWDT == 1)
            io_outpb(WDT_r[n].signal_reg, 0x0D << 4); // system reset
        else
        {
            unlockWDT0();
            writeWDT0(WDT_r[n].signal_reg, 0x0D << 4);
            lockWDT0();
        }
        wdt_mode = SYSTEM_RESET;
	}
	else
	{
		if (selectedWDT == 1)
            io_outpb(WDT_r[n].signal_reg, 0x05 << 4); // use IRQ7
        else
        {
            unlockWDT0();
            writeWDT0(WDT_r[n].signal_reg, 0x05 << 4);
            lockWDT0();
        }
        wdt_mode = INTERRUPT;
	}

	// set time period
    wdt_settimer(selectedWDT, microseconds);

	if (type == true) _wdt_enable(selectedWDT);

    timerWDTInit = true;
}

static void UT_LOOP(void) {
    if (UT_flag) {
        if (TimerWDT.isrCallback != NULL) TimerWDT.isrCallback();
        #if defined (__86DUINO_QEC_M2) || defined (__86DUINO_QEC_M02) || defined (__86DUINO_QEC) || defined (__86DUINO_QEC_7) || defined (__86DUINO_QEC_9) || defined (__86DUINO_QEC_15)
        if (TimerWDT1.isrCallback != NULL) TimerWDT1.isrCallback();
        #endif
        UT_flag = false;
        helperYield();
    }
}

void TimerWatchdogTimer::attachInterrupt(void (*isr)(void), long microseconds, int _rt) {
    unsigned char val;
    
    if (timerWDTInit == false || isr == NULL || wdt_mode == SYSTEM_RESET) return;
    
    if (timerWDTIntEnable == false)
	{
		if (irq_Setting(WDTIRQ, IRQ_LEVEL_TRIGGER) == false)
	    {
	        printf("WDT IRQ Setting fail\n"); return;
	    }
        // if (usingFPU) {irq_Setting_RequestFPU(WDTIRQ); usingFPU_INT = true;}
		if (irq_InstallISR(WDTIRQ, timerwdt_isr_handler, (void*)&selectedWDT) == false)
		{
		    printf("irq_install fail\n"); return;
		}
        timerWDTIntEnable = true;
	}

    io_DisableINT();
    isrCallback = isr;
    rt[selectedWDT] = _rt;
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

    if (microseconds > 0) setPeriod(microseconds);

    _wdt_enable(selectedWDT);
}

void TimerWatchdogTimer::attachInterrupt(void (*isr)(void), long microseconds) {
    attachInterrupt(isr, microseconds, DPC_MODE);
}

void TimerWatchdogTimer::attachRTInterrupt(void (*isr)(void), long microseconds) {
    attachInterrupt(isr, microseconds, RT_MODE);
}

void TimerWatchdogTimer::attachHTInterrupt(void (*isr)(void), long microseconds) {
    attachInterrupt(isr, microseconds, HT_MODE);
}

void TimerWatchdogTimer::detachInterrupt() {
    if (timerWDTInit == false || timerWDTIntEnable == false) return;
    
    irq_UninstallISR(WDTIRQ, NULL);
    // if (usingFPU_INT) {irq_Setting_QuitFPU(WDTIRQ); usingFPU_INT = false;}
    if (DPCh != NULL) {
        if (irq_FreeDPC(DPCh) == false)
            printf("We free the DPC handle fail\n");
        else
            DPCh = NULL;
    } else if (_task != NULL) {
        _task->kill();
        _task = NULL;
    }
    
    rt[selectedWDT] = DPC_MODE;
    timerWDTIntEnable = false;
	// No close WDT
}

void TimerWatchdogTimer::setPeriod(long microseconds) {
	if (timerWDTInit == false || microseconds <= 0) return;
	if (timerWDTEnable[selectedWDT] == true)
    {
		_wdt_disable(selectedWDT);
		wdt_settimer(selectedWDT, microseconds);
	    _wdt_enable(selectedWDT);
    }
	else
		wdt_settimer(selectedWDT, microseconds);
}

void TimerWatchdogTimer::reset(void) {
    if (timerWDTEnable[selectedWDT] == false) return;
    if (selectedWDT == 1) {
        // io_outpb(WDT_r[1].stat_reg, 0x40); // no effect, because this bit 6 is empty!! it is diffrent form WDT0. 
        io_outpb(WDT_r[1].reload_reg, 0xff); // WDT1 reload register
    } else
    {
        io_outpb(WDT_r[0].reload_reg, 0xff); // do not need unlockWDT0 and lockWDT0
        // unlockWDT0();
        // writeWDT0(WDT_r[0].reload_reg, 0xff); // Under "System Reset" mode, it is no effect!!! don't know why
        // writeWDT0(WDT_r[0].stat_reg, 0x40); // Only use this method to reload OK!!
        // lockWDT0();
    }
}

void TimerWatchdogTimer::stop(void) {
    if (timerWDTInit == false) return;
	detachInterrupt();
	_wdt_disable(selectedWDT);

	restore_WDT_registers();

    timerWDTInit = false;
}

bool TimerWatchdogTimer::isResetByWDT(void) {
    unsigned char tmp;
    if (selectedWDT == 1)
    {
        tmp = io_inpb(WDT_r[1].stat_reg);
    }
    else
    {
        unlockWDT0();
        tmp = readWDT0(WDT_r[0].stat_reg);
        lockWDT0();
    }
    return ((tmp & 0x80) == 0) ? (false) : (true);
}

#if defined (__86DUINO_QEC_M2) || defined (__86DUINO_QEC_M02) || defined (__86DUINO_QEC) || defined (__86DUINO_QEC_7) || defined (__86DUINO_QEC_9) || defined (__86DUINO_QEC_15)
TimerWatchdogTimer TimerWDT(0);
TimerWatchdogTimer TimerWDT1(1);
#else
TimerWatchdogTimer TimerWDT(1); // 0: WDT0      1: WDT1
#endif

#endif // #ifndef TIMERWDT_cpp
