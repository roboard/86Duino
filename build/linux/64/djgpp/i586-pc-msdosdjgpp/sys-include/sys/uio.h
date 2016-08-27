/* Copyright (C) 2012 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2003 DJ Delorie, see COPYING.DJ for details */
#ifndef __dj_include_sys_uio_h__
#define __dj_include_sys_uio_h__

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef __dj_ENFORCE_ANSI_FREESTANDING

#include <sys/djtypes.h>

#if (defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L) \
  || !defined(__STRICT_ANSI__) || defined(__cplusplus)

#endif /* (__STDC_VERSION__ >= 199901L) || !__STRICT_ANSI__ */

#ifndef __STRICT_ANSI__

#ifndef _SIZE_T
__DJ_size_t
#define _SIZE_T
#endif

#ifndef _SSIZE_T
__DJ_ssize_t
#define _SSIZE_T
#endif

#define IOV_MAX	16
	
struct iovec {
  void   *iov_base; /* Base address of a memory region for I/O */
  size_t  iov_len;  /* Size of memory region                   */
};

extern ssize_t readv (int _fd, const struct iovec *_iov, int _iovcnt);
extern ssize_t writev (int _fd, const struct iovec *_iov, int _iovcnt);

#ifndef _POSIX_SOURCE

#endif /* !_POSIX_SOURCE */
#endif /* !__STRICT_ANSI__ */
#endif /* !__dj_ENFORCE_ANSI_FREESTANDING */

#ifndef __dj_ENFORCE_FUNCTION_CALLS
#endif /* !__dj_ENFORCE_FUNCTION_CALLS */

#ifdef __cplusplus
}
#endif

#endif /* __dj_include_sys_uio_h__ */
