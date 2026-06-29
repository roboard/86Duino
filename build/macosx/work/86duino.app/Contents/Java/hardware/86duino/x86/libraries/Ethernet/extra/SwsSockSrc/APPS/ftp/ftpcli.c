/*****************************************************************************
 *
 * File name   : ftpcli.c
 * Function    : FTP command line interpreter
 * Authors     : Lawrence Rust
 * Systems     : ANSI C
 *
 *****************************************************************************
 * File History
 * ------------
 *
 * 09-Jul-05, LVR, Created.
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
#include "ftpcli.h"

/* Imports */

/* ANSI C library */
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <time.h>
#include <ctype.h>
#include <assert.h>
#if _MSC_VER >= 1000
#include <crtdbg.h>
#endif

#include "sws_ftp.h"
#include "sws_net.h"
#include "sws_str.h"   /* SWS_stricmp */


/***************************************************************************
 * Macros & constants
 ***************************************************************************/

#define cNul  '\0'

#define EINTERRUPTED 11000
#define EQUIT        -2

#define INPUT_BUFFER_SIZE     128 /* Command file input buffer size */


/***************************************************************************
 * Data types
 ***************************************************************************/

/* Command function */
typedef struct SMethod SMethod;
typedef int FMethod( SWS_FtpClientH, char* pszTail);

/* Command descriptor */
struct SMethod
  {
  const char* pszName;
  FMethod* pfnExecute;
  const char* pszParams;
  const char* pszHelp;
  };


/***************************************************************************
 * Local function prototypes
 ***************************************************************************/

static int Interpret( SWS_FtpClientH, FILE*, int);
static int ExecuteRequest( SWS_FtpClientH, char* pszCmd);
static const SMethod* FindCommand( const char*);

static SWS_FFtpGet Response;
static SWS_FFtpGet ListOutput;
static SWS_FFtpGet GetFile;
static SWS_FFtpPut PutFile;


/***************************************************************************
 * Module data
 ***************************************************************************/

extern sig_atomic_t volatile g_exit; /* SIGINT indicator */

static FILE* s_logfile;
static int s_iVerbosity;

static int s_isBinary;

/* The method table */
static FMethod HelpMethod;
static FMethod NilMethod;
static FMethod NoopMethod;
static FMethod QuitCommand;
static FMethod UserMethod;
static FMethod PassMethod;
static FMethod PasvMethod;
static FMethod TypeMethod;
static FMethod CwdMethod;
static FMethod PwdMethod;
static FMethod MkdMethod;
static FMethod RmdMethod;
static FMethod GetMethod;
static FMethod PutMethod;
static FMethod RenMethod;
static FMethod DeleMethod;
static FMethod ListMethod;
static FMethod NlstMethod;
static FMethod StatMethod;
static FMethod SystMethod;
static FMethod RhlpMethod;
static FMethod CmdMethod;
static FMethod EchoMethod;
static FMethod ExecMethod;
static const SMethod methods[] = {
  /* FTP equivalents */
  { "quit",  &QuitCommand, "",              "Quit" },
  { "user",  &UserMethod,  "<username>",    "Login" },
  { "pass",  &PassMethod,  "<password>",    "User password" },
  { "list",  &ListMethod,  "<filespec>",    "Directory listing" },
  { "nlst",  &NlstMethod,  "<filespec>",    "Short directory listing" },
  { "stat",  &StatMethod,  "[filespec]",    "File listing" },
  { "get",   &GetMethod,   "<remote> [local]", "Fetch file from server" },
  { "put",   &PutMethod,   "<local> [remote]", "Store file on server" },
  { "ren",   &RenMethod,   "<old> <new>",   "Rename a file" },
  { "del",   &DeleMethod,  "<filename>",    "Delete a file" },
  { "pwd",   &PwdMethod,   "",              "Print working directory" },
  { "cwd",   &CwdMethod,   "<path>",        "Change working directory" },
  { "cd",    &CwdMethod,   "<path>",        "Change working directory" },
  { "mkd",   &MkdMethod,   "<path>",        "Create a directory" },
  { "rmd",   &RmdMethod,   "<path>",        "Delete a directory" },
  { "type",  &TypeMethod,  "[n]",           "Transfer type: 0= text, 1= binary" },
  { "syst",  &SystMethod,  "",              "Site info" },
  { "rhlp",  &RhlpMethod,  "[text]",        "Server help" },
  { "noop",  &NoopMethod,  "",              "No operation" },
  /* Others */
  { "pasv",  &PasvMethod,  "[n]",           "1= passive connections, 0= default" },
  { "cmd" ,  &CmdMethod,   "<command>",     "Execute server command" },
  { "echo",  &EchoMethod,  "<on|off>",      "Command/response echoing" },
  /* Scripting */
  { "exec",  &ExecMethod,  "[filename]",    "Execute commands from a script" },
  { "rem",   &NilMethod,   "[text]",        "Script comment" },
  /* Others */
  { "help",  &HelpMethod,  "[command]",     "Show command help" },
  { "?",     &HelpMethod,  "[command]",     NULL },
  { "",      &NilMethod,   "",              NULL }
};


/***************************************************************************
 * Functions
 ***************************************************************************/

/*
 * FTP command line interpreter
 */
extern int FtpCli(
  int isScript,                   /* 0= interactive */
  const char* pszServer,          /* Server hostname */
  unsigned uPort,                 /* Server port */
  FILE* logfile,                  /* Logfile, NULL= none */
  int iVerbosity                  /* verbosity */
) {
  int iRet;
  SWS_FtpClientH hClient;

  s_logfile = NULL != logfile ? logfile : stdout;
  s_iVerbosity = iVerbosity;

  hClient = FtpClientCreate( logfile, iVerbosity);
  if ( NULL == hClient)
    return ENOMEM;

  /* Set command response callback */
  if ( iVerbosity > 0)
    FtpSetCallback( hClient, &Response, stdout);

  /* Connect to server */
  iRet = FtpConnect( hClient, pszServer, uPort);
  if ( 220 == iRet)
    {
    /* Interpret stdin */
    iRet = Interpret( hClient, stdin, isScript);
    if ( EQUIT == iRet)
      iRet = 0;
    }
  else switch ( iRet)
    {
  case EINVAL:
    fputs( "No such server\n", stderr);
    break;
  case EHOST_NOT_FOUND:
    fprintf( stderr, "Unable to resolve %s\n", pszServer );
    break;
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
    fputs( "Connection refused.\n", stderr);
    break;
  default:
    fprintf( stderr, "Failed (%d)\n", iRet);
    break;
    }

  /* Disconnect server */
  FtpClientDestroy( hClient);

  return iRet;
  }

SWS_CB( int, Response(          /* Return 0 to continue */
  SWS_PTR( void) pv,                  /* IN: user context */
  SWS_FtpClientH hClient,             /* IN, client handle */
  SWS_PTR( const void) pvData,        /* IN: -> data buffer */
  int len                             /* IN: buffer size */
)){
  (void)hClient;

  return (size_t)len == fwrite( pvData, 1, (size_t)len, (FILE*)pv) ? 0 : 1;
  }


/*
 * Command line interpreter
 */
static int Interpret( SWS_FtpClientH hClient, FILE* file, int isScript)
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
    if ( NULL == fgets( szBuf, sizeof szBuf, file))
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
    else if ( !feof( file))
      {
      fputs( "Input truncated\n", stderr);
      if ( !isScript)
        {
        rewind( file);
        continue;
        }

      iRet = 1;
      break;
      }

    /* Interpret command */
    iRet = ExecuteRequest( hClient, szBuf);
    }
  while ( (iRet >= 0 && iRet < 400) || (iRet > 0 && !isScript));

  --s_iDepth;

  return iRet;
  }


/*
 * Execute a command
 */
static int ExecuteRequest(
  SWS_FtpClientH hClient,
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
  if ( stdout != s_logfile)
    fprintf( s_logfile, "%s\n", pszCmd);

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
    iRet = (*pMethod->pfnExecute)( hClient, pszTail);
    }
  else
    {
    fprintf( stderr, "Unknown command : %s\n", pszCmd);
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

static int HelpMethod( SWS_FtpClientH hClient, char* pszTail)
  {
  const SMethod* pMethod;
  int iRet = 0;

  (void)hClient;

  if ( cNul == *pszTail)
    {
    int i;

    fprintf( stdout, "Commands available:\n");

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
    fprintf( stdout, "Unknown command: %s\n", pszTail);
    iRet = 1;
    }

  return iRet;
  }


static int NilMethod( SWS_FtpClientH hClient, char* pszTail)
  {
  (void)hClient;
  (void)pszTail;
  return 0;
  }


static int EchoMethod( SWS_FtpClientH hClient, char* pszTail)
  {
  (void)hClient;

  if ( '\0' == *pszTail)
    {
    fprintf( stdout, "Echo is %s\n", s_iVerbosity ? "on" : "off");
    }
  else if ( 0 == SWS_stricmp( pszTail, "on"))
    {
    s_iVerbosity = 1;
    FtpSetCallback( hClient, &Response, stdout);
    }
  else if ( 0 == SWS_stricmp( pszTail, "off"))
    {
    s_iVerbosity = 0;
    FtpSetCallback( hClient, 0, NULL);
    }
  else
    {
    fputs( "Invalid argument\n", stderr);
    return 1;
    }

  return 0;
  }


/*
 * Logging in
 */
static int QuitCommand( SWS_FtpClientH hClient, char* pszTail)
  {
  if ( cNul != *pszTail)
    {
    fputs( "Unexpected argument\n", stderr);
    return 1;
    }

  FtpQuit( hClient);
  return EQUIT;
  }


static int UserMethod( SWS_FtpClientH hClient, char* pszTail)
  {
  return FtpUser( hClient, pszTail);
  }


static int PassMethod( SWS_FtpClientH hClient, char* pszTail)
  {
  return FtpPass( hClient, pszTail);
  }


/*
 * Passive data connections
 */
static int PasvMethod( SWS_FtpClientH hClient, char* pszTail)
  {
  int iRet;

  if ( cNul == *pszTail)
    {
    iRet = 1;
    }
  else if ( 1 != sscanf( pszTail, "%d", &iRet))
    {
    fputs( "Invalid argument\n", stderr);
    return 1;
    }

  return FtpPasv( hClient, iRet);
  }


/*
 * Binary/text mode
 */
static int TypeMethod( SWS_FtpClientH hClient, char* pszTail)
  {
  int iMode, iRet;

  if ( cNul == *pszTail)
    {
    iMode = 0;
    }
  else if ( 1 != sscanf( pszTail, "%d", &iMode))
    {
    fputs( "Invalid argument\n", stderr);
    return 1;
    }

  iRet = FtpType( hClient, iMode);
  if ( 200 == iRet)
    s_isBinary = iMode;

  return iRet;
  }


/*
 * Listings
 */
static int StatMethod( SWS_FtpClientH hClient, char* pszTail)
  {
  int iRet;

  if ( s_iVerbosity < 1)
    FtpSetCallback( hClient, &Response, stdout);

  iRet = FtpStat( hClient, pszTail);

  if ( s_iVerbosity < 1)
    FtpSetCallback( hClient, 0, NULL);

  return iRet;
  }

static int ListMethod( SWS_FtpClientH hClient, char* pszTail)
  {
  return FtpList( hClient, pszTail, ListOutput, stdout);
  }

static int NlstMethod( SWS_FtpClientH hClient, char* pszTail)
  {
  return FtpNlst( hClient, pszTail, &ListOutput, stdout);
  }

SWS_CB( int, ListOutput(              /* Return 0 to continue */
  SWS_PTR( void) pv,                  /* IN: user context */
  SWS_FtpClientH hClient,             /* IN, client handle */
  SWS_PTR( const void) pvData,        /* IN: -> data buffer */
  int len                             /* IN: buffer size */
)){
  (void)hClient;
  return (size_t)len == fwrite( pvData, 1, (size_t)len, (FILE*)pv) ? 0 : 1;
  }


/*
 * Rename a file
 */
static int RenMethod( SWS_FtpClientH hClient, char* pszTail)
  {
  char* pszOld, *pszNew;

  if ( cNul == *pszTail)
    {
    fputs( "Missing old filename\n", stderr);
    return 1;
    }
  pszOld = pszTail;

  /* Step over old filename */
  while ( cNul != *pszTail && !isspace( *pszTail))
    ++pszTail;

  if ( cNul == *pszTail)
    {
    fputs( "Missing new filename\n", stderr);
    return 1;
    }

  *pszTail = cNul;
  pszNew = ++pszTail;

  return FtpRen( hClient, pszOld, pszNew);
  }


/*
 * Delete a file
 */
static int DeleMethod( SWS_FtpClientH hClient, char* pszTail)
  {
  return FtpDel( hClient, pszTail);
  }


/*
 * Fetch file from server
 */
static int GetMethod( SWS_FtpClientH hClient, char* pszTail)
  {
  char* pszSrc, *pszDst;
  FILE* outfile;
  int iRet;

  if ( cNul == *pszTail)
    {
    fputs( "Missing source file\n", stderr);
    return 1;
    }
  pszSrc = pszTail;

  /* Step over src (remote) filename */
  while ( cNul != *pszTail && !isspace( *pszTail))
    ++pszTail;

  if ( cNul == *pszTail)
    {
    outfile = stdout;
    }
  else
    {
    *pszTail = cNul;
    pszDst = ++pszTail;

    outfile = fopen( pszDst, "r");
    if ( NULL != outfile)
      {
      fprintf( stderr, "%s exists\n", pszDst );
      fclose( outfile);
      return 1;
      }

    /* Open the target file */
    outfile = fopen( pszDst, s_isBinary ? "wb" : "w");
    if ( NULL == outfile)
      {
      iRet = errno;
      fprintf( stderr, "Failed to create '%s': %s\n",
        pszDst, strerror( iRet) );
      return iRet;
      }
    }

  iRet = FtpGet( hClient, pszSrc, &GetFile, outfile);

  if ( stdout != outfile)
    fclose( outfile);

  return iRet;
  }

SWS_CB( int, GetFile(                 /* Return 0 to continue */
  SWS_PTR( void) pv,                  /* IN: user context */
  SWS_FtpClientH hClient,             /* IN, client handle */
  SWS_PTR( const void) pvData,        /* IN: -> data buffer */
  int len                             /* IN: buffer size */
)){
  (void)hClient;

  return (size_t)len == fwrite( pvData, 1, (size_t)len, (FILE*)pv) ? 0 : 1;
  }


/*
 * Store file on server
 */
static int PutMethod( SWS_FtpClientH hClient, char* pszTail)
  {
  char* pszSrc, *pszDst;
  FILE* infile;
  int iRet;

  if ( cNul == *pszTail)
    {
    fputs( "Missing source file\n", stderr);
    return 1;
    }
  pszSrc = pszTail;

  /* Step over src (local) filename */
  while ( cNul != *pszTail && !isspace( *pszTail))
    ++pszTail;

  if ( cNul == *pszTail)
    {
    pszDst = pszSrc;
    }
  else
    {
    *pszTail = cNul;
    pszDst = ++pszTail;
    }

  /* Open the source file */
  infile = fopen( pszSrc, s_isBinary ? "rb" : "r");
  if ( NULL == infile)
    {
    iRet = errno;
    fprintf( stderr, "Failed to open '%s': %s\n",
      pszDst, strerror( iRet) );
    return iRet;
    }

  iRet = FtpPut( hClient, pszDst, &PutFile, infile);

  fclose( infile);
  return iRet;
  }

SWS_CB( int, PutFile(                 /* Return bytes read */
  SWS_PTR( void) pv,                  /* IN: user context */
  SWS_FtpClientH hClient,             /* IN, client handle */
  SWS_PTR( void) pvData,              /* IN: -> data buffer */
  int len                             /* IN: buffer size */
)){
  size_t sizeRead;
  (void)hClient;

  sizeRead = fread( pvData, 1, (size_t)len, (FILE*)pv);
  if ( sizeRead != (size_t)len)
    {
    if ( ferror( (FILE*)pv))
      return -1;
    }

  return (int)sizeRead;
  }


/*
 * Directories
 */
static int PwdMethod( SWS_FtpClientH hClient, char* pszTail)
  {
  int iRet;

  if ( cNul != *pszTail)
    {
    fputs( "Unexpected argument\n", stderr);
    return 1;
    }

  iRet = FtpPwd( hClient, INPUT_BUFFER_SIZE - 4, pszTail);
  if ( 257 == iRet && s_iVerbosity < 1)
    printf( "%s\n", pszTail);

  return iRet;
  }

static int CwdMethod( SWS_FtpClientH hClient, char* pszTail)
  {
  return FtpCwd( hClient, pszTail);
  }

static int MkdMethod( SWS_FtpClientH hClient, char* pszTail)
  {
  return FtpMkd( hClient, pszTail);
  }

static int RmdMethod( SWS_FtpClientH hClient, char* pszTail)
  {
  return FtpRmd( hClient, pszTail);
  }


/*
 * Utils
 */
static int NoopMethod( SWS_FtpClientH hClient, char* pszTail)
  {
  if ( cNul != *pszTail)
    {
    fputs( "Unexpected argument\n", stderr);
    return 1;
    }

  return FtpNoop( hClient);
  }


static int SystMethod( SWS_FtpClientH hClient, char* pszTail)
  {
  int iRet;

  if ( cNul != *pszTail)
    {
    fputs( "Unexpected argument\n", stderr);
    return 1;
    }

  iRet = FtpSyst( hClient, INPUT_BUFFER_SIZE - 4, pszTail);
  if ( 215 == iRet && s_iVerbosity < 1)
    printf( "%s\n", pszTail);

  return iRet;
  }


static int RhlpMethod( SWS_FtpClientH hClient, char* pszTail)
  {
  int iRet;

  if ( s_iVerbosity < 1)
    FtpSetCallback( hClient, &Response, stdout);

  iRet = FtpHelp( hClient, pszTail);

  if ( s_iVerbosity < 1)
    FtpSetCallback( hClient, 0, NULL);

  return iRet;
  }


/*
 * Execute literal command
 */
int CmdMethod( SWS_FtpClientH hClient, char* pszTail)
  {
  int iRet;

  if ( s_iVerbosity < 1)
    FtpSetCallback( hClient, &Response, stdout);

  iRet = FtpCmd( hClient, pszTail);

  if ( s_iVerbosity < 1)
    FtpSetCallback( hClient, 0, NULL);

  return iRet;
  }


/*
 * Redirect input
 */
int ExecMethod( SWS_FtpClientH hClient, char* pszTail)
  {
  static const char kszWhiteSpace[] = " \t\n";
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
    fprintf( stderr, "Failed to open %s\n", psz);
    return 1;
    }

  iRet = Interpret( hClient, file, stdin != file);

  if ( stdin != file)
    fclose( file);

  return iRet;
  }

/* End of file */
