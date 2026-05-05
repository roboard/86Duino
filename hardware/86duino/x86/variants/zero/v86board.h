#ifndef __V86_CLOCK_H
#define __V86_CLOCK_H

#include <stdlib.h>
#include <stdint.h>

#define __86DUINO_ZERO // for libraries
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
	
#define PINS                       (52) // GPIO PINS (45) + A/D PINS (7) = 52 pINS
#define EXTERNAL_NUM_INTERRUPTS    (12)
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

static unsigned char vusbd_onoff_port = 7;
static unsigned char vusbd_onoff_pin  = 1;

static struct pinsconfig PIN86[PINS] = 
{
    {11, NOSTATUS, NOSTATUS, 0, NOPWM, NOPWM, NOENC, NOENC, NULL}, // pin 0
    {10, NOSTATUS, NOSTATUS, 1, NOPWM, NOPWM, NOENC, NOENC, NULL}, // pin 1
    {39, NOSTATUS, NOSTATUS, 2, NOPWM, NOPWM, NOENC, NOENC, NULL}, // pin 2
    {23, NOSTATUS, NOSTATUS, 3, MC_MODULE3, MCPWM_MODULEA, NOENC, NOENC, NULL}, // pin 3
    {37, NOSTATUS, NOSTATUS, 4, NOPWM, NOPWM, NOENC, NOENC, NULL}, // pin 4
    {20, NOSTATUS, NOSTATUS, 5, MC_MODULE0, MCPWM_MODULEC, NOENC, NOENC, NULL}, // pin 5
    {19, NOSTATUS, NOSTATUS, 6, MC_MODULE0, MCPWM_MODULEB, NOENC, NOENC, NULL}, // pin 6
    {35, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, NOENC, NOENC, NULL}, // pin 7
    {33, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, NOENC, NOENC, NULL}, // pin 8
    {17, NOSTATUS, NOSTATUS, NOAD, MC_MODULE0, MCPWM_MODULEA, NOENC, NOENC, NULL}, // pin 9
    {28, NOSTATUS, NOSTATUS, NOAD, MC_MODULE1, MCPWM_MODULEC, NOENC, NOENC, NULL}, // pin 10
    {27, NOSTATUS, NOSTATUS, NOAD, MC_MODULE1, MCPWM_MODULEB, NOENC, NOENC, NULL}, // pin 11
    {32, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, NOENC, NOENC, NULL}, // pin 12
    {25, NOSTATUS, NOSTATUS, NOAD, MC_MODULE1, MCPWM_MODULEA, NOENC, NOENC, NULL}, // pin 13
    {12, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, NOENC, NOENC, NULL}, // pin 14
    {13, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, NOENC, NOENC, NULL}, // pin 15
    {14, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, NOENC, NOENC, NULL}, // pin 16
    {15, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, NOENC, NOENC, NULL}, // pin 17
    {24, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, MC_MODULE1, MCPWM_MODULEB, NULL}, // pin 18
    {26, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, MC_MODULE1, MCPWM_MODULEB, NULL}, // pin 19
    {29, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, MC_MODULE1, MCPWM_MODULEB, NULL}, // pin 20
    {47, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, NOENC, NOENC, NULL}, // pin 21
    {46, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, NOENC, NOENC, NULL}, // pin 22
    {45, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, NOENC, NOENC, NULL}, // pin 23
    {44, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, NOENC, NOENC, NULL}, // pin 24
    {43, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, NOENC, NOENC, NULL}, // pin 25
    {42, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, NOENC, NOENC, NULL}, // pin 26
    {41, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, NOENC, NOENC, NULL}, // pin 27
    {40, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, NOENC, NOENC, NULL}, // pin 28
    { 1, NOSTATUS, NOSTATUS, NOAD, MC_MODULE2, MCPWM_MODULEA, NOENC, NOENC, NULL}, // pin 29
    { 3, NOSTATUS, NOSTATUS, NOAD, MC_MODULE2, MCPWM_MODULEB, NOENC, NOENC, NULL}, // pin 30
    { 4, NOSTATUS, NOSTATUS, NOAD, MC_MODULE2, MCPWM_MODULEC, NOENC, NOENC, NULL}, // pin 31
    {31, NOSTATUS, NOSTATUS, NOAD, MC_MODULE3, MCPWM_MODULEB, NOENC, NOENC, NULL}, // pin 32
    { 0, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, MC_MODULE2, MCPWM_MODULEB, NULL}, // pin 33
    { 2, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, MC_MODULE2, MCPWM_MODULEB, NULL}, // pin 34
    { 5, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, MC_MODULE2, MCPWM_MODULEB, NULL}, // pin 35
    {22, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, MC_MODULE3, MCPWM_MODULEB, NULL}, // pin 36
    {30, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, MC_MODULE3, MCPWM_MODULEB, NULL}, // pin 37
    { 6, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, MC_MODULE3, MCPWM_MODULEB, NULL}, // pin 38
    {38, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, NOENC, NOENC, NULL}, // pin 39
    {36, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, NOENC, NOENC, NULL}, // pin 40
    {34, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, NOENC, NOENC, NULL}, // pin 41
    {16, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, MC_MODULE0, MCPWM_MODULEB, NULL}, // pin 42
    {18, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, MC_MODULE0, MCPWM_MODULEB, NULL}, // pin 43
    {21, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, MC_MODULE0, MCPWM_MODULEB, NULL}, // pin 44
    {NOUSED, NOSTATUS, NOSTATUS, 0, NOPWM, NOPWM, NOENC, NOENC, NULL}, // pin 45
    {NOUSED, NOSTATUS, NOSTATUS, 1, NOPWM, NOPWM, NOENC, NOENC, NULL}, // pin 46
    {NOUSED, NOSTATUS, NOSTATUS, 2, NOPWM, NOPWM, NOENC, NOENC, NULL}, // pin 47
    {NOUSED, NOSTATUS, NOSTATUS, 3, NOPWM, NOPWM, NOENC, NOENC, NULL}, // pin 48
    {NOUSED, NOSTATUS, NOSTATUS, 4, NOPWM, NOPWM, NOENC, NOENC, NULL}, // pin 49
    {NOUSED, NOSTATUS, NOSTATUS, 5, NOPWM, NOPWM, NOENC, NOENC, NULL}, // pin 50
    {NOUSED, NOSTATUS, NOSTATUS, 6, NOPWM, NOPWM, NOENC, NOENC, NULL} // pin 51
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

#endif