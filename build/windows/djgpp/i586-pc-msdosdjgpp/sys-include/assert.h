/* Copyright (C) 2003 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1997 DJ Delorie, see COPYING.DJ for details */
#ifndef __dj_ENFORCE_ANSI_FREESTANDING

#undef assert

#if defined(NDEBUG)
#define assert(test) ((void)0) 
#else
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
#define assert(test) ((void)((test)||(__dj_assert(#test,__FILE__,__LINE__,__func__),0)))
#else
#define assert(test) ((void)((test)||(__dj_assert(#test,__FILE__,__LINE__,(char *)0),0)))
#endif
#endif

#ifndef __dj_include_assert_h_
#define __dj_include_assert_h_

#ifdef __cplusplus
extern "C" {
#endif

void	__dj_assert(const char *,const char *,int,const char *) __attribute__((__noreturn__));

#ifdef __cplusplus
}
#endif

#endif /* !__dj_include_assert_h_ */

#endif /* !__dj_ENFORCE_ANSI_FREESTANDING */
