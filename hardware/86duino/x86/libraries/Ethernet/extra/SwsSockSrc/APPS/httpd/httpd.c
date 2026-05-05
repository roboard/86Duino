/*****************************************************************************
 *
 * File name   : httpd.c
 * Function    : HTTP server. RFC 2616 HTTP/1.1, RFC 1945 HTTP/1.0
 * Authors     : Lawrence Rust
 * Systems     : ANSI C, sockets
 *
 *****************************************************************************
 * File History
 * ------------
 *
 * 05-Jun-01, LVR, Created.
 * 07-Jun-01, LVR, Made select timeouts unsigned long constants
 * 04-Nov-02, LVR, Increased HttpRawWrite timeout to 30 secs
 * 21-Feb-03, LVR, Support multiple connections.  Basic HTTP/1.1
 * 07-Aug-09, LVR, v2.01 - Catch SIGPIPE
 * 06-Nov-09, LVR, v2.10 - Added proxy support
 * 13-Jan-10, LVR, v2.11 - Reworked authentication mechanism
 * 26-Jan-10, LVR, v2.12 - Wildcard aliases
 *
 *****************************************************************************/
#define kszServerName "Software Systems httpd/" kszVer
#define kszVer STRING( HTTPD_VER_MAJOR) "." STRING( HTTPD_VER_MINOR)

/* Compilation options */
#ifndef OPT_MAX_CONNECTIONS
#define OPT_MAX_CONNECTIONS 4U
#endif
#if OPT_MAX_CONNECTIONS > 63
#define FD_SETSIZE (1 + OPT_MAX_CONNECTIONS)
#endif

#ifndef OPT_UPNP
#define OPT_UPNP 0      /* !0 to export service to UPnP */
#endif

#ifndef OPT_HTTP_TINY
#define OPT_HTTP_TINY 0 /* !0 for a minimal http server */
#endif

#ifndef OPT_HTTP_REALMS
#define OPT_HTTP_REALMS  (!OPT_HTTP_TINY) /* !0 to support realms */
#endif

#ifndef OPT_HTTP_ALIAS
#define OPT_HTTP_ALIAS  (!OPT_HTTP_TINY) /* !0 to support aliases */
#endif

#ifndef OPT_HTTP_REDIR
#define OPT_HTTP_REDIR  (!OPT_HTTP_TINY) /* !0 to support server re-directs */
#endif

#ifndef OPT_SAVE_BODY
#define OPT_SAVE_BODY 0 /* !0 to save any entity body */
#endif


/* Exports */
#include "httpd.h"

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
#include <assert.h>
#if _MSC_VER >= 1000
#include <crtdbg.h>
#endif

#include "sws_net.h"
#include "sws_str.h"   /* SWS_stricmp */
#include "sws_time.h"
#include "sws_cfg.h"
#if OPT_UPNP
#include "sws_upnp.h"
#endif
#if OPT_HTTP_REALMS
#include "sws_ba64.h"
#endif

#if defined _MSC_VER && _MSC_VER <= 800 /* VC1.5 */
#pragma warning ( disable : 4705) /* statement has no effect: (void) cast */
#endif


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

/* Compile time assertion
 * Generates 'invalid array dimension' compiler error if condition false
 */
#undef STATIC_ASSERT
#ifdef __GNUC__
# define STATIC_ASSERT(e) extern char _dummy[(e) ? 1 : -1] __attribute__((unused))
#else
# define STATIC_ASSERT(e) extern char _dummy[(e) ? 1 : -1]
#endif

#ifndef MSG_NOSIGNAL
#define MSG_NOSIGNAL 0        /* Flag to send() to disable SIG_PIPE */
#endif

#define POLL_TIMEOUT    333U /* select timeout in millisecs */
#define SEND_TIMEOUT  60000U /* send/connect timeout in millisecs */
#define IDLE_TIMEOUT 120U    /* Input idle timeout in seconds */
#define CONN_TIMEOUT   2U    /* Connection re-use timeout in seconds */
#define INVALID_TIME ((time_t)-1)

/* Buffer sizes */
#ifndef MAX_LINE
#define MAX_LINE 1023         /* Max request line length */
#endif
#ifndef MAX_HDR
#define MAX_HDR 2047          /* Max size of headers */
#endif

/* String constants */
#ifndef kszPrefix
#define kszPrefix "HTTPD: "   /* Log file prefix */
#endif
#define cNul       '\0'
#define cReturn    '\x0d'
#define cLinefeed  '\x0a'
#define CRLF "\x0d\x0a"

/* ini file values */
#define kszLogVerbosity   "log"
#define kszMaxConnections "max_conn"
#define kszIdleTimeout    "idle_time"
#define kszKeepTimeout    "keep_time"


/***************************************************************************
 * Data types
 ***************************************************************************/

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
  kStateIdle = 0,

  /* Server */
  kStateRequest,    /* Client connected, awaiting request line - get/post... */
  kStateHeader,     /* Receiving headers, awaiting end of headers */
  kStateExecute,    /* Processing request, awaiting server response */
  kStateSend,       /* Sending response body */

#if !OPT_HTTP_TINY
  kStateRecv,       /* Receiving post/put request body */

  /* Client (proxy) */
  kStateConnect,
  kStateResponse,
  kStateRespHdr,
#endif

  kStateQuit
  } EState;


#if OPT_HTTP_REALMS
/* Authenticated realms */

struct SHost { ui32 ulNet, ulMask; /* IP4 addr/mask */ };
#define SIZE_REALM_ACCT 63
struct SAcct { char szAcct[ SIZE_REALM_ACCT + 1]; /* Base64 username:password */ };
struct STime { unsigned uOnTime, uOffTime; /* Minutes after midnight */ };

typedef struct SRealm
  {
  SDeque list;
#define LIST2REALM( p) CONTAINER( p, SRealm, list)

  char szTag[ SWS_CFG_MAX_KEY + 1];
#define SIZE_REALM_NAME 63
  char szName[ SIZE_REALM_NAME + 1];
#define SIZE_REALM_ROOT 31
  char szRoot[ SIZE_REALM_ROOT + 1]; /* Local path prefix for files */

  unsigned uPermitHosts;
  struct SHost* aPermitHost;

  unsigned uDenyHosts;
  struct SHost* aDenyHost;

  unsigned uAccts;
  struct SAcct* aAcct;

  unsigned uTimes;
  struct STime* aTime;
  } SRealm;
#endif /* OPT_HTTP_REALMS */


/* Connection state */
struct SHttpConnection
  {
  SDeque list;
#define LIST2CONN( p) CONTAINER( p, SHttpConnection, list)

  SOCKET s;
  struct sockaddr_in client;
  time_t tIdle;                 /* Connection idle timeout */

  /* Telnet state */
  ETelnetState eTelnetState;
  int telnetCR, telnetEOL;
  size_t telnetCount, telnetSize;
  char* pTelnetInput;           /* -> input buffer */

  /* Connection state */
  EState eState;
#if !OPT_HTTP_TINY
  int isServer;
#endif

  /* Request state */
  const char* pszURI;           /* The original URI, excluding query & params */
  const char* pszQuery;         /* The original query text */
  const char* pszParam;         /* The original param text */
  FHttpMethod* pfnAction;       /* -> Callback handler */
#if OPT_HTTP_REALMS
  FHttpMethod* pfnAuthAction;   /* -> Callback handler if authenicated OK */
  SRealm* pRealm;               /* Authorization realm */
  int isAuth;                   /* !0 if request was authenticated */
#endif
#if !OPT_HTTP_TINY
  SHttpRequest* pRqst;          /* -> requst for connected callback */
#endif
  long lContent;                /* Bytes remaining in body, -1 if unknown */
#if OPT_SAVE_BODY
  FILE* fileBody;
#endif
  size_t sizePending;           /* Bytes pending output */
  const char* pPending;         /* -> pending output */
  SHttpRequest rqst;            /* Public request info */

  /* Buffers */
  char header[ MAX_HDR + 1];    /* Request headers */
  char input[ MAX_LINE + 1];    /* Telnet input buffer */
  char alias[ 256];             /* For URI aliases */
  };

#if OPT_HTTP_REDIR || OPT_HTTP_ALIAS
/* URI redirection entry */
struct SRedirect
  {
#define SIZE_REDIR 127
  char szOld[ SIZE_REDIR + 1];
  char szNew[ SIZE_REDIR + 1];
  };
/* URI redirection list */
struct SRedirList
  {
  size_t size;
  unsigned uCount;
  struct SRedirect* aList;
  };
#endif


/***************************************************************************
 * Function prototypes
 ***************************************************************************/

static void RunServer( SOCKET, size_t);
static void SessionStart( SHttpConnection*);
static void SessionFinish( SHttpConnection*);
static void SessionInput( SHttpConnection*);
static int SessionOutput( SHttpConnection*);
static void SessionAbort( SHttpConnection*);

static void NewConnection( SHttpConnection* pConn);
static void EndRequest( SHttpConnection* pConn);

static void HttpGetRequest( SHttpConnection*);
static void HttpGetHeader( SHttpConnection*);
#if !OPT_HTTP_TINY
static void HttpGetBody( SHttpConnection*);
static void HttpConnected( SHttpConnection*);
static void HttpGetResponse( SHttpConnection*);
#endif

static int TelnetRead( SHttpConnection*, void*, size_t);
static int TelnetReadLine( SHttpConnection*);
static int ReadRaw( SHttpConnection*, void*, size_t);
static int WriteRaw( SHttpConnection*, const void*, size_t);

static void Log( int, const struct sockaddr_in*, const char [], ...) HTTP_FORMAT(printf,3,4);
static void LogResponse( SHttpConnection*, int, const char*, long, const char*);
static void vLog( int, const struct sockaddr_in*, const char[], va_list ap);

static SDeque* DequeRemove( SDeque* const);
static void DequeAppend( SDeque* const pDeque, SDeque* const pItem);
#if OPT_HTTP_REALMS
static void DequePrepend( SDeque* const pDeque, SDeque* const pItem);
#endif
#define DEQUE_INIT( p)     ((p)->tail = (p)->head = (p))
#define DEQUE_IS_EMPTY( p) ((p)->tail == (p))
#define DEQUE_FIRST( p)    ((p)->tail)
#define DEQUE_LAST( p)     ((p)->head)

#if OPT_SAVE_BODY
#define L_UNIQUE 16
static char* UniqueName( char [ L_UNIQUE] );
#endif

/* URI/method lookup */
static size_t FindCommand( SHttpConnection*, const char*, const char*);
static FHttpMethod UriNotFound;
static FHttpMethod MethodNotImplemented;
static FHttpMethod OptionsMethod;

#if OPT_HTTP_REDIR
/* Server redirection */
static FHttpMethod UriRedirect;
static int CmpRedirect( const void*, const void*);
#endif

#if OPT_HTTP_ALIAS
/* Aliases */
static const struct SRedirect* FindAlias( const char*);
static const char* AliasURI( SHttpConnection*, const struct SRedirect*, const char*);
#endif
#if OPT_HTTP_REDIR || OPT_HTTP_ALIAS
static SWS_FCfgCallback EnumRedirected;
#endif

#if OPT_HTTP_REALMS
/* Realms */
static FHttpMethod* GetRealmAction( SHttpConnection*, const char*, FHttpMethod*);
static int Authenticate( const struct SRealm*, const char*, const char*);
static SRealm* FindRealm( const char*, int);
static SWS_FCfgCallback EnumRealm;
static FHttpMethod Forbidden;
static FHttpMethod NoService;
static FHttpMethod AuthRequired;
static FHttpMethod ProxyAuthRequired;
#endif


/***************************************************************************
 * Module data
 ***************************************************************************/

extern sig_atomic_t volatile g_exit;
extern sig_atomic_t volatile g_running;

/* Logging */
static int s_iVerbosity;
static FILE* s_logfile;

/* Configuration */
static unsigned s_uIdleTimeout;
static unsigned s_uKeepTimeout;

/* Operation */
static SDeque s_free, s_active; /* List of free and active connections */
#if OPT_HTTP_REALMS
static SDeque s_realms;         /* List of realms */
#endif
#if OPT_HTTP_REDIR
static struct SRedirList s_redirects; /* URI redirection list */
#endif
#if OPT_HTTP_ALIAS
static struct SRedirList s_aliases;   /* Alias list */
#else
static char s_szRootRequest[ 256];    /* URI substitute for / */
#endif

static SHttpAction* s_pActions; /* -> the URI action table */
static size_t s_uActions;       /* No. entries in the URI action table */

/* Table of known URI actions */
static const SHttpAction actionArray[] = {
  { "\\*", /* Literal "*" */
    {
      {kszHttpOpts, &OptionsMethod, NULL}
    }
  },
#if !OPT_HTTP_TINY
  { "//?*", /* Network URI */
    {
      {kszHttpGet,     &UriNotFound, NULL},
      {kszHttpHead,    &UriNotFound, NULL},
      {kszHttpPut,     &UriNotFound, NULL},
      {kszHttpDelete,  &UriNotFound, NULL},
      {kszHttpPost,    &UriNotFound, NULL},
      {kszHttpConn,    &UriNotFound, NULL}
    }
  },
#endif
  { "?*:/*", /* Absolute URI */
    {
      {kszHttpGet,     &UriNotFound, NULL},
      {kszHttpHead,    &UriNotFound, NULL},
      {kszHttpPut,     &UriNotFound, NULL},
      {kszHttpDelete,  &UriNotFound, NULL},
      {kszHttpPost,    &UriNotFound, NULL},
      {kszHttpConn,    &UriNotFound, NULL}
    }
  }
};


/***************************************************************************
 * Functions
 ***************************************************************************/

/**
 * Append to the URI action table
 */
extern void HttpActions(
  const SHttpAction actions[],
  unsigned uActions
) {
  SHttpAction* pActions;

  /* Resize action list */
  pActions = (SHttpAction*) realloc(
    s_pActions,
    sizeof( *s_pActions) * (s_uActions + uActions)
  );
  if ( NULL != pActions)
    {
    /* Append to end of action list */
    memcpy( pActions + s_uActions, actions, sizeof( actions[0]) * uActions);
    s_pActions = pActions;
    s_uActions += uActions;

#if OPT_HTTP_REALMS
    /* Scan actions for new realms */
    { int i;
    for ( i = 0; uActions-- > 0; ++i)
      {
      const SHttpMethod* pM;
      unsigned v;
      for ( pM = &actions[i].methods[v = 0]; v < lengthof( actions[i].methods); ++v, ++pM)
        {
        const char* pszRealm = pM->pszRealm;
        if ( NULL != pszRealm)
          {
          Log( kHttpMost, NULL, "Realm for %s %s = %s\n",
            pM->pszName, actions[i].pszStem, pszRealm);

          /* Find or append the realm */
          (void)FindRealm( pszRealm, 1);
          }
        }
      }
    }
#endif
    }
  else
    Log( kHttpError, NULL, "ERROR %d: HttpActions realloc\n", errno);
  }


/**
 * The http server
 */
extern int Httpd(
  const void* pHost,
  int iVerbosity,
  FILE* logfile
) {
  int iRet;
  unsigned val;
  SOCKET s;
  SHttpConnection* pConnArray;
  size_t maxConn;
  SDeque* pDeque, *pNext;
  SHttpConnection* pConn;

  /* Read configurable values */
  s_uIdleTimeout = (unsigned)SWS_CfgGetValue( kszHttpSect, kszIdleTimeout, IDLE_TIMEOUT);
  s_uKeepTimeout = (unsigned)SWS_CfgGetValue( kszHttpSect, kszKeepTimeout, CONN_TIMEOUT);
  maxConn = (size_t)SWS_CfgGetValue( kszHttpSect, kszMaxConnections, OPT_MAX_CONNECTIONS);
  s_iVerbosity = (int)SWS_CfgGetValue( kszHttpSect, kszLogVerbosity, iVerbosity);
  s_logfile = logfile;

  /* Open server socket */
  s = socket( ((const struct sockaddr*)pHost)->sa_family, SOCK_STREAM, 0);
  if ( INVALID_SOCKET == s)
    {
    iRet = GetLastError();
    Log( kHttpError, NULL, "ERROR %d: socket: %s\n", iRet, strerror( iRet));
    /* NB free s_pActions as this is fatal */
    goto error1;
    }

  /* Server port specifed? */
  if ( AF_INET == ((const struct sockaddr*)pHost)->sa_family
    && 0 == ((const struct sockaddr_in*)pHost)->sin_port
  ) {
    /* No, set default */
    ((struct sockaddr_in*)pHost)->sin_port = htons( HTTPD_PORT);
    }

  /* Select the port to receive on */
  iRet = bind( s, (const struct sockaddr*)pHost, sizeof(struct sockaddr) );
  if ( iRet)
    {
    iRet = GetLastError();
    Log( kHttpError, (const struct sockaddr_in*)pHost, "ERROR %d: bind: %s\n",
      iRet, strerror( iRet) );
    closesocket( s);
    /* NB don't free s_pActions as we may be restarted */
    return iRet;
    }

  /* Disable Nagle */
  val = 1;
  iRet = setsockopt(
    s,
    IPPROTO_TCP, TCP_NODELAY,
    (char*)&val, sizeof val
  );

  /* Setup a listener */
  iRet = listen( s, maxConn);
  if ( iRet)
    {
    iRet = GetLastError();
    Log( kHttpError, (const struct sockaddr_in*)pHost, "ERROR %d: listen: %s\n",
      iRet, strerror( iRet) );
    goto error;
    }

  /* Get max no. connections */
  if ( maxConn + 1 > FD_SETSIZE)
    {
    Log( kHttpError, NULL, "ERROR: %u connections greater than compilation limit of %u\n",
      (unsigned)maxConn, FD_SETSIZE - 1);
    iRet = ENOMEM;
    goto error;
    }

  /* Allocate free connection list */
  pConnArray = (SHttpConnection*) calloc( maxConn, sizeof( SHttpConnection) );
  if ( NULL == pConnArray)
    {
    iRet = errno;
    Log( kHttpError, NULL, "ERROR %d: calloc for %u connections\n",
      errno, (unsigned)maxConn);
    goto error;
    }

  DEQUE_INIT( &s_active);
  DEQUE_INIT( &s_free);
  pConn = pConnArray;
  for ( val = 0; val < maxConn; ++val)
    {
    DEQUE_INIT( &pConn->list);
    DequeAppend( &s_free, &pConn->list);
    pConn = (SHttpConnection*)((char*)(pConn + 1));
    }

  /* Add standard actions */
  HttpActions( actionArray, lengthof( actionArray) );

#ifdef SIGPIPE
  /* Ignore SIGPIPE if connection reset during send() */
  signal( SIGPIPE, SIG_IGN);
#endif

#if OPT_HTTP_REALMS
  /* Init Realms cache */
  if ( NULL == DEQUE_FIRST( &s_realms))
    DEQUE_INIT( &s_realms);
#endif

#if OPT_HTTP_REDIR
  /* Create URI re-direction table */
  SWS_CfgEnum( kszHttpRedir, &EnumRedirected, &s_redirects);
  if ( NULL != s_redirects.aList && s_redirects.uCount > 0)
    qsort( s_redirects.aList, s_redirects.uCount, sizeof s_redirects.aList[0], CmpRedirect);
#endif

#if OPT_HTTP_ALIAS
  /* Create URI alias table */
  SWS_CfgEnum( kszHttpAlias, &EnumRedirected, &s_aliases);
#else
  SWS_CfgGetString( kszHttpAlias, "/", "/" HTTP_INDEX_FILE, s_szRootRequest, sizeof s_szRootRequest);
#endif

#if OPT_UPNP
  {
  unsigned uExtPort = SWS_CfgGetValue( kszHttpSect, "UpnpExtPort", 0);
  if ( uExtPort && SWS_UpnpAddPortMap(
      "TCP",
      ntohs( ((const struct sockaddr_in*)pHost)->sin_port),
      inet_ntoa( ((const struct sockaddr_in*)pHost)->sin_addr),
      uExtPort,
      "Httpd"
     )
  )
    uExtPort = 0;
#endif

  /* Start the server */
  Log( kHttpWarn, (const struct sockaddr_in*)pHost, "LISTENING\n");
  ++g_running;
  RunServer( s, maxConn);
  --g_running;

  /* Terminate connections */
  pNext = NULL;
  for (
    pDeque = DEQUE_FIRST( &s_active);
    pDeque != &s_active;
    pDeque = pNext
  ) {
    pNext = DEQUE_FIRST( pDeque);
    pConn = LIST2CONN( pDeque);
    setsockasync( pConn->s, 0);
    SessionAbort( pConn);
    SessionFinish( pConn);
    closesocket( pConn->s);
    }

#if OPT_UPNP
  if ( uExtPort)
    SWS_UpnpDelPortMap( "TCP", uExtPort);
  }
#endif

#if OPT_HTTP_ALIAS
  /* Free the alias cache */
  free( s_aliases.aList), s_aliases.aList = NULL;
#endif

#if OPT_HTTP_REDIR
  /* Free the redirection cache */
  free( s_redirects.aList), s_redirects.aList = NULL;
#endif

  /* Free the connection list */
  free( pConnArray);

error:
  closesocket( s);

error1:
  /* Free the URI actions list */
  free( s_pActions), s_pActions = NULL, s_uActions = 0;

#if OPT_HTTP_REALMS
  /* Free the realm cache */
  if ( NULL != DEQUE_FIRST( &s_realms))
    {
    while ( &s_realms != (pDeque = DEQUE_FIRST( &s_realms)))
      {
      SRealm* pRealm = LIST2REALM( pDeque);
      DequeRemove( pDeque);
      free( pRealm->aPermitHost);
      free( pRealm->aDenyHost);
      free( pRealm->aAcct);
      free( pRealm->aTime);
      free( pRealm);
      }
    }
#endif

  return iRet;
  }


static const char* StateName( EState eState)
  {
  switch ( eState)
    {
  case kStateIdle:    return "Idle";
  case kStateQuit:    return "Quit";
  case kStateRequest: return "Request";
  case kStateHeader:  return "Header";
  case kStateExecute: return "Execute";
  case kStateSend:    return "Send";
#if !OPT_HTTP_TINY
  case kStateRecv:    return "Recv";
  case kStateResponse: return "Response";
  case kStateRespHdr: return "RespHdr";
  case kStateConnect: return "Connect";
#endif
    }
  return "Unknown";
  }


/*
 * The server loop
 */
static void RunServer( SOCKET s, size_t maxConn)
  {
  SHttpConnection* pConn;
  SDeque* pDeque, *pNext;
#if !OPT_HTTP_TINY
  size_t srvConn = 0; /* No. active server connections */
  /* Test if proxy requests supported */
  const int isProxy = 0 < FindCommand( NULL, kszHttpGet, "http://x/");
#else
  (void)maxConn;
#endif

  while ( !g_exit)
    {
    static fd_set setRd, setWr, setEx;
    time_t t;
    struct timeval tval;

#ifdef _WINDOWS /* QuickWin */
    _wyield();
#endif

    FD_ZERO( &setEx);
    FD_ZERO( &setWr);
    FD_ZERO( &setRd);
    if ( !DEQUE_IS_EMPTY( &s_free)
#if !OPT_HTTP_TINY
      && (!isProxy || srvConn * 2 < maxConn) /* Keep some free conns for proxy */
#endif
    ) {
      FD_SET( s, &setRd);
      }

    /* Add active sockets to the select sets */
    for (
      pDeque = DEQUE_FIRST( &s_active);
      pDeque != &s_active;
      pDeque = DEQUE_FIRST( pDeque)
    ) {
      pConn = LIST2CONN( pDeque);
      switch ( pConn->eState)
        {
      case kStateIdle:
      case kStateQuit:
      case kStateExecute:
        break;

      case kStateSend:
        FD_SET( pConn->s, &setWr);
        break;

      case kStateRequest:
      case kStateHeader:
#if !OPT_HTTP_TINY
      case kStateRecv:
      case kStateResponse:
      case kStateRespHdr:
#endif
        FD_SET( pConn->s, &setRd);
        break;
#if !OPT_HTTP_TINY
      case kStateConnect:
        FD_SET( pConn->s, &setWr);
        FD_SET( pConn->s, &setEx);
        break;
#endif
        }
      }

    /* Set timeout for select */
    tval.tv_usec = (POLL_TIMEOUT % 1000) * 1000UL;
    tval.tv_sec = POLL_TIMEOUT / 1000;

    /* Wait for listen event */
    if ( SOCKET_ERROR == select( FD_SETSIZE, &setRd, &setWr, &setEx, &tval))
      continue;

    t = time( NULL);

    if ( FD_ISSET( s, &setRd))
      {
      socklen_t len;

      /* Incoming connection */
      pConn = LIST2CONN( DequeRemove( DEQUE_FIRST( &s_free) ));

      /* Accept the incoming connection */
      len = sizeof pConn->client;
      pConn->s = accept( s, (struct sockaddr*)&pConn->client, &len);
      if ( INVALID_SOCKET == pConn->s)
        {
        int iRet = GetLastError();
        Log( kHttpError, NULL, "ERROR %d: accept: %s\n", iRet, strerror( iRet));
        DequeAppend( &s_free, &pConn->list);
        }
      else
        {
        if ( s_logfile != stdout && s_logfile != stderr)
          Log( kHttpError, &pConn->client, "CONNECT\n");

        /* Initialise connection data */
        pConn->tIdle = t + s_uIdleTimeout; /* Start the idle timer */
        pConn->rqst.tRequest = t;

        DequeAppend( &s_active, &pConn->list);

        /* Enable non-blocking mode */
        setsockasync( pConn->s, 1);

        /* Set a send timeout */
        { DECL_TIMEOUT( tv, SEND_TIMEOUT);
        setsockopt(
          pConn->s,
          SOL_SOCKET, SO_SNDTIMEO,
          (char*)&tv, sizeof tv
        ); }

        /* Start a session */
        SessionStart( pConn);
#if !OPT_HTTP_TINY
        pConn->isServer = 1;
        ++srvConn;
#endif
        }
      }

    /* Scan active connections for an event */
    pNext = NULL;
    for (
      pDeque = DEQUE_FIRST( &s_active);
      pDeque != &s_active;
      pDeque = pNext
    ) {
      pNext = DEQUE_FIRST( pDeque);
      pConn = LIST2CONN( pDeque);

      if ( FD_ISSET( pConn->s, &setWr))
        {
        /* More output */
        switch ( pConn->eState)
          {
#if !OPT_HTTP_TINY
        case kStateConnect:
          HttpConnected( pConn);
          pConn->tIdle = t + s_uIdleTimeout; /* Restart the idle timer */
          break;
#endif
        default:
          if ( SessionOutput( pConn) > 0 && kStateSend == pConn->eState)
            pConn->tIdle = t + s_uIdleTimeout; /* Restart the idle timer */
          break;
          }
        }

      if ( FD_ISSET( pConn->s, &setEx))
        {
        /* Restart the idle timer */
        pConn->tIdle = t + s_uIdleTimeout;

#if !OPT_HTTP_TINY
        switch ( pConn->eState)
          {
        case kStateConnect:
          HttpConnected( pConn);
          break;
        default:
          break;
          }
#endif
        }

      if ( FD_ISSET( pConn->s, &setRd))
        {
        /* Restart the idle timer */
        pConn->tIdle = t + s_uIdleTimeout;

        /* More input */
        SessionInput( pConn);
        }
      else if ( INVALID_TIME != pConn->tIdle && t > pConn->tIdle)
        {
        /* Timed out */
        switch ( pConn->eState)
          {
        case kStateIdle:
        case kStateQuit:
        case kStateRequest:
          Log( kHttpMore, &pConn->client, "Idle timeout\n");
          break;
        case kStateHeader:
        case kStateExecute:
        case kStateSend:
#if !OPT_HTTP_TINY
        case kStateRecv:
        case kStateResponse:
        case kStateRespHdr:
        case kStateConnect:
#endif
          Log( kHttpError, &pConn->client, "Unexpected idle timeout in state %s\n",
            StateName( pConn->eState));
          break;
          }

        SessionAbort( pConn);
        }

      /* Is the session finished? */
      if ( kStateQuit == pConn->eState)
        {
        /* Yes */
        if ( s_logfile != stdout && s_logfile != stderr)
          Log( kHttpError, &pConn->client, "DISCONNECT\n");

        /* Disable non-blocking mode */
        setsockasync( pConn->s, 0);

        SessionFinish( pConn);
        closesocket( pConn->s);

        /* Return it to the free list */
        DequeRemove( &pConn->list);
        DequeAppend( &s_free, &pConn->list);
#if !OPT_HTTP_TINY
        if ( pConn->isServer)
          --srvConn;
#endif
        }
      }
    }
  }


/*
 * Start a session
 */
static void SessionStart( SHttpConnection* pConn)
  {
  Log( kHttpMost, &pConn->client, "CONNECT-FROM\n");

  NewConnection( pConn);
  pConn->eState = kStateRequest;
  }


/*
 * End a session
 */
static void SessionFinish( SHttpConnection* pConn)
  {
  Log( kHttpMost, &pConn->client, "DISCONNECT\n");

  while ( pConn->sizePending)
    SessionOutput( pConn);

  /* Send FIN */
  shutdown( pConn->s, SD_SEND);

#if OPT_SAVE_BODY
  if ( NULL == pConn->fileBody)
    {
    fclose( pConn->fileBody);
    pConn->fileBody = NULL;
    }
#endif

  /* Tidy up on error */
  if ( pConn->pfnAction)
    (*pConn->pfnAction)( NULL, &pConn->rqst);
  }


/*
 * Abort a session
 */
static void SessionAbort( SHttpConnection* pConn)
  {
  struct linger lgr;

  /* Reset the connection when closed */
  lgr.l_onoff = 1;
  lgr.l_linger = 0;
  setsockopt( pConn->s, SOL_SOCKET, SO_LINGER, (char*)&lgr, sizeof lgr);

  pConn->eState = kStateQuit;
  }


/*
 * Session input ready
 */
static void SessionInput( SHttpConnection* pConn)
  {
  switch ( pConn->eState)
    {
  case kStateRequest:
    HttpGetRequest( pConn);
    break;

  case kStateHeader:
    HttpGetHeader( pConn);
    break;

#if !OPT_HTTP_TINY
  case kStateRecv:
    HttpGetBody( pConn);
    break;

  case kStateConnect:
    break;

  case kStateResponse:
    HttpGetResponse( pConn);
    break;

  case kStateRespHdr:
    HttpGetHeader( pConn);
    break;
#endif

  case kStateIdle:
  case kStateExecute:
  case kStateSend:
  case kStateQuit:
    break;
    }
  }


/*
 * Session output ready
 */
static int SessionOutput( SHttpConnection* pConn)
  {
  FHttpMethod* pfnAction;
  int iRet = -1;

  if ( kStateQuit == pConn->eState
    || SOCKET_ERROR == (iRet = WriteRaw( pConn, pConn->pPending, pConn->sizePending))
  ) {
    /* Error termination */
    pConn->eState = kStateQuit;
    pConn->sizePending = 0;
    pfnAction = pConn->pfnAction;
    if ( pfnAction)
      {
      pConn->pfnAction = 0;
      (*pfnAction)( NULL, &pConn->rqst);
      }
    }
  else
    {
    /* Deduct amount sent from that pending */
    pConn->pPending += (size_t)iRet;
    pConn->sizePending -= (size_t)iRet;
    if ( 0 == pConn->sizePending)
      {
      /* Read next block */
      pfnAction = pConn->pfnAction;
      if ( pfnAction)
        {
        pConn->pfnAction = 0;
        (*pfnAction)( pConn, &pConn->rqst);
        }
      }
    }
  return iRet;
  }


/*
 * Initialise a connection
 */
static void NewConnection( SHttpConnection* pConn)
  {
  pConn->eState = kStateIdle;
#if !OPT_HTTP_TINY
  pConn->isServer = 0;
#endif
#if OPT_HTTP_REALMS
  pConn->pRealm = NULL;
  pConn->pfnAuthAction = 0;
  pConn->isAuth = 0;
#endif
  pConn->pfnAction = 0;

  pConn->eTelnetState = kTelnetData;
  pConn->telnetCR = pConn->telnetEOL = 0;
  pConn->telnetCount = 0;
  pConn->pTelnetInput = pConn->input;
  pConn->telnetSize = sizeof pConn->input;

  pConn->lContent = 0;
#if OPT_SAVE_BODY
  pConn->fileBody = NULL;
#endif
  pConn->sizePending = 0;
  }


/*
 * End of request
 */
static void EndRequest( SHttpConnection* pConn)
  {
  if ( pConn->rqst.iConnClose)
    {
    pConn->eState = kStateQuit;
    }
  else
    {
    pConn->eState = kStateRequest;
    pConn->tIdle = time( NULL) + s_uKeepTimeout;
    }
  }


/*
 * Get an HTTP request
 */
void HttpGetRequest(
  SHttpConnection* pConn
) {
  int iRet;
  char* psz;

  /* Input a line */
  iRet = TelnetReadLine( pConn);
  if ( 0 == iRet)
    return; /* Not complete */
  else if ( SOCKET_ERROR == iRet)
    {
    /* Disconnected */
    if ( pConn->telnetCount > 0)
      Log( kHttpWarn, &pConn->client,
        "WARN: Unexpected disconnect in request: %.*s\n",
         (int)pConn->telnetCount, pConn->pTelnetInput
      );
    pConn->eState = kStateQuit;
    return;
    }

  if ( !pConn->telnetEOL)
    {
    /* Invalid input */
    pConn->eState = kStateQuit;
    return;
    }

  /* Init the request context */
  pConn->rqst.tRequest = time( NULL);
#if OPT_HTTP_REALMS
  pConn->isAuth = 0;
#endif
  pConn->rqst.uMajor = 0;
  pConn->rqst.uMinor = 9;
  pConn->rqst.sizeRequest = (size_t)iRet;
  pConn->rqst.pszMethod = NULL;
  pConn->pszURI = pConn->rqst.pszPath = "";
  pConn->pszQuery = pConn->rqst.pszQuery = NULL;
  pConn->pszParam = pConn->rqst.pszParam = NULL;
  pConn->rqst.pszTail = "";
  pConn->rqst.pszHeaders = pConn->header + iRet;
  pConn->header[ iRet] = cNul;
  pConn->rqst.sizeHeaders = 0;

  pConn->rqst.iConnClose = -1; /* Close connection when complete */
  pConn->rqst.tIfModifiedSince = INVALID_TIME;
  pConn->rqst.lContent = -1; /* Unspecified length */
  pConn->rqst.sizeAvail = 0;
  pConn->rqst.uCode = 0; /* !0= proxy request */

  /* Parse the request */
  psz = (char*)strncpy( pConn->header, pConn->pTelnetInput, sizeof pConn->header);
  Log( kHttpMost, &pConn->client, "<= %s\n", psz);

  /* Strip leading whitespace from method */
  while ( isspace( *psz))
    ++psz;

  /* Split method from Request-URI */
  pConn->rqst.pszMethod = psz;
  while ( cNul != *psz && !isspace( *psz) )
    {
    /* Ensure method is uppercase */
    *psz = (char)toupper( *psz);
    ++psz;
    }

  if ( cNul != *psz)
    {
    *(psz++) = cNul; /* Nul terminate the method */
    }
  else
    {
    HttpReply( pConn, HTTPD_BAD_REQUEST, "Missing URI", 0, NULL);
    return;
    }

  /* Strip leading whitespace from Request-URI */
  while ( isspace( *psz) )
    ++psz;

  /* Split the URI from the HTTP version */
  pConn->pszURI = psz;
  while ( cNul != *psz && !isspace( *psz) )
    ++psz;

  if ( cNul != *psz)
    {
    *(psz++) = cNul; /* Nul terminate the URI */

    /* Strip leading whitespace from Request-tail */
    while ( isspace( *psz) )
      ++psz;

    pConn->rqst.pszTail = psz;

    /* Read HTTP-Version */
    if ( 2 == sscanf( psz, "HTTP/%u.%u", &pConn->rqst.uMajor, &pConn->rqst.uMinor))
      {
      if ( pConn->rqst.uMajor > 1
        || (1 == pConn->rqst.uMajor && pConn->rqst.uMinor >= 1)
      ) {
        /* HTTP 1.1 maintains connection by default */
        pConn->rqst.iConnClose = 0;
        }

      /* General and request headers next */
      pConn->eState = kStateHeader;
      }
    else if ( cNul != *psz)
      {
      HttpReply( pConn, HTTPD_BAD_REQUEST, "Bad request", 0, NULL);
      return;
      }
    }

  /* Split query from URI */
  psz = (char*)strchr( pConn->pszURI, '?');
  if ( NULL != psz)
    *(psz++) = cNul;
  pConn->pszQuery = pConn->rqst.pszQuery = psz;

  /* Split parameters from URI */
  psz = (char*)strchr( pConn->pszURI, ';');
  if ( NULL != psz)
    *(psz++) = cNul;
  pConn->pszParam = pConn->rqst.pszParam = psz;

  /* Lookup the method/URI */
  FindCommand( pConn, pConn->rqst.pszMethod, pConn->pszURI);

  if ( kStateRequest == pConn->eState)
    {
    /* Execute the request */
    pConn->eState = kStateExecute;
    (*pConn->pfnAction)( pConn, &pConn->rqst);
    }
  }


/*
 * Find a command descriptor given an HTTP request
 */
size_t FindCommand(
  SHttpConnection* pConn,
  const char* pszMethod,
  const char* pszPath
) {
  const SHttpAction* pAction;
  const char* pszRealm;
  FHttpMethod* pfnBestMethod;
  size_t count, matchLen;

#if OPT_HTTP_ALIAS
  if ( NULL != pConn)
    {
    const struct SRedirect* pRedir;

    /* Check for aliases */
    while ( NULL != (pRedir = FindAlias( pszPath)))
      {
      Log( kHttpMore, &pConn->client, "%s %s ALIASED-TO ", pszMethod, pszPath);
      pszPath = AliasURI( pConn, pRedir, pszPath);
      Log( kHttpMore, NULL, "&%s\n", pszPath);
      if ( '/' == pRedir->szOld[0])
        break;
      }
    }
#else
  /* Substitute root request for local root */
  if ( 0 == strcmp( pszPath, "/"))
    pszPath = s_szRootRequest;
#endif

#if OPT_HTTP_REDIR
  if ( NULL != pConn)
    {
    const struct SRedirect* pRedir;

    /* Check for redirects */
    if ( NULL != s_redirects.aList && s_redirects.uCount > 0
      && NULL != (pRedir = (struct SRedirect*)bsearch(
        pszPath, s_redirects.aList, s_redirects.uCount, sizeof s_redirects.aList[0], CmpRedirect))
    ) {
      /* NB passing pszPath as the key requires the following */
      STATIC_ASSERT( 0 == offsetof( struct SRedirect, szOld));
      pConn->pfnAction = &UriRedirect;
      pConn->rqst.pszPath = pRedir->szNew;
      return 1;
      }
    }
#endif

  pfnBestMethod = &UriNotFound;
  pszRealm = NULL;
  matchLen = 0;

  /* Search action list for best URI match */
  for (
    pAction = s_pActions, count = s_uActions;
    count > 0;
    --count, ++pAction
  ) {
    /* Does this action support the URI? */
    if ( SWS_StrMatch( pAction->pszStem, pszPath, 0)
      && ('/' != pszPath[0] || '/' == pAction->pszStem[0])
    ) {
      /* Is this action a better match than the current best? */
      size_t const len = strlen( pAction->pszStem);
      if ( len >= matchLen)
        {
        const SHttpMethod* pMethod;
        size_t j;

        if ( len > matchLen)
          pfnBestMethod = &MethodNotImplemented;

        /* Does this action support the requested method? */
        for (
          pMethod = pAction->methods, j = lengthof( pAction->methods);
          j > 0 && NULL != pMethod->pszName;
          --j, ++pMethod
        ) {
          if ( 0 == strcmp( pszMethod, pMethod->pszName))
            {
            /* Make this the current best */
            pfnBestMethod = pMethod->pfnExecute;
            pszRealm = pMethod->pszRealm;
            matchLen = len;
            break;
            }
          }
        }
      }
    }

  if ( NULL != pConn)
    {
    pConn->rqst.pszPath = pszPath;
#if OPT_HTTP_REALMS
    pConn->pfnAction = GetRealmAction( pConn, pszRealm, pfnBestMethod);
#else
    pConn->pfnAction = pfnBestMethod;
#endif
    }

  return matchLen;
  }


/*
 * Read HTTP request/response headers
 */
void HttpGetHeader(
  SHttpConnection* pConn
) {
  int iRet;
  char* psz;

  /* Input a line */
  iRet = TelnetReadLine( pConn);
  if ( 0 == iRet)
    return; /* Not complete */
  else if ( SOCKET_ERROR == iRet)
    {
    /* Disconnected */
    Log( kHttpWarn, &pConn->client, "WARN: Unexpected disconnect within header\n");
    pConn->eState = kStateQuit;
    return;
    }

  psz = pConn->pTelnetInput;
  if ( cNul != psz[ 0])
    {
    size_t const sizeHeaders = pConn->rqst.sizeHeaders;
    size_t len;
    int isDiscard = 0;
    char* pszValue, *pszHeader;

    Log( kHttpMost, &pConn->client, "<=%s\n", psz);

    /* Buffer the entity-header */
    pszHeader = (char*)pConn->rqst.pszHeaders + sizeHeaders;
    len = strlen( psz);
    if ( pszHeader + len + sizeof CRLF <= pConn->header + sizeof pConn->header)
      {
      strcpy ( pszHeader, psz);
      if ( !pConn->telnetEOL)
        {
        pConn->rqst.sizeHeaders += len;
        }
      else
        {
        /* Append CRLF */
        strcpy( pszHeader + len, CRLF);
        pConn->rqst.sizeHeaders += len + sizeof CRLF - 1;
        }
      }

    /* NB HTTP/1.0 headers may be folded onto multiple lines if each
     * continuation line begins with a space or horizontal tab */
    if ( isspace( *psz))
      {
      Log( kHttpMore, &pConn->client, "NOTE: folded header\n");
      return;
      }

    /* Split HTTP-header into field-name & field-value */
    for ( pszValue = psz; cNul != *pszValue; ++pszValue)
      {
      if ( ':' == *pszValue)
        {
        /* Nul terminate field-name */
        *(pszValue++) = cNul;
        break;
        }
      if ( isspace( *pszValue) )
        *pszValue = cNul; /* Nul terminate the field-name */
      }

    if ( cNul != *pszValue)
      {
      /* Strip leading whitespace from field-value */
      while ( isspace( *pszValue))
        ++pszValue;

      /* Entity headers */
      if ( 0 == SWS_stricmp( psz, kszHttpContentLen))
        {
        pConn->rqst.lContent = strtol( pszValue, &pszValue, 0);
        isDiscard = 1;
        }
#if !OPT_HTTP_TINY
      else if ( 0 == SWS_stricmp( psz, kszHttpContentType))
        {
        /* Overwrite field-name so that httpcgi can use strstr */
        memcpy( pszHeader, kszHttpContentType, sizeof kszHttpContentType - 1);
        }
#endif
#if 0 && !OPT_HTTP_TINY
      else if ( 0 == SWS_stricmp( psz, kszHttpVia))
        {
        /* Overwrite field-name so that HttpReply can use strstr */
        memcpy( pszHeader, kszHttpVia, sizeof kszHttpVia - 1);
        /* TODO append our hoostname */
        }
#endif
      /* General headers */
      else if ( 0 == SWS_stricmp( psz, kszHttpConnection))
        {
        /* Connection persistence */
        if ( 0 == SWS_stricmp( pszValue, kszHttpConnectionClose))
          {
#if !OPT_HTTP_TINY
          if ( pConn->isServer)
#endif
            pConn->rqst.iConnClose = 1;
          }
        else if ( 0 == SWS_stricmp( pszValue, kszHttpConnectionKeep))
          {
#if !OPT_HTTP_TINY
          if ( pConn->isServer)
#endif
            pConn->rqst.iConnClose = 0;
          }
        isDiscard = 1; /* RFC 2616 para 13.5.1 - Hop-by-hop */
        }
#if !OPT_HTTP_TINY
      else if ( 0 == SWS_stricmp( psz, kszHttpHost))
        {
        /* Overwrite field-name so that httpcgi can use strstr */
        memcpy( pszHeader, kszHttpHost, sizeof kszHttpHost - 1);

#if 0   /* TODO Dispatch to originating host RFC 2616 14.23 */
        FindCommand( pConn, pConn->rqst.pszMethod, pConn->pszURI);
        /* Re-authenticate */
        if ( pConn->pfnAuthAction && pConn->isAuth)
          pConn->pfnAction = pConn->pfnAuthAction;
#endif
        }
      else if ( 0 == SWS_stricmp( psz, kszHttpKeepAlive))
        isDiscard = 1; /* RFC 2616 para 13.5.1 - Hop-by-hop */
      else if ( 0 == SWS_stricmp( psz, kszHttpTE))
        isDiscard = 1; /* RFC 2616 para 13.5.1 - Hop-by-hop */
      else if ( 0 == SWS_stricmp( psz, kszHttpTrailer))
        isDiscard = 1; /* RFC 2616 para 13.5.1 - Hop-by-hop */
#if 0 /* Needed by proxy client to decode compressed body */
      else if ( 0 == SWS_stricmp( psz, kszHttpTransferEncoding))
        isDiscard = 1; /* RFC 2616 para 13.5.1 - Hop-by-hop */
#endif
      else if ( 0 == SWS_stricmp( psz, kszHttpUpgrade))
        isDiscard = 1; /* RFC 2616 para 13.5.1 - Hop-by-hop */
      else if ( 0 == SWS_stricmp( psz, kszHttpProxyConnection))
        isDiscard = 1;
      /* Request headers */
      else if ( 0 == SWS_stricmp( psz, kszHttpIfModified))
        pConn->rqst.tIfModifiedSince = SWS_TimeFromStr( pszValue);
#endif /* !OPT_HTTP_TINY */
      else if ( 0 == SWS_stricmp( psz, kszHttpProxyAuthorization))
        {
        isDiscard = 1; /* RFC 2616 para 13.5.1 - Hop-by-hop */
#if OPT_HTTP_REALMS
        if ( pConn->pfnAuthAction)
          switch ( Authenticate( pConn->pRealm, kszHttpProxyAuthorization, pszValue))
            {
          case 1: pConn->pfnAction = pConn->pfnAuthAction; break;
          case -1: pConn->pfnAction = &Forbidden; break;
            }
#endif
        }
      else if ( 0 == SWS_stricmp( psz, kszHttpAuthorization))
        {
#if !OPT_HTTP_TINY
        /* Overwrite field-name so that httpcgi can use strstr */
        memcpy( pszHeader, kszHttpAuthorization, sizeof kszHttpAuthorization - 1);
#endif
#if OPT_HTTP_REALMS
        if ( pConn->pfnAuthAction)
          switch ( Authenticate( pConn->pRealm, kszHttpAuthorization, pszValue))
            {
          case 1: pConn->pfnAction = pConn->pfnAuthAction; pConn->isAuth = 1; break;
          case -1: pConn->pfnAction = &Forbidden; break;
            }
#endif
        }
      }
    else
      Log( kHttpInfo, &pConn->client, "<= Missing value: %s\n", psz);

    if ( isDiscard)
      {
      /* Discard this header */
      *pszHeader = cNul;
      pConn->rqst.sizeHeaders = sizeHeaders;
      }
    }
  else
    {
    /* End of headers */
    Log( kHttpMost, &pConn->client, "End of headers\n");

#if 0 && !OPT_HTTP_TINY
    if ( !pConn->isServer && s_iVerbosity >= kHttpMost)
      {
      /* Proxy request complete */
      LogResponse( pConn, pConn->rqst.uCode, pConn->rqst.pszResponse,
        pConn->rqst.lContent, pConn->rqst.pszHeaders);
      }
#endif

    /* Execute the request */
    pConn->eState = kStateExecute;
    assert( 0 != pConn->pfnAction);
    (*pConn->pfnAction)( pConn, &pConn->rqst);
    }
  }


/**
 * Send an HTTP status line and optional headers
 */
extern int HttpReply(
  SHttpConnection* pConn,
  int code,                           /* IN: Status code */
  const char* pszStatus,              /* IN: Status message */
  long lContent,                      /* IN: Content length */
  const char* pszInfo                 /* IN: Additional headers or NULL */
) {
  char* pszBuf;

  if ( !pConn)
    return code;

  LogResponse( pConn, code, pszStatus, lContent, pszInfo);

  if ( code >= 500 || lContent < 0 || !strcmp( pConn->rqst.pszMethod, kszHttpHead))
    pConn->rqst.iConnClose = 1;

  if ( pConn->rqst.uMajor >= 1
    && kStateQuit != pConn->eState
  ) {
    /* Format response */
    pszBuf = pConn->input;
    pszBuf += sprintf(
      pszBuf,
      "HTTP/1.%u %d %.64s" CRLF, /* Status line */
      (pConn->rqst.uMajor > 1 || (pConn->rqst.uMajor == 1 && pConn->rqst.uMinor >= 1)) ? 1 : 0,
      code, pszStatus
    );

    /* Format response headers */
    if ( 0 == pConn->rqst.uCode)
      {
      pszBuf += sprintf( pszBuf, kszHttpServer ": " kszServerName CRLF);
#if !OPT_HTTP_TINY
      { char szTime[40];
      pszBuf += sprintf( pszBuf, kszHttpDate ": %s" CRLF,
        SWS_TimeToStr( szTime, sizeof szTime, time( NULL)) ); }

      /* Let client know we don't handle partial range requests */
      if ( lContent > 0
        && (pConn->rqst.uMajor > 1 || (pConn->rqst.uMajor == 1 && pConn->rqst.uMinor >= 1))
        && ( NULL == pszInfo || NULL == strstr( pszInfo, kszHttpAcceptRange))
      )
        pszBuf += sprintf( pszBuf, kszHttpAcceptRange ": " kszHttpAcceptRangeNone CRLF);
#endif

#if 1 /* BUG The ContentType header is needed by Firefox 3.5.5 to display errors */
      if ( code >= 400
        && (NULL == pszInfo || NULL == strstr( pszInfo, kszHttpContentType ":"))
      )
        pszBuf += sprintf( pszBuf, kszHttpContentType ": application/octet-stream" CRLF);
#endif
      }
#if 0 && !OPT_HTTP_TINY
    else if ( NULL == pszInfo || NULL == strstr( pszInfo, kszHttpVia))
      pszBuf += sprintf( pszBuf, kszHttpVia ": 1.0 %s (" kszServerName ")" CRLF, s_szHostname);
#endif

    if ( lContent > 0 || (0 == lContent && !pConn->rqst.iConnClose))
      pszBuf += sprintf( pszBuf, kszHttpContentLen ": %ld" CRLF, lContent);

    if ( pConn->rqst.iConnClose)
      pszBuf += sprintf( pszBuf, kszHttpConnection ": " kszHttpConnectionClose CRLF );
    else
      pszBuf += sprintf( pszBuf, kszHttpConnection ": " kszHttpConnectionKeep CRLF );

    if ( NULL == pszInfo || cNul == *pszInfo)
      pszBuf += sprintf( pszBuf, CRLF);

    /* Disable non-blocking mode */
    setsockasync( pConn->s, 0);

    /* Send the respnse */
    if ( WriteRaw( pConn, pConn->input, (size_t)(pszBuf - pConn->input)) == SOCKET_ERROR)
      pConn->eState = kStateQuit;
    else if ( NULL != pszInfo && *pszInfo)
      {
      /* Send additional headers */
      if ( WriteRaw( pConn, pszInfo, strlen( pszInfo)) == SOCKET_ERROR
        || WriteRaw( pConn, CRLF, sizeof CRLF - 1) == SOCKET_ERROR
      ) {
        pConn->eState = kStateQuit;
        }
      }

    /* Re-enable non-blocking mode */
    setsockasync( pConn->s, 1);
    }

  pConn->pfnAction = 0;
  if ( 0 == lContent || !strcmp( pConn->rqst.pszMethod, kszHttpHead))
    EndRequest( pConn);

  return code;
  }


/*
 * Log a response
 */
static void LogResponse(
  SHttpConnection* pConn,
  int code,                           /* IN: Status code */
  const char* pszStatus,              /* IN: Status message */
  long lContent,                      /* IN: Content length */
  const char* pszInfo                 /* IN: Headers or NULL */
) {
  int n;
  const char *pszQuery, *pszQuerySep, *pszParam, *pszParamSep, *psz;
  const enum EHttpLog eLog = code < 400 ? kHttpInfo : kHttpWarn;

  if ( NULL == (pszQuery = pConn->pszQuery))
    {
    pszQuery = "";
    pszQuerySep = "";
    }
  else
    {
    pszQuerySep = "?";
    }

  if ( NULL == (pszParam = pConn->pszParam))
    {
    pszParam = "";
    pszParamSep = "";
    }
  else
    {
    pszParamSep = ";";
    }

  /* Log the request */
  Log( eLog, &pConn->client, "%s %s%s%s%s%s",
    pConn->rqst.pszMethod, pConn->pszURI,
    pszQuerySep, pszQuery,
    pszParamSep, pszParam
  );

  /* Log the request headers */
  if ( pConn->rqst.lContent >= 0
#if !OPT_HTTP_TINY
    && pConn->isServer
#endif
  ) Log( eLog, NULL, "& [[" kszHttpContentLen ":%ld]]", pConn->rqst.lContent);

  psz = pConn->rqst.pszHeaders;
  if ( NULL != psz
#if !OPT_HTTP_TINY
    && pConn->isServer
#endif
  ) while ( 1 <= sscanf( psz,
      " %"STRING(MAX_LINE)"[^"CRLF"]%n", pConn->input, &n)
  ) {
    enum EHttpLog eLogHdr = eLog;

    psz += n;

    if ( 0 == SWS_strnicmp( pConn->input, "X-", 2 )
      || 0 == SWS_strnicmp( pConn->input, "If-", 3 )
      || 0 == SWS_strnicmp( pConn->input, kszHttpAccept ":", sizeof kszHttpAccept )
      || 0 == SWS_strnicmp( pConn->input, kszHttpAcceptCharset ":", sizeof kszHttpAcceptCharset )
      || 0 == SWS_strnicmp( pConn->input, kszHttpAcceptEncoding ":", sizeof kszHttpAcceptEncoding )
      || 0 == SWS_strnicmp( pConn->input, kszHttpPragma ":", sizeof kszHttpPragma )
      || 0 == SWS_strnicmp( pConn->input, kszHttpCookie ":", sizeof kszHttpCookie)
      || 0 == SWS_strnicmp( pConn->input, kszHttpETag ":", sizeof kszHttpETag )
    ) {
      eLogHdr = kHttpMore;
      }
    else if ( 0 == SWS_strnicmp( pConn->input, kszHttpHost ":", sizeof kszHttpHost )
      || 0 == SWS_strnicmp( pConn->input, kszHttpCacheControl ":", sizeof kszHttpCacheControl )
    ) {
      eLogHdr = kHttpMost;
      }

    Log( eLogHdr, NULL, "& [[%s]]", pConn->input);
    }

  /* Log the response */
  Log( eLog, &pConn->client, "& %d \"%s\"", code, pszStatus);

  /* Log the response headers */
  if ( lContent > 0 || (0 == lContent && !pConn->rqst.iConnClose))
    Log( eLog, NULL, "& [[" kszHttpContentLen ":%ld]]", lContent);

#if !OPT_HTTP_TINY
  if ( pConn->isServer)
#endif
    Log( kHttpMore, NULL, "& [[" kszHttpConnection ":%s]]",
      pConn->rqst.iConnClose ? kszHttpConnectionClose : kszHttpConnectionKeep);

  if ( NULL != pszInfo) while ( 1 <= sscanf( pszInfo,
      " %"STRING(MAX_LINE)"[^"CRLF"]%n", pConn->input, &n)
  ) {
    enum EHttpLog eLogHdr = eLog;

    pszInfo += n;

    if ( 0 == SWS_strnicmp( pConn->input, kszHttpServer ":", sizeof kszHttpServer)
      || 0 == SWS_strnicmp( pConn->input, kszHttpSetCookie ":", sizeof kszHttpSetCookie)
      || 0 == SWS_strnicmp( pConn->input, kszHttpSetCookie2 ":", sizeof kszHttpSetCookie2)
      || 0 == SWS_strnicmp( pConn->input, kszHttpP3P ":", sizeof kszHttpP3P)
    ) {
      eLogHdr = kHttpMore;
      }
    else if ( 0 == SWS_strnicmp( pConn->input, kszHttpDate ":", sizeof kszHttpDate)
      || 0 == SWS_strnicmp( pConn->input, kszHttpLastModified ":", sizeof kszHttpLastModified)
      || 0 == SWS_strnicmp( pConn->input, kszHttpExpires ":", sizeof kszHttpExpires )
      || 0 == SWS_strnicmp( pConn->input, kszHttpAge ":", sizeof kszHttpAge )
      || 0 == SWS_strnicmp( pConn->input, kszHttpCacheControl ":", sizeof kszHttpCacheControl )
      || 0 == SWS_strnicmp( pConn->input, kszHttpVary ":", sizeof kszHttpVary )
    ) {
      eLogHdr = kHttpMost;
      }
    else if ( 0 == SWS_strnicmp( pConn->input, "X-", 2 )
      || 0 == SWS_strnicmp( pConn->input, "If-", 3 )
      || 0 == SWS_strnicmp( pConn->input, kszHttpAccept ":", sizeof kszHttpAccept )
      || 0 == SWS_strnicmp( pConn->input, kszHttpAcceptCharset ":", sizeof kszHttpAcceptCharset )
      || 0 == SWS_strnicmp( pConn->input, kszHttpAcceptEncoding ":", sizeof kszHttpAcceptEncoding )
      || 0 == SWS_strnicmp( pConn->input, kszHttpPragma ":", sizeof kszHttpPragma )
      || 0 == SWS_strnicmp( pConn->input, kszHttpCookie ":", sizeof kszHttpCookie)
      || 0 == SWS_strnicmp( pConn->input, kszHttpETag ":", sizeof kszHttpETag )
    ) {
      eLogHdr = kHttpMore;
      }

    Log( eLogHdr, NULL, "& [[%s]]", pConn->input);
    }

  Log( eLog, NULL, "&\n");
  }


/**
 * Async send raw data to client
 */
extern int HttpSend(
  SHttpConnection* pConn,             /* IN: Connection */
  const void* pv,                     /* IN: -> buffer */
  size_t size,                        /* IN: buffer size */
  FHttpMethod* pfnAction              /* IN: Callback */
) {
  if ( pConn)
    {
    pConn->pfnAction = 0;

    while ( pConn->sizePending)
      SessionOutput( pConn);

    if ( kStateQuit != pConn->eState)
      {
      pConn->pPending = (const char*)pv;
      pConn->sizePending = size;

      pConn->pfnAction = pfnAction;
      if ( size || pfnAction)
        pConn->eState = kStateSend;
      else
        EndRequest( pConn);
      }
    else if ( pfnAction)
      (*pfnAction)( NULL, &pConn->rqst);
    }
  return HTTPD_OK;
  }


/**
 * Get local path prefix for a given realm
 */
extern const char* HttpRealmPath(
  SHttpConnection* pConn
) {
#if OPT_HTTP_REALMS
  /*const SRealm* pRealm = pConn ? pConn->pRealm : NULL;*/
  const SRealm* pRealm = pConn->pRealm;
  if ( NULL != pRealm && pRealm->szRoot[0])
    return pRealm->szRoot;
#else
  (void)pConn;
#endif
  return NULL;
  }


#if !OPT_HTTP_TINY
/**
 * Re-direct a request
 */
extern int HttpRedirect(
  SHttpConnection* pConn,             /* IN: Connection */
  const char* pszPath
) {
  const char *pszQuery, *pszQuerySep, *pszParam, *pszParamSep;
  char* psz;

  if ( NULL == (pszQuery = pConn->pszQuery))
    {
    pszQuery = "";
    pszQuerySep = "";
    }
  else
    {
    pszQuerySep = "?";
    }

  if ( NULL == (pszParam = pConn->pszParam))
    {
    pszParam = "";
    pszParamSep = "";
    }
  else
    {
    pszParamSep = ";";
    }

  Log( kHttpMore, &pConn->client, "%s %s%s%s%s%s REDIRECT-TO %s\n",
    pConn->rqst.pszMethod, pConn->pszURI,
    pszQuerySep, pszQuery,
    pszParamSep, pszParam,
    pszPath
  );

  pConn->rqst.lContent = -1;
  pConn->rqst.sizeAvail = 0;

  pszPath = strncpy( pConn->input, pszPath, sizeof pConn->input);

  /* Split query from URI */
  psz = (char*)strchr( pszPath, '?');
  if ( NULL != psz)
    *(psz++) = cNul;
  pConn->rqst.pszQuery = psz;

  /* Split parameters from URI */
  psz = (char*)strchr( pszPath, ';');
  if ( NULL != psz)
    *(psz++) = cNul;
  pConn->rqst.pszParam = psz;

  /* Lookup the method/path */
  FindCommand( pConn, pConn->rqst.pszMethod, pszPath);

#if OPT_HTTP_REALMS
  /* Re-authenticate the request */
  if ( pConn->pfnAuthAction && pConn->isAuth)
    pConn->pfnAction = pConn->pfnAuthAction;
#endif

  /* Execute the request */
  pConn->eState = kStateExecute;
  return (*pConn->pfnAction)( pConn, &pConn->rqst);
  }


/**
 * Async receive raw data. Callback when complete
 */
extern int HttpRecv(
  SHttpConnection* pConn,             /* IN: Connection */
  FHttpMethod* pfnAction              /* IN: Callback */
) {
  if ( pConn)
    {
    pConn->eState = kStateRecv;
    pConn->lContent = pConn->rqst.lContent;
    pConn->pfnAction = pfnAction;
    if ( !pfnAction)
      EndRequest( pConn);
    }
  return HTTPD_OK;
  }


/*
 * Fetch the Entity-Body
 */
static void HttpGetBody(
  SHttpConnection* pConn
) {
  size_t size;
  int iRet;

  size = sizeof pConn->rqst.buffer;
  if ( pConn->lContent >= 0 && size > (size_t)pConn->lContent)
    size = (size_t)pConn->lContent;

  /* Fetch content data */
  iRet = ReadRaw( pConn, pConn->rqst.buffer, size);
  if ( 0 == iRet)
    return; /* Not complete */
  if ( SOCKET_ERROR == iRet)
    {
    /* Disconnected */
    pConn->eState = kStateQuit;
    if ( pConn->lContent > 0)
      {
      Log( kHttpWarn, &pConn->client,
        "WARN: Failed to read body. %ld of %ld fetched\n",
        pConn->rqst.lContent - pConn->lContent, pConn->rqst.lContent
      );
      }
    pConn->rqst.sizeAvail = 0;
    }
  else if ( pConn->lContent > 0)
    {
    pConn->lContent -= iRet;
    pConn->rqst.sizeAvail = (size_t)iRet;
    }
  else
    pConn->rqst.sizeAvail = (size_t)iRet;

#if OPT_SAVE_BODY
  if ( NULL == pConn->fileBody)
    {
    char szTemp[ L_UNIQUE];
    if ( NULL != UniqueName( szTemp) )
      {
      pConn->fileBody = fopen( szTemp, "wb");
      Log( kHttpWarn, &pConn->client, "WARN: content saved in %s\n", szTemp);
      }
    }

  if ( NULL != pConn->fileBody)
    {
    fwrite( pConn->rqst.buffer, pConn->rqst.sizeAvail, 1, pConn->fileBody);
    if ( 0 == pConn->lContent)
      {
      fclose( pConn->fileBody);
      pConn->fileBody = NULL;
      }
    }
#endif

  /* Execute the request */
  if ( pConn->pfnAction)
    (*pConn->pfnAction)( pConn, &pConn->rqst);

  if ( 0 == pConn->lContent)
    {
    if ( pConn->pfnAction)
      {
      /* Indicate end of input */
      pConn->eState = kStateIdle;
      pConn->rqst.sizeAvail = 0;
      (*pConn->pfnAction)( pConn, &pConn->rqst);
      }
    else
      EndRequest( pConn);
    }
  }


/**
 * Async connect
 */
extern int HttpConnect(
  const char* pszHost,                /* IN: Host */
  FHttpMethod* pfnAction,             /* IN: Callback */
  SHttpRequest* pRqst                 /* IN: Callback context */
) {
  SHttpConnection* pConn;
  SWS_PTR( struct hostent) pHost;
  char szHost[ 256];
  const char* psz;
  unsigned uPort = HTTPD_PORT;
  int iRet;
  socklen_t len;

  psz = strchr( pszHost, '@');
  if ( NULL != psz)
    pszHost = psz + 1;

  psz = strrchr( pszHost, ':');
  if ( NULL != psz)
    {
    strncpy( szHost, pszHost, (size_t)(psz - pszHost));
    szHost[ (size_t)(psz - pszHost)] = cNul;
    pszHost = szHost;
    if ( 1 != sscanf( psz, ":%u", &uPort))
      return HTTPD_BAD_REQUEST;
    }

  /* Lookup host */
  if ( NULL == (pHost = gethostbyname( pszHost)))
    return HTTPD_BAD_GATEWAY;
  if ( AF_INET != pHost->h_addrtype)
    return HTTPD_NOT_IMPLEMENTED;

  if ( DEQUE_IS_EMPTY( &s_free))
    {
    Log( kHttpWarn, NULL, "WARN: HttpConnect out of connections\n");
    return HTTPD_NO_SERVICE;
    }

  pConn = LIST2CONN( DequeRemove( DEQUE_FIRST( &s_free) ));
  NewConnection( pConn);

  pConn->eState = kStateConnect;
  pConn->pfnAction = pfnAction;
  pConn->pRqst = pRqst;

  pConn->client.sin_family = AF_INET;
  pConn->client.sin_port = htons( (u_short)uPort);
  memcpy( &pConn->client.sin_addr, pHost->h_addr, pHost->h_length);

  pConn->s = socket( PF_INET, SOCK_STREAM, 0);
  if ( INVALID_SOCKET == pConn->s)
    {
    iRet = GetLastError();
    Log( kHttpError, NULL, "ERROR %d: socket: %s\n", iRet, strerror( iRet));
    DequeAppend( &s_free, &pConn->list);
    return HTTPD_NO_SERVICE;
    }

  /* Enable non-blocking mode */
  setsockasync( pConn->s, 1);

  /* Set a send/connect timeout */
  { DECL_TIMEOUT( tv, SEND_TIMEOUT);
  setsockopt(
    pConn->s,
    SOL_SOCKET, SO_SNDTIMEO,
    (char*)&tv, sizeof tv
  ); }

  /* Connect to server */
  pConn->rqst.tRequest = time( NULL);
  iRet = connect( pConn->s, (const struct sockaddr*)&pConn->client,
    sizeof pConn->client);

  len = sizeof pConn->client; 
  getsockname( pConn->s, (struct sockaddr*)&pConn->client, &len);
  Log( kHttpMost, &pConn->client, "CONNECT-TO %s\n", pszHost);

  if ( 0 == iRet)
    {
    /* Connected */
    pConn->tIdle = pConn->rqst.tRequest + s_uIdleTimeout;
    DequeAppend( &s_active, &pConn->list);

    HttpConnected( pConn);
    }
  else if ( EWOULDBLOCK == (iRet = GetLastError())
    || EINPROGRESS == iRet
  ) {
    /* In progress */
    pConn->tIdle = INVALID_TIME;
    DequeAppend( &s_active, &pConn->list);
    }
  else
    {
    Log( kHttpWarn, &pConn->client, "WARN %d: connect(%s): %s\n",
      iRet, pszHost, strerror( iRet) );
    closesocket( pConn->s);
    DequeAppend( &s_free, &pConn->list);
    return HTTPD_BAD_GATEWAY;
    }

  return HTTPD_OK;
  }


/*
 * Connection complete
 */
static void HttpConnected(
  SHttpConnection* pConn
) {
  SHttpRequest* const pRqst = pConn->pRqst;
  FHttpMethod* const pfnAction = pConn->pfnAction;
  int iRet;
  socklen_t n;

  pConn->pfnAction = 0;

  /* Get connect status */
  n = sizeof iRet;
  if ( getsockopt( pConn->s, SOL_SOCKET, SO_ERROR, (char*)&iRet, &n) || iRet)
    {
    Log( kHttpWarn, &pConn->client, "WARN %d: CONNECT-TO: %s\n", iRet, strerror( iRet));
    closesocket( pConn->s);
    DequeRemove( &pConn->list);
    DequeAppend( &s_free, &pConn->list);
    pConn = NULL;
    }
  else
    {
    Log( kHttpMost, &pConn->client, "CONNECT-TO OK\n");
    pConn->eState = kStateExecute;
    }

  /* Completion callback */
  if ( pfnAction)
    (*pfnAction)( pConn, pRqst);
  else if ( pConn)
    {
    closesocket( pConn->s);
    DequeRemove( &pConn->list);
    DequeAppend( &s_free, &pConn->list);
    pConn = NULL;
    }

  if ( pConn)
    {
    /* Copy handler's private area */
    memcpy( pConn->rqst.reserve, pRqst->reserve, sizeof pConn->rqst.reserve);
    }
  }


/**
 * Send an HTTP request
 */
extern int HttpRequest(
  SHttpConnection* pConn,             /* IN: Connection */
  const char* pszRqst                 /* IN: Request headers */
) {
  int n;
  char* psz;

  if ( kStateQuit ==  pConn->eState)
    return HTTPD_BAD_GATEWAY;

  pConn->rqst.pszMethod = NULL;
  pConn->pszURI = pConn->rqst.pszPath = "";
  pConn->pszQuery = pConn->rqst.pszQuery = NULL;
  pConn->pszParam = pConn->rqst.pszParam = NULL;
  pConn->rqst.pszTail = "";
  pConn->rqst.sizeRequest = 0;
  pConn->rqst.pszHeaders = NULL;
  pConn->rqst.sizeHeaders = 0;
  pConn->rqst.iConnClose = -1; /* Close connection when complete */
  pConn->rqst.uCode = 0; /* !0= proxy request */

  psz = (char*)strncpy( pConn->header, pszRqst, sizeof pConn->header);
  pConn->header[ sizeof pConn->header - 1] = cNul;

  /* Strip leading whitespace from method */
  while ( isspace( *psz))
    ++psz;
  pConn->rqst.pszMethod = psz;

  /* Split method from Request-URI */
  while ( cNul != *psz && !isspace( *psz) )
    ++psz;
  if ( cNul != *psz)
    {
    *(psz++) = cNul; /* Nul terminate the method */

    /* Strip leading whitespace from Request-URI */
    while ( isspace( *psz) )
      ++psz;
    pConn->pszURI = psz;

    /* Split the URI from the HTTP version */
    while ( cNul != *psz && !isspace( *psz) )
      ++psz;
    if ( cNul != *psz)
      {
      *(psz++) = cNul; /* Nul terminate the URI */

      /* Strip leading whitespace from Request-tail */
      while ( isspace( *psz) )
        ++psz;
      pConn->rqst.pszTail = psz;

      psz = strstr( psz, CRLF);
      if ( NULL != psz)
        {
        *psz = cNul;
        psz += sizeof CRLF -1;
        pConn->rqst.pszHeaders = psz;
        pConn->rqst.sizeHeaders = strlen( psz);
        }
      }
    }

  pConn->rqst.sizeRequest = (size_t)(psz - pConn->header);

  setsockasync( pConn->s, 0);
  n = WriteRaw( pConn, pszRqst, strlen( pszRqst));
  setsockasync( pConn->s, 1);
  if ( SOCKET_ERROR == n)
    {
    pConn->eState = kStateQuit;
    return HTTPD_BAD_GATEWAY;
    }
  return HTTPD_OK;
  }


/**
 * Async receive http response. Callback when complete
 */
extern int HttpResponse(
  SHttpConnection* pConn,             /* IN: Connection */
  FHttpMethod* pfnAction              /* IN: Callback */
) {
  if ( pConn)
    {
    pConn->eTelnetState = kTelnetData;
    pConn->telnetCR = 0;
    pConn->telnetCount = 0;

    pConn->eState = kStateResponse;
    pConn->pfnAction = pfnAction;

    /* Init the request context */
    pConn->rqst.uMajor = 0;
    pConn->rqst.uMinor = 9;
    pConn->rqst.lContent = -1; /* Unspecified */
    }
  return HTTPD_OK;
  }


/*
 * Get an HTTP response
 */
static void HttpGetResponse(
  SHttpConnection* pConn
) {
  int iRet;

  /* Input a line */
  iRet = TelnetReadLine( pConn);
  if ( 0 == iRet)
    return; /* Not complete */
  else if ( SOCKET_ERROR == iRet)
    {
    /* Disconnected */
    pConn->eState = kStateQuit;
    return;
    }

  Log( kHttpMost, &pConn->client, "<= %s\n", pConn->pTelnetInput);
  if ( !pConn->telnetEOL)
    {
    /* Invalid input */
    pConn->eState = kStateQuit;
    return;
    }

  /* Validate HTTP response header */
  if ( pConn->rqst.sizeRequest + 64 < sizeof pConn->header)
    {
    char* psz = pConn->header + pConn->rqst.sizeRequest;
    *psz = cNul;
    pConn->rqst.pszResponse = psz;
    if ( 3 <= sscanf( pConn->pTelnetInput, " HTTP/%u.%u %u %63[^"CRLF"]",
        &pConn->rqst.uMajor, &pConn->rqst.uMinor, &pConn->rqst.uCode, psz)
    ) {
      psz += strlen( psz) + 1;
      *psz = cNul;
      pConn->rqst.pszHeaders = psz;
      pConn->rqst.sizeHeaders = 0; /* No headers */

      /* General and response headers next */
      pConn->eState = kStateRespHdr;
      }
    else
      {
      pConn->eState = kStateQuit;
      }
    }
  else if ( 2 == sscanf( pConn->pTelnetInput, " HTTP/%u.%u %u",
      &pConn->rqst.uMajor, &pConn->rqst.uMinor, &pConn->rqst.uCode)
  ) {
    Log( kHttpWarn, &pConn->client, "WARN: HttpGetResponse no space\n");
    pConn->rqst.pszResponse = "";
    pConn->rqst.pszHeaders = pConn->header + sizeof pConn->header;
    pConn->rqst.sizeHeaders = 0; /* No headers */

    /* General and response headers next */
    pConn->eState = kStateRespHdr;
    }
  else
    {
    pConn->eState = kStateQuit;
    }
  }
#endif /* !OPT_HTTP_TINY */


/****************************************************************************
 * Builtin actions
 ****************************************************************************/
/*
 * Handle a request for an unknown URI
 */
int UriNotFound(
  SHttpConnection* pConn,
  SHttpRequest* pRqst
) {
  (void)pRqst;
  return HttpReply( pConn, HTTPD_BAD_REQUEST, "Unknown URI", 0, NULL);
  }


/*
 * Handle a request for an unsupported method
 */
int MethodNotImplemented(
  SHttpConnection* pConn,
  SHttpRequest* pRqst
) {
  (void)pRqst;
  return HttpReply( pConn, HTTPD_NOT_IMPLEMENTED,
    "Method not implemented for URI", 0, NULL);
  }


/*
 * Handle server OPTIONS *
 */
int OptionsMethod(
  SHttpConnection* pConn,
  SHttpRequest* pRqst
) {
  (void)pRqst;

  pRqst->iConnClose = 1;
  return HttpReply( pConn, HTTPD_OK, "OK", 0, kszHttpAllow
    ": " kszHttpGet
    ", " kszHttpHead
    ", " kszHttpOpts
    CRLF
  );
  }


#if OPT_HTTP_REDIR
/****************************************************************************
 * Server redirects
 ****************************************************************************/
/*
 * Handle a request for a redirected URI
 */
static int UriRedirect(
  SHttpConnection* pConn,
  SHttpRequest* pRqst
) {
  sprintf( pRqst->buffer, kszHttpLocation ": %s" CRLF, pRqst->pszPath);
  return HttpReply( pConn, HTTPD_MOVED_PERM, "Moved", 0, pRqst->buffer);
  }


/*
 * qsort/bsearch callback to compare redirects
 */
static int CmpRedirect( const void* pvKey, const void* pvMmbr)
  {
  return strcmp(
    ((const struct SRedirect*)pvKey)->szOld, ((const struct SRedirect*)pvMmbr)->szOld);
  }
#endif


#if OPT_HTTP_ALIAS
/****************************************************************************
 * URI aliases
 ****************************************************************************/
/*
 * Lookup URI alias
 */
const struct SRedirect* FindAlias(const char* pszPath)
  {
  const struct SRedirect* pRedir = s_aliases.aList;
  unsigned u;

  for ( pRedir = &s_aliases.aList[ u = 0]; u < s_aliases.uCount; ++u,++pRedir)
    {
    if ( SWS_StrMatch( pRedir->szOld, pszPath, 0))
      return pRedir;
    }

  return NULL;
  }


/*
 * Alias a URI with wildcard replacement
 */
const char* AliasURI( 
  SHttpConnection* pConn,
  const struct SRedirect* pRedir,
  const char* pszPath
) {
  const char* pszPattern = pRedir->szOld;
  const char* pszAlias = pRedir->szNew;
  char* const pszEnd = pConn->alias + sizeof pConn->alias;
  char* const pszBegin = pConn->alias;
  char* pszOut = pszBegin;

  for (;;)
    {
    char cSub;

    /* while *pattern && *pattern == *path : pattern++, path++ */
    while ( cNul != (cSub = *pszPattern))
      {
      if ( cSub == '*' || cSub == '?')
        break;
      assert( cSub == *pszPath);
      ++pszPath;
      ++pszPattern;
      }

    /* while *alias && '*' != *alias : *out++ = *alias++ */
    while ( '*' != (cSub = *pszAlias++) && '?' != cSub)
      {
      *pszOut++ = cSub;
      if ( cNul == cSub)
        return pszBegin;
      if ( pszOut + 1 >= pszEnd)
        return *pszOut = cNul, pszBegin;
      }

    /* if end of pattern (or path) done */
    if ( cNul == *pszPattern)
      {
      assert( cNul == *pszPath);
      *pszOut = cNul;
      return pszBegin;
      }

    /* pattern++ (all wildcards) */
    while ( '*' == *pszPattern || '?' == *pszPattern)
      ++pszPattern;

    /* while !regcmp(pattern, path) *path++ = *out++ */
    /* Replace matched section in path */
    while ( !SWS_StrMatch( pszPattern, pszPath, 0))
      {
      /* Alias '*' == copy src */
      if ( '*' == cSub)
        {
        assert( *pszPath);
        *pszOut++ = *pszPath++;
        }
      /* Alias '?' == replace from alias until next ? */
      else if ( '?' == cSub)
        {
        char cAlias = *pszAlias++;
        if ( '?' == cAlias)
          {
          cSub = cNul;
          continue;
          }
        else if ( cNul == (*pszOut++ = cAlias))
          return pszBegin;
        }
      else
        {
        ++pszPath;
        continue;
        }

      if ( pszOut + 1 >= pszEnd)
        return *pszOut = cNul, pszBegin;
      }
    }
  }
#endif /* OPT_HTTP_ALIAS*/


#if OPT_HTTP_REDIR || OPT_HTTP_ALIAS
/*
 * Redirected/aliased URIs enumeration callback
 */
SWS_CB( int, EnumRedirected( void* pvCtx, const char* pszVal))
  {
  struct SRedirList* pList = (struct SRedirList*)pvCtx;
  struct SRedirect* pRedir;
  unsigned uCount;

  uCount = pList->uCount;
  if ( uCount * sizeof pList->aList[0] >= pList->size)
    {
    /* Extend the array */
#ifndef INITIAL_REDIR
#define INITIAL_REDIR 16
#endif
    size_t size = pList->size ? pList->size * 2
      : INITIAL_REDIR * sizeof pList->aList[0];
    void* pv = realloc( pList->aList, size);
    if ( NULL == pv)
      {
      Log( kHttpError, NULL, "ERROR %d: EnumRedirected realloc\n", errno);
      return -1;
      }

    pList->aList = (struct SRedirect*)pv;
    pList->size = size;
    }
  pRedir = &pList->aList[ uCount++];

  /* Parse the entry */
  if ( 2 == sscanf( pszVal,
    " %"STRING(SIZE_REDIR)"[^; =] = %"STRING(SIZE_REDIR)"[^ ;]",
     pRedir->szOld, pRedir->szNew)
  ) {
    Log( kHttpMost, NULL, "%s %s = %s\n",
#if OPT_HTTP_ALIAS
      &s_aliases == pList ? kszHttpAlias :
#endif
#if OPT_HTTP_REDIR
      &s_redirects == pList ? kszHttpRedir :
#endif
      "other",
      pRedir->szOld, pRedir->szNew);
    pList->uCount = uCount;
    }

  return 0;
  }
#endif


#if OPT_HTTP_REALMS
/****************************************************************************
 * Realms
 ****************************************************************************/

/*
 * Get Authentication action
 */
FHttpMethod* GetRealmAction(
  SHttpConnection* pConn,
  const char* pszRealm,
  FHttpMethod* pfnAction
) {
  struct SRealm* pRealm;
  const struct SHost* pHost;
  ui32 addr;
  unsigned u;

  pConn->pRealm = NULL;
  pConn->pfnAuthAction = 0;

  /* Is there an authorization realm? */
  if ( NULL == pszRealm)
    return pfnAction;

  /* see if cached */
  pRealm = FindRealm( pszRealm, 0);
  if ( NULL == pRealm)
    {
    Log( kHttpError, &pConn->client, "ERROR: missing realm %s\n", pszRealm);
    return &NoService;
    }

  /* Keep cache LRU */
  DequeRemove( &pRealm->list);
  DequePrepend( &s_realms, &pRealm->list);

  pConn->pRealm = pRealm;

  /* Search permitted hosts */
  addr = ntohl( pConn->client.sin_addr.s_addr);
  for ( u = pRealm->uPermitHosts, pHost = pRealm->aPermitHost; u > 0; --u, ++pHost)
    {
    if ( INADDR_ANY == pHost->ulNet || pHost->ulNet == (addr & pHost->ulMask))
      {
      Log( kHttpMore, &pConn->client, "Permitted host for realm %s\n", pszRealm);
      return pfnAction;
      }
    }

  /* Search denied hosts */
  for ( u = pRealm->uDenyHosts, pHost = pRealm->aDenyHost; u > 0; --u, ++pHost)
    {
    if ( INADDR_ANY == pHost->ulNet || pHost->ulNet == (addr & pHost->ulMask))
      {
      Log( kHttpMore, &pConn->client, "Forbidden host for realm %s\n", pszRealm);
      return &Forbidden;
      }
    }

  /* Search permitted times */
  if ( pRealm->uTimes)
    {
    const struct STime* pTime;
    struct tm const tLocal = *localtime( &pConn->rqst.tRequest);
    unsigned uMins = tLocal.tm_min + tLocal.tm_hour * 60;

    for ( u = pRealm->uTimes, pTime = pRealm->aTime; u > 0; --u, ++pTime)
      {
      /* Within access time limits? */
      if ( pTime->uOffTime != pTime->uOnTime)
        {
        if ( (pTime->uOffTime > pTime->uOnTime && (uMins >= pTime->uOnTime && uMins < pTime->uOffTime))
          || (pTime->uOffTime < pTime->uOnTime && (uMins >= pTime->uOnTime || uMins < pTime->uOffTime))
        ) {
          /* Permitted */
          Log( kHttpMore, &pConn->client, "Permitted time for realm %s\n", pszRealm);
          return pfnAction;
          }
        }
      else
        {
        /* Unrestricted access time */
        Log( kHttpMore, &pConn->client, "Unrestricted time for realm %s\n", pszRealm);
        return pfnAction;
        }
      }
    }

  if ( 0 == pRealm->uAccts)
    {
    Log( kHttpMore, &pConn->client, "No accounts for realm %s\n", pszRealm);
    return &Forbidden;
    }

  pConn->pfnAuthAction = pfnAction;
  if ( 0 == strcmp( kszHttpProxy, pRealm->szTag)
    || 0 == strcmp( kszHttpProxyConn, pRealm->szTag)
  ) {
    Log( kHttpMore, &pConn->client, "Proxy authorisation required for realm %s\n", pszRealm);
    return &ProxyAuthRequired;
    }

  Log( kHttpMore, &pConn->client, "Authorisation required for realm %s\n", pszRealm);
  return &AuthRequired;
  }


/*
 * Lookup realm tag in cache
 */
SRealm* FindRealm( const char* pszTag, int isNew)
  {
  SDeque* pDeque;
  SRealm* pRealm = NULL;

  pDeque = DEQUE_FIRST( &s_realms);
  if ( NULL == pDeque)
    {
    /* Init the linked list */
    DEQUE_INIT( &s_realms);
    pDeque = DEQUE_FIRST( &s_realms);
    }

  for ( ; pDeque != &s_realms; pDeque = DEQUE_FIRST( pDeque) )
    {
    pRealm = LIST2REALM( pDeque);
    if ( 0 == strncmp( pRealm->szTag, pszTag, sizeof pRealm->szTag))
      return pRealm;
    }

  /* No such realm, add it? */
  if ( !isNew)
    return NULL;

  /* Create new realm */
  pRealm = (struct SRealm*)calloc( 1, sizeof *pRealm);
  if ( NULL != pRealm)
    {
    DEQUE_INIT( &pRealm->list);
    strncpy( pRealm->szTag, pszTag, sizeof( pRealm->szTag) -1);
    strncpy( pRealm->szName, "Restricted Access", sizeof( pRealm->szName) -1);
    SWS_CfgEnum( pRealm->szTag, &EnumRealm, pRealm);
    DequeAppend( &s_realms, &pRealm->list);
    }
  else
    Log( kHttpError, NULL, "ERROR %d: FindRealm calloc\n", errno);

  return pRealm;
  }


/*
 * Verify client credentials
 */
int Authenticate(                     /* 1= yes, 0= no, -1= reject */
  const struct SRealm* pRealm,
  const char* pszHeader,
  const char* pszValue
) {
  struct SAcct* pAcct;
  unsigned u;

  if ( NULL == pRealm)
    return 0;

  /* If it's a proxy realm tag then ensure it's a proxy authorization */
  if ( 0 == strcmp( kszHttpProxy, pRealm->szTag)
    || 0 == strcmp( kszHttpProxyConn, pRealm->szTag)
  ) {
    if ( strcmp( kszHttpProxyAuthorization, pszHeader))
      return 0;
    }
  else if ( 0 == strcmp( kszHttpProxyAuthorization, pszHeader))
    return 0;

  if ( SWS_strnicmp( pszValue, "Basic", 5))
    return 0;

  pszValue += 5;
  if ( !isspace( *pszValue))
    return 0;

  do
    ++pszValue;
  while( isspace( *pszValue));

  for ( u = pRealm->uAccts, pAcct = pRealm->aAcct; u > 0; --u, ++pAcct)
    {
    if ( 0 == strcmp( pszValue, pAcct->szAcct))
      {
      Log( kHttpMore, NULL, "Authenticated realm %s\n", pRealm->szTag);
      return 1;
      }
    }

  Log( kHttpMore, NULL, "Authentication failed for realm %s\n", pRealm->szTag);
  return 0; /* 0= not authenticated, -1= reject */
  }


/*
 * Handle a forbidden request
 */
int Forbidden(
  SHttpConnection* pConn,
  SHttpRequest* pRqst
) {
  (void)pRqst;
  return HttpReply( pConn, HTTPD_FORBIDDEN, "Forbidden", 0, NULL);
  }


/*
 * Handle a no service request
 */
int NoService(
  SHttpConnection* pConn,
  SHttpRequest* pRqst
) {
  (void)pRqst;
  return HttpReply( pConn, HTTPD_NO_SERVICE, "No service", 0, NULL);
  }


/*
 * Handle an unauthenicated request
 */
int AuthRequired(
  SHttpConnection* pConn,
  SHttpRequest* pRqst
) {
  sprintf( pRqst->buffer,
    kszHttpWwwAuthenticate ": Basic realm=\"%s\"" CRLF,
    pConn->pRealm->szName
  );
  return HttpReply( pConn, HTTPD_UNAUTHORIZED,
    "Authentication required", 0, pRqst->buffer);
  }


int ProxyAuthRequired(
  SHttpConnection* pConn,
  SHttpRequest* pRqst
) {
  sprintf( pRqst->buffer,
    kszHttpProxyAuthenticate ": Basic realm=\"%s\"" CRLF,
    pConn->pRealm->szName
  );
  return HttpReply( pConn, HTTPD_PROXY_AUTH_REQD,
    "Proxy authentication required", 0, pRqst->buffer);
  }


/*
 * SWS_CfgEnum callback to parse realm data
 */
SWS_CB( int, EnumRealm( void* pvCtx, const char* pszVal))
  {
  struct SRealm* pRealm = (struct SRealm*)pvCtx;
  char szNet[ 16], szAcct[ SIZE_REALM_ACCT + 1];
  ui32 ulNet;
  unsigned uCidr;
  unsigned uHr1, uMn1, uHr2, uMn2;
  void* pv;
  struct SHost* pHost;
  enum EHttpLog const eLog = kHttpMore;

  /* Parse entry */
  uCidr = 32; /* Classless Internet Domain Routing */
  if ( 1 <= sscanf( pszVal, " permit = %15[0123456789.] / %u", szNet, &uCidr))
    {
    Log( eLog, NULL, "%s: permit = %s/%u\n", pRealm->szTag, szNet, uCidr);

    if ( INADDR_NONE != (ulNet = ntohl( inet_addr( szNet)))
      && uCidr <= 32
    ) {
      pv = realloc( pRealm->aPermitHost,
        (pRealm->uPermitHosts + 1) * sizeof pRealm->aPermitHost[0]);
      if ( NULL != pv)
        {
        pRealm->aPermitHost = (struct SHost*)pv;
        pHost = &pRealm->aPermitHost[ pRealm->uPermitHosts++];
        pHost->ulMask = 0xffffffffL << (32 - uCidr);
        pHost->ulNet = ulNet & pHost->ulMask;
        }
      else
        Log( kHttpError, NULL, "ERROR %d: EnumRealm realloc\n", errno);
      }
    }
  else if ( 1 <= sscanf( pszVal, " deny = %15[0123456789.] / %u", szNet, &uCidr))
    {
    Log( eLog, NULL, "%s: deny = %s/%u\n", pRealm->szTag, szNet, uCidr);

    if ( INADDR_NONE != (ulNet = ntohl( inet_addr( szNet)))
      && uCidr <= 32
    ) {
      pv = realloc( pRealm->aDenyHost,
        (pRealm->uDenyHosts + 1) * sizeof pRealm->aDenyHost[0]);
      if ( NULL != pv)
        {
        pRealm->aDenyHost = (struct SHost*)pv;
        pHost = &pRealm->aDenyHost[ pRealm->uDenyHosts++];
        pHost->ulMask = 0xffffffffL << (32 - uCidr);
        pHost->ulNet = ulNet & pHost->ulMask;
        }
      else
        Log( kHttpError, NULL, "ERROR %d: EnumRealm realloc\n", errno);
      }
    }
  else if ( 1 == sscanf( pszVal, " acct = %"STRING(SIZE_REALM_ACCT)"s", szAcct))
    {
    Log( eLog, NULL, "%s: acct = ***\n", pRealm->szTag);
    pv = realloc( pRealm->aAcct,
      (pRealm->uAccts + 1) * sizeof pRealm->aAcct[0]);
    if ( NULL != pv)
      {
      struct SAcct* pAcct;

      pRealm->aAcct = (struct SAcct*)pv;
      pAcct = &pRealm->aAcct[ pRealm->uAccts++];
      SWS_Base64Encode( pAcct->szAcct, sizeof pAcct->szAcct,
        (unsigned char*)szAcct, strlen( szAcct) );
      }
    else
      Log( kHttpError, NULL, "ERROR %d: EnumRealm realloc\n", errno);
    }
  else if ( 4 == sscanf( pszVal, " time = %u:%u %u:%u", &uHr1, &uMn1, &uHr2, &uMn2))
    {
    Log( eLog, NULL, "%s: time = %u:%u %u:%u\n", pRealm->szTag,
      uHr1, uMn1, uHr2, uMn2);
    pv = realloc( pRealm->aTime,
      (pRealm->uTimes + 1) * sizeof pRealm->aTime[0]);
    if ( NULL != pv)
      {
      struct STime* pTime;

      pRealm->aTime = (struct STime*)pv;
      pTime = &pRealm->aTime[ pRealm->uTimes++];
      pTime->uOnTime = uHr1 * 60 + uMn1;
      pTime->uOffTime = uHr2 * 60 + uMn2;
      }
    else
      Log( kHttpError, NULL, "ERROR %d: EnumRealm realloc\n", errno);
    }
  else if ( 1 == sscanf( pszVal, " path = %"STRING(SIZE_REALM_ROOT)"s", pRealm->szRoot))
    Log( eLog, NULL, "%s: path = %s\n", pRealm->szTag, pRealm->szRoot);
  else if ( 1 == sscanf( pszVal, " text = %"STRING( SIZE_REALM_NAME)"[^;]", pRealm->szName))
    Log( eLog, NULL, "%s: text = %s\n", pRealm->szTag, pRealm->szName);

  return 0;
  }
#endif /*OPT_HTTP_REALMS*/


/****************************************************************************
 * Telnet operations
 ****************************************************************************/
/*
 * Read a line
 */
int TelnetReadLine(
  SHttpConnection* pConn
) {
  int iRet;

  pConn->telnetEOL = 0;
  while ( pConn->telnetCount + 1 < pConn->telnetSize)
    {
    unsigned char c;

    iRet = TelnetRead( pConn, &c, sizeof c);
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
        pConn->telnetEOL = 1;
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
      pConn->pTelnetInput[ pConn->telnetCount++] = (char)c;
      break;

    case cReturn:
      pConn->telnetCR = 1;
      break;

    case cLinefeed:
      pConn->telnetEOL = 1;
      goto done;
      }
    }

done:
  /* Terminate string with a nul */
  pConn->pTelnetInput[ iRet = pConn->telnetCount] = cNul;
  pConn->telnetCount = 0;

  return (int)(iRet + 1); /* A CRLF on its own returns 1 */
  }


/*
 * Get a buffer from the client
 */
int TelnetRead(
  SHttpConnection* pConn,
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
        (void)WriteRaw( pConn, buf, 3);
        break;

      case kTelnetWont:
        pConn->eTelnetState = kTelnetData;
        break;

      case kTelnetDo:
        pConn->eTelnetState = kTelnetData;
        buf[0] = kTelnetIAC;
        buf[1] = kTelnetWont;
        buf[2] = c;
        (void)WriteRaw( pConn, buf, 3);
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
 * Write raw data
 */
int WriteRaw(
  SHttpConnection* pConn,
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
      Log( kHttpWarn, &pConn->client, "WARN %d: connection reset in send\n", err);
      SessionAbort( pConn);
      break;
    case ECONNABORTED:
      Log( kHttpWarn, &pConn->client, "WARN %d: connection aborted in send\n", err);
      SessionAbort( pConn);
      break;
#ifdef EPIPE
    case EPIPE: /* Numerous bots evoke this */
      Log( kHttpWarn, &pConn->client, "WARN %d: broken pipe in send\n", err);
      break;
#endif
    default:
      Log( kHttpWarn, &pConn->client, "WARN %d: send: %s\n", err, strerror( err));
      break;
      }
    }

  return iRet;
  }


/*
 * Read raw data
 */
int ReadRaw(
  SHttpConnection* pConn,
  void* pv,
  size_t size
) {
  int iRet;

  iRet = recv( pConn->s, (char*)pv, size, 0);
  if ( SOCKET_ERROR == iRet)
    {
    int err = GetLastError();
    if ( EWOULDBLOCK == err || EAGAIN == err)
      {
      return 0;
      }
    else switch ( err)
      {
    case ECONNRESET:
      /* NB many clients reset the connection rather than a graceful close */
      Log( kHttpMore, &pConn->client, "WARN %d: connection reset in recv\n", err);
      SessionAbort( pConn);
      break;
    case ECONNABORTED:
      Log( kHttpWarn, &pConn->client, "WARN %d: connection aborted in recv\n", err);
      SessionAbort( pConn);
      break;
    case ENOTCONN:
      Log( kHttpWarn, &pConn->client, "WARN %d: disconnected in recv\n", err);
      break;
    default:
      Log( kHttpWarn, &pConn->client, "WARN %d: recv: %s\n", err, strerror( err));
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


/****************************************************************************
 * Support functions
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


#if OPT_HTTP_REALMS
/*
 * Add an item to the head of a deque
 */
void DequePrepend( SDeque* const pDeque, SDeque* const pItem)
  {
  pItem->head = pDeque;
  pItem->tail = pDeque->tail;

  pDeque->tail->head = pItem;
  pDeque->tail = pItem;
  }
#endif


#if OPT_SAVE_BODY
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

    sprintf( szBuf, "ZH%06lu", s_u++);
    f = fopen( szBuf, "r");
    if ( NULL == f)
      return szBuf;

    fclose( f);
    }
  return NULL;
  }
#endif


/*
 * Log a message
 */
static void Log(
  int iLevel,
  const struct sockaddr_in* pSock,
  const char szFormat[], ...
) {
  va_list ap;
  va_start( ap, szFormat);
  vLog( iLevel, pSock, szFormat, ap);
  va_end( ap);
  }

static void vLog(
  int iLevel,
  const struct sockaddr_in* pSock,
  const char szFormat[],
  va_list ap
) {
  FILE* file;
  char szTime[ 32];
  time_t t;

  if ( kHttpError == iLevel)
    file = stderr;
  else if ( kHttpWarn == iLevel)
    file = s_logfile ? s_logfile : stderr;
  else if ( s_iVerbosity >= iLevel && s_logfile)
    file = s_logfile;
  else
    return;

  if ( '&' != szFormat[0])
    {
    t = time( NULL);
    strftime( szTime, sizeof szTime, "%Y/%m/%d %H:%M:%S", localtime( &t));
    fprintf( file, "%s " kszPrefix, szTime);
    if ( NULL != pSock)
      fprintf( file, "%s:%u ", inet_ntoa( pSock->sin_addr), ntohs( pSock->sin_port));
    vfprintf( file, szFormat, ap);
    }
  else
    vfprintf( file, szFormat + 1, ap);
  }


/**
 * Log a message
 */
extern void HttpLog(
  enum EHttpLog eLevel,
  SHttpConnection* pConn,             /* IN: Connection or NULL */
  const char szFormat[], ...          /* IN: Format string */
) {
  va_list ap;
  va_start( ap, szFormat);
  vLog( eLevel, pConn ? &pConn->client : NULL, szFormat, ap);
  va_end( ap);
  }


#if !OPT_HTTP_TINY
/**
 * Get the client/server address
 * Used by CGI
 */
extern const char* HttpClientAddr(
  SHttpConnection* pConn              /* IN: Connection */
) {
  return inet_ntoa( pConn->client.sin_addr);
  }

extern unsigned HttpClientPort(
  SHttpConnection* pConn              /* IN: Connection */
) {
  return ntohs( pConn->client.sin_port);
  }

extern const char* HttpServerAddr(
  SHttpConnection* pConn              /* IN: Connection */
) {
  struct sockaddr_in addr;
  socklen_t len = sizeof addr;

  return 0 == getsockname( pConn->s, (struct sockaddr*)&addr, &len)
    ? inet_ntoa( addr.sin_addr) : "";
  }


/**
 * Expand a URL encoded string. RFC 2396 section 2
 */
extern void HttpDecodeString( char* pszPath, const char* pszURI)
  {
  enum { kAny, kHex, kUnicode } eState = kAny;
  int digits = 0;
  unsigned esc = 0;
  char c;

  while ( cNul != (c = *pszURI++))
    {
    switch ( eState)
      {
    case kAny:
      switch ( c)
        {
      default:
        *(pszPath++) = c;
        break;

      case '%': /* Start escape */
        eState = kHex;
        digits = 0;
        esc = 0;
        break;
        }
      break;

    case kHex:
    case kUnicode:
      esc *= 0x10;
      switch ( c)
        {
      case '0': ++digits; break;
      case '1': esc += 0x1; ++digits; break;
      case '2': esc += 0x2; ++digits; break;
      case '3': esc += 0x3; ++digits; break;
      case '4': esc += 0x4; ++digits; break;
      case '5': esc += 0x5; ++digits; break;
      case '6': esc += 0x6; ++digits; break;
      case '7': esc += 0x7; ++digits; break;
      case '8': esc += 0x8; ++digits; break;
      case '9': esc += 0x9; ++digits; break;
      case 'A':
      case 'a': esc += 0xa; ++digits; break;
      case 'B':
      case 'b': esc += 0xb; ++digits; break;
      case 'C':
      case 'c': esc += 0xc; ++digits; break;
      case 'D':
      case 'd': esc += 0xd; ++digits; break;
      case 'E':
      case 'e': esc += 0xe; ++digits; break;
      case 'F':
      case 'f': esc += 0xf; ++digits; break;

      case 'u': /* UTF16 */
        if ( 0 == digits)
          eState = kUnicode;
        else
          eState = kAny;
        break;

      default:
        if ( 0 == digits)
          *(pszPath++) = c;
        eState = kAny;
        break;
        }

      if ( kHex == eState && digits >= 2)
        {
        *(pszPath++) = (char)esc;
        eState = kAny;
        }
      else if ( kUnicode == eState && digits >= 4)
        {
        *(pszPath++) = (char)(esc & 0xff);
        *(pszPath++) = (char)(esc >> 8);
        eState = kAny;
        }
      break;
      }
    }
  *pszPath = cNul;
  }

#endif /* !OPT_HTTP_TINY */

/* End of file */
