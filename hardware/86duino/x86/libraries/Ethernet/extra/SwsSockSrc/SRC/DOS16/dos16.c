/****************************************************************************
 *
 * File name   : dos16.c
 * Function    : DOS16 support functions.
 * Project     : SwsSock
 * Authors     : Lawrence Rust
 * Systems     : ANSI C, DOS.
 *
 ****************************************************************************
 *
 * Created by Lawrence Rust, Software Systems Consultants
 * lvr@softsystem.co.uk. Tel/Fax +33 5 49 72 79 63
 *
 * 08-May-01, LVR, Created.
 * 07-Jun-01, LVR, Added option for packet driver.
 * 09-Jan-03, LVR, Added threads and SLIP driver..
 * 09-Jun-05, LVR, Added PKEY_DOS_YIELD
 * 04-Aug-05, LVR, Conert SLIP to P2P
 *
 * Comments:
 * --------
 * This file only contains functions that are DOS16 specific
 *
 ****************************************************************************
 */

/* Compilation options */
#include "config.h"

#if !defined OPT_DOS_PKTDRVR && OPT_DATALINK_ETHERNET
#define OPT_DOS_PKTDRVR 1
#endif

#ifdef _MSC_VER
#pragma warning( disable : 4704) /* in-line assembler precludes global optimizations */
#endif

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

/* DOS */
#include <dos.h> /* enable/disable */
#include <conio.h> /* kbhit */
#include <bios.h>
#ifdef _MSC_VER
#include <malloc.h> /* _stackavail, _heapchk */
#endif

#ifdef _WINDOWS /* QuickWin */
#include <windows.h>
#include <mmsystem.h> /* timeGetTime */
#include <io.h> /* _wyield */
#endif

/* sws_sock */
#include "sws_timr.h"
#include "sws_msgq.h"
#include "sws_dlnk.h"
#include "sws_cfg.h"
#include "sws_sock.h"

#if OPT_DOS_PKTDRVR
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

/* Critical section control */
#define PlatformBeginCritical() AtomicIncrement( &s_critical)
#define PlatformEndCritical() AtomicDecrement( &s_critical)
#ifdef __WATCOMC__
 static int __far ErrorHandler( unsigned, unsigned, unsigned __far*);
#elif !defined WIN16
 static void __far ErrorHandler( unsigned, unsigned, unsigned __far*);
#endif


/*
 * Module data
 */
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

#ifndef WIN16
    /* Install a critical error handler */
    _harderr( &ErrorHandler);
#endif

#if defined MTC_TRACE_LEVEL && defined _WINDOWS
    /* Increase timeGetTime() resolution for TraceToLog */
    timeBeginPeriod( 1);
#endif

    /* Get configuration data */
    s_events = (size_t)PlatformPropertyGetUint( PKEY_EVENT_COUNT, OPT_EVENTS );
    uDpcs = (size_t)PlatformPropertyGetUint( PKEY_DPC_COUNT, OPT_DPCS );
    s_bYield = 0 != PlatformPropertyGetUint( PKEY_DOS_YIELD, 1);

    /* Create event pool */
    s_pEvents = (SEvent*)calloc( s_events, sizeof(*s_pEvents));
    if ( NULL == s_pEvents)
      {
      TRACE( 0, ("!!!PlatformInit out of memory for events\n"));
      return SWS_ENOMEM;
      }

    /* Create DPC queue */
    s_hDpcQ = SWS_MsgqCreate( uDpcs, sizeof( SDpc));
    if ( NULL == s_hDpcQ)
      {
      free( s_pEvents);
      s_pEvents = NULL;
      return SWS_ENOBUFS;
      }
    }

  return 0;
  }


#ifndef WIN16
/*
 * The critical error handler
 */
static
#ifdef __WATCOMC__
  int
#else
  void
#endif
__far ErrorHandler(
  unsigned deverr,
  unsigned doserr,
  unsigned __far* lpHdr
) {
  (void)deverr;
  (void)doserr;
  (void)lpHdr;

  /* Fail the DOS call */
  _hardresume( _HARDERR_FAIL);
#ifdef __WATCOMC__
  return 0;
#endif
  }
#endif


/*
 * SwsSock has now been initialised
 */
int PlatformPostInit( void)
  {
  TRACE( 9, ("PlatformPostInit()\n"));
  ASSERT( s_iInited >= 1);
  ASSERT( PlatformIsCritical());

#if OPT_DOS_PKTDRVR
  /* system() is not available in QuickWin */
  /* system() doesn't work properly with TSR drivers in BC3.1 */
#if !defined WIN16 && !defined __BORLANDC__
  {
  char szDriver[ 128];

  /* Get the driver name */
  if ( PlatformPropertyGetString( PKEY_DOS_DRIVER, "", szDriver, sizeof(szDriver) ))
    {
    /* Install the packet driver */
    if ( system( szDriver))
      TRACE( 1, ("*** PlatformPostInit system(%s) failed %d\n", szDriver, errno));
    }
  }
#endif

  /* Install the packet driver */
  SWS_PacketInstall(); /* Ignore errors */
#endif

#if OPT_DATALINK_P2P
  /* Install the P2P driver */
  (void)SWS_PcCommsInstall();
#endif

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
    /* Cleanup the p2p driver */
    SWS_PcCommsCleanup();
#endif
#if OPT_DOS_PKTDRVR
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

#if defined MTC_TRACE_LEVEL && defined _WINDOWS
    timeEndPeriod( 1);
#endif
    AtomicWrite( &s_critical, 1);
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
#ifdef _WINDOWS /* QuickWin */
  COMPILATION_REQUIRES( SWS_TIMER_TICKS == 1000);
  return timeGetTime();
#elif 1
  unsigned long t0, t1;
  unsigned counter;
  volatile unsigned long __far * const lpulTicks = MK_FP( 0x40, 0x6C);

  t1 = *lpulTicks;
  do
    {
    /* 8253 Counter/timer registers */
    enum { CTC_timer0 = 0x40, CTC_timer1, CTC_timer2, CTC_control};

    t0 = t1;

    _disable();
    outp( CTC_control, 0x00);         /* Latch timer 0 count */
    counter = inp(  CTC_timer0);      /* Read LSB */
    counter += inp( CTC_timer0) << 8; /* Read MSB */
    _enable();

    t1 = *lpulTicks;
    }
  /* Repeat if timer underflowed */
  while ( t1 != t0);

  /* Standard tick interval (65536 clocks) is 54.926 ms */
#  define UCLOCKS_PER_SEC 1193180UL /* Fixed in h/w: 14.318160 MHz TV xtal / 12 */
#  define UCLOCKS_PER_TICK 65536UL
#  define kMillisecsPerTick ((UCLOCKS_PER_TICK * 1000) / UCLOCKS_PER_SEC)
#  define kMicrosecsPerTick (((UCLOCKS_PER_TICK - (kMillisecsPerTick * UCLOCKS_PER_SEC) / 1000) * 1000000UL) / UCLOCKS_PER_SEC)

  /* Subtract downcount from next tick */
  ++t1;
  return t1 * kMillisecsPerTick
    + (t1 * kMicrosecsPerTick) / 1000
    - (1000UL * counter) / UCLOCKS_PER_SEC;
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
    isIdle = 0;
    (*dpc.pfn)( dpc.pvContext, dpc.pvParam, dpc.uParam);
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

  /* Yield CPU */
  if ( isIdle)
    {
#if defined _WINDOWS /* QuickWin */
    _wyield();
#else
    /* Make a DOS call to pump ctrl-c's */
    kbhit();
#endif
    }
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
#ifdef __BORLANDC__
#pragma option -N- /* disable stack checking */
#pragma option -r- /* disable register variables */
#endif
#if defined _MSC_VER
#pragma check_stack( off)
#endif

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

  /* Disable IRQ's */
  _disable();

  /* Add the request to the DPC queue */
  iRet = SWS_MsgqPut( s_hDpcQ, sizeof(dpc), &dpc);

  /* Restore IRQ state */
  _enable();

  if ( iRet)
    TRACE( 1, ( "*** PlatformDpc(%p) Qfull\n",pfnDpc));

  return iRet;
  }


#if !OPT_ATOMIC_INTRINSIC
/*
 * Atomic operations
 */
#if defined _MSC_VER
#pragma warning ( disable : 4035)    /* no return value */
#endif

int AtomicIncrement( Atomic_t* p)
  {
#if defined __WATCOMC__
  extern int SWS_AtomicInc_( Atomic_t __far *);
# pragma aux SWS_AtomicInc_ = \
    "mov ax, es:[bx]" \
    "lock inc word ptr es:[bx]" \
    "inc ax" \
    parm [es bx] \
    value [ax]
  return SWS_AtomicInc_( p);
#else
  int const v0 = p->v + 1;

#if defined _M_I86CM || defined _M_I86LM || defined _M_I86HM
  __asm les bx, p;
  __asm inc es:[bx]Atomic_t.v;
#elif defined __COMPACT__ || defined __LARGE__ || defined __HUGE__
  __asm les bx, p;
  __asm inc WORD PTR es:[bx].(Atomic_t)v;
#elif defined __TINY__ || defined __SMALL__ || defined __MEDIUM__
  __asm mov bx, p;
  __asm inc WORD PTR [bx].(Atomic_t)v;
#else
  __asm mov bx, p;
  __asm inc [bx]Atomic_t.v;
#endif

  return v0;
#endif
  }

int AtomicDecrement( Atomic_t* p)
  {
#if defined __WATCOMC__
  extern int SWS_AtomicDec_( Atomic_t __far *);
# pragma aux SWS_AtomicDec_ = \
    "mov ax, es:[bx]" \
    "lock dec word ptr es:[bx]" \
    "dec ax" \
    parm [es bx] \
    value [ax]
  return SWS_AtomicDec_( p);
#else
  int const v0 = p->v - 1;

#if defined _M_I86CM || defined _M_I86LM || defined _M_I86HM
  __asm les bx, p;
  __asm dec es:[bx]Atomic_t.v;
#elif defined __COMPACT__ || defined __LARGE__ || defined __HUGE__
  __asm les bx, p;
  __asm dec WORD PTR es:[bx].(Atomic_t)v;
#elif defined __TINY__ || defined __SMALL__ || defined __MEDIUM__
  __asm mov bx, p;
  __asm dec WORD PTR [bx].(Atomic_t)v;
#else
  __asm mov bx, p;
  __asm dec [bx]Atomic_t.v;
#endif

  return v0;
#endif
  }

int AtomicAdd( Atomic_t* p, int v)
  {
#if defined __WATCOMC__
  extern int SWS_AtomicAdd_( Atomic_t __far *, int);
# pragma aux SWS_AtomicAdd_ = \
    "mov dx, es:[bx]" \
    "lock add es:[bx], ax" \
    "add ax, dx" \
    parm [es bx] [ax] \
    modify [dx] \
    value [ax]
  return SWS_AtomicAdd_( p, v);
#else
  int const v0 = p->v + v;

#if defined _M_I86CM || defined _M_I86LM || defined _M_I86HM
  __asm mov ax, v;
  __asm les bx, p;
  __asm lock add es:[bx]Atomic_t.v, ax;
#elif defined __COMPACT__ || defined __LARGE__ || defined __HUGE__
  __asm mov ax, v;
  __asm les bx, p;
  __asm lock add es:[bx].(Atomic_t)v, ax;
#elif defined __TINY__ || defined __SMALL__ || defined __MEDIUM__
  __asm mov ax, v;
  __asm mov bx, p;
  __asm lock add [bx].(Atomic_t)v, ax;
#else
  __asm mov ax, v;
  __asm mov bx, p;
  __asm lock add [bx]Atomic_t.v, ax;
#endif

  return v0;
#endif
  }

int AtomicXchg( Atomic_t* p, int v)
  {
#if defined __WATCOMC__
  extern int SWS_AtomicXchg_( Atomic_t __far *, int);
# pragma aux SWS_AtomicXchg_ = \
    "lock xchg es:[bx], ax" \
    parm [es bx] [ax] \
    value [ax]
  return SWS_AtomicXchg_( p, v);
#else

#if defined _M_I86CM || defined _M_I86LM || defined _M_I86HM
  __asm mov ax, v;
  __asm les bx, p;
  __asm xchg es:[bx]Atomic_t.v, ax;
  __asm mov v, ax;
#elif defined __COMPACT__ || defined __LARGE__ || defined __HUGE__
  __asm mov ax, v;
  __asm les bx, p;
  __asm xchg es:[bx].(Atomic_t)v, ax;
  __asm mov v, ax;
#elif defined __TINY__ || defined __SMALL__ || defined __MEDIUM__
  __asm mov ax, v;
  __asm mov bx, p;
  __asm xchg [bx].(Atomic_t)v, ax;
  __asm mov v, ax;
#else
  __asm mov ax, v;
  __asm mov bx, p;
  __asm xchg [bx]Atomic_t.v, ax;
  __asm mov v, ax;
#endif

  return v;
#endif
  }

int AtomicCmpXchg( Atomic_t* p, int compare, int update)
  {
  int v0;

  /* Disable IRQ's */
  _disable();

  v0 = p->v;
  if ( p->v == compare)
    p->v = update;

  /* Restore IRQ state */
  _enable();

  return v0;
  }

#if defined _MSC_VER
#pragma warning ( default : 4035)
#endif

#else /* if OPT_ATOMIC_INTRINSIC */
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
#endif /* if OPT_ATOMIC_INTRINSIC */

#ifdef __BORLANDC__
#pragma option -r.
#pragma option -N.
#endif
#ifdef _MSC_VER
#pragma check_stack()
#endif


#if OPT_THREADS == OPT_THREADS_COOP
/****************************************************************************
 * Cooperative Threads
 ****************************************************************************/

/* BUGBUG Microsoft's set/longjmp do not preserve SS */

/* WATCOM stjmp086.asm longjmp calls __longjmp_handler (see ljmphdl.c/h)
 * which by default does nothing (see ljmphdl.c).  If C++ exceptions are
 * enabled, ljmpinit.cpp redirects this to ljmpdtor.cpp which invokes all
 * destructors for local objects.
 * Therefore can't use longjmp with C++ if exceptions enabled (-xs option).
 */
#if defined _MSC_VER || defined __WATCOMC__
typedef struct PlatformJmp_buf
  {
  unsigned buf[ 9]; /* sp ss cs ip bp di es si ds */
  } PlatformJmp_buf[1];
#else

/* Use compiler's setjmp/longjmp */
#define PlatformJmp_buf jmp_buf
#endif


/* Thread instance data */
typedef struct SThread
  {
  SWS_Thread thrd;
# define DOS_THREAD( p) SWS_CONTAINER( p, SThread, thrd)

  /* Context */
  PlatformJmp_buf jbuf;
  SWS_VARRAY( double, stack); /* NB variable length */
  } SThread;

#ifndef OPT_STACKSIZE /* Default stack size */
# ifdef _WINDOWS
#  define OPT_STACKSIZE 8192
# else
#  define OPT_STACKSIZE 5192
# endif
#endif

#define STACK_USE 512 /* Min operational stack */
#define STACK_MIN 1024 /* Min initial stack */
#define STACK_MAX (63U*1024U) /* Max initial stack */


/* Prototypes */
static void PlatformThreadExecute( void);

#ifdef __WATCOMC__
extern int SWS_PlatformSetjmp( PlatformJmp_buf __far);
#pragma aux SWS_PlatformSetjmp = \
  "cld" \
  "mov ax, sp" \
  "stosw" \
  "mov ax, ss" \
  "stosw" \
  "mov ax, cs" \
  "stosw" \
  "call near ptr getpc" \
  "jmp short end" \
  "getpc: pop ax" \
  "stosw" \
  "mov ax, bp" \
  "stosw" \
  "mov ax, si" \
  "stosw" \
  "mov ax, ds" \
  "stosw" \
  "sub ax, ax" \
  "end:" \
  parm [es di] \
  value [ax]

extern void SWS_PlatformLongjmp( PlatformJmp_buf __far, int);
#pragma aux SWS_PlatformLongjmp aborts
#pragma aux SWS_PlatformLongjmp = \
  "cmp dx, 1" \
  "adc dx, 0" \
  "cld" \
  "lodsw" \
  "mov bx, ax" \
  "lodsw" \
  "mov ss, ax" \
  "mov sp, bx" \
  "lodsw" \
  "push ax" \
  "lodsw" \
  "push ax" \
  "lodsw" \
  "mov bp, ax" \
  "lodsw" \
  "mov ds, [si]" \
  "mov si, ax" \
  "mov ax, dx" \
  "retf"\
  parm [ds si] [dx]

extern void __far * SWS_PlatformSwitchStack( void __far*);
#pragma aux SWS_PlatformSwitchStack = \
  "mov dx, ss" \
  "mov ss, bx" \
  "xchg ax, sp" \
  parm [bx ax] \
  value [dx ax]

#elif defined _MSC_VER
static int SWS_PlatformSetjmp( PlatformJmp_buf);
static void SWS_PlatformLongjmp( PlatformJmp_buf, int);
static void __far * SWS_PlatformSwitchStack( void __far*);

#else
#define SWS_PlatformLongjmp longjmp
#define SWS_PlatformSetjmp setjmp
static void __far * SWS_PlatformSwitchStack( void __far*);
#endif


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
  else if ( stack < STACK_MIN)
    {
    static size_t s_uStackSize;
    if ( 0 == s_uStackSize)
      {
      s_uStackSize = (size_t)PlatformPropertyGetUint( PKEY_DOS_STACK, OPT_STACKSIZE );
      if ( s_uStackSize < STACK_MIN)
        s_uStackSize = STACK_MIN;
      }

    stack = s_uStackSize;
    }

  if ( stack > STACK_MAX)
    stack = STACK_MAX;

  /* Allocate instance data plus stack */
  pThread = (SThread*) malloc( sizeof( SThread) + stack);
  if ( NULL == pThread)
    {
    TRACE( 0, ("!!! PlatformThreadAlloc() out of memory\n"));
    return NULL;
    }

  /* Main thread has no stack */
  if ( stack > 0)
    {
    static struct SThread* s_pThread;
    static void __far * s_SP;
    void __far* pv;
    unsigned uSS, uSP;

    /* Set ss:0 to start of stack and ss:sp to end of stack */
    pv = pThread->stack;
    uSP = FP_OFF( pv);
    uSS = FP_SEG( pv);
#ifdef _WINDOWS /* QuickWin */
    {
    DWORD dwBase = GetSelectorBase( uSS);
    uSS = AllocSelector( 0);
    SetSelectorBase( uSS, dwBase + uSP);
    SetSelectorLimit( uSS, size);
    uSP = size;
    }
#else
    uSS += uSP >> 4;
    uSP &= 0xf;
    if ( uSP)
      {
      ++uSS;
      uSP += stack - 16;
      }
    else
      {
      uSP += stack;
      }

#if defined _MSC_VER
    /* Offset SP to pass M$'s stack checking code */
    {
    extern unsigned STKHQQ;
    unsigned u = STKHQQ;

    /* Deduct min useable stack */
    if ( u >= STACK_USE)
      u -= STACK_USE;

    u = (u + 15) >> 4;
    uSS -= u;
    uSP += u << 4;
    }
#endif
#endif
    TRACE( 3, ("PlatformThreadAlloc() stack @ %p len %#x, SS:SP= %04x:%04x\n",
       pThread->stack, stack, uSS, uSP));

    /* Switch to new stack */
    s_pThread = pThread;
    s_SP = SWS_PlatformSwitchStack( MK_FP( uSS, uSP) );
    /* NB auto vars no longer accessible */

    /* NB setjmp must be called inline otherwise the new thread's stack
     * will be corrupted after the return
     * C99 7.13.1.1: setjmp is a macro and a reserved identifier
     */
    if ( 0 != SWS_PlatformSetjmp( s_pThread->jbuf) )
      {
      /* NB Do not use local vars */

      /* Run the current thread */
      PlatformThreadExecute();

      /* Should never get here */
      ASSERT( !"Zombie thread");
      }

#if defined _MSC_VER && defined _DEBUG
    /* NB _stackavail aborts with R6000 if SP < STKHQQ */
    if ( _stackavail() < 1024)
      TRACE( 0, ("!!! PlatformThreadAlloc() stack < 1024\n"));
#endif

    /* Restore stack */
    (void)SWS_PlatformSwitchStack( s_SP);
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

  free( DOS_THREAD( pThread));
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
    || 0 == SWS_PlatformSetjmp( DOS_THREAD( pThread)->jbuf )
  ) {
    /* Restore context for next runnable thread */
    SWS_PlatformLongjmp( DOS_THREAD( pNextThread)->jbuf, 1);
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


#if !defined __WATCOMC__
/*
 * Switch stacks
 */
#pragma check_stack( off)
void __far * SWS_PlatformSwitchStack( void __far * pvStack)
  {
#if defined _M_I86LM || defined _M_I86HM || defined _M_I86MM \
 || defined __MEDIUM__ || defined __LARGE__ || defined __HUGE__

  __asm les ax, pvStack

  /* Undo prolog */
#if defined _WINDOWS
  __asm pop di
  __asm pop si
#else
  __asm pop si
  __asm pop di
#endif
  __asm mov sp, bp
  __asm pop bp

  /* Pop the return address */
  __asm pop bx          /* offset */
  __asm pop cx          /* seg */

  /* Switch stack */
  __asm sub ax, 4       /* Account for stack cleanup on return */
  __asm mov dx, ss
  __asm push es
  __asm pop ss
  __asm xchg ax, sp     /* Return old SS:SP in dx:ax */

  __asm push cx
  __asm push bx
  __asm retf

#else

  __asm les ax, pvStack

  /* Undo prolog */
#if defined _WINDOWS
  __asm pop di
  __asm pop si
#else
  __asm pop si
  __asm pop di
#endif
  __asm mov sp, bp
  __asm pop bp

  /* Pop the return address */
  __asm pop bx          /* offset */

  /* Switch stack */
  __asm sub ax, 4       /* Account for stack cleanup on return */
  __asm mov dx, ss
  __asm push es
  __asm pop ss
  __asm xchg ax, sp     /* Return old SS:SP in dx:ax */

  __asm jmp bx

#endif
  return 0;
  }
#pragma check_stack()
#endif /* !defined __WATCOMC__ */


#if defined _MSC_VER
/*
 * Save context for SWS_PlatformLongjmp
 */
#pragma check_stack( off)
int SWS_PlatformSetjmp( PlatformJmp_buf env)
  {
  /* On entry, the stack frame is:

    TOS (lowest address)
#   if SS != DS and DS loaded on entry
            caller's DS
#   endif
            SI
            DI
            BP  (local BP points here)
            IP
#   if defined _M_I86MM || defined _M_I86LM || defined _M_I86HM
            CS
#   endif
            env OFFSET
#   if defined _M_I86CM || defined _M_I86LM || defined _M_I86HM
            env SEGMENT
#   endif
  */

  __asm mov dx, es /* Save es */
  __asm mov cx, di

#if defined _M_I86CM || defined _M_I86LM || defined _M_I86HM
  __asm les di, [env]
#else
  __asm mov di, [env]
  __asm mov ax, ds
  __asm mov es, ax
#endif

  __asm cld
  __asm lea ax, [env]
  __asm stosw        /* SP */
  __asm mov ax, ss
  __asm stosw        /* SS */

#if defined _M_I86MM || defined _M_I86LM || defined _M_I86HM
  __asm mov ax, word ptr [env - 2]
#else
  __asm mov ax, cs
#endif
  __asm stosw        /* CS */

#if defined _M_I86MM || defined _M_I86LM || defined _M_I86HM
  __asm mov ax, word ptr [env - 4]
#else
  __asm mov ax, word ptr [env - 2]
#endif
  __asm stosw       /* IP */

  __asm mov ax, [bp]
  __asm stosw       /* BP */
  __asm mov ax, cx
  __asm stosw       /* DI */
  __asm mov ax, dx
  __asm stosw       /* ES */
  __asm mov ax, si
  __asm stosw       /* SI */
#if defined __HUGE__ /* If DS loaded on entry */
  __asm pop ax
  __asm push ax     /* Caller's DS */
#else
  __asm mov ax, ds
#endif
  __asm stosw       /* DS */

  __asm mov es, dx  /* restore ES */

  return 0;
  }
#pragma check_stack()


/*
 * Restore context saved by SWS_PlatformSetjmp
 */
#pragma check_stack( off)
void SWS_PlatformLongjmp( PlatformJmp_buf env, int retval)
  {
  __asm mov dx, [retval]
  __asm cmp dx, 1 /* set cy if dx=0 */
  __asm adc dx, 0 /* if was 0, now it's 1  */

#if defined _M_I86CM || defined _M_I86LM || defined _M_I86HM
  __asm lds si, [env]
#else
  __asm mov si, [env]
#endif

  __asm cld

  /* Change stack */
  __asm lodsw       /* SP */
  __asm mov bx, ax
  __asm lodsw       /* SS */
  __asm mov ss, ax
  __asm mov sp, bx

  __asm lodsw       /* CS */
  __asm push ax
  __asm lodsw       /* IP */
  __asm push ax

  __asm lodsw       /* BP */
  __asm mov bp, ax
  __asm lodsw       /* DI */
  __asm mov di, ax
  __asm lodsw       /* ES */
  __asm mov es, ax
  __asm lodsw       /* SI */
  __asm mov ds, [si] /* DS */
  __asm mov si, ax

  __asm mov ax, dx /* put result in ax */
  __asm retf /* return to original caller of setjmp */
  }
#pragma check_stack()
#endif

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
