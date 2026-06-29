/****************************************************************************
 *
 * File name   : sws_dmpi.c
 * Function    : IP packet dumper
 * Authors     : Lawrence Rust
 * Systems     : ANSI C.
 *
 ****************************************************************************
 *
 * Created by Lawrence Rust, Software Systems Consultants
 * lvr@softsystem.co.uk. Tel/Fax +33 5 49 72 79 63
 *
 * 16-Apr-01, LVR, Created.
 * 03-Oct-01, LVR, Fix TRACE warnings for ARM SDT 2.51 .
 * 29-Oct-02, LVR, Fix TCP OPTs causing invalid checksum.
 *                 Change timestamp to time of day.
 * 22-Nov-02, LVR, Add DHCP.
 * 03-Dec-02, LVR, Add DNS & NETBIOS.
 * 22-Dec-02, LVR, Added IGMP.
 * 06-Jan-03, LVR, Added RIP.
 * 02-May-05, LVR, Display IP offset if > 0 & verbosity >= 1
 * 05-May-05, LVR, Display gateway for ICMP redirect
 * 08-May-05, LVR, Replace PlatformGetUint with SWS_CfgGetValue
 *
 * Comments:
 * --------
 * This file implements an IP packet dumper.
 *
 ****************************************************************************
 */

/* Compilation options */
#ifndef OPT_DUMP_NETBIOS
#define OPT_DUMP_NETBIOS 1
#endif
#ifndef OPT_DUMP_DHCP
#define OPT_DUMP_DHCP 1
#endif
#ifndef OPT_DUMP_DNS
#define OPT_DUMP_DNS 1
#endif
#ifndef OPT_DUMP_RIP
#define OPT_DUMP_RIP 1
#endif

#ifdef _MSC_VER
# pragma warning ( disable : 4127)    /* conditional expression is constant */
# pragma warning ( disable : 4514)    /* unreferenced inline function has been removed */
# if _MSC_VER <= 800 /* VC++ 1.5 & earlier */
#  pragma warning( disable : 4705)    /* statement has no effect */
# endif
#endif

/* Exports */
#include "sws_dmpi.h"

/* Imports */
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>   /* va_list */
#include <ctype.h>
#if __STDC_VERSION__ >= 199901L
#include <wctype.h>
#include <wchar.h>
#endif

#include "sws_gef.h"
#include "sws_buf.h"
#include "sws_cfg.h"
#include "sws_sock.h"

#include "sws_ip.h"
#include "sws_icmp.h"
#include "sws_igmp.h"
#include "sws_udp.h"
#include "sws_tcp.h"
#if OPT_DUMP_DHCP
#include "sws_dhcp.h"
#endif
#if OPT_DUMP_NETBIOS || OPT_DUMP_DNS
#include "sws_dns.h"
#include "sws_smb.h"
#endif
#if OPT_DUMP_RIP
#include "sws_rip.h"
#endif


/* Read/write in network (MSB first, or big-endian) byte order */
#ifndef SWS_PEEK8
#define SWS_PEEK8(_v,_p,_o)\
  _v = (*(SWS_PTR(const ui8))((SWS_PTR(const char))(_p) + (_o)))
#endif
#ifndef SWS_PEEK16
#define SWS_PEEK16(_v,_p,_o)\
  _v = SWS_ntohs(*(SWS_PTR(SWS_PACKED(const ui16)))((SWS_PTR(const char))(_p) + (_o)))
#endif
#ifndef SWS_PEEK32
#define SWS_PEEK32(_v,_p,_o)\
  _v = SWS_ntohl(*(SWS_PTR(SWS_PACKED(const ui32)))((SWS_PTR(const char))(_p) + (_o)))
#endif

/* Dump an IP packet */
static int IpDump( SWS_PTR( char) const, char, SWS_BufferH);
static int IpDumpUnknown( char*,SWS_BufferH,unsigned,unsigned,ui32,ui32,unsigned,int);
static int IpDumpIcmp( char*,SWS_BufferH,unsigned,unsigned,ui32,ui32,unsigned,int);
static int IpDumpIgmp( char*,SWS_BufferH,unsigned,unsigned,ui32,ui32,unsigned,int);
static int IpDumpUdp( char*,SWS_BufferH,unsigned,unsigned,ui32,ui32,unsigned,int);
static int IpDumpTcp( char*,SWS_BufferH,unsigned,unsigned,ui32,ui32,unsigned,int);

static int ValToAddress( char*, ui32);
static int SecsToTime( char*, ui32 secs);
static int HexDump( char*, SWS_BufferH,unsigned,unsigned);

#if OPT_DUMP_DHCP
static int IpDumpDhcp( char*, SWS_BufferH,unsigned,unsigned);
#endif

#if OPT_DUMP_DNS || OPT_DUMP_NETBIOS
static int IpDumpDns( char*, SWS_BufferH,unsigned,unsigned,int);
static unsigned IpDumpResourceRecord( char*, SWS_BufferH, unsigned, unsigned, int);
static unsigned IpDumpName( char*, const void*, unsigned, int, SWS_size_t);
#endif

#if OPT_DUMP_NETBIOS
static int IpDumpNetbiosSessionService( char*, SWS_BufferH,unsigned,unsigned);
#endif

#if OPT_DUMP_RIP
static int IpDumpRip( char*, SWS_BufferH,unsigned,unsigned);
#endif

#if OPT_DUMP_DHCP || OPT_DUMP_RIP
static unsigned CountOnes( ui32);
#endif


/*
 * Module data
 */
static int s_iIpVerbosity = -1;
static unsigned s_tcp, s_udp, s_icmp, s_igmp; /* b0=TxEn b1=RxEn */
static unsigned s_portx;
static ui32 s_ipsrc, s_ipmask;


/*
 * Configure the packet dumper
 */
SWS_EXTERN( void, SWS_DmpIpSetup(
  int iVerbosity                      /* IN: Verbosity 0= off, 1= normal, 2= more etc */
)){
  char szBuf[ 24]; /* Enough for uuu.uuu.uuu.uuu/uu */
  unsigned uMask, ip[4];

  if ( iVerbosity >= 0)
    s_iIpVerbosity = iVerbosity;
  else if ( s_iIpVerbosity < 0)
    s_iIpVerbosity = (int)SWS_CfgGetValue( "log", "pdump", 0);

  s_tcp = (unsigned)SWS_CfgGetValue( "log", "tcp", 0xffff);
  s_udp = (unsigned)SWS_CfgGetValue( "log", "udp", 0xffff);
  s_icmp = (unsigned)SWS_CfgGetValue( "log", "icmp", 0xffff);
  s_igmp = (unsigned)SWS_CfgGetValue( "log", "igmp", 0xffff); /* b2= classdEn */
  s_portx = (unsigned)SWS_CfgGetValue( "log", "noport", 0);

  SWS_CfgGetString( "log", "ipfilt", "", szBuf, sizeof szBuf);
  uMask = 32;
  if ( 4 <= sscanf( szBuf, " %u.%u.%u.%u/%u",
      &ip[0], &ip[1], &ip[2], &ip[3], &uMask)
  ) {
    s_ipsrc = ((ui32)ip[0] << 24)
      + ((ui32)ip[1] << 16)
      + (ip[2] << 8)
      + ip[3];
    s_ipmask = 0xffffffffL << (32 - uMask);
    s_ipsrc &= s_ipmask;
    }
  else
    {
    s_ipmask = 0;
    s_ipsrc = 0;
    }
  }


/*
 * GEF handler
 */
SWS_EXTERN( int, SWS_DmpIp(
  SWS_PTR( char) pszBuf,              /* OUT: text */
  int event,                          /* IN: SWS_IpEvent */
  va_list va                          /* IN: Tx: Next IP address, Rx: isBroadcast */
)){
  int count = 0;
  SWS_BufferH hBuffer;
  const char* pszErr;
  int err;

  switch ( event)
    {
  default:
    break;

  case SWS_kGefIpTx:
    hBuffer = va_arg( va, SWS_BufferH);
    count = IpDump( pszBuf, 'T', hBuffer);
    break;

  case SWS_kGefIpRx:
    hBuffer = va_arg( va, SWS_BufferH);
    err = va_arg( va, int); /* Rx: isBroadcast */
    count = IpDump( pszBuf, (char)(err ? 'B' : 'R'), hBuffer);
    break;

  case SWS_kGefIpError:
    if ( s_iIpVerbosity > 0)
      {
      hBuffer = va_arg( va, SWS_BufferH);
      err = va_arg( va, int); /* Error code */
      pszErr = va_arg( va, const char*);

      switch ( err)
        {
      case SWS_kErrorTxDropped:   /* Out of Tx buffers */
        count = sprintf( pszBuf, " *** IP Tx drop: %s", pszErr );
        break;

      case SWS_kErrorTxFailed:    /* Data link error + int error code */
        count = sprintf( pszBuf, " *** IP Tx fail: %s (%d)", pszErr, va_arg( va, int) );
        break;

      case SWS_kErrorRxDropped:   /* Out of Rx buffers */
        count = sprintf( pszBuf, " *** IP Rx drop: %s", pszErr );
        break;

      case SWS_kErrorRxBadHdr:    /* Bad IP header */
      case SWS_kErrorRxBadHost:   /* IP destination != host address */
      case SWS_kErrorRxProtocol:  /* No protocol handler */
      case SWS_kErrorTxNoRoute:   /* Not possible to route packet */
        break;

      case SWS_kErrorIpGleaned:   /* IP address gleaned */
        count = sprintf( pszBuf, " *** IP %s", pszErr );
        break;

      default:
        count = sprintf( pszBuf, " *** IP ERROR %d: %s", err, pszErr );
        break;
        }
      }
    break;

  case SWS_kGefTcpError:
    if ( s_iIpVerbosity > 0)
      {
      pszErr = va_arg( va, const char*);
      count = sprintf( pszBuf, " *** TCP %s", pszErr );
      }
    break;

  case SWS_kGefUdpError:
    if ( s_iIpVerbosity > 0)
      {
      pszErr = va_arg( va, const char*);
      count = sprintf( pszBuf, " *** UDP %s", pszErr );
      }
    break;

  case SWS_kGefRawError:
    if ( s_iIpVerbosity > 0)
      {
      pszErr = va_arg( va, const char*);
      count = sprintf( pszBuf, " *** RAW %s", pszErr );
      }
    break;
    }

  return count;
  }


/*
 * Dump a packet
 */
SWS_EXTERN( int, SWS_DmpIpPacket(     /* Returns space used in buffer */
  SWS_PTR( char) pszBuf,              /* OUT: Text */
  SWS_BufferH hBuffer                 /* IN: packet */
)){
  SWS_PTR( void) pvIpHdr;
  SWS_size_t uLen;
  ui32 src, dst;
  unsigned total, offset;
  unsigned version, ihl, protocol;
  int df, mf;
  SWS_PTR( char) const pszStart = pszBuf;

  if ( NULL == pszBuf || NULL == hBuffer)
    return 0;

  /* Get -> IP header */
  pvIpHdr = SWS_BufferGetFragment( hBuffer, 0, &uLen);
  if ( NULL == pvIpHdr || uLen < SWS_IPHDR_MINLEN)
    return sprintf( pszBuf, " ***Incomplete header");

  /* Get IP header info */
  SWS_PEEK8( ihl, pvIpHdr, SWS_IPHDR_VER_IHL);
  version = ihl >> 4;
  if ( 4 != version)
    return sprintf( pszBuf, " ***IPv%u", version);

  ihl &= 0xf;
  ihl *= 4;
  if ( ihl < SWS_IPHDR_MINLEN)
    return sprintf( pszBuf, " ***IHL:%u too small", ihl);

  uLen = SWS_BufferSize( hBuffer);

  SWS_PEEK16( total, pvIpHdr, SWS_IPHDR_TOTAL);
  if ( total > uLen)
    pszBuf += sprintf( pszBuf, " ***TRUNCATED:%u", total);
  else if ( total < uLen)
    pszBuf += sprintf( pszBuf, "(%u)", total);

  if ( ihl > uLen)
    {
    pszBuf += sprintf( pszBuf, " ***IHL:%u too big", ihl);
    return (int)(pszBuf - pszStart);
    }

  /* Check header checksum */
  if ( SWS_IpChecksum( hBuffer, 0, ihl, 0))
    {
    unsigned sum;
    SWS_PEEK16( sum, pvIpHdr, SWS_IPHDR_CHECKSUM);
    pszBuf += sprintf( pszBuf, " ***BAD IP CSUM:%#x", sum);
    }

  SWS_PEEK16( offset, pvIpHdr, SWS_IPHDR_FLAGS_OFFSET);
  df = (offset & SWS_IP_FLAG_DF) ? 1 : 0;
  mf = (offset & SWS_IP_FLAG_MF) ? 1 : 0;
  if ( offset & SWS_IP_FLAG_RESV)
    pszBuf += sprintf( pszBuf, " ***RF***");
  offset &= SWS_IP_OFFSET_MASK;

  /* Display src & dst */
  SWS_PEEK32( src, pvIpHdr, SWS_IPHDR_SRC);
  SWS_PEEK32( dst, pvIpHdr, SWS_IPHDR_DST);

  pszBuf += sprintf( pszBuf, " ");
  pszBuf += ValToAddress( pszBuf, src);
  pszBuf += sprintf( pszBuf, " >");
  pszBuf += ValToAddress( pszBuf, dst);

  if ( s_iIpVerbosity >= 3
    || (s_iIpVerbosity >= 2 && (mf || offset))
  ) {
    /* Display packet ID */
    unsigned ident;
    SWS_PEEK16( ident, pvIpHdr, SWS_IPHDR_ID);
    pszBuf += sprintf( pszBuf, " ID:%u", ident);
    }

  if ( s_iIpVerbosity >= 2)
    {
    /* Display verbose IP info */
    unsigned tos, ttl, indx;

    SWS_PEEK8( ttl, pvIpHdr, SWS_IPHDR_TTL);
    pszBuf += sprintf( pszBuf, " TTL:%u", ttl);

    if ( df)
      pszBuf += sprintf( pszBuf, " DF");
    if ( mf)
      pszBuf += sprintf( pszBuf, " MF");
    if ( mf || offset)
      pszBuf += sprintf( pszBuf, " OFF:%u", 8 * offset);

    SWS_PEEK8( tos, pvIpHdr, SWS_IPHDR_TOS);
    if ( tos & 1)
      {
      /* Probably Explicit Congestion Notification (ECN) RFC 3168 */
      pszBuf += sprintf( pszBuf, " ECN:%#x", tos);
      }
    else
      {
      /* Precedence */
      switch ( tos & SWS_IP_PREC_MASK)
        {
      case SWS_IP_PREC_ROUTINE:
        break;
      case SWS_IP_PREC_PRI:
        pszBuf += sprintf( pszBuf, " PRI");
        break;
      case SWS_IP_PREC_IMM:
        pszBuf += sprintf( pszBuf, " IMM");
        break;
      case SWS_IP_PREC_FLASH:
        pszBuf += sprintf( pszBuf, " FLSH");
        break;
      case SWS_IP_PREC_FLASHOR:
        pszBuf += sprintf( pszBuf, " OVER");
        break;
      case SWS_IP_PREC_CRIT:
        pszBuf += sprintf( pszBuf, " CRIT");
        break;
      case SWS_IP_PREC_INWC:
        pszBuf += sprintf( pszBuf, " ICTL");
        break;
      case SWS_IP_PREC_NETC:
        pszBuf += sprintf( pszBuf, " NCTL");
        break;
        }

      /* Type of service */
      switch ( tos & SWS_IP_TOS_MASK)
        {
      case SWS_IP_TOS_DELAY:
        pszBuf += sprintf( pszBuf, " DLY");
        break;
      case SWS_IP_TOS_THRU:
        pszBuf += sprintf( pszBuf, " SPD");
        break;
      case SWS_IP_TOS_REL:
        pszBuf += sprintf( pszBuf, " REL");
        break;
      case SWS_IP_TOS_COST:
        pszBuf += sprintf( pszBuf, " CST");
        break;
      case SWS_IP_TOS_NORM:
        break;
      default:
        pszBuf += sprintf( pszBuf, " TOS:%#x", tos & SWS_IP_TOS_MASK);
        break;
        }
      }

    if ( ihl > SWS_IPHDR_MINLEN)
      pszBuf += sprintf( pszBuf, " IHL:%u", ihl);

    /* Display IP options */
    indx = SWS_IPHDR_MINLEN;
    while ( indx < ihl)
      {
      unsigned opt, len;
      unsigned v1, v2;
      ui32 v3, v4;

      /* Get option kind */
      SWS_PEEK8( opt, pvIpHdr, indx);

      /* Decode option */
      if ( SWS_IP_OPT_END == opt)
        break;
      switch ( opt)
        {
      case SWS_IP_OPT_NOP:
        len = 1;
        break;

      case SWS_IP_OPT_SECURITY:
        SWS_PEEK8( len, pvIpHdr, indx + 1);
        if ( len < 9)
          {
          pszBuf += sprintf( pszBuf, " ***INVALID OPT%#x***",opt);
          break;
          }

        SWS_PEEK16( v1, pvIpHdr, indx + 2); /* security */
        SWS_PEEK16( v2, pvIpHdr, indx + 4); /* compartment */
        SWS_PEEK16( v3, pvIpHdr, indx + 6); /* handling */
        SWS_PEEK32( v4, pvIpHdr, indx + 8); /* tcc */
        v4 >>= 8;
        pszBuf += sprintf( pszBuf, " SECURITY:%04x-%u-%04lx-%06lx", v1,v2,v3,v4);
        break;

      case SWS_IP_OPT_LOOSE_ROUTE:
        SWS_PEEK8( len, pvIpHdr, indx + 1);
        if ( len < 3)
          {
          pszBuf += sprintf( pszBuf, " ***INVALID OPT%#x***",opt);
          break;
          }

        SWS_PEEK8( v1, pvIpHdr, indx + 2); /* pointer */
        pszBuf += sprintf( pszBuf, " LOOSE_ROUTE(%u/%u)",v1,len);
        for ( v2 = 3; v2 + 4 <= len; v2 += 4)
          {
          SWS_PEEK32( v3, pvIpHdr, indx + v2);
          pszBuf += sprintf( pszBuf, "~");
          pszBuf += ValToAddress( pszBuf, v3);
          }
        break;

      case SWS_IP_OPT_STRICT_ROUTE:
        SWS_PEEK8( len, pvIpHdr, indx + 1);
        if ( len < 3)
          {
          pszBuf += sprintf( pszBuf, " ***INVALID OPT%#x***",opt);
          break;
          }

        SWS_PEEK8( v1, pvIpHdr, indx + 2); /* pointer */
        pszBuf += sprintf( pszBuf, " STRICT_ROUTE(%u/%u)",v1,len);
        for ( v2 = 3; v2 + 4 <= len; v2 += 4)
          {
          SWS_PEEK32( v3, pvIpHdr, indx + v2);
          pszBuf += sprintf( pszBuf, "~");
          pszBuf += ValToAddress( pszBuf, v3);
          }
        break;

      case SWS_IP_OPT_RECORD_ROUTE:
        SWS_PEEK8( len, pvIpHdr, indx + 1);
        if ( len < 3)
          {
          pszBuf += sprintf( pszBuf, " ***INVALID OPT%#x***",opt);
          break;
          }

        SWS_PEEK8( v1, pvIpHdr, indx + 2); /* pointer */
        pszBuf += sprintf( pszBuf, " RECORD_ROUTE(%u/%u)",v1,len);
        for ( v2 = 3; v2 + 1 < v1 && v2 < len; v2 += 4)
          {
          SWS_PEEK32( v3, pvIpHdr, indx + v2);
          pszBuf += sprintf( pszBuf, "~");
          pszBuf += ValToAddress( pszBuf, v3);
          }
        break;

      case SWS_IP_OPT_SID:
        SWS_PEEK8( len, pvIpHdr, indx + 1);
        if ( len < 4)
          {
          pszBuf += sprintf( pszBuf, " ***INVALID OPT%#x***",opt);
          break;
          }

        SWS_PEEK16( v1, pvIpHdr, indx + 2);
        pszBuf += sprintf( pszBuf, " SID(%u)",v1);
        break;

      case SWS_IP_OPT_TIMESTAMP:
        SWS_PEEK8( len, pvIpHdr, indx + 1);
        if ( len < 4)
          {
          pszBuf += sprintf( pszBuf, " ***INVALID OPT%#x***",opt);
          break;
          }

        SWS_PEEK8( v1, pvIpHdr, indx + 2); /* pointer */
        SWS_PEEK8( opt, pvIpHdr, indx + 3); /* flags */
        pszBuf += sprintf( pszBuf, " TIMESTAMP(%u/%u,%#x)",v1,len,opt);
        v2 = 4;
        while ( v2 + 1 < v1 && v2 < len)
          {
          /* Time is milliseconds since midnight UT */
          if ( 0 == (opt & 0xf))
            {
            SWS_PEEK32( v3, pvIpHdr, indx + v2);
            v3 &= 0x7fffffffL; /* Non-standard times set MSB */
            pszBuf += sprintf( pszBuf, "~");
            pszBuf += SecsToTime( pszBuf, v3 / 1000U);
            pszBuf += sprintf( pszBuf, ".%03lu", v3 % 1000U);
            v2 += 4;
            }
          else
            {
            SWS_PEEK32( v3, pvIpHdr, indx + v2);
            SWS_PEEK32( v4, pvIpHdr, indx + v2 + 4);
            v4 &= 0x7fffffffL; /* Non-standard times set MSB */
            pszBuf += sprintf( pszBuf, "~");
            pszBuf += ValToAddress( pszBuf, v3);
            pszBuf += sprintf( pszBuf, "@");
            pszBuf += SecsToTime( pszBuf, v4 / 1000U);
            pszBuf += sprintf( pszBuf, ".%03lu", v4 % 1000U);
            v2 += 8;
            }
          }
        break;

      case SWS_IP_OPT_ROUTER_ALERT:
        SWS_PEEK8( len, pvIpHdr, indx + 1);
        if ( len < 4)
          {
          pszBuf += sprintf( pszBuf, " ***INVALID OPT%#x***",opt);
          break;
          }

        SWS_PEEK16( v1, pvIpHdr, indx + 2);
        pszBuf += sprintf( pszBuf, " ROUTER_ALERT(%#x)",v1);
        break;

      default:
        SWS_PEEK8( len, pvIpHdr, indx + 1);
        pszBuf += sprintf( pszBuf, " OPT%#x,%u",opt,len);
        break;
        }

      if ( 0 == len)
        break;

      indx += len;
      }
    }
  else if ( s_iIpVerbosity >= 1 && (mf || offset))
    {
    if ( mf)
      pszBuf += sprintf( pszBuf, " MF");

    pszBuf += sprintf( pszBuf, " OFF:%u", 8 * offset);
    }

  /* Append protocol specific info */
  SWS_PEEK8( protocol, pvIpHdr, SWS_IPHDR_PROTOCOL);
  switch ( protocol)
    {
  case SWS_ICMP_PROTOCOL:
    pszBuf += sprintf( pszBuf, " ICMP");
    pszBuf += IpDumpIcmp( pszBuf,  hBuffer, total, ihl, src, dst, offset, mf);
    break;

  case SWS_IGMP_PROTOCOL:
    pszBuf += sprintf( pszBuf, " IGMP");
    pszBuf += IpDumpIgmp( pszBuf,  hBuffer, total, ihl, src, dst, offset, mf);
    break;

  case SWS_UDP_PROTOCOL:
    pszBuf += sprintf( pszBuf, " UDP");
    pszBuf += IpDumpUdp( pszBuf,  hBuffer, total, ihl, src, dst, offset, mf);
    break;

  case SWS_TCP_PROTOCOL:
    pszBuf += sprintf( pszBuf, " TCP");
    pszBuf += IpDumpTcp( pszBuf,  hBuffer, total, ihl, src, dst, offset, mf);
    break;

  default:
    pszBuf += sprintf( pszBuf, " PRTCL:%u", protocol);
    pszBuf += IpDumpUnknown( pszBuf,  hBuffer, total, ihl, src, dst, offset, mf);
    break;
    }

  return (int)(pszBuf - pszStart);
  }


/****************************************************************************
 * Internal functions
 ****************************************************************************/

/*
 * IP packet dump
 */
static int IpDump(
  SWS_PTR( char) const pszBuf,        /* OUT: text */
  char mode,
  SWS_BufferH hBuf
) {
  if ( s_iIpVerbosity > 0)
    {
    SWS_PTR( void) pvIpHdr;
    SWS_size_t uLen;
    int count;

    /* Packet filtering */
    pvIpHdr = SWS_BufferGetFragment( hBuf, 0, &uLen);
    if ( NULL != pvIpHdr && uLen >= SWS_IPHDR_MINLEN)
      {
      unsigned protocol;
      ui32 addr;

      /* Protocol filter */
      SWS_PEEK8( protocol, pvIpHdr, SWS_IPHDR_PROTOCOL);
      switch ( protocol)
        {
      case SWS_ICMP_PROTOCOL:
        if ( 0 == (('T' == mode ? 1 : 2) & s_icmp))
          return 0;
        break;

      case SWS_IGMP_PROTOCOL:
        if ( 0 == (('T' == mode ? 1 : 2) & s_igmp))
          return 0;
        SWS_PEEK32( addr, pvIpHdr, SWS_IPHDR_DST);
        if ( SWS_IN_CLASSD( addr) && !(s_igmp & 4))
          return 0;
        break;

      case SWS_UDP_PROTOCOL:
        if ( 0 == (('T' == mode ? 1 : 2) & s_udp))
          return 0;
        break;

      case SWS_TCP_PROTOCOL:
        if ( 0 == (('T' == mode ? 1 : 2) & s_tcp))
          return 0;

        if ( s_portx)
          {
          unsigned ihl;

          SWS_PEEK8( ihl, pvIpHdr, SWS_IPHDR_VER_IHL);
          ihl &= 0xf;
          ihl *= 4;
          if ( uLen >= ihl + 4)
            {
            SWS_PEEK16( protocol, pvIpHdr, ihl); /* src */
            if ( protocol == s_portx)
              return 0;
            if ( 'T' != mode)
              {
              SWS_PEEK16( protocol, pvIpHdr, ihl + 2); /* dst */
              if ( protocol == s_portx)
                return 0;
              }
            }
          }
        break;

      default:
        break;
        }

      /* IP address filter */
      if ( s_ipmask)
        {
        SWS_PEEK32( addr, pvIpHdr, SWS_IPHDR_DST);
        if ( s_ipsrc == (addr & s_ipmask))
          return 0;
        if ( 'T' != mode)
          {
          SWS_PEEK32( addr, pvIpHdr, SWS_IPHDR_SRC);
          if ( s_ipsrc == (addr & s_ipmask))
            return 0;
          }
        }
      }

    /* Prepend direction and length */
    uLen = SWS_BufferSize( hBuf);
    count = sprintf( pszBuf, " %c:%u", mode, (unsigned)uLen);

    return count + SWS_DmpIpPacket( pszBuf + count, hBuf);
    }

  return 0;
  }


/*
 * Unknown packet type dump
 */
static int IpDumpUnknown(
  char* pszBuf,                       /* OUT: text */
  SWS_BufferH hBuffer,                /* IN: IP packet */
  unsigned total,
  unsigned ihl,
  ui32 src,
  ui32 dst,
  unsigned offset,
  int mf
) {
  (void)total;
  (void)dst;
  (void)src;
  (void)mf;

  if ( 0 == offset && s_iIpVerbosity >= 2)
    return HexDump( pszBuf, hBuffer, ihl, 8);

  return 0;
  }


/****************************************************************************
 * ICMP
 ****************************************************************************/
/*
 * ICMP packet dump
 */
static int IpDumpIcmp(
  char* pszBuf,                       /* OUT: text */
  SWS_BufferH hBuffer,                /* IN: IP packet */
  unsigned total,
  unsigned ihl,
  ui32 src,
  ui32 dst,
  unsigned offset,
  int mf
) {
  char* const pszStart = pszBuf;
  SWS_PTR( void) pvIpHdr;
  SWS_size_t uLen;
  (void)src;

  if ( 0 != offset)
    return 0;

  pvIpHdr = SWS_BufferGetFragment( hBuffer, ihl, &uLen);

  if ( NULL == pvIpHdr || uLen < SWS_ICMP_HDR)
    pszBuf += sprintf( pszBuf, " Truncated");
  else
    {
    unsigned type, code;
    int iHdr = 0; /* Assume no original packet header */

    SWS_PEEK8( type, pvIpHdr, SWS_ICMP_TYPE);
    SWS_PEEK8( code, pvIpHdr, SWS_ICMP_CODE);

    switch ( type)
      {
    case SWS_ICMP_DEST_UNREACH:
      iHdr = 1; /* Header from original packet maybe attached */
      switch ( code)
        {
      default:
        pszBuf += sprintf( pszBuf, " DEST_UNREACH(%u)", code);
        break;
      case SWS_ICMP_NET_UNREACH:
        pszBuf += sprintf( pszBuf, " NET_UNREACH");
        break;
      case SWS_ICMP_HOST_UNREACH:
        pszBuf += sprintf( pszBuf, " HOST_UNREACH");
        break;
      case SWS_ICMP_PROT_UNREACH:
        pszBuf += sprintf( pszBuf, " PROTOCOL_UNREACH");
        break;
      case SWS_ICMP_PORT_UNREACH:
        pszBuf += sprintf( pszBuf, " PORT_UNREACH");
        break;
      case SWS_ICMP_FRAG_NEEDED:
        pszBuf += sprintf( pszBuf, " FRAG_NEEDED");
        break;
      case SWS_ICMP_ROUTE_FAIL:
        pszBuf += sprintf( pszBuf, " SRCROUTE_FAIL");
        break;
      case SWS_ICMP_ADMIN_PROHIB:
        pszBuf += sprintf( pszBuf, " ADMIN_PROHIB");
        break;
        }
      break;

    case SWS_ICMP_TIME_EXCEED:
      iHdr = 1; /* Header from original packet maybe attached */
      switch ( code)
        {
      default:
        pszBuf += sprintf( pszBuf, " TIME_EXCEED(%u)", code);
        break;
      case SWS_ICMP_TTL_EXCEED:
        pszBuf += sprintf( pszBuf, " TTL_EXCEED");
        break;
      case SWS_ICMP_FRAG_EXCEED:
        pszBuf += sprintf( pszBuf, " FRAG_EXCEED");
        break;
        }
      break;

    case SWS_ICMP_PARAM_PROB:
      iHdr = 1; /* Header from original packet maybe attached */
      pszBuf += sprintf( pszBuf, " PARAM_PROB(%u)", code);
      break;
    case SWS_ICMP_QUENCH:
      iHdr = 1; /* Header from original packet maybe attached */
      pszBuf += sprintf( pszBuf, " QUENCH");
      break;

    case SWS_ICMP_REDIRECT:
      iHdr = 1; /* Header from original packet maybe attached */
      switch ( code)
        {
      case SWS_ICMP_REDR_NET:
        pszBuf += sprintf( pszBuf, " REDIRECT_NET ");
        break;
      case SWS_ICMP_REDR_HOST:
        pszBuf += sprintf( pszBuf, " REDIRECT_HOST ");
        break;
      case SWS_ICMP_REDR_TOS_NET:
        pszBuf += sprintf( pszBuf, " REDIRECT_TOS/NET ");
        break;
      case SWS_ICMP_REDR_TOS_HOST:
        pszBuf += sprintf( pszBuf, " REDIRECT_TOS/HOST ");
        break;
      default:
        pszBuf += sprintf( pszBuf, " REDIRECT(%u) ", code);
        break;
        }
      /* display new gateway */
      SWS_PEEK32( dst, pvIpHdr, SWS_ICMP_DATA);
      pszBuf += ValToAddress( pszBuf, dst);
      pszBuf += sprintf( pszBuf, "<");
      break;

    case SWS_ICMP_TIMESTAMP:
      pszBuf += sprintf( pszBuf, " TIMESTAMP");
      break;
    case SWS_ICMP_TIME_REPLY:
      pszBuf += sprintf( pszBuf, " TIME_REPLY");
      break;
    case SWS_ICMP_ECHO:
      pszBuf += sprintf( pszBuf, " ECHO");
      if ( 0 != code)
        pszBuf += sprintf( pszBuf, "(%u)", code);
      if ( s_iIpVerbosity >= 2)
        {
        SWS_PEEK16( code, pvIpHdr, SWS_ICMP_DATA);
        pszBuf += sprintf( pszBuf, " ID:%#x", code);
        SWS_PEEK16( code, pvIpHdr, SWS_ICMP_DATA + 2);
        pszBuf += sprintf( pszBuf, " SEQ:%#x", code);
        }
      break;
    case SWS_ICMP_ECHO_REPLY:
      pszBuf += sprintf( pszBuf, " ECHO_REPLY");
      if ( 0 != code)
        pszBuf += sprintf( pszBuf, "(%u)", code);
      if ( s_iIpVerbosity >= 2)
        {
        SWS_PEEK16( code, pvIpHdr, SWS_ICMP_DATA);
        pszBuf += sprintf( pszBuf, " ID:%#x", code);
        SWS_PEEK16( code, pvIpHdr, SWS_ICMP_DATA + 2);
        pszBuf += sprintf( pszBuf, " SEQ:%#x", code);
        }
      break;
    case SWS_ICMP_INFO_RQST:
      pszBuf += sprintf( pszBuf, " INFO_RQST(%u)", code);
      break;
    case SWS_ICMP_INFO_REPLY:
      pszBuf += sprintf( pszBuf, " INFO_REPLY(%u)", code);
      break;
    case SWS_ICMP_ADDR_MASK:
      pszBuf += sprintf( pszBuf, " ADDR_MASK");
      break;
    case SWS_ICMP_ADDR_MASK_REPLY:
      pszBuf += sprintf( pszBuf, " ADDR_MASK_REPLY");
      break;
    case SWS_ICMP_IPSP:
      pszBuf += sprintf( pszBuf, " IPSP C:%u", code);
      break;
    default:
      pszBuf += sprintf( pszBuf, " T:%u C:%u", type, code);
      break;
      }

    /* Is original IP packet attached? */
    if ( iHdr && uLen >= SWS_ICMP_HDR + SWS_IPHDR_MINLEN + 4)
      {
      /* Yes, get original IP dest */
      SWS_PEEK32( dst, pvIpHdr, SWS_ICMP_HDR + SWS_IPHDR_DST);

      pszBuf += sprintf( pszBuf, " ");
      pszBuf += ValToAddress( pszBuf, dst);

      /* Get original IP header len */
      SWS_PEEK8( code, pvIpHdr, SWS_ICMP_HDR + SWS_IPHDR_VER_IHL);
      code &= 0x0f;
      code *= 4;

      /* Get original protocol */
      SWS_PEEK8( type, pvIpHdr, SWS_ICMP_HDR + SWS_IPHDR_PROTOCOL);
      switch ( type)
        {
      case SWS_TCP_PROTOCOL:
        SWS_PEEK16( src, pvIpHdr, SWS_ICMP_HDR + code + SWS_TCP_SRC);
        SWS_PEEK16( dst, pvIpHdr, SWS_ICMP_HDR + code + SWS_TCP_DST);
        pszBuf += sprintf( pszBuf, " TCP %u >%u", (unsigned)src, (unsigned)dst);
        break;

      case SWS_UDP_PROTOCOL:
        SWS_PEEK16( src, pvIpHdr, SWS_ICMP_HDR + code + SWS_UDP_SRC);
        SWS_PEEK16( dst, pvIpHdr, SWS_ICMP_HDR + code + SWS_UDP_DST);
        pszBuf += sprintf( pszBuf, " UDP %u >%u", (unsigned)src, (unsigned)dst);
        break;

      case SWS_ICMP_PROTOCOL:
        SWS_PEEK8( src, pvIpHdr, SWS_ICMP_HDR + code + SWS_ICMP_TYPE);
        switch ( src)
          {
        case SWS_ICMP_ECHO:
          pszBuf += sprintf( pszBuf, " ICMP ECHO");
          break;
        default:
          SWS_PEEK8( dst, pvIpHdr, SWS_ICMP_HDR + code + SWS_ICMP_CODE);
          pszBuf += sprintf( pszBuf, " ICMP type:%u code:%u",
            (unsigned)src, (unsigned)dst);
          break;
          }
        break;

      default:
        pszBuf += sprintf( pszBuf, " PRTCL %u", type);
        break;
        }
      }

    if ( !mf)
      {
      unsigned len = total - ihl;

      if ( len > SWS_BufferSize( hBuffer))
        pszBuf += sprintf( pszBuf, " ***TRUNCATED***");
      else if ( SWS_IpChecksum( hBuffer, ihl, len, 0))
        {
        SWS_PEEK16( len, pvIpHdr, SWS_ICMP_CSUM);
        pszBuf += sprintf( pszBuf, " ***BADSUM:%#x", len);
        }
      }
    }

  return (int)(pszBuf - pszStart);
  }


/****************************************************************************
 * IGMP
 ****************************************************************************/
/*
 * IGMP packet dump
 */
static int IpDumpIgmp(
  char* pszBuf,                       /* OUT: text */
  SWS_BufferH hBuffer,                /* IN: IP packet */
  unsigned total,
  unsigned ihl,
  ui32 src,
  ui32 dst,
  unsigned offset,
  int mf
) {
  char* const pszStart = pszBuf;
  SWS_PTR( void) pvIpHdr;
  SWS_size_t uLen;
  (void)dst;
  (void)src;

  if ( 0 != offset)
    return 0;

  pvIpHdr = SWS_BufferGetFragment( hBuffer, ihl, &uLen);

  if ( NULL == pvIpHdr || uLen < SWS_IGMP_HDR)
    pszBuf += sprintf( pszBuf, " Truncated");
  else
    {
    unsigned type, resp;
    ui32 group;

    SWS_PEEK32( group, pvIpHdr, SWS_IGMP_GROUP);
    SWS_PEEK8( type, pvIpHdr, SWS_IGMP_TYPE);
    switch ( type)
      {
    case SWS_IGMP_QUERY:
      SWS_PEEK8( resp, pvIpHdr, SWS_IGMP_TIME);
      pszBuf += sprintf( pszBuf, " QUERY T:%u ", resp);
      pszBuf += ValToAddress( pszBuf, group);
      if ( uLen > SWS_IGMP_HDR)
        pszBuf += HexDump( pszBuf, hBuffer, ihl + SWS_IGMP_HDR, uLen - SWS_IGMP_HDR);
      break;

    case SWS_IGMP_REPORT:
      pszBuf += sprintf( pszBuf, " REPORT ");
      pszBuf += ValToAddress( pszBuf, group);
      break;

    case SWS_IGMP_REPORT2:
      pszBuf += sprintf( pszBuf, " REPORTV2 ");
      pszBuf += ValToAddress( pszBuf, group);
      break;

    case SWS_IGMP_LEAVE:
      pszBuf += sprintf( pszBuf, " LEAVE ");
      pszBuf += ValToAddress( pszBuf, group);
      break;

    case SWS_IGMP_REPORT3:
      SWS_PEEK16( resp, pvIpHdr, SWS_IGMP_GROUP + 2);
      pszBuf += sprintf( pszBuf, " REPORTV3 grps:%u", resp);
      if ( uLen > SWS_IGMP_HDR)
        pszBuf += HexDump( pszBuf, hBuffer, ihl + SWS_IGMP_HDR, uLen - SWS_IGMP_HDR);
      break;

    default:
      pszBuf += sprintf( pszBuf, " TYPE:%#x ", type);
      pszBuf += HexDump( pszBuf, hBuffer, ihl + SWS_IGMP_GROUP, uLen - SWS_IGMP_GROUP);
      break;
      }

    if ( !mf)
      {
      unsigned len = total - ihl;

      if ( len > SWS_BufferSize( hBuffer))
        pszBuf += sprintf( pszBuf, " ***TRUNCATED***");
      else if ( SWS_IpChecksum( hBuffer, ihl, len, 0))
        {
        SWS_PEEK16( len, pvIpHdr, SWS_IGMP_CSUM);
        pszBuf += sprintf( pszBuf, " ***BADSUM:%#x", len);
        }
      }
    }

  return (int)(pszBuf - pszStart);
  }


/****************************************************************************
 * UDP
 ****************************************************************************/
/*
 * UDP packet dump
 */
static int IpDumpUdp(
  char* pszBuf,                       /* OUT: text */
  SWS_BufferH hBuffer,                /* IN: IP packet */
  unsigned total,
  unsigned ihl,
  ui32 src,
  ui32 dst,
  unsigned offset,
  int mf
) {
  char* const pszStart = pszBuf;
  SWS_PTR( void) pvIpHdr;
  SWS_size_t uLen;

  if ( 0 != offset)
    return 0;

  pvIpHdr = SWS_BufferGetFragment( hBuffer, ihl, &uLen);

  if ( NULL == pvIpHdr || uLen < SWS_UDP_LEN)
    pszBuf += sprintf( pszBuf, " Truncated");
  else
    {
    unsigned srcPort, dstPort, len, sum;

    SWS_PEEK16( srcPort, pvIpHdr, SWS_UDP_SRC);
    SWS_PEEK16( dstPort, pvIpHdr, SWS_UDP_DST);
    SWS_PEEK16( len, pvIpHdr, SWS_UDP_LEN);
    SWS_PEEK8( sum, pvIpHdr, SWS_UDP_SUM);

    if ( s_iIpVerbosity >= 3)
      pszBuf += sprintf( pszBuf, " LEN:%u", len - SWS_UDP_HDR);

    if ( !mf && len != total - ihl)
      {
      pszBuf += sprintf( pszBuf, " ***BADLEN:%u***",len);
      len = total - ihl;
      }

    if ( !mf)
      {
      if ( len > SWS_BufferSize( hBuffer))
        {
        pszBuf += sprintf( pszBuf, " ***TRUNCATED***");
        }
      else if ( 0 != sum)
        {
        ui32 uSum = SWS_IpPseudoHdr( src, dst, SWS_UDP_PROTOCOL, len);
        if ( SWS_IpChecksum( hBuffer, ihl, len, uSum))
          pszBuf += sprintf( pszBuf, " ***BADSUM:%#x", sum);
        }
      }

    if ( s_iIpVerbosity >= 2 && 0 == sum)
      pszBuf += sprintf( pszBuf, " NOSUM");

    pszBuf += sprintf( pszBuf, " %u >%u", srcPort, dstPort);

#if OPT_DUMP_NETBIOS || OPT_DUMP_DNS
    if ( len >= SWS_UDP_HDR + SWS_DNS_HDR_LEN)
      {
      if ( SWS_DNS_SERVER == dstPort || SWS_DNS_SERVER == srcPort)
        {
        pszBuf += sprintf( pszBuf, " DNS");
        if ( s_iIpVerbosity >= 2)
          pszBuf += IpDumpDns( pszBuf, hBuffer, ihl + SWS_UDP_HDR, len - SWS_UDP_HDR, 0);
        }
      else if ( SWS_DNS_NB == dstPort || SWS_DNS_NB == srcPort)
        {
        pszBuf += sprintf( pszBuf, " NBNS");
        if ( s_iIpVerbosity >= 2)
          pszBuf += IpDumpDns( pszBuf, hBuffer, ihl + SWS_UDP_HDR, len - SWS_UDP_HDR, 1);
        }
      }
#endif

#if OPT_DUMP_DHCP
    if ( (SWS_DHCP_SERVER == dstPort && SWS_DHCP_CLIENT == srcPort)
      || (SWS_DHCP_SERVER == srcPort && SWS_DHCP_CLIENT == dstPort)
    ) {
      if ( len >= SWS_UDP_HDR + SWS_DHCP_OPTS)
        {
        pszBuf += IpDumpDhcp( pszBuf, hBuffer, ihl + SWS_UDP_HDR, len - SWS_UDP_HDR);
        }
      }
#endif

#if OPT_DUMP_RIP
    if ( SWS_RIP_SERVER == dstPort || SWS_RIP_SERVER == srcPort)
      {
      if ( len >= SWS_UDP_HDR + SWS_RIP_HDR_LEN)
        {
        pszBuf += sprintf( pszBuf, " RIP");
        if ( s_iIpVerbosity >= 2)
          pszBuf += IpDumpRip( pszBuf, hBuffer, ihl + SWS_UDP_HDR, len - SWS_UDP_HDR);
        }
      }
#endif
    }

  return (int)(pszBuf - pszStart);
  }


/****************************************************************************
 * TCP
 ****************************************************************************/
/*
 * TCP packet dump
 */
static int IpDumpTcp(
  char* pszBuf,                       /* OUT: text */
  SWS_BufferH hBuffer,                /* IN: IP packet */
  unsigned total,
  unsigned ihl,
  ui32 src,
  ui32 dst,
  unsigned offset,
  int mf
) {
  char* const pszStart = pszBuf;
  SWS_PTR( void) pvIpHdr;
  SWS_size_t uLen;

  if ( 0 != offset)
    return 0;

  pvIpHdr = SWS_BufferGetFragment( hBuffer, ihl, &uLen);

  if ( NULL == pvIpHdr || uLen < SWS_TCP_OPT)
    pszBuf += sprintf( pszBuf, " Truncated");
  else
    {
    unsigned srcPort, dstPort, flags, window, urgent, hdrLen, len, indx;
    ui32 seq, ack;

    SWS_PEEK16( srcPort, pvIpHdr, SWS_TCP_SRC);
    SWS_PEEK16( dstPort, pvIpHdr, SWS_TCP_DST);
    SWS_PEEK32( seq, pvIpHdr, SWS_TCP_SEQ);
    SWS_PEEK32( ack, pvIpHdr, SWS_TCP_ACK);
    SWS_PEEK16( flags, pvIpHdr, SWS_TCP_FLG);
    hdrLen = 4 * (flags >> 12);
    SWS_PEEK16( window, pvIpHdr, SWS_TCP_WND);
    SWS_PEEK16( urgent, pvIpHdr, SWS_TCP_URG);

    len = total - ihl;
    pszBuf += sprintf( pszBuf, " %u >%u", srcPort, dstPort);

    if ( flags & SWS_TCP_FINF)
      pszBuf += sprintf( pszBuf, " FIN");
    if ( flags & SWS_TCP_SYNF)
      pszBuf += sprintf( pszBuf, " SYN");
    if ( flags & SWS_TCP_RSTF)
      pszBuf += sprintf( pszBuf, " RST");
    if ( flags & SWS_TCP_PSHF)
      pszBuf += sprintf( pszBuf, " PSH");

    /* RFC 3168 Explicit Congestion Notification */
    if ( flags & SWS_TCP_ECEF)
      pszBuf += sprintf( pszBuf, " ECE");
    if ( flags & SWS_TCP_CWRF)
      pszBuf += sprintf( pszBuf, " CWR");

    if ( flags & SWS_TCP_MBZF)
      pszBuf += sprintf( pszBuf, " ***MBZ:%#x", flags & SWS_TCP_MBZF);

    if ( flags & SWS_TCP_ACKF)
      pszBuf += sprintf( pszBuf, " ACK:%#lx", ack);
    if ( flags & SWS_TCP_URGF)
      pszBuf += sprintf( pszBuf, " URG:%u", urgent);

    pszBuf += sprintf( pszBuf, " WND:%u", window);

    pszBuf += sprintf( pszBuf, " SEQ:%#lx", seq);
    if ( s_iIpVerbosity >= 3)
      pszBuf += sprintf( pszBuf, " LEN:%u", len - hdrLen);

    if ( s_iIpVerbosity >= 2)
      {
      /* Display options */
      indx = SWS_TCP_MINLEN;
      while ( indx < hdrLen)
        {
        unsigned opt, optlen;

        /* Get option kind */
        SWS_PEEK8( opt, pvIpHdr, indx);

        /* Decode option */
        if ( SWS_TCP_OPT_END == opt)
          break;

        if ( SWS_TCP_OPT_NOP == opt)
          optlen = 1;
        else
          {
          unsigned optval;
          ui32 v1, v2;

          SWS_PEEK8( optlen, pvIpHdr, indx + 1);
          switch ( opt)
            {
          case SWS_TCP_OPT_MSS:
            SWS_PEEK16( optval, pvIpHdr, indx + 2);
            pszBuf += sprintf( pszBuf, " MSS:%u", optval);
            break;
          case SWS_TCP_OPT_WS: /* RFC1323 */
            SWS_PEEK8( optval, pvIpHdr, indx + 2);
            pszBuf += sprintf( pszBuf, " WS:%u", optval);
            break;
          case SWS_TCP_OPT_SACKP: /* RFC1072 */
            pszBuf += sprintf( pszBuf, " SACKP");
            break;
          case SWS_TCP_OPT_SACK: /* RFC1072 */
            pszBuf += sprintf( pszBuf, " SACK");
            break;
          case SWS_TCP_OPT_ECHO: /* RFC1072 */
            SWS_PEEK32( v1, pvIpHdr, indx + 2);
            pszBuf += sprintf( pszBuf, " ECHO:%lu",v1);
            break;
          case SWS_TCP_OPT_ECHOR: /* RFC1072 */
            SWS_PEEK32( v1, pvIpHdr, indx + 2);
            pszBuf += sprintf( pszBuf, " ECHOR:%lu",v1);
            break;
          case SWS_TCP_OPT_TSOPT: /* RFC1323 */
            SWS_PEEK32( v1, pvIpHdr, indx + 2); /* Timestamp clock of the sender */
            SWS_PEEK32( v2, pvIpHdr, indx + 6); /* Timestamp echo reply (iff ACK set) */
            pszBuf += sprintf( pszBuf, " TSOPT:%lu:%lu",v1,v2);
            break;
          case SWS_TCP_OPT_POCP: /* RFC1693 */
            pszBuf += sprintf( pszBuf, " POCP");
            break;
          case SWS_TCP_OPT_POSP: /* RFC1693 */
            pszBuf += sprintf( pszBuf, " POSP");
            break;
          case SWS_TCP_OPT_CC:
            pszBuf += sprintf( pszBuf, " CC");
            break;
          case SWS_TCP_OPT_CCNEW:
            pszBuf += sprintf( pszBuf, " CCNEW");
            break;
          case SWS_TCP_OPT_CCECHO:
            pszBuf += sprintf( pszBuf, " CCECHO");
            break;
          case SWS_TCP_OPT_CSUMREQ: /* RFC1146 */
            pszBuf += sprintf( pszBuf, " CSUMREQ");
            break;
          case SWS_TCP_OPT_CSUMDATA: /* RFC1146 */
            pszBuf += sprintf( pszBuf, " CSUMDATA");
            break;
          case SWS_TCP_OPT_SKEETER:
            pszBuf += sprintf( pszBuf, " SKEETER");
            break;
          case SWS_TCP_OPT_BUBBA:
            pszBuf += sprintf( pszBuf, " BUBBA");
            break;
          case SWS_TCP_OPT_TRAIL:
            pszBuf += sprintf( pszBuf, " TRAIL");
            break;
          default:
            pszBuf += sprintf( pszBuf, " OPT:%u(%u)", opt, optlen);
            break;
            }
          }

        indx += optlen;
        }
      }

    /* Verify TCP checksum */
    if ( !mf)
      {
      if ( len > SWS_BufferSize( hBuffer))
        pszBuf += sprintf( pszBuf, " ***TRUNCATED***");
      else
        {
        ui32 uSum = SWS_IpPseudoHdr( src, dst, SWS_TCP_PROTOCOL, len);
        if ( SWS_IpChecksum( hBuffer, ihl, len, uSum))
          {
          SWS_PEEK16( urgent, pvIpHdr, SWS_TCP_SUM);
          pszBuf += sprintf( pszBuf, " ***BADSUM:%#x", urgent);
          }
        }
      }

#if OPT_DUMP_NETBIOS
    if ( SWS_NBSS == dstPort || SWS_NBSS == srcPort
      || SWS_CIFS == dstPort || SWS_CIFS == srcPort /* no NetBT encapsulation */
    ) {
      if ( s_iIpVerbosity >= 2 && len > hdrLen)
        {
        pszBuf += sprintf( pszBuf, " NBSS");
        pszBuf += IpDumpNetbiosSessionService(
          pszBuf, hBuffer, ihl + hdrLen, len - hdrLen);
        }
      }
#endif
    }

  return (int)(pszBuf - pszStart);
  }


#if OPT_DUMP_DHCP
/****************************************************************************
 * DHCP
 ****************************************************************************/
/*
 * DHCP packet dump (RFC2131/2)
 */
static int IpDumpDhcp(
  char* pszBuf,                       /* OUT: text */
  SWS_BufferH hBuffer,                /* IN: data */
  unsigned uOffset,
  unsigned seglen
) {
  char* const pszStart = pszBuf;
  SWS_PTR( unsigned char) buf;
  SWS_size_t uLen;

  buf = (SWS_PTR( unsigned char)) SWS_BufferGetFragment( hBuffer, uOffset, &uLen);

  if ( NULL == buf || uLen < seglen || seglen < SWS_DHCP_OPTS)
    {
    pszBuf += sprintf( pszBuf, " Message truncated");
    }
  else
    {
    int iDhcp;

    if ( seglen >= (SWS_DHCP_OPTS + 4)
      && SWS_DHCP_COOKIE1 == buf[ SWS_DHCP_OPTS + 0]
      && SWS_DHCP_COOKIE2 == buf[ SWS_DHCP_OPTS + 1]
      && SWS_DHCP_COOKIE3 == buf[ SWS_DHCP_OPTS + 2]
      && SWS_DHCP_COOKIE4 == buf[ SWS_DHCP_OPTS + 3]
    ) {
      iDhcp = 1;
      pszBuf += sprintf( pszBuf, " DHCP");
      }
    else
      {
      iDhcp = 0;
      pszBuf += sprintf( pszBuf, " BOOTP");
      }

    if ( s_iIpVerbosity >= 2)
      {
      unsigned uVal, uHlen;
      unsigned long ulVal;
      long lVal;

      SWS_PEEK8( uVal, buf, SWS_DHCP_OP);
      switch ( uVal)
        {
      case SWS_DHCP_OP_BOOTREQUEST:
        pszBuf += sprintf( pszBuf, "RQST");
        break;
      case SWS_DHCP_OP_BOOTREPLY:
        pszBuf += sprintf( pszBuf, "RPLY");
        break;
      default:
        pszBuf += sprintf( pszBuf, "OP:%u", uVal);
        break;
        }

      SWS_PEEK8( uVal, buf, SWS_DHCP_HTYPE);
      switch ( uVal)
        {
      case SWS_DHCP_HTYPE_ETHERNET:
        if ( s_iIpVerbosity >= 3)
          pszBuf += sprintf( pszBuf, " ENET");
        break;
      default:
        pszBuf += sprintf( pszBuf, " HTYPE:%u", uVal);
        break;
        }

      SWS_PEEK8( uHlen, buf, SWS_DHCP_HLEN);

      if ( s_iIpVerbosity >= 3)
        {
        SWS_PEEK8( uVal, buf, SWS_DHCP_HOPS);
        pszBuf += sprintf( pszBuf, " HOPS:%u", uVal);

        SWS_PEEK32( ulVal, buf, SWS_DHCP_XID);
        pszBuf += sprintf( pszBuf, " XID:%#lx", ulVal);

        SWS_PEEK16( uVal, buf, SWS_DHCP_SECS);
        pszBuf += sprintf( pszBuf, " SECS:%u", uVal);
        }

      SWS_PEEK16( uVal, buf, SWS_DHCP_FLAGS);
      if ( uVal)
        pszBuf += sprintf( pszBuf, " FLAGS:%#x", uVal);

      SWS_PEEK32( ulVal, buf, SWS_DHCP_CIADDR);
      if ( ulVal)
        {
        pszBuf += sprintf( pszBuf, " CI:");
        pszBuf += ValToAddress( pszBuf, ulVal);
        }

      SWS_PEEK32( ulVal, buf, SWS_DHCP_YIADDR);
      if ( ulVal)
        {
        pszBuf += sprintf( pszBuf, " YI:");
        pszBuf += ValToAddress( pszBuf, ulVal);
        }

      SWS_PEEK32( ulVal, buf, SWS_DHCP_SIADDR);
      if ( ulVal)
        {
        pszBuf += sprintf( pszBuf, " SI:");
        pszBuf += ValToAddress( pszBuf, ulVal);
        }

      SWS_PEEK32( ulVal, buf, SWS_DHCP_GIADDR);
      if ( ulVal)
        {
        pszBuf += sprintf( pszBuf, " GI:");
        pszBuf += ValToAddress( pszBuf, ulVal);
        }

      pszBuf += sprintf( pszBuf, " CHADDR");
      pszBuf += HexDump( pszBuf, hBuffer, uOffset + SWS_DHCP_CHADDR, uHlen);

      SWS_PEEK8( uVal, buf, SWS_DHCP_SNAME);
      if ( uVal)
        pszBuf += sprintf( pszBuf, " SNAM'%s'", buf + SWS_DHCP_SNAME);

      SWS_PEEK8( uVal, buf, SWS_DHCP_FILE);
      if ( uVal)
        pszBuf += sprintf( pszBuf, " FILE'%s'", buf + SWS_DHCP_FILE);

      if ( iDhcp)
        {
        unsigned offset = SWS_DHCP_OPTS + 4;

        /* Display DHCP options */
        do
          {
          unsigned len, u;

          /* Get option and length */
          SWS_PEEK8( uVal, buf, offset);
          ++offset;

          if ( SWS_DHCP_OPT_END == uVal)
            break;

          switch ( uVal)
            {
          case SWS_DHCP_OPT_NOP:
            len = 0;
            break;

          case SWS_DHCP_OPT_MSGTYPE:
            SWS_PEEK8( len, buf, offset);
            ++offset;
            SWS_PEEK8( uVal, buf, offset);
            switch ( uVal)
              {
            case SWS_DHCP_MSGTYPE_DHCPDISCOVER:
              pszBuf += sprintf( pszBuf, " DHCP_DISCOVER");
              break;
            case SWS_DHCP_MSGTYPE_DHCPOFFER:
              pszBuf += sprintf( pszBuf, " DHCP_OFFER");
              break;
            case SWS_DHCP_MSGTYPE_DHCPREQUEST:
              pszBuf += sprintf( pszBuf, " DHCP_REQUEST");
              break;
            case SWS_DHCP_MSGTYPE_DHCPDECLINE:
              pszBuf += sprintf( pszBuf, " DHCP_DECLINE");
              break;
            case SWS_DHCP_MSGTYPE_DHCPACK:
              pszBuf += sprintf( pszBuf, " DHCP_ACK");
              break;
            case SWS_DHCP_MSGTYPE_DHCPNAK:
              pszBuf += sprintf( pszBuf, " DHCP_NAK");
              break;
            case SWS_DHCP_MSGTYPE_DHCPRELEASE:
              pszBuf += sprintf( pszBuf, " DHCP_RELEASE");
              break;
            default:
              pszBuf += sprintf( pszBuf, " DHCP_MSG_TYPE:%u", uVal);
              break;
              }
            break;

          case SWS_DHCP_OPT_SUBNET_MASK:
            SWS_PEEK8( len, buf, offset);
            ++offset;
            SWS_PEEK32( ulVal, buf, offset);
            pszBuf += sprintf( pszBuf, " SUBNET/%u", CountOnes( ulVal) );
            break;

          case SWS_DHCP_OPT_ROUTER:
            SWS_PEEK8( len, buf, offset);
            ++offset;
            pszBuf += sprintf( pszBuf, " ROUTER");
            for ( u = 0; u + 4 <= len; u += 4)
              {
              SWS_PEEK32( ulVal, buf, offset + u);
              pszBuf += sprintf( pszBuf, ":");
              pszBuf += ValToAddress( pszBuf, ulVal);
              }
            break;

          case SWS_DHCP_OPT_NAME_SERVER:
            SWS_PEEK8( len, buf, offset);
            ++offset;
            pszBuf += sprintf( pszBuf, " NAME_SERVER");
            for ( u = 0; u + 4 <= len; u += 4)
              {
              SWS_PEEK32( ulVal, buf, offset + u);
              pszBuf += sprintf( pszBuf, ":");
              pszBuf += ValToAddress( pszBuf, ulVal);
              }
            break;

          case SWS_DHCP_OPT_DNS:
            SWS_PEEK8( len, buf, offset);
            ++offset;
            pszBuf += sprintf( pszBuf, " DNS");
            for ( u = 0; u + 4 <= len; u += 4)
              {
              SWS_PEEK32( ulVal, buf, offset + u);
              pszBuf += sprintf( pszBuf, ":");
              pszBuf += ValToAddress( pszBuf, ulVal);
              }
            break;

          case SWS_DHCP_OPT_HOST_NAME:
            SWS_PEEK8( len, buf, offset);
            ++offset;
            pszBuf += sprintf( pszBuf, " HOST'%.*s'", (int)len, buf + offset);
            break;

          case SWS_DHCP_OPT_DOMAIN_NAME:
            SWS_PEEK8( len, buf, offset);
            ++offset;
            pszBuf += sprintf( pszBuf, " DOMAIN'%.*s'", (int)len, buf + offset);
            break;

          case SWS_DHCP_OPT_IP_TTL:
            SWS_PEEK8( len, buf, offset);
            ++offset;
            SWS_PEEK8( uVal, buf, offset);
            pszBuf += sprintf( pszBuf, " TTL:%u", uVal);
            break;

          case SWS_DHCP_OPT_MTU:
            SWS_PEEK8( len, buf, offset);
            ++offset;
            SWS_PEEK16( uVal, buf, offset);
            pszBuf += sprintf( pszBuf, " MTU:%u", uVal );
            break;

          case SWS_DHCP_OPT_IP_ADDRESS:
            SWS_PEEK8( len, buf, offset);
            ++offset;
            SWS_PEEK32( ulVal, buf, offset);
            pszBuf += sprintf( pszBuf, " IP:");
            pszBuf += ValToAddress( pszBuf, ulVal);
            break;

          case SWS_DHCP_OPT_LEASE_TIME:
            SWS_PEEK8( len, buf, offset);
            ++offset;
            SWS_PEEK32( ulVal, buf, offset);
            pszBuf += sprintf( pszBuf, " LEASE:");
            pszBuf += SecsToTime( pszBuf, ulVal);
            break;

          case SWS_DHCP_OPT_OVERLOAD:
            SWS_PEEK8( len, buf, offset);
            ++offset;
            SWS_PEEK8( uVal, buf, offset);
            pszBuf += sprintf( pszBuf, " OVERLOAD:%u", uVal );
            break;

          case SWS_DHCP_OPT_SERVERID:
            SWS_PEEK8( len, buf, offset);
            ++offset;
            SWS_PEEK32( ulVal, buf, offset);
            pszBuf += sprintf( pszBuf, " SERVER:");
            pszBuf += ValToAddress( pszBuf, ulVal);
            break;

          case SWS_DHCP_OPT_PARAMETERS:
            SWS_PEEK8( len, buf, offset);
            ++offset;
            pszBuf += sprintf( pszBuf, " WANT[");
            pszBuf += HexDump( pszBuf, hBuffer, uOffset + offset, len);
            pszBuf += sprintf( pszBuf, "]");
            break;

          case SWS_DHCP_OPT_MESSAGE:
            SWS_PEEK8( len, buf, offset);
            ++offset;
            pszBuf += sprintf( pszBuf, " MSG'%.*s'", (int)len, buf + offset);
            break;

          case SWS_DHCP_OPT_MAX_SIZE:
            SWS_PEEK8( len, buf, offset);
            ++offset;
            SWS_PEEK16( uVal, buf, offset);
            pszBuf += sprintf( pszBuf, " MAXMSG:%u", uVal );
            break;

          case SWS_DHCP_OPT_T1_TIME:
            SWS_PEEK8( len, buf, offset);
            ++offset;
            SWS_PEEK32( ulVal, buf, offset);
            pszBuf += sprintf( pszBuf, " RENEW:" );
            pszBuf += SecsToTime( pszBuf, ulVal);
            break;

          case SWS_DHCP_OPT_T2_TIME:
            SWS_PEEK8( len, buf, offset);
            ++offset;
            SWS_PEEK32( ulVal, buf, offset);
            pszBuf += sprintf( pszBuf, " REBIND:" );
            pszBuf += SecsToTime( pszBuf, ulVal);
            break;

          case SWS_DHCP_OPT_CLASSID:
            SWS_PEEK8( len, buf, offset);
            ++offset;
            pszBuf += sprintf( pszBuf, " CLASS'%.*s'", (int)len, buf + offset);
            break;

          case SWS_DHCP_OPT_CLIENTID:
            SWS_PEEK8( len, buf, offset);
            ++offset;
            SWS_PEEK8( uVal, buf, offset);
            pszBuf += sprintf( pszBuf, " CID:%u [", uVal);
            pszBuf += HexDump( pszBuf, hBuffer, uOffset + offset + 1, len - 1);
            pszBuf += sprintf( pszBuf, "]");
            break;

          case SWS_DHCP_OPT_TOFFSET:
            SWS_PEEK8( len, buf, offset);
            ++offset;
            SWS_PEEK32( lVal, buf, offset);
            pszBuf += sprintf( pszBuf, " TOFF:");
            if ( lVal < 0)
              {
              pszBuf += sprintf( pszBuf, "-");
              ulVal = (ui32)-lVal;
              }
            else
              {
              ulVal = (ui32)lVal;
              }
            pszBuf += SecsToTime( pszBuf, ulVal);
            break;

          case SWS_DHCP_OPT_TIME_SERVER:
            SWS_PEEK8( len, buf, offset);
            ++offset;
            pszBuf += sprintf( pszBuf, " TIME_SERVER");
            for ( u = 0; u + 4 <= len; u += 4)
              {
              SWS_PEEK32( ulVal, buf, offset + u);
              pszBuf += sprintf( pszBuf, ":");
              pszBuf += ValToAddress( pszBuf, ulVal);
              }
            break;

          default:
            SWS_PEEK8( len, buf, offset);
            ++offset;
            pszBuf += sprintf( pszBuf, " OPT:%u [", uVal);
            pszBuf += HexDump( pszBuf, hBuffer, uOffset + offset, len);
            pszBuf += sprintf( pszBuf, "]");
            break;
            }

          offset += len;
          }
        while ( offset < seglen);
        }
      }
    }

  return (int)(pszBuf - pszStart);
  }

#endif


#if OPT_DUMP_NETBIOS || OPT_DUMP_DNS
/****************************************************************************
 * DNS
 ****************************************************************************/
/*
 * DNS packet dump (RFC1035 (STD13))
 */
static int IpDumpDns(
  char* pszBuf,                       /* OUT: text */
  SWS_BufferH hBuffer,                /* IN: data */
  unsigned uOffset,
  unsigned seglen,
  int netbios
) {
  SWS_PTR( void) pvIpHdr;
  SWS_size_t uLen;
  char* const pszStart = pszBuf;

  pvIpHdr = SWS_BufferGetFragment( hBuffer, uOffset, &uLen);

  if ( NULL == pvIpHdr || uLen < seglen)
    pszBuf += sprintf( pszBuf, " Truncated header");
  else if ( seglen < SWS_DNS_HDR_LEN)
    pszBuf += sprintf( pszBuf, " Invalid header");
  else
    {
    unsigned qr, opcode, qdcount, ancount, nscount, arcount;
    unsigned len, offset;

    SWS_PEEK16( qr, pvIpHdr, SWS_DNS_HDR_OP);

    if ( !(SWS_DNS_QR_MASK & qr))
      {
      opcode = qr & SWS_DNS_OPCODE_MASK;
      switch ( opcode)
        {
      case SWS_DNS_OP_QUERY:
        pszBuf += sprintf( pszBuf, " QUERY");
        break;
      case SWS_DNS_OP_IQUERY:
        pszBuf += sprintf( pszBuf, " IQUERY");
        break;
      case SWS_DNS_OP_CQUERYM:
        pszBuf += sprintf( pszBuf, " CQUERYM");
        break;
      case SWS_DNS_OP_CQUERYU:
        pszBuf += sprintf( pszBuf, " CQUERYU");
        break;
      case SWS_DNS_OP_REGIST:
        pszBuf += sprintf( pszBuf, " REGIST");
        break;
      case SWS_DNS_OP_RELEASE:
        pszBuf += sprintf( pszBuf, " RELEASE");
        break;
      case SWS_DNS_OP_WACK:
        pszBuf += sprintf( pszBuf, " WACK");
        break;
      case SWS_DNS_OP_REFRESH:
        pszBuf += sprintf( pszBuf, " REFRESH");
        break;
      default:
        pszBuf += sprintf( pszBuf, " QUERY:%#x", opcode);
        break;
        }
      }
    else
      {
      opcode = qr & SWS_DNS_RCODE_MASK;
      switch ( opcode)
        {
      case SWS_DNS_RC_GOOD:
        pszBuf += sprintf( pszBuf, " REPLY");
        break;
      case SWS_DNS_RC_FORMAT:
        pszBuf += sprintf( pszBuf, " RC_FORMAT");
        break;
      case SWS_DNS_RC_SFAIL:
        pszBuf += sprintf( pszBuf, " RC_SFAIL");
        break;
      case SWS_DNS_RC_NAMERR:
        pszBuf += sprintf( pszBuf, " RC_NAMERR");
        break;
      case SWS_DNS_RC_NOTIMP:
        pszBuf += sprintf( pszBuf, " RC_NOTIMP");
        break;
      case SWS_DNS_RC_REFUSED:
        pszBuf += sprintf( pszBuf, " RC_REFUSED");
        break;
      default:
        pszBuf += sprintf( pszBuf, " REPLY:%u", opcode);
        break;
        }
      }

    if ( s_iIpVerbosity >= 2)
      {
      unsigned id;
      SWS_PEEK16( id, pvIpHdr, SWS_DNS_HDR_ID);
      pszBuf += sprintf( pszBuf, " ID:%u", id);

      if ( SWS_DNS_AA_MASK & qr)
        pszBuf += sprintf( pszBuf, " AA"); /* Authoritative Answer */
      if ( SWS_DNS_RD_MASK & qr)
        pszBuf += sprintf( pszBuf, " RD"); /* Recursion Desired */
      if ( SWS_DNS_RA_MASK & qr)
        pszBuf += sprintf( pszBuf, " RA"); /* Recursion Available */
      if ( SWS_DNS_BB_MASK & qr)
        pszBuf += sprintf( pszBuf, " BB"); /* NetBIOS Broadcast */
      }

    if ( SWS_DNS_TC_MASK & qr)
      pszBuf += sprintf( pszBuf, " TC"); /* Truncation Flag */

    SWS_PEEK16( qdcount, pvIpHdr, SWS_DNS_HDR_QDCOUNT);
    SWS_PEEK16( ancount, pvIpHdr, SWS_DNS_HDR_ANCOUNT);
    SWS_PEEK16( nscount, pvIpHdr, SWS_DNS_HDR_NSCOUNT);
    SWS_PEEK16( arcount, pvIpHdr, SWS_DNS_HDR_ARCOUNT);
    offset = SWS_DNS_HDR_LEN;

    /* QUESTION SECTION */
    while ( qdcount-- > 0)
      {
      pszBuf += sprintf( pszBuf, " [");

      /* QUESTION_NAME */
      offset = IpDumpName( pszBuf, pvIpHdr, offset, netbios, uLen);
      pszBuf += strlen( pszBuf);

      /* QUESTION_TYPE */
      SWS_PEEK16( len, pvIpHdr, SWS_DNS_QTYPE + offset);
      switch ( len)
        {
      default:
        pszBuf += sprintf( pszBuf, " QTYPE:%u", len);
        break;
      case SWS_DNS_TYPE_A:
        pszBuf += sprintf( pszBuf, " A");
        break;
      case SWS_DNS_TYPE_NS:
        pszBuf += sprintf( pszBuf, " NS");
        break;
      case SWS_DNS_TYPE_CNAME:
        pszBuf += sprintf( pszBuf, " CNAME");
        break;
      case SWS_DNS_TYPE_PTR:
        pszBuf += sprintf( pszBuf, " PTR");
        break;
      case SWS_DNS_QTYPE_NB:
        pszBuf += sprintf( pszBuf, " NB"); /* NetBIOS general Name Service */
        break;
      case SWS_DNS_QTYPE_NBSTAT:
        pszBuf += sprintf( pszBuf, " NBSTAT"); /* NODE STATUS request */
        break;
      case SWS_DNS_QTYPE_STAR:
        pszBuf += sprintf( pszBuf, " QTYPE*");
        break;
        }

      /* QUESTION_CLASS */
      SWS_PEEK16( len, pvIpHdr, SWS_DNS_QCLASS + offset);
      switch ( len)
        {
      default:
        pszBuf += sprintf( pszBuf, " QCLASS:%u", len);
        break;
      case SWS_DNS_CLASS_IN:
        break;
      case SWS_DNS_CLASS_CH:
        pszBuf += sprintf( pszBuf, " CHAOS");
        break;
      case SWS_DNS_QCLASS_STAR:
        pszBuf += sprintf( pszBuf, " QCLASS*");
        break;
        }

      offset += SWS_DNS_QLEN;
      pszBuf += sprintf( pszBuf, "]");
      }

    while ( ancount-- > 0)
      {
      pszBuf += sprintf( pszBuf, " ANS[");
      offset = IpDumpResourceRecord( pszBuf, hBuffer, uOffset, offset, netbios);
      pszBuf += strlen( pszBuf);
      pszBuf += sprintf( pszBuf, "]");
      }

    if ( s_iIpVerbosity < 2)
      {
      if ( nscount > 0)
        pszBuf += sprintf( pszBuf, " AUTHS:%u", nscount);
      if ( arcount > 0)
        pszBuf += sprintf( pszBuf, " ADDS:%u", arcount);
      }
    else
      {
      while ( nscount-- > 0)
        {
        pszBuf += sprintf( pszBuf, " AUTH[");
        offset = IpDumpResourceRecord( pszBuf, hBuffer, uOffset, offset, netbios);
        pszBuf += strlen( pszBuf);
        pszBuf += sprintf( pszBuf, "]");
        }

      while ( arcount-- > 0)
        {
        pszBuf += sprintf( pszBuf, " ADD[");
        offset = IpDumpResourceRecord( pszBuf, hBuffer, uOffset, offset, netbios);
        pszBuf += strlen( pszBuf);
        pszBuf += sprintf( pszBuf, "]");
        }
      }
    }

  return (int)(pszBuf - pszStart);
  }


/*
 * Dump a DNS Resource Record
 */
static unsigned IpDumpResourceRecord(
  char* pszBuf,                       /* OUT: text */
  SWS_BufferH hBuffer,                /* IN: data */
  unsigned uBufOffset,
  unsigned offset,
  int netbios
) {
  unsigned u, len;
  ui32 ulVal;
  const void* pvRec;
  SWS_size_t uLen;

  pvRec = SWS_BufferGetFragment( hBuffer, uBufOffset, &uLen);

  /* Name */
  if ( s_iIpVerbosity >= 3 || netbios)
    {
    offset = IpDumpName( pszBuf, pvRec, offset, netbios, uLen);
    strcat( pszBuf, " ");
    pszBuf += strlen( pszBuf);
    }
  else
    {
    offset = IpDumpName( NULL, pvRec, offset, netbios, uLen);
    }

  /* RDATA length */
  SWS_PEEK16( len, pvRec, SWS_DNS_RR_RDLEN + offset);

  /* Type */
  SWS_PEEK16( u, pvRec, SWS_DNS_RR_TYPE + offset);
  switch ( u)
    {
  case SWS_DNS_TYPE_A:
    SWS_PEEK32( ulVal, pvRec, SWS_DNS_RR_RDATA + offset);
    pszBuf += ValToAddress( pszBuf, ulVal);
    break;

  case SWS_DNS_TYPE_NS:
    pszBuf += sprintf( pszBuf, "NS ");
    IpDumpName( pszBuf, pvRec, offset + SWS_DNS_RR_RDATA, netbios, uLen);
    pszBuf += strlen( pszBuf);
    break;
  case SWS_DNS_TYPE_CNAME:
    pszBuf += sprintf( pszBuf, "CNAME ");
    IpDumpName( pszBuf, pvRec, offset + SWS_DNS_RR_RDATA, netbios, uLen);
    pszBuf += strlen( pszBuf);
    break;
  case SWS_DNS_TYPE_PTR:
    pszBuf += sprintf( pszBuf, "PTR ");
    IpDumpName( pszBuf, pvRec, offset + SWS_DNS_RR_RDATA, netbios, uLen);
    pszBuf += strlen( pszBuf);
    break;

  case SWS_DNS_TYPE_SOA:
    pszBuf += sprintf( pszBuf, "SOA ");
    /* mname */
    u = IpDumpName( pszBuf, pvRec, offset + SWS_DNS_RR_RDATA, netbios, uLen);
    pszBuf += strlen( pszBuf);
    /* rname */
    *(pszBuf++) = ' ';
    u = IpDumpName( pszBuf, pvRec, u, netbios, uLen);
    pszBuf += strlen( pszBuf);
    if ( s_iIpVerbosity >= 3)
      {
      SWS_PEEK32( ulVal, pvRec, 0 + u);
      pszBuf += sprintf( pszBuf, " SERIAL:%lu", ulVal);
      SWS_PEEK32( ulVal, pvRec, 4 + u);
      pszBuf += sprintf( pszBuf, " RFRSH:%lu", ulVal);
      SWS_PEEK32( ulVal, pvRec, 8 + u);
      pszBuf += sprintf( pszBuf, " RTRY:%lu", ulVal);
      SWS_PEEK32( ulVal, pvRec, 12 + u);
      pszBuf += sprintf( pszBuf, " EXP:");
      pszBuf += SecsToTime( pszBuf, ulVal);
      SWS_PEEK32( ulVal, pvRec, 16 + u);
      pszBuf += sprintf( pszBuf, " MIN:%lu", ulVal);
      }
    break;

  case SWS_DNS_QTYPE_NB:
    pszBuf += sprintf( pszBuf, "NB");
    if ( s_iIpVerbosity >= 3)
      pszBuf += HexDump( pszBuf, hBuffer, uBufOffset + offset + SWS_DNS_RR_RDATA, len);
    break;

  case SWS_DNS_QTYPE_NBSTAT:
    pszBuf += sprintf( pszBuf, "NBSTAT");
    if ( s_iIpVerbosity >= 3)
      pszBuf += HexDump( pszBuf, hBuffer, uBufOffset + offset + SWS_DNS_RR_RDATA, len);
    break;

  default:
    pszBuf += sprintf( pszBuf, "TYPE:%u", u);
    if ( s_iIpVerbosity >= 3)
      pszBuf += HexDump( pszBuf, hBuffer, uBufOffset + offset + SWS_DNS_RR_RDATA, len);
    break;
    }

  /* Class */
  SWS_PEEK16( u, pvRec, SWS_DNS_RR_CLASS + offset);
  switch ( u)
    {
  default:
    pszBuf += sprintf( pszBuf, " CLASS:%u", u);
    break;
  case SWS_DNS_CLASS_IN:
    break;
    }

  if ( s_iIpVerbosity >= 3)
    {
    SWS_PEEK32( ulVal, pvRec, SWS_DNS_RR_TTL + offset);
    pszBuf += sprintf( pszBuf, " TTL:");
    pszBuf += SecsToTime( pszBuf, ulVal);
    }

  offset += len + SWS_DNS_RR_RDATA;
  return offset;
  }


/*
 * Dump a DNS name
 */
static unsigned IpDumpName(
  char* pszBuf,                       /* OUT: text */
  const void* pv,
  unsigned offset,
  int netbios,
  SWS_size_t maxLen
) {
  unsigned len;

  if ( NULL != pszBuf)
    *pszBuf = '\0';

  if ( offset >= maxLen)
    return maxLen;

  SWS_PEEK8( len, pv, offset);

  while ( len > 0)
    {
    if ( len < 64) /* Top 2 bits indicate ptr */
      {
      /* Length byte */
      const char* pszName = (const char*)pv + offset + 1;
      offset += 1 + len;

      if ( NULL == pszBuf)
        ;
      else if ( netbios)
        {
        /* NetBIOS name unmangling */
        while ( len >= 2)
          {
          int c;

          len -= 2;

          c = (*(pszName++) - 'A') << 4;
          c |= *(pszName++) - 'A';
          if ( 0 == c)
            break;
          if ( isprint( c))
            *(pszBuf++) = (char)c;
          else
            pszBuf += sprintf( pszBuf, "<%02X>", c);
          }

        *pszBuf = '\0';
        }
      else
        {
        pszBuf += sprintf( pszBuf, "%.*s", (int)len, pszName);
        }

      SWS_PEEK8( len, pv, offset);
      if ( len > 0 && NULL != pszBuf)
        *(pszBuf++) = '.';
      }
    else
      {
      /* pointer */
      if ( NULL != pszBuf)
        {
        SWS_PEEK16( len, pv, offset);
        len &= 0x3fff;
        IpDumpName( pszBuf, pv, len, netbios, maxLen);
        }
      return offset + 2;
      }
    }

  return offset + 1;
  }

#endif /* OPT_DUMP_NETBIOS || OPT_DUMP_DNS */


#if OPT_DUMP_NETBIOS
/****************************************************************************
 * NetBIOS
 ****************************************************************************/

#define NB_NAMELEN 34 /* Count byte, 32 chars plus nul */
#define NBSS_PREFIX 4 /* Size of NetBIOS session service header */

/*
 * Netbios session service packet dump (STD19,p29)
 * SMB decode: ftp://ftp.microsoft.com/developr/drg/CIFS/smbpub.zip (doc)
 */
static int IpDumpNetbiosSessionService(
  char* pszBuf,                       /* OUT: text */
  SWS_BufferH hBuffer,                /* IN: data */
  unsigned uOffset,
  unsigned seglen
) {
  SWS_PTR( ui8) pHdr;
  SWS_size_t uLen;
  char* const pszStart = pszBuf;
  static unsigned long s_inMessage;

  pHdr = (SWS_PTR( ui8))SWS_BufferGetFragment( hBuffer, uOffset, &uLen);

  if ( NULL == pHdr || uLen < seglen)
    {
    pszBuf += sprintf( pszBuf, " Incomplete record");
    }
  /* NB Decoding session requests can be error prone because
    * the start of a packet does not necessarily correspond to the
    * start of a request.  Per connection state must be maintained
    * for proper decoding.
    */
  else if ( s_inMessage)
    {
    /* More data */
    if ( s_inMessage <= seglen)
      s_inMessage -= seglen;
    else
      s_inMessage = 0;

    pszBuf += sprintf( pszBuf, " DATA");
    }
  else if ( seglen < NBSS_PREFIX)
    {
    pszBuf += sprintf( pszBuf, " Truncated header");
    }
  else
    {
    unsigned type, offset;
    unsigned long len;

    SWS_PEEK32( len, pHdr, 0); /* type:8 flags:7 len:17 */
    if ( len & 0x00fe0000L) /* flags should be 0 */
      pszBuf += sprintf( pszBuf, " FLAGS:%#x", (unsigned)(len >> 16) & 0xfe);

    type = (unsigned)(len >> 24);
    len &= 0x1ffffL;
    if ( len + NBSS_PREFIX != seglen)
      pszBuf += sprintf( pszBuf, " LEN:%lu",len);

    pHdr += NBSS_PREFIX;

    switch ( type)
      {
    case SWS_NBSS_MESSAGE:
      pszBuf += sprintf( pszBuf, " MSG");

      /* Verify SMB header */
      if ( len > SWS_SMB_WCT
        && SWS_SMB_ISVALID( pHdr)
      ) {
        unsigned long ulVal;
        unsigned plen;
        const char* psz;

        pszBuf += sprintf( pszBuf, " SMB");
        if ( pHdr[ SWS_SMB_FLG] & SWS_SMB_FLG_REPLY)
          pszBuf += sprintf( pszBuf, " reply");

        /* Decode any error status */
        switch ( pHdr[ SWS_SMB_RCLS])
          {
        case SWS_SMB_RCLS_SUCCESS:
          break;
        case SWS_SMB_RCLS_ERRDOS:
          pszBuf += sprintf( pszBuf, " OSERR:%u",
            pHdr[ SWS_SMB_ERR] + 256U * pHdr[ SWS_SMB_ERR + 1] );
          break;
        case SWS_SMB_RCLS_ERRSRV:
          pszBuf += sprintf( pszBuf, " SVRERR:%u",
            pHdr[ SWS_SMB_ERR] + 256U * pHdr[ SWS_SMB_ERR + 1] );
          break;
        case SWS_SMB_RCLS_ERRHRD:
          pszBuf += sprintf( pszBuf, " HWERR:%u",
            pHdr[ SWS_SMB_ERR] + 256U * pHdr[ SWS_SMB_ERR + 1] );
          break;
        case SWS_SMB_RCLS_ERRCMD:
          pszBuf += sprintf( pszBuf, " CMDERR:%u",
            pHdr[ SWS_SMB_ERR] + 256U * pHdr[ SWS_SMB_ERR + 1] );
          break;
        default:
          pszBuf += sprintf( pszBuf, " RCLS:%u ERR:%u",
            pHdr[ SWS_SMB_RCLS], pHdr[ SWS_SMB_ERR] + 256U * pHdr[ SWS_SMB_ERR + 1] );
          break;
          }

        pszBuf += sprintf( pszBuf, " UID:%u",
          pHdr[ SWS_SMB_UID] + 256U * pHdr[ SWS_SMB_UID + 1]
        );
        pszBuf += sprintf( pszBuf, " TID:%u",
          pHdr[ SWS_SMB_TID] + 256U * pHdr[ SWS_SMB_TID + 1]
        );

        if ( s_iIpVerbosity >= 3)
          {
          pszBuf += sprintf( pszBuf, " PID:%u",
            pHdr[ SWS_SMB_PID] + 256U * pHdr[ SWS_SMB_PID + 1]
          );
          pszBuf += sprintf( pszBuf, " MID:%u",
            pHdr[ SWS_SMB_MID] + 256U * pHdr[ SWS_SMB_MID + 1]
          );
          pszBuf += sprintf( pszBuf, " FLG:%#x FLG2:%#x",
            pHdr[ SWS_SMB_FLG],
            pHdr[ SWS_SMB_FLG2] + 256U * pHdr[ SWS_SMB_FLG2 + 1]
          );
          }

        offset = SWS_SMB_BCC( pHdr);
        pszBuf += sprintf( pszBuf, " WC:%u BC:%u",
          pHdr[ SWS_SMB_WCT], pHdr[ offset] + 256U * pHdr[ offset + 1] );

        /* Decode command */
        switch ( pHdr[ SWS_SMB_COM] )
          {
        case SWS_SMBnegprot:
          pszBuf += sprintf( pszBuf, " NEGOTIATE_PROTOCOL");

          if ( pHdr[ SWS_SMB_FLG] & SWS_SMB_FLG_REPLY)
            {
            /* Chosen protocol indx is 1st word param */
            if ( 1 > pHdr[ SWS_SMB_WCT] )
              break;

            SWS_SMB_GET_WPARAM( offset, pHdr, 0);
            pszBuf += sprintf( pszBuf, " %u", offset);
            }
          else
            {
            /* Display protocols */
            psz = (const char*)&pHdr[ SWS_SMB_BUF( pHdr)];

            offset = SWS_SMB_BCC( pHdr);
            plen = pHdr[ offset] + 256U * pHdr[ offset + 1];
            if ( plen > 1024)
              plen = 1024;

            while ( plen)
              {
              unsigned cnt;

              /* Check data type */
              --plen;
              if ( SWS_SMB_FORMAT_DIALECT != *psz++)
                break;

              pszBuf += sprintf( pszBuf, " '%.*s'", (int)plen, psz);

              cnt = 1 + strlen( psz);
              psz += cnt;
              if ( plen >= cnt)
                plen -= cnt;
              else
                plen = 0;
              }
            }
          break;

        case SWS_SMBsesssetupX:
          pszBuf += sprintf( pszBuf, " SESSION_SETUP_ANDX");
          if ( pHdr[ SWS_SMB_FLG] & SWS_SMB_FLG_REPLY)
            {
            /* 3 or 4 word params: AndXCmd, AndXOffset & option bits */
            if ( 2 > pHdr[ SWS_SMB_WCT] )
              break;

            SWS_SMB_GET_WPARAM( offset, pHdr, 0);
            if ( SWS_SMBnoX != offset)
              {
              pszBuf += sprintf( pszBuf, " X:%#x", offset);

              SWS_SMB_GET_WPARAM( offset, pHdr, 1);
              pszBuf += sprintf( pszBuf, " XOff:%u", offset);
              }

            if ( pHdr[ SWS_SMB_WCT] >= 3)
              {
              SWS_SMB_GET_WPARAM( offset, pHdr, 2);
              if ( offset & 1)
                pszBuf += sprintf( pszBuf, " GUEST");
              if ( offset & ~1)
                pszBuf += sprintf( pszBuf, " FLAGS:%04x", offset);
              }

            offset = SWS_SMB_BCC( pHdr);
            plen = SWS_SMB_GET_WORD( pHdr, offset);
            psz = (const char*)&pHdr[ SWS_SMB_BUF( pHdr)];

            if ( pHdr[ SWS_SMB_WCT] >= 4)
              {
              /* NT LM 0.12 has SecurityBlobLength */
              SWS_SMB_GET_WPARAM( offset, pHdr, 3);
              pszBuf += sprintf( pszBuf, " BlobLen:%u", offset);
              psz += offset;
              plen -= offset;
              }

            if ( plen > 1024)
              plen = 1024;

            /* Byte strings */
            offset = 0;
            while ( plen)
              {
              static const char* const kszType[] = {
                "OS",
                "LAN",
                "DOM",
              };
              unsigned cnt, step;

              pszBuf += sprintf( pszBuf, " %s'", offset < 3 ? kszType[ offset] : "");
              ++offset;

              /* NT LM 0.12 uses unicode */
              step = pHdr[ SWS_SMB_WCT] >= 4 ? 2 : 1;
              for ( cnt = 0; cnt < plen; cnt += step)
                {
                char c = psz[ cnt];
                if ( '\0' == c)
                  {
                  if ( 1 == step
                    && cnt + 2 < plen
                    && isprint( psz[ cnt +1])
                    && 0 == psz[ cnt + 2]
                  ) {
                    --cnt;
                    step = 2;
                    continue;
                    }
                  cnt += step;
                  break;
                  }
                *pszBuf++ = (char)(isprint( c) ? c : '?');
                }
              *pszBuf++ = '\'';
              *pszBuf = '\0';
              psz += cnt;

              if ( step == cnt && offset >= 3)
                break;
              else if ( plen >= cnt)
                plen -= cnt;
              else
                break;
              }
            }
          else
            {
            unsigned pwd;

            if ( 10 > pHdr[ SWS_SMB_WCT] )
              break;

            SWS_SMB_GET_WPARAM( offset, pHdr, 0);
            if ( SWS_SMBnoX != offset)
              {
              pszBuf += sprintf( pszBuf, " X:%#x", offset);

              SWS_SMB_GET_WPARAM( offset, pHdr, 1);
              pszBuf += sprintf( pszBuf, " XOff:%u", offset);
              }

            SWS_SMB_GET_WPARAM( offset, pHdr, 2);
            pszBuf += sprintf( pszBuf, " MaxBuf:%u", offset);

            SWS_SMB_GET_WPARAM( offset, pHdr, 3);
            pszBuf += sprintf( pszBuf, " MaxMPX:%u", offset);

            SWS_SMB_GET_WPARAM( offset, pHdr, 4);
            pszBuf += sprintf( pszBuf, " VC:%u", offset);

            SWS_SMB_GET_WPARAM( offset, pHdr, 5);
            SWS_SMB_GET_WPARAM( ulVal, pHdr, 6);
            ulVal = (ulVal << 16) + offset;
            pszBuf += sprintf( pszBuf, " KEY:%08lx", ulVal);

            offset = SWS_SMB_BCC( pHdr);
            plen = SWS_SMB_GET_WORD( pHdr, offset);
            psz = (const char*)&pHdr[ SWS_SMB_BUF( pHdr)];

            SWS_SMB_GET_WPARAM( pwd, pHdr, 7); /* Password len */
            pszBuf += sprintf( pszBuf, " PWDLEN:%u", pwd);
            plen -= pwd;
            psz += pwd;

            if ( pHdr[ SWS_SMB_WCT] >= 13)
              {
              /* NT LM 0.12 */
              SWS_SMB_GET_WPARAM( pwd, pHdr, 8); /* Case sens password len */
              pszBuf += sprintf( pszBuf, " CSPWDLEN:%u", pwd);
              plen -= pwd;
              psz += pwd;
              offset = 0;
              }
           else  if ( pHdr[ SWS_SMB_WCT] >= 12)
              {
              /* NT LM 0.12 with SecurityBlobLength */
              offset = 2; /* No user or domain name */
              }
            else
              offset = 0;

            if ( plen > 1024)
              plen = 1024;

            /* Username etc */
            while ( plen)
              {
              static const char* const kszType[] = {
                "USER",
                "DOM",
                "OS",
                "LAN"
              };
              unsigned cnt, step;

              pszBuf += sprintf( pszBuf, " %s'", offset < 4 ? kszType[ offset] : "");
              ++offset;

              /* NT LM 0.12 uses unicode */
              step = pHdr[ SWS_SMB_WCT] >= 12 ? 2 : 1;
              for ( cnt = 0; cnt < plen; cnt += step)
                {
                char c = psz[ cnt];
                if ( '\0' == c)
                  {
                  if ( 1 == step
                    && cnt + 2 < plen
                    && isprint( psz[ cnt +1])
                    && 0 == psz[ cnt + 2]
                  ) {
                    --cnt;
                    step = 2;
                    continue;
                    }
                  cnt += step;
                  break;
                  }
                *pszBuf++ = (char)(isprint( c) ? c : '?');
                }
              *pszBuf++ = '\'';
              *pszBuf = '\0';
              psz += cnt;

              if ( step == cnt && offset >= 4)
                break;
              else if ( plen >= cnt)
                plen -= cnt;
              else
                break;
              }
            }
          break;

        case SWS_SMBtcon:
          pszBuf += sprintf( pszBuf, " TREE_CONNECT");

          if ( pHdr[ SWS_SMB_FLG] & SWS_SMB_FLG_REPLY)
            {
            /* 2 word params: MaxXmit, TID */
            if ( 2 > pHdr[ SWS_SMB_WCT] )
              break;

            SWS_SMB_GET_WPARAM( offset, pHdr, 0);
            pszBuf += sprintf( pszBuf, " MaxXmit:%u", offset);

            SWS_SMB_GET_WPARAM( offset, pHdr, 1);
            pszBuf += sprintf( pszBuf, " TID:%u", offset);
            }
          else
            {
            offset = 1 + SWS_SMB_BUF( pHdr);
            pszBuf += sprintf( pszBuf, " share'%s'", (char*)pHdr + offset);

            offset += 2 + strlen( (char*)pHdr + offset);
            pszBuf += sprintf( pszBuf, " pwd'%s'", (char*)pHdr + offset);

            offset += 2 + strlen( (char*)pHdr + offset);
            pszBuf += sprintf( pszBuf, " service'%s'", (char*)pHdr + offset);
            }
          break;

        case SWS_SMBtconX:
          pszBuf += sprintf( pszBuf, " TREE_CONNECT_ANDX");

          if ( pHdr[ SWS_SMB_FLG] & SWS_SMB_FLG_REPLY)
            {
            /* 2 word params: AndXCmd, AndXOffset */
            if ( 2 > pHdr[ SWS_SMB_WCT] )
              break;

            SWS_SMB_GET_WPARAM( offset, pHdr, 0);
            if ( SWS_SMBnoX != offset)
              {
              pszBuf += sprintf( pszBuf, " X:%#x", offset);

              SWS_SMB_GET_WPARAM( offset, pHdr, 1);
              pszBuf += sprintf( pszBuf, " XOff:%u", offset);
              }
            }
          else
            {
            /* 4 word params: AndXCmd, AndXOffset, Flags, Password len */
            if ( 4 > pHdr[ SWS_SMB_WCT] )
              break;

            SWS_SMB_GET_WPARAM( offset, pHdr, 0);
            if ( SWS_SMBnoX != offset)
              {
              SWS_SMB_GET_WPARAM( offset, pHdr, 0);
              pszBuf += sprintf( pszBuf, " X:%#x", offset);

              SWS_SMB_GET_WPARAM( offset, pHdr, 1);
              pszBuf += sprintf( pszBuf, " XOff:%u", offset);
              }

            SWS_SMB_GET_WPARAM( offset, pHdr, 2);
            pszBuf += sprintf( pszBuf, " Flags:%#x", offset);

            /* Get password len */
            SWS_SMB_GET_WPARAM( plen, pHdr, 3);

            offset = SWS_SMB_BUF( pHdr);
            pszBuf += sprintf( pszBuf, " pwd'%.*s'", (int)plen, (char*)pHdr + offset);

            offset += plen;
            pszBuf += sprintf( pszBuf, " share'%s'", (char*)pHdr + offset);

            offset += 1 + strlen( (char*)pHdr + offset);
            pszBuf += sprintf( pszBuf, " service'%s'", (char*)pHdr + offset);
            }
          break;

        case SWS_SMBtrans:
          pszBuf += sprintf( pszBuf, " TRANSACTION");

          if ( pHdr[ SWS_SMB_FLG] & SWS_SMB_FLG_REPLY)
            {
            if ( 10 > pHdr[ SWS_SMB_WCT] )
              break;

            SWS_SMB_GET_WPARAM( offset, pHdr, 0);
            pszBuf += sprintf( pszBuf, " TPCNT:%u", offset);

            SWS_SMB_GET_WPARAM( offset, pHdr, 1);
            pszBuf += sprintf( pszBuf, " TDCNT:%u", offset);

            SWS_SMB_GET_WPARAM( offset, pHdr, 9);
            pszBuf += sprintf( pszBuf, " SUCNT:%u", offset);

            /* Display parameters */
            SWS_SMB_GET_WPARAM( plen, pHdr, 3);
            if ( plen)
              {
              SWS_SMB_GET_WPARAM( offset, pHdr, 4);
              offset += NBSS_PREFIX;
              pszBuf += sprintf( pszBuf, " PRMS:");
              pszBuf += HexDump( pszBuf, hBuffer, uOffset + offset, plen);
              }

            SWS_SMB_GET_WPARAM( offset, pHdr, 6);
            pszBuf += sprintf( pszBuf, " DATALEN:%u", offset);
            }
          else
            {
            if ( 14 > pHdr[ SWS_SMB_WCT] )
              break;

            SWS_SMB_GET_WPARAM( offset, pHdr, 0);
            pszBuf += sprintf( pszBuf, " TPCNT:%u", offset);

            SWS_SMB_GET_WPARAM( offset, pHdr, 1);
            pszBuf += sprintf( pszBuf, " TDCNT:%u", offset);

            SWS_SMB_GET_WPARAM( offset, pHdr, 2);
            pszBuf += sprintf( pszBuf, " MPRCNT:%u", offset);

            SWS_SMB_GET_WPARAM( offset, pHdr, 3);
            pszBuf += sprintf( pszBuf, " MDRCNT:%u", offset);

            SWS_SMB_GET_WPARAM( offset, pHdr, 4);
            pszBuf += sprintf( pszBuf, " MSUCNT:%u", offset);

            SWS_SMB_GET_WPARAM( offset, pHdr, 5);
            pszBuf += sprintf( pszBuf, " FLAGS:%04x", offset);

            SWS_SMB_GET_WPARAM( offset, pHdr, 6);
            SWS_SMB_GET_WPARAM( ulVal, pHdr, 7);
            ulVal = (ulVal << 16) + offset;
            pszBuf += sprintf( pszBuf, " WAIT:%lu", ulVal);

            SWS_SMB_GET_WPARAM( offset, pHdr, 13);
            pszBuf += sprintf( pszBuf, " SUCNT:%u", offset);

            /* Get byte count */
            offset = SWS_SMB_BCC( pHdr);
            plen = pHdr[ offset] + 256U * pHdr[ offset + 1];

            /* Display filename */
            offset += 2;
            pszBuf += sprintf( pszBuf, " NAME'%.*s'", (int)plen, &pHdr[ offset]);

            /* Display parameters */
            SWS_SMB_GET_WPARAM( plen, pHdr, 9);
            if ( plen)
              {
              if ( s_iIpVerbosity < 3)
                pszBuf += sprintf( pszBuf, " PCNT:%u", plen);
              else
                {
                SWS_SMB_GET_WPARAM( offset, pHdr, 10);
                offset += NBSS_PREFIX;
                pszBuf += sprintf( pszBuf, " PRMS:");
                pszBuf += HexDump( pszBuf, hBuffer, uOffset + offset, plen);
                }
              }

            /* Display data */
            SWS_SMB_GET_WPARAM( offset, pHdr, 11);
            if ( offset)
              pszBuf += sprintf( pszBuf, " DCNT:%u", offset);
            }
          break;

        case SWS_SMBcreate:
          pszBuf += sprintf( pszBuf, " CREATE");

          if ( pHdr[ SWS_SMB_FLG] & SWS_SMB_FLG_REPLY)
            {
            /* 1 word params: File handle */
            if ( 1 > pHdr[ SWS_SMB_WCT] )
              break;

            SWS_SMB_GET_WPARAM( offset, pHdr, 0);
            pszBuf += sprintf( pszBuf, " hndl:%u", offset);
            }
          else
            {
            /* 3 word params: File attribs, time, date */
            if ( 3 > pHdr[ SWS_SMB_WCT] )
              break;

            SWS_SMB_GET_WPARAM( offset, pHdr, 0);
            pszBuf += sprintf( pszBuf, " attr:%#x", offset);

            SWS_SMB_GET_WPARAM( offset, pHdr, 1);
            pszBuf += sprintf( pszBuf, " time:%u", offset);

            SWS_SMB_GET_WPARAM( offset, pHdr, 2);
            pszBuf += sprintf( pszBuf, " date:%u", offset);

            /* Buffer format 4 ascii */
            offset = SWS_SMB_BUF( pHdr);
            pszBuf += sprintf( pszBuf, " %s", (char*)pHdr + offset + 1);
            }
          break;

        case SWS_SMBntcreateX:
          pszBuf += sprintf( pszBuf, " NTCREATE_ANDX");

          if ( pHdr[ SWS_SMB_FLG] & SWS_SMB_FLG_REPLY)
            {
            /* 26 word params */
            if ( 26 > pHdr[ SWS_SMB_WCT] )
              break;

            offset = SWS_SMB_GET_WORD( pHdr, SWS_SMB_BYT( 5));
            pszBuf += sprintf( pszBuf, " hndl:%u", offset);
            }
          else
            {
            /* 24 word params */
            if ( 24 > pHdr[ SWS_SMB_WCT] )
              break;

            /* Name. NB this can be unicode */
            plen = SWS_SMB_GET_WORD( pHdr, SWS_SMB_BYT( 5)); /* Name length */
            offset = SWS_SMB_BUF( pHdr);
            pszBuf += sprintf( pszBuf, " %.*s", (int)plen, (char*)pHdr + offset);
            }
          break;

        case SWS_SMBwrite:
        case SWS_SMBread:
          if ( SWS_SMBread == pHdr[ SWS_SMB_COM])
            pszBuf += sprintf( pszBuf, " READ");
          else
            pszBuf += sprintf( pszBuf, " WRITE");

          if ( pHdr[ SWS_SMB_FLG] & SWS_SMB_FLG_REPLY)
            {
            /* 1 word params: bytes read/written */
            if ( 1 > pHdr[ SWS_SMB_WCT] )
              break;

            SWS_SMB_GET_WPARAM( offset, pHdr, 0);
            pszBuf += sprintf( pszBuf, " cnt:%u", offset);
            }
          else
            {
            /* 5 word params: handle, bytes, offset(2), remaining */
            if ( 5 > pHdr[ SWS_SMB_WCT] )
              break;

            SWS_SMB_GET_WPARAM( offset, pHdr, 0);
            pszBuf += sprintf( pszBuf, " hndl:%u", offset);

            SWS_SMB_GET_WPARAM( offset, pHdr, 1);
            pszBuf += sprintf( pszBuf, " cnt:%u", offset);

            SWS_SMB_GET_WPARAM( offset, pHdr, 2);
            SWS_SMB_GET_WPARAM( ulVal, pHdr, 3);
            ulVal = (ulVal << 16) + offset;
            pszBuf += sprintf( pszBuf, " offset:%lu", ulVal);

            SWS_SMB_GET_WPARAM( offset, pHdr, 4);
            pszBuf += sprintf( pszBuf, " rem:%u", offset);

            /* Buffer format 1 */
            offset = SWS_SMB_BUF( pHdr);
            pszBuf += sprintf( pszBuf, " %u bytes",
              pHdr[ 1 + offset] + 256 * pHdr[ 2 + offset]);
            }
          break;

        case SWS_SMBclose:
          pszBuf += sprintf( pszBuf, " CLOSE");

          if ( pHdr[ SWS_SMB_FLG] & SWS_SMB_FLG_REPLY)
            {
            /* 0 word params */
            }
          else
            {
            /* 3 word params: File handle, time, date */
            if ( 3 > pHdr[ SWS_SMB_WCT] )
              break;

            SWS_SMB_GET_WPARAM( offset, pHdr, 0);
            pszBuf += sprintf( pszBuf, " hndl:%u", offset);

            SWS_SMB_GET_WPARAM( offset, pHdr, 1);
            pszBuf += sprintf( pszBuf, " time:%u", offset);

            SWS_SMB_GET_WPARAM( offset, pHdr, 2);
            pszBuf += sprintf( pszBuf, " date:%u", offset);
            }
          break;

        case SWS_SMBopen:
          pszBuf += sprintf( pszBuf, " OPEN");

          if ( pHdr[ SWS_SMB_FLG] & SWS_SMB_FLG_REPLY)
            {
            /* 7 word params, hndl, atts, time, date,sizelo, sizehi, access */
            if ( 7 > pHdr[ SWS_SMB_WCT] )
              break;

            SWS_SMB_GET_WPARAM( offset, pHdr, 0);
            pszBuf += sprintf( pszBuf, " hndl:%u", offset);

            SWS_SMB_GET_WPARAM( offset, pHdr, 1);
            pszBuf += sprintf( pszBuf, " attr:%#x", offset);

            SWS_SMB_GET_WPARAM( offset, pHdr, 2);
            pszBuf += sprintf( pszBuf, " time:%u", offset);

            SWS_SMB_GET_WPARAM( offset, pHdr, 3);
            pszBuf += sprintf( pszBuf, " date:%u", offset);

            SWS_SMB_GET_WPARAM( offset, pHdr, 4);
            SWS_SMB_GET_WPARAM( ulVal, pHdr, 5);
            ulVal = (ulVal << 16) + offset;
            pszBuf += sprintf( pszBuf, " offset:%lu", ulVal);

            SWS_SMB_GET_WPARAM( offset, pHdr, 6);
            pszBuf += sprintf( pszBuf, " acc:%#x", offset);
            }
          else
            {
            /* 2 word params: Mode, attribs */
            if ( 2 > pHdr[ SWS_SMB_WCT] )
              break;

            SWS_SMB_GET_WPARAM( offset, pHdr, 0);
            pszBuf += sprintf( pszBuf, " mode:%#x", offset);

            SWS_SMB_GET_WPARAM( offset, pHdr, 1);
            pszBuf += sprintf( pszBuf, " attr:%#x", offset);
            }
          break;

        case SWS_SMBtdis:
          pszBuf += sprintf( pszBuf, " TREE DISCONNECT");
          break;

        case SWS_SMBecho:
          pszBuf += sprintf( pszBuf, " ECHO");
          if ( pHdr[ SWS_SMB_FLG] & SWS_SMB_FLG_REPLY)
            {
            if ( 1 > pHdr[ SWS_SMB_WCT] )
              break;

            SWS_SMB_GET_WPARAM( offset, pHdr, 0);
            pszBuf += sprintf( pszBuf, " SEQ:%u", offset);
            }
          else
            {
            if ( 1 > pHdr[ SWS_SMB_WCT] )
              break;

            SWS_SMB_GET_WPARAM( offset, pHdr, 0);
            pszBuf += sprintf( pszBuf, " CNT:%u", offset);
            }
          break;

        default:
          pszBuf += sprintf( pszBuf, " CMD:%u", pHdr[ SWS_SMB_COM]);
          break;
          }
        }
      else
        {
        pszBuf += sprintf( pszBuf, " invalid SMB header");
        }

      if ( len > (seglen - NBSS_PREFIX))
        s_inMessage = len - (seglen - NBSS_PREFIX);
      break;

    case SWS_NBSS_REQUEST:
      pszBuf += sprintf( pszBuf, " REQ '");

      offset = 0;
      if ( NB_NAMELEN <= seglen)
        {
        /* Called name (target) */
        offset = IpDumpName( pszBuf, pHdr, offset, 1, uLen);
        pszBuf += strlen( pszBuf);
        }

      pszBuf += sprintf( pszBuf, "' from '");

      if ( offset + NB_NAMELEN <= seglen)
        {
        /* Calling name (source) - irrelevant to SMB/CIFS) */
        offset = IpDumpName( pszBuf, pHdr, offset, 1, uLen);
        pszBuf += strlen( pszBuf);
        }

      pszBuf += sprintf( pszBuf, "'");
      break;

    case SWS_NBSS_ACK:
      pszBuf += sprintf( pszBuf, " ACK");
      break;

    case SWS_NBSS_NAK:
      SWS_PEEK8( type, pHdr, 0);
      pszBuf += sprintf( pszBuf, " NAK %#x", type);
      break;

    case SWS_NBSS_RETARGET:
      pszBuf += sprintf( pszBuf, " RETARGET ");
      SWS_PEEK32( len, pHdr, 0);
      pszBuf += ValToAddress( pszBuf, len);
      SWS_PEEK16( type, pHdr, 4);
      pszBuf += sprintf( pszBuf, ":%u", type);
      break;

    case SWS_NBSS_KEEP_ALIVE:
      pszBuf += sprintf( pszBuf, " KEEP-ALIVE");
      break;

    default:
      pszBuf += sprintf( pszBuf, " SESSION TYPE:%u", type);
      break;
      }
    }

  return (int)(pszBuf - pszStart);
  }
#endif /*OPT_DUMP_NETBIOS*/


#if OPT_DUMP_RIP
/****************************************************************************
 * RIP
 ****************************************************************************/
/*
 * RIP packet dump (RFC2453)
 */
static int IpDumpRip(
  char* pszBuf,                       /* OUT: text */
  SWS_BufferH hBuffer,                /* IN: data */
  unsigned uOffset,
  unsigned seglen
) {
  char* const pszStart = pszBuf;
  SWS_PTR( unsigned char) buf;
  SWS_size_t uLen;

  buf = (SWS_PTR( unsigned char)) SWS_BufferGetFragment( hBuffer, uOffset, &uLen);

  if ( NULL == buf || uLen < seglen || seglen < SWS_RIP_HDR_LEN + SWS_RIP_RTE_LEN )
    {
    pszBuf += sprintf( pszBuf, " Message truncated");
    }
  else
    {
    unsigned ver, cmd, offset;

    SWS_PEEK8( ver, buf, SWS_RIP_HDR_VER);
    pszBuf += sprintf( pszBuf, "%u", ver);

    SWS_PEEK8( cmd, buf, SWS_RIP_HDR_MBZ);
    if ( 0 != cmd)
      pszBuf += sprintf( pszBuf, " ***MBZ:%#x", cmd);

    SWS_PEEK8( cmd, buf, SWS_RIP_HDR_CMD);
    switch ( cmd)
      {
    case SWS_RIP_CMD_REQUEST:
      pszBuf += sprintf( pszBuf, " RQST");
      break;
    case SWS_RIP_CMD_RESPONSE:
      pszBuf += sprintf( pszBuf, " RPLY");
      break;
    default:
      pszBuf += sprintf( pszBuf, " CMD:%#x", cmd);
      break;
      }

    offset = SWS_RIP_HDR_LEN;
    while ( offset + SWS_RIP_RTE_LEN <= seglen)
      {
      unsigned tag, af;
      ui32 addr;

      pszBuf += sprintf( pszBuf, " [");

      SWS_PEEK16( tag, buf, offset + SWS_RIP_RTE_TAG);
      if ( 0 != tag)
        {
        if ( 2 == ver)
          pszBuf += sprintf( pszBuf, " TAG:%u ", tag);
        else
          pszBuf += sprintf( pszBuf, " ***MBZ1:%#x ", tag);
        }

      SWS_PEEK16( af, buf, offset + SWS_RIP_RTE_AF);
      switch ( af)
        {
      case SWS_AF_INET:
        SWS_PEEK32( addr, buf, offset + SWS_RIP_RTE_ADDR);
        if ( SWS_INADDR_ANY != addr)
          pszBuf += ValToAddress( pszBuf, addr);
        else
          pszBuf += sprintf( pszBuf, "ANY");

        SWS_PEEK32( addr, buf, offset + SWS_RIP_RTE_MASK);
        if ( 0 != addr)
          {
          if ( 2 == ver)
            pszBuf += sprintf( pszBuf, "/%u", CountOnes( addr) );
          else
            pszBuf += sprintf( pszBuf, " ***MBZ2:%#lx", addr);
          }

        SWS_PEEK32( addr, buf, offset + SWS_RIP_RTE_NEXT);
        if ( 0 != addr)
          {
          if ( 2 == ver)
            {
            pszBuf += sprintf( pszBuf, " >");
            pszBuf += ValToAddress( pszBuf, addr);
            }
          else
            {
            pszBuf += sprintf( pszBuf, " ***MBZ3:%#lx", addr);
            }
          }

        SWS_PEEK32( addr, buf, offset + SWS_RIP_RTE_METRIC);
        pszBuf += sprintf( pszBuf, " D:%lu", addr);
        break;

      case 0xffff:
        /* Authentication */
        if ( 2 == ver)
          {
          pszBuf += sprintf( pszBuf, " AUTH");
          HexDump( pszBuf, hBuffer, uOffset + offset + SWS_RIP_RTE_ADDR, 16);
          break;
          }
        /* Fall thru */

      default:
        pszBuf += sprintf( pszBuf, " AF:%#x", af);
        HexDump( pszBuf, hBuffer, uOffset + offset, 12);

        SWS_PEEK32( addr, buf, offset + SWS_RIP_RTE_METRIC);
        pszBuf += sprintf( pszBuf, " D:%lu", addr);
        break;
        }

      pszBuf += sprintf( pszBuf, "]");
      offset += SWS_RIP_RTE_LEN;
      }
    }

  return (int)(pszBuf - pszStart);
  }

#endif /* OPT_DUMP_RIP */


/****************************************************************************
 * Support functions
 ****************************************************************************/
/*
 * Format an IP address
 */
static int ValToAddress(
  char* psz,                          /* OUT: Address in ASCIIZ */
  ui32 addr
) {
  return sprintf( psz, "%u.%u.%u.%u",
    (unsigned)((addr >> 24) & 0xff), (unsigned)((addr >> 16) & 0xff),
    (unsigned)((addr >> 8) & 0xff), (unsigned)(addr & 0xff)
  );
  }


/*
 * Format time as hh:mm:ss
 */
static int SecsToTime(
  char* psz,                          /* OUT: Time as hh:mm:ss.iii */
  ui32 sc
) {
  ui32 hr, mn;

  mn = sc / 60U;
  sc -= 60U * mn;
  hr = mn / 60U;
  mn -= 60U * hr;
  return sprintf( psz, "%lu:%02lu:%02lu", hr, mn, sc);
  }


/*
 * Count no. consecutive one bits
 * Converts subnet mask to CIDR run length
 */
static unsigned CountOnes( ui32 u)
  {
  unsigned bits = 0;
  while ( u & 0x80000000L)
    {
    u <<= 1;
    ++bits;
    }
  return bits;
  }


/*
 * Hex dump
 */
static int HexDump(
  char* pszBuf,                       /* OUT: text */
  SWS_BufferH hBuffer,                /* IN: data */
  unsigned uOffset,
  unsigned uLen
) {
  int out = 0;
  SWS_PTR( unsigned char) p;

  while ( uLen)
    {
    SWS_size_t uFrag;

    /* Get next fragment */
    p = (SWS_PTR( unsigned char)) SWS_BufferGetFragment( hBuffer, uOffset, &uFrag);
    if ( NULL == p)
      {
      out += sprintf( pszBuf + out, " ?");
      break;
      }

    if ( uFrag <= uLen)
      uLen -= uFrag;
    else
      {
      uFrag = uLen;
      uLen = 0;
      }
    uOffset += uFrag;

    while ( uFrag--)
      {
      out += sprintf( pszBuf + out, " %02X", *(p++) );
      }
    }

  return out;
  }

/* End of file */
