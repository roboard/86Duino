/* Copyright (C) 2012 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2003 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1999 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#ifndef __dj_include_search_h_
#define __dj_include_search_h_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __dj_ENFORCE_ANSI_FREESTANDING

#include <sys/djtypes.h>

#ifndef _SIZE_T
__DJ_size_t
#define _SIZE_T
#endif

#if (defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L) \
  || !defined(__STRICT_ANSI__) || defined(__cplusplus)

#endif /* (__STDC_VERSION__ >= 199901L) || !__STRICT_ANSI__ */

#ifndef __STRICT_ANSI__

/* ACTION type for hsearch(). */
typedef enum {
  FIND,
  ENTER
} ACTION;

/* ENTRY type for hsearch(). */
typedef struct entry {
  char    *key;
  void    *data;
} ENTRY;


int	hcreate(size_t _nel);
void	hdestroy(void);
ENTRY *	hsearch(ENTRY _item, ACTION _action);
void *	lfind(const void *_key, void *_base, size_t *_nelp, size_t _width,
	      int(*_compar)(const void *, const void *));
void *	lsearch(const void *_key, void *_base, size_t *_nelp, size_t _width,
		int(*_compar)(const void *, const void *));

#ifndef _POSIX_SOURCE

/* qelem type for insque and remque. */
typedef struct qelem {
  struct qelem *q_forw;
  struct qelem *q_back;
  char q_data[0];
} qelem;

unsigned long _hash2v(const unsigned char *s, unsigned long *v2);
void insque(struct qelem *_elem, struct qelem *_pred);
void remque(struct qelem *_elem);

#endif /* !_POSIX_SOURCE */
#endif /* !__STRICT_ANSI__ */
#endif /* !__dj_ENFORCE_ANSI_FREESTANDING */

#ifndef __dj_ENFORCE_FUNCTION_CALLS
#endif /* !__dj_ENFORCE_FUNCTION_CALLS */

#ifdef __cplusplus
}
#endif

#endif /* !__dj_include_search_h_ */
