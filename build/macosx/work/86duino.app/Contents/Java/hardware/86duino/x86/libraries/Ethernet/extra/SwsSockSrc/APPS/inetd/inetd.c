/*****************************************************************************
 *
 * File name   : inetd.c
 * Function    : Multiple TCP/IP services using threads
 * Authors     : Lawrence Rust
 * Systems     : ANSI C, sockets
 * Version     : 1.0
 * Release Date: 18-Dec-02
 *
 *****************************************************************************
 * File History
 * ------------
 *
 * 16-Dec-02, LVR, Created.
 * 29-Jan-10, LVR, V1.01, New table driven task creation
 *
 *
 * Comments
 * --------
 *
 *****************************************************************************/
#define kszApp "inetd"
#define kszVer "1.01"
#define kszBanner kszApp " " kszVer " (" SWS_NET_ID ")" ", " TIMESTAMP "."
#define kszAbout "Internet services host."

/* Compilation options */
#if INETD_OPT
#include "options.h"
#endif

#ifndef SWS_USE_SOCKLIB
#define SWS_USE_SOCKLIB 0 /* 0= native sockets, !0 for sws_sock */
#endif

#ifndef PACKET_DUMP
#define PACKET_DUMP 0 /* !0 to include SwsSock packet dump */
#endif

/* Tasks */
#ifndef INETD_FTP
#define INETD_FTP 0
#endif
#ifndef INETD_TELNET
#define INETD_TELNET 0
#endif
#ifndef INETD_SMTP
#define INETD_SMTP 0
#endif
#ifndef INETD_HTTP
#define INETD_HTTP 0
#endif
#ifndef INETD_POP
#define INETD_POP 0
#endif
#ifndef INETD_ECHO
#define INETD_ECHO 0
#endif
#ifndef INETD_DISCARD
#define INETD_DISCARD 0
#endif
#ifndef INETD_IDENT
#define INETD_IDENT 0
#endif
#ifndef INETD_NETBIOS
#define INETD_NETBIOS 0
#endif

#ifndef INETD_DAEMON
#define INETD_DAEMON 0 /* !0 to add daemon mode */
#endif


/* Exports */

/* Imports */

/* ANSI C library */
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include <assert.h>

#include "sws_net.h"
#if SWS_USE_SOCKLIB && PACKET_DUMP
#include "sws_dump.h"
#endif
#include "sws_thrd.h"
#include "sws_cfg.h"
#include "sws_ansi.h"

#if INETD_HTTP
#include "httpd.h"
#include "httpfile.h"
# if HTTP_PROXY
#  include "httpprxy.h"
# endif
# if HTTP_CGI
#  include "httpcgi.h"
# endif
# if HTTP_V4L
#  include "httpv4l.h"
# endif
# if HTTP_LFG
#  include "httplfg.h"
# endif
#endif
#if INETD_FTP
#include "ftpd.h"
#endif
#if INETD_TELNET
#include "telnetd.h"
#endif
#if INETD_SMTP
#include "smtpd.h"
#endif
#if INETD_POP
#include "popd.h"
#endif
#if INETD_ECHO || INETD_DISCARD
#include "echod.h"
#endif
#if INETD_IDENT
#include "identd.h"
#endif
#if INETD_NETBIOS
#include "netbiosd.h"
#include "sws_smb.h"
#endif

#if INETD_DAEMON && (defined linux || defined __linux || defined __linux__)
# define OPT_DAEMON 1
#endif


/***************************************************************************
 * Macros & constants
 ***************************************************************************/

/* Length of an array - sorely missing from stddef.h */
#ifndef lengthof
#define lengthof( _a) ((size_t)( sizeof(_a)/sizeof((_a)[0]) ))
#endif

#ifdef __TIMESTAMP__
# define TIMESTAMP "edited " __TIMESTAMP__
#else
# define TIMESTAMP "built " __TIME__ " " __DATE__
#endif

/* Expand and stringize a macro name */
#define STR(s) #s
#define STRING(s) STR(s)

/* ini file values */
#define kszInetdSect "inetd"


/***************************************************************************
 * Data types
 ***************************************************************************/

/* Parameters */
typedef struct SConfig
  {
  const char* pszIni;
#if OPT_DAEMON
  const char* pszPid;
#endif
  } SConfig;

/* Task info */
struct STask;
typedef int FThreadInit( const struct STask*);
struct STask
  {
  struct STask* pNext;
  const char* pszName;        /* .ini file value for disable */
  FThreadInit* pfnInit;       /* optional init function */
  SWS_ThreadFunction* pfnThread;
  SWS_ThreadHandle hThread;
  unsigned uPort;             /* Port to listen on */
  };


/***************************************************************************
 * Local function prototypes
 ***************************************************************************/

#if OPT_DAEMON
# define DAEMON_SETID 1
/*# define DAEMON_UMASK 026 * -rw-r----- access rights */
# define DAEMON_UMASK 022 /* -rw-r--r-- access rights */
# undef socklen_t
# undef gethostname
# include "sws_damn.h"
#endif
#ifdef SIGINT
static void OnSigInt( int);
#endif
#ifdef SIGHUP
static void OnSigHup( int);
#endif
#ifdef SIGSEGV
static void OnSigSegv( int);
#endif
static int ParseArgs( SConfig* const, int, const char* const [] );
static void Help( const char* const);
static void LogInit( FILE*, int);

static int inetd( const SConfig* );


/***************************************************************************
 * Module data
 ***************************************************************************/

sig_atomic_t volatile g_exit;
sig_atomic_t volatile g_running;        /* No. threads that signalled running */
static sig_atomic_t volatile s_threads; /* No. threads started */

static int s_iVerbosity;
static FILE* s_logfile;
static const char* s_logname;
#if OPT_DAEMON
static const char* s_pszDaemon;
#endif

/* Macro to create a task */
#define DEFTASK( name, port, fnRun, argRun, fnInit)\
  static SWS_ThreadFunction TASK_FN(name);\
  static struct STask TASK(name) = {\
    TASK_HEAD, STRING(name), fnInit, TASK_FN(name), NULL, port\
  };\
  static SWS_CB( i32, TASK_FN(name)( SWS_PTR( void) pvContext))\
    {\
    const struct STask* const pTask = (const struct STask*)pvContext;\
    int iRet = -1, i;\
    struct sockaddr_in sa;\
    sa.sin_family = AF_INET;\
    sa.sin_port = htons( (u_short)pTask->uPort);\
    sa.sin_addr.s_addr = htonl( INADDR_ANY);\
    for ( i = 90; i > 0 && !g_exit; --i, Sleep( 1000))\
      if ( EADDRINUSE != (iRet = fnRun argRun)) break;\
    --s_threads;\
    return iRet;\
    }
#define TASK_SA sa                  /* Interface */
#define TASK_FN(name) name##Thread  /* Thread name */
#define TASK(name) s_##name##Task   /* Table name */

/* Head of the task list */
#define TASK_HEAD NULL

/* Task definitions */
#if INETD_FTP
DEFTASK( ftp, FTPD_PORT, Ftpd, (&TASK_SA, s_iVerbosity, s_logfile), 0 )
#undef TASK_HEAD
#define TASK_HEAD &TASK( ftp)
#endif

#if INETD_HTTP
static FThreadInit HttpInit;
DEFTASK( http, HTTPD_PORT, Httpd, (&TASK_SA, s_iVerbosity, s_logfile), HttpInit )
#undef TASK_HEAD
#define TASK_HEAD &TASK( http)
#endif

#if INETD_TELNET
DEFTASK( telnet, TELNETD_PORT, Telnetd, (&TASK_SA, s_iVerbosity, s_logfile, 0, NULL), 0 )
#undef TASK_HEAD
#define TASK_HEAD &TASK( telnet)
#endif

#if INETD_SMTP
DEFTASK( smtp, SMTPD_PORT, Smtpd, (&TASK_SA, s_iVerbosity, s_logfile), 0 )
#undef TASK_HEAD
#define TASK_HEAD &TASK( smtp)
#endif

#if INETD_POP
DEFTASK( pop, POPD_PORT, Popd, (&TASK_SA, s_iVerbosity, s_logfile), 0 )
#undef TASK_HEAD
#define TASK_HEAD &TASK( pop)
#endif

#if INETD_ECHO
DEFTASK( echo, ECHOD_PORT, Echod, ((struct sockaddr*)&TASK_SA, 1, s_iVerbosity, s_logfile), 0 )
#undef TASK_HEAD
#define TASK_HEAD &TASK( echo)
#endif

#if INETD_DISCARD
DEFTASK( discard, DSCRD_PORT, Echod, ((struct sockaddr*)&TASK_SA, 0, s_iVerbosity, s_logfile), 0 )
#undef TASK_HEAD
#define TASK_HEAD &TASK( discard)
#endif

#if INETD_IDENT
DEFTASK( ident, IDENTD_PORT, Identd, ((struct sockaddr*)&TASK_SA, s_iVerbosity, s_logfile), 0 )
#undef TASK_HEAD
#define TASK_HEAD &TASK( ident)
#endif

#if INETD_NETBIOS
DEFTASK( netbios, SWS_NBSS, Netbiosd, (&TASK_SA, kModeAck, s_iVerbosity, s_logfile), 0 )
#undef TASK_HEAD
#define TASK_HEAD &TASK( netbios)
#endif


/***************************************************************************
 * Functions
 ***************************************************************************/


/*
 * Show command line syntax
 */
static void Help( const char* const pszArg)
  {
  const char* pszCmd;
  const char* pszDot;
  size_t c;

  fputs( kszBanner "\n" kszAbout "\n", stderr);

  /* Get command name without path */
  if ( NULL != pszArg)
    {
    pszCmd = strrchr( pszArg, '/');
    if ( NULL != pszCmd)
      ++pszCmd;
    else
      {
      pszCmd = strrchr( pszArg, '\\');
      if ( NULL != pszCmd)
        ++pszCmd;
      else
        pszCmd = pszArg;
      }
    }
  else
    {
    pszCmd = kszApp;
    }

  /* Count command name length less any '.' extension */
  pszDot = strrchr( pszCmd, '.');
  if ( NULL != pszDot)
    c = (size_t)(pszDot - pszCmd);
  else
    c = strlen( pszCmd);

  fprintf( stderr, "\nSyntax: %.*s [options] args\nArg:\n", (int)c, pszCmd);
  { struct STask* pTask;
  for ( pTask = TASK_HEAD; NULL != pTask; pTask = pTask->pNext)
    {
    int i = fprintf( stderr, "%s=<n>", pTask->pszName);
    fprintf( stderr, "%.*s : Set %s port (0= disable)\n",
      i < 10 ? 10 - i : 0, "        ", pTask->pszName);
    }
  }
  fputs( "Option:\n"
#if OPT_DAEMON
    "-d[s] : Run as daemon, username s. Blank or '-' for current user\n"
    "-p<s> : Daemon PID filename\n"
#endif
    "-c<s> : Configuration filename\n"
    "-o[s] : Set logfile\n"
    "-t    : Decrease verbosity of output\n"
    "-v    : Increase verbosity of output\n"
    "-h    : Help\n", stderr
  );
  }


/*
 * M A I N
 */
int main( int const argc, const char* const argv[] )
  {
  SConfig cfg;
  int iRet;
  struct hostent* pHost;
  char szHost[ 256];
  time_t t;

  /* Set default options */
  s_iVerbosity = 1;
  s_logfile = stdout;
#if OPT_DAEMON
  s_pszDaemon = NULL;
  cfg.pszPid = NULL;
#endif
  cfg.pszIni = SWS_CFGFILE;

#ifdef DAEMON_UMASK
  umask( DAEMON_UMASK); /* Set file access rights */
#endif

  /* Parse args */
  iRet = ParseArgs( &cfg, argc, argv );
  if ( EXIT_SUCCESS != iRet)
    return iRet;

  if ( s_iVerbosity >= 0
#if OPT_DAEMON
    && !s_pszDaemon
#endif
  ) fputs( kszBanner "\n", stderr);

  /* Specify the configuration file */
  if ( !SWS_CfgSetName( cfg.pszIni, argv[ 0]))
    fprintf( stderr, "WARNING: configuration file '%s' not found\n", cfg.pszIni);

#ifdef SIGINT
  /* Register a handler for ctrl-c */
  signal( SIGINT, &OnSigInt);
#endif
#ifdef SIGSEGV
  /* Register a handler for seg-fault */
  signal( SIGSEGV, &OnSigSegv);
#endif

  /* Initialise the sockets library */
#if PACKET_DUMP
# define GEF( _f, _v) SWS_PKTDUMP( _f, _v)
#else
# define GEF( _f, _v) 0
#endif
  SWS_SockUp( GEF( s_logfile, s_iVerbosity), NULL);

  /* Wait (with timeout) until network is up */
  t = time( NULL) + 60;
  while ( gethostname( szHost, sizeof(szHost))
    || NULL == (pHost = gethostbyname( szHost))
  ) {
#if OPT_DAEMON
    if ( !s_pszDaemon)
#endif
    fprintf( stderr, "Waiting for network\n");
    if ( g_exit)
      return EXIT_FAILURE;
#if OPT_DAEMON
    if ( !s_pszDaemon)
#endif
    if ( time( NULL) > t)
      return EXIT_FAILURE;
    Sleep( 3000);
    }

  if ( s_iVerbosity >= 0
#if OPT_DAEMON
    && !s_pszDaemon
#endif
  ) {
    int i;

    fprintf( stderr, "This host is %s [", szHost);

    /* Display the list of local IP addresses */
    for ( i = 0; NULL != pHost->h_addr_list[i]; ++i)
      {
      if ( i > 0)
        fputs( ", ", stderr);
      fprintf( stderr, "%s", inet_ntoa( *(const struct in_addr*)pHost->h_addr_list[i] ) );
      }
    fputs( "]\n", stderr);
    }

#if OPT_DAEMON
  if ( s_pszDaemon)
    {
    /* Fork off the daemon and terminate the parent */
    if ( SWS_Daemon())
      return EXIT_FAILURE;

    /* Set group memberships for the daemon acct */
    SWS_DaemonSetGid( s_pszDaemon);

    /* Change the owner/group of the logfile */
    if ( s_logname)
      SWS_DaemonChown( s_logfile, s_pszDaemon);

    /* Create the PID run file */
    if ( cfg.pszPid)
      SWS_DaemonPidFile( cfg.pszPid, s_pszDaemon);
    }
#endif

#ifdef SIGHUP
  if ( s_logname)
    signal( SIGHUP, &OnSigHup);
#endif

  /* Start configured services */
  iRet = inetd( &cfg);

  if ( s_logname)
    {
    t = time( NULL);
    fprintf( s_logfile, "Logging stopped %s", ctime( &t) );
    }

  return iRet ? EXIT_FAILURE : EXIT_SUCCESS;
  }


/*
 * Start all configured services
 */
static int inetd( const SConfig* pCfg)
  {
  struct STask* pTask;
  (void)pCfg;

  /* Start tasks */
  for ( pTask = TASK_HEAD; NULL != pTask; pTask = pTask->pNext)
    {
    if ( 0 != pTask->uPort
      && 0 != (pTask->uPort = SWS_CfgGetValue( kszInetdSect, pTask->pszName, pTask->uPort))
      && (0 == pTask->pfnInit || 0 == (*pTask->pfnInit)( pTask))
    ) {
      if ( NULL != (pTask->hThread = SWS_ThreadCreate( pTask->pfnThread, pTask, 0)))
        ++s_threads, SWS_ThreadStart( pTask->hThread);
      }
    }

#if OPT_DAEMON
   if ( s_pszDaemon)
    {
    /* Redirect stderr to the log */
    dup2( fileno( s_logfile), fileno( stderr) );
    }
  else
#endif
  if ( s_iVerbosity > 0)
    fputs( ANSI_INVRS "Ctrl+C" ANSI_RST " to exit...\n", stderr);

  /* Wait for all threads to exit */
  do
    {
    Sleep( 500);
#ifdef OPT_DAEMON
    { static int s_iOnce;
    if ( 0 == s_iOnce && !g_exit && g_running == s_threads)
      {
      s_iOnce = 1;
      /* Irreversibly drop privileges */
      if ( s_pszDaemon)
        SWS_DaemonSetUid( s_pszDaemon);
      }
    }
#endif
    }
  while ( s_threads > 0);

  /* Cleanup threads */
  for ( pTask = TASK_HEAD; NULL != pTask; pTask = pTask->pNext)
    {
    if ( NULL != pTask->hThread)
      {
      SWS_ThreadStop( pTask->hThread);
      SWS_ThreadDestroy( pTask->hThread);
      pTask->hThread = NULL;
      }
    }

  /* Destroy the current (main) thread */
  SWS_ThreadDestroy( NULL );

  return 0;
  }


/*
 * HTTP initialisation
 */
static int HttpInit( const struct STask* pTask)
  {
  (void)pTask;

  /* Install HTTP file access */
  if ( !HttpFileInit())
    fputs( "WARNING: HTTP root file system is invalid.\n", stderr);
#if HTTP_PROXY
  if ( !HttpProxyInit())
    fputs( "WARNING: HTTP proxy disabled\n", stderr);
#endif
#if HTTP_CGI
  if ( !HttpCgiInit())
    fputs( "WARNING: HTTP root CGI is invalid\n", stderr);
#endif
#if HTTP_V4L
  if ( !HttpV4lInit())
    fputs( "WARNING: Video4Linux module startup failure\n", stderr);
#endif
#if HTTP_LFG
  if ( !HttpLfgInit())
    fputs( "WARNING: LFG module startup failure\n", stderr);
#endif
  return 0;
  }


#ifdef SIGINT
/*
 * SIGINT handler (ctrl-c)
 */
static void OnSigInt( int sig)
  {
  if ( ++g_exit >= 2)
    exit( EXIT_FAILURE);

#ifdef NDEBUG
  signal( sig, &OnSigInt);
#else
  (void)sig;
#endif
  }
#endif


#ifdef SIGSEGV
/*
 * SIGSEGV handler
 */
static void OnSigSegv( int sig)
  {
  (void)sig;
  fputs( "ERROR: Segmentation violation\n", stderr);
  abort();
  }
#endif


#ifdef SIGHUP
/*
 * SIGHUP handler
 */
static void OnSigHup( int sig)
  {
  const time_t t = time( NULL);

  assert( s_logname);

  /* Reopen the logfile */
  fprintf( s_logfile, "OnSigHup %s", ctime( &t) );
  if ( NULL == freopen( s_logname, "a", s_logfile))
    perror( "OnSigHup failed to redirect logfile");
  else
    {
    LogInit( s_logfile, s_iVerbosity);

    /* Redirect stderr to the logfile */
    if ( NULL == freopen( s_logname, "a", stderr))
      perror( "OnSigHup failed to redirect stderr");

#if OPT_DAEMON
    SWS_DaemonChown( s_logfile, s_pszDaemon);
#endif
    }

  signal( sig, &OnSigHup);
  }
#endif


/*
 * Log file initialisation
 */
static void LogInit( FILE* file, int iVerbosity)
  {
  time_t t = time( NULL);
  setvbuf( file, NULL, _IONBF, 0 ); /* Unbuffered otherwise tail doesn't follow in realtime */
  fputs( kszBanner "\n", file);
  fprintf( file, "Logging(%d) started %s", iVerbosity, ctime( &t) );
  }


/*
 * Scan the command line for options
 */
static int ParseArgs(                 /* Returns !0 on error */
  SConfig* const pCfg,                /* OUT: Updated parameters */
  int iArgc,                          /* IN: No. args */
  const char* const pszArgvArray[]    /* IN: Array of arg strings */
) {
  int i;
  int bHelp = 0;

  /* Scan args */
  for ( i = 1; i < iArgc; ++i)
    {
    char c;
    const char* pszArg;
    char szName[ 16];
    unsigned uVal;

    pszArg = pszArgvArray[i];


    /* Check for switch options ('-' or '/' prefix) */
#   define isswitch( _c) ('-' == _c)
    c = *pszArg++;
    if ( isswitch( c) )
      {
      /* Option found */
      do
        {
#       define GetArg() \
          ('\0' != *pszArg \
            || i + 1 >= iArgc \
            || isswitch( pszArgvArray[ i + 1][ 0] ) \
          ) ? pszArg : pszArgvArray[ ++i]

        switch ( *pszArg++)
          {
        case 'h':
        case '?':
          bHelp = !0;
          break;

        case 'V':
          fputs( kszBanner "\n", stderr);
          return EXIT_FAILURE;

        case 't':
          /* terse */
          s_iVerbosity--;
          break;

        case 'v':
          /* verbose */
          s_iVerbosity++;
          break;

        case 'o':
          /* logfile */
          pszArg = GetArg();
          if ( '\0' == *pszArg)
            {
            fputs( "Missing logfile name\n", stderr);
            return EXIT_FAILURE;
            }
          else if ( stdout != s_logfile)
            {
            fputs( "Duplicate logfile\n", stderr);
            return EXIT_FAILURE;
            }
#if OPT_DAEMON
          else if ( s_pszDaemon)
            {
            if ( NULL == freopen( pszArg, "a", s_logfile))
              {
              fprintf( stderr, "Failed to open logfile '%s'\n", pszArg);
              return EXIT_FAILURE;
              }
            }
#endif
          else if ( NULL == (s_logfile = fopen( pszArg, "a")))
            {
            fprintf( stderr, "Failed to open logfile '%s'\n", pszArg);
            return EXIT_FAILURE;
            }

          s_logname = pszArg;
          LogInit( s_logfile, s_iVerbosity);
          pszArg = NULL;
          break;

        case 'c':
          /* config file */
          pszArg = GetArg();
          pCfg->pszIni = pszArg;
          pszArg = NULL;
          break;

#if OPT_DAEMON
        case 'd':
          pszArg = GetArg();
          s_pszDaemon = strcmp( "-", pszArg) ? pszArg : "";
          pszArg = NULL;
          break;

        case 'p':
          pszArg = GetArg();
          pCfg->pszPid = pszArg;
          pszArg = NULL;
          break;
#endif

        default:
          fprintf( stderr, "Invalid switch: '%s'\n", pszArgvArray[i] );
          return EXIT_FAILURE;
          }
        }
      while ( NULL != pszArg && '\0' != *pszArg);
      }
    /* Non-switch argument */
    else if ( 2 == sscanf( pszArgvArray[i], "%15[^=]=%u", szName, &uVal))
      {
      struct STask* pTask;
      for ( pTask = TASK_HEAD; NULL != pTask; pTask = pTask->pNext)
        {
        if ( 0 == strncmp( szName, pTask->pszName, sizeof szName))
          {
          pTask->uPort = uVal;
          break;
          }
        }
      if ( NULL == pTask)
        {
        fprintf( stderr, "Unknown service: '%s'\n", szName );
        return EXIT_FAILURE;
        }
      }
    else
      {
      fprintf( stderr, "Invalud argument: '%s'\n", pszArgvArray[i] );
      return EXIT_FAILURE;
      }
    }

  if ( bHelp)
    {
    Help( pszArgvArray[0] );
    return EXIT_FAILURE;
    }

  return EXIT_SUCCESS;
  }

/* End of file */
