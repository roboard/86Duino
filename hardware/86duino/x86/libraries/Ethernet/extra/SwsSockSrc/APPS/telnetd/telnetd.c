/*****************************************************************************
 *
 * File name   : telnetd.c
 * Function    : Telnet server (RFC 854)
 * Project     : SwsSock
 * Authors     : Lawrence Rust
 * Systems     : ANSI C, sockets
 * Version     :
 * Release Date:
 *
 *****************************************************************************
 * File History
 * ------------
 *
 * 20-Feb-03, LVR, Created.
 * 07-Aug-09, LVR, v2.02 - Catch SIGPIPE
 * 23-Dec-09, LVR, v2.03 - Added Log()
 *
 *
 * Comments
 * --------
 *
 *****************************************************************************/
#define kszApp "Software Systems Telnet service"
#define kszVer STRING( TELNETD_VER_MAJOR) "." STRING( TELNETD_VER_MINOR)
#define kszBanner kszApp " " kszVer " (" SWS_NET_ID ")"

/* Compilation options */
#ifndef SWS_USE_SOCKLIB
#define SWS_USE_SOCKLIB 0 /* 0= native sockets, !0 for sws_sock */
#endif

#ifndef OPT_MAX_CONNECTIONS
#define OPT_MAX_CONNECTIONS 3U /* Max no. simultaneous clients */
#endif
#if OPT_MAX_CONNECTIONS > 63 /* FD_SETSIZE is at least 64 */
#define FD_SETSIZE (1 + OPT_MAX_CONNECTIONS)
#endif

#ifndef TELNETD_EXT
#define TELNETD_EXT 0        /* !0 to submit unknown commands to host */
#endif
#define TELNETD_EXT_SYSTEM 1 /* Submit external commands using system() */
#define TELNETD_EXT_POSIX  2 /* Submit external commands using POSIX popen */

#ifndef OPT_UPNP
# define OPT_UPNP 0        /* !0 to export service to UPnP */
#endif


/* Exports */
#include "telnetd.h"

/* Imports */

/* ANSI C library */
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <signal.h>
#include <time.h>
#if _MSC_VER >= 1000
#include <crtdbg.h>
#endif

#if TELNETD_EXT == TELNETD_EXT_POSIX

#if defined WIN32 || defined _WIN32 || defined __WIN32__ || defined _DOS || defined _DOS16
#include <io.h> /* _dup2 */
#define popen  _popen
#define pclose _pclose
#else
#include <unistd.h> /* dup2 */
#endif

#elif TELNETD_EXT == TELNETD_EXT_SYSTEM

#ifndef TELNETD_STDOUT
# define TELNETD_STDOUT " >"
#endif
#if !defined TELNETD_STDERR
# if defined WIN32 || defined _WIN32 || defined __WIN32__
#  define TELNETD_STDERR " 2>&1"
# elif defined __CYGWIN__
#  define TELNETD_STDERR " 2>&1"
# elif defined __unix__ || defined __unix || defined unix
#  define TELNETD_STDERR " 2>&1"
# endif
#endif

#endif

/* SwsSock portable utils */
#include "sws_net.h"
#include "sws_str.h"   /* SWS_stricmp */
#include "sws_cfg.h"
#if OPT_UPNP
#include "sws_upnp.h"
#endif

#if SWS_USE_SOCKLIB
#include "sws_dlnk.h"   /* SWS_DatalinkGetProperty */
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

#ifdef __GNUC__
#define FORMAT(a,f,l)  __attribute__ ((format (a, f, l)))
#else
#define FORMAT(a,f,l)
#endif

#define kszPrefix "TLNTD: "  /* Log file prefix */

#define POLL_TIMEOUT    333U /* select timeout in millisecs */
#define SEND_TIMEOUT  60000U /* Send timeout in millisecs */
#define IDLE_TIMEOUT 180U    /* Recv idle timeout in seconds */

#ifndef MSG_NOSIGNAL
#define MSG_NOSIGNAL 0
#endif

#define cNul       '\0'
#define cReturn    '\x0d'
#define cLinefeed  '\x0a'

#define CRLF          "\x0d\x0a"
#define kszPathSep    "/"

#define kszLogin      "Login: "
#define kszPassword   "Password: "
#define kszPromptR    "- "
#define kszPromptW    "> "
#define kszPromptC    "$ "

#define MAX_ATTEMPTS  3 /* Max login attempts */

#define MAX_USERNAME 32 /* Max username length inc nul */
#define MAX_PROMPT   16 /* Max prompt inc nul */
#define MAX_LINE    256 /* Maximum input line length */
#define MAX_FQDN    256 /* Max hostname length inc nul */
#ifndef MAX_PATH
#define MAX_PATH    256
#endif
#define BUFFER_SIZE 1024 /* Buffer size */

/* ini file values */
#define kszTelnetSect     "telnet"
#define kszLogVerbosity   "log"
#define kszMaxConnections "max_conn"
#define kszIdleTimeout    "idle_time"
#define kszPermitted      "permit"
#define kszDenied         "deny"

#define kszTelnetUsersSect "telnet_users"


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
  kTelnetData     = 0,
  kTelnetBeginSub = 250U,
  kTelnetWill     = 251U,
  kTelnetWont     = 252U,
  kTelnetDo       = 253U,
  kTelnetDont     = 254U,
  kTelnetIAC      = 255U
  } ETelnetState;

/* Telnet options */
enum
  {
  kOptionBinary     = 0,
  kOptionEcho       = 1U,
  kOptionNoGoAhead  = 3U
  };

/* User permissions */
typedef enum EPermission
  {
  kPermitNone = 0,
  kPermitRead,
  kPermitWrite,
  kPermitChange
  } EPermission;

typedef enum EState
  {
  kStateUser,
  kStatePassword,
  kStateInterpret,
  kStateQuit
  } EState;

typedef void FOutput( struct STelnetConnection*);

/* Connection state */
struct STelnetConnection
  {
  SDeque list;
#define LIST2CONN( p) CONTAINER( p, STelnetConnection, list)

  SOCKET s;                     /* client connection */
  struct sockaddr_in client;    /* client's address */
  time_t tIdle;                 /* Time when idle connection will be closed */
  FOutput* pfnOutput;

  /* Telnet state */
  ETelnetState eTelnetState;
  int telnetCR;
  size_t telnetCount;
  char echoEnable;
  char binaryEnable;
  char outputDisable;
  char goAheadDisable;

  char echoEnableSave;
  int goAheadSent;
  int iInterrupt;

  /* User info */
  EState eState;                /* state */
  int iAttempts;                /* Password attempts */
  EPermission ePermit;          /* Access rights */
  char szUser[ MAX_USERNAME];
  char szPrompt[ MAX_PROMPT];   /* Prompt */
  char szHome[ MAX_PATH];       /* Home directory */

#if TELNETD_EXT == TELNETD_EXT_POSIX || TELNETD_EXT == TELNETD_EXT_SYSTEM
  FILE* outputFile;
  int cstderr;
  size_t sizePending;
  char* pPending;
#define L_UNIQUE 64
  char szTmpFile[ L_UNIQUE];    /* Redirected output file */
#endif

  const char* pszTail;          /* -> command tail */

  /* Input buffer */
  char input[ MAX_LINE];

  /* General purpose buffer */
  char buffer[ BUFFER_SIZE];
  };


/***************************************************************************
 * Function prototypes
 ***************************************************************************/

static void SessionStart( STelnetConnection*);
static void SessionFinish( STelnetConnection*);
static void SessionInput( STelnetConnection*);
static void SessionAbort( STelnetConnection*);

static int ExecuteRequest( STelnetConnection*, char*);
static const STelnetCommand* TelnetFindCommand( const char*, const STelnetCommand [], size_t);
static void TelnetPrompt( STelnetConnection*, int);

static int WriteRaw( STelnetConnection*, const void*, size_t);
static int ReadRaw( STelnetConnection*, void*, size_t);

static void Log( int, const struct sockaddr_in*, const char [], ...) FORMAT(printf,3,4);

static int TelnetUser( STelnetConnection*, char*);
static int TelnetPassword( STelnetConnection*, char*);
static void TelnetWelcome( STelnetConnection*);
static int ExternalCommand( STelnetConnection*, char*);
static EPermission ValidateUser( STelnetConnection* pConn, const char*);

#if TELNETD_EXT == TELNETD_EXT_POSIX || TELNETD_EXT == TELNETD_EXT_SYSTEM
static void ExternalStdout( STelnetConnection*);
static void ExternalFinal( STelnetConnection*);
#endif

static SDeque* DequeRemove( SDeque* const);
static void DequeAppend( SDeque* const pDeque, SDeque* const pItem);
#define DEQUE_INIT( p)     ((p)->tail = (p)->head = (p))
#define DEQUE_IS_EMPTY( p) ((p)->tail == (p))
#define DEQUE_FIRST( p)    ((p)->tail)
#define DEQUE_LAST( p)     ((p)->head)

#if TELNETD_EXT == TELNETD_EXT_POSIX || TELNETD_EXT == TELNETD_EXT_SYSTEM
static char* UniqueName( char [ L_UNIQUE] );
#endif
static int PlatformChdir( const char*, char*, size_t);


/***************************************************************************
 * Module data
 ***************************************************************************/

/* Configuration */
static unsigned s_uIdleTimeout;

/* Logging */
static int s_iVerbosity;
static FILE* s_logfile;

extern sig_atomic_t volatile g_exit;
extern sig_atomic_t volatile g_running;
static SDeque s_free, s_active;

/* The command table */
static FTelnetCommand HelpCommand;
static FTelnetCommand QuitCommand;
static FTelnetCommand NoopCommand;
static FTelnetCommand IpconfigCommand;
static FTelnetCommand ChdirCommand;

static const STelnetCommand builtin[] = {
  /* Add more commands below */
  { "cd",       &ChdirCommand,    "Change current directory." },
  { "ipconfig", &IpconfigCommand, "Display the IP configuration." },
  { "?",        &HelpCommand,     "Provides command line help."  },
  { "help",     &HelpCommand,     "Provides command line help."  },
  { "",         &NoopCommand,     NULL },
  { "quit",     &QuitCommand,     "Terminate the telnet session." }
};

static const STelnetCommand* s_pCmds;
static size_t s_cmds;


/***************************************************************************
 * Functions
 ***************************************************************************/

/*
 * Telnet server
 */
int Telnetd(
  const void* pHost,
  int iVerbosity,
  FILE* logfile,
  size_t cmds,
  const STelnetCommand pCmds[]
) {
  int iRet, tos;
  unsigned val;
  SOCKET s;
  STelnetConnection* pConnArray;
  size_t maxConn;
  SDeque* pDeque, *pNext;
  STelnetConnection* pConn;

  /* Save user command table */
  s_pCmds = pCmds;
  s_cmds = cmds;

  /* Read configurable values */
  s_uIdleTimeout = (unsigned)SWS_CfgGetValue( kszTelnetSect, kszIdleTimeout, IDLE_TIMEOUT);
  maxConn = (size_t)SWS_CfgGetValue( kszTelnetSect, kszMaxConnections, OPT_MAX_CONNECTIONS);
  s_iVerbosity = (int)SWS_CfgGetValue( kszTelnetSect, kszLogVerbosity, iVerbosity);
  s_logfile = logfile;

  /* Open server socket */
  s = socket( ((const struct sockaddr*)pHost)->sa_family, SOCK_STREAM, 0);
  if ( INVALID_SOCKET == s)
    {
    iRet = GetLastError();
    Log( kLogError, NULL, "ERROR %d: socket: %s\n", iRet, strerror( iRet));
    return iRet;
    }

  /* Set the TOS */
  tos = 0x10; /* Minimise delay - standard for telnet */
  setsockopt(
    s,
    IPPROTO_IP, IP_TOS,
    (char*)&tos, sizeof(tos)
  );

  /* Server port specifed? */
  if ( AF_INET == ((const struct sockaddr*)pHost)->sa_family
    && 0 == ((const struct sockaddr_in*)pHost)->sin_port
  ) {
    /* No, set default */
    ((struct sockaddr_in*)pHost)->sin_port = htons( TELNETD_PORT);
    }

  /* Select the port to receive on */
  iRet = bind( s, (const struct sockaddr*)pHost, sizeof(struct sockaddr_in) );
  if ( iRet)
    {
    iRet = GetLastError();
    Log( kLogError, (const struct sockaddr_in*)pHost, "ERROR %d: bind: %s\n",
      iRet, strerror( iRet) );
    closesocket( s);
    return iRet;
    }

  /* Setup a listener */
  iRet = listen( s, SOMAXCONN);
  if ( iRet)
    {
    iRet = GetLastError();
    Log( kLogError, (const struct sockaddr_in*)pHost, "ERROR %d: listen: %s\n",
      iRet, strerror( iRet) );
    closesocket( s);
    return iRet;
    }

  if ( maxConn + 1 > FD_SETSIZE)
    {
    Log( kLogError, NULL, "ERROR: %u connections greater than compilation limit of %u\n",
      (unsigned)maxConn, FD_SETSIZE - 1);
    closesocket( s);
    return EXIT_FAILURE;
    }

  /* Allocate free connection list */
  pConnArray = (STelnetConnection*)calloc( maxConn, sizeof(STelnetConnection) );
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
  unsigned uExtPort = SWS_CfgGetValue( kszTelnetSect, "UpnpExtPort", 0);
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
    static fd_set setRd, setWr;
    time_t t;
    struct timeval tval;

    FD_ZERO( &setRd);
    FD_ZERO( &setWr);
    if ( !DEQUE_IS_EMPTY( &s_free))
      FD_SET( s, &setRd);

    /* Add active sockets to the select sets */
    for (
      pDeque = DEQUE_FIRST( &s_active);
      pDeque != &s_active;
      pDeque = DEQUE_FIRST( pDeque)
    ) {
      pConn = LIST2CONN( pDeque);

      if ( pConn->pfnOutput)
        FD_SET( pConn->s, &setWr);
      else
        FD_SET( pConn->s, &setRd);
      }

    /* Set timeout for select */
    tval.tv_usec = (POLL_TIMEOUT % 1000) * 1000UL;
    tval.tv_sec = POLL_TIMEOUT / 1000;

    /* Wait for listen event */
    iRet = select( FD_SETSIZE, &setRd, &setWr, NULL, &tval);
    if ( SOCKET_ERROR == iRet)
      continue;

    t = time( NULL);

    /* Process receive event */
    if ( FD_ISSET( s, &setRd))
      {
      socklen_t len;

      /* Incoming connection */
      pConn = LIST2CONN( DequeRemove( DEQUE_FIRST( &s_free) ));

      pConn->tIdle = t + s_uIdleTimeout; /* Start the idle timer */
      pConn->pfnOutput = 0;

      pConn->eTelnetState = kTelnetData;
      pConn->telnetCR = 0;
      pConn->telnetCount = 0;

      /* Accept the incoming connection */
      len = sizeof( pConn->client);
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
      pDeque = DEQUE_FIRST( &s_active);
      pDeque != &s_active;
      pDeque = pNext
    ) {
      pNext = DEQUE_FIRST( pDeque);
      pConn = LIST2CONN( pDeque);

      if ( FD_ISSET( pConn->s, &setRd))
        {
        /* Restart the idle timer */
        pConn->tIdle = t + s_uIdleTimeout;

        /* More input */
        SessionInput( pConn);
        }

      if ( pConn->pfnOutput
        && FD_ISSET( pConn->s, &setWr)
      ) {
        /* Restart the idle timer */
        pConn->tIdle = t + s_uIdleTimeout;

        /* Ready for output */
        (*pConn->pfnOutput)( pConn);
        }

      if ( t > pConn->tIdle)
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

        /* Disable non-blocking mode */
        setsockasync( pConn->s, 0);

        SessionFinish( pConn);
        closesocket( pConn->s);

        /* Return it to the free list */
        DequeRemove( &pConn->list);
        DequeAppend( &s_free, &pConn->list);
        }
      }
    }
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
void SessionStart( STelnetConnection* pConn)
  {
  /* Initial options */
  static const unsigned char options[] = {
    kTelnetIAC, kTelnetWill, kOptionEcho, /* WILL echo */
    kTelnetIAC, kTelnetWill, kOptionNoGoAhead  /* WILL suppress go-ahead */
  };
  /* Greeting */
  static const char kszGreeting[] = kszBanner "." CRLF CRLF;

  strcpy( pConn->input, "OPEN");
  pConn->pszTail = "";
#if TELNETD_EXT == TELNETD_EXT_POSIX || TELNETD_EXT == TELNETD_EXT_SYSTEM
  pConn->cstderr = -1;
  pConn->outputFile = NULL;
  pConn->sizePending = 0;
#endif

  pConn->binaryEnable = 0;
  pConn->echoEnable = 0;
  pConn->outputDisable = 0;
  pConn->goAheadDisable = 0;
  pConn->goAheadSent = 0;
  pConn->iInterrupt = 0;

  pConn->eState = kStateUser;
  strcpy( pConn->szPrompt, kszLogin);
  pConn->iAttempts = 0;
  pConn->ePermit = kPermitNone;
  pConn->szUser[ 0] = cNul;
  pConn->szHome[ 0] = cNul;

  /* Send initial options and greeting */
  WriteRaw( pConn, options, sizeof( options) );
  WriteRaw( pConn, kszGreeting, sizeof( kszGreeting) - 1);

  /* Issue a prompt */
  TelnetPrompt( pConn, 0);
  }


/*
 * End a session
 */
void SessionFinish( STelnetConnection* pConn)
  {
  Log( kLogInfo, &pConn->client, "CLOSE\n");

  /* Send FIN */
  shutdown( pConn->s, SD_SEND);

#if TELNETD_EXT == TELNETD_EXT_POSIX || TELNETD_EXT == TELNETD_EXT_SYSTEM
  ExternalFinal( pConn);
#endif
  }


/*
 * Abort a session
 */
void SessionAbort( STelnetConnection* pConn)
  {
  struct linger lgr;

  /* Reset the connection when closed */
  lgr.l_onoff = 1;
  lgr.l_linger = 0;
  setsockopt( pConn->s, SOL_SOCKET, SO_LINGER, (char*)&lgr, sizeof( lgr));

  pConn->eState = kStateQuit;
  }


/*
 * Session input available
 */
void SessionInput( STelnetConnection* pConn)
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
    switch ( pConn->eState)
      {
    case kStateUser:
      iRet = TelnetUser( pConn, pConn->input);
      break;

    case kStatePassword:
      iRet = TelnetPassword( pConn, pConn->input);
      break;

    case kStateInterpret:
      /* Execute the request */
      iRet = ExecuteRequest( pConn, pConn->input);
      break;

    case kStateQuit:
      return;
      }

    /* Issue a prompt */
    if ( 0 == pConn->pfnOutput)
      TelnetPrompt( pConn, iRet);
    }
  }


/*
 * Login a user
 */
int TelnetUser(
  STelnetConnection* pConn,
  char* pszUser
) {
  /* Save supplied username */
  strncpy( pConn->szUser, pszUser, sizeof( pConn->szUser) );

  strcpy( pConn->input, "USER");
  pConn->pszTail = pConn->szUser;

  /* Validate user */
  pConn->ePermit = ValidateUser( pConn, NULL);
  if ( kPermitNone != pConn->ePermit)
    {
    /* No password needed for this account */
    pConn->eState = kStateInterpret;
    TelnetWelcome( pConn);
    }
  else
    {
    /* Disable echo */
    pConn->echoEnableSave = pConn->echoEnable;
    pConn->echoEnable = 0;

    /* Prompt for password */
    strcpy( pConn->szPrompt, kszPassword);
    pConn->eState = kStatePassword;
    }

  return kStatePassword == pConn->eState ? 1 : 0;
  }


/*
 * Validate password
 */
int TelnetPassword(
  STelnetConnection* pConn,
  char* pszPassword
) {
  int iRet;

  /* Restore echo */
  pConn->echoEnable = pConn->echoEnableSave;

  /* Verify user & password */
  pConn->ePermit = ValidateUser( pConn, pszPassword);
  if ( kPermitNone != pConn->ePermit)
    {
    /* Login OK */
    pConn->eState = kStateInterpret;
    iRet = 0;
    }
  else if ( ++pConn->iAttempts < MAX_ATTEMPTS)
    {
    /* Login failed */
    pConn->eState = kStateUser;
    strcpy( pConn->szPrompt, kszLogin);
    iRet = 1;
    }
  else
    {
    pConn->eState = kStateQuit;
    iRet = 2;
    }

  strcpy( pConn->input, "PASS");
  pConn->pszTail = "";

  TelnetWelcome( pConn);

  return iRet;
  }


/*
 * Setup for new user
 */
void TelnetWelcome( STelnetConnection* pConn)
  {
  char* pszBuf = pConn->buffer;
  int count = 0;
  const char* pszUser;

  if ( cNul != pConn->szUser[0])
    pszUser = pConn->szUser;
  else
    pszUser = "bashful";

  switch ( pConn->ePermit)
    {
  case kPermitNone:
    count = sprintf( pszBuf, CRLF "Login failed" CRLF CRLF);
    break;

  case kPermitRead:
    count = sprintf( pszBuf, CRLF "Welcome %s. Read only access." CRLF CRLF,
      pszUser);
    break;

  case kPermitWrite:
    count = sprintf( pszBuf, CRLF "Welcome %s. Restrictions apply." CRLF CRLF,
      pszUser);
    break;

  case kPermitChange:
    count = sprintf( pszBuf, CRLF "Welcome %s." CRLF CRLF, pszUser);
    break;
    }
  WriteRaw( pConn, pszBuf, count );
  }


/*
 * Execute a command
 */
int ExecuteRequest(
  STelnetConnection* pConn,
  char* pszCmd
) {
  char *pszTail;
  const STelnetCommand* pCmd;
  int iRet;
  char szCwd[ MAX_PATH];

  /* Set user's home directory */
  PlatformChdir( pConn->szHome, szCwd, sizeof szCwd);

  /* Skip leading space */
  while ( isspace( *pszCmd))
    ++pszCmd;

  /* Break into command name + tail */
  pszTail = pszCmd;
  while ( cNul != *pszTail && !isspace( *pszTail))
    {
    ++pszTail;
    }
  if ( cNul != *pszTail)
    {
    *(pszTail++) = cNul;
    while ( cNul != *pszTail && isspace( *pszTail))
      ++pszTail;
    }
  pConn->pszTail = pszTail;

  /* Find the command */
  if ( NULL != (pCmd = TelnetFindCommand( pszCmd, s_pCmds, s_cmds))
    || NULL != (pCmd = TelnetFindCommand( pszCmd, builtin, lengthof( builtin)))
  ) {
    /* Execute the command */
    iRet = (*pCmd->pfnExecute)( pConn, pszTail );
    }
  else if ( kPermitChange == pConn->ePermit)
    {
    /* Concat tail onto command */
    pConn->pszTail = "";
    do
      --pszTail;
    while ( isspace( *pszTail));
    if ( cNul == *pszTail)
      *pszTail = ' ';

    /* Execute external command */
    iRet = ExternalCommand( pConn, pszCmd);
    }
  else
    {
    static const char kszPrivilege[] = "Insufficient privilege." CRLF;
    WriteRaw( pConn, kszPrivilege, sizeof( kszPrivilege) - 1);
    iRet = 3;
    }

  /* Restore cwd */
  PlatformChdir( szCwd, NULL, 0);

  return iRet;
  }


/*
 * Find a command descriptor given a string
 */
const STelnetCommand* TelnetFindCommand(
  const char* pszCmd,
  const STelnetCommand pCmds [],
  size_t cmds
) {
  size_t i;

  /* Find the command */
  for ( i = 0; i < cmds; ++i, ++pCmds)
    {
    if ( !SWS_stricmp( pszCmd, pCmds->pszName) )
      return pCmds;
    }

  return NULL;
  }


/*
 * Issue a prompt
 */
void TelnetPrompt(
  STelnetConnection* pConn,
  int iRet
) {
  const char* pszSep;

  if ( kStateQuit != pConn->eState)
    {
    /* Set blocking mode */
    setsockasync( pConn->s, 0);

    WriteRaw( pConn, pConn->szPrompt, strlen( pConn->szPrompt) );

    /* Restore non-blocking mode */
    setsockasync( pConn->s, 1);
    }

  /* Log the last request */
  pszSep = cNul != pConn->pszTail[ 0] ? " " : "";
  Log( !iRet ? kLogInfo : kLogWarn, &pConn->client, "%s%s%s ; %d\n",
    pConn->input, pszSep, pConn->pszTail, iRet
  );
  }


/****************************************************************************
 * Builtin commands
 ****************************************************************************/
/*
 * quit command
 */
static int QuitCommand(
  STelnetConnection* pConn,
  const char* pszTail
) {
  (void)pszTail;

  pConn->eState = kStateQuit;
  return 0;
  }


/*
 * noop command
 */
static int NoopCommand(
  STelnetConnection* pConn,
  const char* pszTail
) {
  (void)pConn;
  (void)pszTail;

  return 0;
  }


/*
 * cd command
 */
static int ChdirCommand(
  STelnetConnection* pConn,
  const char* pszTail
) {
  int iRet;

  if ( cNul == *pszTail)
    {
    strcpy( pConn->buffer, pConn->szHome);
    strcat( pConn->buffer, CRLF);
    WriteRaw( pConn, pConn->buffer, strlen( pConn->buffer));
    iRet = 0;
    }
  else if ( kPermitChange != pConn->ePermit)
    {
    static const char kszPrivilege[] = "Insufficient privilege." CRLF;
    WriteRaw( pConn, kszPrivilege, sizeof( kszPrivilege) - 1);
    iRet = 3;
    }
  else if ( 0 == PlatformChdir( pszTail, NULL, 0))
    {
    PlatformChdir( NULL, pConn->szHome, sizeof pConn->szHome);
    iRet = 0;
    }
  else
    {
    static const char kszPrivilege[] = "Invalid path." CRLF;
    WriteRaw( pConn, kszPrivilege, sizeof( kszPrivilege) - 1);
    iRet = 3;
    }

  return iRet;
  }


/*
 * ipconfig command
 */
static int IpconfigCommand(
  STelnetConnection* pConn,
  const char* pszTail
) {
  char* const pszBuf = pConn->buffer;
  char szHost[ MAX_FQDN];
  struct hostent* pHost;
  int iRet;

  if ( cNul != *pszTail)
    {
    static const char kszInvalid[] = "Invalid parameter." CRLF;
    WriteRaw( pConn, kszInvalid, sizeof( kszInvalid) - 1);
    iRet = 1;
    }
  else if ( SOCKET_ERROR != gethostname( szHost, sizeof(szHost))
    && NULL != (pHost = gethostbyname( szHost))
  ) {
    int len, i;

    len = sprintf( pszBuf, "Hostname: %s [", szHost);

    /* Display the list of local IP addresses */
    for ( i = 0; NULL != pHost->h_addr_list[ i]; ++i)
      {
      len += sprintf(
        pszBuf + len,
        " %s",
        inet_ntoa( *(const struct in_addr*)pHost->h_addr_list[ i] )
      );
      }

    len += sprintf( pszBuf + len, "]" CRLF);
    WriteRaw( pConn, pszBuf, len);

#if SWS_USE_SOCKLIB
    {
    const unsigned interfaces = SWS_DatalinkCount();
    unsigned u;

    /* Display properties for all IP interfaces */
    for ( u = 0; u < interfaces; ++u)
      {
      ui32 ip, mask, gateway;
      struct in_addr addr;
      size_t size;

      size = sizeof( szHost);
      SWS_DatalinkGetProperty(
        u,
        SWS_kDatalinkProperty_Ident,
        szHost,
        &size
      );
      len = sprintf( pszBuf, CRLF "Adapter: %s" CRLF, szHost);
      WriteRaw( pConn, pszBuf, len);

      size = sizeof( ip);
      SWS_DatalinkGetProperty(
        u,
        SWS_kDatalinkProperty_Address,
        &ip,
        &size
      );
      addr.s_addr = htonl( ip);
      len = sprintf( pszBuf, "  IP address %s" CRLF, inet_ntoa( addr) );
      WriteRaw( pConn, pszBuf, len);

      size = sizeof( mask);
      SWS_DatalinkGetProperty(
        u,
        SWS_kDatalinkProperty_SubnetMask,
        &mask,
        &size
      );
      addr.s_addr = htonl( mask);
      len = sprintf( pszBuf, "  Subnet mask %s" CRLF, inet_ntoa( addr) );
      WriteRaw( pConn, pszBuf, len);

      size = sizeof( gateway);
      SWS_DatalinkGetProperty(
        u,
        SWS_kDatalinkProperty_Gateway,
        &gateway,
        &size
      );
      addr.s_addr = htonl( gateway);
      len = sprintf( pszBuf, "  Default gateway %s" CRLF, inet_ntoa( addr) );
      WriteRaw( pConn, pszBuf, len);
      }
    }
#endif

    iRet = 0;
    }
  else
    {
    static const char kszNoHost[] = "Unknown host name." CRLF;
    WriteRaw( pConn, kszNoHost, sizeof( kszNoHost) - 1);
    iRet = 1;
    }

  return iRet;
  }


/*
 * help command
 */
static int HelpCommand(
  STelnetConnection* pConn,
  const char* pszTail
) {
  const STelnetCommand* pCmd;
  int iRet;
  char* const pszBuf = pConn->buffer;

  if ( '\0' != *pszTail)
    {
    if ( NULL != (pCmd = TelnetFindCommand( pszTail, s_pCmds, s_cmds))
      || NULL != (pCmd = TelnetFindCommand( pszTail, builtin, lengthof( builtin) ))
    ) {
      WriteRaw( pConn, pszBuf, sprintf( pszBuf, "%-9.9s %s" CRLF,
          pCmd->pszName,
          NULL != pCmd->pszHelp ? pCmd->pszHelp : ""
      ) );
      iRet = 0;
      }
    else
      {
      static const char kszNotFound[] = "No such command" CRLF;
      WriteRaw( pConn, kszNotFound, sizeof( kszNotFound) - 1);
      iRet = 1;
      }
    }
  else
    {
    static const char kszHelp[] = "Builtin commands:" CRLF CRLF;
    size_t i;

    WriteRaw( pConn, kszHelp, sizeof( kszHelp) - 1 );

    for ( i = 0, pCmd = s_pCmds; i < s_cmds; ++i, ++pCmd)
      {
      if ( NULL != pCmd->pszHelp)
        {
        WriteRaw( pConn, pszBuf, sprintf( pszBuf, "%-9.9s %s" CRLF,
          pCmd->pszName, pCmd->pszHelp) );
        }
      }

    for ( i = 0, pCmd = builtin; i < lengthof( builtin); ++i, ++pCmd)
      {
      if ( NULL != pCmd->pszHelp)
        {
        WriteRaw( pConn, pszBuf, sprintf( pszBuf, "%-9.9s %s" CRLF,
          pCmd->pszName, pCmd->pszHelp) );
        }
      }

    iRet = 0;
    }

  return iRet;
  }


/****************************************************************************
 * External commands
 ****************************************************************************/

/*
 * Execute an external command
 */
static int ExternalCommand(
  STelnetConnection* pConn,
  char* pszCmd
) {
  int iRet;

#if TELNETD_EXT == TELNETD_EXT_POSIX
  const char* pszErr = NULL;

  pConn->cstderr = dup( fileno( stderr) );
  if ( -1 != pConn->cstderr )
    {
    /* Redirect stderr to a file */
    if ( NULL != UniqueName( pConn->szTmpFile)
      && NULL != freopen( pConn->szTmpFile, "w+", stderr)
    ) {
      /* Execute an external command and pipe the output.*/
      pConn->outputFile = popen( pszCmd, "r");
      if ( NULL != pConn->outputFile)
        {
        /* Request callback when ready for output */
        pConn->pfnOutput = &ExternalStdout;
        pConn->sizePending = 0;
        iRet = 0;
        }
      else
        {
        /* popen failed so cleanup */
        pszErr = strerror( errno);
        ExternalFinal( pConn);
        iRet = 1;
        }
      }
    else
      {
      pszErr = strerror( errno);
      ExternalFinal( pConn);
      iRet = 1;
      }
    }
  else
    {
    pszErr = strerror( errno);
    iRet = 1;
    }

  if ( NULL != pszErr)
    {
    size_t count = (size_t)sprintf( pConn->buffer, "%s" CRLF, pszErr);
    WriteRaw( pConn, pConn->buffer, count );
    Log( kLogWarn, &pConn->client, "WARN: ExternalCommand: %s\n", pszErr);
    }

#elif TELNETD_EXT == TELNETD_EXT_SYSTEM
  const char* pszErr = NULL;

  /* Redirect stdout */
  strcpy( pConn->buffer, pszCmd );
  if ( NULL != UniqueName( pConn->szTmpFile))
    {
    strcat( pConn->buffer, TELNETD_STDOUT );
    strcat( pConn->buffer, pConn->szTmpFile);
#ifdef TELNETD_STDERR
    /* Redirect stderr to stdout */
    strcat( pConn->buffer, TELNETD_STDERR );
#endif
    }
  else
    pszErr = strerror( errno);

  /* Execute an external command */
  iRet = system( pConn->buffer);
  if ( -1 != iRet)
    {
    /* Open the redirected output file */
    pConn->outputFile = fopen( pConn->szTmpFile, "r");
    if ( NULL != pConn->outputFile )
      {
      /* Request callback when ready for output */
      pConn->pfnOutput = &ExternalStdout;
      pConn->sizePending = 0;
      iRet = 0;
      }
    else
      pszErr = strerror( errno);
    }
  else
    pszErr = strerror( errno);

  if ( NULL != pszErr)
    {
    size_t count = (size_t)sprintf( pConn->buffer, "%s" CRLF, pszErr);
    WriteRaw( pConn, pConn->buffer, count );
    Log( kLogWarn, &pConn->client, "WARN: ExternalCommand: %s\n", pszErr);
    }

#else
  {
  static const char kszError[] = "Unrecognised internal command.  Try help" CRLF;
  (void)pszCmd;

  WriteRaw( pConn, kszError, sizeof kszError - 1 );
  iRet = 2;
  }
#endif

  return iRet;
  }


#if TELNETD_EXT == TELNETD_EXT_POSIX || TELNETD_EXT == TELNETD_EXT_SYSTEM
/*
 * Ready for output from external command
 */
void ExternalStdout( STelnetConnection* pConn)
  {
  char* pBuf;
  size_t count = 0;

  if ( 0 != pConn->sizePending)
    {
    count = pConn->sizePending;
    pBuf = pConn->pPending;
    }
#if TELNETD_EXT == TELNETD_EXT_POSIX
  /* Piped output available? */
  else if ( NULL == pConn->outputFile)
    {
    /* No, get any redirected stderr output */
    pBuf = fgets( pConn->buffer, sizeof( pConn->buffer) - 2, stderr);
    }
  else if ( feof( pConn->outputFile))
    {
    /* Close the pipe */
    if ( -1 == pclose( pConn->outputFile))
      Log( kLogWarn, &pConn->client, "WARN: ExternalStdout pclose: %s\n", strerror( errno));
    pConn->outputFile = NULL;

    /* Get any redirected stderr output */
    fflush( stderr);
    rewind( stderr);
    pBuf = fgets( pConn->buffer, sizeof( pConn->buffer) - 2, stderr);
    }
#endif
  else
    {
    /* Get redirected output */
    pBuf = fgets( pConn->buffer, sizeof( pConn->buffer) - 2, pConn->outputFile);
    }

  if ( 0 == count
    && NULL != pBuf
    && 0 != (count = strlen( pBuf))
  ) {
    /* Convert newline to CRLF */
    if ( cLinefeed == pBuf[ count -1] )
      {
      strcpy( &pBuf[ count - 1], CRLF);
      ++count;
      }
    }

  if ( 0 != count)
    {
    int sent = WriteRaw( pConn, pBuf, count);
    if ( SOCKET_ERROR == sent)
      {
      count = 0;
      }
    else if ( (size_t)sent < count)
      {
      pConn->sizePending = count - (size_t)sent;
      pConn->pPending = pBuf + (size_t)sent;
      }
    }

  if ( 0 == count
#if TELNETD_EXT == TELNETD_EXT_POSIX
    && NULL == pConn->outputFile
#endif
  ) {
    ExternalFinal( pConn);
    TelnetPrompt( pConn, 0);
    }
  }


void ExternalFinal( STelnetConnection* pConn)
  {
  if ( NULL != pConn->outputFile)
    {
#if TELNETD_EXT == TELNETD_EXT_POSIX
    if ( -1 == pclose( pConn->outputFile))
      Log( kLogWarn, &pConn->client, "WARN: ExternalFinal pclose: %s\n", strerror( errno));
#elif TELNETD_EXT == TELNETD_EXT_SYSTEM
    if ( fclose( pConn->outputFile))
      Log( kLogWarn, &pConn->client, "WARN: ExternalFinal fclose( output): %s\n",
         strerror( errno)
      );
    if ( remove( pConn->szTmpFile))
      Log( kLogWarn, &pConn->client, "WARN: ExternalFinal remove( %s): %s\n",
        pConn->szTmpFile, strerror( errno)
      );
#endif
    pConn->outputFile = NULL;
    }

#if TELNETD_EXT == TELNETD_EXT_POSIX
  if ( -1 != pConn->cstderr )
    {
    /* Restore stderr. This also closes the associated file */
    if ( -1 == dup2( pConn->cstderr, fileno( stderr) ))
      Log( kLogWarn, &pConn->client, "WARN: ExternalFinal dup2: %s\n", strerror( errno));
    pConn->cstderr = -1;

    /* Ensure stderr is unbuffered */
    setbuf( stderr, NULL);
    rewind( stderr);

    if ( remove( pConn->szTmpFile))
      Log( kLogWarn, &pConn->client, "WARN: ExternalFinal remove( %s): %s\n",
        pConn->szTmpFile, strerror( errno)
      );
    }
#endif

  pConn->pfnOutput = 0;
  }
#endif


/****************************************************************************
 * Support
 ****************************************************************************/
/*
 * Add an item to the tail of a deque
 */
static void DequeAppend( SDeque* const pDeque, SDeque* const pItem)
  {
  pItem->tail = pDeque;
  pItem->head = pDeque->head;

  pDeque->head->tail = pItem;
  pDeque->head = pItem;
  }


/*
 * Remove an item from a deque
 */
static SDeque* DequeRemove( SDeque* const pItem)
  {
  pItem->head->tail = pItem->tail;
  pItem->tail->head = pItem->head;

  DEQUE_INIT( pItem);

  return pItem;
  }


/*
 * Validate a user
 */
static SWS_FCfgCallback EnumUserCallback;
typedef struct SUserInfo
  {
  const char* pszUser;
  char* pszHome;
  char* pszPrompt;
  char szPassword[ 32];
  EPermission ePermission;
  in_addr_t net; /* Permitted hosts */
  unsigned uCidr; /* Classless Internet Domain Routing */
  } SUserInfo;

static EPermission ValidateUser(
  STelnetConnection* pConn,
  const char* pszPassword
) {
  SUserInfo info;

  info.pszUser = pConn->szUser;
  info.pszHome = pConn->szHome;
  info.pszPrompt = pConn->szPrompt;
  info.net = INADDR_ANY;
  info.uCidr = 32;

  if ( SWS_CfgEnum( kszTelnetUsersSect, &EnumUserCallback, &info) > 0)
    {
    if ( INADDR_ANY != info.net)
      {
      /* Verify client is a permitted host or sub-net */
      ui32 host = ntohl( pConn->client.sin_addr.s_addr);
      if ( INADDR_LOOPBACK != host)
        {
        ui32 ulNet = ntohl( info.net);
        ui32 ulMask = 0xffffffffL << (32 - info.uCidr);
        if ( (host & ulMask) != (ulNet & ulMask) )
          return kPermitNone;
        }
      }

    /* Password required? */
    if ( 0 == strcmp( "-", info.szPassword))
      return info.ePermission; /* No */

    if ( NULL == pszPassword)
      return kPermitNone;

    /* Wildcard password? */
    if ( 0 == strcmp( "*", info.szPassword))
      return info.ePermission; /* Yes */

    /* Validate password */
    if ( 0 == strcmp( info.szPassword, pszPassword) )
      return info.ePermission;
    }

  return kPermitNone;
  }

static SWS_CB( int, EnumUserCallback( void* pv, const char* pszVal))
  {
  SUserInfo* pInfo = (SUserInfo*)pv;
  char szName[ MAX_USERNAME], szPermission[ 4], szNet[ 16];

  szNet[0] = cNul;

  /* Parse username, password, home, prompt and permission */
  { STATIC_ASSERT( sizeof szName > 31);
    STATIC_ASSERT( sizeof pInfo->szPassword > 31);
    STATIC_ASSERT( MAX_PATH > 255);
    STATIC_ASSERT( sizeof szPermission > 3);
    STATIC_ASSERT( sizeof szNet > 15);
  }
  if ( 4 <= sscanf( pszVal, " %31s %31s %255s %3s %15[0123456789.] / %u",
      szName, pInfo->szPassword, pInfo->pszHome, szPermission, szNet, &pInfo->uCidr)
    && ( 0 == SWS_stricmp( pInfo->pszUser, szName)
      || 0 == strcmp( "*", szName)
    )
  ) {
    if ( 0 == SWS_stricmp( "r", szPermission))
      {
      pInfo->ePermission = kPermitRead;
      strcpy( pInfo->pszPrompt, kszPromptR);
      }
    else if ( 0 == SWS_stricmp( "w", szPermission))
      {
      pInfo->ePermission = kPermitWrite;
      strcpy( pInfo->pszPrompt, kszPromptW);
      }
    else if ( 0 == SWS_stricmp( "c", szPermission))
      {
      pInfo->ePermission = kPermitChange;
      strcpy( pInfo->pszPrompt, kszPromptC);
      }
    else
      {
      pInfo->ePermission = kPermitNone;
      strcpy( pInfo->pszPrompt, "? ");
      }

    strcat( pInfo->pszHome, kszPathSep);

    if ( cNul != szNet[ 0])
      pInfo->net = inet_addr( szNet);

    return 1;
    }

  return 0;
  }


/****************************************************************************
 * Telnet console I/O
 ****************************************************************************/
/*
 * Read a line
 */
int TelnetReadLine(
  STelnetConnection* pConn
) {
  int iRet;

  if ( !pConn->goAheadDisable
    && !pConn->goAheadSent
  ) {
    static const unsigned char goAhead[] = { kTelnetIAC, 249};

    /* Send Go Ahead */
    pConn->goAheadSent = 1;
    WriteRaw( pConn, goAhead, sizeof( goAhead) );
    }

  while ( pConn->telnetCount < sizeof(pConn->input) - 1 )
    {
    static const char eraseChar[] = {'\b',' ','\b'};
    unsigned char c;

    iRet = TelnetRead( pConn, &c, sizeof c);
    if ( iRet <= 0)
      return iRet; /* Connection closed or no data */

    if ( pConn->binaryEnable)
      {
      pConn->input[ pConn->telnetCount++] = (char)c;
      if ( pConn->echoEnable)
        WriteRaw( pConn, &c, 1);
      }
    else if ( pConn->telnetCR)
      {
      pConn->telnetCR = 0;
      switch ( c)
        {
      case cNul:
      case cLinefeed:
      case cReturn: /* For apps that don't send CRLF */
        if ( pConn->echoEnable)
          WriteRaw( pConn, CRLF, 2 );

        goto done;

      default:
        if ( pConn->telnetCount < sizeof(pConn->input) - 3)
          {
          pConn->input[ pConn->telnetCount++] = cReturn;
          pConn->input[ pConn->telnetCount++] = (char)c;
          }
        if ( pConn->echoEnable)
          {
          char buf[3] = CRLF;
          buf[ 2] = c;
          WriteRaw( pConn, buf, 3 );
          }
        break;
        }
      }
    else switch ( c)
      {
    default:
      if ( pConn->telnetCount < sizeof(pConn->input) - 2
        && isprint(c)
      ) {
        pConn->input[ pConn->telnetCount++] = (char)c;
        if ( pConn->echoEnable )
          WriteRaw( pConn, &c, 1);
        }
      break;

    case 7:
    case 9:
    case 11:
    case 12:
      if ( pConn->echoEnable)
        WriteRaw( pConn, &c, 1);
      break;

    case 8:
      if ( pConn->telnetCount > 0)
        {
        --(pConn->telnetCount);
        if ( pConn->echoEnable)
          WriteRaw( pConn, eraseChar, sizeof eraseChar);
        }
      break;

    case cLinefeed:
      if ( pConn->echoEnable)
        WriteRaw( pConn, CRLF, 2 );

      goto done;

    case cReturn:
      pConn->telnetCR = 1;
      break;
      }
    }

done:
  pConn->input[ iRet = pConn->telnetCount] = cNul;
  pConn->telnetCount = 0;
  pConn->telnetCR = 0;
  pConn->goAheadSent = 0;

  return iRet + 1;
  }


/*
 * Get a buffer from the client
 */
int TelnetRead(
  STelnetConnection* pConn,
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
      break; /* No data */

    /* Parse received data for telnet commands */
    pBuf = (unsigned char*)pvBuf;
    while (  iRet--)
      {
      static const unsigned char s_sync[] = { kTelnetIAC, 242};
      static const char ayt[] = {'!'};
      unsigned char c, buf[3];

      c = *pBuf++;
      switch ( pConn->eTelnetState)
        {
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
        pConn->eTelnetState = kTelnetData;
        switch ( c)
          {
        default:
        case 240: /* End of subnegotiation */
        case 241: /* NOP */
        case 242: /* Data mark (sync) */
        case 243: /* Break */
          break;

        case 244: /* Interrupt Process */
          pConn->iInterrupt = 1;
          break;

        case 245: /* Abort Output */
          /* Send sync (OOB data mark) */
          WriteRaw( pConn, &s_sync, sizeof(s_sync) );

          /* Allow current process to run to completion but dump the output */
          pConn->outputDisable = 1;
          break;

        case 246: /* Are You There */
          WriteRaw( pConn, ayt, sizeof(ayt));
          break;

        case 247: /* Erase Character */
          if ( !pConn->binaryEnable)
            ((unsigned char*)pvBuf)[ len++] = 8;
          break;

        case 248: /* Erase Line */
          break;

        case 249: /* Go ahead */
          break;

        case kTelnetBeginSub:/* Begin subnegotiation */
        case kTelnetWill:
        case kTelnetWont:
        case kTelnetDo:
        case kTelnetDont:
          pConn->eTelnetState = (ETelnetState)c;
          break;

        case kTelnetIAC:
          ((unsigned char*)pvBuf)[ len++] = kTelnetIAC;
          break;
          }
        break;

      case kTelnetBeginSub: /* Begin subnegotiation */
        if ( 240 == c) /* End of subnegotiation */
          pConn->eTelnetState = kTelnetData;
        break;

      case kTelnetWill:
        pConn->eTelnetState = kTelnetData;
        buf[0] = kTelnetIAC;
        buf[2] = c;
        switch ( c)
          {
        case kOptionEcho: /* echo */
          if ( !pConn->echoEnable)
            {
            pConn->echoEnable = 1;
            buf[1] = kTelnetDo;
            WriteRaw( pConn, buf, 3);
            }
          break;

        case kOptionNoGoAhead: /* suppress go-ahead */
          if ( !pConn->goAheadDisable)
            {
            pConn->goAheadDisable = 1;
            buf[1] = kTelnetDo;
            WriteRaw( pConn, buf, 3);
            }
          break;

        default:
          Log( kLogWarn, &pConn->client, "WARN: Unsupported Will %#x\n", c);
          buf[1] = kTelnetDont;
          WriteRaw( pConn, buf, 3);
          break;
          }
        break;

      case kTelnetWont:
        pConn->eTelnetState = kTelnetData;
        break;

      case kTelnetDo:
        pConn->eTelnetState = kTelnetData;

        buf[0] = kTelnetIAC;
        buf[2] = c;
        switch ( c)
          {
        case kOptionBinary: /* binary */
          if ( !pConn->binaryEnable)
            {
            pConn->binaryEnable = 1;
            buf[1] = kTelnetWill;
            WriteRaw( pConn, buf, 3);
            }
          break;

        case kOptionEcho: /* echo */
          if ( !pConn->echoEnable)
            {
            pConn->echoEnable = 1;
            buf[1] = kTelnetWill;
            WriteRaw( pConn, buf, 3);
            }
          break;

        case kOptionNoGoAhead: /* suppress go-ahead */
          if ( !pConn->goAheadDisable)
            {
            pConn->goAheadDisable = 1;
            buf[1] = kTelnetWill;
            WriteRaw( pConn, buf, 3);
            }
          break;

        default:
          /* Send won't */
          buf[1] = kTelnetWont;
          WriteRaw( pConn, buf, 3);
          break;
          }
        break;

      case kTelnetDont:
        pConn->eTelnetState = kTelnetData;
        break;
        }
      }
    }

  return (int)len;
  }


/*
 * Send a buffer to the telnet client
 */
int TelnetWrite(
  STelnetConnection* pConn,
  const void* pv,
  size_t size
) {
  const unsigned char* pBuf = (const unsigned char*)pv;
  size_t count = 0;
  int iRet = 0;

  if ( pConn->outputDisable)
    return 0;

  /* Set blocking mode */
  setsockasync( pConn->s, 0);

  while ( size-- > 0)
    {
    static const unsigned char escape[] = { kTelnetIAC, 255};
    static const char crlf[] = { cReturn, cLinefeed};

    switch ( pBuf[ count++] )
      {
    default:
      break;

    case cLinefeed:
      if ( !pConn->binaryEnable)
        {
        iRet = WriteRaw( pConn, pBuf, count - 1);
        if ( SOCKET_ERROR == iRet)
          goto done;

        /* Convert newline to CR LF */
        iRet = WriteRaw( pConn, crlf, sizeof( crlf) );
        if ( SOCKET_ERROR == iRet)
          goto done;

        pBuf += count;
        count = 0;
        }
      break;

    case kTelnetIAC:
      iRet = WriteRaw( pConn, pBuf, count - 1);
      if ( SOCKET_ERROR == iRet)
        goto done;

      /* Escape data that matches the IAC character */
      iRet = WriteRaw( pConn, escape, sizeof( escape) );
      if ( SOCKET_ERROR == iRet)
        goto done;

      pBuf += count;
      count = 0;
      break;
      }
    }

  if ( count)
    iRet = WriteRaw( pConn, pBuf, count);

done:
  /* Restore non-blocking mode */
  setsockasync( pConn->s, 1);

  return iRet;
  }


/****************************************************************************
 * Network I/O
 ****************************************************************************/

/*
 * Write raw data to the client
 */
static int WriteRaw(
  STelnetConnection* pConn,
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
      Log( kLogWarn, &pConn->client, "WARN %d: Connection reset in write\n", err);
      SessionAbort( pConn);
      break;
    case ECONNABORTED:
      Log( kLogWarn, &pConn->client, "WARN %d: Connection aborted in write\n", err);
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
  STelnetConnection* pConn,
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
    return SOCKET_ERROR;
    }

  return iRet;
  }


#if TELNETD_EXT == TELNETD_EXT_POSIX || TELNETD_EXT == TELNETD_EXT_SYSTEM
/*
 * Generate a unique filename
 */
static char* UniqueName( char szBuf[ L_UNIQUE] )
  {
  static unsigned long s_u;
  long i;
  const char* pszPath;

  if ( ( NULL == (pszPath = getenv( "TMPDIR"))
      && NULL == (pszPath = getenv( "KDETMP"))
      && NULL == (pszPath = getenv( "TMP"))
      && NULL == (pszPath = getenv( "TEMP")) )
    || strlen( pszPath) + 10 > L_UNIQUE
  )
    pszPath = ".";

  for ( i = 0; i < 1000000L; ++i)
    {
    FILE* f;

    sprintf( szBuf, "%s/ZT%06lu", pszPath, s_u++);
    f = fopen( szBuf, "r");
    if ( NULL == f)
      return szBuf;

    fclose( f);
    }
  errno = ENFILE;
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
 * Platform specific code
 ****************************************************************************/

#if defined WIN32 || defined _WIN32 || defined __WIN32__ || defined WIN16 \
 || defined _MSDOS || defined _DOS || defined _DOS16
/****************************************************************************
 * Win32/16 & MSDOS
 ****************************************************************************/

#include <direct.h>

#ifdef __WATCOMC__
#define _chdir chdir
#endif


/*
 * Change working directory
 */
int PlatformChdir(
  const char* pszPath,
  char* pszCwd,
  size_t size
) {
  if ( NULL != pszCwd)
    _getcwd( pszCwd, size);
  return NULL != pszPath ? _chdir( pszPath) : 0;
  }

#elif defined _POSIX_SOURCE || defined _POSIX_C_SOURCE || defined __DJGPP__
/****************************************************************************
 * POSIX
 ****************************************************************************/

#include <unistd.h>

/*
 * Change working directory
 */
int PlatformChdir(
  const char* pszPath,
  char* pszCwd,
  size_t size
) {
  if ( NULL != pszCwd && NULL == getcwd( pszCwd, size))
    return -1;
  return NULL != pszPath ? chdir( pszPath) : 0;
  }

#else

int PlatformChdir( const char* pszPath, char* pszCwd, size_t size)
  {
  (void)pszPath;
  (void)pszCwd;
  (void)size;
  {int WARN_PlatformChdir_STUB;}
  return -1;
  }

#endif

/* End of file */
