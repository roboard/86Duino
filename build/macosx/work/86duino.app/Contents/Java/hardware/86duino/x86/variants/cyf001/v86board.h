#ifndef __V86_CLOCK_H
#define __V86_CLOCK_H

#include <stdlib.h>
#include <stdint.h>

#define __86DUINO_CYF001 // for libraries
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
	
#define PINS                       (78) 
#define EXTERNAL_NUM_INTERRUPTS    (24)
#define CRBTABLE_SIZE              (0x130)

#define ENCSSI_MODE_AVAILABLE

// #define CRB_DEBUGMODE

static const uint8_t SS   = 10;
static const uint8_t MOSI = 11;
static const uint8_t MISO = 12;
static const uint8_t SCK  = 13;

static const uint8_t LED_BUILTIN = 13;

static const uint8_t A0 = 18;
static const uint8_t A1 = 19;
static const uint8_t A2 = 20;
static const uint8_t A3 = 21;

static struct pinsconfig PIN86[PINS] = 
{
    {0, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, NOENC, NOENC, NULL}, // pin 0
    {1, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, NOENC, NOENC, NULL}, // pin 1
    {2, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, NOENC, NOENC, NULL}, // pin 2
    {8, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, NOENC, NOENC, NULL}, // pin 3
    {9, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, NOENC, NOENC, NULL}, // pin 4
    {10, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, NOENC, NOENC, NULL}, // pin 5
    {24, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, NOENC, NOENC, NULL}, // pin 6
    {25, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, NOENC, NOENC, NULL}, // pin 7
    {26, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, NOENC, NOENC, NULL}, // pin 8
    {27, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, NOENC, NOENC, NULL}, // pin 9
    {28, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, NOENC, NOENC, NULL}, // pin 10
    {29, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, NOENC, NOENC, NULL}, // pin 11
    {30, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, NOENC, NOENC, NULL}, // pin 12
    {31, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, NOENC, NOENC, NULL}, // pin 13
    {32, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, NOENC, NOENC, NULL}, // pin 14
    {33, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, NOENC, NOENC, NULL}, // pin 15
    {34, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, NOENC, NOENC, NULL}, // pin 16
    {35, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, NOENC, NOENC, NULL}, // pin 17
    {40, NOSTATUS, NOSTATUS, 4, NOPWM, NOPWM, NOENC, NOENC, NULL},    // pin 18
    {41, NOSTATUS, NOSTATUS, 5, NOPWM, NOPWM, NOENC, NOENC, NULL},    // pin 19
    {42, NOSTATUS, NOSTATUS, 6, NOPWM, NOPWM, NOENC, NOENC, NULL},    // pin 20
    {43, NOSTATUS, NOSTATUS, 7, NOPWM, NOPWM, NOENC, NOENC, NULL},    // pin 21
    {48, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, NOENC, NOENC, NULL},// pin 22
    {49, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, NOENC, NOENC, NULL},// pin 23
    {50, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, NOENC, NOENC, NULL},// pin 24
    {51, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, NOENC, NOENC, NULL},// pin 25
    {52, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, NOENC, NOENC, NULL},// pin 26
    {53, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, NOENC, NOENC, NULL},// pin 27
    {54, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, NOENC, NOENC, NULL},// pin 28
    {55, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, NOENC, NOENC, NULL},// pin 29
    {64, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, NOENC, NOENC, NULL},// pin 30
    {65, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, NOENC, NOENC, NULL},// pin 31
    {66, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, NOENC, NOENC, NULL},// pin 32
    {67, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, NOENC, NOENC, NULL},// pin 33
    {68, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, NOENC, NOENC, NULL},// pin 34
    {69, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, NOENC, NOENC, NULL},// pin 35
    {70, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, NOENC, NOENC, NULL},// pin 36
    {71, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, NOENC, NOENC, NULL},// pin 37
    {72, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, NOENC, NOENC, NULL},// pin 38
    {73, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, NOENC, NOENC, NULL},// pin 39
    {74, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, NOENC, NOENC, NULL},// pin 40
    {75, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, NOENC, NOENC, NULL},// pin 41
    {76, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, NOENC, NOENC, NULL},// pin 42
    {77, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, NOENC, NOENC, NULL},// pin 43
    {78, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, NOENC, NOENC, NULL},// pin 44
    {79, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, NOENC, NOENC, NULL},// pin 45
    {80, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, NOENC, NOENC, NULL},// pin 46
    {81, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, NOENC, NOENC, NULL},// pin 47
    {82, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, NOENC, NOENC, NULL},// pin 48
    {83, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, NOENC, NOENC, NULL},// pin 49
    {84, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, NOENC, NOENC, NULL},// pin 50
    {85, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, NOENC, NOENC, NULL},// pin 51
    {86, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, NOENC, NOENC, NULL},// pin 52
    {87, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, NOENC, NOENC, NULL},// pin 53
    {88, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, MC_MODULE0, MCSIF_MODULEB, NULL}, // pin 54
    {89, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, MC_MODULE0, MCSIF_MODULEA, NULL}, // pin 55
    {90, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, MC_MODULE0, MCSIF_MODULEB, NULL}, // pin 56
    {91, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, MC_MODULE0, MCSIF_MODULEA, NULL}, // pin 57
    {92, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, MC_MODULE0, MCSIF_MODULEA, NULL}, // pin 58
    {93, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, MC_MODULE0, MCSIF_MODULEB, NULL}, // pin 59
    {94, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, MC_MODULE3, MCSIF_MODULEB, NULL}, // pin 60
    {95, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, MC_MODULE3, MCSIF_MODULEA, NULL}, // pin 61
    {96, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, MC_MODULE1, MCSIF_MODULEB, NULL}, // pin 62
    {97, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, MC_MODULE1, MCSIF_MODULEA, NULL}, // pin 63
    {98, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, MC_MODULE1, MCSIF_MODULEB, NULL}, // pin 64
    {99, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, MC_MODULE1, MCSIF_MODULEA, NULL}, // pin 65
    {100, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, MC_MODULE1, MCSIF_MODULEA, NULL}, // pin 66
    {101, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, MC_MODULE1, MCSIF_MODULEB, NULL}, // pin 67
    {102, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, MC_MODULE3, MCSIF_MODULEB, NULL}, // pin 68
    {103, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, MC_MODULE3, MCSIF_MODULEA, NULL}, // pin 69
    {104, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, MC_MODULE2, MCSIF_MODULEB, NULL}, // pin 70
    {105, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, MC_MODULE2, MCSIF_MODULEA, NULL}, // pin 71
    {106, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, MC_MODULE2, MCSIF_MODULEB, NULL}, // pin 72
    {107, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, MC_MODULE2, MCSIF_MODULEA, NULL}, // pin 73
    {108, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, MC_MODULE2, MCSIF_MODULEA, NULL}, // pin 74
    {109, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, MC_MODULE2, MCSIF_MODULEB, NULL}, // pin 75
    {110, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, MC_MODULE3, MCSIF_MODULEB, NULL}, // pin 76
    {111, NOSTATUS, NOSTATUS, NOAD, NOPWM, NOPWM, MC_MODULE3, MCSIF_MODULEA, NULL}  // pin 77
};

static int INTPINSMAP[EXTERNAL_NUM_INTERRUPTS] = {54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77};

#if defined CRB_DEBUGMODE                 //0x00,    0x01,,   0x02,,   0x03,,   0x04,,   0x05,,   0x06,,    0x07,,   0x08,,   0x09,,   0x0A,,   0x0B,,   0x0C,   0x0D,,   0x0E,,   0x0F,
static int CROSSBARTABLE[CRBTABLE_SIZE] = {0x01, 0x0A, 0x02, 0x0C, 0x00, 0x03, 0x0F, 0x10, 0x00, 0x00, 0x1B, 0x1C, 0x00, 0x00, 0x00, 0x00, //0x00
                                         0x00, 0x00, 0x00, 0x00, 0x2E, 0x2F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x13, 0x14, 0x15, 0x16, 0x17,//0x10
                                         0x00, 0x00, 0x00, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,//0x20
                                         0x0D, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x14, 0x15,//0x30
                                         0x01, 0x02, 0x03, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,//0x40
                                         0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,//0x50
                                         0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,//0x60
                                         0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,//0x70
                                         0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,//0x80
                                         0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x34, 0x34, 0x34, 0x34, 0x34, 0x34, 0x34, 0x34,//0x90
                                         0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,//0xA0
                                         0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,//0xB0
                                         0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,//0xC0
                                         0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,//0xD0
                                         0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,//0xE0
                                         0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x08, 0x08, 0x01, 0x01, 0x08, 0x08, 0x04, 0x04,//0xF0
                                         0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,//0x100
                                         0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,//0x110
                                         0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01};//0x120
#endif

#endif