/*
  TimerOne.cpp - DM&P Vortex86 TimerOne library
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

  (If you need a commercial license, please contact soc@dmp.com.tw 
   to get more information.)
*/

#ifndef TIMERONE_cpp
#define TIMERONE_cpp

#include <stdio.h>
#include "io.h"
#include "mcm.h"
#include "irq.h"
#include "TimerOne.h"

TimerOne Timer1;
TimerRealTimeClock TimerRTC;

static int mc = 3, md = 2;

/*****************/
/*   Interrupt   */                                                                       
/*****************/
static int mcint_offset[3] = {0, 8, 16};
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


// TimerOne ISR
static char* isrname_one = "TimerOne";
static int timer1_isr_handler(int irq, void* data) {
    if((mc_inp(mc, 0x04) & (PULSE_END_INT << mcint_offset[md])) == 0) return ISR_NONE;
    
	mc_outp(mc, 0x04, (PULSE_END_INT << mcint_offset[md]));
    
    if(Timer1.isrCallback != NULL) Timer1.isrCallback();
     
    return ISR_HANDLED;
}



TimerOne::TimerOne() {
	isrCallback = NULL;
	timer1Enable = false;
}


static bool init_mc_irq(void) {
	static bool timerOneInitInt = false;
	if(timerOneInitInt == false)
	{	
		if(irq_InstallISR(GetMCIRQ(), timer1_isr_handler, isrname_one) == false)
		{
		    printf("irq_install fail\n"); return false;
		}
		timerOneInitInt = true;
	}
    return true;
}

static void pwmInit(int mcn, int mdn) {
	mcpwm_ReloadPWM(mcn, mdn, MCPWM_RELOAD_CANCEL);
	mcpwm_SetOutMask(mcn, mdn, MCPWM_HMASK_NONE + MCPWM_LMASK_NONE);
	mcpwm_SetOutPolarity(mcn, mdn, MCPWM_HPOL_NORMAL + MCPWM_LPOL_NORMAL);
	mcpwm_SetDeadband(mcn, mdn, 0L);
	mcpwm_ReloadOUT_Unsafe(mcn, mdn, MCPWM_RELOAD_NOW);
	
	mcpwm_SetWaveform(mcn, mdn, MCPWM_EDGE_A0I1);
	mcpwm_SetSamplCycle(mcn, mdn, 1999L); // sample cycle: 20ms
}

static bool timerOneInit = false;
void TimerOne::initialize(long microseconds) {
	timerOneInit = true;
	setPeriod(microseconds);
}

#define YES    (1)
#define NO     (2)
static int mc_md_inuse[PINS];
static int isPwmInit[PINS];
void TimerOne::setPeriod(long microseconds)	{ // AR modified for atomic access
	int mcn, mdn, i;
	if(timerOneInit == false || microseconds <= 0) return;
	
	pwmPeriod = microseconds;
	_period = (double)(microseconds*SYSCLK);
	
	if(timer1Enable == true) // if call attachInterrupt(), timer1Enable is ture
	{
		mcpwm_SetWidth(mc, md, _period-1.0, 0L);
	    mcpwm_ReloadPWM(mc, md, MCPWM_RELOAD_PEREND);
	}
	
	for(i=0; i<PINS; i++)
	{
		if(mc_md_inuse[i] == YES) // if call pwm()...
		{
			mcn = arduino_to_mc_md[0][i];
	    	mdn = arduino_to_mc_md[1][i];
			mcpwm_SetWidth(mcn, mdn, _period-1.0, _duty[i]-1.0);
		    mcpwm_ReloadPWM(mcn, mdn, MCPWM_RELOAD_PEREND);
		}
	}
}

static void safeClosePwmModule(int mcn, int mdn, double _period) {
	mcpwm_SetWidth(mcn, mdn, _period-1.0, 0L);
	mcpwm_ReloadPWM(mcn, mdn, MCPWM_RELOAD_PEREND);
	while(mcpwm_ReadReloadPWM(mcn, mdn) != 0L);
	while(mcpwm_ReadSTATREG2(mcn, mdn) > (_period - 1L));
	mcpwm_Disable(mcn, mdn);
}

void TimerOne::setPwmDuty(char pin, int duty) {
	int mcn, mdn;
	unsigned short crossbar_ioaddr = sb_Read16(0x64)&0xfffe;
		
	if(pin < 0 || pin >= PINS || timerOneInit == false) return;
	
	mcn = arduino_to_mc_md[0][pin];
    mdn = arduino_to_mc_md[1][pin];
    
    if(mcn == NOPWM || mdn == NOPWM) return;

	if(duty <= 0)
	{
		if(mc_md_inuse[pin] == YES)
		{
			safeClosePwmModule(mcn, mdn, _period);
			mc_md_inuse[pin] = NO;
		}
		pinMode(pin, OUTPUT);
		digitalWrite(pin, LOW);
		return;
	}
	
	io_DisableINT();
    if(mc_md_inuse[pin] == 0 || mc_md_inuse[pin] == NO)
	{
		pwmInit(mcn, mdn);
		
		if (pin <= 9)
			io_outpb(crossbar_ioaddr + 2, 0x01); // GPIO port2: 0A, 0B, 0C, 3A
		else if (pin > 28)
	    	io_outpb(crossbar_ioaddr, 0x03); // GPIO port0: 2A, 2B, 2C, 3C
		else
			io_outpb(crossbar_ioaddr + 3, 0x02); // GPIO port3: 1A, 1B, 1C, 3B
    }
    
    _duty[pin] = _period*duty/1024.0;
    
	mcpwm_SetWidth(mcn, mdn, _period-1.0, _duty[pin]-1.0);
	mcpwm_ReloadPWM(mcn, mdn, MCPWM_RELOAD_PEREND);
    io_RestoreINT();
    
    if(isPwmInit[pin] == 1 && (mc_md_inuse[pin] == 0 || mc_md_inuse[pin] == NO))
    {
    	io_outpb(crossbar_ioaddr + 0x90 + pinMap[pin], 0x08); // Enable PWM pin
		mcpwm_Enable(mcn, mdn);
		mc_md_inuse[pin] = YES;
    }
}

void TimerOne::pwm(char pin, int duty, long microseconds) { // expects duty cycle to be 10 bit (1024)
	int mcn, mdn;
	if(pin < 0 || pin >= PINS || timerOneInit == false) return;
	isPwmInit[pin] = 1; // for Arduino compatibility
	
	if(microseconds > 0) setPeriod(microseconds);
	
	mcn = arduino_to_mc_md[0][pin];
	mdn = arduino_to_mc_md[1][pin];
	  
	if(mcn == NOPWM || mdn == NOPWM) return;
  	
  	setPwmDuty(pin, duty);
}

void TimerOne::disablePwm(char pin) {
	unsigned short crossbar_ioaddr = sb_Read16(0x64)&0xfffe;
	int mcn, mdn;
	
	if(pin < 0 || pin >= PINS || timerOneInit == false) return;
	
	mcn = arduino_to_mc_md[0][pin];
    mdn = arduino_to_mc_md[1][pin];
    
    if(mcn == NOPWM || mdn == NOPWM) return;
    
	if(mc_md_inuse[pin] == YES)
	{
		safeClosePwmModule(mcn, mdn, _period);
		mc_md_inuse[pin] = NO;
		isPwmInit[pin] = 0;
		io_outpb(crossbar_ioaddr + 0x90 + pinMap[pin], 0x01); // Disable PWM pin (To be GPIO)
	}
}
 
void TimerOne::attachInterrupt(void (*isr)(void), long microseconds) {
	int i;
	if(timerOneInit == false || isr == NULL) return;
	mcpwm_Disable(mc, md);
	
	if(microseconds > 0)                       
	{
		//if(microseconds < 50L) microseconds = 50L;
		setPeriod(microseconds);
	} 
	
	pwmInit(mc, md);
	disable_MCINT();
    
	clear_INTSTATUS();
	if(init_mc_irq() == false)
	{
		printf("Init MC IRQ fail\n");
		return;
    }
    enable_MCINT(PULSE_END_INT);

	mcpwm_SetWidth(mc, md, _period-1.0, 0L);
	
	io_DisableINT();
	isrCallback = isr; // register the user's callback with the real ISR
	io_RestoreINT();
	
	mcpwm_Enable(mc, md);
	timer1Enable = true;												
}

void TimerOne::detachInterrupt() {
	if(timerOneInit == false || timer1Enable == false) return;
	disable_MCINT(); // timer continues to count without calling the isr
}

void TimerOne::resume()	{
	start();
}

void TimerOne::restart() { // Depricated - Public interface to start at zero - Lex 10/9/2011
	start();				
}

void TimerOne::start() { // AR addition, renamed by Lex to reflect it's actual role
	int mcn, mdn, i;
	if(timer1Enable == false)
	{
		//if(isrCallback != NULL) disable_MCINT();        // AR added 
	    mcpwm_SetWidth(mc, md, _period-1.0, 0L);
		mcpwm_Enable(mc, md);
		//if(isrCallback != NULL) enable_MCINT(PULSE_END_INT);
		timer1Enable = true;
	}
	
	for(i=0; i<PINS; i++)
	{
		if(mc_md_inuse[i] == NO)
		{
			mcn = arduino_to_mc_md[0][i];
			mdn = arduino_to_mc_md[1][i];
			mcpwm_SetWidth(mcn, mdn, _period-1.0, _duty[i]-1.0);
			mcpwm_Enable(mcn, mdn);
			mc_md_inuse[i] = YES;
		}
	}
}

void TimerOne::stop() {
	int mcn, mdn, i;
	if(timer1Enable == true)
	{
		mcpwm_Disable(mc, md);
		timer1Enable = false;
	}
	
	for(i=0; i<PINS; i++)
	{
		if(mc_md_inuse[i] == YES)
		{
			mcn = arduino_to_mc_md[0][i];
			mdn = arduino_to_mc_md[1][i];
			mcpwm_Disable(mcn, mdn);
			mc_md_inuse[i] = NO;
		}
	}
}

unsigned long TimerOne::read() {	//returns the value of the timer in microseconds
	unsigned long tmp = mcpwm_ReadSTATREG2(mc, md);
	return ((_period - tmp) / SYSCLK);
}



/* for debugging
unsigned int testcount = 0;
unsigned int TimerRealTimeClock::readTestCount(void) {
	unsigned int tmp;
	io_DisableINT();
	tmp = testcount;
	io_EnableINT();
	return tmp;
}
*/

#define RTCIRQ    (8)
TimerRealTimeClock::TimerRealTimeClock() {
	isrCallback = NULL;
	timerRTCEnable = false;
}

static char* isrname_rtc = "TimerRTC";
static int timerrtc_isr_handler(int irq, void* data) {
	unsigned char tmp;
	io_outpb(0x70, 0x0C); // enable NMI and read RTC register C 
	tmp = io_inpb(0x71); // clear RTC interrupt state

	if((tmp & 0x40) > 0 && TimerRTC.isrCallback != NULL) TimerRTC.isrCallback();
    if((tmp & 0x70) == 0) return ISR_NONE;

    return ISR_HANDLED;
}

DMP_INLINE(unsigned char) inpb_cmos(unsigned char reg) {
    unsigned char tmp;
	io_DisableINT();
    io_outpb(0x70, 0x80 | reg); // disable NMI (by setting the 0x80 bit) and assign a RTC register address
    tmp = io_inpb(0x71);
    io_RestoreINT();
    return tmp;
}

DMP_INLINE(void) outpb_cmos(unsigned char reg, unsigned char data) {
    io_DisableINT();
    io_outpb(0x70, 0x80 | reg); // disable NMI (by setting the 0x80 bit) and assign a RTC register address
    io_outpb(0x71, data);
    io_RestoreINT();
}

static bool timerRTCInit = false;
void TimerRealTimeClock::initialize(long microseconds) {
    unsigned char tmp;
    timerRTCInit = true;
    setPeriod(microseconds);
    
    tmp = inpb_cmos(0x0A);
    outpb_cmos(0x0A, (tmp & 0xF0) | _freq);
}

void TimerRealTimeClock::attachInterrupt(void (*isr)(void), long microseconds) {
    unsigned char tmp;
    
    if(timerRTCInit == false || isr == NULL) return;
    
    if(timerRTCEnable == false)
	{
		if(irq_Setting(RTCIRQ, IRQ_EDGE_TRIGGER) == false)
	    {
	        printf("MCM IRQ Setting fail\n"); return;
	    }
		if(irq_InstallISR(RTCIRQ, timerrtc_isr_handler, isrname_rtc) == false)
		{
		    printf("irq_install fail\n"); return;
		}
	}
	
    if(microseconds > 0) setPeriod(microseconds);
    
    io_DisableINT();
    isrCallback = isr;
    io_RestoreINT();
    
	tmp = inpb_cmos(0x0B);
    outpb_cmos(0x0B, tmp | 0x40);
    timerRTCEnable = true;
}

void TimerRealTimeClock::detachInterrupt() {
    unsigned char tmp;
    if(timerRTCInit == false || timerRTCEnable == false) return;
    
	tmp = inpb_cmos(0x0B);
    outpb_cmos(0x0B, tmp & ~(0x40));
    
    irq_UninstallISR(RTCIRQ, (void*)isrname_rtc);
    timerRTCEnable = false;
}

void TimerRealTimeClock::setPeriod(long microseconds) {
	if(timerRTCInit == false || microseconds <= 0) return;
	
	if     (microseconds < 183L)    _freq = 3;
	else if(microseconds < 366L)    _freq = 4;
	else if(microseconds < 732L)    _freq = 5;
	else if(microseconds < 1464L)   _freq = 6;
	else if(microseconds < 2929L)   _freq = 7;
	else if(microseconds < 5859L)   _freq = 8;
	else if(microseconds < 11718L)  _freq = 9;
	else if(microseconds < 23437L)  _freq = 10;
	else if(microseconds < 46875L)  _freq = 11;
	else if(microseconds < 93750L)  _freq = 12;
	else if(microseconds < 187500L) _freq = 13;
	else if(microseconds < 375000L) _freq = 14;
	else if(microseconds < 500000L) _freq = 15;
	else                            _freq = 15;
}

/*
void TimerRealTimeClock::resume()	{
	start();
}

void TimerRealTimeClock::restart() {
	start();				
}

void TimerRealTimeClock::start() {

}

void TimerRealTimeClock::stop() {

}
*/
#endif