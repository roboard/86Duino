/****************************************************************************
 *
 * File name   : startup.c
 * Function    : Library startup and cleanup.
 * Project     : SwsSock
 * Authors     : Lawrence Rust
 * Systems     : ANSI C.
 *
 ****************************************************************************
 *
 * Created by Lawrence Rust, Software Systems Consultants
 * lvr@softsystem.co.uk. Tel/Fax +33 5 49 72 79 63
 *
 * 10-Apr-01, LVR, Created.
 * 03-Oct-01, LVR, Fix TRACE warnings for ARM SDT 2.51 .
 * 30-Oct-02, LVR, Added SWS_EnetDump
 * 18-Dec-02, LVR, Added cooperative threads
 * 06-Sep-05, LVR, Added P2P & GEF
 *
 * Comments:
 * --------
 * This file implements SwsSock initialisation and cleanup
 *
 ****************************************************************************
 */

/* Compilation options */
#include "config.h"

#ifndef OPT_DUMP
#define OPT_DUMP 0 /* !0 to include packet dumper */
#endif

/* Exports */
#include "sws_sock.h"   /* SWS_SockStartup/SWS_SockCleanup */

/* Imports */
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "sws_timr.h"   /* SWS_TimerInit */
#include "sws_buf.h"    /* SWS_BufferInit */
#include "sws_tprt.h"   /* SWS_TransportInit */
#include "sws_gef.h"    /* SWS_GefInstall */
#include "ip.h"         /* SWS_IpInit */
#if OPT_PROTO_IPRAW
#include "rawip.h"      /* SWS_RawipInit */
#endif
#if OPT_PROTO_UDP
#include "sws_udp.h"    /* SWS_UdpInit */
#endif
#if OPT_PROTO_TCP
#include "sws_tcp.h"    /* SWS_TcpInit */
#endif
#if OPT_DATALINK_ETHERNET
#include "sws_enet.h"   /* SWS_EnetInit */
#endif
#if OPT_DATALINK_P2P
#include "p2p.h"        /* SWS_P2pInit */
#endif
#if OPT_DHCP
#include "sws_dhcp.h"   /* SWS_DhcpInit */
#endif
#if OPT_THREADS == OPT_THREADS_COOP
#include "thread.h"     /* SWS_ThreadInit */
#endif
#if OPT_DUMP
#include "sws_dump.h"   /* SWS_DumpSetup & SWS_DumpPacket */
#endif
#include "platform.h"

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
static FPlatformCallback SockStartup;
static FPlatformCallback SockCleanup;


/*
 * Module data
 */
static int s_bInited; /* Library inited */
#ifdef DEBUG_TRACE_LEVEL
FILE* g_logfile;
#else
#define g_logfile stderr
#endif


/*
 * Initialise the library.
 * >>>MUST<<< be called before any other function
 */
SWS_EXPORT( int, SWS_SockStartup(     /* Returns !0 on error */
  SWS_FNPTR( SWS_FGef) pfnGef,        /* IN: Event filter */
  SWS_PTR( void) pvGef                /* IN: Event filter context */
)){
  int iRet = 0;

  TRACE( 9, ("SWS_SockStartup(%p,%p)\n",pfnGef,pvGef));

  while ( !s_bInited)
    {
    s_bInited = !0;
    SWS_SetLastError( 0);

#ifdef DEBUG_TRACE_LEVEL
    g_logfile = stderr;
#endif

    /* Platform specific initialisation */
    iRet = PlatformInit();
    if ( iRet)
      {
      s_bInited = 0;
      break;
      }

#if DEBUG
    {
    Atomic_t v;
    ASSERT(  0 == AtomicWrite( &v, 0));
    ASSERT(  0 == AtomicRead( &v));
    ASSERT( -1 == AtomicWrite( &v, -1));
    ASSERT( -1 == AtomicRead( &v));
    ASSERT(  0 == AtomicIncrement( &v));
    ASSERT(  0 == AtomicRead( &v));
    ASSERT( -1 == AtomicAdd( &v, -1));
    ASSERT( -1 == AtomicRead( &v));
    ASSERT( -2 == AtomicDecrement( &v));
    ASSERT( -2 == AtomicRead( &v));
    ASSERT( -2 == AtomicXchg( &v, 2));
    ASSERT(  2 == AtomicRead( &v));
    ASSERT(  2 == AtomicCmpXchg( &v, 2, 3));
    ASSERT(  3 == AtomicRead( &v));
    ASSERT(  3 == AtomicCmpXchg( &v, 2, 4));
    }
#endif

#ifdef DEBUG_TRACE_LEVEL
    /* Open the logfile if specified */
    {
    char szLogName[ 256];
    if ( PlatformPropertyGetString(
        PKEY_LOG_FILE, OPT_LOGFILE, szLogName, sizeof(szLogName)
      )
    ) {
      if ( !strcmp( szLogName, "stderr"))
        g_logfile = stderr;
      else if ( !strcmp( szLogName, "stdout"))
        g_logfile = stdout;
      else
        {
        g_logfile = fopen( szLogName, "a+");
        if ( NULL != g_logfile)
          setvbuf( g_logfile, NULL, _IONBF, 0 );
        }
      }
    else
      g_logfile = NULL;
    }
#endif

    /* Setup trace level from config data */
    DEBUG_SET_TRACE_LEVEL(
      (int)PlatformPropertyGetUint( PKEY_LOG_TRACE, DEBUG_TRACE_LEVEL));
    TRACE( 1, ("TRACE level %d\n", DEBUG_TRACE_LEVEL));

#if OPT_DUMP
    if ( SWS_DumpSetup( -1, NULL, NULL)
      && 0 == pfnGef
    ) {
      TRACE( 2, ("SWS_SockStartup() installing packet dumper\n"));
      pfnGef = &SWS_DumpPacket;
      }
#endif

    /* Install the event filter */
    SWS_GefInstall( pfnGef, pvGef);

    /* Ensure startup is single threaded */
    iRet = PlatformSynchronise( &SockStartup, NULL);
    if ( iRet)
      {
      PlatformCleanup();
      s_bInited = 0;
      break;
      }

    /* Optional services */
#if OPT_DHCP
    TRACE( 2, ("SWS_SockStartup() DhcpInit\n"));
    SWS_DhcpInit();
#endif

    /* Run any DPC's */
    TRACE( 2, ("SWS_SockStartup() Running DPC's\n"));
    PlatformIdle();

    TRACE( 2, ("SWS_SockStartup() complete\n"));
    }

  return iRet;
  }

SWS_CB( int, SockStartup( SWS_PTR( void) pv))
  {
  int iRet;
  (void)pv;

  TRACE( 9, ("SockStartup()\n"));
  ASSERT( PlatformIsCritical());

  /* Init required modules */
  TRACE( 2, ("SockStartup() BufferInit\n"));
  iRet = SWS_BufferInit();
  if ( iRet)
    return iRet;

  TRACE( 2, ("SockStartup() TimerInit\n"));
  iRet = SWS_TimerInit();
  if ( iRet)
    return iRet;

#if OPT_THREADS == OPT_THREADS_COOP
  TRACE( 2, ("SockStartup() ThreadInit\n"));
  iRet = SWS_ThreadInit();
#endif

  TRACE( 2, ("SockStartup() TransportInit\n"));
  iRet = SWS_TransportInit();
  if ( iRet)
    return iRet;

  TRACE( 2, ("SockStartup() IpInit\n"));
  iRet = SWS_IpInit();
  if ( iRet)
    return iRet;

  /* Init optional modules */
#if OPT_PROTO_IPRAW
  TRACE( 2, ("SockStartup() RawipInit\n"));
  iRet = SWS_RawipInit();
#endif
#if OPT_PROTO_UDP
  TRACE( 2, ("SockStartup() UdpInit\n"));
  iRet = SWS_UdpInit();
#endif
#if OPT_PROTO_TCP
  TRACE( 2, ("SockStartup() TcpInit\n"));
  iRet = SWS_TcpInit();
#endif

#if OPT_DATALINK_ETHERNET
  TRACE( 2, ("SockStartup() EnetInit\n"));
  iRet = SWS_EnetInit();
#endif

#if OPT_DATALINK_P2P
  TRACE( 2, ("SockStartup() P2pInit\n"));
  iRet = SWS_P2pInit();
#endif

  TRACE( 2, ("SockStartup() PlatformPostInit\n"));
  iRet = PlatformPostInit();
  ASSERT( !iRet);

  return iRet;
  }


/*
 * Finalise the library
 */
SWS_EXPORT( void, SWS_SockCleanup( void))
  {
  TRACE( 2, ("SWS_SockCleanup\n"));

  /* Optional services */
#if OPT_DHCP
  SWS_DhcpCleanup();
#endif

  /* Platform specific pre-cleanup */
  PlatformPreCleanup();

  /* Ensure cleanup is single threaded */
  PlatformSynchronise( &SockCleanup, NULL);

  SWS_GefRaise( SWS_kGefCleanup);
  SWS_GefInstall( 0, NULL);

  /* Platform specific cleanup */
  PlatformCleanup();

  TRACE( 2, ("SWS_SockCleanup() complete\n"));

#ifdef DEBUG_TRACE_LEVEL
  if ( NULL != g_logfile
    && stderr != g_logfile
    && stdout != g_logfile
  ) {
    fclose( g_logfile);
    g_logfile = NULL;
    }
#endif
  }

SWS_CB( int, SockCleanup( SWS_PTR( void) pv))
  {
  ASSERT( PlatformIsCritical());
  (void)pv;

  if ( s_bInited)
    {
    s_bInited = 0;

    /* Shutdown the transport layer */
    SWS_TransportCleanup();

    /* Shutdown devices */
#if OPT_DATALINK_P2P
    SWS_P2pCleanup();
#endif
#if OPT_DATALINK_ETHERNET
    SWS_EnetCleanup();
#endif

    /* Shutdown protocols */
#if OPT_PROTO_TCP
    SWS_TcpCleanup();
#endif
#if OPT_PROTO_UDP
    SWS_UdpCleanup();
#endif
#if OPT_PROTO_IPRAW
    SWS_RawipCleanup();
#endif

#if OPT_THREADS == OPT_THREADS_COOP
    SWS_ThreadCleanup();
#endif

    SWS_IpCleanup();

    /* Cleanup other modules in reverse order */
    SWS_TimerCleanup();
    SWS_BufferCleanup();
    }

  return 0;
  }


/*
 * atexit handler
 */
SWS_EXTERNC( void, SWS_SockExit( void))
  {
  SWS_SockCleanup();
  }

/* End of file */
