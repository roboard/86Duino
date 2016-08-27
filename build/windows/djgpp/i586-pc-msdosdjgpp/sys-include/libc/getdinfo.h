/* Copyright (C) 2012 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2003 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2002 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2001 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2000 DJ Delorie, see COPYING.DJ for details */
#ifndef __dj_include_libc_getdinfo_h__
#define __dj_include_libc_getdinfo_h__

#ifndef __dj_ENFORCE_ANSI_FREESTANDING

#if (defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L) \
  || !defined(__STRICT_ANSI__) || defined(__cplusplus)

#endif /* (__STDC_VERSION__ >= 199901L) || !__STRICT_ANSI__ */

#ifndef __STRICT_ANSI__

#ifndef _POSIX_SOURCE

/* For parsing the bits returned by _get_dev_info.  */
#define _DEV_STDIN	0x0001
#define _DEV_STDOUT	0x0002
#define _DEV_NUL	0x0004
#define _DEV_CLOCK	0x0008
#define _DEV_RAW	0x0020
#define _DEV_CDEV	0x0080
#define _DEV_NO_INHERIT	0x1000 /* Undocumented. */
#define _DEV_IOCTRL	0x4000
#define _DEV_REMOTE	0x8000

#endif /* !_POSIX_SOURCE */
#endif /* !__STRICT_ANSI__ */
#endif /* !__dj_ENFORCE_ANSI_FREESTANDING */

#endif
