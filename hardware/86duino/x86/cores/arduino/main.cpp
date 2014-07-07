#include "Arduino.h"
#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include <process.h>

unsigned _stklen = 4096 * 1024;
DPMI_MEMORY_ALL_LOCK(0)
int main(void)
{
	init();	      
	setup();
    
	for (;;)
	{
		loop();
		if (serialEventRun) serialEventRun();
	}	
	return 0;
}

