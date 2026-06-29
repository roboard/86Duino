/****************************************************************************
 *
 * File name   : sws_dmpp.c
 * Function    : P2P/PPP packet dumper
 * Authors     : Lawrence Rust
 * Systems     : ANSI C.
 *
 ****************************************************************************
 *
 * Created by Lawrence Rust, Software Systems Consultants
 * lvr@softsystem.co.uk. Tel/Fax +33 5 49 72 79 63
 *
 * 25-Aug-05, LVR, Created.
 *
 * Comments:
 * --------
 * This file implements a P2P/PPP packet dumper.
 *
 ****************************************************************************
 */

/* Compilation options */
#ifdef _MSC_VER
# pragma warning ( disable : 4127)    /* conditional expression is constant */
# pragma warning ( disable : 4514)    /* unreferenced inline function has been removed */
#endif

/* Exports */
#include "sws_dmpp.h"

/* Imports */
#include <stddef.h>
#include <stdarg.h>   /* va_list */
#include <string.h>
#if _MSC_VER >= 1000
#include <crtdbg.h>
#endif

#include "sws_gef.h"
#include "sws_buf.h"
#include "sws_cfg.h"
#include "sws_sock.h"

#include "sws_ppp.h"


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


/*
 * Prototypes
 */
static int PppDump( SWS_PTR( char) const, char, ui16, SWS_BufferH);
static int LcpDump( unsigned, SWS_PTR( char), SWS_PTR( const void), SWS_size_t);
static int LcpOptDump( SWS_PTR( char), SWS_PTR( const void), SWS_size_t);
static int IpcpOptDump( SWS_PTR( char), SWS_PTR( const void), SWS_size_t);
static int CcpOptDump( SWS_PTR( char), SWS_PTR( const void), SWS_size_t);
static int PapDump( SWS_PTR( char), SWS_PTR( const void), SWS_size_t);
static int ChapDump( SWS_PTR( char), SWS_PTR( const void), SWS_size_t);
static int HexDump( SWS_PTR( char), SWS_PTR( const void), unsigned);


/*
 * Module data
 */
static int s_iPppVerbosity = -1;
static int s_iP2pVerbosity = -1;


/*
 * Configure the packet dumper
 */
SWS_EXTERN( void, SWS_DmpPppSetup(
  int iVerbosity                      /* IN: Verbosity 0= off, 1= normal, 2= more etc */
)){
  if ( iVerbosity >= 0)
    s_iPppVerbosity = iVerbosity;
  else if ( -1 == s_iPppVerbosity)
    s_iPppVerbosity = (int)SWS_CfgGetValue( "log", "pppdump", 0);
  }

SWS_EXTERN( void, SWS_DmpP2pSetup(
  int iVerbosity                      /* IN: Verbosity 0= off, 1= normal, 2= more etc */
)){
  if ( iVerbosity >= 0)
    s_iP2pVerbosity = iVerbosity;
  else if ( -1 == s_iP2pVerbosity)
    s_iP2pVerbosity = (int)SWS_CfgGetValue( "log", "p2pdump", 0);
  }


/*
 * GEF handler
 */
SWS_EXTERN( int, SWS_DmpPpp(
  SWS_PTR( char) pszBuf,              /* OUT: text */
  int event,                          /* IN: SWS_IpEvent */
  va_list va                          /* IN: Tx: Next IP address, Rx: isBroadcast */
)){
  int count;
  unsigned protocol;
  SWS_BufferH hBuffer;
  SWS_PTR( const void) pvHdr;
  SWS_size_t len;

  switch ( event)
    {
  default:
    count = 0;
    break;

  case SWS_kGefPppTx:
    protocol = va_arg( va, unsigned);
    hBuffer = va_arg( va, SWS_BufferH);
    count = PppDump( pszBuf, 'T', (ui16)protocol, hBuffer);
    break;

  case SWS_kGefPppRx:
    protocol = va_arg( va, unsigned);
    hBuffer = va_arg( va, SWS_BufferH);
    count = PppDump( pszBuf, 'R', (ui16)protocol, hBuffer);
    break;

  case SWS_kGefPppError:
    count = sprintf( pszBuf, " *** PPP %s", va_arg( va, const char*));
    break;

  case SWS_kGefSlipTx:
    hBuffer = va_arg( va, SWS_BufferH);
    pvHdr = SWS_BufferGetFragment( hBuffer, 0, &len);

    SWS_PEEK8( protocol, pvHdr, 0);
    if ( 0x80 & protocol)
      protocol = SWS_PPP_VJ;
    else if ( protocol >= 0x70)
      protocol = SWS_PPP_VJU;
    else
      protocol = SWS_PPP_IP;

    count = PppDump( pszBuf, 'T', (ui16)protocol, hBuffer);
    break;

  case SWS_kGefSlipRx:
    hBuffer = va_arg( va, SWS_BufferH);
    pvHdr = SWS_BufferGetFragment( hBuffer, 0, &len);

    SWS_PEEK8( protocol, pvHdr, 0);
    if ( 0x80 & protocol)
      protocol = SWS_PPP_VJ;
    else if ( protocol >= 0x70)
      protocol = SWS_PPP_VJU;
    else
      protocol = SWS_PPP_IP;

    count = PppDump( pszBuf, 'R', (ui16)protocol, hBuffer);
    break;

  case SWS_kGefSlipError:
    count = sprintf( pszBuf, " *** SLIP %s", va_arg( va, const char*));
    break;
    }

  return count;
  }


SWS_EXTERN( int, SWS_DmpP2p(
  SWS_PTR( char) pszBuf,              /* OUT: text */
  int event,                          /* IN: SWS_IpEvent */
  va_list va                          /* IN: Tx: Next IP address, Rx: isBroadcast */
)){
  int count;
  ui32 uLen;
  SWS_PTR( const char) pszMsg;
  SWS_PTR( const char) pszEnd;
  int len;

  switch ( event)
    {
  default:
    count = 0;
    break;

  case SWS_kGefP2pTx:
    if ( s_iP2pVerbosity > 0)
      {
      uLen = va_arg( va, ui32);
      pszMsg = va_arg( va, SWS_PTR( const char));
      pszEnd = strpbrk( pszMsg, "\r\n");
      len = NULL != pszEnd ? (int)(pszEnd - pszMsg) : 255;
      count = sprintf( pszBuf, " T:%lu P2P %.*s", uLen, len, pszMsg);
      }
    else
      count = 0;
    break;

  case SWS_kGefP2pRx:
    if ( s_iP2pVerbosity > 0)
      {
      uLen = va_arg( va, ui32);
      pszMsg = va_arg( va, SWS_PTR( const char));
      pszEnd = strpbrk( pszMsg, "\r\n");
      len = NULL != pszEnd ? (int)(pszEnd - pszMsg) : 255;
      count = sprintf( pszBuf, " R:%lu P2P %.*s", uLen, len, pszMsg);
      }
    else
      count = 0;
    break;

  case SWS_kGefP2pError:
    pszMsg = va_arg( va, SWS_PTR( const char));
    count = sprintf( pszBuf, " *** P2P %s", pszMsg);
    break;
    }

  return count;
  }


/*
 * Dump a PPP packet
 */
SWS_EXTERN( int, SWS_DmpPppPacket(    /* Returns space used in buffer */
  SWS_PTR( char) pszBuf,              /* OUT: Buffer */
  ui16 protocol,                      /* IN: Protocol */
  SWS_PTR( const void) pvHdr,         /* IN: -> packet */
  SWS_size_t uLen                     /* IN: packet len */
)){
  SWS_PTR( char) const pszStart = pszBuf;

  switch ( protocol)
    {
  case SWS_PPP_IPCP:
    pszBuf += sprintf( pszBuf, " IPCP");
    pszBuf += LcpDump( protocol, pszBuf, pvHdr, uLen);
    break;

  case SWS_PPP_CCP:
    pszBuf += sprintf( pszBuf, " CCP");
    pszBuf += LcpDump( protocol, pszBuf, pvHdr, uLen);
    break;

  case SWS_PPP_LCP:
    pszBuf += sprintf( pszBuf, " LCP");
    pszBuf += LcpDump( protocol, pszBuf, pvHdr, uLen);
    break;

  case SWS_PPP_PAP:
    pszBuf += sprintf( pszBuf, " PAP");
    pszBuf += PapDump( pszBuf, pvHdr, uLen);
    break;

  case SWS_PPP_CHAP:
    pszBuf += sprintf( pszBuf, " CHAP");
    pszBuf += ChapDump( pszBuf, pvHdr, uLen);
    break;

  case SWS_PPP_IP:
    pszBuf += sprintf( pszBuf, " IP");
    break;

  case SWS_PPP_VJ:
    pszBuf += sprintf( pszBuf, " VJ");
    /* VJ header is 3..16 bytes */
    pszBuf += HexDump( pszBuf, pvHdr, uLen < 16 ? uLen : 16);
    break;

  case SWS_PPP_VJU:
    /* As IP but SWS_IPHDR_PROTOCOL = compression slot */
    pszBuf += sprintf( pszBuf, " VJU");
    break;

  default:
    pszBuf += sprintf( pszBuf, " %#04x", protocol);
    pszBuf += HexDump( pszBuf, pvHdr, uLen < 40 ? uLen : 40);
    break;
    }

  return (int)(pszBuf - pszStart);
  }


/*****************************************************************************
 * Internal functions
 ****************************************************************************/

/*
 * PPP packet dump
 */
static int PppDump(
  SWS_PTR( char) const pszBuf,        /* OUT: text */
  char mode,
  ui16 protocol,                      /* IN: Protocol */
  SWS_BufferH hBuf
) {
  int count = 0;

  if ( NULL != hBuf
    && s_iPppVerbosity > 0
  ) {
    SWS_PTR( void) pvHdr;
    SWS_size_t uLen;

    switch ( protocol)
      {
    case SWS_PPP_IP:
    case SWS_PPP_VJU:
    case SWS_PPP_VJ:
      if ( s_iPppVerbosity < 3)
        return 0;
      break;
      }

    /* Append direction and length */
    uLen = SWS_BufferSize( hBuf);
    count += sprintf( pszBuf + count, " %c:%u", mode, (unsigned)uLen);

    pvHdr = SWS_BufferGetFragment( hBuf, 0, &uLen);
    count += SWS_DmpPppPacket( pszBuf + count, protocol, pvHdr, uLen);
    }

  return count;
  }


/*
 * LCP/IPCP & CCP packet dump
 * See RFC 1661 p27
 */
static int LcpDump(
  unsigned type,
  SWS_PTR( char) pszBuf,
  SWS_PTR( const void) pvHdr,
  SWS_size_t uLen
) {
  SWS_size_t len;
  unsigned code, id, protocol;
  ui32 magic;
  SWS_PTR( char) const pszStart = pszBuf;

  if ( NULL == pvHdr || uLen < SWS_PPP_HDR_DATA)
    return 0;

  /* Parse the LCP header */
  SWS_PEEK8( id, pvHdr, SWS_PPP_HDR_ID);
  pszBuf += sprintf( pszBuf, ":%u", id);

  SWS_PEEK16( len, pvHdr, SWS_PPP_HDR_LEN);
  if ( len != uLen)
    {
    pszBuf += sprintf( pszBuf, " %u/%u", (unsigned)uLen, (unsigned)len);
    if ( len > uLen)
      len = uLen;
    }

  SWS_PEEK8( code, pvHdr, SWS_PPP_HDR_CODE);
  switch ( code)
    {
  case SWS_PPP_CONFIGURE_VENDOR: /* RFC2153 */
    pszBuf += sprintf( pszBuf, " VENDOR");
    if ( len >= 8)
      {
      SWS_PEEK32( magic, pvHdr, SWS_PPP_HDR_DATA);
      pszBuf += sprintf( pszBuf, "(%#lx)",magic);
      }
    if ( len >= 12)
      {
      SWS_PEEK32( magic, pvHdr, SWS_PPP_HDR_DATA + 4);
      pszBuf += sprintf( pszBuf, " KIND-OUI:%#lx",magic);
      }
    if ( len > 12)
      pszBuf += HexDump( pszBuf, (SWS_PTR(const char))pvHdr + 12, len - 12);
    break;

  case SWS_PPP_CONFIGURE_REQ:
    pszBuf += sprintf( pszBuf, " CNFG-REQ");
    goto options;

  case SWS_PPP_CONFIGURE_ACK:
    pszBuf += sprintf( pszBuf, " CNFG-ACK");
    goto options;

  case SWS_PPP_CONFIGURE_NAK:
    pszBuf += sprintf( pszBuf, " CNFG-NAK");
    goto options;

  case SWS_PPP_CONFIGURE_REJ:
    pszBuf += sprintf( pszBuf, " CNFG-REJ");

  options:
    if ( len <= SWS_PPP_HDR_DATA)
      break;

    switch ( type)
      {
    case SWS_PPP_LCP:
      pszBuf += LcpOptDump(
        pszBuf, (SWS_PTR(const char))pvHdr + SWS_PPP_HDR_DATA, len - SWS_PPP_HDR_DATA);
      break;
    case SWS_PPP_IPCP:
      pszBuf += IpcpOptDump(
        pszBuf, (SWS_PTR(const char))pvHdr + SWS_PPP_HDR_DATA, len - SWS_PPP_HDR_DATA);
      break;
    case SWS_PPP_CCP:
      pszBuf += CcpOptDump(
        pszBuf, (SWS_PTR(const char))pvHdr + SWS_PPP_HDR_DATA, len - SWS_PPP_HDR_DATA);
      break;
    default:
      pszBuf += HexDump(
        pszBuf, (SWS_PTR(const char))pvHdr + SWS_PPP_HDR_DATA, len - SWS_PPP_HDR_DATA);
      break;
      }
    break;

  case SWS_PPP_TERMINATE_REQ:
    pszBuf += sprintf( pszBuf, " TERM-REQ");
    pszBuf += HexDump(
      pszBuf, (SWS_PTR(const char))pvHdr + SWS_PPP_HDR_DATA, len - SWS_PPP_HDR_DATA);
    break;

  case SWS_PPP_TERMINATE_ACK:
    pszBuf += sprintf( pszBuf, " TERM-ACK");
    pszBuf += HexDump(
      pszBuf, (SWS_PTR(const char))pvHdr + SWS_PPP_HDR_DATA, len - SWS_PPP_HDR_DATA);
    break;

  case SWS_PPP_CODE_REJ:
    pszBuf += sprintf( pszBuf, " CODE-REJ");
    /* Rejected packet follows */
    pszBuf += HexDump(
      pszBuf, (SWS_PTR(const char))pvHdr + SWS_PPP_HDR_DATA, len - SWS_PPP_HDR_DATA);
    break;

  case SWS_PPP_PROTOCOL_REJ:
    pszBuf += sprintf( pszBuf, " PRTC-REJ:");
    if ( len >= SWS_PPP_HDR_DATA + 2)
      {
      SWS_PEEK16( protocol, pvHdr, SWS_PPP_HDR_DATA);
      switch ( protocol)
        {
      case SWS_PPP_LCP:
        pszBuf += sprintf( pszBuf, "LCP");
        break;
      case SWS_PPP_IPCP:
        pszBuf += sprintf( pszBuf, "IPCP");
        break;
      case SWS_PPP_CCP:
        pszBuf += sprintf( pszBuf, "CCP");
        break;
      case SWS_PPP_CHAP:
        pszBuf += sprintf( pszBuf, "CHAP");
        break;
      case SWS_PPP_PAP:
        pszBuf += sprintf( pszBuf, "PAP");
        break;
      default:
        pszBuf += sprintf( pszBuf, "%#06x", protocol);
        break;
        }

      /* Rejected packet follows */
      if ( s_iPppVerbosity >= 2)
        {
        pszBuf += HexDump(
          pszBuf,
          (SWS_PTR(const char))pvHdr + SWS_PPP_HDR_DATA + 2,
          len - (SWS_PPP_HDR_DATA + 2)
        );
        }
      }
    break;

  case SWS_PPP_ECHO_REQ:
    pszBuf += sprintf( pszBuf, " ECHO-REQ");
    if ( len >= SWS_PPP_HDR_DATA + 4)
      {
      SWS_PEEK32( magic, pvHdr, SWS_PPP_HDR_DATA);
      pszBuf += sprintf( pszBuf, "(%#lx)",magic);
      pszBuf += HexDump(
        pszBuf,
        (SWS_PTR(const char))pvHdr + SWS_PPP_HDR_DATA + 4,
        len - (SWS_PPP_HDR_DATA + 4)
      );
      }
    break;

  case SWS_PPP_ECHO_REPLY:
    pszBuf += sprintf( pszBuf, " ECHO-RPY");
    if ( len >= SWS_PPP_HDR_DATA + 4)
      {
      SWS_PEEK32( magic, pvHdr, SWS_PPP_HDR_DATA);
      pszBuf += sprintf( pszBuf, "(%#lx)",magic);
      pszBuf += HexDump(
        pszBuf,
        (SWS_PTR(const char))pvHdr + SWS_PPP_HDR_DATA + 4,
        len - (SWS_PPP_HDR_DATA + 4)
      );
      }
    break;

  case SWS_PPP_DISCARD_REQ:
    pszBuf += sprintf( pszBuf, " DCRD-REQ");
    if ( len >= SWS_PPP_HDR_DATA + 4)
      {
      SWS_PEEK32( magic, pvHdr, SWS_PPP_HDR_DATA);
      pszBuf += sprintf( pszBuf, "(%#lx)",magic);
      pszBuf += HexDump(
        pszBuf,
        (SWS_PTR(const char))pvHdr + SWS_PPP_HDR_DATA + 4,
        len - (SWS_PPP_HDR_DATA + 4)
      );
      }
    break;

  case SWS_PPP_IDENT: /* LCP extension RFC1570 */
    pszBuf += sprintf( pszBuf, " IDENT");
    if ( len >= SWS_PPP_HDR_DATA + 4)
      {
      SWS_PEEK32( magic, pvHdr, SWS_PPP_HDR_DATA);
      pszBuf += sprintf( pszBuf, "(%#lx)",magic);
      }
    if ( len > SWS_PPP_HDR_DATA + 4)
      pszBuf += sprintf( pszBuf,
        " %.*s",
        (int)len - (SWS_PPP_HDR_DATA + 4),
        (SWS_PTR(const char))pvHdr + SWS_PPP_HDR_DATA + 4
      );
    break;

  case SWS_PPP_TIMEREM: /* LCP extension RFC1570 */
    pszBuf += sprintf( pszBuf, " TIME-REM");
    if ( len >= SWS_PPP_HDR_DATA + 4)
      {
      SWS_PEEK32( magic, pvHdr, SWS_PPP_HDR_DATA);
      pszBuf += sprintf( pszBuf, "(%#lx)",magic);
      }
    if ( len >= SWS_PPP_HDR_DATA + 8)
      {
      SWS_PEEK32( magic, pvHdr, SWS_PPP_HDR_DATA);
      pszBuf += sprintf( pszBuf, ":%lu",magic);
      pszBuf += HexDump(
        pszBuf,
        (SWS_PTR(const char))pvHdr + SWS_PPP_HDR_DATA + 8,
        len - (SWS_PPP_HDR_DATA + 8)
      );
      }
    break;

  case SWS_PPP_CCP_RST_REQ: /* CCP RFC1962 */
    pszBuf += sprintf( pszBuf, " RST-REQ");
    break;

  case SWS_PPP_CCP_RST_RPY: /* CCP RFC1962 */
    pszBuf += sprintf( pszBuf, " RST-RPY");
    break;

  default:
    pszBuf += sprintf( pszBuf, " CODE-%u", code);
    pszBuf += HexDump(
      pszBuf, (SWS_PTR(const char))pvHdr + SWS_PPP_HDR_DATA, len - SWS_PPP_HDR_DATA);
    break;
    }

  return (int)(pszBuf - pszStart);
  }


/*
 * LCP config options dump
 * See RFC 1661 p39, RFC 1570 & 2153
 */
static int LcpOptDump(
  SWS_PTR( char) pszBuf,
  SWS_PTR( const void) pvHdr,
  SWS_size_t uLen
) {
  int out = 0;

  while ( uLen >= SWS_PPP_OPT_DATA)
    {
    ui8 type, len;
    ui16 data;
    ui32 magic;

    SWS_PEEK8( len, pvHdr, SWS_PPP_OPT_LEN);
    if ( len < SWS_PPP_OPT_DATA)
      break;
    if ( len > uLen)
      break;

    SWS_PEEK8( type, pvHdr, SWS_PPP_OPT_TYPE);
    switch ( type)
      {
    case SWS_PPP_LCP_VENDOR: /* RFC2153 */
      out += sprintf( pszBuf + out, " VENDOR");
      break;

    case SWS_PPP_LCP_MRU:
      SWS_PEEK16( data, pvHdr, SWS_PPP_OPT_DATA);
      out += sprintf( pszBuf + out, " MRU:%u", data);
      break;

    case SWS_PPP_LCP_ACCM:
      SWS_PEEK32( magic, pvHdr, SWS_PPP_OPT_DATA);
      out += sprintf( pszBuf + out, " ACCM:%#lx", magic);
      break;

    case SWS_PPP_LCP_AUTH:
      out += sprintf( pszBuf + out, " AUTH:");

      SWS_PEEK16( data, pvHdr, SWS_PPP_OPT_DATA);
      switch ( data)
        {
      case SWS_PPP_PAP:
        out += sprintf( pszBuf + out, "PAP");
        break;
      case SWS_PPP_CHAP:
        out += sprintf( pszBuf + out, "CHAP");
        break;
      default:
        out += sprintf( pszBuf + out, "%#x", data);
        break;
        }
      if ( len > SWS_PPP_HDR_DATA)
        out += HexDump( pszBuf + out, (SWS_PTR(const char))pvHdr + SWS_PPP_HDR_DATA, len - SWS_PPP_HDR_DATA);
      break;

    case SWS_PPP_LCP_QUAL:
      SWS_PEEK16( data, pvHdr, SWS_PPP_OPT_DATA);
      out += sprintf( pszBuf + out, " QUAL:%u", data);
      if ( len > SWS_PPP_HDR_DATA)
        out += HexDump( pszBuf + out, (SWS_PTR(const char))pvHdr + SWS_PPP_HDR_DATA, len - SWS_PPP_HDR_DATA);
      break;

    case SWS_PPP_LCP_MAGIC:
      SWS_PEEK32( magic, pvHdr, SWS_PPP_OPT_DATA);
      out += sprintf( pszBuf + out, " MAGIC(%08lx)", magic);
      break;

    case SWS_PPP_LCP_PFC:
      out += sprintf( pszBuf + out, " PFC");
      break;

    case SWS_PPP_LCP_ACFC:
      out += sprintf( pszBuf + out, " ACFC");
      break;

    case SWS_PPP_LCP_FCS: /* RFC1570 */
      SWS_PEEK8( data, pvHdr, SWS_PPP_OPT_DATA);
      out += sprintf( pszBuf + out, " FCS:%u", data);
      break;

    case SWS_PPP_LCP_PAD: /* RFC1570 */
      SWS_PEEK8( data, pvHdr, SWS_PPP_OPT_DATA);
      out += sprintf( pszBuf + out, " PAD:%u", data);
      break;

    case SWS_PPP_LCP_NUMMODE: /* RFC1663 */
      out += sprintf( pszBuf + out, " NUM-MODE");
      break;

    case SWS_PPP_LCP_CALLBACK: /* LCP extension RFC1570 */
      SWS_PEEK8( data, pvHdr, SWS_PPP_OPT_DATA);
      out += sprintf( pszBuf + out, " CALLBACK:%u", data);
      if ( len > 3)
        out += HexDump( pszBuf + out, (SWS_PTR(const char))pvHdr + 3, len - 3);
      break;

    case SWS_PPP_LCP_BACP: /* Link Discriminator for BACP RFC 2125 */
      SWS_PEEK16( data, pvHdr, SWS_PPP_OPT_DATA);
      out += sprintf( pszBuf + out, " BACP:%u", data);
      break;

    case SWS_PPP_LCP_ML_MRRU: /* RFC1717 Max-Receive-Reconstructed-Unit */
      SWS_PEEK16( data, pvHdr, SWS_PPP_OPT_DATA);
      out += sprintf( pszBuf + out, " ML_MRRU:%u", data);
      break;

    case SWS_PPP_LCP_ML_SSNH: /* RFC1717 Short Sequence Number  */
      SWS_PEEK16( data, pvHdr, SWS_PPP_OPT_DATA);
      out += sprintf( pszBuf + out, " ML_SSNH:%u", data);
      break;

    case SWS_PPP_LCP_ML_EPD:  /* RFC1717 */
      SWS_PEEK8( data, pvHdr, SWS_PPP_OPT_DATA);
      out += sprintf( pszBuf + out, " ML_EPD:%u", data);
      if ( s_iPppVerbosity >= 2)
        out += HexDump( pszBuf + out, (SWS_PTR(const char))pvHdr + 3, len - 3);
      break;

    case 28: /* Internationalization RFC2484 */
    default:
      out += sprintf( pszBuf + out, " OPT:%u", type);
      out += HexDump( pszBuf + out, (SWS_PTR(const char))pvHdr + 2, len - 2);
      break;
      }

    pvHdr = (SWS_PTR(const char))pvHdr + len;
    uLen -= len;
    }

  return out;
  }


/*
 * IPCP config options dump
 * See RFC 1332 & 1877
 */
static int IpcpOptDump(
  SWS_PTR( char) pszBuf,
  SWS_PTR( const void) pvHdr,
  SWS_size_t uLen
) {
  int out = 0;

  while ( uLen >= SWS_PPP_OPT_DATA)
    {
    ui8 type, len;
    ui16 data;
    ui32 addr;

    SWS_PEEK8( len, pvHdr, SWS_PPP_OPT_LEN);
    if ( len < SWS_PPP_OPT_DATA)
      break;
    if ( len > uLen)
      break;

    SWS_PEEK8( type, pvHdr, SWS_PPP_OPT_TYPE);
    switch ( type)
      {
    case SWS_PPP_IPCP_VENDOR:
      out += sprintf( pszBuf + out, " VENDOR");
      out += HexDump( pszBuf + out, (SWS_PTR(const char))pvHdr + 2, len - 2);
      break;

    case SWS_PPP_IPCP_IPADDRS:
      out += sprintf( pszBuf + out, " IP-ADDRS");
      out += HexDump( pszBuf + out, (SWS_PTR(const char))pvHdr + 2, len - 2);
      break;

    case SWS_PPP_IPCP_IPCOMP:
      out += sprintf( pszBuf + out, " IP-COMP");
      if ( len >= SWS_PPP_OPT_DATA + 2)
        {
        SWS_PEEK16( data, pvHdr, SWS_PPP_OPT_DATA);
        switch ( data)
          {
        case SWS_PPP_VJ:
          out += sprintf( pszBuf + out, ":VJ");
          break;
        default:
          out += sprintf( pszBuf + out, ":%#x", data);
          break;
          }
        }
      if ( len > SWS_PPP_OPT_DATA + 2)
        out += HexDump( pszBuf + out, (SWS_PTR(const char))pvHdr + 4, len - 4);
      break;

    case SWS_PPP_IPCP_IPADDR:
      out += sprintf( pszBuf + out, " IP-ADDR");
      if ( len >= SWS_PPP_OPT_DATA + 4)
        {
        SWS_PEEK32( addr, pvHdr, SWS_PPP_OPT_DATA);
        out += sprintf( pszBuf + out, ":%u.%u.%u.%u",
          (unsigned)((addr >> 24) & 0xff), (unsigned)((addr >> 16) & 0xff),
          (unsigned)((addr >> 8) & 0xff), (unsigned)(addr & 0xff)
        );
        }
      break;

    case SWS_PPP_IPCP_PRIDNS: /* RFC1877 */
      out += sprintf( pszBuf + out, " PRI-DNS");
      if ( len >= SWS_PPP_OPT_DATA + 4)
        {
        SWS_PEEK32( addr, pvHdr, SWS_PPP_OPT_DATA);
        out += sprintf( pszBuf + out, ":%u.%u.%u.%u",
          (unsigned)((addr >> 24) & 0xff), (unsigned)((addr >> 16) & 0xff),
          (unsigned)((addr >> 8) & 0xff), (unsigned)(addr & 0xff)
        );
        }
      break;

    case SWS_PPP_IPCP_PRINBNS: /* RFC1877 */
      out += sprintf( pszBuf + out, " PRI-NBNS");
      if ( len >= SWS_PPP_OPT_DATA + 4)
        {
        SWS_PEEK32( addr, pvHdr, SWS_PPP_OPT_DATA);
        out += sprintf( pszBuf + out, ":%u.%u.%u.%u",
          (unsigned)((addr >> 24) & 0xff), (unsigned)((addr >> 16) & 0xff),
          (unsigned)((addr >> 8) & 0xff), (unsigned)(addr & 0xff)
        );
        }
      break;

    case SWS_PPP_IPCP_SECDNS: /* RFC1877 */
      out += sprintf( pszBuf + out, " SEC-DNS");
      if ( len >= SWS_PPP_OPT_DATA + 4)
        {
        SWS_PEEK32( addr, pvHdr, SWS_PPP_OPT_DATA);
        out += sprintf( pszBuf + out, ":%u.%u.%u.%u",
          (unsigned)((addr >> 24) & 0xff), (unsigned)((addr >> 16) & 0xff),
          (unsigned)((addr >> 8) & 0xff), (unsigned)(addr & 0xff)
        );
        }
      break;

    case SWS_PPP_IPCP_SECNBNS: /* RFC1877 */
      out += sprintf( pszBuf + out, " SEC-NBNS");
      if ( len >= SWS_PPP_OPT_DATA + 4)
        {
        SWS_PEEK32( addr, pvHdr, SWS_PPP_OPT_DATA);
        out += sprintf( pszBuf + out, ":%u.%u.%u.%u",
          (unsigned)((addr >> 24) & 0xff), (unsigned)((addr >> 16) & 0xff),
          (unsigned)((addr >> 8) & 0xff), (unsigned)(addr & 0xff)
        );
        }
      break;

    default:
      out += sprintf( pszBuf + out, " OPT:%u", type);
      out += HexDump( pszBuf + out, (SWS_PTR(const char))pvHdr + 2, len - 2);
      break;
      }

    pvHdr = (SWS_PTR(const char))pvHdr + len;
    uLen -= len;
    }

  return out;
  }


/*
 * CCP config options dump
 * See RFC 1962
 */
static int CcpOptDump(
  SWS_PTR( char) pszBuf,
  SWS_PTR( const void) pvHdr,
  SWS_size_t uLen
) {
  int out = 0;

  while ( uLen >= SWS_PPP_OPT_DATA)
    {
    ui8 type, len;
    ui32 addr;

    SWS_PEEK8( len, pvHdr, SWS_PPP_OPT_LEN);
    if ( len < SWS_PPP_OPT_DATA)
      break;
    if ( len > uLen)
      break;

    SWS_PEEK8( type, pvHdr, SWS_PPP_OPT_TYPE);
    switch ( type)
      {
    case 0:
      out += sprintf( pszBuf + out, " OUI");
      if ( len >= 6)
        {
        SWS_PEEK32( addr, pvHdr, SWS_PPP_OPT_DATA);
        out += sprintf( pszBuf + out, ":%#lx", addr);
        len -= 4;
        pvHdr = (SWS_PTR(const char))pvHdr + 4;
        }
      break;

    case 1:
      out += sprintf( pszBuf + out, " PRED1");
      break;

    case 2:
      out += sprintf( pszBuf + out, " PRED2");
      break;

    case 3:
      out += sprintf( pszBuf + out, " PUDDLE-JUMPER");
      break;

    case 17:
      out += sprintf( pszBuf + out, " STAC-LZS");
      break;

    case 18:
      out += sprintf( pszBuf + out, " MSFT-PPC");
      break;

    case 19:
      out += sprintf( pszBuf + out, " GANDALF-FZA");
      break;

    case 20:
      out += sprintf( pszBuf + out, " V42-bis");
      break;

    case 21:
      out += sprintf( pszBuf + out, " BSD-LZW");
      break;

    default:
      out += sprintf( pszBuf + out, " TYPE:%u", type);
      break;
      }

    out += HexDump( pszBuf + out, (SWS_PTR(const char))pvHdr + 2, len - 2);

    pvHdr = (SWS_PTR(const char))pvHdr + len;
    uLen -= len;
    }

  return out;
  }


/*
 * CHAP packet dump
 * See RFC 1994
 */
static int ChapDump(
  SWS_PTR( char) pszBuf,
  SWS_PTR( const void) pvHdr,
  SWS_size_t uLen
) {
  SWS_size_t len;
  unsigned code, id;
  char* const pszStart = pszBuf;

  if ( NULL == pvHdr || uLen < SWS_PPP_HDR_DATA)
    return 0;

  /* Parse the header */
  SWS_PEEK8( id, pvHdr, SWS_PPP_HDR_ID);
  pszBuf += sprintf( pszBuf, ":%u", id);

  SWS_PEEK16( len, pvHdr, SWS_PPP_HDR_LEN);
  if ( len != uLen)
    {
    pszBuf += sprintf( pszBuf, " %u/%u", (unsigned)uLen, (unsigned)len);
    if ( len > uLen)
      len = uLen;
    }

  SWS_PEEK8( code, pvHdr, SWS_PPP_HDR_CODE);
  switch ( code)
    {
  case SWS_PPP_CHAP_CHALLENGE:
    pszBuf += sprintf( pszBuf, " CHALLENGE");
    goto resp;

  case SWS_PPP_CHAP_RESPONSE:
    pszBuf += sprintf( pszBuf, " RESPONSE");

  resp:
    if ( len >= SWS_PPP_HDR_DATA + 1)
      {
      /* len bytes */
      SWS_PEEK8( code, pvHdr, SWS_PPP_HDR_DATA);
      pszBuf += HexDump(
        pszBuf, (SWS_PTR(const char))pvHdr + SWS_PPP_HDR_DATA + 1, code);

      code += SWS_PPP_HDR_DATA + 1;
      if ( len > code)
        pszBuf += sprintf(
          pszBuf, " %.*s", (int)(len - code), (SWS_PTR(const char))pvHdr + code);
      }
    break;

  case SWS_PPP_CHAP_SUCCESS:
    pszBuf += sprintf( pszBuf, " SUCCESS");
    goto fail;

  case SWS_PPP_CHAP_FAILURE:
    pszBuf += sprintf( pszBuf, " FAILURE");

  fail:
    if ( len > SWS_PPP_HDR_DATA)
      pszBuf += sprintf(
        pszBuf, " %.*s",
        (int)len - SWS_PPP_HDR_DATA,
        (SWS_PTR(const char))pvHdr + SWS_PPP_HDR_DATA
      );
    break;

  default:
    pszBuf += sprintf( pszBuf, " CODE:%u", code);
    if ( len > SWS_PPP_HDR_DATA)
      pszBuf += HexDump(
        pszBuf, (SWS_PTR(const char))pvHdr + SWS_PPP_HDR_DATA, len - SWS_PPP_HDR_DATA);
    break;
    }

  return (int)(pszBuf - pszStart);
  }


/*
 * PAP packet dump
 * See RFC 1334
 */
static int PapDump(
  SWS_PTR( char) pszBuf,
  SWS_PTR( const void) pvHdr,
  SWS_size_t uLen
) {
  SWS_size_t len;
  unsigned code, id;
  char* const pszStart = pszBuf;

  if ( NULL == pvHdr || uLen < SWS_PPP_HDR_DATA)
    return 0;

  /* Parse the header */
  SWS_PEEK8( id, pvHdr, SWS_PPP_HDR_ID);
  pszBuf += sprintf( pszBuf, ":%u", id);

  SWS_PEEK16( len, pvHdr, SWS_PPP_HDR_LEN);
  if ( len != uLen)
    {
    pszBuf += sprintf( pszBuf, " %u/%u", (unsigned)uLen, (unsigned)len);
    if ( len > uLen)
      len = uLen;
    }

  SWS_PEEK8( code, pvHdr, SWS_PPP_HDR_CODE);
  switch ( code)
    {
  case SWS_PPP_PAP_REQ:
    pszBuf += sprintf( pszBuf, " REQ");
    if ( len >= SWS_PPP_HDR_DATA + 1)
      {
      /* Peer-ID Length */
      SWS_PEEK8( code, pvHdr, SWS_PPP_HDR_DATA);
      if ( len >= SWS_PPP_HDR_DATA + 1 + code)
        {
        /* Peer-ID */
        pszBuf += sprintf(
          pszBuf, " %.*s",
          (int)code,
          (SWS_PTR(const char))pvHdr + SWS_PPP_HDR_DATA + 1
        );
        }

      /* Passwd-Length */
      SWS_PEEK8( id, pvHdr, SWS_PPP_HDR_DATA + 1 + code);
      if ( len >= SWS_PPP_HDR_DATA + 2 + code + id)
        {
        /* Passwd */
        pszBuf += sprintf(
          pszBuf, " %.*s",
          (int)id,
          (SWS_PTR(const char))pvHdr + SWS_PPP_HDR_DATA + 2 + code
        );
        }
      }
    break;

  case SWS_PPP_PAP_ACK:
    pszBuf += sprintf( pszBuf, " ACK");
    goto resp;

  case SWS_PPP_PAP_NAK:
    pszBuf += sprintf( pszBuf, " NAK");

  resp:
    if ( len >= SWS_PPP_HDR_DATA + 1)
      {
      /* Msg-Length */
      SWS_PEEK8( code, pvHdr, SWS_PPP_HDR_DATA);
      if ( len >= SWS_PPP_HDR_DATA + 1 + code)
        {
        /* Msg */
        pszBuf += sprintf(
          pszBuf, " %.*s",
          (int)code,
          (SWS_PTR(const char))pvHdr + SWS_PPP_HDR_DATA + 1
        );
        }
      }
    break;

  default:
    pszBuf += sprintf( pszBuf, " CODE:%u", code);
    if ( len > SWS_PPP_HDR_DATA)
      pszBuf += HexDump(
        pszBuf, (SWS_PTR(const char))pvHdr + SWS_PPP_HDR_DATA, len - SWS_PPP_HDR_DATA);
    break;
    }

  return (int)(pszBuf - pszStart);
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

  if ( NULL != pv)
    {
    SWS_PTR( const unsigned char) p = (SWS_PTR( const unsigned char)) pv;

    while ( uLen--)
      out += sprintf( pszBuf + out, " %02X", *(p++) );
    }

  return out;
  }

/* End of file */
