/****************************************************************************
 *
 * File name   : sws_dmpe.c
 * Function    : Ethernet packet dumper
 * Authors     : Lawrence Rust
 * Systems     : ANSI C.
 *
 ****************************************************************************
 *
 * Created by Lawrence Rust, Software Systems Consultants
 * lvr@softsystem.co.uk. Tel/Fax +33 5 49 72 79 63
 *
 * 29-Oct-02, LVR, Created.
 * 08-May-05, LVR, Replace PlatformGetUint with SWS_CfgGetValue
 * 10-Jun-05, LVR, Add SWS_kEnetEventError .
 * 30-Jun-05, LVR, Added PPP frame types & HexDump of unknown frame types .
 *
 * Comments:
 * --------
 * This file implements an Ethernet packet dumper.
 *
 ****************************************************************************
 */

/* Compilation options */
#ifndef OPT_DUMP_PPP
#define OPT_DUMP_PPP 1
#endif

#ifdef _MSC_VER
# pragma warning ( disable : 4127)    /* conditional expression is constant */
# pragma warning ( disable : 4514)    /* unreferenced inline function has been removed */
#endif


/* Exports */
#include "sws_dmpe.h"

/* Imports */
#include <stddef.h>
#include <stdarg.h>   /* va_list */
#include <stdio.h>
#include <string.h>
#if _MSC_VER >= 1000
#include <crtdbg.h>
#endif

#include "sws_gef.h"
#include "sws_buf.h"
#include "sws_cfg.h"
#include "sws_sock.h"

#include "sws_enet.h"
#if OPT_DUMP_PPP
#include "sws_ppp.h"
#include "sws_dmpp.h"
#endif


/*
 * Macros & Constants
 */
/* Read/write in network (MSB first, or big-endian) byte order */
#ifndef SWS_PEEK8
#define SWS_PEEK8(_v,_p,_o) _v = (*(SWS_PTR(const ui8))((SWS_PTR(const char))(_p) + (_o)))
#endif
#ifndef SWS_PEEK16
#define SWS_PEEK16(_v,_p,_o) _v = SWS_ntohs(*(SWS_PTR(SWS_PACKED(const ui16)))((SWS_PTR(const char))(_p) + (_o)))
#endif
#ifndef SWS_PEEK32
#define SWS_PEEK32(_v,_p,_o) _v = SWS_ntohl(*(SWS_PTR(SWS_PACKED(const ui32)))((SWS_PTR(const char))(_p) + (_o)))
#endif

enum
  {
  kEthMinPacket = 60,     /* smallest legal size packet, no fcs */
  kEthMaxPacket = 1514,   /* largest legal size packet, no fcs */
  kEthMax802p3Len = 0x5dc /* Max value for 802.3 length field */
  };

/* Ethernet types from STD2 (RFC1700) p168 */
#define kEthIpType   ((ui16)0x800)    /* IPv4 */
#define kEthArpType  ((ui16)0x806)    /* ARP */
#define kEthRarpType ((ui16)0x8035)   /* RARP */
#define kEthSnmpType ((ui16)0x814c)   /* SNMP */

/* PPPoE types from RFC2516 */
#define kEthTypePppoeDiscovery ((ui16)0x8863)   /* PPPoE Discovery */
#define kEthTypePppoeSession   ((ui16)0x8864)   /* PPPoE Session */

/* MAC header offsets */
enum
  {
  kMacDstAddress  = 0,
  kMacSrcAddress  = kMacDstAddress + sizeof( SWS_EnetMacAddress),
  kMacType        = kMacSrcAddress + sizeof( SWS_EnetMacAddress),
  kMacSize        = kMacType + 2
  };

/* 802.3 header offsets */
enum
  {
  /* LLC, 3 bytes */
  kLlcDsap = 0,
  kLlcSsap,
  kLlcControl,

  /* SNAP, 5 bytes */
  kSnapProtocol, /* 3 bytes */
  kSnapType = kSnapProtocol + 3, /* 2 bytes */

  kEth8023Prefix = kSnapType + 2 /* Total 802.3 header size */
  };

/* ARP packet field offsets from RFC 826 */
enum
  {
  kArpHrd = 0,        /* Hardware address type */
  kArpPro = 2,        /* Protocol address type  - kEthXxxType */
  kArpHln = 4,        /* Hardware address byte len */
  kArpPln = 5,        /* Protocol address byte len */
  kArpOp  = 6,        /* Opcode */
  kArpHrdSender = 8,  /* sender's h/w address */
  kArpProSender = kArpHrdSender + sizeof( SWS_EnetMacAddress),
  kArpHrdTarget = kArpProSender + sizeof( ui32),
  kArpProTarget = kArpHrdTarget + sizeof( SWS_EnetMacAddress),
  kArpLen = kArpProTarget + sizeof(ui32)
  };

/* ARP hardware address types from RFC 1700 p163 */
enum
  {
  kArpHwEthernet = 1,
  kArpHwSerial   = 20
  };

/* ARP opcodes from RFC 826 */
enum
  {
  kArpOpRequest = 1,
  kArpOpReply,

  kArpOpRequestReverse, /* RFC903 */
  kArpOpReplyReverse,

  kDrarpOpRequest,
  kDrarpOpReply,
  kDrarpOpError,

  kInarpOpRequest, /* RFC1293 */
  kInarpOpReply,

  kArpOpNak
  };


/*
 * Prototypes
 */
static int EnetDump( SWS_PTR( char), char, SWS_BufferH);
static int EnetDumpMacAddr( SWS_PTR( char), SWS_PTR( const void), SWS_size_t);
static int EnetDumpArp( SWS_PTR( char), SWS_PTR( const void), SWS_size_t);
static int EnetDump8023( SWS_PTR( char), SWS_PTR( const void), SWS_size_t);
static int HexDump( SWS_PTR( char), SWS_PTR( const void), unsigned);


/*
 * Module data
 */
static int s_iEnetVerbosity = -1;
static unsigned s_arp, s_ip, s_802p3; /* b0=TxEn b1=RxEn */


/*
 * Configure the packet dumper
 */
SWS_EXTERN( void, SWS_DmpEnetSetup(
  int iVerbosity                      /* IN: Verbosity 0= off, 1= normal, 2= more etc */
)){
  if ( iVerbosity >= 0)
    s_iEnetVerbosity = iVerbosity;
  else if ( s_iEnetVerbosity < 0)
    s_iEnetVerbosity = (int)SWS_CfgGetValue( "log", "edump", 0);

  s_arp = (unsigned)SWS_CfgGetValue( "log", "arp", 0xffff);
  s_ip = (unsigned)SWS_CfgGetValue( "log", "enetip", s_iEnetVerbosity < 4 ? 0 : 0xffff);
  s_802p3 = (unsigned)SWS_CfgGetValue( "log", "enet802p3", 0xffff);
  }


/*
 * GEF handler
 */
SWS_EXTERN( int, SWS_DmpEnet(
  SWS_PTR( char) pszBuf,              /* OUT: text */
  int event,                          /* IN: SWS_IpEvent */
  va_list va                          /* IN: Tx: Next IP address, Rx: isBroadcast */
)){
  int count = 0;
  const char* psz;

  switch ( event)
    {
  default:
    break;

  case SWS_kGefEnetTx:
    if ( s_iEnetVerbosity > 0)
      count = EnetDump( pszBuf, 'T', va_arg( va, SWS_BufferH) );
    break;

  case SWS_kGefEnetRx:
    if ( s_iEnetVerbosity > 0)
      {
      SWS_BufferH hBuf;
      SWS_PTR( void) pvData;
      SWS_size_t uLen;
      char mode;

      pvData = va_arg( va, SWS_PTR( void));
      uLen = (SWS_size_t)va_arg( va, ui32);
      mode = (char)(va_arg( va, int) ? 'B' : 'R');

      hBuf = SWS_BufferGetShadow( uLen, SWS_kBufferPrefix, pvData);
      if ( NULL != hBuf)
        {
        count = EnetDump( pszBuf, mode, hBuf);
        SWS_BufferReleaseShadow( hBuf);
        }
      }
    break;

  case SWS_kGefEnetError:
    count = va_arg( va, int); /* Error */
    psz = va_arg( va, const char*);
    count = sprintf( pszBuf, " *** Enet %s %d", psz, count);
    break;
    }

  return count;
  }


/*****************************************************************************
 * Internal functions
 ****************************************************************************/

/*
 * Dump an ethernet packet
 */
static int EnetDump(
  SWS_PTR( char) pszBuf,              /* OUT: text */
  char mode,
  SWS_BufferH hBuf
) {
  SWS_PTR( char) const pszStart = pszBuf;
  SWS_PTR( const void) pvMacHdr;
  SWS_PTR( const void) pvPayload;
  SWS_size_t uLen;
  unsigned len;
  ui16 type;

  pvMacHdr = SWS_BufferGetFragment( hBuf, 0, &uLen);
  if ( NULL == pvMacHdr || uLen < kMacSize)
    return 0;

  /* Get frame type/length */
  SWS_PEEK16( type, pvMacHdr, kMacType);
  len = (unsigned)uLen;

  /* Skip Mac header */
  pvPayload = SWS_BufferGetFragment( hBuf, kMacSize, &uLen);

  if ( type >= kEth8023Prefix && type <= kEthMax802p3Len)
    {
    if ( 0 == (('T' == mode ? 1 : 2) & s_802p3))
      return 0;

    pszBuf += sprintf( pszBuf, " %c:%u", mode, len);
    pszBuf += EnetDumpMacAddr( pszBuf, pvMacHdr, uLen);
    pszBuf += sprintf( pszBuf, " 802.3");
    pszBuf += EnetDump8023( pszBuf, pvPayload, uLen);
    }
  else switch ( type)
    {
  case kEthArpType:
    if ( 0 == (('T' == mode ? 1 : 2) & s_arp))
      return 0;

    pszBuf += sprintf( pszBuf, " %c:%u", mode, len);
    pszBuf += EnetDumpMacAddr( pszBuf, pvMacHdr, uLen);
    pszBuf += sprintf( pszBuf, " ARP");
    pszBuf += EnetDumpArp( pszBuf, pvPayload, uLen);
    break;

  case kEthIpType:
    if ( 0 == (('T' == mode ? 1 : 2) & s_ip))
      return 0;

    pszBuf += sprintf( pszBuf, " %c:%u", mode, len);
    pszBuf += EnetDumpMacAddr( pszBuf, pvMacHdr, uLen);
    pszBuf += sprintf( pszBuf, " IPv4");
    break;

#if OPT_DUMP_PPP
  /* PPP types */
  case SWS_PPP_IPCP:
  case SWS_PPP_CCP:
  case SWS_PPP_LCP:
  case SWS_PPP_CHAP:
  case SWS_PPP_PAP:
    pszBuf += sprintf( pszBuf, " %c:%u", mode, len);
    pszBuf += EnetDumpMacAddr( pszBuf, pvMacHdr, uLen);
    pszBuf += SWS_DmpPppPacket( pszBuf, type, pvPayload, uLen);
    break;
#endif

  default:
    pszBuf += sprintf( pszBuf, " %c:%u", mode, len);
    pszBuf += EnetDumpMacAddr( pszBuf, pvMacHdr, uLen);
    pszBuf += sprintf( pszBuf, " %#06x", type);
    pszBuf += HexDump( pszBuf, pvPayload, uLen < 40 ? uLen : 40);
    break;
    }

  return (int)(pszBuf - pszStart);
  }


/*
 * MAC header dump
 */
static int EnetDumpMacAddr(
  SWS_PTR( char) pszBuf,
  SWS_PTR( const void) pvHdr,
  SWS_size_t uLen
) {
  SWS_PTR( char) const pszStart = pszBuf;

  if ( uLen < kMacType)
    return 0;

  /* Test for M$ PPP encapsulation */
  if ( 0 == memcmp( pvHdr, " SEND", 5))
    {
    pszBuf += sprintf( pszBuf, " PPP-SEND%u",
      ((SWS_PTR(const ui8))pvHdr)[ 5] );
    }
  else if ( 0 == memcmp( pvHdr, " RECV", 5))
    {
    pszBuf += sprintf( pszBuf, " PPP-RECV%u",
      ((SWS_PTR(const ui8))pvHdr)[ 5] );
    }
  else
    {
    /* Display Mac header */
    pszBuf += sprintf( pszBuf, " %02X%02X%02X%02X%02X%02X >%02X%02X%02X%02X%02X%02X",
      ((SWS_PTR(const ui8))pvHdr)[ kMacSrcAddress +0],
      ((SWS_PTR(const ui8))pvHdr)[ kMacSrcAddress +1],
      ((SWS_PTR(const ui8))pvHdr)[ kMacSrcAddress +2],
      ((SWS_PTR(const ui8))pvHdr)[ kMacSrcAddress +3],
      ((SWS_PTR(const ui8))pvHdr)[ kMacSrcAddress +4],
      ((SWS_PTR(const ui8))pvHdr)[ kMacSrcAddress +5],
      ((SWS_PTR(const ui8))pvHdr)[ kMacDstAddress +0],
      ((SWS_PTR(const ui8))pvHdr)[ kMacDstAddress +1],
      ((SWS_PTR(const ui8))pvHdr)[ kMacDstAddress +2],
      ((SWS_PTR(const ui8))pvHdr)[ kMacDstAddress +3],
      ((SWS_PTR(const ui8))pvHdr)[ kMacDstAddress +4],
      ((SWS_PTR(const ui8))pvHdr)[ kMacDstAddress +5]
    );
    }

  return (int)(pszBuf - pszStart);
  }


/*
 * ARP packet dump
 */
static int EnetDumpArp(
  SWS_PTR( char) pszBuf,
  SWS_PTR( const void) pvHdr,
  SWS_size_t uLen
) {
  SWS_PTR( char) const pszStart = pszBuf;
  unsigned w, ln;
  ui32 src, dst;

  if ( uLen < kArpLen)
    return 0;

  /* Parse the ARP header */
  SWS_PEEK16( w, pvHdr, kArpHrd); /* Hardware address space */
  if ( kArpHwEthernet != w)
    return sprintf( pszBuf, " ?type %u",w);

  SWS_PEEK16( w, pvHdr, kArpPro); /* Protocol address space */
  if ( kEthIpType != w)
    return sprintf( pszBuf, " ?protocol %u",w);

  SWS_PEEK8( ln, pvHdr, kArpHln); /* Byte len of each hardware address */
  if ( sizeof( SWS_EnetMacAddress) != ln)
    return sprintf( pszBuf, " ?hwLen %u",w);

  SWS_PEEK8( ln, pvHdr, kArpPln); /* Byte len of each protocol address */
  if ( sizeof( ui32) != ln)
    return sprintf( pszBuf, " ?ptclLen %u",w);

  SWS_PEEK16( w, pvHdr, kArpOp);
  switch ( w)
    {
  case kArpOpRequest:
    pszBuf += sprintf( pszBuf, "rqst");
    break;
  case kArpOpReply:
    pszBuf += sprintf( pszBuf, "rply");
    break;
  default:
    pszBuf += sprintf( pszBuf, "op %u", w);
    break;
    }

  /* Display src & dst */
  SWS_PEEK32( src, pvHdr, kArpProSender); /* Sender's IP address */
  SWS_PEEK32( dst, pvHdr, kArpProTarget);

#if 0
  pszBuf += sprintf( pszBuf,
    " %u.%u.%u.%u (%02X%02X%02X%02X%02X%02X) < %u.%u.%u.%u (%02X%02X%02X%02X%02X%02X)",
    (unsigned)((dst >> 24) & 0xff), (unsigned)((dst >> 16) & 0xff),
    (unsigned)((dst >> 8) & 0xff), (unsigned)(dst & 0xff),
    ((SWS_PTR(const ui8))pvHdr)[ kArpHrdTarget +0],
    ((SWS_PTR(const ui8))pvHdr)[ kArpHrdTarget +1],
    ((SWS_PTR(const ui8))pvHdr)[ kArpHrdTarget +2],
    ((SWS_PTR(const ui8))pvHdr)[ kArpHrdTarget +3],
    ((SWS_PTR(const ui8))pvHdr)[ kArpHrdTarget +4],
    ((SWS_PTR(const ui8))pvHdr)[ kArpHrdTarget +5],
    (unsigned)((src >> 24) & 0xff), (unsigned)((src >> 16) & 0xff),
    (unsigned)((src >> 8) & 0xff), (unsigned)(src & 0xff),
    ((SWS_PTR(const ui8))pvHdr)[ kArpHrdSender +0],
    ((SWS_PTR(const ui8))pvHdr)[ kArpHrdSender +1],
    ((SWS_PTR(const ui8))pvHdr)[ kArpHrdSender +2],
    ((SWS_PTR(const ui8))pvHdr)[ kArpHrdSender +3],
    ((SWS_PTR(const ui8))pvHdr)[ kArpHrdSender +4],
    ((SWS_PTR(const ui8))pvHdr)[ kArpHrdSender +5]
  );
#else
  pszBuf += sprintf( pszBuf,
    " %u.%u.%u.%u > %u.%u.%u.%u",
    (unsigned)((src >> 24) & 0xff), (unsigned)((src >> 16) & 0xff),
    (unsigned)((src >> 8) & 0xff), (unsigned)(src & 0xff),
    (unsigned)((dst >> 24) & 0xff), (unsigned)((dst >> 16) & 0xff),
    (unsigned)((dst >> 8) & 0xff), (unsigned)(dst & 0xff)
  );
#endif

  return (int)(pszBuf - pszStart);
  }


/*
 * Ethernet 802.3 framing see RFC 1042
 */
static int EnetDump8023(
  SWS_PTR( char) pszBuf,
  SWS_PTR( const void) pvHdr,
  SWS_size_t uLen
) {
  unsigned dsap, ssap, control, type;
  ui32 protocol;

  if ( uLen < kEth8023Prefix)
    return 0;

  /* LLC */
  SWS_PEEK8( dsap, pvHdr, kLlcDsap);
  SWS_PEEK8( ssap, pvHdr, kLlcSsap);
  SWS_PEEK8( control, pvHdr, kLlcControl);

  /* SNAP */
  SWS_PEEK32( protocol, pvHdr, kSnapProtocol);
  SWS_PEEK16( type, pvHdr, kSnapType);

  return sprintf( pszBuf, " dsap:%u ssap:%u ctrl:%u ptcl:%#lx type:%u",
    dsap, ssap, control, protocol, type);
  }


/*
 * Hex dump
 */
static int HexDump(
  SWS_PTR( char) pszBuf,              /* OUT: text */
  SWS_PTR( const void) pv,
  unsigned uLen
) {
  int out = 0;
  SWS_PTR( const unsigned char) p = (SWS_PTR( const unsigned char)) pv;

  while ( uLen--)
    out += sprintf( pszBuf + out, " %02X", *(p++) );

  return out;
  }

/* End of file */
