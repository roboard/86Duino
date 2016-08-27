/* Copyright (C) 2012 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2011 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2003 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2001 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1999 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#ifndef __dj_include_setjmp_h_
#define __dj_include_setjmp_h_

#ifdef __cplusplus
namespace std {
  extern "C" {
#endif

#ifndef __dj_ENFORCE_ANSI_FREESTANDING

typedef struct __jmp_buf {
  unsigned long __eax, __ebx, __ecx, __edx, __esi;
  unsigned long __edi, __ebp, __esp, __eip, __eflags;
  unsigned short __cs, __ds, __es, __fs, __gs, __ss;
  unsigned long __sigmask; /* for POSIX signals only */
  unsigned long __signum; /* for expansion */
  unsigned long __exception_ptr; /* pointer to previous exception */
  unsigned char __fpu_state[108]; /* for future use */
} jmp_buf[1];

void	longjmp(jmp_buf env, int val) __attribute__((__noreturn__));
int	setjmp(jmp_buf env);
#define setjmp setjmp   /* required by C++ standard */

#if (defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L) \
  || !defined(__STRICT_ANSI__) || defined(__cplusplus)

#endif /* (__STDC_VERSION__ >= 199901L) || !__STRICT_ANSI__ */

#ifndef __STRICT_ANSI__

typedef jmp_buf sigjmp_buf;

int	sigsetjmp(sigjmp_buf env, int savemask);
void	siglongjmp(sigjmp_buf env, int val) __attribute__((__noreturn__));

#ifndef _POSIX_SOURCE

#endif /* !_POSIX_SOURCE */
#endif /* !__STRICT_ANSI__ */
#endif /* !__dj_ENFORCE_ANSI_FREESTANDING */

#ifndef __dj_ENFORCE_FUNCTION_CALLS
#endif /* !__dj_ENFORCE_FUNCTION_CALLS */

#ifdef __cplusplus
  }
}
#endif

#endif /* !__dj_include_setjmp_h_ */


#if defined(__cplusplus) && !defined(__dj_ENFORCE_ANSI_FREESTANDING)

using std::setjmp;

#ifndef __dj_via_cplusplus_header_

using std::longjmp;
using std::jmp_buf;

#ifndef __STRICT_ANSI__

using std::sigjmp_buf;
using std::sigsetjmp;
using std::siglongjmp;

#endif /* !__STRICT_ANSI__ */
#endif /* !__dj_via_cplusplus_header_ */
#endif /* __cplusplus && !__dj_ENFORCE_ANSI_FREESTANDING */
