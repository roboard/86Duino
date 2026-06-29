/*******************************************************************************

  io_timer.h - Part of DM&P Vortex86 Base I/O Library
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


/*******************************  CPU-Time Utils  *****************************/
#ifdef DMP_DOS_BC
    __dmp_inline(void) rdtsc_getclocks64(unsigned long* nowclocks_lsb, unsigned long* nowclocks_msb) {
        unsigned long tsc_lsb, tsc_msb;

        __emit__(0x66); _asm push ax;
        __emit__(0x66); _asm push dx;

        __emit__(0x0f, 0x31);
        __emit__(0x66); _asm mov WORD PTR tsc_lsb, ax;
        __emit__(0x66); _asm mov WORD PTR tsc_msb, dx;

        __emit__(0x66); _asm pop dx;
        __emit__(0x66); _asm pop ax;
    
        *nowclocks_lsb = tsc_lsb;
        *nowclocks_msb = tsc_msb;
    }

    DMPAPI(void) timer_GetClocks64(unsigned long* nowclocks_lsb, unsigned long* nowclocks_msb) {
        rdtsc_getclocks64(nowclocks_lsb, nowclocks_msb);
    }
#else
    __dmp_inline(unsigned long long) rdtsc_getclocks64(void) {
        unsigned long long nowclocks;

        #if defined(DMP_DOS_DJGPP) || defined(DMP_LINUX)
            __asm__ __volatile__ ("rdtsc" : "=A"(nowclocks) );
        #elif defined(DMP_DOS_WATCOM) || defined(DMP_WIN32_MSVC) || defined(DMP_WINCE_MSVC)
            _asm {
                push eax
                push edx        
                rdtsc  //  not yet tested on older Watcom C/C++
                mov DWORD PTR nowclocks, eax
                lea eax, nowclocks
                mov DWORD PTR [eax+4], edx
                pop edx
                pop eax        
            };
        #else
            #error rdtsc_getclocks64() unrecognizes the target platform...
        #endif

        return nowclocks;
    }

    DMPAPI(unsigned long long) timer_GetClocks64(void) {
        return rdtsc_getclocks64();
    }
#endif

DMPAPI(unsigned long) timer_GetClocks(void) {
    #ifdef DMP_DOS_BC
        unsigned long nowclocks_lsb, nowclocks_msb;
        rdtsc_getclocks64(&nowclocks_lsb, &nowclocks_msb);
        return nowclocks_lsb;
    #else
        return (unsigned long)rdtsc_getclocks64();
    #endif
}

__dmp_inline(void) rdtsc_delay_us(unsigned long us) {  // assuming us < (2^32)/cpu_clocks_per_us
    unsigned long start_clocks = timer_GetClocks();
    for (us = us * vx86_CpuCLK(); (timer_GetClocks() - start_clocks) < us; );
}

DMPAPI(void) timer_Delay(unsigned long ms) {
    for (; ms > 100L; ms = ms - 100L) rdtsc_delay_us(100000L);
    if (ms > 0L) rdtsc_delay_us(ms * 1000L);
}

DMPAPI(void) timer_DelayMicroseconds(unsigned long us) {
    for (; us > 100000L; us = us - 100000L) rdtsc_delay_us(100000L);
    if (us > 0L) rdtsc_delay_us(us);
}

static unsigned long TIMER_initTime = 0L;
DMPAPI(unsigned long) timer_NowTime(void) {
    #ifdef DMP_DOS_BC
        unsigned long nowclocks_l, nowclocks_h, nowtime;
        unsigned long clocks_per_ms = vx86_CpuCLK() * 1000L;
        int i;
        
        rdtsc_getclocks64(&nowclocks_l, &nowclocks_h);

        // assuming clocks_per_ms is a 24-bit integer and using a stupid but simple code to calculate nowclocks/clocks_per_ms:p
        nowtime     = nowclocks_h / clocks_per_ms;
        nowclocks_h = nowclocks_h % clocks_per_ms;
        for (i=3; i>=0; i--)
        {
            nowclocks_h = (nowclocks_h << 8) + (unsigned long)((unsigned char*)&nowclocks_l)[i];
            nowtime     = nowtime << 8 + nowclocks_h / clocks_per_ms;
            nowclocks_h = nowclocks_h % clocks_per_ms;
        }

        return nowtime - TIMER_initTime;
    #elif defined(DMP_DOS_DJGPP) || defined(DMP_DOS_WATCOM)
        return (unsigned long)(rdtsc_getclocks64()/((unsigned long long)vx86_CpuCLK()*1000L)) - TIMER_initTime;
    #elif defined(DMP_WINDOWS)
        LARGE_INTEGER freq;
        LARGE_INTEGER tsc;

        QueryPerformanceFrequency(&freq);
        QueryPerformanceCounter(&tsc);
        return (unsigned long)(tsc.QuadPart*1000L/freq.QuadPart) - TIMER_initTime;
    #elif defined(DMP_LINUX)
        /*
        struct tms cputime;
        return (unsigned long)((unsigned long long)times(&cputime)*1000L/sysconf(_SC_CLK_TCK)) - TIMER_initTime;
        */
        struct timespec cputime;
        clock_gettime(CLOCK_MONOTONIC, &cputime);
        return (unsigned long)((unsigned long long)(cputime.tv_sec)*1000L + (unsigned long long)(cputime.tv_nsec/1000000L)) - TIMER_initTime;
    #else
        #error timer_NowTime() unrecognizes the target platform...
    #endif
}
/*---------------------------  end. CPU-Time Utils  --------------------------*/

