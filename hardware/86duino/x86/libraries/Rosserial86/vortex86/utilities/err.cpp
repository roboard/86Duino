/*******************************************************************************

  err.cpp - DM&P Error Logging Library
  Copyright (c) 2014 AAA <aaa@dmp.com.tw>. All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

  (If you need a commercial license, please contact soc@dmp.com.tw 
   to get more information.)

*******************************************************************************/


#define __ERR_LIB
////////////////////////////////////////////////////////////////////////////////
//    note that most of functions in this lib assume no paging issue when 
//    using them in ISR; so to use this lib in ISR in DJGPP, it is suggested 
//    to employ PMODE/DJ or HDPMI instead of CWSDPMI.
////////////////////////////////////////////////////////////////////////////////



#include <dmpcfg.h>

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#if defined(DMP_DOS_DJGPP) || defined(DMP_DOS_WATCOM)
    #include <unistd.h>  // for fsync()
#elif defined(DMP_LINUX)
    #include <unistd.h>
    #include <pthread.h>
#elif defined(DMP_WINDOWS)
	#include <windows.h>
#endif

#include "io.h"
#include "irq.h"
#include "err.h"

#ifndef DMP_DOS_DJGPP
    #define DPMI_END_OF_LOCKED_FUNC(fname)
    #define DPMI_END_OF_LOCKED_STATIC_FUNC(fname)
#endif


#ifdef DEBUG_WITH_UART
    #include "err_uart.h"
#endif


static FILE* ERR_outputDevice = stderr;
static volatile char* ERR_msgPool = NULL;
#ifdef DMP_DOS_BC
    #define MSGPOOL_SIZE    (1<<4)
#else
    #define MSGPOOL_SIZE    (1<<6)
#endif
static volatile int ERR_msgStart = 0;
static volatile int ERR_msgEnd   = 0;

// lock functions for ERR_msgPool[...]
#if defined(DMP_DOS_DJGPP) || defined(DMP_DOS_WATCOM) || defined(DMP_DOS_BC)

    #define MSGPOOL_LOCK()      io_DisableINT()
    #define MSGPOOL_UNLOCK()    io_RestoreINT()
    #define MSGPOOL_INITLOCK()  (true)
    #define MSGPOOL_FREELOCK()  do {} while(0)

#elif defined(DMP_LINUX)

    static pthread_mutex_t ERR_msgPoolMutex;
    #define MSGPOOL_LOCK()      pthread_mutex_lock(&ERR_msgPoolMutex)
    #define MSGPOOL_UNLOCK()    pthread_mutex_unlock(&ERR_msgPoolMutex)
    #define MSGPOOL_FREELOCK()  pthread_mutex_destroy(&ERR_msgPoolMutex)

    __dmp_inline(bool) MSGPOOL_INITLOCK() {
        if (pthread_mutex_init(&ERR_msgPoolMutex, NULL) != 0) return false; else return true;
    }

#elif defined(DMP_WINDOWS)

    static CRITICAL_SECTION ERR_msgPoolMutex;
    #define MSGPOOL_LOCK()      EnterCriticalSection(&ERR_msgPoolMutex)
    #define MSGPOOL_UNLOCK()    LeaveCriticalSection(&ERR_msgPoolMutex)
    #define MSGPOOL_FREELOCK()  DeleteCriticalSection(&ERR_msgPoolMutex)

    __dmp_inline(bool) MSGPOOL_INITLOCK() {
        InitializeCriticalSection(&ERR_msgPoolMutex);
        return true;
    }

#endif

__dmp_inline(void) print2pool(const char* fmt, va_list args) {
    if (ERR_msgPool == NULL) return;

    MSGPOOL_LOCK();
    {
        #ifdef DMP_DOS_BC
            vsprintf((char*)(&ERR_msgPool[ERR_msgEnd << 8]), fmt, args);
        #else
            vsnprintf((char*)(&ERR_msgPool[ERR_msgEnd << 8]), 256, fmt, args);
        #endif

        #ifdef DEBUG_WITH_UART
            dbur_WriteString((char*)(&ERR_msgPool[ERR_msgEnd << 8]));
        #endif
        
        ERR_msgEnd = (ERR_msgEnd + 1) & MSGPOOL_SIZE;
        if (ERR_msgEnd == ERR_msgStart) ERR_msgStart = (ERR_msgStart + 1) & MSGPOOL_SIZE;
    }
    MSGPOOL_UNLOCK();
}

DMPAPI_C(void) err_print(const char* fmt, ...) {  // can be used without err_Init()
    va_list args;

    va_start(args, fmt);

    if (irq_InInterrupt() == true)
        print2pool(fmt, args);  // actually in DJGPP this isn't really safe if virtual memory isn't disabled :p
    else
    {
        vfprintf(ERR_outputDevice, fmt, args);
        fflush(ERR_outputDevice);
    }

    va_end(args);
} DPMI_END_OF_LOCKED_FUNC(err_print)

DMPAPI_C(void) err_printk(const char* fmt, ...) {
    va_list args;

    va_start(args, fmt);
    print2pool(fmt, args);
    va_end(args);
} DPMI_END_OF_LOCKED_FUNC(err_printk)

#ifdef DEBUG_WITH_UART
DMPAPI(unsigned int) err_getch(void) {
    unsigned int ch;
    while ((ch = dbur_ReadByte()) == 0xffff);
    return ch;
} DPMI_END_OF_LOCKED_FUNC(err_getch)
#endif


DMPAPI(bool) err_Dump2Buf(char* buf) {
    bool has_next_msg;

    buf[0] = '\0';
    if (ERR_msgPool == NULL) return false;

    MSGPOOL_LOCK();
    {
        if (ERR_msgStart != ERR_msgEnd)
        {
            strncpy(buf, (char*)(&ERR_msgPool[ERR_msgStart << 8]), 256);
            ERR_msgStart = (ERR_msgStart + 1) & MSGPOOL_SIZE;
        }
        has_next_msg = (ERR_msgStart != ERR_msgEnd)? true : false;
    }
    MSGPOOL_UNLOCK();

    return has_next_msg;
}

DMPAPI(bool) err_Dump(void) {
    bool has_next_msg;
    char buf[256];

    has_next_msg = err_Dump2Buf(buf);
    err_print(buf);
    return has_next_msg;
}

DMPAPI(void) err_DumpAll(void) {
    while (err_Dump() == true);

    #if defined(DMP_DOS_DJGPP) || defined(DMP_DOS_WATCOM) || defined(DMP_LINUX)
        fsync(fileno(ERR_outputDevice));  // fflush() has no effect in DJGPP
    #endif
}

DMPAPI(bool) err_AutoDump(void) {
    if (atexit(err_DumpAll) == 0) return true; else return false;
}


DMPAPI(bool) err_Init(const char* logfile) {
    #if defined(DMP_DOS_DJGPP)
        DPMI_LOCK_VAR(ERR_msgPool);
        DPMI_LOCK_VAR(ERR_msgStart);
        DPMI_LOCK_VAR(ERR_msgEnd);
        
        DPMI_LOCK_FUNC(err_print);
        DPMI_LOCK_FUNC(err_printk);

        #ifdef DEBUG_WITH_UART
            DPMI_LOCK_FUNC(err_getch);
            DPMI_LOCK_FUNC(dbur_WriteString);
            DPMI_LOCK_FUNC(dbur_ReadByte);
        #endif
    #endif

    if (ERR_msgPool == NULL)
    {
        if (MSGPOOL_INITLOCK() == true) //&&
        if ((ERR_msgPool = (volatile char*)ker_Malloc(MSGPOOL_SIZE << 8)) == NULL)
            MSGPOOL_FREELOCK();

        if (ERR_msgPool != NULL)
        {
            ERR_msgPool[0] = ERR_msgPool[(MSGPOOL_SIZE << 8) - 1] = '\0';
            #ifdef DEBUG_WITH_UART
                dbur_Init();
            #endif
        }
        ERR_msgStart = 0; ERR_msgEnd = 0;
    }

    if ((ERR_outputDevice != stderr) && (ERR_outputDevice != NULL)) fclose(ERR_outputDevice);
	if (logfile == NULL)
	{
		ERR_outputDevice = stderr;
		return true;
	}

	if ((ERR_outputDevice = fopen(logfile, "w")) != NULL) return true;

	ERR_outputDevice = stderr;
    return false;
}

DMPAPI(bool) err_Close(void) {
    if (ERR_msgPool != NULL)
    {
        ker_Mfree((void*)ERR_msgPool);
        ERR_msgPool = NULL;

        MSGPOOL_FREELOCK();

        #ifdef DEBUG_WITH_UART
            dbur_Close();
        #endif
    }

    if ((ERR_outputDevice != stderr) && (ERR_outputDevice != NULL)) fclose(ERR_outputDevice);
	ERR_outputDevice = stderr;
	return true;
}

