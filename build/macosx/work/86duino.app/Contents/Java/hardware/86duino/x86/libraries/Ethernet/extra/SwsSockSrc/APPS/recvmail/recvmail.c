/*****************************************************************************
 *
 * File name   : recvmail.c
 * Function    : POP3 client (RFC 1939)
 * Authors     : Lawrence Rust
 * Systems     : ANSI C
 * Release Date: 13-Feb-04
 *
 *****************************************************************************
 * File History
 * ------------
 *
 * 13-Feb-04, LVR, Created.
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
#include "recvmail.h"

/* Imports */

/* ANSI C library */
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <ctype.h>
#include <signal.h> /* sig_atomic_t */
#if _MSC_VER >= 1000
#include <crtdbg.h>
#endif

#include "sws_net.h"
#include "sws_str.h"   /* SWS_stricmp */
#include "sws_ba64.h"
#include "sws_time.h"


/***************************************************************************
 * Macros & constants
 ***************************************************************************/

/* Length of an array - sorely missing from stddef.h */
#ifndef lengthof
#define lengthof( _a) ((size_t)( sizeof(_a)/sizeof((_a)[0]) ))
#endif

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

#define CRLF  "\x0d\x0a"
#define cNul  '\0'
#define QUOTE "\"" /* String constant quote mark */

#define EINTERRUPTED 11000
#define EQUIT        -2
#define INVALID_TIME ((time_t)-1)

#ifndef INPUT_BUFFER_SIZE
#define INPUT_BUFFER_SIZE 128 /* Command file input buffer size */
#endif

#ifndef MSG_NOSIGNAL
#define MSG_NOSIGNAL 0
#endif

#ifndef MAX_RELAY_SECS
#define MAX_RELAY_SECS (4 * 60 * 60) /* Max forwarding time */
#endif

#ifndef MAX_BACKWARD_SECS
#define MAX_BACKWARD_SECS (15 * 60) /* Max non-monotonic time */
#endif

/* If operators */
#define OP_EQ( s)       (0 == strcmp( s,"=") || 0 == SWS_stricmp( s,"EQ"))
#define OP_GT( s)       (0 == strcmp( s,">") || 0 == SWS_stricmp( s,"GT"))
#define OP_LT( s)       (0 == strcmp( s,"<") || 0 == SWS_stricmp( s,"LT"))
#define OP_AND( s)      (0 == strcmp( s,"&") || 0 == SWS_stricmp( s,"AND"))
#define OP_CONTAINS( s) (0 == SWS_stricmp( s,"contains"))

/* RFC 1939 constants */
#define POP3_PORT   110U
#ifndef POP3_BUFFER_SIZE
#define POP3_BUFFER_SIZE 512 /* RFC 1939 p2 input buffer size */
#endif

/* POP3 timeouts */
#define INIT_TIMEOUT (1 * 60) /* Seconds */
#define GEN_TIMEOUT  (1 * 60) /* Seconds */
#define SEND_TIMEOUT (30 * 1000) /* millisecs */


/***************************************************************************
 * Data types
 ***************************************************************************/

/* Command function */
typedef struct SMethod SMethod;
typedef int FMethod( SOCKET, char* pszTail);

/* Command descriptor */
struct SMethod
  {
  const char* pszName;
  FMethod* pfnExecute;
  const char* pszParams;
  const char* pszHelp;
  };

/* 'Received' trace info */
typedef struct SRecvTraceInfo
  {
  time_t tRecv;
  unsigned long ulSrcIp; /* Network form */
  char szFrom[ 64];
  char szHelo[ 64];
  char szExtra[ 80];
  char szBy[ 64];
  char szWith[ 32];
  } SRecvTraceInfo;

/* Message descriptor */
typedef struct SMessage
  {
  unsigned uId;
  int isPresent;
  int isDeleted;
  unsigned long ulSize;

  /* Required fields */
  time_t tDate;
  char szFromMbx[ 64];
  char szFromInfo[ 64];

  /* Info fields */
  char szTo[ 128];
  char szCc[ 128];
  char szSubj[ 256];
  char szContentType[ 32];
  char szCharset[ 32];
  char szMessageId[ 128];

  /* Trace info */
  char szReturnMbx[ 64];
  unsigned uRelays;
  int isRecvForged;
  int isRoutedLocally;
  SRecvTraceInfo recvInfo;
  } SMessage;

/* If condition function */
typedef struct SIfCondition SIfCondition;
typedef int FRule( SIfCondition*);

/* If condition descriptor */
typedef struct SRule
  {
  const char* pszName;
  FRule* pfnExecute;
  } SRule;

/* Do action function */
typedef struct SAction SAction;
typedef int FAction( SOCKET, SMessage*, void*);

/* Do action descriptor */
struct SAction
  {
  const char* pszName;
  FAction* pfnExecute;
  };

/* If actions */
typedef int FIfOp( const SIfCondition*, const SMessage*);
struct SIfCondition
  {
  SIfCondition *pPrev, *pNext;
  FIfOp* pfnOp;
  int bNot;
  FAction* pfnAction;
  union
    {
    struct
      {
      unsigned long ulIp; /* Network form */
      unsigned long ulMask;
      } addr;
    unsigned long ulValue;
    const char* pszValue;
    time_t tValue;
    unsigned uValue;
    } u;
  };


/***************************************************************************
 * Local function prototypes
 ***************************************************************************/

static int Interpret( SOCKET, int);
static int ExecuteRequest( SOCKET, char* pszCmd);
static const SMethod* FindCommand( const char*);

static int IfCondition( SOCKET, char*, SIfCondition*);
static const SRule* FindRule( const char*);
static const SAction* FindAction( const char*);
static int Apply( SOCKET, FAction*, void*);
static const char* ParseStringConstant( char*);

static int Retrieve( SOCKET, unsigned, const char* pszName);

static SMessage* GetMsg( SOCKET, unsigned);
static int GetMsgInfo( SOCKET s, SMessage*);
static void DisplayMsg( const SMessage*, FILE*, int);
static void SummMsg( const SMessage*, unsigned, FILE*);
static int SummPart( SOCKET, unsigned, unsigned, FILE*);
static int GetPart( SOCKET s, unsigned, const char*, FILE*);
static void MsgRecvInit( SRecvTraceInfo*);
static void MsgRecvUpdate( SMessage*, const SRecvTraceInfo*);
static int MsgInfoRecv( SRecvTraceInfo*, char*);
static int MsgInfoComment( SRecvTraceInfo*, char*);

static int MimeIsStringEncoded( const char*);
static int MimeDecodeString( const char*, unsigned char*, size_t);
static size_t MimeDecodeQuoted( unsigned char*, size_t, const char*, size_t);

static int DoCommand( SOCKET, const char*, char[], size_t, int);
static int GetResponse( SOCKET, char[], size_t, unsigned);
static int ReadLine( SOCKET, char [], size_t, unsigned);
static int ReadLines( SOCKET, FILE*, unsigned);

static int ConnectAsync( SOCKET, const struct sockaddr_in*, int);
static int RawRead( SOCKET, void*, size_t, unsigned);
static int RawWrite( SOCKET, const void*, size_t);

static char* SWS_strlwr( char * pszIo);


/***************************************************************************
 * Module data
 ***************************************************************************/

extern sig_atomic_t volatile g_exit; /* SIGINT indicator */
static FILE* s_logfile;
static int s_iVerbosity;
static unsigned s_uLines;
static char s_szBuf[ 1 + POP3_BUFFER_SIZE]; /* common buffer */

/* The method table */
static FMethod HelpMethod;
static FMethod CondsMethod;
static FMethod ActsMethod;
static FMethod NilMethod;
static FMethod NoopMethod;
static FMethod QuitCommand;
static FMethod Quit0Command;
static FMethod UserMethod;
static FMethod PassMethod;
static FMethod StatMethod;
static FMethod ListMethod;
static FMethod RetrMethod;
static FMethod TopMethod;
static FMethod DeleMethod;
static FMethod ResetMethod;
static FMethod ScanMethod;
static FMethod LinesMethod;
static FMethod SummMethod;
static FMethod SumpMethod;
static FMethod PartMethod;
static FMethod IfMethod;
static FMethod RptMethod;
static FMethod ExecMethod;
static const SMethod methods[] = {
  /* POP3 equivalents */
  { "user",  &UserMethod,  "<username>",    "Login" },
  { "pass",  &PassMethod,  "<password>",    "User password" },
  { "stat",  &StatMethod,  "",              "Show count and total size of messages" },
  { "list",  &ListMethod,  "[n]",           "Show message scan listing(s)" },
  { "top" ,  &TopMethod,   "<n> [m] [>file]", "Show top m lines of message n" },
  { "get",   &RetrMethod,  "<n> [>file]",   "Retrieve a message" },
  { "del",   &DeleMethod,  "<n>",           "Delete a message" },
  { "rst" ,  &ResetMethod, "",              "Cancel all pending deletions" },
  { "nop" ,  &NoopMethod,  "",              "No operation" },
  { "quit",  &QuitCommand, "",              "Quit" },
  /* Macros */
  { "quit0", &Quit0Command,"",              "Quit if no messages" },
  { "scan",  &ScanMethod,  "[n]",           "Show message(s) summary" },
  { "lines", &LinesMethod, "<n>",           "Set number of lines for scan" },
  { "summ",  &SummMethod,  "[n]",           "Show message(s) summary" },
  { "sump",  &SumpMethod,  "<n> [m]",       "Show summary of part m of message n" },
  { "part",  &PartMethod,  "<n> <s> [file]","Retrieve part s of message n" },
  /* Scripting */
  { "if",    &IfMethod,    "<cond> [and <cond> ...] <action>", "Conditionally apply action to messages" },
  { "cond",  &CondsMethod, "[cond]",        "List conditions" },
  { "act",   &ActsMethod,  "[action]",      "List actions" },
  { "rpt",   &RptMethod,   "<action>",      "Apply action to all messages" },
  { "exec",  &ExecMethod,  "[filename]",    "Execute commands from a script" },
  /* Others */
  { "help",  &HelpMethod,  "[command]",     "Show command help" },
  { "?",     &HelpMethod,  "[command]",     NULL },
  { "",      &NilMethod,   "",              NULL }
};

/* If conditions */
static FAction IfAction;
static FAction IfAndAction;

static FRule IfNot;

static FRule IfSize;
static FIfOp IfSizeEq;
static FIfOp IfSizeGt;
static FIfOp IfSizeLt;

static FRule IfFrom;
static FIfOp IfFromEq;
static FIfOp IfFromContains;

static FRule IfDate;
static FIfOp IfDateEq;
static FIfOp IfDateGt;
static FIfOp IfDateLt;

static FRule IfTo;
static FIfOp IfToEq;
static FIfOp IfToContains;

static FRule IfSubject;
static FIfOp IfSubjectEq;
static FIfOp IfSubjectContains;

static FRule IfContentType;
static FIfOp IfContentTypeEq;
static FIfOp IfContentTypeContains;

static FRule IfMessageId;
static FIfOp IfMessageIdEq;
static FIfOp IfMessageIdContains;

static FRule IfHelo;
static FIfOp IfHeloEq;
static FIfOp IfHeloContains;

static FRule IfSrcIp;
static FIfOp IfSrcIpEq;

static FRule IfRelays;
static FIfOp IfRelaysEq;
static FIfOp IfRelaysGt;
static FIfOp IfRelaysLt;

static FRule IfRecvWith;
static FIfOp IfRecvWithEq;
static FIfOp IfRecvWithContains;

static FRule IfRecvBy;
static FIfOp IfRecvByEq;
static FIfOp IfRecvByContains;

static FRule IfForged;
static FIfOp IfRecvForged;

static FRule IfComment;
static FIfOp IfCommentContains;

static FRule IfReturnPath;
static FIfOp IfReturnPathEq;
static FIfOp IfReturnPathContains;

static FRule IfRecvFrom;
static FIfOp IfRecvFromEq;
static FIfOp IfRecvFromContains;

static const SRule conditions[] = {
  { "not",     &IfNot },
  { "size",    &IfSize },
  { "from",    &IfFrom },
  { "date",    &IfDate },
  { "to",      &IfTo },
  { "subject", &IfSubject },
  { "id",      &IfMessageId },
  { "type",    &IfContentType },
  { "return",  &IfReturnPath },
  { "forgery", &IfForged },
  { "helo",    &IfHelo },
  { "srcip",   &IfSrcIp },
  { "relays",  &IfRelays },
  { "recvfrom", &IfRecvFrom },
  { "with",    &IfRecvWith },
  { "by",      &IfRecvBy },
  { "comment", &IfComment }
};

/* Do actions */
static FAction CopyAction;
static FAction MoveAction;
static FAction DelAction;

static const SAction actions[] = {
  { "copy",    &CopyAction },
  { "move",    &MoveAction },
  { "delete",  &DelAction }
};

static const char kszWhiteSpace[] = " \t\n";


/***************************************************************************
 * Functions
 ***************************************************************************/

/*
 * POP3 command interpreter
 */
extern int RecvMail(
  int isScript,                   /* 0= interactive */
  const void* pvHost,             /* -> server sockaddr_in */
  FILE* logfile,                  /* Logfile, NULL= none */
  int iVerbosity                  /* verbosity */
) {
  const struct sockaddr_in* const pAddr = (const struct sockaddr_in*)pvHost;
  SOCKET s;
  int iRet;

  s_logfile = logfile;
  s_iVerbosity = iVerbosity;
  s_uLines = NULL != s_logfile ? 4U : 0U;

  /* Server port specifed? */
  if ( AF_INET == pAddr->sin_family
    && 0 == pAddr->sin_port
  ) {
    /* No, use default */
    ((struct sockaddr_in*)pAddr)->sin_port = htons( POP3_PORT);
    }

  /* Create a socket */
  s = socket( pAddr->sin_family, SOCK_STREAM, 0);
  if ( INVALID_SOCKET == s)
    {
    iRet = GetLastError();
    fprintf( stderr, "Failed (%d) to create socket: %s\n",
      iRet, strerror( iRet));
    return iRet;
    }

  /* Set a send timeout */
  { DECL_TIMEOUT( tv, SEND_TIMEOUT);
  setsockopt(
    s,
    SOL_SOCKET, SO_SNDTIMEO,
    (char*)&tv, sizeof tv
  ); }

  /* Connect to server */
  if ( iVerbosity > 1 || (!isScript && iVerbosity > 0))
    {
    fprintf( stderr, "Connecting to %s:%u...",
      inet_ntoa( pAddr->sin_addr), ntohs( pAddr->sin_port)
    );
    }

  iRet = ConnectAsync( s, pAddr, iVerbosity > 1 || (!isScript && iVerbosity > 0));
  if ( 0 != iRet)
    {
    closesocket( s);
    switch ( iRet)
      {
    case EINTERRUPTED:
      fputs( " Interrupted.\n", stderr);
      break;
    case ETIMEDOUT:
    case EWOULDBLOCK:
#if EAGAIN != EWOULDBLOCK
    case EAGAIN:
#endif
      if ( iVerbosity >= 0)
        fputs( "Host not responding.\n", stderr);
      break;
    case ECONNREFUSED:
      if ( iVerbosity >= 0)
        fputs( "Connection refused.\n", stderr);
      break;
    default:
      fprintf( stderr, "Failed (%d): %s\n", iRet, strerror( iRet));
      break;
      }

    return iRet;
    }

  if ( iVerbosity > 1 || (!isScript && iVerbosity > 0))
    fputs( " ok\n", stderr);

  /* Get signon */
  iRet = GetResponse( s, s_szBuf, sizeof s_szBuf, INIT_TIMEOUT);
  if ( 0 == iRet)
    {
    if ( iVerbosity > 0 && (s_logfile == NULL || s_logfile != stdout))
      fputs( s_szBuf, stdout), fflush( stdout);

    /* Interpret stdin */
    iRet = Interpret( s, isScript);
    if ( EQUIT == iRet)
      iRet = 0;
    }
  else
    {
    /* Bad signon, probably not a POP3 service */
    if ( s_szBuf[0] && iVerbosity > 0)
      fprintf( stderr, "Invalid sign-on: %s\n", s_szBuf);

    iRet = 1;
    }

  /* Disconnect server */
  closesocket( s);

  return iRet;
  }


/*
 * Command line interpreter
 */
static int Interpret( SOCKET s, int isScript)
  {
  static int s_iDepth;
  int iRet = 0;

  if ( ++s_iDepth > 4)
    {
    --s_iDepth;
    fputs( "Nesting too deep\n", stderr);
    return 1;
    }

  do
    {
    char* psz;
    static char szBuf[ INPUT_BUFFER_SIZE];

    if ( g_exit)
      {
      g_exit = 0;
      iRet = EINTERRUPTED;
      break;
      }

    /* Issue a prompt */
    if ( !isScript)
      {
      fprintf( stderr, "%d>", s_iDepth);
#if SWS_USE_SOCKLIB
      Sleep( 200); /* Pump swssock events */
#endif
      }

    /* Get user input */
    if ( NULL == fgets( szBuf, sizeof( szBuf), stdin))
      {
      /* EOF */
      if ( !isScript)
        fputs( "\n", stderr);

      iRet = g_exit ? EINTERRUPTED : 0;
      g_exit = 0;
      break;
      }

    if ( !isScript)
      fputs( "\r", stderr);

    /* Zap the trailing newline */
    psz = strchr( szBuf, '\n');
    if ( NULL != psz)
      {
      *psz = '\0';
      }
    else if ( !feof( stdin))
      {
      fputs( "Input truncated\n", stderr);
      if ( !isScript)
        {
        rewind( stdin);
        continue;
        }

      iRet = 1;
      break;
      }

    /* Interpret command */
    iRet = ExecuteRequest( s, szBuf);
    }
  while ( iRet == 0 || (iRet > 0 && !isScript));

  --s_iDepth;

  return iRet;
  }


/*
 * Execute a command
 */
static int ExecuteRequest(
  SOCKET s,
  char* pszCmd
) {
  char *pszTail;
  const SMethod* pMethod;
  int iRet;

  /* Skip leading space */
  while ( isspace( *pszCmd))
    ++pszCmd;

  if ( ';' == *pszCmd)
    return 0;

  /* Log the command */
  if ( NULL != s_logfile)
    fprintf( s_logfile, ">%s\n", pszCmd);

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
    /* Skip leading space */
    while ( cNul != *pszTail && isspace( *pszTail))
      ++pszTail;
    }

  /* Find the command */
  pMethod = FindCommand( pszCmd);
  if ( NULL != pMethod)
    {
    /* Execute the command */
    iRet = (*pMethod->pfnExecute)( s, pszTail);
    }
  else
    {
    fprintf( stderr, "Unknown command: %s\n", pszCmd);
    iRet = 1;
    }

  return iRet;
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


/***************************************************************************
 * Commands
 ***************************************************************************/
static int HelpMethod( SOCKET s, char* pszTail)
  {
  const SMethod* pMethod;
  int iRet = 0;

  (void)s;

  if ( cNul == *pszTail)
    {
    int i;

    fputs( "Commands available:\n", stdout);

    for (
      pMethod = methods, i = sizeof( methods) / sizeof( methods[0]);
      --i >= 0;
      ++pMethod
    ) {
      if ( pMethod->pszHelp)
        fprintf( stdout, "%-5s %-16s : %s\n",
          pMethod->pszName, pMethod->pszParams, pMethod->pszHelp);
      }
    }
  else if ( NULL != (pMethod = FindCommand( pszTail)) )
    {
    fprintf( stdout, "%s %s : %s\n",
      pMethod->pszName, pMethod->pszParams, pMethod->pszHelp);
    }
  else
    {
    fprintf( stderr, "Unknown command: %s\n", pszTail);
    iRet = 1;
    }

  return iRet;
  }


static int CondsMethod( SOCKET s, char* pszTail)
  {
  const SRule* pRule;
  int iRet = 0;

  (void)s;

  if ( cNul == *pszTail)
    {
    int i;

    for (
      pRule = conditions, i = lengthof( conditions);
      --i >= 0;
      ++pRule
    ) {
      fprintf( stdout, "%s ", pRule->pszName);
      }
    fputs( "\n", stdout);
    }
  else if ( NULL != (pRule = FindRule( pszTail)) )
    {
    fprintf( stdout, "%s\n", pRule->pszName);
    }
  else
    {
    fprintf( stderr, "Unknown condition: %s\n", pszTail);
    iRet = 1;
    }

  return iRet;
  }


static int ActsMethod( SOCKET s, char* pszTail)
  {
  const SAction* pAct;
  int iRet = 0;

  (void)s;

  if ( cNul == *pszTail)
    {
    int i;

    for (
      pAct = actions, i = lengthof( actions);
      --i >= 0;
      ++pAct
    ) {
      fprintf( stdout, "%s ", pAct->pszName);
      }
    fputs( "\n", stdout);
    }
  else if ( NULL != (pAct = FindAction( pszTail)) )
    {
    fprintf( stdout, "%s\n", pAct->pszName);
    }
  else
    {
    fprintf( stderr, "Unknown action: %s\n", pszTail);
    iRet = 1;
    }

  return iRet;
  }


static int NilMethod( SOCKET s, char* pszTail)
  {
  (void)s;
  (void)pszTail;
  return 0;
  }


static int NoopMethod( SOCKET s, char* pszTail)
  {
  STATIC_ASSERT( sizeof s_szBuf > 260);
  sprintf( s_szBuf, "NOOP%c%.255s", *pszTail ? ' ' : cNul, pszTail);
  return DoCommand( s, s_szBuf, s_szBuf, sizeof s_szBuf, s_iVerbosity > 0 );
  }


static int QuitCommand( SOCKET s, char* pszTail)
  {
  sprintf( s_szBuf, "QUIT%c%.255s", *pszTail ? ' ' : cNul, pszTail);
  (void)DoCommand( s, s_szBuf, s_szBuf, sizeof s_szBuf, s_iVerbosity > 0 );

  return EQUIT;
  }

/*
 * Quit if no messages to download
 */
static int Quit0Command( SOCKET s, char* pszTail)
  {
  int iRet;
  unsigned uMessages;
  unsigned long ulBytes;

  if ( 0 != (iRet = DoCommand( s, "STAT", s_szBuf, sizeof s_szBuf, s_iVerbosity > 1 )) )
    return iRet;

  if ( 2 != sscanf( s_szBuf, "+OK %u %lu", &uMessages, &ulBytes))
    return 0;

  if ( 0 != uMessages && 0 != ulBytes )
    return 0;

  sprintf( s_szBuf, "QUIT%c%.255s", *pszTail ? ' ' : cNul, pszTail);
  (void)DoCommand( s, s_szBuf, s_szBuf, sizeof s_szBuf, s_iVerbosity > 0 );

  return EQUIT;
  }


static int UserMethod( SOCKET s, char* pszTail)
  {
  GetMsg( INVALID_SOCKET, 0); /* Reset cached header info */
  sprintf( s_szBuf, "USER%c%.255s", *pszTail ? ' ' : cNul, pszTail);
  return DoCommand( s, s_szBuf, s_szBuf, sizeof s_szBuf, s_iVerbosity > 0 );
  }


static int PassMethod( SOCKET s, char* pszTail)
  {
  sprintf( s_szBuf, "PASS%c%.255s", *pszTail ? ' ' : cNul, pszTail);
  return DoCommand( s, s_szBuf, s_szBuf, sizeof s_szBuf, 0 );
  }


static int StatMethod( SOCKET s, char* pszTail)
  {
  sprintf( s_szBuf, "STAT%c%.255s", *pszTail ? ' ' : cNul, pszTail);
  return DoCommand( s, s_szBuf, s_szBuf, sizeof s_szBuf, s_iVerbosity > 0 );
  }


static int ListMethod( SOCKET s, char* pszTail)
  {
  int iRet;

  sprintf( s_szBuf, "LIST%c%.255s", *pszTail ? ' ' : cNul, pszTail);
  iRet = DoCommand( s, s_szBuf, s_szBuf, sizeof s_szBuf, s_iVerbosity > 0 );
  if ( 0 == iRet && cNul == *pszTail)
    {
    /* Echo multi line response */
    iRet = ReadLines( s, stdout, GEN_TIMEOUT);
    }

  return iRet;
  }


static int DeleMethod( SOCKET s, char* pszTail)
  {
  SMessage* pMsg;
  int iRet;

  /* Grab the message list before any deletions */
  pMsg = GetMsg( s, atoi( pszTail));

  sprintf( s_szBuf, "DELE%c%.255s", *pszTail ? ' ' : cNul, pszTail);
  iRet = DoCommand( s, s_szBuf, s_szBuf, sizeof s_szBuf, s_iVerbosity > 0 );
  if ( 0 == iRet && NULL != pMsg)
    pMsg->isDeleted = 1;

  return iRet;
  }


static int ResetMethod( SOCKET s, char* pszTail)
  {
  int iRet;

  sprintf( s_szBuf, "RSET%c%.255s", *pszTail ? ' ' : cNul, pszTail);
  iRet = DoCommand( s, s_szBuf, s_szBuf, sizeof s_szBuf, s_iVerbosity > 0 );
  if ( 0 == iRet && cNul == *pszTail)
    {
    /* Reset deleted bit on cached message info */
    SMessage* pMsg;
    unsigned uMsg;
    for ( uMsg = 1; NULL != (pMsg = GetMsg( INVALID_SOCKET, uMsg)); ++uMsg)
      pMsg->isDeleted = 0;
    }

  return iRet;
  }


static int RetrMethod( SOCKET s, char* pszTail)
  {
  char* pszName;
  unsigned uMsg;

  /* Get filename */
  pszName = strchr( pszTail, '>');
  if ( NULL != pszName)
    {
    /* Strip trailing whitespace from command tail */
    char* psz = pszName - 1;
    while ( isspace( *psz))
      *psz-- = cNul;

    /* Skip leading whitespace from filename */
    *(pszName++) = cNul;
    while ( isspace( *pszName))
      ++pszName;
    }

  if ( 1 != sscanf( pszTail, "%u", &uMsg))
    {
    fputs( "Invalid message number\n", stderr);
    return 1;
    }

  return Retrieve( s, uMsg, pszName);
  }

static int Retrieve( SOCKET s, unsigned uId, const char* pszName)
  {
  int iRet;
  FILE* outfile;
  char szName[ 13];

  if ( NULL == pszName)
    {
    /* Generate a unique filename */
    pszName = szName;
    for (;;)
      {
      static unsigned s_uCount;
      sprintf( szName, "msg%05u.eml", s_uCount++);
      outfile = fopen( pszName, "r");
      if ( NULL == outfile)
        break;
      fclose( outfile);
      }
    }

  /* Open the output file */
  outfile = fopen( pszName, "w");
  if ( NULL == outfile)
    {
    iRet = errno;
    fprintf( stderr, "Failed to create '%s': %s\n",
      pszName, strerror( iRet) );
    return iRet;
    }

  sprintf( s_szBuf, "RETR %u", uId);
  iRet = DoCommand( s, s_szBuf, s_szBuf, sizeof s_szBuf, s_iVerbosity > 0 );
  if ( 0 == iRet)
    {
    /* Direct multi line response to outfile */
    iRet = ReadLines( s, outfile, GEN_TIMEOUT);
    }

  fclose( outfile);
  if ( 0 == iRet)
    {
    if ( NULL != s_logfile)
      fprintf( s_logfile, "Message %u written to %s\n", uId, pszName);
    if ( s_logfile != stdout)
      fprintf( stdout, "Message %u written to %s\n", uId, pszName);
    }
  else
    {
    remove( pszName);
    }

  return iRet;
  }


static int TopMethod( SOCKET s, char* pszTail)
  {
  int iRet;
  FILE* outfile;
  char *pszName;

  /* Get filename */
  pszName = strchr( pszTail, '>');
  if ( NULL != pszName)
    {
    /* Strip trailing whitespace from command tail */
    char* psz = pszName - 1;
    while ( isspace( *psz))
      *psz-- = cNul;

    /* Skip leading whitespace from filename */
    *(pszName++) = cNul;
    while ( isspace( *pszName))
      ++pszName;

    /* Open the output file */
    outfile = fopen( pszName, "w");
    if ( NULL == outfile)
      {
      iRet = errno;
      fprintf( stderr, "Failed to create '%s': %s\n",
        pszName, strerror( iRet));
      return iRet;
      }
    }
  else
    {
    outfile = stdout;
    }

  sprintf( s_szBuf, "TOP%c%.255s", *pszTail ? ' ' : cNul, pszTail);
  iRet = DoCommand( s, s_szBuf, s_szBuf, sizeof s_szBuf, s_iVerbosity > 0 );
  if ( 0 == iRet)
    {
    /* Direct multi line response to outfile */
    iRet = ReadLines( s, outfile, GEN_TIMEOUT);
    }

  if ( NULL != pszName)
    {
    fclose( outfile);
    if ( 0 != iRet)
      remove( pszName);
    else
      {
      if ( NULL != s_logfile)
        fprintf( s_logfile, "Message header written to %s\n", pszName);
      if ( stdout != s_logfile)
        fprintf( stdout, "Message header written to %s\n", pszName);
      }
    }

  return iRet;
  }


/*
 * Display summary of all messages
 */
static int ScanMethod( SOCKET s, char* pszTail)
  {
  int iRet = 1; /* Assume failure */
  unsigned uMsg;
  const SMessage* pMsg;

  if ( 1 == sscanf( pszTail, "%u", &uMsg))
    {
    if ( 0 == uMsg)
      {
      /* Empty the message cache */
      fputs( "Message cache cleared\n", stdout);
      (void)GetMsg( INVALID_SOCKET, 0);
      iRet = 0;
      }
    else if ( NULL != (pMsg = GetMsg( s, uMsg)) )
      {
      if ( NULL != s_logfile)
        DisplayMsg( pMsg, s_logfile, 2);
      if ( stdout != s_logfile)
        DisplayMsg( pMsg, stdout, s_iVerbosity);
      iRet = 0;
      }
    else
      {
      fputs( "No such message\n", stderr);
      }
    }
  else if ( *pszTail)
    {
    fputs( "Invalid message number\n", stderr);
    }
  else
    {
    /* Scan all messages */
    iRet = 0;
    for ( uMsg = 1; NULL != (pMsg = GetMsg( s, uMsg)); ++uMsg)
      {
      if ( NULL != s_logfile)
        DisplayMsg( pMsg, s_logfile, 2);
      if ( stdout != s_logfile)
        DisplayMsg( pMsg, stdout, s_iVerbosity);
      }
    }

  return iRet;
  }


/*
 * Set number of lines read by GetMsgInfo
 */
static int LinesMethod( SOCKET s, char* pszTail)
  {
  int iRet;
  (void)s;

  if ( 1 == sscanf( pszTail, "%u", &s_uLines))
    {
    iRet = 0;
    }
  else if ( *pszTail)
    {
    fputs( "Invalid number\n", stderr);
    iRet = 1;
    }
  else
    {
    fprintf( stdout, "Lines= %u\n", s_uLines);
    iRet = 0;
    }

  return iRet;
  }


/*
 * Display summary of message(s)
 */
static int SummMethod( SOCKET s, char* pszTail)
  {
  int iRet = 1; /* Assume failure */
  unsigned uMsg;
  const SMessage* pMsg;

  if ( 1 == sscanf( pszTail, "%u", &uMsg))
    {
    if ( 0 == uMsg)
      {
      /* Empty the message cache */
      fputs( "Message cache cleared\n", stdout);
      (void)GetMsg( INVALID_SOCKET, 0);
      iRet = 0;
      }
    else if ( NULL != (pMsg = GetMsg( s, uMsg)) )
      {
      SummMsg( pMsg, pMsg->uId, stdout);
      iRet = 0;
      }
    else
      {
      fputs( "No such message\n", stderr);
      }
    }
  else if ( *pszTail)
    {
    fputs( "Invalid message number\n", stderr);
    }
  else
    {
    /* Scan all messages */
    unsigned uId = 0;
    iRet = 0;
    for ( uMsg = 1; NULL != (pMsg = GetMsg( s, uMsg)); ++uMsg)
      {
      if ( !pMsg->isDeleted)
        SummMsg( pMsg, ++uId, stdout);
      }
    }

  return iRet;
  }


/*
 * Display summary of message part(s)
 */
static int SumpMethod( SOCKET s, char* pszTail)
  {
  int iRet = 1; /* Assume failure */
  unsigned uMsg, uPart = 0;

  if ( 1 <= sscanf( pszTail, "%u %u", &uMsg, &uPart))
    {
    unsigned uMsgs;

    if ( uMsg > 0
      &&  0 == DoCommand( s, "STAT", s_szBuf, sizeof s_szBuf, 0)
      && 1 == sscanf( s_szBuf, "+OK %u", &uMsgs)
      && uMsg <= uMsgs
    ) {
      iRet = SummPart( s, uMsg, uPart, stdout);
      }
    else
      fputs( "No such message\n", stderr);
    }
  else
    fputs( "Invalid syntax\n", stderr);

  return iRet;
  }


/*
 * Store part of a multipart message
 */
static int PartMethod( SOCKET s, char* pszTail)
  {
  int iRet = 1; /* Assume failure */
  unsigned uMsg;
  char szPath[ 256] = "";
  char szPart[ 128];

  if ( 2 <= sscanf( pszTail, "%u %127s %255s", &uMsg, szPart, szPath))
    {
    unsigned uMsgs;

    if ( uMsg > 0
      &&  0 == DoCommand( s, "STAT", s_szBuf, sizeof s_szBuf, 0)
      && 1 == sscanf( s_szBuf, "+OK %u", &uMsgs)
      && uMsg <= uMsgs
    ) {
      FILE* file = szPath[0] ? fopen( szPath, "w") : stdout;
      if ( NULL != file)
        {
        iRet = GetPart( s, uMsg, szPart, file);

        if ( stdout != file)
          fclose( file);
        }
      else
        fprintf( stderr, "Can't open %s: %s\n", szPath, strerror( errno));
      }
    else
      fputs( "No such message\n", stderr);
    }
  else
    fputs( "Invalid syntax\n", stderr);

  return iRet;
  }


/*
 * Conditionally apply a command to all messages
 */
static int IfMethod( SOCKET s, char* pszTail)
  {
  /* Parse the condition */
  return IfCondition( s, strtok( pszTail, kszWhiteSpace), NULL);
  }

static int IfCondition(
  SOCKET s,
  char* pszCond,
  SIfCondition* pCond
) {
  int iRet;
  char* psz;
  const SRule* pRule;
  const SAction* pAction;
  SIfCondition c;

  if ( NULL == pszCond)
    {
    fputs( "Missing condition\n", stderr);
    return 1;
    }

  /* Lookup the condition */
  pRule = FindRule( pszCond);
  if ( NULL == pRule)
    {
    fputs( "Unsupported condition\n", stderr);
    return 1;
    }

  /* Append condition to 'and' list */
  c.pNext = NULL;
  c.pPrev = pCond;
  if ( NULL != pCond)
    pCond->pNext = &c;

  c.bNot = 0;

  /* Execute the condition handler code */
  iRet = (*pRule->pfnExecute)( &c);
  if ( iRet)
    return iRet;

  /* Parse action */
  psz = strtok( NULL, kszWhiteSpace);
  if ( NULL == psz)
    {
    fputs( "Missing action\n", stderr);
    return 1;
    }

  if ( OP_AND( psz))
    {
    /* Recursively parse next condition */
    c.pfnAction = &IfAndAction;
    return IfCondition( s, strtok( NULL, kszWhiteSpace), &c);
    }

  pAction = FindAction( psz);
  if ( NULL == pAction)
    {
    fputs( "Unsupported action\n", stderr);
    return 1;
    }
  c.pfnAction = pAction->pfnExecute;

  /* Find start of condtion list */
  pCond = &c;
  while ( NULL != pCond->pPrev)
    pCond = pCond->pPrev;

  /* Apply the rule to all messages */
  return Apply( s, &IfAction, pCond);
  }


static int IfAction( SOCKET s, SMessage* pMsg, void* pv)
  {
  const SIfCondition* const pCond = (const SIfCondition*)pv;
  int bResult;

  bResult = (*pCond->pfnOp)( pCond, pMsg);
  if ( pCond->bNot)
    bResult = !bResult;

  /* If condition is met then do action */
  if ( bResult)
    return (*pCond->pfnAction)( s, pMsg, (void*)pCond);

  return 0;
  }

static int IfAndAction( SOCKET s, SMessage* pMsg, void* pv)
  {
  const SIfCondition* pCond = (const SIfCondition*)pv;

  /* Step onto next condition */
  pCond = pCond->pNext;
  if ( NULL == pCond)
    return 0;

  return IfAction( s, pMsg, (void*)pCond);
  }


/*
 * Unconditionally apply a command to all messages
 */
static int RptMethod( SOCKET s, char* pszTail)
  {
  char* psz;
  const SAction* pAction;

  /* Get the action */
  psz = strtok( pszTail, kszWhiteSpace);
  if ( NULL == psz)
    {
    fputs( "Missing action\n", stderr);
    return 1;
    }

  /* Lookup action */
  pAction = FindAction( psz);
  if ( NULL == pAction)
    {
    fputs( "Unsupported action\n", stderr);
    return 1;
    }

  /* Apply action to all messages */
  return Apply( s, pAction->pfnExecute, NULL);
  }


/*
 * Redirect input
 */
static int ExecMethod( SOCKET s, char* pszTail)
  {
  char* psz;
  FILE* file;
  int iRet;

  /* Get the filename */
  psz = strtok( pszTail, kszWhiteSpace);
  if ( NULL == psz)
    {
    file = stdin;
    }
  else if ( NULL == (file = fopen( psz, "r")) )
    {
    fputs( "Failed to open file\n", stderr);
    return 1;
    }

  iRet = Interpret( s, 1);

  if ( NULL != psz)
    fclose( file);

  return iRet;
  }


/****************************************************************************
 * If conditions
 ***************************************************************************/

static const SRule* FindRule( const char* psz)
  {
  const SRule* pRule;
  int i;

  for ( pRule = conditions, i = lengthof( conditions); i; --i, ++pRule)
    {
    if ( 0 == SWS_stricmp( psz, pRule->pszName) )
      return pRule;
    }
  return NULL;
  }


/*
 * If not
 */
static int IfNot( SIfCondition* pCond)
  {
  const SRule* pRule;
  char* pszCond;

  /* Get condition */
  pszCond = strtok( NULL, kszWhiteSpace);
  if ( NULL == pszCond)
    {
    fputs( "Missing condition\n", stderr);
    return 1;
    }

  /* Lookup the condition */
  pRule = FindRule( pszCond);
  if ( NULL == pRule)
    {
    fputs( "Unsupported condition\n", stderr);
    return 1;
    }

  pCond->bNot = 1;

  /* Execute the condition handler code */
  return (*pRule->pfnExecute)( pCond);
  }


/*
 * If message size
 */
static int IfSize( SIfCondition* pCond)
  {
  const char* psz;

  /* Parse operator */
  psz = strtok( NULL, kszWhiteSpace);
  if ( NULL == psz)
    {
    fputs( "Missing operator\n", stderr);
    return 1;
    }
  if ( OP_GT( psz))
    {
    pCond->pfnOp = &IfSizeGt;
    }
  else if ( OP_LT( psz))
    {
    pCond->pfnOp = &IfSizeLt;
    }
  else if ( OP_EQ( psz))
    {
    pCond->pfnOp = &IfSizeEq;
    }
  else
    {
    fputs( "Unknown operator\n", stderr);
    return 1;
    }

  /* Parse rhs value */
  psz = strtok( NULL, kszWhiteSpace);
  if ( NULL == psz)
    {
    fputs( "Missing numeric value\n", stderr);
    return 1;
    }
  if ( 1 != sscanf( psz, "%lu", &pCond->u.ulValue))
    {
    fputs( "Invalid value\n", stderr);
    return 1;
    }

  return 0;
  }

static int IfSizeGt( const SIfCondition* pCond, const SMessage* pMsg)
  {
  return pMsg->ulSize > pCond->u.ulValue;
  }

static int IfSizeLt( const SIfCondition* pCond, const SMessage* pMsg)
  {
  return pMsg->ulSize < pCond->u.ulValue;
  }

static int IfSizeEq( const SIfCondition* pCond, const SMessage* pMsg)
  {
  return pMsg->ulSize == pCond->u.ulValue;
  }


/*
 * If message from name
 */
static int IfFrom( SIfCondition* pCond)
  {
  const char* psz;
  char* pszValue;

  /* Parse operator */
  psz = strtok( NULL, kszWhiteSpace);
  if ( NULL == psz)
    {
    fputs( "Missing operator\n", stderr);
    return 1;
    }
  if ( OP_EQ( psz))
    {
    pCond->pfnOp = &IfFromEq;
    }
  else if ( OP_CONTAINS( psz))
    {
    pCond->pfnOp = &IfFromContains;
    }
  else
    {
    fputs( "Unknown operator\n", stderr);
    return 1;
    }

  /* Parse rhs value */
  psz = ParseStringConstant( strtok( NULL, ""));
  if ( NULL == psz)
    {
    fputs( "Missing or invalid string constant\n", stderr);
    return 1;
    }

  pszValue = (char*)malloc( 1 + strlen( psz));
  if ( NULL == pszValue)
    {
    fputs( "Out of memory\n", stderr);
    return 1;
    }
  pCond->u.pszValue = strcpy( pszValue, psz);

  return 0;
  }

static int IfFromEq( const SIfCondition* pCond, const SMessage* pMsg)
  {
  return 0 == SWS_stricmp( pMsg->szFromMbx, pCond->u.pszValue)
    || 0 == SWS_stricmp( pMsg->szFromInfo, pCond->u.pszValue);
  }

static int IfFromContains( const SIfCondition* pCond, const SMessage* pMsg)
  {
  return NULL != strstr( pMsg->szFromMbx, pCond->u.pszValue)
    || NULL != strstr( pMsg->szFromInfo, pCond->u.pszValue);
  }


/*
 * If message date
 */
static int IfDate( SIfCondition* pCond)
  {
  const char* psz;

  /* Parse operator */
  psz = strtok( NULL, kszWhiteSpace);
  if ( NULL == psz)
    {
    fputs( "Missing operator\n", stderr);
    return 1;
    }
  if ( OP_GT( psz))
    {
    pCond->pfnOp = &IfDateGt;
    }
  else if ( OP_LT( psz))
    {
    pCond->pfnOp = &IfDateLt;
    }
  else if ( OP_EQ( psz))
    {
    pCond->pfnOp = &IfDateEq;
    }
  else
    {
    fputs( "Unknown operator\n", stderr);
    return 1;
    }

  /* Parse rhs value */
  psz = ParseStringConstant( strtok( NULL, ""));
  if ( NULL == psz)
    {
    fputs( "Missing or invalid date\n", stderr);
    return 1;
    }

  pCond->u.tValue = SWS_TimeFromStr( psz);
  if ( INVALID_TIME == pCond->u.tValue)
    {
    fputs( "Invalid date\n", stderr);
    return 1;
    }

  return 0;
  }

static int IfDateEq( const SIfCondition* pCond, const SMessage* pMsg)
  {
  return INVALID_TIME != pMsg->tDate
    && difftime( pMsg->tDate, pCond->u.tValue) == 0;
  }

static int IfDateLt( const SIfCondition* pCond, const SMessage* pMsg)
  {
  return INVALID_TIME != pMsg->tDate
    && difftime( pMsg->tDate, pCond->u.tValue) < 0;
  }

static int IfDateGt( const SIfCondition* pCond, const SMessage* pMsg)
  {
  return INVALID_TIME != pMsg->tDate
    && difftime( pMsg->tDate, pCond->u.tValue) > 0;
  }


/*
 * If message to name
 */
static int IfTo( SIfCondition* pCond)
  {
  const char* psz;
  char* pszValue;

  /* Parse operator */
  psz = strtok( NULL, kszWhiteSpace);
  if ( NULL == psz)
    {
    fputs( "Missing operator\n", stderr);
    return 1;
    }
  if ( OP_EQ( psz))
    {
    pCond->pfnOp = &IfToEq;
    }
  else if ( OP_CONTAINS( psz))
    {
    pCond->pfnOp = &IfToContains;
    }
  else
    {
    fputs( "Unknown operator\n", stderr);
    return 1;
    }

  /* Parse rhs value */
  psz = ParseStringConstant( strtok( NULL, ""));
  if ( NULL == psz)
    {
    fputs( "Missing or invalid string constant\n", stderr);
    return 1;
    }

  pszValue = (char*)malloc( 1 + strlen( psz));
  if ( NULL == pszValue)
    {
    fputs( "Out of memory\n", stderr);
    return 1;
    }
  pCond->u.pszValue = strcpy( pszValue, psz);

  return 0;
  }

static int IfToEq( const SIfCondition* pCond, const SMessage* pMsg)
  {
  return 0 == SWS_stricmp( pMsg->szTo, pCond->u.pszValue);
  }

static int IfToContains( const SIfCondition* pCond, const SMessage* pMsg)
  {
  return NULL != strstr( pMsg->szTo, pCond->u.pszValue);
  }


/*
 * If message subject name
 */
static int IfSubject( SIfCondition* pCond)
  {
  const char* psz;
  char* pszValue;

  /* Parse operator */
  psz = strtok( NULL, kszWhiteSpace);
  if ( NULL == psz)
    {
    fputs( "Missing operator\n", stderr);
    return 1;
    }
  if ( OP_EQ( psz))
    {
    pCond->pfnOp = &IfSubjectEq;
    }
  else if ( OP_CONTAINS( psz))
    {
    pCond->pfnOp = &IfSubjectContains;
    }
  else
    {
    fputs( "Unknown operator\n", stderr);
    return 1;
    }

  /* Parse rhs value */
  psz = ParseStringConstant( strtok( NULL, ""));
  if ( NULL == psz)
    {
    fputs( "Missing or invalid string constant\n", stderr);
    return 1;
    }

  pszValue = (char*)malloc( 1 + strlen( psz));
  if ( NULL == pszValue)
    {
    fputs( "Out of memory\n", stderr);
    return 1;
    }
  pCond->u.pszValue = strcpy( pszValue, psz);

  return 0;
  }

static int IfSubjectEq( const SIfCondition* pCond, const SMessage* pMsg)
  {
  return 0 == SWS_stricmp( pMsg->szSubj, pCond->u.pszValue);
  }

static int IfSubjectContains( const SIfCondition* pCond, const SMessage* pMsg)
  {
  return NULL != strstr( pMsg->szSubj, pCond->u.pszValue);
  }


/*
 * If message content-type
 */
static int IfContentType( SIfCondition* pCond)
  {
  const char* psz;
  char* pszValue;

  /* Parse operator */
  psz = strtok( NULL, kszWhiteSpace);
  if ( NULL == psz)
    {
    fputs( "Missing operator\n", stderr);
    return 1;
    }
  if ( OP_EQ( psz))
    {
    pCond->pfnOp = &IfContentTypeEq;
    }
  else if ( OP_CONTAINS( psz))
    {
    pCond->pfnOp = &IfContentTypeContains;
    }
  else
    {
    fputs( "Unknown operator\n", stderr);
    return 1;
    }

  /* Parse rhs value */
  psz = ParseStringConstant( strtok( NULL, ""));
  if ( NULL == psz)
    {
    fputs( "Missing or invalid string constant\n", stderr);
    return 1;
    }

  pszValue = (char*)malloc( 1 + strlen( psz));
  if ( NULL == pszValue)
    {
    fputs( "Out of memory\n", stderr);
    return 1;
    }
  pCond->u.pszValue = SWS_strlwr( strcpy( pszValue, psz));

  return 0;
  }

static int IfContentTypeEq( const SIfCondition* pCond, const SMessage* pMsg)
  {
  return 0 == SWS_stricmp( pMsg->szContentType, pCond->u.pszValue);
  }

static int IfContentTypeContains( const SIfCondition* pCond, const SMessage* pMsg)
  {
  return NULL != strstr( pMsg->szContentType, pCond->u.pszValue);
  }


/*
 * If message id
 */
static int IfMessageId( SIfCondition* pCond)
  {
  const char* psz;
  char* pszValue;

  /* Parse operator */
  psz = strtok( NULL, kszWhiteSpace);
  if ( NULL == psz)
    {
    fputs( "Missing operator\n", stderr);
    return 1;
    }
  if ( OP_EQ( psz))
    {
    pCond->pfnOp = &IfMessageIdEq;
    }
  else if ( OP_CONTAINS( psz))
    {
    pCond->pfnOp = &IfMessageIdContains;
    }
  else
    {
    fputs( "Unknown operator\n", stderr);
    return 1;
    }

  /* Parse rhs value */
  psz = ParseStringConstant( strtok( NULL, ""));
  if ( NULL == psz)
    {
    fputs( "Missing or invalid string constant\n", stderr);
    return 1;
    }

  pszValue = (char*)malloc( 1 + strlen( psz));
  if ( NULL == pszValue)
    {
    fputs( "Out of memory\n", stderr);
    return 1;
    }
  pCond->u.pszValue = strcpy( pszValue, psz);

  return 0;
  }

static int IfMessageIdEq( const SIfCondition* pCond, const SMessage* pMsg)
  {
  return 0 == SWS_stricmp( pMsg->szMessageId, pCond->u.pszValue);
  }

static int IfMessageIdContains( const SIfCondition* pCond, const SMessage* pMsg)
  {
  return NULL != strstr( pMsg->szMessageId, pCond->u.pszValue);
  }


/*
 * If message received by
 */
static int IfReturnPath( SIfCondition* pCond)
  {
  const char* psz;
  char* pszValue;

  /* Parse operator */
  psz = strtok( NULL, kszWhiteSpace);
  if ( NULL == psz)
    {
    fputs( "Missing operator\n", stderr);
    return 1;
    }
  if ( OP_EQ( psz))
    {
    pCond->pfnOp = &IfReturnPathEq;
    }
  else if ( OP_CONTAINS( psz))
    {
    pCond->pfnOp = &IfReturnPathContains;
    }
  else
    {
    fputs( "Unknown operator\n", stderr);
    return 1;
    }

  /* Parse rhs value */
  psz = ParseStringConstant( strtok( NULL, ""));
  if ( NULL == psz)
    {
    fputs( "Missing or invalid string constant\n", stderr);
    return 1;
    }

  pszValue = (char*)malloc( 1 + strlen( psz));
  if ( NULL == pszValue)
    {
    fputs( "Out of memory\n", stderr);
    return 1;
    }
  pCond->u.pszValue = strcpy( pszValue, psz);

  return 0;
  }

static int IfReturnPathEq( const SIfCondition* pCond, const SMessage* pMsg)
  {
  return 0 == SWS_stricmp( pMsg->szReturnMbx, pCond->u.pszValue);
  }

static int IfReturnPathContains( const SIfCondition* pCond, const SMessage* pMsg)
  {
  return NULL != strstr( pMsg->szReturnMbx, pCond->u.pszValue);
  }


/*
 * If message received from
 */
static int IfRecvFrom( SIfCondition* pCond)
  {
  const char* psz;
  char* pszValue;

  /* Parse operator */
  psz = strtok( NULL, kszWhiteSpace);
  if ( NULL == psz)
    {
    fputs( "Missing operator\n", stderr);
    return 1;
    }
  if ( OP_EQ( psz))
    {
    pCond->pfnOp = &IfRecvFromEq;
    }
  else if ( OP_CONTAINS( psz))
    {
    pCond->pfnOp = &IfRecvFromContains;
    }
  else
    {
    fputs( "Unknown operator\n", stderr);
    return 1;
    }

  /* Parse rhs value */
  psz = ParseStringConstant( strtok( NULL, ""));
  if ( NULL == psz)
    {
    fputs( "Missing or invalid string constant\n", stderr);
    return 1;
    }

  pszValue = (char*)malloc( 1 + strlen( psz));
  if ( NULL == pszValue)
    {
    fputs( "Out of memory\n", stderr);
    return 1;
    }
  pCond->u.pszValue = strcpy( pszValue, psz);

  return 0;
  }

static int IfRecvFromEq( const SIfCondition* pCond, const SMessage* pMsg)
  {
  return 0 == SWS_stricmp( pMsg->recvInfo.szFrom, pCond->u.pszValue);
  }

static int IfRecvFromContains( const SIfCondition* pCond, const SMessage* pMsg)
  {
  return NULL != strstr( pMsg->recvInfo.szFrom, pCond->u.pszValue);
  }


/*
 * If message helo name
 */
static int IfHelo( SIfCondition* pCond)
  {
  const char* psz;
  char* pszValue;

  /* Parse operator */
  psz = strtok( NULL, kszWhiteSpace);
  if ( NULL == psz)
    {
    fputs( "Missing operator\n", stderr);
    return 1;
    }
  if ( OP_EQ( psz))
    {
    pCond->pfnOp = &IfHeloEq;
    }
  else if ( OP_CONTAINS( psz))
    {
    pCond->pfnOp = &IfHeloContains;
    }
  else
    {
    fputs( "Unknown operator\n", stderr);
    return 1;
    }

  /* Parse rhs value */
  psz = ParseStringConstant( strtok( NULL, ""));
  if ( NULL == psz)
    {
    fputs( "Missing or invalid string constant\n", stderr);
    return 1;
    }

  pszValue = (char*)malloc( 1 + strlen( psz));
  if ( NULL == pszValue)
    {
    fputs( "Out of memory\n", stderr);
    return 1;
    }
  pCond->u.pszValue = strcpy( pszValue, psz);

  return 0;
  }

static int IfHeloEq( const SIfCondition* pCond, const SMessage* pMsg)
  {
  return 0 == SWS_stricmp( pMsg->recvInfo.szHelo, pCond->u.pszValue);
  }

static int IfHeloContains( const SIfCondition* pCond, const SMessage* pMsg)
  {
  return NULL != strstr( pMsg->recvInfo.szHelo, pCond->u.pszValue);
  }


/*
 * If message source IP
 */
static int IfSrcIp( SIfCondition* pCond)
  {
  const char* psz;
  char* pszCidr;

  /* Parse operator */
  psz = strtok( NULL, kszWhiteSpace);
  if ( NULL == psz)
    {
    fputs( "Missing operator\n", stderr);
    return 1;
    }
  if ( OP_EQ( psz))
    {
    pCond->pfnOp = &IfSrcIpEq;
    }
  else
    {
    fputs( "Unknown operator\n", stderr);
    return 1;
    }

  /* Parse rhs value */
  psz = strtok( NULL, kszWhiteSpace);
  if ( NULL == psz)
    {
    fputs( "Missing IP address\n", stderr);
    return 1;
    }

  pszCidr = (char*)strchr( psz, '/');
  if ( NULL != pszCidr)
    {
    unsigned uCidr;

    /* Parse the CIDR value */
    *pszCidr++ = cNul;
    if ( 1 > sscanf( pszCidr, "%u", &uCidr)
      || uCidr > 32
    ) {
      fputs( "Invalid subnet\n", stderr);
      return 1;
      }

    if ( uCidr > 0)
      pCond->u.addr.ulMask = htonl( INADDR_BROADCAST << (32 - uCidr));
    else
      pCond->u.addr.ulMask = 0;
    }
  else
    {
    pCond->u.addr.ulMask = 0;
    }

  /* Parse the IP address */
  pCond->u.addr.ulIp = inet_addr( psz);
  if ( INADDR_NONE == pCond->u.addr.ulIp
    || INADDR_ANY == pCond->u.addr.ulIp
  ) {
    fputs( "Invalid IP\n", stderr);
    return 1;
    }

  pCond->u.addr.ulIp &= pCond->u.addr.ulMask;

  return 0;
  }

static int IfSrcIpEq( const SIfCondition* pCond, const SMessage* pMsg)
  {
  return INADDR_NONE != pMsg->recvInfo.ulSrcIp
    && (pMsg->recvInfo.ulSrcIp & pCond->u.addr.ulMask) == pCond->u.addr.ulIp;
  }


/*
 * If message relays
 */
static int IfRelays( SIfCondition* pCond)
  {
  const char* psz;

  /* Parse operator */
  psz = strtok( NULL, kszWhiteSpace);
  if ( NULL == psz)
    {
    fputs( "Missing operator\n", stderr);
    return 1;
    }
  if ( OP_GT( psz))
    {
    pCond->pfnOp = &IfRelaysGt;
    }
  else if ( OP_LT( psz))
    {
    pCond->pfnOp = &IfRelaysLt;
    }
  else if ( OP_EQ( psz))
    {
    pCond->pfnOp = &IfRelaysEq;
    }
  else
    {
    fputs( "Unknown operator\n", stderr);
    return 1;
    }

  /* Parse rhs value */
  psz = strtok( NULL, kszWhiteSpace);
  if ( NULL == psz)
    {
    fputs( "Missing numeric value\n", stderr);
    return 1;
    }
  if ( 1 != sscanf( psz, "%u", &pCond->u.uValue))
    {
    fputs( "Invalid value\n", stderr);
    return 1;
    }

  return 0;
  }

static int IfRelaysEq( const SIfCondition* pCond, const SMessage* pMsg)
  {
  return pMsg->uRelays == pCond->u.uValue;
  }

static int IfRelaysLt( const SIfCondition* pCond, const SMessage* pMsg)
  {
  return pMsg->uRelays < pCond->u.uValue;
  }

static int IfRelaysGt( const SIfCondition* pCond, const SMessage* pMsg)
  {
  return pMsg->uRelays > pCond->u.uValue;
  }


/*
 * If message received with
 */
static int IfRecvWith( SIfCondition* pCond)
  {
  const char* psz;
  char* pszValue;

  /* Parse operator */
  psz = strtok( NULL, kszWhiteSpace);
  if ( NULL == psz)
    {
    fputs( "Missing operator\n", stderr);
    return 1;
    }
  if ( OP_EQ( psz))
    {
    pCond->pfnOp = &IfRecvWithEq;
    }
  else if ( OP_CONTAINS( psz))
    {
    pCond->pfnOp = &IfRecvWithContains;
    }
  else
    {
    fputs( "Unknown operator\n", stderr);
    return 1;
    }

  /* Parse rhs value */
  psz = ParseStringConstant( strtok( NULL, ""));
  if ( NULL == psz)
    {
    fputs( "Missing or invalid string constant\n", stderr);
    return 1;
    }

  pszValue = (char*)malloc( 1 + strlen( psz));
  if ( NULL == pszValue)
    {
    fputs( "Out of memory\n", stderr);
    return 1;
    }
  pCond->u.pszValue = strcpy( pszValue, psz);

  return 0;
  }

static int IfRecvWithEq( const SIfCondition* pCond, const SMessage* pMsg)
  {
  return 0 == SWS_stricmp( pMsg->recvInfo.szWith, pCond->u.pszValue);
  }

static int IfRecvWithContains( const SIfCondition* pCond, const SMessage* pMsg)
  {
  return NULL != strstr( pMsg->recvInfo.szWith, pCond->u.pszValue);
  }


/*
 * If message received by
 */
static int IfRecvBy( SIfCondition* pCond)
  {
  const char* psz;
  char* pszValue;

  /* Parse operator */
  psz = strtok( NULL, kszWhiteSpace);
  if ( NULL == psz)
    {
    fputs( "Missing operator\n", stderr);
    return 1;
    }
  if ( OP_EQ( psz))
    {
    pCond->pfnOp = &IfRecvByEq;
    }
  else if ( OP_CONTAINS( psz))
    {
    pCond->pfnOp = &IfRecvByContains;
    }
  else
    {
    fputs( "Unknown operator\n", stderr);
    return 1;
    }

  /* Parse rhs value */
  psz = ParseStringConstant( strtok( NULL, ""));
  if ( NULL == psz)
    {
    fputs( "Missing or invalid string constant\n", stderr);
    return 1;
    }

  pszValue = (char*)malloc( 1 + strlen( psz));
  if ( NULL == pszValue)
    {
    fputs( "Out of memory\n", stderr);
    return 1;
    }
  pCond->u.pszValue = strcpy( pszValue, psz);

  return 0;
  }

static int IfRecvByEq( const SIfCondition* pCond, const SMessage* pMsg)
  {
  return 0 == SWS_stricmp( pMsg->recvInfo.szBy, pCond->u.pszValue);
  }

static int IfRecvByContains( const SIfCondition* pCond, const SMessage* pMsg)
  {
  return NULL != strstr( pMsg->recvInfo.szBy, pCond->u.pszValue);
  }


/*
 * If message trace forged
 */
static int IfForged( SIfCondition* pCond)
  {
  pCond->pfnOp = &IfRecvForged;
  return 0;
  }

static int IfRecvForged( const SIfCondition* pCond, const SMessage* pMsg)
  {
  (void)pCond;
  return pMsg->isRecvForged;
  }


/*
 * If message comment
 */
static int IfComment( SIfCondition* pCond)
  {
  const char* psz;
  char* pszValue;

  /* Parse operator */
  psz = strtok( NULL, kszWhiteSpace);
  if ( NULL == psz)
    {
    fputs( "Missing operator\n", stderr);
    return 1;
    }
  if ( OP_CONTAINS( psz))
    {
    pCond->pfnOp = &IfCommentContains;
    }
  else
    {
    fputs( "Unknown operator\n", stderr);
    return 1;
    }

  /* Parse rhs value */
  psz = ParseStringConstant( strtok( NULL, ""));
  if ( NULL == psz)
    {
    fputs( "Missing or invalid string constant\n", stderr);
    return 1;
    }

  pszValue = (char*)malloc( 1 + strlen( psz));
  if ( NULL == pszValue)
    {
    fputs( "Out of memory\n", stderr);
    return 1;
    }
  pCond->u.pszValue = strcpy( pszValue, psz);

  return 0;
  }

static int IfCommentContains( const SIfCondition* pCond, const SMessage* pMsg)
  {
  return NULL != strstr( pMsg->recvInfo.szExtra, pCond->u.pszValue);
  }


/*
 * Parse string constant
 */
static const char* ParseStringConstant( char* psz)
  {
  if ( NULL == psz)
    return NULL;

  /* Skip leading whitespace */
  while ( isspace( *psz))
    ++psz;

  /* Ensure string starts and ends with QUOTE */
  if ( QUOTE[0] != *psz
    || NULL == strchr( psz + 1, QUOTE[0])
  ) {
    return NULL;
    }

  /* Parse the string */
  if ( QUOTE[0] == psz[0] && QUOTE[0] == psz[1])
    {
    (void)strtok( psz + 1, kszWhiteSpace);
    return "";
    }

  return strtok( psz, QUOTE);
  }


/****************************************************************************
 * Actions
 ***************************************************************************/

/*
 * Apply an action to all messages
 */
static int Apply( SOCKET s, FAction* pfn, void* pv)
  {
  unsigned uMsg;
  SMessage* pMsg;

  for ( uMsg = 1; !g_exit && NULL != (pMsg = GetMsg( s, uMsg)); ++uMsg)
    (*pfn)( s, pMsg, pv);

  return 0;
  }


static int CopyAction( SOCKET s, SMessage* pMsg, void* pv)
  {
  (void)pv;

  if ( pMsg->isDeleted)
    return 1;

  sprintf( s_szBuf, "%u", pMsg->uId);
  return RetrMethod( s, s_szBuf);
  }


static int DelAction( SOCKET s, SMessage* pMsg, void* pv)
  {
  int iRet;
  (void)pv;

  if ( pMsg->isDeleted)
    return 1;

  sprintf( s_szBuf, "DELE %u", pMsg->uId);
  iRet = DoCommand( s, s_szBuf, s_szBuf, sizeof s_szBuf, s_iVerbosity > 0 );
  pMsg->isDeleted = 1;

  return iRet;
  }


static int MoveAction( SOCKET s, SMessage* pMsg, void* pv)
  {
  int iRet = CopyAction( s, pMsg, pv);
  if ( 0 == iRet)
    iRet = DelAction( s, pMsg, pv);
  return iRet;
  }


static const SAction* FindAction( const char* psz)
  {
  int i;
  const SAction* pAction;

  for ( pAction = actions, i = lengthof(actions); i; --i, ++pAction)
    {
    if ( 0 == SWS_stricmp( psz, pAction->pszName) )
      return pAction;
    }
  return NULL;
  }


/****************************************************************************
 * Message header handling
 ***************************************************************************/

/*
 * Get message header info
 * Reset the cache if msg=0
 */
static SMessage* GetMsg( SOCKET s, unsigned uMessage)
  {
  /* Message info cache */
  static unsigned s_uMsgs;
  static SMessage* s_pMsgs;
  SMessage* pMsg;

  if ( 0 == uMessage)
    {
    /* Dispose of cached info */
    s_uMsgs = 0;
    if ( NULL != s_pMsgs)
      {
      free( s_pMsgs);
      s_pMsgs = NULL;
      }
    return NULL;
    }

  if ( NULL == s_pMsgs && INVALID_SOCKET != s)
    {
    unsigned uMsg;

    /* Get max message ID */
    if ( 0 != DoCommand( s, "STAT", s_szBuf, sizeof s_szBuf, 0)
      || 1 != sscanf( s_szBuf, "+OK %u", &uMsg)
    ) {
      return NULL;
      }

    /* Allocate message array */
    pMsg = (SMessage*) calloc( uMsg, sizeof( SMessage));
    if ( NULL == pMsg)
      return NULL;

    s_pMsgs = pMsg;
    s_uMsgs = uMsg;

    /* Init messages */
    for ( uMsg = 1; uMsg <= s_uMsgs; ++uMsg, ++pMsg)
      {
      pMsg->uId = uMsg;
      pMsg->tDate = INVALID_TIME;
      MsgRecvInit( &pMsg->recvInfo);
      }
    }

  if ( uMessage > s_uMsgs)
    return NULL;

  pMsg = &s_pMsgs[ uMessage - 1];
  if ( INVALID_SOCKET != s && !pMsg->isPresent)
    pMsg->isPresent = 0 == GetMsgInfo( s, pMsg);

  return pMsg;
  }


/*
 * Display message info
 */
static void DisplayMsg( const SMessage* pMsg, FILE* file, int iVerbosity)
  {
  char cSize, cAttachment, cDeleted;
  unsigned uSize;

  if ( pMsg->ulSize < 1000)
    {
    uSize = (unsigned)pMsg->ulSize;
    cSize = ' ';
    }
  else if ( pMsg->ulSize < 1000 * 1024UL)
    {
    uSize = (unsigned)((pMsg->ulSize + 512) / 1024),
    cSize = 'k';
    }
  else
    {
    uSize = (unsigned)((pMsg->ulSize + 512 * 1024UL) / (1024 * 1024UL)),
    cSize = 'M';
    }

  cAttachment = (char)(NULL != strstr( pMsg->szContentType, "multipart/mixed") ? '+' : ' ');
  cDeleted = (char)(pMsg->isDeleted ? '#' : ' ');

  fprintf( file, "%c%2u %3u%c%c %-12.12s  %-12.12s",
    cDeleted,
    pMsg->uId,
    uSize, cSize,
    cAttachment,
    pMsg->szSubj,
    pMsg->szFromInfo[0] ? pMsg->szFromInfo : pMsg->szFromMbx
  );

  if ( pMsg->isRecvForged)
    fputs( " ***", file);

  if ( iVerbosity > 1 && pMsg->uRelays > 0)
    fprintf( file, " %u hops", pMsg->uRelays);

  if ( iVerbosity > 0 && pMsg->recvInfo.szHelo[0])
    fprintf( file, " helo %s", pMsg->recvInfo.szHelo);

  if ( (iVerbosity > 1 || !pMsg->recvInfo.szHelo[0])
    && pMsg->recvInfo.szFrom[0]
    && '[' != pMsg->recvInfo.szFrom[0]
    && SWS_stricmp( "unknown", pMsg->recvInfo.szFrom)
  ) {
    fprintf( file, " %s", pMsg->recvInfo.szFrom);
    }

  if ( iVerbosity > 0 && INADDR_NONE != pMsg->recvInfo.ulSrcIp)
    {
    struct in_addr addr;
    addr.s_addr = pMsg->recvInfo.ulSrcIp;
    fprintf( file, " [%s]", inet_ntoa( addr) );
    }

  if ( iVerbosity > 1)
    {
    if ( pMsg->recvInfo.szExtra[0])
      fprintf( file, " %s", pMsg->recvInfo.szExtra);

    if ( pMsg->recvInfo.szWith[0])
      fprintf( file, " with %s", pMsg->recvInfo.szWith);

    if ( pMsg->recvInfo.szBy[0])
      fprintf( file, " by %s", pMsg->recvInfo.szBy);

    if ( pMsg->recvInfo.tRecv > 0)
      {
      char szTime[ 32];
      strftime( szTime, sizeof szTime, "%Y/%m/%d %H:%M:%S",
        localtime( &pMsg->recvInfo.tRecv)
      );
      fprintf( file, " ;%s", szTime);
      }
    }

  fputs( "\n", file);
  }


/*
 * Display message info
 */
static void SummMsg( const SMessage* pMsg, unsigned uId, FILE* file)
  {
  /* Intrinsic */
  fprintf( file, "Id: %u\n", uId);
  fprintf( file, "Size: %lu\n", pMsg->ulSize);

  /* Required */
  if ( pMsg->szFromMbx[0])
    fprintf( file, "From: %s\n", pMsg->szFromMbx);
  if ( pMsg->szFromInfo[0])
    fprintf( file, "From-Info: %s\n", pMsg->szFromInfo);
  if ( INVALID_TIME != pMsg->tDate)
    fprintf( file, "Date: %lu\n", (unsigned long)pMsg->tDate);

  /* Optional */
  if ( pMsg->szTo[0])
    fprintf( file, "To: %s\n", pMsg->szTo);
  if ( pMsg->szCc[0])
    fprintf( file, "Cc: %s\n", pMsg->szCc);
  if ( pMsg->szSubj[0])
    fprintf( file, "Subject: %s\n", pMsg->szSubj);
  if ( pMsg->szContentType[0])
    fprintf( file, "Content-Type: %s\n", pMsg->szContentType);
  if ( pMsg->szCharset[0])
    fprintf( file, "Charset: %s\n", pMsg->szCharset);

  /* Trace should be present */
  if ( pMsg->szReturnMbx[0])
    fprintf( file, "Return-Path: %s\n", pMsg->szReturnMbx);
  if ( INVALID_TIME != pMsg->recvInfo.tRecv)
    fprintf( file, "Received-Date: %lu\n", (unsigned long)pMsg->recvInfo.tRecv);

  fputs( "\n", file);
  }


/*
 * Display info about a part(s) of a message
 */
static int SummPart(
  SOCKET s,             /* IN: POP3 connection */
  unsigned uId,         /* IN: Message ID */
  unsigned uSection,    /* IN: section no. 0= all */
  FILE* file
) {
  enum { kKeyNone, kKeyContentType, kKeyOther } eLastKey = kKeyNone;
#define MAX_NESTING 3 /* Max part nesting level */
#define MAX_BOUND 100 /* boundary string length */
  struct
    {
    size_t size;
    char szText[ MAX_BOUND];
    } aBound[ MAX_NESTING], *pBound = &aBound[ MAX_NESTING];
  int iRet, isBody;
  unsigned uPart;

  sprintf( s_szBuf, "RETR %u", uId);
  iRet = DoCommand( s, s_szBuf, s_szBuf, sizeof s_szBuf, 0 );
  if ( 0 != iRet)
    return iRet;

  uPart = 0;
  isBody = 0;

  /* Get multi line response */
  while ( SOCKET_ERROR != ReadLine( s, s_szBuf, sizeof s_szBuf -1, GEN_TIMEOUT) )
    {
    char* psz;

    /* Check if last line */
    if ( 0 == strcmp( s_szBuf, ".\n"))
      {
      fputs( "\n", file);
      return 0;
      }

    if ( pBound < &aBound[ MAX_NESTING]
      && 0 == strncmp( "--", s_szBuf, 2)
      && 0 == strncmp( pBound->szText, &s_szBuf[2], pBound->size)
    ) {
      /* Next part */
      isBody = 0;
      eLastKey = kKeyNone;

      if ( strcmp( "--\n", &s_szBuf[ pBound->size +2]))
        {
        ++uPart;
        if ( 0 == uSection || uSection == uPart)
          fprintf( file, "Part: %u\n", uPart);
        }
      else
        ++pBound; /* Last part */
      }
    else if ( isBody)
      {
      }
    /* Check for end of header */
    else if ( 0 == strcmp( "\n", s_szBuf))
      {
      if ( uPart > 0 && uSection == uPart)
        {
        fputs( "\n", file);
        return 0;
        }

      isBody = 1;
      eLastKey = kKeyNone;
      }
    /* Check for header continuation line */
    else if ( isspace( s_szBuf[0]) )
      {
      /* Fold all leading whitespace into a single space */
      psz = s_szBuf;
      do
        *psz++ = ' ';
      while ( isspace( *psz));
      --psz;
      psz = strtok( psz, "\n");

      switch ( eLastKey)
        {
      /*default:*/
      case kKeyNone:
      case kKeyOther:
        break;

      case kKeyContentType:
        psz = strtok( psz, " =");
        if ( 0 == SWS_stricmp( psz, "boundary"))
          {
          psz = strtok( NULL, " \"");
          if ( NULL != psz)
            {
            size_t size = strlen( psz);
            if ( size <= sizeof pBound->szText
              && pBound > &aBound[0]
            ) {
              --pBound;
              pBound->size = size;
              strncpy( pBound->szText, psz, size);
              }
            }
          }
        else if ( 0 == SWS_stricmp( psz, "charset"))
          {
          psz = strtok( NULL, " \";");
          if ( NULL != psz && uPart > 0 && ( 0 == uSection || uSection == uPart))
            fprintf( file, "Charset: %s\n", psz);
          }
        else if ( 0 == SWS_stricmp( psz, "name"))
          {
          psz = strtok( NULL, " \";");
          if ( NULL != psz && uPart > 0 && ( 0 == uSection || uSection == uPart))
            fprintf( file, "Content-Name: %s\n", psz);
          }
        break;
        }
      }
    else
      {
      /* Parse MIME header */
      char* pszKey = strtok( s_szBuf, " \t");
      char* pszVal = strtok( NULL, "\n");
      if ( NULL != pszVal)
        {
        /* Remove leading whitespace */
        while ( isspace( *pszVal))
          ++pszVal;
        }
      else
        {
        static char sz[] = "";
        pszVal = sz;
        };

      if ( 0 == SWS_stricmp( pszKey, "Content-Type:") )
        {
        eLastKey = kKeyContentType;

        strtok( pszVal, ";");
        if ( uPart > 0 && ( 0 == uSection || uSection == uPart))
          fprintf( file, "Content-Type: %s\n", pszVal);

        while ( NULL != (pszVal = strtok( NULL, " =")))
          {
          if ( 0 == SWS_stricmp( "boundary", pszVal))
            {
            pszVal = strtok( NULL, " \"");
            if ( NULL != pszVal)
              {
              size_t size = strlen( pszVal);
              if ( size <= sizeof pBound->szText
                && pBound > &aBound[0]
              ) {
                --pBound;
                pBound->size = size;
                strncpy( pBound->szText, pszVal, size);
                }
              }
            }
          else if ( 0 == SWS_stricmp( "charset", pszVal))
            {
            pszVal = strtok( NULL, " \";");
            if ( NULL != pszVal && uPart > 0 && ( 0 == uSection || uSection == uPart))
              fprintf( file, "Charset: %s\n", pszVal);
            }
          else if ( 0 == SWS_stricmp( "name", pszVal))
            {
            pszVal = strtok( NULL, " \";");
            if ( NULL != pszVal && uPart > 0 && ( 0 == uSection || uSection == uPart))
              fprintf( file, "Content-Name: %s\n", pszVal);
            }
          else
            strtok( NULL, ";");
          }
        }
      else if ( 0 == SWS_stricmp( pszKey, "Content-Transfer-Encoding:") )
        {
        eLastKey = kKeyOther;
        pszVal = strtok( pszVal, ";");
        if ( uPart > 0 && ( 0 == uSection || uSection == uPart))
          fprintf( file, "Encoding: %s\n", pszVal);
        }
      else if ( 0 == SWS_stricmp( pszKey, "Content-Disposition:") )
        {
        eLastKey = kKeyOther;
        pszVal = strtok( pszVal, ";");
        if ( uPart > 0 && ( 0 == uSection || uSection == uPart))
          fprintf( file, "Disposition: %s\n", pszVal);
        }
      else
        {
        eLastKey = kKeyOther;
        }
      }
    }

  fputs( "\n", file);
  return 1;
  }


/*
 * Fetch & store a part of a MIME message
 */
static int GetPart(
  SOCKET s,             /* IN: POP3 connection */
  unsigned uId,         /* IN: Message ID */
  const char* pszType,  /* IN: MIME content type or name */
  FILE* file            /* OUT: message */
) {
  enum { kKeyNone, kKeyContentType, kKeyContentTE, kKeyOther } eLastKey = kKeyNone;
  enum { k7bit, kQuoted, kBase64, kOther } eEncoding = k7bit;
  char szContentType[ 32] = "text/plain";
  char szContentName[ 128] = "";
  struct
    {
    size_t size;
    char szText[ MAX_BOUND];
    } aBound[ MAX_NESTING], *pBound = &aBound[ MAX_NESTING];
  int isFound = 0;
  int iRet, isBody, isMatch;

  sprintf( s_szBuf, "RETR %u", uId);
  iRet = DoCommand( s, s_szBuf, s_szBuf, sizeof s_szBuf, 0 );
  if ( 0 != iRet)
    return iRet;

  /* Get multi line response */
  isMatch = isBody = 0;
  while ( SOCKET_ERROR != ReadLine( s, s_szBuf, sizeof s_szBuf -1, GEN_TIMEOUT) )
    {
    char* psz;

    /* Check if last line */
    if ( 0 == strcmp( s_szBuf, ".\n"))
      return isFound ? 0 : 1;

    if ( pBound < &aBound[ MAX_NESTING]
      && 0 == strncmp( "--", s_szBuf, 2)
      && 0 == strncmp( pBound->szText, &s_szBuf[2], pBound->size)
    ) {
      /* Next part */
      isBody = 0;
      eLastKey = kKeyNone;
      eEncoding = k7bit;
      szContentType[0] = cNul;
      szContentName[0] = cNul;
      if ( isFound)
        return 0;

      if ( 0 == strcmp( "--\n", &s_szBuf[ pBound->size +2]))
        ++pBound; /* Last part */
      }
    else if ( isBody)
      {
      if ( isMatch)
        {
        isFound = 1;

        switch ( eEncoding)
          {
        case k7bit:
        case kOther:
          fputs( s_szBuf, file);
          break;
        case kQuoted:
          MimeDecodeQuoted( (unsigned char*)s_szBuf, sizeof s_szBuf, s_szBuf, sizeof s_szBuf);
          fputs( s_szBuf, file);
          break;
        case kBase64:
          fwrite( s_szBuf,
            SWS_Base64Decode( (unsigned char*)s_szBuf, sizeof s_szBuf, s_szBuf), 1,
            file
          );
          break;
          }
        }
      }
    /* Check for end of header */
    else if ( 0 == strcmp( "\n", s_szBuf))
      {
      isBody = 1;
      eLastKey = kKeyNone;

      if ( 0 == SWS_stricmp( pszType, szContentType) || 0 == SWS_stricmp( pszType, szContentName))
        isMatch = 1;
      }
    /* Check for header continuation line */
    else if ( isspace( s_szBuf[0]) )
      {
      /* Fold all leading whitespace into a single space */
      psz = s_szBuf;
      do
        *psz++ = ' ';
      while ( isspace( *psz));
      --psz;
      psz = strtok( psz, "\n");

      switch ( eLastKey)
        {
      /*default:*/
      case kKeyNone:
      case kKeyContentTE:
      case kKeyOther:
        break;

      case kKeyContentType:
        psz = strtok( psz, " =");
        if ( 0 == SWS_stricmp( psz, "boundary"))
          {
          psz = strtok( NULL, " \"");
          if ( NULL != psz)
            {
            size_t size = strlen( psz);
            if ( size <= sizeof pBound->szText
              && pBound > &aBound[0]
            ) {
              --pBound;
              pBound->size = size;
              strncpy( pBound->szText, psz, size);
              }
            }
          }
        else if ( 0 == SWS_stricmp( psz, "name"))
          {
          psz = strtok( NULL, " \";");
          if ( NULL != psz)
            strncpy( szContentName, psz, sizeof szContentName - 1);
          }
        break;
        }
      }
    else
      {
      /* Parse MIME header */
      char* pszKey = strtok( s_szBuf, " \t");
      char* pszVal = strtok( NULL, "\n");
      if ( NULL != pszVal)
        {
        /* Remove leading whitespace */
        while ( isspace( *pszVal))
          ++pszVal;
        }
      else
        {
        static char sz[] = "";
        pszVal = sz;
        };

      if ( 0 == SWS_stricmp( pszKey, "Content-Type:") )
        {
        eLastKey = kKeyContentType;

        strtok( pszVal, ";");
        strncpy( szContentType, pszVal, sizeof szContentType - 1);

        while ( NULL != (pszVal = strtok( NULL, " =")))
          {
          if ( 0 == SWS_stricmp( "boundary", pszVal))
            {
            pszVal = strtok( NULL, " \"");
            if ( NULL != pszVal)
              {
              size_t size = strlen( pszVal);
              if ( size <= sizeof pBound->szText
                && pBound > &aBound[0]
              ) {
                --pBound;
                pBound->size = size;
                strncpy( pBound->szText, pszVal, size);
                }
              }
            }
          else if ( 0 == SWS_stricmp( "name", pszVal))
            {
            pszVal = strtok( NULL, " \";");
            if ( NULL != pszVal)
              strncpy( szContentName, pszVal, sizeof szContentName - 1);
            }
          else
            strtok( NULL, ";");
          }
        }
      else if ( 0 == SWS_stricmp( pszKey, "Content-Transfer-Encoding:") )
        {
        eLastKey = kKeyContentTE;
        pszVal = strtok( pszVal, ";");
        if ( 0 == SWS_stricmp( pszVal, "7bit"))
          eEncoding = k7bit;
        else if ( 0 == SWS_stricmp( pszVal, "quoted-printable"))
          eEncoding = kQuoted;
        else if ( 0 == SWS_stricmp( pszVal, "base64"))
          eEncoding = kBase64;
        else
          eEncoding = kOther;
        }
      else
        {
        eLastKey = kKeyOther;
        }
      }
    }

  return 1;
  }


/*
 * Read message info from server
 */
static int GetMsgInfo( SOCKET s, SMessage* pMsg)
  {
  int iRet, bBody;
  enum {
    kKeyNone, kKeySubj, kKeyFrom, kKeyTo, kKeyCc, kKeyDate,
    kKeyRecv, kKeyRecvDate, kKeyReturn, kKeyContentType, kKeyMessageId,
    kKeyOther
  } eLastKey = kKeyNone;
  SRecvTraceInfo info;

  /* Get message size */
  sprintf( s_szBuf, "LIST %u", pMsg->uId);
  iRet = DoCommand( s, s_szBuf, s_szBuf, sizeof s_szBuf, 0);
  if ( iRet)
    return iRet;

  sscanf( s_szBuf, "+OK %*u %lu", &pMsg->ulSize);

  /* Get message header */
  sprintf( s_szBuf, "TOP %u %u", pMsg->uId, s_uLines);
  iRet = DoCommand( s, s_szBuf, s_szBuf, sizeof s_szBuf, 0);
  if ( 0 != iRet)
    return iRet;

  MsgRecvInit( &info);
  bBody = 0;

  /* Get multi line response */
  while ( SOCKET_ERROR != ReadLine( s, s_szBuf, sizeof s_szBuf -1, GEN_TIMEOUT) )
    {
    char* pszKey;
    char* psz;

    /* Check if last line */
    if ( 0 == strcmp( s_szBuf, ".\n"))
      return 0;

    if ( s_uLines > 0)
      {
      if ( NULL != s_logfile)
        fputs( s_szBuf, s_logfile);
      if ( stdout != s_logfile)
        fputs( s_szBuf, stdout);
      }

    if ( bBody)
      continue;

    /* Check for end of header */
    if ( '\n' == s_szBuf[0])
      {
      bBody = 1;
      if ( MimeIsStringEncoded( pMsg->szSubj))
        MimeDecodeString( pMsg->szSubj, (unsigned char*)pMsg->szSubj, sizeof pMsg->szSubj);
      continue;
      }

    /* Check for continuation line */
    if ( isspace( s_szBuf[0]) )
      {
      /* Fold all leading whitespace into a single space */
      psz = s_szBuf;
      do
        *psz++ = ' ';
      while ( isspace( *psz));
      --psz;

      switch ( eLastKey)
        {
      /*default:*/
      case kKeyNone:
      case kKeyFrom:
      case kKeyDate:
      case kKeyReturn:
      case kKeyMessageId:
      case kKeyOther:
        break;

      case kKeyTo:
        if ( strlen( psz) + strlen( pMsg->szTo) < sizeof pMsg->szTo)
          strcat( pMsg->szTo, strtok( psz, "\n"));
        break;

      case kKeyCc:
        if ( strlen( psz) + strlen( pMsg->szCc) < sizeof pMsg->szCc)
          strcat( pMsg->szCc, strtok( psz, "\n"));
        break;

      case kKeySubj:
        if ( strlen( psz) + strlen( pMsg->szSubj) < sizeof pMsg->szSubj)
          strcat( pMsg->szSubj, strtok( psz, "\n"));
        break;

      case kKeyContentType:
        psz = strtok( psz, " =");
        if ( 0 == SWS_stricmp( psz, "charset"))
          {
          psz = strtok( NULL, " \";");
          if ( NULL != psz)
            strncpy( pMsg->szCharset, psz, sizeof pMsg->szCharset - 1);
          }
        break;

      case kKeyRecv:
        if ( MsgInfoRecv( &info, psz))
          {
          eLastKey = kKeyRecvDate;
          MsgRecvUpdate( pMsg, &info);
          }
        break;

      case kKeyRecvDate:
        if ( INVALID_TIME == info.tRecv)
          {
          if ( NULL != strchr( psz, ';'))
            MsgInfoRecv( &info, psz);
          else
            info.tRecv = SWS_TimeFromStr( psz);
          MsgRecvUpdate( pMsg, &info);
          }
        break;
        }

      continue;
      }

    /* Parse keyword */
    pszKey = strtok( s_szBuf, " \t");
    if ( NULL != pszKey)
      {
      char* pszVal;

      /* Get remainer of line without terminating \n */
      pszVal = strtok( NULL, "\n");
      if ( NULL != pszVal)
        {
        /* Remove leading whitespace */
        while ( isspace( *pszVal))
          ++pszVal;
        }
      else
        {
        static char sz[] = "";
        pszVal = sz;
        };

      /* Required keys */
      if ( 0 == SWS_stricmp( pszKey, "From:") )
        {
        eLastKey = kKeyFrom;

        strtok( pszVal, "<");
        psz = strtok( NULL, ">");
        if ( NULL != psz)
          {
          strncpy( pMsg->szFromMbx, psz, sizeof pMsg->szFromMbx - 1);
          psz = strtok( pszVal, "\"");
          strtok( NULL, "\"<");
          strncpy( pMsg->szFromInfo, psz, sizeof pMsg->szFromInfo - 1);
          }
        else
          {
          strtok( pszVal, " ");
          strncpy( pMsg->szFromMbx, pszVal, sizeof pMsg->szFromMbx - 1);
          psz = strtok( NULL, "()");
          if ( NULL != psz)
            strncpy( pMsg->szFromInfo, psz, sizeof pMsg->szFromInfo - 1);
          }
        if ( MimeIsStringEncoded( pMsg->szFromInfo))
          MimeDecodeString( pMsg->szFromInfo, (unsigned char*)pMsg->szFromInfo, sizeof pMsg->szFromInfo);
        }
      else if ( 0 == SWS_stricmp( pszKey, "Date:") )
        {
        eLastKey = kKeyDate;

        pMsg->tDate = SWS_TimeFromStr( pszVal);
        }
      /* Info */
      else if ( 0 == SWS_stricmp( pszKey, "To:") )
        {
        eLastKey = kKeyTo;

        strncpy( pMsg->szTo, pszVal, sizeof pMsg->szTo - 1);
        }
      else if ( 0 == SWS_stricmp( pszKey, "Cc:") )
        {
        eLastKey = kKeyCc;

        strncpy( pMsg->szCc, pszVal, sizeof pMsg->szCc - 1);
        }
      else if ( 0 == SWS_stricmp( pszKey, "Subject:") )
        {
        eLastKey = kKeySubj;

        strncpy( pMsg->szSubj, pszVal, sizeof pMsg->szSubj - 1);
        }
      else if ( 0 == SWS_stricmp( pszKey, "Content-Type:") )
        {
        eLastKey = kKeyContentType;

        strtok( pszVal, ";");
        strncpy( pMsg->szContentType, SWS_strlwr( pszVal), sizeof pMsg->szContentType - 1);

        while ( NULL != (pszVal = strtok( NULL, " =")))
          {
          if ( 0 == SWS_stricmp( "charset", pszVal))
            {
            pszVal = strtok( NULL, " \";");
            if ( NULL != pszVal)
              strncpy( pMsg->szCharset, pszVal, sizeof pMsg->szCharset - 1);
            }
          else
            strtok( NULL, ";");
          }
        }
      else if ( 0 == SWS_stricmp( pszKey, "Message-Id:") )
        {
        eLastKey = kKeyMessageId;

        strncpy( pMsg->szMessageId, pszVal, sizeof pMsg->szMessageId - 1);
        }
      /* Trace */
      else if ( 0 == SWS_stricmp( pszKey, "Return-Path:") )
        {
        eLastKey = kKeyReturn;

        strtok( pszVal, "<");
        psz = strtok( NULL, " >");
        strncpy( pMsg->szReturnMbx, psz ? psz : pszVal, sizeof pMsg->szReturnMbx - 1);
        }
      else if ( 0 == SWS_stricmp( pszKey, "Received:") )
        {
        eLastKey = kKeyRecv;

        MsgRecvInit( &info);
        if ( MsgInfoRecv( &info, pszVal))
          {
          eLastKey = kKeyRecvDate;
          MsgRecvUpdate( pMsg, &info);
          }
        }
      else
        {
        eLastKey = kKeyOther;
        }
      }
    }

  return 1;
  }


/*
 * Init a 'Received:' trace entry
 */
static void MsgRecvInit( SRecvTraceInfo* pRecvd)
  {
  pRecvd->tRecv = INVALID_TIME;
  pRecvd->ulSrcIp = INADDR_NONE;
  pRecvd->szFrom[ 0] = cNul;
  pRecvd->szHelo[ 0] = cNul;
  pRecvd->szExtra[ 0] = cNul;
  pRecvd->szBy[ 0] = cNul;
  pRecvd->szWith[ 0] = cNul;
  }


/*
 * Parse a 'Received:' trace entry
 */
static int MsgInfoRecv(
  SRecvTraceInfo* pRecvd,
  char* pszSubkey
) {
  char* pszVal, *psz;
  int iRet = 0, iFrom = 0;
  unsigned long ulVal;

  if ( NULL == pszSubkey)
    return 0;

  /* Parse leading comments */
  while ( MsgInfoComment( NULL, pszSubkey))
    {
    if ( NULL == (pszSubkey = strtok( NULL, "")) )
      return 0;
    }

  /* Check for trailing datestamp */
  psz = strrchr( pszSubkey, ';');
  if ( NULL != psz)
    {
    /* Terminate preceding text */
    *psz++ = cNul;

    /* Parse the MIME format date stamp */
    pRecvd->tRecv = SWS_TimeFromStr( psz);
    iRet = 1; /* Date stamp seen */
    }

  /* Get the subkey */
  pszSubkey = strtok( pszSubkey, kszWhiteSpace);
  if ( NULL == pszSubkey)
    return iRet;

  /* Parse the subkey */
  if ( 0 == SWS_stricmp( "from", pszSubkey) )
    {
    iFrom = 1;

    /* Parse leading comments */
    while ( NULL != (pszVal = strtok( NULL, "")) )
      {
      if ( !MsgInfoComment( pRecvd, pszVal))
        break;
      }

    /* Get the 'from' text */
    if ( NULL != (pszVal = strtok( pszVal, kszWhiteSpace)))
      strncpy( pRecvd->szFrom, pszVal, sizeof pRecvd->szFrom - 1);

    /* Extract IP address if given */
    if ( INADDR_NONE != (ulVal = inet_addr( pszVal))
      || ( '[' == pszVal[0]
        && NULL != (psz = strchr( &pszVal[1], ']'))
        && (*psz = cNul, INADDR_NONE != (ulVal = inet_addr( &pszVal[1])))
        && INADDR_ANY != ulVal
      )
    ) {
      pRecvd->ulSrcIp = ulVal;
      }
    }
  else if ( 0 == SWS_stricmp( "by", pszSubkey) )
    {
    /* Parse leading comments */
    while ( NULL != (pszVal = strtok( NULL, "")) )
      {
      if ( !MsgInfoComment( NULL, pszVal))
        break;
      }

    /* Get the 'by' text */
    if ( NULL != (pszVal = strtok( pszVal, kszWhiteSpace)))
      strncpy( pRecvd->szBy, pszVal, sizeof pRecvd->szBy - 1);
    }
  else if ( 0 == SWS_stricmp( "with", pszSubkey) )
    {
    /* Parse leading comments */
    while ( NULL != (pszVal = strtok( NULL, "")) )
      {
      if ( !MsgInfoComment( NULL, pszVal))
        break;
      }

    /* Get the 'with' text */
    if ( NULL != (pszVal = strtok( pszVal, kszWhiteSpace)))
      strncpy( pRecvd->szWith, pszVal, sizeof pRecvd->szWith - 1);
    }
  else
    {
    /* Parse leading comments */
    while ( NULL != (pszVal = strtok( NULL, "")) )
      {
      if ( !MsgInfoComment( NULL, pszVal))
        break;
      }

    /* Skip the value */
    (void)strtok( pszVal, kszWhiteSpace);
    }

  /* Parse trailing comments */
  while ( NULL != (pszVal = strtok( NULL, "")) )
    {
    if ( !MsgInfoComment( iFrom ? pRecvd : NULL, pszVal))
      break;
    }

  /* Recurse on the next subkey */
  MsgInfoRecv( pRecvd, pszVal);
  return iRet;
  }


/*
 * Parse a trace comment
 */
static int MsgInfoComment( SRecvTraceInfo* pRecvd, char* pszVal)
  {
  /* Remove any leading whitespace */
  while ( isspace( *pszVal) )
    ++pszVal;

  /* Check for start of comment */
  if ( '(' != *pszVal)
    return 0; /* Not a comment */

  /* Split text at end of comment */
  if ( NULL != (pszVal = strtok( pszVal + 1, ")"))
    && NULL != pRecvd
  ) {
    /* Process 'received from' comments */
    size_t len;
    unsigned long ulVal;
    char* psz;

    /* Remove any leading whitespace */
    while ( isspace( *pszVal) )
      ++pszVal;

    /* Is it a HELO atom */
    if ( 'h' == tolower( pszVal[0])
      && 'e' == tolower( pszVal[1])
      && 'l' == tolower( pszVal[2])
      && 'o' == tolower( pszVal[3])
    ) {
      psz = &pszVal[ 4];
      while ( isspace( *psz) || '=' == *psz || ':' == *psz)
        ++psz;

      strncpy( pRecvd->szHelo, psz, sizeof pRecvd->szHelo - 1);
      }
    /* Or a naked IP address */
    else if ( INADDR_NONE != (ulVal = inet_addr( pszVal))
      && INADDR_ANY != ulVal
    ) {
      pRecvd->ulSrcIp = ulVal;
      }
    /* Or an IP address surrounded with [] */
    else if ( NULL != (psz = strchr( pszVal, '['))
      && NULL != (psz = strchr( psz, ']'))
      && (*psz = cNul, INADDR_NONE != (ulVal = inet_addr( strchr( pszVal, '[') + 1)))
      && INADDR_ANY != ulVal
    ) {
      pRecvd->ulSrcIp = ulVal;
      }
    /* Else append to extras */
    else if ( (len = strlen( pRecvd->szExtra)) < sizeof pRecvd->szExtra - 4)
      {
      if ( len)
        pRecvd->szExtra[ len++] = ' ';

      sprintf( &pRecvd->szExtra[ len],
        "(%.*s)",
        (int)(sizeof pRecvd->szExtra - 3 - len),
        pszVal
      );
      }
    }

  return 1; /* 1= comment processed */
  }


/*
 * Update the 'Received:' trace entry
 */
static void MsgRecvUpdate(
  SMessage* pMsg,
  const SRecvTraceInfo* pSrc
) {
  if ( pMsg->isRecvForged)
    return;

  if ( INVALID_TIME == pSrc->tRecv)
    return;

  if ( INVALID_TIME == pMsg->recvInfo.tRecv)
    {
    pMsg->recvInfo.tRecv = pSrc->tRecv;
    }
  else
    {
    double dSecs = difftime( pMsg->recvInfo.tRecv, pSrc->tRecv);

    if ( dSecs < - MAX_BACKWARD_SECS
      || dSecs > MAX_RELAY_SECS
    ) {
      /* Src is forged */
      pMsg->isRecvForged = 1;
      return;
      }
    }

  if ( pSrc->szFrom[0] )
    {
    unsigned long ulVal;

    if ( pMsg->recvInfo.szHelo[0]
      && INADDR_NONE != (ulVal = inet_addr( pMsg->recvInfo.szHelo))
      && INADDR_ANY != ulVal
      && ulVal != pMsg->recvInfo.ulSrcIp
      && 0 == strcmp( pMsg->recvInfo.szHelo, pSrc->szBy)
    ) {
      /* Src is forged */
      pMsg->isRecvForged = 1;
      }
    else if ( INADDR_NONE != pSrc->ulSrcIp
      && ntohl( pSrc->ulSrcIp) >= 0xe0000000 /* Reserved */
    ) {
      /* Src is forged */
      pMsg->isRecvForged = 1;
      }
    else
      {
      if ( 0 != strcmp( pMsg->recvInfo.szFrom, pSrc->szFrom))
        pMsg->uRelays++;

      if ( 0 == SWS_stricmp( pSrc->szFrom, "localhost")
        || (INADDR_NONE != (ulVal = ntohl( pSrc->ulSrcIp))
          && ( 0x0a000000 == (0xff000000 & ulVal) /* 10.0.0.0    - 10.255.255.255 */
            || 0x7f000000 == (0xff000000 & ulVal) /* 127.0.0.0   - 127.255.255.255 */
            || 0xb0100000 == (0xfff00000 & ulVal) /* 172.16.0.0  - 172.31.255.255 */
            || 0xc0a80000 == (0xffff0000 & ulVal) /* 192.168.0.0 - 192.168.255.255 */
          )
        )
      ) {
        /* Local address */
        pMsg->isRoutedLocally = 1;
        }
      else
        {
        pMsg->recvInfo = *pSrc;
        }
      }
    }
  }


/*
 * Test if string is encoded as per RFC2047
 */
int MimeIsStringEncoded(         /* Returns 0 if not encoded */
  const char* psz                       /* IN: String */
) {
  char cEncoding;
  int iLen = 0;

  if ( 1 <= sscanf( psz, " =?%*[^?]?%c?%*[^?]?=%n", &cEncoding, &iLen)
    && iLen > 0
  ) {
    switch ( cEncoding)
      {
    case 'B':
    case 'b':
      return 'B';

    case 'Q':
    case 'q':
      return 'Q';
      }
    }

  return 0;
  }


/*
 * Decode a string as per RFC2047
 */
int MimeDecodeString(            /* Returns !0 on error */
  const char* pszSrc,                   /* IN: Encoded string */
  unsigned char* pszDst,                /* OUT: Buffer for decoded string */
  size_t size                           /* IN: Buffer size */
) {
  char cEncoding, szCharset[ 32];
  int i = 0, iLen = 0;

  if ( 2 <= sscanf( pszSrc, " =?%31[^?]?%c?%n%*[^?]?=%n",
      szCharset, &cEncoding, &i, &iLen
    )
    && i > 0
    && iLen > 0
  ) {
    switch ( cEncoding)
      {
    case 'B':
    case 'b':
      /* Base64 */
      SWS_Base64Decode( pszDst, size, pszSrc + i);
      return 0;

    case 'Q':
    case 'q':
      /* Quoted-Printable */
      MimeDecodeQuoted( pszDst, size, pszSrc + i, iLen - i - 2);
      return 0;
      }
    }

  if ( (char*) pszDst != pszSrc)
    {
    strncpy( (char*) pszDst, pszSrc, size - 1);
    pszDst[ size - 1] = cNul;
    }

  return 1;
  }


/*
 * Decode a quoted-printable string
 */
size_t MimeDecodeQuoted(
  unsigned char* pszDst,                /* OUT: Buffer for decoded string */
  size_t sizeDst,                       /* IN: Buffer size */
  const char* pszSrc,                   /* IN: Encoded string */
  size_t sizeSrc                        /* IN: Buffer size */
) {
  unsigned char * const pStart = pszDst;
  enum { kNormal, kHexMsn, kHexLsn } eState = kNormal;
  unsigned char ucData = 0;
  char c;

  while ( sizeDst > 1 && sizeSrc > 0 && cNul != (c = *(pszSrc++)) )
    {
    --sizeSrc;

    switch ( eState)
      {
    case kNormal:
      switch ( c)
        {
      case '=':
        eState = kHexMsn;
        break;

#if 0
      case '_':
        *(pszDst++) = ' ';
        --sizeDst;
        break;
#endif

      default:
        *(pszDst++) = c;
        --sizeDst;
        break;
        }
      break;

    case kHexMsn:
      eState = kHexLsn;
      switch ( c)
        {
      case '0': ucData = 0x00; break;
      case '1': ucData = 0x10; break;
      case '2': ucData = 0x20; break;
      case '3': ucData = 0x30; break;
      case '4': ucData = 0x40; break;
      case '5': ucData = 0x50; break;
      case '6': ucData = 0x60; break;
      case '7': ucData = 0x70; break;
      case '8': ucData = 0x80; break;
      case '9': ucData = 0x90; break;
      case 'A': ucData = 0xa0; break;
      case 'B': ucData = 0xb0; break;
      case 'C': ucData = 0xc0; break;
      case 'D': ucData = 0xd0; break;
      case 'E': ucData = 0xe0; break;
      case 'F': ucData = 0xf0; break;

      default:
#if 0
        *(pszDst++) = c;
        --sizeDst;
#endif
        eState = kNormal;
        break;
        }
      break;

    case kHexLsn:
      eState = kNormal;
      switch ( c)
        {
      case '0': break;
      case '1': ucData += 0x1; break;
      case '2': ucData += 0x2; break;
      case '3': ucData += 0x3; break;
      case '4': ucData += 0x4; break;
      case '5': ucData += 0x5; break;
      case '6': ucData += 0x6; break;
      case '7': ucData += 0x7; break;
      case '8': ucData += 0x8; break;
      case '9': ucData += 0x9; break;
      case 'A': ucData += 0xa; break;
      case 'B': ucData += 0xb; break;
      case 'C': ucData += 0xc; break;
      case 'D': ucData += 0xd; break;
      case 'E': ucData += 0xe; break;
      case 'F': ucData += 0xf; break;

      default:
        eState = kNormal;
        continue;
        }

      *(pszDst++) = ucData;
      --sizeDst;
      break;
      }
    }

  *pszDst = cNul;
  return (size_t)(pszDst - pStart);
  }


/****************************************************************************
 * Server comms
 ***************************************************************************/
/*
 * Issue a command
 */
static int DoCommand(
  SOCKET s,
  const char* pszCmd,
  char szBuf[],
  size_t buffersize,
  int bEcho
) {
  int n, iRet;
  char szCmd[ 256];

  /* Echo command string to logfile */
  if ( NULL != s_logfile)
    fprintf( s_logfile, "%s ", pszCmd), fflush( s_logfile);
  if ( bEcho && stdout != s_logfile)
    fprintf( stdout, "%s ", pszCmd), fflush( stdout);

  n = sprintf( szCmd, "%.253s"CRLF, pszCmd);
  if ( SOCKET_ERROR == RawWrite( s, szCmd, n) )
    {
    if ( NULL != s_logfile && stdout != s_logfile)
      fputs( "Connection closed by server\n", s_logfile);

    return SOCKET_ERROR;
    }

  iRet = GetResponse( s, szBuf, buffersize, GEN_TIMEOUT);
  if ( NULL != s_logfile)
    fputs( szBuf, s_logfile);
  if ( bEcho && stdout != s_logfile)
    fputs( szBuf, stdout);
  else if ( iRet && stdout != s_logfile)
    fprintf( stderr, "%.*s %s", n - (int)(sizeof CRLF - 1), szCmd, szBuf);

  return iRet;
  }


/*
 * Get a server response
 */
static int GetResponse(
  SOCKET s,
  char szBuf[],
  size_t buffersize,
  unsigned timeout
) {
  int iRet;

  iRet = ReadLine( s, szBuf, buffersize, timeout );
  if ( SOCKET_ERROR == iRet)
    {
    if ( NULL != s_logfile && stdout != s_logfile)
      fputs( "Connection closed by server\n", s_logfile);

    return SOCKET_ERROR;
    }

  /* Echo response to logfile */
  if ( NULL != s_logfile)
    fputs( szBuf, s_logfile), fflush( s_logfile);

  /* Check response code */
  switch ( szBuf[0])
    {
  case '+':
    iRet = 0;
    break;
  case '-':
    iRet = 1;
    break;
  default:
    iRet = 2;
    break;
    }

  return iRet;
  }


/*
 * Get a multi line response
 */
static int ReadLines(
  SOCKET s,
  FILE* outfile,
  unsigned timeout
) {
  unsigned long ulBytes = 0, ulLast = 1024;

#if 1 /* Reading a block at a time is faster */
  enum {
    kStateNL, kStateLine, kStateCR, kStateDot, kStateDotCR
  } eState = kStateNL;

  for ( ;;)
    {
    int iLen, iCount;
    char* psz, *pszLine;

    /* Get a buffer */
    iLen = RawRead(
      s,
      s_szBuf, sizeof s_szBuf - 1,
      timeout
    );
    if ( SOCKET_ERROR == iLen)
      {
      if ( NULL != s_logfile && stdout != s_logfile)
        fputs( "Server closed connection during download\n", s_logfile);

      return SOCKET_ERROR;
      }

    s_szBuf[ iLen] = '\0';
    ulBytes += (unsigned)iLen;

    if ( stdout != outfile
      && stderr != outfile
      && s_iVerbosity > 0
      && ulBytes >= ulLast
    ) {
      ulLast += 1024;
      putc( '.', stderr);
      }

    /* Parse buffer, line at a time */
    iCount = 0;
    pszLine = psz = s_szBuf;
    while ( iLen-- > 0)
      {
      const char c = *psz++;

      switch ( eState)
        {
      case kStateNL:
        switch ( c)
          {
        default:
          eState = kStateLine;
          iCount = 1;
          break;
        case 13:
          eState = kStateCR;
          iCount = 0;
          break;
        case 10:
          eState = kStateNL;
          iCount = 0;
          break;
        case '.':
          eState = kStateDot;
          iCount = 0;
          ++pszLine;
          break;
          }
        break;

      case kStateLine:
        switch ( c)
          {
        default:
          ++iCount;
          break;
        case 13:
          eState = kStateCR;
          break;
        case 10:
          eState = kStateNL;
          break;
          }
        break;

      case kStateCR:
        switch ( c)
          {
        default:
          eState = kStateLine;
          iCount = (psz - pszLine);
          break;
        case 10:
          eState = kStateNL;
          break;
          }
        break;

      case kStateDot:
        switch ( c)
          {
        default:
          eState = kStateLine;
          iCount = 1;
          break;
        case 13:
          eState = kStateDotCR;
          break;
        case 10:
          goto done;
          }
        break;

      case kStateDotCR:
        switch ( c)
          {
        default:
          eState = kStateLine;
          iCount = 2;
          break;
        case 10:
          goto done;
          }
        break;
        }

      if ( kStateNL == eState)
        {
        /* Write line */
        pszLine[ iCount] = '\0';
        iCount = 0;

        fprintf( outfile, "%s\n", pszLine);
        if ( NULL != s_logfile && outfile != s_logfile)
          fprintf( s_logfile, "%s\n", pszLine);

        pszLine = psz;
        }
      else if ( 0 == iLen)
        {
        /* Write part line */
        pszLine[ iCount] = '\0';
        iCount = 0;

        fputs( pszLine, outfile);
        if ( NULL != s_logfile && outfile != s_logfile)
          fputs( pszLine, s_logfile);

        pszLine = psz;
        }
      }
    }
done:

#else /* line at a time, slow on Win98se */
  int gotCR = 1;

  for ( ;;)
    {
    /* Get a line */
    int iLen = ReadLine( s, s_szBuf, sizeof s_szBuf -1, timeout );
    if ( SOCKET_ERROR == iLen)
      {
      if ( NULL != s_logfile && stdout != s_logfile)
        fputs( "Server closed connection during download\n", s_logfile);

      return SOCKET_ERROR;
      }

    ulBytes += (unsigned)iLen;
    if ( stdout != outfile
      && stderr != outfile
      && s_iVerbosity > 0
      && ulBytes >= ulLast
    ) {
      ulLast += 1024;
      putc( '.', stderr);
      }

    /* Check if last line */
    if ( gotCR && 0 == strcmp( s_szBuf, ".\n"))
      break;

    gotCR = iLen > 0 && '\n' == s_szBuf[ iLen - 1];

    /* Write it */
    if ( 0 == strcmp( s_szBuf, "..\n"))
      {
      fputs( ".\n", outfile);
      if ( NULL != s_logfile && outfile != s_logfile)
        fputs( ".\n", s_logfile);
      }
    else
      {
      fputs( s_szBuf, outfile);
      if ( NULL != s_logfile && outfile != s_logfile)
        fputs( s_szBuf, s_logfile);
      }
    }
#endif

  if ( stdout != outfile
    && stderr != outfile
    && s_iVerbosity > 0
    && ulBytes >= 1024
  ) {
    fputs( "\n", stderr);
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

  while ( count < size)
    {
    unsigned char c;
#if 1 /* Buffered read */
    static struct
      {
      size_t begin, end;
      unsigned char data[ 1500 * 4];
      }
    s_buf;

    if ( s_buf.begin >= s_buf.end)
      {
      iRet = RawRead( s, s_buf.data, sizeof s_buf.data, timeout);
      if ( SOCKET_ERROR == iRet)
        return SOCKET_ERROR;
      s_buf.begin = 0;
      s_buf.end = (size_t)iRet;
      }

    c = s_buf.data[ s_buf.begin++];
#else
    iRet = RawRead( s, &c, 1, timeout);
    if ( SOCKET_ERROR == iRet)
      return SOCKET_ERROR;
#endif

    if ( gotCR)
      {
      gotCR = 0;
      switch ( c)
        {
      case 0:
      case 10:
        pBuf[ count++ ] = '\n';
        if ( count < size)
          pBuf[ count] = '\0';
        iRet = count;
        size = 0; /* Force exit */
        break;

      default:
        pBuf[ count = 0] = (char)c;
        break;
        }
      }
    else switch ( c)
      {
    default:
      if ( /*isprint( c) &&*/ count + 1 <= size)
        pBuf[ count++] = (char)c;
      break;

    case 10:
      pBuf[ count++ ] = '\n';
      if ( count < size)
        pBuf[ count] = '\0';
      iRet = count;
      size = 0; /* Force exit */
      break;

    case 13:
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

  /* Until timeout */
  while ( time( NULL) <= t )
    {
    fd_set set;
    int iRet;
    struct timeval tval;

    if ( g_exit)
      return SOCKET_ERROR;

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
          fprintf( stderr, "\nConnection reset while reading\n" );
          break;
        case ECONNABORTED:
          fprintf( stderr, "\nConnection aborted while reading\n" );
          break;
        case ENOTCONN:
          fprintf( stderr, "\nDisconnected while reading\n" );
          break;
        case EWOULDBLOCK:
#if EAGAIN != EWOULDBLOCK
        case EAGAIN:
#endif
          continue;
        default:
          fprintf( stderr, "\nReceive failed (%d): %s\n",
            err, strerror( err));
          break;
          }
        return SOCKET_ERROR;
        }
      else if ( 0 == iRet)
        {
        /* Graceful disconnect */
        fprintf( stderr, "\nDisconnected\n" );
        return SOCKET_ERROR;
        }

      return iRet;
      }
    }

  fprintf( stderr, "\nTimeout in read\n" );
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
      fprintf( stderr, "Connection reset while writing\n" );
      break;
    case ECONNABORTED:
      fprintf( stderr, "Connection aborted while writing\n" );
      break;
    case ETIMEDOUT:
      fprintf( stderr, "Connection timed out while writing\n" );
      break;
    case EWOULDBLOCK:
#if EAGAIN != EWOULDBLOCK
    case EAGAIN:
#endif
      /* Win98se sometimes returns EWOULDBLOCK even though select()
      * previously indicated socket is ready to be written */
      return 0;
    default:
      fprintf( stderr, "Send failed (%d): %s\n",
        err, strerror( err));
      break;
      }
    }

  return iRet;
  }


/*
 * Non-blocking connect
 */
static int ConnectAsync(
  SOCKET s,
  const struct sockaddr_in* pTo,
  int isInteractive
) {
  int iRet = 0;

  /* Enable non-blocking mode */
  setsockasync( s, 1);

  if ( connect( s, (const struct sockaddr*)pTo, sizeof *pTo)
    && (EWOULDBLOCK == (iRet = GetLastError()) || EINPROGRESS == iRet || EAGAIN == iRet)
  ) {
    int count;

    for ( count = 0; ; ++count)
      {
      fd_set setw, sete;
      struct timeval tval;

      /* Check for SIGINT */
      if ( g_exit)
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
  setsockasync( s, 0);

  return iRet;
  }


char* SWS_strlwr( char * pszIo)
  {
  char c, *psz;

  for ( psz = pszIo; cNul != (c = *psz); ++psz)
    *psz = (char)tolower( c);

  return pszIo;
  }

/* End of file */
