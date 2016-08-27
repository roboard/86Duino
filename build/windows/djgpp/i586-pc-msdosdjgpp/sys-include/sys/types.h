/* Copyright (C) 2012 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2003 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2002 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2001 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2000 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1999 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1997 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#ifndef __dj_include_sys_types_h_
#define __dj_include_sys_types_h_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __dj_ENFORCE_ANSI_FREESTANDING

#if (defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L) \
  || !defined(__STRICT_ANSI__) || defined(__cplusplus)

#endif /* (__STDC_VERSION__ >= 199901L) || !__STRICT_ANSI__ */

#ifndef __STRICT_ANSI__

#include <sys/djtypes.h>

typedef int             blkcnt_t;
typedef int             blksize_t;
typedef int		dev_t;
typedef unsigned long	fsblkcnt_t;
typedef unsigned long	fsfilcnt_t;
typedef int		ino_t;
typedef int		mode_t;
typedef int		nlink_t;

#ifndef _GID_T
__DJ_gid_t
#define _GID_T
#endif
#ifndef _OFF_T
__DJ_off_t
#define _OFF_T
#endif
#ifndef _OFF64_T
__DJ_off64_t
#define _OFF64_T
#endif
#ifndef _OFFSET_T
__DJ_offset_t
#define _OFFSET_T
#endif
#ifndef _PID_T
__DJ_pid_t
#define _PID_T
#endif
#ifndef _SIZE_T
__DJ_size_t
#define _SIZE_T
#endif
#ifndef _SSIZE_T
__DJ_ssize_t
#define _SSIZE_T
#endif
#ifndef _UID_T
#define _UID_T
__DJ_uid_t
#endif

#ifndef _POSIX_SOURCE

/* Allow including program to override.  */
#ifndef FD_SETSIZE
#define FD_SETSIZE 256
#endif

typedef struct fd_set {
  unsigned char fd_bits [((FD_SETSIZE) + 7) / 8];
} fd_set;

#define FD_SET(n, p)    ((p)->fd_bits[(n) / 8] |= (1 << ((n) & 7)))
#define FD_CLR(n, p)	((p)->fd_bits[(n) / 8] &= ~(1 << ((n) & 7)))
#define FD_ISSET(n, p)	((p)->fd_bits[(n) / 8] & (1 << ((n) & 7)))
#define FD_ZERO(p)	memset ((void *)(p), 0, sizeof (*(p)))

#ifndef _TIME_T
__DJ_time_t
#define _TIME_T
#endif

#endif /* !_POSIX_SOURCE */
#endif /* !__STRICT_ANSI__ */
#endif /* !__dj_ENFORCE_ANSI_FREESTANDING */

#ifndef __dj_ENFORCE_FUNCTION_CALLS
#endif /* !__dj_ENFORCE_FUNCTION_CALLS */

#ifdef __cplusplus
}
#endif

#endif /* !__dj_include_sys_types_h_ */
