/* Copyright (C) 2012 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2003 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2001 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2000 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#ifndef __dj_include_pwd_h_
#define __dj_include_pwd_h_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __dj_ENFORCE_ANSI_FREESTANDING

#if (defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L) \
  || !defined(__STRICT_ANSI__) || defined(__cplusplus)

#endif /* (__STDC_VERSION__ >= 199901L) || !__STRICT_ANSI__ */

#ifndef __STRICT_ANSI__

#include <sys/djtypes.h>

#ifndef _GID_T
__DJ_gid_t
#define _GID_T
#endif
#ifndef _UID_T
__DJ_uid_t
#define _UID_T
#endif

struct passwd {
  char *	pw_name;		/* Username.  */
  uid_t 	pw_uid;			/* User ID.  */
  gid_t 	pw_gid;			/* Group ID.  */
  char *	pw_dir;			/* Home directory.  */
  char *	pw_shell;		/* Shell program.  */
  char *	pw_gecos;		/* Real name.  */
  char *	pw_passwd;		/* Password.  */
};
  
struct passwd *	getpwuid(uid_t _uid);
struct passwd *	getpwnam(const char *_name);

#ifndef _POSIX_SOURCE

struct passwd	*getpwent(void);
void		setpwent(void);
void		endpwent(void);

#endif /* !_POSIX_SOURCE */
#endif /* !__STRICT_ANSI__ */
#endif /* !__dj_ENFORCE_ANSI_FREESTANDING */

#ifndef __dj_ENFORCE_FUNCTION_CALLS
#endif /* !__dj_ENFORCE_FUNCTION_CALLS */

#ifdef __cplusplus
}
#endif

#endif /* !__dj_include_pwd_h_ */
