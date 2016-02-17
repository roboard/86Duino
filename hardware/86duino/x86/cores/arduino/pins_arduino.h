#ifndef Pins_Arduino_h
#define Pins_Arduino_h

#include <stdint.h>
#include "mcm.h"

#define LOW          (0x00)
#define HIGH         (0x01)
#define CHANGE       (0x02)
#define FALLING      (0x03)
#define RISING       (0x04)

#define INPUT          (0x00)
#define OUTPUT         (0x01)
#define INPUT_PULLUP   (0x02)
#define INPUT_PULLDOWN (0x03)

// for 86Duino
#define SB_CROSSBASE  (0x64)
#define CROSSBARBASE  (0x0A00)
#define SB_FCREG      (0xC0)
#define SB_GPIOBASE   (0x62)
#define GPIOCTRLBASE  (0xF100)
#define GPIODATABASE  (0xF200)
#define GPIODIRBASE   (0xF202)

#define NOUSED  (-1)
#define NOAD    (-1)
#define NOPWM   (-1)
#define NOENC   (-1)
#define NONE    (-1)

//#define CRB_DEBUGMODE

struct pinsconfig {
	// GPIO
	int gpN;
	int gpMode;
	int gpOutput;
	
	// AD
	int AD;
	
	// PWM
	int PWMMC;
	int PWMMD;
	
	// Interrupt
	int ENCMC;
	int ENCMD;
	void (*userfunc)(void);
};
	
#if defined (__86DUINO_ONE) || defined (__86DUINO_ZERO) || defined (__86DUINO_EDUCAKE)
	#define PINS                       (52) // GPIO PINS (45) + A/D PINS (7) = 52 pINS
	#define EXTERNAL_NUM_INTERRUPTS    (12)
	#define CRBTABLE_SIZE              (0xC0)

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

	static struct pinsconfig PIN86[PINS] = 
	{
		{11, NONE, NONE, 0, NOPWM, NOPWM, NOENC, NOENC, NULL}, // pin 0
		{10, NONE, NONE, 1, NOPWM, NOPWM, NOENC, NOENC, NULL}, // pin 1
		{39, NONE, NONE, 2, NOPWM, NOPWM, NOENC, NOENC, NULL}, // pin 2
		{23, NONE, NONE, 3, MC_MODULE3, MCPWM_MODULEA, NOENC, NOENC, NULL}, // pin 3
		{37, NONE, NONE, 4, NOPWM, NOPWM, NOENC, NOENC, NULL}, // pin 4
		{20, NONE, NONE, 5, MC_MODULE0, MCPWM_MODULEC, NOENC, NOENC, NULL}, // pin 5
		{19, NONE, NONE, 6, MC_MODULE0, MCPWM_MODULEB, NOENC, NOENC, NULL}, // pin 6
		{35, NONE, NONE, NOAD, NOPWM, NOPWM, NOENC, NOENC, NULL}, // pin 7
		{33, NONE, NONE, NOAD, NOPWM, NOPWM, NOENC, NOENC, NULL}, // pin 8
		{17, NONE, NONE, NOAD, MC_MODULE0, MCPWM_MODULEA, NOENC, NOENC, NULL}, // pin 9
		{28, NONE, NONE, NOAD, MC_MODULE1, MCPWM_MODULEC, NOENC, NOENC, NULL}, // pin 10
		{27, NONE, NONE, NOAD, MC_MODULE1, MCPWM_MODULEB, NOENC, NOENC, NULL}, // pin 11
		{32, NONE, NONE, NOAD, NOPWM, NOPWM, NOENC, NOENC, NULL}, // pin 12
		{25, NONE, NONE, NOAD, MC_MODULE1, MCPWM_MODULEA, NOENC, NOENC, NULL}, // pin 13
		{12, NONE, NONE, NOAD, NOPWM, NOPWM, NOENC, NOENC, NULL}, // pin 14
		{13, NONE, NONE, NOAD, NOPWM, NOPWM, NOENC, NOENC, NULL}, // pin 15
		{14, NONE, NONE, NOAD, NOPWM, NOPWM, NOENC, NOENC, NULL}, // pin 16
		{15, NONE, NONE, NOAD, NOPWM, NOPWM, NOENC, NOENC, NULL}, // pin 17
		{24, NONE, NONE, NOAD, NOPWM, NOPWM, MC_MODULE1, MCPWM_MODULEB, NULL}, // pin 18
		{26, NONE, NONE, NOAD, NOPWM, NOPWM, MC_MODULE1, MCPWM_MODULEB, NULL}, // pin 19
		{29, NONE, NONE, NOAD, NOPWM, NOPWM, MC_MODULE1, MCPWM_MODULEB, NULL}, // pin 20
		{47, NONE, NONE, NOAD, NOPWM, NOPWM, NOENC, NOENC, NULL}, // pin 21
		{46, NONE, NONE, NOAD, NOPWM, NOPWM, NOENC, NOENC, NULL}, // pin 22
		{45, NONE, NONE, NOAD, NOPWM, NOPWM, NOENC, NOENC, NULL}, // pin 23
		{44, NONE, NONE, NOAD, NOPWM, NOPWM, NOENC, NOENC, NULL}, // pin 24
		{43, NONE, NONE, NOAD, NOPWM, NOPWM, NOENC, NOENC, NULL}, // pin 25
		{42, NONE, NONE, NOAD, NOPWM, NOPWM, NOENC, NOENC, NULL}, // pin 26
		{41, NONE, NONE, NOAD, NOPWM, NOPWM, NOENC, NOENC, NULL}, // pin 27
		{40, NONE, NONE, NOAD, NOPWM, NOPWM, NOENC, NOENC, NULL}, // pin 28
		{ 1, NONE, NONE, NOAD, MC_MODULE2, MCPWM_MODULEA, NOENC, NOENC, NULL}, // pin 29
		{ 3, NONE, NONE, NOAD, MC_MODULE2, MCPWM_MODULEB, NOENC, NOENC, NULL}, // pin 30
		{ 4, NONE, NONE, NOAD, MC_MODULE2, MCPWM_MODULEC, NOENC, NOENC, NULL}, // pin 31
		{31, NONE, NONE, NOAD, MC_MODULE3, MCPWM_MODULEB, NOENC, NOENC, NULL}, // pin 32
		{ 0, NONE, NONE, NOAD, NOPWM, NOPWM, MC_MODULE2, MCPWM_MODULEB, NULL}, // pin 33
		{ 2, NONE, NONE, NOAD, NOPWM, NOPWM, MC_MODULE2, MCPWM_MODULEB, NULL}, // pin 34
		{ 5, NONE, NONE, NOAD, NOPWM, NOPWM, MC_MODULE2, MCPWM_MODULEB, NULL}, // pin 35
		{22, NONE, NONE, NOAD, NOPWM, NOPWM, MC_MODULE3, MCPWM_MODULEB, NULL}, // pin 36
		{30, NONE, NONE, NOAD, NOPWM, NOPWM, MC_MODULE3, MCPWM_MODULEB, NULL}, // pin 37
		{ 6, NONE, NONE, NOAD, NOPWM, NOPWM, MC_MODULE3, MCPWM_MODULEB, NULL}, // pin 38
		{38, NONE, NONE, NOAD, NOPWM, NOPWM, NOENC, NOENC, NULL}, // pin 39
		{36, NONE, NONE, NOAD, NOPWM, NOPWM, NOENC, NOENC, NULL}, // pin 40
		{34, NONE, NONE, NOAD, NOPWM, NOPWM, NOENC, NOENC, NULL}, // pin 41
		{16, NONE, NONE, NOAD, NOPWM, NOPWM, MC_MODULE0, MCPWM_MODULEB, NULL}, // pin 42
		{18, NONE, NONE, NOAD, NOPWM, NOPWM, MC_MODULE0, MCPWM_MODULEB, NULL}, // pin 43
		{21, NONE, NONE, NOAD, NOPWM, NOPWM, MC_MODULE0, MCPWM_MODULEB, NULL}, // pin 44
		{NOUSED, NONE, NONE, 0, NOPWM, NOPWM, NOENC, NOENC, NULL}, // pin 45
		{NOUSED, NONE, NONE, 1, NOPWM, NOPWM, NOENC, NOENC, NULL}, // pin 46
		{NOUSED, NONE, NONE, 2, NOPWM, NOPWM, NOENC, NOENC, NULL}, // pin 47
		{NOUSED, NONE, NONE, 3, NOPWM, NOPWM, NOENC, NOENC, NULL}, // pin 48
		{NOUSED, NONE, NONE, 4, NOPWM, NOPWM, NOENC, NOENC, NULL}, // pin 49
		{NOUSED, NONE, NONE, 5, NOPWM, NOPWM, NOENC, NOENC, NULL}, // pin 50
		{NOUSED, NONE, NONE, 6, NOPWM, NOPWM, NOENC, NOENC, NULL} // pin 51
	};

	static int INTPINSMAP[EXTERNAL_NUM_INTERRUPTS] = {42, 43, 44, 18, 19, 20, 33, 34, 35, 36, 37, 38};
	
	#if defined CRB_DEBUGMODE
	static int CROSSBARTABLE[CRBTABLE_SIZE] = {0x03, 0x08, 0x01, 0x02, 0x00, 0x00, 0x06, 0x00, 0x0A, 0x09, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	                                           0x0E, 0x0D, 0x01, 0x02, 0x05, 0x06, 0x03, 0x04, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x1F, 0x11, 0x12,
											   0x09, 0x0A, 0x0C, 0x0B, 0x07, 0x08, 0x0F, 0x10, 0xF0, 0x0F, 0x01, 0xF8, 0x7F, 0x00, 0x00, 0x00,
											   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
											   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
											   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
											   0x01, 0x01, 0x01, 0x02, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
											   0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x02, 0x02, 0x02, 0x02, 0x01, 0x00, 0x00, 0x00,
											   0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x08, 0x01, 0x08, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
											   0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x08, 0x08, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
											   0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01};
	#endif
	
#elif defined (__86DUINO_PLC)
	#define PINS                       (14)
	#define EXTERNAL_NUM_INTERRUPTS    (4)
	#define CRBTABLE_SIZE              (0xC0)

	static const uint8_t LED_BUILTIN = 13;

	static struct pinsconfig PIN86[PINS] = 
	{
		{1, OUTPUT, LOW, NOAD, MC_MODULE0, MCPWM_MODULEA, NOENC, NOENC, NULL}, // pin 0
		{3, OUTPUT, LOW, NOAD, MC_MODULE0, MCPWM_MODULEB, NOENC, NOENC, NULL}, // pin 1
		{4, OUTPUT, LOW, NOAD, MC_MODULE0, MCPWM_MODULEC, NOENC, NOENC, NULL}, // pin 2
		{7, OUTPUT, LOW, NOAD, MC_MODULE3, MCPWM_MODULEA, NOENC, NOENC, NULL}, // pin 3
		{0, INPUT, NONE, NOAD, NOPWM, NOPWM, MC_MODULE0, MCPWM_MODULEB, NULL}, // pin 4
		{2, INPUT, NONE, NOAD, NOPWM, NOPWM, MC_MODULE0, MCPWM_MODULEB, NULL}, // pin 5
		{5, INPUT, NONE, NOAD, NOPWM, NOPWM, MC_MODULE0, MCPWM_MODULEB, NULL}, // pin 6
		{6, INPUT, NONE, NOAD, NOPWM, NOPWM, MC_MODULE3, MCPWM_MODULEB, NULL}, // pin 7
		{NOUSED, NONE, NONE, NOAD, NOPWM, NOPWM, NOENC, NOENC, NULL}, // pin 8
		{NOUSED, NONE, NONE, NOAD, NOPWM, NOPWM, NOENC, NOENC, NULL}, // pin 9
		{NOUSED, NONE, NONE, NOAD, NOPWM, NOPWM, NOENC, NOENC, NULL}, // pin 10
		{NOUSED, NONE, NONE, NOAD, NOPWM, NOPWM, NOENC, NOENC, NULL}, // pin 11
		{NOUSED, NONE, NONE, NOAD, NOPWM, NOPWM, NOENC, NOENC, NULL}, // pin 12
		{8, NONE, NONE, NOAD, NOPWM, NOPWM, NOENC, NOENC, NULL} // pin 13
	};
	
	static int INTPINSMAP[EXTERNAL_NUM_INTERRUPTS] = {4, 5, 6, 7};
	
	#if defined CRB_DEBUGMODE
	static int CROSSBARTABLE[CRBTABLE_SIZE] = {0x01, 0x00, 0x00, 0x08, 0x00, 0x00, 0x06, 0x00, 0x09, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	                                           0x0E, 0x0D, 0x07, 0x08, 0x00, 0x1E, 0x1F, 0x21, 0x09, 0x0A, 0x0C, 0x0B, 0x17, 0x18, 0x0F, 0x10,
											   0x27, 0x28, 0x29, 0x2A, 0x2B, 0x1A, 0x11, 0x12, 0x00, 0x00, 0x01, 0xE0, 0x7F, 0x00, 0x00, 0x00,
											   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
											   0x01, 0x00, 0x00, 0x01, 0x01, 0x00, 0x01, 0x01, 0x01, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00,
											   0x01, 0x00, 0x00, 0x01, 0x01, 0x00, 0x01, 0x01, 0x01, 0x00, 0x00, 0x01, 0x01, 0x00, 0x01, 0x01,
											   0x01, 0x01, 0x01, 0x02, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
											   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x02, 0x02, 0x02, 0x02, 0x01, 0x00, 0x00, 0x00,
											   0x00, 0x00, 0x00, 0x00, 0x02, 0x02, 0x08, 0x01, 0x08, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
											   0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
											   0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08};
	#endif
#endif

#endif
