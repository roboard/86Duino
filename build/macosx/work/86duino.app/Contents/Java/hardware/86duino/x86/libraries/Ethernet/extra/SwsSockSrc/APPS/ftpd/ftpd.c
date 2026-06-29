/*****************************************************************************
 *
 * File name   : ftpd.c
 * Function    : FTP server per RFC 959
 * Authors     : Lawrence Rust
 * Systems     : ANSI C, sockets
 *
 *****************************************************************************
 * File History
 * ------------
 *
 * 12-Nov-02, LVR, Created.
 * 13-Aug-09, LVR, v1.02 - Add SIZE and catch SIGPIPE
 * 15-Sep-09, LVR, v1.03 - Add CHMOD and MDTM
 * 23-Dec-09, LVR, v1.04 - Added Log()
 * 22-Jan-10, LVR, v1.05 - Added kPermitExplore 'x' user permission to cd
 * 30-Aug-10, LVR, v1.06 - Added FEAT method
 * 30-Jan-11, LVR, v1.07 - Added max login attemps and IP lockout
 *
 *
 * Comments
 * --------
 *
 *****************************************************************************/
#define kszApp "Software Systems FTP service"
#define kszVer STRING( FTPD_VER_MAJOR) "." STRING( FTPD_VER_MINOR)
#define kszBanner kszApp " " kszVer " (" SWS_NET_ID ")"

/* Compilation options */
#ifndef OPT_MAX_CONNECTIONS
#define OPT_MAX_CONNECTIONS 4U
#endif
#if OPT_MAX_CONNECTIONS > 31
#define FD_SETSIZE (1 + 2 * OPT_MAX_CONNECTIONS)
#endif

#ifndef OPT_UPNP
# define OPT_UPNP 0        /* !0 to export service to UPnP */
#endif


/* Exports */
#include "ftpd.h"

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

#include "sws_net.h"
#include "sws_str.h"   /* SWS_stricmp */
#include "sws_cfg.h"
#if OPT_UPNP
#include "sws_upnp.h"
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

#ifndef MAX_PATH
# ifdef _POSIX_PATH_MAX
#  define MAX_PATH _POSIX_PATH_MAX
# else
#  define MAX_PATH 256
# endif
#endif

/* Log file prefix */
#define kszPrefix "FTPD: "

#define POLL_TIMEOUT    500U /* select timeout in millisecs */
#define SEND_TIMEOUT  60000U /* Control & data send timeout in millisecs */
#define IDLE_TIMEOUT 180U    /* Control idle timeout in seconds. RFC 1123 4.1.3.2 says >= 300 secs */

#ifndef MSG_NOSIGNAL
#define MSG_NOSIGNAL 0
#endif

#ifndef FTP_BUFFER_SIZE
#define FTP_BUFFER_SIZE 3000 /* Data buffer size, >= 2 * MAX_PATH */
#endif
#if FTP_BUFFER_SIZE < 2 * MAX_PATH
#error FTP_BUFFER_SIZE too small
#endif

#define FTP_MAX_INPUT (MAX_PATH + 8)    /* Maximum input line length */
#define USERNAME_MAX 32     /* Max username length */

#define BLOCKED_CLIENTS 16  /* Max number of blocked client IP's */

#define cNul       '\0'
#define cSlash     '/'
#define cBackSlash '\\'
#define cReturn    '\x0d'
#define cLinefeed  '\x0a'

#define CRLF        "\x0d\x0a"
#define kszMatchAny "*.*"
#define kszRoot     "/"
#define kszRNFR     "RNFR"

/* ini file values */
#define kszFtpSect        "ftp"
#define kszLogVerbosity   "log"
#define kszMaxConnections "max_conn"
#define kszFtpUsersSect   "ftp_users"
#define kszIdleTimeout    "idle_time"
#define kszLoginAttempts  "login_attempts"
#define kszLockoutTime    "lockout_time"


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

/* FTP control state */
typedef enum EControlState
  {
  kFtpControlLoggedOut = 0,
  kFtpControlPassword,
  kFtpControlLoggedIn,
  kFtpControlQuit
  } EControlState;

/* FTP data state */
typedef enum EDataState
  {
  kFtpDataIdle = 0,
  kFtpDataAccepting,
  kFtpDataConnecting,
  kFtpDataConnected,
  kFtpDataRead,
  kFtpDataWrite,
  kFtpDataAborted
  } EDataState;

/* User permissions */
typedef enum EPermission
  {
  kPermitNone = 0,
  kPermitRead,
  kPermitExplore,
  kPermitWrite,
  kPermitChange
  } EPermission;

/* Connection state */
typedef struct SConnection SConnection;
typedef void FFtpData( SConnection*);

struct SConnection
  {
  SDeque list;
#define LIST2CONN( p) CONTAINER( p, SConnection, list)

  SOCKET s;                     /* Control connection */
  struct sockaddr_in client;    /* Client control connection address & port */
  struct sockaddr_in clientdata;/* Client data connection address & port */
  u_short data_port;            /* Default server data port */

  time_t tIdle;

  /* Telnet state */
  ETelnetState eTelnetState;
  int telnetCR;
  size_t telnetCount, telnetSize;
  char* pTelnetInput;           /* -> input buffer */

  /* FTP control state */
  EControlState eControlState;
  char szUser[ USERNAME_MAX]; /* Logged in user */
  EPermission ePermit;    /* Client permission */
  unsigned uLoginAttempts; /* Number of login attempts */
  char type;              /* Data type, A= ASCII, I= binary */
  char format;            /* Data format: N= non-print T=Telnet, C=Carriage */
  char mode;              /* Transfer mode. S= stream */
  char stru;              /* Structure mode, F= file mode */
  long lSeek;             /* Restore offset */
  char szHome[ MAX_PATH]; /* Home directory */
  char szDir[ MAX_PATH];  /* Current directory relative to szHome */

  /* Data connection */
  EDataState eDataState;
  SOCKET sData;           /* Data connection */
  FFtpData* pfnData;      /* Data connection event handler */
  unsigned long ulBytes;  /* Data bytes transferred so far */
  FILE* file;
  size_t sizePending;     /* Size of pending data buffer */
  char* pDataPending;     /* -> data pending transmit space */

  const char* pszTail;

  /* Control input buffer */
  char input[ FTP_MAX_INPUT];

  /* General purpose transfer buffer */
  char buffer[ FTP_BUFFER_SIZE];
  };


/* ftp command function */
typedef int FFtpMethod( SConnection*, const char*, char*);

/* ftp command descriptor */
typedef struct SMethod
  {
  const char* pszName;
  FFtpMethod* pfnExecute;
  const char* pszHelp;
  } SMethod;


/* Blocked client */
struct SBlockedIP
  {
  SDeque list;
#define LIST2BLOCKEDIP( p) CONTAINER( p, struct SBlockedIP, list)
  struct in_addr addr;
  time_t tBlock;
  };


/***************************************************************************
 * Function prototypes
 ***************************************************************************/

static void SessionStart( SConnection*);
static void SessionFinish( SConnection*);
static void SessionInput( SConnection*);
static void SessionAbort( SConnection*);

static int ExecuteRequest( SConnection*, char*);
static const SMethod* FindCommand( const char*, const SMethod [], unsigned);

static int FtpReply( SConnection*, int, const char*);
static int FtpConnect( SConnection*);

static int TelnetRead( SConnection*, void*, size_t);
static int TelnetReadLine( SConnection*);

static void Log( int, const struct sockaddr_in*, const char [], ...) FORMAT(printf,3,4);

static int WriteControl( SConnection*, const void*, size_t);
static int ReadControl( SConnection*, void*, size_t, int);
static int WriteData( SConnection*, SOCKET, const char*, size_t);
static int ReadData( SConnection*, char*, size_t);

static char* MakePath( char*, const SConnection*, const char*);
static EPermission ValidateUser( SConnection*, const char*);
static int ListFiles( SConnection*, const char*, char*, int);
static int StoreFile( SConnection*, const char*, char*, int);

static SDeque* DequeRemove( SDeque* const);
static void DequeAppend( SDeque* const pDeque, SDeque* const pItem);
#define DEQUE_INIT( p)     ((p)->tail = (p)->head = (p))
#define DEQUE_IS_EMPTY( p) ((p)->tail == (p))
#define DEQUE_FIRST( p)    ((p)->tail)
#define DEQUE_LAST( p)     ((p)->head)

#define L_UNIQUE 16
static char* UniqueName( char [ L_UNIQUE] );

static int PlatformList( SConnection*, SOCKET, char*, int full);
static int PlatformIsDir( const char*);
static int PlatformMkdir( const char*);
static int PlatformRmdir( const char*);
enum EAccessMode { kAccessExist = 0, kAccessWrite = 2, kAccessRead = 4};
static int PlatformAccess( const char*, enum EAccessMode);
static long PlatformFileLength( FILE*);
static int PlatformChmod( const char*, unsigned);
static time_t PlatformMdtm( const char*);


/***************************************************************************
 * Module data
 ***************************************************************************/

/* Configuration */
static unsigned s_uIdleTimeout;
static unsigned s_uMaxLoginAttempts;
static unsigned s_uLockoutTime;

/* Logging */
static int s_iVerbosity;
static FILE* s_logfile;

/* Operation */
extern sig_atomic_t volatile g_exit;
extern sig_atomic_t volatile g_running;
static SDeque s_free, s_active; /* Free and active connection lists */
static SDeque s_freeBlockedIP, s_activeBlockedIP; /* Free and active blocked client list */

/* The method table */
static FFtpMethod UnimplementedMethod;
static FFtpMethod QuitCommand;
static FFtpMethod HelpCommand;
static FFtpMethod UserMethod;
static FFtpMethod PasswordMethod;
static FFtpMethod NoopMethod;
static FFtpMethod PortMethod;
static FFtpMethod SystemMethod;
static FFtpMethod TypeMethod;
static FFtpMethod StructureMethod;
static FFtpMethod ModeMethod;
static FFtpMethod RetrieveMethod;
static FFtpMethod StoreMethod;
static FFtpMethod AppendMethod;
static FFtpMethod ListMethod;
static FFtpMethod PwdMethod;
static FFtpMethod PasvMethod;
static FFtpMethod NlstMethod;
static FFtpMethod CdupMethod;
static FFtpMethod CwdMethod;
static FFtpMethod MkdMethod;
static FFtpMethod RmdMethod;
static FFtpMethod DeleMethod;
static FFtpMethod AccountMethod;
static FFtpMethod RnfrMethod;
static FFtpMethod RntoMethod;
static FFtpMethod StatMethod;
static FFtpMethod ReinitMethod;
static FFtpMethod RestoreMethod;
static FFtpMethod AbortMethod;
static FFtpMethod SizeMethod;
static FFtpMethod MdtmMethod;
static FFtpMethod SiteMethod;
static FFtpMethod SiteHelpCommand;
static FFtpMethod ChmodMethod;
static FFtpMethod FeatMethod;

static const SMethod methods[] = {
  /* Access control */
  { "USER", &UserMethod,           "<SP><user> Login a new user." }, /* RFC 959 */
  { "PASS", &PasswordMethod,       "<SP><text> Enter user password." },
  { "ACCT", &AccountMethod,        "<SP><text> Enter account details" }, /* RFC 1123 */
  { "CWD",  &CwdMethod,            "<SP><pathname>" }, /* RFC 1123 */
  { "XCWD", &CwdMethod,            NULL }, /* RFC 1123 */
  { "CDUP", &CdupMethod,           "Change to parent directory" }, /* RFC 1123 */
  { "XCUP", &CdupMethod,           NULL }, /* RFC 1123 */
  { "SMNT", &UnimplementedMethod,  NULL },
  { "REIN", &ReinitMethod,         "Reinitialise" },
  { "QUIT", &QuitCommand,          "Terminate the session." }, /* RFC 959 */
  /* Transfer parameters */
  { "PORT", &PortMethod,           "<SP><h1,h2,h3,h4,p1,p2> Set data port." }, /* Required */
  { "PASV", &PasvMethod,           "Set passive mode" }, /* RFC 1123 */
  { "TYPE", &TypeMethod,           "<SP><A|I> [<SP>N] Set data type" }, /* RFC 959 */
  { "STRU", &StructureMethod,      "<SP><F> Set file type" }, /* RFC 959 */
  { "MODE", &ModeMethod,           "<SP><S> Set data transfer mode" }, /* RFC 959 */
  /* Service */
  { "RETR", &RetrieveMethod,       "<SP><filename> Fetch a file" }, /* RFC 959 */
  { "STOR", &StoreMethod,          "<SP><filename> Store a file" }, /* RFC 959 */
  { "STOU", &UnimplementedMethod,  NULL },
  { "APPE", &AppendMethod,         "<SP><filename> Append to a file" }, /* RFC 1123 */
  { "ALLO", &NoopMethod,           NULL },
  { "REST", &RestoreMethod,        "<SP><integer> Start transfer at offset" },
  { "RNFR", &RnfrMethod,           "<SP><filename> Rename from" }, /* RFC 1123 */
  { "RNTO", &RntoMethod,           "<SP><filename> Rename to" }, /* RFC 1123 */
  { "ABOR", &AbortMethod,          "Abort a file operation" },
  { "DELE", &DeleMethod,           "<SP><filename> Delete a file" }, /* RFC 1123 */
  { "RMD",  &RmdMethod,            "<SP><pathname> Delete a directory" }, /* RFC 1123 */
  { "XRMD", &RmdMethod,            NULL }, /* RFC 1123 */
  { "MKD",  &MkdMethod,            "<SP><pathname> Make a directory" }, /* RFC 1123 */
  { "XMKD", &MkdMethod,            NULL }, /* RFC 1123 */
  { "PWD",  &PwdMethod,            "Show the current directory" },
  { "XPWD", &PwdMethod,            NULL },
  { "LIST", &ListMethod,           "[<SP>filespec] Directory listing" }, /* RFC 1123 */
  { "NLST", &NlstMethod,           "[<SP>filespec] List files" }, /* RFC 1123 */
  { "SITE", &SiteMethod,           "<SP><command> Site specific commands" },
  { "SYST", &SystemMethod,         "Show system name." }, /* RFC 1123 */
  { "STAT", &StatMethod,           "[<SP>filespec] Display status" }, /* RFC 1123 */
  { "HELP", &HelpCommand,          "[<SP>cmd] Provide command line help." }, /* RFC 1123 */
  /* Extensions */
  { "FEAT", &FeatMethod,           "Feature negotiation mechanism" }, /* RFC 2389 */
  { "SIZE", &SizeMethod,           "<SP><filename> Size of file" }, /* RFC 3659 */
  { "MDTM", &MdtmMethod,           "<SP><filename> File modification time" }, /* RFC 3659 */
  /* End of list */
  { "",     &NoopMethod,           NULL },
  { "NOOP", &NoopMethod,           "No operation" } /* RFC 959 */
};
/* SITE methods */
static const SMethod sitemethods[] = {
  { "CHMOD", &ChmodMethod,         "<SP><N> <filename> Set file permissions" },
  { "HELP", &SiteHelpCommand,      "[<SP>cmd] Provide command line help." },
  { "",     &NoopMethod,           NULL }
};
/* FEATures implemented */
static const char kszFeatures[] =
  " MDTM" CRLF
  " SIZE" CRLF
  " REST STREAM" CRLF
;


/***************************************************************************
 * Functions
 ***************************************************************************/

/*
 * FTP server
 */
int Ftpd(
  const void* pHost,
  int iVerbosity,
  FILE* logfile
) {
  int iRet;
  unsigned val;
  SOCKET s;
  SConnection* pConnArray;
  size_t maxConn;
  SDeque* pDeque, *pNext;
  SConnection* pConn;
  struct SBlockedIP* pBlockList;

  /* Read configurable values */
  s_uIdleTimeout = (unsigned)SWS_CfgGetValue( kszFtpSect, kszIdleTimeout, IDLE_TIMEOUT);
  s_uMaxLoginAttempts = (unsigned)SWS_CfgGetValue( kszFtpSect, kszLoginAttempts, 3);
  s_uLockoutTime = (unsigned)SWS_CfgGetValue( kszFtpSect, kszLockoutTime, 30);

  maxConn = (size_t)SWS_CfgGetValue( kszFtpSect, kszMaxConnections, OPT_MAX_CONNECTIONS);
  s_iVerbosity = (int)SWS_CfgGetValue( kszFtpSect, kszLogVerbosity, iVerbosity);
  s_logfile = logfile;

  /* Open server socket */
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
    ((struct sockaddr_in*)pHost)->sin_port = htons( FTPD_PORT);
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

  /* Disable Nagle */
  val = 1;
  iRet = setsockopt(
    s,
    IPPROTO_TCP, TCP_NODELAY,
    (char*)&val, sizeof(val)
  );

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

  if ( 2 * maxConn + 1 > FD_SETSIZE)
    {
    Log( kLogError, NULL, "ERROR: %u connections greater than compilation limit of %u\n",
      (unsigned)maxConn, FD_SETSIZE - 1);
    closesocket( s);
    return EINVAL;
    }

  /* Allocate free connection list */
  pConnArray = (SConnection*) calloc( maxConn, sizeof( SConnection) );
  if ( NULL == pConnArray)
    {
    Log( kLogError, NULL, "ERROR: Out of memory for %u connections\n", (unsigned)maxConn);
    closesocket( s);
    return ENOMEM;
    }

  DEQUE_INIT( &s_active);
  DEQUE_INIT( &s_free);
  for ( val = 0; val < maxConn; ++val)
    {
    DEQUE_INIT( &pConnArray[ val].list);
    DequeAppend( &s_free, &pConnArray[ val].list);
    }

  /* Setup blocked client list */
  DEQUE_INIT( &s_freeBlockedIP);
  DEQUE_INIT( &s_activeBlockedIP);
  pBlockList = (struct SBlockedIP*) calloc( BLOCKED_CLIENTS, sizeof( struct SBlockedIP) );
  if ( NULL != pBlockList)
    {
    for ( val = 0; val < BLOCKED_CLIENTS; ++val)
      {
      DEQUE_INIT( &pBlockList[ val].list);
      DequeAppend( &s_freeBlockedIP, &pBlockList[ val].list);
      }
    }
  else
    Log( kLogError, NULL, "ERROR: Out of memory for blocked client list\n");

#ifdef SIGPIPE
  /* Ignore SIGPIPE if connection reset during send() */
  signal( SIGPIPE, SIG_IGN);
#endif

#if OPT_UPNP
  {
  unsigned uExtPort = SWS_CfgGetValue( kszFtpSect, "UpnpExtPort", 0);
  if ( uExtPort && SWS_UpnpAddPortMap(
      "TCP",
      ntohs( ((const struct sockaddr_in*)pHost)->sin_port),
      inet_ntoa( ((const struct sockaddr_in*)pHost)->sin_addr),
      uExtPort,
      "Ftpd"
     )
  )
    uExtPort = 0;
#endif

  Log( kLogWarn, (const struct sockaddr_in*)pHost, "LISTENING verbosity %d\n", s_iVerbosity);

  /* The server loop */
  ++g_running;
  while ( !g_exit)
    {
    static fd_set setRd, setWr, setEx;
    time_t t;
    struct timeval tval;

    FD_ZERO( &setEx);
    FD_ZERO( &setWr);
    FD_ZERO( &setRd);

    if ( DEQUE_IS_EMPTY( &s_free))
      {
      /* Abort and re-use oldest connection */
      pConn = LIST2CONN( DEQUE_FIRST( &s_active));
      Log( kLogWarn, &pConn->client, "WARN: Recycling connection\n");

      SessionAbort( pConn);
      SessionFinish( pConn);

      /* Return it to the free list */
      DequeRemove( &pConn->list);
      DequeAppend( &s_free, &pConn->list);
      }

    FD_SET( s, &setRd);

    /* Add active sockets to the select sets */
    for (
      pDeque = DEQUE_FIRST( &s_active);
      pDeque != &s_active;
      pDeque = DEQUE_FIRST( pDeque)
    ) {
      pConn = LIST2CONN( pDeque);

      /* Receive control data */
      FD_SET( pConn->s, &setRd);

      switch ( pConn->eDataState)
        {
      case kFtpDataIdle:
      case kFtpDataConnected:
      case kFtpDataAborted:
        break;
      case kFtpDataAccepting:
        FD_SET( pConn->sData, &setRd);
        break;
      case kFtpDataConnecting:
        FD_SET( pConn->sData, &setWr);
        FD_SET( pConn->sData, &setEx);
        break;
      case kFtpDataRead:
        FD_SET( pConn->sData, &setRd);
        break;
      case kFtpDataWrite:
        FD_SET( pConn->sData, &setWr);
        break;
        }
      }

    /* Set timeout for select */
    tval.tv_usec = (POLL_TIMEOUT % 1000) * 1000UL;
    tval.tv_sec = POLL_TIMEOUT / 1000;

    /* Wait for event */
    Log( kLogMost, NULL, "Waiting\n");
    iRet = select( FD_SETSIZE, &setRd, &setWr, &setEx, &tval);
    if ( SOCKET_ERROR == iRet)
      {
      int err = GetLastError();
      Log( kLogMore, NULL, "WARN %d: select: %s\n", err, strerror( err));
      continue;
      }
    Log( kLogMost, NULL, "Ready\n");

    t = time( NULL);

    if ( FD_ISSET( s, &setRd))
      {
      socklen_t len;

      /* Incoming connection */
      pConn = LIST2CONN( DequeRemove( DEQUE_FIRST( &s_free) ));

      pConn->data_port = htons( (u_short)(ntohs( ((const struct sockaddr_in*)pHost)->sin_port) - 1));
      pConn->tIdle = t + s_uIdleTimeout; /* Start the idle timer */

      pConn->eTelnetState = kTelnetData;
      pConn->telnetCR = 0;
      pConn->telnetCount = 0;
      pConn->telnetSize = 0;
      pConn->pTelnetInput = NULL;

      /* Initialise FTP state */
      pConn->eControlState = kFtpControlLoggedOut;
      pConn->eDataState = kFtpDataIdle;
      pConn->ePermit = kPermitNone;
      pConn->uLoginAttempts = 0;
      pConn->type = 'A';   /* 8-bit ASCII <A|E|I> */
      pConn->format = 'N'; /* Non-print <N|T|C> */
      pConn->mode = 'S';   /* Stream S|B|C */
      pConn->stru = 'F';   /* File (no structure) F|R|P */
      pConn->lSeek = 0;
      pConn->szDir[ 0] = cNul;

      pConn->sData = INVALID_SOCKET;
      pConn->pfnData = 0;
      pConn->file = NULL;

      /* Accept the incoming connection */
      len = sizeof(pConn->client);
      pConn->s = accept( s, (struct sockaddr*)&pConn->client, &len);
      if ( INVALID_SOCKET == pConn->s)
        {
        int err = GetLastError();
        Log( kLogWarn, NULL, "WARN %d: accept: %s\n", err, strerror( err));
        DequeAppend( &s_free, &pConn->list);
        }
      else
        {
        if ( s_logfile != stdout && s_logfile != stderr)
          Log( kLogError, &pConn->client, "CONNECT\n");

        pConn->clientdata = pConn->client;
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

      switch ( pConn->eDataState)
        {
      case kFtpDataIdle:
      case kFtpDataConnected:
      case kFtpDataAborted:
        break;

      case kFtpDataAccepting:
        if ( FD_ISSET( pConn->sData, &setRd))
          {
          SOCKET sData;
          struct sockaddr_in from;
          socklen_t len = sizeof from;

          /* Accept passive data connection */
          Log( kLogMore, &pConn->client, "Accept passive connection\n");
          sData = accept( pConn->sData, (struct sockaddr*)&from, &len);

          closesocket( pConn->sData);
          pConn->sData = sData;
          if ( INVALID_SOCKET != sData)
            {
            pConn->eDataState = kFtpDataConnected;
            Log( kLogMore, &pConn->client, "Passive connection from %s:%u\n",
              inet_ntoa( from.sin_addr), ntohs( from.sin_port));
            }
          else
            {
            int err = GetLastError();
            Log( kLogWarn, &pConn->client, "WARN %d: passive accept: %s\n", err, strerror( err));
            pConn->eDataState = kFtpDataIdle;
#if OPT_UPNP
            /*{int TODO_UnMapPasvPort;}*/
#endif
            }

          /* Restart the idle timer */
          pConn->tIdle = t + s_uIdleTimeout;

          if ( pConn->pfnData)
            (*pConn->pfnData)( pConn);
          }
        break;

      case kFtpDataConnecting:
        if ( FD_ISSET( pConn->sData, &setEx))
          {
          pConn->eDataState = kFtpDataIdle;
          (*pConn->pfnData)( pConn);
          }
        else if ( FD_ISSET( pConn->sData, &setWr))
          {
          /* Restart the idle timer */
          pConn->tIdle = t + s_uIdleTimeout;

          pConn->eDataState = kFtpDataConnected;
          (*pConn->pfnData)( pConn);
          }
        break;

      case kFtpDataRead:
        if ( FD_ISSET( pConn->sData, &setRd))
          {
          /* Restart the idle timer */
          pConn->tIdle = t + s_uIdleTimeout;

          (*pConn->pfnData)( pConn);
          }
        break;

      case kFtpDataWrite:
        if ( FD_ISSET( pConn->sData, &setWr))
          {
          /* Restart the idle timer */
          pConn->tIdle = t + s_uIdleTimeout;

          (*pConn->pfnData)( pConn);
          }
        break;
        }

      if ( FD_ISSET( pConn->s, &setRd))
        {
        /* Restart the idle timer */
        pConn->tIdle = t + s_uIdleTimeout;

        /* Process the input */
        SessionInput( pConn);
        }

      if ( s_uIdleTimeout > 0 && t > pConn->tIdle)
        {
        /* Timed out */
        Log( kLogMore, &pConn->client, "Idle timeout\n" );
        SessionAbort( pConn);
        }

      /* Is the session finished? */
      if ( kFtpControlQuit == pConn->eControlState)
        {
        /* Yes */
        if ( s_logfile != stdout && s_logfile != stderr)
          Log( kLogError, &pConn->client, "DISCONNECT\n");

        SessionFinish( pConn);

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
    }

#if OPT_UPNP
  if ( uExtPort)
    SWS_UpnpDelPortMap( "TCP", uExtPort);
  }
#endif

  free( pBlockList);
  free( pConnArray);
  closesocket( s);

  return iRet;
  }


/****************************************************************************
 * Session I/O
 ****************************************************************************/

/*
 * Start a session
 */
void SessionStart( SConnection* pConn)
  {
  SDeque* pDeque, *pNext;
  time_t const t = time( NULL);

  pConn->pTelnetInput = pConn->input;
  pConn->telnetSize = sizeof(pConn->input);

  strcpy( pConn->input, "OPEN");
  pConn->pszTail = "";

  /* Check if the client IP is blocked */
  for (
    pDeque = DEQUE_FIRST( &s_activeBlockedIP);
    pDeque != &s_activeBlockedIP;
    pDeque = pNext
  ) {
    struct SBlockedIP* pBlock = LIST2BLOCKEDIP( pDeque);
    pNext = DEQUE_FIRST( pDeque);

    /* Check if timeout expired */
    if ( 0 != pBlock->tBlock && pBlock->tBlock < t)
      {
      /* Yes, move item to free list */
      DequeRemove( pDeque);
      DequeAppend( &s_freeBlockedIP, pDeque);
      }
    else if ( pBlock->addr.s_addr == pConn->client.sin_addr.s_addr)
      {
      FtpReply( pConn, 421, "Client blocked due to abuse.");
      /*pBlock->tBlock = t + 60 * s_uLockoutTime;*/
      SessionAbort( pConn);
      return;
      }
    }

  /* Greeting */
  if ( kFtpControlLoggedIn == pConn->eControlState)
    {
    /* User logged in, proceed */
    FtpReply( pConn, 230, "-" kszBanner ".");
    FtpReply( pConn, 230, "ok.");
    }
  else
    {
    /* Service ready for new user */
    FtpReply( pConn, 220, "-" kszBanner ".");
    FtpReply( pConn, 220, "Proceed to login.");
    }
  }


/*
 * End a session
 */
static void SessionFinish( SConnection* pConn)
  {
  Log( kLogInfo, &pConn->client, "CLOSE\n");

  switch ( pConn->eDataState)
    {
  case kFtpDataIdle:
  case kFtpDataAccepting:
  case kFtpDataAborted:
    break;

  case kFtpDataConnecting:
  case kFtpDataConnected:
  case kFtpDataRead:
  case kFtpDataWrite:
    pConn->eDataState = kFtpDataAborted;
    if ( pConn->pfnData)
      (*pConn->pfnData)( pConn);
    break;
    }

  /* Release resources */
#if OPT_UPNP
  /*{int TODO_UnMapPasvPort;}*/
#endif
  if ( INVALID_SOCKET != pConn->sData)
    closesocket( pConn->sData);
  if ( NULL != pConn->file)
    fclose( pConn->file);

  /* Disable non-blocking mode */
  setsockasync( pConn->s, 0);
  closesocket( pConn->s);
  }


/*
 * Abort a session
 */
static void SessionAbort( SConnection* pConn)
  {
  struct linger lgr;

  /* Reset the connection when closed */
  lgr.l_onoff = 1;
  lgr.l_linger = 0;
  setsockopt( pConn->s, SOL_SOCKET, SO_LINGER, (char*)&lgr, sizeof( lgr));

  pConn->eControlState = kFtpControlQuit;
  }


/*
 * Session input ready
 */
void SessionInput( SConnection* pConn)
  {
  int iRet;

  /* Input a command line */
  iRet = TelnetReadLine( pConn);
  if ( 0 == iRet)
    return; /* Not complete */

  if ( SOCKET_ERROR == iRet)
    {
    /* Disconnected */
    pConn->eControlState = kFtpControlQuit;
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
  char szResponse[ FTP_MAX_INPUT];
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

  szResponse[ 0] = cNul;

  /* Find the command */
  pMethod = FindCommand( pszCmd, methods, lengthof( methods));
  if ( NULL != pMethod)
    {
    /* Execute the command */
    iRet = (*pMethod->pfnExecute)( pConn, pszTail, szResponse);
    }
  else
    {
    iRet = UnimplementedMethod( pConn, pszTail, szResponse);
    }

  return FtpReply( pConn, iRet, szResponse);
  }



/*
 * Find a command descriptor given a string
 */
const SMethod* FindCommand(
  const char* pszCmd,
  const SMethod aMethods[],
  unsigned uMethods
) {
  unsigned i;
  for ( i = 0; i < uMethods; ++i)
    {
    if ( 0 == SWS_stricmp( pszCmd, aMethods[ i].pszName) )
      return &aMethods[ i];
    }

  return NULL;
  }


/*
 * Send an FTP response
 */
int FtpReply(
  SConnection* pConn,
  int code,
  const char* pszInfo
) {
  int iLen;
  char const prefix = (char)('-' == *pszInfo ? *(pszInfo++) : ' ');
  char szBuf[ FTP_MAX_INPUT];

  /* Disable non-blocking mode */
  setsockasync( pConn->s, 0);

  { STATIC_ASSERT( sizeof szBuf >= 80); }
  iLen = sprintf( szBuf, "%d%c%.73s" CRLF, code, prefix, pszInfo);
  WriteControl( pConn, szBuf, iLen);

  /* Restore non-blocking mode */
  setsockasync( pConn->s, 1);

  /* Log the request and final response */
  if ( '-' != prefix /* More output to follow */
    && 125 != code   /* Transfer starting */
    && 150 != code   /* About to open data connection */
  ) {
    const char* pszTail;
    const char* pszSep;

    /* Mask valid passwords for full accounts */
    pszTail = pConn->pszTail;
    if ( 230 == code
      && 0 == SWS_stricmp( "PASS", pConn->input)
      && kPermitChange <= pConn->ePermit
    ) {
      pszTail = "####";
      }
    pszSep = cNul != pszTail[ 0] ? " " : "";

    Log( code < 400 ? kLogInfo : kLogWarn, &pConn->client, "%s%s%s ; %d %s\n",
      pConn->input, pszSep, pszTail, code, pszInfo
    );
    }

  return code;
  }


/****************************************************************************
 * FTP methods
 ****************************************************************************/

/*
 * USER command
 */
static int UserMethod(
  SConnection* pConn,
  const char* pszTail,
  char* pszInfo
) {
  if ( cNul == *pszTail)
    {
    strcpy( pszInfo, "Username required.");
    return 501; /* Syntax error in parameters or arguments */
    }

  strncpy( pConn->szUser, pszTail, sizeof( pConn->szUser));
  pConn->szUser[ sizeof(pConn->szUser) - 1] = cNul;

  pConn->ePermit = ValidateUser( pConn, NULL);
  switch ( pConn->ePermit)
    {
  case kPermitNone:
    /* Don't leak info if username is unknown or not permitted */
    pConn->eControlState = kFtpControlPassword;
    strcpy( pszInfo, "Password required.");
    return 331; /* User name okay, need password.*/

  case kPermitRead:
  case kPermitExplore:
    sprintf( pszInfo, "%s login. Read only access", pConn->szUser);
    break;

  case kPermitWrite:
    sprintf( pszInfo, "%s login. Restrictions apply", pConn->szUser);
    break;

  case kPermitChange:
    sprintf( pszInfo, "%s login. Full access", pConn->szUser);
    break;
    }

  pConn->eControlState = kFtpControlLoggedIn;
  return 230; /* User logged in, proceed 332= Need account for login */
  }


/*
 * PASS command
 */
static int PasswordMethod(
  SConnection* pConn,
  const char* pszTail,
  char* pszInfo
) {
  switch ( pConn->eControlState)
    {
  case kFtpControlLoggedOut:
    strcpy( pszInfo, "Not logged in.");
    return 530; /* Not logged in */

  case kFtpControlPassword:
  case kFtpControlLoggedIn:
    break;

  default:
    return 503; /* Bad sequence of commands */
    }

  pConn->uLoginAttempts++;
  pConn->ePermit = ValidateUser( pConn, pszTail);
  switch ( pConn->ePermit)
    {
  case kPermitNone:
    if ( s_uMaxLoginAttempts > 0 && pConn->uLoginAttempts >= s_uMaxLoginAttempts)
      {
      struct SBlockedIP* pBlock;

      SessionAbort( pConn);

      if ( !DEQUE_IS_EMPTY( &s_freeBlockedIP))
        pBlock = LIST2BLOCKEDIP( DequeRemove( DEQUE_FIRST( &s_freeBlockedIP) ));
      else if ( !DEQUE_IS_EMPTY( &s_activeBlockedIP))
        pBlock = LIST2BLOCKEDIP( DequeRemove( DEQUE_LAST( &s_activeBlockedIP) ));
      else
        {
        strcpy( pszInfo, "Incorrect login. Too many attempts");
        return 421;
        }

      pBlock->addr = pConn->client.sin_addr;

      if ( s_uLockoutTime > 0)
        {
        sprintf( pszInfo,
          "Incorrect login. Too many attempts - client blocked for %u minutes",
          s_uLockoutTime
        );
        pBlock->tBlock = time( NULL) + s_uLockoutTime * 60;
        }
      else
        {
        strcpy( pszInfo, "Incorrect login. Too many attempts - client blocked");
        pBlock->tBlock = 0;
        }

      DequeAppend( &s_activeBlockedIP, &pBlock->list);
      return 421;
      }

    pConn->eControlState = kFtpControlLoggedOut;
    strcpy( pszInfo, "Incorrect login");
    return 530; /* Not logged in  */

  case kPermitRead:
  case kPermitExplore:
    sprintf( pszInfo, "%s login. Read only access", pConn->szUser);
    break;

  case kPermitWrite:
    sprintf( pszInfo, "%s login. Restrictions apply", pConn->szUser);
    break;

  case kPermitChange:
    sprintf( pszInfo, "%s login. Full access", pConn->szUser);
    break;
    }

  pConn->eControlState = kFtpControlLoggedIn;
  pConn->uLoginAttempts = 0;
  return 230; /* User logged in, proceed. 332= Need account for login */
  }


/*
 * ACCT command
 */
static int AccountMethod(
  SConnection* pConn,
  const char* pszTail,
  char* pszInfo
) {
  (void)pszTail;

  switch ( pConn->eControlState)
    {
  case kFtpControlLoggedOut:
    strcpy( pszInfo, "Not logged in.");
    return 530; /* Not logged in */
  case kFtpControlLoggedIn:
    break;
  default:
    strcpy( pszInfo, "Login not completed.");
    return 503; /* Bad sequence of commands */
    }

#if 0
  (void)pConn;
  strcpy( pszInfo, "ok.");
  return 230; /* User logged in, proceed */
#else
  (void)pConn;
  strcpy( pszInfo, "Account not required.");
  return 202; /* Command not implemented, superfluous at this site */
#endif
  }


/*
 * REIN command
 */
static int ReinitMethod(
  SConnection* pConn,
  const char* pszTail,
  char* pszInfo
) {
  if ( *pszTail)
    {
    strcpy( pszInfo, "Unexpected parameter.");
    return 501; /* Syntax error in parameters or arguments */
    }

  pConn->szUser[ 0] = cNul;
  pConn->eControlState = kFtpControlLoggedOut;
  pConn->ePermit = kPermitNone;
  strcpy( pszInfo, "ok.");

  return 220; /* Service ready for new user */
  }


/*
 * QUIT command (required)
 */
static int QuitCommand(
  SConnection* pConn,
  const char* pszTail,
  char* pszInfo
) {
  (void)pConn;
  (void)pszTail;

  pConn->eControlState = kFtpControlQuit;
  strcpy( pszInfo, "Goodbye.");
  return 221; /* Quit */
  }


/*
 * RETR method (required)
 */
static void RetrieveComplete( SConnection*);
static FFtpData RetrieveProgress;

static int RetrieveMethod(
  SConnection* pConn,
  const char* pszTail,
  char* pszInfo
) {
  int iRet = 0;
  char* pszPath;

  switch ( pConn->eControlState)
    {
  case kFtpControlLoggedIn:
    break;
  default:
    strcpy( pszInfo, "Not logged in.");
    return 530; /* Not logged in */
    }

  switch ( pConn->eDataState)
    {
  case kFtpDataIdle:
  case kFtpDataAccepting:
  case kFtpDataConnected:
    break;

  default:
    strcpy( pszInfo, "Transfer in progress.");
    return 503; /* Bad sequence of commands */
    }

  if ( cNul == *pszTail)
    {
    strcpy( pszInfo, "Missing filespec.");
    return 501; /* Syntax error in parameters or arguments */
    }

  /* Disallow ".." in pathname */
  if ( strstr( pszTail, ".."))
    {
    strcpy( pszInfo, "Forbidden .. in filename");
    return 553; /* Requested action not taken.File name not allowed. */
    }

#if 0
  if ( pConn->ePermit < kPermitRead)
    {
    strcpy( pszInfo, "Insufficient privilege");
    return 550;
    }
#endif

  /* Form local pathname */
  pszPath = pConn->buffer;
  MakePath( pszPath, pConn, pszTail);
  if ( PlatformIsDir( pszPath))
    {
    strcpy( pszInfo, "Directory not permitted");
    return 550; /* Requested file action not taken - File unavailable */
    }

  /* Open the file */
  pConn->file = fopen( pszPath, 'I' == pConn->type ? "rb" : "r");
  if ( NULL == pConn->file)
    {
    strcpy( pszInfo, strerror( errno) );
    return 550; /* Requested file action not taken - File unavailable */
    }

  fseek( pConn->file, pConn->lSeek, SEEK_CUR);
  pConn->lSeek = 0;

  pConn->ulBytes = 0;
  pConn->pfnData = &RetrieveProgress;
  pConn->sizePending = 0;

  /* Initiate data connection */
  FtpConnect( pConn);
  switch ( pConn->eDataState)
    {
  case kFtpDataIdle:
  case kFtpDataAborted:
    RetrieveComplete( pConn);
    strcpy( pszInfo, "Data connection failed.");
    iRet = 425; /* Can't open data connection */
    break;

  case kFtpDataAccepting:
  case kFtpDataConnecting:
    strcpy( pszInfo, "ok.");
    iRet = 150; /* File status okay; about to open data connection */
    break;

  case kFtpDataConnected:
  case kFtpDataRead:
    pConn->eDataState = kFtpDataWrite;
  case kFtpDataWrite:
    strcpy( pszInfo, "ok.");
    iRet = 125; /* Data connection already open; transfer starting */
    break;
    }

  return iRet;
  }

/* RETR event callback */
static void RetrieveProgress(
  SConnection* pConn
) {
  int i;

  switch ( pConn->eDataState)
    {
  case kFtpDataAccepting:
  case kFtpDataConnecting:
    return;

  case kFtpDataConnected:
    /* Enable non-blocking mode */
    setsockasync( pConn->sData, 1);
  case kFtpDataRead:
    pConn->eDataState = kFtpDataWrite;
    return; /* Wait till writable */

  case kFtpDataWrite:
    break;

  case kFtpDataAborted:
    /* Connection closed; transfer aborted */
    RetrieveComplete( pConn);
    FtpReply( pConn, 426, "Operation aborted.");
    return;

  case kFtpDataIdle:
    RetrieveComplete( pConn);
    FtpReply( pConn, 425, "Data connection failed.");
    return;
    }

  if ( pConn->sizePending > 0)
    {
    i = WriteData( pConn, pConn->sData, pConn->pDataPending, pConn->sizePending);
    if ( SOCKET_ERROR != i)
      {
      pConn->sizePending -= (size_t)i;
      pConn->pDataPending += (size_t)i;
      pConn->ulBytes += (unsigned)i;
      }
    else
      {
      RetrieveComplete( pConn);
      FtpReply( pConn, 426, "Transfer aborted.");
      }
    }
  else if ( !feof( pConn->file))
    {
    size_t count = 0;

    if ( 'I' == pConn->type)
      {
      count = fread( pConn->buffer, 1, FTP_BUFFER_SIZE, pConn->file);
      }
    else if ( NULL != fgets( pConn->buffer, FTP_BUFFER_SIZE - 2, pConn->file))
      {
      count = strlen( pConn->buffer);
      if ( count > 0 && '\n' == pConn->buffer[ count - 1] )
        {
        /* Convert newline to CRLF */
        strcpy( pConn->buffer + (count - 1), CRLF);
        ++count;
        }
      }

    if ( count > 0)
      {
      i = WriteData( pConn, pConn->sData, pConn->buffer, count);
      if ( SOCKET_ERROR != i)
        {
        pConn->sizePending = count - (size_t)i;
        pConn->pDataPending = pConn->buffer + (size_t)i;
        pConn->ulBytes += (unsigned)i;
        }
      else
        {
        RetrieveComplete( pConn);
        FtpReply( pConn, 426, "Transfer aborted.");
        }
      }
    else if ( ferror( pConn->file))
      {
      RetrieveComplete( pConn);
      FtpReply( pConn, 450, "File read error");
      }
    }
  else
    {
    char szInfo[ 32];

    RetrieveComplete( pConn);

    sprintf( szInfo, "%lu bytes sent ok.", pConn->ulBytes);
    FtpReply( pConn, 226, szInfo);
    }
  }


/* RETR cleanup */
static void RetrieveComplete(
  SConnection* pConn
) {
  pConn->eDataState = kFtpDataIdle;
  pConn->pfnData = 0;

  if ( INVALID_SOCKET != pConn->sData)
    {
    /* Disable non-blocking mode */
    setsockasync( pConn->sData, 0);

#if OPT_UPNP
    /*{int TODO_UnMapPasvPort;}*/
#endif
    closesocket( pConn->sData);
    pConn->sData = INVALID_SOCKET;
    }

  fclose( pConn->file);
  pConn->file = NULL;

  pConn->pDataPending = NULL;
  pConn->sizePending = 0;
  }


/*
 * STOR method (required)
 */
static int StoreMethod(
  SConnection* pConn,
  const char* pszTail,
  char* pszInfo
) {
  return StoreFile( pConn, pszTail, pszInfo, 0);
  }


/*
 * APPE method (required)
 */
static int AppendMethod(
  SConnection* pConn,
  const char* pszTail,
  char* pszInfo
) {
  return StoreFile( pConn, pszTail, pszInfo, 1);
  }


/*
 * Store a file
 */
static void StoreComplete( SConnection*, int);
static FFtpData StoreProgress;

static int StoreFile(
  SConnection* pConn,
  const char* pszTail,
  char* pszInfo,
  int iAppend
) {
  int iRet = 0;
  char *pszPath, *pszTemp;

  switch ( pConn->eControlState)
    {
  case kFtpControlLoggedIn:
    break;
  default:
    strcpy( pszInfo, "Not logged in.");
    return 530; /* Not logged in */
    }

  switch ( pConn->eDataState)
    {
  case kFtpDataIdle:
  case kFtpDataAccepting:
  case kFtpDataConnected:
    break;

  default:
    strcpy( pszInfo, "Transfer in progress.");
    return 503; /* Bad sequence of commands */
    }

  if ( cNul == *pszTail)
    {
    strcpy( pszInfo, "Missing filespec.");
    return 501; /* Syntax error in parameters or arguments */
    }

  /* Disallow ".." in pathname */
  if ( strstr( pszTail, ".."))
    {
    strcpy( pszInfo, "Forbidden .. in filename");
    return 553; /* Requested action not taken.File name not allowed. */
    }

  /* Form local pathname */
  pszPath = pConn->buffer + FTP_BUFFER_SIZE - (MAX_PATH + L_UNIQUE);
  pszTemp = pszPath + MAX_PATH;
  MakePath( pszPath, pConn, pszTail);

  /* Does file exist? */
  if ( PlatformAccess( pszPath, kAccessExist) )
    {
    /* Yes, then check for change permission */
    if ( pConn->ePermit < kPermitChange)
      {
      strcpy( pszInfo, "Insufficient privilege");
      return 550;
      }
    if ( !PlatformAccess( pszPath, kAccessWrite) )
      {
      strcpy( pszInfo, "File is read only");
      return 550;
      }
    }
  /* Check for write permission */
  else if ( pConn->ePermit < kPermitWrite)
    {
    strcpy( pszInfo, "Insufficient privilege");
    return 550;
    }

  if ( iAppend)
    {
    pConn->file = fopen( pszPath, 'I' == pConn->type ? "ab" : "a");
    *pszTemp = cNul;
    }
  else
    {
    /* Ensure file can be open/creat/ed */
    pConn->file = fopen( pszPath, "a");
    if ( NULL == pConn->file)
      {
      strcpy( pszInfo, strerror( errno));
      return 550; /* Requested file action not taken - File unavailable */
      }
    fclose( pConn->file);

    /* Create a temp file */
    if ( NULL == UniqueName( pszTemp) )
      {
      strcpy( pszInfo, "Can't create temp file" );
      return 450; /* Requested file action not taken */
      }

    pConn->file = fopen( pszTemp, 'I' == pConn->type ? "wb" : "w");
    }

  if ( NULL == pConn->file)
    {
    strcpy( pszInfo, strerror( errno) );
    return 550; /* Requested file action not taken - File unavailable */
    }
  fseek( pConn->file, pConn->lSeek, SEEK_CUR);
  pConn->lSeek = 0;

  pConn->ulBytes = 0;
  pConn->pfnData = &StoreProgress;

  pConn->pDataPending = pConn->buffer;
  pConn->sizePending = (size_t)(pszPath - pConn->buffer);

  /* Initiate data connection */
  FtpConnect( pConn);
  switch ( pConn->eDataState)
    {
  case kFtpDataIdle:
  case kFtpDataAborted:
    StoreComplete( pConn, 1);
    strcpy( pszInfo, "Data connection failed.");
    iRet = 425; /* Can't open data connection */
    break;

  case kFtpDataAccepting:
  case kFtpDataConnecting:
    strcpy( pszInfo, "ok.");
    iRet = 150; /* File status okay; about to open data connection */
    break;

  case kFtpDataConnected:
  case kFtpDataWrite:
    pConn->eDataState = kFtpDataRead;
  case kFtpDataRead:
    strcpy( pszInfo, "ok.");
    iRet = 125; /* Data connection already open; transfer starting */
    break;
    }

  return iRet;
  }

/* STOR/APPE event callback */
static void StoreProgress(
  SConnection* pConn
) {
  int count;

  switch ( pConn->eDataState)
    {
  case kFtpDataAccepting:
  case kFtpDataConnecting:
    return;

  case kFtpDataConnected:
  case kFtpDataWrite:
    pConn->eDataState = kFtpDataRead;
    return; /* Wait till readable */

  case kFtpDataRead:
    break;

  case kFtpDataAborted:
    /* Connection closed; transfer aborted */
    StoreComplete( pConn, 1);
    FtpReply( pConn, 426, "Operation aborted.");
    return;

  case kFtpDataIdle:
    StoreComplete( pConn, 1);
    FtpReply( pConn, 425, "Data connection failed.");
    return;
    }

  /* Read data */
  count = ReadData( pConn, pConn->pDataPending, pConn->sizePending );
  if ( SOCKET_ERROR == count)
    {
    StoreComplete( pConn, 1);
    FtpReply( pConn, 426, "Transfer aborted.");
    }
  else if ( 0 == count)
    {
    /* Connection closed */
    char* pszPath = pConn->pDataPending + pConn->sizePending;
    char* pszTemp = pszPath + MAX_PATH;

    StoreComplete( pConn, 0);

    /* Temp file used? */
    if ( cNul != *pszTemp )
      {
      /* Yes, delete existing file */
      remove( pszPath);

      /* Rename temp file */
      if ( rename( pszTemp, pszPath ))
        {
        /* File name not allowed.*/
        FtpReply( pConn, 553, strerror( errno) );
        return;
        }
      }

    /* Operation succeeded */
    sprintf( pszPath, "%lu bytes received ok.", pConn->ulBytes);
    FtpReply( pConn, 226, pszPath);
    }
  else
    {
    pConn->ulBytes += (unsigned)count;

    if ( 'A' == pConn->type)
      {
      /* ASCII mode */
      char *pBuf = pConn->pDataPending, *pc;
      int len = count;

      /* Find CR's */
      while ( len > 0 && NULL != (pc = (char*)memchr( pBuf, cReturn, len)) )
        {
        /* Remove CR */
        len -=  1 + (pc - pBuf);
        pBuf = pc;
        memmove( pBuf, pBuf + 1, len);
        --count;
        }
      }

    /* Write to file */
    if ( 1 != fwrite( pConn->pDataPending, count, 1, pConn->file) )
      {
      /* Exceeded storage allocation */
      StoreComplete( pConn, 1);
      FtpReply( pConn, 552, "Exceeded storage allocation.");
      }
    }
  }


/* STOR cleanup */
static void StoreComplete(
  SConnection* pConn,
  int reset
) {
  pConn->eDataState = kFtpDataIdle;
  pConn->pfnData = 0;

  if ( INVALID_SOCKET != pConn->sData)
    {
    if ( reset)
      {
      /* Reset connection when closed */
      struct linger lgr;
      lgr.l_onoff = 1;
      lgr.l_linger = 0;
      setsockopt( pConn->sData, SOL_SOCKET, SO_LINGER, (char*)&lgr, sizeof( lgr));
      }

#if OPT_UPNP
    /*{int TODO_UnMapPasvPort;}*/
#endif
    closesocket( pConn->sData);
    pConn->sData = INVALID_SOCKET;
    }

  fclose( pConn->file);
  pConn->file = NULL;

  /* Delete temp file on failure */
  if ( reset && NULL != pConn->pDataPending)
    {
    const char* pszPath = pConn->pDataPending + pConn->sizePending;
    if ( cNul != pszPath[ MAX_PATH] )
      remove( pszPath + MAX_PATH);
    }

  pConn->pDataPending = NULL;
  pConn->sizePending = 0;
  }


/*
 * ABOR method
 */
int AbortMethod(
  SConnection* pConn,
  const char* pszTail,
  char* pszInfo
) {
  (void)pszTail;

  switch ( pConn->eControlState)
    {
  case kFtpControlLoggedIn:
    break;
  default:
    strcpy( pszInfo, "Not logged in.");
    return 530; /* Not logged in */
    }

  switch ( pConn->eDataState)
    {
  case kFtpDataIdle:
  case kFtpDataAccepting:
  case kFtpDataAborted:
    break;

  case kFtpDataConnecting:
  case kFtpDataConnected:
  case kFtpDataRead:
  case kFtpDataWrite:
    pConn->eDataState = kFtpDataAborted;
    if ( pConn->pfnData)
      (*pConn->pfnData)( pConn);
    break;
    }

  strcpy( pszInfo, "ok");
  return 226; /* Requested file action pending further information */
  }


/*
 * STAT method
 */
int StatMethod(
  SConnection* pConn,
  const char* pszTail,
  char* pszInfo
) {
  int i;

  if ( cNul != *pszTail)
    {
    char* pszPath = pConn->buffer;

    switch ( pConn->eControlState)
      {
    case kFtpControlLoggedIn:
      break;
    default:
      strcpy( pszInfo, "Not logged in.");
      return 530; /* Not logged in */
      }

    switch ( pConn->eDataState)
      {
    case kFtpDataIdle:
    case kFtpDataAccepting:
    case kFtpDataConnected:
      break;

    default:
      strcpy( pszInfo, "Transfer in progress.");
      return 503; /* Bad sequence of commands */
      }

    /* Disallow ".." in pathname */
    if ( strstr( pszTail, ".."))
      {
      strcpy( pszInfo, "Forbidden .. in filespec.");
      return 553; /* Requested action not taken.File name not allowed. */
      }

#if 0
    if ( pConn->ePermit < kPermitRead)
      {
      strcpy( pszInfo, "Insufficient privilege");
      return 550;
      }
#endif

    /* Form local pathname */
    MakePath( pszPath, pConn, pszTail);
    if ( cNul == *pszTail)
      strcat( pszPath, "/" kszMatchAny);

#if 0  /* Disable non-blocking mode */
    setsockasync( pConn->s, 0);
#endif

    FtpReply( pConn, 212, "-File status:");

    PlatformList( pConn, pConn->s, pszPath, 1);

#if 0 /* Restore non-blocking mode */
    setsockasync( pConn->s, 1);
#endif

    strcpy( pszInfo, "ok.");
    return 212; /* File status. */
    }

  FtpReply( pConn, 211, "-" kszBanner " status");

  if ( kFtpControlLoggedIn == pConn->eControlState)
    i = sprintf( pConn->buffer, " Logged in as %s" CRLF, pConn->szUser);
  else
    i = sprintf( pConn->buffer, " Not logged in" CRLF );

  i += sprintf( pConn->buffer + i, " TYPE %c, MODE %c, STRU %c" CRLF,
    pConn->type, pConn->mode, pConn->stru
  );

  switch ( pConn->eDataState)
    {
  case kFtpDataIdle:
  case kFtpDataAborted:
    i += sprintf( pConn->buffer + i, " No data connection" CRLF);
    break;

  case kFtpDataAccepting:
    i += sprintf( pConn->buffer + i, " Passive listen" CRLF);
    break;

  case kFtpDataConnecting:
    i += sprintf( pConn->buffer + i, " Connecting" CRLF);
    break;

  case kFtpDataConnected:
  case kFtpDataRead:
  case kFtpDataWrite:
    i += sprintf( pConn->buffer + i, " Connected, %lu bytes transferred" CRLF,
      pConn->ulBytes);
    break;
    }

  WriteControl( pConn, pConn->buffer, i);

  strcpy( pszInfo, "ok.");
  return 211; /* System status, or system help reply */
  }


/*
 * REST method
 */
int RestoreMethod(
  SConnection* pConn,
  const char* pszTail,
  char* pszInfo
) {
  char* pszEnd;

  switch ( pConn->eControlState)
    {
  case kFtpControlLoggedIn:
    break;
  default:
    strcpy( pszInfo, "Not logged in.");
    return 530; /* Not logged in */
    }

  pConn->lSeek = strtol( pszTail, &pszEnd, 10);
  if ( *pszEnd)
    {
    strcpy( pszInfo, "Invalid integer.");
    return 501; /* Syntax error in parameters or arguments */
    }

  strcpy( pszInfo, "Ready for transfer.");
  return 350; /* Requested file action pending further information */
  }


/*
 * DELE Method
 */
int DeleMethod(
  SConnection* pConn,
  const char* pszTail,
  char* pszInfo
) {
  char* pszPath = pConn->buffer;

  switch ( pConn->eControlState)
    {
  case kFtpControlLoggedIn:
    break;
  default:
    strcpy( pszInfo, "Not logged in.");
    return 530; /* Not logged in */
    }

  if ( cNul == *pszTail)
    {
    strcpy( pszInfo, "Missing filespec.");
    return 501; /* Syntax error in parameters or arguments */
    }

  /* Disallow ".." in pathname */
  if ( strstr( pszTail, ".."))
    {
    strcpy( pszInfo, "Forbidden .. in filespec.");
    return 553; /* Requested action not taken.File name not allowed. */
    }

  if ( pConn->ePermit < kPermitChange)
    {
    strcpy( pszInfo, "Insufficient privilege");
    return 550;
    }

  /* Form local pathname */
  MakePath( pszPath, pConn, pszTail);

  if ( !PlatformAccess( pszPath, kAccessWrite) )
    {
    strcpy( pszInfo, "File is read only");
    return 550;
    }

  if ( remove( pszPath))
    {
    strcpy( pszInfo, strerror( errno) );
    return 550; /* Requested file action not taken - File unavailable */
    }

  strcpy( pszInfo, "ok.");
  return 250;
  }


/*
 * NLST method
 */
int NlstMethod(
  SConnection* pConn,
  const char* pszTail,
  char* pszInfo
) {
  return ListFiles( pConn, pszTail, pszInfo, 0);
  }


/*
 * LIST method
 */
int ListMethod(
  SConnection* pConn,
  const char* pszTail,
  char* pszInfo
) {
  return ListFiles( pConn, pszTail, pszInfo, 1);
  }


static void ListComplete( SConnection*);
static FFtpData ListProgress;

int ListFiles(
  SConnection* pConn,
  const char* pszTail,
  char* pszInfo,
  int full
) {
  int iRet = 0;

  switch ( pConn->eControlState)
    {
  case kFtpControlLoggedIn:
    break;
  default:
    strcpy( pszInfo, "Not logged in.");
    return 530; /* Not logged in */
    }

  switch ( pConn->eDataState)
    {
  case kFtpDataIdle:
  case kFtpDataAccepting:
  case kFtpDataConnected:
    break;

  default:
    strcpy( pszInfo, "Transfer in progress.");
    return 503; /* Bad sequence of commands */
    }

  /* Disallow ".." in pathname */
  if ( strstr( pszTail, ".."))
    {
    strcpy( pszInfo, "Forbidden .. in filespec.");
    return 553; /* Requested action not taken.File name not allowed. */
    }

#if 0
  if ( pConn->ePermit < kPermitRead)
    {
    strcpy( pszInfo, "Insufficient privilege");
    return 550;
    }
#endif

  /* Ignore leading '-' options, used by Konqueror */
  while ( '-' == *pszTail)
    {
    /* Scan for end of options */
    char c;
    do
      c = *++pszTail;
    while ( cNul != c && !isspace( c));

    /* Skip trailing whitespace */
    while ( isspace( *pszTail))
      ++pszTail;
    }

  /* Form local pathname */
  MakePath( pConn->buffer, pConn, pszTail);
  if ( cNul == *pszTail)
    {
    if ( 0 == strcmp( pConn->buffer, kszRoot))
      strcat( pConn->buffer, kszMatchAny);
    else
      strcat( pConn->buffer, "/" kszMatchAny);
    }

  pConn->ulBytes = 0;
  pConn->pfnData = &ListProgress;
  pConn->sizePending = (size_t)full;

  /* Initiate data connection */
  FtpConnect( pConn);
  switch ( pConn->eDataState)
    {
  case kFtpDataIdle:
  case kFtpDataAborted:
    ListComplete( pConn);
    strcpy( pszInfo, "Data connection failed.");
    iRet = 425; /* Can't open data connection */
    break;

  case kFtpDataAccepting:
  case kFtpDataConnecting:
    strcpy( pszInfo, "ok.");
    iRet = 150; /* File status okay; about to open data connection */
    break;

  case kFtpDataConnected:
  case kFtpDataRead:
    pConn->eDataState = kFtpDataWrite;
  case kFtpDataWrite:
    strcpy( pszInfo, "ok.");
    iRet = 125; /* Data connection already open; transfer starting */
    break;
    }

  return iRet;
  }


/* Data event callback */
static void ListProgress(
  SConnection* pConn
) {
  int iRet;
  char* pszPath;

  switch ( pConn->eDataState)
    {
  case kFtpDataAccepting:
  case kFtpDataConnecting:
    return;

  case kFtpDataConnected:
    /* Disable non-blocking mode */
    setsockasync( pConn->sData, 0);
    /* Fall thru */
  case kFtpDataRead:
    pConn->eDataState = kFtpDataWrite;
    break;

  case kFtpDataWrite:
    break;

  case kFtpDataAborted:
    /* Connection closed; transfer aborted */
    ListComplete( pConn);
    FtpReply( pConn, 426, "Operation aborted.");
    return;

  case kFtpDataIdle:
    ListComplete( pConn);
    FtpReply( pConn, 425, "Data connection failed.");
    return;
    }

  pszPath = pConn->buffer;
  iRet = PlatformList( pConn, pConn->sData, pszPath, (int)pConn->sizePending);
  if ( SOCKET_ERROR != iRet)
    {
    iRet = 226; /* Requested file action successful. */
    strcpy( pszPath, "ok.");
    }
  else
    {
    iRet = 426; /* Connection closed; transfer aborted. */
    strcpy( pszPath, "Transfer aborted.");
    }

  ListComplete( pConn);
  FtpReply( pConn, iRet, pszPath);
  }


/* Cleanup */
static void ListComplete(
  SConnection* pConn
) {
  pConn->eDataState = kFtpDataIdle;
  pConn->pfnData = 0;

  if ( INVALID_SOCKET != pConn->sData)
    {
#if OPT_UPNP
    /*{int TODO_UnMapPasvPort;}*/
#endif
    closesocket( pConn->sData);
    pConn->sData = INVALID_SOCKET;
    }
  }


/*
 * FEAT method
 * RFC 2389
 */
static int FeatMethod(
  SConnection* pConn,
  const char* pszTail,
  char* pszInfo
) {
#if 0
  switch ( pConn->eControlState)
    {
  case kFtpControlLoggedIn:
    break;
  default:
    strcpy( pszInfo, "Not logged in.");
    return 530; /* Not logged in */
    }
#endif

  if ( *pszTail)
    {
    strcpy( pszInfo, "Unexpected parameter.");
    return 501; /* Syntax error in parameters or arguments */
    }

  FtpReply( pConn, 211, "-Extensions supported:");
  WriteControl( pConn, kszFeatures, sizeof kszFeatures - 1);
  strcpy( pszInfo, "END");
  return 211; /* System status, or system help reply */
  }


/*
 * SIZE method
 * RFC 3659
 */
static int SizeMethod(
  SConnection* pConn,
  const char* pszTail,
  char* pszInfo
) {
  FILE* file;
  char* pszPath;
  long lSize;
  int iRet;

  switch ( pConn->eControlState)
    {
  case kFtpControlLoggedIn:
    break;
  default:
    strcpy( pszInfo, "Not logged in.");
    return 530; /* Not logged in */
    }

  if ( cNul == *pszTail)
    {
    strcpy( pszInfo, "Missing filespec.");
    return 501; /* Syntax error in parameters or arguments */
    }

  /* Disallow ".." in pathname */
  if ( strstr( pszTail, ".."))
    {
    strcpy( pszInfo, "Forbidden .. in filename");
    return 553; /* Requested action not taken.File name not allowed. */
    }

#if 0
  if ( pConn->ePermit < kPermitRead)
    {
    strcpy( pszInfo, "Insufficient privilege");
    return 550;
    }
#endif

  /* Form local pathname */
  pszPath = pConn->buffer;
  MakePath( pszPath, pConn, pszTail);
  if ( PlatformIsDir( pszPath))
    {
    strcpy( pszInfo, "Directory not permitted");
    return 550; /* Requested file action not taken - File unavailable */
    }

  /* Open the file */
  file = fopen( pszPath, 'I' == pConn->type ? "rb" : "r");
  if ( NULL == file)
    {
    strcpy( pszInfo, strerror( errno) );
    return 550; /* Requested file action not taken - File unavailable */
    }

  lSize = PlatformFileLength( file);
  if ( -1 != lSize)
    {
    /* RFC 3659 p12 */
    sprintf( pszInfo, "%lu", (unsigned long)lSize);
    iRet = 213;
    }
  else
    {
    strcpy( pszInfo, strerror( errno) );
    iRet = 550;
    }

  fclose( file);
  return iRet;
  }


/*
 * MDTM method
 * RFC 3659
 */
static int MdtmMethod(
  SConnection* pConn,
  const char* pszTail,
  char* pszInfo
) {
  char* pszPath;
  time_t t;
  int iRet;

  switch ( pConn->eControlState)
    {
  case kFtpControlLoggedIn:
    break;
  default:
    strcpy( pszInfo, "Not logged in.");
    return 530; /* Not logged in */
    }

  if ( cNul == *pszTail)
    {
    strcpy( pszInfo, "Missing filespec.");
    return 501; /* Syntax error in parameters or arguments */
    }

  /* Disallow ".." in pathname */
  if ( strstr( pszTail, ".."))
    {
    strcpy( pszInfo, "Forbidden .. in filename");
    return 553; /* Requested action not taken.File name not allowed. */
    }

#if 0
  if ( pConn->ePermit < kPermitRead)
    {
    strcpy( pszInfo, "Insufficient privilege");
    return 550;
    }
#endif

  /* Form local pathname */
  pszPath = pConn->buffer;
  MakePath( pszPath, pConn, pszTail);

  t = PlatformMdtm( pszPath);
  if ( (time_t)-1 != t)
    {
    /* RFC 3659 p6 */
    strftime( pszInfo, 15, "%Y%m%d%H%M%S", gmtime( &t));
    iRet = 213;
    }
  else
    {
    strcpy( pszInfo, strerror( errno) );
    iRet = 550;
    }

  return iRet;
  }


/*
 * CWD method
 */
int CwdMethod(
  SConnection* pConn,
  const char* pszTail,
  char* pszInfo
) {
  char* pszPath = pConn->buffer;

  switch ( pConn->eControlState)
    {
  case kFtpControlLoggedIn:
    break;
  default:
    strcpy( pszInfo, "Not logged in.");
    return 530; /* Not logged in */
    }

  if ( cNul == *pszTail)
    {
    strcpy( pszInfo, "Missing pathname.");
    return 501; /* Syntax error in parameters or arguments */
    }

  if ( pConn->ePermit < kPermitExplore)
    {
    strcpy( pszInfo, "Insufficient privilege");
    return 550;
    }

  if ( 0 == strcmp( pszTail, ".."))
    return CdupMethod( pConn, "", pszInfo);

  if ( 0 == strcmp( pszTail, "."))
    {
    strcpy( pszInfo, "ok.");
    return 200;
    }

  if ( 0 == strcmp( pszTail, kszRoot)
    || 0 == strcmp( pszTail, "\\")
  ) {
    pConn->szDir[0] = cNul;
    strcpy( pszInfo, "ok.");
    return 200;
    }

  /* Disallow ".." in pathname */
  if ( strstr( pszTail, ".."))
    {
    strcpy( pszInfo, "Forbidden .. in path");
    return 553; /* Requested action not taken.File name not allowed. */
    }

  MakePath( pszPath, pConn, pszTail);

  if ( !PlatformIsDir( pszPath))
    {
    strcpy( pszInfo, "No such directory");
    return 550;
    }

  strcpy( pConn->szDir, pszPath + strlen( pConn->szHome) );

  strcpy( pszInfo, "ok.");
  return 250;
  }


/*
 * CDUP method
 */
int CdupMethod(
  SConnection* pConn,
  const char* pszTail,
  char* pszInfo
) {
  char* psz;

  switch ( pConn->eControlState)
    {
  case kFtpControlLoggedIn:
    break;
  default:
    strcpy( pszInfo, "Not logged in.");
    return 530; /* Not logged in */
    }

  if ( *pszTail)
    {
    strcpy( pszInfo, "Unexpected parameter.");
    return 501; /* Syntax error in parameters or arguments */
    }

  if ( pConn->ePermit < kPermitExplore)
    {
    strcpy( pszInfo, "Insufficient privilege");
    return 550;
    }

  if ( cNul == pConn->szDir[0] )
    {
    strcpy( pszInfo, "At top level.");
    return 550;
    }

  psz = strrchr( pConn->szDir, cSlash);
  if ( NULL == psz)
    psz = pConn->szDir;
  *psz = cNul;

  strcpy( pszInfo, "ok.");
  return 200;
  }


/*
 * PWD method
 */
static int PwdMethod(
  SConnection* pConn,
  const char* pszTail,
  char* pszInfo
) {
  switch ( pConn->eControlState)
    {
  case kFtpControlLoggedIn:
    break;
  default:
    strcpy( pszInfo, "Not logged in.");
    return 530; /* Not logged in */
    }

  if ( cNul != *pszTail)
    {
    strcpy( pszInfo, "Syntax error.");
    return 501; /* Syntax error in parameters or arguments */
    }

#if 0
  if ( pConn->ePermit < kPermitExplore)
    {
    strcpy( pszInfo, "Insufficient privilege");
    return 550;
    }
#endif

  /* RFC 959 p63 APPENDIX II */
  sprintf( pszInfo, "\"" kszRoot "%s\"", pConn->szDir);
  return 257;
  }


/*
 * MKD Method
 */
int MkdMethod(
  SConnection* pConn,
  const char* pszTail,
  char* pszInfo
) {
  char* pszPath = pConn->buffer;

  switch ( pConn->eControlState)
    {
  case kFtpControlLoggedIn:
    break;
  default:
    strcpy( pszInfo, "Not logged in.");
    return 530; /* Not logged in */
    }

  if ( cNul == *pszTail)
    {
    strcpy( pszInfo, "Missing pathname.");
    return 501; /* Syntax error in parameters or arguments */
    }

  /* Disallow ".." in pathname */
  if ( strstr( pszTail, ".."))
    {
    strcpy( pszInfo, "Forbidden .. in filespec.");
    return 553; /* Requested action not taken.File name not allowed. */
    }

  if ( pConn->ePermit < kPermitWrite)
    {
    strcpy( pszInfo, "Insufficient privilege");
    return 550;
    }

  /* Form local pathname */
  MakePath( pszPath, pConn, pszTail);
  if ( strlen( pszPath) > MAX_PATH)
    {
    strcpy( pszInfo, "Path too long.");
    return 553; /* Requested action not taken.File name not allowed. */
    }

  if ( PlatformMkdir( pszPath))
    {
    strcpy( pszInfo, strerror( errno) );
    return 550; /* Requested file action not taken - File unavailable */
    }

  /* RFC 959 p63 APPENDIX II */
  sprintf( pszInfo, "\"" kszRoot "%.240s\" directory created.", pszPath + strlen( pConn->szHome) );
  return 257;
  }


/*
 * RMD Method
 */
int RmdMethod(
  SConnection* pConn,
  const char* pszTail,
  char* pszInfo
) {
  char* pszPath = pConn->buffer;

  switch ( pConn->eControlState)
    {
  case kFtpControlLoggedIn:
    break;
  default:
    strcpy( pszInfo, "Not logged in.");
    return 530; /* Not logged in */
    }

  if ( cNul == *pszTail)
    {
    strcpy( pszInfo, "Missing pathname.");
    return 501; /* Syntax error in parameters or arguments */
    }

  /* Disallow ".." in pathname */
  if ( strstr( pszTail, ".."))
    {
    strcpy( pszInfo, "Forbidden .. in filespec.");
    return 553; /* Requested action not taken.File name not allowed. */
    }

  if ( pConn->ePermit < kPermitChange)
    {
    strcpy( pszInfo, "Insufficient privilege");
    return 550;
    }

  /* Form local pathname */
  MakePath( pszPath, pConn, pszTail);

  if ( PlatformRmdir( pszPath))
    {
    strcpy( pszInfo, strerror( errno) );
    return 550; /* Requested file action not taken - File unavailable */
    }

  strcpy( pszInfo, "ok.");
  return 250;
  }


/*
 * RNFR Method
 */
int RnfrMethod(
  SConnection* pConn,
  const char* pszTail,
  char* pszInfo
) {
  char* pszPath = pConn->buffer;

  switch ( pConn->eControlState)
    {
  case kFtpControlLoggedIn:
    break;
  default:
    strcpy( pszInfo, "Not logged in.");
    return 530; /* Not logged in */
    }

  if ( cNul == *pszTail)
    {
    strcpy( pszInfo, "Missing filename.");
    return 501; /* Syntax error in parameters or arguments */
    }

  /* Disallow ".." in pathname */
  if ( strstr( pszTail, ".."))
    {
    strcpy( pszInfo, "Forbidden .. in filespec.");
    return 553; /* Requested action not taken.File name not allowed. */
    }

  if ( pConn->ePermit < kPermitChange)
    {
    strcpy( pszInfo, "Insufficient privilege");
    return 550;
    }

  strcpy( pszPath, kszRNFR);
  pszPath += sizeof( kszRNFR);

  /* Form local pathname */
  MakePath( pszPath, pConn, pszTail);

  strcpy( pszInfo, "Proceed with RNTO.");
  return 350; /* Requested file action pending further information */
  }


/*
 * RNTO Method
 */
int RntoMethod(
  SConnection* pConn,
  const char* pszTail,
  char* pszInfo
) {
  char* pszFrom = pConn->buffer;
  char* pszTo = pConn->buffer + MAX_PATH;

  switch ( pConn->eControlState)
    {
  case kFtpControlLoggedIn:
    break;
  default:
    strcpy( pszInfo, "Not logged in.");
    return 530; /* Not logged in */
    }

  if ( cNul == *pszTail)
    {
    strcpy( pszInfo, "Missing filename.");
    return 501; /* Syntax error in parameters or arguments */
    }

  /* Disallow ".." in pathname */
  if ( strstr( pszTail, ".."))
    {
    strcpy( pszInfo, "Forbidden .. in filespec.");
    return 553; /* Requested action not taken.File name not allowed. */
    }

  /* Check valid RNFR */
  if ( strcmp( pszFrom, kszRNFR) )
    {
    strcpy( pszInfo, "RNFR required.");
    return 503; /* Bad sequence of commands. */
    }
  *pszFrom = cNul;
  pszFrom += sizeof( kszRNFR);

  if ( pConn->ePermit < kPermitChange)
    {
    strcpy( pszInfo, "Insufficient privilege");
    return 550;
    }

  /* Form local pathname */
  MakePath( pszTo, pConn, pszTail);

  if ( rename( pszFrom, pszTo) )
    {
    strcpy( pszInfo, strerror( errno) );
    return 553; /* Requested file action not taken - File name not allowed */
    }

  strcpy( pszInfo, "ok.");
  return 250;
  }


/*
 * PORT method
 */
static int PortMethod(
  SConnection* pConn,
  const char* pszTail,
  char* pszInfo
) {
  unsigned h1,h2,h3,h4,p1,p2;
  int i;
  u_long addr;

  switch ( pConn->eControlState)
    {
  case kFtpControlLoggedIn:
    break;
  default:
    strcpy( pszInfo, "Not logged in.");
    return 530; /* Not logged in */
    }

  i = sscanf( pszTail, " %u , %u , %u , %u , %u , %u",
    &h1,&h2,&h3,&h4,&p1,&p2
  );
  if ( 6 != i
    || h1 > 255
    || h2 > 255
    || h3 > 255
    || h4 > 255
    || p1 > 255
    || p2 > 255
  ) {
    strcpy( pszInfo, "Invalid parameters.");
    return 501; /* Syntax error in parameters or arguments */
    }

  addr = ((u_long)h1 << 24) | ((u_long)h2 << 16) | ((u_long)h3 << 8) | ((u_long)h4);

#if 1 /* For security ensure IP address = client address */
  if ( ntohl( pConn->clientdata.sin_addr.s_addr) != addr)
    {
    strcpy( pszInfo, "Redirection not permitted.");
    return 550; /* Requested action not taken. */
    }
#else
  pConn->clientdata.sin_addr.s_addr = htonl( addr);
#endif
  pConn->clientdata.sin_port = htons( (u_short)((p1 << 8) | p2));

  strcpy( pszInfo, "ok.");
  return 200;
  }


/*
 * PASV method
 */
static int PasvMethod(
  SConnection* pConn,
  const char* pszTail,
  char* pszInfo
) {
  struct sockaddr_in from;
  int i;
  socklen_t len;
  unsigned port;
  u_long addr;

  switch ( pConn->eControlState)
    {
  case kFtpControlLoggedIn:
    break;
  default:
    strcpy( pszInfo, "Not logged in.");
    return 530; /* Not logged in */
    }

  if ( *pszTail)
    {
    strcpy( pszInfo, "Unexpected parameter.");
    return 501; /* Syntax error in parameters or arguments */
    }

  if ( INVALID_SOCKET == pConn->sData)
    {
    /* Setup data port listener */
    SOCKET s;

    len = sizeof( from);
    if ( getsockname( pConn->s, (struct sockaddr*)&from, &len ))
      {
      strcpy( pszInfo, "Not connected.");
      return 425; /* Can't open data connection */
      }

    /* Get a socket */
    s = socket( from.sin_family, SOCK_STREAM, 0);
    if ( INVALID_SOCKET == s)
      {
      strcpy( pszInfo, "Too many connections.");
      return 425; /* Can't open data connection */
      }

    /* Bind to same adapter as client */
    from.sin_port = 0;
    if ( bind( s, (struct sockaddr*)&from, sizeof( from) ) )
      {
      closesocket( s);
      strcpy( pszInfo, "Too many open connections.");
      return 425; /* Can't open data connection */
      }

    /* Disable Nagle */
    i = 1;
    setsockopt(
      s,
      IPPROTO_TCP, TCP_NODELAY,
      (char*)&i, sizeof(i)
    );

    /* Setup a listener */
    if ( listen( s, 1) )
      {
      closesocket( s);
      strcpy( pszInfo, "Insufficient resources.");
      return 425; /* Can't open data connection */
      }

#if OPT_UPNP
    /*{int TODO_MapPasvPort;}*/
#endif
    pConn->sData = s;
    pConn->eDataState = kFtpDataAccepting;
    }

  /* Get listening address */
  len = sizeof( from);
  if ( getsockname( pConn->sData, (struct sockaddr*)&from, &len ))
    {
#if OPT_UPNP
    /*{int TODO_UnMapPasvPort;}*/
#endif
    closesocket( pConn->sData);
    pConn->sData = INVALID_SOCKET;
    strcpy( pszInfo, "System error.");
    return 425; /* Can't open data connection */
    }

  port = ntohs( from.sin_port);
  addr = ntohl( from.sin_addr.s_addr);
  sprintf( pszInfo, "Entering Passive Mode (%u,%u,%u,%u,%u,%u)",
    (unsigned)(addr >> 24),
    (unsigned)(addr >> 16) & 255,
    (unsigned)(addr >> 8) & 255,
    (unsigned)addr & 255,
    port / 256,
    port % 256
  );
  return 227; /* Entering Passive Mode */
  }


/*
 * TYPE method (required)
 * <A|E|I|L> <N|T|C>
 */
static int TypeMethod(
  SConnection* pConn,
  const char* pszTail,
  char* pszInfo
) {
  char type, form;

  switch ( pConn->eControlState)
    {
  case kFtpControlLoggedIn:
    break;
  default:
    strcpy( pszInfo, "Not logged in.");
    return 530; /* Not logged in */
    }

  type = pszTail[0];
  if ( cNul == type)
    {
    strcpy( pszInfo, "Syntax error.");
    return 501; /* Syntax error in parameters or arguments */
    }

  switch ( pszTail[1] )
    {
  case ' ':
    form = pszTail[2];
    if ( cNul != pszTail[3] )
      {
      strcpy( pszInfo, "Syntax error.");
      return 501; /* Syntax error in parameters or arguments */
      }
    break;

  case cNul:
    form = cNul;
    break;

  default:
    strcpy( pszInfo, "Syntax error.");
    return 501; /* Syntax error in parameters or arguments */
    }

  switch ( type)
    {
  case 'A': /* ASCII */
  case 'a':
    switch ( form)
      {
    case cNul:
    case 'N': /* Non-print */
    case 'n':
      pConn->format = 'N';
      break;

    case 'T': /* Telnet */
    case 't':
      pConn->format = 'T';
      break;

    case 'C': /* Carriage control */
    case 'c':
      strcpy( pszInfo, "Type not implemented.");
      return 504; /* Command not implemented for that parameter */

    default:
      strcpy( pszInfo, "Syntax error.");
      return 501; /* Syntax error in parameters or arguments */
      }
    pConn->type = 'A';
    break;

  case 'E': /* EBCDIC */
  case 'e':
    strcpy( pszInfo, "Type not implemented.");
    return 504; /* Command not implemented for that parameter */

  case 'I': /* Image (binary) */
  case 'i':
    if ( cNul != form)
      {
      strcpy( pszInfo, "Syntax error.");
      return 501; /* Syntax error in parameters or arguments */
      }
    pConn->type = 'I';
    break;

  case 'L':
  case 'l':
    if ( 8 != atoi( pszTail))
      {
      strcpy( pszInfo, "Type not implemented.");
      return 504; /* Command not implemented for that parameter */
      }
    break;

  default:
    strcpy( pszInfo, "Syntax error.");
    return 501; /* Syntax error in parameters or arguments */
    }

  strcpy( pszInfo, "ok.");
  return 200;
  }


/*
 * STRU method (required)
 * <F|R|P>
 */
static int StructureMethod(
  SConnection* pConn,
  const char* pszTail,
  char* pszInfo
) {
  switch ( pConn->eControlState)
    {
  case kFtpControlLoggedIn:
    break;
  default:
    strcpy( pszInfo, "Not logged in.");
    return 530; /* Not logged in */
    }

  switch ( pszTail[0] )
    {
  case cNul:
    strcpy( pszInfo, "Syntax error.");
    return 501; /* Syntax error in parameters or arguments */

  case 'F': /* File structure */
  case 'f':
  case 'R': /* Record structure */
  case 'r':
    if ( cNul != pszTail[1] )
      {
      strcpy( pszInfo, "Syntax error.");
      return 501; /* Syntax error in parameters or arguments */
      }
    pConn->stru = pszTail[0];
    break;

  case 'P': /* Page structure */
  case 'p':
    strcpy( pszInfo, "Type not implemented.");
    return 504; /* Command not implemented for that parameter */

  default:
    strcpy( pszInfo, "Syntax error.");
    return 501; /* Syntax error in parameters or arguments */
    }

  strcpy( pszInfo, "ok.");
  return 200;
  }


/*
 * MODE method (required)
 * <S|B|C>
 */
static int ModeMethod(
  SConnection* pConn,
  const char* pszTail,
  char* pszInfo
) {
  switch ( pConn->eControlState)
    {
  case kFtpControlLoggedIn:
    break;
  default:
    strcpy( pszInfo, "Not logged in.");
    return 530; /* Not logged in */
    }

  switch ( pszTail[0] )
    {
  case cNul:
    strcpy( pszInfo, "Missing mode parameter.");
    return 501; /* Syntax error in parameters or arguments */

  case 'S': /* Stream */
  case 's':
    if ( cNul != pszTail[1] )
      {
      strcpy( pszInfo, "Syntax error.");
      return 501; /* Syntax error in parameters or arguments */
      }
    pConn->mode = 'S';
    break;

  case 'B': /* Block */
  case 'b':
  case 'C': /* Compressed */
  case 'c':
    strcpy( pszInfo, "Parameter not implemented.");
    return 504; /* Command not implemented for that parameter */

  default:
    strcpy( pszInfo, "Syntax error.");
    return 501; /* Syntax error in parameters or arguments */
    }

  strcpy( pszInfo, "ok.");
  return 200;
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

  return 200;
  }


/*
 * SYST method
 */
static int SystemMethod(
  SConnection* pConn,
  const char* pszTail,
  char* pszInfo
) {
  (void)pszTail;

  switch ( pConn->eControlState)
    {
  case kFtpControlLoggedIn:
    break;
  default:
    strcpy( pszInfo, "Not logged in.");
    return 530; /* Not logged in */
    }

#if 1
  strcpy( pszInfo, "UNIX Type: L8");
#else
  strcpy( pszInfo, "UNKNOWN " kszBanner);
#endif
  return 215;
  }


/*
 * HELP command
 */
static int HelpCommand(
  SConnection* pConn,
  const char* pszTail,
  char* pszInfo
) {
  const SMethod* pCmd;

  if ( cNul != *pszTail)
    {
    pCmd = FindCommand( pszTail, methods, lengthof( methods));
    if ( NULL != pCmd
      && NULL != pCmd->pszHelp
    ) {
      sprintf( pszInfo, "%-5.5s %s", pCmd->pszName, pCmd->pszHelp);
      }
    else
      {
      return 501; /* Syntax error in parameters or arguments */
      }
    }
  else
    {
    size_t i;

    FtpReply( pConn, 214, "-Commands available:");

    for ( i = 0, pCmd = methods; i < lengthof( methods); ++i, ++pCmd)
      {
      if ( NULL != pCmd->pszHelp)
        {
        sprintf( pszInfo, "-%-5.5s %s", pCmd->pszName, pCmd->pszHelp);
        FtpReply( pConn, 214, pszInfo);
        }
      }
    strcpy( pszInfo, "ok.");
    }

  return 214;
  }


/*
 * SITE HELP command
 */
static int SiteHelpCommand(
  SConnection* pConn,
  const char* pszTail,
  char* pszInfo
) {
  const SMethod* pCmd;

  if ( cNul != *pszTail)
    {
    pCmd = FindCommand( pszTail, sitemethods, lengthof( sitemethods));
    if ( NULL != pCmd
      && NULL != pCmd->pszHelp
    ) {
      sprintf( pszInfo, "%-5.5s %s", pCmd->pszName, pCmd->pszHelp);
      }
    else
      {
      return 501; /* Syntax error in parameters or arguments */
      }
    }
  else
    {
    size_t i;

    FtpReply( pConn, 214, "-Commands available:");

    for ( i = 0, pCmd = sitemethods; i < lengthof( sitemethods); ++i, ++pCmd)
      {
      if ( NULL != pCmd->pszHelp)
        {
        sprintf( pszInfo, "-%-5.5s %s", pCmd->pszName, pCmd->pszHelp);
        FtpReply( pConn, 214, pszInfo);
        }
      }
    strcpy( pszInfo, "ok.");
    }

  return 214;
  }


/*
 * Unimplememted request
 */
static int UnimplementedMethod(
  SConnection* pConn,
  const char* pszTail,
  char* pszInfo
) {
  (void)pConn;
  (void)pszTail;

  strcpy( pszInfo, "Not implemented.");
  return 502; /* Command not implemented */
  }


/*
 * SITE method
 */
static int SiteMethod(
  SConnection* pConn,
  const char* pszTail,
  char* pszInfo
) {
  int iRet;
  const SMethod* pMethod;
  char szCmd[ FTP_MAX_INPUT], *pszCmd;

  /* Break into command name + tail */
  pszCmd = szCmd;
  while ( cNul != *pszTail && !isspace( *pszTail))
    {
    *pszCmd++ = (char)toupper( *pszTail);
    ++pszTail;
    }
  *pszCmd = cNul;
  while ( cNul != *pszTail && isspace( *pszTail))
    ++pszTail;

  /* Find the command */
  pMethod = FindCommand( szCmd, sitemethods, lengthof( sitemethods));
  if ( NULL != pMethod)
    {
    /* Execute the command */
    iRet = (*pMethod->pfnExecute)( pConn, pszTail, pszInfo);
    }
  else
    {
    iRet = UnimplementedMethod( pConn, pszTail, pszInfo);
    }

  return iRet;
  }


/*
 * SITE CHMOD method
 */
int ChmodMethod(
  SConnection* pConn,
  const char* pszTail,
  char* pszInfo
) {
  char* pszPath = pConn->buffer;
  char* pszEnd;
  unsigned long ulVal;

  switch ( pConn->eControlState)
    {
  case kFtpControlLoggedIn:
    break;
  default:
    strcpy( pszInfo, "Not logged in.");
    return 530; /* Not logged in */
    }

  ulVal = strtoul( pszTail, &pszEnd, 8);
  if ( 0 == ulVal)
    {
    strcpy( pszInfo, "Invalid parameter.");
    return 501; /* Syntax error in parameters or arguments */
    }

  while ( isspace( *pszEnd))
    ++pszEnd;

  if ( cNul == *pszEnd)
    {
    strcpy( pszInfo, "Missing pathname.");
    return 501; /* Syntax error in parameters or arguments */
    }

  /* Disallow ".." in pathname */
  if ( strstr( pszEnd, ".."))
    {
    strcpy( pszInfo, "Forbidden .. in filespec.");
    return 553; /* Requested action not taken.File name not allowed. */
    }

  if ( pConn->ePermit < kPermitChange)
    {
    strcpy( pszInfo, "Insufficient privilege");
    return 550;
    }

  /* Form local pathname */
  MakePath( pszPath, pConn, pszEnd);

  switch ( PlatformChmod( pszPath, (unsigned)ulVal))
    {
  case 0:
    return 200;
  case -1:
    strcpy( pszInfo, strerror( errno) );
    return 550; /* Requested file action not taken - File unavailable */
  default:
    strcpy( pszInfo, "Not implemented.");
    return 502; /* Command not implemented */
    }
  }


/****************************************************************************
 * FTP support
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
 * Validate a user
 */
static SWS_FCfgCallback EnumUserCallback;
typedef struct SUserInfo
  {
  const char* pszUser;
  char* pszHome;
  char szPassword[ 32];
  EPermission ePermission;
  in_addr_t net; /* Permitted hosts */
  unsigned uCidr; /* Classless Internet Domain Routing */
  } SUserInfo;

EPermission ValidateUser( SConnection* pConn, const char* pszPassword)
  {
  SUserInfo info;

  info.pszUser = pConn->szUser;
  info.pszHome = pConn->szHome;
  info.net = INADDR_ANY;
  info.uCidr = 32;

  if ( SWS_CfgEnum( kszFtpUsersSect, &EnumUserCallback, &info) > 0)
    {
    /* Check for host or sub-net restriction */
    if ( INADDR_ANY != info.net)
      {
      /* Always permit login from localhost */
      ui32 host = ntohl( pConn->client.sin_addr.s_addr);
      if ( INADDR_LOOPBACK != host)
        {
        /* Verify client is a permitted host or sub-net */
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

SWS_CB( int, EnumUserCallback( void* pv, const char* pszVal))
  {
  SUserInfo* pInfo = (SUserInfo*)pv;
  char szName[ USERNAME_MAX], szPermission[ 4], szNet[ 16];

  szNet[0] = cNul;

  /* Parse username, password, home and permission */
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
      pInfo->ePermission = kPermitRead;
    else if ( 0 == SWS_stricmp( "x", szPermission))
      pInfo->ePermission = kPermitExplore;
    else if ( 0 == SWS_stricmp( "w", szPermission))
      pInfo->ePermission = kPermitWrite;
    else if ( 0 == SWS_stricmp( "c", szPermission))
      pInfo->ePermission = kPermitChange;
    else
      pInfo->ePermission = kPermitNone;

    strcat( pInfo->pszHome, kszRoot);

    if ( cNul != szNet[ 0])
      pInfo->net = inet_addr( szNet);

    return 1;
    }

  return 0;
  }


/*
 * Form path
 */
char* MakePath(
  char* pszPath,
  const SConnection* pConn,
  const char* pszTail
) {
  char* psz;

  /* NB szHome is cSlash terminated */
  strcpy( pszPath, pConn->szHome);
  if ( cSlash != *pszTail && cBackSlash != *pszTail)
    {
    if ( cNul != pConn->szDir[0])
      {
      strcat( pszPath, pConn->szDir);
      strcat( pszPath, kszRoot);
      }
    strcat( pszPath, pszTail);
    }
  else
    {
    strcat( pszPath, pszTail + 1);
    }

  /* Translate cBackSlash to cSlash */
  while ( NULL != (psz = strchr( pszPath, cBackSlash)) )
    *psz = cSlash;

  /* Remove any trailing cSlash */
  psz = strrchr( pszPath, cSlash);
  if ( NULL != psz && cNul == psz[1])
    *psz = cNul;

  return pszPath;
  }


/*
 * Establish a data connection with the client
 */
static int FtpConnect(
  SConnection* pConn
) {
  if ( INVALID_SOCKET == pConn->sData)
    {
    SOCKET s;
    int iRet;
    struct sockaddr_in from;

    pConn->eDataState = kFtpDataIdle;

    /* Get a socket */
    s = socket( pConn->client.sin_family, SOCK_STREAM, 0);
    if ( INVALID_SOCKET == s)
      {
      iRet = GetLastError();
      Log( kLogWarn, &pConn->client, "WARN %d: socket: %s\n",
        iRet, strerror( iRet) );
      return 0;
      }

    /* Disable Nagle */
    iRet = 1;
    setsockopt(
      s,
      IPPROTO_TCP, TCP_NODELAY,
      (char*)&iRet, sizeof(iRet)
    );

    /* Enable address reuse */
    iRet = 1;
    setsockopt(
      s,
      SOL_SOCKET, SO_REUSEADDR,
      (char*)&iRet, sizeof(iRet)
    );

    /* Bind to local data port */
    from.sin_family = pConn->client.sin_family;
    from.sin_addr.s_addr = INADDR_ANY;
    from.sin_port = pConn->data_port;
    if ( bind( s, (struct sockaddr*)&from, sizeof( from) ) )
      {
      iRet = GetLastError();
      Log( kLogMore, &pConn->client, "Failed (%d) to bind to data port %u: %s\n",
        iRet, ntohs( from.sin_port), strerror( iRet) );
#if 0 /* Fails if daemon with non-root id */
      closesocket( s);
      return 0;
#endif
      }

    /* Enable non-blocking mode */
    setsockasync( s, 1);

    /* Initiate connection to client */
    if ( connect( s, (struct sockaddr*)&pConn->clientdata, sizeof( pConn->clientdata) )
      && (EWOULDBLOCK != (iRet = GetLastError()) && EINPROGRESS != iRet && EAGAIN != iRet)
    ) {
      Log( kLogWarn, &pConn->clientdata, "WARN %d: connect: %s\n",
        iRet, strerror( iRet) );
      closesocket( s);
      return 0;
      }

    pConn->eDataState = kFtpDataConnecting;
    pConn->sData = s;
    }

  return 1;
  }


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
        if ( pConn->telnetCount + 3 < sizeof(pConn->input))
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
    iRet = ReadControl( pConn, (char*)pvBuf + len, size - len, 0);
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

        case 242: /* Data mark (sync) */
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
        WriteControl( pConn, buf, 3);
        break;

      case kTelnetWont:
        pConn->eTelnetState = kTelnetData;
        break;

      case kTelnetDo:
        pConn->eTelnetState = kTelnetData;
        buf[0] = kTelnetIAC;
        buf[1] = kTelnetWont;
        buf[2] = c;
        WriteControl( pConn, buf, 3);
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
 * Write raw data to the control port
 */
static int WriteControl(
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
      Log( kLogWarn, &pConn->client, "WARN %d: Control connection reset in write\n", err);
      break;
    case ECONNABORTED:
      Log( kLogWarn, &pConn->client, "WARN %d: Control connection aborted in write\n", err);
      break;
    default:
      Log( kLogWarn, &pConn->client, "WARN %d: Control send: %s\n", err, strerror( err));
      break;
      }
    }

  return iRet;
  }


/*
 * Read raw data from the control port
 */
static int ReadControl(
  SConnection* pConn,
  void* pv,
  size_t size,
  int flags
) {
  int iRet;

  iRet = recv( pConn->s, (char*)pv, size, flags);
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
      Log( kLogMore, &pConn->client, "Control connection reset in read\n");
      break;
    case ECONNABORTED:
      Log( kLogWarn, &pConn->client, "WARN %d: Control connection aborted in read\n", err);
      break;
    case ENOTCONN:
      Log( kLogWarn, &pConn->client, "WARN %d: Control disconnected in read\n", err);
      break;
    default:
      Log( kLogWarn, &pConn->client, "WARN %d: Control recv failed: %s\n", err, strerror( err));
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


/*
 * Write data to the ftp client
 */
int WriteData(
  SConnection* pConn,
  SOCKET s,
  const char* pBuf,
  size_t size
) {
  int iRet;

  iRet = send( s, pBuf, size, MSG_NOSIGNAL);

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
      Log( kLogWarn, &pConn->client, "WARN %d: Data connection reset in write\n", err);
      SessionAbort( pConn);
      break;
    case ECONNABORTED:
      Log( kLogWarn, &pConn->client, "WARN %d: Data connection aborted in write\n", err);
      SessionAbort( pConn);
      break;
    default:
      Log( kLogWarn, &pConn->client, "WARN %d: Data send failed: %s\n", err, strerror( err));
      break;
      }
    }

  return iRet;
  }


/*
 * Read text data from the ftp client
 */
int ReadData(
  SConnection* pConn,
  char* pBuf,
  size_t size
) {
  int iRet;

  iRet = recv( pConn->sData, pBuf, size, 0);

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
      Log( kLogMore, &pConn->client, "Data connection reset in read\n");
      break;
    case ECONNABORTED:
      Log( kLogWarn, &pConn->client, "WARN %d: Data connection aborted in read\n", err);
      break;
    case ENOTCONN:
      Log( kLogWarn, &pConn->client, "WARN %d: Data disconnected in read\n", err);
      break;
    default:
      Log( kLogWarn, &pConn->client, "WARN %d: Data receive failed: %s\n", err, strerror( err));
      break;
      }
    }

  return iRet;
  }


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

    sprintf( szBuf, "ZF%06lu", s_u++);
    f = fopen( szBuf, "r");
    if ( NULL == f)
      return szBuf;

    fclose( f);
    }
  return NULL;
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
 * Platform specific code
 ****************************************************************************/

#if defined WIN32 || defined _WIN32 || defined __WIN32__ || defined WIN16 \
 || defined _MSDOS || defined _DOS || defined _DOS16
/****************************************************************************
 * Win32/16 & MSDOS
 ****************************************************************************/

#include <io.h> /* _findfirst, _access, _filelength _chmod */
#include <direct.h> /* _mkdir _rmdir */
#include <sys/stat.h> /* _stat _fstat */

#if defined __BORLANDC__
#define _fileno fileno
#define _filelength filelength
#define _stat stat
#define _S_IFDIR S_IFDIR

#elif defined __WATCOMC__
#define _filelength filelength
#define _rmdir rmdir
#define _chmod chmod

#elif defined __MWERKS__
#define _S_IFDIR S_IFDIR

#endif


/*
 * Send directory listing
 */
int PlatformList(
  SConnection* pConn,
  SOCKET s,
  char* pszPath,
  int full
) {
  int i = 0;
  time_t const t = time( NULL);
  int const year = gmtime( &t)->tm_year;
  const char* const pszOwner =
    kPermitChange == pConn->ePermit ? pConn->szUser : "ftp";
  struct _finddata_t info;
  long const hFind = _findfirst( pszPath, &info);

  if ( -1 ==  hFind)
    return 0;

  do
    {
    if ( 0 != strcmp( ".", info.name)
      && 0 != strcmp( "..", info.name)
    ) {
      if ( full)
        {
        char szTime[16];
        struct tm gmt = *gmtime( &info.time_write);

        if ( year == gmt.tm_year) /* month3 day2 hour2 minute2 */
          strftime( szTime, sizeof szTime, "%b %d %H:%M", &gmt);
        else
          strftime( szTime, sizeof szTime, "%b %d  %Y", &gmt);

        i = sprintf(
          pszPath,
          /* ls -la form: flgs links owner group size [m d y]|[m d h:m] name */
          "%cr%c%cr--r-- 1 %5.16s %5.16s %9lu %s %.255s" CRLF,
          (info.attrib & _A_SUBDIR) ? 'd' : '-',
          (info.attrib & _A_RDONLY) ? '-' : 'w',
          (info.attrib & _A_SUBDIR) ? 'x' : '-',
          pszOwner, pszOwner,
          (unsigned long)info.size, szTime, info.name
        );
        }
      else
        {
        i = sprintf( pszPath, "%.255s" CRLF, info.name);
        }

      i = WriteData( pConn, s, pszPath, i);
      if ( SOCKET_ERROR == i)
        break;
      }
    }
  while ( 0 == _findnext( hFind, &info) );

  _findclose( hFind);

  return i;
  }


/*
 * Test if file is writable
 */
int PlatformAccess( const char* pszPath, enum EAccessMode mode)
  {
  int i;
  switch ( mode)
    {
  case kAccessExist: i = 0; break;
  case kAccessWrite: i = 2; break;
  case kAccessRead:  i = 4; break;
  default: return 0;
    }
  return 0 == _access( pszPath, i);
  }


/*
 * Test if directory exists
 */
int PlatformIsDir( const char* pszPath)
  {
  struct _stat info;

  if ( _stat( pszPath, &info))
    return 0; /* Doesn't exist */

  return (info.st_mode & _S_IFDIR) ? 1 : 0;
  }


/*
 * Make a directory
 */
int PlatformMkdir( const char* pszPath)
  {
  return _mkdir( pszPath);
  }


/*
 * Delete a directory
 */
int PlatformRmdir( const char* pszPath)
  {
  return _rmdir( pszPath);
  }


/*
 * Get file size
 */
long PlatformFileLength( FILE* file )
  {
  return _filelength( _fileno( file));;
  }


/*
 * Set file attributes
 */
int PlatformChmod( const char* pszPath, unsigned uVal)
  {
  return _chmod( pszPath, uVal);
  }


/*
 * Get file modification time
 */
time_t PlatformMdtm( const char* pszPath)
  {
  struct _stat info;

  if ( _stat( pszPath, &info))
    return (time_t)-1; /* Doesn't exist */

  return info.st_mtime;
  }


#elif defined _POSIX_SOURCE || defined _POSIX_C_SOURCE
/****************************************************************************
 * POSIX
 ****************************************************************************/

#undef fd_set
#undef gethostname
#undef socklen_t

#include <dirent.h> /* opendir */
#include <unistd.h> /* access */
#include <sys/stat.h> /* stat fstat */


/*
 * Wild card name matching
 * Much smaller than POSIX regcomp
 */
static int NameMatch( const char* pszPattern, const char* pszName)
  {
  if ( 0 == strcmp( kszMatchAny, pszPattern))
    return 1;

  for ( ;;)
    {
    char c1, c2;

    while ( (c1 = *(pszPattern++)) == (c2 = *(pszName++)) )
      {
      if ( cNul == c1)
        return 1; /* Matched */
      }

    if ( '?' == c1)
      {
      /* '?' doesn't match a period */
      if ( cNul == c2 || '.' == c2)
        break; /* Mismatched */

      continue;
      }

    if ( '*' != c1)
      break; /* Mismatched */

    /* Match text following '*' */
    if ( NameMatch( pszPattern, pszName - 1))
      return 1; /* Matched */

    if ( cNul == c2)
      break; /* Mismatched */

#if 0 /* '*' doesn't match a period */
    if ( '.' == c2)
      break; /* Mismatched */
#endif

    --pszPattern;
    }

  return 0; /* Mismatched */
  }


/*
 * Send directory listing
 */
int PlatformList(
  SConnection* pConn,
  SOCKET s,
  char* pszPath,
  int full
) {
  int iRet = 0;
  DIR* dir;
  struct dirent* de;
  const char* pszName;
  time_t const t = time( NULL);
  int const year = gmtime( &t)->tm_year;
  const char* const pszOwner =
    kPermitChange == pConn->ePermit ? pConn->szUser : "ftp";

  /* Is the path a directory? */
  dir = opendir( pszPath);
  if ( NULL != dir)
    {
    /* Yes, all files */
    pszName = kszMatchAny;
    }
  else
    {
    /* No, split into pathname and filespec */
    char* psz = strrchr( pszPath, cSlash);
    if ( NULL == psz)
      return 0; /* No such directory */

    /* split into pathname and filespec */
    *psz = cNul;

    /* Root path? */
    if ( psz == pszPath)
      {
      /* Yes, open root */
      dir = opendir( kszRoot);
      }
    else
      {
      /* No, open pathname */
      dir = opendir( pszPath);
      }
    if ( NULL == dir)
      return 0; /* No such directory */

    pszName = ++psz;
    if ( cNul == *pszName)
      pszName = kszMatchAny;
    }

  while ( NULL != (de = readdir( dir)) )
    {
    if ( 0 != strcmp( ".", de->d_name)
      && 0 != strcmp( "..", de->d_name)
      && NameMatch( pszName, de->d_name)
    ) {
      int i;
      char szPath[ MAX_PATH + sizeof de->d_name];
      char szInfo[ 80 + sizeof de->d_name];
      struct stat sbuf;

      if ( full
        && !stat(
          strcat( strcat( strcpy( szPath, pszPath), kszRoot), de->d_name),
          &sbuf
        )
      ) {
        char szTime[16];
        struct tm gmt = *gmtime( &sbuf.st_mtime);

        if ( year == gmt.tm_year) /* month3 day2 hour2 minute2 */
          strftime( szTime, sizeof szTime, "%b %d %H:%M", &gmt);
        else
          strftime( szTime, sizeof szTime, "%b %d  %Y", &gmt);

        i = sprintf(
          szInfo,
          /* ls -la form: flgs links owner group size [m d y]|[m d h:m] name */
          "%c%c%c%c%c%c%c%c%c%c 1 %5.16s %5.16s %9lu %s %.255s" CRLF,
          S_ISDIR( sbuf.st_mode) ? 'd' : '-',
          (sbuf.st_mode & S_IRUSR) ? 'r' : '-',
          (sbuf.st_mode & S_IWUSR) ? 'w' : '-',
          (sbuf.st_mode & S_IXUSR) ? 'x' : '-',
          (sbuf.st_mode & S_IRGRP) ? 'r' : '-',
          (sbuf.st_mode & S_IWGRP) ? 'w' : '-',
          (sbuf.st_mode & S_IXGRP) ? 'x' : '-',
          (sbuf.st_mode & S_IROTH) ? 'r' : '-',
          (sbuf.st_mode & S_IWOTH) ? 'w' : '-',
          (sbuf.st_mode & S_IXOTH) ? 'x' : '-',
          pszOwner, pszOwner,
          (unsigned long)sbuf.st_size,
          szTime,
          de->d_name
        );
        }
      else
        {
        i = sprintf( szInfo, "%.255s" CRLF, de->d_name);
        }

      iRet = WriteData( pConn, s, szInfo, i);
      if ( SOCKET_ERROR == iRet)
        break;
      }
    }

  closedir( dir);

  return iRet;
  }


/*
 * Test if file is writable
 */
int PlatformAccess( const char* pszPath, enum EAccessMode mode)
  {
  int i;

  switch ( mode)
    {
  case kAccessExist: i = F_OK; break;
  case kAccessWrite: i = W_OK; break;
  case kAccessRead:  i = R_OK; break;
  default: return 0;
    }
  return 0 == access( pszPath, i);
  }


/*
 * Test if directory exists
 */
int PlatformIsDir( const char* pszPath)
  {
  struct stat info;

  if ( stat( pszPath, &info))
    return 0; /* Doesn't exist */

  return S_ISDIR( info.st_mode) ? 1 : 0;
  }


/*
 * Make a directory
 */
int PlatformMkdir( const char* pszPath)
  {
  return mkdir( pszPath,
    S_IWUSR | S_IRUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
  }


/*
 * Delete a directory
 */
int PlatformRmdir( const char* pszPath)
  {
  return rmdir( pszPath);
  }


/*
 * Get file size
 */
long PlatformFileLength( FILE* file )
  {
  struct stat info;

  if ( fstat( fileno( file), &info))
    return -1;

  return (long)info.st_size;
  }


/*
 * Set file attributes
 */
int PlatformChmod( const char* pszPath, unsigned uVal)
  {
  return chmod( pszPath, uVal);
  }


/*
 * Get file modification time
 */
time_t PlatformMdtm( const char* pszPath)
  {
  struct stat info;

  if ( stat( pszPath, &info))
    return (time_t)-1; /* Doesn't exist */

  return info.st_mtime;
  }


#elif defined __DJGPP__
/****************************************************************************
 * DJGPP
 ****************************************************************************/

#undef fd_set
#undef socklen_t
#undef gethostname

#include <dir.h> /* findfirst */
#include <io.h> /* filelength */
#include <unistd.h> /* rmdir, access */
#include <sys/stat.h> /* mkdir */
#include <dos.h> /* getftimeh */

/*
 * Send directory listing
 */
int PlatformList(
  SConnection* pConn,
  SOCKET s,
  char* pszPath,
  int full
) {
  struct ffblk info;
  int i = 0, done;
  time_t const t = time( NULL);
  int const year = gmtime( &t)->tm_year;
  const char* const pszOwner =
    kPermitChange == pConn->ePermit ? pConn->szUser : "ftp";

  done = findfirst( pszPath, &info,
    (0*FA_HIDDEN) | (0*FA_SYSTEM) | FA_RDONLY | FA_DIREC | FA_ARCH);

  while ( !done)
    {
    if ( 0 != strcmp( ".", info.ff_name)
      && 0 != strcmp( "..", info.ff_name)
    ) {
      if ( full)
        {
        static const char* const s_months[12] = {
          "Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"
        };
        int yr;

        i = sprintf(
          pszPath,
          /* ls -la form: flags  links owner group size mmm dd */
          "%cr%c%cr--r-- 1 %5.16s %5.16s %9lu %s %02u",
          (info.ff_attrib & FA_DIREC) ? 'd' : '-',
          (info.ff_attrib & FA_RDONLY) ? '-' : 'w',
          (info.ff_attrib & FA_DIREC) ? 'x' : '-',
          pszOwner, pszOwner,
          (unsigned long)info.ff_fsize,
          s_months[ (info.ff_fdate >> 5) & 0xf ],
          info.ff_fdate & 0x1f
        );

        yr = 1980 + (info.ff_fdate >> 9);
        if ( year == yr)
          {
          /* hh:mm */
          i += sprintf(
            pszPath + i,
            " %02u:%02u",
            (info.ff_ftime >> 11) & 0x1f,
            (info.ff_ftime >> 5) & 0x3f
          );
          }
        else
          {
          /* yyyy */
          i += sprintf(  pszPath + i, "  %u", yr);
          }

        /* name */
        i += sprintf( pszPath + i, " %.255s" CRLF, info.ff_name);
        }
      else
        {
        i = sprintf( pszPath, "%.255s" CRLF, info.ff_name);
        }

      i = WriteData( pConn, s, pszPath, i);
      if ( SOCKET_ERROR == i)
        break;
      }

    done = findnext( &info);
    }

  return i;
  }


/*
 * Test if directory exists
 */
int PlatformIsDir( const char* pszPath)
  {
  struct ffblk info;
  return findfirst( pszPath, &info, FA_DIREC) ? 0 : 1;
  }


/*
 * Make a directory
 */
int PlatformMkdir( const char* pszPath)
  {
  return mkdir( pszPath,
    S_IWUSR | S_IRUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
  }


/*
 * Delete a directory
 */
int PlatformRmdir( const char* pszPath)
  {
  return rmdir( pszPath);
  }


/*
 * Test if file is writable
 */
int PlatformAccess( const char* pszPath, enum EAccessMode mode)
  {
  int i;

  switch ( mode)
    {
  case kAccessExist: i = F_OK; break;
  case kAccessWrite: i = W_OK; break;
  case kAccessRead:  i = R_OK; break;
  default: return 0;
    }
  return 0 == access( pszPath, i);
  }


/*
 * Get file size
 */
long PlatformFileLength( FILE* file )
  {
  return filelength( fileno( file) );
  }


/*
 * Set file attributes
 */
int PlatformChmod( const char* pszPath, unsigned uVal)
  {
  return chmod( pszPath, uVal);
  }


/*
 * Get file modification time
 */
time_t PlatformMdtm( const char* pszPath)
  {
#if 0
  struct stat info;

  if ( stat( pszPath, &info))
    return (time_t)-1; /* Doesn't exist */

  return info.st_mtime;
#else
  struct ftime info;
  struct tm date;
  FILE* file;
  int i;

  file = fopen( pszPath, "r");
  if ( NULL == file)
    return (time_t)-1; /* Doesn't exist */

  /* Get file timestamp (localtime), years since 1980 */
  i = getftime( fileno( file), &info);
  fclose( file);
  if ( i)
    return (time_t)-1; /* Doesn't exist */

  date.tm_sec  = info.ft_tsec * 2;
  date.tm_min  = info.ft_min;
  date.tm_hour = info.ft_hour;
  date.tm_mday = info.ft_day;
  date.tm_mon  = info.ft_month - 1;
  date.tm_year = info.ft_year + 80; /* Years since 1900 */
  date.tm_wday = 0;
  date.tm_yday = 0;
  date.tm_isdst = -1;

  return mktime( &date); /* local to UTC */
#endif
  }


#else
/****************************************************************************
 * ANSI Standard C
 ****************************************************************************/

/* Send directory listing */
int PlatformList(
  SConnection* pConn,
  SOCKET s,
  char* pszPath,
  int full
) {
  (void)pConn;
  (void)s;
  (void)pszPath;
  (void)full;
  {int WARN_PlatformList_STUB;}
  return 0;
  }

/* Test if directory exists */
int PlatformIsDir( const char* pszPath)
  {
  (void)pszPath;
  return 0;
  }

/* Make a directory */
int PlatformMkdir( const char* pszPath)
  {
  (void)pszPath;
  return -1;
  }

/* Delete a directory */
int PlatformRmdir( const char* pszPath)
  {
  (void)pszPath;
  return -1;
  }

/* Test if file is accessible */
int PlatformAccess( const char* pszPath, enum EAccessMode mode)
  {
  FILE* file;
  const char* pszMode;

  switch ( mode)
    {
  case kAccessExist:
  case kAccessRead:
    pszMode = "r";
    break;

  case kAccessWrite:
    pszMode = "r+";
    break;

  default:
    return 0;
    }

  file = fopen( pszPath, pszMode);
  if ( NULL == file)
    return 0;
  fclose( file);
  return 1;
  }

/* Get file size */
long PlatformFileLength( FILE* file )
  {
  long len, pos;

  pos = ftell( file);
  if ( -1 == pos)
    return -1;

  len = 0 == fseek( file, 0, SEEK_END) ? ftell( file) : -1;
  fseek( file, pos, SEEK_SET);

  return len;
  }

/* Set file attributes */
int PlatformChmod( const char* pszPath, unsigned uVal)
  {
  (void)pszPath;
  (void)uVal;
  return 1;
  }

/* Get file modification time */
time_t PlatformMdtm( const char* pszPath)
  {
  (void)pszPath;
  return (time_t)-1; /* Doesn't exist */
  }

#endif

/* End of file */
