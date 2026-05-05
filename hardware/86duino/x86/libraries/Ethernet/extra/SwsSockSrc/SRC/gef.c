/****************************************************************************
 *
 * File name   : gef.c
 * Function    : Global event filter.
 * Project     : SwsSock
 * Authors     : Lawrence Rust
 * Systems     : ANSI C
 *
 ****************************************************************************
 * File History
 * ------------
 * 28-Aug-05, LVR, v1.00 - Created.
 *
 *
 * Comments:
 * --------
 * This file implements the Global Event Filter
 *
 ****************************************************************************/

/* Compilation options */
#include "config.h"

/* Exports */
#include "sws_gef.h"

/* Imports */
#include <stddef.h>

#include "debug.h"


/*
 * Macros & Constants
 */


/*
 * Local data types
 */


/*
 * Prototypes
 */


/*
 * Module data
 */

/* Filter */
static SWS_FNPTR( SWS_FGef) s_pfnFilter;
static SWS_PTR( void) s_pvFilter;


/****************************************************************************
 * External functions
 ****************************************************************************/

/*
 * Install an event filter
 */
SWS_EXPORT( SWS_FNPTR( SWS_FGef), SWS_GefInstall( /* Returns previous */
  SWS_FNPTR( SWS_FGef) pfn,           /* IN: -> Filter callback */
  SWS_PTR( void) pv                   /* IN: callback context */
)){
  SWS_FNPTR( SWS_FGef) pfnOld;

  TRACE( 9, ("SWS_GefInstall(%p,%p)\n", pfn,pv));

  pfnOld = s_pfnFilter;
  s_pfnFilter = 0;
  s_pvFilter = pv;
  s_pfnFilter = pfn;

  return pfnOld;
  }


/*
 * Signal an event. NB C calling convention required  for ...
 */
extern int SWS_GefRaise(              /* Returns GEF result or 0 if none */
  int iEvent,                         /* IN: SWS_EGefEvent */
  ...                                 /* IN: Depends on event */
) {
  int iRet;

  TRACE( 9, ("SWS_GefRaise(%d)\n", iEvent));
  ASSERT( iEvent >= SWS_kGefNone);
  ASSERT( iEvent < SWS_kGefCount);

  if ( s_pfnFilter)
    {
    /* Call the packet filter */
    va_list va;
    va_start( va, iEvent);
    iRet = (*s_pfnFilter)( s_pvFilter, iEvent, va);
    va_end( va);
    }
  else
    {
    iRet = 0;
    }

  return iRet;
  }

/* End of file */
