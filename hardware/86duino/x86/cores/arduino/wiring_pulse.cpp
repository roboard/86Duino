/*
  wpulse.cpp - pulseIn() function for 86Duino
  Copyright (c) 2013 Android Lin <acen@dmp.com.tw>. All right reserved.

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

#include <stdio.h>
#include "Arduino.h"
#include "io.h"


unsigned long pulseIn(uint8_t pin, uint8_t state, unsigned long timeout) {
	uint8_t _state;
	unsigned long long int _timeout, now_time, width = 0L;
	
	if(pin >= PINS) return 0x00;
	
	_timeout = ((unsigned long long int)timeout)*333L + timer_GetClocks64(); // CPUCLK = 300MHz
	while(digitalRead(pin) == state)
		if(timer_GetClocks64() >= _timeout)
			return 0;
	while(digitalRead(pin) != state)
		if(timer_GetClocks64() >= _timeout)
			return 0;
	now_time = timer_GetClocks64();
	while(digitalRead(pin) == state)
		if(timer_GetClocks64() >= _timeout)
			return 0;
	width = timer_GetClocks64() - now_time;
	width = (unsigned long long int)(width * 0.00333); // CPUCLK = 300MHz
  
	return ((unsigned long)width); // unit is micro second 
}
