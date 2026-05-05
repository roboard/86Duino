/****************************************************************************
 *
 * File name   : fakenic.c
 * Function    : Fake Ethernet driver
 * Project     : SwsSock
 * Authors     : Lawrence Rust
 * Systems     : ANSI C.
 *
 ****************************************************************************
 *
 * Created by Lawrence Rust, Software Systems Consultants
 * lvr@softsystem.co.uk. Tel/Fax +33 5 49 72 79 63
 *
 * 26-Apr-01, LVR, Created.
 *
 * Comments:
 * --------
 * This file implements a fake Ethernet driver
 *
 ****************************************************************************
 */

/* Compilation options */
#include "config.h"   /* Build options */

/* Exports */
#include "fakenic.h"

#if OPT_DATALINK_ETHERNET
/* Imports */
#include <stddef.h>
#include <string.h>   /* memcpy */

#include "sws_enet.h" /* Ethernet driver registration */
#include "sws_buf.h"  /* Buffer operations */

#include "debug.h"    /* TRACE, ASSERT & VERIFY */


/*
 * Macros & Constants
 */


/*
 * Local data types
 */


/*
 * Prototypes
 */
static SWS_FEnetSend FakeSend;
static SWS_FEnetRecvAck FakeRecvAck;
static SWS_FEnetGetMacAddr FakeGetMacAddr;


/*
 * Module data
 */

/* Fake Ethernet address */
static const SWS_EnetMacAddress s_macAddr = { {0x00,0x12,0x34,0x56,0x78,0x9A} };

/* Ethernet device descriptor */
static const SWS_EnetDevice s_fake =
  {
  sizeof( s_fake),
  SWS_ENET_VERSION,
  SWS_FAKE_IDENT,
  &FakeSend,
  &FakeRecvAck,
  &FakeGetMacAddr,
  0, /* SWS_FEnetShutdown */
  0  /* SWS_FEnetIoctl */
  };

/* Up call info */
static SWS_EnetCallbacks s_upcalls;

/* Fake output buffer */
static char s_buffer[ 1514 /* Max length of ethernet packet */];


/*
 * Initialise the driver
 */
int SWS_FakeInstall()                 /* Returns !0 on error */
  {
  int iRet;

  TRACE( 9, ("SWS_FakeInstall()\n"));

  /* Register the Ethernet adapter with the SwsSock stack */
  s_upcalls.uSize = sizeof( s_upcalls);
  iRet = SWS_EnetAddAdapter( &s_fake, NULL, &s_upcalls);
  if ( iRet)
    return iRet;

  ASSERT( s_upcalls.pfnSendAck);
  ASSERT( s_upcalls.pfnRecvInd);

  return 0;
  }


/*
 * Transmit a buffer, possibly fragmented
 * Single threaded
 */
static SWS_CB( int, FakeSend(         /* Return !0 on error, -1 if pending */
  SWS_PTR( void) pvContext,           /* IN: Driver's context */
  SWS_BufferH hBuffer                 /* IN: Buffer to send (owned by caller) */
)){
  SWS_size_t len;

  TRACE( 9, ("FakeSend(%p,%p)\n",pvContext,hBuffer));
  (void)pvContext;

  len = SWS_BufferSize( hBuffer);
  ASSERT( len >= 60 /* Min Ethernet frame */);
  ASSERT( len <= sizeof( s_buffer) );

  /* Copy frame to output buffer */
#if 0
  VERIFY( !SWS_BufferCopyToMemory( s_buffer, hBuffer, 0, len));
#else /* Do inline */
  {
  SWS_size_t uLen = len, uSrc = 0;
  void* pvDst = s_buffer;
  while ( uLen)
    {
    SWS_PTR( const void) pvSrc;
    SWS_size_t uLenSrc;

    pvSrc = SWS_BufferGetFragment( hBuffer, uSrc, &uLenSrc);
    ASSERT( NULL != pvSrc);

    if ( uLen < uLenSrc)
      uLenSrc = uLen;

    memcpy( pvDst, pvSrc, uLenSrc);

    uSrc += uLenSrc;
    pvDst = (char*)pvDst + uLenSrc;
    uLen -= uLenSrc;
    }
  }
#endif

  /* Pass data back up */
  /* NB the RecvInd callback may be made from IRQ mode */
  (*s_upcalls.pfnRecvInd)( s_upcalls.pvContext, s_buffer, (size_t)len);

  return -1; /* -1 = busy - make SendAck callback when ready */
  }


/*
 * Receive complete.  Buffer is returned to driver
 * Single threaded
 */
static SWS_CB( void, FakeRecvAck(
  SWS_PTR( void) pvContext,           /* IN: Driver's context */
  SWS_PTR( void) pvData,              /* IN: Buffer */
  size_t len                          /* IN: Buffer size */
)){
  TRACE( 9, ("FakeRecvAck(%p,%p,%u)\n",pvContext,pvData,len));
  ASSERT( s_buffer == pvData);
  ASSERT( len <= sizeof( s_buffer) );
  (void)pvContext;
  (void)pvData;
  (void)len;

  /* Loopback complete.  Output buffer is now ready to be re-used */
  /* NB the SendAck callback may be made from IRQ mode */
  (*s_upcalls.pfnSendAck)( s_upcalls.pvContext, 0/* 0= sent OK */);
  }


/*
 * Get the adapter's MAC address
 */
static SWS_CB( int, FakeGetMacAddr(   /* Return !0 on error */
  SWS_PTR( void) pvContext,           /* IN: Driver's context */
  SWS_PTR( SWS_EnetMacAddress) pMac   /* OUT: Mac address */
)){
  TRACE( 9, ("FakeGetMacAddr(%p,%p)\n",pvContext,pMac));
  (void)pvContext;

  *pMac = s_macAddr;
  return 0;
  }

#endif /*OPT_DATALINK_ETHERNET*/
/* End of file */
