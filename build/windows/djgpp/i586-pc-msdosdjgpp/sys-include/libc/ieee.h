/* Copyright (C) 2012 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2003 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#ifndef __dj_include_libc_ieee_h__
#define __dj_include_libc_ieee_h__

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __dj_ENFORCE_ANSI_FREESTANDING

#if (defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L) \
  || !defined(__STRICT_ANSI__) || defined(__cplusplus)

#endif /* (__STDC_VERSION__ >= 199901L) || !__STRICT_ANSI__ */

#ifndef __STRICT_ANSI__

#ifndef _POSIX_SOURCE

typedef struct {
  unsigned mantissa:23;
  unsigned exponent:8;
  unsigned sign:1;
} float_t;

typedef struct {
  unsigned mantissal:32;
  unsigned mantissah:20;
  unsigned exponent:11;
  unsigned sign:1;
} double_t;

typedef struct {
  unsigned mantissal:32;
  unsigned mantissah:32;
  unsigned exponent:15;
  unsigned sign:1;
} long_double_t;


typedef union
{
  float f;
  float_t ft;
} _float_union_t;

typedef union
{
  double d;
  double_t dt;
} _double_union_t;

typedef union
{
  long double ld;
  long_double_t ldt;
} _longdouble_union_t;

typedef union
{
  float f;
  long  l;
} _float_long_union;

#endif /* !_POSIX_SOURCE */
#endif /* !__STRICT_ANSI__ */
#endif /* !__dj_ENFORCE_ANSI_FREESTANDING */

#ifndef __dj_ENFORCE_FUNCTION_CALLS
#endif /* !__dj_ENFORCE_FUNCTION_CALLS */

#ifdef __cplusplus
}
#endif

#endif /* __dj_include_libc_ieee_h__ */
