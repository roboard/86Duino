/* -------- malldbg.h ----------- */

/* Copyright (c) 2003 by Charles B. Falconer
   Licensed under the terms of the GNU LIBRARY GENERAL PUBLIC
   LICENSE and/or the terms of COPYING.DJ, all available at
   <http://www.delorie.com>.

   Bug reports to <mailto:djgpp@delorie.com>

   Revised 2015 for use in DJGPP libs by Andris Pavenis <andris.pavenis@iki.fi>
*/

#ifndef nmalloc_malldbg_h
#define nmalloc_malldbg_h

/* This is to be used in conjunction with a version of
   nmalloc.c compiled with:

     gcc -DNDEBUG -o malloc.o -c nmalloc.c

   after which linking malldbg.o and malloc.o will
   provide the usual malloc, free, realloc calls.
   Both malloc.o and malldbg.o can be components
   of the normal run time library.
*/

#include <stddef.h>
#include <libc/sysquery.h>

struct mallinfo {
   int arena;           /* Total space being managed */
   int ordblks;         /* Count of allocated & free blocks */
   int smblks;
   int hblks;           /* Count of free blocks */
   int hblkhd;          /* Size of the 'lastsbrk' block */
   int usmblks;
   int fsmblks;
   int uordblks;        /* Heap space in use w/o overhead */
   int fordblks;        /* Total space in free lists */
   int keepcost;        /* Overhead in tracking storage */
};

struct mallinfo mallinfo(void);
int             malloc_verify(void);
int             malloc_debug(int level);
void            mallocmap(void);
FILE           *malldbgdumpfile(FILE *fp);
M_HOOKFN        mallsethook(enum m_hook_kind which,
                            M_HOOKFN newhook);

#endif
/* -------- malldbg.h ----------- */
