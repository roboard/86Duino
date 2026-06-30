#ifndef Arduino_h
#define Arduino_h

#include <string>
#include <vector>
#include <map>
#include <iostream>

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include "binary.h"
#include "itoa.h"

#include "avr/pgmspace.h"
#include "global.h"
#include "io.h"
#include "pins_arduino.h"

#ifdef __cplusplus
extern "C"{
#endif

void yield(void);
void mainLoopRunOneTime(void);
void stopMainLoopTimerInSCoop(void);
void helperYield(void);

#define LOW          (0x00)
#define HIGH         (0x01)
#define CHANGE       (0x02)
#define FALLING      (0x03)
#define RISING       (0x04)

#define INPUT          (0x00)
#define OUTPUT         (0x01)
#define INPUT_PULLUP   (0x02)
#define INPUT_PULLDOWN (0x03)
#define INPUT_NOCHANGE (0x04)

// for 86Duino
#if defined (__86DUINO_EX2)
	#define HYB_BASEADDRESS     (0x00008A00)
	#define CROSSBARBASE        (0x00008000)
	#define NB_GPIO_CBA_REG 	(0xB0)
	#define NB_GPIO_INT_CBA_REG (0xB4)
	#define CONFIG_BASEADDR     (0x0D00)
	extern unsigned short DIR_BA[16];
    extern unsigned short DATA_BA[16];
    extern unsigned short CONTROL_BA[16];
#elif defined (__86DUINO_EX)
    #define SB_CROSSBASE  (0x64)
	#define CROSSBARBASE  (0x0A00)
	#define GPIOCTRLBASE  (0xF100)
	#define GPIODATABASE  (0xF200)
	#define GPIODIRBASE   (0xF202)
#endif

#define SB_FCREG            (0xC0)
#define SB_GPIOBASE         (0x62)

#define PCSPEAKER    (250)

//#define PI 3.1415926535897932384626433832795
#define HALF_PI 1.5707963267948966192313216916398
#define TWO_PI 6.283185307179586476925286766559
#define DEG_TO_RAD 0.017453292519943295769236907684886
#define RAD_TO_DEG 57.295779513082320876798154814105

#define SERIAL   (0x00)
#define DISPLAY  (0x01)

#define LSBFIRST (0)
#define MSBFIRST (1)

#define INTERNAL 3
#define DEFAULT 1
#define EXTERNAL 0

// undefine stdlib's abs if encountered
#ifdef abs
#undef abs
#endif

#ifndef min
	#define min(a,b) ((a)<(b)?(a):(b))
#endif

#ifndef max
	#define max(a,b) ((a)>(b)?(a):(b))
#endif

//#define abs(x) ((x)>0?(x):-(x))
#define constrain(amt,low,high) ((amt)<(low)?(low):((amt)>(high)?(high):(amt)))
#define round(x)                ((x)>=0?(long)((x)+0.5):(long)((x)-0.5))
#define radians(deg)            ((deg)*DEG_TO_RAD)
#define degrees(rad)            ((rad)*RAD_TO_DEG)
#define sq(x)                   ((x)*(x))

#define interrupts()   io_EnableINT()
#define noInterrupts() io_DisableINT()

#define F_CPU                        (300000000L)
#define clockCyclesPerMicrosecond()  (F_CPU / 1000000L)
#define clockCyclesToMicroseconds(a) ((a) / clockCyclesPerMicrosecond())
#define microsecondsToClockCycles(a) ((a) * clockCyclesPerMicrosecond())

#define lowByte(w)  ((uint8_t) ((w) & 0xff))
#define highByte(w) ((uint8_t) ((w) >> 8))

#define bitRead(value, bit)            (((value) >> (bit)) & 0x01)
#define bitSet(value, bit)             ((value) |= (1UL << (bit)))
#define bitClear(value, bit)           ((value) &= ~(1UL << (bit)))
#define bitWrite(value, bit, bitvalue) (bitvalue ? bitSet(value, bit) : bitClear(value, bit))

typedef unsigned short word;

typedef int clockid_t;
  
struct timespec {
    unsigned long long tv_sec;
    unsigned long long tv_nsec;
};

#define bit(b)    (1UL << (b))
#define DEGREE_C    (0)
#define DEGREE_F    (1)

typedef bool boolean;
typedef uint8_t byte;

bool init(void);
void initVariant(void);

void pinMode(uint8_t, uint8_t);
void digitalWrite(uint8_t, uint8_t);
int  digitalRead(uint8_t);
int  digitalPinToInterrupt(int);
void digitalWriteAll(uint32_t);
uint32_t digitalReadAll(void);
int  analogRead(uint8_t);
void analogReference(uint8_t mode);
void analogWrite(uint8_t, uint32_t);
void analogReadResolution(int res);
void analogWriteResolution(int res);
void Close_Pwm(uint8_t);

unsigned long millis(void);
unsigned long micros(void);
#ifndef USE_DOSH
void delay(unsigned long);
#endif
void delayMicroseconds(unsigned long us);
unsigned long pulseIn(uint8_t pin, uint8_t state, unsigned long timeout);

void shiftOut(uint8_t dataPin, uint8_t clockPin, uint8_t bitOrder, uint8_t val);
uint8_t shiftIn(uint8_t dataPin, uint8_t clockPin, uint8_t bitOrder);

void attachInterrupt(uint8_t, void (*userFunc)(void), int mode);
void attachRTInterrupt(uint8_t, void (*userFunc)(void), int mode);
void attachHTInterrupt(uint8_t, void (*userFunc)(void), int mode);
void detachInterrupt(uint8_t);

// wdt1 functions
extern bool rebootByWDT;
void wdt_enable(unsigned long nTime);
void wdt_disable(void);
void wdt_reset(void);
void wdt_init(void);
bool get_wdt_timeout(void);

void setup();
void loop(void);

#if defined (__86DUINO_QEC) || defined (__86DUINO_QEC_M2) || defined (__86DUINO_QEC_7) || defined (__86DUINO_QEC_9) || defined (__86DUINO_QEC_15) || defined (__86DUINO_QEC_M02) || defined (__86DUINO_QEC_PPC104) || defined (__86DUINO_QEC_PPC9) || defined (__86DUINO_PPC9S) || defined (__86DUINO_PPC15S)  || defined (__86DUINO_DUO)
double systemPowerVoltage();
double systemPowerCurrent();
double peripheralPowerVoltage();
double peripheralPowerCurrent();
extern void qecm02_stopCallback() __attribute__((weak));
extern unsigned long TIMER_shiftTime;
#endif

/*
#if defined PLCBOARDID
unsigned short getDeviceID(void);
#endif
*/

#ifdef __cplusplus
} // extern "C"
#endif

#ifdef __cplusplus

double cpuTemperature(void);
double cpuTemperature(int);


extern unsigned long CLOCKS_PER_MICROSEC;
extern unsigned long VORTEX86EX_CLOCKS_PER_MS;

extern bool Global_irq_Init;
extern bool timer1_pin32_isUsed; // defined in tone.cpp

#define Serial4    Serial485
#define Serial6    Serial232

#include "WCharacter.h"
#include "WString.h"
#include "avr/dtostrf.h"
#include "HardwareSerial.h"
#include "_Helper.h"

uint16_t makeWord(uint16_t w);
uint16_t makeWord(byte h, byte l);

#define word(...) makeWord(__VA_ARGS__)
unsigned long pulseIn(uint8_t pin, uint8_t state, unsigned long timeout = 1000000L);

void tone(uint8_t _pin, unsigned int frequency, unsigned long duration = 0);
void tones(uint8_t _pin, unsigned int frequency, unsigned long duration = 0);
void noTone(uint8_t _pin);

// WMath prototypes
long random(long);
long random(long, long);
void randomSeed(unsigned int);
long map(long, long, long, long, long);

// irq setting functions
bool irq_Setting_RequestFPU(int irq);
void irq_Setting_QuitFPU(int irq);

// 86Duino : the status for isInHelper
#define IN_MAIN         (0)
#define IN_MAIN_LOOP    (1)
#define IN_HELPER       (2)
#define IN_SCOOP        (3)

extern unsigned int coroutineState;

extern char* backupBPath;
extern char* internalResourcePath;
extern char* userInternalResourcePath;
extern char* userExternalResourcePath;
extern char* ethercatFirmwarePath;
extern char* _86EVAResourcePath;
extern char* userProgramPath;
extern char* bootloaderPath;
extern char* sdcardPath;
extern char* emmcDiskPath;
extern char* usbDiskPath;

extern char* getBoardName(void);

#endif

#endif
