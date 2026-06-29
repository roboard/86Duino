/****************************************************************************
 *
 * File name   : icmp.c
 * Function    : ICMP handling
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
 * 18-Oct-01, LVR, SWS_IcmpRecv tests buffer len before getting 1st frag
 * 08-Jul-09, LVR, Copy buffer before calling IcmpEcho
 *
 * Comments:
 * --------
 * This file implements ICMP as described in RFC 792
 *
 ****************************************************************************
 */

/* Compilation options */
#include "config.h"

/* Exports */
#include "sws_icmp.h"

#if OPT_PROTO_ICMP
/* Imports */
#include <stddef.h>
#include <string.h>
#include <limits.h>

#include "sws_buf.h"
#include "sws_ip.h"
#include "sws_sock.h"
#include "ip.h"
#include "platform.h"

#include "debug.h"
COMPILATION_REQUIRES( 8 == CHAR_BIT);


/* Prototypes */
#if OPT_ICMP_ECHO_SERVER
static int IcmpEcho( unsigned, SWS_BufferH, size_t, ui32, ui32);
#endif


/* Module data */
#if OPT_ICMP_ECHO_SERVER
static int s_echo_server;
#endif

/* Stats */
static Atomic_t s_aTxPackets;         /* Total packets sent */
static Atomic_t s_aTxBytes;           /* Total bytes sent */
static Atomic_t s_aRxPackets;         /* Total packets received */
static Atomic_t s_aRxBytes;           /* Total bytes received */


/*
 * Initialise the module
 */
extern int SWS_IcmpInit( void)        /* Returns !0 on error */
  {
  TRACE( 9, ("SWS_IcmpInit()\n"));

#if OPT_ICMP_ECHO_SERVER
  s_echo_server = (int)PlatformPropertyGetUint(
    PKEY_ICMP_ECHO_SERVER, OPT_ICMP_ECHO_SERVER);
#endif

  AtomicWrite( &s_aTxPackets, 0);
  AtomicWrite( &s_aTxBytes, 0);
  AtomicWrite( &s_aRxPackets, 0);
  AtomicWrite( &s_aRxBytes, 0);

  return 0;
  }


/*
 * Finalise the module
 */
extern void SWS_IcmpFinal( void)
  {
  TRACE( 1, ("ICMP: TxPackets:%u TxBytes:%u RxPackets:%u RxBytes:%u\n",
    AtomicRead( &s_aTxPackets), AtomicRead( &s_aTxBytes),
    AtomicRead( &s_aRxPackets), AtomicRead( &s_aRxBytes)
  ));
  }


/*
 * Send an ICMP datagram
 */
SWS_EXPORT( int, SWS_IcmpSend(      /* Returns !0 on error */
  ui32 src,                           /* IN: Source IP address or SWS_INADDR_ANY */
  ui32 dst,                           /* IN: Destination IP address, NOT SWS_INADDR_ANY */
  unsigned type,                      /* IN: ICMP type */
  unsigned code,                      /* IN: ICMP code */
  ui32 data,                          /* IN: ICMP optional data (identifier & sequence) */
  SWS_BufferH hBuffer,                /* IN: Datagram content, NULL = none */
  unsigned ident,                     /* IN: IP ident, 0= don't care */
  SWS_PTR( struct SWS_IpOptions) pIpOpts /* IN: IP option data */
)){
  SWS_PTR( void) pvIcmp;
  int iRet;
  SWS_size_t uLen, uHdr;
  unsigned hdr;

  TRACE( 9, ("SWS_IcmpSend(%08lx,%08lx,%u,%u,%08lx,%p,%u,%p)\n",
    src,dst,type,code,data,hBuffer,ident,pIpOpts));

  if ( SWS_INADDR_ANY == dst)
    {
    /* This will happen for replies to BOOTP datagrams */
    TRACE( 2, ("SWS_IcmpSend() invalid destination\n"));
    return SWS_EINVAL;
    }

  /* Verify correct data length */
  uLen = NULL != hBuffer ? SWS_BufferSize( hBuffer) : 0;
  TRACE( 3, ("SWS_IcmpSend() src:%08lx dst:%08lx type %u data %u\n",
    src, dst, type, uLen));

  switch ( type)
    {
  case SWS_ICMP_DEST_UNREACH:
  case SWS_ICMP_TIME_EXCEED:
  case SWS_ICMP_PARAM_PROB:
  case SWS_ICMP_QUENCH:
  case SWS_ICMP_REDIRECT:
    /* Header + at least 8 bytes of data */
    if ( 0 == uLen)
      {
      hdr = SWS_IPHDR_MINLEN;
      }
    else
      {
      pvIcmp = SWS_BufferGetFragment( hBuffer, 0, &uHdr);
      if ( uHdr >= SWS_IPHDR_MINLEN)
        {
        SWS_PEEK8( hdr, pvIcmp, SWS_IPHDR_VER_IHL);
        hdr = 4 * (hdr & 0xf);
        }
      else
        {
        hdr = SWS_IPHDR_MINLEN;
        }
      }
    if ( uLen < hdr)
      {
      TRACE( 1, ("***SWS_IcmpSend() missing IP header\n"));
      return SWS_EINVAL;
      }

    /* Truncate buffer to just header + 8 bytes */
    if ( uLen > (hdr + 8))
      VERIFY( !SWS_BufferRemoveBack( hBuffer, uLen - (hdr + 8)));
    break;

  case SWS_ICMP_TIMESTAMP:
  case SWS_ICMP_TIME_REPLY:
    if ( uLen < 12)
      {
      TRACE( 1, ("***SWS_IcmpSend() invalid timestamp data\n"));
      return SWS_EINVAL;
      }
    break;

  case SWS_ICMP_ECHO_REPLY:
  case SWS_ICMP_ECHO:
    /* Variable data */
    break;

  case SWS_ICMP_INFO_RQST:
  case SWS_ICMP_INFO_REPLY:
    /* These requests are obsolete see  RFC1123 */
    break;

  case SWS_ICMP_ADDR_MASK:
  case SWS_ICMP_ADDR_MASK_REPLY:
  case SWS_ICMP_IPSP:
  default:
    break;
    }

  /* Get buffer for ICMP header */
  if ( NULL == hBuffer)
    hBuffer = SWS_BufferGet( SWS_ICMP_HDR, SWS_kBufferPrefix);
  else
    SWS_BufferAddRef( hBuffer);

  /* Prepend fixed ICMP header */
  if ( NULL == hBuffer || SWS_BufferAppendFront( hBuffer, SWS_ICMP_HDR))
    {
    TRACE( 1, ("*** SWS_IcmpSend() Out of buffers\n"));
    SWS_BufferRelease( hBuffer);
    return SWS_ENOBUFS;
    }

  pvIcmp = SWS_BufferGetFragment( hBuffer, 0, &uLen);
  ASSERT( NULL != pvIcmp);
  ASSERT( uLen >= SWS_ICMP_HDR);

  SWS_POKE8( pvIcmp, SWS_ICMP_TYPE, (ui8)type);
  SWS_POKE8( pvIcmp, SWS_ICMP_CODE, (ui8)code);
  SWS_POKE16( pvIcmp, SWS_ICMP_CSUM, 0); /* 0 the checksum */
  SWS_POKE32( pvIcmp, SWS_ICMP_DATA, data);

#if OPT_ICMP_ECHO_SERVER
  if ( SWS_ICMP_TIME_REPLY == type)
    {
    /* Set the Transmit Timestamp */
    ui32 ticks = PlatformTime();
    SWS_PTR( void) pv = SWS_BufferGetFragment( hBuffer, SWS_ICMP_HDR + 8, &uHdr);
    ASSERT( NULL != pv);
    ASSERT( uHdr >= 4);
    SWS_POKE32( pv, 0, ticks);
    }
#endif

  /* Calc. ICMP checksum */
  uLen = SWS_BufferSize( hBuffer);
  SWS_POKECSUM( pvIcmp, SWS_ICMP_CSUM, SWS_IpChecksum( hBuffer, 0, uLen, 0));
  /*ASSERT( !SWS_IpChecksum( hBuffer, 0, uLen, 0));*/

  AtomicIncrement( &s_aTxPackets);
  AtomicAdd( &s_aTxBytes, (int)uLen);

  iRet = SWS_IpSend(
    src, dst,
    SWS_ICMP_PROTOCOL,
    hBuffer,
    ident,
    pIpOpts
  );

  SWS_BufferRelease( hBuffer);

  return iRet;
  }


/*
 * Process a received ICMP datagram
 */
SWS_EXPORT( int, SWS_IcmpRecv(
  SWS_BufferH hBuffer,                /* IN: IP packet including header */
  size_t uHdrLen,                     /* IN: IP header length including options */
  ui32 src,
  ui32 dst,
  ui32 uHostIP                        /* IN: specific adapter IP address */
)){
  SWS_PTR( void) pvHdr;
  SWS_size_t uSize, uLen;
  unsigned type, code;
  ui32 len;
#if !OPT_ICMP_ECHO_SERVER
  (void)dst;
#endif

  TRACE( 9, ("SWS_IcmpRecv(%p,%u,%08lx,%08lx)\n",
    hBuffer,uHdrLen,src,uHostIP));
  ASSERT( NULL != hBuffer);

  uSize = SWS_BufferSize( hBuffer);
  AtomicIncrement( &s_aRxPackets);
  AtomicAdd( &s_aRxBytes, (int)(uSize - uHdrLen));

  /* Get -> ICMP header */
  pvHdr = SWS_BufferGetFragment( hBuffer, uHdrLen, &uLen);
  if ( NULL == pvHdr || uLen < SWS_ICMP_HDR)
    {
    TRACE( 1, ("*** SWS_IcmpRecv() invalid header\n"));
    return SWS_EINVAL;
    }

  /* Verify ICMP checksum */
  if ( SWS_IpChecksum( hBuffer, uHdrLen, uSize - uHdrLen, 0))
    {
    TRACE( 1, ("*** SWS_IcmpRecv() Invalid checksum\n"));
    return SWS_EINVAL;
    }

  /* Select action */
  SWS_PEEK8( type, pvHdr, SWS_ICMP_TYPE);
  TRACE( 3, ("SWS_IcmpRecv() src:%08lx i/f:%08lx type %u data %u\n",
    src, uHostIP, type, uSize - uHdrLen - SWS_ICMP_HDR));
  switch ( type)
    {
    /* Error messages */
  case SWS_ICMP_DEST_UNREACH:
  case SWS_ICMP_TIME_EXCEED:
  case SWS_ICMP_PARAM_PROB:
  case SWS_ICMP_QUENCH:
    /* Data contains original IP header + 8 bytes of data */
    if ( uSize < uHdrLen + SWS_ICMP_HDR + SWS_IPHDR_MINLEN + 8)
      {
      TRACE( 1, ("***SWS_IcmpRecv() missing IP header context\n"));
      return SWS_EINVAL;
      }

    SWS_PEEK8( code, pvHdr, SWS_ICMP_CODE);
    SWS_PEEK32( len, pvHdr, SWS_ICMP_DATA);

    /* Pass error message up to protocol */
    SWS_IpIcmpError(
      hBuffer,
      (size_t)uHdrLen + SWS_ICMP_HDR,
      src, uHostIP,
      type, code,
      len
    );
    break;

  case SWS_ICMP_REDIRECT:
    /* Data contains original IP header + 8 bytes of data */
    if ( uSize < uHdrLen + SWS_ICMP_HDR + SWS_IPHDR_MINLEN + 8)
      {
      TRACE( 1, ("***SWS_IcmpRecv() missing IP header context\n"));
      return SWS_EINVAL;
      }

    SWS_PEEK8( code, pvHdr, SWS_ICMP_CODE);
    SWS_PEEK32( len, pvHdr, SWS_ICMP_DATA);

    /* Pass up to router */
    SWS_IpIcmpRedirect(
      hBuffer,
      (size_t)uHdrLen + SWS_ICMP_HDR,
      src, uHostIP,
      code, len
    );
    break;

  case SWS_ICMP_TIMESTAMP:
    /* 3x32-bit time stamps */
    if ( uSize < uHdrLen + SWS_ICMP_HDR + 3*4)
      {
      TRACE( 1, ("***SWS_IcmpRecv() invalid timestamp data\n"));
      return SWS_EINVAL;
      }

#if OPT_ICMP_ECHO_SERVER
    if ( s_echo_server)
      {
      /* Don't reply if unconfigured IP address or multicast */
      if ( SWS_INADDR_ANY != uHostIP
        && !SWS_IN_MULTICAST( dst)
      ) {
        hBuffer = SWS_BufferDup( hBuffer, 0);
        if ( NULL != hBuffer)
          {
          /* Set the Receive Timestamp */
          len = PlatformTime();
          SWS_POKE32( pvHdr, SWS_ICMP_HDR + 4, len);

          IcmpEcho( SWS_ICMP_TIME_REPLY, hBuffer, uHdrLen, src, uHostIP);
          SWS_BufferRelease( hBuffer);
          }
        }
      }
#endif
    break;

  case SWS_ICMP_TIME_REPLY:
    /* 3* 32-bit timestamps */
    break;

  case SWS_ICMP_ECHO:
#if OPT_ICMP_ECHO_SERVER
    if ( s_echo_server)
      {
      /* Don't reply if unconfigured IP address or multicast */
      if ( SWS_INADDR_ANY != uHostIP
        && !SWS_IN_MULTICAST( dst)
      ) {
        hBuffer = SWS_BufferDup( hBuffer, 0);
        if ( NULL != hBuffer)
          {
          IcmpEcho( SWS_ICMP_ECHO_REPLY, hBuffer, uHdrLen, src, uHostIP);
          SWS_BufferRelease( hBuffer);
          }
        }
      }
#endif
    break;

  case SWS_ICMP_ECHO_REPLY:
    /* Variable data */
    break;

  case SWS_ICMP_INFO_RQST:
  case SWS_ICMP_INFO_REPLY:
    /* These requests are obsolete see RFC1123 */
    break;

  case SWS_ICMP_ADDR_MASK:
  case SWS_ICMP_ADDR_MASK_REPLY:
  case SWS_ICMP_IPSP:
  default:
    break;
    }

  return 0;
  }


#if OPT_ICMP_ECHO_SERVER
/*
 * ICMP echo server
 */
static int IcmpEcho(
  unsigned type,
  SWS_BufferH hBuffer,                /* IN: IP packet including header */
  size_t uHdrLen,                     /* IN: IP header length including options */
  ui32 src,
  ui32 dst                            /* IN: IP spec address */
) {
  SWS_PTR( void) pvIpHdr;
  SWS_size_t uLen;
  unsigned code;
  ui32 idseq;
  SWS_IpOptions options;
#if OPT_IP_OPTIONS
  unsigned hdr;
#endif

  TRACE( 9, ("IcmpEcho(%u,%p,%u,%08lx,%08lx)\n",type,hBuffer,uHdrLen,src,dst));
  ASSERT( NULL != hBuffer);

  memset( &options, 0, sizeof( options) );

  /* Get -> IP header */
  pvIpHdr = SWS_BufferGetFragment( hBuffer, 0, &uLen);
  ASSERT( NULL != pvIpHdr);
  ASSERT( uLen >= SWS_IPHDR_MINLEN);

  /* Copy TOS */
  SWS_PEEK8( code, pvIpHdr, SWS_IPHDR_TOS);
  options.tos = (ui8)code;

  /* Copy DF */
  SWS_PEEK16( code, pvIpHdr, SWS_IPHDR_FLAGS_OFFSET);
  if ( code & SWS_IP_FLAG_DF)
    options.flags.df = 1;

#if OPT_IP_OPTIONS
  SWS_PEEK8( hdr, pvIpHdr, SWS_IPHDR_VER_IHL);
  hdr = 4 * (hdr & 0xf);

  /* Are there IP options? */
  if ( hdr > SWS_IPHDR_MINLEN)
    {
    /* Yes, so parse them */
    unsigned offset;

    memset( &options, 0, sizeof( options));

    offset = SWS_IPHDR_MINLEN;
    while ( offset < hdr)
      {
      unsigned opt, len, ptr;

      /* Get option kind */
      SWS_PEEK8( opt, pvIpHdr, offset);

      /* Decode option */
      if ( SWS_IP_OPT_END == opt)
        break;

      switch ( opt)
        {
      case SWS_IP_OPT_NOP:
        ++offset;
        break;

      case SWS_IP_OPT_TIMESTAMP:
        SWS_PEEK8( len, pvIpHdr, offset + 1);

        /* NB the timestamp is filled in by the IP module */
        memcpy( &options.opts[ options.uOptLen], (char*)pvIpHdr + offset, len);

        options.uOptLen = (ui8)(options.uOptLen + len);
        offset += len;
        break;

      case SWS_IP_OPT_LOOSE_ROUTE:
      case SWS_IP_OPT_STRICT_ROUTE:
        SWS_PEEK8( len, pvIpHdr, offset + 1);

        /* Reverse the source route */
        SWS_POKE8( options.opts, options.uOptLen, (ui8)opt);
        SWS_POKE8( options.opts, options.uOptLen + 1, (ui8)len); /* len */
        SWS_POKE8( options.opts, options.uOptLen + 2, (ui8)(opt = 4)); /* ptr */

        SWS_PEEK8( ptr, pvIpHdr, offset + 2); /* pointer */
        if ( ptr >= 8)
          {
          /* Get last host */
          ptr -= 4;
          SWS_PEEK32( idseq, pvIpHdr, offset + ptr - 1);

          while ( ptr >= 8)
            {
            ui32 addr;

            ptr -= 4;
            SWS_PEEK32( addr, pvIpHdr, offset + ptr - 1);
            SWS_POKE32( options.opts, options.uOptLen + opt - 1, addr);
            opt += 4;
            }

          /* Add src as last hop */
          SWS_POKE32( options.opts, options.uOptLen + opt - 1, src);

          /* Send this back to the last host */
          src = idseq;
          }

        options.uOptLen = (ui8)(options.uOptLen + len);
        offset += len;
        break;

      case SWS_IP_OPT_RECORD_ROUTE:
        SWS_PEEK8( len, pvIpHdr, offset + 1);

        /* NB this option is updated by the IP module */
        memcpy( &options.opts[ options.uOptLen], (char*)pvIpHdr + offset, len);

        options.uOptLen = (ui8)(options.uOptLen + len);
        offset += len;
        break;

      default:
        SWS_PEEK8( len, pvIpHdr, offset + 1);
        offset += len;
        break;
        }
      }

    if ( options.uOptLen > 0)
      {
      SWS_POKE8( options.opts, options.uOptLen, SWS_IP_OPT_END);
      ++options.uOptLen;
      }
    }
#endif

  /* Get -> ICMP header */
  pvIpHdr = SWS_BufferGetFragment( hBuffer, uHdrLen, &uLen);
  ASSERT( NULL != pvIpHdr);
  ASSERT( uLen >= SWS_ICMP_HDR);

  /* Get sender's ICMP identifier/sequence */
  SWS_PEEK8( code, pvIpHdr, SWS_ICMP_CODE);
  SWS_PEEK32( idseq, pvIpHdr, SWS_ICMP_DATA);

  /* Strip off IP & ICMP headers leaving sender's data */
  VERIFY( !SWS_BufferRemoveFront( hBuffer, uHdrLen + SWS_ICMP_HDR));

  return SWS_IcmpSend(
    dst, src,
    type, code, idseq,
    hBuffer,
    SWS_IP_IDENT_DEFAULT,
    &options
  );
  }
#endif
#else
char SWS_Icmp;
#endif /* OPT_ICMP */
/* End of file */
