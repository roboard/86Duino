/* Copyright (C) 2012 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2003 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2002 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2001 DJ Delorie, see COPYING.DJ for details */
#ifndef __dj_include_sys_statvfs_h_
#define __dj_include_sys_statvfs_h_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __dj_ENFORCE_ANSI_FREESTANDING

#if (defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L) \
  || !defined(__STRICT_ANSI__) || defined(__cplusplus)

#endif /* (__STDC_VERSION__ >= 199901L) || !__STRICT_ANSI__ */

#ifndef __STRICT_ANSI__

struct statvfs {
  unsigned long f_bsize;
  unsigned long f_frsize;
  fsblkcnt_t    f_blocks;
  fsblkcnt_t    f_bfree;
  fsblkcnt_t    f_bavail;
  fsfilcnt_t    f_files;
  fsfilcnt_t    f_ffree;
  fsfilcnt_t    f_favail;
  unsigned long f_fsid;
  unsigned long f_flag;
  unsigned long f_namemax;
};

#define ST_RDONLY 0x0001
#define ST_NOSUID 0x0002

int fstatvfs (int _fd, struct statvfs *_buf);
int statvfs (const char *_path, struct statvfs *_buf);

#endif /* !__STRICT_ANSI__ */
#endif /* !__dj_ENFORCE_ANSI_FREESTANDING */

#ifdef __cplusplus
}
#endif

#endif /* !__dj_include_sys_statvfs_h_ */
