/*****************************************************************************
 *
 * File name   : echod.c
 * Function    : echo/discard server (RFC 862 & 863)
 * Authors     : Lawrence Rust
 * Systems     : ANSI C, BSD sockets
 *
 *****************************************************************************
 * File History
 * ------------
 *
 * 04-Mar-03, LVR, Created.
 * 04-Jul-09, LVR, Made dumpfile per connection
 *
 *
 * Comments
 * --------
 *
 *****************************************************************************/

/* Compilation options */
#ifndef OPT_MAX_CONNECTIONS
#define OPT_MAX_CONNECTIONS 3U
#endif
#if OPT_MAX_CONNECTIONS > 63
#define FD_SETSIZE (1 + OPT_MAX_CONNECTIONS)
#endif

#ifndef OPT_HEXDUMP
#define OPT_HEXDUMP 0   /* !0 to save rxdata in hex */
#endif

#ifndef OPT_UPNP
# define OPT_UPNP 0        /* !0 to export service to UPnP */
#endif


/* Exports */

/* Imports */

/* ANSI C library */
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>
#include <time.h>
#include <errno.h>
#if _MSC_VER >= 1000
#include <crtdbg.h>
#endif

#include "sws_net.h"
#if OPT_UPNP
#include "sws_upnp.h"
#endif

#include "echod.h"
#include "sws_cfg.h"


/***************************************************************************
 * Macros & constants
 ***************************************************************************/

/* Length of an array - sorely missing from stddef.h */
#ifndef lengthof
#define lengthof( _a) ((size_t)( sizeof(_a)/sizeof((_a)[0]) ))
#endif

/* Get a ptr to the start of a struct given a ptr to a field within it */
#define CONTAINER( ptr, type, field) \
  ((type*)( (char*)(ptr) - offsetof( type, field) ))

#define POLL_TIMEOUT    333U /* select timeout in millisecs */
#define SEND_TIMEOUT  60000U /* Send timeout in millisecs */
#define IDLE_TIMEOUT 120U    /* Recv idle timeout in seconds RFC 2821 4.5.3.2 says >= 300 secs */

#define DEF_SIZE_BUF 3000U   /* Default receive buffer size, ideally >= 2*MSS */

#ifndef MSG_NOSIGNAL
#define MSG_NOSIGNAL 0
#endif

/* ini file values */
#define kszEchoSection    "echo"
#define kszDiscardSection "discard"
#define kszLogVerbosity   "log"
#define kszMaxConnections "max_conn"
#define kszIdleTimeout    "idle_time"
#define kszDump           "save"
#define kszSizeBuf        "buffer"


/***************************************************************************
 * Data types
 ***************************************************************************/

/* Double ended queue item */
typedef struct SDeque
  {
  struct SDeque* head;
  struct SDeque* tail;
  } SDeque;

/* Connection state */
typedef enum EState
  {
  kStateDisconnected,
  kStateConnected,
  kStateQuit
  } EState;

typedef struct SConnection
  {
  SDeque list;
#define LIST2CONN( p) CONTAINER( p, SConnection, list)

  SOCKET s;                     /* client connection */
  u_short port;
  struct sockaddr_in client;    /* client's address */
  time_t tIdle, tRequest;

  /* Logging */
  int iVerbosity;
  FILE* logfile;
#define L_UNIQUE 16
  char szTmpnam[ L_UNIQUE];
  FILE* dumpfile;

  EState eState;
  int iEcho;                    /* Echo received data */
  const char* pszPrefix;

  void* pvBuf;
  size_t sizeBuf;
  } SConnection;


/***************************************************************************
 * Local function prototypes
 ***************************************************************************/

static void SessionStart( SConnection*, int);
static void SessionFinish( SConnection*);
static void SessionInput( SConnection*);
static void SessionAbort( SConnection*);

static int WriteRaw( SConnection*, const void*, size_t);
static int ReadRaw( SConnection*, void*, size_t);

static SDeque* DequeRemove( SDeque* const);
static void DequeAppend( SDeque* const pDeque, SDeque* const pItem);
#define DEQUE_INIT( p)     ((p)->tail = (p)->head = (p))
#define DEQUE_IS_EMPTY( p) ((p)->tail == (p))
#define DEQUE_FIRST( p)    ((p)->tail)
#define DEQUE_LAST( p)     ((p)->head)

static char* UniqueName( char[ L_UNIQUE]);
#if OPT_HEXDUMP
static void HexDump( FILE*, const void*, size_t);
#endif


/***************************************************************************
 * Module data
 * NB This module is multithreaded for echo & discard servers
 ***************************************************************************/

/* Operation */
extern sig_atomic_t volatile g_exit;
extern sig_atomic_t volatile g_running;


/***************************************************************************
 * Functions
 ***************************************************************************/

/*
 * Echo/discard server
 * CAUTION use static variables cautiously since this code is multithreaded
 * for echo & discard servers.
 */
extern int Echod(
  struct sockaddr* pHost,
  int iEcho,
  int iVerbosity,
  FILE* logfile
) {
  int iRet;
  SDeque* pDeque, *pNext;
  SDeque freeQ, activeQ; /* Free and active connection lists */
  SConnection* pConnArray = NULL, *pConn;
  SOCKET sStrm = INVALID_SOCKET, sDgrm = INVALID_SOCKET;
  size_t val, maxConn;
  void* pvBuf = NULL;
  size_t sizeBuf;
  int isDumpFile;
  char szDump[ 128];
  const char* const kszPrefix = iEcho ? "ECHOD" : "DSCRD";
  const char* const kszSection = iEcho ? kszEchoSection : kszDiscardSection;
  unsigned uIdleTimeout;

  /* Read configurable values */
  uIdleTimeout = (unsigned)SWS_CfgGetValue( kszSection, kszIdleTimeout, IDLE_TIMEOUT);
  maxConn = (size_t)SWS_CfgGetValue( kszSection, kszMaxConnections, OPT_MAX_CONNECTIONS);
  sizeBuf = (size_t)SWS_CfgGetValue( kszSection, kszSizeBuf, DEF_SIZE_BUF);
  isDumpFile = (int)SWS_CfgGetValue( kszSection, kszDump, 0);
  iVerbosity = (int)SWS_CfgGetValue( kszSection, kszLogVerbosity, iVerbosity);

  if ( AF_UNSPEC == pHost->sa_family)
    pHost->sa_family = AF_INET;

  /* Open server sockets */
  sStrm = socket( pHost->sa_family, SOCK_STREAM, 0);
  if ( INVALID_SOCKET == sStrm)
    {
    iRet = GetLastError();
    fprintf( stderr, "%s: Failed (%d) to create socket: %s\n",
      kszPrefix, iRet, strerror( iRet));
    return iRet;
    }

  sDgrm = socket( pHost->sa_family, SOCK_DGRAM, 0);
  if ( INVALID_SOCKET == sDgrm)
    {
    iRet = GetLastError();
    switch ( iRet)
      {
    case ESOCKTNOSUPPORT:
      fprintf( stderr, "%s: UDP protocol not installed\n", kszPrefix);
      break;
    default:
      fprintf( stderr, "%s: Failed (%d) to create socket: %s\n",
        kszPrefix, iRet, strerror( iRet));
      break;
      }
    closesocket( sStrm);
    return iRet;
    }

  DEQUE_INIT( &activeQ);
  DEQUE_INIT( &freeQ);

  /* Server port specifed? */
  if ( AF_INET == pHost->sa_family
    && 0 == ((struct sockaddr_in*)pHost)->sin_port
  ) {
    /* No, set default */
    ((struct sockaddr_in*)pHost)->sin_port = htons(
      (u_short)(iEcho ? ECHOD_PORT : DSCRD_PORT) );
    }

  /* Select the port to receive on */
  if ( bind( sStrm, pHost, sizeof *pHost)
    || bind( sDgrm, pHost, sizeof *pHost)
  ) {
    iRet = GetLastError();
    fprintf( stderr, "%s: Failed (%d) to bind to port %u: %s\n",
      kszPrefix, iRet, ntohs( ((struct sockaddr_in*)pHost)->sin_port), strerror( iRet));
    goto error;
    }

  /* Disable Nagle */
  iRet = 1;
  setsockopt(
    sStrm,
    IPPROTO_TCP, TCP_NODELAY,
    (char*)&iRet, sizeof(iRet)
  );

  /* Set send timeout */
  { DECL_TIMEOUT( tv, SEND_TIMEOUT);
  setsockopt(
    sStrm,
    SOL_SOCKET, SO_SNDTIMEO,
    (char*)&tv, sizeof tv
  );
  setsockopt(
    sDgrm,
    SOL_SOCKET, SO_SNDTIMEO,
    (char*)&tv, sizeof tv
  );
  }

  /* Set send buffer size */
  iRet = (int)sizeBuf;
  setsockopt(
    sStrm,
    SOL_SOCKET, SO_SNDBUF,
    (char*)&iRet, sizeof(iRet)
  );

  /* Set receive buffer size */
  iRet = (int)sizeBuf;
  setsockopt(
    sStrm,
    SOL_SOCKET, SO_RCVBUF,
    (char*)&iRet, sizeof(iRet)
  );

  /* Setup a listener */
  if ( listen( sStrm, 1) )
    {
    iRet = GetLastError();
    fprintf( stderr, "%s: Failed (%d) to listen: %s\n",
      kszPrefix, iRet, strerror( iRet));
    goto error;
    }

  /* Allocate free connection list */
  if ( maxConn + 1 > FD_SETSIZE)
    {
    fprintf( stderr, "%s: %u connections greater than compilation limit of %u\n",
      kszPrefix, (unsigned)maxConn, FD_SETSIZE - 1);
    iRet = ERANGE;
    goto error;
    }

  pConnArray = (SConnection*)calloc( maxConn, sizeof(SConnection) );
  if ( NULL == pConnArray)
    {
    fprintf( stderr, "%s: Out of memory for %u connections\n",
      kszPrefix, (unsigned)maxConn);
    iRet = errno;
    goto error;
    }

  /* Build free connection list */
  for ( val = 0; val < maxConn; ++val)
    {
    DEQUE_INIT( &pConnArray[ val].list);
    DequeAppend( &freeQ, &pConnArray[ val].list);
    }

  /* Allocate a common buffer */
  pvBuf = malloc( sizeBuf);
  if ( NULL == pvBuf)
    {
    fprintf( stderr, "%s: Out of memory for buffer\n", kszPrefix);
    iRet = errno;
    goto error;
    }

#ifdef SIGPIPE
  /* Ignore SIGPIPE if connection reset during send() */
  signal( SIGPIPE, SIG_IGN);
#endif

#if OPT_UPNP
  {
  unsigned uExtPort = SWS_CfgGetValue( kszSection, "UpnpExtPort", 0);
  if ( uExtPort && SWS_UpnpAddPortMap(
      "TCP",
      ntohs( ((const struct sockaddr_in*)pHost)->sin_port),
      inet_ntoa( ((const struct sockaddr_in*)pHost)->sin_addr),
      uExtPort,
      kszPrefix
     )
  )
    uExtPort = 0;
#endif

  if ( NULL != logfile)
    {
    time_t t = time( NULL);

    strftime( szDump, sizeof(szDump), "%Y/%m/%d %H:%M:%S",
      localtime( &t) );
    fprintf( logfile, "%s %s: %s:%u LISTENING\n",
      szDump,
      kszPrefix,
      inet_ntoa( ((struct sockaddr_in*)pHost)->sin_addr),
      ntohs( ((struct sockaddr_in*)pHost)->sin_port)
    );
    }

  /* The server loop */
  ++g_running;
  while ( !g_exit)
    {
    fd_set setRd;
    socklen_t len;
    time_t t;
    struct timeval tval;

    FD_ZERO( &setRd);
    if ( INVALID_SOCKET != sDgrm)
      FD_SET( sDgrm, &setRd);
    if ( !DEQUE_IS_EMPTY( &freeQ))
      FD_SET( sStrm, &setRd);

    /* Add active sockets to the select sets */
    for (
      pDeque = DEQUE_FIRST( &activeQ);
      pDeque != &activeQ;
      pDeque = DEQUE_FIRST( pDeque)
    ) {
      pConn = LIST2CONN( pDeque);

      /* Receive data */
      FD_SET( pConn->s, &setRd);
      }

    /* Set timeout for select */
    tval.tv_usec = (POLL_TIMEOUT % 1000) * 1000UL;
    tval.tv_sec = POLL_TIMEOUT / 1000;

    /* Wait for receive event */
    iRet = select( FD_SETSIZE, &setRd, NULL, NULL, &tval);
    if ( SOCKET_ERROR == iRet)
      continue;

    t = time( NULL);

    if ( FD_ISSET( sStrm, &setRd))
      {
      /* Incoming connection */
      pConn = LIST2CONN( DequeRemove( DEQUE_FIRST( &freeQ) ));

      pConn->logfile = logfile;
      pConn->iVerbosity = iVerbosity;
      pConn->tIdle = t + uIdleTimeout; /* Start the idle timer */
      pConn->eState = kStateDisconnected;
      pConn->iEcho = iEcho;
      pConn->pszPrefix = kszPrefix;
      pConn->dumpfile = NULL;
      pConn->pvBuf = pvBuf;
      pConn->sizeBuf = sizeBuf;
      pConn->port = ntohs( ((struct sockaddr_in*)pHost)->sin_port);

      /* Accept the incoming connection */
      len = sizeof( pConn->client);
      pConn->s = accept( sStrm, (struct sockaddr*)&pConn->client, &len);
      if ( INVALID_SOCKET == pConn->s)
        {
        iRet = GetLastError();
        fprintf( stderr, "%s: Accept failed (%d): %s\n",
          kszPrefix, iRet, strerror( iRet));

        DequeAppend( &freeQ, &pConn->list);
        }
      else
        {
        pConn->eState = kStateConnected;
        DequeAppend( &activeQ, &pConn->list);

        /* Set a send timeout */
        { DECL_TIMEOUT( tv, SEND_TIMEOUT);
        setsockopt(
          pConn->s,
          SOL_SOCKET, SO_SNDTIMEO,
          (char*)&tv, sizeof tv
        ); }

        /* Start a session */
        pConn->tRequest = t;
        SessionStart( pConn, isDumpFile);
        }
      }

    if ( INVALID_SOCKET != sDgrm
      && FD_ISSET( sDgrm, &setRd)
    ) {
      /* Datagram received */
      SConnection conn;

      conn.s = sDgrm;
      conn.client.sin_family = AF_UNSPEC;
      conn.logfile = logfile;
      conn.iVerbosity = iVerbosity;
      conn.eState = kStateDisconnected;
      conn.iEcho = iEcho;
      conn.pszPrefix = kszPrefix;
      if ( isDumpFile && NULL != UniqueName( conn.szTmpnam) )
        {
#if OPT_HEXDUMP
        conn.dumpfile = fopen( conn.szTmpnam, "w");
#else
        conn.dumpfile = fopen( conn.szTmpnam, "wb");
#endif
        }
      else
        conn.dumpfile = NULL;
      conn.pvBuf = pvBuf;
      conn.sizeBuf = sizeBuf;
      conn.port = ntohs( ((struct sockaddr_in*)pHost)->sin_port);

      /* Receive a datagram */
      conn.tRequest = t;
      SessionInput( &conn);

      if ( NULL != conn.dumpfile)
        fclose( conn.dumpfile);
      }

    /* Scan active connections for an event */
    pNext = NULL;
    for (
      pDeque = DEQUE_FIRST( &activeQ);
      pDeque != &activeQ;
      pDeque = pNext
    ) {
      pNext = DEQUE_FIRST( pDeque);
      pConn = LIST2CONN( pDeque);

      if ( FD_ISSET( pConn->s, &setRd))
        {
        /* Restart the idle timer */
        pConn->tIdle = t + uIdleTimeout;

        /* More input */
        pConn->tRequest = t;
        SessionInput( pConn);
        }
      else if ( t > pConn->tIdle)
        {
        /* Timed out */
        SessionAbort( pConn);
        }

      /* Is the session finished? */
      if ( kStateQuit == pConn->eState)
        {
        /* Yes */
        pConn->tRequest = t;
        SessionFinish( pConn);

        /* Return it to the free list */
        DequeRemove( &pConn->list);
        DequeAppend( &freeQ, &pConn->list);
#if 0 /* Only for profiling */
        {int PROFILING;}
        goto error;
#endif
        }
      } /* Next active connection */

    } /* select loop */
  --g_running;

#if OPT_UPNP
  if ( uExtPort)
    SWS_UpnpDelPortMap( "TCP", uExtPort);
  }
#endif

error:
  if ( NULL != pvBuf)
    free( pvBuf);

  if ( NULL != pConnArray)
    {
    pNext = NULL;
    for (
      pDeque = DEQUE_FIRST( &activeQ);
      pDeque != &activeQ;
      pDeque = pNext
    ) {
      pNext = DEQUE_FIRST( pDeque);
      pConn = LIST2CONN( pDeque);
      pConn->tRequest = time( NULL);
      SessionAbort( pConn);
      SessionFinish( pConn);
      }
    free( pConnArray);
    }

  closesocket( sDgrm);
  closesocket( sStrm);
  return iRet;
  }


/*
 * Start a session
 */
void SessionStart( SConnection* pConn, int isDumpFile)
  {
  if ( isDumpFile && NULL != UniqueName( pConn->szTmpnam) )
    {
#if OPT_HEXDUMP
    pConn->dumpfile = fopen( pConn->szTmpnam, "w");
#else
    pConn->dumpfile = fopen( pConn->szTmpnam, "wb");
#endif
    }

  if ( pConn->iVerbosity > 0)
    {
    char szBuf[ 32];

    strftime( szBuf, sizeof szBuf, "%Y/%m/%d %H:%M:%S",
      localtime( &pConn->tRequest) );

    if ( NULL != pConn->logfile)
      fprintf( pConn->logfile,
        "%s %s: %s:%u OPEN\n",
        szBuf,
        pConn->pszPrefix,
        inet_ntoa( pConn->client.sin_addr),
        ntohs( pConn->client.sin_port)
      );

    if ( pConn->logfile != stdout && pConn->logfile != stderr)
      fprintf( stderr,
        "%s %s: %s:%u CONNECT\n",
        szBuf,
        pConn->pszPrefix,
        inet_ntoa( pConn->client.sin_addr),
        ntohs( pConn->client.sin_port)
      );
    }
  }


/*
 * Abort a session
 */
void SessionAbort( SConnection* pConn)
  {
  struct linger lgr;

  /* Reset the connection when closed */
  lgr.l_onoff = 1;
  lgr.l_linger = 0;
  setsockopt( pConn->s, SOL_SOCKET, SO_LINGER, (char*)&lgr, sizeof( lgr));

  pConn->eState = kStateQuit;
  }


/*
 * End a session
 */
static void SessionFinish( SConnection* pConn)
  {
  if ( pConn->iVerbosity > 0 )
    {
    char szBuf[ 32];

    strftime( szBuf, sizeof(szBuf), "%Y/%m/%d %H:%M:%S",
      localtime( &pConn->tRequest) );

    if ( NULL != pConn->logfile)
      fprintf( pConn->logfile, "%s %s: %s:%u CLOSE\n",
        szBuf,
        pConn->pszPrefix,
        inet_ntoa( pConn->client.sin_addr),
        ntohs( pConn->client.sin_port)
      );

    if ( pConn->logfile != stdout && pConn->logfile != stderr)
      fprintf( stderr,
        "%s %s: %s:%u DISCONNECT\n",
        szBuf,
        pConn->pszPrefix,
        inet_ntoa( pConn->client.sin_addr),
        ntohs( pConn->client.sin_port)
      );
    }

  closesocket( pConn->s);
  if ( NULL != pConn->dumpfile)
    fclose( pConn->dumpfile);
  }



/*
 * Session input ready
 */
void SessionInput( SConnection* pConn)
  {
  int iRet;

  /* Input a line */
  iRet = ReadRaw( pConn, pConn->pvBuf, pConn->sizeBuf);
  if ( SOCKET_ERROR == iRet)
    {
    /* Disconnected */
    pConn->eState = kStateQuit;
    }
  else
    {
    if ( NULL != pConn->logfile
      && pConn->iVerbosity > 0
    ) {
      char szBuf[ 32];
      strftime( szBuf, sizeof(szBuf), "%Y/%m/%d %H:%M:%S",
        localtime( &pConn->tRequest) );

      fprintf( pConn->logfile, "%s %s: %s:%u %s RECV %u%s%s\n",
        szBuf,
        pConn->pszPrefix,
        inet_ntoa( pConn->client.sin_addr),
        ntohs( pConn->client.sin_port),
        kStateDisconnected == pConn->eState ? "UDP" : "TCP",
        (unsigned)iRet,
        NULL != pConn->dumpfile ? " > " : "",
        NULL != pConn->dumpfile ? pConn->szTmpnam : ""
      );
      }

    if ( NULL != pConn->dumpfile)
      {
#if OPT_HEXDUMP
      HexDump( pConn->dumpfile, pConn->pvBuf, (size_t)iRet);
#else
      fwrite( pConn->pvBuf, (size_t)iRet, 1, pConn->dumpfile);
#endif
      }

    if ( pConn->iEcho)
      {
      /* Echo the data */
      WriteRaw( pConn, pConn->pvBuf, (size_t)iRet);
      }
    }
  }


/****************************************************************************
 * Support
 ****************************************************************************/

/*
 * Add an item to the tail of a deque
 */
void DequeAppend( SDeque* const pDeque, SDeque* const pItem)
  {
  pItem->tail = pDeque;
  pItem->head = pDeque->head;

  pDeque->head->tail = pItem;
  pDeque->head = pItem;
  }


/*
 * Remove an item from a deque
 */
SDeque* DequeRemove( SDeque* const pItem)
  {
  pItem->head->tail = pItem->tail;
  pItem->tail->head = pItem->head;

  DEQUE_INIT( pItem);

  return pItem;
  }


#if OPT_HEXDUMP
/*
 * Hex dump
 */
void HexDump(
  FILE* file,                         /* OUT: text */
  const void* pv,                     /* IN: data */
  size_t len
) {
  const unsigned char* p = (const unsigned char*)pv;
  size_t offset;

  offset = 0;
  do
    {
    int i;

    /* Dump a line */
    fprintf( file, "%04X:", offset);
    for ( i = 0; i < 16 && offset < len; ++i, ++offset)
      fprintf( file, " %02X", *(p++) );
    fputs( "\n", file);
    }
  while ( offset < len);
  }
#endif


/*
 * Generate a unique filename
 */
static char* UniqueName( char szBuf[ L_UNIQUE] )
  {
  static unsigned long s_u;
  long i;
  for ( i = 0; i < 1000000L; ++i)
    {
    FILE* f;

    sprintf( szBuf, "ZE%06lu", s_u++);
    f = fopen( szBuf, "r");
    if ( NULL == f)
      return szBuf;

    fclose( f);
    }
  return NULL;
  }


/****************************************************************************
 * Network I/O
 ****************************************************************************/

/*
 * Write raw data to the client
 */
static int WriteRaw(
  SConnection* pConn,
  const void* pv,
  size_t size
) {
  int iRet;

  if ( kStateDisconnected == pConn->eState)
    {
    iRet = sendto( pConn->s, (const char*)pv, size, 0,
      (struct sockaddr*)&pConn->client, sizeof pConn->client);
    }
  else
    {
    iRet = send( pConn->s, (const char*)pv, size, MSG_NOSIGNAL);
    }

  if ( SOCKET_ERROR == iRet)
    {
    int err = GetLastError();

    /* Win98se sometimes returns EWOULDBLOCK even though select()
     * previously indicated socket is ready to be written */
    if ( EWOULDBLOCK == err || EAGAIN == err)
      {
      return 0;
      }
    else if ( pConn->iVerbosity >= 0)
      {
      FILE* file = NULL != pConn->logfile ? pConn->logfile : stderr;
      char szTime[ 32];

      strftime( szTime, sizeof szTime, "%Y/%m/%d %H:%M:%S",
        localtime( &pConn->tRequest) );
      fprintf( file, "%s %s: %s:%u ",
        szTime,
        pConn->pszPrefix,
        inet_ntoa( pConn->client.sin_addr),
        ntohs( pConn->client.sin_port)
      );
      switch ( err)
        {
      case ECONNRESET:
        fputs( "Connection reset in write\n", file);
        SessionAbort( pConn);
        break;
      case ECONNABORTED:
        fputs( "Connection aborted in write\n", file);
        SessionAbort( pConn);
        break;
      default:
        fprintf( file, "Send failed (%d): %s\n", err, strerror( err));
        break;
        }
      }
    }

  return iRet;
  }


/*
 * Read raw data from the client
 */
static int ReadRaw(
  SConnection* pConn,
  void* pv,
  size_t size
) {
  int iRet;

  if ( kStateDisconnected == pConn->eState)
    {
    socklen_t len = sizeof( pConn->client);
    iRet = recvfrom( pConn->s, (char*)pv, size, 0,
      (struct sockaddr*)&pConn->client, &len);
    }
  else
    {
    iRet = recv( pConn->s, (char*)pv, size, 0);
    }

  if ( SOCKET_ERROR == iRet)
    {
    int err = GetLastError();
    if ( EWOULDBLOCK == err || EAGAIN == err )
      {
      return 0;
      }
    else if ( pConn->iVerbosity > 0)
      {
      FILE* file = NULL != pConn->logfile ? pConn->logfile : stderr;
      char szTime[ 32];

      strftime( szTime, sizeof szTime, "%Y/%m/%d %H:%M:%S",
        localtime( &pConn->tRequest) );
      fprintf( file, "%s %s: %s:%u ",
        szTime,
        pConn->pszPrefix,
        inet_ntoa( pConn->client.sin_addr),
        ntohs( pConn->client.sin_port)
      );
      switch ( err)
        {
      case ECONNRESET:
        fputs( "Connection reset in read\n", file);
        break;
      case ECONNABORTED:
        fputs( "Connection aborted in read\n", file);
        break;
      case ENOTCONN:
        fputs( "Disconnected in read\n", file);
        break;
      default:
        fprintf( file, "Receive failed (%d): %s\n", err, strerror( err));
        break;
        }
      }
    }
  else if ( 0 == iRet)
    {
    /* Graceful disconnect */
    iRet = SOCKET_ERROR;
    }

  return iRet;
  }

/* End of file */
