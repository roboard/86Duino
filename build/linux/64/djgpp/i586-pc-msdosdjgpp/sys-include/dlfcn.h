/* Copyright (C) 2015 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2012 DJ Delorie, see COPYING.DJ for details */
#ifndef __dj_include_dlfcn_h_
#define __dj_include_dlfcn_h_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __dj_ENFORCE_ANSI_FREESTANDING

#if (defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L) \
  || !defined(__STRICT_ANSI__) || defined(__cplusplus)

#endif /* (__STDC_VERSION__ >= 199901L) || !__STRICT_ANSI__ */

#ifndef __STRICT_ANSI__

#ifndef _POSIX_SOURCE

/* The MODE argument to `dlopen' contains one of the following: */
#define RTLD_LAZY	0x0001	/* Lazy function call binding.  */
#define RTLD_NOW	0x0002	/* Immediate function call binding.  */

/* If the following bit is set in the MODE argument to `dlopen',
   the symbols of the loaded object and its dependencies are made
   visible as if the object were linked directly into the program. */
#define RTLD_GLOBAL	0x0100

/* Unix98 demands the following flag which is the inverse to RTLD_GLOBAL.
   The implementation does this by default and so we can define the
   value to zero.  */
#define RTLD_LOCAL	0x0000

/* If the first argument to `dlsym' is set to RTLD_DEFAULT the run-time
   address of the symbol called NAME in the global scope is returned. */
#define RTLD_DEFAULT	NULL

/* Open the shared object FILE and map it in; return a handle that can be
   passed to `dlsym' to get symbol values from it.  */
extern void *dlopen (const char *filename, int mode);

/* Unmap and close a shared object opened by `dlopen'.
   The handle cannot be used again after calling `dlclose'.  */
extern int dlclose (void *handle);

/* Find the run-time address in the shared object HANDLE refers to
   of the symbol called NAME.  */
extern void *dlsym (void *handle, const char *name);

/* When any of the above functions fails, call this function
   to return a string describing the error.  Each call resets
   the error string so that a following call returns null. */
extern const char *dlerror (void);

#endif /* !_POSIX_SOURCE */
#endif /* !__STRICT_ANSI__ */
#endif /* !__dj_ENFORCE_ANSI_FREESTANDING */

#ifndef __dj_ENFORCE_FUNCTION_CALLS
#endif /* !__dj_ENFORCE_FUNCTION_CALLS */

#ifdef __cplusplus
}
#endif

#endif /* !__dj_include_dlfcn_h_ */
