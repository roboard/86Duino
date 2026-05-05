/*****************************************************************************
 *
 * File name   : main.c
 * Function    : Telnet server
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
 * 01-Jun-01, LVR, Created.
 * 15-Oct-01, LVR, v1.01 Add VRP_Open call.
 * 20-Feb-03, LVR, v2.00
 *
 *
 * Comments
 * --------
 *
 *****************************************************************************/
#define kszApp "Telnetd"
#define kszVer STRING( TELNETD_VER_MAJOR) "." STRING( TELNETD_VER_MINOR)
#define kszBanner kszApp " " kszVer " (" SWS_NET_ID ")" ", " TIMESTAMP "."
#define kszAbout "Telnet server."

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
#include <assert.h>
#include <time.h>

#include "sws_net.h"
#if SWS_USE_SOCKLIB && PACKET_DUMP
#include "sws_dump.h"
#endif
#include "telnetd.h"
#include "sws_cfg.h"


/***************************************************************************
 * Macros & constants
 ***************************************************************************/

/* Expand and stringize a macro name */
#define STR(s) #s
#define STRING(s) STR(s)

#ifdef __TIMESTAMP__
# define TIMESTAMP "edited " __TIMESTAMP__
#else
# define TIMESTAMP "built " __TIME__ " " __DATE__
#endif


/***************************************************************************
 * Data types
 ***************************************************************************/

/* Parameters */
typedef struct SConfig
  {
  int iVerbosity;
  FILE* logfile;
  unsigned short uPort;
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
    "-p    : Port, default " STRING( TELNETD_PORT) "\n"
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
  struct hostent* pHost;
  char szHost[256];
  struct sockaddr_in host;

  /* Set default options */
  cfg.iVerbosity = 1;
  cfg.logfile = stdout;
  cfg.uPort = 0;

  /* Parse args */
  iRet = ParseArgs( &cfg, argc, argv );
  if ( EXIT_SUCCESS != iRet)
    return iRet;

  if ( cfg.iVerbosity >= 0)
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

  /* Check network up */
  if ( gethostname( szHost, sizeof(szHost))
    || NULL == (pHost = gethostbyname( szHost))
  ) {
    fprintf( stderr, "No network (packet driver not found)\n");
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

  if ( cfg.iVerbosity >= 0)
    fputs( "CTRL-C to exit.\n\n", stderr);

  /* Run the telnet server */
  iRet = Telnetd(
    &host,
    cfg.iVerbosity,
    cfg.logfile,
    0, NULL /* command table */
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
          pCfg->uPort = (unsigned short)lVal;
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
