/****************************************************************************
 *
 * File name   : posix.c
 * Function    : Posix support functions.
 * Project     : SwsSock
 * Authors     : Lawrence Rust
 * Systems     : ANSI C, Posix.
 *
 ****************************************************************************
 *
 * Created by Lawrence Rust, Software Systems Consultants
 * lvr@softsystem.co.uk.
 *
 * 11-Aug-06, LVR, Created.
 * 14-Aug-06, LVR, Support packet sockets
 *
 * Comments:
 * --------
 * This file only contains functions that are Posix specific
 *
 ****************************************************************************
 */

/* Compilation options */
#ifndef _GNU_SOURCE
#define _GNU_SOURCE     /* For F_SETSIG */
#endif

#include "config.h"

#ifndef OPT_PACKETX
#define OPT_PACKETX 0 /* !0 for packet sockets */
#endif
#ifndef OPT_POSIXCOMMS
#define OPT_POSIXCOMMS 0 /* !0 for p2p serial comms */
#endif

/* Exports */
#include "platform.h"

/* Imports */
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <ctype.h>
#include <signal.h>

/* Posix */
#include <unistd.h> /* usleep */
#include <sys/time.h> /* gettimeofday */
#if OPT_THREADS == OPT_THREADS_COOP
# include <setjmp.h>
#elif OPT_THREADS == OPT_THREADS_NATIVE
# include <pthread.h> /* POSIX threads */
#endif

/* sws_sock */
#include "sws_sock.h"
#include "sws_timr.h"
#include "sws_msgq.h"
#include "sws_cfg.h"
#if OPT_THREADS == OPT_THREADS_COOP
# include "thread.h"
#elif OPT_THREADS == OPT_THREADS_NATIVE
# include "sws_thrd.h"
#endif

#if OPT_DATALINK_P2P
# if OPT_POSIXCOMMS
#  include "psxcomms.h"
# else
#  include "fakep2p.h"
# endif
#endif

#if OPT_DATALINK_ETHERNET
# if OPT_PACKETX
#  include "packetx.h" /* ethernet via packet socket */
# else
#  include "fakenic.h"
# endif
#endif

#include "debug.h"


/* Macros & constants */
#ifndef PSECT_POSIX
#define PSECT_POSIX "posix"
#endif

/* The signal that's waited on by the DPC handler thread */
#define PKEY_SIGNAL "signal"  /* Numeric */
#ifndef POSIX_SIGNAL
#define POSIX_SIGNAL SIGUSR1
#endif


/*
 * Local data types
 */
/* An event */
typedef struct SEvent
  {
  Atomic_t val;
#if OPT_THREADS == OPT_THREADS_NATIVE
  pthread_mutex_t mtx;
  pthread_cond_t cond;
#endif
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


/*
 * Prototypes
 */

static void timer_signal( int iSig);

/* Critical section control */
#if OPT_THREADS == OPT_THREADS_NATIVE
#define PlatformBeginCritical()\
  (pthread_mutex_lock( &s_mutexCrit), AtomicIncrement( &s_critical))
#define PlatformEndCritical()\
  (AtomicDecrement( &s_critical), pthread_mutex_unlock( &s_mutexCrit))
static void* PlatformDpcDispatcher( void*);

#else
#define PlatformBeginCritical() AtomicIncrement( &s_critical)
#define PlatformEndCritical() AtomicDecrement( &s_critical)
#endif


/*
 * Module data
 */

/* Configuration */
static size_t s_events;               /* No. events */
#if OPT_THREADS == OPT_THREADS_NATIVE
static int s_iSignal;                 /* signal to the DPC handler */
#endif

static int s_iInited;                 /* Module initialised */
static SEvent* s_pEvents;             /* Array of events */
#if OPT_THREADS == OPT_THREADS_NATIVE
static pthread_t s_idIdle;
static pthread_mutex_t s_mutexCrit;   /* Thread critical */
#endif
static Atomic_t s_critical;           /* Critical section nesting depth */
static SWS_MsgqH s_hDpcQ;             /* DPC queue */

/* TimerQ */
static SWS_FNPTR( FPlatformTimeout) s_pfnTimer;
static SWS_PTR( void) s_pvTimerContext;

/* TRACE support */
#ifdef DEBUG_TRACE_LEVEL
 static void TraceToLog( const char*);
# if !defined DEBUG
#  define DEBUG 1 /* Default trace level */
# endif
#endif
DEBUG_TRACE( DEBUG, &TraceToLog)

/* ASSERT support */
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
  ASSERT( !PlatformIsCritical());

  if ( 1 == ++s_iInited)
    {
    size_t uDpcs;

    /* Get configuration data */
    uDpcs = (size_t)PlatformPropertyGetUint( PKEY_DPC_COUNT, OPT_DPCS );
    s_events = (size_t)PlatformPropertyGetUint( PKEY_EVENT_COUNT, OPT_EVENTS );
#if OPT_THREADS == OPT_THREADS_NATIVE
    /* The signal that's waited on by the DPC handler */
    s_iSignal = PlatformPropertyGetUint( PSECT_POSIX, PKEY_SIGNAL, POSIX_SIGNAL );
#endif

    /* Create event pool */
    s_pEvents = (SEvent*)calloc( s_events, sizeof *s_pEvents);
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
      free ( s_pEvents), s_pEvents = NULL;
      s_iInited = 0;
      return SWS_ENOBUFS;
      }

#if OPT_THREADS == OPT_THREADS_NATIVE
    {
    pthread_mutexattr_t attr;
    sigset_t sigs;

    /* Create the 'critical section' mutex, which is recursive */
    pthread_mutexattr_init( &attr);
#ifdef PTHREAD_MUTEX_RECURSIVE_NP
    pthread_mutexattr_settype( &attr, PTHREAD_MUTEX_RECURSIVE_NP);
#else
    pthread_mutexattr_settype( &attr, PTHREAD_MUTEX_RECURSIVE);
#endif
    if ( pthread_mutex_init( &s_mutexCrit, &attr))
      TRACE( 0, ("!!! PlatformInit pthread_mutex_init failed\n"));
    pthread_mutexattr_destroy( &attr);

    /* Block the signal that's waited on by the DPC handler */
    sigemptyset( &sigs);
    sigaddset( &sigs, s_iSignal);
    sigprocmask( SIG_BLOCK, &sigs, NULL);

    /* Create DPC handler thread */
    if ( pthread_create( &s_idIdle, NULL, &PlatformDpcDispatcher, NULL))
      TRACE( 0, ("!!! PlatformInit pthread_create failed: %s\n", strerror( errno)));
    }
#endif
    }

  return 0;
  }


/*
 * SwsSock has now been initialised
 */
int PlatformPostInit( void)
  {
  TRACE( 9, ("PlatformPostInit()\n"));
  ASSERT( s_iInited > 0);
  ASSERT( PlatformIsCritical());

#if OPT_DATALINK_ETHERNET
# if OPT_PACKETX
  /* Install the packet socket datalink */
  (void)SWS_PacketXInstall();
# elif defined DEBUG
  /* Install the fake NIC */
  (void)SWS_FakeInstall();
# endif
#endif

#if OPT_DATALINK_P2P
# if OPT_POSIXCOMMS
  /* Install the serial comms adapter */
  (void)SWS_PosixCommsInstall();
# elif defined DEBUG
  /* Install the fake P2P adapter */
  SWS_FakeP2pInstall();
# endif
#endif

  return 0;
  }


/*
 * SwsSock is about to terminate
 */
void PlatformPreCleanup( void)
  {
  TRACE( 9, ("PlatformPreCleanup()\n"));
  ASSERT( s_iInited > 0);

  PlatformTimerStop();

#if OPT_THREADS == OPT_THREADS_NATIVE
  if ( s_idIdle)
    pthread_cancel( s_idIdle);
#endif
  }


/*
 * Platform specific cleanup
 */
void PlatformCleanup( void)
  {
  TRACE( 9, ("PlatformCleanup()\n"));

  PlatformBeginCritical();

  ASSERT( s_iInited > 0);
  if ( 0 == --s_iInited)
    {
#if OPT_POSIXCOMMS && OPT_POSIXCOMMS
    SWS_PosixCommsCleanup();
#endif

#if OPT_DATALINK_ETHERNET && OPT_PACKETX
    SWS_PacketXCleanup();
#endif

#if OPT_THREADS == OPT_THREADS_NATIVE
    if ( s_idIdle)
      {
      void* pv;
      pthread_join( s_idIdle, &pv);
      s_idIdle = 0;
      }
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

    AtomicWrite( &s_critical, 0);
#if OPT_THREADS == OPT_THREADS_NATIVE
    pthread_mutex_destroy( &s_mutexCrit);
#endif
    return;
    }

  PlatformEndCritical();
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
  struct timeval tv;
  struct timezone tz;
  static time_t s_tMidnight;

  gettimeofday( &tv, &tz);

  if ( 0 == s_tMidnight)
    s_tMidnight = tv.tv_sec - (tv.tv_sec % (24 * 60 * 60)) + tz.tz_minuteswest * 60;

  /* Return milliseconds from midnight */
  return (ui32)((tv.tv_sec - s_tMidnight) * SWS_TIMER_TICKS
    + (SWS_TIMER_TICKS * tv.tv_usec) / 1000000);
  }


/*
 * Start the timer
 */
int PlatformTimerStart(
  ui32 uTimeout,
  SWS_FNPTR( FPlatformTimeout) pfn,
  SWS_PTR( void) pvContext
) {
  ui32 uNow;

  TRACE( 4, ("PlatformTimerStart(%lu,%p,%p)\n",uTimeout,pfn,pvContext));
  ASSERT( pfn);

  PlatformBeginCritical();
#if OPT_THREADS == OPT_THREADS_NATIVE
  pthread_cleanup_push( (void (*)(void*))pthread_mutex_unlock, &s_critical);
#endif

  PlatformTimerStop();

  uNow = PlatformTime();
  if ( SWS_TimerLE( uTimeout, uNow))
    {
    (*pfn)( pvContext);
    }
  else
    {
    struct itimerval tv;

    s_pvTimerContext = pvContext;
    s_pfnTimer = pfn;
    if ( SIG_ERR == signal( SIGALRM, &timer_signal))
      TRACE( 1, ("*** PlatformTimerStart() signal failed %d\n", errno));

    tv.it_interval.tv_sec = 0;
    tv.it_interval.tv_usec = 0;
    uTimeout -= uNow;
    tv.it_value.tv_sec = uTimeout / SWS_TIMER_TICKS;
    tv.it_value.tv_usec = (uTimeout % SWS_TIMER_TICKS) * (1000000UL / SWS_TIMER_TICKS);
    if ( setitimer( ITIMER_REAL, &tv, 0))
      {
      TRACE( 1, ("*** PlatformTimerStart() setitimer failed %d\n", errno));
      signal( SIGALRM, SIG_DFL);
      }
    }

  PlatformEndCritical();
#if OPT_THREADS == OPT_THREADS_NATIVE
  pthread_cleanup_pop( 0);
#endif
  return 0;
  }


/*
 * setitimer signal handler
 */
static void timer_signal( int iSig)
  {
  SWS_FNPTR( FPlatformTimeout) pfn = s_pfnTimer;
  SWS_PTR( void) pv = s_pvTimerContext;

  TRACE( 4, ("timer_signal(%d)\n",iSig));
  (void)iSig;

  s_pfnTimer = 0;
  s_pvTimerContext = NULL;
  if ( pfn)
    PlatformDpc( (SWS_FNPTR( FPlatformDpc))pfn, pv, NULL, 0);
  }


/*
 * Stop the timer
 */
void PlatformTimerStop( void )
  {
  struct itimerval tv;

  TRACE( 4, ("PlatformTimerStop()\n"));

  /* Stop the timer */
  tv.it_interval.tv_sec = 0;
  tv.it_interval.tv_usec = 0;
  tv.it_value.tv_sec = 0;
  tv.it_value.tv_usec = 0;
  if ( setitimer( ITIMER_REAL, &tv, 0))
    TRACE( 1, ("*** PlatformTimerStop() setitimer failed %d\n", errno));

  signal( SIGALRM, SIG_IGN);
  s_pfnTimer = 0;
  s_pvTimerContext = NULL;
  }


/*
 * Idle time processing
 */
void PlatformIdle( void)
  {
  SDpc dpc;

  TRACE( 9, ("PlatformIdle()\n"));

  PlatformBeginCritical();
#if OPT_THREADS == OPT_THREADS_NATIVE
  pthread_cleanup_push( (void (*)(void*))pthread_mutex_unlock, &s_critical);
#endif

  /* Process deferred procedure calls */
  while ( !SWS_MsgqGet( s_hDpcQ, sizeof(dpc), &dpc))
    {
    ASSERT( dpc.pfn);
    (*dpc.pfn)( dpc.pvContext, dpc.pvParam, dpc.uParam);
    }

  PlatformEndCritical();
#if OPT_THREADS == OPT_THREADS_NATIVE
  pthread_cleanup_pop( 0);
#elif OPT_THREADS == OPT_THREADS_COOP
  usleep( 1); /* Needed for SIGALRM delivery. NB 0= forever */
  SWS_ThreadYield();
#else
  usleep( 1);
#endif
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
#if OPT_THREADS == OPT_THREADS_NATIVE
  pthread_cleanup_push( (void (*)(void*))pthread_mutex_unlock, &s_critical);
#endif

  iRet = (*pfn)( pv);

  PlatformEndCritical();
#if OPT_THREADS == OPT_THREADS_NATIVE
  pthread_cleanup_pop( 0);
#endif

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
      TRACE( 1, ("*** PlatformCreateEvent()\n Out of handles"));
      PlatformEndCritical();
      return NULL;
      }
    }
  s_uLast = u;

  pEvent = &s_pEvents[ u];
  pEvent->inUse = 1;
  AtomicWrite ( &pEvent->val, 0);

  PlatformEndCritical();

#if OPT_THREADS == OPT_THREADS_NATIVE
  if ( pthread_mutex_init( &pEvent->mtx, NULL))
    {
    TRACE( 1, ("*** PlatformCreateEvent()\n pthread_mutex_init failed"));
    pEvent->inUse = 0;
    return NULL;
    }
  if ( pthread_cond_init( &pEvent->cond, NULL))
    {
    TRACE( 1, ("*** PlatformCreateEvent()\n pthread_cond_init failed"));
    pthread_mutex_destroy( &pEvent->mtx);
    pEvent->inUse = 0;
    return NULL;
    }
#endif

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

#if OPT_THREADS == OPT_THREADS_NATIVE
  pthread_cond_destroy( &hEvent->cond);
  pthread_mutex_destroy( &hEvent->mtx);
#endif
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

#if OPT_THREADS == OPT_THREADS_NATIVE
  pthread_mutex_lock( &hEvent->mtx);
  pthread_cleanup_push( (void (*)(void*))pthread_mutex_unlock, &hEvent->mtx);
#endif

  AtomicWrite( &hEvent->val, iVal);

#if OPT_THREADS == OPT_THREADS_NATIVE
  pthread_cond_signal( &hEvent->cond);
  pthread_mutex_unlock( &hEvent->mtx);
  pthread_cleanup_pop( 0);
#endif
  }


/*
 * Get an event
 */
i32 PlatformGetEvent( EventH hEvent)
  {
  i32 val;
  TRACE( 9, ("PlatformGetEvent(%p)\n", hEvent));
  ASSERT( hEvent >= s_pEvents);
  ASSERT( hEvent < s_pEvents + s_events);
  ASSERT( hEvent->inUse );

#if OPT_THREADS == OPT_THREADS_NATIVE
  pthread_mutex_lock( &hEvent->mtx);
#endif

  val = AtomicRead( &hEvent->val);

#if OPT_THREADS == OPT_THREADS_NATIVE
  pthread_mutex_unlock( &hEvent->mtx);
#endif

  return val;
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
#if OPT_THREADS == OPT_THREADS_NATIVE
    pthread_mutex_lock( &hEvent->mtx);
#endif
    val = AtomicXchg( &hEvent->val, 0);
#if OPT_THREADS == OPT_THREADS_NATIVE
    pthread_mutex_unlock( &hEvent->mtx);
#endif
    break;

  case WAIT_FOREVER:
    /* Indefinite wait */
#if OPT_THREADS == OPT_THREADS_NATIVE
    pthread_mutex_lock( &hEvent->mtx);
    pthread_cleanup_push( (void (*)(void*))pthread_mutex_unlock, &hEvent->mtx);
    while ( 0 == (val = AtomicXchg( &hEvent->val, 0)))
      pthread_cond_wait( &hEvent->cond, &hEvent->mtx);
    pthread_mutex_unlock( &hEvent->mtx);
    pthread_cleanup_pop( 0);
#else
    while ( 0 == (val = AtomicXchg( &hEvent->val, 0)))
      PlatformIdle();
#endif
    break;

  default:
    /* Timed wait */
#if OPT_THREADS == OPT_THREADS_NATIVE
    {
    struct timespec ts;
    struct timeval tv;

    gettimeofday( &tv, NULL);
    ts.tv_sec = tv.tv_sec + ticks / SWS_TIMER_TICKS;
    ts.tv_nsec = tv.tv_usec * 1000
      + (ticks % SWS_TIMER_TICKS) * (1000000000L / SWS_TIMER_TICKS);
    if ( ts.tv_nsec >= 1000000000L)
      {
      ts.tv_nsec -= 1000000000L;
      ts.tv_sec++;
      }

    pthread_mutex_lock( &hEvent->mtx);
    pthread_cleanup_push( (void (*)(void*))pthread_mutex_unlock, &hEvent->mtx);
    while ( 0 == (val = AtomicXchg( &hEvent->val, 0)))
      {
      if ( pthread_cond_timedwait( &hEvent->cond, &hEvent->mtx, &ts))
        break;
      }
    pthread_mutex_unlock( &hEvent->mtx);
    pthread_cleanup_pop( 0);
    }
#else
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
#endif
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
  int iRet;
  SDpc dpc;

  TRACE( 10, ("PlatformDpc(%p,%p,%p,%lu)\n", pfnDpc,pv1,pv2,uParam));

  dpc.pfn = pfnDpc;
  dpc.pvContext = pv1;
  dpc.pvParam = pv2;
  dpc.uParam = uParam;

  /* Add the request to the DPC queue */
  iRet = SWS_MsgqPut( s_hDpcQ, sizeof(dpc), &dpc);

#if OPT_THREADS == OPT_THREADS_NATIVE
  /* Signal the DPC handler to process the DPCs */
  if ( s_idIdle)
    pthread_kill( s_idIdle, s_iSignal);
#endif

  return iRet;
  }


#if !OPT_ATOMIC_INTRINSIC
/*
 * Atomic operations
 */
#if !defined ATOMIC_BUILTIN && defined __GNUC__ && !defined __CYGWIN__
#define ATOMIC_BUILTIN 1 /* !0 to use GNUC builtins */
#endif

SWS_FASTCALL( int, AtomicIncrement( Atomic_t* p))
  {
#if ATOMIC_BUILTIN
  return __sync_add_and_fetch( &p->v, 1);

#elif defined __i386__
  int v;
  __asm__ __volatile (
    "lock movl (%1), %0\n"
    "lock incl (%1)\n"
    : "=&r" (v)
    : "r" (&p->v)
    : "cc"
  );
  return v + 1;

#elif defined __powerpc__ || defined __powerpc || defined powerpc
  int v0;
  __asm__ __volatile (
    "0:"
    "lwarx %0, 0, %1\n"     /* Load and reserve */
    "addi  %0, %0, 1\n"     /* ++ */
    "stwcx. %0, 0, %1\n"    /* *ptr = new */
    "bne-  0b\n"            /* Loop if reservation lost */
    : "=&r" (v0)
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

#elif defined __i386__
  int v;
  __asm__ __volatile (
    "lock movl (%1), %0\n"
    "lock decl (%1)\n"
    : "=&r" (v)
    : "r" (&p->v)
    : "cc"
  );
  return v - 1;

#elif defined __powerpc__ || defined __powerpc || defined powerpc
  int v0;
  __asm__ __volatile (
    "0:"
    "lwarx %0, 0, %1\n"     /* Load and reserve */
    "subi  %0, %0, 1\n"     /* -- */
    "stwcx. %0, 0, %1\n"    /* *ptr = new */
    "bne-  0b\n"            /* Loop if reservation lost */
    : "=&r" (v0)            /* NB & for unique o/p reg */
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

#elif defined __i386__
  int v0;
  __asm__ __volatile (
    "lock movl (%1), %0\n"
    "lock addl %2, (%1)\n"
    : "=&r" (v0)
    : "r" (&p->v), "r" (v)
    : "cc"
  );
  return v0 + v;

#elif defined __powerpc__ || defined __powerpc || defined powerpc
  int v0;
  __asm__ __volatile (
    "0:"
    "lwarx %0, 0, %1\n"     /* Load and reserve */
    "add   %0, %0, %2\n"    /* += */
    "stwcx. %0, 0, %1\n"    /* *ptr = new */
    "bne-  0b\n"            /* Loop if reservation lost */
    : "=&r" (v0)            /* NB & for unique o/p reg */
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

#elif defined __i386__
  int c = v;
  __asm__ __volatile (
    "xchgl %0, (%1)\n"
    : "+r" (c)
    : "r" (&p->v)
  );
  return c;

#elif defined __powerpc__ || defined __powerpc || defined powerpc
  int v0;
  __asm__ __volatile (
    "0:"
    "lwarx %0, 0, %1\n"     /* Load and reserve */
    "stwcx. %2, 0, %1\n"    /* *ptr = new */
    "bne-  0b\n"            /* Loop if reservation lost */
    : "=&r" (v0)            /* NB & for unique o/p reg */
    : "r" (p), "r" (v), "g" (p->v)
    : "cc"
  );
  return v0;

#else
  const int i_state = disable();
  const int v0 = p->v;
  p->v = v;
  if ( i_state)
    enable();
  return v0;
#endif
  }

SWS_FASTCALL( int, AtomicCmpXchg( Atomic_t* p, int compare, int update))
  {
#if ATOMIC_BUILTIN
  return __sync_val_compare_and_swap( &p->v, compare, update);

#elif defined __i386__
  int c;
  __asm__ __volatile (
    "movl %1, %%eax\n"
    "cmpxchgl %3, (%2)\n" /* if (*p == c) *p = update */
    "movl %%eax, %0\n"
    : "=r"(c)
    : "r" (compare), "r" (&p->v), "r" (update)
    : "cc", "%eax"
  );
  return c;

#elif defined __powerpc__ || defined __powerpc || defined powerpc
  int v0;
  __asm__ __volatile (
    "0:"
    "lwarx %0, 0, %1\n"     /* Load and reserve */
    "cmpw  %0, %2\n"        /* *ptr == compare */
    "bne-  1f\n"            /* No then jump */
    "stwcx. %3, 0, %1\n"    /* *ptr = hNew */
    "bne-  0b\n"            /* Loop if reservation lost */
    "1:"
    : "=&r" (v0)            /* NB & for unique o/p reg */
    : "r" (p), "r" (compare), "r" (update), "g" (p->v)
    : "cc"
  );
  return v0;

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

#else /* !OPT_ATOMIC_INTRINSIC */

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

#define POSIX_THREAD( p) SWS_CONTAINER( p, SThread, thrd)
#define OPT_STACKSIZE 32768 /* Default stack size */

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
#if defined __i386__

#define PlatformSwitchStack( newSP)\
  ({ void* __oldSP;\
  __asm__ __volatile (\
    "xchgl %0, %%esp\n" /* NB assume SS=DS */\
    : "=r" (__oldSP)\
    : "0" (newSP)\
  );\
  __oldSP;\
  })

#elif defined __powerpc__ || defined __powerpc || defined powerpc

#define PlatformSwitchStack( newSP)\
  ({ void* __oldSP;\
  __asm__ __volatile (\
    "mr  %0, %%sp\n"      /* old sp */\
    "mr  %%sp, %1\n"      /* new sp */\
    : "=&r" (__oldSP)\
    : "r" (newSP)\
  );\
  __oldSP;\
  })

#else
#error CPU not supported
#endif


/*
 * Allocate a thread instance
 */
SWS_Thread* PlatformThreadAlloc( size_t stack)
  {
  /* C99 7.13.2.3: After setjmp, auto vars not qualified by volatile have
   * indeterminate value */
  struct SThread* volatile pThread;

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
      s_uStackSize = (size_t)PlatformPropertyGetUint( "posix", "stack", OPT_STACKSIZE );

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
    s_SP = PlatformSwitchStack( pvTOS);
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

  free( POSIX_THREAD( pThread));
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
    || 0 == setjmp( POSIX_THREAD( pThread)->jbuf )
  ) {
    /* Restore context for next runnable thread */
    longjmp( POSIX_THREAD( pNextThread)->jbuf, 1);
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


#elif OPT_THREADS == OPT_THREADS_NATIVE
/****************************************************************************
 * Native Threads
 ****************************************************************************/

/* Thread instance data */
typedef struct SWS_Thread
  {
  ui32 magic;
# define THREAD_MAGIC SWS_MAKEFOURCC( 't','h','r','d')

  SWS_FNPTR( SWS_ThreadFunction) pfn;
  SWS_PTR( void) pvContext;

  pthread_t id;
  volatile i32 iExit;
  } SWS_Thread;

#define ThreadIsValid( h) (NULL != (h) && THREAD_MAGIC == (h)->magic)


/* Thread specific data */
static pthread_key_t s_key;

static void make_key( void)
  {
  (void) pthread_key_create( &s_key, NULL);
  }


/*
 * Native thread function
 */
static void* ThreadFunc( void* pv)
  {
  SWS_Thread* pThread = (SWS_Thread*)pv;
  static pthread_once_t s_key_once = PTHREAD_ONCE_INIT;

  TRACE( 8, ("ThreadFunc(%p)\n",pv));
  ASSERT( ThreadIsValid( pThread) );

  /* One-off creation of thread specific key */
  pthread_once( &s_key_once, &make_key);

  /* Save thread instance */
  pthread_setspecific( s_key, pThread);

  /* Run the thread */
  SWS_ThreadExit( (*pThread->pfn)( pThread->pvContext) );

  /* Should never get here */
  ASSERT( !"Zombie thread");
  return 0;
  }


/*
 * Create a thread
 */
SWS_EXPORT( SWS_ThreadHandle, SWS_ThreadCreate( /* OUT: Thread handle, 0= error */
  SWS_FNPTR( SWS_ThreadFunction) pfn,   /* IN: -> thread function */
  SWS_PTR( void) pvContext,             /* IN: Client context for SWS_ThreadFunction */
  SWS_size_t size                       /* IN: Stack size in bytes, 0= default */
)){
  SWS_Thread* pThread;
  int err;
  (void)size;

  TRACE( 9, ("SWS_ThreadCreate(%p,%p,%u)\n", pfn, pvContext, size));
  ASSERT( pfn);

  /* Allocate instance data */
  pThread = (SWS_Thread*) malloc( sizeof( SWS_Thread) );
  if ( NULL == pThread)
    {
    TRACE( 0, ("!!! SWS_ThreadCreate() out of memory\n"));
    return NULL;
    }

  /* Init thread data */
  memset( pThread, 0, sizeof( SWS_Thread) );
  pThread->pfn = pfn;
  pThread->pvContext = pvContext;
  pThread->magic = THREAD_MAGIC;

  err = pthread_create( &pThread->id, NULL, &ThreadFunc, pThread);
  if ( err)
    {
    TRACE( 0, ("*** SWS_ThreadCreate pthread_create failed %d\n", err));
    free( pThread);
    return NULL;
    }

  TRACE( 2, ("SWS_ThreadCreate(%p,%p,%u) => %p id:%#lx\n",
    pfn, pvContext, size, pThread, (long)pThread->id));
  ASSERT( ThreadIsValid( pThread) );
  return pThread;
  }


/*
 * Destroy a thread.
 * NB it is an error to destroy the thread that called SWS_SockStartup
 */
SWS_EXPORT( int, SWS_ThreadDestroy(     /* OUT: 0= no error */
  SWS_ThreadHandle hThread              /* IN: Thread, NULL= current thread */
)){
  pthread_t id;

  TRACE( 8, ("SWS_ThreadDestroy(%p)\n", hThread));

  if ( !ThreadIsValid( hThread))
    return SWS_EINVAL;

  /* Destroying the current thread? */
  id = hThread->id;
  if ( pthread_equal( id, pthread_self() ) )
    {
    /* Yes */
    hThread->magic = 0;
    free( hThread);
    pthread_exit( NULL);
    }
  /* Terminate the thread if it's still active */
  else
    {
    if ( id)
      {
      void* pv;
      pthread_cancel( id);
      pthread_join( id, &pv);
      }

    hThread->magic = 0;
    free( hThread);
    }

  return 0;
  }


/*
 * Exit a thread.
 * NB the thread that called SWS_SockStartup will call exit().
 */
SWS_EXPORT( void, SWS_ThreadExit(
  i32 iExit                            /* IN: Exit status */
)){
  SWS_Thread* hThread = SWS_ThreadCurrent();

  TRACE( 2, ("SWS_ThreadExit %p (%ld)\n",hThread,iExit));

  if ( ThreadIsValid( hThread))
    {
    /* Show thread terminated normally */
    hThread->iExit = iExit;
    hThread->id = 0;

    pthread_exit( hThread);
    }
  else if ( NULL == hThread)
    exit( (int)iExit);
  }


/*
 * Get the current thread handle
 */
SWS_EXPORT( SWS_ThreadHandle, SWS_ThreadCurrent( void))
  {
  TRACE( 9, ("SWS_ThreadCurrent()\n"));
  return (SWS_Thread*) pthread_getspecific( s_key);
  }


/*
 * Suspend and resume a thread
 */
SWS_EXPORT( int, SWS_ThreadStop(        /* OUT: 0= no error */
  SWS_ThreadHandle hThread              /* IN: Thread */
)){
  pthread_t id = ThreadIsValid( hThread) ? hThread->id : 0;

  TRACE( 8, ("SWS_ThreadStop(%p)\n", hThread));
  ASSERT( ThreadIsValid( hThread) );

  return id ? pthread_kill( id, SIGSTOP) : -1;
  }

SWS_EXPORT( int, SWS_ThreadStart(       /* OUT: 0= no error */
  SWS_ThreadHandle hThread              /* IN: Thread */
)){
  pthread_t id = ThreadIsValid( hThread) ? hThread->id : 0;

  TRACE( 8, ("SWS_ThreadStart(%p)\n", hThread));
  ASSERT( ThreadIsValid( hThread) );

  return id ? pthread_kill( id, SIGCONT) : -1;
  }


/*
 * Yield the current thread
 */
SWS_EXPORT( void, SWS_ThreadYield( void))
  {
  usleep( 1);
  }


/*
 * The DPC dispatcher thread
 */
static void* PlatformDpcDispatcher( void* pv)
  {
  sigset_t sigs;
  (void)pv;

  TRACE( 9, ("PlatformDpcDispatcher(%p)\n",pv));

  sigemptyset( &sigs);
  sigaddset( &sigs, s_iSignal);
  pthread_sigmask( SIG_BLOCK, &sigs, NULL);

  for (;;)
    {
    int iSig;

    /* Wait for signal from PlatformDpc */
    sigwait( &sigs, &iSig);

    /* Process deferred procedure calls */
    PlatformIdle();
    }

  return NULL;
  }
#endif


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
