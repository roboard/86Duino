/* Copyright (C) 2015 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2013 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2003 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1999 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1998 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#ifndef __dj_include_math_h_
#define __dj_include_math_h_

#ifdef _USE_LIBM_MATH_H
#include <libm/math.h>
#else

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __dj_ENFORCE_ANSI_FREESTANDING

extern double __dj_huge_val;

#define HUGE_VAL  __dj_huge_val

double acos(double _x);
double asin(double _x);
double atan(double _x);
double atan2(double _y, double _x);
double ceil(double _x);
double cos(double _x);
double cosh(double _x);
double exp(double _x);
double fabs(double _x);
double floor(double _x);
double fmod(double _x, double _y);
double frexp(double _x, int *_pexp);
double ldexp(double _x, int _exp);
double log(double _y);
double log10(double _x);
double log2(double _x);
double modf(double _x, double *_pint);
double pow(double _x, double _y);
double sin(double _x);
double sinh(double _x);
double sqrt(double _x);
double tan(double _x);
double tanh(double _x);

/* The C++ standard has overloaded versions of cos, etc. for floats
 * and long doubles. libstdc++ calls the C99-standardised float
 * and long double versions of cos, etc. in the global namespace
 * to provide the overloaded versions. So we place the float
 * and long double versions in the global namespace for C++
 * even when __STRICT_ANSI__ is defined.
 */
#if (defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L) \
  || !defined(__STRICT_ANSI__) \
  || defined(__cplusplus)

long double modfl(long double _x, long double *_pint);

/* These are in libm.a (Cygnus).  You must link -lm to get these */
/* See libm/math.h for comments */

extern float acosf(float);
extern float asinf(float);
extern float atanf(float);
extern float atan2f(float, float);
extern float cosf(float);
extern float sinf(float);
extern float tanf(float);
extern float coshf(float);
extern float sinhf(float);
extern float tanhf(float);
extern float expf(float);
extern float frexpf(float, int *);
extern float ldexpf(float, int);
extern float logf(float);
extern float log10f(float);
extern float log2f(float);
extern float modff(float, float *);
extern float powf(float, float);
extern float sqrtf(float);
extern float ceilf(float);
extern float fabsf(float);
extern float floorf(float);
extern float fmodf(float, float);

/* End libm.a. */

#endif

#if (defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L) \
  || !defined(__STRICT_ANSI__) || defined(__cplusplus)

extern float       __dj_huge_valf;
extern long double __dj_huge_vall;

#define HUGE_VALF __dj_huge_valf
#define HUGE_VALL __dj_huge_vall

#define INFINITY  HUGE_VALF

extern float       __dj_nan;
#define NAN        __dj_nan

#define FP_INFINITE     0x00000001
#define FP_NAN          0x00000002
#define FP_NORMAL       0x00000004
#define FP_SUBNORMAL    0x00000008
#define FP_ZERO         0x00000010
/* Extended with Unnormals (for long doubles). */
#define FP_UNNORMAL     0x00010000

#define fpclassify(x)         (__extension__ ({__typeof__(x) __xfp = (x); \
                                               (sizeof(__xfp) == sizeof(float)) ? __fpclassifyf(__xfp) :  \
                                               (sizeof(__xfp) == sizeof(double)) ? __fpclassifyd(__xfp) : \
                                               __fpclassifyld(__xfp); \
                                              }))

#define signbit(x)            (__extension__ ({__typeof__(x) __xsb = (x); \
                                               (sizeof(__xsb) == sizeof(float)) ? __signbitf(__xsb) :  \
                                               (sizeof(__xsb) == sizeof(double)) ? __signbitd(__xsb) : \
                                               __signbitld(__xsb); \
                                              }))

#define isfinite(x)           ((fpclassify(x) & (FP_NORMAL | FP_SUBNORMAL | FP_ZERO)) != 0)
#define isinf(x)              (fpclassify(x) == FP_INFINITE)
#define isnan(x)              (fpclassify(x) == FP_NAN)
#define isnormal(x)           (fpclassify(x) == FP_NORMAL)

#define isgreater(x, y)       (__extension__ ({__typeof__(x) __x_ig = (x); \
                                               __typeof__(y) __y_ig = (y); \
                                               !isunordered(__x_ig, __y_ig) && (__x_ig > __y_ig); \
                                              }))
#define isgreaterequal(x, y)  (__extension__ ({__typeof__(x) __x_ige = (x); \
                                               __typeof__(y) __y_ige = (y); \
                                               !isunordered(__x_ige, __y_ige) && (__x_ige >= __y_ige); \
                                              }))
#define isless(x, y)          (__extension__ ({__typeof__(x) __x_il = (x); \
                                               __typeof__(y) __y_il = (y); \
                                               !isunordered(__x_il, __y_il) && (__x_il < __y_il); \
                                              }))
#define islessequal(x, y)     (__extension__ ({__typeof__(x) __x_ile = (x); \
                                               __typeof__(y) __y_ile = (y); \
                                               !isunordered(__x_ile, __y_ile) && (__x_ile <= __y_ile); \
                                              }))
#define islessgreater(x, y)   (__extension__ ({__typeof__(x) __x_ilg = (x); \
                                               __typeof__(y) __y_ilg = (y); \
                                               !isunordered(__x_ilg, __y_ilg) && (__x_ilg < __y_ilg || __x_ilg > __y_ilg); \
                                              }))
#define isunordered(x, y)     (__extension__ ({__typeof__(x) __x_iu = (x); \
                                               __typeof__(y) __y_iu = (y); \
                                               (fpclassify(__x_iu) == FP_NAN) || (fpclassify(__y_iu) == FP_NAN); \
                                              }))

int         __fpclassifyf(float) __attribute__((const));
int         __fpclassifyd(double) __attribute__((const));
int         __fpclassifyld(long double) __attribute__((const));
double      nan(const char *);
float       nanf(const char *);
long double nanl(const char *);
int         __signbitf(float x) __attribute__((const));
int         __signbitd(double x) __attribute__((const));
int         __signbitld(long double x) __attribute__((const));

#define MATH_ERRNO     1
#define MATH_ERREXCEPT 2

int __get_math_errhandling(void);
#define math_errhandling (__get_math_errhandling())

double acosh(double _x);
double asinh(double _x);
double atanh(double _x);
double cbrt(double _x);
double exp2(double _x);
double expm1(double _x);
double hypot(double _x, double _y);
double log1p(double _x);

/* These are in libm.a (Cygnus).  You must link -lm to get these */
/* See libm/math.h for comments */

extern double erf(double);
extern double erfc(double);
extern double lgamma(double);
extern double logb(double);
extern double nextafter(double, double);
extern double remainder(double, double);
extern double copysign(double, double);
extern int ilogb(double);
extern double rint(double);
extern long double rintl(long double);
extern double scalbn(double, int);
extern double trunc(double);
extern long double truncl(long double);
extern long int lrint(double);
extern long int lrintl(long double);
extern long long int llrint(double);
extern long long int llrintl(long double);
extern double round(double);
extern long double roundl(long double);
extern long int lround(double);
extern long int lroundl(long double);
extern long long int llround(double);
extern long long int llroundl(long double);
extern float erff(float);
extern float erfcf(float);
extern float hypotf(float, float);
extern float lgammaf(float);
extern float acoshf(float);
extern float asinhf(float);
extern float atanhf(float);
extern float cbrtf(float);
extern float logbf(float);
extern float nextafterf(float, float);
extern float remainderf(float, float);
extern float scalbf(float, float);
extern float copysignf(float, float);
extern int ilogbf(float);
extern float rintf(float);
extern float scalbnf(float, int);
extern float expm1f(float);
extern float log1pf(float);
extern float truncf(float);
extern long int lrintf(float);
extern long long int llrintf(float);
extern float roundf(float);
extern long int lroundf(float);
extern long long int llroundf(float);
extern int isinfl(long double);
extern int isnanl(long double);
extern int finitel(long double);

/* End libm.a. */

#endif /* (__STDC_VERSION__ >= 199901L) || !__STRICT_ANSI__ */
  
#ifndef __STRICT_ANSI__

#ifndef _POSIX_SOURCE

#define M_E             2.7182818284590452354
#define M_LOG2E         1.4426950408889634074
#define M_LOG10E        0.43429448190325182765
#define M_LN2           0.69314718055994530942
#define M_LN10          2.30258509299404568402
#define M_PI            3.14159265358979323846
#define M_PI_2          1.57079632679489661923
#define M_PI_4          0.78539816339744830962
#define M_1_PI          0.31830988618379067154
#define M_2_PI          0.63661977236758134308
#define M_2_SQRTPI      1.12837916709551257390
#define M_SQRT2         1.41421356237309504880
#define M_SQRT1_2       0.70710678118654752440
#define PI              M_PI
#define PI2             M_PI_2

double exp10(double _x);
double pow10(double _x);
double pow2(double _x);
double powi(double _base, int _exp);
void   sincos(double _x, double *_sin, double *_cos);

/* These are in libm.a (Cygnus).  You must link -lm to get these */
/* See libm/math.h for comments */

#ifndef __cplusplus
struct exception {
  int type;
  const char *name;
  double arg1;
  double arg2;
  double retval;
  int err;
};
#endif

extern double gamma(double);
extern int finite(double);
extern double j0(double);
extern double j1(double);
extern double jn(int, double);
extern double y0(double);
extern double y1(double);
extern double yn(int, double);
extern double scalb(double, double);
#ifndef __cplusplus
extern int matherr(struct exception *);
#endif
extern double significand(double);
extern double drem(double, double);
extern double gamma_r(double, int *);
extern double lgamma_r(double, int *);
extern float gammaf(float);
extern int isinff(float);
extern int isnanf(float);
extern int finitef(float);
extern float j0f(float);
extern float j1f(float);
extern float jnf(int, float);
extern float y0f(float);
extern float y1f(float);
extern float ynf(int, float);
extern float significandf(float);
extern float dremf(float, float);
extern float gammaf_r(float, int *);
extern float lgammaf_r(float, int *);

#endif /* !_POSIX_SOURCE */
#endif /* !__STRICT_ANSI__ */
#endif /* !__dj_ENFORCE_ANSI_FREESTANDING */

#ifndef __dj_ENFORCE_FUNCTION_CALLS
#endif /* !__dj_ENFORCE_FUNCTION_CALLS */

#ifdef __cplusplus
}
#endif

#endif /* _USE_LIBM_MATH_H */

#endif /* !__dj_include_math_h_ */
