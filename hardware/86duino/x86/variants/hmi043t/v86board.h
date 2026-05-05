#ifndef __V86_CLOCK_H
#define __V86_CLOCK_H

#include <stdlib.h>
#include <stdint.h>

#define __86DUINO_HMI043T // for libraries
#define __86DUINO_EX // for cores

#define MCM_VORTEX86EX

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
	
#define PINS                       (9) // GPIO PINS (45) + A/D PINS (7) = 52 pINS
#define EXTERNAL_NUM_INTERRUPTS    (3)
#define CRBTABLE_SIZE              (0xC0)
#define FLASH_CRASH_LED_PIN        (13)

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

static unsigned char vusbd_onoff_port = 9;
static unsigned char vusbd_onoff_pin  = 1;

static struct pinsconfig PIN86[PINS] = 
{
    {56, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, MC_MODULE0, MCPWM_MODULEB, NULL}, // pin 0
    {57, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, NOENC, NOENC, NULL}, // pin 1
    {58, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, MC_MODULE0, MCPWM_MODULEB, NULL}, // pin 2
    {59, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, NOENC, NOENC, NULL}, // pin 3
    {60, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, NOENC, NOENC, NULL}, // pin 4
    {61, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, MC_MODULE0, MCPWM_MODULEB, NULL}, // pin 5
    {62, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, NOENC, NOENC, NULL}, // pin 6
    {63, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, NOENC, NOENC, NULL}, // pin 7
    {22, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, NOENC, NOENC, NULL}  // pin 8: LCD Backlight
};

static int INTPINSMAP[EXTERNAL_NUM_INTERRUPTS] = {0, 2, 5};

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