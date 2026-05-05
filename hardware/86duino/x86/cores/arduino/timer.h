#ifndef __WZ_TIMER_H
#define __WZ_TIMER_H

#if defined(DJGPP)
    #include "io.h"
#else
    #include <time.h>
#endif
#if defined(DJGPP)
    #include "io.h"
#elif defined(__GNUC__)
    #include <time.h>
#elif defined(_MSC_VER)
    #include <windows.h>
#endif

typedef struct _timer {
    unsigned long long start_time;
    unsigned long long stop_time;
} timer_t;

static inline unsigned long long __timer_get_current_time()
{
#if defined(DJGPP)
    return timer_GetClocks64();
#elif defined(__GNUC__)
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000000 + ts.tv_nsec / 1000;
#elif defined(_MSC_VER)
    FILETIME fTime;
    GetSystemTimePreciseAsFileTime(&fTime);
    return (((unsigned long long)fTime.dwHighDateTime << 32) | fTime.dwLowDateTime) / 10;
#else
    return 0;
#endif
}

static inline unsigned long long __timer_add_usec(unsigned long long time, unsigned long us)
{
#if defined(DJGPP)
    return time + (unsigned long long)us * vx86_CpuCLK();
#else
    return time + us;
#endif
}

static inline void timer_start(timer_t *tm, unsigned long timeout_us)
{
    tm->start_time = __timer_get_current_time();
    tm->stop_time = __timer_add_usec(tm->start_time, timeout_us);
}

static inline void timer_restart(timer_t *tm, unsigned long timeout_us)
{
    tm->stop_time = __timer_add_usec(tm->stop_time, timeout_us);
}

static inline int timer_expired(timer_t *tm)
{
    return __timer_get_current_time() >= tm->stop_time;
}

#endif
