#include "Arduino.h"
#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include <process.h>

unsigned _stklen = 4096 * 1024;

// Weak empty variant initialization function.
// May be redefined by variant files.
void initVariant() __attribute__((weak));
void initVariant() { }

DPMI_MEMORY_ALL_LOCK(0)
int main(void)
{
	init();
	
	initVariant();
		      
	setup();
    
	for (;;)
	{
		loop();
		if (serialEventRun) serialEventRun();
	}	
	return 0;
}

