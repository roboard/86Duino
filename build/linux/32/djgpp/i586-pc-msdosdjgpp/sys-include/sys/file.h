/* Copyright (C) 2012 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2003 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2001 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#ifndef __dj_include_sys_file_h_
#define __dj_include_sys_file_h_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __dj_ENFORCE_ANSI_FREESTANDING

#if (defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L) \
  || !defined(__STRICT_ANSI__) || defined(__cplusplus)

#endif /* (__STDC_VERSION__ >= 199901L) || !__STRICT_ANSI__ */

#ifndef __STRICT_ANSI__

#ifndef _POSIX_SOURCE

#endif /* !_POSIX_SOURCE */
#endif /* !__STRICT_ANSI__ */
#endif /* !__dj_ENFORCE_ANSI_FREESTANDING */

#include <fcntl.h>
#include <unistd.h>
#define L_SET  0
#define L_CURR 1
#define L_INCR 1
#define L_XTND 2

/* Operations for the `flock' call.  */
#define        LOCK_SH 1       /* Shared lock.  */
#define        LOCK_EX 2       /* Exclusive lock.  */
#define        LOCK_UN 8       /* Unlock.  */

/* Can be OR'd in to one of the above.  */
#define        LOCK_NB 4       /* Don't block when locking.  */

/* Apply or remove an advisory lock, according to OPERATION,
   on the file FD refers to.  */
int	flock (int _fildes, int _op);
  
#ifndef __dj_ENFORCE_FUNCTION_CALLS
#endif /* !__dj_ENFORCE_FUNCTION_CALLS */

#ifdef __cplusplus
}
#endif

#endif /* !__dj_include_sys_file_h_ */
