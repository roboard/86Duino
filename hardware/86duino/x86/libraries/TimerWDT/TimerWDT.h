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

#ifndef TIMERWDT_h
#define TIMERWDT_h

#define SYSTEM_RESET    (0)
#define INTERRUPT       (1)

class TimerWatchdogTimer {
	public:
		TimerWatchdogTimer();
		void initialize(long microseconds=500000L,bool type=false);
		void attachInterrupt(void (*isr)(), long microseconds=-1);
		void detachInterrupt();
		void setPeriod(long microseconds);
        void (*isrCallback)();
		void reset();
		bool isResetByWDT();
		void stop();
		//unsigned int readTestCount();
};

extern TimerWatchdogTimer TimerWDT;
#endif