#include "Arduino.h"
#include <stdlib.h>
#include <stdio.h>

#include <conio.h>
#include <process.h>
#include <signal.h>
#include "irq.h"
#include <sys/exceptn.h>

char* backupBPath = NULL;
char* internalResourcePath = NULL;
char* userInternalResourcePath = NULL;
char* userExternalResourcePath = NULL;
char* ethercatFirmwarePath = NULL;
char* _86EVAResourcePath = NULL;
char* userProgramPath = NULL;
char* bootloaderPath = NULL;
char* sdcardPath = NULL;
char* emmcDiskPath = NULL;
char* usbDiskPath = NULL;

unsigned _stklen = 4096 * 1024;

// Error process
#define LONG_TIME		(1000L)
#define SHORT_TIME	(50L)
#define ATIMESIZE 	(39)

static unsigned int _ledtime[ATIMESIZE+1] = {SHORT_TIME, LONG_TIME, SHORT_TIME, LONG_TIME, SHORT_TIME, LONG_TIME,
                                          SHORT_TIME, SHORT_TIME, SHORT_TIME, SHORT_TIME, SHORT_TIME, SHORT_TIME, SHORT_TIME, SHORT_TIME,
                                          SHORT_TIME, SHORT_TIME, SHORT_TIME, SHORT_TIME, SHORT_TIME, SHORT_TIME, SHORT_TIME, SHORT_TIME,
                                          SHORT_TIME, SHORT_TIME, SHORT_TIME, SHORT_TIME, SHORT_TIME, SHORT_TIME, SHORT_TIME, SHORT_TIME,
                                          SHORT_TIME, SHORT_TIME, SHORT_TIME, SHORT_TIME, SHORT_TIME, SHORT_TIME, SHORT_TIME, SHORT_TIME,
                                          SHORT_TIME, LONG_TIME
                                          };

void error_led_blink(int ledpin) {
	static int led_stat = 0;
	static bool first = true;
	static unsigned long nowledtime = 0L;
	if(first == true)
	{
		nowledtime = timer_NowTime();
		if((led_stat%2) == 0) digitalWrite(ledpin, HIGH); else digitalWrite(ledpin, LOW);
		first = false;
	}
	
	if((timer_NowTime() - nowledtime) > _ledtime[led_stat])
	{
		if(led_stat == ATIMESIZE) led_stat = 0; else led_stat++;
		first = true;
	}
}

extern void __yields(void);
extern unsigned char usbdev_irq;
void _86Duino_error_process(int num) {
	int ledpin = FLASH_CRASH_LED_PIN;
	// disable all irq except usb irq (5)
	for (int i=0; i<16; i++)
    {
        if (i == usbdev_irq || i == 2 || i == 15) continue;
        i8259_DisableIRQ(i);
    }
		
	// print error message
	printf("\nOop, this program is crash :(\n");
	printf("May have a bug in your sketch, check and fix it and upload again.\n");
	
	// led blink pattern
	pinMode(ledpin, OUTPUT);
	while(1)
	{
		error_led_blink(ledpin);
	}
}

// Weak empty variant initialization function.
// May be redefined by variant files.
void initVariant(void) __attribute__((weak));
void initVariant(void) { }

// Weak hook: a linked RTOS library (e.g. Arduino_FreeRTOS) overrides this to
// start its scheduler after setup().  Default is a no-op.
extern "C" void startSchedulerHook(void) __attribute__((weak));
extern "C" void startSchedulerHook(void) { }

static __attribute__((constructor(101))) void _f_init()
{
	init();
	signal(SIGSEGV, _86Duino_error_process);
	signal(SIGFPE, _86Duino_error_process);
}

char unknownBoard[] = "Unknown Board";
char* getBoardName(void) {
    static char* oneShot = NULL;
    char buf[128];
    
    if (oneShot != NULL) return oneShot;
    
    FILE* fp = fopen("A:\\_brdn.v86", "r");
    if (fp == NULL) {
        fp = fopen("B:\\_brdn.v86", "r");
        if (fp == NULL) {
            oneShot = unknownBoard;
            return unknownBoard;
        }
    }
    
    char* boardName = (char*) malloc(128);
    if (fgets(buf, 128, fp) != NULL) {
        
        if (strcmp(buf, "ZERO") == 0) {
            sprintf(boardName, "86Duino Zero");
        } else if (strcmp(buf, "ONE") == 0) {
            sprintf(boardName, "86Duino One");
        } else if (strcmp(buf, "EDUCAKE") == 0) {
            sprintf(boardName, "86Duino Educake");
        } else if (strcmp(buf, "DUO") == 0) {
            sprintf(boardName, "86Duino DUO");
        } else if (strcmp(buf, "QEC") == 0) {
            sprintf(boardName, "QEC-M-043T");
        } else if (strcmp(buf, "QEC7") == 0) {
            sprintf(boardName, "QEC-M-070T");
        } else if (strcmp(buf, "QEC9") == 0) {
            sprintf(boardName, "QEC-M-090T");
        } else if (strcmp(buf, "QEC15") == 0) {
            sprintf(boardName, "QEC-M-150T");
        } else if (strcmp(buf, "QECM2") == 0) {
            sprintf(boardName, "QEC-M-01");
        } else if (strcmp(buf, "QECM02") == 0) {
            sprintf(boardName, "QEC-M-02");
        } else if (strcmp(buf, "QECPPC9") == 0) {
            sprintf(boardName, "QEC-PPC-090T");
        } else if (strcmp(buf, "QEC104") == 0) {
            sprintf(boardName, "QEC-PPC-104T");
        } else if (strcmp(buf, "PPC9S") == 0) {
            sprintf(boardName, "PPC-090T");
        } else if (strcmp(buf, "PPC15S") == 0) {
            sprintf(boardName, "PPC-150T");
        } else if (strcmp(buf, "HMI043T") == 0) {
            sprintf(boardName, "HMI-043T");
        } else {
            sprintf(boardName, "No such board");
        }
        
    } else 
        sprintf(boardName, "Board name is not found");

    fclose(fp);
    oneShot = boardName;
    return oneShot;

}

#include "util/DetectQECInfo.h"
#include "util/QECM02.h"

int _86DuinoState = IN_MAIN;
DPMI_MEMORY_ALL_LOCK(0)
int main(void)
{
	__djgpp_set_ctrl_c(0);

	initVariant();
	
    backupBPath = getenv("BKBPATH");
    internalResourcePath = getenv("IRSPATH");
    userInternalResourcePath = getenv("UIRSPATH");
    userExternalResourcePath = getenv("UERSPATH");
    ethercatFirmwarePath = getenv("ECFPATH");
    _86EVAResourcePath = getenv("EVAPATH");
    userProgramPath = getenv("UPPATH");
    bootloaderPath = getenv("BLPATH");
    sdcardPath = getenv("SDPATH");
    emmcDiskPath = getenv("EMMCPATH");
    usbDiskPath = getenv("USBPATH");
    
    changeQECM02envPath();
    
    // Vs, Vp, Is, Ip detect function
    detectQECInfoInit();
    
    setup();
    
    startSchedulerHook();
    
    _86DuinoState = IN_MAIN_LOOP;
	for (;;)
	{
        QEC_RoutineWork();
        
		loop();
        mainLoopRunOneTime();
        __yields();
        
		if (serialEventRun) serialEventRun();
	}
    
	return 0;
}
