/*
  wdt.cpp - WDT library for 86Duino
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

#include "Arduino.h"
#include "io.h"

static bool wdt1_init_f = false;
static struct wdt_status {
	unsigned char ctrl_reg;   // 0xA8
    unsigned char sigsel_reg; // 0xA9
    unsigned char count0_reg; // 0xAA
    unsigned char count1_reg; // 0xAB
    unsigned char count2_reg; // 0xAC
    unsigned char stat_reg;   // 0xAD

    unsigned char reload_reg; // 0xAE
} WDT_t;

void wdt1_timer_watchdog(unsigned long usec) {
	unsigned long ival;
	double dval;
	dval = ((double)usec) / 30.5;
	ival = (unsigned long)dval;

	if(ival > 0x00ffffffL) ival = 0x00ffffffL;

	io_outpb(0xac, (ival >> 16) & 0x00ff);
	io_outpb(0xab, (ival >> 8) & 0x00ff);
	io_outpb(0xaa, ival & 0x00ff);
}

void wdt_enable(unsigned long nTime) { // us unit
    unsigned char val;

    if(wdt1_init_f == true) return;

	// restore current WDT1 register
    WDT_t.ctrl_reg   = io_inpb(0xA8);
    WDT_t.sigsel_reg = io_inpb(0xA9);
    WDT_t.count0_reg = io_inpb(0xAA);
    WDT_t.count1_reg = io_inpb(0xAB);
    WDT_t.count2_reg = io_inpb(0xAC);
    WDT_t.stat_reg   = io_inpb(0xAD);
    WDT_t.reload_reg = io_inpb(0xAE);

    io_outpb(0xad, 0x80); // clear timeout event bit

	// set reset system mode
    io_outpb(0xa9, 0x0D << 4);

	// set time period
    wdt1_timer_watchdog(nTime);

    // start WDT1.
	val = io_inpb(0xa8);
	io_outpb(0xa8, val | 0x40);  // set bit 6 to enable WDT1

    wdt1_init_f = true;
}

void wdt_disable(void) {
    unsigned char val;
	if(wdt1_init_f == true)
	{
		val = io_inpb(0xa8);
		io_outpb(0xa8, val & (~0x40));  // reset bit 6 to disable WDT1

        io_outpb(0xA8, WDT_t.ctrl_reg);
		io_outpb(0xA9, WDT_t.sigsel_reg);
		io_outpb(0xAA, WDT_t.count0_reg);
		io_outpb(0xAB, WDT_t.count1_reg);
		io_outpb(0xAC, WDT_t.count2_reg);
		io_outpb(0xAD, WDT_t.stat_reg);
		io_outpb(0xAE, WDT_t.reload_reg);

        wdt1_init_f = false;
	}
}

void wdt_reset(void) {
    if(wdt1_init_f == false) return;
	io_outpb(0xae, 0xff);  // WDT1 reload register
}

bool rebootByWDT = false;
void wdt_init(void) {
    unsigned char val;
	val = io_inpb(0xa8);
	io_outpb(0xa8, val & (~0x40));

	if((io_inpb(0xad) & 0x80) == 0)
		rebootByWDT = false;
	else
		rebootByWDT = true;
}

bool get_wdt_timeout(void) {
	return ((io_inpb(0xad) & 0x80) == 0) ? (false) : (true);
}