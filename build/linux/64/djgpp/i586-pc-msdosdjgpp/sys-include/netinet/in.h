/* Copyright (C) 2012 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2011 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 2003 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1998 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#ifndef __dj_include_netinet_in_h_
#define __dj_include_netinet_in_h_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __dj_ENFORCE_ANSI_FREESTANDING

#if (defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L) \
  || !defined(__STRICT_ANSI__) || defined(__cplusplus)

#endif /* (__STDC_VERSION__ >= 199901L) || !__STRICT_ANSI__ */

#ifndef __STRICT_ANSI__

#ifndef _POSIX_SOURCE

unsigned long  htonl(unsigned long _val);
unsigned long  ntohl(unsigned long _val);
unsigned short htons(unsigned short _val);
unsigned short ntohs(unsigned short _val);

#endif /* !_POSIX_SOURCE */
#endif /* !__STRICT_ANSI__ */
#endif /* !__dj_ENFORCE_ANSI_FREESTANDING */

#ifndef __dj_ENFORCE_FUNCTION_CALLS
#include <sys/cdefs.h>

_EXTERN_INLINE unsigned long
htonl(unsigned long _val)
{
  return (_val << 24) | ((_val&0xff00) << 8) | ((_val&0xff0000) >> 8) | (_val >> 24);
}
#define ntohl(x) htonl(x)

_EXTERN_INLINE unsigned short
htons(unsigned short _val)
{
  return (_val << 8) | (_val >> 8);
}
#define ntohs(x) htons(x)

#endif /* !__dj_ENFORCE_FUNCTION_CALLS */

#ifdef __cplusplus
}
#endif

#endif /* !__dj_include_netinet_in_h_ */
