#ifndef __V86_CLOCK_H
#define __V86_CLOCK_H

#include <stdlib.h>
#include <stdint.h>

#define __86DUINO_AI // for libraries
#define __86DUINO_EX // for cores

#define MCM_VORTEX86EX

#define NOUSED   (-1)
#define NOAD     (-1)
#define NOPWM    (-1)
#define NOENC    (-1)
#define NOSTATUS (-1)

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
	
#define PINS                       (42) // GPIO PINS (38) + A/D PINS (2) + IMU DETECT PINS(2) = 42 PINS
#define EXTERNAL_NUM_INTERRUPTS    (12)
#define CRBTABLE_SIZE              (0xC0)
#define FLASH_CRASH_LED_PIN        (13)

#define ENCSSI_MODE_AVAILABLE

#define CRB_DEBUGMODE

static const uint8_t SS   = 10;
static const uint8_t MOSI = 11;
static const uint8_t MISO = 12;
static const uint8_t SCK  = 13;

static const uint8_t LED_BUILTIN = 13;

static const uint8_t A0 = 38;
static const uint8_t A1 = 39;

static const uint8_t SDA = 40;
static const uint8_t SCL = 41;

static unsigned char vusbd_onoff_port = 7;
static unsigned char vusbd_onoff_pin  = 1;

static struct pinsconfig PIN86[PINS] = 
{
    {25, NOSTATUS, NOSTATUS, 0, MC_MODULE0, MCPWM_MODULEA, NOENC, NOENC, NULL}, // pin 0
    {27, NOSTATUS, NOSTATUS, 1, MC_MODULE0, MCPWM_MODULEB, NOENC, NOENC, NULL}, // pin 1
    {28, NOSTATUS, NOSTATUS, NOAD, MC_MODULE0, MCPWM_MODULEC, NOENC, NOENC, NULL}, // pin 2
    {31, NOSTATUS, NOSTATUS, NOAD, MC_MODULE3, MCPWM_MODULEA, NOENC, NOENC, NULL}, // pin 3
    {24, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, MC_MODULE0, MCPWM_MODULEB, NULL}, // pin 4
    {26, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, MC_MODULE0, MCPWM_MODULEB, NULL}, // pin 5
    {29, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, MC_MODULE0, MCPWM_MODULEB, NULL}, // pin 6
    {17, NOSTATUS, NOSTATUS, NOAD, MC_MODULE1, MCPWM_MODULEA, NOENC, NOENC, NULL}, // pin 7
    {19, NOSTATUS, NOSTATUS, NOAD, MC_MODULE1, MCPWM_MODULEB, NOENC, NOENC, NULL}, // pin 8
    {20, NOSTATUS, NOSTATUS, NOAD, MC_MODULE1, MCPWM_MODULEC, NOENC, NOENC, NULL}, // pin 9
    {16, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, MC_MODULE1, MCPWM_MODULEB, NULL}, // pin 10
    {18, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, MC_MODULE1, MCPWM_MODULEB, NULL}, // pin 11
    {21, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, MC_MODULE1, MCPWM_MODULEB, NULL}, // pin 12
    { 1, NOSTATUS, NOSTATUS, NOAD, MC_MODULE2, MCPWM_MODULEA, NOENC, NOENC, NULL}, // pin 13
    { 3, NOSTATUS, NOSTATUS, NOAD, MC_MODULE2, MCPWM_MODULEB, NOENC, NOENC, NULL}, // pin 14
    { 4, NOSTATUS, NOSTATUS, NOAD, MC_MODULE2, MCPWM_MODULEC, NOENC, NOENC, NULL}, // pin 15
    { 0, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, MC_MODULE2, MCPWM_MODULEB, NULL}, // pin 16
    { 2, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, MC_MODULE2, MCPWM_MODULEB, NULL}, // pin 17
    { 5, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, MC_MODULE2, MCPWM_MODULEB, NULL}, // pin 18
    {30, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, MC_MODULE3, MCPWM_MODULEB, NULL}, // pin 19
    {22, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, MC_MODULE3, MCPWM_MODULEB, NULL}, // pin 20
    { 6, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, MC_MODULE3, MCPWM_MODULEB, NULL}, // pin 21
    { 7, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, NOENC, NOENC, NULL}, // pin 22
    {23, NOSTATUS, NOSTATUS, NOAD, MC_MODULE3, MCPWM_MODULEB, NOENC, NOENC, NULL}, // pin 23
    {64, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, NOENC, NOENC, NULL}, // pin 24
    {65, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, NOENC, NOENC, NULL}, // pin 25
    {12, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, NOENC, NOENC, NULL}, // pin 26
    {13, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, NOENC, NOENC, NULL}, // pin 27
    {14, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, NOENC, NOENC, NULL}, // pin 28
    {15, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, NOENC, NOENC, NULL}, // pin 29
    {66, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, NOENC, NOENC, NULL}, // pin 30
    {67, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, NOENC, NOENC, NULL}, // pin 31
    {68, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, NOENC, NOENC, NULL}, // pin 32
    {69, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, NOENC, NOENC, NULL}, // pin 33
    { 8, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, NOENC, NOENC, NULL}, // pin 34
    { 9, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, NOENC, NOENC, NULL}, // pin 35
    {10, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, NOENC, NOENC, NULL}, // pin 36
    {11, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, NOENC, NOENC, NULL}, // pin 37
    {NOUSED, NOSTATUS, NOSTATUS, 0, NOPWM, NOPWM, NOENC, NOENC, NULL}, // pin 38
    {NOUSED, NOSTATUS, NOSTATUS, 1, NOPWM, NOPWM, NOENC, NOENC, NULL}, // pin 39
    {32, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, NOENC, NOENC, NULL}, // pin 40
    {33, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, NOENC, NOENC, NULL} // pin 41
};

static int INTPINSMAP[EXTERNAL_NUM_INTERRUPTS] = {4, 5, 6, 10, 11, 12, 16, 17, 18, 19, 20, 21};

#if defined CRB_DEBUGMODE
static int CROSSBARTABLE[CRBTABLE_SIZE] = {0x03, 0x08, 0x02, 0x01, 0x00, 0x09, 0x06, 0x00, 0x00, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                           0x05, 0x06, 0x07, 0x08, 0x01, 0x02, 0x03, 0x04, 0x00, 0x00, 0x00, 0x00, 0x1F, 0x1E, 0x11, 0x12,
                                           0x09, 0x0A, 0x0C, 0x0B, 0x00, 0x00, 0x0E, 0x0D, 0xC0, 0x0B, 0x03, 0xE0, 0xFF, 0x00, 0x00, 0x00,
                                           0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01,
                                           0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                           0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                           0x01, 0x01, 0x01, 0x02, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                           0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01,
                                           0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x08, 0x01, 0x01, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                           0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x08, 0x08, 0x08, 0x08, 0x01, 0x01,
                                           0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01};
#endif

#endif