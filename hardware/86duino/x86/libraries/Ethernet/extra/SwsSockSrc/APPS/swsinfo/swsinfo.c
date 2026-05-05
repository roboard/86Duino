/*****************************************************************************
 *
 * File name   : swsinfo.c
 * Function    : Display SwsSock configration
 * Authors     : Lawrence Rust
 * Systems     : ANSI C, SwsSock
 * Version     : 1.0
 * Release Date: 19-Jul-10
 *
 *****************************************************************************
 * File History
 * ------------
 *
 * 19-Jul-10, LVR, Created.
 *
 *
 * Comments
 * --------
 *
 *****************************************************************************/
#define kszApp "SwsInfo"
#define kszVer "1.01"
#define kszBanner kszApp " " kszVer " (" SWS_NET_ID ")" ", " TIMESTAMP "."
#define kszAbout "Display SwsSock configuration."

/* Compilation options */

/* Exports */

/* Imports */

/* ANSI C library */
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

/* SwsSock */
#define SWS_USE_SOCKLIB 1
#include "sws_net.h"
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
  } SConfig;


/***************************************************************************
 * Local function prototypes
 ***************************************************************************/

static int ParseArgs( SConfig* const, int, const char* const [] );
static void Help( const char* const);


/***************************************************************************
 * Module data
 ***************************************************************************/


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
  unsigned link, count;
  char szHost[ 256];
  ui32 uVal;

  /* Set default options */
  cfg.iVerbosity = 1;

  /* Parse args */
  iRet = ParseArgs( &cfg, argc, argv );
  if ( EXIT_SUCCESS != iRet)
    return iRet;

  if ( cfg.iVerbosity >= 0)
    fputs( kszBanner "\n", stderr);

  /* Specify the configuration file */
  if ( !SWS_CfgSetName( NULL, argv[0] ))
    fprintf( stderr, "WARNING: configuration file '" SWS_CFGFILE "' not found\n");
  else
    fprintf( stdout, "Configuration file: %s\n", SWS_CfgGetName());

  /* Initialise the sockets library */
  SWS_SockUp( 0, NULL);

  /* Display hostname and IP addresses */
  if ( 0 == gethostname( szHost, sizeof szHost))
    {
    int i;
    SWS_PTR( struct hostent) pHost;

    fprintf( stdout, "Hostname: %s", szHost);

    /* Get local IP address */
    if ( NULL == (pHost = gethostbyname( szHost)))
      {
      fprintf( stdout, " [no IP address]");
      }
    /* Display the list of local IP addresses */
    else for ( i = 0; NULL != pHost->h_addr_list[i]; ++i)
      {
      fprintf( stdout, " [%s]",
        inet_ntoa( *(const struct in_addr*)pHost->h_addr_list[i])
      );
      }
    fputs( "\n", stdout);
    }

  /* DNS servers */
  uVal = SWS_DnsGetPriServer();
  if ( SWS_INADDR_NONE != uVal)
    {
    fprintf( stdout, "DNS server: %lu.%lu.%lu.%lu",
      (uVal>>24)%256, (uVal>>16)%256, (uVal>>8)%256, uVal%256 );

    uVal = SWS_DnsGetSecServer();
    if ( SWS_INADDR_NONE == uVal || SWS_INADDR_ANY == uVal)
      fputs( "\n", stdout);
    else
      fprintf( stdout, ", %lu.%lu.%lu.%lu\n",
        (uVal>>24)%256, (uVal>>16)%256, (uVal>>8)%256, uVal%256 );
    }

  /* Enumerate datalinks */
  count = SWS_DatalinkCount();
  if ( 0 == count)
    fputs( "WARNING: no adapters found\n", stderr);
  else for ( link = 0; link < count; ++link)
    {
    size_t size, mtu;
    char szBuf[ 48];
    ui8 mac[6];

    /* Get adapter name */
    size = sizeof szBuf;
    iRet = SWS_DatalinkGetProperty(
      link,
      SWS_kDatalinkProperty_Ident,
      szBuf,
      &size
    );
    if ( 0 == iRet)
      fprintf( stdout, "%s\n", szBuf);

    /* Get IP address for datalink */
    size = sizeof uVal;
    iRet = SWS_DatalinkGetProperty(
      link,
      SWS_kDatalinkProperty_Address,
      &uVal,
      &size
    );
    if ( 0 == iRet)
      {
      fputs( " IP address: ", stdout);
      if ( SWS_INADDR_NONE == uVal)
        fputs( "none\n", stdout);
      else
        {
        fprintf( stdout, "%lu.%lu.%lu.%lu\n",
          (uVal>>24)%256, (uVal>>16)%256, (uVal>>8)%256, uVal%256
        );
        }
      }

    /* Subnet mask */
    size = sizeof uVal;
    iRet = SWS_DatalinkGetProperty(
      link,
      SWS_kDatalinkProperty_SubnetMask,
      &uVal,
      &size
    );
    if ( 0 == iRet)
      {
      fprintf( stdout, " Subnet mask: %lu.%lu.%lu.%lu\n",
        (uVal>>24)%256, (uVal>>16)%256, (uVal>>8)%256, uVal%256
      );
      }

    /* Gateway */
    size = sizeof uVal;
    iRet = SWS_DatalinkGetProperty(
      link,
      SWS_kDatalinkProperty_Gateway,
      &uVal,
      &size
    );
    if ( 0 == iRet)
      {
      fputs( " Gateway: ", stdout);
      switch ( uVal)
        {
      case SWS_INADDR_NONE:
        fputs( "none\n", stdout);
        break;
      case SWS_INADDR_ANY:
        fputs( "proxy ARP\n", stdout);
        break;
      default:
        fprintf( stdout, "%lu.%lu.%lu.%lu\n",
          (uVal>>24)%256, (uVal>>16)%256, (uVal>>8)%256, uVal%256
        );
        break;
        }
      }

    /* MTU */
    size = sizeof mtu;
    iRet = SWS_DatalinkGetProperty(
      link,
      SWS_kDatalinkProperty_MTU,
      &mtu,
      &size
    );
    if ( 0 == iRet)
      fprintf( stdout, " MTU: %u\n", (unsigned)mtu);

    /* MAC address */
    size = sizeof mac;
    iRet = SWS_DatalinkGetProperty(
      link,
      SWS_kDatalinkProperty_HwAddress,
      mac,
      &size
    );
    if ( 0 == iRet)
      {
      fprintf( stdout, " MAC address: %02x:%02x:%02x:%02x:%02x:%02x\n",
        mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]
      );
      }
    }

  return 0;
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
        /*SWS_u_long ulVal;*/

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
