/* Copyright (C) 2011 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2003 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1998 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1994 DJ Delorie, see COPYING.DJ for details */
#ifndef __dj_include_sys_cdefs_h_
#define __dj_include_sys_cdefs_h_

#undef __P
#if defined(__STDC__) || defined(__cplusplus)
#define __P(p) p
#else
#define __P(p)
#endif
#define	_PTR		void *
#define	_AND		,
#define	_NOARGS		void
#define	_CONST		const
#define	_VOLATILE	volatile
#define	_SIGNED		signed
#define	_DOTS		, ...
#define	_VOID void
#define	_EXFUN(name, proto)		(name) proto
#define	_DEFUN(name, arglist, args)	(name)(args)
#define	_DEFUN_VOID(name)		(name)(_NOARGS)
#define	_CAST_VOID (void)
#ifndef	_LONG_DOUBLE
#define	_LONG_DOUBLE long double
#endif
#ifndef	_PARAMS
#define	_PARAMS(paramlist)		paramlist
#endif

/* Support gcc's __attribute__ facility.  */

#define _ATTRIBUTE(attrs) __attribute__ ((attrs))


/* Ensure that always traditional GNU extern inline semantics are used
   (aka -fgnu89-inline) even if ISO C99 semantics have been specified.  */
#if __GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 2)
# define _EXTERN_INLINE  extern __inline__ __attribute__ ((__gnu_inline__))
#else
# define _EXTERN_INLINE  extern __inline__
#endif

#if defined(__cplusplus)
#define __BEGIN_DECLS	extern "C" {
#define __END_DECLS	}
#else
#define __BEGIN_DECLS
#define __END_DECLS
#endif

#endif /* !__dj_include_sys_cdefs_h_ */
