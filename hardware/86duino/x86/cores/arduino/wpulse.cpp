#include <stdio.h>
#include "Arduino.h"
#include "io.h"
#define USE_COMMON
#include "common.h"


unsigned long pulseIn(uint8_t pin, uint8_t state, unsigned long timeout) {
	uint8_t _state;
	unsigned long long int _timeout, now_time, width = 0L;
	
	if(pin >= PINS) return 0x00;
	
	_timeout = ((unsigned long long int)timeout)*333L + getclocks64(); // CPUCLK = 300MHz
	while(digitalRead(pin) == state)
		if(getclocks64() >= _timeout)
			return 0;
	while(digitalRead(pin) != state)
		if(getclocks64() >= _timeout)
			return 0;
	now_time = getclocks64();
	while(digitalRead(pin) == state)
		if(getclocks64() >= _timeout)
			return 0;
	width = getclocks64() - now_time;
	width = (unsigned long long int)(width * 0.00333); // CPUCLK = 300MHz
  
	return ((unsigned long)width); // unit is micro second 
}
