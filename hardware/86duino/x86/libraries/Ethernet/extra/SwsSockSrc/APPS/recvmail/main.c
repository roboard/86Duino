/*****************************************************************************
 *
 * File name   : main.c
 * Function    : Front end for POP3 mail client
 * Authors     : Lawrence Rust
 * Systems     : ANSI C
 * Release Date: 31-Oct-02
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
#define kszApp "Recvmail"
#define kszVer STRING( RECVMAIL_VER_MAJOR) "." STRING( RECVMAIL_VER_MINOR)
#define kszBanner kszApp " " kszVer " (" SWS_NET_ID ")" ", " TIMESTAMP "."
#define kszAbout "Pop3 mail client."

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
#include <errno.h>
#include <signal.h>
#include <time.h>

#include "sws_net.h"
#if SWS_USE_SOCKLIB && PACKET_DUMP
#include "sws_dump.h"
#endif

#if SWS_USE_SOCKLIB
#include "sws_cfg.h"
#endif
#include "recvmail.h"


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
  int isScript;

  const char* pszServer;
  u_short uPort;
  } SConfig;


/***************************************************************************
 * Local function prototypes
 ***************************************************************************/

static int ParseArgs( SConfig* const, int const, const char* const []);
static void Help( const char* const);
static void LogInit( FILE*, int);

#ifdef SIGINT
static void OnSigInt( int);
#endif


/***************************************************************************
 * Module data
 ***************************************************************************/
sig_atomic_t volatile g_exit;


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
    "\nSyntax: %.*s [options] <host>\n"
    "-p<n> : Port\n"
    "-i<s> : Specify text file containing commands\n"
    "-o<s> : Set logfile\n"
    "-t    : Decrease verbosity of output\n"
    "-v    : Increase verbosity of output\n"
    "-V    : Show version and exit\n"
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
  char szHost[ 256];
  struct sockaddr_in server;

  /* Set default options */
  cfg.iVerbosity = 1;
  cfg.logfile = NULL;
  cfg.isScript = 0;
  cfg.pszServer = NULL;
  cfg.uPort = 0;

  /* Parse args */
  iRet = ParseArgs( &cfg, argc, argv );
  if ( EXIT_SUCCESS != iRet)
    return iRet;

  /* Verify required arguments */
  if ( NULL == cfg.pszServer)
    {
    Help( argv[0]);
    fprintf( stderr, "\nHost required\n");
    return EXIT_FAILURE;
    }

#if SWS_USE_SOCKLIB
  /* Specify the configuration file */
  if ( !SWS_CfgSetName( NULL, argv[0] ))
    fprintf( stderr, "WARNING: configuration file '" SWS_CFGFILE "' not found\n");
#endif

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

  /* Check network is up */
  if ( gethostname( szHost, sizeof(szHost))
    || NULL == (pHost = gethostbyname( szHost))
  ) {
    fprintf( stderr, "No network (driver not found)\n");
    return EXIT_FAILURE;
    }

  server.sin_family = AF_UNSPEC;
  server.sin_port = htons( cfg.uPort);
  server.sin_addr.s_addr = inet_addr( cfg.pszServer );
  if ( INADDR_ANY == server.sin_addr.s_addr)
    {
    printf( "Invalid host %s\n", cfg.pszServer );
    return EXIT_FAILURE;
    }
  else if ( INADDR_NONE != server.sin_addr.s_addr)
    {
    server.sin_family = AF_INET;
    }
  /* Try a hostname */
  else if ( NULL != (pHost = gethostbyname( cfg.pszServer )) )
    {
    server.sin_family = pHost->h_addrtype;
    memcpy( &server.sin_addr, pHost->h_addr, pHost->h_length);
    }
  else
    {
    printf( "Unable to resolve %s\n", cfg.pszServer );
    return EXIT_FAILURE;
    }

  /* POP3 mail client */
  iRet = RecvMail( cfg.isScript, &server, cfg.logfile, cfg.iVerbosity);

  if ( NULL != cfg.logfile && stdout != cfg.logfile)
    fclose( cfg.logfile);

  return iRet ? EXIT_FAILURE : EXIT_SUCCESS;
  }


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
  int const iArgc,                    /* IN: No. args */
  const char* const pszArgvArray []    /* IN: Array of arg strings */
) {
  int i;
  int bHelp = 0;
  int iArgs = 0;

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

          pCfg->uPort = (u_short)lVal;
          pszArg = NULL;
          break;

        case 'i':
          /* Input script */
          pszArg = GetArg();
          if ( *pszArg
            && strcmp( "-", pszArg)
            && NULL == freopen( pszArg, "r", stdin)
          ) {
            fputs( kszBanner "\n", stderr);
            fprintf( stderr, "Failed to open '%s': %s\n", pszArg, strerror( errno));
            return EXIT_FAILURE;
            }
          pCfg->isScript = 1;
          pszArg = NULL;
          break;

        case 'o':
          /* logfile */
          if ( NULL != pCfg->logfile)
            {
            fputs( kszBanner "\n", stderr);
            fprintf( stderr, "Duplicate logfile '%s'\n", pszArg);
            return EXIT_FAILURE;
            }

          pszArg = GetArg();
          if ( 0 == strcmp( "", pszArg) || 0 == strcmp( "-", pszArg))
            pCfg->logfile = stdout;
          else if ( NULL == (pCfg->logfile = fopen( pszArg, "a")))
            {
            fputs( kszBanner "\n", stderr);
            fprintf( stderr, "Failed to open logfile '%s': %s\n", pszArg, strerror( errno));
            return EXIT_FAILURE;
            }
          else
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
      switch ( iArgs)
        {
      case 0:
        /* Host */
        pCfg->pszServer = pszArgvArray[i];
        ++iArgs;
        break;

      default:
        fputs( kszBanner "\n", stderr);
        fprintf( stderr, "Excess argument: '%s'\n", pszArgvArray[i] );
        return EXIT_FAILURE;
        }
      }
    }

  if ( bHelp)
    {
    Help( pszArgvArray[0] );
    return EXIT_FAILURE;
    }

  return EXIT_SUCCESS;
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

/* End of file */
