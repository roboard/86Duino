/* Copyright (C) 2012 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2003 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2002 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2001 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1997 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#ifndef __dj_include_locale_h_
#define __dj_include_locale_h_

#ifdef __cplusplus
namespace std {
extern "C" {
#endif

#ifndef __dj_ENFORCE_ANSI_FREESTANDING

#define LC_ALL		0x1f
#define LC_COLLATE	0x01
#define LC_CTYPE	0x02
#define LC_MONETARY	0x04
#define LC_NUMERIC	0x08
#define LC_TIME		0x10

/* Some programs think they know better... */
#undef NULL
#if (__GNUC__ >= 4) && defined(__cplusplus)
#  define NULL          __null
#elif defined(__cplusplus)
#  define NULL          0
#else
#  define NULL          ((void*)0)
#endif

struct lconv {
  char *currency_symbol;
  char *decimal_point;
  char *grouping;
  char *int_curr_symbol;
  char *mon_decimal_point;
  char *mon_grouping;
  char *mon_thousands_sep;
  char *negative_sign;
  char *positive_sign;
  char *thousands_sep;
  char frac_digits;
  char int_frac_digits;
  char n_cs_precedes;
  char n_sep_by_space;
  char n_sign_posn;
  char p_cs_precedes;
  char p_sep_by_space;
  char p_sign_posn;
  char int_n_cs_precedes;
  char int_n_sep_by_space;
  char int_n_sign_posn;
  char int_p_cs_precedes;
  char int_p_sep_by_space;
  char int_p_sign_posn;
};

struct lconv *	localeconv(void);
char *		setlocale(int _category, const char *_locale);

#if (defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L) \
  || !defined(__STRICT_ANSI__) || defined(__cplusplus)

#endif /* (__STDC_VERSION__ >= 199901L) || !__STRICT_ANSI__ */

#ifndef __STRICT_ANSI__

#ifndef _POSIX_SOURCE

#endif /* !_POSIX_SOURCE */
#endif /* !__STRICT_ANSI__ */
#endif /* !__dj_ENFORCE_ANSI_FREESTANDING */

#ifndef __dj_ENFORCE_FUNCTION_CALLS
#endif /* !__dj_ENFORCE_FUNCTION_CALLS */

#ifdef __cplusplus
}
}
#endif

#endif /* !__dj_include_locale_h_ */


#if defined(__cplusplus) && !defined(__dj_ENFORCE_ANSI_FREESTANDING)
#ifndef __dj_via_cplusplus_header_

using std::lconv;
using std::localeconv;
using std::setlocale;

#endif /* !__dj_via_cplusplus_header_ */
#endif /* __cplusplus && !__dj_ENFORCE_ANSI_FREESTANDING */
