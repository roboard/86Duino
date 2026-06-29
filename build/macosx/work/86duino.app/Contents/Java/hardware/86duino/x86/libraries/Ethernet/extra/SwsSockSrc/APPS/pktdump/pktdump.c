/*****************************************************************************
 *
 * File name   : pktdump.c
 * Function    : TCP/IP & Ethernet Packet dumper
 * Authors     : Lawrence Rust
 * Systems     : ANSI C, SwsSock
 * Version     : 1.0
 * Release Date: 31-Oct-02
 *
 *****************************************************************************
 * File History
 * ------------
 *
 * 29-Oct-02, LVR, Created.
 * 23-May-05, LVR, SwsSock only.
 * 30-Jun-05, LVR, v1.02 Show adapter names.
 *
 *
 * Comments
 * --------
 *
 *****************************************************************************/
#define kszApp "PKTDUMP"
#define kszVer "1.02"
#define kszBanner kszApp " " kszVer " (" SWS_NET_ID ")" ", " TIMESTAMP "."
#define kszAbout "TCP/IP, Ethernet & P2P packet dumper."

/* Compilation options */

/* Exports */

/* Imports */

/* ANSI C library */
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <time.h>

/* SwsSock */
#define SWS_USE_SOCKLIB 1
#include "sws_net.h"
#include "sws_dump.h"
#include "sws_dlnk.h"   /* SWS_DatalinkSetProperty */
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


/***************************************************************************
 * Data types
 ***************************************************************************/

/* Parameters */
typedef struct SConfig
  {
  int iVerbosity;
  FILE* logfile;
  SWS_u_long ipaddr;
  int isPromiscuous;
  } SConfig;


/***************************************************************************
 * Local function prototypes
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

static sig_atomic_t volatile s_exit;


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
    "-p[0|1] : En/dis/able promiscuous receive\n"
    "-i<ip>  : Set IP address\n"
    "-o file : Set logfile\n"
    "-t      : Decrease verbosity of output\n"
    "-v      : Increase verbosity of output\n"
    "-V      : Display version and exit\n"
    "-h      : Help\n",
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
  unsigned link, count, adapters;

  /* Set default options */
  cfg.iVerbosity = 1;
  cfg.logfile = stdout;
  cfg.ipaddr = SWS_INADDR_NONE;
  cfg.isPromiscuous = 1;

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
#define GEF( _f, _v)\
  (SWS_DumpSetup(\
    (_v) >= 0 ? (_v) +1 : 1,\
    stdout,\
    stdout == (_f) ? NULL : (_f)\
  ), &SWS_DumpPacket)
  SWS_SockUp( GEF( cfg.logfile, cfg.iVerbosity), NULL);

  /* Enumerate datalinks */
  adapters = 0;
  count = SWS_DatalinkCount();
  for ( link = 0; link < count; ++link)
    {
    ui32 uVal;
    size_t size;

    /* Get IP address for datalink */
    size = sizeof( uVal);
    iRet = SWS_DatalinkGetProperty(
      link,
      SWS_kDatalinkProperty_Address,
      &uVal,
      &size
    );
    if ( iRet)
      continue;
    if ( SWS_INADDR_NONE == uVal)
      continue; /* Invalid */
    if ( SWS_INADDR_LOOPBACK == uVal)
      continue;

    ++adapters;

    if ( cfg.iVerbosity >= 0)
      {
      char szBuf[ 48];

      /* Get adapter name */
      size = sizeof szBuf;
      iRet = SWS_DatalinkGetProperty(
        link,
        SWS_kDatalinkProperty_Ident,
        szBuf,
        &size
      );
      if ( 0 == iRet)
        fprintf( stderr, "%s ", szBuf);
      }

    if ( SWS_INADDR_NONE != cfg.ipaddr)
      {
      /* Set the IP address */
      iRet = SWS_DatalinkSetProperty(
        link,
        SWS_kDatalinkProperty_Address,
        &cfg.ipaddr,
        sizeof( cfg.ipaddr)
      );
      if ( cfg.iVerbosity >= 0 && 0 == iRet)
        {
        struct SWS_in_addr addr;
        addr.s_addr = SWS_htonl( cfg.ipaddr);
        fprintf( stderr, "[%s] ", inet_ntoa( addr));
        }

      cfg.ipaddr++;

      /* Set the default subnet mask */
      uVal = 0;
      SWS_DatalinkSetProperty(
        link,
        SWS_kDatalinkProperty_SubnetMask,
        &uVal,
        sizeof( uVal)
      );

      /* Set the default gateway */
      uVal = SWS_INADDR_ANY;
      SWS_DatalinkSetProperty(
        link,
        SWS_kDatalinkProperty_Gateway,
        &uVal,
        sizeof( uVal)
      );
      }
    /* Is the IP address valid? */
    else /*if ( SWS_INADDR_ANY == uVal)*/
      {
      /* Reset the IP address */
      uVal = SWS_INADDR_ANY;
      SWS_DatalinkSetProperty(
        link,
        SWS_kDatalinkProperty_Address,
        &uVal,
        sizeof( uVal)
      );

      /* Disable IP address gleaning */
      uVal = 0x80000000L;
      SWS_DatalinkSetProperty(
        link,
        SWS_kDatalinkProperty_SubnetMask,
        &uVal,
        sizeof( uVal)
      );
      }

    if ( cfg.isPromiscuous)
      {
      /* Enable promiscuous mode */
      uVal = 1;
      SWS_DatalinkSetProperty(
        link,
        SWS_kDatalinkProperty_Promiscuity,
        &uVal,
        sizeof( uVal)
      );
      }
    }

  if ( 0 == adapters)
    {
    fputs( "No network (driver not found)\n", stderr);
    return EXIT_FAILURE;
    }

  if ( cfg.iVerbosity >= 0)
    {
    if ( cfg.isPromiscuous)
      fputs( "promiscuous mode.\n", stderr);
    else
      fputs( "\n", stderr);
    fputs( "CTRL-C to exit...\n", stderr);
    }

  while ( !s_exit)
    {
    SWS_SockSleep( 300);
    }

  if ( stdout != cfg.logfile)
    fclose( cfg.logfile);

#ifdef __DJGPP__
  if ( cfg.isPromiscuous)
    fputs( "\nNB the packet driver may need to be restarted.\n", stderr);
#endif

  return 0;
  }


#ifdef SIGINT
/*
 * SIGINT handler (ctrl-c)
 */
static void OnSigInt( int sig)
  {
  if ( ++s_exit >= 2)
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
        SWS_u_long ulVal;

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

        case 'i':
          /* Set IP address */
          pszArg = GetArg();
          ulVal = SWS_inet_addr( pszArg);
          if ( SWS_INADDR_NONE != ulVal && SWS_INADDR_ANY != ulVal)
            {
            pCfg->ipaddr = SWS_ntohl( ulVal);
            pCfg->isPromiscuous = 0;
            }
          else
            {
            fprintf( stderr, "Invalid IP address %s\n", pszArg);
            return EXIT_FAILURE;
            }

          pszArg = NULL;
          break;

        case 'p':
          /* Promiscuity */
          switch ( *pszArg)
            {
          case '\0':
            pCfg->isPromiscuous = 1;
            break;
          case '1':
            pCfg->isPromiscuous = 1;
            pszArg = NULL;
            break;
          case '0':
            pCfg->isPromiscuous = 0;
            pszArg = NULL;
            break;
          default:
            fprintf( stderr, "Invalid mode %s\n", pszArg);
            return EXIT_FAILURE;
            break;
            }
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
          fprintf( stderr, "Invalid switch: '%s'\n", pszArgvArray[i] );
          return EXIT_FAILURE;
          }
        }
      while ( NULL != pszArg && '\0' != *pszArg);
      }
    else
      {
      /* Non-switch argument */
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
