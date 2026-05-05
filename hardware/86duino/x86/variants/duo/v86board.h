#ifndef __V86_CLOCK_H
#define __V86_CLOCK_H

#include <stdlib.h>
#include <stdint.h>

#define __86DUINO_DUO // for libraries
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
	
#define PINS                       (8) // GPIO PINS (8)
#define EXTERNAL_NUM_INTERRUPTS    (15)
#define CRBTABLE_SIZE              (0x130)
#define FLASH_CRASH_LED_PIN        (13)

#define ENCSSI_MODE_AVAILABLE

// #define CRB_DEBUGMODE

// EX3 EBOX Settings
static const uint8_t SS   = 10;
static const uint8_t MOSI = 11;
static const uint8_t MISO = 12;
static const uint8_t SCK  = 13;

static const uint8_t LED_BUILTIN = 13;

static const uint8_t A0 = 42;

static struct pinsconfig PIN86[PINS] = 
{
    { 16, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, NOENC, NOENC, NULL}, // pin 0
    { 17, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, NOENC, NOENC, NULL}, // pin 1
    { 18, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, NOENC, NOENC, NULL}, // pin 2
    { 19, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, NOENC, NOENC, NULL}, // pin 3
    { 20, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, NOENC, NOENC, NULL}, // pin 4
    { 21, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, NOENC, NOENC, NULL}, // pin 5
    { 22, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, NOENC, NOENC, NULL}, // pin 6
    { 23, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, NOENC, NOENC, NULL}  // pin 7
};

static int INTPINSMAP[EXTERNAL_NUM_INTERRUPTS] = {0, 0, 0};

/*// DUO Settings
static const uint8_t SS   = 10;
static const uint8_t MOSI = 11;
static const uint8_t MISO = 12;
static const uint8_t SCK  = 13;

static const uint8_t LED_BUILTIN = 13;

static const uint8_t A0 = 42;
static const uint8_t A1 = 43;
static const uint8_t A2 = 44;
static const uint8_t A3 = 45;
static const uint8_t A4 = 46;
static const uint8_t A5 = 47;
static const uint8_t A6 = 48;
static const uint8_t A7 = 49;
static const uint8_t A8 = 50;
static const uint8_t A9 = 51;
static const uint8_t A10 = 52;
static const uint8_t A11 = 53;

static struct pinsconfig PIN86[PINS] = 
{
    { 92, NOSTATUS, NOSTATUS, 0, NOPWM, NOPWM, NOENC, NOENC, NULL}, // pin 0
    { 91, NOSTATUS, NOSTATUS, 1, NOPWM, NOPWM, NOENC, NOENC, NULL}, // pin 1
    { 30, NOSTATUS, NOSTATUS, 2, NOPWM, NOPWM, NOENC, NOENC, NULL}, // pin 2
    { 25, NOSTATUS, NOSTATUS, 3, MC_MODULE4, MCPWM_MODULEA, NOENC, NOENC, NULL}, // pin 3
    {112, NOSTATUS, NOSTATUS, 4, NOPWM, NOPWM, NOENC, NOENC, NULL}, // pin 4
    { 27, NOSTATUS, NOSTATUS, 5, MC_MODULE4, MCPWM_MODULEB, NOENC, NOENC, NULL}, // pin 5
    { 28, NOSTATUS, NOSTATUS, 6, MC_MODULE4, MCPWM_MODULEC, NOENC, NOENC, NULL}, // pin 6
    {113, NOSTATUS, NOSTATUS, 7, NOPWM, NOPWM, NOENC, NOENC, NULL}, // pin 7
    {114, NOSTATUS, NOSTATUS, 8, NOPWM, NOPWM, NOENC, NOENC, NULL}, // pin 8
    { 31, NOSTATUS, NOSTATUS, 9, MC_MODULE7, MCPWM_MODULEA, NOENC, NOENC, NULL},  // pin 9
    { 17, NOSTATUS, NOSTATUS, 10, MC_MODULE2, MCPWM_MODULEA, NOENC, NOENC, NULL}, // pin 10
    { 19, NOSTATUS, NOSTATUS, 11, MC_MODULE2, MCPWM_MODULEB, NOENC, NOENC, NULL}, // pin 11
    {115, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, NOENC, NOENC, NULL}, // pin 12
    {116, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, NOENC, NOENC, NULL}, // pin 13
    { 8, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, MC_MODULE1, MCPWM_MODULEB, NULL}, // pin 14
    {10, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, MC_MODULE1, MCPWM_MODULEB, NULL}, // pin 15
    {13, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, MC_MODULE1, MCPWM_MODULEB, NULL}, // pin 16
    { 0, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, MC_MODULE0, MCPWM_MODULEB, NULL}, // pin 17
    { 2, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, MC_MODULE0, MCPWM_MODULEB, NULL}, // pin 18
    { 5, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, MC_MODULE0, MCPWM_MODULEB, NULL}, // pin 19
    { 1, NOSTATUS, NOSTATUS, NOAD, MC_MODULE0, MCPWM_MODULEA, NOENC, NOENC, NULL}, // pin 20
    { 3, NOSTATUS, NOSTATUS, NOAD, MC_MODULE0, MCPWM_MODULEB, NOENC, NOENC, NULL}, // pin 21
    {24, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, MC_MODULE4, MCPWM_MODULEB, NULL}, // pin 22
    {26, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, MC_MODULE4, MCPWM_MODULEB, NULL}, // pin 23
    {29, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, MC_MODULE4, MCPWM_MODULEB, NULL}, // pin 24
    {16, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, MC_MODULE2, MCPWM_MODULEB, NULL}, // pin 25
    {18, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, MC_MODULE2, MCPWM_MODULEB, NULL}, // pin 26
    {21, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, MC_MODULE2, MCPWM_MODULEB, NULL}, // pin 27
    {20, NOSTATUS, NOSTATUS, NOAD, MC_MODULE2, MCPWM_MODULEC, NOENC, NOENC, NULL}, // pin 28
    {23, NOSTATUS, NOSTATUS, NOAD, MC_MODULE3, MCPWM_MODULEC, NOENC, NOENC, NULL}, // pin 29
    { 6, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, MC_MODULE3, MCPWM_MODULEB, NULL}, // pin 30
    {14, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, MC_MODULE3, MCPWM_MODULEB, NULL}, // pin 31
    {22, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, MC_MODULE3, MCPWM_MODULEB, NULL}, // pin 32
    { 9, NOSTATUS, NOSTATUS, NOAD, MC_MODULE1, MCPWM_MODULEA, NOENC, NOENC, NULL}, // pin 33
    {11, NOSTATUS, NOSTATUS, NOAD, MC_MODULE1, MCPWM_MODULEB, NOENC, NOENC, NULL}, // pin 34
    {12, NOSTATUS, NOSTATUS, NOAD, MC_MODULE1, MCPWM_MODULEC, NOENC, NOENC, NULL}, // pin 35
    {15, NOSTATUS, NOSTATUS, NOAD, MC_MODULE3, MCPWM_MODULEB, NOENC, NOENC, NULL}, // pin 36
    { 4, NOSTATUS, NOSTATUS, NOAD, MC_MODULE0, MCPWM_MODULEC, NOENC, NOENC, NULL}, // pin 37
    { 7, NOSTATUS, NOSTATUS, NOAD, MC_MODULE3, MCPWM_MODULEA, NOENC, NOENC, NULL}, // pin 38
    {93, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, NOENC, NOENC, NULL}, // pin 39
    {90, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, NOENC, NOENC, NULL}, // pin 40
    {88, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, NOENC, NOENC, NULL}, // pin 41
    {32, NOSTATUS, NOSTATUS, 0, NOPWM, NOPWM, NOENC, NOENC, NULL}, // pin 42
    {33, NOSTATUS, NOSTATUS, 1, NOPWM, NOPWM, NOENC, NOENC, NULL}, // pin 43
    {34, NOSTATUS, NOSTATUS, 2, NOPWM, NOPWM, NOENC, NOENC, NULL}, // pin 44
    {35, NOSTATUS, NOSTATUS, 3, NOPWM, NOPWM, NOENC, NOENC, NULL}, // pin 45
    {40, NOSTATUS, NOSTATUS, 4, NOPWM, NOPWM, NOENC, NOENC, NULL}, // pin 46
    {41, NOSTATUS, NOSTATUS, 5, NOPWM, NOPWM, NOENC, NOENC, NULL}, // pin 47
    {42, NOSTATUS, NOSTATUS, 6, NOPWM, NOPWM, NOENC, NOENC, NULL}, // pin 48
    {43, NOSTATUS, NOSTATUS, 7, NOPWM, NOPWM, NOENC, NOENC, NULL}, // pin 49
    {44, NOSTATUS, NOSTATUS, 8, NOPWM, NOPWM, NOENC, NOENC, NULL}, // pin 50
    {45, NOSTATUS, NOSTATUS, 9, NOPWM, NOPWM, NOENC, NOENC, NULL}, // pin 51
    {46, NOSTATUS, NOSTATUS, 10, NOPWM, NOPWM, NOENC, NOENC, NULL}, // pin 52
    {47, NOSTATUS, NOSTATUS, 11, NOPWM, NOPWM, NOENC, NOENC, NULL}, // pin 53
    {118, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, NOENC, NOENC, NULL}, // pin 54: TX LED
    {119, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, NOENC, NOENC, NULL}, // pin 55: RX LED
    {117, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, NOENC, NOENC, NULL}, // pin 56: IMU-INT
    {94, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, NOENC, NOENC, NULL}, // pin 57: Internal - IMU detector
    {95, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, NOENC, NOENC, NULL} // pin 58:  Internal - IMU detector
};

static int INTPINSMAP[EXTERNAL_NUM_INTERRUPTS] = {17, 18, 19, 14, 15, 16, 25, 26, 27, 30, 31, 32, 22, 23, 24};

#if defined CRB_DEBUGMODE
static int CROSSBARTABLE[CRBTABLE_SIZE] = {0x03, 0x08, 0x02, 0x01, 0x00, 0x09, 0x06, 0x00, 0x00, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                           0x05, 0x06, 0x07, 0x08, 0x01, 0x02, 0x04, 0x03, 0x00, 0x00, 0x00, 0x00, 0x1F, 0x1E, 0x11, 0x12,
                                           0x09, 0x0A, 0x0C, 0x0B, 0x00, 0x00, 0x0E, 0x0D, 0xC0, 0x0B, 0x03, 0xE0, 0xFF, 0x00, 0x00, 0x00,
                                           0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01,
                                           0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                           0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                           0x01, 0x01, 0x01, 0x02, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                           0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01,
                                           0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x08, 0x01, 0x01, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                           0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x08, 0x08, 0x01, 0x01, 0x01, 0x01,
                                           0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01};
#endif
*/


#endif