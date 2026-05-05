/*****************************************************************************
 *
 * File name   : whois.c
 * Function    : WHOIS/NICNAME query
 * Authors     : Lawrence Rust
 * Systems     : ANSI C, BSD sockets
 * Version     : 1.0
 *
 *****************************************************************************
 * File History
 * ------------
 *
 * 02-Mar-03, LVR, Created.
 *
 *
 * Comments
 * --------
 *
 *****************************************************************************/
#define kszApp "Whois"
#define kszVer "1.00"
#define kszBanner kszApp " " kszVer " (" SWS_NET_ID ")" ", " TIMESTAMP "."
#define kszAbout "Whois/nicname client."

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

/* platform */
#include "sws_net.h"
#if SWS_USE_SOCKLIB && PACKET_DUMP
#include "sws_dump.h"
#endif

/* App */
#include "whois.h"
#include "sws_cfg.h"


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

#define kszIniFile "whois.ini"


/***************************************************************************
 * Data types
 ***************************************************************************/

/* Parameters */
typedef struct SConfig
  {
  int iVerbosity;
  const char* pszServer;
  u_short uPort;
  char* pszQuery;
  } SConfig;


/***************************************************************************
 * Local function prototypes
 ***************************************************************************/

#ifdef SIGINT
static void OnSigInt( int);
#endif

static int ParseArgs( SConfig* const, int, const char* const [] );
static void Help( const char* const);


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
    "\nSyntax: %.*s [options] {hostname|IP}\n"
    "-n{s} : Set server (hostname|IP)\n"
    "-p{n} : Set server port, default " STRING( WHOIS_SERVER) "\n"
#if SWS_USE_SOCKLIB && PACKET_DUMP
    "-o{s} : Set logfile\n"
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
  struct sockaddr_in host;

  /* Set default options */
  cfg.iVerbosity = 1;
  cfg.pszServer = NULL;
  cfg.uPort = 0;
  cfg.pszQuery = NULL;

  /* Parse args */
  iRet = ParseArgs( &cfg, argc, argv );
  if ( EXIT_SUCCESS != iRet)
    return iRet;

  if ( NULL == cfg.pszQuery)
    {
    fputs( "Query required\n", stderr);
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
  SWS_SockUp( GEF( stdout, cfg.iVerbosity), NULL);

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

  if ( NULL != cfg.pszServer)
    {
    /* Try an IP address */
    host.sin_addr.s_addr = inet_addr( cfg.pszServer);
    if ( INADDR_ANY == host.sin_addr.s_addr)
      {
      printf( "Invalid host %s\n", cfg.pszServer );
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
    }

  /* Select the whois configuration file */
  if ( !SWS_CfgSetName( kszIniFile, argv[0] ))
    fprintf( stderr, "WARNING: configuration file '" kszIniFile "' not found\n");

  /* Query the whois server */
  iRet = WhoisQuery(
    stdout,
    cfg.pszQuery,
    cfg.iVerbosity,
    (struct sockaddr*)&host
  );
  if ( 0 != iRet)
    fprintf( stderr, "FAILED (%d): %s\n", iRet, strerror( iRet));

  if ( NULL != cfg.pszQuery)
    free( cfg.pszQuery);

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
            fprintf( stderr, "Invalid port %s\n", pszArg);
            return EXIT_FAILURE;
            }

          pCfg->uPort = (u_short)lVal;
          pszArg = NULL;
          break;

        case 'n':
          /* server */
          pCfg->pszServer = GetArg();
          pszArg = NULL;
          break;

#if SWS_USE_SOCKLIB && PACKET_DUMP
        case 'o':
          /* logfile */
          if ( NULL == freopen( pszArg, "a", stdout))
            {
            fprintf( stderr, "Failed to open logfile '%s'\n", pszArg);
            return EXIT_FAILURE;
            }

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
    else
      {
      /* Expand query string buffer */
      size_t size = NULL == pCfg->pszQuery ? 0 : strlen( pCfg->pszQuery);
      pCfg->pszQuery = (char*) realloc( pCfg->pszQuery, size + 2 + strlen( pszArgvArray[i]));
      if ( NULL != pCfg->pszQuery)
        {
        /* Append arg to query */
        if ( size > 0)
          pCfg->pszQuery[ size++] = ' ';
        strcpy( pCfg->pszQuery + size, pszArgvArray[i]);
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
