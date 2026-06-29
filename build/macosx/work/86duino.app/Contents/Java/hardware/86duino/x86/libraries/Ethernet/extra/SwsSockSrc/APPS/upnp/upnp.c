/*****************************************************************************
 *
 * Project     : upnp
 * Function    : Universal Plug'n'Play (www.upnp.org) tool
 * Authors     : Lawrence Rust
 * Systems     : ANSI C, sockets
 *
 *****************************************************************************
 * File History
 * ------------
 *
 * 30-Aug-09, LVR, v1.00 Created.
 *
 *****************************************************************************/
#define kszApp "UPnP"
#define kszVer "1.00"
#define kszBanner kszApp " " kszVer " (" SWS_NET_ID ")" ", " TIMESTAMP "."
#define kszAbout ": Universal Plug'n'Play tool."

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
#include <signal.h>
#include <time.h>
#if _MSC_VER >= 1000
#include <crtdbg.h>
#endif

#include "sws_net.h"
#include "sws_upnp.h"
#if SWS_USE_SOCKLIB
#include "sws_cfg.h"
#endif
#if SWS_USE_SOCKLIB && PACKET_DUMP
#include "sws_dump.h"
#endif


/***************************************************************************
 * Macros & constants
 ***************************************************************************/
#ifdef __TIMESTAMP__
# define TIMESTAMP __TIMESTAMP__
#else
# define TIMESTAMP "built " __TIME__ " " __DATE__
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


/***************************************************************************
 * Data types
 ***************************************************************************/

enum EOpt
  {
  kOptList,
  kOptAdd,
  kOptDel,
  kOptConn,
  kOptExtIP,
  kOptLink
  };

/* Parameters */
typedef struct SConfig
  {
  enum EOpt eOpt;
#define DEF_OPT kOptList
  struct SWS_UpnpPortMap map;

#if SWS_USE_SOCKLIB && PACKET_DUMP
  int iVerbosity;
  FILE* logfile;
#endif
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
    "\nSyntax: %.*s [options] [args]\n"
    "-a    : Add IGD port mapping: TCP|UDP extport [clientIP [inport]]\n"
    "-d    : Delete IGD port mapping: TCP|UDP extport\n"
    "-l    : List IGD port mappings (default)\n"
    "-c    : Display IGD connection status\n"
    "-k    : Display IGD link status\n"
    "-x    : Display IGD external IP address\n"
#if SWS_USE_SOCKLIB && PACKET_DUMP
    "-o<s> : Set logfile\n"
    "-v    : Increase verbosity of output\n"
    "-t    : Decrease verbosity of output\n"
#endif
    "-V    : Show version and exit\n"
    "-h    : Help\n",
    (int)c, pszCmd
  );
  }


/*
 * M A I N
 */
int main( int argc, const char* argv[] )
  {
  SConfig cfg;
  int iRet;
  struct hostent* pHost;
  char szHost[256];

  /* Set default options */
  cfg.eOpt = DEF_OPT;
  memset( &cfg.map, 0, sizeof cfg.map);
#if SWS_USE_SOCKLIB && PACKET_DUMP
  cfg.iVerbosity = 1;
  cfg.logfile = stdout;
#endif

  /* Parse args */
  iRet = ParseArgs( &cfg, argc, argv );
  if ( EXIT_SUCCESS != iRet)
    return iRet;

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

  /* Check the network's up */
  if ( gethostname( szHost, sizeof szHost)
    || NULL == (pHost = gethostbyname( szHost))
  ) {
    fprintf( stderr, "ERROR: no network (driver not found)\n");
    return EXIT_FAILURE;
    }

  switch ( cfg.eOpt)
    {
    struct SWS_UpnpPortMap map;
    struct SWS_UpnpConnStatus conn;
    struct SWS_UpnpLinkStatus lnk;
    unsigned u;

  case kOptList:
    /* List IGD port mappings */
    for ( u = 0; 0 == (iRet = SWS_UpnpGetPortMap( u, &map)); ++u)
      {
      printf( "%s %s %s:%u => %s:%u",
        map.szInfo, map.szProto,
        map.szRemote[0] ? map.szRemote : "*", map.uExtern,
        map.szClient, map.uIntern
      );
      if ( map.uLease)
        printf( " %u secs", map.uLease);

      if ( !map.uEnabled)
        fputs( " DISABLED", stdout);

      fputs( "\n", stdout);
      }
    if ( 0 == u)
      {
      if ( 500 != iRet)
        fprintf( stderr, "UPnP error %d\n", iRet);
      }
    break;

  case kOptAdd:
    /* Add an IGD port mapping */
    if ( '\0' == cfg.map.szProto[0])
      {
      fputs( "Required args missing\n", stderr);
      return EXIT_FAILURE;
      }
    iRet = SWS_UpnpAddPortMap(
      cfg.map.szProto,
      cfg.map.uIntern ? cfg.map.uIntern : cfg.map.uExtern,
      '\0' != cfg.map.szClient[0] ? cfg.map.szClient :
        inet_ntoa( *(const struct in_addr*)pHost->h_addr),
      cfg.map.uExtern,
      kszApp "_" kszVer
    );
    if ( iRet)
      fprintf( stderr, "UPnP error %d\n", iRet);
    break;

  case kOptDel:
    /* Delete an IGD port mapping */
    if ( '\0' == cfg.map.szProto[0])
      {
      fputs( "Required args missing\n", stderr);
      return EXIT_FAILURE;
      }
    iRet = SWS_UpnpDelPortMap(
      cfg.map.szProto,
      cfg.map.uExtern
    );
    if ( iRet)
      fprintf( stderr, "UPnP error %d\n", iRet);
    break;

  case kOptExtIP:
    /* Get IGD external IP address */
    iRet = SWS_UpnpGetExtIP( szHost);
    if ( !iRet)
      printf( "%s\n", szHost);
    else
      perror( "UPnP error");
    break;

  case kOptConn:
    /* Get IGD connection status */
    iRet = SWS_UpnpGetConnStatus( &conn);
    if ( !iRet)
      {
      unsigned d, h, m, s;

      d = (unsigned)(conn.ulUptime / (24 * 3600UL));
      conn.ulUptime -= d * (24 * 3600UL);
      h = (unsigned)(conn.ulUptime / 3600);
      conn.ulUptime -= h * 3600;
      m = (unsigned)(conn.ulUptime / 60);
      s = (unsigned)(conn.ulUptime - m * 60);
      printf( "%s(%s) %u days %u hrs %u mins %u secs\n",
        conn.szStatus, conn.szError, d, h, m, s);
      }
    else
      perror( "UPnP error");
    break;

  case kOptLink:
    /* Get IGD link status */
    iRet = SWS_UpnpGetLinkStatus( &lnk);
    if ( !iRet)
      {
      printf( "Link type: %s(%s), %lu bps up, %lu bps down\n"
        "Stats: %lu bytes up, %lu bytes down, %lu packets up, %lu packets down\n",
        lnk.szType, lnk.szStatus,
        lnk.ulUp, lnk.ulDown,
        lnk.ulBytesSent, lnk.ulBytesRecvd,
        lnk.ulPacketsSent, lnk.ulPacketsRecvd
      );
      }
    else
      perror( "UPnP error");
    break;
    }

  return iRet ? EXIT_FAILURE : EXIT_SUCCESS;
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
  int bHelp = 0; /* iArgc <= 1;  Do help if no args */
  int iArgs = 0;

  /* Scan args */
  for ( i = 1; i < iArgc; ++i)
    {
    char c;
    const char* pszArg;
    unsigned long ulVal;
    char* pszEnd;

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

        case 'a':
          /* Add mapping */
          pCfg->eOpt = kOptAdd;
          break;

        case 'd':
          /* Delete mapping */
          pCfg->eOpt = kOptDel;
          break;

        case 'l':
          /* List */
          pCfg->eOpt = kOptList;
          break;

        case 'c':
          /* Connection status */
          pCfg->eOpt = kOptConn;
          break;

        case 'k':
          /* Link status */
          pCfg->eOpt = kOptLink;
          break;

        case 'x':
          /* External IP */
          pCfg->eOpt = kOptExtIP;
          break;

#if SWS_USE_SOCKLIB && PACKET_DUMP
        case 't':
          /* terse */
          pCfg->iVerbosity--;
          break;

        case 'v':
          /* verbose */
          pCfg->iVerbosity++;
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
#endif

        default:
          fprintf( stderr, "Invalid switch: '%s'\n", pszArgvArray[i] );
          return EXIT_FAILURE;
          }
        }
      while ( NULL != pszArg && '\0' != *pszArg);
      }
    /* Non-switch argument */
    else if ( kOptAdd == pCfg->eOpt) switch ( iArgs++)
      {
    case 0:
      strncpy( pCfg->map.szProto, pszArgvArray[i], sizeof pCfg->map.szProto);
      break;

    case 1:
      ulVal = strtoul( pszArgvArray[i], &pszEnd, 0);
      if ( *pszEnd || ulVal > 65535L)
        {
        fprintf( stderr, "Invalid port %s\n", pszArgvArray[i]);
        return EXIT_FAILURE;
        }
      pCfg->map.uExtern = (unsigned)ulVal;
      break;

    case 2:
      strncpy( pCfg->map.szClient, pszArgvArray[i], sizeof pCfg->map.szClient);
      break;

    case 3:
      ulVal = strtoul( pszArgvArray[i], &pszEnd, 0);
      if ( *pszEnd || ulVal < 1 || ulVal > 65535L)
        {
        fprintf( stderr, "Invalid port %s\n", pszArgvArray[i]);
        return EXIT_FAILURE;
        }
      pCfg->map.uIntern = (unsigned)ulVal;
      break;

    default:
      fprintf( stderr, "Excess argument: '%s'\n", pszArgvArray[i] );
      return EXIT_FAILURE;
      }
    /* Non-switch argument */
    else if ( kOptDel == pCfg->eOpt) switch ( iArgs++)
      {
    case 0:
      strncpy( pCfg->map.szProto, pszArgvArray[i], sizeof pCfg->map.szProto);
      break;

    case 1:
      ulVal = strtoul( pszArgvArray[i], &pszEnd, 0);
      if ( *pszEnd || ulVal > 65535L)
        {
        fprintf( stderr, "Invalid port %s\n", pszArgvArray[i]);
        return EXIT_FAILURE;
        }
      pCfg->map.uExtern = (unsigned)ulVal;
      break;

    default:
      fprintf( stderr, "Excess argument: '%s'\n", pszArgvArray[i] );
      return EXIT_FAILURE;
      }
    /* Non-switch argument */
    else
      {
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
