#ifndef Pins_Arduino_h
#define Pins_Arduino_h

#include "stdint.h"
#include "mcm.h"

static const uint8_t SS   = 10;
static const uint8_t MOSI = 11;
static const uint8_t MISO = 12;
static const uint8_t SCK  = 13;

static const uint8_t LED_BUILTIN = 13;

static const uint8_t A0 = 45;
static const uint8_t A1 = 46;
static const uint8_t A2 = 47;
static const uint8_t A3 = 48;
static const uint8_t A4 = 49;
static const uint8_t A5 = 50;
static const uint8_t A6 = 51;

static const uint8_t SDA = 52;
static const uint8_t SCL = 53;

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
static void (*(_userfunc[EXTERNAL_NUM_INTERRUPTS]))(void) = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};

#if defined __86DUINO_ONE || defined __86DUINO_ZERO || defined __86DUINO_EDUCAKE
	static int pin_offset[EXTERNAL_NUM_INTERRUPTS] = {16, 18, 21, 24, 26, 29, 0, 2, 5, 22, 30, 6};
#endif

#endif
