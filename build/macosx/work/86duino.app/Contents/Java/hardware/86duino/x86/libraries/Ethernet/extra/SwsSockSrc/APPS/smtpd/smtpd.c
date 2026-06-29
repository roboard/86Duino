/*****************************************************************************
 *
 * File name   : smtpd.c
 * Function    : SMTP server per RFC 5321 (2821)
 * Authors     : Lawrence Rust
 * Systems     : ANSI C, sockets
 *
 *****************************************************************************
 * File History
 * ------------
 *
 * 20-Jan-03, LVR, Created.
 * 07-Aug-09, LVR, v1.02 - Catch SIGPIPE
 * 22-Nov-09, LVR, v1.03 - Added AUTH
 * 23-Dec-09, LVR, v1.04 - Added Log()
 * 29-Jan-10, LVR, v1.05 - Run external script after data received
 * 25-Jul-10, LVR,       - IsDynamicHostname uses SWS_stristr
 * 11-Feb-11, LVR, v1.06 - Added max login attemps and IP lockout
 *
 *****************************************************************************/
#define kszApp "SoftMail"
#define kszVer STRING( SMTPD_VER_MAJOR) "." STRING( SMTPD_VER_MINOR)
#define kszBanner kszApp " " kszVer ", www.softsystem.co.uk"

/* Compilation options */
#ifndef OPT_MAX_CONNECTIONS
#define OPT_MAX_CONNECTIONS 3U
#endif
#if OPT_MAX_CONNECTIONS > 63
#define FD_SETSIZE (1 + OPT_MAX_CONNECTIONS)
#endif

#ifndef OPT_FORWARD
#define OPT_FORWARD 0 /* !0 to relay external mail */
#endif

#ifndef OPT_UPNP
# define OPT_UPNP 0        /* !0 to export service to UPnP */
#endif


/* Exports */
#include "smtpd.h"

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
#if _MSC_VER >= 1000
#include <crtdbg.h>
#endif

/* SwsSock portable utils */
#include "sws_net.h"
#include "sws_str.h"   /* SWS_StrMatch, SWS_stricmp  SWS_stristr */
#include "sws_dns.h"
#include "sws_cfg.h"
#include "sws_ba64.h"
#include "sws_time.h"
#if OPT_UPNP
#include "sws_upnp.h"
#endif

/* Application */
#include "sws_ansi.h"
#if OPT_FORWARD
#include "sendmail.h"
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

#define kszPrefix "SMTPD: " /* Log file prefix */

#define POLL_TIMEOUT    333U /* select timeout in millisecs */
#define SEND_TIMEOUT  60000U /* Send timeout in millisecs */
#define IDLE_TIMEOUT 120U    /* Recv idle timeout in secs. RFC 2821 4.5.3.2 says >= 300 secs */

#ifndef MSG_NOSIGNAL
#define MSG_NOSIGNAL 0        /* Flag to send() to disable SIG_PIPE */
#endif

#define BLOCKED_CLIENTS 16  /* Max number of blocked client IP's */

#define cNul       '\0'
#define cReturn    '\x0d'
#define cLinefeed  '\x0a'
#define cHyphen    '-'

#define CRLF          "\x0d\x0a"
#define kszPathSep    "/"
#define kszPostmaster "Postmaster"
#define kszDropFileExt "eml" /* NB file name extension shared with POP */

#define MAX_INPUTLINE (512+26) /* Maximum input line length. RFC2821 + RFC 1870 SIZE */
#define MAX_LINE     1000  /* Max mail body text line RFC 5321 p63 */
#define MAX_FQDN      256  /* Max hostname length inc nul */
#define MAX_MBX       256  /* Max forward or reverse pathname length inc nul */
#define MAX_RECIPIENTS 100 /* RFC 5321 p64 >= 100 */
#define MAX_IPV4       15  /* Max IP4 dotted address excl nul */
#define MAX_REPLY (MAX_FQDN + 200)  /* Max response line. RFC 5321 p63 <= 512 */
#define BUFFER_SIZE MAX_LINE /* Data buffer size */


/* SMTP RFC 5321 p51 reply codes */
#define SMTP_SYSTEM_STATUS    211
#define SMTP_HELP             214
#define SMTP_READY            220
#define SMTP_CLOSING          221
#define SMTP_AUTH_OK          235 /* RFC 4954 */
#define SMTP_OK               250
#define SMTP_FORWARDING       251
#define SMTP_NOT_VRFY         252
#define SMTP_AUTH_SASL        334 /* RFC 4954 challenge */
#define SMTP_START_MAIL       354
#define SMTP_NO_SERVICE       421
#define SMTP_UNAVAILABLE      450
#define SMTP_ABORTED          451
#define SMTP_STORAGE          452
#define SMTP_BAD_PARAM        455
#define SMTP_COMMAND_SYNTAX   500
#define SMTP_PARAM_SYNTAX     501
#define SMTP_COMMAND_UNIMPL   502
#define SMTP_BAD_SEQUENCE     503
#define SMTP_PARAM_UNIMPL     504
#define SMTP_AUTH_INVALID     535 /* RFC 4954 */
#define SMTP_NOT_TAKEN        550
#define SMTP_NOT_LOCAL        551
#define SMTP_TOO_BIG          552
#define SMTP_MBX_SYNTAX       553
#define SMTP_FAILED           554
#define SMTP_BAD_MAIL         555

/* ini file values */
#define kszMailSect       "smtp"
#define kszLogVerbosity   "log"
#define kszMaxConnections "max_conn"
#define kszIdleTimeout    "idle_time"
#define kszLoginAttempts  "login_attempts"
#define kszLockoutTime    "lockout_time"
#define kszMaxMail        "maxsize"
#define kszDenyDefer      "deny_defer"
#define kszDenyUnnamed    "deny_unnamed"
#define kszDenyBadHello   "deny_badhello"
#define kszDenyDynamicIP  "deny_dynamicip"
#if OPT_FORWARD
#define kszDefaultMx      "default_mx"
#define kszRelayMx        "relay_mx"
#endif
#define kszHostname       "hostname"
#define kszGreeting       "greeting"
#define kszBye            "bye"
#define kszVerifySender   "verify_sender"
#define kszScript         "script"

#define kszPermitHostSect "smtp_permit"
#define kszDeniedHostSect "smtp_deny"
#define kszBannedHostSect "smtp_banned_hosts"
#define kszBannedSender   "smtp_banned_sender"
#define kszMailHosts      "smtp_mail_names"
#define kszDynamicHosts   "smtp_dynamic_names"
#define kszStaticHosts    "smtp_static_names"
#define kszRelayHostSect  "smtp_relay"
#define kszMbxSect        "smtp_users"
#define kszMxSect         "smtp_mx"

#define kszPopSect        "pop_users"


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

/* Recipient info */
typedef struct SRecipient
  {
  SMailbox mbx;
  FILE* file;                   /* Drop file */
#if OPT_FORWARD
  int isRelayed;
  char szMx[ MAX_FQDN];
#endif
  } SRecipient;

/* SMTP state */
typedef enum EState
  {
  kStateUnidentified = 0,
  kStateDenied,
  kStateReset,
  kStateMail,
  kStateRcpt,
  kStateData,
  kStateAuth,
  kStateQuit
  } EState;

typedef enum EHost
  {
  kHostPermitted = 0,
  kHostUnknown,
  kHostUnnamed,
  kHostDynamic,
  kHostBadName,
  kHostMisName,
  kHostBadHello
  } EHost;

/* Connection state */
typedef struct SConnection
  {
  SDeque list;
#define LIST2CONN( p) CONTAINER( p, SConnection, list)

  SOCKET s;                     /* client connection */
  struct sockaddr_in client;    /* client's address */

  time_t tIdle;

  /* Telnet state */
  ETelnetState eTelnetState;
  int telnetCR;
  size_t telnetCount, telnetSize;
  char* pTelnetInput;           /* -> input buffer */

  /* SMTP state */
  EState eState;
  int isAuthenticated;
  const char* pszProtocol;      /* SMTP or ESMTP */
  EHost eHost;
  char szHello[ MAX_FQDN];      /* Hostname provided with HELO */
  char szHostName[ MAX_FQDN + 3 + MAX_IPV4];   /* client's "FQDN [IPv4]" */
  char szMailFrom[ MAX_MBX];    /* mail from */

  unsigned long ulRequestedSize;/* Data SIZE specified */
  unsigned long ulData;         /* Data received */
  size_t recipients;            /* recipient list size */
  SRecipient* rcptArray;        /* recipient list */

  time_t tRequest;
  const char* pszTail;

  /* Control input buffer */
  char input[ MAX_INPUTLINE];

  /* General purpose buffer */
  char buffer[ BUFFER_SIZE];
  } SConnection;


/* Command function */
typedef struct SMethod SMethod;
typedef int FMethod( SConnection*, const char*, char*);

/* Command descriptor */
struct SMethod
  {
  const char* pszName;
  FMethod* pfnExecute;
  const char* pszHelp;
  };

/* Local MX routing info */
typedef struct SMxInfo
  {
  char szDom[ MAX_FQDN];  /* Domain */
  char szMx[ MAX_FQDN];   /* Special MX, empty= use DNS */
  } SMxInfo;
struct SMxList { unsigned cnt; SMxInfo* aInfo; size_t size; };

/* Permitted/denied host IP info */
struct SHostNet { ui32 ulNet, ulMask; };
struct SHostList { unsigned cnt;  struct SHostNet* aHost; size_t size; };

/* Local mailboxes */
struct SMbxList { unsigned cnt; SMailbox* aMbx; size_t size; };

/* Banned hostnames */
struct SHostName { char szName[ MAX_FQDN]; };
struct SHostNameList { unsigned cnt;  struct SHostName* aHost; size_t size; };

/* Blocked hostt */
struct SBlockedIP
  {
  SDeque list;
#define LIST2BLOCKEDIP( p) CONTAINER( p, struct SBlockedIP, list)
  struct in_addr addr;
  time_t tBlock;
  unsigned uLoginAttempts;
  };


/***************************************************************************
 * Function prototypes
 ***************************************************************************/

static void SessionStart( SConnection*);
static void SessionFinish( SConnection*);
static void SessionInput( SConnection*);
static void SessionAbort( SConnection*);

static int ExecuteRequest( SConnection*, char*);
static const SMethod* FindCommand( const char*);
static int Response( SConnection*, int, const char*);
static void Log( int, const struct sockaddr_in*, const char [], ...) FORMAT(printf,3,4);

static int TelnetRead( SConnection*, void*, size_t);
static int TelnetReadLine( SConnection*);

static int WriteRaw( SConnection*, const void*, size_t);
static int ReadRaw( SConnection*, void*, size_t);

static void DataInput( SConnection*, size_t);
static void AuthInput( SConnection*);
static int AuthPlain( SConnection*, const char*, char*);
static void ResetTransaction( SConnection* pConn);

static int IsValidMailbox( char[ MAX_FQDN], const char*);
static SWS_FCfgCallback EnumMbxCallback;
static int GetMx( char[ MAX_FQDN], const char*);

static int IsPermittedHost( ui32 addr);
static int IsDeniedHost( ui32 addr);
static SWS_FCfgCallback EnumHostIpsCallback;

static int IsBannedHostname( const char*);
static SWS_FCfgCallback EnumHostNamesCallback;

static int IsBlockedSender( const char*);
static SWS_FCfgCallback EnumRegcmp;

static int IsDynamicHostname( const char*, u_long);
static SWS_FCfgCallback EnumStrstr;
static int cmpUnsigned(const void*, const void*);

#if OPT_FORWARD
static int IsRelayPermitted( ui32 addr);
static int RelayMessage( SConnection*, SRecipient*);
#endif
static int IsValidAcct( const char*, const char*);
static SWS_FCfgCallback EnumAcctCallback;

static int GetMxRecord( char[ MAX_FQDN], const char*);
static SWS_FCfgCallback EnumMxCallback;

static const char* GetHostname( void);
static const char* GetFQDN( char []);

static SDeque* DequeRemove( SDeque* const);
static void DequeAppend( SDeque* const pDeque, SDeque* const pItem);
#define DEQUE_INIT( p)     ((p)->tail = (p)->head = (p))
#define DEQUE_IS_EMPTY( p) ((p)->tail == (p))
#define DEQUE_FIRST( p)    ((p)->tail)
#define DEQUE_LAST( p)     ((p)->head)

static int PlatformIsDir( const char*);
static int PlatformMkdir( const char*);


/***************************************************************************
 * Module data
 ***************************************************************************/

/* Configuration */
static unsigned s_uIdleTimeout;
static unsigned s_uMaxLoginAttempts;
static unsigned s_uLockoutTime;
static int s_iDenyDefer;
static int s_iDenyUnnamed;
static int s_iDenyBadHelo;
static int s_iDenyDynamicIP;
static unsigned long s_ulMaxSize;
#if OPT_FORWARD
static char s_szDefaultMx[ MAX_FQDN];
static char s_szRelayMx[ MAX_FQDN];
static struct SHostList s_relays;
#endif
static int s_iVerifySender;
static char s_szScript[ 128];
static struct SMxList s_mx;
static struct SMbxList s_mbxs;
static struct SHostList s_permit;
static struct SHostList s_denied;
static struct SHostNameList s_banned;
static char s_szGreet[ MAX_REPLY - MAX_FQDN - 32 - 4]; /* Response less time & padding */
static char s_szBye[ MAX_REPLY - MAX_FQDN - 2];
static char s_szHostName[ MAX_FQDN];

/* Logging */
static int s_iVerbosity;
static FILE* s_logfile;

/* Operation */
extern sig_atomic_t volatile g_exit;
extern sig_atomic_t volatile g_running;
static SDeque s_free, s_active; /* Free and active connection lists */
static SDeque s_freeBlockedIP, s_activeBlockedIP; /* Free and active blocked client list */

/* The method table */
static FMethod UnimplementedMethod;
static FMethod HelpCommand;
static FMethod NoopMethod;
static FMethod QuitCommand;

static FMethod HeloMethod;
static FMethod EhloMethod;
static FMethod AuthMethod;
static FMethod MailMethod;
static FMethod RcptMethod;
static FMethod DataMethod;
static FMethod ResetMethod;
static FMethod VerifyMethod;

static const SMethod methods[] = {
  /* REQ */ { "HELO", &HeloMethod,           "<SP><FQDN> Login a new user" },
  /* EXT */ { "EHLO", &EhloMethod,           "<SP><FQDN> Login a new user" },
  /* RFC 4954 */ { "AUTH", &AuthMethod,      "<SP><[PLAIN|LOGIN]> SASL authentication" },
  /* REQ */ { "MAIL", &MailMethod,           "<SP>FROM:<[name@domain]>[<SP> params] Identify sender" },
  /* EXT */ { "SEND", &UnimplementedMethod,  "Not implemented" },
  /* EXT */ { "SOML", &UnimplementedMethod,  "Not implemented" },
  /* EXT */ { "SAML", &UnimplementedMethod,  "Not implemented" },
  /* REQ */ { "RCPT", &RcptMethod,           "<SP>TO:<name@domain> Add recipient" },
  /* REQ */ { "DATA", &DataMethod,           "Begin mail body" },
  /* REQ */ { "RSET", &ResetMethod,          "Abort current transaction" },
  /* REQ */ { "VRFY", &VerifyMethod,         "<SP> mailbox. Confirm valid mailbox" },
  /* OPT */ { "EXPN", &UnimplementedMethod,  "Not implemented" },
  /* EXT */ { "HELP", &HelpCommand,          "[<SP>cmd] Provide command line help" },
  /* REQ */ { "NOOP", &NoopMethod,           "No operation" },
  /* REQ */ { "",     &NoopMethod,           NULL },
  /* REQ */ { "QUIT", &QuitCommand,          "Terminate the session" }
};


/***************************************************************************
 * Functions
 ***************************************************************************/

/*
 * SMTP server
 */
extern int Smtpd(
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
  s_uIdleTimeout = (unsigned)SWS_CfgGetValue( kszMailSect, kszIdleTimeout, IDLE_TIMEOUT);
  s_uMaxLoginAttempts = (unsigned)SWS_CfgGetValue( kszMailSect, kszLoginAttempts, 3);
  s_uLockoutTime = (unsigned)SWS_CfgGetValue( kszMailSect, kszLockoutTime, 30);
  maxConn = (size_t)SWS_CfgGetValue( kszMailSect, kszMaxConnections, OPT_MAX_CONNECTIONS);
  s_iVerbosity = (int)SWS_CfgGetValue( kszMailSect, kszLogVerbosity, iVerbosity);
  s_logfile = logfile;

  s_ulMaxSize = SWS_CfgGetValue( kszMailSect, kszMaxMail, 0); /* 0= unlimited */
  s_iDenyDefer = 0 != SWS_CfgGetValue( kszMailSect, kszDenyDefer, 1);
  s_iDenyUnnamed = 0 != SWS_CfgGetValue( kszMailSect, kszDenyUnnamed, 0);
  s_iDenyBadHelo = 0 != SWS_CfgGetValue( kszMailSect, kszDenyBadHello, 0);
  s_iDenyDynamicIP = 0 != SWS_CfgGetValue( kszMailSect, kszDenyDynamicIP, 0);
  s_iVerifySender = 0 != SWS_CfgGetValue( kszMailSect, kszVerifySender, 1);
#if OPT_FORWARD
  SWS_CfgGetString( kszMailSect, kszDefaultMx, "", s_szDefaultMx, sizeof s_szDefaultMx );
  SWS_CfgGetString( kszMailSect, kszRelayMx, "", s_szRelayMx, sizeof s_szRelayMx );
#endif
  SWS_CfgGetString( kszMailSect, kszScript, "", s_szScript, sizeof s_szScript);
  SWS_CfgGetString( kszMailSect, kszGreeting, "ESMTP", s_szGreet, sizeof s_szGreet);
  SWS_CfgGetString( kszMailSect, kszBye, "closing", s_szBye, sizeof s_szBye);

  /* Get our hostname */
  if ( !SWS_CfgGetString( kszMailSect, kszHostname, "", s_szHostName, sizeof s_szHostName ))
    gethostname( s_szHostName, sizeof s_szHostName );

  /* Open server socket */
  s = socket( ((const struct sockaddr*)pHost)->sa_family, SOCK_STREAM, 0);
  if ( INVALID_SOCKET == s)
    {
    iRet = GetLastError();
    Log( kLogError, NULL, "ERROR %d in socket: %s\n", iRet, strerror( iRet));
    return iRet;
    }

  /* Server port specifed? */
  if ( AF_INET == ((const struct sockaddr*)pHost)->sa_family
    && 0 == ((const struct sockaddr_in*)pHost)->sin_port
  ) {
    /* No, set default */
    ((struct sockaddr_in*)pHost)->sin_port = htons( SMTPD_PORT);
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
  (void) setsockopt(
    s,
    IPPROTO_TCP, TCP_NODELAY,
    (char*)&val, sizeof val
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

  if ( maxConn + 1 > FD_SETSIZE)
    {
    Log( kLogError, NULL, "ERROR %u connections greater than compilation limit of %u\n",
      (unsigned)maxConn, FD_SETSIZE - 1);
    closesocket( s);
    return EXIT_FAILURE;
    }

  /* Allocate free connection list */
  pConnArray = (SConnection*) calloc( maxConn, sizeof( SConnection) );
  if ( NULL == pConnArray)
    {
    Log( kLogError, NULL, "ERROR %d in calloc for %u connections\n", errno, (unsigned)maxConn);
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

  /* Build dynamic config lists */
#if OPT_FORWARD
  SWS_CfgEnum( kszRelayHostSect, &EnumHostIpsCallback, &s_relays);
#endif
  SWS_CfgEnum( kszMxSect, &EnumMxCallback, &s_mx);
  SWS_CfgEnum( kszMbxSect, &EnumMbxCallback, &s_mbxs);
  SWS_CfgEnum( kszPermitHostSect, &EnumHostIpsCallback, &s_permit);
  SWS_CfgEnum( kszDeniedHostSect, &EnumHostIpsCallback, &s_denied);
  SWS_CfgEnum( kszBannedHostSect, &EnumHostNamesCallback, &s_banned);

#if OPT_UPNP
  {
  unsigned uExtPort = SWS_CfgGetValue( kszMailSect, "UpnpExtPort", 0);
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
    static fd_set setRd;
    time_t t;
    struct timeval tval;

    FD_ZERO( &setRd);
    if ( !DEQUE_IS_EMPTY( &s_free))
      FD_SET( s, &setRd);

    /* Add active sockets to the select sets */
    for (
      pDeque = DEQUE_FIRST( &s_active);
      pDeque != &s_active;
      pDeque = DEQUE_FIRST( pDeque)
    ) {
      pConn = LIST2CONN( pDeque);

      /* Receive data */
      FD_SET( pConn->s, &setRd);
      }

    /* Set timeout for select */
    tval.tv_usec = (POLL_TIMEOUT % 1000) * 1000UL;
    tval.tv_sec = POLL_TIMEOUT / 1000;

    /* Wait for listen event */
    iRet = select( FD_SETSIZE, &setRd, NULL, NULL, &tval);
    if ( SOCKET_ERROR == iRet)
      continue;

    t = time( NULL);

    if ( FD_ISSET( s, &setRd))
      {
      socklen_t len;

      /* Incoming connection */
      pConn = LIST2CONN( DequeRemove( DEQUE_FIRST( &s_free) ));

      pConn->tIdle = t + s_uIdleTimeout; /* Start the idle timer */

      pConn->eTelnetState = kTelnetData;
      pConn->telnetCR = 0;
      pConn->telnetCount = 0;
      pConn->telnetSize = 0;
      pConn->pTelnetInput = NULL;

      /* SMTP state */
      pConn->eState = kStateUnidentified;
      pConn->isAuthenticated = 0;
      pConn->pszProtocol = NULL;
      pConn->eHost = kHostUnknown;
      pConn->ulRequestedSize = 0;
      pConn->rcptArray = NULL;
      pConn->recipients = 0;

      /* Accept the incoming connection */
      len = sizeof( pConn->client);
      pConn->s = accept( s, (struct sockaddr*)&pConn->client, &len);
      if ( INVALID_SOCKET == pConn->s)
        {
        iRet = GetLastError();
        Log( kLogWarn, NULL, "ERROR %d in accept: %s\n", iRet, strerror( iRet));
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
        pConn->tRequest = t;
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

  /* Free dynamic config lists */
  free( s_banned.aHost);
  free( s_denied.aHost);
  free( s_permit.aHost);
  free( s_mbxs.aMbx);
  free( s_mx.aInfo);
#if OPT_FORWARD
  free( s_relays.aHost);
#endif

  free( pBlockList);
  free( pConnArray);
  closesocket( s);

  return iRet;
  }


/*
 * Search blocked IP list for client
 */
static struct SBlockedIP* FindBlockedIP( SConnection* pConn)
  {
  SDeque* pDeque, *pNext;
  time_t const t = time( NULL);

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
      return pBlock;
    }
  return NULL;
  }


/*
 * Start a session
 */
void SessionStart( SConnection* pConn)
  {
  char* const pszResponse = pConn->buffer + (sizeof pConn->buffer - MAX_REPLY);
  struct SBlockedIP* pBlock;
  int iResponse;
  ui32 ulHost;

  strcpy( pConn->input, "OPEN");
  pConn->pszTail = "";
  pConn->pTelnetInput = pConn->input;
  pConn->telnetSize = sizeof pConn->input;

  /* Check if the client IP is blocked */
  pBlock = FindBlockedIP( pConn);
  if ( NULL != pBlock
    && pBlock->uLoginAttempts >= s_uMaxLoginAttempts
  ) {
    pConn->eState = kStateDenied;
    Response( pConn, SMTP_FAILED, "SMTP service denied due to abuse.");
    /*pBlock->tBlock = t + 60 * s_uLockoutTime;*/
    SessionAbort( pConn);
    return;
    }

  ulHost = ntohl( pConn->client.sin_addr.s_addr);
  pConn->eHost = !IsPermittedHost( ulHost) ? kHostUnknown : kHostPermitted;
  if ( kHostPermitted == pConn->eHost
    || !IsDeniedHost( ulHost)
  ) {
    char* pszBuf;

    iResponse = SMTP_READY;
#if 0
    sprintf( pszResponse, "-%."STRING(MAX_FQDN)"s", GetFQDN( pConn->buffer));
    Response( pConn, iResponse, pszResponse);
    pszBuf = pszResponse;
    pszBuf += sprintf( pszBuf, "%s; ", s_szGreet);
#else
    pszBuf = pszResponse;
    pszBuf += sprintf( pszBuf, "%."STRING(MAX_FQDN)"s %s; ",
      GetFQDN( pConn->buffer), s_szGreet);
#endif

    SWS_TimeToStr( pszBuf, 32, pConn->tRequest);
    }
  else
    {
    /* Host denied */
    pConn->eState = kStateDenied;
    iResponse = SMTP_FAILED;
    strcpy( pszResponse, "SMTP service denied");
    }

  /* Send sign-on greeting */
  Response( pConn, iResponse, pszResponse);
  }


/*
 * End a session
 */
static void SessionFinish( SConnection* pConn)
  {
  Log( kLogInfo, &pConn->client, "CLOSE\n");
  ResetTransaction( pConn);

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
  lgr.l_onoff = 1; /* 1= enable */
  lgr.l_linger = 0; /* seconds, 0= none */
  setsockopt( pConn->s, SOL_SOCKET, SO_LINGER, (const char*)&lgr, sizeof lgr);

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
  else if ( kStateData == pConn->eState)
    {
    /* Receive data, discounting trailing CRLF */
    pConn->pszTail = "";
    pConn->ulData += iRet;
    DataInput( pConn, (size_t)(iRet - 2));
    }
  else if ( kStateAuth == pConn->eState)
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
 * Execute an SMTP request
 */
static int ExecuteRequest(
  SConnection* pConn,
  char* pszCmd
) {
  char* const pszResponse = pConn->buffer + (sizeof pConn->buffer - MAX_REPLY);
  char *pszTail;
  const SMethod* pMethod;
  int iRet;

  pConn->tRequest = time( NULL);

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

  pszResponse[0] = cNul;

  /* Find the command */
  pMethod = FindCommand( pszCmd);
  if ( NULL != pMethod)
    {
    /* Execute the command */
    Log( kLogMore, &pConn->client, "%s %s\n", pszCmd, pszTail);
    iRet = (*pMethod->pfnExecute)( pConn, pszTail, pszResponse);
    }
  else
    {
    strcpy( pszResponse, "Command not recognized");
    iRet = SMTP_COMMAND_SYNTAX; /* Syntax error, command unrecognized */
    }

  return Response( pConn, iRet, pszResponse);
  }


/*
 * Find a command descriptor given a string
 */
const SMethod* FindCommand(
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
 * Send an SMTP response
 */
static int Response(
  SConnection* pConn,
  int code,
  const char* pszInfo
) {
  int iLen;
  char const prefix = (char)(cHyphen != *pszInfo ? ' ' : (++pszInfo, cHyphen));
  const char* const pszSep = cNul != pConn->pszTail[ 0] ? " " : "";

#define HILITE( _s) ANSI_SGR( ANSI_SGR_FG_RED ";" ANSI_SGR_BOLD) _s\
  ANSI_SGR( ANSI_SGR_NRML ";" ANSI_SGR_RESET ";" ANSI_SGR_FG_WHT ";" ANSI_SGR_FG_RST)

  /* Log the request and final response */
  Log( code >= 400 ? kLogWarn : cHyphen != prefix ? kLogInfo : kLogMore,
    &pConn->client,
    code >= 400 ? HILITE("%s%s%s ; %d%c%s") "\n" : "%s%s%s ; %d%c%s\n",
    pConn->input, pszSep, pConn->pszTail, code, prefix, pszInfo
  );

#if 1 /* Non-conforming but give spammers a hard time */
  if ( kStateDenied == pConn->eState
    && SMTP_BAD_SEQUENCE == code
  ) {
    SessionAbort( pConn); /* Reset & close the connection */

    iLen = sprintf( pConn->buffer, "5#$-");
    for ( ; (size_t)iLen < sizeof pConn->buffer; ++iLen)
      pConn->buffer[ iLen] = (char)rand();
    }
  else
#endif
    {
    STATIC_ASSERT( sizeof pConn->buffer >= 1000);
    iLen = sprintf( pConn->buffer, "%d%c%.993s" CRLF, code, prefix, pszInfo);
    }

  setsockasync( pConn->s, 0); /* Disable non-blocking mode */
  (void)WriteRaw( pConn, pConn->buffer, iLen);
  setsockasync( pConn->s, 1); /* Restore non-blocking mode */

  if ( SMTP_NO_SERVICE == code)
    pConn->eState = kStateQuit;

  return code;
  }


/****************************************************************************
 * SMTP command handlers
 ****************************************************************************/

/*
 * HELO command
 */
static int HeloMethod(
  SConnection* pConn,
  const char* pszTail,
  char* pszInfo
) {
  struct hostent* pHost;

  switch ( pConn->eState)
    {
  case kStateDenied:
    strcpy( pszInfo, "No SMTP service here");
    return SMTP_BAD_SEQUENCE; /* Bad sequence of commands */

  default:
    break;
    }

  /* Ensure hostname present */
  if ( cNul == *pszTail)
    {
    strcpy( pszInfo, "Syntax: HELO hostname");
    return SMTP_PARAM_SYNTAX; /* Syntax error */
    }

  if ( strlen( pszTail) >= sizeof( pConn->szHello))
    {
    strcpy( pszInfo, "Hostname too long");
    return SMTP_PARAM_SYNTAX; /* Syntax error */
    }

  ResetTransaction( pConn); /* Sets pConn->eState to kStateReset */
  strcpy( pConn->szHello, pszTail);
  pConn->pszProtocol = "SMTP";

  /* Is hello a valid FQDN? */
  if ( INADDR_NONE != inet_addr( pConn->szHello))
    pHost = NULL; /* No, it's an IP address */
  else if ( NULL != (pHost = gethostbyname( pConn->szHello)))
    {
    int i;
    /* Yes, does client match any of the known address(es) */
    for ( i = 0; NULL != pHost->h_addr_list[i]; ++i)
      {
      if ( ((const struct in_addr*)pHost->h_addr_list[i])->s_addr
        == pConn->client.sin_addr.s_addr
      )
        break;
      }
    if ( NULL == pHost->h_addr_list[i])
      pHost = NULL;
    }

  if ( NULL == pHost)
    {
    /* Reverse DNS to get client's FQDN */
    pHost = gethostbyaddr(
      (const char*)&pConn->client.sin_addr.s_addr,
      sizeof( pConn->client.sin_addr.s_addr),
      pConn->client.sin_family
    );
    if ( NULL != pHost
      && kHostUnknown == pConn->eHost
      && s_iDenyUnnamed
#if 0
      /* Don't check for canonical DNS if known mail server */
      && 0 >= SWS_CfgEnum( kszMailHosts, &EnumRegcmp, (void*)pHost->h_name)
#endif
    ) {
      /* Verify that the hostname resolves - dynamic names often fail */
      pConn->eHost = kHostBadName;
      pHost = gethostbyname( pHost->h_name);
      if ( NULL != pHost)
        {
        int i;
        /* Does client match any of the known address(es) */
        pConn->eHost = kHostMisName;
        for ( i = 0; NULL != pHost->h_addr_list[i]; ++i)
          {
          /* Match address exluding lsb */
          if ( (ntohl(((const struct in_addr*)pHost->h_addr_list[i])->s_addr) & ~0xffL)
            == (ntohl(pConn->client.sin_addr.s_addr) & ~0xffL)
          ) {
            pConn->eHost = kHostUnknown;
            break;
            }
          }
        }

      /* Restore reverse DNS info */
      pHost = gethostbyaddr(
        (const char*)&pConn->client.sin_addr.s_addr,
        sizeof( pConn->client.sin_addr.s_addr),
        pConn->client.sin_family
      );

      if ( !s_iDenyDefer) switch ( pConn->eHost)
        {
      case kHostBadName:
        pConn->eState = kStateDenied;
        sprintf( pszInfo, "SMTP service denied. No canonical client DNS %."STRING(MAX_FQDN)"s",
          pHost->h_name);
        break;
      case kHostMisName:
        pConn->eState = kStateDenied;
        sprintf( pszInfo, "SMTP service denied. Mismatched canonical DNS %."STRING(MAX_FQDN)"s",
          pHost->h_name);
        break;
      default:
        break;
        }
      }
    }

  if ( NULL != pHost)
    {
    STATIC_ASSERT( sizeof pConn->szHostName >= MAX_FQDN + MAX_IPV4 + 3);
    sprintf( pConn->szHostName, "%."STRING(MAX_FQDN)"s [%."STRING(MAX_IPV4)"s]",
      pHost->h_name, inet_ntoa( pConn->client.sin_addr) );

    /* Check if hostname is banned */
    if ( kHostPermitted != pConn->eHost
      && IsBannedHostname( pHost->h_name)
    ) {
      pConn->eHost = kHostBadName;
      pConn->eState = kStateDenied;
      sprintf( pszInfo, "SMTP service denied. Banned client hostname %."STRING(MAX_FQDN)"s",
        pHost->h_name);
      }
    else if ( kHostUnknown == pConn->eHost
      && s_iDenyDynamicIP
      && IsDynamicHostname( pHost->h_name, ntohl( pConn->client.sin_addr.s_addr))
    ) {
      pConn->eHost = kHostDynamic;
      if ( !s_iDenyDefer)
        {
        pConn->eState = kStateDenied;
        sprintf( pszInfo, "SMTP service denied. Dynamic client hostname %."STRING(MAX_FQDN)"s",
          pHost->h_name);
        }
      }
    }
  else
    {
    STATIC_ASSERT( sizeof pConn->szHostName > MAX_FQDN + 2);
    sprintf( pConn->szHostName, "[%."STRING(MAX_FQDN)"s]", inet_ntoa( pConn->client.sin_addr) );

    if ( kHostUnknown == pConn->eHost)
      {
      pConn->eHost = kHostUnnamed;
      if ( s_iDenyUnnamed && !s_iDenyDefer)
        {
        pConn->eState = kStateDenied; /* Deny hosts with no domain name */
        strcpy( pszInfo, "SMTP service denied. No reverse DNS for client");
        }
      }
    }

  if ( kStateReset != pConn->eState)
    return SMTP_NOT_TAKEN; /* Requested action not taken: mailbox unavailable */

  if ( kHostUnknown == pConn->eHost && s_iDenyBadHelo)
    {
    /* Reject hosts that don't supply a matching domain name.
     * This prevent virus infected PCs (often using DHCP) sending mail directly.
     *
     * NB This is contrary to RFC2821 4.1.4 para6 pg39
     */

    /* Is it an address literal? (4.1.1.1 and 4.1.3) */
    if ( NULL == SWS_stristr( pConn->szHello, inet_ntoa( pConn->client.sin_addr)) )
      {
      /* Does it match the official hostname? */
      if ( NULL != pHost
        && SWS_stricmp( pConn->szHello, pHost->h_name)
      ) {
        /* No, try aliases */
        int i;
        for ( i = 0; NULL != pHost->h_aliases[i]; ++i)
          {
          if ( 0 == SWS_stricmp( pConn->szHello, pHost->h_aliases[i]))
            goto valid;
          }

        pConn->eHost = kHostBadHello;
        if ( !s_iDenyDefer)
          {
          sprintf( pszInfo, "SMTP service denied. Client reverse DNS %."STRING(MAX_FQDN)"s != HELO name",
            pHost->h_name);
          return SMTP_NOT_TAKEN; /* Requested action not taken: mailbox unavailable */
          }
        }
    valid:
      ;
      }
    }

  /* Format response */
  sprintf( pszInfo, "%."STRING(MAX_FQDN)"s Hello %s", GetHostname(), pConn->szHostName);

  return SMTP_OK; /* Requested mail action okay, completed */
  }


/*
 * EHLO command
 */
static int EhloMethod(
  SConnection* pConn,
  const char* pszTail,
  char* pszInfo
) {
  int iRet;

  if ( cNul == *pszTail)
    {
    strcpy( pszInfo, "Syntax: EHLO hostname");
    return SMTP_PARAM_SYNTAX; /* Syntax error */
    }

  *pszInfo = cHyphen;
  iRet = HeloMethod( pConn, pszTail, pszInfo + 1);
  if ( SMTP_OK == iRet)
    {
    char* psz;

    pConn->pszProtocol = "ESMTP";

    psz = pszInfo;
    psz += strlen( psz);
    psz += sprintf( psz, CRLF
      "250-HELP" CRLF
      "250-VRFY" CRLF
      "250-AUTH PLAIN LOGIN" CRLF
      "250-8BITMIME" CRLF    /* RFC 1652 */
      "250-PIPELINING" CRLF  /* RFC 2920 */
      "250-SIZE"             /* RFC 1870 */
    );
    if ( s_ulMaxSize > 0)
      sprintf( psz, " %lu", s_ulMaxSize);

    Response( pConn, SMTP_OK, pszInfo);

    sprintf( pszInfo, "OK %s", pConn->szHostName);
    }
  else
    {
    memmove( pszInfo, pszInfo + 1, 1 + strlen( pszInfo));
    }

  return iRet;
  }


/*
 * AUTH command. RFC 4954 (obsoletes 2554)
 */
static int AuthMethod(
  SConnection* pConn,
  const char* pszTail,
  char* pszInfo
) {
  switch ( pConn->eState)
    {
  case kStateUnidentified:
    strcpy( pszInfo, "HELO or EHLO required");
    return SMTP_BAD_SEQUENCE; /* Bad sequence of commands */

  case kStateDenied:
    strcpy( pszInfo, "No SMTP service here");
    return SMTP_BAD_SEQUENCE; /* Bad sequence of commands */

  case kStateReset:
    break;

  case kStateMail:
  case kStateRcpt:
  case kStateData:
  case kStateAuth:
  default:
    return SMTP_BAD_SEQUENCE; /* Bad sequence of commands */
    }

  if ( pConn->isAuthenticated)
    return SMTP_BAD_SEQUENCE; /* Bad sequence of commands */

  if ( 0 == SWS_strnicmp( pszTail, "PLAIN", 5))
    {
    if ( cNul != pszTail[5] && !isspace( pszTail[5]) )
      {
      strcpy( pszInfo, "Syntax: AUTH PLAIN [text]");
      return SMTP_PARAM_SYNTAX; /* Syntax error */
      }
    pszTail += 5;
    while ( isspace( *pszTail))
      ++pszTail;

    if ( cNul == *pszTail)
      {
      /* Return NULL server challenge, await initial-response */
      pConn->eState = kStateAuth;
      strcpy( pszInfo, " ");
      return SMTP_AUTH_SASL;
      }

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
      return SMTP_PARAM_SYNTAX; /* Syntax error */
      }

    /* Return server challenge in base64 */
    strcpy( pszInfo, "VXNlcm5hbWU6"); /* "Username:" */
    pConn->isAuthenticated = -1; /* Expect username */
    pConn->eState = kStateAuth;
    return SMTP_AUTH_SASL;
    }
  else
    {
    strcpy( pszInfo, "Mechanism not supported");
    return SMTP_PARAM_UNIMPL; /* Command parameter not implemented */
    }
  }


/*
 * Handle an unsuccessful AUTH
 */
static int BadLogin( SConnection* pConn, char* pszInfo)
  {
  if ( s_uMaxLoginAttempts > 0)
    {
    struct SBlockedIP* pBlock = FindBlockedIP( pConn);
    if ( NULL != pBlock)
      pBlock->uLoginAttempts++;
    else
      {
      if ( !DEQUE_IS_EMPTY( &s_freeBlockedIP))
        pBlock = LIST2BLOCKEDIP( DequeRemove( DEQUE_FIRST( &s_freeBlockedIP) ));
      else if ( !DEQUE_IS_EMPTY( &s_activeBlockedIP))
        pBlock = LIST2BLOCKEDIP( DequeRemove( DEQUE_LAST( &s_activeBlockedIP) ));
      else
        goto errx;

      pBlock->addr = pConn->client.sin_addr;
      pBlock->tBlock = time( NULL) + s_uLockoutTime * 60;
      pBlock->uLoginAttempts = 1;
      DequeAppend( &s_activeBlockedIP, &pBlock->list);
      }

    if ( pBlock->uLoginAttempts >= s_uMaxLoginAttempts)
      {
      if ( s_uLockoutTime > 0)
        {
        sprintf( pszInfo,
          "Authentication failed. Too many attempts - client blocked for %u minutes",
          s_uLockoutTime
        );
        pBlock->tBlock = time( NULL) + s_uLockoutTime * 60;
        }
      else
        {
        strcpy( pszInfo, "Authentication failed. Too many attempts - client blocked");
        pBlock->tBlock = 0;
        }
      return SMTP_NO_SERVICE;
      }
    }

errx:
  strcpy( pszInfo, "Authentication failed");
  return SMTP_AUTH_INVALID; /* Command parameter not implemented */
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
    strcpy( pszInfo, "Invalid base64 text");
    return SMTP_PARAM_SYNTAX; /* Syntax error */
    }
  memset( pConn->buffer + len, cNul, 3);

  /* Skip optional authorization identity RFC 4616 */
  pszAuthcid = pConn->buffer;
  if ( cNul != *pszAuthcid)
    pszAuthcid += 1 + strlen( pszAuthcid);
  else
    ++pszAuthcid;

  len = 1 + strlen( pszAuthcid);
  if ( !IsValidAcct( pszAuthcid, pszAuthcid + len))
    {
    Log( kLogWarn, &pConn->client, "WARN: AuthPlain failed:%.255s:%.255s:\n",
        pszAuthcid, pszAuthcid + len);
    return BadLogin( pConn, pszInfo);
    }
  Log( kLogMore, &pConn->client, "AuthPlain acct:%.255s\n", pszAuthcid);

  pConn->isAuthenticated = 1;
  pConn->eHost = kHostPermitted;
  strcpy( pszInfo, "Authentication successful");
  return SMTP_AUTH_OK;
  }


/*
 * Process AUTH response
 */
static void AuthInput( SConnection* pConn)
  {
  char* const pszResponse = pConn->buffer + (sizeof pConn->buffer - MAX_REPLY);
  int iRet;
  size_t len;

  switch ( pConn->isAuthenticated)
    {
  case 0:
    pConn->eState = kStateReset;
    iRet = AuthPlain( pConn, pConn->pTelnetInput, pszResponse);
    Response( pConn, iRet, pszResponse);
    break;

  case -1: /* Login username */
    len = SWS_Base64Decode(
      (unsigned char*)pConn->szMailFrom, sizeof pConn->szMailFrom - 1,
      pConn->pTelnetInput
    );
    pConn->szMailFrom[ len] = cNul;
    Log( kLogMore, &pConn->client, "AuthLogin user:%s\n", pConn->szMailFrom);
    pConn->isAuthenticated = -2; /* Expect password */
    Response( pConn, SMTP_AUTH_SASL, "UGFzc3dvcmQ6"); /* Password: */
    break;

  case -2: /* Login password */
    pConn->eState = kStateReset;
    len = SWS_Base64Decode(
      (unsigned char*)pConn->buffer, sizeof pConn->buffer - 1, pConn->pTelnetInput);
    pConn->buffer[ len] = cNul;
    if ( !IsValidAcct( pConn->szMailFrom, pConn->buffer))
      {
      Log( kLogWarn, &pConn->client, "WARN: AuthLogin failed:%.255s:%.255s:\n",
        pConn->szMailFrom, pConn->buffer);
      pConn->isAuthenticated = 0;
      iRet = BadLogin( pConn, pszResponse);
      Response( pConn, iRet, pszResponse);
      }
    else
      {
      /*Log( kLogMore, &pConn->client, "AuthLogin password:%.255s:\n",pConn->buffer);*/
      pConn->isAuthenticated = 1;
      pConn->eHost = kHostPermitted;
      Response( pConn, SMTP_AUTH_OK, "Authentication successful");
      }
    break;

  default:
    pConn->isAuthenticated = 0;
    pConn->eState = kStateReset;
    Response( pConn, SMTP_AUTH_INVALID, "Authentication failed");
    break;
    }
  }


/*
 * MAIL command
 */
static int MailMethod(
  SConnection* pConn,
  const char* pszTail,
  char* pszInfo
) {
  char* pszEnd;

  switch ( pConn->eState)
    {
  case kStateUnidentified:
    strcpy( pszInfo, "HELO or EHLO required");
    return SMTP_BAD_SEQUENCE; /* Bad sequence of commands */

  case kStateDenied:
    strcpy( pszInfo, "No SMTP service here");
    return SMTP_BAD_SEQUENCE; /* Bad sequence of commands */

  case kStateReset:
  case kStateMail:
  case kStateRcpt:
    break;

  case kStateData:
  case kStateAuth:
  default:
    return SMTP_BAD_SEQUENCE; /* Bad sequence of commands */
    }

  /* Validate syntax and extract sender's address */
  if ( SWS_strnicmp( pszTail, "FROM", 4))
    {
    strcpy( pszInfo, "Expected 'FROM'. Syntax: MAIL FROM:<name@domain>");
    return SMTP_PARAM_SYNTAX; /* Syntax error */
    }
  pszTail += 4;

  while ( isspace( *pszTail))
    ++pszTail;

  if ( ':' != *pszTail)
    {
    strcpy( pszInfo, "Expected ':'. Syntax: MAIL FROM:<name@domain>");
    return SMTP_PARAM_SYNTAX; /* Syntax error */
    }
  ++pszTail;

  while ( isspace( *pszTail))
    ++pszTail;

  if ( '<' != *pszTail)
    {
    strcpy( pszInfo, "Expected '<'. Syntax: MAIL FROM:<name@domain>");
    return SMTP_PARAM_SYNTAX; /* Syntax error */
    }
  ++pszTail;

  pszEnd = (char*)strchr( pszTail, '>');
  if ( NULL == pszEnd)
    {
    strcpy( pszInfo, "Expected '>'. Syntax: MAIL FROM:<name@domain>");
    return SMTP_PARAM_SYNTAX; /* Syntax error */
    }
  else if ( (size_t)(pszEnd - pszTail) > sizeof pConn->szMailFrom)
    {
    strcpy( pszInfo, "Return path too long");
    return SMTP_PARAM_SYNTAX; /* Path too long */
    }
  strncpy( pConn->szMailFrom, pszTail, pszEnd - pszTail);
  pConn->szMailFrom[ pszEnd - pszTail] = cNul;
  ++pszEnd;

  /* Validate return path */
  if ( kHostPermitted != pConn->eHost)
    {
    if ( IsBlockedSender( pConn->szMailFrom))
      {
#if 1 /* Deny service to spammers */
      pConn->eState = kStateDenied;
#endif
      strcpy( pszInfo, "Sender blocked");
      return SMTP_NOT_TAKEN; /* mailbox unavailable */
      }
    else if ( s_iVerifySender
      && cNul != pConn->szMailFrom[0]           /* Empty path  */
      && SWS_stricmp( kszPostmaster, pConn->szMailFrom)  /* or Postmaster */
      && !IsValidMailbox( NULL, pConn->szMailFrom)  /* or valid MX rec */
    ) {
      strcpy( pszInfo, "Sender invalid");
      return SMTP_NOT_TAKEN; /* mailbox unavailable */
      }
    }

  ResetTransaction( pConn); /* Sets pConn->eState to kStateReset */

  /* Parse parameters */
  while ( isspace( *pszEnd))
    ++pszEnd;
  while ( '\0' != *pszEnd)
    {
    unsigned long ulVal;
    int count = 0;

    /* RFC 1652: "BODY=7BIT" "BODY=8BITMIME" */
    if ( 2 <= sscanf( pszEnd, " BODY = %lu%32s%n", &ulVal, pszInfo, &count))
      {
      switch ( ulVal)
        {
      case 8:
      case 7:
        break;
      default:
        strcpy( pszInfo, "Syntax error: invalid BODY value");
        return SMTP_PARAM_SYNTAX; /* Syntax error */
        }
      }
    /* RFC 1870: "SIZE=uuu" */
    else if ( 1 <= sscanf( pszEnd, " SIZE = %lu%n", &ulVal, &count))
      {
      if ( s_ulMaxSize > 0 && ulVal > s_ulMaxSize)
        {
        strcpy( pszInfo, "Max size exceeded");
        return SMTP_TOO_BIG; /* Exceeded storage allocation */
        }

      pConn->ulRequestedSize = ulVal;
      }
    /* RFC 4954: "AUTH=uuu" */
    else if ( 1 <= sscanf( pszEnd, " AUTH = %64s%n", pszInfo, &count))
      {
      /* TODO */
      Log( kLogWarn, &pConn->client, "WARN: MAIL AUTH=%.255s\n", pszInfo);
      }
    else
      {
      strcpy( pszInfo, "Unimplemented parameter");
      return SMTP_BAD_MAIL; /* MAIL FROM/RCPT TO parameters not recognized or not implemented */
      }

    pszEnd += count;

    /* Skip whitespace */
    while ( isspace( *pszEnd))
      ++pszEnd;
    }

  pConn->eState = kStateMail;

  sprintf( pszInfo, "Sender <%s> OK", pConn->szMailFrom);
  return SMTP_OK;
  }


/*
 * RCPT command
 */
static int RcptMethod(
  SConnection* pConn,
  const char* pszTail,
  char* pszInfo
) {
  char* pszEnd;
  SRecipient* pRcpt;
  size_t size;
  char c, szAddr[ MAX_IPV4 +1], * const pszMbx = pConn->buffer;

  switch ( pConn->eState)
    {
  case kStateUnidentified:
    strcpy( pszInfo, "HELO or EHLO required");
    return SMTP_BAD_SEQUENCE; /* Bad sequence of commands */

  case kStateDenied:
    strcpy( pszInfo, "No SMTP service here");
    return SMTP_BAD_SEQUENCE; /* Bad sequence of commands */

  case kStateReset:
    strcpy( pszInfo, "MAIL FROM: required");
    return SMTP_BAD_SEQUENCE; /* Bad sequence of commands */

  case kStateMail:
  case kStateRcpt:
    break;

  case kStateData:
  case kStateAuth:
  default:
    return SMTP_BAD_SEQUENCE; /* Bad sequence of commands */
    }

  /* Validate syntax and extract recipient's address */
  if ( SWS_strnicmp( pszTail, "TO", 2))
    {
    strcpy( pszInfo, "Expected 'TO'. Syntax: RCPT TO:<name@domain>");
    return SMTP_PARAM_SYNTAX; /* Syntax error */
    }
  pszTail += 2;

  while ( isspace( *pszTail))
    ++pszTail;

  if ( ':' != *pszTail)
    {
    strcpy( pszInfo, "Expected ':'. Syntax: RCPT TO:<name@domain>");
    return SMTP_PARAM_SYNTAX; /* Syntax error */
    }
  ++pszTail;

  while ( isspace( *pszTail))
    ++pszTail;

  if ( '<' != *pszTail)
    {
    strcpy( pszInfo, "Expected '<'. Syntax: RCPT TO:<name@domain>");
    return SMTP_PARAM_SYNTAX; /* Syntax error */
    }
  ++pszTail;

  pszEnd = (char*)strchr( pszTail, '>');
  if ( NULL == pszEnd)
    {
    strcpy( pszInfo, "Expected '>'. Syntax: RCPT TO:<name@domain>");
    return SMTP_PARAM_SYNTAX; /* Syntax error */
    }
  else if ( pszEnd - pszTail >= MAX_MBX)
    {
    strcpy( pszInfo, "Forward path too long");
    return SMTP_PARAM_SYNTAX; /* Path too long */
    }
  strncpy( pszMbx, pszTail, pszEnd - pszTail);
  pszMbx[ pszEnd - pszTail] = cNul;

  /* Recipient must be a FQDN or the Postmaster */
  if ( SWS_stricmp( kszPostmaster, pszMbx)
    && NULL == strchr( pszMbx, '@')
  ) {
    sprintf( pszInfo, "Invalid recipient <%."STRING(MAX_FQDN)"s>", pszMbx);
    return SMTP_MBX_SYNTAX; /* Requested action not taken: mailbox name not allowed */
    }

  switch ( pConn->eHost)
    {
  case kHostPermitted:
  case kHostUnknown:
    break;
  case kHostDynamic:
    if ( s_iDenyDynamicIP)
      {
#if 1 /* Deny service to spammers */
      pConn->eState = kStateDenied;
#endif
      sprintf( pszInfo, "Sender <%.128s> rejected - dynamic client hostname %s",
        pConn->szMailFrom, pConn->szHostName);
      return SMTP_NOT_TAKEN; /* mailbox unavailable */
      }
    break;
  case kHostBadName:
    if ( s_iDenyUnnamed)
      {
#if 1 /* Deny service to spammers */
      pConn->eState = kStateDenied;
#endif
      sprintf( pszInfo, "Sender <%.128s> rejected - no canonical client DNS %s",
        pConn->szMailFrom, pConn->szHostName);
      return SMTP_NOT_TAKEN; /* mailbox unavailable */
      }
    break;
  case kHostMisName:
    if ( s_iDenyUnnamed)
      {
#if 1 /* Deny service to spammers */
      pConn->eState = kStateDenied;
#endif
      sprintf( pszInfo, "Sender <%.128s> rejected - mismatched canonical client DNS %s",
        pConn->szMailFrom, pConn->szHostName);
      return SMTP_NOT_TAKEN; /* mailbox unavailable */
      }
    break;
  case kHostUnnamed:
    if ( s_iDenyUnnamed)
      {
#if 1 /* Deny service to spammers */
      pConn->eState = kStateDenied;
#endif
      sprintf( pszInfo, "Sender <%.128s> rejected - no reverse client DNS %s",
        pConn->szMailFrom, pConn->szHostName);
      return SMTP_NOT_TAKEN; /* mailbox unavailable */
      }
    break;
  case kHostBadHello:
    if ( s_iDenyBadHelo)
      {
#if 1 /* Deny service to spammers */
      pConn->eState = kStateDenied;
#endif
      sprintf( pszInfo, "Sender <%.128s> rejected - client reverse DNS %s != HELO",
        pConn->szMailFrom, pConn->szHostName);
      return SMTP_NOT_TAKEN; /* mailbox unavailable */
      }
    break;
    }

  /* Callout to mail filter script */
  if ( s_szScript[0])
    {
    /* Verify buffer is big enough to hold script command line */
    size = strlen( pszMbx) + 1;
    pszEnd = pszMbx + size;
    if ( pConn->buffer + BUFFER_SIZE - (4 + 6 + 4) > pszEnd
      + strlen( s_szScript)
      + size
      + strlen( pConn->szMailFrom)
      + strlen( pConn->szHostName)
    ) {
      /* Format script command line */
      sprintf( pszEnd, "%s TO %s FROM %s BY %s",
        s_szScript, pszMbx, pConn->szMailFrom, pConn->szHostName);
      if ( 0 == system( pszEnd))
        {
        sprintf( pszInfo, "Mail from <%s> by %s rejected",
          pConn->szMailFrom, pConn->szHostName);
        return SMTP_NOT_TAKEN; /* mailbox unavailable */
        }
      }
    else
      Log( kLogWarn, &pConn->client, "WARN: Buffer too small for RCPT script\n");
    }

  /* Expand the recipent list */
  if ( pConn->recipients >= MAX_RECIPIENTS)
    {
    strcpy( pszInfo, "Too many recipients");
    return SMTP_STORAGE; /* Too many recipients */
    }

  size = (1 + pConn->recipients) * sizeof( *pRcpt);
  pRcpt = (SRecipient*) realloc( pConn->rcptArray, size);
  if ( NULL == pRcpt)
    {
    Log( kLogError, &pConn->client, "ERROR %d: realloc\n", errno);
    strcpy( pszInfo, "Too many recipients");
    return SMTP_STORAGE; /* Too many recipients */
    }
  pConn->rcptArray = pRcpt;
  pRcpt += pConn->recipients;

  pRcpt->file = NULL;
#if OPT_FORWARD
  pRcpt->isRelayed = 0;
  pRcpt->szMx[0] = cNul;
#endif

  /* Strip [ ] from around raw IP addresses */
  pszEnd = strchr( pszMbx, '@');
  if ( NULL != pszEnd
    && 2 == sscanf( ++pszEnd, "[%"STRING(MAX_IPV4)"[0123456789.]%c", szAddr, &c)
    && ']' == c
  ) {
    strcpy( pszEnd, szAddr);
    }

  /* Is the mailbox local? */
  if ( !SmtpMailboxRecord( &pRcpt->mbx, pszMbx))
    {
#if OPT_FORWARD
    /* No, is relaying permitted? */
    if ( !IsRelayPermitted( ntohl( pConn->client.sin_addr.s_addr)) )
#endif
      {
#if 1 /* Deny service to spammers */
      pConn->eState = kStateDenied;
#endif
      sprintf( pszInfo, "Relay to <%."STRING(MAX_FQDN)"s> not permitted", pszMbx);
      return SMTP_NOT_TAKEN; /* mailbox unavailable */
      }

#if OPT_FORWARD
    pRcpt->mbx.szInbox[0] = cNul;
    pRcpt->mbx.ulQuota = s_ulMaxSize ? s_ulMaxSize : ULONG_MAX;

    /* Find MaileXchange for the mailbox */
    if ( !IsValidMailbox( pRcpt->szMx, pszMbx))
      {
      /* None */
      sprintf( pszInfo, "No MX for <%."STRING(MAX_FQDN)"s>", pszMbx);
      return SMTP_NOT_TAKEN; /* mailbox unavailable */
      }
    else if ( pConn->ulRequestedSize > pRcpt->mbx.ulQuota)
      {
      sprintf( pszInfo, "Max size %lu exceeded", pRcpt->mbx.ulQuota);
      return SMTP_TOO_BIG; /* Exceeded storage allocation */
      }

    pRcpt->isRelayed = 1;
#endif
    }
  else if ( 0 == pRcpt->mbx.ulQuota)
    {
    sprintf( pszInfo, "Mailbox <%."STRING(MAX_FQDN)"s> disabled", pszMbx);
#if 0 /* Deny service to spammers */
    pConn->eState = kStateDenied;
#endif
    return SMTP_NOT_TAKEN; /* mailbox unavailable */
    }
  else if ( pConn->ulRequestedSize > pRcpt->mbx.ulQuota)
    {
    sprintf( pszInfo, "Max size %lu exceeded", pRcpt->mbx.ulQuota);
    return SMTP_TOO_BIG; /* Exceeded storage allocation */
    }

  strncpy( pRcpt->mbx.szMailbox, pszMbx, sizeof pRcpt->mbx.szMailbox);

  ++(pConn->recipients);
  pConn->eState = kStateRcpt;

  sprintf( pszInfo, "Mail from <%s> OK", pConn->szMailFrom);
  return SMTP_OK;
  }


/*
 * DATA command
 */
static int DataMethod(
  SConnection* pConn,
  const char* pszTail,
  char* pszInfo
) {
  size_t i;
  SRecipient* pRcpt;

  switch ( pConn->eState)
    {
  case kStateUnidentified:
    strcpy( pszInfo, "HELO or EHLO required");
#if 1 /* Not conforming but let's not waste anymore bandwidth */
    SessionAbort( pConn);
#endif
    return SMTP_BAD_SEQUENCE; /* Bad sequence of commands */

  case kStateDenied:
    strcpy( pszInfo, "No SMTP service here");
    return SMTP_BAD_SEQUENCE; /* Bad sequence of commands */

  case kStateReset:
    strcpy( pszInfo, "MAIL FROM: required");
#if 1 /* Not conforming but let's not waste anymore bandwidth */
    SessionAbort( pConn);
#endif
    return SMTP_BAD_SEQUENCE; /* Bad sequence of commands */

  case kStateMail:
    strcpy( pszInfo, "No recipients");
#if 1 /* Not conforming but let's not waste anymore bandwidth */
    SessionAbort( pConn);
#endif
    return SMTP_FAILED; /* no valid recipients */

  case kStateRcpt:
    break;

  case kStateData:
  case kStateAuth:
  default:
    return SMTP_BAD_SEQUENCE; /* Bad sequence of commands */
    }

  if ( *pszTail)
    {
    strcpy( pszInfo, "Syntax error: unexpected parameter");
    return SMTP_PARAM_SYNTAX; /* Syntax error */
    }

  /* For each recipient */
  pRcpt = pConn->rcptArray;
  for ( i = pConn->recipients; i > 0; --i, ++pRcpt)
    {
#if OPT_FORWARD
    if ( pRcpt->isRelayed)
      {
      /* Relaying */
      pRcpt->file = tmpfile();
      if ( NULL == pRcpt->file)
        {
        strcpy( pszInfo, "Out of storage");
        ResetTransaction( pConn); /* Sets pConn->eState to kStateReset */
        return SMTP_TOO_BIG; /* exceeded storage allocation */
        }
      }
    else
#endif
      {
      static unsigned long s_ulIndex;
      unsigned long ulIndex;
      char* pszPath;

      /* Create the inbox if necessary */
      if ( !PlatformIsDir( pRcpt->mbx.szInbox)
        && PlatformMkdir( pRcpt->mbx.szInbox)
      ) {
        sprintf( pszInfo, "Inbox for %s unavailable", pRcpt->mbx.szMailbox);
        ResetTransaction( pConn); /* Sets pConn->eState to kStateReset */
        return SMTP_NOT_TAKEN; /* Requested action not taken: mailbox unavailable */
        }

      /* Create a drop file in the assigned inbox */
      pszPath = pRcpt->mbx.szInbox + strlen( pRcpt->mbx.szInbox);
      ulIndex = ++s_ulIndex;
      do
        {
        sprintf( pszPath, kszPathSep "%08lu." kszDropFileExt, s_ulIndex);
        pRcpt->file = fopen( pRcpt->mbx.szInbox, "r");
        if ( NULL == pRcpt->file)
          {
          /* Remove filename extension until contents valid */
          *strrchr( pszPath, '.') = cNul;
          pRcpt->file = fopen( pRcpt->mbx.szInbox, "w");
          break;
          }
        fclose( pRcpt->file), pRcpt->file = NULL;
        }
      while ( ulIndex != ++s_ulIndex );

      if ( NULL == pRcpt->file)
        {
        sprintf( pszInfo, "Inbox for %s full", pRcpt->mbx.szMailbox);
        ResetTransaction( pConn); /* Sets pConn->eState to kStateReset */
        return SMTP_TOO_BIG; /* exceeded storage allocation */
        }
      }

    /* Prepend trace headers */
#if OPT_FORWARD
    if ( !pRcpt->isRelayed)
#endif
      fprintf( pRcpt->file, "Return-Path: <%s>\n", pConn->szMailFrom);

    fprintf( pRcpt->file,
      "Received: FROM %s (%s)\n"
      "\tBY %s (" kszBanner ") WITH %s\n",
      pConn->szHello, pConn->szHostName,
      GetFQDN( pConn->buffer), pConn->pszProtocol
    );
    fprintf( pRcpt->file, "\tFOR <%s>", pRcpt->mbx.szMailbox);
    SWS_TimeToStr( pConn->buffer, sizeof pConn->buffer, pConn->tRequest);
    fprintf( pRcpt->file, "; %s\n", pConn->buffer);
    }

  pConn->pTelnetInput = pConn->buffer;
  pConn->telnetSize = sizeof( pConn->buffer);

  /* Enter the DATA phase */
  pConn->ulData = 0;
  pConn->eState = kStateData;

  strcpy( pszInfo, "Start mail input; end with <CRLF>.<CRLF>");
  return SMTP_START_MAIL;
  }


/*
 * Process DATA input
 */
void DataInput( SConnection* pConn, size_t len)
  {
  char* const pszResponse = pConn->buffer + (sizeof( pConn->buffer) - MAX_REPLY);
  char* pszBuf;
  SRecipient* pRcpt;
  int iRet = 0;
  int i;

  pszBuf = pConn->pTelnetInput;

  /* Read a line at a time until <CRLF>.<CRLF> */
  if ( 0 == strcmp( ".", pszBuf))
    {
    /* Last line so close all drop files */
    pRcpt = pConn->rcptArray;
    for ( i = pConn->recipients; i > 0; --i, ++pRcpt)
      {
#if OPT_FORWARD
      if ( pRcpt->isRelayed)
        {
        /* Relay this message */
        iRet = RelayMessage( pConn, pRcpt);

        /* Close/delete message file */
        fclose( pRcpt->file), pRcpt->file = NULL;
        if ( cNul != pRcpt->mbx.szInbox[0 ])
          {
          (void)remove( pRcpt->mbx.szInbox);
          pRcpt->mbx.szInbox[ 0] = cNul;
          }

        if ( iRet)
          break;
        }
      else
#endif
        {
        /* Local mailbox */
        long const lFilelen = ftell( pRcpt->file);
        iRet = fclose( pRcpt->file);
        pRcpt->file = NULL;
        if ( iRet)
          {
          /* Abort required coz data following will be interpreted as commands */
          SessionAbort( pConn);

          iRet = SMTP_TOO_BIG; /* exceeded storage allocation */
          sprintf( pszResponse, "%s out of storage", pRcpt->mbx.szMailbox);
          /* ResetTransaction will remove the file */
          break;
          }
        else if ( cNul != pRcpt->mbx.szInbox[ 0])
          {
          char szPath[ sizeof pRcpt->mbx.szInbox];

          /* Rename to valid dropfile */
          strcat( strcpy( szPath, pRcpt->mbx.szInbox), "." kszDropFileExt);
          if ( 0 == rename( pRcpt->mbx.szInbox, szPath))
            strcpy( pRcpt->mbx.szInbox, szPath);
          else
            {
            iRet = SMTP_TOO_BIG; /* exceeded storage allocation */
            sprintf( pszResponse, "%s out of storage", szPath);
            /* ResetTransaction will remove the file */
            }
          }

        Log( kLogInfo, &pConn->client, "STORED %s %ld bytes\n",
          pRcpt->mbx.szInbox, lFilelen);
        }
      }

    if ( 0 == iRet && s_szScript[0])
      {
      /* Format script command line */
      sprintf( pConn->buffer, "%s DATA %s", s_szScript, pRcpt->mbx.szInbox);
      if ( 0 == system( pConn->buffer))
        {
        strcpy( pszResponse, "Content rejected");
        iRet = SMTP_NOT_TAKEN; /* mailbox unavailable */
        }
      }

    if ( 0 == iRet)
      {
      /* Success so don't delete the drop files in ResetTransaction */
      pRcpt = pConn->rcptArray;
      for ( i = pConn->recipients; i > 0; --i, ++pRcpt)
        pRcpt->mbx.szInbox[ 0] = cNul;

      iRet = SMTP_OK;
      sprintf( pszResponse, "OK. %lu bytes received", pConn->ulData);
      }
    }
  else
    {
    /* Transparency */
    if ( '.' == *pszBuf)
      {
      ++pszBuf;
      --len;
      }

    /* Append newline */
    pszBuf[ len++] = '\n';

    /* Append to each drop file */
    pRcpt = pConn->rcptArray;
    for ( i = pConn->recipients; i > 0; --i, ++pRcpt)
      {
      if ( pConn->ulData > pRcpt->mbx.ulQuota)
        {
        iRet = SMTP_TOO_BIG; /* exceeded storage allocation */
        sprintf( pszResponse, "%s quota exceeded", pRcpt->mbx.szMailbox);
        break;
        }

      /* Save to file */
      if ( len != fwrite( pszBuf, 1, len, pRcpt->file))
        {
        iRet = SMTP_TOO_BIG; /* exceeded storage allocation */
        sprintf( pszResponse, "%s out of storage", pRcpt->mbx.szMailbox);
        break;
        }
      }
    }

  if ( iRet)
    {
    ResetTransaction( pConn);
    Response( pConn, iRet, pszResponse);
    }
  }


/*
 * RSET command
 */
static int ResetMethod(
  SConnection* pConn,
  const char* pszTail,
  char* pszInfo
) {
  switch ( pConn->eState)
    {
  case kStateDenied:
    strcpy( pszInfo, "No SMTP service here");
    return SMTP_NOT_TAKEN; /* Requested action not taken: mailbox unavailable */

  case kStateUnidentified:
    strcpy( pszInfo, "HELO or EHLO required");
    return SMTP_BAD_SEQUENCE; /* Bad sequence of commands */

  default:
    break;
    }

  if ( *pszTail)
    {
    strcpy( pszInfo, "Syntax error: unexpected parameter");
    return SMTP_PARAM_SYNTAX; /* Syntax error */
    }

  ResetTransaction( pConn); /* Sets pConn->eState to kStateReset */

  strcpy( pszInfo, "OK");
  return SMTP_OK;
  }


/*
 * VRFY command
 */
static int VerifyMethod(
  SConnection* pConn,
  const char* pszTail,
  char* pszInfo
) {
  SMailbox* pMbx = (SMailbox*)pConn->buffer;

  switch ( pConn->eState)
    {
  case kStateDenied:
    strcpy( pszInfo, "No SMTP service here");
    return SMTP_BAD_SEQUENCE; /* Bad sequence of commands */

  default:
    break;
    }

  /* Is it a local mailbox? */
  if ( SmtpMailboxRecord( pMbx, pszTail))
    {
    /* Yes */
    if ( 0 == pMbx->ulQuota)
      {
      sprintf( pszInfo, "%."STRING(MAX_FQDN)"s not known", pszTail);
      return SMTP_NOT_TAKEN; /* mailbox unavailable */
      }
    else if ( '@' != pMbx->szMailbox[0])
      {
      sprintf( pszInfo, "%s OK", pMbx->szMailbox);
      }
    else if ( NULL == strchr( pszTail, '@'))
      {
      sprintf( pszInfo, "%."STRING(MAX_FQDN)"s%s OK", pszTail, pMbx->szMailbox);
      }
    else
      {
      sprintf( pszInfo, "%."STRING(MAX_FQDN)"s OK", pszTail);
      }

    return SMTP_OK;
    }

  if ( !IsValidMailbox( NULL, pszTail) )
    {
    sprintf( pszInfo, "%."STRING(MAX_FQDN)"s not known", pszTail);
    return SMTP_NOT_TAKEN; /* mailbox unavailable */
    }

  sprintf( pszInfo, "%."STRING(MAX_FQDN)"s OK", pszTail);
  return SMTP_OK;
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

  if ( *pszTail)
    {
    strcpy( pszInfo, "Syntax error: unexpected parameter");
    return SMTP_PARAM_SYNTAX; /* Syntax error */
    }

  pConn->eState = kStateQuit;

  sprintf( pszInfo, "%."STRING(MAX_FQDN)"s %s", GetHostname(), s_szBye);
  return SMTP_CLOSING; /* Quit */
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

  return SMTP_OK;
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
    pCmd = FindCommand( pszTail);
    if ( NULL != pCmd
      && NULL != pCmd->pszHelp
    ) {
      sprintf( pszInfo, "%-5.5s %s", pCmd->pszName, pCmd->pszHelp);
      }
    else
      {
      return SMTP_PARAM_SYNTAX; /* Syntax error in parameters or arguments */
      }
    }
  else
    {
    size_t i;

    Response( pConn, SMTP_HELP, "-Commands available:");

    for ( i = 0, pCmd = methods; i < lengthof( methods); ++i, ++pCmd)
      {
      if ( NULL != pCmd->pszHelp)
        {
        sprintf( pszInfo, "-%-5.5s%s", pCmd->pszName, pCmd->pszHelp);
        Response( pConn, SMTP_HELP, pszInfo);
        }
      }
    strcpy( pszInfo, "OK.");
    }

  return SMTP_HELP;
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
  return SMTP_COMMAND_UNIMPL; /* Command not implemented */
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


/*
 * Abort the current transaction
 */
static void ResetTransaction( SConnection* pConn)
  {
  SRecipient* pRcpt;

  pConn->eState = kStateReset;
  pConn->ulRequestedSize = 0;

  pConn->pTelnetInput = pConn->input;
  pConn->telnetSize = sizeof( pConn->input);

  pRcpt = pConn->rcptArray;
  if ( NULL != pRcpt)
    {
    int i;

    for ( i = pConn->recipients; i > 0; --i, ++pRcpt)
      {
      /* Close the drop file */
      if ( NULL != pRcpt->file)
        {
        fclose( pRcpt->file), pRcpt->file = NULL;

        /* Delete the drop file on error */
        if ( cNul != pRcpt->mbx.szInbox[ 0])
          {
          (void)remove( pRcpt->mbx.szInbox);
          pRcpt->mbx.szInbox[ 0] = cNul;
          }
        }
      }

    free( pConn->rcptArray), pConn->rcptArray = NULL;
    }
  pConn->recipients = 0;
  }


/*
 * Validate acct and password
 */
typedef struct SAcctInfo
  {
  const char* pszAcct;
  size_t sizeAcct;
  const char* pszPwd;
  size_t sizePwd;
  } SAcctInfo;

static int IsValidAcct( const char* pszAcct, const char* pszPwd)
  {
  SAcctInfo info;

  info.pszAcct = pszAcct;
  info.sizeAcct = strlen( pszAcct);
  info.pszPwd = pszPwd;
  info.sizePwd = strlen( pszPwd);
  return SWS_CfgEnum( kszPopSect, &EnumAcctCallback, &info) > 0;
  }

SWS_CB( int, EnumAcctCallback( void* pv, const char* pszVal))
  {
  SAcctInfo* pInfo = (SAcctInfo*)pv;

  if ( ';' == *pszVal)
    return 0;

  /* Expect username  password  inbox */
  while ( isspace( *pszVal))
    ++pszVal;

  /* Compare username */
  if ( strncmp( pszVal, pInfo->pszAcct, pInfo->sizeAcct))
    return 0; /* Continue, not found */

  pszVal += pInfo->sizeAcct;
  if ( !isspace( *pszVal))
    return 0; /* Continue, not found */
  do
    ++pszVal;
  while ( isspace( *pszVal));


  /* Compare password */
  if ( strncmp( pszVal, pInfo->pszPwd, pInfo->sizePwd))
    return -1; /* Stop, incorrect */

  pszVal += pInfo->sizePwd;
  return isspace( *pszVal) ? 1 : -1; /* Stop, correct or incorrect */
  }


#if OPT_FORWARD
/*
 * Relay a message
 */
static int RelayMessage(
  SConnection* pConn,
  SRecipient* pRcpt
) {
  char* const pszResponse = pConn->buffer + (sizeof( pConn->buffer) - MAX_REPLY);
  int iRet;
  char* psz, *pszMx, *pszAuth;
  const char* rcptArray[ 1];
  u_short unPort;

  if ( cNul != s_szRelayMx[ 0])
    strcpy( pRcpt->szMx, s_szRelayMx);

  Log( kLogInfo, &pConn->client, "RELAY %s to %s\n", pRcpt->mbx.szMailbox, pRcpt->szMx);

  /* parse the MX for auth+mx:port */
  pszAuth = NULL; /* NULL -> no AUTH */
  pszMx = strrchr( pRcpt->szMx, '+');
  if ( NULL == pszMx)
    pszMx = pRcpt->szMx;
  else
    {
    *pszMx++ = cNul;
    pszAuth = pRcpt->szMx;
    }

  psz = strrchr( pszMx, ':');
  if ( NULL == psz)
    unPort = SMTPD_PORT;
  else
    {
    *psz++ = cNul;
    unPort = (u_short) strtol( psz, &psz, 10);
    }

  /* Relay the message */
  rcptArray[ 0] = pRcpt->mbx.szMailbox;
  iRet = SendMail(
    pRcpt->file, 0,
    lengthof( rcptArray), rcptArray,
    pConn->szMailFrom,
    NULL, /* NULL Reply-To -> none */
    NULL, /* NULL subject -> no envelope */
    pszMx, unPort,
    GetHostname(), /* NULL -> HELO hostname, "" -> HELO [ip[addr] */
    pszAuth,
    s_logfile, s_iVerbosity > 0 ? 1 : 0
  );
  switch ( iRet)
    {
  case 0:
    break;

  case EHOST_NOT_FOUND:
    sprintf( pszResponse, "Mail eXchange for %s not known",
      pRcpt->mbx.szMailbox);
    iRet = SMTP_FAILED;
    break;

  case ETIMEDOUT:
  case EWOULDBLOCK:
#if EAGAIN != EWOULDBLOCK
  case EAGAIN:
#endif
    sprintf( pszResponse, "Timed out connecting to Mail eXchange for %s",
      pRcpt->mbx.szMailbox);
    iRet = SMTP_FAILED;
    break;

  case ECONNREFUSED:
    sprintf( pszResponse, "Connection refused by Mail eXchange for %s",
      pRcpt->mbx.szMailbox);
    iRet = SMTP_FAILED;
    break;

  case ECONNABORTED:
    sprintf( pszResponse, "Login rejected by Mail eXchange for %s",
      pRcpt->mbx.szMailbox);
    iRet = SMTP_FAILED;
    break;

  case 421:
    sprintf( pszResponse, "Mail eXchange for %s not available",
      pRcpt->mbx.szMailbox);
    iRet = SMTP_FAILED;
    break;

  default:
    if ( iRet >= 400 && iRet < 600)
      {
      sprintf( pszResponse, "Relay to %s failed", pRcpt->mbx.szMailbox);
      }
    else
      {
      sprintf( pszResponse, "Error relaying to %s", pRcpt->mbx.szMailbox);
      iRet = SMTP_FAILED;
      }
    break;
    }

  return iRet;
  }
#endif


/*
 * Check if mail sender is blocked
 */
int IsBlockedSender( const char* pszMbx)
  {
  return SWS_CfgEnum( kszBannedSender, &EnumRegcmp, (void*)pszMbx) > 0;
  }
SWS_CB( int, EnumRegcmp( void* pv, const char* pszVal))
  {
  return ';' != *pszVal ? SWS_StrMatch( pszVal, (const char*)pv, 1) : 0;
  }


/*
 * Check if hostname is dynamic
 */
int IsDynamicHostname( const char* pszName, u_long addr)
  {
  char szHost[ MAX_FQDN];
  char szBuf[ 9], *p;
  int matches, n;
  unsigned ip[4];

  /* Exclude common mail hostnames */
  if ( SWS_CfgEnum( kszMailHosts, &EnumRegcmp, (void*)pszName) > 0)
    return 0;

  /* Add/subtract weight for dynamic/static name fragments */
  matches = 0;
  if ( SWS_CfgEnum( kszDynamicHosts, &EnumStrstr, (void*)pszName) > 0)
    ++matches;
  else if ( SWS_CfgEnum( kszStaticHosts, &EnumStrstr, (void*)pszName) > 0)
    --matches;

  /* Sort IP address elements to match longest first */
  ip[0] = (unsigned)(addr >> 24);
  ip[1] = (unsigned)(addr >> 16) & 255;
  ip[2] = (unsigned)(addr >> 8) & 255;
  ip[3] = (unsigned)addr & 255;
  qsort( ip, lengthof( ip), sizeof ip[0], cmpUnsigned);

  /* Deny hosts with 2 or more IP address elements in their domain name */
  strncpy( szHost, pszName, sizeof szHost);
  n = sprintf( szBuf, "%u", ip[3]);
  if ( NULL != (p = (char*)strstr( szHost, szBuf)))
    {
    memset( p, ' ', n);
    ++matches;
    }

  n = sprintf( szBuf, "%u", ip[2]);
  if ( NULL != (p = (char*)strstr( szHost, szBuf)))
    {
    memset( p, ' ', n);
    ++matches;
    }

  n = sprintf( szBuf, "%u", ip[1]);
  if ( NULL != (p = (char*)strstr( szHost, szBuf)))
    {
    memset( p, ' ', n);
    ++matches;
    }

  n = sprintf( szBuf, "%u", ip[0]);
  if ( NULL != (p = (char*)strstr( szHost, szBuf)))
    {
    memset( p, ' ', n);
    ++matches;
    }

  if ( matches < 2)
    {
    sprintf( szBuf, "%02x%02x%02x%02x",
      (unsigned)(addr >> 24),
      (unsigned)(addr >> 16) & 255,
      (unsigned)(addr >> 8) & 255,
      (unsigned)addr & 255
    );
    if ( SWS_stristr( pszName, szBuf))
      matches = 4;
    else if ( SWS_stristr( pszName, szBuf + 2))
      matches = 3;
    else if ( SWS_stristr( pszName, szBuf + 4))
      matches = 2;
    else if ( szBuf[6] = 0, SWS_stristr( pszName, szBuf))
      matches = 3;
    else if ( SWS_stristr( pszName, szBuf + 2))
      matches = 2;
    else if ( szBuf[4] = 0, SWS_stristr( pszName, szBuf))
      matches = 2;
    }

  if ( matches < 2)
    {
    sprintf( szBuf, "%02x%02x%02x%02x",
      (unsigned)addr & 255,
      (unsigned)(addr >> 8) & 255,
      (unsigned)(addr >> 16) & 255,
      (unsigned)(addr >> 24)
    );
    if ( SWS_stristr( pszName, szBuf))
      matches = 4;
    else if ( SWS_stristr( pszName, szBuf + 2))
      matches = 3;
    else if ( SWS_stristr( pszName, szBuf + 4))
      matches = 2;
    else if ( szBuf[6] = 0, SWS_stristr( pszName, szBuf))
      matches = 3;
    else if ( SWS_stristr( pszName, szBuf + 2))
      matches = 2;
    else if ( szBuf[4] = 0, SWS_stristr( pszName, szBuf))
      matches = 2;
    }

  return matches >= 2;
  }

SWS_CB( int, EnumStrstr( void* pv, const char* pszVal))
  {
  return ';' != *pszVal ? NULL != SWS_stristr( (const char*)pv, pszVal) : 0;
  }

/* qsort callback */
static int cmpUnsigned( const void* p1, const void* p2)
  {
  unsigned u1 = *(const unsigned*)p1;
  unsigned u2 = *(const unsigned*)p2;
  return u1 == u2 ? 0 : u1 < u2 ? -1 : 1;;
  }


/*
 * Validate a mailbox name
 */
static int IsValidMailbox( char szMx[ MAX_FQDN], const char* pszMailbox)
  {
  char c;
  struct in_addr inaddr;
  char szHost[ MAX_FQDN];

  /* Ensure valid name */
  if ( NULL == pszMailbox)
    return 0;

  while ( cNul != (c = *pszMailbox++))
    {
    if ( !isprint( c))
      return 0;

    if ( '@' == c)
      break;
    }

  /* Is there a domain name? */
  if ( cNul == c)
    return 0; /* Invalid, no domain */

  if ( 0 == SWS_stricmp( pszMailbox, "localhost"))
    {
    if ( NULL != szMx)
      strcpy( szMx, pszMailbox);
    return !0;
    }

  /* Is there a local MX entry? */
  switch ( GetMxRecord( szHost, pszMailbox))
    {
  case 1: /* Yes */
    if ( NULL != szMx)
      strcpy( szMx, szHost);
    return !0;

  case -1: /* Yes, forcing DNS */
    if ( NULL == szMx)
      return !0;
    break;

  case 0:
#if OPT_FORWARD
    /* Is there a default MX? */
    if ( NULL != szMx && cNul != s_szDefaultMx[0])
      {
      /* Yes, use it */
      strcpy( szMx, s_szDefaultMx);
      return !0;
      }
#endif
    break;

  default:
    return 0;
    }

  /* Try a simple IPv4 address */
  inaddr.s_addr = inet_addr( pszMailbox);
  if ( INADDR_NONE != inaddr.s_addr
    && INADDR_ANY != inaddr.s_addr
  ) {
    if ( NULL != szMx)
      strcpy( szMx, inet_ntoa( inaddr));
    return !0;
    }

  /* Try a [IPv4] address */
  if ( 2 == sscanf( pszMailbox, "[%"STRING(MAX_IPV4)"[0123456789.]%c", szHost, &c)
    && ']' == c
  ) {
    inaddr.s_addr = inet_addr( szHost);
    if ( INADDR_NONE != inaddr.s_addr
      && INADDR_ANY != inaddr.s_addr
    ) {
      if ( NULL != szMx)
        strcpy( szMx, inet_ntoa( inaddr));
      return !0;
      }
    }

  /* Lookup MX record for domain */
  szHost[0] = cNul;
  switch ( GetMx( szHost, pszMailbox))
    {
  default:
    break;

  case SWS_DNS_RC_NWERR:
  case SWS_DNS_RC_NOREPLY:
    return NULL == szMx; /* Assume valid if no name server */

  case SWS_DNS_RC_NAMERR:
    if ( NULL == szMx)
      return 0; /* MX not found if testing valid */
    break;
    }

  if ( cNul == szHost[ 0])
    return 0; /* MX not found */

  /* MX found */
  if ( NULL != szMx)
    strcpy( szMx, szHost);

  return !0;
  }


/*
 * Get Mail eXchange for a host
 * See RFC 974
 */
int GetMx( char szMx[ MAX_FQDN], const char* pszRemote)
  {
  int iRet;
  SWS_DnsQHandle hQuery;
  SWS_SDnsQRecord rec;
  ui16 pref = 0xffff;

  szMx[0] = cNul;

  /* Lookup MX record for domain */
  hQuery = SWS_DnsOpenQuery(
    SWS_DNS_OP_QUERY | SWS_DNS_RD_MASK,
    pszRemote,
    SWS_DNS_TYPE_MX,
    SWS_DNS_CLASS_IN,
    INADDR_ANY,
    0
  );
  if ( NULL == hQuery)
    return SWS_DNS_RC_NWERR;

  iRet = SWS_DnsQueryRcode( hQuery);
  iRet &= SWS_DNS_RCODE_MASK;

  /* Search for MX records */
  while ( !SWS_DnsQueryRead( hQuery, &rec))
    {
    if ( SWS_kDnsAnswer == rec.eRecordType
      && SWS_DNS_CLASS_IN == rec.rclass
    ) {
      ui16 u1;
      switch ( rec.rtype)
        {
      case SWS_DNS_TYPE_MX:
        /* Get MX preference */
        SWS_DnsQueryData( &u1, sizeof( u1), hQuery, rec.dataIndex);
        u1 = ntohs( u1);
        if ( u1 < pref) /* Lower is preferred */
          {
          pref = u1;
          SWS_DnsQueryName( szMx, MAX_FQDN, hQuery, rec.dataIndex + 2);
          }
        break;

      case SWS_DNS_TYPE_A:
        /* If no MX then use this */
        if ( 0xffff == pref)
          SWS_DnsQueryName( szMx, MAX_FQDN, hQuery, rec.nameIndex);
        break;

      case SWS_DNS_TYPE_CNAME:
        /* If no MX then lookup alias */
        if ( 0xffff == pref)
          {
          SWS_DnsQueryName( szMx, MAX_FQDN, hQuery, rec.dataIndex);
          SWS_DnsCloseQuery( hQuery);

          hQuery = SWS_DnsOpenQuery(
            SWS_DNS_OP_QUERY | SWS_DNS_RD_MASK,
            szMx,
            SWS_DNS_TYPE_MX,
            SWS_DNS_CLASS_IN,
            INADDR_ANY,
            0
          );
          if ( NULL == hQuery)
            return SWS_DNS_RC_NWERR; /* MX not found */

          szMx[0] = cNul;
          }
        break;

      default:
        break;
        }
      }
    }

  SWS_DnsCloseQuery( hQuery);

  /* If no MX then use domain name */
  if ( cNul == szMx[0])
    strcpy( szMx, pszRemote);

  return iRet;
  }


/*
 * Find a local Mail eXchange record
 */
int GetMxRecord(                      /* !0 if successful */
  char szMx[ MAX_FQDN],               /* OUT: name of MX */
  const char* pszDomain               /* IN: FQDN */
) {
  const struct SMxInfo* pInfo;
  unsigned u;

  for ( pInfo = &s_mx.aInfo[ u = 0]; u < s_mx.cnt; ++u, ++pInfo)
    {
    if ( 0 == SWS_stricmp( pInfo->szDom, pszDomain))
      return strcpy( szMx, pInfo->szMx)[0] ? 1 : -1;
    }
  return 0;
  }

SWS_CB( int, EnumMxCallback( void* pvCtx, const char* pszVal))
  {
  struct SMxList* pList = (struct SMxList*)pvCtx;
  SMxInfo* pInfo;
  unsigned uCount;

  if ( ';' == *pszVal)
    return 0;

  uCount = pList->cnt;
  if ( uCount * sizeof pList->aInfo[0] >= pList->size)
    {
    /* Extend the array. NB this is never freed */
    size_t size = pList->size ? pList->size * 2 : 2 * sizeof pList->aInfo[0];
    void* pv = realloc( pList->aInfo, size);
    if ( NULL == pv)
      {
      Log( kLogError, NULL, "ERROR %d: realloc\n", errno);
      return -1;
      }
    pList->aInfo = (SMxInfo*)pv;
    pList->size = size;
    }
  pInfo = &pList->aInfo[ uCount++];

  /* Parse MX entry */
  { STATIC_ASSERT( sizeof pInfo->szDom > 255 && sizeof pInfo->szMx > 255); }
  switch ( sscanf( pszVal, " %255[^; =] = %255[^; ]", pInfo->szDom, pInfo->szMx))
    {
  case 2: /* Use the given MX if the hostname matches */
    Log( kLogMost, NULL, kszMxSect ": %s = %s\n", pInfo->szDom, pInfo->szMx);
    pList->cnt = uCount;
    break;

  case 1: /* Use DNS MX */
    Log( kLogMost, NULL, kszMxSect ": %s =\n", pInfo->szDom);
    pInfo->szMx[0] = cNul;
    pList->cnt = uCount;
    break;
    }

  return 0;
  }


/**
 * Find a mailbox record given a username
 */
extern int SmtpMailboxRecord(         /* !0 if successful */
  SMailbox* pMbxOut,                  /* OUT: Best match */
  const char* pszMailbox              /* IN: Username or mailbox name */
) {
  const char* pszDom;
  char c;
  unsigned u;
  const SMailbox* pMbx;

  /* Verify username */
  for ( pszDom = pszMailbox; cNul != (c = *pszDom); ++pszDom)
    {
    if ( '@' == c)
      break;
    if ( !isgraph( c))
      return 0;
    }

  for ( pMbx = &s_mbxs.aMbx[ u = 0]; u < s_mbxs.cnt; ++u, ++pMbx)
    {
    /* Is it a wildcard usermame? */
    if ( '@' == pMbx->szMailbox[ 0] )
      {
      /* Yes */
      if ( '@' != pszDom[ 0])
        return 0; /* No domain supplied */

      /* Match domain */
      if ( 0 == SWS_stricmp( pMbx->szMailbox, pszDom))
        {
        *pMbxOut = *pMbx;
        return 1;
        }
      }
    /* Try full match */
    else if ( 0 == SWS_stricmp( pMbx->szMailbox, pszMailbox))
      {
      *pMbxOut = *pMbx;
      return 1;
      }
    /* Domain given? */
    else if ( '@' != pszDom[ 0])
      {
      char* psz;

      /* No, remove domain name from mbx */
      *pMbxOut = *pMbx;
      psz = strchr( pMbxOut->szMailbox, '@');
      if ( NULL != psz)
        *psz = cNul; /* Terminate username */

      /* Match username */
      if ( 0 == SWS_stricmp( pMbxOut->szMailbox, pszMailbox))
        {
        if ( NULL != psz)
          *psz = '@';
        return 1;
        }
      }
    }
  return 0;
  }

SWS_CB( int, EnumMbxCallback( void* pvCtx, const char* pszVal))
  {
  struct SMbxList* pList = (struct SMbxList*)pvCtx;
  SMailbox* pMbx;
  unsigned uCount;

  if ( ';' == *pszVal)
    return 0;

  uCount = pList->cnt;
  if ( uCount * sizeof pList->aMbx[0] >= pList->size)
    {
    /* Extend the array. NB this is never freed */
    size_t size = pList->size ? pList->size * 2 : 4 * sizeof pList->aMbx[0];
    void* pv = realloc( pList->aMbx, size);
    if ( NULL == pv)
      {
      Log( kLogError, NULL, "ERROR %d: realloc\n", errno);
      return -1;
      }
    pList->aMbx = (SMailbox*)pv;
    pList->size = size;
    }
  pMbx = &pList->aMbx[ uCount++];
  pMbx->ulQuota = s_ulMaxSize ? s_ulMaxSize : ULONG_MAX;

  /* Parse entry */
  if ( 2 <= sscanf( pszVal, " %255[^ ;] %255[^ ;] %lu",
      pMbx->szMailbox, pMbx->szInbox, &pMbx->ulQuota)
  ) {
    Log( kLogMost, NULL, kszMbxSect ": %s %s %lu\n",
      pMbx->szMailbox, pMbx->szInbox, pMbx->ulQuota);
    pList->cnt = uCount;
    }
  return 0;
  }


/*
 * Check if host is permitted
 */
int IsPermittedHost( ui32 addr)
  {
  const struct SHostNet* pHost;
  unsigned u;

  for ( pHost = &s_permit.aHost[ u = 0]; u < s_permit.cnt; ++u, ++pHost)
    {
    if ( INADDR_ANY == pHost->ulNet || pHost->ulNet == (addr & pHost->ulMask) )
      return 1;
    }
  return 0;
  }


/*
 * Check if host is denied
 */
int IsDeniedHost( ui32 addr)
  {
  const struct SHostNet* pHost;
  unsigned u;

  for ( pHost = &s_denied.aHost[ u = 0]; u < s_denied.cnt; ++u, ++pHost)
    {
    if ( INADDR_ANY == pHost->ulNet || pHost->ulNet == (addr & pHost->ulMask) )
      return 1;
    }
  return 0;
  }


SWS_CB( int, EnumHostIpsCallback( void* pvCtx, const char* pszVal))
  {
  struct SHostList* pList = (struct SHostList*)pvCtx;
  char szNet[ MAX_IPV4 +1];
  ui32 ulNet;
  unsigned uCidr = 32; /* Classless Internet Domain Routing */

  /* Parse host entry */
  if ( 1 <= sscanf( pszVal, " ip4 = %"STRING(MAX_IPV4)"[0123456789.] / %u", szNet, &uCidr)
    && INADDR_NONE != (ulNet = ntohl( inet_addr( szNet)))
    && uCidr > 0
    && uCidr <= 32
  ) {
    struct SHostNet* pHost;
    unsigned uCount = pList->cnt;

    Log( kLogMost, NULL, "%s: ip4 = %s %u\n",
      &s_denied == pList ? kszDeniedHostSect
        : &s_permit == pList ? kszPermitHostSect
#if OPT_FORWARD
        : &s_relays == pList ? kszRelayHostSect
#endif
        : "other",
      szNet, uCidr);
    if ( uCount * sizeof pList->aHost[0] >= pList->size)
      {
      /* Extend the array. NB this is never freed */
      size_t size = pList->size ? pList->size * 2 : 16 * sizeof pList->aHost[0];
      void* pv = realloc( pList->aHost, size);
      if ( NULL == pv)
        {
        Log( kLogError, NULL, "ERROR %d: realloc\n", errno);
        return -1;
        }
      pList->aHost = (struct SHostNet*)pv;
      pList->size = size;
      }
    pHost = &pList->aHost[ uCount++];
    pHost->ulMask = ~0UL << (32 - uCidr);
    pHost->ulNet = ulNet & pHost->ulMask;
    pList->cnt = uCount;
    }
  return 0;
  }


/*
 * Check if hostname is banned
 */

static int IsBannedHostname( const char* pszHostname)
  {
  const struct SHostName* pHost;
  unsigned u;

  for ( pHost = &s_banned.aHost[ u = 0]; u < s_banned.cnt; ++u, ++pHost)
    {
    if ( SWS_StrMatch( pHost->szName, pszHostname, 1))
      return 1;
    }
  return 0;
  }


SWS_CB( int, EnumHostNamesCallback( void* pvCtx, const char* pszVal))
  {
  struct SHostNameList* pList = (struct SHostNameList*)pvCtx;
  char szHost[ MAX_FQDN + 1];

  /* Parse host entry */
  if ( 1 == sscanf( pszVal, " hostname = %"STRING(MAX_FQDN)"[^ ;]", szHost))
    {
    struct SHostName* pHost;
    unsigned uCount = pList->cnt;

    Log( kLogMost, NULL, kszBannedHostSect ": hostname = %s\n", szHost);
    if ( uCount * sizeof pList->aHost[0] >= pList->size)
      {
      /* Extend the array. NB this is never freed */
      size_t size = pList->size ? pList->size * 2 : 16 * sizeof pList->aHost[0];
      void* pv = realloc( pList->aHost, size);
      if ( NULL == pv)
        {
        Log( kLogError, NULL, "ERROR %d: realloc\n", errno);
        return -1;
        }
      pList->aHost = (struct SHostName*)pv;
      pList->size = size;
      }
    pHost = &pList->aHost[ uCount++];
    strncpy( pHost->szName, szHost, sizeof( pHost->szName) -1);
    pList->cnt = uCount;
    }
  return 0;
  }


#if OPT_FORWARD
/*
 * Check if host is permitted to relay
 */
static int IsRelayPermitted( ui32 addr)
  {
  const struct SHostNet* pHost;
  unsigned u;

  for ( pHost = &s_relays.aHost[ u = 0]; u < s_relays.cnt; ++u, ++pHost)
    {
    if ( pHost->ulNet == (addr & pHost->ulMask) )
      return 1;
    }
  return 0;
  }
#endif


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

    if ( pConn->telnetCR) switch ( c)
      {
    case cLinefeed:
      pConn->telnetCR = 2;
      goto done;

    default:
      pConn->telnetCR = 0;
      if ( pConn->telnetCount + 3 <= pConn->telnetSize )
        {
        pConn->pTelnetInput[ pConn->telnetCount++] = cReturn;
        pConn->pTelnetInput[ pConn->telnetCount++] = (char)c;
        }
      break;
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
  iRet += pConn->telnetCR; /* Including CRLF */
  pConn->telnetCR = 0;

  return iRet;
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
  /* NB Response() disables async mode so should have sent all */
  else if ( (size_t)iRet != size)
    Log( kLogWarn, &pConn->client, "WARN: sent %u/%u\n", (unsigned)iRet, (unsigned)size);

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
      Log( kLogMore, &pConn->client, "Connection reset in read\n");
      SessionAbort( pConn);
      break;
    case ECONNABORTED:
      Log( kLogWarn, &pConn->client, "WARN %d: connection aborted in read\n", err);
      SessionAbort( pConn);
      break;
    case ENOTCONN:
      Log( kLogWarn, &pConn->client, "WARN %d: disconnected in read\n", err);
      break;
    default:
      Log( kLogWarn, &pConn->client, "WARN %d: recv: %s\n", err, strerror( err));
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


static const char* GetHostname( void)
  {
  return s_szHostName;
  }


static const char* GetFQDN( char szBuf[])
  {
  const char* const szHostname = GetHostname();
  struct hostent* const pServer = gethostbyname( szHostname);
  if ( NULL != pServer)
    {
    sprintf( szBuf, "%."STRING(MAX_FQDN)"s [%."STRING(MAX_IPV4)"s]",
      /*pServer->h_name*/ szHostname, /* GetHostname may be an alias */
      inet_ntoa( *(const struct in_addr*)pServer->h_addr)
    );
    return szBuf;
    }
  return szHostname;
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

#if defined WIN32 || defined _WIN32 || defined __WIN32__ || defined _DOS || defined _DOS16
/****************************************************************************
 * Win32 & DOS
 ****************************************************************************/

#include <sys/stat.h> /* _S_xxx flags & _stat */
#include <direct.h> /* _mkdir _rmdir */

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

#elif defined _POSIX_SOURCE || defined _POSIX_C_SOURCE || defined __DJGPP__
/****************************************************************************
 * POSIX
 ****************************************************************************/

#undef fd_set
#undef socklen_t
#undef gethostname

#include <unistd.h>
#include <sys/stat.h>


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

#else

int PlatformIsDir( const char* pszPath)
  {
  (void)pszPath;
  {int WARN_PlatformIsDir_STUB;}
  return 0;
  }


int PlatformMkdir( const char* pszPath)
  {
  (void)pszPath;
  return -1;
  }
#endif

/* End of file */
