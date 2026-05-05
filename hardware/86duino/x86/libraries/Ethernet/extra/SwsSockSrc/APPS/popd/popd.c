/*****************************************************************************
 *
 * File name   : popd.c
 * Function    : POP3 server. RFC 1939 (STD 53),
 *               POP3 Extension Mechanism (CAPA): RFC 2449
 *               POP3 SASL Authentication Mechanism: RFC 5034
 * Authors     : Lawrence Rust
 * Systems     : ANSI C, sockets
 *
 *****************************************************************************
 * File History
 * ------------
 *
 * 20-Jan-03, LVR, Created.
 * 24-Mar-03, LVR, Split into network and message server modules
 * 23-Feb-05, LVR, Read sign-off message from ini file
 * 07-Aug-09, LVR, v1.03 - Catch SIGPIPE
 * 23-Dec-09, LVR, v1.04 - Added Log()
 * 09-Mar-10, LVR, v1.05 - Added AUTH plain & login
 *
 *
 * Comments
 * --------
 *
 *****************************************************************************/
#define kszApp "SoftPop"
#define kszVer STRING( POPD_VER_MAJOR) "." STRING( POPD_VER_MINOR)
#define kszBanner kszApp " " kszVer " (" SWS_NET_ID ")"

/* Compilation options */
#ifndef OPT_MAX_CONNECTIONS
#define OPT_MAX_CONNECTIONS 4U
#endif
#if OPT_MAX_CONNECTIONS > 63
#define FD_SETSIZE (1 + OPT_MAX_CONNECTIONS)
#endif

#ifndef OPT_UPNP
# define OPT_UPNP 0        /* !0 to export service to UPnP */
#endif

#ifndef OPT_CRAM_MD5
#define OPT_CRAM_MD5 0    /* !0 for CRAM MD5 authorisation */
#endif


/* Exports */
#include "popd.h"

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
#include <limits.h>
#include <assert.h>
#if _MSC_VER >= 1000
#include <crtdbg.h>
#endif

#include "sws_net.h"
#include "sws_str.h"   /* SWS_stricmp */
#include "sws_cfg.h"
#if OPT_UPNP
#include "sws_upnp.h"
#endif
#include "sws_ba64.h"
#if OPT_CRAM_MD5
#include "sws_md5.h"
#endif

#include "popmsg.h"       /* Message database */


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

#define kszPrefix "POP3D: "

#define POLL_TIMEOUT    333U  /* Default select timeout in millisecs */
#define SEND_TIMEOUT  60000U  /* Send timeout in millisecs */
#define IDLE_TIMEOUT 120U     /* Default recv idle timeout in seconds */
#define MAX_ATTEMPTS 2        /* Max login attempts until disconnect */

#ifndef MSG_NOSIGNAL
#define MSG_NOSIGNAL 0
#endif

#define cNul       '\0'
#define cReturn    '\x0d'
#define cLinefeed  '\x0a'
#define CRLF       "\x0d\x0a"

#define POP_MAX_INPUT 512 /* Maximum input line length */
#define MAX_LINE     1000 /* Max mail body text line RFC2822 2.3 */
#define MAX_FQDN      256 /* Max hostname length inc nul */
#define MAX_ARG        64 /* Max arg length inc nul. RFC 1939 3 says 40 excl nul */
#define MAX_REPLY (MAX_ARG + 80)

#if OPT_CRAM_MD5
# define MAX_PASS 40
# define MD5_LEN 32       /* Length of MD5 digest as hex string */
#endif

/* ini file values */
#define kszPopSect        "pop"
#define kszLogVerbosity   "log"
#define kszMaxConnections "max_conn"
#define kszMaxAttempts    "max_attempts" /* Max login attempts until disconnect */
#define kszIdleTimeout    "idle_time"   /* Recv idle timeout in seconds */
#define kszPollTimeout    "poll_time"   /* select timeout in millisecs */
#define kszGreeting       "greeting"
#define kszBye            "bye"
#define kszPopUsersSect   "pop_users"


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

/* Mail message */
typedef struct SMessage
  {
  int isDeleted;
  unsigned long ulSize;
  } SMessage;

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

/* POP3 state */
typedef enum EState
  {
  kStateAuthorise = 0,
  kStateSasl,
  kStatePassword,
  kStateTransaction,
  kStateUpdate,
  kStateQuit
  } EState;

/* Connection state */
typedef struct SConnection
  {
  SDeque list;
#define LIST2CONN( p) CONTAINER( p, SConnection, list)

  SOCKET s;                     /* client connection */
  struct sockaddr_in client;    /* client's address */
  time_t tIdle;                 /* Time when idle connection will be closed */

  /* Telnet state */
  ETelnetState eTelnetState;
  int telnetCR;
  size_t telnetCount;

  /* POP state */
  EState eState;
  int iAttempts;                /* Password attempts */
  int isAuthenticated;          /* SASL state */
  char szUser[ MAX_ARG];
#if OPT_CRAM_MD5
  char szChallenge[ MAX_ARG + 64]; /* CRAM-MD5 challenge: <rnd.time@host> */
#endif

  /* RETR/TOP output state */
  SPopMsg* hMsg;                /* Message file handle */
  size_t sizeToWrite;           /* Output bytes remaining */
  const char* pcToWrite;        /* -> next output */
  unsigned uTop;                /* TOP lines */
  int inBody, iEol;             /* Flags */

  SPopUser* pUser;              /* User handle */
  unsigned uMessages;           /* No. messages */
  SMessage* pMsgs;              /* Array of messages */

  /* Command input buffer */
  const char* pszTail;          /* -> command tail */
  char input[ POP_MAX_INPUT];

  /* General purpose buffer */
  char buffer[ MAX_LINE + 1];
  } SConnection;


/* Command function */
typedef struct SMethod SMethod;
typedef int FMethod( SConnection*, const char*, char*);

/* Command descriptor */
struct SMethod
  {
  const char* pszName;
  FMethod* pfnExecute;
  };


/***************************************************************************
 * Function prototypes
 ***************************************************************************/

static void SessionStart( SConnection*);
static void SessionFinish( SConnection*);
static void SessionInput( SConnection*);
static void SessionOutput( SConnection*);
static void SessionAbort( SConnection*);

static int ExecuteRequest( SConnection*, char*);
static const SMethod* FindCommand( const char*);
static int Response( SConnection*, int, const char*);

static int TelnetRead( SConnection*, void*, size_t);
static int TelnetReadLine( SConnection*);

static int WriteRaw( SConnection*, const void*, size_t);
static int ReadRaw( SConnection*, void*, size_t);

static void Log( int, const struct sockaddr_in*, const char [], ...) FORMAT(printf,3,4);
static void Reset( SConnection*);
static int EnumMessages( SConnection*);

static int AuthPlain( SConnection*, const char*, char*);
static void AuthInput( SConnection*);

#if OPT_CRAM_MD5
static void CramMD5digest( char [ MD5_LEN], const char*, const char*);
static int FindUser( char [], const char*);
static SWS_FCfgCallback EnumPopCallback;
#endif

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

/* Operation */
extern sig_atomic_t volatile g_exit;
extern sig_atomic_t volatile g_running;
static SDeque s_free, s_active;

static char s_szGreeting[ MAX_REPLY];
static char s_szBye[ MAX_REPLY];
static int s_iMaxAttempts;

/* The method table */
static FMethod UnimplementedMethod;
static FMethod NoopMethod;
static FMethod QuitCommand;

static FMethod UserMethod;
static FMethod PassMethod;
static FMethod AuthMethod;
static FMethod StatMethod;
static FMethod ListMethod;
static FMethod RetrMethod;
static FMethod DeleMethod;
static FMethod ResetMethod;
static FMethod UidlMethod;
static FMethod CapaMethod;

static const SMethod methods[] = {
  /* REQ */ { "USER", &UserMethod },
  /* REQ */ { "PASS", &PassMethod },
  /* REQ */ { "STAT", &StatMethod },
  /* REQ */ { "LIST", &ListMethod },
  /* REQ */ { "RETR", &RetrMethod },
  /* REQ */ { "DELE", &DeleMethod },
  /* REQ */ { "RSET", &ResetMethod },
  /* REQ */ { "NOOP", &NoopMethod },
  /* OPT */ { "",     &NoopMethod },
  /* OPT */ { "TOP" , &RetrMethod },
  /* OPT */ { "UIDL", &UidlMethod },
  /* OPT */ { "CAPA", &CapaMethod }, /* RFC 2449 */
  /* OPT */ { "APOP", &UnimplementedMethod },
  /* OPT */ { "AUTH", &AuthMethod }, /* RFC 5034 (obsoletes 1734) */
  /* REQ */ { "QUIT", &QuitCommand }
};


/***************************************************************************
 * Functions
 ***************************************************************************/

/*
 * Set log file
 */
extern void PopdSetLog(
  int iVerbosity,
  FILE* logfile
) {
  s_iVerbosity = iVerbosity;
  s_logfile = logfile;
  }


/*
 * POP3 server
 */
int Popd(
  const void* pHost,
  int verbosity,
  FILE* logfile
) {
  int iRet;
  unsigned uIdleTimeout, uPollTimeout, val;
  SOCKET s;
  SConnection* pConnArray;
  size_t maxConn;
  SDeque* pDeque, *pNext;
  SConnection* pConn;
  time_t t;

  /* Read configurable values */
  uIdleTimeout = (unsigned) SWS_CfgGetValue( kszPopSect, kszIdleTimeout, IDLE_TIMEOUT);
  maxConn = (size_t) SWS_CfgGetValue( kszPopSect, kszMaxConnections, OPT_MAX_CONNECTIONS);
  uPollTimeout = (unsigned) SWS_CfgGetValue( kszPopSect, kszPollTimeout, POLL_TIMEOUT);
  s_iMaxAttempts = SWS_CfgGetValue( kszPopSect, kszMaxAttempts, MAX_ATTEMPTS);
  PopdSetLog( (int)SWS_CfgGetValue( kszPopSect, kszLogVerbosity, verbosity), logfile);
  SWS_CfgGetString( kszPopSect, kszGreeting, kszBanner " ready", s_szGreeting, sizeof s_szGreeting);
  SWS_CfgGetString( kszPopSect, kszBye, kszApp " signing off", s_szBye, sizeof s_szBye);
#if OPT_CRAM_MD5
  srand( (unsigned)time( NULL));
#endif

  /* Create server socket */
  s = socket( ((const struct sockaddr*)pHost)->sa_family, SOCK_STREAM, 0);
  if ( INVALID_SOCKET == s)
    {
    iRet = GetLastError();
    Log( kLogError, NULL, "ERROR %d: socket: %s\n", iRet, strerror( iRet));
    return iRet;
    }

  /* Server port specifed? */
  if ( AF_INET == ((const struct sockaddr*)pHost)->sa_family
    && 0 == ((const struct sockaddr_in*)pHost)->sin_port
  ) {
    /* No, set default */
    ((struct sockaddr_in*)pHost)->sin_port = htons( POPD_PORT);
    }

  /* Specify the service port */
  iRet = bind( s, (const struct sockaddr*)pHost, sizeof(struct sockaddr_in) );
  if ( iRet)
    {
    iRet = GetLastError();
    Log( kLogError, (const struct sockaddr_in*)pHost, "ERROR %d: bind: %s\n",
      iRet, strerror( iRet) );
    closesocket( s);
    return iRet;
    }

  /* Disable Nagle */
  val = 1;
  iRet = setsockopt(
    s,
    IPPROTO_TCP, TCP_NODELAY,
    (char*)&val, sizeof(val)
  );

  /* Start server listening */
  iRet = listen( s, SOMAXCONN);
  if ( iRet)
    {
    iRet = GetLastError();
    Log( kLogError, (const struct sockaddr_in*)pHost, "ERROR %d: listen: %s\n",
      iRet, strerror( iRet) );
    closesocket( s);
    return iRet;
    }

  if ( maxConn >= FD_SETSIZE)
    {
    Log( kLogError, NULL, "ERROR: %u connections greater than compilation limit of %u\n",
      (unsigned)maxConn, FD_SETSIZE - 1);
    closesocket( s);
    return EXIT_FAILURE;
    }

  /* Allocate free connection list */
  pConnArray = (SConnection*) calloc( maxConn, sizeof(SConnection) );
  if ( NULL == pConnArray)
    {
    Log( kLogError, NULL, "ERROR: Out of memory for %u connections\n", (unsigned)maxConn);
    closesocket( s);
    return EXIT_FAILURE;
    }

  DEQUE_INIT( &s_active);
  DEQUE_INIT( &s_free);
  for ( val = 0; val < maxConn; ++val)
    {
    DEQUE_INIT( &pConnArray[ val].list);
    DequeAppend( &s_free, &pConnArray[ val].list);
    }

#ifdef SIGPIPE
  /* Ignore SIGPIPE if connection reset during send() */
  signal( SIGPIPE, SIG_IGN);
#endif

#if OPT_UPNP
  {
  unsigned uExtPort = SWS_CfgGetValue( kszPopSect, "UpnpExtPort", 0);
  if ( uExtPort && SWS_UpnpAddPortMap(
      "TCP",
      ntohs( ((const struct sockaddr_in*)pHost)->sin_port),
      inet_ntoa( ((const struct sockaddr_in*)pHost)->sin_addr),
      uExtPort,
      kszApp
     )
  )
    uExtPort = 0;
#endif

  Log( kLogWarn, (const struct sockaddr_in*)pHost, "LISTENING\n");

  /* The server loop */
  ++g_running;
  while ( !g_exit)
    {
#if OPT_MAX_CONNECTIONS > 63
    static /* BUGBUG: static alloc precludes multithreading */
#endif
    fd_set setRd, setWr;
    struct timeval tval;

    FD_ZERO( &setWr);
    FD_ZERO( &setRd);

    if ( !DEQUE_IS_EMPTY( &s_free))
      FD_SET( s, &setRd);

    /* Add active connections to read set */
    for (
      pDeque = DEQUE_FIRST( &s_active);
      pDeque != &s_active;
      pDeque = DEQUE_FIRST( pDeque)
    ) {
      pConn = LIST2CONN( pDeque);
      if ( pConn->hMsg)
        FD_SET( pConn->s, &setWr);
      else
        FD_SET( pConn->s, &setRd);
      }

    /* Set timeout for select */
    tval.tv_usec = (uPollTimeout % 1000) * 1000UL;
    tval.tv_sec = uPollTimeout / 1000;

    /* Wait for event */
    if ( SOCKET_ERROR == select( FD_SETSIZE, &setRd, &setWr, NULL, &tval) )
      continue;

    t = time( NULL);

    /* Process event */
    if ( FD_ISSET( s, &setRd))
      {
      socklen_t len;

      /* Incoming connection */
      pConn = LIST2CONN( DequeRemove( DEQUE_FIRST( &s_free) ));

      pConn->tIdle = t + uIdleTimeout; /* Start the idle timer */

      pConn->eTelnetState = kTelnetData;
      pConn->telnetCount = 0;
      pConn->telnetCR = 0;

      pConn->eState = kStateAuthorise;
      pConn->szUser[ 0] = cNul;
      pConn->iAttempts = 0;
      pConn->isAuthenticated = 0;
      pConn->pUser = NULL;
      pConn->uMessages = 0;
      pConn->pMsgs = NULL;
      pConn->hMsg = 0;
      pConn->sizeToWrite = 0;

      /* Accept the connection */
      len = sizeof(pConn->client);
      pConn->s = accept( s, (struct sockaddr*)&pConn->client, &len);
      if ( INVALID_SOCKET == pConn->s)
        {
        iRet = GetLastError();
        Log( kLogWarn, NULL, "WARN %d: accept: %s\n", iRet, strerror( iRet));
        DequeAppend( &s_free, &pConn->list);
        }
      else
        {
        if ( s_logfile != stdout && s_logfile != stderr)
          Log( kLogError, &pConn->client, "CONNECT\n");

        DequeAppend( &s_active, &pConn->list);

        /* Enable non-blocking mode */
        setsockasync( pConn->s, 1);

        /* Set the send timeout */
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
      pDeque = DEQUE_FIRST( &s_active);
      pDeque != &s_active;
      pDeque = pNext
    ) {
      pNext = DEQUE_FIRST( pDeque);
      pConn = LIST2CONN( pDeque);

      if ( FD_ISSET( pConn->s, &setRd))
        {
        /* Restart the idle timer */
        pConn->tIdle = t + uIdleTimeout;

        /* Process the input */
        SessionInput( pConn);
        }
      else if ( FD_ISSET( pConn->s, &setWr))
        {
        /* Restart the idle timer */
        pConn->tIdle = t + uIdleTimeout;

        /* More output */
        SessionOutput( pConn);
        }
      else if ( t > pConn->tIdle)
        {
        /* Timed out */
        Log( kLogMore, &pConn->client, "Idle timeout\n" );
        SessionAbort( pConn);
        }

      if ( kStateUpdate == pConn->eState)
        {
        SMessage* pMsg;
        unsigned uMsg;

        /* Delete messages */
        pMsg = pConn->pMsgs;
        for ( uMsg = 1; uMsg <= pConn->uMessages; ++uMsg, ++pMsg)
          {
          if ( pMsg->isDeleted)
            (void)PopMsgDel( pConn->pUser, uMsg);
          }
        PopMsgCommit( pConn->pUser);

        Reset( pConn);
        pConn->eState = kStateQuit;
        }

      /* Is the session finished? */
      if ( kStateQuit == pConn->eState)
        {
        /* Yes */
        if ( s_logfile != stdout && s_logfile != stderr)
          Log( kLogError, &pConn->client, "DISCONNECT\n");

        SessionFinish( pConn);

        /* Disable non-blocking mode and close */
        setsockasync( pConn->s, 0);
        closesocket( pConn->s);

        /* Return it to the free list */
        DequeRemove( &pConn->list);
        DequeAppend( &s_free, &pConn->list);
        }
      }
    } /* while ( !g_exit) */
  --g_running;

  pNext = NULL;
  for (
    pDeque = DEQUE_FIRST( &s_active);
    pDeque != &s_active;
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
  closesocket( s);

  return iRet;
  }


/*
 * Start a session
 */
void SessionStart( SConnection* pConn)
  {
  strcpy( pConn->input, "OPEN");
  pConn->pszTail = "";
  pConn->eState = kStateAuthorise;

  /* Send sign-on greeting */
  Response( pConn, 0, s_szGreeting);
  }


/*
 * Terminate a session
 */
static void SessionFinish( SConnection* pConn)
  {
  Log( kLogInfo, &pConn->client, "CLOSE\n");

  Reset( pConn);
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

  Reset( pConn);
  pConn->eState = kStateQuit;
  }


/*
 * Input a command line and execute it
 */
void SessionInput( SConnection* pConn)
  {
  int iRet;

  /* Get a command line */
  iRet = TelnetReadLine( pConn);
  if ( 0 == iRet)
    return; /* Not complete */

  if ( SOCKET_ERROR == iRet)
    {
    /* Disconnected */
    Reset( pConn);
    pConn->eState = kStateQuit;
    }
  else if ( kStateSasl == pConn->eState)
    {
    /* Process client response, discounting trailing CRLF */
    pConn->pszTail = "";
    AuthInput( pConn);
    }
  else
    {
    /* Execute the request */
    ExecuteRequest( pConn, pConn->input);
    }
  }


/*
 * Execute a command
 */
int ExecuteRequest(
  SConnection* pConn,
  char* pszCmd
) {
  char* pszResponse = pConn->buffer + (sizeof(pConn->buffer) - MAX_REPLY);
  char *pszTail;
  const SMethod* pMethod;
  int iRet;

  /* Skip leading space */
  while ( isspace( *pszCmd))
    ++pszCmd;

  /* Break into command name + tail */
  pszTail = pszCmd;
  while ( cNul != *pszTail && !isspace( *pszTail))
    {
    *pszTail = (char)toupper( *pszTail);
    ++pszTail;
    }
  if ( cNul != *pszTail)
    {
    *(pszTail++) = cNul;
    while ( cNul != *pszTail && isspace( *pszTail))
      ++pszTail;
    }
  pConn->pszTail = pszTail;

  *pszResponse = cNul;

  /* Find the command */
  pMethod = FindCommand( pszCmd);
  if ( NULL != pMethod)
    {
    /* Execute the command */
    iRet = (*pMethod->pfnExecute)( pConn, pszTail, pszResponse);
    }
  else
    {
    iRet = UnimplementedMethod( pConn, pszTail, pszResponse);
    }

  return Response( pConn, iRet, pszResponse);
  }


/*
 * Find a command descriptor given a string
 */
static const SMethod* FindCommand(
  const char* pszCmd
) {
  const SMethod* pMethod = methods;
  int i;
  for ( i = sizeof( methods) / sizeof( methods[0]); --i >= 0; ++pMethod)
    {
    if ( 0 == SWS_stricmp( pszCmd, pMethod->pszName) )
      return pMethod;
    }

  return NULL;
  }


/*
 * Send a POP3 response
 */
static int Response(
  SConnection* pConn,
  int code,
  const char* pszInfo
) {
  const char* pszPrefix;

  if ( -1 == code || 0 == strcmp( pszInfo, "."))
    pszPrefix = "";
  else if ( 0 == code)
    pszPrefix = "+OK ";
  else
    pszPrefix = "-ERR ";

  /* Disable non-blocking mode */
  setsockasync( pConn->s, 0);

  (void)WriteRaw(
    pConn,
    pConn->buffer,
    sprintf( pConn->buffer, "%s%s" CRLF, pszPrefix, pszInfo)
  );

  /* Restore non-blocking mode */
  setsockasync( pConn->s, 1);

  if ( -1 == code)
    Log( kLogMore, &pConn->client, "%s\n", pszInfo);
  else if ( *pszPrefix)
    {
    /* Log the request and final response */
    Log( 0 == code ? kLogInfo : kLogWarn, &pConn->client, "%s%s%s %s%s\n",
      pConn->input, cNul != pConn->pszTail[ 0] ? " " : "", pConn->pszTail,
      pszPrefix, pszInfo
    );
    }

  return code;
  }


/****************************************************************************
 * POP3 command handlers
 ****************************************************************************/
/*
 * CAPA command
 */
static int CapaMethod(
  SConnection* pConn,
  const char* pszTail,
  char* pszInfo
) {
  while ( isspace( *pszTail))
    ++pszTail;

  if ( *pszTail)
    {
    strcpy( pszInfo, "Unsupported argument");
    return 1;
    }

  Response( pConn, 0, "Capability list follows");

  Response( pConn, -1, "CAPA");
  Response( pConn, -1, "TOP");
  Response( pConn, -1, "USER");
#if OPT_CRAM_MD5
#define kszMD5 " CRAM-MD5"
#else
#define kszMD5 ""
#endif
  Response( pConn, -1, "SASL PLAIN LOGIN" kszMD5);
  Response( pConn, -1, "RESP-CODES");
  Response( pConn, -1, "AUTH-RESP-CODE");
  Response( pConn, -1, "PIPELINING");
  Response( pConn, -1, "EXPIRE NEVER");
  Response( pConn, -1, "UIDL");
  Response( pConn, -1, "IMPLEMENTATION " kszBanner);

  strcpy( pszInfo, ".");

  return 0;
  }


/*
 * USER command
 */
static int UserMethod(
  SConnection* pConn,
  const char* pszTail,
  char* pszInfo
) {
  int iErr;

  /* Ensure username present */
  if ( cNul == *pszTail)
    {
    strcpy( pszInfo, "Syntax: USER username");
    return 1;
    }

  while ( isspace( *pszTail))
    ++pszTail;

  if ( strlen( pszTail) >= sizeof( pConn->szUser))
    {
    strcpy( pszInfo, "Username too long");
    return 1;
    }
  strcpy( pConn->szUser, pszTail);

  Reset( pConn);

  /* Attempt login with no password */
  iErr = PopMsgCreateUser( &pConn->pUser, pConn->szUser, NULL, s_iVerbosity, s_logfile);
  switch ( iErr)
    {
  case kPopMsgErrNone:
    pConn->iAttempts = 0;

    /* Login OK so enumerate messages */
    if ( !EnumMessages( pConn))
      {
      pConn->eState = kStateTransaction;
      sprintf( pszInfo, "Welcome %s, you have %u message(s)",
        pConn->szUser, pConn->uMessages);
      iErr = 0;
      }
    else
      {
      strcpy( pszInfo, "[SYS/TEMP] Out of memory");
      iErr = 1;
      }
    break;

  case kPopMsgErrBadUser:
    pConn->iAttempts = 0;
    pConn->eState = kStatePassword;
    strcpy( pszInfo, "Password required");
    iErr = 0;
    break;

  case kPopMsgErrMbxLocked:
    pConn->eState = kStateQuit;
    strcpy( pszInfo, "[IN-USE] Mailbox locked by another user");
    iErr = 1;
    break;

  case kPopMsgErrNoMem:
    pConn->eState = kStateQuit;
    strcpy( pszInfo, "[SYS/TEMP] Out of memory");
    iErr = 1;
    break;

  case kPopMsgErrFailed:
    pConn->eState = kStateQuit;
    strcpy( pszInfo, PopMsgLastError());
    iErr = 1;
    break;

  default:
    pConn->eState = kStateQuit;
    strcpy( pszInfo, "{SYS} Unable to login user");
    iErr = 1;
    break;
    }

  return iErr;
  }


/*
 * PASS command
 */
static int PassMethod(
  SConnection* pConn,
  const char* pszTail,
  char* pszInfo
) {
  int iErr;

  if ( kStatePassword != pConn->eState)
    {
    strcpy( pszInfo, "Enter USER first");
    *(char*)pszTail = cNul; /* Suppress printing of password */
    return 1;
    }

  /* Validate login */
  iErr = PopMsgCreateUser( &pConn->pUser, pConn->szUser, pszTail, s_iVerbosity, s_logfile);
  *(char*) pszTail = cNul; /* Suppress printing of password */
  switch ( iErr)
    {
  case kPopMsgErrNone:
    pConn->iAttempts = 0;

    /* Login OK so enumerate messages */
    if ( !EnumMessages( pConn))
      {
      pConn->eState = kStateTransaction;
      sprintf( pszInfo, "Welcome %s, you have %u message(s)",
        pConn->szUser, pConn->uMessages);
      iErr = 0;
      }
    else
      {
      strcpy( pszInfo, "[SYS/TEMP] Out of memory");
      iErr = 1;
      }
    break;

  case kPopMsgErrBadUser:
    strcpy( pszInfo, "[AUTH] Incorrect login");
    iErr = 1;
    break;

  case kPopMsgErrMbxLocked:
    strcpy( pszInfo, "[IN-USE] Mailbox locked by another user");
    iErr = 1;
    break;

  case kPopMsgErrNoMem:
    strcpy( pszInfo, "[SYS/TEMP] Out of memory");
    iErr = 1;
    break;

  case kPopMsgErrFailed:
    strcpy( pszInfo, PopMsgLastError());
    iErr = 1;
    break;

  case kPopMsgErrParam:
  default:
    strcpy( pszInfo, "[SYS] Unable to login user");
    iErr = 1;
    break;
    }

  if ( kStatePassword == pConn->eState
    && ++pConn->iAttempts >= s_iMaxAttempts
  ) {
    pConn->eState = kStateQuit;
    }

  return iErr;
  }


/*
 * AUTH command
 */
static int AuthMethod(
  SConnection* pConn,
  const char* pszTail,
  char* pszInfo
) {
  while ( isspace( *pszTail))
    ++pszTail;

  if ( cNul == *pszTail)
    {
    /* Non-standard auth enumeration */
    Response( pConn, 0, "");
#if OPT_CRAM_MD5
    Response( pConn, -1, "CRAM-MD5");
#endif
    Response( pConn, -1, "PLAIN");
    Response( pConn, -1, "LOGIN");
    strcpy( pszInfo, ".");
    return 0;
    }
#if OPT_CRAM_MD5
  else if ( 0 == SWS_stricmp( pszTail, "CRAM-MD5"))
    {
    /* RFC 2195 */
    char szBuf[ 64];
    size_t n;
    socklen_t len;
    struct sockaddr_in addr;

    Reset( pConn);

    /* Form a challenge */
    len = sizeof addr;
    getsockname( pConn->s, (struct sockaddr*)&addr, &len);
    n = sprintf( pConn->szChallenge, "<%08lx.%08lx@%s>",
     (unsigned long)rand(),
     (unsigned long)time( NULL),
     inet_ntoa( addr.sin_addr)
    );
    Log( kLogInfo, &pConn->client, "AUTH CRAM-MD5 %s\n", pConn->szChallenge);

    /* Return server challenge in base64, await initial-response */
    SWS_Base64Encode( szBuf, sizeof szBuf, (const unsigned char*)pConn->szChallenge, n);
    sprintf( pszInfo, "+ %s", szBuf);
    pConn->isAuthenticated = -3;
    pConn->eState = kStateSasl;
    return -1;
    }
#endif
  else if ( 0 == SWS_strnicmp( pszTail, "PLAIN", 5))
    {
    /* RFC 4616 & 2585 */
    if ( cNul != pszTail[5] && !isspace( pszTail[5]) )
      {
      strcpy( pszInfo, "Syntax: AUTH PLAIN [text]");
      return 1;
      }
    pszTail += 5;
    while ( isspace( *pszTail))
      ++pszTail;

    Reset( pConn);

    if ( cNul == *pszTail)
      {
      /* Return NULL server challenge, await initial-response */
      pConn->isAuthenticated = 0;
      pConn->eState = kStateSasl;
      strcpy( pszInfo, "+ "); /* NB single space after + */
      Log( kLogInfo, &pConn->client, "AUTH PLAIN %s\n", pszInfo);
      return -1;
      }

    pConn->eState = kStatePassword;
    return AuthPlain( pConn, pszTail, pszInfo);
    }
  else if ( 0 == SWS_strnicmp( pszTail, "LOGIN", 5))
    {
    pszTail += 5;
    while ( isspace( *pszTail))
      ++pszTail;

    if ( cNul != *pszTail)
      {
      strcpy( pszInfo, "Syntax: AUTH LOGIN");
      return 1;
      }

    Reset( pConn);

    /* Return server challenge in base64 */
    strcpy( pszInfo, "+ VXNlcm5hbWU6"); /* "Username:" */
    Log( kLogInfo, &pConn->client, "AUTH LOGIN %s\n", pszInfo);

    pConn->isAuthenticated = -1; /* Expect username */
    pConn->eState = kStateSasl;
    return -1;
    }

  strcpy( pszInfo, "Mechanism not supported");
  return 1;
  }


/*
 * Verify AUTH PLAIN response
 */
static int AuthPlain(
  SConnection* pConn,
  const char* pszText,
  char* pszInfo
) {
  char* pszAuthcid;
  size_t len;

  len = SWS_Base64Decode(
    (unsigned char*)pConn->buffer, sizeof pConn->buffer - 3, pszText);
  if ( len < 2)
    {
    pConn->eState = kStateAuthorise;
    strcpy( pszInfo, "Invalid base64 text");
    return 1;
    }
  memset( pConn->buffer + len, cNul, 3);

  /* Skip optional authorization identity RFC 4616 */
  pszAuthcid = pConn->buffer;
  if ( cNul != *pszAuthcid)
    pszAuthcid += 1 + strlen( pszAuthcid);
  else
    ++pszAuthcid;

  /* Validate login */
  strncpy( pConn->szUser, pszAuthcid, sizeof pConn->szUser - 1);
  return PassMethod( pConn, pszAuthcid + 1 + strlen( pszAuthcid), pszInfo);
  }


/*
 * Process AUTH response
 */
static void AuthInput( SConnection* pConn)
  {
  char* const pszInfo = pConn->buffer + (sizeof pConn->buffer - MAX_REPLY);
  int iErr;
  size_t len;

  /* RFC 5034 */
  if ( 0 == strcmp( "*", pConn->input))
    {
    pConn->isAuthenticated = 0;
    pConn->eState = kStateAuthorise;
    strcpy( pConn->input, "AUTH");
    strcpy( pszInfo, "[AUTH] Cancelled");
    iErr = 1;
    }
  else switch ( pConn->isAuthenticated)
    {
  case 0:
    /* Setup log output from Response() */
    strcpy( pConn->buffer, pConn->input);
    sprintf( pConn->input, "AUTH PLAIN %s", pConn->buffer);

    /* Validate login */
    pConn->eState = kStatePassword;
    iErr = AuthPlain( pConn, pConn->buffer, pszInfo);
    break;

  case -1: /* Login username */
    len = SWS_Base64Decode(
      (unsigned char*) pConn->szUser, sizeof  pConn->szUser - 1, pConn->input);
    pConn->szUser[ len] = cNul;

    pConn->isAuthenticated = -2; /* Expect password */
    strcpy( pszInfo, "+ UGFzc3dvcmQ6"); /* Password: */
    Log( kLogInfo, &pConn->client, "AUTH LOGIN %s %s\n", pConn->szUser, pszInfo);
    iErr = -1;
    break;

  case -2: /* Login password */
    len = SWS_Base64Decode(
      (unsigned char*)pConn->buffer, sizeof pConn->buffer - 1, pConn->input);
    pConn->buffer[ len] = cNul;
    strcpy( pConn->input, "AUTH LOGIN");

    /* Validate login */
    pConn->eState = kStatePassword;
    iErr = PassMethod( pConn, pConn->buffer, pszInfo);
    break;

#if OPT_CRAM_MD5
  case -3: /* CRAM-MD5 */
    {
    char szResponse[ MD5_LEN + sizeof pConn->szUser +2], *pszDigest;
    char szPass[ MAX_PASS +1], szDigest[ MD5_LEN +1];

    /* Decode response */
    len = SWS_Base64Decode( (unsigned char*) szResponse, sizeof szResponse - 1, pConn->input);
    szResponse[ len] = cNul;
    if ( 0 == len)
      {
      pConn->isAuthenticated = 0;
      pConn->eState = kStateAuthorise;
      strcpy( pszInfo, "Not base64");
      iErr = 1;
      break;
      }

    /* Extract username from response */
    len = strcspn( szResponse, " ");
    strncpy( pConn->szUser, szResponse, len);
    pConn->szUser[ len] = cNul;
    pszDigest = szResponse + len + 1;
    sprintf( pConn->input, "AUTH CRAM-MD5 %s", pConn->szUser);

    /* Get password */
    if ( !FindUser( szPass, pConn->szUser))
      {
      pConn->eState = kStatePassword;
      iErr = PassMethod( pConn, "", pszInfo);
      break;
      }

    /* Compare digests */
    CramMD5digest( szDigest, pConn->szChallenge, szPass);
    if ( strcmp( pszDigest, szDigest))
      strcpy( szPass, "-");

    /* Validate login */
    pConn->eState = kStatePassword;
    iErr = PassMethod( pConn, szPass, pszInfo);
    }
    break;
#endif


  default:
    pConn->isAuthenticated = 0;
    pConn->eState = kStateAuthorise;
    iErr = 1;
    break;
    }

  Response( pConn, iErr, pszInfo);
  }


/*
 * STAT command
 */
static int StatMethod(
  SConnection* pConn,
  const char* pszTail,
  char* pszInfo
) {
  unsigned uMsg, uMessages;
  SMessage* pMsg;
  unsigned long ulSize;

  if ( kStateTransaction != pConn->eState)
    {
    strcpy( pszInfo, "Login required");
    return 1;
    }

  if ( *pszTail)
    {
    strcpy( pszInfo, "Syntax error: unexpected parameter");
    return 1;
    }

  /* Calc message total and size */
  uMessages = 0;
  ulSize = 0;
  pMsg = pConn->pMsgs;
  for ( uMsg = 1; uMsg <= pConn->uMessages; ++uMsg, ++pMsg)
    {
    if ( !pMsg->isDeleted)
      {
      ++uMessages;
      ulSize += pMsg->ulSize;
      }
    }

  sprintf( pszInfo, "%u %lu", pConn->uMessages, ulSize);
  return 0;
  }


/*
 * LIST command
 */
static int ListMethod(
  SConnection* pConn,
  const char* pszTail,
  char* pszInfo
) {
  unsigned uMsg;
  SMessage* pMsg;

  if ( kStateTransaction != pConn->eState)
    {
    strcpy( pszInfo, "Login required");
    return 1;
    }

  while ( isspace( *pszTail))
    ++pszTail;

  if ( *pszTail)
    {
    if ( 1 != sscanf( pszTail, " %u", &uMsg)
      || uMsg < 1
      || uMsg > pConn->uMessages
    ) {
      strcpy( pszInfo, "No such message");
      return 1;
      }

    pMsg = pConn->pMsgs + (uMsg - 1);
    if ( pMsg->isDeleted)
      {
      strcpy( pszInfo, "Message is deleted");
      return 1;
      }

    sprintf( pszInfo, "%u %lu", uMsg, pMsg->ulSize);
    }
  else
    {
    unsigned uMessages = 0;

    pMsg = pConn->pMsgs;
    for ( uMsg = 1; uMsg <= pConn->uMessages; ++uMsg, ++pMsg)
      {
      if ( !pMsg->isDeleted)
        ++uMessages;
      }

    sprintf( pszInfo, "%u messages", uMessages);
    Response( pConn, 0, pszInfo);

    pMsg = pConn->pMsgs;
    for ( uMsg = 1; uMsg <= pConn->uMessages; ++uMsg, ++pMsg)
      {
      if ( !pMsg->isDeleted)
        {
        sprintf( pszInfo, "%u %lu", uMsg, pMsg->ulSize);
        Response( pConn, -1, pszInfo);
        }
      }

    strcpy( pszInfo, ".");
    }

  return 0;
  }


/*
 * UIDL command
 */
static int UidlMethod(
  SConnection* pConn,
  const char* pszTail,
  char* pszInfo
) {
  unsigned uMsg;
  SMessage* pMsg;
  const char* pszUidl;

  if ( kStateTransaction != pConn->eState)
    {
    strcpy( pszInfo, "Login required");
    return 1;
    }

  while ( isspace( *pszTail))
    ++pszTail;

  if ( *pszTail)
    {
    if ( 1 != sscanf( pszTail, " %u", &uMsg)
      || uMsg < 1
      || uMsg > pConn->uMessages
    ) {
      strcpy( pszInfo, "No such message");
      return 1;
      }

    pMsg = pConn->pMsgs + (uMsg - 1);
    if ( pMsg->isDeleted)
      {
      strcpy( pszInfo, "Message is deleted");
      return 1;
      }

    pszUidl = PopMsgUidl( pConn->pUser, uMsg);
    if ( NULL == pszUidl)
      {
      strcpy( pszInfo, "Not supported");
      return 1;
      }

    sprintf( pszInfo, "%u %.70s", uMsg, pszUidl);
    }
  else
    {
    Response( pConn, 0, "unique-id listing follows");

    pMsg = pConn->pMsgs;
    for ( uMsg = 1; uMsg <= pConn->uMessages; ++uMsg, ++pMsg)
      {
      if ( !pMsg->isDeleted)
        {
        pszUidl = PopMsgUidl( pConn->pUser, uMsg);
        if ( NULL != pszUidl)
          sprintf( pszInfo, "%u %.70s", uMsg, pszUidl);
        else
          sprintf( pszInfo, "%u ", uMsg);

        Response( pConn, -1, pszInfo);
        }
      }

    strcpy( pszInfo, ".");
    }

  return 0;
  }


/*
 * RETR command
 */
static int RetrMethod(
  SConnection* pConn,
  const char* pszTail,
  char* pszInfo
) {
  unsigned uMsg;
  SMessage* pMsg;

  if ( kStateTransaction != pConn->eState)
    {
    strcpy( pszInfo, "Login required");
    return 1;
    }

  /* Parse args */
  pConn->uTop = UINT_MAX;
  if ( sscanf( pszTail, " %u %u", &uMsg, &pConn->uTop) < 1)
    {
    strcpy( pszInfo, "Syntax: RETR message [top]");
    return 1;
    }

  if ( uMsg < 1
    || uMsg > pConn->uMessages
  ) {
    strcpy( pszInfo, "No such message");
    return 1;
    }
  pMsg = pConn->pMsgs + (uMsg - 1);

  if ( pMsg->isDeleted)
    {
    strcpy( pszInfo, "Message is deleted");
    return 1;
    }

  pConn->hMsg = PopMsgOpen( pConn->pUser, uMsg);
  if ( !pConn->hMsg)
    {
    strcpy( pszInfo, "Message not found");
    return 1;
    }

  pConn->inBody = 0;
  pConn->iEol = 1; /* CRLF sent */
  pConn->sizeToWrite = 0;

  strcpy( pszInfo, "Message follows");
  return 0;
  }


/*
 * Ready for output
 */
void SessionOutput( SConnection* pConn)
  {
  char* psz;
  int iWritten;

  /* More in buffer? */
  if ( pConn->sizeToWrite > 0)
    {
    /* Yes, send remainder */
    iWritten = WriteRaw( pConn, pConn->pcToWrite, pConn->sizeToWrite);
    if ( SOCKET_ERROR != iWritten )
      {
      pConn->sizeToWrite -= (size_t)iWritten;
      psz = (char*)(pConn->pcToWrite += (size_t)iWritten);
      }
    else
      {
      psz = NULL;
      }
    }
  /* Read another line */
  else if ( NULL != (psz = PopMsgRead( pConn->buffer + 1, MAX_LINE, pConn->hMsg)) )
    {
    size_t count = strlen( psz);

    /* Transparency */
    if ( pConn->iEol && '.' == *psz )
      {
      /* Prepend a period */
      *--psz = '.';
      ++count;
      }

    /* Remove trailing '\n' */
    if ( count > 0 && '\n' == psz[ count - 1] )
      {
      pConn->iEol = 1;
      if ( 0 == --count)
        pConn->inBody = 1; /* Blank line starts the body */

      /* Append CR LF */
      strcpy( psz + count, CRLF);
      count += sizeof( CRLF) - 1;
      }
    else
      {
      pConn->iEol = 0;
      }

    iWritten = WriteRaw( pConn, psz, count);
    if ( SOCKET_ERROR != iWritten)
      {
      pConn->pcToWrite = psz + (size_t)iWritten;
      pConn->sizeToWrite = count - (size_t)iWritten;
      }
    else
      {
      psz = NULL;
      }
    }

  /* Decrement body lines remaining to send */
  if ( NULL != psz
    && 0 == pConn->sizeToWrite
    && pConn->inBody
    && UINT_MAX != pConn->uTop
    && 0 == pConn->uTop--
  ) {
    psz = NULL;
    }

  /* Close on EOF or error */
  if ( NULL == psz)
    {
    if ( pConn->hMsg)
      {
      PopMsgClose( pConn->pUser, pConn->hMsg);
      pConn->hMsg = 0;
      }

    /* Signal 'end of data' */
    if ( pConn->iEol)
      Response( pConn, 0, ".");
    else
      Response( pConn, 0, CRLF ".");
    }
  }


/*
 * DELE command
 */
static int DeleMethod(
  SConnection* pConn,
  const char* pszTail,
  char* pszInfo
) {
  unsigned uMsg;
  SMessage* pMsg;

  if ( kStateTransaction != pConn->eState)
    {
    strcpy( pszInfo, "Login required");
    return 1;
    }

  /* Ensure message number present */
  if ( 1 != sscanf( pszTail, " %u", &uMsg))
    {
    strcpy( pszInfo, "Syntax: DELE message");
    return 1;
    }

  if ( uMsg < 1
    || uMsg > pConn->uMessages
  ) {
    strcpy( pszInfo, "No such message");
    return 1;
    }
  pMsg = pConn->pMsgs + (uMsg - 1);

  if ( pMsg->isDeleted)
    {
    strcpy( pszInfo, "Message is deleted");
    return 1;
    }
  pMsg->isDeleted = 1;

  strcpy( pszInfo, "Message scheduled for deletion");
  return 0;
  }


/*
 * RSET command
 */
static int ResetMethod(
  SConnection* pConn,
  const char* pszTail,
  char* pszInfo
) {
  SMessage* pMsg;
  unsigned uMsg;

  if ( kStateTransaction != pConn->eState)
    {
    strcpy( pszInfo, "Login required");
    return 1;
    }

  if ( *pszTail)
    {
    strcpy( pszInfo, "Syntax error: unexpected parameter");
    return 1;
    }

  pMsg = pConn->pMsgs;
  for ( uMsg = 1; uMsg <= pConn->uMessages; ++uMsg, ++pMsg)
    pMsg->isDeleted = 0;

  sprintf( pszInfo, "%u messages", pConn->uMessages);
  return 0;
  }


/*
 * QUIT command (required)
 */
static int QuitCommand(
  SConnection* pConn,
  const char* pszTail,
  char* pszInfo
) {
  if ( *pszTail)
    {
    strcpy( pszInfo, "Syntax error: unexpected parameter");
    return 1;
    }

  if ( kStateTransaction == pConn->eState)
    {
    /* Enter the update state */
    pConn->eState = kStateUpdate;
    }
  else
    {
    pConn->eState = kStateQuit;
    }

  strcpy( pszInfo, s_szBye);
  return 0;
  }


/*
 * NOOP command (required)
 */
static int NoopMethod(
  SConnection* pConn,
  const char* pszTail,
  char* pszInfo
) {
  (void)pConn;
  (void)pszTail;
  (void)pszInfo;

  return 0;
  }


/*
 * All unimplememted commands
 */
static int UnimplementedMethod(
  SConnection* pConn,
  const char* pszTail,
  char* pszInfo
) {
  (void)pConn;
  (void)pszTail;

  strcpy( pszInfo, "Not implemented");
  return 1;
  }


/****************************************************************************
 * Support
 ****************************************************************************/
/*
 * Reset a session
 */
static void Reset( SConnection* pConn)
  {
  if ( pConn->hMsg)
    {
    PopMsgClose( pConn->pUser, pConn->hMsg);
    pConn->hMsg = 0;
    }

  if ( NULL != pConn->pUser)
    {
    PopMsgDestroyUser( pConn->pUser);
    pConn->pUser = NULL;
    }

  if ( NULL != pConn->pMsgs)
    {
    free( pConn->pMsgs);
    pConn->pMsgs = NULL;
    }
  pConn->uMessages = 0;
  pConn->eState = kStateAuthorise;
  }


/*
 * Enumerate messages
 */
int EnumMessages( SConnection* pConn)
  {
  pConn->uMessages = PopMsgCount( pConn->pUser);
  if ( pConn->uMessages > 0)
    {
    unsigned uMsg;
    SMessage* pMsg;

    pConn->pMsgs = (SMessage*) calloc( pConn->uMessages, sizeof( SMessage) );
    if ( NULL == pConn->pMsgs)
      {
      PopMsgDestroyUser( pConn->pUser);
      pConn->pUser = NULL;
      return 1;
      }

    for ( uMsg = 1, pMsg = pConn->pMsgs; uMsg <= pConn->uMessages; ++uMsg, ++pMsg)
      {
      pMsg->isDeleted = 0;
      pMsg->ulSize = PopMsgSize( pConn->pUser, uMsg);
      }
    }

  return 0;
  }


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


#if OPT_CRAM_MD5
/*
 * Block xor
 */
static void xxor(
  char data[],          /* out src1 ^ src2 */
  int len,              /* in len */
  const char v1[],      /* in src1 */
  const char v2[]       /* in src2 */
) {
  register unsigned int* idata = (unsigned int*)data;
  register const unsigned int* iv1 = (const unsigned int*)v1;
  register const unsigned int* iv2 = (const unsigned int*)v2;

  switch ( len)
    {
  case 4 * sizeof(unsigned int):
    *idata++ = *iv1++ ^ *iv2++;
  case 3 * sizeof(unsigned int):
    *idata++ = *iv1++ ^ *iv2++;
  case 2 * sizeof(unsigned int):
    *idata++ = *iv1++ ^ *iv2++;
  case 1 * sizeof(unsigned int):
    *idata++ = *iv1++ ^ *iv2++;
  case 0:
    break;

  default:
    for ( ; len >= (int)sizeof(unsigned int); len -= sizeof(unsigned int) )
      {
      *(unsigned int*)data = *(const unsigned int*)v1 ^ *(const unsigned int*)v2;
      data += sizeof(unsigned int), v1 += sizeof(unsigned int), v2 += sizeof(unsigned int);
      }
    for ( ; len > 0; --len)
      *data++ = *v1++ ^ *v2++;
    break;
    }
  }


/*
 * Calculate MD5 digest per RFC 2195
 */
static void CramMD5digest(
  char szDigest[ MD5_LEN],      /* OUT */
  const char* pszChallenge,     /* IN: Server challenge <rnd.time@host> */
  const char* pszPass           /* IN: Acct password */
) {
  size_t len;
  SWS_MD5 context;
  SWS_MD5Digest d;
  enum { kPadlen = 64 };
  char szBuf[ kPadlen];
  char buffer[ kPadlen + 128];

  len = strlen( pszPass);
  if ( len > kPadlen)
    {
    SWS_MD5Init( &context);
    SWS_MD5Update( &context, pszPass, len);
    SWS_MD5Final( &context, &d);
    memcpy( szBuf, d.digest, sizeof d.digest);
    memset( &szBuf[ sizeof d.digest], 0, kPadlen - sizeof d.digest);
    }
  else
    {
    /* Zero pad the password */
    memcpy( szBuf, pszPass, len);
    memset( &szBuf[ len], 0, kPadlen - len);
    }

  /* MD5( (passord ^ ipad) + challenge) */
  memset( buffer, 0x36, kPadlen); /* RFC 2104 ipad */
  xxor( buffer, kPadlen, szBuf, buffer);
  len = strlen( pszChallenge);
  memcpy( &buffer[ kPadlen], pszChallenge, len);
  SWS_MD5Init( &context);
  SWS_MD5Update( &context, buffer, kPadlen + len);
  SWS_MD5Final( &context, &d);

  /* MD5( (passord ^ opad) + 1st digest) */
  memset( buffer, 0x5C, kPadlen); /* RFC 2104 opad */
  xxor( buffer, kPadlen, szBuf, buffer);
  memcpy( &buffer[ kPadlen], d.digest, sizeof d.digest);
  SWS_MD5Init( &context);
  SWS_MD5Update( &context, buffer, kPadlen + sizeof d.digest);
  SWS_MD5Final( &context, &d);

  /* Convert digest to hex */
  for ( len = 0; len < sizeof d.digest; ++len)
    sprintf( &szDigest[ len *2], "%02x", d.digest[ len]);
  }


/*
 * Find a POP3 password given a username
 */
struct SPopInfo { const char* pszUser; char* pszPass; };
static int FindUser(                  /* !0 if successful */
  char szPass[ MAX_PASS +1],          /* OUT: Password */
  const char* pszUser                 /* IN: Username */
) {
  struct SPopInfo info;

  info.pszUser = pszUser;
  info.pszPass = szPass;
  return SWS_CfgEnum( kszPopUsersSect, &EnumPopCallback, &info) <= 0 ? 0 : 1;
  }

SWS_CB( int, EnumPopCallback( void* pv, const char* pszVal))
  {
  struct SPopInfo* const pInfo = (struct SPopInfo*)pv;
  char szUser[ 41], szPass[ MAX_PASS +1];

  /* Parse mailbox entry */
  if ( 2 == sscanf( pszVal, " %40s %"STRING(MAX_PASS)"s", szUser, szPass)
    && 0 == SWS_stricmp( pInfo->pszUser, szUser)
  ) {
    if ( 0 == strcmp( "-", szPass))
      pInfo->pszPass[0] = '\0';
    else
      strcpy( pInfo->pszPass, szPass);

    return 1;
    }

  return 0;
  }
#endif /*OPT_CRAM_MD5*/


/****************************************************************************
 * Telnet operations
 ****************************************************************************/
/*
 * Read a line
 */
int TelnetReadLine(
  SConnection* pConn
) {
  int iRet;

  while ( pConn->telnetCount < sizeof(pConn->input) - 1 )
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
        if ( pConn->telnetCount < sizeof(pConn->input) - 3 )
          {
          pConn->input[ pConn->telnetCount++] = cReturn;
          pConn->input[ pConn->telnetCount++] = (char)c;
          }
        break;
        }
      }
    else switch ( c)
      {
    default:
      if ( pConn->telnetCount <  sizeof(pConn->input) - 2 )
        pConn->input[ pConn->telnetCount++] = (char)c;
      break;

    case cReturn:
      pConn->telnetCR = 1;
      break;


    case cLinefeed:
      goto done;
      }
    }

done:
  pConn->input[ iRet = pConn->telnetCount] = cNul;
  pConn->telnetCount = 0;
  pConn->telnetCR = 0;

  return iRet + 1; /* Including CR */
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
      return -1; /* Error or connection closed */

    if ( 0 == iRet)
      return 0; /* No data */

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
          ((unsigned char*)pvBuf)[ len++] = c;
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
          pConn->eTelnetState = (ETelnetState) c;
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
    if ( EWOULDBLOCK == err || EAGAIN == err)
      {
      return 0;
      }
    else switch ( err)
      {
    case ECONNRESET:
      Log( kLogWarn, &pConn->client, "WARN %d: connection reset in send\n", err);
      SessionAbort( pConn);
      break;
    case ECONNABORTED:
      Log( kLogWarn, &pConn->client, "WARN %d: connection aborted in send\n", err);
      SessionAbort( pConn);
      break;
    default:
      Log( kLogWarn, &pConn->client, "WARN %d: send: %s\n", err, strerror( err));
      break;
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
      Log( kLogMore, &pConn->client, "WARM %d: connection reset in recv\n", err);
      SessionAbort( pConn);
      break;
    case ECONNABORTED:
      Log( kLogWarn, &pConn->client, "WARN %d: connection aborted in recv\n", err);
      SessionAbort( pConn);
      break;
    case ENOTCONN:
      Log( kLogWarn, &pConn->client, "WARN %d: disconnected in recv\n", err);
      break;
    default:
      Log( kLogWarn, &pConn->client, "WARN %d: recv: %s\n", err, strerror( err));
      break;
      }
    }
  else if ( 0 == iRet)
    {
    /* Graceful disconnect */
    return SOCKET_ERROR;
    }

  return iRet;
  }

/* End of file */
