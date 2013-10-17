#include "Arduino.h"
#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include <process.h>

DPMI_MEMORY_ALL_LOCK(0)
int main(void)
{
	init();	      
	setup();
    
	for (;;)
		loop();
		
	return 0;
}

