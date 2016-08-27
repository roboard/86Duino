/* Copyright (C) 2011 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1998 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1995 DJ Delorie, see COPYING.DJ for details */
#ifndef __dj_include_inline_pc_h_
#define __dj_include_inline_pc_h_

#ifdef __cplusplus
extern "C" {
#endif

#include <sys/cdefs.h>

_EXTERN_INLINE unsigned char
inportb (unsigned short _port)
{
  unsigned char rv;
  __asm__ __volatile__ ("inb %1, %0"
	  : "=a" (rv)
	  : "dN" (_port));
  return rv;
}

_EXTERN_INLINE unsigned short
inportw (unsigned short _port)
{
  unsigned short rv;
  __asm__ __volatile__ ("inw %1, %0"
	  : "=a" (rv)
	  : "dN" (_port));
  return rv;
}

_EXTERN_INLINE unsigned long
inportl (unsigned short _port)
{
  unsigned long rv;
  __asm__ __volatile__ ("inl %1, %0"
	  : "=a" (rv)
	  : "dN" (_port));
  return rv;
}

_EXTERN_INLINE void
outportb (unsigned short _port, unsigned char _data)
{
  __asm__ __volatile__ ("outb %1, %0"
	  :
	  : "dN" (_port),
	    "a" (_data));
}

_EXTERN_INLINE void
outportw (unsigned short _port, unsigned short _data)
{
  __asm__ __volatile__ ("outw %1, %0"
	  :
	  : "dN" (_port),
	    "a" (_data));
}

_EXTERN_INLINE void
outportl (unsigned short _port, unsigned long _data)
{
  __asm__ __volatile__ ("outl %1, %0"
	  :
	  : "dN" (_port),
	    "a" (_data));
}

#ifdef __cplusplus
}
#endif

#endif /* !__dj_include_inline_pc_h_ */
