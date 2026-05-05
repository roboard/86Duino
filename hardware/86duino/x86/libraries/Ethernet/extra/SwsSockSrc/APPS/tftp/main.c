/*****************************************************************************
 *
 * File name   : main.c
 * Function    : TFTP command line front end
 * Authors     : Lawrence Rust
 * Systems     : ANSI C, BSD sockets
 *
 *****************************************************************************
 * File History
 * ------------
 *
 * 30-Aug-03, LVR, Created.
 *
 *
 * Comments
 * --------
 *
 *****************************************************************************/
#define kszApp "Tftp"
#define kszVer STRING( TFTP_VER_MAJOR) "." STRING( TFTP_VER_MINOR)
#define kszBanner kszApp " " kszVer " (" SWS_NET_ID ")" ", " TIMESTAMP "."
#define kszAbout "Tftp client."

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
#include <errno.h>
#include <signal.h>
#include <time.h>

/* platform */
#include "sws_net.h"
#if SWS_USE_SOCKLIB && PACKET_DUMP
#include "sws_dump.h"
#endif

#if SWS_USE_SOCKLIB
#include "sws_cfg.h"
#endif

/* App */
#include "tftp.h"


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

enum EAction { kNone, kPut, kGet };

/* Parameters */
typedef struct SConfig
  {
  int iVerbosity;
#if SWS_USE_SOCKLIB && PACKET_DUMP
  FILE* logfile;
#endif

  const char* pszServer;
  u_short uPort;

  enum EAction eAction;
  const char* pszMode;
  const char* pszSource;
  const char* pszDestination;
  } SConfig;


/***************************************************************************
 * Local function prototypes
 ***************************************************************************/

#ifdef SIGINT
static void OnSigInt( int);
#endif

static int ParseArgs( SConfig* const, int, const char* const [] );
static void Help( const char* const);
#if SWS_USE_SOCKLIB && PACKET_DUMP
static void LogInit( FILE*, int);
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
    "\nSyntax: %.*s [options] host <PUT | GET> source [destination]\n"
    "-i    : Set binary mode\n"
    "-p<n> : Set host port\n"
    "host  : The TFTP server (hostname or IP address)\n"
    "PUT   : Transfer a local source file to the remote destination\n"
    "GET   : Transfer a remote source file to the local destination\n"
#if SWS_USE_SOCKLIB && PACKET_DUMP
    "-o<s> : Set logfile\n"
#endif
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
  FILE* file;
  struct sockaddr_in host;

  /* Set default options */
  cfg.iVerbosity = 1;
#if SWS_USE_SOCKLIB && PACKET_DUMP
  cfg.logfile = stdout;
#endif
  cfg.pszServer = NULL;
  cfg.uPort = 0;
  cfg.eAction = kNone;
  cfg.pszMode = TFTP_TEXT;
  cfg.pszSource = NULL;
  cfg.pszDestination = NULL;

  /* Parse args */
  iRet = ParseArgs( &cfg, argc, argv );
  if ( EXIT_SUCCESS != iRet)
    return iRet;

  /* Check for required args */
  if ( NULL == cfg.pszServer)
    {
    Help( argv[0]);
    fprintf( stderr, "\nHost required\n");
    return EXIT_FAILURE;
    }

  if ( kNone == cfg.eAction)
    {
    Help( argv[0]);
    fprintf( stderr, "\nPUT or GET required\n");
    return EXIT_FAILURE;
    }

  if ( NULL == cfg.pszSource)
    {
    Help( argv[0]);
    fprintf( stderr, "\nSource required\n");
    return EXIT_FAILURE;
    }

  if ( NULL == cfg.pszDestination)
    cfg.pszDestination = cfg.pszSource;

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

  host.sin_family = AF_UNSPEC;
  host.sin_port = htons( cfg.uPort);
  host.sin_addr.s_addr = INADDR_NONE;

  /* Try an IP address */
  host.sin_addr.s_addr = inet_addr( cfg.pszServer);
  if ( INADDR_ANY == host.sin_addr.s_addr)
    {
    printf( "Invalid host %s\n", cfg.pszServer);
    return EXIT_FAILURE;
    }
  else if ( INADDR_NONE != host.sin_addr.s_addr)
    {
    host.sin_family = AF_INET;
    }
  /* Try a hostname */
  else if ( NULL != (pHost = gethostbyname( cfg.pszServer)) )
    {
    host.sin_family = pHost->h_addrtype;
    memcpy( &host.sin_addr, pHost->h_addr, pHost->h_length);
    }
  else
    {
    fprintf( stderr, "Unable to resolve %s\n", cfg.pszServer );
    return EXIT_FAILURE;
    }

  switch ( cfg.eAction)
    {
  case kPut:
    file = fopen( cfg.pszSource, strcmp( cfg.pszMode, TFTP_OCTET) ? "r" : "rb");
    if ( NULL == file)
      {
      fprintf( stderr, "Failed to open source : %s\n", strerror( errno));
      return EXIT_FAILURE;
      }

    iRet = TftpPut( (struct sockaddr*)&host, file, cfg.pszDestination, cfg.pszMode, cfg.iVerbosity);
    fclose( file);
    if ( 0 == iRet && cfg.iVerbosity > 0)
      fputs( "Ok\n", stdout);
    break;

  case kGet:
    file = fopen( cfg.pszDestination, strcmp( cfg.pszMode, TFTP_OCTET) ? "w" : "wb");
    if ( NULL == file)
      {
      fprintf( stderr, "Failed to create destination : %s\n", strerror( errno));
      return EXIT_FAILURE;
      }

    iRet = TftpGet( (struct sockaddr*)&host, cfg.pszSource, file, cfg.pszMode, cfg.iVerbosity);
    fclose( file);

    /* Remove dest on error */
    if ( 0 != iRet)
      remove( cfg.pszDestination);
    else if ( cfg.iVerbosity > 0)
      fputs( "Ok\n", stdout);
    break;

  default:
    fprintf( stderr, "Invalid action\n");
    return EXIT_FAILURE;
    }

#if SWS_USE_SOCKLIB && PACKET_DUMP
  if ( stdout != cfg.logfile)
    fclose( cfg.logfile);
#endif

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


#if SWS_USE_SOCKLIB && PACKET_DUMP
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
#endif


/*
 * Scan the command line for options
 */
static int ParseArgs(                 /* Returns !0 on error */
  SConfig* const pCfg,                /* OUT: Updated parameters */
  int iArgc,                          /* IN: No. args */
  const char* const pszArgvArray[]    /* IN: Array of arg strings */
) {
  int i;
  int bHelp = iArgc <= 1; /* Do help if no args */
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
        long lVal;
        char* pszEnd;

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
          /* Port */
          pszArg = GetArg();
          lVal = strtol( pszArg, &pszEnd, 0);
          if ( *pszEnd || lVal <= 0 || lVal > 65535L)
            {
            fputs( kszBanner "\n", stderr);
            fprintf( stderr, "Invalid port %s\n", pszArg);
            return EXIT_FAILURE;
            }

          pCfg->uPort = (u_short)lVal;
          pszArg = NULL;
          break;

        case 'i':
          /* binary mode */
          pCfg->pszMode = TFTP_OCTET;
          break;

#if SWS_USE_SOCKLIB && PACKET_DUMP
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
#endif

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
        /* host */
        pCfg->pszServer = pszArgvArray[i];
        ++iArgs;
        break;

      case 1:
        /* action */
        pszArg = pszArgvArray[i];
        ++iArgs;

        if ( !strcmp( "put", pszArg) || !strcmp( "PUT", pszArg))
          pCfg->eAction = kPut;
        else if ( !strcmp( "get", pszArg) || !strcmp( "GET", pszArg))
          pCfg->eAction = kGet;
        else
          {
          fputs( kszBanner "\n", stderr);
          printf( "Invalid action %s\n", pszArg);
          return EXIT_FAILURE;
          }
        break;

      case 2:
        /* Source */
        pCfg->pszSource = pszArgvArray[i];
        ++iArgs;
        break;

      case 3:
        /* Source */
        pCfg->pszDestination = pszArgvArray[i];
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

/* End of file */
