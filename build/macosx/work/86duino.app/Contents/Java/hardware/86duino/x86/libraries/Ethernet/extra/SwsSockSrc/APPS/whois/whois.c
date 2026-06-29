/*****************************************************************************
 *
 * File name   : whois.c
 * Function    : WHOIS client (RFC 3912 & 954)
 * Authors     : Lawrence Rust
 * Systems     : ANSI C
 * Release Date: 03-Mar-03
 *
 *****************************************************************************
 * File History
 * ------------
 *
 * 29-Nov-02, LVR, Created.
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

/* Exports */
#if !SWS_USE_SOCKLIB
#include "whois.h"
#endif

/* Imports */

/* ANSI C library */
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include <ctype.h>
#include <assert.h>

/* Platform */
#include "sws_net.h"
#include "whois.h"

/* Utils */
#include "sws_str.h"   /* SWS_stricmp */
#include "sws_cfg.h"


/***************************************************************************
 * Macros & constants
 ***************************************************************************/

/* Compile time assertion
 * Generates 'invalid array dimension' compiler error if condition false
 */
#undef STATIC_ASSERT
#ifdef __GNUC__
# define STATIC_ASSERT(e) extern char _dummy[(e) ? 1 : -1] __attribute__((unused))
#else
# define STATIC_ASSERT(e) extern char _dummy[(e) ? 1 : -1]
#endif

#define SEND_TIMEOUT 2000 /* Milliseconds */
#define RECV_TIMEOUT 60   /* Seconds */

#ifndef MSG_NOSIGNAL
#define MSG_NOSIGNAL 0
#endif

#define EINTERRUPTED 11000

#define cNul       '\0'
#define cReturn    '\x0d'
#define cLinefeed  '\x0a'

#define CRLF "\x0d\x0a"

/* Ini file entries */
#define kszWhoisSect "whois"
#define kszServer    "server"
#define kszServerDefault "whois.ripe.net"
#define kszWhoisTlds "whois_tld"
#define kszWhoisIpv4 "whois_ipv4"


/***************************************************************************
 * Data types
 ***************************************************************************/

/* SWS_CfgEnum callback context */
typedef struct SInfo
  {
  char* pszHost;
  size_t size;
  const char* pszQuery;
  u_long addr;
  } SInfo;


/***************************************************************************
 * Local function prototypes
 ***************************************************************************/

static int GetWhoisServer( char*, size_t, const char*);
static SWS_FCfgCallback WhoisName;
static SWS_FCfgCallback WhoisAddr;

static int ReadLine( SOCKET, char [], size_t, unsigned);
static int ConnectAsync( SOCKET, struct sockaddr*, int);
static int RawRead( SOCKET, void*, size_t, unsigned);
static int RawWrite( SOCKET, const void*, size_t);


/***************************************************************************
 * Module data
 ***************************************************************************/

extern sig_atomic_t volatile g_exit;


/***************************************************************************
 * Functions
 ***************************************************************************/

/*
 * Get whois info
 */
int WhoisQuery(                       /* Returns !0 on error */
  FILE* outfile,                      /* OUT: response */
  const char* pszQuery,               /* IN: Query */
  int iVerbosity,                     /* IN: stderr verbosity */
  struct sockaddr* pSock              /* I/O: whois server */
) {
  SOCKET s;
  int iRet;
  char szBuf[ 256];

  assert( NULL != outfile);
  assert( NULL != pszQuery);
  assert( NULL != pSock);

  /* Whois server specified? */
  if ( AF_UNSPEC == pSock->sa_family)
    {
    struct hostent* pHost;
    u_long addr;

    /* No, select the appropriate whois server */
    GetWhoisServer( szBuf, sizeof szBuf, pszQuery);
    if ( iVerbosity > 1)
      fprintf( stderr, "Referring to %s ", szBuf);

    /* Try an IP address */
    addr = inet_addr( szBuf);
    if ( INADDR_ANY == addr)
      {
      /* Empty or 0 address */
      return EADDRNOTAVAIL;
      }
    else if ( INADDR_NONE != addr)
      {
      /* Valid IPv4 address */
      ((struct sockaddr_in*)pSock)->sin_addr.s_addr = addr;
      pSock->sa_family = AF_INET;
      }
    /* Try a hostname */
    else if ( NULL != (pHost = gethostbyname( szBuf)) )
      {
      /* Valid hostname */
      pSock->sa_family = pHost->h_addrtype;
      memcpy( &(((struct sockaddr_in*)pSock)->sin_addr), pHost->h_addr, pHost->h_length);
      }
    else
      {
      return EHOST_NOT_FOUND;
      }
    }

  /* Server port specifed? */
  if ( AF_INET == pSock->sa_family
    && 0 == ((struct sockaddr_in*)pSock)->sin_port
  ) {
    /* No, set default */
    ((struct sockaddr_in*)pSock)->sin_port = htons( WHOIS_SERVER);
    }

  /* Create a socket */
  s = socket( pSock->sa_family, SOCK_STREAM, 0);
  if ( INVALID_SOCKET == s)
    return GetLastError();

  /* Set a send timeout */
  { DECL_TIMEOUT( tv, SEND_TIMEOUT);
  setsockopt(
    s,
    SOL_SOCKET, SO_SNDTIMEO,
    (char*)&tv, sizeof tv
  ); }

  if ( iVerbosity > 1)
    {
    fprintf( stderr, "%s:%u...",
      inet_ntoa( ((struct sockaddr_in*)pSock)->sin_addr),
      ntohs( ((struct sockaddr_in*)pSock)->sin_port)
    );
    }

  /* Connect to server */
  iRet = ConnectAsync( s, pSock, iVerbosity > 0);
  if ( 0 != iRet)
    {
    switch ( iRet)
      {
    case EINTERRUPTED:
      fputs( "Interrupted.\n", stderr);
      break;
    case ETIMEDOUT:
    case EWOULDBLOCK:
#if EAGAIN != EWOULDBLOCK
    case EAGAIN:
#endif
      fputs( "Host not responding.\n", stderr);
      break;
    case ECONNREFUSED:
      fputs( "Not a whois server.\n", stderr);
      break;
      }

    closesocket( s);
    return iRet;
    }
  if ( iVerbosity > 1)
    {
    fputs( " ok\n", stderr);
    fprintf( outfile, "Whois: %s\n\n", pszQuery);
    }

  /* Send query */
  iRet = sprintf( szBuf, "%.253s" CRLF, pszQuery);
  RawWrite( s, szBuf, iRet);

  /* Get response */
  while ( 0 != (iRet = ReadLine( s, szBuf, sizeof szBuf, RECV_TIMEOUT)) )
    {
    if ( SOCKET_ERROR == iRet)
      {
      iRet = GetLastError();
      break;
      }

    /* Check for broken output pipe */
    if ( ferror( stdout))
      {
      iRet = 1;
      break;
      }

    /* Copy to output */
    fputs( szBuf, outfile);
    }

  /* Disconnect server */
  closesocket( s);
  return iRet;
  }


/*
 * Get the best whois server for an address
 */
int GetWhoisServer( char* pszHost, size_t size, const char* pszQuery)
  {
  SInfo info;
  int iRet;

  info.pszHost = pszHost;
  info.size = size;
  info.pszQuery = pszQuery;

  /* Is the query an IP address? */
  info.addr = ntohl( inet_addr( pszQuery));

  /* Enumerate TLD's or IPv4 nets */
  iRet = SWS_CfgEnum(
    INADDR_NONE == info.addr ? kszWhoisTlds : kszWhoisIpv4,
    INADDR_NONE == info.addr ? &WhoisName : &WhoisAddr,
    &info
  );

  /* Matched? */
  if ( 0 == iRet)
    {
    /* No, use default */
    iRet = SWS_CfgGetString( kszWhoisSect, kszServer, kszServerDefault, pszHost, size);
    }

  return iRet > 0;
  }

/* Match query with TLD's */
SWS_CB( int, WhoisName(
  SWS_PTR( void) pv,                  /* IN: Context passed to SWS_CfgEnum */
  SWS_PTR( const char) psz            /* IN: Configuration string */
)){
  SInfo* pInfo = (SInfo*)pv;
  char szTLD[ 32];

  /* Comments begin with # */
  if ( '#' == *psz)
    return 0;

  /* Entries are formatted: [.x].tld whois.server */
  { STATIC_ASSERT( sizeof szTLD > 31); }
  assert( pInfo->size > 255);
  if ( 2 == sscanf( psz, " %31s %255s", szTLD, pInfo->pszHost) )
    {
    size_t len1, len2;
    len1 = strlen( pInfo->pszQuery);
    len2 = strlen( szTLD);
    if ( len1 >= len2
      && 0 == SWS_stricmp( szTLD, pInfo->pszQuery + (len1 - len2) )
    ) {
      return 1;
      }
    }

  return 0;
  }

/* Match query with IPv4 nets */
SWS_CB( int, WhoisAddr(
  SWS_PTR( void) pv,                  /* IN: Context passed to SWS_CfgEnum */
  SWS_PTR( const char) psz            /* IN: Configuration string */
)){
  SInfo* pInfo = (SInfo*)pv;
  char szIP[ 16];
  unsigned uCIDR;

  /* Comments begin with # */
  if ( '#' == *psz)
    return 0;

  /* Entries are formatted: w.x.y.z/n whois.server */
  { STATIC_ASSERT( sizeof szIP > 15); }
  assert( pInfo->size > 255);
  if ( 3 == sscanf( psz, " %15[0123456789.] / %u %255s", szIP, &uCIDR, pInfo->pszHost) )
    {
    u_long addr = ntohl( inet_addr( szIP));
    u_long mask = 0xffffffffL << (32 - uCIDR);

    if ( (pInfo->addr & mask) == addr)
      return 1;
    }

  return 0;
  }


/*
 * Read a line
 */
static int ReadLine(
  SOCKET s,
  char pBuf[],
  size_t size,
  unsigned timeout
) {
  size_t count = 0;
  int iRet = 0;
  int gotCR = 0;

  while ( count + 1 < size)
    {
    unsigned char c;

    iRet = RawRead( s, &c, sizeof( c), timeout);
    if ( SOCKET_ERROR == iRet)
      break;
    if ( 0 == iRet)
      break; /* Disconnected */

    if ( gotCR)
      {
      gotCR = 0;
      switch ( c)
        {
      case cNul:
      case cLinefeed:
        pBuf[ count++ ] = '\n';
        pBuf[ count] = cNul;
        iRet = count;
        size = 0; /* Force exit */
        break;

      default:
        if ( count + 3 <= size )
          {
          pBuf[ count++] = cReturn;
          pBuf[ count++] = (char)c;
          }
        break;
        }
      }
    else switch ( c)
      {
    default:
      if ( isprint( c) && count + 2 <= size)
        pBuf[ count++] = (char)c;
      break;

    case cLinefeed:
      pBuf[ count++ ] = '\n';
      pBuf[ count] = cNul;
      iRet = count;
      size = 0; /* Force exit */
      break;

    case cReturn:
      gotCR = 1;
      break;
      }
    }

  return iRet;
  }


/*
 * Non-blocking read data
 */
static int RawRead(
  SOCKET s,
  void* pv,
  size_t size,
  unsigned timeout
) {
  time_t const t = time( NULL) + timeout;

  /* Until SIGINT or timeout */
  while ( !g_exit
    && time( NULL) <= t
  ) {
    fd_set set;
    int iRet;
    struct timeval tval;

    FD_ZERO( &set);
    FD_SET( s, &set);

    /* Set timeout for select */
    tval.tv_usec = 333 * 1000UL;
    tval.tv_sec = 0;

    /* Wait for a receive event or timeout */
    iRet = select( FD_SETSIZE, &set, NULL, NULL, &tval);
    if ( 0 == iRet)
      continue; /* Timeout */

    if ( SOCKET_ERROR == iRet)
      return SOCKET_ERROR;

    /* Process receive event */
    if ( FD_ISSET( s, &set))
      {
      iRet = recv( s, (char*)pv, size, 0);
      if ( SOCKET_ERROR == iRet)
        {
        int err = GetLastError();
        switch ( err)
          {
        case ECONNRESET:
          fprintf( stderr, "\nConnection reset in read\n" );
          break;
        case ECONNABORTED:
          fprintf( stderr, "\nConnection aborted in read\n" );
          break;
        case ETIMEDOUT:
          fprintf( stderr, "\nConnection timed out reading\n" );
          break;
        case EWOULDBLOCK:
#if EAGAIN != EWOULDBLOCK
        case EAGAIN:
#endif
          return 0;
        default:
          fprintf( stderr, "\nRead failed (%d): %s\n", err, strerror( err));
          break;
          }
        }
      return iRet;
      }
    }

  return SOCKET_ERROR;
  }


/*
 * Write raw data
 */
static int RawWrite(
  SOCKET s,
  const void* pv,
  size_t size
) {
  int iRet;

  iRet = send( s, (const char*)pv, size, MSG_NOSIGNAL);
  if ( SOCKET_ERROR == iRet)
    {
    int err = GetLastError();
    switch ( err)
      {
    case ECONNRESET:
      fprintf( stderr, "\nConnection reset while writing\n" );
      break;
    case ECONNABORTED:
      fprintf( stderr, "\nConnection aborted while writing\n" );
      break;
    case ETIMEDOUT:
      fprintf( stderr, "\nConnection timed out while writing\n" );
      break;
    case EWOULDBLOCK:
#if EAGAIN != EWOULDBLOCK
    case EAGAIN:
#endif
      /* Win98se sometimes returns EWOULDBLOCK even though select()
      * previously indicated socket is ready to be written */
      return 0;
    default:
      fprintf( stderr, "\nSend failed (%d): %s\n", err, strerror( err));
      break;
      }
    }

  return iRet;
  }


/*
 * Non-blocking connect
 */
int ConnectAsync(
  SOCKET s,
  struct sockaddr* pTo,
  int isInteractive
) {
  int iRet = 0;

  /* Enable non-blocking mode */
  setsockasync( s, 1);

  if ( connect( s, pTo, sizeof( *pTo) )
    && (EWOULDBLOCK == (iRet = GetLastError()) || EINPROGRESS == iRet || EAGAIN == iRet)
  ) {
    int count;

    for ( count = 0; ; ++count)
      {
      fd_set setw, sete;
      struct timeval tval;

      /* Check for SIGINT or broken output pipe */
      if ( g_exit || ferror( stdout))
        {
        iRet = EINTERRUPTED;
        break;
        }

      FD_ZERO( &setw);
      FD_SET( s, &setw);
      FD_ZERO( &sete);
      FD_SET( s, &sete);

      /* Set timeout for select */
      tval.tv_usec = 333 * 1000UL;
      tval.tv_sec = 0;

      /* Wait for a connect, error or timeout */
      iRet = select( FD_SETSIZE, NULL, &setw, &sete, &tval);
      if ( 0 == iRet)
        {
        /* Timeout */
        if ( isInteractive)
          {
          static const char progress[] = "|/-\\";
          fprintf( stderr, "%c\b", progress[ count % (sizeof progress -1)] );
          }
        continue;
        }
      else if ( SOCKET_ERROR == iRet)
        {
        iRet = GetLastError();
        break;
        }
      else if ( FD_ISSET( s, &sete) || FD_ISSET( s, &setw))
        {
        /* Get error */
        socklen_t len = sizeof iRet;
        getsockopt( s, SOL_SOCKET, SO_ERROR, (char*)&iRet, &len);
        break;
        }
      }
    }

  /* Disable non-blocking mode */
  setsockasync( s, 1);

  return iRet;
  }

/* End of file */
