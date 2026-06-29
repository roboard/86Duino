/****************************************************************************
 *
 * File name   : loopback.c
 * Function    : Loopback data link driver
 * Project     : SwsSock
 * Authors     : Lawrence Rust
 * Systems     : ANSI C.
 *
 ****************************************************************************
 *
 * Created by Lawrence Rust, Software Systems Consultants
 * lvr@softsystem.co.uk. Tel/Fax +33 5 49 72 79 63
 *
 * 02-Apr-01, LVR, Created.
 * 11-Apr-02, LVR, Made conditional on OPT_DATALINK_LOOPBACK.
 * 31-Aug-03, LVR, Added code to make installation.dependent on ini setting
 *
 * Comments:
 * --------
 * This file contains the loopback data link driver
 *
 ****************************************************************************
 */

/* Compilation options */
#include "config.h"

/* Exports */
#include "loopback.h"

#if OPT_DATALINK_LOOPBACK
/* Imports */
#include <stddef.h>
#include <string.h>

#include "sws_sock.h"  /* for SWS_INADDR_LOOPBACK */
#include "sws_buf.h"
#include "sws_dlnk.h"
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
static SWS_FDatalinkGetProperty LoopbackGetProperty;
static SWS_FDatalinkSend LoopbackSend;


/*
 * Module data
 */

static const char kszIdent[] = "Loopback";

/* Data link descriptor */
static SWS_DatalinkDesc const s_loopback = {
  sizeof(s_loopback),
  SWS_DATALINK_VERSION,
  &LoopbackSend,
  &LoopbackGetProperty,
  0, /* SetProperty */
  0, /* SWS_FDatalinkShutdown */
  0  /* SWS_FDatalinkSetMulticast */
};

/* Up calls */
static SWS_DatalinkCallbacks s_upcalls;


/*
 * Initialise the module
 * Single threaded
 */
int SWS_LoopbackInit( void)
  {
  TRACE( 9, ("SWS_LoopbackInit\n"));
  ASSERT( PlatformIsCritical());

  if ( PlatformPropertyGetUint( PKEY_LOOPBACK_ENABLE, 1) )
    {
    /* Register as a data link */
    return SWS_DatalinkRegister( &s_loopback, NULL, &s_upcalls);
    }

  return 0;
  }


/*
 * Get an interface property
 */
SWS_CB( int, LoopbackGetProperty(
  SWS_PTR( void) pvContext,
  int type,
  SWS_PTR( void) pvBuf,
  SWS_PTR( size_t) puLen
)){
  TRACE( 9, ("LoopbackGetProperty(%p,%d,%p,%p)\n", pvContext,type,pvBuf,puLen));
  (void)pvContext;
  ASSERT( NULL != pvBuf);
  ASSERT( NULL != puLen);

  switch ( type)
    {
  default:
    /* Not implemented */
    TRACE( 3, ("LoopbackGetProperty %d not implemented\n", type));
    return SWS_EBADF;

  case SWS_kDatalinkProperty_Address:
    if ( *puLen < sizeof(ui32))
      {
      TRACE( 0, ("!!!LoopbackGetProperty buffer too small\n"));
      return SWS_EINVAL;
      }

    *(SWS_PTR(ui32))pvBuf = SWS_INADDR_LOOPBACK;
    *puLen = sizeof(ui32);
    break;

  case SWS_kDatalinkProperty_SubnetMask:
    if ( *puLen < sizeof(ui32))
      {
      TRACE( 0, ("!!!LoopbackGetProperty buffer too small\n"));
      return SWS_EINVAL;
      }

    *(SWS_PTR(ui32))pvBuf = 0xff000000L;
    *puLen = sizeof(ui32);
    break;

  case SWS_kDatalinkProperty_MTU:
    if ( *puLen < sizeof(size_t))
      {
      TRACE( 0, ("!!!LoopbackGetProperty buffer too small\n"));
      return SWS_EINVAL;
      }

    *(SWS_PTR(size_t))pvBuf = 0xffff; /*SWS_BufferMax();*/
    *puLen = sizeof(size_t);
    break;

  case SWS_kDatalinkProperty_Gateway:
    if ( *puLen < sizeof(ui32))
      {
      TRACE( 0, ("!!!LoopbackGetProperty buffer too small\n"));
      return SWS_EINVAL;
      }

    *(SWS_PTR(ui32))pvBuf = SWS_INADDR_NONE; /* No gateway */
    *puLen = sizeof(ui32);
    break;

  case SWS_kDatalinkProperty_Ident:
    if ( *puLen < sizeof(kszIdent))
      {
      TRACE( 0, ("!!!LoopbackGetProperty buffer too small\n"));
      return SWS_EINVAL;
      }

    memcpy( pvBuf, kszIdent, sizeof(kszIdent));
    *puLen = sizeof(kszIdent);
    break;
    }

  return 0;
  }


/*
 * Send a packet
 */
SWS_CB( int, LoopbackSend(
  void* pvContext,
  SWS_BufferH hBuffer,
  ui32 dst,
  unsigned tos
)){
  int broadcast;

  TRACE( 9, ("LoopbackSend(%p,%p,%08lx,%u)\n", pvContext, hBuffer, dst, tos));
  ASSERT( NULL != hBuffer);
  (void)pvContext;
  (void)tos;

  switch ( dst)
    {
  case SWS_INADDR_BROADCAST:
  case SWS_INADDR_LOOPBACK | 0xffffffL:
    broadcast = 1;
    break;

  default:
    broadcast = 0;
    break;
    }

  /* Pass the datagram back up the stack */
  ASSERT( s_upcalls.pfnRecvInd);
  (*s_upcalls.pfnRecvInd)( s_upcalls.pvContext, hBuffer, broadcast);

#if 0 /* Enable to test async completion */
  ASSERT( s_upcalls.pfnSendAck);
  (*s_upcalls.pfnSendAck)( s_upcalls.pvContext, 0);
  return -1;
#else
  /* Immediate completion */
  return 0;
#endif
  }

#else
char SWS_Loopback;
#endif /*OPT_DATALINK_LOOPBACK */
/* End of file */
