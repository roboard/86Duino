/****************************************************************************
 *
 * File name   : wcdos32.c
 * Function    : Watcom DOS32 support functions.
 * Project     : SwsSock
 * Authors     : Lawrence Rust
 * Systems     : Watcom C, DOS.
 *
 ****************************************************************************
 *
 * Copyright (C) 2006 L.V.Rust, Software Systems Consultants
 * lvr@softsystem.co.uk. Tel/Fax +33.549727963
 *
 * 02-Mar-06, LVR, Created.
 *
 * Comments:
 * --------
 * This file contains functions that are Watcom DOS32 specific
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

/* WC */
#include <dos.h> /* enable/disable */
#include <conio.h> /* kbhit */
#include <bios.h> /* enable/disable */
#include "dpmi.h" /* __dpmi_yield */

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
    s_bYield = PlatformPropertyGetUint( PKEY_DJGPP_YIELD, 1);

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
#if 1
  long t0, t1;
  unsigned counter;

#if 1
  dosmemget( 0x046C, sizeof(t1), &t1);
#else
  _bios_timeofday( _TIME_GETCLOCK, &t1);
#endif
  do
    {
    /* 8253 Counter/timer registers */
    enum { CTC_timer0 = 0x40, CTC_timer1, CTC_timer2, CTC_control};

    t0 = t1;

    outp( CTC_control, 0x00);         /* Latch timer 0 count */
    counter = inp(  CTC_timer0);      /* Read LSB */
    counter += inp( CTC_timer0) << 8; /* Read MSB */

#if 1
    dosmemget( 0x046C, sizeof(t1), &t1);
#else
    _bios_timeofday( _TIME_GETCLOCK, &t1);
#endif
    }
  /* Repeat if timer underflowed */
  while ( t1 != t0);

  /* Standard tick interval (65536 clocks) is 54.926 ms */
# define UCLOCKS_PER_SEC 1193180UL /* Fixed in h/w: 14.318160 MHz TV xtal / 12 */
# define UCLOCKS_PER_TICK 65536UL
# define kMillisecsPerTick ((UCLOCKS_PER_TICK * 1000) / UCLOCKS_PER_SEC)
# define kMicrosecsPerTick (((UCLOCKS_PER_TICK - (kMillisecsPerTick * UCLOCKS_PER_SEC) / 1000) * 1000000UL) / UCLOCKS_PER_SEC)

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
  switch ( isIdle)
    {
  case 0:
    break;

  case 1:
    /* BUGBUG: The Causeway and Dos32/a extenders cause GP faults on Win2ksp4 & NT4sp6 when
     * a packet is received (which makes a rmcb) while kbhit (or any DOS fn) is in progress.
     * DOS4G/W is OK as are DOS 6.22, WinXPsp2 and Win98se */
    kbhit(); /* Make a DOS call to pump ctrl-c's */

    /* Fall thru */
  default:
    __dpmi_yield();
    break;
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
int PlatformDpc(
  SWS_FNPTR( FPlatformDpc) pfnDpc,
  SWS_PTR( void) pv1,
  SWS_PTR( void) pv2,
  ui32 uParam
) {
  SDpc dpc;
  int iRet;
  int iEnable;

  TRACE( 10, ("PlatformDpc(%p,%p,%p,%lu)\n", pfnDpc,pv1,pv2,uParam));

  dpc.pfn = pfnDpc;
  dpc.pvContext = pv1;
  dpc.pvParam = pv2;
  dpc.uParam = uParam;

  /* Disable IRQ's */
  iEnable = __dpmi_disable();

  /* Add the request to the DPC queue */
  iRet = SWS_MsgqPut( s_hDpcQ, sizeof(dpc), &dpc);

  /* Restore IRQ state */
  if ( iEnable)
    __dpmi_enable();

  if ( iRet)
    TRACE( 1, ( "*** PlatformDpc(%p) Qfull\n",pfnDpc));

  return iRet;
  }


#if !OPT_ATOMIC_INTRINSIC
/*
 * Atomic operations
 */
COMPILATION_REQUIRES( 0 == offsetof( Atomic_t, v));
COMPILATION_REQUIRES( 4 == sizeof( Atomic_t));

SWS_FASTCALL( int, AtomicIncrement( Atomic_t* p))
  {
  extern int SWS_AtomicInc_( Atomic_t*);
# pragma aux SWS_AtomicInc_ = \
    "mov eax, [ebx]" \
    "lock inc dword ptr [ebx]" \
    "inc eax" \
    parm [ebx] \
    value [eax]

  return SWS_AtomicInc_( p);
  }

SWS_FASTCALL( int, AtomicDecrement( Atomic_t* p))
  {
  extern int SWS_AtomicDec_( Atomic_t*);
# pragma aux SWS_AtomicDec_ = \
    "mov eax, [ebx]" \
    "lock dec dword ptr [ebx]" \
    "dec eax" \
    parm [ebx] \
    value [eax]

  return SWS_AtomicDec_( p);
  }

SWS_FASTCALL( int, AtomicAdd( Atomic_t* p, int v))
  {
  extern int SWS_AtomicAdd_( Atomic_t*, int);
# pragma aux SWS_AtomicAdd_ = \
    "mov edx, [ebx]" \
    "lock add [ebx], eax" \
    "add eax, edx" \
    parm [ebx] [eax] \
    modify [edx] \
    value [eax]

  return SWS_AtomicAdd_( p, v);
  }

SWS_FASTCALL( int, AtomicXchg( Atomic_t* p, int v))
  {
  extern int SWS_AtomicXchg_( Atomic_t*, int);
# pragma aux SWS_AtomicXchg_ = \
    "lock xchg [ebx], eax" \
    parm [ebx] [eax] \
    value [eax]

  return SWS_AtomicXchg_( p, v);
  }

SWS_FASTCALL( int, AtomicCmpXchg(
  register Atomic_t* p,
  int compare,
  register int update
)){
#if defined __SW_4 || defined __SW_5 || defined __SW_6
  extern int SWS_AtomicCmpXchg_( Atomic_t*, int, int);
# pragma aux SWS_AtomicCmpXchg_ = \
    "cmpxchg [ebx], edx" \
    parm [ebx] [eax] [edx] \
    value [eax]

  return SWS_AtomicCmpXchg_( p, compare, update);
#else
  register int i;
  int iEnable;

  /* Disable IRQ's */
  iEnable = __dpmi_disable();

  i = p->v;
  if ( i == compare)
    p->v = update;

  /* Restore IRQ state */
  if ( iEnable)
    __dpmi_enable();

  return i;
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

/* NB in stjmp386.asm longjmp calls __longjmp_handler (see ljmphdl.c/h)
 * which by default does nothing (see ljmphdl.c).  If C++ exceptions are
 * enabled, ljmpinit.cpp redirects this to ljmpdtor.cpp which invokes all
 * destructors for local objects.
 * Therefore can't use longjmp with C++ if exceptions enabled (-xs option).
 */
typedef struct PlatformJmp_buf
  {
  unsigned buf[ 4]; /* esp eip ebp esi */
  } PlatformJmp_buf[1];

/* Thread instance data */
typedef struct SThread
  {
  SWS_Thread thrd;

  /* Context */
  PlatformJmp_buf jbuf;

  SWS_VARRAY( double, stack); /* NB variable length */
  } SThread;

#define WC_THREAD( p) SWS_CONTAINER( p, SThread, thrd)
#define OPT_STACKSIZE 32768 /* Default stack size */

/* Prototypes */
static void PlatformThreadExecute( void);


/*
 * Save context for PlatformLongjmp
 */
extern int SWS_PlatformSetjmp( PlatformJmp_buf);
#pragma aux SWS_PlatformSetjmp = \
  "cld" \
  "mov eax, esp" \
  "stosd" \
  "call near ptr getpc" \
  "jmp short end" \
  "getpc: pop eax" \
  "stosd" \
  "mov eax, ebp" \
  "stosd" \
  "mov eax, esi" \
  "stosd" \
  "sub eax, eax" \
  "end:" \
  parm [edi] \
  value [eax]


/*
 * Restore context saved by PlatformSetjmp
 */
extern void SWS_PlatformLongjmp( PlatformJmp_buf, int);
#pragma aux SWS_PlatformLongjmp aborts
#pragma aux SWS_PlatformLongjmp = \
  "cmp edx, 1" \
  "adc edx, 0" \
  "cld" \
  "lodsd" \
  "mov esp, eax" \
  "lodsd" \
  "push eax" \
  "lodsd" \
  "mov ebp, eax" \
  "lodsd" \
  "mov esi, eax" \
  "mov eax, edx" \
  "ret"\
  parm [esi] [edx]


/*
 * Switch stacks
 */
extern void* SWS_PlatformSwitchStack( void*);
#pragma aux SWS_PlatformSwitchStack = \
  "xchg eax, esp" \
  parm [eax] \
  value [eax]


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

    /* Switch to new stack */
    s_pThread = pThread;
    s_SP = SWS_PlatformSwitchStack( (char*)(pThread->stack) + stack );
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

  free( WC_THREAD( pThread));
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
    || 0 == SWS_PlatformSetjmp( WC_THREAD( pThread)->jbuf )
  ) {
    /* Restore context for next runnable thread */
    SWS_PlatformLongjmp( WC_THREAD( pNextThread)->jbuf, 1);
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
