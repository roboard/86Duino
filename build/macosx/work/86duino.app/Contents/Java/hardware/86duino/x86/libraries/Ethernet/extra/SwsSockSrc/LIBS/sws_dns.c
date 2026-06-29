/*****************************************************************************
 *
 * File name   : sws_dns.c
 * Function    : DNS client RFC 1034 and 1035 (STD13)
 * Authors     : Lawrence Rust
 * Systems     : ANSI C, BSD sockets
 *
 *****************************************************************************
 * File History
 * ------------
 *
 * 22-Nov-02, LVR, Created.
 * 28-Mar-05, LVR, Remove dependency upon PlatformPropertyGetUint
 * 29-Jun-05, LVR, Move SWS_DnsGet/SetServer to sockets.c
 * 08-Jul-09, LVR, Enable broadcasts if SWS_DNS_BB_MASK set
 *
 *
 * Comments
 * --------
 *
 *****************************************************************************/

/* Compilation options */
#ifndef SWS_USE_SOCKLIB
#define SWS_USE_SOCKLIB 0 /* 0= native sockets, !0 for sws_sock */
#endif

#if defined _MSC_VER && _MSC_VER <= 800 /* VC++ 1.5 & earlier */
# pragma warning( disable : 4705)   /* statement has no effect */
#endif

/* Exports */
#include "sws_dns.h"

/* Imports */

/* ANSI C library */
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <errno.h> /* For EINVAL, ENOMEM */
#if defined linux || defined __linux || defined __linux__
#include <ctype.h> /* isspace */
#endif

#if defined WIN32 || defined _WIN32 || defined __WIN32__
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <iphlpapi.h> /* GetNetworkParams - DNS server */
#if defined _MSC_VER
#pragma comment( lib, "iphlpapi.lib") /* For GetNetworkParams */
#pragma comment( lib, "delayimp.lib") /* Required for delay loading */
#pragma comment( linker, "/delayload:iphlpapi.dll") /* Delayload iphlpapi.dll */
#endif
#endif

#include "sws_net.h"
#include "sws_cfg.h"


/***************************************************************************
 * Macros & constants
 ***************************************************************************/

/* Read/write in network (MSB first, or big-endian) byte order */
#ifndef SWS_POKE8
#define SWS_POKE8(_p,_o,_v) (*(SWS_PTR(ui8))((SWS_PTR(char))(_p) + (_o)) = (_v))
#define SWS_POKE16(_p,_o,_v) (*(SWS_PTR(SWS_PACKED(ui16)))((SWS_PTR(char))(_p) + (_o)) = htons(_v))
#define SWS_POKE32(_p,_o,_v) (*(SWS_PTR(SWS_PACKED(ui32)))((SWS_PTR(char))(_p) + (_o)) = htonl(_v))
#define SWS_PEEK8(_v,_p,_o) _v = (*(SWS_PTR(const ui8))((SWS_PTR(const char))(_p) + (_o)))
#define SWS_PEEK16(_v,_p,_o) _v = ntohs(*(SWS_PTR(SWS_PACKED(const ui16)))((SWS_PTR(const char))(_p) + (_o)))
#define SWS_PEEK32(_v,_p,_o) _v = ntohl(*(SWS_PTR(SWS_PACKED(const ui32)))((SWS_PTR(const char))(_p) + (_o)))
#endif

#ifndef SWS_MAKEFOURCC
#define SWS_MAKEFOURCC( ch0, ch1, ch2, ch3) (\
  (ui32)(ui8)(ch0) | ((ui32)(ui8)(ch1) << 8) | \
  ((ui32)(ui8)(ch2) << 16) | ((ui32)(ui8)(ch3) << 24) )
#endif

#ifndef DNS_SEND_TIMEOUT
#define DNS_SEND_TIMEOUT 2000 /* Milliseconds */
#endif
#ifndef DNS_RECV_TIMEOUT
#define DNS_RECV_TIMEOUT 1000 /* Milliseconds */
#endif
#ifndef DNS_RETRIES
#define DNS_RETRIES 4
#endif

#define kszLocalHost "localhost"


/***************************************************************************
 * Data types
 ***************************************************************************/

#define QUERY_MAGIC SWS_MAKEFOURCC('d','n','s','q')
#define IsValidQuery( h) (NULL != h && QUERY_MAGIC == h->magic)

typedef struct SWS_DnsSQuery
  {
  ui32 magic; /* QUERY_MAGIC */

  /* Query variables */
  ui16 op;    /* Query opcode */
  ui16 qclass;
  ui16 qtype;
  ui16 id;

  struct sockaddr_in to;
  int iTry;
  unsigned timeout; /* Receive timeout in milliseconds */

  /* Next record to read */
  unsigned uRecord;
  size_t index;

  /* Query data */
  size_t len;    /* Reply size */
  ui8 buf[ 512]; /* Max DNS message is 512 bytes */
  } SWS_DnsSQuery;


/***************************************************************************
 * Local function prototypes
 ***************************************************************************/
static size_t HostToDns( ui8*, const char*);
static size_t DnsToHost( char*, SWS_PTR(const ui8), size_t);


/***************************************************************************
 * Module data
 ***************************************************************************/

static ui16 s_id; /* Query ID */


/***************************************************************************
 * Exported functions
 ***************************************************************************/

/*
 * Issue a DNS query
 */
SWS_EXTERN( SWS_DnsQHandle, SWS_DnsOpenQuery(
  unsigned uOp,                       /* IN, DNS opcode */
  SWS_PTR( const char) pszName,       /* IN: Query text */
  unsigned uType,                     /* IN: Query type */
  unsigned uClass,                    /* IN: Query class */
  ui32 server,                        /* IN: Preferred server or INADDR_ANY */
  unsigned port                       /* IN: Port. 0= default */
)){
  int iRet;
  SWS_DnsQHandle hQuery;
  SOCKET s;

  if ( INADDR_ANY == server)
    server = SWS_DnsDefaultServer();
  if ( INADDR_ANY == server || INADDR_NONE == server)
    return NULL;

  if ( 0 == port)
    port = SWS_DNS_SERVER;

  /* Allocate a query handle */
  hQuery = (SWS_DnsQHandle) malloc( sizeof( SWS_DnsSQuery));
  if ( NULL == hQuery)
    return NULL;

  hQuery->magic = QUERY_MAGIC;
  hQuery->op = (ui16)uOp;
  hQuery->qclass = (ui16)uClass;
  hQuery->qtype = (ui16)uType;
  hQuery->id = ++s_id; /* Bump the query ID */
  hQuery->to.sin_family = AF_INET;
  hQuery->to.sin_addr.s_addr = htonl( server);
  hQuery->to.sin_port = htons( (unsigned short)port);
  hQuery->iTry = 0;
  hQuery->timeout = DNS_RECV_TIMEOUT; /* initial receive timeout */
  hQuery->uRecord = 0;
  hQuery->index = 0;
  hQuery->len = 0;

  /* Create a socket */
  s = socket( PF_INET, SOCK_DGRAM, 0);
  if ( INVALID_SOCKET == s)
    {
    iRet = GetLastError();
#if 1
    hQuery->op = SWS_DNS_RC_NWERR;
    return hQuery;
#else
    SWS_DnsCloseQuery( hQuery);
    return NULL;
#endif
    }

  /* Set a send timeout */
  { DECL_TIMEOUT( tv, DNS_SEND_TIMEOUT);
  setsockopt(
    s,
    SOL_SOCKET, SO_SNDTIMEO,
    (char*)&tv, sizeof tv
  ); }

#ifdef IP_TOS
  /* Set the TOS */
  iRet = 0x50; /* Precedence: immediate, low delay */
  setsockopt(
    s,
    IPPROTO_IP, IP_TOS,
    (char*)&iRet, sizeof(iRet)
  );
#endif

#ifdef IP_DONTFRAGMENT
  /* Set the DF flag */
  iRet = 1;
  setsockopt(
    s,
    IPPROTO_IP, IP_DONTFRAGMENT,
    (char*)&iRet, sizeof(iRet)
  );
#endif

  if ( SWS_DNS_BB_MASK & uOp)
    {
    /* Enable broadcasts */
    iRet = 1;
    setsockopt(
      s,
      SOL_SOCKET, SO_BROADCAST,
      (char*)&iRet, sizeof iRet
    );
    }

  iRet = ETIMEDOUT; /* Assume retries exceeded */
  for ( hQuery->iTry = 0; hQuery->iTry <= DNS_RETRIES; ++(hQuery->iTry) )
    {
    size_t offset;
    unsigned uVal;
    int count;

    /* Format DNS message header */
    SWS_POKE16( hQuery->buf, SWS_DNS_HDR_ID, hQuery->id);
    SWS_POKE16( hQuery->buf, SWS_DNS_HDR_OP, hQuery->op);
    SWS_POKE16( hQuery->buf, SWS_DNS_HDR_QDCOUNT, 1);
    SWS_POKE16( hQuery->buf, SWS_DNS_HDR_ANCOUNT, 0);
    SWS_POKE16( hQuery->buf, SWS_DNS_HDR_NSCOUNT, 0);
    SWS_POKE16( hQuery->buf, SWS_DNS_HDR_ARCOUNT, 0);
    offset = SWS_DNS_HDR_LEN;

    /* Append DNS question */
    offset += HostToDns( hQuery->buf + offset, pszName);

    SWS_POKE16( hQuery->buf, SWS_DNS_QTYPE + offset, hQuery->qtype);
    SWS_POKE16( hQuery->buf, SWS_DNS_QCLASS + offset, hQuery->qclass);
    offset += SWS_DNS_QLEN;

    /* Send the request */
    if ( SOCKET_ERROR == sendto(
        s,
        (const char*)hQuery->buf, offset,
        0,
        (struct sockaddr*)&hQuery->to, sizeof( hQuery->to)
      )
    ) {
      iRet = GetLastError();
      if ( ENETUNREACH == iRet
        || EHOSTUNREACH == iRet
      ) {
        /* ICMP net/host unreachable */
        break;
        }
      continue;
      }

    /* Set recv timeout */
    { DECL_TIMEOUT( tv, hQuery->timeout);
    setsockopt(
      s,
      SOL_SOCKET, SO_RCVTIMEO,
      (char*)&tv, sizeof tv
    ); }

    /* Get the reply */
    count = recv( s, (char*)hQuery->buf, sizeof( hQuery->buf), 0);
    if ( SOCKET_ERROR == count)
      {
      iRet = GetLastError();
      if ( ETIMEDOUT == iRet)
        {
        hQuery->timeout *= 2; /* Exponential backoff */
        }
      else if ( ECONNRESET == iRet)
        {
        /* ICMP port unreachable */
        break;
        }

      continue;
      }

    /* Ensure header present */
    if ( (unsigned)count < SWS_DNS_HDR_LEN)
      continue;

    /* Check matching ID */
    SWS_PEEK16( uVal, hQuery->buf, SWS_DNS_HDR_ID);
    if ( uVal != hQuery->id)
      continue;

    /* Verify it's a response */
    SWS_PEEK16( uVal, hQuery->buf, SWS_DNS_HDR_OP);
    if ( !(SWS_DNS_QR_MASK & uVal))
      continue;

    hQuery->len = (size_t)count;
    iRet = SWS_DnsQueryRewind( hQuery);
    break;
    }

  closesocket( s);

  switch ( iRet)
    {
  case 0:
    /* Save response code */
    SWS_PEEK16( hQuery->op, hQuery->buf, SWS_DNS_HDR_OP);
    break;
  case ETIMEDOUT:
    hQuery->op = SWS_DNS_RC_NOREPLY;
    break;
  default:
    hQuery->op = SWS_DNS_RC_NWERR;
    break;
    }

  return hQuery;
  }


/*
 * Free a DNS query
 */
SWS_EXTERN( int, SWS_DnsCloseQuery(
  SWS_DnsQHandle hQuery              /* IN, DNS query handle */
)){
  if ( !IsValidQuery( hQuery))
    return EINVAL;

  free( hQuery);
  return 0;
  }


/*
 * Get DNS query result code
 */
SWS_EXTERN( int, SWS_DnsQueryRcode(
  SWS_DnsQHandle hQuery              /* IN, DNS query handle */
)){
  if ( !IsValidQuery( hQuery))
    return EINVAL;

  return hQuery->op;
  }


/*
 * Rewind query to the first record
 */
SWS_EXTERN( int, SWS_DnsQueryRewind(
  SWS_DnsQHandle hQuery               /* IN, DNS query handle */
)){
  unsigned uRecs;
  size_t offset;

  if ( !IsValidQuery( hQuery))
    return EINVAL;

  offset = SWS_DNS_HDR_LEN;

  /* Skip over question section */
  SWS_PEEK16( uRecs, hQuery->buf, SWS_DNS_HDR_QDCOUNT);
  while ( uRecs-- > 0)
    {
    /* Skip name */
    offset = DnsToHost( NULL, hQuery->buf, offset);
    offset += SWS_DNS_QLEN;
    if ( offset > hQuery->len)
      {
      return EINVAL;
      }
    }

  hQuery->index = offset;
  hQuery->uRecord = 0;

  return 0;
  }


/*
 * Get next resource record
 */
SWS_EXTERN( int, SWS_DnsQueryRead(
  SWS_DnsQHandle hQuery,              /* IN, DNS query handle */
  SWS_PTR( SWS_SDnsQRecord) pRec      /* OUT */
)){
  unsigned uVal, uRec;
  size_t offset;

  if ( !IsValidQuery( hQuery))
    return EINVAL;

  /* What type of rexcord? */
  uRec = hQuery->uRecord;

  SWS_PEEK16( uVal, hQuery->buf, SWS_DNS_HDR_ANCOUNT);
  if ( hQuery->uRecord < uVal)
    {
    pRec->eRecordType = SWS_kDnsAnswer;
    }
  else
    {
    uRec -= uVal;

    SWS_PEEK16( uVal, hQuery->buf, SWS_DNS_HDR_NSCOUNT);
    if ( uRec < uVal)
      {
      pRec->eRecordType = SWS_kDnsAuthority;
      }
    else
      {
      uRec -= uVal;

      SWS_PEEK16( uVal, hQuery->buf, SWS_DNS_HDR_ARCOUNT);
      if ( uRec < uVal)
        {
        pRec->eRecordType = SWS_kDnsAdditional;
        }
      else
        {
        /* No more */
        return ENOMEM;
        }
      }
    }

  /* Return record info */
  offset = hQuery->index;

  pRec->nameIndex = (ui16)offset;
  offset = DnsToHost( NULL, hQuery->buf, offset);

  SWS_PEEK16( pRec->rtype, hQuery->buf, SWS_DNS_RR_TYPE + offset);
  SWS_PEEK16( pRec->rclass, hQuery->buf, SWS_DNS_RR_CLASS + offset);
  SWS_PEEK32( pRec->ttl, hQuery->buf, SWS_DNS_RR_TTL + offset);
  SWS_PEEK16( pRec->rdlen, hQuery->buf, SWS_DNS_RR_RDLEN + offset);
  pRec->dataIndex = (ui16)(SWS_DNS_RR_RDATA + offset);

  offset += SWS_DNS_RR_RDATA + pRec->rdlen;
  if ( offset > hQuery->len)
    {
    return EINVAL;
    }

  hQuery->index = offset;
  ++(hQuery->uRecord);
  return 0;
  }


/*
 * Get DNS query name
 */
SWS_EXTERN( unsigned, SWS_DnsQueryName(
  SWS_PTR( char) psz,                 /* OUT: ASCIIZ name */
  size_t size,                        /* IN: buffer size */
  SWS_DnsQHandle hQuery,              /* IN, DNS query handle */
  unsigned uIndex                     /* IN: Start index */
)){
  (void)size;

  if ( !IsValidQuery( hQuery))
    return 0;

  if ( uIndex >= hQuery->len)
    {
    return 0;
    }

  return DnsToHost( psz, hQuery->buf, uIndex);
  }


/*
 * Get DNS query data
 */
SWS_EXTERN( void, SWS_DnsQueryData(
  SWS_PTR( void) pv,                  /* OUT: Data */
  size_t size,                        /* IN: buffer size */
  SWS_DnsQHandle hQuery,              /* IN, DNS query handle */
  unsigned uIndex                     /* IN: Start index */
)){
  if ( IsValidQuery( hQuery))
    memcpy( pv, hQuery->buf + uIndex, size);
  }


/***************************************************************************
 * Local support functions
 ***************************************************************************/

/*
 * Convert ASCIIZ hostname to DNS form
 */
size_t HostToDns( ui8* pBuf, const char* pszHost)
  {
  size_t count = 0;

  for ( ;;)
    {
    size_t len;
    const char* psz;

    psz = strchr( pszHost, '.');
    if ( NULL != psz)
      len = (size_t)(psz - pszHost);
    else
      len = strlen( pszHost);

    /* Prefix string with a count */
    pBuf[ count++] = (ui8)len;
    if ( 0 == len)
      break;

    memcpy( pBuf + count, pszHost, len);
    count += len;
    pszHost += len;
    if ( NULL != psz)
      ++pszHost;
    }

  return count;
  }


/*
 * Convert DNS name to ASCIIZ
 */
size_t DnsToHost( char* pszBuf, SWS_PTR( const ui8) pBuf, size_t offset)
  {
  unsigned len;

  if ( NULL != pszBuf)
    *pszBuf = '\0';

  SWS_PEEK8( len, pBuf, offset);

  while ( len > 0)
    {
    if ( len < 64) /* Top 2 bits indicate ptr */
      {
      /* Length byte */
      const char* pszName = (const char*)pBuf + offset + 1;
      offset += 1 + len;
      if ( NULL != pszBuf)
        pszBuf += sprintf( pszBuf, "%.*s", (int)len, pszName);

      SWS_PEEK8( len, pBuf, offset);
      if ( len > 0 && NULL != pszBuf)
        *(pszBuf++) = '.';
      }
    else
      {
      /* pointer */
      if ( NULL != pszBuf)
        {
        SWS_PEEK16( len, pBuf, offset);
        len &= 0x3fff;
        DnsToHost( pszBuf, pBuf, len);
        }
      return offset + 2;
      }
    }

  return ++offset;
  }


/*
 * Get the default DNS server
 */
SWS_EXTERN( ui32, SWS_DnsDefaultServer( void))
  {
  static ui32 s_server = INADDR_NONE;
  ui32 val;

#if SWS_USE_SOCKLIB
  if ( INADDR_NONE == s_server)
    {
    val = SWS_DnsGetPriServer();
    if ( INADDR_ANY != val && INADDR_NONE != val)
      s_server = val;
    }

#elif defined WIN32 || defined _WIN32 || defined __WIN32__
  if ( INADDR_NONE == s_server)
    {
    UINT const mode = SetErrorMode( SEM_FAILCRITICALERRORS);

#ifdef _MSC_VER
    /* catch DLL not found exception if Iphlpapi.lib is delay loaded */
    __try
      {
#endif
      char buf[ sizeof( FIXED_INFO) + 0x100];
      PFIXED_INFO pInfo = (PFIXED_INFO)buf;
      ULONG size = sizeof( buf);

      /* NB This can take a few seconds on a slow (<=200MHz) PC */
      if ( ERROR_SUCCESS == GetNetworkParams( pInfo, &size) )
        {
        /* Use last DNS server in list (most recently added) */
        PIP_ADDR_STRING pAddr = &pInfo->DnsServerList;
        while ( pAddr->Next)
          pAddr = pAddr->Next;

        val = inet_addr( pAddr->IpAddress.String);
        if ( INADDR_NONE != val && INADDR_ANY != val)
          s_server = ntohl( val);
        }
#ifdef _MSC_VER
      }
    __except( EXCEPTION_EXECUTE_HANDLER)
      {
      }
#endif

    SetErrorMode( mode);
    }

#elif defined linux || defined __linux || defined __linux__
  if ( INADDR_NONE == s_server)
    {
    FILE* file = fopen( "/etc/resolv.conf", "r");
    if ( NULL != file)
      {
      char szBuf[ 160];

      /* Read the file one line at a time */
      while ( NULL != fgets( szBuf, sizeof szBuf, file))
        {
        /* Search for a line: nameserver w.x.y.z */
        static const char kszTag[] = "nameserver";
        if ( 0 == strncasecmp( kszTag, szBuf, sizeof kszTag -1))
          {
          const char* psz = szBuf + sizeof kszTag -1;
          while ( isspace( *psz))
            ++psz;

          val = inet_addr( psz);
          if ( INADDR_NONE != val)
            {
            s_server = ntohl( val);
            break;
            }
          }
        }

      fclose( file);
      }
    }

#else
  if ( INADDR_NONE == s_server)
    {
    char szBuf[ 16]; /* Max len dotted ipv4 address */

    /* Get value as a string */
    if ( 0 != SWS_CfgGetString( "dns", "server", "", szBuf, sizeof( szBuf) ))
      {
      /* Try a dotted IP address */
      val = inet_addr( szBuf);
      if ( INADDR_NONE != val)
        s_server = ntohl( val);
      }
    }
#endif

  return s_server;
  }

/* End of file */
