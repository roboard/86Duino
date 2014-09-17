/*
  Encoder.cpp - DM&P Vortex86 Encoder library
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

#ifndef __ENC86_H__
#define __ENC86_H__

#include <Arduino.h>

// Encoder mode
#define MODE_NOSET       (0xFF)
#define MODE_STEP_DIR    (0)
#define MODE_CWCCW       (1)
#define MODE_AB_PHASE    (2)
#define MODE_CAPTURE     (3)
#define MODE_SSI         (4) // continue mode, no interrupt event
#define MODE_STEP_DIR_x2 (5) // another mode
#define MODE_CWCCW_x2    (6) // another mode
#define MODE_AB_PHASE_x2 (7) // another mode

// attachInterrupt flag
#define INTR_COMPARE        (0x01)
#define INTR_INDEX          (0x02)
#define INTR_OVERFLOW       (0x04)
#define INTR_UNDERFLOW      (0x08)
#define INTR_A_PULSE_LOW    (0x80)
#define INTR_A_PULSE_HIGH   (0x81)
#define INTR_B_PULSE_LOW    (0x82)
#define INTR_B_PULSE_HIGH   (0x83)
#define INTR_Z_PULSE_LOW    (0x84)
#define INTR_Z_PULSE_HIGH   (0x85)

class Encoder
{
	private:
		int mcn;
		int mdn;
		int mode;

		void _pdirInit(int samplerate);
		void _cwccwInit(int samplerate);
		void _pabInit(int samplerate);
		void _pcapInit();
		void _ssiInit(unsigned long bits, unsigned long clk, unsigned long wtime, bool gray2bin);

		unsigned long long int _pulseIn(uint8_t pin, uint8_t state, unsigned long timeout);
		
		bool _setZPol;
		int _dir;
	public:
		Encoder(int mc);

		void begin(int mode, unsigned long bits=12L, unsigned long ssi_clk=1000000L, unsigned long ssi_wtime=20L, bool ssi_gray2bin=false);
		void end(void);

		void setDigitalFilter(unsigned long width); // unit: 10 nano second
		void setIndexReset(bool condition=true);
		void setInputPolarity(bool pinA, bool pinB, bool pinZ); // parameters: HIGH/LOW
		void setComparator(unsigned long evncnt, bool condition=true);
		void setRange(unsigned long val=0xFFFFFFFFL, bool condition=true);
		
		void write(unsigned long cnt);
		unsigned long read(void);
		unsigned long readNanoseconds(void); // for pulse capture mode in interrupt callback
		int indexRead(void);
		int directionRead(void);

		unsigned long pulseInNanoseconds(uint8_t pin, uint8_t state, unsigned long timeout);
		unsigned long pulseIn(uint8_t pin, uint8_t state, unsigned long timeout);
		unsigned long pulseIn(uint8_t pin, uint8_t state); // no timeout

		void attachInterrupt(void (*callback)(int)); // for Pulse/DIR and CW/CCW and Pulse A/B 
		void detachInterrupt();
		
		//unsigned long readtestcount(); //for debugging
};

extern Encoder Enc0;
extern Encoder Enc1;
extern Encoder Enc2;
extern Encoder Enc3;

#endif