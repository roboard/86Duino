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
#include "mcm.h"
#include "irq.h"
#include "TimerWDT.h"

#define SYSTEM_RESET    (0)
#define INTERRUPT       (1)

TimerWatchdogTimer TimerWDT;

static bool timerWDTEnable = false;
static bool timerWDTInit = false;
static int wdt_mode;
static bool timerWDTIntEnable = false;
static char* isrname_wdt = "TimerWDT";

static struct wdt_status {
	unsigned char ctrl_reg;   // 0xA8
    unsigned char sigsel_reg; // 0xA9
    unsigned char count0_reg; // 0xAA
    unsigned char count1_reg; // 0xAB
    unsigned char count2_reg; // 0xAC
    unsigned char stat_reg;   // 0xAD

    unsigned char reload_reg; // 0xAE
} WDT_t;

void wdt_settimer(unsigned long usec) {
	unsigned long ival;
	double dval;
	dval = ((double)usec) / 30.5;
	ival = (unsigned long)dval;

	if(ival > 0x00ffffffL) ival = 0x00ffffffL;

	io_outpb(0xac, (ival >> 16) & 0x00ff);
	io_outpb(0xab, (ival >> 8) & 0x00ff);
	io_outpb(0xaa, ival & 0x00ff);
}

void _wdt_enable(void) {
    unsigned char val;
    if(timerWDTEnable == true) return;
	val = io_inpb(0xa8);
	io_outpb(0xa8, val | 0x40);  // reset bit 6 to disable WDT1
	timerWDTEnable = true;
}

void _wdt_disable(void) {
    unsigned char val;
    if(timerWDTEnable == false) return;
	val = io_inpb(0xa8);
	io_outpb(0xa8, val & (~0x40));  // reset bit 6 to disable WDT1
	timerWDTEnable = false;
}

#define WDTIRQ    (7)
TimerWatchdogTimer::TimerWatchdogTimer(void) {
	isrCallback = NULL;
	timerWDTEnable = false;
}

TimerWatchdogTimer::~TimerWatchdogTimer(void) {
	if(timerWDTInit == true && timerWDTIntEnable == true)
	    irq_UninstallISR(WDTIRQ, (void*)isrname_wdt);

	if(timerWDTInit == true)
	{
        _wdt_disable();

		io_outpb(0xA8, WDT_t.ctrl_reg);
		io_outpb(0xA9, WDT_t.sigsel_reg);
		io_outpb(0xAA, WDT_t.count0_reg);
		io_outpb(0xAB, WDT_t.count1_reg);
		io_outpb(0xAC, WDT_t.count2_reg);
		io_outpb(0xAD, WDT_t.stat_reg);
		io_outpb(0xAE, WDT_t.reload_reg);
	}
}


static int timerwdt_isr_handler(int irq, void* data) {
	unsigned char val;
	if((io_inpb(0xad) & 0x80) == 0) return ISR_NONE;

	_wdt_disable();
	if(TimerWDT.isrCallback != NULL) TimerWDT.isrCallback();
    _wdt_enable();

    return ISR_HANDLED;
}


void TimerWatchdogTimer::initialize(long microseconds, bool type) {
	if(timerWDTInit == true) return;

    // restore current WDT1 register
    WDT_t.ctrl_reg   = io_inpb(0xA8);
    WDT_t.sigsel_reg = io_inpb(0xA9);
    WDT_t.count0_reg = io_inpb(0xAA);
    WDT_t.count1_reg = io_inpb(0xAB);
    WDT_t.count2_reg = io_inpb(0xAC);
    WDT_t.stat_reg   = io_inpb(0xAD);
    WDT_t.reload_reg = io_inpb(0xAE);

    io_outpb(0xad, 0x80); // clear timeout event bit

	// set reset signal mode
	if(type == true)
	{
		io_outpb(0xa9, 0x0D << 4); // system reset
        wdt_mode = SYSTEM_RESET;
	}
	else
	{
		io_outpb(0xa9, 0x05 << 4); // use IRQ7
        wdt_mode = INTERRUPT;
	}

	// set time period
    wdt_settimer(microseconds);

	if(type == true) _wdt_enable();

    timerWDTInit = true;
}

void TimerWatchdogTimer::attachInterrupt(void (*isr)(void), long microseconds) {
    unsigned char val;
    
    if(timerWDTInit == false || isr == NULL || wdt_mode == SYSTEM_RESET) return;
    
    if(timerWDTIntEnable == false)
	{
		if(irq_Setting(WDTIRQ, IRQ_LEVEL_TRIGGER) == false)
	    {
	        printf("WDT IRQ Setting fail\n"); return;
	    }
		if(irq_InstallISR(WDTIRQ, timerwdt_isr_handler, isrname_wdt) == false)
		{
		    printf("irq_install fail\n"); return;
		}
        timerWDTIntEnable = true;
	}

    io_DisableINT();
    isrCallback = isr;
    io_RestoreINT();

    if(microseconds > 0) setPeriod(microseconds);

    _wdt_enable();
}

void TimerWatchdogTimer::detachInterrupt() {
    if(timerWDTInit == false || timerWDTIntEnable == false) return;
    
    irq_UninstallISR(WDTIRQ, (void*)isrname_wdt);
    timerWDTIntEnable = false;
	// No close WDT
}

void TimerWatchdogTimer::setPeriod(long microseconds) {
	if(timerWDTInit == false || microseconds <= 0) return;
	if(timerWDTEnable == true)
    {
		_wdt_disable();
		wdt_settimer(microseconds);
	    _wdt_enable();
    }
	else
		wdt_settimer(microseconds);
}

void TimerWatchdogTimer::reset(void) {
    if(timerWDTEnable == false) return;
	io_outpb(0xae, 0xff);  // WDT1 reload register
}

void TimerWatchdogTimer::stop(void) {
    if(timerWDTInit == false) return;
	detachInterrupt();
	_wdt_disable();

	io_outpb(0xA8, WDT_t.ctrl_reg);
	io_outpb(0xA9, WDT_t.sigsel_reg);
	io_outpb(0xAA, WDT_t.count0_reg);
	io_outpb(0xAB, WDT_t.count1_reg);
	io_outpb(0xAC, WDT_t.count2_reg);
	io_outpb(0xAD, WDT_t.stat_reg);
	io_outpb(0xAE, WDT_t.reload_reg);

    timerWDTInit = false;
}

bool TimerWatchdogTimer::isResetByWDT(void) {
    return ((io_inpb(0xad) & 0x80) == 0) ? (false) : (true);
}

#endif