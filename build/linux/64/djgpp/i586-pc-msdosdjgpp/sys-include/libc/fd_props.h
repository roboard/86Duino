/* Copyright (C) 2012 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2003 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2001 DJ Delorie, see COPYING.DJ for details */
#ifndef __dj_include_libc_fdprops_h__
#define __dj_include_libc_fdprops_h__

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __dj_ENFORCE_ANSI_FREESTANDING

#include <string.h>

#if (defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L) \
  || !defined(__STRICT_ANSI__) || defined(__cplusplus)

#endif /* (__STDC_VERSION__ >= 199901L) || !__STRICT_ANSI__ */

#ifndef __STRICT_ANSI__

#ifndef _POSIX_SOURCE

/* Delete file when the last descriptor referencing it is closed.  */
#define FILE_DESC_TEMPORARY         0x01

/* Tell write and _write to test for file offset greater than EOF.  If so,
   they will fill the gap with zeroes.  */
#define FILE_DESC_ZERO_FILL_EOF_GAP 0x02

/* Set when there can't be an EOF gap or it should be left alone.  */
#define FILE_DESC_DONT_FILL_EOF_GAP 0x04

/* Set when the descriptor is used for pipe emulation.  */
#define FILE_DESC_PIPE              0x08

/* Set when the descriptor is opened for append only. */
#define FILE_DESC_APPEND            0x10

/* Set when the descriptor is used for directory emulation. */
#define FILE_DESC_DIRECTORY         0x20

typedef struct fd_properties fd_properties;

struct fd_properties
{
  unsigned char ref_count;
  char *filename;
  unsigned long flags;
  fd_properties *prev;
  fd_properties *next;
};

extern fd_properties ** __fd_properties;

int __set_fd_properties(int _fd, const char * _file, int _oflags);
void __dup_fd_properties(int _from, int _to);
int __clear_fd_properties(int _fd);

static __inline__ int __has_fd_properties(int _fd)
{
  return _fd >= 0 && __fd_properties && __fd_properties[_fd];
}

static __inline__ void __set_fd_flags(int _fd, unsigned long _flags)
{
  __fd_properties[_fd]->flags |= _flags;  
}

static __inline__ void __clear_fd_flags(int _fd, unsigned long _flags)
{
  __fd_properties[_fd]->flags &= ~_flags;
}

static __inline__ unsigned long __get_fd_flags(int _fd)
{
  return __has_fd_properties(_fd) ? __fd_properties[_fd]->flags : 0;
}

static __inline__ const char * __get_fd_name(int _fd)
{
  return __has_fd_properties(_fd) ? __fd_properties[_fd]->filename : NULL;
}

#endif /* !_POSIX_SOURCE */
#endif /* !__STRICT_ANSI__ */
#endif /* !__dj_ENFORCE_ANSI_FREESTANDING */

#ifdef __cplusplus
}
#endif

#endif /* __dj_include_libc_fdprops_h__  */
