/*****************************************************************************
 *
 * File name   : identd.c
 * Function    : ident (RFC 1413) server
 * Authors     : Lawrence Rust
 * Systems     : ANSI C, sockets
 *
 *****************************************************************************
 * File History
 * ------------
 *
 * 05-Jan-05, LVR, Created.
 * 24-Dec-09, LVR, v1.02 - Added Log()
  *
 *****************************************************************************/

/* Compilation options */
#ifndef SWS_USE_SOCKLIB
#define SWS_USE_SOCKLIB 0 /* 0= native sockets, !0 for sws_sock */
#endif

#ifndef OPT_MAX_CONNECTIONS
#define OPT_MAX_CONNECTIONS 3U
#endif
#if OPT_MAX_CONNECTIONS > 63
#define FD_SETSIZE (1 + OPT_MAX_CONNECTIONS)
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
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>
#include <time.h>
#if _MSC_VER >= 1000
#include <crtdbg.h>
#endif

#include "sws_net.h"
#include "sws_cfg.h"
#if OPT_UPNP
#include "sws_upnp.h"
#endif

#if SWS_USE_SOCKLIB
#include "sws_tcp.h"
#elif defined WIN32 || defined _WIN32 || defined __WIN32__
#include <winnetwk.h> /* For WNetGetUser */
#include <iphlpapi.h> /* GetTcpTable */
#include <malloc.h> /* alloca */
#ifdef _MSC_VER
#pragma comment( lib, "iphlpapi.lib") /* For GetTcpTable */
#pragma comment( lib, "delayimp.lib") /* Required for delay loading */
#pragma comment( linker, "/delayload:iphlpapi.dll")
#endif
#elif defined linux || defined __linux || defined __linux__
#include <pwd.h>
#endif

#include "identd.h"


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

/* Expand and stringize a macro name */
#define STR(s) #s
#define STRING(s) STR(s)

#ifdef __GNUC__
#define FORMAT(a,f,l)  __attribute__ ((format (a, f, l)))
#else
#define FORMAT(a,f,l)
#endif

/* Log file prefix */
#define kszPrefix "IDNTD: "

#define POLL_TIMEOUT    333U /* select timeout in millisecs */
#define SEND_TIMEOUT  60000U /* Send timeout in millisecs */
#define IDLE_TIMEOUT 120U    /* Recv idle timeout in seconds */

#ifndef MSG_NOSIGNAL
#define MSG_NOSIGNAL 0
#endif

#define MAX_REQUEST      16  /* Max len request: nnnnn , nnnnn CRLF */
#define MAX_RESPONSE     64  /* Max len response: :USERID:OTHER,US-ASCII:...CRLF NUL */

#define cNul       '\0'
#define cReturn    '\x0d'
#define cLinefeed  '\x0a'
#define CRLF       "\x0d\x0a"

/* ini file values */
#define kszSection        "ident"
#define kszLogVerbosity   "log"
#define kszMaxConnections "max_conn"
#define kszIdleTimeout    "idle_time"
#define kszUserName       "user"


/***************************************************************************
 * Data types
 ***************************************************************************/

/* Log level */
enum { kLogError = -1, kLogWarn = 0, kLogInfo, kLogMore, kLogMost };

/* Double ended queue item */
typedef struct SDeque
  {
  struct SDeque* head;
  struct SDeque* tail;
  } SDeque;

/* Telnet commands */
typedef enum ETelnetState
  {
  kTelnetData  = 0,
  kTelnetWill  = 251U,
  kTelnetWont  = 252U,
  kTelnetDo    = 253U,
  kTelnetDont  = 254U,
  kTelnetIAC   = 255U
  } ETelnetState;

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
  struct sockaddr_in client;    /* client's address */

  time_t tIdle;
  EState eState;

  /* Telnet state */
  ETelnetState eTelnetState;
  int telnetCR;
  size_t telnetCount, telnetSize;
  char* pTelnetInput;           /* -> input buffer */

  /* Input/output buffer */
  char buffer[ MAX_REQUEST + MAX_RESPONSE];
  } SConnection;

#if SWS_USE_SOCKLIB
struct STcpInfo
  {
  u_long remoteHost;
  u_short remotePort;
  u_short localPort;
  };
#endif


/***************************************************************************
 * Local function prototypes
 ***************************************************************************/

static void SessionStart( SConnection*);
static void SessionFinish( SConnection*);
static void SessionInput( SConnection*);
static void SessionAbort( SConnection*);

static int GetUser( SConnection*, unsigned, unsigned);
#if SWS_USE_SOCKLIB
static SWS_FTcpEnum EnumTcp;
#endif

static int TelnetRead( SConnection*, void*, size_t);
static int TelnetReadLine( SConnection*);

static int WriteRaw( SConnection*, const void*, size_t);
static int ReadRaw( SConnection*, void*, size_t);

static void Log( int, const struct sockaddr_in*, const char [], ...) FORMAT(printf,3,4);
static SDeque* DequeRemove( SDeque* const);
static void DequeAppend( SDeque* const pDeque, SDeque* const pItem);
#define DEQUE_INIT( p)     ((p)->tail = (p)->head = (p))
#define DEQUE_IS_EMPTY( p) ((p)->tail == (p))
#define DEQUE_FIRST( p)    ((p)->tail)
#define DEQUE_LAST( p)     ((p)->head)


/***************************************************************************
 * Module data
 ***************************************************************************/

/* Logging */
static int s_iVerbosity;
static FILE* s_logfile;

/* Configuration */
static char s_szUser[32];

/* Operation */
extern sig_atomic_t volatile g_exit;
extern sig_atomic_t volatile g_running;


/***************************************************************************
 * Functions
 ***************************************************************************/

/*
 * Ident server
 */
extern int Identd(
  struct sockaddr* pHost,
  int iVerbosity,
  FILE* logfile
) {
  int iRet;
  SDeque* pDeque, *pNext;
  SDeque freeQ, activeQ; /* Free and active connection lists */
  SConnection* pConnArray, *pConn;
  SOCKET sIdent;
  size_t val, maxConn;
  unsigned uIdleTimeout;

  /* Read configurable values */
  uIdleTimeout = (unsigned)SWS_CfgGetValue( kszSection, kszIdleTimeout, IDLE_TIMEOUT);
  maxConn = (size_t)SWS_CfgGetValue( kszSection, kszMaxConnections, OPT_MAX_CONNECTIONS);
  SWS_CfgGetString( kszSection, kszUserName, "", s_szUser, sizeof s_szUser);
  s_iVerbosity = (int)SWS_CfgGetValue( kszSection, kszLogVerbosity, iVerbosity);
  s_logfile = logfile;

  if ( AF_UNSPEC == pHost->sa_family)
    pHost->sa_family = AF_INET;

  /* Open server sockets */
  sIdent = socket( pHost->sa_family, SOCK_STREAM, 0);
  if ( INVALID_SOCKET == sIdent)
    {
    iRet = GetLastError();
    Log( kLogError, NULL, "ERROR %d: socket: %s\n", iRet, strerror( iRet));
    return iRet;
    }

  /* Server port specifed? */
  if ( AF_INET == pHost->sa_family
    && 0 == ((struct sockaddr_in*)pHost)->sin_port
  ) {
    /* No, set default */
    ((struct sockaddr_in*)pHost)->sin_port = htons( IDENTD_PORT );
    }

  /* Select the port to receive on */
  if ( bind( sIdent, pHost, sizeof(*pHost)) )
    {
    iRet = GetLastError();
    Log( kLogError, (const struct sockaddr_in*)pHost, "ERROR %d: bind: %s\n",
      iRet, strerror( iRet) );
    closesocket( sIdent);
    return iRet;
    }

  /* Disable Nagle */
  iRet = 1;
  setsockopt(
    sIdent,
    IPPROTO_TCP, TCP_NODELAY,
    (char*)&iRet, sizeof(iRet)
  );

  /* Setup a listener */
  if ( listen( sIdent, 1) )
    {
    iRet = GetLastError();
    Log( kLogError, (const struct sockaddr_in*)pHost, "ERROR %d: listen: %s\n",
      iRet, strerror( iRet) );
    closesocket( sIdent);
    return iRet;
    }

  if ( (maxConn + 1) > FD_SETSIZE)
    {
    Log( kLogError, NULL, "ERROR: %u connections greater than compilation limit of %u\n",
      (unsigned)maxConn, FD_SETSIZE - 1);
    closesocket( sIdent);
    return EXIT_FAILURE;
    }

  /* Allocate free connection list */
  pConnArray = (SConnection*) calloc( maxConn, sizeof( SConnection) );
  if ( NULL == pConnArray)
    {
    Log( kLogError, NULL, "ERROR: Out of memory for %u connections\n", (unsigned)maxConn);
    closesocket( sIdent);
    return EXIT_FAILURE;
    }

  DEQUE_INIT( &activeQ);
  DEQUE_INIT( &freeQ);
  for ( val = 0; val < maxConn; ++val)
    {
    DEQUE_INIT( &pConnArray[ val].list);
    DequeAppend( &freeQ, &pConnArray[ val].list);
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
      "Identd"
     )
  )
    uExtPort = 0;
#endif

  Log( kLogWarn, (const struct sockaddr_in*)pHost, "LISTENING\n");

  /* The server loop */
  ++g_running;
  while ( !g_exit)
    {
    fd_set setRd;
    time_t t;
    int iErr;
    struct timeval tval;

    FD_ZERO( &setRd);
    if ( !DEQUE_IS_EMPTY( &freeQ))
      FD_SET( sIdent, &setRd);

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
    iErr = select( FD_SETSIZE, &setRd, NULL, NULL, &tval);
    if ( SOCKET_ERROR == iErr)
      continue;

    t = time( NULL);

    if ( FD_ISSET( sIdent, &setRd))
      {
      socklen_t len;

      /* Incoming connection */
      pConn = LIST2CONN( DequeRemove( DEQUE_FIRST( &freeQ) ));

      pConn->tIdle = t + uIdleTimeout; /* Start the idle timer */

      pConn->eTelnetState = kTelnetData;
      pConn->telnetCR = 0;
      pConn->telnetCount = 0;
      pConn->telnetSize = 0;
      pConn->pTelnetInput = NULL;

      pConn->eState = kStateDisconnected;

      /* Accept the incoming connection */
      len = sizeof( pConn->client);
      pConn->s = accept( sIdent, (struct sockaddr*)&pConn->client, &len);
      if ( INVALID_SOCKET == pConn->s)
        {
        iErr = GetLastError();
        Log( kLogWarn, NULL, "WARN %d: accept: %s\n", iErr, strerror( iErr));

        DequeAppend( &freeQ, &pConn->list);
        }
      else
        {
        if ( s_logfile != stdout && s_logfile != stderr)
          Log( kLogError, &pConn->client, "CONNECT\n");

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
        SessionStart( pConn);
        }
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
        SessionInput( pConn);
        }
      else if ( t > pConn->tIdle)
        {
        /* Timed out */
        Log( kLogMore, &pConn->client, "Idle timeout\n" );
        SessionAbort( pConn);
        }

      /* Is the session finished? */
      if ( kStateQuit == pConn->eState)
        {
        /* Yes */
        if ( s_logfile != stdout && s_logfile != stderr)
          Log( kLogError, &pConn->client, "DISCONNECT\n");

        SessionFinish( pConn);
        closesocket( pConn->s);

        /* Return it to the free list */
        DequeRemove( &pConn->list);
        DequeAppend( &freeQ, &pConn->list);
        }
      } /* Next active connection */
    } /* select loop */
  --g_running;

  pNext = NULL;
  for (
    pDeque = DEQUE_FIRST( &activeQ);
    pDeque != &activeQ;
    pDeque = pNext
  ) {
    pNext = DEQUE_FIRST( pDeque);
    pConn = LIST2CONN( pDeque);
    SessionAbort( pConn);
    SessionFinish( pConn);
    closesocket( pConn->s);
    }

#if OPT_UPNP
  if ( uExtPort)
    SWS_UpnpDelPortMap( "TCP", uExtPort);
  }
#endif

  free( pConnArray);
  closesocket( sIdent);

  return iRet;
  }


/*
 * Start a session
 */
void SessionStart( SConnection* pConn)
  {
  pConn->pTelnetInput = pConn->buffer;
  pConn->telnetSize = MAX_REQUEST;

  Log( kLogMore, &pConn->client, "OPEN\n");
  }


/*
 * End a session
 */
static void SessionFinish( SConnection* pConn)
  {
  Log( kLogMore, &pConn->client, "CLOSE\n");

  /* Send FIN */
  shutdown( pConn->s, SD_SEND);
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
 * Session input ready
 */
void SessionInput( SConnection* pConn)
  {
  int iRet;

  /* Input a line */
  iRet = TelnetReadLine( pConn);
  if ( 0 == iRet)
    return; /* Not complete */

  if ( SOCKET_ERROR == iRet)
    {
    /* Disconnected */
    pConn->eState = kStateQuit;
    }
  else
    {
    char* const pszOut = pConn->buffer + iRet - 1;
    unsigned uLocal, uRemote;
    int i;

    /* Parse the request */
    if ( 2 <= sscanf(
        pConn->buffer, " %u , %u%n %n", &uLocal, &uRemote, &i, &i)
      && uLocal > 0 && uLocal <= 65535U
      && uRemote > 0 && uRemote <= 65535U
      && (i + 1) == iRet
    ) {
      strcpy( pszOut, ":USERID:");
      /* NB GetUser appends user ID */
      switch ( (iRet = GetUser( pConn, uLocal, uRemote)))
        {
      case 0:
        break;
      case 1:
        strcpy( pszOut, ":ERROR:NO-USER");
        break;
      case 2:
        strcpy( pszOut, ":ERROR:HIDDEN-USER");
        break;
      default:
        strcpy( pszOut, ":ERROR:UNKNOWN-ERROR");
        break;
        }
      }
    else
      {
      strcpy( pszOut, ":ERROR:INVALID-PORT");
      iRet = 1;
      }
    strcat( pConn->buffer, CRLF);

    /* Send the response */
    WriteRaw( pConn, pConn->buffer, strlen( pConn->buffer) );

    /* Log the request and response */
    Log( 0 == iRet ? kLogInfo : kLogWarn, &pConn->client, "%s", pConn->buffer);

    /* Drop the connection */
    pConn->eState = kStateQuit;
    }
  }


/*
 * Get info on specified connection
 */
int GetUser( SConnection* pConn, unsigned uLocal, unsigned uRemote)
  {
#if SWS_USE_SOCKLIB
  struct STcpInfo ctx;
  size_t len;

  strcat( pConn->buffer, "OTHER,US-ASCII:");
  len = strlen( pConn->buffer);

  if ( s_szUser[0] )
    {
    strcpy( pConn->buffer + len, s_szUser);
    return 0;
    }

  ctx.remoteHost = ntohl( pConn->client.sin_addr.s_addr);
  ctx.remotePort = (u_short)uRemote;
  ctx.localPort = (u_short)uLocal;

  /* Enumerate all TCP connections */
  if ( 0 == SWS_TcpEnum( &EnumTcp, &ctx))
    return 1; /* User not found */

  gethostname( pConn->buffer + len, sizeof pConn->buffer - len);
  return 0;

#elif defined WIN32 || defined _WIN32 || defined __WIN32__
  UINT mode;

  strcat( pConn->buffer, "WIN32:");
  if ( s_szUser[0] )
    {
    strcat( pConn->buffer, s_szUser);
    return 0;
    }

  uRemote = htons( (u_short)uRemote);
  uLocal = htons( (u_short)uLocal);

  mode = SetErrorMode( SEM_FAILCRITICALERRORS);
#ifdef _MSC_VER
  /* Catch DLL not found exception if Iphlpapi.lib is delay loaded */
  __try
#endif
    {
    DWORD dwSize;
    PMIB_TCPTABLE pTable;
    PMIB_TCPROW pRow;

    /* Get TCP connection table */
    dwSize = 0;
    GetTcpTable( NULL, &dwSize, FALSE);
    pTable = (PMIB_TCPTABLE) _alloca( dwSize);
    if ( NULL == pTable)
      goto error;
    if ( NO_ERROR != GetTcpTable( pTable, &dwSize, FALSE))
      goto error;

    /* Search table for requested end points */
    for (
      pRow = &pTable->table[ dwSize = 0];
      dwSize < pTable->dwNumEntries;
      ++pRow, ++dwSize
    ) {
      if ( pConn->client.sin_addr.s_addr == pRow->dwRemoteAddr
        && uLocal == pRow->dwLocalPort
        && uRemote == pRow->dwRemotePort
      ) {
        /* Connection found */
        size_t len = strlen( pConn->buffer);

#if 1 /* CAUTION returning the username could be a security risk */
        DWORD dwLen = sizeof pConn->buffer - len;

        /* get username */
        if ( NO_ERROR != WNetGetUser( NULL, pConn->buffer + len, &dwLen))
          strcpy( pConn->buffer + len, "anonymous");
#else
        gethostname( pConn->buffer + len, sizeof pConn->buffer - len);
#endif

        SetErrorMode( mode);
        return 0;
        }
      }

    SetErrorMode( mode);
    return 1; /* User not found */
    }
#ifdef _MSC_VER
  __except( EXCEPTION_EXECUTE_HANDLER)
    {
    }
#endif

error:
  SetErrorMode( mode);
  return 3;

#elif defined linux || defined __linux || defined __linux__
  int iRet;
  FILE* file;
  char szBuf[ 160];

  strcat( pConn->buffer, "UNIX:");
  if ( s_szUser[0] )
    {
    strcat( pConn->buffer, s_szUser);
    return 0;
    }

  file = fopen( "/proc/net/tcp", "r");
  if ( NULL == file)
    return 3;

  iRet = 1; /* Assume user not found */

  /* Read the file one line at a time */
  while ( NULL != fgets( szBuf, sizeof szBuf, file))
    {
    unsigned long ulRemoteIP;
    unsigned uLocalPort, uRemotePort, uid;

    /* see cat /proc/net/tcp  and man proc
       sl  local_address rem_address   st tx_queue rx_queue tr tm->when retrnsmt   uid  timeout inode
        0: 00000000:008B 00000000:0000 0A 00000000:00000000 00:00000000 00000000     0        0 6752 1 df0d0840 299 0 0 2 -1
     */
    uid = 0;
    if ( 3 <= sscanf( szBuf, " %*x : %*x:%x %lx:%x %*x %*x : %*x %*x : %*x %*x %u",
        &uLocalPort, &ulRemoteIP, &uRemotePort, &uid)
    ) {
      if ( ulRemoteIP == pConn->client.sin_addr.s_addr
        && uRemotePort == uRemote
        && uLocalPort == uLocal
      ) {
        /* Connection found */
        size_t len = strlen( pConn->buffer);

#if 1 /* CAUTION returning the username could be a security risk */
        struct passwd* pwd;

        if ( uid && NULL != (pwd = getpwuid( uid)))
          strncpy( pConn->buffer + len, pwd->pw_name, sizeof pConn->buffer - len);
        else
          strcpy( pConn->buffer + len, "anonymous");
        iRet = 0; /* Found */
#else
        iRet = gethostname( pConn->buffer + len, sizeof pConn->buffer - len) ? 3 : 0;
#endif
        break;
        }
      }
    }

  fclose( file);
  return iRet;

#elif 1
  size_t len;
  (void)uLocal;
  (void)uRemote;

  strcat( pConn->buffer, "OTHER,US-ASCII:");
  len = strlen( pConn->buffer);

  if ( s_szUser[0] )
    {
    strcpy( pConn->buffer + len, s_szUser);
    return 0;
    }

  return gethostname( pConn->buffer + len, sizeof pConn->buffer - len) ? 3 : 0;
#else
  {int WARN_GetUser_Unimplemented;}
  (void)uLocal;
  (void)uRemote;

  if ( s_szUser[0] )
    {
    strcat( pConn->buffer, "OTHER,US-ASCII:");
    strcat( pConn->buffer, s_szUser);
    return 0;
    }

  return 3;
#endif
  }

#if SWS_USE_SOCKLIB
/*
 * SWS_TcpEnum callback
 */
SWS_CB( int, EnumTcp(
  SWS_PTR( void) pv,                  /* IN: Context to SWS_TcpEnum */
  SWS_PTR( const SWS_STcpConn) pInfo   /* IN: End point info */
)){
  if ( pInfo->localPort == ((SWS_PTR( const struct STcpInfo)) pv)->localPort
    && pInfo->remotePort == ((SWS_PTR( const struct STcpInfo)) pv)->remotePort
    && pInfo->remoteHost == ((SWS_PTR( const struct STcpInfo)) pv)->remoteHost
  ) {
    return 1;
    }
  return 0;
  }
#endif


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


/*
 * Log a message
 */
static void Log(
  int iLevel,
  const struct sockaddr_in* pSock,
  const char szFormat[], ...
) {
  FILE* file;
  char szTime[ 32];
  time_t t;
  va_list ap;

  if ( kLogError == iLevel)
    file = stderr;
  else if ( kLogWarn == iLevel)
    file = s_logfile ? s_logfile : stderr;
  else if ( s_iVerbosity >= iLevel && s_logfile)
    file = s_logfile;
  else
    return;

  va_start( ap, szFormat);

  t = time( NULL);
  strftime( szTime, sizeof szTime, "%Y/%m/%d %H:%M:%S", localtime( &t));
  fprintf( file, "%s " kszPrefix, szTime);
  if ( NULL != pSock)
    fprintf( file, "%s:%u ", inet_ntoa( pSock->sin_addr), ntohs( pSock->sin_port));
  vfprintf( file, szFormat, ap);

  va_end( ap);
  }


/****************************************************************************
 * Telnet operations
 ****************************************************************************/
int TelnetReadLine(
  SConnection* pConn
) {
  int iRet;

  while ( pConn->telnetCount + 1 < pConn->telnetSize)
    {
    unsigned char c;

    iRet = TelnetRead( pConn, &c, sizeof( c) );

    if ( SOCKET_ERROR == iRet)
      return SOCKET_ERROR; /* Error or connection closed */

    if ( 0 == iRet)
      return 0; /* No data */

    if ( pConn->telnetCR)
      {
      pConn->telnetCR = 0;
      switch ( c)
        {
      case cLinefeed:
        goto done;

      default:
        if ( pConn->telnetCount + 3 <= pConn->telnetSize )
          {
          pConn->pTelnetInput[ pConn->telnetCount++] = cReturn;
          pConn->pTelnetInput[ pConn->telnetCount++] = (char)c;
          }
        break;
        }
      }
    else switch ( c)
      {
    default:
      if ( pConn->telnetCount + 2 <= pConn->telnetSize )
        pConn->pTelnetInput[ pConn->telnetCount++] = (char)c;
      break;

    case cReturn:
      pConn->telnetCR = 1;
      break;

    case cLinefeed:
      goto done;
      }
    }

done:
  pConn->pTelnetInput[ iRet = pConn->telnetCount] = cNul;
  pConn->telnetCount = 0;
  pConn->telnetCR = 0;

  return (int)(iRet + 1); /* Including CR */
  }


/*
 * Get a buffer from the client
 */
int TelnetRead(
  SConnection* pConn,
  void* pvBuf,
  size_t size
) {
  size_t len = 0;

  while ( size > len)
    {
    int iRet;
    unsigned char* pBuf;

    /* Receive a buffer */
    iRet = ReadRaw( pConn, (char*)pvBuf + len, size - len);
    if ( SOCKET_ERROR == iRet)
      return SOCKET_ERROR; /* Error or connection closed */

    if ( 0 == iRet)
      break; /* No data */

    /* Parse received data for telnet commands */
    pBuf = (unsigned char*)pvBuf;
    while (  iRet--)
      {
      unsigned char c, buf[3];

      c = *pBuf++;
      switch ( pConn->eTelnetState)
        {
      default:
      case kTelnetData: /* Data state */
        switch ( c)
          {
        default:
          ((unsigned char*)pvBuf)[ len++] = c;
          break;

        case kTelnetIAC: /* IAC */
          pConn->eTelnetState = kTelnetIAC;
          break;
          }
        break;

      case kTelnetIAC: /* Command state */
        switch ( c)
          {
        default:
        case 240: /* End of subnegotiation */
        case 241: /* NOP */
        case 242: /* Data mark (sync) */
        case 243: /* Break */
        case 244: /* Interrupt Process */
        case 245: /* Abort Output */
        case 246: /* Are You There */
        case 247: /* Erase Character */
        case 248: /* Erase Line */
        case 249: /* Go ahead */
        case 250: /* Begin subnegotiation */
          pConn->eTelnetState = kTelnetData;
          break;

        case kTelnetWill:
        case kTelnetWont:
        case kTelnetDo:
        case kTelnetDont:
          pConn->eTelnetState = (ETelnetState)c;
          break;

        case kTelnetIAC:
          pConn->eTelnetState = kTelnetData;
          ((unsigned char*)pvBuf)[ len++] = kTelnetIAC;
          break;
          }
        break;

      case kTelnetWill:
        pConn->eTelnetState = kTelnetData;
        buf[0] = kTelnetIAC;
        buf[1] = kTelnetDont;
        buf[2] = c;
        WriteRaw( pConn, buf, 3);
        break;

      case kTelnetWont:
        pConn->eTelnetState = kTelnetData;
        break;

      case kTelnetDo:
        pConn->eTelnetState = kTelnetData;
        buf[0] = kTelnetIAC;
        buf[1] = kTelnetWont;
        buf[2] = c;
        WriteRaw( pConn, buf, 3);
        break;

      case kTelnetDont:
        pConn->eTelnetState = kTelnetData;
        break;
        }
      }
    }

  return (int)len;
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

  iRet = send( pConn->s, (const char*)pv, size, MSG_NOSIGNAL);

  if ( SOCKET_ERROR == iRet)
    {
    int err = GetLastError();

    /* Win98se sometimes returns EWOULDBLOCK even though select()
     * previously indicated socket is ready to be written */
    if ( EWOULDBLOCK == err || EAGAIN == err )
      {
      return 0;
      }
    else switch ( err)
      {
    case ECONNRESET:
      Log( kLogWarn, &pConn->client, "WARN %d: Connection reset in write\n", err);
      break;
    case ECONNABORTED:
      Log( kLogWarn, &pConn->client, "WARN %d: Connection aborted in write\n", err);
      break;
    default:
      Log( kLogWarn, &pConn->client, "WARN %d: Send failed: %s\n", err, strerror( err));
      break;
      }
    }

  return iRet;
  }


/*
 * Read raw data from the client
 */
int ReadRaw(
  SConnection* pConn,
  void* pv,
  size_t size
) {
  int iRet;

  iRet = recv( pConn->s, (char*) pv, size, 0);
  if ( SOCKET_ERROR == iRet)
    {
    int err = GetLastError();
    if ( EWOULDBLOCK == err || EAGAIN == err )
      {
      return 0;
      }
    else switch ( err)
      {
    case ECONNRESET:
      /* NB many clients reset the connection rather than a graceful close */
      Log( kLogMore, &pConn->client, "Connection reset in read\n");
      break;
    case ECONNABORTED:
      Log( kLogWarn, &pConn->client, "WARN %d: Connection aborted in read\n", err);
      break;
    case ENOTCONN:
      Log( kLogWarn, &pConn->client, "WARN %d: Disconnected in read\n", err);
      break;
    default:
      Log( kLogWarn, &pConn->client, "WARN %d: Receive failed: %s\n", err, strerror( err));
      break;
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
