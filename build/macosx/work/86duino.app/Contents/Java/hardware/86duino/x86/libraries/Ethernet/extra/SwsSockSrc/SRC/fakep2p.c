/****************************************************************************
 *
 * File name   : fakep2p.c
 * Function    : Fake P2P driver
 * Project     : SwsSock
 * Authors     : Lawrence Rust
 * Systems     : ANSI C.
 *
 ****************************************************************************
 *
 * Created by Lawrence Rust, Software Systems Consultants
 * lvr@softsystem.co.uk. Tel/Fax +33 5 49 72 79 63
 *
 * 23-Oct-01, LVR, Created.
 * 21-Aug-05, LVR, Changed name from fakeslip to fakep2p
 *
 * Comments:
 * --------
 * This file implements a fake P2P driver
 *
 ****************************************************************************
 */

/* Compilation options */
#include "config.h"   /* Build options */

/* Exports */
#include "fakep2p.h"

#if OPT_DATALINK_P2P
/* Imports */
#include <stddef.h>
#include <string.h>   /* memcpy */

#include "sws_sock.h" /* SWS_ENOBUFS */
#include "sws_buf.h"  /* Buffer operations */
#include "p2p.h"      /* P2P driver registration */
#include "platform.h"

#include "debug.h"    /* TRACE, ASSERT & VERIFY */


/*
 * Macros & Constants
 */


/*
 * Local data types
 */
typedef struct SFakeP2p
  {
  SWS_P2pCallbacks upcalls;           /* Up call info */

  SWS_PTR( void) pvData;
  size_t len;
  } SFakeP2p;


/*
 * Prototypes
 */
static SWS_FP2pSend FakeP2pSend;
static SWS_FP2pRecv FakeP2pRecv;
static SWS_FP2pShutdown FakeP2pShutdown;


/*
 * Module data
 */

/* P2P device descriptor */
static const SWS_P2pDevice s_fakeP2p =
  {
  sizeof( SWS_P2pDevice),
  SWS_P2P_VERSION,
  SWS_FAKEP2P_IDENT,
  &FakeP2pSend,
  &FakeP2pRecv,
  &FakeP2pShutdown,
  0  /* SWS_FP2pIoctl */
  };

static SFakeP2p s_inst;/* Instance */


/*
 * Initialise the driver
 */
int SWS_FakeP2pInstall()                 /* Returns !0 on error */
  {
  SFakeP2p* pAdapter = &s_inst;
  int iRet;

  TRACE( 9, ("SWS_FakeP2pInstall()\n"));

  /* Register a P2P adapter */
  iRet = SWS_P2pAddAdapter( &s_fakeP2p, pAdapter, &pAdapter->upcalls);
  if ( iRet)
    return iRet;

  ASSERT( pAdapter->upcalls.pfnSendAck);
  ASSERT( pAdapter->upcalls.pfnRecvInd);

  return 0;
  }


/*
 * Finalise the driver
 */
static SWS_CB( void, FakeP2pShutdown(
  SWS_PTR( void) pvContext            /* IN: Driver's context */
)){
  SFakeP2p* pAdapter = (SFakeP2p*)pvContext;

  TRACE( 9, ("FakeP2pShutdown(%p)\n",pvContext));
  ASSERT( pAdapter == &s_inst);

#if 0
  if ( pAdapter->len)
    {
    pAdapter->len = 0;

    /* Indicate pending receive cancelled */
    ASSERT( pAdapter->upcalls.pfnRecvInd);
    (*pAdapter->upcalls.pfnRecvInd)(
      pAdapter->upcalls.pvContext, 0, SWS_ENETDOWN);
    }
#else
  (void)pAdapter;
#endif
  }


/*
 * Transmit a buffer
 * Single threaded
 */
static SWS_CB( int, FakeP2pSend(     /* Return !0 on error, -1 if pending */
  SWS_PTR( void) pvContext,           /* IN: Driver's context */
  SWS_PTR( const void) pvData,        /* IN: Data to send */
  size_t len                          /* IN: Byte count */
)){
  SFakeP2p* pAdapter = (SFakeP2p*)pvContext;

  TRACE( 9, ("FakeP2pSend(%p,%p,%u)\n",pvContext,pvData,len));
  ASSERT( NULL != pvData);
  ASSERT( len > 0);
  ASSERT( PlatformIsCritical());
  ASSERT( pAdapter == &s_inst);

  while ( len > 0)
    {
    size_t count;

    count = pAdapter->len;
    if ( 0 == count)
      break; /* No receive buffer */

    if ( count > len)
      count = len;

    /* Copy data to receive buffer
     * NB This should stop at the first delimiter */
    memcpy( pAdapter->pvData, pvData, count);

    /* Indicate receive fulfilled */
    pAdapter->len = 0;
    pAdapter->pvData = NULL;
    ASSERT( pAdapter->upcalls.pfnRecvInd);
    (*pAdapter->upcalls.pfnRecvInd)( pAdapter->upcalls.pvContext, count, 0);

    len -= count;
    }

#if 1 /* !0 to indicate send completed async */
  (*pAdapter->upcalls.pfnSendAck)(
    pAdapter->upcalls.pvContext, len ? SWS_ENOBUFS : 0);
  return -1; /* -1 = pending - make SendAck callback when done */
#else
  return 0; /* Sync completion */
#endif
  }


/*
 * Start receiving.
 * Single threaded
 */
static SWS_CB( int, FakeP2pRecv(
  SWS_PTR( void) pvContext,           /* IN: Driver's context */
  SWS_PTR( void) pvData,              /* IN: Buffer */
  SWS_PTR( size_t) pLen,              /* I/O: Buffer size */
  int iEnd,                           /* IN: Delimiter, -ve if none */
  ui32 uTimeout                       /* IN: Timeout, 0= none, 1= immediate */
)){
  SFakeP2p* pAdapter = (SFakeP2p*)pvContext;
  (void)iEnd;

  TRACE( 9, ("FakeP2pRecv(%p,%p,%p,%d,%lu)\n",
    pvContext,pvData,pLen,iEnd,uTimeout));
  ASSERT( NULL != pvData);
  ASSERT( NULL != pLen);
  ASSERT( *pLen > 0);
  ASSERT( PlatformIsCritical());
  ASSERT( pAdapter == &s_inst);
  ASSERT( 0 == pAdapter->len );

  switch ( uTimeout)
    {
  default:
    return SWS_EBADF;

  case 0:
    /* Ininite */
    break;

  case 1:
    /* Immediate */
    *pLen = 0;
    return 0;
    }

  pAdapter->pvData = pvData;
  pAdapter->len = *pLen;

  return -1; /* Pending */
  }

#endif /*OPT_DATALINK_P2P*/
/* End of file */
