/* Copyright (C) 2012 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2003 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2001 DJ Delorie, see COPYING.DJ for details */
#ifndef __dj_include_libc_malloc_h__
#define __dj_include_libc_malloc_h__

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __dj_ENFORCE_ANSI_FREESTANDING

#if (defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L) \
  || !defined(__STRICT_ANSI__) || defined(__cplusplus)

#endif /* (__STDC_VERSION__ >= 199901L) || !__STRICT_ANSI__ */

#ifndef __STRICT_ANSI__

#ifndef _POSIX_SOURCE

typedef struct BLOCK {
  size_t size;
  struct BLOCK *next;
  int bucket;
} BLOCK;

#define BEFSZ(bp)	(*(size_t *)((char *)bp - 4))
#define ENDSZ(bp)	(*(size_t *)((char *)bp + (bp->size & ~1) + 4))
#define BEFORE(bp)	((BLOCK *)((char *)bp - (BEFSZ(bp) & ~1) - 8))
#define AFTER(bp)	((BLOCK *)((char *)bp + ((bp->size & ~1) + 8)))
#define DATA(bp)	((char *)&(bp->next))

#ifndef NUMSMALL
#define NUMSMALL	0
#endif
#define ALIGN		8
#define SMALL		(NUMSMALL*ALIGN)

#endif /* !_POSIX_SOURCE */
#endif /* !__STRICT_ANSI__ */
#endif /* !__dj_ENFORCE_ANSI_FREESTANDING */

#ifndef __dj_ENFORCE_FUNCTION_CALLS
#endif /* !__dj_ENFORCE_FUNCTION_CALLS */

#ifdef __cplusplus
}
#endif

#endif /* __dj_include_libc_atexit_h__ */
