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

#ifndef TIMERONE_h
#define TIMERONE_h

#include "Arduino.h"

class TimerOne
{
	private:
		bool timer1Enable;
		double _period;
		double _duty[PINS];
         
	public:
		TimerOne();
		
		// properties
		unsigned int pwmPeriod;
		
		// methods
		void initialize(long microseconds=1000000L);
		void start();
		void stop();
		void restart();
		void resume();
		unsigned long read();
		void pwm(char pin, int duty, long microseconds=-1);
		void disablePwm(char pin);
		void attachInterrupt(void (*isr)(), long microseconds=-1);
		void detachInterrupt();
		void setPeriod(long microseconds);
		void setPwmDuty(char pin, int duty);
		void (*isrCallback)();
};

class TimerRealTimeClock {
	private:
		unsigned char _freq;
		bool timerRTCEnable;
		
	public:
		TimerRealTimeClock();
		void initialize(long microseconds=500000L);
		void attachInterrupt(void (*isr)(), long microseconds=-1);
		void detachInterrupt();
		void setPeriod(long microseconds);
		void (*isrCallback)();
		//unsigned int readTestCount();
};


extern TimerOne Timer1;
extern TimerRealTimeClock TimerRTC;
#endif