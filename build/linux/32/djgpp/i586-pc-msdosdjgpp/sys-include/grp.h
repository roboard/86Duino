/* Copyright (C) 2012 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2003 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2001 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2000 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#ifndef __dj_include_grp_h_
#define __dj_include_grp_h_

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

struct group {
  gid_t		gr_gid;		/* Group ID.	*/
  char **      	gr_mem;		/* Member list.	*/
  char *	gr_name;	/* Group name.	*/
  char *	gr_passwd;	/* Password.	*/
};

struct group *	getgrgid(gid_t _gid);
struct group *	getgrnam(const char *_name);

#ifndef _POSIX_SOURCE

void		endgrent(void);
struct group *	getgrent(void);
struct group *	fgetgrent(void *_f);
void		setgrent(void);

#endif /* !_POSIX_SOURCE */
#endif /* !__STRICT_ANSI__ */
#endif /* !__dj_ENFORCE_ANSI_FREESTANDING */

#ifndef __dj_ENFORCE_FUNCTION_CALLS
#endif /* !__dj_ENFORCE_FUNCTION_CALLS */

#ifdef __cplusplus
}
#endif

#endif /* !__dj_include_grp_h_ */
