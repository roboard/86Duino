#ifndef __V86_CLOCK_H
#define __V86_CLOCK_H

#include <stdlib.h>
#include <stdint.h>

#define __86DUINO_QEC_7 // for libraries
#define __86DUINO_EX2  // for cores
//#define USBSERIAL_REMOTE
// #define ETHERNET_REMOTE
// #define SD_REMOTE
// #define USBHOST_REMOTE
// #define EEPROM_REMOTE

#define MCM_VORTEX86EX2

/* USE the IPCNET and Virtual Dual Port RAM libraries */
#define __IPCFIFO_LIB
#define __VIRTUAL_DPMEM_LIB

#define __COPROC_LIB
#define __COPROC_CLIENT
#define __ETHERCAT_MASTER_LIB

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
	
#define PINS                       (27)
#define EXTERNAL_NUM_INTERRUPTS    (6)
#define CRBTABLE_SIZE              (0x130)
#define FLASH_CRASH_LED_PIN        (13)

static const uint8_t SS   = 10;
static const uint8_t MOSI = 11;
static const uint8_t MISO = 12;
static const uint8_t SCK  = 13;

static const uint8_t LED_BUILTIN = 13;
static const uint8_t POWER_LED_GREEN = 69; // no used
static const uint8_t POWER_LED_RED = 68; //  no used

static const uint8_t A0 = 14;
static const uint8_t A1 = 15;
static const uint8_t A2 = 16;
static const uint8_t A3 = 17;
static const uint8_t A4 = 18;
static const uint8_t A5 = 19;
static const uint8_t AVD = 26;

static struct pinsconfig PIN86[PINS] = 
{
    { 73, NOSTATUS, NOSTATUS, 0, NOPWM, NOPWM, NOENC, NOENC, NULL}, // pin 0
    { 72, NOSTATUS, NOSTATUS, 1, NOPWM, NOPWM, NOENC, NOENC, NULL}, // pin 1
    { 0,  NOSTATUS, NOSTATUS, 2, NOPWM, NOPWM, MC_MODULE0, MCPWM_MODULEB, NULL}, // pin 2
    { 1,  NOSTATUS, NOSTATUS, 3, MC_MODULE0, MCPWM_MODULEA, NOENC, NOENC, NULL}, // pin 3
    { 2,  NOSTATUS, NOSTATUS, 10, NOPWM, NOPWM, MC_MODULE0, MCPWM_MODULEB, NULL}, // pin 4
    { 3,  NOSTATUS, NOSTATUS, 11, MC_MODULE0, MCPWM_MODULEB, NOENC, NOENC, NULL}, // pin 5
    { 4,  NOSTATUS, NOSTATUS, NOAD, MC_MODULE0, MCPWM_MODULEC, NOENC, NOENC, NULL}, // pin 6
    { 5,  NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, MC_MODULE0, MCPWM_MODULEB, NULL}, // pin 7
    { 24, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, MC_MODULE2, MCPWM_MODULEB, NULL}, // pin 8
    { 7,  NOSTATUS, NOSTATUS, NOAD, MC_MODULE3, MCPWM_MODULEA, NOENC, NOENC, NULL}, // pin 9
    { 11, NOSTATUS, NOSTATUS, NOAD, MC_MODULE1, MCPWM_MODULEB, NOENC, NOENC, NULL}, // pin 10
    { 12, NOSTATUS, NOSTATUS, NOAD, MC_MODULE1, MCPWM_MODULEC, NOENC, NOENC, NULL}, // pin 11
    { 25, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, NOENC, NOENC, NULL}, // pin 12
    { 15, NOSTATUS, NOSTATUS, NOAD, MC_MODULE3, MCPWM_MODULEB, NOENC, NOENC, NULL}, // pin 13
    { 32, NOSTATUS, NOSTATUS, 0, NOPWM, NOPWM, NOENC, NOENC, NULL}, // pin 14
    { 33, NOSTATUS, NOSTATUS, 1, NOPWM, NOPWM, NOENC, NOENC, NULL}, // pin 15
    { 34, NOSTATUS, NOSTATUS, 2, NOPWM, NOPWM, NOENC, NOENC, NULL}, // pin 16
    { 35, NOSTATUS, NOSTATUS, 3, NOPWM, NOPWM, NOENC, NOENC, NULL}, // pin 17
    { 46, NOSTATUS, NOSTATUS, 10, NOPWM, NOPWM, NOENC, NOENC, NULL}, // pin 18
    { 47, NOSTATUS, NOSTATUS, 11, NOPWM, NOPWM, NOENC, NOENC, NULL}, // pin 19
    { 26, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, MC_MODULE2, MCPWM_MODULEB, NULL}, // pin 20, if this pin be switched to MCM, because the pin is not pull outside, user need to solder by himself.
    { 27, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, MC_MODULE2, MCPWM_MODULEA, NULL}, // pin 21
    { 28, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, MC_MODULE2, MCPWM_MODULEA, NULL}, // pin 22
    { 29, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, MC_MODULE2, MCPWM_MODULEB, NULL}, // pin 23
    { 30, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, NOENC, NOENC, NULL}, // pin 24
    { 31, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, NOENC, NOENC, NULL}, // pin 25
    { 45, NOSTATUS, NOSTATUS, 9, NOPWM, NOPWM, NOENC, NOENC, NULL} // pin26 --> this is voltage detected pin
};

static int INTPINSMAP[EXTERNAL_NUM_INTERRUPTS] = {2, 4, 7, 8, 20, 23};

#if defined CRB_DEBUGMODE
static int CROSSBARTABLE[CRBTABLE_SIZE] = {0x00, 0x00, 0x09, 0x08, 0x00, 0x1A, 0x07, 0x0A, 0x04, 0x05, 0x06, 0x01, 0x02, 0x03, 0x00, 0x1B,
                                         0x00, 0x00, 0x00, 0x00, 0x00, 0x2C, 0x00, 0x00, 0x2E, 0x2F, 0x07, 0x08, 0x0C, 0x05, 0x06, 0x0B,
                                         0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                         0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                         0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                         0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                         0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                         0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                         0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                         0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x31,
                                         0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                         0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                         0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                         0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                         0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                         0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x08, 0x01,
                                         0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
                                         0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
                                         0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x01, 0x08, 0x08};
#endif

#endif