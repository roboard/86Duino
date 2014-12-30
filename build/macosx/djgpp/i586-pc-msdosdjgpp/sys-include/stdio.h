/* Copyright (C) 2014 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2013 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2003 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2002 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2001 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2000 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1998 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */

#ifndef __dj_include_stdio_h_
#define __dj_include_stdio_h_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __dj_ENFORCE_ANSI_FREESTANDING

#include <sys/version.h>
#include <sys/djtypes.h>
  
#define _IOFBF    	00001  /*  File is full buffered.  */
#define _IONBF    	00002  /*  File is not buffered.  */
#define _IOLBF    	00004  /*  File is line buffered.  */

/* Some programs think they know better... */
#undef NULL
#if (__GNUC__ >= 4) && defined(__cplusplus)
#  define NULL          __null
#elif defined(__cplusplus)
#  define NULL          0
#else
#  define NULL          ((void*)0)
#endif

#define BUFSIZ		16384
#define EOF		(-1)
#define FILENAME_MAX	260
#define FOPEN_MAX	20
#define L_tmpnam	260
#define TMP_MAX		999999

#define SEEK_SET	0
#define SEEK_CUR	1
#define SEEK_END	2

#ifndef _VA_LIST
__DJ_va_list
#define _VA_LIST
#endif

#ifndef _SIZE_T
__DJ_size_t
#define _SIZE_T
#endif

#ifndef _SSIZE_T
__DJ_ssize_t
#define _SSIZE_T
#endif

/* Note that the definitions of these fields are NOT guaranteed!  They
   may change with any release without notice!  The fact that they
   are here at all is to comply with ANSI specifictions. */
   
typedef struct {
  ssize_t  _cnt;
  char    *_ptr;
  char    *_base;
  size_t   _bufsiz;
  int      _flag;
  int      _file;
  char    *_name_to_remove;
  size_t   _fillsize;
} FILE;

typedef unsigned long		fpos_t;

extern FILE __dj_stdin, __dj_stdout, __dj_stderr;
#define stdin	(&__dj_stdin)
#define stdout	(&__dj_stdout)
#define stderr	(&__dj_stderr)

void	clearerr(FILE *_stream);
int	fclose(FILE *_stream);
int	feof(FILE *_stream);
int	ferror(FILE *_stream);
int	fflush(FILE *_stream);
int	fgetc(FILE *_stream);
int	fgetpos(FILE *_stream, fpos_t *_pos);
char *	fgets(char *_s, int _n, FILE *_stream);
FILE *	fopen(const char *_filename, const char *_mode);
int	fprintf(FILE *_stream, const char *_format, ...);
int	fputc(int _c, FILE *_stream);
int	fputs(const char *_s, FILE *_stream);
size_t	fread(void *_ptr, size_t _size, size_t _nelem, FILE *_stream);
FILE *	freopen(const char *_filename, const char *_mode, FILE *_stream);
int	fscanf(FILE *_stream, const char *_format, ...);
int	fseek(FILE *_stream, long _offset, int _mode);
int	fsetpos(FILE *_stream, const fpos_t *_pos);
long	ftell(FILE *_stream);
size_t	fwrite(const void *_ptr, size_t _size, size_t _nelem, FILE *_stream);
int	getc(FILE *_stream);
int	getchar(void);
char *	gets(char *_s);
void	perror(const char *_s);
int	printf(const char *_format, ...);
int	putc(int _c, FILE *_stream);
int	putchar(int _c);
int	puts(const char *_s);
int	remove(const char *_filename);
int	rename(const char *_old, const char *_new);
void	rewind(FILE *_stream);
int	scanf(const char *_format, ...);
void	setbuf(FILE *_stream, char *_buf);
int	setvbuf(FILE *_stream, char *_buf, int _mode, size_t _size);
int	sprintf(char *_s, const char *_format, ...);
int	sscanf(const char *_s, const char *_format, ...);
FILE *	tmpfile(void);
char *	tmpnam(char *_s);
int	ungetc(int _c, FILE *_stream);
int	vfprintf(FILE *_stream, const char *_format, va_list _ap);
int	vprintf(const char *_format, va_list _ap);
int	vsprintf(char *_s, const char *_format, va_list _ap);

#if (defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L) \
  || !defined(__STRICT_ANSI__) || defined(__cplusplus)

int	snprintf(char *str, size_t n, const char *fmt, ...);
int	vfscanf(FILE *_stream, const char *_format, va_list _ap);
int	vscanf(const char *_format, va_list _ap);
int	vsnprintf(char *str, size_t n, const char *fmt, va_list ap);
int	vsscanf(const char *_s, const char *_format, va_list _ap);

#endif /* (__STDC_VERSION__ >= 199901L) || !__STRICT_ANSI__ */

#ifndef __STRICT_ANSI__

#define L_ctermid 20
#define L_cusrid
/* #define STREAM_MAX	20 - DOS can change this */

int	dprintf(int _fd, const char *_format, ...) __attribute__ ((__format__ (__printf__, 2, 3)));
int	fileno(FILE *_stream);
FILE *	fdopen(int _fildes, const char *_type);
int	mkstemp(char *_template);
int	pclose(FILE *_pf);
FILE *	popen(const char *_command, const char *_mode);
char *	tempnam(const char *_dir, const char *_prefix);
int	vdprintf(int _fd, const char *_format, va_list _ap) __attribute__ ((__format__ (__printf__, 2, 0)));

#ifndef _POSIX_SOURCE

extern FILE __dj_stdprn, __dj_stdaux;
#define stdprn	(&__dj_stdprn)
#define stdaux	(&__dj_stdaux)

#define P_tmpdir "c:/"

void	_djstat_describe_lossage(FILE *_to_where);
int	_doprnt(const char *_fmt, va_list _args, FILE *_f);
int	_doscan(FILE *_f, const char *_fmt, va_list _args);
int	_doscan_low(FILE *, int (*)(FILE *_get), int (*_unget)(int, FILE *), const char *_fmt, va_list _args);
int	fpurge(FILE *_f);
int	getw(FILE *_f);
char *	mktemp(char *_template);
int	putw(int _v, FILE *_f);
void	setbuffer(FILE *_f, void *_buf, int _size);
void	setlinebuf(FILE *_f);
int	_rename(const char *_old, const char *_new);	/* Simple (no directory) */
int	asprintf(char **_sp, const char *_format, ...) __attribute__((format (__printf__, 2, 3)));
char *	asnprintf(char *_s, size_t *_np, const char *_format, ...) __attribute__((format (__printf__, 3, 4)));
int	vasprintf(char **_sp, const char *_format, va_list _ap) __attribute__((format (__printf__, 2, 0)));
char *	vasnprintf(char *_s, size_t *_np, const char *_format, va_list _ap) __attribute__((format (__printf__, 3, 0)));

#ifndef _OFF_T
__DJ_off_t
#define _OFF_T
#endif
#ifndef _OFF64_T
__DJ_off64_t
#define _OFF64_T
#endif
int	fseeko(FILE *_stream, off_t _offset, int _mode);
off_t	ftello(FILE *_stream);
int	fseeko64(FILE *_stream, off64_t _offset, int _mode);
off64_t	ftello64(FILE *_stream);

#endif /* !_POSIX_SOURCE */
#endif /* !__STRICT_ANSI__ */
#endif /* !__dj_ENFORCE_ANSI_FREESTANDING */

#ifndef __dj_ENFORCE_FUNCTION_CALLS
#endif /* !__dj_ENFORCE_FUNCTION_CALLS */

#ifdef __cplusplus
}
#endif

#endif /* !__dj_include_stdio_h_ */
