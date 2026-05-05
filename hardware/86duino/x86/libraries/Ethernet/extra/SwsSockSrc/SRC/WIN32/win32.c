/****************************************************************************
 *
 * File name   : win32.c
 * Function    : Win32 support functions.
 * Project     : SwsSock
 * Authors     : Lawrence Rust
 * Systems     : ANSI C, Win32 (MSVC).
 *
 ****************************************************************************
 *
 * Created by Lawrence Rust, Software Systems Consultants
 * lvr@softsystem.co.uk. Tel/Fax +33 5 49 72 79 63
 *
 * 03-Apr-01, LVR, Created.
 * 24-Nov-02, LVR, Added SWS_kDatalinkProperty_HwAddress to Win32GetProperty
 * 03-Apr-03, LVR, Rework to add threads, NDIS driver and APC's
 *
 * Comments:
 * --------
 * This file only contains functions that are Win32 specific
 *
 ****************************************************************************
 */

/* Compilation options */
#if !defined OPT_RAW_SOCK && defined SWS_SOCK_TUNNEL
#define OPT_RAW_SOCK 1 /* !0 for SOCK_RAW/IPPROTO_RAW winsock datalink */
#endif

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0400 /* For waitable timers */
#endif

#include "config.h"


/* Exports */
#include "platform.h"

/* Imports */
/* ANSI C library */
#include <stddef.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

/* Win32 platform */
#if defined __MWERKS__
# pragma ANSI_strict off
#elif defined _MSC_VER
# pragma warning ( disable : 4201) /* nonstandard extension used : nameless struct/union */
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <mmsystem.h>

#ifdef _MSC_VER
#pragma comment( lib, "winmm.lib")
#endif

#if OPT_THREADS == OPT_THREADS_NATIVE
#include <process.h> /* _beginthreadex */
#endif

#if defined __MWERKS__
# pragma ANSI_strict reset
#elif defined _MSC_VER
# pragma warning ( default : 4201)
#endif

/* SwsSock */
#include "sws_timr.h"
#include "sws_msgq.h"
#include "sws_cfg.h"
#include "sws_sock.h"
#if OPT_DATALINK_P2P
#include "w32comms.h" /* Serial driver */
#endif
#if OPT_RAW_SOCK
#include "raw32.h" /* SOCK_RAW/IPPROTO_RAW winsock datalink */
#elif OPT_DATALINK_ETHERNET
#include "packet32.h" /* ethernet via ndis protocol driver */
#endif
#if OPT_THREADS == OPT_THREADS_COOP
#include "thread.h"
#endif
#include "debug.h"


/*
 * Macros & constants
 */
#define kszAppName "SwsSock"
#define kszCompanyName "Software Systems"
#define kszURL "lvr@softsystem.co.uk"


/*
 * Local data types
 */
/* An event */
typedef struct SEvent
  {
  Atomic_t val;
  char inUse;
  HANDLE hEvent;
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
/* Critical section control */
#define PlatformBeginCritical() do { \
  EnterCriticalSection( &s_section); AtomicIncrement( &s_critical); \
} while ( 0)
#define PlatformEndCritical() do { \
  AtomicDecrement( &s_critical); LeaveCriticalSection( &s_section); \
} while ( 0)

/* APC's */
static VOID APIENTRY TimerExpired( LPVOID, DWORD, DWORD);
static VOID APIENTRY ServiceDPC( ULONG_PTR);

#ifdef DEBUG_TRACE_LEVEL
static void TraceToLog( const char*);
static BOOL isDebuggerPresent( void);
#endif

#ifdef ALERT_FN
/* ASSERT handler */
static short DebugAlert( const char*);
static BOOL isDebuggerPresent( void);
#endif


/*
 * Module data
 */
/* Configuration */
static size_t s_events;               /* No. events */

static int s_iInited;                 /* Module initialised */
static SEvent* s_pEvents;             /* Array of events */
static Atomic_t s_critical;           /* Critical section nesting depth */
static CRITICAL_SECTION s_section;    /* Critical section */
static SWS_MsgqH s_hDpcQ;             /* DPC queue */
static CRITICAL_SECTION s_sectionDpc; /* DPC Critical section */
static LONG s_lDpcPending;            /* DPC(s) pending */
static HANDLE s_hTimer;               /* Waitable timer */
static HANDLE s_hThread;              /* Main thread */
static HANDLE s_hEvent;               /* Set on any event */
static DWORD s_dwTimeOffset;

/* TimerQ */
static SWS_FNPTR( FPlatformTimeout) s_pfnTimer;
static SWS_PTR( void) s_pvTimerContext;

/* Debug/trace data */
#if !defined DEBUG && DEBUG_TRACE_ENABLE
# define DEBUG 1 /* Default trace level */
#endif
#define DEBUG_LOGFILENAME kszAppName ".LOG"
DEBUG_TRACE( DEBUG, &TraceToLog) /* Default trace level & handler fn */
ALERT_DEFINE_HANDLER( &DebugAlert)

#if defined _DEBUG && _MSC_VER >= 1000
static _CrtMemState s_checkPt;
#endif


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
    SYSTEMTIME systime;

#if defined _DEBUG && _MSC_VER >= 1000
    /* Debug heap options */
    {
    int flag = _CrtSetDbgFlag( _CRTDBG_REPORT_FLAG );
    flag |= _CRTDBG_LEAK_CHECK_DF; /* Enable leak detection */
    //flag |= _CRTDBG_CHECK_ALWAYS_DF; /* Verify heap integrity on every call */
    //flag |= _CRTDBG_DELAY_FREE_MEM_DF; /* Dont reuse freed blocks */
    _CrtSetDbgFlag( flag );
    _CrtMemCheckpoint( &s_checkPt );
    }
#endif

#ifdef DEBUG_TRACE_LEVEL
    if ( isDebuggerPresent() && DEBUG_TRACE_LEVEL < 3)
      DEBUG_SET_TRACE_LEVEL( 2); /* Trace level w/debugger */
#endif

    /* Increase timeGetTime() resolution */
    timeBeginPeriod( 1);

    GetLocalTime( &systime);
    s_dwTimeOffset = systime.wMilliseconds
      + systime.wSecond * 1000
      + systime.wMinute * 60000
      + systime.wHour * 3600000
      - timeGetTime()
    ;

    /* Get a global handle to the current thread */
    s_hThread = INVALID_HANDLE_VALUE;
    if ( !DuplicateHandle(
        GetCurrentProcess(),
        GetCurrentThread(),
        GetCurrentProcess(),
        &s_hThread,
        0,
        FALSE,
        DUPLICATE_SAME_ACCESS
      )
    ) {
      TRACE( 0, ("!!! PlatformInit() DuplicateHandle failed %#x\n", GetLastError() ));
      s_iInited = 0;
      return SWS_ENOMEM;
      }
    ASSERT( INVALID_HANDLE_VALUE != s_hThread);

    /* Create critical section */
    InitializeCriticalSection( &s_section);
    InitializeCriticalSection( &s_sectionDpc);

    /* Create waitable timer */
    s_hTimer = CreateWaitableTimer( NULL, TRUE, NULL);
    if ( NULL == s_hTimer)
      {
      TRACE( 0, ("!!! PlatformInit() CreateWaitableTimer failed %#x\n",
        GetLastError() ));
      DeleteCriticalSection( &s_sectionDpc);
      DeleteCriticalSection( &s_section);
      CloseHandle( s_hThread);
      s_hThread = NULL;
      s_iInited = 0;
      return SWS_ENOMEM;
      }

    s_hEvent = CreateEvent( NULL, FALSE /*auto*/, FALSE,NULL);
    if ( NULL == s_hEvent)
      {
      TRACE( 0, ("!!! PlatformInit()\n CreateEvent failed"));
      CloseHandle( s_hTimer);
      s_hTimer = NULL;
      DeleteCriticalSection( &s_sectionDpc);
      DeleteCriticalSection( &s_section);
      CloseHandle( s_hThread);
      s_hThread = NULL;
      s_iInited = 0;
      return SWS_ENOMEM;
      }

    /* Get configuration data */
    uDpcs = (size_t)PlatformPropertyGetUint( PKEY_DPC_COUNT, OPT_DPCS );
    s_events = (size_t)PlatformPropertyGetUint( PKEY_EVENT_COUNT, OPT_EVENTS );

    /* Create event pool */
    s_pEvents = (SEvent*)calloc( s_events, sizeof(*s_pEvents));
    if ( NULL == s_pEvents)
      {
      TRACE( 0, ("!!!PlatformInit out of memory for events\n"));
      CloseHandle( s_hEvent);
      CloseHandle( s_hTimer);
      s_hTimer = NULL;
      DeleteCriticalSection( &s_sectionDpc);
      DeleteCriticalSection( &s_section);
      CloseHandle( s_hThread);
      s_hThread = NULL;
      s_iInited = 0;
      return SWS_ENOMEM;
      }

    /* Create DPC queue */
    s_hDpcQ = SWS_MsgqCreate( uDpcs, sizeof( SDpc));
    if ( NULL == s_hDpcQ)
      {
      free( s_pEvents);
      s_pEvents = NULL;
      CloseHandle( s_hEvent);
      CloseHandle( s_hTimer);
      s_hTimer = NULL;
      DeleteCriticalSection( &s_sectionDpc);
      DeleteCriticalSection( &s_section);
      CloseHandle( s_hThread);
      s_hThread = NULL;
      s_iInited = 0;
      return SWS_ENOBUFS;
      }
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

#if OPT_RAW_SOCK
  /* Install the RAW_IP datalink */
  RawIpInstall();
#elif OPT_DATALINK_ETHERNET
  /* Install the ndis protocol driver datalink */
  SWS_Packet32Install();
#endif

#if OPT_DATALINK_P2P
  SWS_Win32CommsInstall();
#endif

#if DEBUG_TRACE_ENABLE && _MSC_VER >= 1000
  if ( DEBUG_TRACE_LEVEL >= 4)
    {
    /* Let's see what's been allocated */
    _CrtMemState s1, s2;
    _CrtMemCheckpoint( &s1 );
    _CrtMemDifference( &s2, &s_checkPt, &s1);
    _CrtMemDumpStatistics( &s2 );
    _CrtMemDumpAllObjectsSince( &s_checkPt);
    }
#endif

  return 0;
  }


/*
 * SwsSock is about to terminate
 */
void PlatformPreCleanup( void)
  {
  TRACE( 9, ("PlatformPreCleanup()\n"));

#if OPT_RAW_SOCK
  RawIpPreCleanup();
#endif
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
    SWS_Win32CommsCleanup();
#endif

#if OPT_RAW_SOCK
    RawIpCleanup();
#elif OPT_DATALINK_ETHERNET
    SWS_Packet32Cleanup();
#endif

    if ( NULL != s_hEvent)
      {
      CloseHandle( s_hEvent);
      s_hEvent = NULL;
      }

    if ( NULL != s_hTimer)
      {
      CancelWaitableTimer( s_hTimer);
      CloseHandle( s_hTimer);
      s_hTimer = NULL;
      }

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

    if ( NULL != s_hThread)
      {
      CloseHandle( s_hThread);
      s_hThread = NULL;
      }

    DeleteCriticalSection( &s_sectionDpc);

    AtomicWrite( &s_critical, 1);
    timeEndPeriod( 1);
    }

  PlatformEndCritical();

  if ( 0 == s_iInited)
    DeleteCriticalSection( &s_section);

#if DEBUG_TRACE_ENABLE && _MSC_VER >= 1000
  if ( DEBUG_TRACE_LEVEL >= 3)
    {
    /* Debug heap stats */
    _CrtMemState s1, s2;
    _CrtMemCheckpoint( &s1 );
    _CrtMemDifference( &s2, &s_checkPt, &s1);
    _CrtMemDumpStatistics( &s2 );
    /*_CrtMemDumpAllObjectsSince( &s_checkPt);*/
    }
#endif
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


int PlatformPropertyGetString( /* Returns no. chars copied to pszBuf */
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

  return SWS_CfgGetString( pszKey, pszVal, pszDefault, pszBuf, len);
  }


/*
 * Monotonic millisecond timer
 */
ui32 PlatformTime( void)
  {
  /* Return milliseconds from midnight */
  return s_dwTimeOffset + timeGetTime();
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

  uNow = PlatformTime();

  PlatformBeginCritical();
  if ( SWS_TimerLE( uTimeout, uNow))
    {
    (*pfn)( pvContext);
    }
  else
    {
    LARGE_INTEGER t;
    t.QuadPart = (LONGLONG)(uTimeout - uNow) * -10000; /* ms to 100ns ticks */
    s_pvTimerContext = pvContext;
    s_pfnTimer = pfn;
    VERIFY( SetWaitableTimer( s_hTimer, &t, 0, &TimerExpired, pvContext, TRUE));
    }
  PlatformEndCritical();

  return 0;
  }

static VOID APIENTRY TimerExpired(
  LPVOID pv,
  DWORD dwTimerLowValue,
  DWORD dwTimerHighValue
) {
  SWS_FNPTR( FPlatformTimeout) pfn;
  (void)dwTimerLowValue;
  (void)dwTimerHighValue;

  TRACE( 4, ("TimerExpired(%p)\n",pv));

  PlatformBeginCritical();

  pfn = s_pfnTimer;
  if ( pfn && pv == s_pvTimerContext)
    {
    s_pfnTimer = 0;
    s_pvTimerContext = 0;
    (*pfn)( pv);
    }

  PlatformEndCritical();
  }


/*
 * Stop the timer
 */
void PlatformTimerStop( void )
  {
  TRACE( 4, ("PlatformTimerStop()\n"));

  CancelWaitableTimer( s_hTimer);
  s_pfnTimer = 0;
  s_pvTimerContext = NULL;
  }


/*
 * Idle time processing
 */
void PlatformIdle( void)
  {
  TRACE( 9, ("PlatformIdle()\n"));

  /* Dispatch APC's for this thread */
  SleepEx( 0, TRUE);

#if OPT_THREADS == OPT_THREADS_COOP
  SWS_ThreadYield();
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

  pEvent->hEvent = CreateEvent( NULL, FALSE, FALSE,NULL);
  if ( NULL == pEvent->hEvent)
    {
    TRACE( 1, ("*** PlatformCreateEvent()\n CreateEvent failed"));
    pEvent->inUse = 0;
    return NULL;
    }

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

  CloseHandle( hEvent->hEvent);
  hEvent->hEvent = NULL;
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

  AtomicWrite( &hEvent->val, iVal);
  if ( 0 != iVal)
    SetEvent( hEvent->hEvent);
  else
    ResetEvent( hEvent->hEvent);

  SetEvent( s_hEvent);
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


static INLINE( i32, ClaimEvent( EventH hEvent))
  {
  i32 iVal;

  PlatformBeginCritical();

  iVal = AtomicXchg( &hEvent->val, 0);
  if ( 0 != iVal)
    ResetEvent( hEvent->hEvent);

  PlatformEndCritical();
  return iVal;
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

  if ( WAIT_NEVER == ticks)
    {
    /* No wait */
    val = ClaimEvent( hEvent);
    }
  else if ( WAIT_FOREVER != ticks)
    {
    /* Timed wait */
    ui32 tNow = PlatformTime();
    ui32 tEnd = tNow + ticks;
    do
      {
#if OPT_THREADS == OPT_THREADS_COOP
      PlatformWaitAnyEvent( ticks);
#else
      /* Dispatch APC's for this thread */
      WaitForSingleObjectEx( hEvent->hEvent, tEnd - tNow, TRUE);
#endif
      val = ClaimEvent( hEvent);
      if ( 0 != val)
        break;

      tNow = PlatformTime();
      }
    while ( !SWS_TimerLE( tEnd, tNow));
    }
  else
    {
    /* Indefinite wait */
    do
      {
#if OPT_THREADS == OPT_THREADS_COOP
      PlatformWaitAnyEvent( ticks);
#else
      /* Dispatch APC's for this thread */
      WaitForSingleObjectEx( hEvent->hEvent, INFINITE, TRUE);
#endif
      val = ClaimEvent( hEvent);
      }
    while ( 0 == val);
    }

  return (i32) val;
  }


/*
 * Wait for any event to be set
 */
void PlatformWaitAnyEvent( ui32 ticks)
  {
  DWORD dwMillisecs;

  TRACE( 9, ("PlatformWaitAnyEvent(%lu)\n", ticks));

#if OPT_THREADS == OPT_THREADS_COOP
  (void)ticks;
  SWS_ThreadYield();
  dwMillisecs = 1;

#else
  switch ( ticks)
    {
  case WAIT_FOREVER:
    dwMillisecs = INFINITE;
    break;
  case WAIT_NEVER:
    dwMillisecs = 0;
    break;
  default:
#if 1
    dwMillisecs = 1;
#elif SWS_TIMER_TICKS == 1000
    dwMillisecs = ticks;
#else
    dwMillisecs = (1000 * ticks) / SWS_TIMER_TICKS;
#endif
    break;
    }
#endif

  /* Dispatch APC's for this thread */
  /* NB s_hEvent is auto reset after releasing 1 thread */
  WaitForSingleObjectEx( s_hEvent, dwMillisecs, TRUE);
  }


/*
 * Schedule a deferred procedure
 */
int PlatformDpc(
  SWS_FNPTR( FPlatformDpc) pfnDpc,
  SWS_PTR( void) pv1,
  SWS_PTR( void) pv2,
  ui32 uParam
) {
  SDpc dpc;
  int iRet;

  TRACE( 10, ("PlatformDpc(%p,%p,%p,%lu)\n", pfnDpc,pv1,pv2,uParam));

  dpc.pfn = pfnDpc;
  dpc.pvContext = pv1;
  dpc.pvParam = pv2;
  dpc.uParam = uParam;

  /* Add the request to the DPC queue
   * NB The messagQ is safe for concurrent get/put op
   * but needs protection for concurrent puts
   */
  EnterCriticalSection( &s_sectionDpc);
  iRet = SWS_MsgqPut( s_hDpcQ, sizeof(dpc), &dpc);
  if ( iRet)
    TRACE( 1, ( "*** PlatformDpc(%p) Qfull\n",pfnDpc));
  LeaveCriticalSection( &s_sectionDpc);

  if ( 0 == InterlockedExchange( &s_lDpcPending, 1))
    {
    /* Schedule an APC for the thread that called PlatformInit */
    VERIFY( QueueUserAPC( &ServiceDPC, s_hThread, 0));
    }

  return iRet;
  }

static VOID APIENTRY ServiceDPC( ULONG_PTR dwParam)
  {
  SDpc dpc;
  (void)dwParam;

  TRACE( 9, ("ServiceDPC()\n"));

  s_lDpcPending = 0;

  PlatformBeginCritical();

  /* Process deferred procedure calls */
  while ( !SWS_MsgqGet( s_hDpcQ, sizeof(dpc), &dpc))
    {
    ASSERT( dpc.pfn);
    (*dpc.pfn)( dpc.pvContext, dpc.pvParam, dpc.uParam);
    }

  PlatformEndCritical();
  }


#if !OPT_ATOMIC_INTRINSIC
/*
 * Atomic operations
 */
SWS_FASTCALL( int, AtomicIncrement( Atomic_t* p))
  {
  COMPILATION_REQUIRES( sizeof(LONG) == sizeof(int));
  return InterlockedIncrement( (LPLONG)&p->v);
  }

SWS_FASTCALL( int, AtomicDecrement( Atomic_t* p))
  {
  COMPILATION_REQUIRES( sizeof(LONG) == sizeof(int));
  return InterlockedDecrement( (LPLONG)&p->v);
  }

SWS_FASTCALL( int, AtomicAdd( Atomic_t* p, int v))
  {
  COMPILATION_REQUIRES( sizeof(LONG) == sizeof(int));
  return v + InterlockedExchangeAdd( (LPLONG)&p->v, v);
  }

SWS_FASTCALL( int, AtomicXchg( Atomic_t* p, int v))
  {
  COMPILATION_REQUIRES( sizeof(LONG) == sizeof(int));
  return InterlockedExchange( (LPLONG)&p->v, v);
  }

SWS_FASTCALL( int, AtomicCmpXchg( Atomic_t* p, int compare, register int update))
  {
  COMPILATION_REQUIRES( sizeof(LONG) == sizeof(int));
  return InterlockedCompareExchange( (LPLONG)&p->v, update, compare);
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


#if defined _USRDLL && defined SWS_DLL
/*
 * DLL entrypoint
 */
BOOL WINAPI DllMain(
  HINSTANCE hModule,
  DWORD dwReason,
  LPVOID lpReserved
) {
  (void)lpReserved;

  switch ( dwReason)
    {
  case DLL_PROCESS_ATTACH:
    TRACE( 2, (kszAppName " DllMain(%p,%u,%p) begin attach process\n",
      hModule,dwReason,lpReserved));

    DisableThreadLibraryCalls( hModule);
#if 0
    PlatformInit();
#endif

    TRACE( 2, (kszAppName " DllMain(%p,%u,%p) end attach process\n",
      hModule,dwReason,lpReserved));
    break;

  case DLL_THREAD_ATTACH:
  case DLL_THREAD_DETACH:
    break;

  case DLL_PROCESS_DETACH:
    TRACE( 2, (kszAppName " DllMain(%p,%u,%p) begin detach process\n",
      hModule,dwReason,lpReserved));

    if ( s_iInited > 0)
      {
      s_iInited = 1;
      PlatformCleanup();
      }

    TRACE( 2, (kszAppName " DllMain(%p,%u,%p) end detach process\n",
      hModule,dwReason,lpReserved));
    break;
    }

  return TRUE;
  }
#endif


#if OPT_THREADS == OPT_THREADS_NATIVE
/****************************************************************************
 * Native Threads
 ****************************************************************************/

#include "sws_thrd.h"

/* Thread instance data */
typedef struct SWS_Thread
  {
  ui32 magic;
# define THREAD_MAGIC SWS_MAKEFOURCC( 't','h','r','d')

  SWS_FNPTR( SWS_ThreadFunction) pfn;
  SWS_PTR( void) pvContext;

  HANDLE ulThreadH;
  unsigned uThreadID;
  } SWS_Thread;

#define ThreadIsValid( h) (NULL != (h) && THREAD_MAGIC == (h)->magic)


#define OPT_STACKSIZE 0
static size_t s_uStackSize;           /* Default stack size */

static SWS_Thread s_mainThread;       /* Main thread instance data */
static DWORD s_dwTlsIndex;            /* Thread local storage for thread handle */


/*
 * Init main thread instance
 */
static BOOL ThreadInit( void)
  {
  SWS_Thread* const pThread = &s_mainThread;

  s_uStackSize = (size_t)PlatformPropertyGetUint( PKEY_WIN32_STACK, OPT_STACKSIZE );

  /* Allocate thread local storage for thread instance data */
  s_dwTlsIndex = TlsAlloc();
  if ( TLS_OUT_OF_INDEXES == s_dwTlsIndex)
    return FALSE;

  /* Create main thread */
  pThread->magic = THREAD_MAGIC;
  ASSERT( ThreadIsValid( pThread) );

  TlsSetValue( s_dwTlsIndex, pThread);

  return TRUE;
  }


/*
 * Native thread function
 */
static unsigned __stdcall PlatformThreadExecute( void* pv)
  {
  SWS_Thread* pThread = (SWS_Thread*)pv;

  TRACE( 8, ("PlatformThreadExecute(%p)\n",pv));
  ASSERT( ThreadIsValid( pThread) );

  /* Save thread instance in TLS */
  TlsSetValue( s_dwTlsIndex, pThread);

  /* Run the thread */
  SWS_ThreadExit( (*pThread->pfn)( pThread->pvContext) );

  /* Should never get here */
  ASSERT( !"Zombie thread");
  return 2;
  }


/*
 * Allocate a thread instance
 */
SWS_EXPORT( SWS_ThreadHandle, SWS_ThreadCreate( /* OUT: Thread handle, NULL= error */
  SWS_FNPTR( SWS_ThreadFunction) pfn,   /* IN: -> thread function */
  SWS_PTR( void) pvContext,             /* IN: Client context for SWS_ThreadFunction */
  SWS_size_t size                       /* IN: Stack size in bytes, 0= default */
)){
  SWS_Thread* pThread;

  TRACE( 9, ("SWS_ThreadCreate(%p,%p,%u)\n", pfn, pvContext, size));
  ASSERT( pfn);

  /* Ensure main thread is created */
  if ( !ThreadIsValid( &s_mainThread)
    && !ThreadInit()
  ) {
    return NULL;
    }

  /* Set stack size */
  if ( 0 == size)
    size = s_uStackSize;

  /* Allocate instance data */
  pThread = (SWS_Thread*) malloc( sizeof( SWS_Thread) );
  if ( NULL == pThread)
    {
    TRACE( 0, ("!!! SWS_ThreadCreate() out of memory\n"));
    return NULL;
    }
  memset( pThread, 0, sizeof( SWS_Thread) );

  /* Create a thread */
  pThread->ulThreadH = (HANDLE)_beginthreadex(
    NULL,
    size,
    &PlatformThreadExecute,
    pThread,
    1*CREATE_SUSPENDED,
    &pThread->uThreadID
  );
  if ( !pThread->ulThreadH)
    {
    TRACE( 0, ("!!! SWS_ThreadCreate() _beginthreadex failed %d\n",errno));
    free( pThread);
    return NULL;
    }

  /* Init thread data */
  pThread->pfn = pfn;
  pThread->pvContext = pvContext;

  /* Set thread signature */
  pThread->magic = THREAD_MAGIC;
  ASSERT( ThreadIsValid( pThread) );
  TRACE( 2, ("SWS_ThreadCreate(%p,%p,%u) => %p id:%#lx\n",
    pfn,pvContext,size,pThread,pThread->ulThreadH));

  return pThread;
  }


/*
 * Destroy a thread - with prejudice!
 */
SWS_EXPORT( int, SWS_ThreadDestroy( SWS_ThreadHandle hThread))
  {
  TRACE( 8, ("SWS_ThreadDestroy(%p)\n", hThread));

  if ( !ThreadIsValid( hThread))
    return SWS_EINVAL;

  /* Can't destroy the main thread */
  if ( hThread == &s_mainThread)
    {
    TRACE( 0, ("!!! SWS_ThreadDestroy() Can't destroy the main thread\n"));
    return SWS_EINVAL;
    }

  /* Destroying the current thread? */
  if ( hThread == SWS_ThreadCurrent() )
    {
    /* Yes */
    CloseHandle( hThread->ulThreadH);
    free( hThread);

    _endthreadex( 0);
    }
  else
    {
    /* Terminate the thread if it's still active */
    if ( NULL != hThread->ulThreadH)
      {
      TerminateThread( hThread->ulThreadH, 1);
      CloseHandle( hThread->ulThreadH);
      }

    free( hThread);
    }

  return 0;
  }


/*
 * Exit a thread
 */
SWS_EXPORT( void, SWS_ThreadExit(
  i32 iExit                         /* IN: Exit status */
)){
  SWS_Thread* pThread = SWS_ThreadCurrent();

  TRACE( 2, ("SWS_ThreadExit %p (%ld)\n",pThread,iExit));

  ASSERT( ThreadIsValid( pThread));

  if ( &s_mainThread == pThread)
    exit( (int)iExit);

  /* Show thread terminated normally */
  CloseHandle( pThread->ulThreadH);
  pThread->ulThreadH = NULL;

  _endthreadex( 0);
  }


/*
 * Resume a thread
 */
SWS_EXPORT( int, SWS_ThreadStart( SWS_ThreadHandle hThread))
  {
  TRACE( 8, ("SWS_ThreadStart(%p)\n", hThread));
  ASSERT( ThreadIsValid( hThread) );

  if ( (DWORD)-1 == ResumeThread( hThread->ulThreadH))
    return SWS_EINVAL;

  return 0;
  }


/*
 * Suspend a thread
 */
SWS_EXPORT( int, SWS_ThreadStop( SWS_ThreadHandle hThread))
  {
  TRACE( 8, ("SWS_ThreadStop(%p)\n", hThread));
  ASSERT( ThreadIsValid( hThread) );

  if ( (DWORD)-1 == SuspendThread( hThread->ulThreadH))
    return SWS_EINVAL;

  return 0;
  }


/*
 * Get handle for current thread
 */
SWS_EXPORT( SWS_ThreadHandle, SWS_ThreadCurrent( void))
  {
  TRACE( 9, ("SWS_ThreadCurrent()\n"));

  return (SWS_Thread*)TlsGetValue( s_dwTlsIndex);
  }


SWS_EXPORT( void, SWS_ThreadYield( void))
  {
  /* Dispatch APC's for this thread */
  SleepEx( 0, TRUE);
  }

#elif OPT_THREADS == OPT_THREADS_COOP
/****************************************************************************
 * Cooperative Threads
 ****************************************************************************/

#include <setjmp.h>

/* Thread instance data */
typedef struct SThread
  {
  /* Task Information Block save area */
  void* pvExcept; /* -> list of SEH's (next, pfn) */
  void* pvStackHi;
  void* pvStackLo;

  /* Context */
  jmp_buf jbuf;

  SWS_Thread thrd;

  SWS_VARRAY( double, stack); /* NB variable length */
  } SThread;

#define WIN32_THREAD( p) SWS_CONTAINER( p, SThread, thrd)
#define OPT_STACKSIZE 16384 /* Default stack size */


/* Prototypes */
static void PlatformThreadExecute( void);
#ifdef __WATCOMC__
extern void* SWS_PlatformSwitchStack( void*);
#pragma aux SWS_PlatformSwitchStack = \
  "xchg eax, esp" \
  parm [eax] \
  value [eax]

#elif defined __GNUC__
#define SWS_PlatformSwitchStack( newSP)\
  ({ void* __oldSP;\
  __asm__ __volatile (\
    "xchgl %0, %%esp\n" /* NB assume SS=DS */\
    : "=r" (__oldSP)\
    : "0" (newSP)\
  );\
  __oldSP;\
  })

#else
static void* __cdecl SWS_PlatformSwitchStack( void*);
#endif
static void PlatformSaveTIB( SThread* const);
static void PlatformRestoreTIB( SThread* const);


/*
 * Allocate thread instance
 */
#if defined _MSC_VER
#pragma check_stack( off)
#endif
SWS_Thread* PlatformThreadAlloc( size_t stack)
  {
  /* C99 7.13.2.3: After setjmp, auto vars not qualified by volatile have
   * indeterminate value */
  SThread* volatile pThread;

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
      s_uStackSize = (size_t)PlatformPropertyGetUint( PKEY_WIN32_STACK, OPT_STACKSIZE );

    stack = s_uStackSize;
    }

  /* Allocate instance data */
  pThread = (SThread*) malloc( sizeof( SThread) + stack);
  if ( NULL == pThread)
    {
    TRACE( 0, ("!!! PlatformThreadAlloc() out of memory\n"));
    return NULL;
    }

  PlatformSaveTIB( pThread);

  /* Main thread has no stack */
  if ( stack)
    {
    static SThread* s_pThread;
    static void* s_SP;

    /* Init the TIB */
#if defined _MSC_VER
    pThread->pvExcept = (void*)0xffffffff;
#endif
    pThread->pvStackHi = (char*)(pThread->stack) + stack;
    pThread->pvStackLo = pThread->stack;

    /* Switch to new stack */
    s_pThread = pThread;
    s_SP = SWS_PlatformSwitchStack( pThread->pvStackHi);
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
    SWS_PlatformSwitchStack( s_SP);
    /* auto vars now accessible */
    }

  return &pThread->thrd;
  }
#if defined _MSC_VER
#pragma check_stack()
#endif


/*
 * Free thread instance
 */
void PlatformThreadFree( SWS_Thread* pThread)
  {
  TRACE( 8, ("PlatformThreadFree(%p)\n",pThread));
  ASSERT( NULL != pThread);

  free( WIN32_THREAD( pThread));
  }


/*
 * Suspend a thread and resume another
 */
void PlatformThreadSwitch( SWS_Thread* pThread, SWS_Thread* pNextThread)
  {
  TRACE( 9, ("PlatformThreadSwitch(%p,%p)\n",pThread,pNextThread));
  ASSERT( NULL != pNextThread);
  ASSERT( pNextThread != pThread);

  /* Save current context */
  if ( NULL == pThread
    || (PlatformSaveTIB( WIN32_THREAD( pThread)), 0 == setjmp( WIN32_THREAD( pThread)->jbuf) )
  ) {
    PlatformRestoreTIB( WIN32_THREAD( pNextThread));

    /* Restore context for next runnable thread */
    longjmp( WIN32_THREAD( pNextThread)->jbuf, 1);
    }
  }


/*
 * Thread function
 */
void PlatformThreadExecute( void)
  {
  SWS_Thread* const pThread = SWS_ThreadCurrent();

#if defined _MSC_VER
  unsigned long ulExcept = 0;
  __try
    {
#endif

    /* Run the thread */
    SWS_ThreadExit( (*pThread->pfn)( pThread->pvContext) );

#if defined _MSC_VER
    }
  __except( (ulExcept = GetExceptionCode(), EXCEPTION_EXECUTE_HANDLER) )
    {
    TRACE( 1, ("*** PlatformThreadExecute() caught exception %#lx\n",ulExcept));
    SWS_ThreadExit( -1);
    }
#endif
  }


/*
 * Switch stacks
 */
#if defined __WATCOMC__
  /* Nothing */
#elif defined __GNUC__
  /* Nothing */
#else
#ifdef _MSC_VER
#pragma check_stack( off)
#endif
__declspec( naked) void* __cdecl SWS_PlatformSwitchStack( void* pvStack)
  {
  (void)pvStack;
  __asm pop edx         /* Return address */
  __asm pop eax         /* pvStack */
  __asm sub eax, 4      /* Account for stack cleanup on return */
  __asm xchg eax, esp   /* Return old SP */
  __asm jmp edx
  }
#ifdef _MSC_VER
#pragma check_stack()
#endif
#endif


/*
 * Save the Task Information Block
 */
void PlatformSaveTIB( SThread* const pThread)
  {
#if defined __WATCOMC__
  COMPILATION_REQUIRES( 0 == offsetof( SThread, pvExcept));
  COMPILATION_REQUIRES( 4 == offsetof( SThread, pvStackHi));
  COMPILATION_REQUIRES( 8 == offsetof( SThread, pvStackLo));
  __asm mov edx, [pThread]
  __asm mov eax, fs:[0]
  __asm mov [ edx].0, eax
  __asm mov eax, fs:[4]
  __asm mov [ edx].4, eax
  __asm mov eax, fs:[8]
  __asm mov [ edx].8, eax
#elif defined __BORLANDC__
  __asm mov edx, [pThread]
  __asm mov eax, fs:[0]
  __asm mov [ edx].(SThread)pvExcept, eax
  __asm mov eax, fs:[4]
  __asm mov [ edx].(SThread)pvStackHi, eax
  __asm mov eax, fs:[8]
  __asm mov [ edx].(SThread)pvStackLo, eax
#elif defined __GNUC__
  __asm__ __volatile ( "mov %%fs:(0), %0\n" : "=r" (pThread->pvExcept));
  __asm__ __volatile ( "mov %%fs:(4), %0\n" : "=r" (pThread->pvStackHi));
  __asm__ __volatile ( "mov %%fs:(8), %0\n" : "=r" (pThread->pvStackLo));
#else /*if defined _MSC_VER || defined __MWERKS__*/
  __asm mov edx, [pThread]
  __asm mov eax, fs:[0]
  __asm mov [ edx] SThread.pvExcept, eax
  __asm mov eax, fs:[4]
  __asm mov [ edx] SThread.pvStackHi, eax
  __asm mov eax, fs:[8]
  __asm mov [ edx] SThread.pvStackLo, eax
#endif
  }


/*
 * Restore the Task Information Block
 */
void PlatformRestoreTIB( SThread* const pThread)
  {
#if defined __WATCOMC__
  COMPILATION_REQUIRES( 0 == offsetof( SThread, pvExcept));
  COMPILATION_REQUIRES( 4 == offsetof( SThread, pvStackHi));
  COMPILATION_REQUIRES( 8 == offsetof( SThread, pvStackLo));
  __asm mov edx, [pThread]
  __asm mov eax, [ edx].0
  __asm mov fs:[0], eax
  __asm mov eax, [ edx].4
  __asm mov fs:[4], eax
  __asm mov eax, [ edx].8
  __asm mov fs:[8], eax
#elif defined __BORLANDC__
  __asm mov edx, [pThread]
  __asm mov eax, [ edx].(SThread)pvExcept
  __asm mov fs:[0], eax
  __asm mov eax, [ edx].(SThread)pvStackHi
  __asm mov fs:[4], eax
  __asm mov eax, [ edx].(SThread)pvStackLo
  __asm mov fs:[8], eax
#elif defined __GNUC__
  __asm__ __volatile ( "mov %0, %%fs:(0)\n" : : "r" (pThread->pvExcept));
  __asm__ __volatile ( "mov %0, %%fs:(4)\n" : : "r" (pThread->pvStackHi));
  __asm__ __volatile ( "mov %0, %%fs:(8)\n" : : "r" (pThread->pvStackLo));
#else /*if defined _MSC_VER || defined __MWERKS__*/
  __asm mov edx, [pThread]
  __asm mov eax, [ edx] SThread.pvExcept
  __asm mov fs:[0], eax
  __asm mov eax, [ edx] SThread.pvStackHi
  __asm mov fs:[4], eax
  __asm mov eax, [ edx] SThread.pvStackLo
  __asm mov fs:[8], eax
#endif

  /* Update the C++/SEH in the jmp_buf otherwise longjmp will unwind
   * an invalid list */
#if defined __WATCOMC__
  pThread->jbuf[11] = (unsigned long)pThread->pvExcept;
#elif defined __BORLANDC__
  pThread->jbuf[0].j_excep = (unsigned)pThread->pvExcept;
#elif defined __GNUC__
  pThread->jbuf[11] = (_JBTYPE)pThread->pvExcept;
#else /*if defined _MSC_VER */
  ((_JUMP_BUFFER*)pThread->jbuf)->Registration = (unsigned long)pThread->pvExcept;
#endif
  }

#endif /*  OPT_THREADS == OPT_THREADS_COOP */


/****************************************************************************
 * Debug
 ****************************************************************************/

#if defined DEBUG_TRACE_LEVEL || defined ALERT_FN

typedef /*WINBASEAPI*/ BOOL WINAPI FIsDebuggerPresent(VOID);
BOOL isDebuggerPresent( void)
  {
  static int s_iDebuggerPresent = -1;

  if ( s_iDebuggerPresent < 0)
    {
    /* Find IsDebuggerPresent */
    HMODULE hmod = LoadLibrary( "kernel32.dll");
    if ( NULL == hmod)
      s_iDebuggerPresent = 0; /* Can't happen 'cos we link against this lib */
    else
      {
      FIsDebuggerPresent *pfn = (FIsDebuggerPresent*) GetProcAddress(
        hmod, "IsDebuggerPresent");
      if ( pfn)
        s_iDebuggerPresent = (*pfn)() ? 1 : 0;
      else
        s_iDebuggerPresent = 0;

      FreeLibrary( hmod);
      }
    }

  return s_iDebuggerPresent;
  }
#endif


#if defined DEBUG_TRACE_LEVEL
extern FILE* g_logfile;

#ifndef DEBUG_BUFFER_SIZE
#define DEBUG_BUFFER_SIZE 0 /*(2*1024) */
#endif

#if DEBUG_BUFFER_SIZE /* Buffered trace */
static char* s_pcBuffer;
static char* s_pcWrite, * s_pcRead;
struct SEntry
  {
  size_t size;
  ui32 uTime;
  SWS_VARRAY( char, szText);
  };

void TraceDump( void)
  {
  if ( NULL != s_pcBuffer)
    {
    FILE* const pf = NULL != g_logfile ? g_logfile : stderr;
    struct SEntry* pEntry;

    while ( s_pcWrite != s_pcRead)
      {
      pEntry = (struct SEntry*)s_pcRead;

      fprintf( pf, "%lu: %s", pEntry->uTime, pEntry->szText);
      if ( isDebuggerPresent())
        OutputDebugString( pEntry->szText);

      s_pcRead += pEntry->size;
      if ( s_pcRead >= s_pcBuffer + DEBUG_BUFFER_SIZE)
        s_pcRead -= DEBUG_BUFFER_SIZE;
      }
    fflush( pf);

    free( s_pcBuffer);
    s_pcBuffer = NULL;
    }
  }
#endif


/*
 * Debug trace handler
 */
void TraceToLog( const char* psz)
  {
#if DEBUG_BUFFER_SIZE /* Buffered trace */
  if ( NULL != psz)
    {
    size_t size, space;
    struct SEntry* pEntry;
    ui32 const t = PlatformTime();

    size = strlen( psz) + sizeof(struct SEntry);
    space = size % sizeof(struct SEntry);
    if ( space)
      size += sizeof(struct SEntry) - space;

    if ( size >= DEBUG_BUFFER_SIZE)
      return;

    if ( NULL == s_pcBuffer)
      {
      s_pcBuffer = (char*)malloc( DEBUG_BUFFER_SIZE + 4);
      if ( NULL == s_pcBuffer)
        return;

      /* Register an exit handler to cleanup */
      atexit( &TraceDump);

      s_pcWrite = s_pcRead = s_pcBuffer;
      }
    else
      {
      for ( ;;)
        {
        if ( s_pcWrite < s_pcRead)
          space = (s_pcRead - s_pcWrite) - 1;
        else if ( s_pcRead > s_pcBuffer)
          space = (s_pcBuffer + DEBUG_BUFFER_SIZE) - s_pcWrite;
        else
          space = (s_pcBuffer + DEBUG_BUFFER_SIZE - 1) - s_pcWrite;

        if ( space >= size)
          break;

        if ( s_pcRead < s_pcWrite)
          {
          /* Wrap write ptr to start of buffer */
          pEntry = (struct SEntry*)s_pcWrite;
          pEntry->size = (s_pcBuffer + DEBUG_BUFFER_SIZE) - s_pcWrite;
          pEntry->szText[0] = '\0';
          s_pcWrite = s_pcBuffer;
          if ( s_pcRead > s_pcWrite)
            continue;
          }

        /* Overwrite oldest entry */
        pEntry = (struct SEntry*)s_pcRead;
        s_pcRead += pEntry->size;
        if ( s_pcRead >= s_pcBuffer + DEBUG_BUFFER_SIZE)
          s_pcRead -= DEBUG_BUFFER_SIZE;
        }
      }

    pEntry = (struct SEntry*)s_pcWrite;
    pEntry->size = size;
    pEntry->uTime = t;
    strcpy( pEntry->szText, psz);

    s_pcWrite += size;
    if ( s_pcWrite >= s_pcBuffer + DEBUG_BUFFER_SIZE)
      s_pcWrite -= DEBUG_BUFFER_SIZE;
    }
  else
    {
    TraceDump();
    }
#else
  if ( NULL != psz)
    {
    char szBuf[ 512];
    int len;

    /* Add the threadID and a timestamp to trace output */
    len = sprintf( szBuf, "%x %u %s",
      (unsigned)GetCurrentThreadId(), (unsigned)PlatformTime(), psz);

    /* Send to debugger or log */
    if ( isDebuggerPresent() || NULL == g_logfile)
      {
      OutputDebugString( szBuf);
      }
    else
      {
      fwrite( szBuf, 1, (size_t)len, g_logfile);
      fflush( g_logfile);
      }
    }
  else
    {
    fflush( g_logfile); /* NULL == all files */
    }
#endif
  }
#endif


#ifdef ALERT_FN
/*
 * Assertion failure handler
 */
short DebugAlert( const char* psz)
  {
  static const char kszText[] =
    kszAppName " has encountered a fatal internal error and must exit immediately.\n"
    "\n"
    "Please send the file " DEBUG_LOGFILENAME " (found in the current directory),\n"
    "a brief description of the events leading up to this failure and\n"
    "a summary of your computer's configuration including OS version to:\n"
    "\n"
    kszURL "\n"
    "\n"
    "and we will endeavour to rectify the problem.  Sorry :-(";

#ifdef _TRACE
  _TRACE( ">>> %s\n", psz);
#endif
#ifdef DEBUG_TRACE_LEVEL
  TraceToLog( NULL);
#endif
  if ( isDebuggerPresent())
    return ALERT_RETRY; /* Asserts break to the debugger without a messagebox */

#if 0
  return MessageBox( NULL, kszText, kszAppName, MB_ABORTRETRYIGNORE);
#else
  (void) MessageBox( NULL, kszText, kszAppName, MB_OK);
  ExitProcess( 1);
  return ALERT_ABORT;
#endif
  }
#endif

/* End of file */
