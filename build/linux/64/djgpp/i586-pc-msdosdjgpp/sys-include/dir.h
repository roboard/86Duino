/* Copyright (C) 2015 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2012 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2003 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1998 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#ifndef __dj_include_dir_h_
#define __dj_include_dir_h_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __dj_ENFORCE_ANSI_FREESTANDING

#if (defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L) \
  || !defined(__STRICT_ANSI__) || defined(__cplusplus)

#endif /* (__STDC_VERSION__ >= 199901L) || !__STRICT_ANSI__ */

#ifndef __STRICT_ANSI__

#ifndef _POSIX_SOURCE

/* ffblk is also enhanced for LFNs; the dos 21 byte reserved area is used to
   hold the extra information.  Fields marked LFN are only valid if the magic
   is set to LFN32 */

struct ffblk {
  char lfn_magic[6];			/* LFN */
  short lfn_handle;			/* LFN */
  unsigned short lfn_ctime;		/* LFN */
  unsigned short lfn_cdate;		/* LFN */
  unsigned short lfn_atime;		/* LFN */
  unsigned short lfn_adate;		/* LFN */
  char _ff_reserved[5];
  unsigned char  ff_attrib;
  unsigned short ff_ftime;
  unsigned short ff_fdate;
  unsigned long  ff_fsize;
  char ff_name[260];
} __attribute__((packed));

struct ffblklfn {
  unsigned long      fd_attrib;
  unsigned long long fd_ctime;
  unsigned long long fd_atime;
  unsigned long long fd_mtime;
  unsigned long      fd_sizehi;
  unsigned long      fd_size;
  unsigned long long fd_reserved;
  char               fd_longname[260];
  char               fd_name[14];
} __attribute__((packed));

/* make sure that structure packing is correct */
typedef int _DJCHK_FFBLK0[(sizeof(struct ffblk)   ==290)*3 - 1];
typedef int _DJCHK_FFBLK1[(sizeof(struct ffblklfn)==318)*3 - 1];

#define FA_RDONLY       1
#define FA_HIDDEN       2
#define FA_SYSTEM       4
#define FA_LABEL        8
#define FA_DIREC        16
#define FA_ARCH         32

/* for fnmerge/fnsplit */
#define MAXPATH   260
#define MAXDRIVE  3
#define MAXDIR	  256
#define MAXFILE   256
#define MAXEXT	  255

#define WILDCARDS 0x01
#define EXTENSION 0x02
#define FILENAME  0x04
#define DIRECTORY 0x08
#define DRIVE	  0x10

int	__file_tree_walk(const char *_dir, int (*_fn)(const char *_path, const struct ffblk *_ff));
int	findfirst(const char *_pathname, struct ffblk *_ffblk, int _attrib);
int	findnext(struct ffblk *_ffblk);
void	fnmerge (char *_path, const char *_drive, const char *_dir, const char *_name, const char *_ext);
int	fnsplit (const char *_path, char *_drive, char *_dir, char *_name, char *_ext);
int	getdisk(void);
char *	searchpath(const char *_program);
int	setdisk(int _drive);

#endif /* !_POSIX_SOURCE */
#endif /* !__STRICT_ANSI__ */
#endif /* !__dj_ENFORCE_ANSI_FREESTANDING */

#ifndef __dj_ENFORCE_FUNCTION_CALLS
#endif /* !__dj_ENFORCE_FUNCTION_CALLS */

#ifdef __cplusplus
}
#endif

#endif /* !__dj_include_dir.h_ */
