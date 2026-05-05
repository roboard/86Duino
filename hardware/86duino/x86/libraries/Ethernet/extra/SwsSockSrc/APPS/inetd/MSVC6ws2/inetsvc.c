/*****************************************************************************
 *
 * File name   : inetsvc.c
 * Function    : TCP/IP services using threads
 * Authors     : Lawrence Rust
 * Systems     : Win32
 * Version     : 1.0
 * Release Date: 25-Feb-03
 *
 *****************************************************************************
 * File History
 * ------------
 *
 * 25-Feb-03, LVR, Created.
 *
 *
 * Comments
 * --------
 *
 *****************************************************************************/
#define kszApp "INETSVC"
#define kszVer "1.00"
#define kszBanner kszApp " " kszVer " (" SWS_NET_ID ")" ", " TIMESTAMP "."
#define kszAbout "FTP service."

/* Compilation options */
#ifndef INETD_FTP
#define INETD_FTP 1
#endif

#ifndef INETD_TELNET
#define INETD_TELNET 1
#endif

#ifndef INETD_SMTP
#define INETD_SMTP 1
#endif

#ifndef INETD_HTTP
#define INETD_HTTP 1
#endif

#ifndef INETD_POP
#define INETD_POP 1
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

/* Win32 */
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#pragma warning ( disable : 4201) /* nonstandard extension used : nameless struct/union */
#include <windows.h>
#include <winsock2.h>
#pragma warning ( default : 4201) /* nonstandard extension used : nameless struct/union */
#pragma warning ( disable : 4127) /* conditional expression is constant */

#include "sws_thrd.h"
#include "sws_cfg.h"

#if INETD_HTTP
#include "httpd.h"
#include "httpfile.h"
#endif
#if INETD_FTP
#include "ftpd.h"
#endif
#if INETD_TELNET
#include "telnetd.h"
#endif
#if INETD_SMTP
#include "smtpd.h"
#endif
#if INETD_POP
#include "popd.h"
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

/* ini file values */
#define kszInetdSect "inetsvc"
#define kszVerbosity "log_detail"
#define kszLogfile   "log_file"
#define kszFtp       "ftp"
#define kszTelnet    "telnet"
#define kszSmtp      "smtp"
#define kszHttp      "http"
#define kszPop       "pop"
#define kszEcho      "echo"
#define kszDiscard   "discard"
#define kszIdent     "ident"


/***************************************************************************
 * Data types
 ***************************************************************************/

/* Parameters */
typedef struct SConfig
  {
  int iVerbosity;
  FILE* logfile;
#if INETD_HTTP
  struct sockaddr_in hostWww;
#endif
#if INETD_FTP
  struct sockaddr_in hostFtp;
#endif
#if INETD_SMTP
  struct sockaddr_in hostSmtp;
#endif
#if INETD_POP
  struct sockaddr_in hostPop;
#endif
#if INETD_TELNET
  struct sockaddr_in hostTel;
#endif
  } SConfig;


/***************************************************************************
 * Local function prototypes
 ***************************************************************************/

static void SockInit( SConfig*);
static void OnExit( void);
static int ParseArgs( SConfig* const, int, const char* const [] );
static void Banner( FILE*);

static VOID WINAPI ServiceMain( DWORD argc, LPTSTR argv[] );
static VOID WINAPI ServiceCtrlHandler( DWORD opcode);
static DWORD ServiceInitialise( DWORD argc, LPTSTR *argv, DWORD *specificError);

static int Start( SConfig* pCfg);
static void Stop( void);
static void Pause( void);
static void Resume( void);

#if INETD_FTP
static SWS_ThreadFunction FtpThread;
#endif
#if INETD_HTTP
static SWS_ThreadFunction HttpThread;
#endif
#if INETD_TELNET
static SWS_ThreadFunction TelnetThread;
#endif
#if INETD_SMTP
static SWS_ThreadFunction SmtpThread;
#endif
#if INETD_POP
static SWS_ThreadFunction PopThread;
#endif


/***************************************************************************
 * Module data
 ***************************************************************************/

sig_atomic_t volatile g_exit;
static int s_iThreads;

static SERVICE_STATUS s_ServiceStatus;
static SERVICE_STATUS_HANDLE s_hServiceStatus;

#if INETD_HTTP
static SWS_ThreadHandle hHttpd;
#endif
#if INETD_FTP
static SWS_ThreadHandle hFtpd;
#endif
#if INETD_TELNET
static SWS_ThreadHandle hTelnetd;
#endif
#if INETD_SMTP
static SWS_ThreadHandle hSmtpd;
#endif
#if INETD_POP
static SWS_ThreadHandle hPopd;
#endif


/***************************************************************************
 * Functions
 ***************************************************************************/

/*
 * M A I N
 */
int main( int argc, const char* argv[])
  {
  static const SERVICE_TABLE_ENTRY DispatchTable[] = {
    { kszApp, &ServiceMain },
    { NULL,   0 }
  };
  (void)argc;

  /* Specify the configuration file */
  if ( !SWS_CfgSetName( "inetsvc.ini", argv[0] ))
    {
    fprintf( stderr, "Configuration file (inetsvc.ini) not found\n");
    return EXIT_FAILURE;
    }

  if ( !StartServiceCtrlDispatcher( &DispatchTable[0] ))
    {
    fprintf( stderr, "Failed to start service (%d)\n", GetLastError() );
    return EXIT_FAILURE;
    }

  return 0;
  }


/*
 * Service main
 */
VOID WINAPI ServiceMain( DWORD argc, LPTSTR argv[] )
  {
  DWORD status;
  DWORD specificError;

  /* Register control handler */
  s_ServiceStatus.dwServiceType        = SERVICE_WIN32;
  s_ServiceStatus.dwCurrentState       = SERVICE_START_PENDING;
  s_ServiceStatus.dwControlsAccepted   = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_PAUSE_CONTINUE;
  s_ServiceStatus.dwWin32ExitCode      = 0;
  s_ServiceStatus.dwServiceSpecificExitCode = 0;
  s_ServiceStatus.dwCheckPoint         = 0;
  s_ServiceStatus.dwWaitHint           = 1000; /* Millisecs */
#ifndef NDEBUG
  DebugBreak();
#endif
  s_hServiceStatus = RegisterServiceCtrlHandler( kszApp, &ServiceCtrlHandler);
  if ( 0 == s_hServiceStatus)
    return;

  /* Initialise */
  status = ServiceInitialise( argc, argv, &specificError);
  if ( NO_ERROR != status)
    {
    s_ServiceStatus.dwCurrentState       = SERVICE_STOPPED;
    s_ServiceStatus.dwCheckPoint         = 0;
    s_ServiceStatus.dwWaitHint           = 0;
    s_ServiceStatus.dwWin32ExitCode      = status;
    s_ServiceStatus.dwServiceSpecificExitCode = specificError;
    SetServiceStatus( s_hServiceStatus, &s_ServiceStatus);
    return;
    }

  /* Report running status */
  s_ServiceStatus.dwCurrentState       = SERVICE_RUNNING;
  s_ServiceStatus.dwCheckPoint         = 0;
  s_ServiceStatus.dwWaitHint           = 0;
  SetServiceStatus( s_hServiceStatus, &s_ServiceStatus);
  }


/*
 * Service control
 */
VOID WINAPI ServiceCtrlHandler( DWORD opcode)
  {
  switch ( opcode)
    {
  case SERVICE_CONTROL_PAUSE:
    Pause();
    s_ServiceStatus.dwCurrentState = SERVICE_PAUSED;
    break;

  case SERVICE_CONTROL_CONTINUE:
    Resume();
    s_ServiceStatus.dwCurrentState = SERVICE_RUNNING;
    break;

  case SERVICE_CONTROL_STOP:
    Stop();
    s_ServiceStatus.dwWin32ExitCode = 0;
    s_ServiceStatus.dwCurrentState  = SERVICE_STOPPED;
    s_ServiceStatus.dwCheckPoint    = 0;
    s_ServiceStatus.dwWaitHint      = 0;
    break;

  case SERVICE_CONTROL_INTERROGATE:
    break;

  case SERVICE_CONTROL_PARAMCHANGE:
  case SERVICE_CONTROL_SHUTDOWN:
  default:
    break;
    }

  SetServiceStatus( s_hServiceStatus, &s_ServiceStatus);
  }


/*
 * Service initialisation
 */
DWORD ServiceInitialise( DWORD argc, LPTSTR argv[], DWORD* pdwSpecificError)
  {
  static SConfig cfg;
  int iRet;
  struct hostent* pHost;
  char szHost[ 256];

  /* Set default options */
  cfg.iVerbosity = (int)SWS_CfgGetValue( kszInetdSect, kszVerbosity, 1);
  if ( SWS_CfgGetString( kszInetdSect, kszLogfile, "", szHost, sizeof(szHost) ))
    cfg.logfile = fopen( szHost, "a"); /* Append */
  else
    cfg.logfile = NULL;

#if INETD_HTTP
  cfg.hostWww.sin_family = AF_INET;
  cfg.hostWww.sin_port = 0;
  cfg.hostWww.sin_addr.s_addr = INADDR_ANY;
#endif

#if INETD_FTP
  cfg.hostFtp.sin_family = AF_INET;
  cfg.hostFtp.sin_port = 0;
  cfg.hostFtp.sin_addr.s_addr = INADDR_ANY;
#endif

#if INETD_SMTP
  cfg.hostSmtp.sin_family = AF_INET;
  cfg.hostSmtp.sin_port = 0;
  cfg.hostSmtp.sin_addr.s_addr = INADDR_ANY;
#endif

#if INETD_POP
  cfg.hostPop.sin_family = AF_INET;
  cfg.hostPop.sin_port = 0;
  cfg.hostPop.sin_addr.s_addr = INADDR_ANY;
#endif

#if INETD_TELNET
  cfg.hostTel.sin_family = AF_INET;
  cfg.hostTel.sin_port = 0;
  cfg.hostTel.sin_addr.s_addr = INADDR_ANY;
#endif

  iRet = ParseArgs( &cfg, argc, argv );
  if ( EXIT_SUCCESS != iRet)
    {
    *pdwSpecificError = ERROR_INVALID_PARAMETER;
    return iRet;
    }

  /* Initialise the sockets library */
  SockInit( &cfg);

  /* Check network is up */
  if ( gethostname( szHost, sizeof(szHost))
    || NULL == (pHost = gethostbyname( szHost))
  ) {
    fprintf( stderr, "No network\n");
    return (*pdwSpecificError = WSAGetLastError());
    }

  Start( &cfg);
  return 0;
  }


/*
 * Start the services
 */
int Start( SConfig* pCfg)
  {
#if INETD_HTTP
  /* Create & start the HTTP server thread */
  if ( SWS_CfgGetValue( kszInetdSect, kszHttp, 1) )
    hHttpd = SWS_ThreadCreate( &HttpThread, pCfg, 0);
  else
    hHttpd = NULL;
  if ( NULL != hHttpd)
    SWS_ThreadStart( hHttpd);
#endif

#if INETD_FTP
  /* Create & start the FTP server thread */
  if ( SWS_CfgGetValue( kszInetdSect, kszFtp, 1) )
    hFtpd = SWS_ThreadCreate( &FtpThread, pCfg, 0);
  else
    hFtpd = NULL;
  if ( NULL != hFtpd)
    SWS_ThreadStart( hFtpd);
#endif

#if INETD_TELNET
  /* Create & start the telnet server thread */
  if ( SWS_CfgGetValue( kszInetdSect, kszTelnet, 1) )
    hTelnetd = SWS_ThreadCreate( &TelnetThread, pCfg, 0);
  else
    hTelnetd = NULL;
  if ( NULL != hTelnetd)
    SWS_ThreadStart( hTelnetd);
#endif

#if INETD_SMTP
  /* Create & start the SMTP server thread */
  if ( SWS_CfgGetValue( kszInetdSect, kszSmtp, 1) )
    hSmtpd = SWS_ThreadCreate( &SmtpThread, pCfg, 0);
  else
    hSmtpd = NULL;
  if ( NULL != hSmtpd)
    SWS_ThreadStart( hSmtpd);
#endif

#if INETD_POP
  /* Create & start the POP server thread */
  if ( SWS_CfgGetValue( kszInetdSect, kszPop, 1) )
    hPopd = SWS_ThreadCreate( &PopThread, pCfg, 0);
  else
    hPopd = NULL;
  if ( NULL != hPopd)
    SWS_ThreadStart( hPopd);
#endif

  return 0;
  }


/*
 * Stop the services
 */
void Stop( void)
  {
  /* Wait for all threads to exit */
  g_exit = 1;
  do
    SleepEx( 300, TRUE);
  while ( s_iThreads > 0);

#if INETD_POP
  if ( NULL != hPopd)
    {
    SWS_ThreadStop( hPopd);
    SWS_ThreadDestroy( hPopd);
    }
#endif

#if INETD_SMTP
  if ( NULL != hSmtpd)
    {
    SWS_ThreadStop( hSmtpd);
    SWS_ThreadDestroy( hSmtpd);
    }
#endif

#if INETD_TELNET
  if ( NULL != hTelnetd)
    {
    SWS_ThreadStop( hTelnetd);
    SWS_ThreadDestroy( hTelnetd);
    }
#endif

#if INETD_FTP
  if ( NULL != hFtpd)
    {
    SWS_ThreadStop( hFtpd);
    SWS_ThreadDestroy( hFtpd);
    }
#endif

#if INETD_HTTP
  if ( NULL != hHttpd)
    {
    SWS_ThreadStop( hHttpd);
    SWS_ThreadDestroy( hHttpd);
    }
#endif

  /* Destroy the current (main) thread */
  SWS_ThreadDestroy( NULL );
  }


/*
 * Pause the services
 */
void Pause( void)
  {
#if INETD_POP
  if ( NULL != hPopd)
    SWS_ThreadStop( hPopd);
#endif

#if INETD_SMTP
  if ( NULL != hSmtpd)
    SWS_ThreadStop( hSmtpd);
#endif

#if INETD_TELNET
  if ( NULL != hTelnetd)
    SWS_ThreadStop( hTelnetd);
#endif

#if INETD_FTP
  if ( NULL != hFtpd)
    SWS_ThreadStop( hFtpd);
#endif

#if INETD_HTTP
  if ( NULL != hHttpd)
    SWS_ThreadStop( hHttpd);
#endif
  }


/*
 * Resume the services
 */
void Resume( void)
  {
#if INETD_POP
  if ( NULL != hPopd)
    SWS_ThreadStart( hPopd);
#endif

#if INETD_SMTP
  if ( NULL != hSmtpd)
    SWS_ThreadStart( hSmtpd);
#endif

#if INETD_TELNET
  if ( NULL != hTelnetd)
    SWS_ThreadStart( hTelnetd);
#endif

#if INETD_FTP
  if ( NULL != hFtpd)
    SWS_ThreadStart( hFtpd);
#endif

#if INETD_HTTP
  if ( NULL != hHttpd)
    SWS_ThreadStart( hHttpd);
#endif
  }


#if INETD_HTTP
/*
 * HTTP server thread
 */
static SWS_CB( i32, HttpThread( SWS_PTR( void) pvContext))
  {
  const SConfig* const pCfg = (const SConfig*)pvContext;
  int iRet;

  /* Install HTTP file access */
  if ( !HttpFileInit())
    return EXIT_FAILURE;

  /* Run the web server */
  ++s_iThreads;
  iRet = Httpd(
    &pCfg->hostWww,
    pCfg->iVerbosity,
    pCfg->logfile
  );
  --s_iThreads;
  return iRet;
  }
#endif


#if INETD_FTP
/*
 * FTP server thread
 */
static SWS_CB( i32, FtpThread( SWS_PTR( void) pvContext))
  {
  const SConfig* const pCfg = (const SConfig*)pvContext;
  int iRet;

  /* Run the ftp server */
  ++s_iThreads;
  iRet = Ftpd(
    &pCfg->hostFtp,
    pCfg->iVerbosity,
    pCfg->logfile
  );
  --s_iThreads;
  return iRet;
  }
#endif


#if INETD_TELNET
/*
 * Telnet server thread
 */
static SWS_CB( i32, TelnetThread( SWS_PTR( void) pvContext))
  {
  const SConfig* const pCfg = (const SConfig*)pvContext;
  int iRet;

  /* Run the telnet server */
  ++s_iThreads;
  iRet = Telnetd(
    &pCfg->hostTel,
    pCfg->iVerbosity,
    pCfg->logfile,
    0, NULL
  );
  --s_iThreads;
  return iRet;
  }
#endif


#if INETD_SMTP
/*
 * SMTP server thread
 */
static SWS_CB( i32, SmtpThread( SWS_PTR( void) pvContext))
  {
  const SConfig* const pCfg = (const SConfig*)pvContext;
  int iRet;

  /* Run the SMTP server */
  ++s_iThreads;
  iRet = Smtpd(
    &pCfg->hostSmtp,
    pCfg->iVerbosity,
    pCfg->logfile
  );
  --s_iThreads;
  return iRet;
  }
#endif


#if INETD_POP
/*
 * POP server thread
 */
static SWS_CB( i32, PopThread( SWS_PTR( void) pvContext))
  {
  const SConfig* const pCfg = (const SConfig*)pvContext;
  int iRet;

  /* Run the SMTP server */
  ++s_iThreads;
  iRet = Popd(
    &pCfg->hostPop,
    pCfg->iVerbosity,
    pCfg->logfile
  );
  --s_iThreads;
  return iRet;
  }
#endif


/*
 * Show program headline
 */
static void Banner( FILE* file)
  {
  fputs( kszBanner "\n" kszAbout "\n", file);
  }


/*
 * Initialise the sockets library
 */
static void SockInit( SConfig* pCfg)
  {
  static int s_inited;

  if ( !s_inited)
    {
    int iRet;
    WSADATA wsaData;

    s_inited = 1;

    if ( NULL != pCfg && NULL != pCfg->logfile)
      {
      time_t const t = time( NULL);

      setvbuf( pCfg->logfile, NULL, _IONBF, 0);

      Banner( pCfg->logfile);
      fprintf( pCfg->logfile, "Logging(%d) started %s\n", pCfg->iVerbosity, asctime( localtime( &t) ) );
      }

    iRet = WSAStartup( 0x202, &wsaData);
    if ( iRet)
      {
      fprintf( stderr, "WSAStartup failed %d", iRet);
      exit( EXIT_FAILURE);
      }

    atexit( &OnExit);
    }
  }


/*
 * Finalise the sockets library
 */
static void OnExit( void)
  {
  WSACleanup();
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

  /* Scan args */
  for ( i = 1; i < iArgc; ++i)
    {
    char c;
    const char* pszArg;

    pszArg = pszArgvArray[i];

    /* Check for switch options ('-' or '/' prefix) */
#   define isswitch( _c) ('-' == _c || '/' == _c)
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

#if INETD_HTTP
        case 'W':
          /* http port */
          pszArg = GetArg();
          lVal = strtol( pszArg, &pszEnd, 10);
          if ( *pszEnd || lVal < 0 || lVal > 65535L)
            {
            fprintf( stderr, "Invalid port %s\n", pszArg);
            return EXIT_FAILURE;
            }

          pszArg = pszEnd;
          pCfg->hostWww.sin_port = htons((u_short)lVal);
          break;
#endif

#if INETD_FTP
        case 'F':
          /* ftp port */
          pszArg = GetArg();
          lVal = strtol( pszArg, &pszEnd, 10);
          if ( *pszEnd || lVal < 0 || lVal > 65535L)
            {
            fprintf( stderr, "Invalid port %s\n", pszArg);
            return EXIT_FAILURE;
            }

          pszArg = pszEnd;
          pCfg->hostFtp.sin_port = htons((u_short)lVal);
          break;
#endif

#if INETD_TELNET
        case 'T':
          /* telnet port */
          pszArg = GetArg();
          lVal = strtol( pszArg, &pszEnd, 10);
          if ( *pszEnd || lVal < 0 || lVal > 65535L)
            {
            fprintf( stderr, "Invalid port %s\n", pszArg);
            return EXIT_FAILURE;
            }

          pszArg = pszEnd;
          pCfg->hostTel.sin_port = htons((u_short)lVal);
          break;
#endif

#if INETD_SMTP
        case 'S':
          /* SMTP port */
          pszArg = GetArg();
          lVal = strtol( pszArg, &pszEnd, 10);
          if ( *pszEnd || lVal < 0 || lVal > 65535L)
            {
            fprintf( stderr, "Invalid port %s\n", pszArg);
            return EXIT_FAILURE;
            }

          pszArg = pszEnd;
          pCfg->hostSmtp.sin_port = htons((u_short)lVal);
          break;
#endif

#if INETD_POP
        case 'P':
          /* POP port */
          pszArg = GetArg();
          lVal = strtol( pszArg, &pszEnd, 10);
          if ( *pszEnd || lVal < 0 || lVal > 65535L)
            {
            fprintf( stderr, "Invalid port %s\n", pszArg);
            return EXIT_FAILURE;
            }

          pszArg = pszEnd;
          pCfg->hostPop.sin_port = htons((u_short)lVal);
          break;
#endif

        case 'o':
          /* logfile */
          if ( NULL != pCfg->logfile)
            fclose ( pCfg->logfile);

          pszArg = GetArg();
          pCfg->logfile = fopen( pszArg, "a");
          if ( NULL == pCfg->logfile)
            {
            fprintf( stderr, "Failed to open logfile '%s'\n", pszArg);
            return EXIT_FAILURE;
            }

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

  return EXIT_SUCCESS;
  }

/* End of file */
