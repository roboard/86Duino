#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#define  USE_COMMON
#include "common.h"

#if defined(DMP_WIN32_MSVC) || defined(DMP_WINCE_MSVC)
	#include <windows.h>
#elif defined(DMP_LINUX)
    #include <unistd.h>
    #include <sys/times.h>
#elif defined(DMP_DOS_BC)
	#include <dos.h>
#elif defined(DMP_DOS_DJGPP)
    #include <time.h>
#elif defined(DMP_DOS_WATCOM)
    #include <time.h>
#endif


/******************  Error Message Functions  ******************/
static int   ERR_Type;
static char  ERR_MsgBuf[512] = {'\0'};

DMPAPI(int) dmpio_GetErrCode(void) {
    return ERR_Type;
}

DMPAPI(char*) dmpio_GetErrMsg(void) {
    return &(ERR_MsgBuf[0]);
}

static FILE* ERR_outputDevice = stderr;

DMPAPI(bool) err_SetLogFile(char* logfile) {
    err_CloseLogFile();

	if (logfile == NULL)
	{
		ERR_outputDevice = NULL;
		return true;
	}

	if ((ERR_outputDevice = fopen(logfile, "w")) != NULL) return true;

	ERR_outputDevice = stderr;
    return false;
}

DMPAPI(void) err_CloseLogFile(void) {
    if ((ERR_outputDevice != stderr) && (ERR_outputDevice != NULL))
        fclose(ERR_outputDevice);

	ERR_outputDevice = stderr;
}

#ifdef _MANAGED
	#pragma managed(push, off)
#endif
DMPAPI_C(void) errmsg(char* fmt, ...) {
    va_list args;

    if (ERR_outputDevice == NULL) return;

	va_start(args, fmt);
	vfprintf(ERR_outputDevice, fmt, args);
	va_end(args);

    fflush(ERR_outputDevice);
}

DMPAPI_C(void) errmsg_exit(char* fmt, ...) {
    va_list args;

    if (ERR_outputDevice != NULL)
    {
	    va_start(args, fmt);
	    vfprintf(ERR_outputDevice, fmt, args);
	    va_end(args);

        fflush(ERR_outputDevice);
    }

	exit(2);
}

DMPAPI_C(void) err_SetMsg(int errtype, char* fmt, ...) {
    char buf[512];
    va_list args;

	va_start(args, fmt);
	vsprintf(buf, fmt, args);
	va_end(args);

	strcpy(ERR_MsgBuf, buf);
	ERR_Type = errtype;
}
#ifdef _MANAGED
	#pragma managed(pop)
#endif
/*-------------- end of Error Message Functions ---------------*/



/******************  Common Timer Functions  *******************/
DMPAPI(unsigned long) timer_nowtime(void) { //in ms
#if defined(DMP_WIN32_MSVC) || defined(DMP_WINCE_MSVC)
	return GetTickCount();
#elif defined(DMP_LINUX)
    static bool usetimer = false;
    static unsigned long long inittime;
    struct tms cputime;

    if (usetimer == false)
    {
        inittime  = (unsigned long long)times(&cputime);
        usetimer = true;
    }

    return (unsigned long)((times(&cputime) - inittime)*1000UL/sysconf(_SC_CLK_TCK));
#elif defined(DMP_DOS_BC)
    static bool usetimer = false;
    static unsigned long far* timeraddr;
    static unsigned long inittime;
    
    if (usetimer == false)
    {
        timeraddr = (unsigned long far*)MK_FP(0x40,0x6c);
        inittime  = *timeraddr;
        usetimer = true;
    }
    
    return ((*timeraddr) - inittime) * 55UL;
#elif defined(DMP_DOS_DJGPP)
    static bool usetimer = false;
    static uclock_t inittime;
    
    if (usetimer == false)
    {
        //inittime  = biostime(0, 0);
        inittime = uclock();
        usetimer = true;
    }
    
    //return (biostime(0, 0) - inittime) * 55UL;
    return (unsigned long)((uclock() - inittime)*1000UL/UCLOCKS_PER_SEC);
#elif defined(DMP_DOS_WATCOM)
	{
		static bool usetimer = false;
		static clock_t inittime;
		
		if (usetimer == false)
		{
			inittime = clock();
			usetimer = true;
		}
		// return (unsigned long) ((clock() - inittime)*1000UL/CLOCKS_PER_SEC);
		return (unsigned long)((double)(clock() - inittime)/CLOCKS_PER_SEC*1000UL);
	}
#else
    #error "ERROR: timer_nowtime() doesn't support the target system!"
#endif
}


DMPAPI(void) delay_ms(unsigned long delaytime) { //delay in ms
#ifdef DMP_DOS_BC
    while (delaytime > 0xffffUL)
    {
        delay(0xffff);
        delaytime = delaytime - 0xffffUL;
    }

    if (delaytime > 0UL) delay((unsigned)delaytime);
#else
	unsigned long begin;
	begin = timer_nowtime();
	delaytime = delaytime + begin;
	if (delaytime < begin)
	{
		while ((long)timer_nowtime() < 0L);
	}
	while (timer_nowtime() < delaytime);
#endif
}

DMPAPI(unsigned long long int) getclocks64(void)
{
    unsigned long nowclocks;
    unsigned long nowclocks_msb;
    
#if defined(DMP_WIN32_MSVC) || defined(DMP_WINCE_MSVC)
    _asm {
        push eax
        push edx
        rdtsc
        mov DWORD PTR nowclocks, eax
        mov DWORD PTR nowclocks_msb, edx
        pop edx
        pop eax 
    }
#elif defined(DMP_DOS_BC30)
    __emit__(0x66); _asm push ax;
    __emit__(0x66); _asm push dx;
    __emit__(0x0f, 0x31);
    __emit__(0x66); _asm mov WORD PTR nowclocks, ax;
    __emit__(0x66); _asm mov WORD PTR nowclocks_msb, dx;
    __emit__(0x66); _asm pop dx;
    __emit__(0x66); _asm pop ax;
#elif defined(DMP_DOS_DJGPP) || defined(DMP_LINUX)
    __asm__ __volatile__ (
        ".byte 0x0f; .byte 0x31"
        : "=a" (nowclocks), "=d" (nowclocks_msb)
        : //no input registers
        : );
#else
	errmsg_exit("ERROR: %s() isn't supported due to unrecognized OS!\n", __FUNCTION__);
#endif

    return (((unsigned long long int)nowclocks_msb << 32) + nowclocks);
}

static unsigned long getclocks(void) {
    unsigned long nowclocks;
 
#if defined(DMP_WIN32_MSVC) || defined(DMP_WINCE_MSVC) || defined(DMP_DOS_WATCOM)
    _asm {
        rdtsc
        mov DWORD PTR nowclocks, eax
        //mov DWORD PTR nowclocks_msb, edx
    }
#elif defined(DMP_DOS_BC)
    __emit__(0x0f, 0x31);
    __emit__(0x66); _asm mov WORD PTR nowclocks, ax;
    //__emit__(0x66); _asm mov WORD PTR nowclocks_msb, dx;
#elif defined(DMP_DOS_DJGPP) || defined(DMP_LINUX)
    __asm__ __volatile__ (
        ".byte 0x0f; .byte 0x31"
        : "=a" (nowclocks) //, "=d"(nowclocks_msb)
        : //no input registers
        : "edx");
#else
	errmsg_exit("ERROR: %s() isn't supported due to unrecognized OS!\n", __FUNCTION__);
#endif

    return nowclocks;
}

DMPAPI(void) delay_us(unsigned long delaytime) { //delay in us
    unsigned long nowclocks;

    #define CLOCKS_PER_MICROSEC (299UL) //only for RoBoard (RB-100)
    nowclocks = getclocks();
    while ((getclocks() - nowclocks)/CLOCKS_PER_MICROSEC < delaytime);
}
/*------------------ end of Timer Functions -------------------*/
