/*****************************************************************************
 *
 * File name   : main.c
 * Function    : http server command line interface
 * Authors     : Lawrence Rust
 * Systems     : ANSI C, sockets
 *
 *****************************************************************************
 * File History
 * ------------
 *
 * 05-Jun-01, LVR, Created.
 * 15-Oct-01, LVR, v1.01 Add VRP_Open call.
 * 07-Mar-03, LVR, v2.00 Rewrite.
 *
 *****************************************************************************/
#define kszApp "Httpd"
#define kszVer STRING( HTTPD_VER_MAJOR) "." STRING( HTTPD_VER_MINOR)
#define kszBanner kszApp " " kszVer " (" SWS_NET_ID ")" ", " TIMESTAMP "."
#define kszAbout "HTTP service."

/* Compilation options */
#ifndef SWS_USE_SOCKLIB
#define SWS_USE_SOCKLIB 0 /* 0= native sockets, !0 for sws_sock */
#endif

#ifndef PACKET_DUMP
#define PACKET_DUMP 0
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

#if defined __GNUC__ && defined MTRACE
#include <mcheck.h>
#endif

#include "sws_net.h"
#if SWS_USE_SOCKLIB && PACKET_DUMP
#include "sws_dump.h"
#endif

#include "sws_cfg.h"
#include "httpd.h"
#include "httpfile.h"
#if OPT_HTTP_CAMERA
# include "httplfg.h"
#elif OPT_HTTP_V4L
# include "httpv4l.h"
#endif
#if OPT_HTTP_PROXY
# include "httpprxy.h"
#endif
#if OPT_HTTP_CGI
# include "httpcgi.h"
#endif

#if defined _MSC_VER && _MSC_VER <= 800 /* VC1.5 */
#pragma warning ( disable : 4705) /* statement has no effect: (void) cast */
#endif


/***************************************************************************
 * Macros & constants
 ***************************************************************************/

#ifdef __TIMESTAMP__
# define TIMESTAMP "edited " __TIMESTAMP__
#else
# define TIMESTAMP "built " __TIME__ " " __DATE__
#endif

/* Expand and stringize a macro name */
#define STR(s) #s
#define STRING(s) STR(s)


/***************************************************************************
 * Data types
 ***************************************************************************/

/* Parameters */
typedef struct SConfig
  {
  int iVerbosity;
  FILE* logfile;
  u_short uPort;
  } SConfig;


/***************************************************************************
 * Function prototypes
 ***************************************************************************/

#ifdef SIGINT
static void OnSigInt( int);
#endif

static int ParseArgs( SConfig* const, int, const char* const [] );
static void Help( const char* const);
static void LogInit( FILE*, int);


/***************************************************************************
 * Module data
 ***************************************************************************/

sig_atomic_t volatile g_exit;
sig_atomic_t volatile g_running;


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

  fprintf( stderr,
    "\nSyntax: %.*s [options]\n"
    "-p<n> : Port\n"
    "-o<s> : Set logfile\n"
    "-t    : Decrease verbosity of output\n"
    "-v    : Increase verbosity of output\n"
    "-h    : Help\n",
    (int)c, pszCmd
  );
  }


/*
 * M A I N
 */
int main( int const argc, const char* const argv[] )
  {
  SConfig cfg;
  int iRet;
  SWS_PTR( struct hostent) pHost;
  char szHost[ 256];
  struct sockaddr_in host;

#if defined __GNUC__ && defined MTRACE
  /* Enable leak tracking iff MALLOC_TRACE is set to a valid filename
   * Execute: 'mtrace prog logfile' to view report */
  mtrace();
#endif

  /* Set default options */
  cfg.iVerbosity = 1;
  cfg.logfile = stdout;
  cfg.uPort = 0;

  /* Parse args */
  iRet = ParseArgs( &cfg, argc, argv );
  if ( EXIT_SUCCESS != iRet)
    return iRet;

  if ( cfg.iVerbosity >= -1)
    fputs( kszBanner "\n", stderr);

  /* Specify the configuration file */
  if ( !SWS_CfgSetName( NULL, argv[0] ))
    fprintf( stderr, "WARNING: configuration file '" SWS_CFGFILE "' not found\n");

#ifdef SIGINT
  /* Register a handler for ctrl-c */
  signal( SIGINT, &OnSigInt);
#endif

  /* Initialise the sockets library */
#if PACKET_DUMP
# define GEF( _f, _v) SWS_PKTDUMP( _f, _v)
#else
# define GEF( _f, _v) 0
#endif
  SWS_SockUp( GEF( cfg.logfile, cfg.iVerbosity), NULL);

  host.sin_family = AF_INET;
  host.sin_port = htons( cfg.uPort);
  host.sin_addr.s_addr = INADDR_ANY;

  /* Get local IP address */
  if ( gethostname( szHost, sizeof(szHost))
    || NULL == (pHost = gethostbyname( szHost))
  ) {
    fprintf( stderr, "No network (driver not found)\n");
    return EXIT_FAILURE;
    }

  if ( cfg.iVerbosity >= 0)
    {
    int i;

    fprintf( stderr, "This host is %s", szHost);

    /* Display the list of local IP addresses */
    for ( i = 0; NULL != pHost->h_addr_list[i]; ++i)
      {
      fprintf( stderr, " [%s]",
        inet_ntoa( *(const struct in_addr*)pHost->h_addr_list[i])
      );
      }
    fputs( "\n", stderr);
    }

  /* Initialise optional modules */
  if ( !HttpFileInit())
    fputs( "Missing or invalid root directory\n", stderr);

#if OPT_HTTP_CAMERA
  if ( !HttpLfgInit())
    fputs( "*** LFG webcam module startup failure\n", stderr);
  else if ( cfg.iVerbosity >= 0)
    fputs( "LFG webcam module installed\n", stderr);
#endif

#if OPT_HTTP_V4L
  if ( !HttpV4lInit())
    fputs( "*** Video4Linux webcam module startup failure\n", stderr);
  else if ( cfg.iVerbosity >= 0)
    fputs( "Video4Linux webcam module installed\n", stderr);
#endif

#if OPT_HTTP_PROXY
  if ( !HttpProxyInit())
    fputs( "Http proxy startup failure\n", stderr);
#endif

#if OPT_HTTP_CGI
  HttpCgiInit();
#endif

  if ( cfg.iVerbosity >= 0)
    fputs( "CTRL-C to exit.\n\n", stderr);

  /* Run the web server */
  iRet = Httpd(
    &host,
    cfg.iVerbosity,
    cfg.logfile
  );

  if ( stdout != cfg.logfile)
    fclose( cfg.logfile);

  return iRet ? EXIT_FAILURE : EXIT_SUCCESS;
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

    pszArg = pszArgvArray[i];

    /* Check for switch options ('-' or '/' prefix) */
#   define isswitch( _c) ('-' == _c)
    c = *pszArg++;
    if ( isswitch( c) )
      {
      /* Option found */
      do
        {
        char* pszEnd;
        long lVal;

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
          pCfg->iVerbosity--;
          break;

        case 'v':
          /* verbose */
          pCfg->iVerbosity++;
          break;

        case 'p':
          /* port */
          pszArg = GetArg();
          lVal = strtol( pszArg, &pszEnd, 10);
          if ( *pszEnd || lVal < 0 || lVal > 65535L)
            {
            fputs( kszBanner "\n", stderr);
            fprintf( stderr, "Invalid port %s\n", pszArg);
            return EXIT_FAILURE;
            }

          pszArg = NULL;
          pCfg->uPort = (u_short)lVal;
          break;

        case 'o':
          /* logfile */
          pszArg = GetArg();
          if ( '\0' == *pszArg)
            {
            fputs( kszBanner "\n", stderr);
            fputs( "Missing logfile name\n", stderr);
            return EXIT_FAILURE;
            }
          else if ( stdout != pCfg->logfile)
            {
            fputs( kszBanner "\n", stderr);
            fputs( "Duplicate logfile\n", stderr);
            return EXIT_FAILURE;
            }
          else if ( NULL == (pCfg->logfile = fopen( pszArg, "a")))
            {
            fputs( kszBanner "\n", stderr);
            fprintf( stderr, "Failed to open logfile '%s'\n", pszArg);
            return EXIT_FAILURE;
            }

          LogInit( pCfg->logfile, pCfg->iVerbosity);
          pszArg = NULL;
          break;

        default:
          fputs( kszBanner "\n", stderr);
          fprintf( stderr, "Invalid switch: '%s'\n", pszArgvArray[i] );
          return EXIT_FAILURE;
          }
        }
      while ( NULL != pszArg && '\0' != *pszArg);
      }
    else
      {
      /* Non-switch argument */
      fputs( kszBanner "\n", stderr);
      fprintf( stderr, "Excess argument: '%s'\n", pszArgvArray[i] );
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
