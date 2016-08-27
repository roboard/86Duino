/* Copyright (C) 2015 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2013 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2011 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2003 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2000 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1999 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1998 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#ifndef __dj_include_time_h_
#define __dj_include_time_h_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __dj_ENFORCE_ANSI_FREESTANDING

/* 65536(tics/hour) / 3600(sec/hour) * 5(scale) = 91.02
   The 5 is to make it a whole number (18.2*5=91) so that
   floating point ops aren't required to use it. */
#define CLOCKS_PER_SEC	91

#include <sys/djtypes.h>
  
/* Some programs think they know better... */
#undef NULL
#if (__GNUC__ >= 4) && defined(__cplusplus)
#  define NULL          __null
#elif defined(__cplusplus)
#  define NULL          0
#else
#  define NULL          ((void*)0)
#endif

#ifndef _CLOCK_T
__DJ_clock_t
#define _CLOCK_T
#endif
#ifndef _SIZE_T
__DJ_size_t
#define _SIZE_T
#endif
#ifndef _TIME_T
__DJ_time_t
#define _TIME_T
#endif

struct tm {
  int tm_sec;
  int tm_min;
  int tm_hour;
  int tm_mday;
  int tm_mon;
  int tm_year;
  int tm_wday;
  int tm_yday;
  int tm_isdst;
  char *__tm_zone;
  int __tm_gmtoff;
};

char *		asctime(const struct tm *_tptr);
clock_t		clock(void);
char *		ctime(const time_t *_cal);
double		difftime(time_t _t1, time_t _t0);
struct tm *	gmtime(const time_t *_tod);
struct tm *	localtime(const time_t *_tod);
time_t		mktime(struct tm *_tptr);
size_t		strftime(char *  _s, size_t _n, const char *  _format, const struct tm *  _tptr);
time_t		time(time_t *_tod);

#if (defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L) \
  || !defined(__STRICT_ANSI__) || defined(__cplusplus)

#endif /* (__STDC_VERSION__ >= 199901L) || !__STRICT_ANSI__ */

#ifndef __STRICT_ANSI__

#define CLK_TCK	CLOCKS_PER_SEC

extern char *tzname[2];

char *		asctime_r(const struct tm * __restrict__ _tptr, char * __restrict__ _buf);
char *		ctime_r(const time_t *_cal, char *_buf);
struct tm *	gmtime_r(const time_t * __restrict__ _tod, struct tm * __restrict__ _tptr);
struct tm * 	localtime_r(const time_t * __restrict__ _tod, struct tm * __restrict__ _tptr);
void		tzset(void);

#ifndef _POSIX_SOURCE

#define tm_zone __tm_zone
#define tm_gmtoff __tm_gmtoff

struct timeval {
  time_t tv_sec;
  long tv_usec;
};

struct timezone {
  int tz_minuteswest;
  int tz_dsttime;
};

#include <sys/types.h>

typedef long long uclock_t;
#define UCLOCKS_PER_SEC 1193180

int		gettimeofday(struct timeval *_tp, struct timezone *_tzp);
unsigned long	rawclock(void);
int		select(int _nfds, fd_set *_readfds, fd_set *_writefds, fd_set *_exceptfds, struct timeval *_timeout);
int		settimeofday(struct timeval *_tp, ...);
void		tzsetwall(void);
uclock_t	uclock(void);

#if ((__GNUC__ == 4 && __GNUC_MINOR__ >= 8) || (__GNUC__ > 4))
/* GCC-4.8.0 and newer versions defines _rdtsc as macro in ia32intrin.h
   included from x86intrin.h (for MMX/SSE/AVX instructions. */
#undef _rdtsc
#endif

#include <sys/cdefs.h>

unsigned long long _rdtsc(void);

_EXTERN_INLINE unsigned long long
_rdtsc(void)
{
#if ((__GNUC__ == 4 && __GNUC_MINOR__ >= 8) || (__GNUC__ > 4))
  return __builtin_ia32_rdtsc();
#else
  unsigned long long result;
  __asm__ __volatile__ ("rdtsc" : "=A"(result) );
  return result;
#endif
}

#endif /* !_POSIX_SOURCE */
#endif /* !__STRICT_ANSI__ */
#endif /* !__dj_ENFORCE_ANSI_FREESTANDING */

#ifndef __dj_ENFORCE_FUNCTION_CALLS
#endif /* !__dj_ENFORCE_FUNCTION_CALLS */

#ifdef __cplusplus
}
#endif

#endif /* !__dj_include_time_h_ */
