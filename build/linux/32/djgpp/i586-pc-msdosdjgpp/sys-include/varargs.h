/* Copyright (C) 2012 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2003 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2001 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2000 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#ifndef __dj_include_vararg_h_
#define __dj_include_vararg_h_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __dj_ENFORCE_ANSI_FREESTANDING

#ifdef __dj_include_stdarg_h_
#error varargs.h and stdarg.h are mutually exclusive
#endif

#include <sys/djtypes.h>

#ifndef _VA_LIST
__DJ_va_list
#define _VA_LIST
#endif

/* For GCC 2.96 or later we use its builtin va_list                        */
/* In most cases these definitions won't be used, because headers provided */
/* by GCC hide them. But we can DTRT if GCC headers are missing.           */
#if ((__GNUC__ == 2) && (__GNUC_MINOR__ >= 96)) || (__GNUC__ >= 3)

#define va_alist     __builtin_va_alist
#define va_dcl int __builtin_va_alist __attribute__((__mode__(__word__))); ...
#define va_start(ap) __builtin_varargs_start((ap))
#define va_end       __builtin_va_end
#define va_arg       __builtin_va_arg


#else /* #if ((__GNUC__ == 2) && (__GNUC_MINOR__ >= 96)) || (__GNUC__ >= 3) */

#define va_alist __dj_last_arg

#define va_dcl int __dj_last_arg;

#define __dj_va_rounded_size(T)  \
  (((sizeof (T) + sizeof (int) - 1) / sizeof (int)) * sizeof (int))

#define va_arg(ap, T) \
    (ap = (va_list) ((char *) (ap) + __dj_va_rounded_size (T)),	\
     *((T *) (void *) ((char *) (ap) - __dj_va_rounded_size (T))))

#define va_end(ap)

#define va_start(ap)  (ap=(char *)(&__dj_last_arg))

#endif /* #if ((__GNUC__ == 2) && (__GNUC_MINOR__ >= 96)) || (__GNUC__ >= 3) */

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
#endif

#endif /* !__dj_include_varargs_h_ */
