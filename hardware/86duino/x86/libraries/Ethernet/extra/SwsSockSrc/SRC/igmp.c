/****************************************************************************
 *
 * File name   : igmp.c
 * Function    : IGMP handling
 * Project     : SwsSock
 * Authors     : Lawrence Rust
 * Systems     : ANSI C.
 *
 ****************************************************************************
 *
 * Created by Lawrence Rust, Software Systems Consultants
 * lvr@softsystem.co.uk. Tel/Fax +33 5 49 72 79 63
 *
 * 21-Dec-02, LVR, Created.
 *
 * Comments:
 * --------
 * This file implements IGMP as described in RFC 1112 appendix 1
 *
 ****************************************************************************
 */

/* Compilation options */
#include "config.h"

/* Exports */
#include "sws_igmp.h"

#if OPT_PROTO_IGMP

/* Imports */
#include <stddef.h>
#include <string.h>

#include "sws_buf.h"
#include "sws_ip.h"
#include "sws_sock.h"
#include "ip.h"
#include "platform.h"

#include "debug.h"


/* Prototypes */


/* Module data */

/* Stats */
static Atomic_t s_aTxPackets;         /* Total packets sent */
static Atomic_t s_aTxBytes;           /* Total bytes sent */
static Atomic_t s_aRxPackets;         /* Total packets received */
static Atomic_t s_aRxBytes;           /* Total bytes received */


/*
 * Initialise the module
 */
extern int SWS_IgmpInit( void)        /* Returns !0 on error */
  {
  TRACE( 9, ("SWS_IgmpInit()\n"));

  AtomicWrite( &s_aTxPackets, 0);
  AtomicWrite( &s_aTxBytes, 0);
  AtomicWrite( &s_aRxPackets, 0);
  AtomicWrite( &s_aRxBytes, 0);

  return 0;
  }


/*
 * Finalise the module
 */
extern void SWS_IgmpFinal( void)
  {
  TRACE( 1, ("IGMP: TxPackets:%u TxBytes:%u RxPackets:%u RxBytes:%u\n",
    AtomicRead( &s_aTxPackets), AtomicRead( &s_aTxBytes),
    AtomicRead( &s_aRxPackets), AtomicRead( &s_aRxBytes)
  ));
  }


/*
 * Send an IGMP datagram
 */
SWS_EXPORT( int, SWS_IgmpSend(        /* Returns !0 on error */
  ui32 src,                           /* IN: Source IP address or SWS_INADDR_ANY */
  ui32 dst,                           /* IN: Destination IP address, NOT SWS_INADDR_ANY */
  unsigned type,                      /* IN: IGMP type */
  unsigned resp,                      /* IN: IGMP response time */
  ui32 group                          /* IN: IGMP group */
)){
  SWS_BufferH hBuffer;
  SWS_PTR( void) pvIgmp;
  int iRet;
  SWS_size_t uLen;
  SWS_IpOptions opts;

  TRACE( 9, ("SWS_IgmpSend(%08lx,%08lx,%#x,%u,%08lx)\n",src,dst,type,resp,group));

  if ( SWS_INADDR_ANY == dst)
    {
    /* This will happen for replies to BOOTP datagrams */
    TRACE( 2, ("SWS_IgmpSend() invalid destination\n"));
    return SWS_EINVAL;
    }

  TRACE( 3, ("SWS_IgmpSend() src:%08lx dst:%08lx type %#x group %08lx\n",
    src, dst, type, group));

  /* Get buffer for IGMP header */
  hBuffer = SWS_BufferGet( SWS_IGMP_HDR, SWS_kBufferPrefix);
  if ( NULL == hBuffer )
    {
    TRACE( 1, ("*** SWS_IgmpSend() Out of buffers\n"));
    return SWS_ENOBUFS;
    }

  /* Set options */
  memset( &opts, 0, sizeof( opts));
  opts.flags.multicast_loop = 0;
  opts.multicast_ttl = 1;
  /* Router alert option RFC 2113 */
  opts.opts[ 0] = (ui8)(0x80 + 20); /* Copied, class 0, opt 20 */
  opts.opts[ 1] = 4; /* len */
  opts.opts[ 2] = 0;
  opts.opts[ 3] = 0;
  opts.uOptLen = 4;

  pvIgmp = SWS_BufferGetFragment( hBuffer, 0, &uLen);
  ASSERT( NULL != pvIgmp);
  ASSERT( uLen >= SWS_IGMP_HDR);

  SWS_POKE8( pvIgmp, SWS_IGMP_TYPE, (ui8)type);
  if ( SWS_IGMP_QUERY == type)
    SWS_POKE8( pvIgmp, SWS_IGMP_TIME, (ui8)resp);
  else
    SWS_POKE8( pvIgmp, SWS_IGMP_TIME, 0);
  SWS_POKE16( pvIgmp, SWS_IGMP_CSUM, 0); /* 0 the checksum */
  SWS_POKE32( pvIgmp, SWS_IGMP_GROUP, group);

  /* Calc. IGMP checksum */
  uLen = SWS_BufferSize( hBuffer);
  SWS_POKECSUM( pvIgmp, SWS_IGMP_CSUM, SWS_IpChecksum( hBuffer, 0, uLen, 0));
  /*ASSERT( !SWS_IpChecksum( hBuffer, 0, uLen, 0));*/

  AtomicIncrement( &s_aTxPackets);
  AtomicAdd( &s_aTxBytes, (int)uLen);

  iRet = SWS_IpSend(
    src, dst,
    SWS_IGMP_PROTOCOL,
    hBuffer,
    SWS_IP_IDENT_DEFAULT,
    &opts
  );

  SWS_BufferRelease( hBuffer);

  return iRet;
  }


/*
 * Process a received IGMP datagram
 */
SWS_EXPORT( int, SWS_IgmpRecv(
  SWS_BufferH hBuffer,                /* IN: IP packet including header */
  size_t uHdrLen,                     /* IN: IP header length including options */
  ui32 src,
  ui32 group,                         /* IN: multicast IP address */
  ui32 uHostIp                        /* IN: specific adapter IP address */
)){
  SWS_PTR( void) pvHdr;
  SWS_size_t uSize, uLen;
  unsigned type;

  TRACE( 9, ("SWS_IgmpRecv(%p,%u,%08lx,%08lx,%08lx)\n",
    hBuffer,uHdrLen,src,group,uHostIp));
  ASSERT( NULL != hBuffer);

  uSize = SWS_BufferSize( hBuffer);
  AtomicIncrement( &s_aRxPackets);
  AtomicAdd( &s_aRxBytes, (int)(uSize - uHdrLen));

  /* Get -> IGMP header */
  pvHdr = SWS_BufferGetFragment( hBuffer, uHdrLen, &uLen);
  if ( NULL == pvHdr || uLen < SWS_IGMP_HDR)
    {
    TRACE( 1, ("*** SWS_IgmpRecv() invalid header\n"));
    return SWS_EINVAL;
    }

  /* Verify IGMP checksum */
  if ( SWS_IpChecksum( hBuffer, uHdrLen, uSize - uHdrLen, 0))
    {
    TRACE( 1, ("*** SWS_IgmpRecv() Invalid checksum\n"));
    return SWS_EINVAL;
    }

  /* Get group address */
  SWS_PEEK32( group, pvHdr, SWS_IGMP_GROUP);

  /* Decode type */
  SWS_PEEK8( type, pvHdr, SWS_IGMP_TYPE);
  TRACE( 3, ("SWS_IgmpRecv() src:%08lx group:%08lx type %u len:%u\n",
    src, group, type, uSize - uHdrLen - SWS_IGMP_HDR));
  switch ( type)
    {
  case SWS_IGMP_QUERY:
    /* Don't reply if unconfigured IP address */
    if ( SWS_INADDR_ANY != uHostIp)
      {
      /* Get max response time */
      SWS_PEEK8( type, pvHdr, SWS_IGMP_TIME);
      SWS_IpIgmpQuery( uHostIp, group, type);
      }
    break;

  case SWS_IGMP_REPORT:
  case SWS_IGMP_REPORT2:
    if ( src != uHostIp)
      SWS_IpIgmpReport( uHostIp, group);
    break;

  case SWS_IGMP_LEAVE:
    break;

  default:
    break;
    }

  return 0;
  }

#else
char SWS_Igmp;
#endif /* OPT_PROTO_IGMP */
/* End of file */
