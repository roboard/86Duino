/*
  tone.cpp - A Tone Generator Library for 86Duino
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

#if defined (DMP_LINUX)
#include <sys/resource.h>
#elif defined (DMP_DOS_DJGPP)
#include "mcm.h"
#include "irq.h"
#include <pc.h>
#endif

#if defined (DMP_LINUX)
struct tone_param
{
	int pin;
	unsigned int frequency;
	long duration;
	int ID;
};

int current_pin = -1;
int thread_control = 0;
long delayAmount;
long loopTime;
unsigned long time1;
unsigned long time2;
unsigned long toneDuration = 0;
OSSPIN t_lock;
bool init_t_lock = false;

void *toneHandler(void *prgs)
{
	setpriority(PRIO_PROCESS, 0, -20);

	tone_param *param;
	param = (tone_param*)prgs;

	int pin = param->pin;
	pinMode(pin, OUTPUT);
	long delayAmount;
	long loopTime;
	unsigned long time1;
	unsigned long time2;
	int currentID = param->ID;

	if((param->frequency) > 0)
	{
		delayAmount = (long)(1000000/(param->frequency))/2 - 1;
		loopTime = (long)((param->duration*1000)/(delayAmount*2));

		if((param->duration) > 0)
		{
			for(int i = 0; i < loopTime; i++)
			{
				if(currentID != thread_control)
				{
					break;
				}
				time1 = micros();
				time2 = time1;
				digitalWrite(pin, HIGH);
				while(((long)(time2 - time1)) <= delayAmount)
				{
					time2 = micros();
				}
				time1 = micros();
				digitalWrite(pin, LOW);
				while(((long)(time2 - time1)) <= delayAmount)
				{
					time2 = micros();
				}
			}
		}
		else
		{
			while(true)
			{
				if(currentID != thread_control)
				{
					break;
				}
				time1 = micros();
				time2 = time1;
				digitalWrite(pin, HIGH);
				while(((long)(time2 - time1)) <= delayAmount)
                {
                    time2 = micros();
                }
                time1 = micros();
                digitalWrite(pin, LOW);
                while(((long)(time2 - time1)) <= delayAmount)
                {
                    time2 = micros();
                }
			}
		}
	}
	else
	{
		time1 = micros();
		time2 = time1;
		delayAmount = (param->duration)*1000;
		while(((long)(time2 - time1)) <= delayAmount)
		{
			time2 = micros();
			if(currentID != thread_control)
			{
				break;
			}
		}
	}
}

#elif defined (DMP_DOS_DJGPP)
#define TIMER_PIN    (32)  // It is a PWM pin (like analogWrite), but it is used as a timer here.
static int mc = 3, md = 1;

static volatile int use_pin_tone  = 255;
static volatile int use_pin_times = 0;
static int8_t toneBegin(uint8_t _pin)
{  
}

/**********/
/*  IRQ   */                                                                       
/**********/
static int mcint_offset[3] = {0, 8, 16};
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

static bool toneInitInt = false;
static char* name = "Tone";

static int h_l = 0;
DMP_INLINE(int) isr_handler(int irq, void* data)
{
    unsigned long toneintmask = PULSE_END_INT << mcint_offset[md];
    if((mc_inp(mc, 0x04) & toneintmask) == 0L) return ISR_NONE;
	
	mc_outp(mc, 0x04, toneintmask);
	
    if(use_pin_times != 0)
	{
		if(h_l == 0)
		{
			if(use_pin_tone != PCSPEAKER) digitalWrite(use_pin_tone, HIGH);
			if(use_pin_times > 0) use_pin_times--;
			h_l = 1;
		}
		else
		{
        	if(use_pin_tone != PCSPEAKER) digitalWrite(use_pin_tone, LOW);
        	if(use_pin_times > 0) use_pin_times--;
        	h_l = 0;
		}
    }
    else
        noTone(use_pin_tone);
     
    return ISR_HANDLED;
}

bool timer1_pin32_isUsed = false;
DMP_INLINE(bool) init_mc_irq(void)
{
	if(toneInitInt == false)
	{
		if(irq_InstallISR(GetMCIRQ(), isr_handler, name) == false)
		{
		    printf("irq_install fail\n"); return false;
		}
		
		if(timer1_pin32_isUsed == false)
		{
			pinMode(TIMER_PIN, INPUT);
			digitalWrite(TIMER_PIN, LOW);
		}
		toneInitInt = true;
	}
    return true;
}


DMPAPI(void) tone_INIT(uint8_t _pin, unsigned int frequency, unsigned long duration){
    mcpwm_ReloadPWM(mc, md, MCPWM_RELOAD_CANCEL);
    mcpwm_SetOutMask(mc, md, MCPWM_HMASK_NONE + MCPWM_LMASK_NONE);
    mcpwm_SetOutPolarity(mc, md, MCPWM_HPOL_NORMAL + MCPWM_LPOL_NORMAL);
    mcpwm_SetDeadband(mc, md, 0L);
    mcpwm_ReloadOUT_Unsafe(mc, md, MCPWM_RELOAD_NOW);
    mcpwm_SetWaveform(mc, md, MCPWM_EDGE_A0I1);
    mcpwm_SetSamplCycle(mc, md, 0x04L);   // sample cycle: 20ms
    mcpwm_SetOutPolarity(mc, md, MCPWM_HPOL_INVERSE + MCPWM_LPOL_INVERSE);
    mcpwm_SetDeadband(mc, md, 0L);
    mcpwm_ReloadOUT_Unsafe(mc, md, MCPWM_RELOAD_NOW);  
    
    disable_MCINT();
    clear_INTSTATUS();
	if(init_mc_irq() == false)
	{
		printf("Init MC IRQ fail\n");
		return;
    }
    enable_MCINT(PULSE_END_INT);
    
	if(_pin != PCSPEAKER) pinMode(_pin, OUTPUT);
    
    if(frequency <= 0)
		mcpwm_SetWidth(mc, md, (duration*1000) * SYSCLK, (duration*1000)/2 * SYSCLK);  // period: (1.0/frequency)*1000000)ms
    else
		mcpwm_SetWidth(mc, md, ((1.0/frequency)*1000000)/2 * SYSCLK, ((1.0/frequency)*500000)/2 * SYSCLK);  // period: (1.0/frequency)*1000000)ms     
    
	io_DisableINT();
    use_pin_tone = _pin;
	io_RestoreINT();
	
	if(_pin == PCSPEAKER) sound(frequency);
	mcpwm_Enable(mc, md);
}

DMPAPI(void) tone_UPDATE(uint8_t _pin, unsigned int frequency, unsigned long duration) {
	if(frequency <= 0)
		mcpwm_SetWidth(mc, md, (duration*1000) * SYSCLK,(duration*1000)/2 * SYSCLK);  // period: (1.0/frequency)*1000000)ms 單位ms 
    else
		mcpwm_SetWidth(mc, md, ((1.0/frequency)*1000000)/2 * SYSCLK, ((1.0/frequency)*500000)/2 * SYSCLK);  // period: (1.0/frequency)*1000000)ms    
    
	if(_pin == PCSPEAKER) sound(frequency);
	mcpwm_ReloadPWM(mc, md, MCPWM_RELOAD_PEREND);    
}

DMPAPI(void) disableTimer(uint8_t _timer)
{
}

#endif

DMPAPI(void) tone(uint8_t _pin, unsigned int frequency, unsigned long duration)
{
#if defined (DMP_LINUX)
	if(current_pin != -1 && current_pin != _pin)
		return;

	if(init_t_lock == false)
	{
		OSSPININIT(t_lock);
		init_t_lock = true;
	}

	OSSPINLOCK(t_lock);
	int tmpControl = ++thread_control;
	OSSPINUNLOCK(t_lock);

	pthread_t toneThread;
	struct tone_param *tparam;
	tparam = (tone_param *)malloc(sizeof(tone_param));
	tparam->pin = _pin;
	tparam->frequency = frequency;
	tparam->duration = duration;
	tparam->ID = tmpControl;
	int err = pthread_create(&toneThread, NULL, toneHandler, (void*)tparam);
	pthread_detach(toneThread);
#elif defined (DMP_DOS_DJGPP)
	if(use_pin_tone != 255 && use_pin_tone !=_pin) return; // tone() has been used before.
	
	io_DisableINT();
	if(frequency >= 10000) frequency = 10000;
	if(frequency > 0)
	{
		if(duration == 0L)
			use_pin_times = -1;
		else
			use_pin_times = duration / (((1.0/(float)frequency)*1000.0)/2.0);
	}
	else
		use_pin_times = 0;
	io_RestoreINT();
	
    if(use_pin_tone == 255)
	{
		tone_INIT(_pin, frequency, duration);
		return;  
    }
    
    if(use_pin_tone ==_pin)
	{
		tone_UPDATE(_pin, frequency, duration);
		return;
    }
#endif
}

DMPAPI(void) noTone(uint8_t _pin)
{
#if defined (DMP_LINUX)
	if(current_pin != _pin)
		return;

	OSSPINLOCK(t_lock);
	++thread_control;
	OSSPINUNLOCK(t_lock);

	current_pin = -1;
#elif defined (DMP_DOS_DJGPP)
	if(_pin == PCSPEAKER) nosound();
	io_DisableINT();
	use_pin_times = 0;
	use_pin_tone  = 255;
	io_RestoreINT();
	mcpwm_Disable(mc, md);
	digitalWrite(_pin, LOW);
#endif
}
