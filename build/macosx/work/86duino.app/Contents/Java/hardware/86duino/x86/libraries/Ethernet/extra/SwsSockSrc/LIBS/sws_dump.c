/****************************************************************************
 *
 * File name   : sws_dump.c
 * Function    : Packet dumper
 * Project     : SwsSock
 * Authors     : Lawrence Rust
 * Systems     : ANSI C.
 *
 ****************************************************************************
 *
 * Created by Lawrence Rust, Software Systems Consultants
 * lvr@softsystem.co.uk. Tel/Fax +33 5 49 72 79 63
 *
 * 28-Aug-05, LVR, Created.
 *
 * Comments:
 * --------
 * This file implements a generic packet dumper.
 *
 ****************************************************************************
 */

/* Compilation options */
#ifndef OPT_DUMP_BUFSIZ
#define OPT_DUMP_BUFSIZ 2048
#endif

#ifndef OPT_DUMP_ENET
#define OPT_DUMP_ENET 1
#endif

#ifndef OPT_DUMP_PPP
#define OPT_DUMP_PPP 1
#endif

#ifndef OPT_DUMP_P2P
#define OPT_DUMP_P2P 1
#endif

#ifdef _MSC_VER
# pragma warning ( disable : 4127)    /* conditional expression is constant */
# pragma warning ( disable : 4514)    /* unreferenced inline function has been removed */
#endif

#if defined _MSC_VER && _MSC_VER <= 800 /* VC++ 1.5 & earlier */
# pragma warning( disable : 4705)   /* statement has no effect */
#endif


/* Exports */
#include "sws_dump.h"

/* Imports */
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>   /* va_list */
#if _MSC_VER >= 1000
#include <crtdbg.h>
#endif

#ifdef __DJGPP__
#include <unistd.h> /* fsync/sync */
#endif

#include "sws_gef.h"
#include "sws_cfg.h"
#include "sws_dmpi.h"
#if OPT_DUMP_ENET
#include "sws_dmpe.h"
#endif
#if OPT_DUMP_PPP || OPT_DUMP_P2P
#include "sws_dmpp.h"
#endif
#ifdef __DJGPP__
#include "sws_timr.h"
#endif
#include "sws_sock.h"


/*
 * Prototypes
 */
static int TicksToTime( char*, ui32 ticks);
#ifdef __DJGPP__
static SWS_FTimer Timeout;
#endif


/*
 * Module data
 */
static int s_isInited;
static FILE * s_file1, *s_file2;
static char* s_pszBuf;
#ifdef __DJGPP__
static SWS_TimerH s_hTimer;
#endif


/****************************************************************************
 * External functions
 ****************************************************************************/

/*
 * Setup the packet dumper
 */
SWS_EXTERN( int, SWS_DumpSetup(       /* Returns !0 if on */
  int iVerbosity,                     /* IN: Verbosity 0= off, 1= normal, 2= more etc */
  FILE* file1,                        /* IN: Primary destination */
  FILE* file2                         /* IN: Secondary destination */
)){
  s_isInited = 1;

  if ( 0 == iVerbosity)
    {
    s_file1 = NULL;
    s_file2 = NULL;
    }
  else if ( NULL != file1)
    {
    s_file1 = file1;
    }
  else if ( NULL == s_file1)
    {
    char szLogName[ 256];

    if ( SWS_CfgGetString( "log", "file", "stdout", szLogName, sizeof(szLogName) ))
      {
      if ( !strcmp( szLogName, "stderr"))
        s_file1 = stderr;
      else if ( !strcmp( szLogName, "stdout"))
        s_file1 = stdout;
      else
        {
        s_file1 = fopen( szLogName, "a+");
        if ( NULL != s_file1)
          setvbuf( s_file1, NULL, _IONBF, 0 );
        }
      }
    }

  if ( 0 != iVerbosity)
    {
    if ( NULL != file2)
      s_file2 = file2;
    if ( s_file2 == s_file1)
      s_file2 = NULL;
    }

  if ( NULL != s_file1 || NULL != s_file2)
    {
    if ( NULL == s_pszBuf && NULL == (s_pszBuf = (char*) malloc( OPT_DUMP_BUFSIZ)) )
      fputs( "*** SWS_DumpSetup: Out of memory\n", stderr);
    }
  else
    {
#ifdef __DJGPP__
    if ( NULL != s_hTimer)
      {
      SWS_TimerRelease( s_hTimer);
      s_hTimer = NULL;
      }
#endif
    free( s_pszBuf);
    s_pszBuf = NULL;
    }

  SWS_DmpIpSetup( iVerbosity);
#if OPT_DUMP_ENET
  SWS_DmpEnetSetup( iVerbosity);
#endif
#if OPT_DUMP_PPP
  SWS_DmpPppSetup( iVerbosity);
#endif
#if OPT_DUMP_P2P
  SWS_DmpP2pSetup( iVerbosity);
#endif

  return NULL != s_pszBuf;
  }


/*
 * The packet dumper event filter
 */
SWS_FGef SWS_DumpPacket;
SWS_EXTERN( int, SWS_DumpPacket(
  SWS_PTR( void) pv,                  /* IN: callback context */
  int event,                          /* IN: SWS_IpEvent */
  va_list va
)){
  char* pszBuf;
  enum { kTimeStampLen = 12 };
  (void)pv;

  if ( !s_isInited)
    SWS_DumpSetup( -1, NULL, NULL);

  pszBuf = s_pszBuf;
  if ( NULL != pszBuf)
    {
    int count;
    ui32 const ticks = SWS_SockTicks();

    pszBuf += kTimeStampLen;

    switch ( event)
      {
    default:
      count = 0;
      break;

    case SWS_kGefCleanup:
#ifdef __DJGPP__
      if ( NULL != s_hTimer)
        {
        SWS_TimerRelease( s_hTimer);
        s_hTimer = NULL;
        }
#endif
      free( s_pszBuf);
      s_pszBuf = NULL;
      s_file1 = NULL;
      s_file2 = NULL;
      s_isInited = 0;
      return 0;

    case SWS_kGefIpTx:
    case SWS_kGefIpRx:
    case SWS_kGefIpError:
    case SWS_kGefTcpError:
    case SWS_kGefUdpError:
    case SWS_kGefRawError:
      count = SWS_DmpIp( pszBuf, event, va);
      break;

#if OPT_DUMP_ENET
    case SWS_kGefEnetTx:
    case SWS_kGefEnetRx:
    case SWS_kGefEnetError:
      count = SWS_DmpEnet( pszBuf, event, va);
      break;
#endif

#if OPT_DUMP_PPP
    case SWS_kGefPppTx:
    case SWS_kGefPppRx:
    case SWS_kGefPppError:
    case SWS_kGefSlipTx:
    case SWS_kGefSlipRx:
    case SWS_kGefSlipError:
      count = SWS_DmpPpp( pszBuf, event, va);
      break;
#endif

#if OPT_DUMP_P2P
    case SWS_kGefP2pTx:
    case SWS_kGefP2pRx:
    case SWS_kGefP2pError:
      count = SWS_DmpP2p( pszBuf, event, va);
      break;
#endif
      }

    if ( count > 0)
      {
      pszBuf += count;
      strcpy( pszBuf, "\n");

      count = TicksToTime( s_pszBuf, ticks);
      s_pszBuf[ count] = ' ';

      if ( s_file1)
        {
        fputs( s_pszBuf, s_file1);
        fflush( s_file1);
        }

      if ( s_file2)
        {
        fputs( s_pszBuf, s_file2);
        fflush( s_file2);
#ifdef __DJGPP__
        /* fflush() doesn't write to disk so request a callback to fsync() */
        if ( NULL == s_hTimer)
          s_hTimer = SWS_TimerAlloc( &Timeout, NULL);
        if ( NULL != s_hTimer && 0 == SWS_TimerRemaining( s_hTimer))
          SWS_TimerStart( s_hTimer, 10 * SWS_TIMER_TICKS);
#endif
        }
      }
    }

  return 0;
  }


/*
 * Format time of day from ticks since midnight
 */
static int TicksToTime(
  char* psz,                          /* OUT: Time as hh:mm:ss.iii */
  ui32 ticks
) {
  unsigned hr, mn, sc, ms;

  sc = (unsigned)(ticks / SWS_TIMER_TICKS);
  ms = (unsigned)(ticks - (ui32)sc * SWS_TIMER_TICKS);
  mn = sc / 60U;
  sc -= 60U * mn;
  hr = mn / 60U;
  mn -= 60U * hr;
  hr %= 24U;
  return sprintf( psz, "%02u:%02u:%02u.%03u", hr, mn, sc, ms);
  }


#ifdef __DJGPP__
/*
 * Timer expiry callback
 */
SWS_CB( void, Timeout(
  SWS_TimerH hTimer,                  /* IN: Timer that expired */
  SWS_PTR( void) pv                   /* IN: User context */
)){
  (void)hTimer;
  (void)pv;
  if ( s_file2)
    fsync( fileno( s_file2));
  }
#endif

/* End of file */
