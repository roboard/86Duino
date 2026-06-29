/*****************************************************************************
 *
 * File name   : main.c
 * Function    : SMTP sendmail command line interface
 * Authors     : Lawrence Rust
 * Systems     : ANSI C, sockets
 *
 *****************************************************************************
 * File History
 * ------------
 *
 * 29-Nov-02, LVR, Created.
 * 24-Feb-10, LVR, -i- option to read from stdin
 *
 *****************************************************************************/
#define kszApp "Sendmail"
#define kszVer STRING( SENDMAIL_VER_MAJOR) "." STRING( SENDMAIL_VER_MINOR)
#define kszBanner kszApp " " kszVer " (" SWS_NET_ID ")" ", " TIMESTAMP "."
#define kszAbout "Smtp mail client."

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
#include <ctype.h>

#include "sws_net.h"
#if SWS_USE_SOCKLIB && PACKET_DUMP
#include "sws_dump.h"
#endif

#include "sws_cfg.h"
#include "sendmail.h"


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

  const char* pszSmtpServer;
  u_short unSmtpPort;

  const char* pszHost; /* Helo name */
  const char* pszLogin; /* Login */

  const char* pszFrom;
  const char* pszReply;
  const char* pszSubject;

  int isInteractive;
  int raw;

  int iArgs;
  const char* const * pszArgvArray;
  } SConfig;


/***************************************************************************
 * Local function prototypes
 ***************************************************************************/

#ifdef SIGINT
static void OnSigInt( int);
#endif

static int ParseArgs( SConfig* const, int const, const char* const []);
static void Help( const char* const);
static void LogInit( FILE*, int);


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
    "\nSyntax: %.*s [options] 1*[recipient]\n"
    "-s<s> : Subject, '-'= empty\n"
    "-i<s> : File to send, '-'= stdin\n"
    "-E    : No envelope\n"
    "-f<s> : From address, '-'= empty\n"
    "-r<s> : Reply address, '-'= empty, default none\n"
    "-a<s> : AUTH PLAIN [usr:pwd | base64]\n"
    "-n<s> : HELO name, '-'= [IP], default is hostname\n"
    "-e<ip>: SMTP server, '-'= MX direct\n"
    "-p<n> : SMTP port\n",
    (int)c, pszCmd
  );
  fputs( 
    "-o<s> : Set logfile\n"
    "-t    : Decrease verbosity of output\n"
    "-v    : Increase verbosity of output\n"
    "-V    : Show version and exit\n"
    "-h    : Help\n"
    "\n"
    "Recipients following cc: will be copied.\n"
    "Recipients following bcc: will be 'blank' copied.\n",
    stderr
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
  char szHost[ 256], *psz;

  /* Set default options */
  cfg.iVerbosity = 1;
  cfg.logfile = stdout;
  cfg.isInteractive = 1;
  cfg.pszSmtpServer = NULL;
  cfg.unSmtpPort = 0;
  cfg.pszHost = NULL;
  cfg.pszLogin = NULL;
  cfg.raw = 0;
  cfg.pszFrom = NULL;
  cfg.pszReply = NULL;
  cfg.pszSubject = NULL;
  cfg.iArgs = 0;
  cfg.pszArgvArray = NULL;

  /* Parse args */
  iRet = ParseArgs( &cfg, argc, argv );
  if ( EXIT_SUCCESS != iRet)
    return iRet;

  /* Specify the configuration file */
  if ( !SWS_CfgSetName( NULL, argv[0] )
    && (NULL ==  cfg.pszFrom || NULL == cfg.pszSmtpServer)
  )
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

  /* Prompt for missing args */
  if ( !cfg.raw && NULL == cfg.pszSubject)
    {
    static char s_szSubject[ 80];

    if ( cfg.isInteractive)
      {
      fputs( "Subject: ", stderr);
      cfg.pszSubject = fgets( s_szSubject, sizeof s_szSubject, stdin);
      fputs( "\n", stderr);
      psz = strchr( s_szSubject, '\n');
      if ( NULL != psz)
        *psz = '\0';
      }
    else
      cfg.pszSubject = "";
    }

  /* Recipients */
  while ( 0 == cfg.iArgs && cfg.isInteractive)
    {
    static char s_szRecipients[ 256];
    enum { kRecipients = 16 };
    static const char* s_pszRecipientArrary[ kRecipients];
    char* pszRcpt;

    cfg.pszArgvArray = s_pszRecipientArrary;

    fputs( "Recipients: ", stderr);
    pszRcpt = fgets( s_szRecipients, sizeof s_szRecipients, stdin);
    fputs( "\n", stderr);
    psz = strchr( s_szRecipients, '\n');
    if ( NULL != psz)
      *psz = '\0';

    while ( '\0' != *pszRcpt && cfg.iArgs < kRecipients)
      {
      /* Skip leading space */
      while ( isspace( *pszRcpt))
        ++pszRcpt;

      s_pszRecipientArrary[ cfg.iArgs++] = pszRcpt;

      while ( isgraph( *pszRcpt))
        ++pszRcpt;

      /* Terminate the name */
      if ( '\0' != *pszRcpt)
        *pszRcpt++ = '\0';
      }
    }

  /* Send mail */
  iRet = SendMail(
    stdin, cfg.isInteractive,
    cfg.iArgs,
    cfg.pszArgvArray,
    cfg.pszFrom,
    cfg.raw ? NULL : cfg.pszReply,
    cfg.raw ? NULL : cfg.pszSubject,
    cfg.pszSmtpServer,
    cfg.unSmtpPort,
    cfg.pszHost,
    cfg.pszLogin,
    cfg.logfile,
    cfg.iVerbosity
  );
  if ( cfg.isInteractive && 0 == iRet)
    fputs( "Mail sent ok.\n", stderr);

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
  int const iArgc,                    /* IN: No. args */
  const char* const pszArgvArray []    /* IN: Array of arg strings */
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

        case 'e':
          /* SMTP server */
          pszArg = GetArg();
          pCfg->pszSmtpServer = strcmp( "-", pszArg) ? pszArg : "";
          pszArg = NULL;
          break;

        case 'p':
          /* SMTP port */
          pszArg = GetArg();
          lVal = strtol( pszArg, &pszEnd, 10);
          if ( *pszEnd || lVal < 0 || lVal > 65535L)
            {
            fputs( kszBanner "\n", stderr);
            fprintf( stderr, "Invalid port %s\n", pszArg);
            return EXIT_FAILURE;
            }

          pCfg->unSmtpPort = (u_short)lVal;
          pszArg = NULL;
          break;

        case 'f':
          /* From address */
          pszArg = GetArg();
          pCfg->pszFrom = strcmp( "-", pszArg) ? pszArg : "";
          pszArg = NULL;
          break;

        case 'r':
          /* Reply address */
          pszArg = GetArg();
          pCfg->pszReply = strcmp( "-", pszArg) ? pszArg : "";
          pszArg = NULL;
          break;

        case 's':
          /* Subject */
          pszArg = GetArg();
          pCfg->pszSubject = strcmp( "-", pszArg) ? pszArg : "";
          pszArg = NULL;
          break;

        case 'i':
          /* Input file */
          pszArg = GetArg();
          if ( *pszArg && strcmp( "-", pszArg)
            && NULL == freopen( pszArg, "r", stdin)
          ) {
            fputs( kszBanner "\n", stderr);
            fprintf( stderr, "Failed to open '%s'\n", pszArg);
            return EXIT_FAILURE;
            }

          pCfg->isInteractive = 0;
          pszArg = NULL;
          break;

        case 'n':
          /* HELO name */
          pszArg = GetArg();
          pCfg->pszHost = strcmp( "-", pszArg) ? pszArg : "";
          pszArg = NULL;
          break;

        case 'a':
          /* AUTH PLAIN text */
          pszArg = GetArg();
          pCfg->pszLogin = pszArg;
          pszArg = NULL;
          break;

        case 'E':
          /* No envelope */
          pCfg->raw = 1;
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
      /* Recipient list */
      pCfg->pszArgvArray = pszArgvArray + i;
      pCfg->iArgs = iArgc - i;
      break;
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
