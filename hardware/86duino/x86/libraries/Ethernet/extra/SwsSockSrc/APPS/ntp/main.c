/*****************************************************************************
 *
 * File name   : main.c
 * Function    : NTP query
 * Authors     : Lawrence Rust
 * Systems     : ANSI C, BSD sockets
 * Version     : 1.0
 *
 *****************************************************************************
 * File History
 * ------------
 *
 * 27-Feb-03, LVR, Created.
 *
 *
 * Comments
 * --------
 *
 *****************************************************************************/
#define kszApp "NTP"
#define kszVer "1.00"
#define kszBanner kszApp " " kszVer " (" SWS_NET_ID ")" ", " TIMESTAMP "."
#define kszAbout "Net Time Protocol client."

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
#include <math.h>

#if defined WIN32 || defined _WIN32 || defined __WIN32__ || defined WIN16
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h> /* for SetSystemTime */
#endif

#include "sws_net.h"
#if SWS_USE_SOCKLIB && PACKET_DUMP
#include "sws_dump.h"
#endif

#include "sws_ntp.h"
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

#define SECS_1900_TO_1970 ((70UL * 365UL + 17UL) * (24UL * 3600UL))


/***************************************************************************
 * Data types
 ***************************************************************************/

/* Parameters */
typedef struct SConfig
  {
  int iVerbosity;
#if SWS_USE_SOCKLIB && PACKET_DUMP
  FILE* logfile;
#endif
  const char* pszServer;
  unsigned short uPort;
  int iSetTime;
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

static int PlatformSetTime( const SWS_SNtpTime*);


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
    "\nSyntax: %.*s [options] [hostname]\n"
    "-p<n> : Set NTP server port\n"
    "-s    : Set system date & time\n"
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
  SWS_SNtpRecord ntp;
  struct sockaddr_in host;

  /* Set default options */
  cfg.iVerbosity = 1;
#if SWS_USE_SOCKLIB && PACKET_DUMP
  cfg.logfile = stdout;
#endif
  cfg.iSetTime = 0;
  cfg.pszServer = NULL;
  cfg.uPort = 0;

  /* Parse args */
  iRet = ParseArgs( &cfg, argc, argv );
  if ( EXIT_SUCCESS != iRet)
    return iRet;

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

  /* server given? */
  if ( NULL != cfg.pszServer)
    {
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
      fprintf( stderr, "Unable to resolve %s\n", cfg.pszServer);
      return EXIT_FAILURE;
      }
    }

  if ( cfg.iVerbosity > 1)
    printf( "Querying NTP server..."), fflush( stdout);

  /* Query the ntp server */
  iRet = SWS_NtpQuery( &ntp, (struct sockaddr*)&host, sizeof host);
  if ( 0 == iRet)
    {
    time_t t;
    struct tm lt, gmt;
    double secs;
    struct in_addr addr;

    if ( cfg.iVerbosity > 1)
      printf( "\b\b\b %s (v%u):\n\n", inet_ntoa( host.sin_addr), ntp.uVersion );

    if ( cfg.iVerbosity >= 0)
      {
      int i;

      /* Local time */
      t = (time_t)(ntp.tSys.secs - SECS_1900_TO_1970);
      lt = *localtime( &t);
      strftime( szHost, sizeof(szHost), "%a, %d %b %Y %H:%M", &lt);
      printf( "Sys time: %s", szHost);
      secs = lt.tm_sec + (double)ntp.tSys.fract / 4294967296.0;
      printf( ":%09.6f", secs);

      gmt = *gmtime( &t);
      i = lt.tm_hour - gmt.tm_hour;
      if ( i <= -12)
        i += 24;
      else if ( i > 12)
        i -= 24;
      i *= 100;
      i += lt.tm_min - gmt.tm_min;
      printf( " %+05d\n", i);
      }

    /* NTP time */
    t = (time_t)(ntp.tNet.secs - SECS_1900_TO_1970);
    lt = *gmtime( &t);
    strftime( szHost, sizeof(szHost), "%a, %d %b %Y %H:%M", &lt);
    printf( "NTP time: %s", szHost);
    secs = lt.tm_sec + (double)ntp.tNet.fract / 4294967296.0;
    printf( ":%09.6f +0000\n", secs);

    if ( cfg.iVerbosity >= 2)
      {
      /* NTP reference time */
      t = (time_t)(ntp.tSet.secs - SECS_1900_TO_1970);
      lt = *gmtime( &t);
      strftime( szHost, sizeof(szHost), "%a, %d %b %Y %H:%M", &lt);
      printf( "Ref time: %s", szHost);
      secs = lt.tm_sec + (double)ntp.tSet.fract / 4294967296.0;
      printf( ":%09.6f +0000\n", secs);

      /* NTP server info */
      printf( "Stratum %u, Precision %.1f Hz, Error %g secs, Drift %g\n",
        ntp.uStratum,
        pow( 2, -ntp.iPrecision),
        (double)ntp.uError / 0x10000,
        (double)ntp.iDrift / 4294967296.0
      );

      addr.s_addr = ntp.addrRef;
      printf( "Reference: %s\n", inet_ntoa( addr) );

      /* RTT */
      secs = ntp.tRtt.secs + (double)ntp.tRtt.fract / 4294967296.0;
      printf( "RTT: %g secs\n", secs);
      }

    if ( cfg.iSetTime)
      {
      /* Set the local time */
      if ( PlatformSetTime( &ntp.tNet))
        puts( "\nTime set successfully");
      else
        puts( "\nTime set failed");
      }
    }
  else if ( INADDR_NONE != host.sin_addr.s_addr)
    {
    printf( "\b\b\b %s FAILED (%d)\n", inet_ntoa( host.sin_addr), iRet);
    }
  else
    {
    printf( "\b\b\b FAILED: NTP server hostname required\n");
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

          pCfg->uPort = (unsigned short)lVal;
          pszArg = NULL;
          break;

        case 's':
          /* Set time */
          pCfg->iSetTime = 1;
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
        /* time server */
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


/****************************************************************************
 * Platform specific code
 ****************************************************************************/

#if defined WIN32 || defined _WIN32 || defined __WIN32__

/*
 * Set date/time (GMT)
 */
int PlatformSetTime( const SWS_SNtpTime* pTime)
  {
  SYSTEMTIME syst;
  struct tm gmt;
  time_t t;

  t = (time_t)(pTime->secs - SECS_1900_TO_1970);
  gmt = *gmtime( &t);

  syst.wYear = (WORD)(gmt.tm_year + 1900);
  syst.wMonth = (WORD)(gmt.tm_mon + 1);
  syst.wDay = (WORD)gmt.tm_mday;
  syst.wDayOfWeek = 0;
  syst.wHour = (WORD)gmt.tm_hour;
  syst.wMinute = (WORD)gmt.tm_min;
  syst.wSecond = (WORD)gmt.tm_sec;

  syst.wMilliseconds = (WORD)(((pTime->fract >> 10) * 1000) >> 22);

  return SetSystemTime( &syst);
  }


#elif defined _POSIX_SOURCE || defined _POSIX_C_SOURCE || defined __DJGPP__
#undef timeval
#include <sys/time.h> /* settimeofday */

/*
 * Set date/time (GMT)
 */
int PlatformSetTime( const SWS_SNtpTime* pTime)
  {
  struct timeval syst;

  syst.tv_sec = (time_t)(pTime->secs - SECS_1900_TO_1970);
  syst.tv_usec = (long)(((pTime->fract >> 16) * (1000000>>4)) >> (16-4));

  return 0 == settimeofday( &syst, NULL);
  }


#elif defined _DOS || defined _DOS16
#include <dos.h> /* _dos_settime */

/*
 * Set date/time (GMT)
 */
int PlatformSetTime( const SWS_SNtpTime* pTime)
  {
  struct tm local;
  time_t t;
  struct dosdate_t dosdate;
  struct dostime_t dostime;

  t = (time_t)(pTime->secs - SECS_1900_TO_1970);
  local = *localtime( &t);

  dosdate.day = (unsigned char)local.tm_mday;
  dosdate.month = (unsigned char)(local.tm_mon + 1);
  dosdate.year = (unsigned short)(local.tm_year + 1900);
  dosdate.dayofweek = (unsigned char)local.tm_wday;
  if ( _dos_setdate( &dosdate))
    return 0;

  dostime.hour = (unsigned char)local.tm_hour;
  dostime.minute = (unsigned char)local.tm_min;
  dostime.second = (unsigned char)local.tm_sec;
  dostime.hsecond = (unsigned char)(((pTime->fract >> 7) * 100) >> (32-7));
  if ( _dos_settime( &dostime))
    return 0;

  return 1;
  }

#else

/*
 * Set date/time (GMT)
 */
int PlatformSetTime( const SWS_SNtpTime* pTime)
  {
  int WARN_PlatformSetTime_NotImplemented;
  (void)pTime;
  return 0; /* Failed */
  }

#endif
/* End of file */
