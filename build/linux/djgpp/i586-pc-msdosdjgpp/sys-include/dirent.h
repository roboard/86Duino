/* Copyright (C) 2012 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2003 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2001 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1998 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#ifndef __dj_include_dirent_h_
#define __dj_include_dirent_h_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __dj_ENFORCE_ANSI_FREESTANDING

#if (defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L) \
  || !defined(__STRICT_ANSI__) || defined(__cplusplus)

#endif /* (__STDC_VERSION__ >= 199901L) || !__STRICT_ANSI__ */

#ifndef __STRICT_ANSI__
#include <sys/types.h>

/* Definition of DIR requires many other headers; not included here to
   avoid namespace pollution. */
typedef struct __dj_DIR DIR;

/* Note: each one of these is exactly one more than the high nibble of
   the corresponding S_IF* constant defined on sys/stat.h.  If you
   break that, DTTOIF will stop working!  */
#define DT_REG      0x1
#define DT_BLK      0x2
#define DT_CHR      0x3
#define DT_DIR      0x4
#define DT_FIFO     0x5
#define DT_LABEL    0x6
#define DT_LNK      0x7
#define DT_SOCK     0x8
#define DT_UNKNOWN  0xf

#define DTTOIF(dt)  (((dt) == DT_UNKNOWN ? 0 : (dt) - 1) << 12)

struct dirent {
  char d_namlen;
  char d_name[256];
  mode_t d_type;
};

int		closedir(DIR *dirp);
DIR *		opendir(const char *_dirname);
struct dirent *	readdir(DIR *_dirp);
void		rewinddir(DIR *_dirp);

#ifndef _POSIX_SOURCE

extern int __opendir_flags; /* default is zero, used only by opendir */
#define __OPENDIR_PRESERVE_CASE	0001
#define __OPENDIR_FIND_HIDDEN	0002 /* ignored; on by default */
#define __OPENDIR_FIND_LABEL	0004
#define __OPENDIR_NO_HIDDEN	0x08 /* NOT 0002 for back-compatibility */
#define __OPENDIR_NO_D_TYPE	0x10

void seekdir(DIR *_dir, long _loc);
long telldir(DIR *_dir);
void __set_need_fake_dot_dotdot(DIR *_dir);
void _lfn_find_close(int _handle);

#endif /* !_POSIX_SOURCE */
#endif /* !__STRICT_ANSI__ */
#endif /* !__dj_ENFORCE_ANSI_FREESTANDING */

#ifndef __dj_ENFORCE_FUNCTION_CALLS
#endif /* !__dj_ENFORCE_FUNCTION_CALLS */

#ifdef __cplusplus
}
#endif

#endif /* !__dj_include_dirent_h_ */
