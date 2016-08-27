/* -------- sysquery.h ------------ */

/* Copyright (c) 2003 by Charles B. Falconer
   Licensed under the terms of the GNU LIBRARY GENERAL PUBLIC
   LICENSE and/or the terms of COPYING.DJ, all available at
   <http://www.delorie.com>.

   Bug reports to <mailto:djgpp@delorie.com>

   Revised 2015 for use in DJGPP libs by Andris Pavenis <andris.pavenis@iki.fi>
*/

#ifndef nmalloc_sysquery_h
#define nmalloc_sysquery_h
#ifdef __cplusplus
   extern "C" {
#endif

/* The settable hooks, identifiers */
/* HKCOUNT is illegal value */
enum m_hook_kind {malloc_HK,  malloc_fail_HK,
                  free_HK,    free_null_HK,
                  realloc_HK, realloc_exit_HK,
                  HKCOUNT};

/* Depending on kind, some params may be meaningless */
typedef void (*M_HOOKFN)(size_t sz, void *bk);

/* returns previous value of the appropriate function */
typedef M_HOOKFN (*set_m_hook)(enum m_hook_kind which,
                               M_HOOKFN newhook);

/* This allows a clean connection to debugging software      */
/* NOTE: ANY value equivalent to -1 means data not available */
/*       for the unsigned chars this means 0xffh.            */
struct _sysquery {
   unsigned char data, gdlo, sz, prvf, nxtf, nxt, prv, ohead;
   void *        nilp;    /* dummy NULL, &freeptrs[1] */
   void *        anchors; /* of memory chains */
   set_m_hook    hookset; /* how to set a hook */
};

/* This can return the above values, hopefully in a register    */
/* NONE is used in nextfree, prevfree as the equivalent of NULL */
/* With the unclean knowledge that nil is actually a pointer to */
/* freehdrs[1], and that lastsbrk is actually freehdrs[0], the  */
/* entire malloc structure is open to debuggery.                */
struct _sysquery _sysmalloc(void);

#ifdef __cplusplus
   }
#endif

#endif
/* -------- sysquery.h ------------ */
