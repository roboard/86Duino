/****************************************************************************
 *
 * File name   : djgpp.c
 * Function    : DJGPP support functions.
 * Project     : SwsSock
 * Authors     : Lawrence Rust
 * Systems     : ANSI C, DOS.
 * Version     : See sws_sock.h for overall library release information.
 * Release Date: See sws_sock.h for overall library release information.
 *
 ****************************************************************************
 *
 * Created by Lawrence Rust, Software Systems Consultants
 * lvr@softsystem.co.uk. Tel/Fax +33 5 49 72 79 63
 *
 * 27-Apr-01, LVR, Created.
 * 29-Oct-02, LVR, PlatformTime uses uclock.
 *                 Add time of day to trace.
 * 14 Dec 02  lvr  Added cooperative threads
 * 20-Sep-03, LVR, Added PlatformExit to ensure driver cleanup in all cases
 * 09-Jun-05, LVR, Added PKEY_DJGPP_YIELD
 *
 * Comments:
 * --------
 * This file only contains functions that are DJGPP specific
 *
 ****************************************************************************
 */

/* Compilation options */
#include "config.h"

/* Exports */
#include "platform.h"

/* Imports */
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <errno.h>
#if OPT_THREADS == OPT_THREADS_COOP
#include <setjmp.h>
#endif

/* DJGPP */
#include <dos.h> /* enable/disable */
#include <crt0.h> /* _CRT0_FLAG_LOCK_MEMORY */
#include <dpmi.h> /* __dpmi_yield */

/* sws_sock */
#include "sws_timr.h"
#include "sws_msgq.h"
#include "sws_cfg.h"
#include "sws_sock.h"

#if OPT_DATALINK_ETHERNET
#include "packet.h"
#endif
#if OPT_DATALINK_P2P
#include "pccomms.h"
#endif
#if OPT_THREADS == OPT_THREADS_COOP
#include "thread.h"
#endif

#include "debug.h"


/* Macros & constants */


/* Local data types */

/* An event */
typedef struct SEvent
  {
  Atomic_t val;
  char inUse;
  } SEvent;

/* The queue entry for a deferred procedure call */
typedef struct SDpc
  {
  SWS_FNPTR( FPlatformDpc) pfn;
  SWS_PTR( void) pvContext;
  SWS_PTR( void) pvParam;
  ui32 uParam;
  } SDpc;


/* Prototypes */
static void PlatformExit( void);

/* Critical section control */
#define PlatformBeginCritical() AtomicIncrement( &s_critical)
#define PlatformEndCritical() AtomicDecrement( &s_critical)


/*
 * Module data
 */
/* Lock all memory since packet and pccomms use interrupts */
int _crt0_startup_flags = _CRT0_FLAG_LOCK_MEMORY;

/* Configuration */
static size_t s_events;               /* No. events */
static int s_bYield;                  /* !0 to yield in PlatformIdle */

static int s_iInited;                 /* Module initialised */
static Atomic_t s_critical;           /* Critical section nesting depth */
static SEvent* s_pEvents;             /* Array of events */
static SWS_MsgqH s_hDpcQ;             /* DPC queue */

/* TimerQ */
static int s_bTimerRunning;
static ui32 s_uTimeout;               /* Next timeout */
static SWS_FNPTR( FPlatformTimeout) s_pfnTimer;
static SWS_PTR( void) s_pvTimerContext;

/* Debug */
#ifdef DEBUG_TRACE_LEVEL
 static void TraceToLog( const char*);
# if !defined DEBUG
#  define DEBUG 1 /* Default trace level */
# endif
#endif
DEBUG_TRACE( DEBUG, &TraceToLog)

#ifdef ALERT_FN
 static short DebugAlert( const char*);
#endif
ALERT_DEFINE_HANDLER( &DebugAlert)


/****************************************************************************
 * Platform adaptation layer
 ****************************************************************************/

/*
 * Platform specific initialisation
 * This MUST be single threaded
 */
int PlatformInit( void)
  {
  TRACE( 9, ("PlatformInit()\n"));

  if ( 1 == ++s_iInited)
    {
    size_t uDpcs;

    PlatformTime();

    /* Get configuration data */
    s_events = (size_t)PlatformPropertyGetUint( PKEY_EVENT_COUNT, OPT_EVENTS );
    uDpcs = (size_t)PlatformPropertyGetUint( PKEY_DPC_COUNT, OPT_DPCS );
    s_bYield = 0 != PlatformPropertyGetUint( PKEY_DJGPP_YIELD, 0);

    /* Create event pool */
    s_pEvents = (SEvent*)calloc( s_events, sizeof(*s_pEvents));
    if ( NULL == s_pEvents)
      {
      TRACE( 0, ("!!!PlatformInit out of memory for events\n"));
      s_iInited = 0;
      return SWS_ENOMEM;
      }

    /* Create DPC queue */
    s_hDpcQ = SWS_MsgqCreate( uDpcs, sizeof( SDpc));
    if ( NULL == s_hDpcQ)
      {
      free( s_pEvents);
      s_pEvents = NULL;
      s_iInited = 0;
      return SWS_ENOBUFS;
      }

    /* Setup time zone info for localtime */
    tzsetwall();
    }

  return 0;
  }


/*
 * SwsSock has now been initialised
 */
int PlatformPostInit( void)
  {
  TRACE( 9, ("PlatformPostInit()\n"));
  ASSERT( s_iInited >= 1);
  ASSERT( PlatformIsCritical());

#if OPT_DATALINK_ETHERNET
  {
  /* Get the packet driver name */
  char szDriver[ 128];
  if ( PlatformPropertyGetString( PKEY_DJGPP_DRIVER, "", szDriver, sizeof(szDriver) ))
    {
    /* Install the packet driver */
    TRACE( 2, ("PlatformPostInit system(%s)\n", szDriver));
    if ( system( szDriver))
      TRACE( 1, ("*** PlatformPostInit system(%s) failed %d\n", szDriver, errno));
    }

  /* Install the packet driver */
  SWS_PacketInstall(); /* Ignore errors */
  }
#endif

#if OPT_DATALINK_P2P
  /* Install the serial driver */
  (void)SWS_PcCommsInstall();
#endif

  /* Failsafe exit handler */
  atexit( &PlatformExit);

  return 0;
  }


/*
 * SwsSock is about to terminate
 */
void PlatformPreCleanup( void)
  {
  TRACE( 9, ("PlatformPreCleanup()\n"));
  }


/*
 * Platform specific cleanup
 */
void PlatformCleanup( void)
  {
  TRACE( 9, ("PlatformCleanup()\n"));

  PlatformBeginCritical();

  ASSERT( s_iInited >= 1);
  if ( 0 == --s_iInited)
    {
#if OPT_DATALINK_P2P
    /* Cleanup the serial driver */
    SWS_PcCommsCleanup();
#endif
#if OPT_DATALINK_ETHERNET
    /* Cleanup the packet driver */
    SWS_PacketCleanup();
#endif

    if ( NULL != s_pEvents)
      {
      free ( s_pEvents);
      s_pEvents = NULL;
      }

    if ( NULL != s_hDpcQ)
      {
      SWS_MsgqDestroy( s_hDpcQ);
      s_hDpcQ = NULL;
      }

    AtomicWrite( &s_critical, 1);
    }

  PlatformEndCritical();
  }


void PlatformExit( void)
  {
  if ( s_iInited > 0)
    PlatformCleanup();
  }


ui32 PlatformPropertyGetUint(
  const char* pszKey,
  const char* pszVal,
  ui32 uDefault
) {
  ui32 val;
  SWS_in_addr_t addr;
  char szBuf[ 20];

  TRACE( 9, ("PlatformPropertyGetUint(%s,%s,%lu)\n",pszKey,pszVal,uDefault));

  /* Get value as a ui32 */
  val = SWS_CfgGetValue( pszKey, pszVal, uDefault);
  if ( val != uDefault)
    {
    TRACE( 9, ("PlatformPropertyGetUint() %lu\n", val));
    return val;
    }

  /* Get value as a string */
  if ( 0 != SWS_CfgGetString( pszKey, pszVal, "", szBuf, sizeof( szBuf) ))
    {
    /* Try a dotted IP address */
    if ( 0 == strcmp( "255.255.255.255", szBuf))
      return SWS_INADDR_NONE;

    addr = SWS_inet_addr( szBuf);
    if ( SWS_INADDR_NONE != addr)
      {
      TRACE( 9, ("PlatformPropertyGetUint() %s\n", szBuf));
      return SWS_ntohl( addr);
      }
    }

  return uDefault;
  }


int PlatformPropertyGetString(
  const char* pszKey,
  const char* pszVal,
  const char* pszDefault,
  char* pszBuf,
  size_t len
) {
  TRACE( 9, ("PlatformPropertyGetString(%s,%s,%s,%p,%lu)\n",
    pszKey,pszVal,pszDefault,pszBuf,len));
  ASSERT( NULL != pszKey);
  ASSERT( NULL != pszVal);
  ASSERT( NULL != pszDefault);

  return SWS_CfgGetString( pszKey, pszVal, pszDefault, pszBuf, len);
  }


/*
 * Monotonic millisecond timer
 */
ui32 PlatformTime( void)
  {
#if 1 || defined NDEBUG /* Best for resolution but locks up rawhide debugger */
  static char s_cTimed;
  static ui32 s_millisecsSinceMidnight;
  uclock_t ticks;
  ui32 ms;
  static ui32 s_ms;

  /* Once off Millisecs since midnight calculation */
  if ( !s_cTimed)
    {
    struct time t;

    s_cTimed = 1;

    gettime( &t); /* Uses DOS int21(2c) time */

    s_millisecsSinceMidnight = (ui32)((ui32)t.ti_hour * 3600 * SWS_TIMER_TICKS
      + (ui32)t.ti_min * 60 * SWS_TIMER_TICKS
      + (ui32)t.ti_sec * SWS_TIMER_TICKS
      + ((ui32)t.ti_hund * SWS_TIMER_TICKS) / 100);
    }

  ticks = uclock(); /* UCLOCKS since system started */

  /* Convert to millisecs */
  ms = (ui32)(SWS_TIMER_TICKS * (ticks / UCLOCKS_PER_SEC))
     + (ui32)((SWS_TIMER_TICKS * (ticks % UCLOCKS_PER_SEC) + (UCLOCKS_PER_SEC / 2)) / UCLOCKS_PER_SEC);

  ms += s_millisecsSinceMidnight;

  /* Ensure monotonic - uclock on Win32 isn't! */
  if ( SWS_TimerLT( ms, s_ms))
    ms = s_ms;
  else
    s_ms = ms;

  return ms;

#elif 1
  struct time t;
  gettime( &t); /* Uses DOS int21(2c) time */

  return (ui32)((ui32)t.ti_hour * 3600 * SWS_TIMER_TICKS
    + (ui32)t.ti_min * 60 * SWS_TIMER_TICKS
    + (ui32)t.ti_sec * SWS_TIMER_TICKS
    + ((ui32)t.ti_hund * SWS_TIMER_TICKS) / 100);

#elif 1
  struct timeval t;
  gettimeofday( &t, NULL); /* Uses DOS int21(2c) time & date */

  return (ui32)((ui32)t.tv_sec * SWS_TIMER_TICKS
    + t.tv_usec / SWS_TIMER_TICKS);

#else
  /* NB clock returns elapsed process time, not real time */
  return (ui32)((clock() * SWS_TIMER_TICKS) / CLOCKS_PER_SEC);
#endif
  }


/*
 * Start the timer
 */
int PlatformTimerStart(
  ui32 uTimeout,
  SWS_FNPTR( FPlatformTimeout) pfn,
  SWS_PTR( void) pvContext
) {
  TRACE( 9, ("PlatformTimerStart(%lu,%p,%p)\n",uTimeout,pfn,pvContext));

  PlatformBeginCritical();

  s_bTimerRunning = 0;
  s_uTimeout = uTimeout;
  s_pfnTimer = pfn;
  s_pvTimerContext = pvContext;
  s_bTimerRunning = 1;

  PlatformEndCritical();

  return 0;
  }


/*
 * Stop the timer
 */
void PlatformTimerStop( void )
  {
  TRACE( 9, ("PlatformTimerStop()\n"));

  PlatformBeginCritical();

  s_bTimerRunning = 0;
  s_uTimeout = 0;
  s_pfnTimer = 0;

  PlatformEndCritical();
  }


/*
 * Idle time processing
 */
void PlatformIdle( void)
  {
  SDpc dpc;
  int isIdle = s_bYield;

  TRACE( 9, ("PlatformIdle()\n"));

  PlatformBeginCritical();

  /* Process deferred procedure calls */
  while ( !SWS_MsgqGet( s_hDpcQ, sizeof(dpc), &dpc))
    {
    ASSERT( dpc.pfn);
    (*dpc.pfn)( dpc.pvContext, dpc.pvParam, dpc.uParam);
    isIdle = 0;
    }

  /* Check for timer expiry */
  if ( s_bTimerRunning)
    {
    ui32 uTicks = PlatformTime();
    if ( SWS_TimerLT( s_uTimeout, uTicks))
      {
      s_bTimerRunning = 0;
      if ( s_pfnTimer)
        {
        (*s_pfnTimer)( s_pvTimerContext);
        isIdle = 0;
        }
      }
    }

  PlatformEndCritical();

#if OPT_THREADS == OPT_THREADS_COOP
  SWS_ThreadYield();
#endif

  /* Yield CPU (if running on Win32) */
  if ( isIdle)
    __dpmi_yield();
  }


/*
 * Serialise access to a shared resource using a callback
 */
int PlatformSynchronise(
  SWS_FNPTR( FPlatformCallback) pfn,
  SWS_PTR( void) pv
) {
  int iRet;

  TRACE( 9, ("PlatformSynchronise(%p,%p)\n", pfn, pv));
  ASSERT( pfn);

  PlatformBeginCritical();
  iRet = (*pfn)( pv);
  PlatformEndCritical();

  return iRet;
  }


/*
 * Get platform critical state
 */
int PlatformIsCritical( void)
  {
  return (int)AtomicRead( &s_critical);
  }


/*
 * Allocate an event
 */
EventH PlatformCreateEvent( void)
  {
  static size_t s_uLast;
  size_t u;
  SEvent* pEvent;

  TRACE( 9, ("PlatformCreateEvent()\n"));

  PlatformBeginCritical();

  u = s_uLast;
  while ( s_pEvents[ u].inUse)
    {
    if ( ++u >= s_events)
      u = 0;

    if ( s_uLast == u)
      {
      TRACE( 1, ("*** PlatformCreateEvent() Out of handles"));
      PlatformEndCritical();
      return NULL;
      }
    }
  s_uLast = u;

  pEvent = &s_pEvents[ u];
  pEvent->inUse = 1;
  AtomicWrite ( &pEvent->val, 0);

  PlatformEndCritical();

  return pEvent;
  }


/*
 * Free an event
 */
void PlatformDestroyEvent( EventH hEvent)
  {
  ASSERT( hEvent >= s_pEvents);
  ASSERT( hEvent < s_pEvents + s_events);
  ASSERT( hEvent->inUse );

  hEvent->inUse = 0;
  }


/*
 * Set an event
 */
void PlatformSetEvent( EventH hEvent, i32 iVal)
  {
  TRACE( 9, ("PlatformSetEvent(%p,%ld)\n", hEvent, iVal));
  ASSERT( hEvent >= s_pEvents);
  ASSERT( hEvent < s_pEvents + s_events);
  ASSERT( hEvent->inUse );

  AtomicWrite ( &hEvent->val, (int)iVal);
  }


/*
 * Get an event
 */
i32 PlatformGetEvent( EventH hEvent)
  {
  TRACE( 9, ("PlatformGetEvent(%p)\n", hEvent));
  ASSERT( hEvent >= s_pEvents);
  ASSERT( hEvent < s_pEvents + s_events);
  ASSERT( hEvent->inUse );

  return AtomicRead( &hEvent->val);
  }


/*
 * Wait for an event
 */
i32 PlatformWaitEvent( EventH hEvent, ui32 ticks)
  {
  int val;

  TRACE( 9, ("PlatformWaitEvent(%p,%lu)\n", hEvent, ticks));
  ASSERT( hEvent >= s_pEvents);
  ASSERT( hEvent < s_pEvents + s_events);
  ASSERT( hEvent->inUse );

  switch ( ticks)
    {
  case WAIT_NEVER:
    /* No wait */
    val = AtomicXchg( &hEvent->val, 0);
    break;

  case WAIT_FOREVER:
    /* Indefinite wait */
    while ( 0 == (val = AtomicXchg( &hEvent->val, 0)))
      {
      PlatformIdle();
      }
    break;

  default:
    /* Timed wait */
    if ( 0 == (val = AtomicXchg( &hEvent->val, 0)))
      {
      ui32 uTimeout = PlatformTime() + ticks;
      do
        {
        PlatformIdle();
        val = AtomicXchg( &hEvent->val, 0);
        }
      while ( 0 == val && !SWS_TimerLT( uTimeout, PlatformTime()) );
      }
    break;
    }

  return (i32) val;
  }


/*
 * Wait for any event to be set
 */
void PlatformWaitAnyEvent( ui32 ticks)
  {
  TRACE( 9, ("PlatformWaitAnyEvent(%lu)\n", ticks));
  (void)ticks;

  PlatformIdle();
  }


/*
 * Schedule a deferred procedure
 * !!! CAUTION IRQ TIME !!!
 */
int PlatformDpc(
  SWS_FNPTR( FPlatformDpc) pfnDpc,
  SWS_PTR( void) pv1,
  SWS_PTR( void) pv2,
  ui32 uParam
) {
  SDpc dpc;
  int iRet, i_state;

  TRACE( 10, ("PlatformDpc(%p,%p,%p,%lu)\n", pfnDpc,pv1,pv2,uParam));

  dpc.pfn = pfnDpc;
  dpc.pvContext = pv1;
  dpc.pvParam = pv2;
  dpc.uParam = uParam;

  /* Add the request to the DPC queue */
  i_state = disable();
  iRet = SWS_MsgqPut( s_hDpcQ, sizeof(dpc), &dpc);
  if ( i_state)
    enable();

  if ( iRet)
    TRACE( 1, ( "*** PlatformDpc(%p) Qfull\n",pfnDpc));
  return iRet;
  }


#if !OPT_ATOMIC_INTRINSIC
/*
 * Atomic operations
 */
#if !defined ATOMIC_BUILTIN && defined __GNUC__
/* BUG DJGPP version 4.3.2 doesn't implement these builtins */
#define ATOMIC_BUILTIN 0 /* !0 to use GNUC builtins */
#endif

SWS_FASTCALL( int, AtomicIncrement( Atomic_t* p))
  {
#if ATOMIC_BUILTIN
  return __sync_add_and_fetch( &p->v, 1);

#elif 1
  int const v0 = p->v + 1;
  __asm__ __volatile (
    "incl (%0)\n"
    :
    : "r" (p), "g" (p->v)
    : "cc"
  );
  return v0;
#else
  const int i_state = disable();
  const int v0 = ++(p->v);
  if ( i_state)
    enable();
  return v0;
#endif
  }

SWS_FASTCALL( int, AtomicDecrement( Atomic_t* p))
  {
#if ATOMIC_BUILTIN
  return __sync_sub_and_fetch( &p->v, 1);

#elif 1
  int const v0 = p->v - 1;
  __asm__ __volatile (
    "decl (%0)\n"
    :
    : "r" (p), "g" (p->v)
    : "cc"
  );
  return v0;
#else
  const int i_state = disable();
  const int v0 = --(p->v);
  if ( i_state)
    enable();
  return v0;
#endif
  }

SWS_FASTCALL( int, AtomicAdd( Atomic_t* p, int v))
  {
#if ATOMIC_BUILTIN
  return __sync_add_and_fetch( &p->v, v);

#elif 1
  int const v0 = p->v + v;
  __asm__ __volatile (
    "lock addl %1, (%0)\n"
    :
    : "r" (p), "r" (v), "g" (p->v)
    : "cc"
  );
  return v0;
#else
  const int i_state = disable();
  const int v0 = (p->v += v);
  if ( i_state)
    enable();
  return v0;
#endif
  }

SWS_FASTCALL( int, AtomicXchg( Atomic_t* p, int v))
  {
#if ATOMIC_BUILTIN
  return __sync_lock_test_and_set( &p->v, v);

#elif 1
  register int c asm( "eax") = v;
  __asm__ __volatile (
    "xchgl %0, (%1)\n"
    : "+r" (c)
    : "r" (p), "g" (p->v)
  );
  return c;
#else
  const int i_state = disable();
  const int v0 = p->v;
  p->v = v;
  if ( i_state)
    enable();
  return v0;
#endif
  }

SWS_FASTCALL( int, AtomicCmpXchg(
  register Atomic_t* p,
  int compare,
  register int update
)){
#if ATOMIC_BUILTIN
  return __sync_val_compare_and_swap( &p->v, compare, update);

#elif 1
  register int c asm( "eax") = compare;
  __asm__ __volatile (
    "cmpxchgl %2, (%1)\n" /* if (*p == c) *p = update */
    : "+r"(c)
    : "r" (p), "r" (update), "g" (p->v)
    : "cc"
  );
  return c;
#else
  const int i_state = disable();
  const int v0 = p->v;
  if ( p->v == compare)
    p->v = update;
  if ( i_state)
    enable();
  return v0;
#endif
  }

#else

SWS_FASTCALL( int, AtomicXchg( Atomic_t* p, int v))
  {
  const int v0 = p->v;
  p->v = v;
  return v0;
  }

SWS_FASTCALL( int, AtomicCmpXchg( Atomic_t* p, int compare, int update))
  {
  const int v0 = p->v;
  if ( p->v == compare)
    p->v = update;
  return v0;
  }
#endif


#if OPT_THREADS == OPT_THREADS_COOP
/****************************************************************************
 * Cooperative Threads
 ****************************************************************************/

/* Thread instance data */
typedef struct SThread
  {
  SWS_Thread thrd;

  /* Context */
  jmp_buf jbuf;
  SWS_VARRAY( double, stack); /* NB variable length */
  } SThread;

#define DJGPP_THREAD( p) SWS_CONTAINER( p, SThread, thrd)
#define OPT_STACKSIZE (10*4096) /* Default stack size */

/* Prototypes */
#ifdef __cplusplus
extern "C" {
#endif

static void PlatformThreadExecute( void);

#ifdef __cplusplus
}
#endif


/*
 * Switch stacks
 */
#define PlatformSwitchStack( newSP)\
  ({ void* __oldSP;\
  __asm__ __volatile (\
    "xchgl %0, %%esp\n" /* NB assume SS=DS */\
    : "=r" (__oldSP)\
    : "0" (newSP)\
  );\
  __oldSP;\
  })


/*
 * Allocate a thread instance
 */
SWS_Thread* PlatformThreadAlloc( size_t stack)
  {
  /* C99 7.13.2.3: After setjmp, auto vars not qualified by volatile have
   * indeterminate value */
  struct SThread* volatile pThread;

  TRACE( 8, ("PlatformThreadAlloc(%u)\n",stack));

  /* Set stack size */
  if ( 1 == stack)
    {
    /* Don't allocate stack for main thread */
    stack = 0;
    }
  else if ( stack < 4096)
    {
    static size_t s_uStackSize;
    if ( 0 == s_uStackSize)
      s_uStackSize = (size_t)PlatformPropertyGetUint( PKEY_DJGPP_STACK, OPT_STACKSIZE );

    stack = s_uStackSize;
    }

  /* Allocate instance data plus stack */
  pThread = (SThread*) malloc( sizeof( SThread) + stack);
  if ( NULL == pThread)
    {
    TRACE( 0, ("!!! PlatformThreadAlloc() out of memory\n"));
    return NULL;
    }

  /* Main thread has no stack */
  if ( stack)
    {
    static struct SThread* s_pThread;
    static void* s_SP;
    void* pvTOS;

    /* NB allow for pre & post-decrement stacks by keeping a guard word */
    pvTOS = (char*)(pThread->stack) + stack - sizeof(double);

    /* Switch to new stack */
    s_pThread = pThread;
    s_SP = PlatformSwitchStack( pvTOS );
    /* NB auto vars no longer accessible */

    /* NB setjmp must be called inline otherwise the new thread's stack
     * will be corrupted after the return
     * C99 7.13.1.1: setjmp is a macro and a reserved identifier
     */
    if ( 0 != setjmp( s_pThread->jbuf) )
      {
      /* NB Do not use local vars */

      /* Run the current thread */
      PlatformThreadExecute();

      /* Should never get here */
      ASSERT( !"Zombie thread");
      }

    /* Restore stack */
    (void)PlatformSwitchStack( s_SP);
    /* auto vars now accessible */
    }

  return &pThread->thrd;
  }


/*
 * Deallocate a thread instance
 */
void PlatformThreadFree( SWS_Thread* pThread )
  {
  TRACE( 8, ("PlatformThreadFree(%p)\n",pThread));
  ASSERT( NULL != pThread);

  free( DJGPP_THREAD( pThread));
  }


/*
 * Suspend a thread and resume another
 */
void PlatformThreadSwitch(
  SWS_Thread* pThread,
  SWS_Thread* pNextThread
) {
  TRACE( 8, ("PlatformThreadSwitch(%p,%p)\n", pThread,pNextThread));
  ASSERT( NULL != pNextThread);
  ASSERT( pNextThread != pThread);

  /* Save current context */
  if ( NULL == pThread
    || 0 == setjmp( DJGPP_THREAD( pThread)->jbuf )
  ) {
    /* Restore context for next runnable thread */
    longjmp( DJGPP_THREAD( pNextThread)->jbuf, 1);
    }
  }


/*
 * Thread function
 */
void PlatformThreadExecute( void)
  {
  SWS_Thread* const pThread = SWS_ThreadCurrent();
  TRACE( 8, ("PlatformThreadExecute() %p\n",pThread));
  ASSERT( NULL != pThread);

  /* Run the thread */
  SWS_ThreadExit( (*pThread->pfn)( pThread->pvContext) );
  }
#endif /* OPT_THREADS == OPT_THREADS_COOP */


/****************************************************************************
 * Debug
 ****************************************************************************/

#ifdef DEBUG_TRACE_LEVEL
extern FILE* g_logfile;

/*
 * Debug trace handler
 */
void TraceToLog( const char* psz)
  {
  if ( NULL != psz)
    {
    if ( NULL != g_logfile)
      {
      ui32 ticks = PlatformTime();
      unsigned hr, mn, sc, ms;

      /* Add a timestamp to trace output */
      sc = (unsigned)(ticks / SWS_TIMER_TICKS);
      ms = (unsigned)(ticks - (ui32)sc * SWS_TIMER_TICKS);
      mn = sc / 60;
      sc -= 60 * mn;
      hr = mn / 60;
      mn -= 60 * hr;
      hr %= 24;

      fprintf( g_logfile, "%02u:%02u:%02u.%03u: %s", hr, mn, sc, ms, psz);
      fflush( g_logfile);
      }
    }
  else
    {
    fflush( g_logfile); /* NULL == all files */
    }
  }
#endif


#ifdef ALERT_FN
/*
 * Assertion failure handler
 */
short DebugAlert( const char* psz)
  {
  (void)psz;

#if defined DEBUG || defined _DEBUG
  return ALERT_RETRY; /* Asserts break to the debugger */
#else
  return ALERT_ABORT;
#endif
  }
#endif

/* End of file */
