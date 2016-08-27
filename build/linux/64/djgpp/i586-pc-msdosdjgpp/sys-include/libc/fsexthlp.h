/* Copyright (C) 2012 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2003 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2002 DJ Delorie, see COPYING.DJ for details */
#ifndef __dj_include_libc_fsexthlp_h__
#define __dj_include_libc_fsexthlp_h__

#include <stdarg.h>
#include <sys/fsext.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __dj_ENFORCE_ANSI_FREESTANDING

#if (defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L) \
  || !defined(__STRICT_ANSI__) || defined(__cplusplus)

#endif /* (__STDC_VERSION__ >= 199901L) || !__STRICT_ANSI__ */

#ifndef __STRICT_ANSI__

#ifndef _POSIX_SOURCE

/* This is a wrapper for an FSEXT function that allows it to be called
 * with a variable number of arguments. */

static inline int
__FSEXT_func_wrapper (__FSEXT_Function *func,
		      __FSEXT_Fnumber fnum,
		      int *rv,
		      ...)
{
  va_list args;
  int ret;

  va_start(args, rv);
  ret = func(fnum, rv, args);
  va_end(args);

  return(ret);
}

/* This is a wrapper for __FSEXT_call_open_handlers that allows it
 * to be called with a variable number of arguments. */

static inline int
__FSEXT_call_open_handlers_wrapper (__FSEXT_Fnumber fnum, int *rv, ...)
{
  va_list args;
  int ret;

  va_start(args, rv);
  ret = __FSEXT_call_open_handlers(fnum, rv, args);
  va_end(args);

  return(ret);
}

#endif /* !_POSIX_SOURCE */
#endif /* !__STRICT_ANSI__ */
#endif /* !__dj_ENFORCE_ANSI_FREESTANDING */

#ifndef __dj_ENFORCE_FUNCTION_CALLS
#endif /* !__dj_ENFORCE_FUNCTION_CALLS */

#ifdef __cplusplus
}
#endif

#endif /* __dj_include_libc_fsexthlp_h__ */
