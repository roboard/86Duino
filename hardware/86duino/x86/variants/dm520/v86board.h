#ifndef __V86_CLOCK_H
#define __V86_CLOCK_H

#include <stdlib.h>
#include <stdint.h>

#define __86DUINO_ZERO // for libraries
#define __86DUINO_EX // for cores

#define MCM_VORTEX86EX
#define __86DUINO_PLC  // for digital
#define __AI_PLC_8I8O  // for digtial

// MC no.
#define MC_GENERAL          (-1)
#define MC_MODULE0          (0)
#define MC_MODULE1          (1)
#define MC_MODULE2          (2)
#define MC_MODULE3          (3)
#define MC_MODULE4          (4)
#define MC_MODULE5          (5)
#define MC_MODULE6          (6)
#define MC_MODULE7          (7)
#define MC_MODULE8          (8)
#define MC_MODULE9          (9)
#define MC_MODULE10         (10)
#define MC_MODULE11         (11)

// Module no.
#define MCPWM_MODULEA       (0)
#define MCPWM_MODULEB       (1)
#define MCPWM_MODULEC       (2)
#define MCSERVO_MODULEA     (0)
#define MCSERVO_MODULEB     (1)
#define MCSERVO_MODULEC     (2)
#define MCSIF_MODULEA       (0)
#define MCSIF_MODULEB       (1)

#define NOUSED   (-1)
#define NOAD     (-1)
#define NOPWM    (-1)
#define NOENC    (-1)
#define NOSTATUS (-1)

#define MYLOW            (0x00)
#define MYHIGH           (0x01)
#define MYINPUT          (0x00)
#define MYOUTPUT         (0x01)

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
	
#define PINS                       (16) // GPIO PINS (16) + A/D PINS (0) = 16 pins
#define EXTERNAL_NUM_INTERRUPTS    (6)
#define CRBTABLE_SIZE              (0xC0)
#define FLASH_CRASH_LED_PIN        (0)

#define ENCSSI_MODE_AVAILABLE

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

static unsigned char vusbd_onoff_port = 7;
static unsigned char vusbd_onoff_pin  = 1;

static const uint8_t POWER_LED_GREEN = 79;
static const uint8_t POWER_LED_RED = 78;

extern uint8_t X00;
extern uint8_t X01;
extern uint8_t X02;
extern uint8_t X03;
extern uint8_t X04;
extern uint8_t X05;
extern uint8_t X06;
extern uint8_t X07;
extern uint8_t X10;
extern uint8_t X11;
extern uint8_t X12;
extern uint8_t X13;
extern uint8_t X14;
extern uint8_t X15;
extern uint8_t X16;
extern uint8_t X17;
extern uint8_t X20;
extern uint8_t X21;
extern uint8_t X22;
extern uint8_t X23;
extern uint8_t X24;
extern uint8_t X25;
extern uint8_t X26;
extern uint8_t X27;
extern uint8_t X30;
extern uint8_t X31;
extern uint8_t X32;
extern uint8_t X33;
extern uint8_t X34;
extern uint8_t X35;
extern uint8_t X36;
extern uint8_t X37;

extern uint8_t Y00;
extern uint8_t Y01;
extern uint8_t Y02;
extern uint8_t Y03;
extern uint8_t Y04;
extern uint8_t Y05;
extern uint8_t Y06;
extern uint8_t Y07;
extern uint8_t Y10;
extern uint8_t Y11;
extern uint8_t Y12;
extern uint8_t Y13;
extern uint8_t Y14;
extern uint8_t Y15;
extern uint8_t Y16;
extern uint8_t Y17;
extern uint8_t Y20;
extern uint8_t Y21;
extern uint8_t Y22;
extern uint8_t Y23;
extern uint8_t Y24;
extern uint8_t Y25;
extern uint8_t Y26;
extern uint8_t Y27;
extern uint8_t Y30;
extern uint8_t Y31;
extern uint8_t Y32;
extern uint8_t Y33;
extern uint8_t Y34;
extern uint8_t Y35;
extern uint8_t Y36;
extern uint8_t Y37;

extern int GPIO_IN_PORT[4];
extern int GPIO_OUT_PORT[4];
static const uint8_t ISA = 100; // no used, just for compile pass

static struct pinsconfig PIN86[PINS] = 
{
    { NOUSED, NOSTATUS, NOSTATUS, 0, NOPWM, NOPWM, NOENC, NOENC, NULL}, // output pin 0 
    { NOUSED, NOSTATUS, NOSTATUS, 0, NOPWM, NOPWM, NOENC, NOENC, NULL}, // output pin 1 
    { NOUSED, NOSTATUS, NOSTATUS, 0, NOPWM, NOPWM, NOENC, NOENC, NULL}, // output pin 2 
    { NOUSED, NOSTATUS, NOSTATUS, 0, NOPWM, NOPWM, NOENC, NOENC, NULL}, // output pin 3 
    { NOUSED, NOSTATUS, NOSTATUS, 0, NOPWM, NOPWM, NOENC, NOENC, NULL}, // output pin 4 
    { NOUSED, NOSTATUS, NOSTATUS, 0, NOPWM, NOPWM, NOENC, NOENC, NULL}, // output pin 5 
    { NOUSED, NOSTATUS, NOSTATUS, 0, NOPWM, NOPWM, NOENC, NOENC, NULL}, // output pin 6 
    { NOUSED, NOSTATUS, NOSTATUS, 0, NOPWM, NOPWM, NOENC, NOENC, NULL}, // output pin 7
    
    { NOUSED, NOSTATUS, NOSTATUS, 0, NOPWM, NOPWM, NOENC, NOENC, NULL}, // input pin 8 
    { NOUSED, NOSTATUS, NOSTATUS, 0, NOPWM, NOPWM, NOENC, NOENC, NULL}, // input pin 9 
    { NOUSED, NOSTATUS, NOSTATUS, 0, NOPWM, NOPWM, NOENC, NOENC, NULL}, // input pin 10 
    { NOUSED, NOSTATUS, NOSTATUS, 0, NOPWM, NOPWM, NOENC, NOENC, NULL}, // input pin 11 
    { NOUSED, NOSTATUS, NOSTATUS, 0, NOPWM, NOPWM, NOENC, NOENC, NULL}, // input pin 12 
    { NOUSED, NOSTATUS, NOSTATUS, 0, NOPWM, NOPWM, NOENC, NOENC, NULL}, // input pin 13 
    { NOUSED, NOSTATUS, NOSTATUS, 0, NOPWM, NOPWM, NOENC, NOENC, NULL}, // input pin 14 
    { NOUSED, NOSTATUS, NOSTATUS, 0, NOPWM, NOPWM, NOENC, NOENC, NULL}  // input pin 15
};

static int INTPINSMAP[EXTERNAL_NUM_INTERRUPTS] = {0, 3, 2, 0, 0, 0};

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

#endif