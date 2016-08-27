/* Copyright (C) 2007 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2002 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1997 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#ifndef __dj_include_libc_stubs_h__
#define __dj_include_libc_stubs_h__

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __dj_ENFORCE_ANSI_FREESTANDING

#ifndef __STRICT_ANSI__

#ifndef _POSIX_SOURCE

/* POSIX functions (for when compiling an ANSI function) */

#define access __access
#define asctime_r __asctime_r
#define chdir __chdir
#define close __close
#define ctime_r __ctime_r
#define dup __dup
#define dup2 __dup2
#define fdopen __fdopen
#define fnmatch __fnmatch
#define fstatvfs __fstatvfs
#define getcwd __getcwd
#define glob __glob
#define gmtime_r __gmtime_r
#define isatty __isatty
#define localtime_r __localtime_r
#define lseek __lseek
#define mkdir __mkdir
#define open __open
#define putenv __putenv
#define read __read
#define statvfs __statvfs
#define strtok_r __strtok_r
#define tzset __tzset
#define write __write

/* DJGPP functions (for compiling POSIX or ANSI functions) */

#define basename __basename
#define crlf2nl __crlf2nl
#define dosmemget __dosmemget
#define dosmemput __dosmemput
#define ecvtbuf __ecvtbuf
#define fchown __fchown
#define fcvtbuf __fcvtbuf
#define fgetgrent __fgetgrent
#define filelength __filelength
#define findfirst __findfirst
#define findnext __findnext
#define fseeko __fseeko
#define fseeko64 __fseeko64
#define fsync __fsync
#define ftello __ftello
#define ftello64 __ftello64
#define getdisk __getdisk
#define getdtablesize __getdtablesize
#define getitimer __getitimer
#define gettimeofday __gettimeofday
#define lfilelength __lfilelength
#define llseek __llseek
#define modfl __modfl
#define moncontrol __moncontrol
#define monstartup __monstartup
#define movedata __movedata
#define pow10 __pow10
#define pow2 __pow2
#define readlink __readlink
#define sbrk __sbrk
#define setitimer __setitimer
#define setmode __setmode
#define spawnve __spawnve
#define spawnvpe __spawnvpe
#define statfs __statfs
#define stricmp __stricmp
#define strnicmp __strnicmp
#define strtold _strtold
#define sync __sync
#define tell __tell
#define tzsetwall __tzsetwall
#define uclock __uclock

#endif /* !_POSIX_SOURCE */
#endif /* !__STRICT_ANSI__ */
#endif /* !__dj_ENFORCE_ANSI_FREESTANDING */

#ifndef __dj_ENFORCE_FUNCTION_CALLS
#endif /* !__dj_ENFORCE_FUNCTION_CALLS */

#ifdef __cplusplus
}
#endif

#endif /* __dj_include_libc_stubs_h__ */
