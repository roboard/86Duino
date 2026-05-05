#ifndef __V86_CLOCK_H
#define __V86_CLOCK_H

#include <stdlib.h>
#include <stdint.h>

#define __86DUINO_QEC_M02 // for libraries
#define __86DUINO_EX2  // for cores
#define __86DUINO_PLC  // for cores
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

#define NOUSED         (-1)
#define NOAD           (-1)
#define NOPWM          (-1)
#define NOENC          (-1)
#define NOSTATUS       (-1)
#define LOW            (0x00)
#define HIGH           (0x01)
#define INPUT          (0x00)
#define OUTPUT         (0x01)
#define INPUT_PULLUP   (0x02)
#define INPUT_PULLDOWN (0x03)
#define INPUT_NOCHANGE (0x04)

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
	
#define PINS                       (72)
#define EXTERNAL_NUM_INTERRUPTS    (6)
#define CRBTABLE_SIZE              (0x130)
#define FLASH_CRASH_LED_PIN        (68)

static const uint8_t SS   = 10;
static const uint8_t MOSI = 11;
static const uint8_t MISO = 12;
static const uint8_t SCK  = 13;

static const uint8_t LED_BUILTIN = 0;
static const uint8_t POWER_LED_GREEN = 69;
static const uint8_t POWER_LED_RED = 68;

static const uint8_t A0 = 64;
static const uint8_t A1 = 65;
static const uint8_t A2 = 66;
static const uint8_t A7 = 67; // Thermistor

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
static const uint8_t ISA = 100;

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
    { NOUSED, NOSTATUS, NOSTATUS, 0, NOPWM, NOPWM, NOENC, NOENC, NULL}, // output pin 8 
    { NOUSED, NOSTATUS, NOSTATUS, 0, NOPWM, NOPWM, NOENC, NOENC, NULL}, // output pin 9 
    { NOUSED, NOSTATUS, NOSTATUS, 0, NOPWM, NOPWM, NOENC, NOENC, NULL}, // output pin 10 
    { NOUSED, NOSTATUS, NOSTATUS, 0, NOPWM, NOPWM, NOENC, NOENC, NULL}, // output pin 11 
    { NOUSED, NOSTATUS, NOSTATUS, 0, NOPWM, NOPWM, NOENC, NOENC, NULL}, // output pin 12 
    { NOUSED, NOSTATUS, NOSTATUS, 0, NOPWM, NOPWM, NOENC, NOENC, NULL}, // output pin 13 
    { NOUSED, NOSTATUS, NOSTATUS, 0, NOPWM, NOPWM, NOENC, NOENC, NULL}, // output pin 14 
    { NOUSED, NOSTATUS, NOSTATUS, 0, NOPWM, NOPWM, NOENC, NOENC, NULL}, // output pin 15
    { NOUSED, NOSTATUS, NOSTATUS, 0, NOPWM, NOPWM, NOENC, NOENC, NULL}, // output pin 16
    { NOUSED, NOSTATUS, NOSTATUS, 0, NOPWM, NOPWM, NOENC, NOENC, NULL}, // output pin 17
    { NOUSED, NOSTATUS, NOSTATUS, 0, NOPWM, NOPWM, NOENC, NOENC, NULL}, // output pin 18
    { NOUSED, NOSTATUS, NOSTATUS, 0, NOPWM, NOPWM, NOENC, NOENC, NULL}, // output pin 19
    { NOUSED, NOSTATUS, NOSTATUS, 0, NOPWM, NOPWM, NOENC, NOENC, NULL}, // output pin 20
    { NOUSED, NOSTATUS, NOSTATUS, 0, NOPWM, NOPWM, NOENC, NOENC, NULL}, // output pin 21
    { NOUSED, NOSTATUS, NOSTATUS, 0, NOPWM, NOPWM, NOENC, NOENC, NULL}, // output pin 22
    { NOUSED, NOSTATUS, NOSTATUS, 0, NOPWM, NOPWM, NOENC, NOENC, NULL}, // output pin 23
    { NOUSED, NOSTATUS, NOSTATUS, 0, NOPWM, NOPWM, NOENC, NOENC, NULL}, // output pin 24
    { NOUSED, NOSTATUS, NOSTATUS, 0, NOPWM, NOPWM, NOENC, NOENC, NULL}, // output pin 25
    { NOUSED, NOSTATUS, NOSTATUS, 0, NOPWM, NOPWM, NOENC, NOENC, NULL}, // output pin 26 
    { NOUSED, NOSTATUS, NOSTATUS, 0, NOPWM, NOPWM, NOENC, NOENC, NULL}, // output pin 27 
    { NOUSED, NOSTATUS, NOSTATUS, 0, NOPWM, NOPWM, NOENC, NOENC, NULL}, // output pin 28 
    { NOUSED, NOSTATUS, NOSTATUS, 0, NOPWM, NOPWM, NOENC, NOENC, NULL}, // output pin 29 
    { NOUSED, NOSTATUS, NOSTATUS, 0, NOPWM, NOPWM, NOENC, NOENC, NULL}, // output pin 30 
    { NOUSED, NOSTATUS, NOSTATUS, 0, NOPWM, NOPWM, NOENC, NOENC, NULL}, // output pin 31
    
    { NOUSED, NOSTATUS, NOSTATUS, 0, NOPWM, NOPWM, NOENC, NOENC, NULL}, // 32 : input pin 0 
    { NOUSED, NOSTATUS, NOSTATUS, 0, NOPWM, NOPWM, NOENC, NOENC, NULL}, // 33 : input pin 1 
    { NOUSED, NOSTATUS, NOSTATUS, 0, NOPWM, NOPWM, NOENC, NOENC, NULL}, // 34 : input pin 2 
    { NOUSED, NOSTATUS, NOSTATUS, 0, NOPWM, NOPWM, NOENC, NOENC, NULL}, // 35 : input pin 3 
    { NOUSED, NOSTATUS, NOSTATUS, 0, NOPWM, NOPWM, NOENC, NOENC, NULL}, // 36 : input pin 4 
    { NOUSED, NOSTATUS, NOSTATUS, 0, NOPWM, NOPWM, NOENC, NOENC, NULL}, // 37 : input pin 5 
    { NOUSED, NOSTATUS, NOSTATUS, 0, NOPWM, NOPWM, NOENC, NOENC, NULL}, // 38 : input pin 6 
    { NOUSED, NOSTATUS, NOSTATUS, 0, NOPWM, NOPWM, NOENC, NOENC, NULL}, // 39 : input pin 7 
    { NOUSED, NOSTATUS, NOSTATUS, 0, NOPWM, NOPWM, NOENC, NOENC, NULL}, // 40 : input pin 8 
    { NOUSED, NOSTATUS, NOSTATUS, 0, NOPWM, NOPWM, NOENC, NOENC, NULL}, // 41 : input pin 9 
    { NOUSED, NOSTATUS, NOSTATUS, 0, NOPWM, NOPWM, NOENC, NOENC, NULL}, // 42 : input pin 10 
    { NOUSED, NOSTATUS, NOSTATUS, 0, NOPWM, NOPWM, NOENC, NOENC, NULL}, // 43 : input pin 11 
    { NOUSED, NOSTATUS, NOSTATUS, 0, NOPWM, NOPWM, NOENC, NOENC, NULL}, // 44 : input pin 12 
    { NOUSED, NOSTATUS, NOSTATUS, 0, NOPWM, NOPWM, NOENC, NOENC, NULL}, // 45 : input pin 13 
    { NOUSED, NOSTATUS, NOSTATUS, 0, NOPWM, NOPWM, NOENC, NOENC, NULL}, // 46 : input pin 14 
    { NOUSED, NOSTATUS, NOSTATUS, 0, NOPWM, NOPWM, NOENC, NOENC, NULL}, // 47 : input pin 15
    { NOUSED, NOSTATUS, NOSTATUS, 0, NOPWM, NOPWM, NOENC, NOENC, NULL}, // 48 : input pin 16
    { NOUSED, NOSTATUS, NOSTATUS, 0, NOPWM, NOPWM, NOENC, NOENC, NULL}, // 49 : input pin 17
    { NOUSED, NOSTATUS, NOSTATUS, 0, NOPWM, NOPWM, NOENC, NOENC, NULL}, // 50 : input pin 18
    { NOUSED, NOSTATUS, NOSTATUS, 0, NOPWM, NOPWM, NOENC, NOENC, NULL}, // 51 : input pin 19
    { NOUSED, NOSTATUS, NOSTATUS, 0, NOPWM, NOPWM, NOENC, NOENC, NULL}, // 52 : input pin 20
    { NOUSED, NOSTATUS, NOSTATUS, 0, NOPWM, NOPWM, NOENC, NOENC, NULL}, // 53 : input pin 21
    { NOUSED, NOSTATUS, NOSTATUS, 0, NOPWM, NOPWM, NOENC, NOENC, NULL}, // 54 : input pin 22
    { NOUSED, NOSTATUS, NOSTATUS, 0, NOPWM, NOPWM, NOENC, NOENC, NULL}, // 55 : input pin 23
    { NOUSED, NOSTATUS, NOSTATUS, 0, NOPWM, NOPWM, NOENC, NOENC, NULL}, // 56 : input pin 24
    { NOUSED, NOSTATUS, NOSTATUS, 0, NOPWM, NOPWM, NOENC, NOENC, NULL}, // 57 : input pin 25
    { NOUSED, NOSTATUS, NOSTATUS, 0, NOPWM, NOPWM, NOENC, NOENC, NULL}, // 58 : input pin 26 
    { NOUSED, NOSTATUS, NOSTATUS, 0, NOPWM, NOPWM, NOENC, NOENC, NULL}, // 59 : input pin 27 
    { NOUSED, NOSTATUS, NOSTATUS, 0, NOPWM, NOPWM, NOENC, NOENC, NULL}, // 60 : input pin 28 
    { NOUSED, NOSTATUS, NOSTATUS, 0, NOPWM, NOPWM, NOENC, NOENC, NULL}, // 61 : input pin 29 
    { NOUSED, NOSTATUS, NOSTATUS, 0, NOPWM, NOPWM, NOENC, NOENC, NULL}, // 62 : input pin 30 
    { NOUSED, NOSTATUS, NOSTATUS, 0, NOPWM, NOPWM, NOENC, NOENC, NULL}, // 63 : input pin 31 
    
    { 40, NOSTATUS, NOSTATUS, 4, NOPWM, NOPWM, NOENC, NOENC, NULL}, // pin 64 
    { 41, NOSTATUS, NOSTATUS, 5, NOPWM, NOPWM, NOENC, NOENC, NULL}, // pin 65
    { 42, NOSTATUS, NOSTATUS, 6, NOPWM, NOPWM, NOENC, NOENC, NULL}, // pin 66 (ADC52) : GPIO IN/OUT port number
    { 47, NOSTATUS, NOSTATUS, 11, NOPWM, NOPWM, NOENC, NOENC, NULL}, // pin 67
    { 22, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, NOENC, NOENC, NULL}, // pin 68, red LED
    { 23, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, NOENC, NOENC, NULL}, // pin 69, green LED
    { 12, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, NOENC, NOENC, NULL}, // pin 70, vs power good
    { 13, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, NOENC, NOENC, NULL} // pin 71, vp power good
};

static int INTPINSMAP[EXTERNAL_NUM_INTERRUPTS] = {0, 3, 2, 0, 0, 0};

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