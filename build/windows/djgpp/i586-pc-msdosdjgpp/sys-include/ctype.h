/* Copyright (C) 2012 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2003 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2002 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1994 DJ Delorie, see COPYING.DJ for details */
#ifndef __dj_include_ctype_h_
#define __dj_include_ctype_h_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __dj_ENFORCE_ANSI_FREESTANDING

int	isalnum(int c);
int	isalpha(int c);
int	iscntrl(int c);
int	isdigit(int c);
int	isgraph(int c);
int	islower(int c);
int	isprint(int c);
int	ispunct(int c);
int	isspace(int c);
int	isupper(int c);
int	isxdigit(int c);
int	tolower(int c);
int	toupper(int c);

#if (defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L) \
  || !defined(__STRICT_ANSI__) || defined(__cplusplus)

int	isblank(int c);

#endif /* (__STDC_VERSION__ >= 199901L) || !__STRICT_ANSI__ */

#ifndef __dj_ENFORCE_FUNCTION_CALLS
#include <inlines/ctype.ha>
#endif /* !__dj_ENFORCE_FUNCTION_CALLS */

#if (defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L) \
  || !defined(__STRICT_ANSI__) || defined(__cplusplus)

#endif /* (__STDC_VERSION__ >= 199901L) || !__STRICT_ANSI__ */

#ifndef __STRICT_ANSI__

int	_tolower(int c);
int	_toupper(int c);

#ifndef __dj_ENFORCE_FUNCTION_CALLS
#include <inlines/ctype.hp>
#endif /* !__dj_ENFORCE_FUNCTION_CALLS */

#ifndef _POSIX_SOURCE

int	isascii(int c);
int	toascii(int c);

#ifndef __dj_ENFORCE_FUNCTION_CALLS
#include <inlines/ctype.hd>
#endif /* !__dj_ENFORCE_FUNCTION_CALLS */

#endif /* !_POSIX_SOURCE */
#endif /* !__STRICT_ANSI__ */
#endif /* !__dj_ENFORCE_ANSI_FREESTANDING */

#ifdef __cplusplus
}
#endif

#endif /* !__dj_include_ctype_h_ */
