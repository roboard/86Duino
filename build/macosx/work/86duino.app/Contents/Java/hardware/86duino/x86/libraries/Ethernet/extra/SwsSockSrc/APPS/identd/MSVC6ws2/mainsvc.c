/*****************************************************************************
 * Copyright (C) 2005 L.V.Rust, Software Systems Consultants.
 *
 * File name   : mainsvc.c
 * Function    : Win32 service startup
 * Authors     : Lawrence Rust
 * Systems     : Win32
 *
 *****************************************************************************
 * File History
 * ------------
 *
 * 05-Apr-05, LVR, Created.
 *
 *
 * Comments
 * --------
 * Front end for the ident service
 *
 *****************************************************************************/
#define VER_MAJOR 1
#define VER_MINOR 01

/* Compilation options */
#ifndef OPT_SERVICE
#define OPT_SERVICE 0 /* !0 to build as a service */
#endif

#if defined _DEBUG && _MSC_VER >= 1000
#define OPT_FIND_LEAKS 1
#endif

#if defined _MSC_VER
# pragma warning ( disable : 4711) /* function selected for automatic inline expansion */
#endif

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

/* Win32 */
#include <io.h>
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x400
#endif
#pragma warning ( disable : 4201) /* nonstandard extension used : nameless struct/union */
#include <windows.h>
#include <winsvc.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#if OPT_SERVICE && defined _MSC_VER
#pragma comment( lib, "advapi32.lib") /* For SCM */
//#pragma comment( lib, "delayimp.lib") /* Required for delay loading */
//#pragma comment( linker, "/delayload:advapi32.dll") /* Delayload dll */
#endif

/* App modules */
#include "identd.h"
#include "sws_cfg.h"


/***************************************************************************
 * Macros & constants
 ***************************************************************************/

#define TIMESTAMP "built " __TIME__ " " __DATE__

/* Expand and stringize a macro name */
#define STR(s) #s
#define STRING(s) STR(s)

#define kszApp "IdentSvc"
#define kszVer STRING( VER_MAJOR) "." STRING( VER_MINOR)
#define kszInfo kszApp " version " kszVer " " TIMESTAMP


/***************************************************************************
 * Data types
 ***************************************************************************/

/* Params to ServiceRun */
struct SParams
  {
  BOOL bNewLog;
  };


/***************************************************************************
 * Local function prototypes
 ***************************************************************************/

static int SockInit( void);
static void OnExit( void);

static DWORD WINAPI ServiceRun( LPVOID);
static FILE* LogBegin( BOOL);
static FILE* LogEnd( void);

#if OPT_SERVICE
static VOID WINAPI ServiceMain( DWORD argc, LPTSTR argv[] );
static VOID WINAPI ServiceCtrlHandler( DWORD opcode);
static DWORD ServiceInit( void);
static void ServiceStop( void);
static void ServiceContine( void);
static void ServicePause( void);
static void ServiceParamChange( void);
typedef DWORD FRegisterServiceProcess( DWORD dwProcessId, DWORD dwType);

#else
static void OnSigInt( int);
#endif


/***************************************************************************
 * Module data
 ***************************************************************************/

sig_atomic_t volatile g_exit;

#if OPT_SERVICE
static SERVICE_STATUS s_ServiceStatus;
static SERVICE_STATUS_HANDLE s_hServiceStatus;
static HANDLE s_hThread;
static int s_iParamsChanged;
#endif

static FILE* s_logFile;

/* Config data */
#ifdef _DEBUG
#define kszConfigFile kszApp "Dbg.ini"
#else
#define kszConfigFile kszApp ".ini"
#endif
#define kszOptionsSection       kszApp
#define kszOptionPort             "port"
#define kszOptionLogfileName      "log_file"
static char s_szLog[ _MAX_PATH];
#define kszOptionLogfileVerbosity "log_detail"
static int s_iVerbosity;


/***************************************************************************
 * Functions
 ***************************************************************************/

/*
 * M A I N
 */
#if OPT_SERVICE
int WINAPI WinMain(
  HINSTANCE hInstance,      // handle to current instance
  HINSTANCE hPrevInstance,  // handle to previous instance
  LPSTR lpCmdLine,          // command line
  int nCmdShow              // show state
) {
  (void)hInstance;
  (void)hPrevInstance;
  (void)lpCmdLine;
  (void)nCmdShow;
#else
int main( void)
  {
#endif
#if OPT_FIND_LEAKS
  /* Debug heap options - define _CRTDBG_MAP_ALLOC for line numbers */
  {
  int flag = _CrtSetDbgFlag( _CRTDBG_REPORT_FLAG );
  flag |= _CRTDBG_LEAK_CHECK_DF; /* Enable leak detection */
  flag |= _CRTDBG_CHECK_ALWAYS_DF; /* Verify heap integrity on every call */
  flag |= _CRTDBG_DELAY_FREE_MEM_DF; /* Dont reuse freed blocks */
  _CrtSetDbgFlag( flag );
  }
#endif

  /* Try to open ini file in cwd */
  if ( !SWS_CfgSetName( kszConfigFile, NULL))
    {
    /* Look for ini file in path to program */
    const char* pszCmdLine = GetCommandLineA();

    if ( SWS_CfgSetName( kszConfigFile, pszCmdLine))
      {
      char szBuf[ MAX_PATH];
      int i;

      /* Get fullpath to program */
      if ( '"' == *pszCmdLine)
        i = sscanf( pszCmdLine + 1, "%255[^\"]", szBuf);
      else
        i = sscanf( pszCmdLine, "%255s", szBuf);

      if ( 1 == i)
        {
        /* Set cwd to match ini file */
        char* psz = strrchr( szBuf, '\\');
        if ( NULL != psz)
          *psz = '\0';

        SetCurrentDirectoryA( szBuf);
        }
      }
    }

#if OPT_SERVICE
  {
  OSVERSIONINFO verinfo;

  verinfo.dwOSVersionInfoSize = sizeof( verinfo);
  GetVersionEx( &verinfo);

  /* Check OS version */
  switch ( verinfo.dwPlatformId)
    {
  case VER_PLATFORM_WIN32_WINDOWS:
    /* Win 95/98/ME */
    {
    HMODULE const hmod = LoadLibrary( "kernel32.dll");
    if ( NULL != hmod)
      {
      FRegisterServiceProcess *pfn = (FRegisterServiceProcess*) GetProcAddress(
        hmod, "RegisterServiceProcess");
      if ( pfn)
        (*pfn)( 0, 1); /* Register current process as a service */

      FreeLibrary( hmod);
      }

    if ( NO_ERROR != ServiceRun( NULL))
      return EXIT_FAILURE;
    }
    break;

  case VER_PLATFORM_WIN32_NT:
    /* NT3.1, NT4, Win2k, XP */
    {
    static const SERVICE_TABLE_ENTRY DispatchTable[] = {
      { TEXT( kszApp), &ServiceMain },
      { NULL,   0 }
    };

    s_ServiceStatus.dwServiceType
      = SERVICE_INTERACTIVE_PROCESS | SERVICE_WIN32_OWN_PROCESS;

    s_ServiceStatus.dwControlsAccepted
      = (1* SERVICE_ACCEPT_STOP)
      | (1* SERVICE_ACCEPT_PAUSE_CONTINUE)
    ;
    /* SERVICE_ACCEPT_PARAMCHANGE requires Win2k and later
     * NT4 won't control the service if it's included */
    if ( verinfo.dwMajorVersion >= 5)
      s_ServiceStatus.dwControlsAccepted |= SERVICE_ACCEPT_PARAMCHANGE;

    /* NB This call doesn't return until all services have terminated */
    if ( !StartServiceCtrlDispatcher( &DispatchTable[0] ))
      return EXIT_FAILURE;
    }
    break;

  default:
    /* OS not supported */
    return EXIT_FAILURE;
    }
  }
#else
  /* Register a ctrl-c handler to signal exit */
  signal( SIGINT, &OnSigInt);

  /* Run the service */
  if ( NO_ERROR != ServiceRun( NULL))
    return EXIT_FAILURE;
#endif

  return 0;
  }


/*
 * Execute the service
 */
DWORD WINAPI ServiceRun( LPVOID pv)
  {
  struct SParams* const pParams = (struct SParams*)pv;
  int iRet;
  struct sockaddr_in host;
  FILE* logFile;

  iRet = SockInit();
  if ( 0 != iRet)
    return iRet;

#if OPT_SERVICE
loop:
#endif

  /* Get config options */
  host.sin_family = AF_INET;
  host.sin_addr.s_addr = htonl( INADDR_ANY);
  host.sin_port = htons( (u_short) SWS_CfgGetValue(
    kszOptionsSection, kszOptionPort, 0));

  SWS_CfgGetString(
    kszOptionsSection, kszOptionLogfileName, "", s_szLog, sizeof(s_szLog) );
  s_iVerbosity = (int) SWS_CfgGetValue(
    kszOptionsSection, kszOptionLogfileVerbosity, 1);

  logFile = LogBegin( NULL != pParams ? pParams->bNewLog : 0);

  /* Close the config file */
  SWS_CfgSetName( "", NULL);

  /* Run the server */
  iRet = Identd(
    (struct sockaddr*) &host,
    s_iVerbosity,
    logFile
  );
  if ( 0 != iRet)
    {
    /* Report failure */
    if ( NULL != logFile)
      {
      char szLog[ _MAX_PATH];

      if ( FormatMessage(
          FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
          NULL,
          iRet,
          0, /* Default language */
          szLog, sizeof( szLog),
          NULL
        )
      ) {
        fprintf( logFile, kszInfo " service failed: %s", szLog);
        }
      else
        {
        fprintf( logFile, kszInfo " service failed: %lu.\n", iRet);
        }
      }
    }

#if OPT_SERVICE
  if ( s_iParamsChanged)
    {
    s_iParamsChanged = 0;
    g_exit = 0;

    if ( NULL != logFile)
      {
      time_t const t = time( NULL);
      fprintf( logFile, kszInfo " parameters changed %s", ctime( &t) );
      }

    /* Open the updated ini file */
    SWS_CfgSetName( kszConfigFile, GetCommandLineA());

    /* Restart server */
    goto loop;
    }
#endif

  LogEnd();

  return iRet;
  }


/*
 * Open new log file
 */
static FILE* LogBegin( BOOL bNewLog)
  {
  FILE* logFile = NULL;

  LogEnd();

  if ( s_szLog[0] )
    {
    /* Open the logfile */
    logFile = fopen( s_szLog, bNewLog ? "w" : "a");
    }
#if !OPT_SERVICE
  else
    {
    logFile = stdout;
    }
#endif

  if ( NULL != logFile)
    {
    time_t const t = time( NULL);

    /* Make logfile unbuffered */
    setvbuf( logFile, NULL, _IONBF, 0 );

#if 0 && OPT_SERVICE
    /* Redirect stderr to log */
    setvbuf( stderr, NULL, _IONBF, 0 );
    if ( logFile != stdout)
      _dup2( _fileno( logFile ), 2);
#endif

    fprintf( logFile, "\n" kszInfo "\nLog started %s", ctime( &t) );
    }

  //SmtpdSetLog( s_iVerbosity, logFile);

  return s_logFile = logFile;
  }


static FILE* LogEnd( void)
  {
  FILE* logFile = s_logFile;
  if ( NULL != logFile)
    {
    time_t const t = time( NULL);

    s_logFile = NULL;
    //SmtpdSetLog( 0, NULL);
    fprintf( logFile, "Log stopped %s", ctime( &t) );

    if ( logFile != stderr && logFile != stdout)
      fclose( logFile);
    }

  return logFile;
  }


#if OPT_SERVICE
/*
 * Service main
 */
VOID WINAPI ServiceMain( DWORD argc, LPTSTR argv[] )
  {
  DWORD status;
  struct SParams params;

  s_ServiceStatus.dwCurrentState       = SERVICE_START_PENDING;
  s_ServiceStatus.dwCheckPoint         = 0;
  s_ServiceStatus.dwWaitHint           = 100; /* millisecs */
  s_ServiceStatus.dwWin32ExitCode      = NO_ERROR;

#if 0 && !defined NDEBUG
  DebugBreak();
#endif

  /* Register control handler */
  s_hServiceStatus = RegisterServiceCtrlHandler( TEXT( kszApp), &ServiceCtrlHandler);
  if ( 0 == s_hServiceStatus)
    return;

  SetServiceStatus( s_hServiceStatus, &s_ServiceStatus);

  /* Initialise */
  status = ServiceInit();
  if ( NO_ERROR != status)
    {
    s_ServiceStatus.dwCurrentState       = SERVICE_STOPPED;
    s_ServiceStatus.dwCheckPoint         = 0;
    s_ServiceStatus.dwWaitHint           = 0;
    s_ServiceStatus.dwWin32ExitCode      = status;
    SetServiceStatus( s_hServiceStatus, &s_ServiceStatus);
    return;
    }

  /* Report running status */
  s_ServiceStatus.dwCurrentState       = SERVICE_RUNNING;
  s_ServiceStatus.dwCheckPoint         = 0;
  s_ServiceStatus.dwWaitHint           = 0;
  SetServiceStatus( s_hServiceStatus, &s_ServiceStatus);

  /* Run the service */
  params.bNewLog = 2 == argc && 0 == stricmp( argv[ 1], "newlog");
  status = ServiceRun( &params);

  /* Update service status to stopped */
  s_ServiceStatus.dwCurrentState       = SERVICE_STOPPED;
  s_ServiceStatus.dwCheckPoint         = 0;
  s_ServiceStatus.dwWaitHint           = 0;
  s_ServiceStatus.dwWin32ExitCode      = status;
  SetServiceStatus( s_hServiceStatus, &s_ServiceStatus);

  CloseHandle( s_hThread);
  s_hThread = NULL;
  }


/*
 * Service control
 */
VOID WINAPI ServiceCtrlHandler( DWORD opcode)
  {
  switch ( opcode)
    {
  case SERVICE_CONTROL_PAUSE:
    s_ServiceStatus.dwCurrentState = SERVICE_PAUSED;
    ServicePause();
    break;

  case SERVICE_CONTROL_CONTINUE:
    s_ServiceStatus.dwCurrentState = SERVICE_RUNNING;
    ServiceContine();
    break;

  case SERVICE_CONTROL_STOP:
    s_ServiceStatus.dwCurrentState  = SERVICE_STOP_PENDING;
    s_ServiceStatus.dwWin32ExitCode = NO_ERROR;
    s_ServiceStatus.dwCheckPoint    = 0;
    s_ServiceStatus.dwWaitHint      = 1000; /* millisecs */
    ServiceStop();
    break;

  case 128:
    /* Close existing log file */
    if ( NULL != LogEnd() && s_szLog[0])
      {
      char szBuf[ _MAX_PATH], *psz;
      unsigned len, u;

      /* Get log filename without extension */
      strcpy( szBuf, s_szLog);
      psz = strrchr( szBuf, '.');
      if ( NULL != psz)
        len = psz - szBuf;
      else
        len = strlen( szBuf);

      /* Form backup filename */
      for ( u = 0; u < 1000; ++u)
        {
        FILE* file;

        sprintf( szBuf, "%.*s.%03u", (int)len, szBuf, u);
        file = fopen( szBuf, "r");
        if ( NULL == file)
          {
          /* Rename old log */
          rename( s_szLog, szBuf);
          break;
          }

        fclose( file);
        }
      }

    /* New log file */
#if 1
    ServiceParamChange();
#else
    LogBegin( TRUE);
#endif
    break;

  case SERVICE_CONTROL_PARAMCHANGE:
    ServiceParamChange();
    break;

  case SERVICE_CONTROL_INTERROGATE:
  case SERVICE_CONTROL_SHUTDOWN:
  default:
    break;
    }

  SetServiceStatus( s_hServiceStatus, &s_ServiceStatus);
  }


/*
 * Start the service
 */
DWORD ServiceInit( void)
  {
  if ( !DuplicateHandle(
      GetCurrentProcess(),
      GetCurrentThread(),
      GetCurrentProcess(),
      &s_hThread,
      THREAD_ALL_ACCESS,
      FALSE,
      0
    )
  ) {
    return GetLastError();
    }

  return NO_ERROR;
  }


/*
 * Stop the services
 */
void ServiceStop( void)
  {
  while ( ResumeThread( s_hThread) >= 1)
    ;
  g_exit = 1;
  }


/*
 * Pause the services
 */
void ServicePause( void)
  {
  SuspendThread( s_hThread);
  }


/*
 * Resume the services
 */
void ServiceContine( void)
  {
  while ( ResumeThread( s_hThread) >= 1)
    ;
  }


/*
 * Parameters chaned
 */
void ServiceParamChange( void)
  {
  s_iParamsChanged = 1;
  g_exit = 1;
  }
#else /*!OPT_SERVICE*/

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

#endif /*!OPT_SERVICE*/


/*
 * Initialise the sockets library
 */
static int SockInit( void)
  {
  static int s_inited;

  if ( !s_inited)
    {
    int iRet;
    WSADATA wsaData;

    iRet = WSAStartup( 0x202, &wsaData);
    if ( iRet)
      {
      fprintf( s_logFile ? s_logFile : stderr, "WSAStartup failed %d\n", iRet);
      return iRet;
      }

    s_inited = 1;

    /* Register an exit handler */
    atexit( &OnExit);
    }

  return 0;
  }


/*
 * Exit handler
 */
static void OnExit( void)
  {
  WSACleanup();
  }

/* End of file */
