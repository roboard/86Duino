/*
  pins_arduino.h - Pin definition functions for Arduino
  Part of Arduino - http://www.arduino.cc/

  Copyright (c) 2007 David A. Mellis

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General
  Public License along with this library; if not, write to the
  Free Software Foundation, Inc., 59 Temple Place, Suite 330,
  Boston, MA  02111-1307  USA

  $Id: wiring.h 249 2007-02-03 16:52:51Z mellis $
*/

#ifndef Pins_Arduino_h
#define Pins_Arduino_h

#include "stdint.h"
#include "mcm.h"

static const uint8_t SS   = 10;
static const uint8_t MOSI = 11;
static const uint8_t MISO = 12;
static const uint8_t SCK  = 13;

static const uint8_t SDA = 18;
static const uint8_t SCL = 19;
static const uint8_t LED_BUILTIN = 13;

static const uint8_t A0 = 0;
static const uint8_t A1 = 1;
static const uint8_t A2 = 2;
static const uint8_t A3 = 3;
static const uint8_t A4 = 4;
static const uint8_t A5 = 5;
static const uint8_t A6 = 6;
static const uint8_t A7 = 7;

#define PINS    (45)
static int pinMap[PINS] = {11, 10, 39, 23, 37, 20, 19, 35, 33, 17,
                           28, 27, 32, 25, 12, 13, 14, 15, 24, 26,
						   29, 47, 46, 45, 44, 43, 42, 41, 40,  1,
						    3,  4, 31,  0,  2,  5, 22, 30,  6, 38,
						   36, 34, 16, 18, 21};

#define NOPWM    (-1)
static int MC_list[] = { MC_MODULE0, MC_MODULE1, MC_MODULE2, MC_MODULE3 };
static int MD_list[] = { MCPWM_MODULEA, MCPWM_MODULEB, MCPWM_MODULEC };
static int arduino_to_mc_md[2][PINS] = { 
    // MC
	{NOPWM, NOPWM, NOPWM, MC_list[3], NOPWM, MC_list[0], MC_list[0], NOPWM, NOPWM, MC_list[0],
	 MC_list[1], MC_list[1], NOPWM, MC_list[1], NOPWM, NOPWM, NOPWM, NOPWM, NOPWM, NOPWM,
	 NOPWM, NOPWM, NOPWM, NOPWM, NOPWM, NOPWM, NOPWM, NOPWM, NOPWM, MC_list[2],
	 MC_list[2], MC_list[2], MC_list[3], NOPWM, NOPWM, NOPWM, NOPWM, NOPWM, NOPWM, NOPWM,
	 NOPWM, NOPWM, NOPWM, NOPWM, NOPWM},
	// MD 
    {NOPWM, NOPWM, NOPWM, MD_list[0], NOPWM, MD_list[2], MD_list[1], NOPWM, NOPWM, MD_list[0],
	 MD_list[2], MD_list[1], NOPWM, MD_list[0], NOPWM, NOPWM, NOPWM, NOPWM, NOPWM, NOPWM,
	 NOPWM, NOPWM, NOPWM, NOPWM, NOPWM, NOPWM, NOPWM, NOPWM, NOPWM, MD_list[0],
	 MD_list[1], MD_list[2], MD_list[1], NOPWM, NOPWM, NOPWM, NOPWM, NOPWM, NOPWM, NOPWM,
	 NOPWM, NOPWM, NOPWM, NOPWM, NOPWM}
	};

#define EXTERNAL_NUM_INTERRUPTS    (12)
static void (*_userfunc[EXTERNAL_NUM_INTERRUPTS])(void) = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
static int pin_offset[EXTERNAL_NUM_INTERRUPTS] = {16, 18, 21, 24, 26, 29, 0, 2, 5, 22, 30, 6};

#endif
