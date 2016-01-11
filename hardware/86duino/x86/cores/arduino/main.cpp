#include "Arduino.h"
#include <stdlib.h>
#include <stdio.h>

#if defined (DMP_DOS_DJGPP)
	#include <conio.h>
	#include <process.h>
	#include <signal.h>
	#include "irq.h"
	#include <sys/exceptn.h>
#elif defined (DMP_LINUX)
	#include <signal.h>
	#include "irq.h"
#endif

#if defined (DMP_DOS_DJGPP)
unsigned _stklen = 4096 * 1024;

// Error process
#define LONG_TIME		(1000L)
#define SHORT_TIME		(50L)
#define ATIMESIZE 		(39)

static int _ledtime[ATIMESIZE+1] = {SHORT_TIME, LONG_TIME, SHORT_TIME, LONG_TIME, SHORT_TIME, LONG_TIME,
                             SHORT_TIME, SHORT_TIME, SHORT_TIME, SHORT_TIME, SHORT_TIME, SHORT_TIME, SHORT_TIME, SHORT_TIME,
                             SHORT_TIME, SHORT_TIME, SHORT_TIME, SHORT_TIME, SHORT_TIME, SHORT_TIME, SHORT_TIME, SHORT_TIME,
                             SHORT_TIME, SHORT_TIME, SHORT_TIME, SHORT_TIME, SHORT_TIME, SHORT_TIME, SHORT_TIME, SHORT_TIME,
                             SHORT_TIME, SHORT_TIME, SHORT_TIME, SHORT_TIME, SHORT_TIME, SHORT_TIME, SHORT_TIME, SHORT_TIME,
                             SHORT_TIME, LONG_TIME
                             };

void error_led_blink(int ledpin) {
	static int led_stat = 0;
	static bool first = true;
	static unsigned long nowledtime = 0L;
	if(first == true)
	{
		nowledtime = millis();
		if((led_stat%2) == 0) digitalWrite(ledpin, HIGH); else digitalWrite(ledpin, LOW);
		first = false;
	}
	
	if((millis() - nowledtime) > _ledtime[led_stat])
	{
		if(led_stat == ATIMESIZE) led_stat = 0; else led_stat++;
		first = true;
	}
}

void _86Duino_error_process(int num) {
	int ledpin = 13;
	// disable all irq except usb irq (5)
	i8259_DisableIRQ(0);
	i8259_DisableIRQ(1);
	i8259_DisableIRQ(3);
	i8259_DisableIRQ(4);
	i8259_DisableIRQ(6);
	i8259_DisableIRQ(7);
	i8259_DisableIRQ(8);
	i8259_DisableIRQ(9);
	i8259_DisableIRQ(10);
	i8259_DisableIRQ(11);
	i8259_DisableIRQ(12);
	i8259_DisableIRQ(13);
	i8259_DisableIRQ(14);
		
	// print error message
	printf("\nOop, this program is crash :(\n");
	printf("You may write a bug in your sketch, check and upload it again.\n");
	
	// led blink pattern	
	pinMode(ledpin, OUTPUT);
	while(1)
	{
		error_led_blink(ledpin);
	}
}
#endif

// Weak empty variant initialization function.
// May be redefined by variant files.
void initVariant() __attribute__((weak));
void initVariant() { }

static __attribute__((constructor(101))) void _f_init()
{
	init();
#if defined (DMP_DOS_DJGPP)
	signal(SIGSEGV, _86Duino_error_process);
	signal(SIGFPE, _86Duino_error_process);
#endif
}

#if defined (DMP_DOS_DJGPP)
DPMI_MEMORY_ALL_LOCK(0)
#endif

int main(void)
{
#if defined (DMP_LINUX)
	interrupt_init();
#elif defined (DMP_DOS_DJGPP)
	__djgpp_set_ctrl_c(0);
#endif
	initVariant();
	
	setup();

	for (;;)
	{
		loop();
		#if defined (DMP_DOS_DJGPP)
		if (serialEventRun) serialEventRun();
		#endif
	}	
	return 0;
}

