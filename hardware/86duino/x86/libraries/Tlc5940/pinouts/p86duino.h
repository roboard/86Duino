/*  Copyright (c) 2009 by Alex Leone <acleone ~AT~ gmail.com>
    Modified 01 November 2014 by Android Lin for 86Duino

    This file is part of the Arduino TLC5940 Library.

    The Arduino TLC5940 Library is free software: you can redistribute it
    and/or modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation, either version 3 of the
    License, or (at your option) any later version.

    The Arduino TLC5940 Library is distributed in the hope that it will be
    useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with The Arduino TLC5940 Library.  If not, see
    <http://www.gnu.org/licenses/>. */

#ifndef __86DUINO_TLC_H
#define __86DUINO_TLC_H


/** VPRG (Arduino digital pin 8) -> VPRG (TLC pin 27) */
#define DEFAULT_VPRG_PIN    8
#define DEFAULT_VPRG_PORT   0
#define DEFAULT_VPRG_DDR    0

/** XERR (Arduino digital pin 12) -> XERR (TLC pin 16) */
#define DEFAULT_XERR_PIN    12
#define DEFAULT_XERR_PORT   0
#define DEFAULT_XERR_DDR    0
#define DEFAULT_XERR_PINS   0

/** SIN (Arduino digital pin 7) -> SIN (TLC pin 26) */
#define DEFAULT_BB_SIN_PIN      7
#define DEFAULT_BB_SIN_PORT     0
#define DEFAULT_BB_SIN_DDR      0
/** SCLK (Arduino digital pin 4) -> SCLK (TLC pin 25) */
#define DEFAULT_BB_SCLK_PIN     4
#define DEFAULT_BB_SCLK_PORT    0
#define DEFAULT_BB_SCLK_DDR     0

/** MOSI (Arduino digital pin 11) -> SIN (TLC pin 26) */
#define TLC_MOSI_PIN     11
#define TLC_MOSI_PORT    0
#define TLC_MOSI_DDR     0

/** SCK (Arduino digital pin 13) -> SCLK (TLC pin 25) */
#define TLC_SCK_PIN      13
#define TLC_SCK_PORT     0
#define TLC_SCK_DDR      0

/** SS will be set to output as to not interfere with SPI master operation.
    If you have changed the pin-outs and the library doesn't seem to work
    or works intermittently, make sure this pin is set correctly.  This pin
    will not be used by the library other than setting its direction to
    output. */
#define TLC_SS_PIN       10
#define TLC_SS_DDR       0

/** OC1A (Arduino digital pin 9) -> XLAT (TLC pin 24) */
#define XLAT_PIN     9
#define XLAT_PORT    0
#define XLAT_DDR     0

/** OC1B (Arduino digital pin 10) -> BLANK (TLC pin 23) */
#define BLANK_PIN    10
#define BLANK_PORT   0
#define BLANK_DDR    0

/** OC2B (Arduino digital pin 3) -> GSCLK (TLC pin 18) */
#define GSCLK_PIN    3
#define GSCLK_PORT   0
#define GSCLK_DDR    0

#endif

