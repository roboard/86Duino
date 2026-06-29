/****************************************************************************
 *
 * File name   : timer.c
 * Function    : Timer manager
 * Project     : SwsSock
 * Authors     : Lawrence Rust
 * Systems     : ANSI C.
 *
 ****************************************************************************
 *
 * Created by Lawrence Rust, Software Systems Consultants
 * lvr@softsystem.co.uk. Tel/Fax +33 5 49 72 79 63
 *
 * 06-Apr-01, LVR, Created.
 * 05-Dec-02, LVR, Fix TimerCallback in case where multiple timers expire
 *
 * Comments:
 * --------
 * This file implements the timer manager
 *
 ****************************************************************************
 */

/* Compilation options */
#include "config.h"

/* Exports */
#include "sws_timr.h"

/* Imports */
#include <stddef.h>
#include <stdlib.h>   /* for malloc */
#include <string.h>   /* for memset */

#include "sws_sock.h"
#include "platform.h"

#include "debug.h"


/*
 * Macros & Constants
 */


/*
 * Local data types
 */
/* A timer descriptor */
typedef struct SWS_Timer
  {
  ui32 uMagic;
#define TIMER_MAGIC SWS_MAKEFOURCC( 't','i','m','r')
  struct SWS_Timer *pNext, *pPrev;    /* Doubly linked list */

  int isRunning;
  ui32 uTimeout;                      /* Time of expiry */

  SWS_FNPTR( SWS_FTimer) pfn;         /* Timer expiry callback */
  SWS_PTR( void) pvContext;           /* Client context for callback */
  } SWS_Timer;


/*
 * Prototypes
 */
static SWS_CB( int, TimerAlloc( SWS_PTR( void) ));
static SWS_CB( int, TimerRelease( SWS_PTR( void) ));
static SWS_CB( int, TimerStop( SWS_PTR( void) ));
static SWS_CB( int, TimerStart( SWS_PTR( void) ));
static FPlatformTimeout TimerCallback;


/*
 * Module data
 */
/* Timer lists */
static SWS_Timer* s_pTimers;          /* Timer heap */
static SWS_Timer* s_pFreeTimers;      /* Unallocated timer list */
static SWS_Timer* s_pRunningTimers;   /* List of running timers, earliest expiry time first */


/*
 * Initialise the timer manager
 * Single threaded
 */
int SWS_TimerInit( void)              /* Returns !0 on error */
  {
  SWS_Timer* pTimer;
  size_t u, timers;

  TRACE( 9, ("SWS_TimerInit()\n"));
  ASSERT( NULL == s_pTimers);
  ASSERT( NULL == s_pFreeTimers);
  ASSERT( NULL == s_pRunningTimers);
  ASSERT( PlatformIsCritical());

  timers = (size_t)PlatformPropertyGetUint( PKEY_TIMER_COUNT, OPT_TIMERS );

  /* Allocate timer pool */
  s_pTimers = (SWS_Timer*) calloc( timers, sizeof(SWS_Timer) );
  if ( NULL == s_pTimers)
    {
    TRACE( 0, ("!!! SWS_TimerInit() out of memory\n"));
    return SWS_ENOMEM;
    }

  /* Create timer free list */
  pTimer = s_pTimers;
  for ( u = 0; u < timers; ++u)
    {
    pTimer->pNext = s_pFreeTimers;
    s_pFreeTimers = pTimer;
    pTimer++;
    }

  return 0;
  }


/*
 * Finalise the timer manager
 * Single threaded
 */
void SWS_TimerCleanup( void)
  {
  TRACE( 9, ("SWS_TimerCleanup()\n"));
  ASSERT( PlatformIsCritical());

  s_pRunningTimers = NULL;
  s_pFreeTimers = NULL;

  if ( NULL != s_pTimers)
    {
    free( s_pTimers);
    s_pTimers = NULL;
    }
  }


/*
 * Allocate a timer
 */
SWS_EXPORT( SWS_TimerH, SWS_TimerAlloc( /* Timer handle or NULL if none */
  SWS_FNPTR( SWS_FTimer) pfn,         /* IN: Optional callback on expiry */
  SWS_PTR( void) pvContext            /* IN: Callback context */
)){
  SWS_TimerH hTimer;

  TRACE( 9, ("SWS_TimerAlloc(%p,%p)\n", pfn,pvContext));

  if ( PlatformSynchronise( &TimerAlloc, &hTimer))
    return NULL;

  hTimer->uMagic = TIMER_MAGIC;
  hTimer->isRunning = 0;
  hTimer->pfn = pfn;
  hTimer->pvContext = pvContext;
  return hTimer;
  }

SWS_CB( int, TimerAlloc(
  SWS_PTR( void) pv
)){
  SWS_TimerH* const phTimer = (SWS_TimerH*)pv;
  SWS_Timer* pTimer;

  TRACE( 9, ("TimerAlloc(%p)\n", pv));
  ASSERT( NULL != phTimer);
  ASSERT( PlatformIsCritical());

  pTimer = s_pFreeTimers;
  if ( NULL == pTimer)
    {
    TRACE( 1, ("***TimerAlloc() Out of timers\n"));
    return SWS_ENOBUFS;
    }

  /* Remove from free list */
  s_pFreeTimers = pTimer->pNext;
  pTimer->pNext = pTimer->pPrev = 0;

  *phTimer = pTimer;

  return 0;
  }


/*
 * Release a timer
 */
SWS_EXPORT( void, SWS_TimerRelease(
  SWS_TimerH hTimer                   /* IN: Timer to free */
)){
  TRACE( 9, ("SWS_TimerRelease(%p)\n", hTimer));
  ASSERT( NULL != hTimer);
  ASSERT( TIMER_MAGIC == hTimer->uMagic);

  PlatformSynchronise( &TimerRelease, hTimer);
  }

SWS_CB( int, TimerRelease(
  SWS_PTR( void) pv
)){
  SWS_TimerH const hTimer = (SWS_TimerH)pv;

  TRACE( 9, ("TimerAlloc(%p)\n", pv));
  ASSERT( NULL != hTimer);
  ASSERT( TIMER_MAGIC == hTimer->uMagic);
  ASSERT( PlatformIsCritical());

  TimerStop( pv);
  ASSERT( NULL == hTimer->pNext);
  ASSERT( NULL == hTimer->pPrev);

  hTimer->uMagic = 0;

  /* Add to free list */
  hTimer->pNext = s_pFreeTimers;
  s_pFreeTimers = hTimer;

  return 0;
  }


/*
 * Start a timer
 */
SWS_EXPORT( int, SWS_TimerStart(      /* Returns !0 on error */
  SWS_TimerH hTimer,                  /* IN: Timer */
  ui32 uTicks                         /* IN: Ticks until expiry */
)){
  TRACE( 9, ("SWS_TimerStart(%p,%lu)\n", hTimer,uTicks));
  ASSERT( NULL != hTimer);
  ASSERT( TIMER_MAGIC == hTimer->uMagic);

  hTimer->uTimeout = PlatformTime() + uTicks;
  return PlatformSynchronise( &TimerStart, hTimer);
  }

SWS_CB( int, TimerStart(
  SWS_PTR( void) pv
)){
  SWS_TimerH const hTimer = (SWS_TimerH)pv;
  SWS_Timer* pTimer, *pPrev;;

  TRACE( 9, ("TimerStart(%p)\n", pv));
  ASSERT( NULL != hTimer);
  ASSERT( TIMER_MAGIC == hTimer->uMagic);
  ASSERT( PlatformIsCritical());

  if ( hTimer->isRunning)
    TimerStop( pv);

  /* Find timer with same or later expiry time */
  pPrev = NULL;
  pTimer = s_pRunningTimers;
  while ( NULL != pTimer)
    {
    if ( SWS_TimerLT( hTimer->uTimeout, pTimer->uTimeout))
      break;
    pPrev = pTimer;
    pTimer = pTimer->pNext;
    }

  hTimer->isRunning = 1;

  if ( NULL == pPrev)
    {
    /* Add to head of list */
    hTimer->pPrev = NULL;
    if ( NULL != s_pRunningTimers)
      {
      ASSERT( NULL == s_pRunningTimers->pPrev);
      s_pRunningTimers->pPrev = hTimer;
      }
    hTimer->pNext = s_pRunningTimers;
    s_pRunningTimers = (SWS_Timer*)hTimer;
    PlatformTimerStart( hTimer->uTimeout, &TimerCallback, hTimer);
    }
  else
    {
    /* Splice in after timer with shorter timeout */
    hTimer->pPrev = pPrev;
    hTimer->pNext = pPrev->pNext;
    pPrev->pNext = (SWS_Timer*)hTimer;
    if ( NULL != hTimer->pNext)
      {
      ASSERT( pPrev == hTimer->pNext->pPrev);
      hTimer->pNext->pPrev = (SWS_Timer*)hTimer;
      }
    }

  return 0;
  }


/*
 * Stop a timer
 */
SWS_EXPORT( ui32, SWS_TimerStop(      /* Returns ticks remaining */
  SWS_TimerH hTimer                   /* IN: Timer */
)){
  ui32 uTicks;

  TRACE( 9, ("SWS_TimerStop(%p)\n", hTimer));
  ASSERT( NULL != hTimer);
  ASSERT( TIMER_MAGIC == hTimer->uMagic);

  PlatformSynchronise( &TimerStop, hTimer);

  uTicks = PlatformTime();
  if ( SWS_TimerLE( hTimer->uTimeout, uTicks))
    uTicks = 0;
  else
    uTicks = hTimer->uTimeout - uTicks;

  return uTicks;
  }

SWS_CB( int, TimerStop(
  SWS_PTR( void) pv
)){
  SWS_Timer* const hTimer = (SWS_Timer*)pv;

  TRACE( 9, ("TimerStop(%p)\n", pv));
  ASSERT( NULL != hTimer);
  ASSERT( TIMER_MAGIC == hTimer->uMagic);
  ASSERT( PlatformIsCritical());

  /* Unsplice from running timerQ */
  if ( NULL != hTimer->pPrev)
    {
    ASSERT( s_pRunningTimers != hTimer);
    ASSERT( hTimer->isRunning);
    hTimer->pPrev->pNext = hTimer->pNext;

    if ( NULL != hTimer->pNext)
      {
      hTimer->pNext->pPrev = hTimer->pPrev;
      hTimer->pNext = NULL;
      }

    hTimer->pPrev = NULL;
    }
  else if ( s_pRunningTimers == hTimer)
    {
    /* Head of list */
   if ( NULL != (s_pRunningTimers = hTimer->pNext))
      hTimer->pNext->pPrev = NULL;

    hTimer->pNext = NULL;
    ASSERT( hTimer->isRunning);

    if ( NULL != s_pRunningTimers)
      PlatformTimerStart( s_pRunningTimers->uTimeout, &TimerCallback, s_pRunningTimers);
    else
      PlatformTimerStop();
    }
  else
    {
    /* Not running */
    ASSERT( NULL == hTimer->pNext );
    }

  hTimer->isRunning = 0;

  return 0;
  }


/*
 * Get ticks remaining
 */
SWS_EXPORT( ui32, SWS_TimerRemaining( /* Ticks remaining, 0= expired */
  SWS_TimerH hTimer                   /* IN: Timer */
)){
  i32 ticks;

  TRACE( 9, ("SWS_TimerRemaining(%p)\n", hTimer));
  ASSERT( NULL != hTimer);
  ASSERT( TIMER_MAGIC == hTimer->uMagic);

  if ( !hTimer->isRunning)
    return 0;

  ticks = SWS_TimerDelta( hTimer->uTimeout, PlatformTime() );
  return ticks > 0 ? (ui32)ticks : 0;
  }


/*
 * Platform timer callback
 * Single threaded
 */
SWS_CB( void, TimerCallback(
  SWS_PTR( void) pv
)){
  SWS_Timer* pTimer;

  TRACE( 9, ("TimerCallback(%p)\n", pv));
  ASSERT( PlatformIsCritical());

  /* Get 1st entry on timer runningQ */
  pTimer = s_pRunningTimers;
  ASSERT( NULL == pTimer || TIMER_MAGIC == pTimer->uMagic);

  if ( NULL != pTimer
    && pv == pTimer
  ) {
    ui32 const uNow = PlatformTime();
    ASSERT( !SWS_TimerLT( uNow, pTimer->uTimeout));

    /* Expire all timers with the same timeout */
    do
      {
      /* Remove from runningQ */
      ASSERT( NULL == pTimer->pPrev);
      s_pRunningTimers = pTimer->pNext;
      if ( NULL != s_pRunningTimers)
        s_pRunningTimers->pPrev = NULL;

      pTimer->pNext = NULL;
      pTimer->isRunning = 0;

      /* Invoke timer callback */
      if ( pTimer->pfn)
        (*pTimer->pfn)( pTimer, pTimer->pvContext);
      /* CAUTION pTimer may no longer exist here */

      pTimer = s_pRunningTimers;
      }
    while ( NULL != pTimer && SWS_TimerLE( pTimer->uTimeout, uNow) );

    /* Restart the timeout */
    if ( NULL != pTimer)
      PlatformTimerStart( pTimer->uTimeout, &TimerCallback, pTimer);
    }
  }

/* End of file */
