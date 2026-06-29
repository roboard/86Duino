/*****************************************************************************
 *
 * File name   : sws_ftp.c
 * Function    : FTP client (RFC 959)
 * Authors     : Lawrence Rust
 * Systems     : ANSI C, sockets
 *
 *****************************************************************************
 * File History
 * ------------
 *
 * 09-Jul-05, LVR, Created.
 *
 *
 * Comments
 * --------
 *
 *****************************************************************************/
/* Compilation options */
#ifndef SWS_USE_SOCKLIB
#define SWS_USE_SOCKLIB 0 /* 0= native sockets, !0 for sws_sock */
#endif

/* Exports */
#include "sws_ftp.h"

/* Imports */

/* ANSI C library */
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <time.h>
#include <stdarg.h>
#include <assert.h>
#if _MSC_VER >= 1000
#include <crtdbg.h>
#endif

#include "sws_net.h"


/***************************************************************************
 * Macros & constants
 ***************************************************************************/

#ifndef SWS_MAKEFOURCC
#define SWS_MAKEFOURCC( ch0, ch1, ch2, ch3) (\
  (ui32)(ui8)(ch0) | ((ui32)(ui8)(ch1) << 8) | \
  ((ui32)(ui8)(ch2) << 16) | ((ui32)(ui8)(ch3) << 24) )
#endif

#define CRLF  "\x0d\x0a"
#define cNul  '\0'

#ifndef MSG_NOSIGNAL
#define MSG_NOSIGNAL 0
#endif

/* RFC 959 constants */
#define FTP_PORT    21U

/* FTP timeouts */
#define INIT_TIMEOUT (1 * 60)     /* Seconds ~*/
#define CONN_TIMEOUT (15)         /* Seconds for data accept ~*/
#define GEN_TIMEOUT  (1 * 60)     /* Seconds ~*/
#define SEND_TIMEOUT (30 * 1000)  /* millisecs */


/***************************************************************************
 * Data types
 ***************************************************************************/

#define FTP_MAGIC SWS_MAKEFOURCC('d','n','s','q')
#define IsValidClient( h) (NULL != h && FTP_MAGIC == h->magic)

/* Client */
typedef struct SWS_FtpClient
  {
  ui32 magic;

  FILE* logfile;
  int iVerbosity;

  struct sockaddr_in controlAddr;
  SOCKET s;                 /* Control connection */

  int isPasv;
  struct sockaddr_in dataAddr; /* Data connection */

  SWS_FNPTR( SWS_FFtpGet) pfnResponse;
  SWS_PTR( void) pvResponse;

  char szBuf[ 4096];
  } SWS_FtpClient;


/***************************************************************************
 * Local function prototypes
 ***************************************************************************/

#ifdef __GNUC__
#define FORMAT(a,f,l)  __attribute__ ((format (a, f, l)))
#else
#define FORMAT(a,f,l)
#endif
static int LogPrintf( SWS_FtpClientH, const char*, ...) FORMAT(printf,2,3);

static int DoCommand( SWS_FtpClientH, const char*);
static int GetResponse( SWS_FtpClientH, unsigned);

static int NewDataSocket( SWS_FtpClientH, SOCKET*);
static int OpenDataConnection( SWS_FtpClientH, SOCKET*);

static int ReadLine( SWS_FtpClientH, char*, size_t, unsigned);
static int ReadData(
  SWS_FtpClientH, SOCKET, SWS_FNPTR( SWS_FFtpGet), SWS_PTR( void), unsigned);
static int WriteData(
  SWS_FtpClientH, SOCKET, SWS_FNPTR( SWS_FFtpPut), SWS_PTR( void) );
static int RawRead( SOCKET, void*, size_t, unsigned);
static int RawWrite( SOCKET, const void*, size_t);

static int ConnectAsync( SOCKET, struct sockaddr*, int);
static SOCKET AcceptAsync( SOCKET, struct sockaddr*r, socklen_t*, unsigned);


/***************************************************************************
 * Module data
 ***************************************************************************/


/***************************************************************************
 * External Functions
 ***************************************************************************/

/*
 * Allocate a client handle
 */
SWS_EXTERN( SWS_FtpClientH, FtpClientCreate(
  FILE* logfile,                      /* IN: log file, NULL= none */
  int iVerbosity                      /* IN: log verbosity */
)){
  SWS_FtpClientH hClient;

  /* Allocate a client handle */
  hClient = (SWS_FtpClientH) calloc( 1, sizeof( SWS_FtpClient));
  if ( NULL == hClient)
    return NULL;

  hClient->magic = FTP_MAGIC;
  hClient->logfile = logfile;
  hClient->iVerbosity = iVerbosity;
  hClient->controlAddr.sin_family = AF_UNSPEC;
  hClient->controlAddr.sin_port = 0;
  hClient->controlAddr.sin_addr.s_addr = INADDR_NONE;
  hClient->s = INVALID_SOCKET;
  hClient->isPasv = 0;
  hClient->dataAddr.sin_family = AF_UNSPEC;
  hClient->dataAddr.sin_port = 0;
  hClient->dataAddr.sin_addr.s_addr = INADDR_NONE;
  hClient->pfnResponse = 0;
  hClient->pvResponse = 0;

  return hClient;
  }


/*
 * Deallocate a client handle
 */
SWS_EXTERN( void, FtpClientDestroy(
  SWS_FtpClientH hClient              /* IN, client handle */
)){
  if ( IsValidClient( hClient))
    {
    hClient->magic = 0;

    if ( INVALID_SOCKET != hClient->s)
      {
      closesocket( hClient->s);
      hClient->s = INVALID_SOCKET;
      }

    free( hClient);
    }
  }


/*
 * Set command output callback
 */
SWS_EXTERN( int, FtpSetCallback(
  SWS_FtpClientH hClient,             /* IN, client handle */
  SWS_FNPTR( SWS_FFtpGet) pfn,        /* IN: callback */
  SWS_PTR( void) pv                   /* IN: callback context */
)){
  if ( !IsValidClient( hClient))
    return EINVAL;

  hClient->pfnResponse = pfn;
  hClient->pvResponse = pv;
  return 0;
  }


/*
 * Connect client to server
 */
SWS_EXTERN( int, FtpConnect(
  SWS_FtpClientH hClient,             /* IN, client handle */
  SWS_PTR( const char) pszServer,     /* IN: FTP server hostname */
  unsigned uPort                      /* IN: FTP server port, 0= default */
)){
  int iRet;
  socklen_t len;
  struct hostent* pHost;

  assert( NULL != pszServer);

  if ( !IsValidClient( hClient))
    return EINVAL;

  if ( INVALID_SOCKET != hClient->s)
    return EISCONN;

  hClient->controlAddr.sin_port = htons( (u_short)(uPort ? uPort : FTP_PORT) );
  hClient->controlAddr.sin_addr.s_addr = inet_addr( pszServer );
  if ( INADDR_ANY == hClient->controlAddr.sin_addr.s_addr)
    {
    return EINVAL;
    }
  else if ( INADDR_NONE != hClient->controlAddr.sin_addr.s_addr)
    {
    hClient->controlAddr.sin_family = AF_INET;
    }
  /* Try a hostname */
  else if ( NULL != (pHost = gethostbyname( pszServer )) )
    {
    hClient->controlAddr.sin_family = pHost->h_addrtype;
    memcpy( &hClient->controlAddr.sin_addr, pHost->h_addr, pHost->h_length);
    }
  else
    {
    return EHOST_NOT_FOUND;
    }

  /* Create a socket */
  hClient->s = socket( hClient->controlAddr.sin_family, SOCK_STREAM, 0);
  if ( INVALID_SOCKET == hClient->s)
    {
    iRet = GetLastError();
    return iRet;
    }

  /* Set a send/connect timeout */
  { DECL_TIMEOUT( tv, SEND_TIMEOUT);
  setsockopt(
    hClient->s,
    SOL_SOCKET, SO_SNDTIMEO,
    (char*)&tv, sizeof tv
  ); }

  /* Connect to server */
  if ( hClient->iVerbosity > 1)
    {
    LogPrintf( hClient, "Connecting to %s [%s]:%u\n",
      pszServer,
      inet_ntoa( hClient->controlAddr.sin_addr),
      ntohs( hClient->controlAddr.sin_port)
    );
    }

  iRet = ConnectAsync(
    hClient->s,
    (struct sockaddr*)&hClient->controlAddr,
    sizeof(hClient->controlAddr)
  );
  if ( 0 != iRet)
    {
    closesocket( hClient->s);
    hClient->s = INVALID_SOCKET;
    return iRet;
    }

  /* Get local addr/port */
  len = sizeof( hClient->dataAddr);
  getsockname( hClient->s, (struct sockaddr*)&hClient->dataAddr, &len);

  /* Get server's signon message */
  return GetResponse( hClient, INIT_TIMEOUT);
  }


/*
 * Disconnect client from server
 */
SWS_EXTERN( int, FtpDisconnect(
  SWS_FtpClientH hClient             /* IN, client handle */
)){
  if ( !IsValidClient( hClient))
    return EINVAL;
  if ( INVALID_SOCKET == hClient->s)
    return EINVAL;

  if ( hClient->iVerbosity > 0)
    {
    LogPrintf( hClient, "Disconnecting %s:%u\n",
      inet_ntoa( hClient->controlAddr.sin_addr),
      ntohs( hClient->controlAddr.sin_port)
    );
    }

  closesocket( hClient->s);
  hClient->s = INVALID_SOCKET;

  return 0;
  }


/*
 * NOOP
 */
SWS_EXTERN( int, FtpNoop(
  SWS_FtpClientH hClient              /* IN, client handle */
)){
  if ( !IsValidClient( hClient))
    return EINVAL;
  if ( INVALID_SOCKET == hClient->s)
    return EINVAL;

  return DoCommand( hClient, "NOOP");
  }


/*
 * USER
 */
SWS_EXTERN( int, FtpUser(
  SWS_FtpClientH hClient,             /* IN, client handle */
  SWS_PTR( const char) psz            /* IN: username */
)){
  if ( !IsValidClient( hClient))
    return EINVAL;
  if ( INVALID_SOCKET == hClient->s)
    return EINVAL;

  sprintf( hClient->szBuf, "USER%s%.255s", *psz ? " " : "", psz);
  return DoCommand( hClient, hClient->szBuf);
  }


/*
 * PASS
 */
SWS_EXTERN( int, FtpPass(
  SWS_FtpClientH hClient,             /* IN, client handle */
  SWS_PTR( const char) psz            /* IN: password */
)){
  if ( !IsValidClient( hClient))
    return EINVAL;
  if ( INVALID_SOCKET == hClient->s)
    return EINVAL;

  sprintf( hClient->szBuf, "PASS%s%.255s", *psz ? " " : "", psz);
  return DoCommand( hClient, hClient->szBuf);
  }


/*
 * ACCT
 */
SWS_EXTERN( int, FtpAcct(
  SWS_FtpClientH hClient,             /* IN, client handle */
  SWS_PTR( const char) psz            /* IN: account name */
)){
  if ( !IsValidClient( hClient))
    return EINVAL;
  if ( INVALID_SOCKET == hClient->s)
    return EINVAL;

  sprintf( hClient->szBuf, "ACCT%s%.255s", *psz ? " " : "", psz);
  return DoCommand( hClient, hClient->szBuf);
  }


/*
 * QUIT
 */
SWS_EXTERN( int, FtpQuit(
  SWS_FtpClientH hClient              /* IN, client handle */
)){
  if ( !IsValidClient( hClient))
    return EINVAL;
  if ( INVALID_SOCKET == hClient->s)
    return EINVAL;

  return DoCommand( hClient, "QUIT");
  }


/*
 * Use passive mode
 */
SWS_EXTERN( int, FtpPasv(
  SWS_FtpClientH hClient,             /* IN, client handle */
  int isPasv                          /* IN: !0 for passive mode */
)){
  if ( !IsValidClient( hClient))
    return EINVAL;
  if ( INVALID_SOCKET == hClient->s)
    return EINVAL;

  hClient->isPasv = isPasv;
  return 0;
  }


/*
 * Binary transfer mode
 */
SWS_EXTERN( int, FtpType(
  SWS_FtpClientH hClient,             /* IN, client handle */
  int iMode                           /* IN: 0= ASCII, 1= binary (default) */
)){
  if ( !IsValidClient( hClient))
    return EINVAL;
  if ( INVALID_SOCKET == hClient->s)
    return EINVAL;

  sprintf( hClient->szBuf, "TYPE %s", iMode ? "I" : "A T");
  return DoCommand( hClient, hClient->szBuf);
  }


/*
 * Site info
 */
SWS_EXTERN( int, FtpSyst(
  SWS_FtpClientH hClient,             /* IN, client handle */
  int len,
  SWS_PTR( char) psz                  /* OUT: */
)){
  int iRet;

  if ( !IsValidClient( hClient))
    return EINVAL;
  if ( INVALID_SOCKET == hClient->s)
    return EINVAL;

  iRet = DoCommand( hClient, "SYST");
  if ( 215 == iRet && len > 0)
    {
    char szFmt[ 16];
    sprintf( szFmt, "%%*u %%%u[^\n]", len);
    *psz = cNul;
    sscanf( hClient->szBuf, szFmt, psz);
    }

  return iRet;
  }


/*
 * Site help
 */
SWS_EXTERN( int, FtpHelp(
  SWS_FtpClientH hClient,             /* IN, client handle */
  SWS_PTR( const char) psz            /* IN: account name */
)){
  if ( !IsValidClient( hClient))
    return EINVAL;
  if ( INVALID_SOCKET == hClient->s)
    return EINVAL;

  sprintf( hClient->szBuf, "HELP%s%.255s", *psz ? " " : "", psz);
  return DoCommand( hClient, hClient->szBuf);
  }


/*
 * Literal command
 */
SWS_EXTERN( int, FtpCmd(
  SWS_FtpClientH hClient,             /* IN, client handle */
  SWS_PTR( const char) psz            /* IN: account name */
)){
  if ( !IsValidClient( hClient))
    return EINVAL;
  if ( INVALID_SOCKET == hClient->s)
    return EINVAL;

  return DoCommand( hClient, psz);
  }


/*
 * Get a file
 */
SWS_EXTERN( int, FtpGet(
  SWS_FtpClientH hClient,             /* IN, client handle */
  SWS_PTR( const char) psz,           /* IN: source filename on server */
  SWS_FNPTR( SWS_FFtpGet) pfn,        /* IN: callback */
  SWS_PTR( void) pv                   /* IN: callback context */
)){
  int iRet;
  SOCKET s;

  if ( !IsValidClient( hClient))
    return EINVAL;
  if ( INVALID_SOCKET == hClient->s)
    return EINVAL;

  iRet = NewDataSocket( hClient, &s);
  if ( iRet)
    return iRet;

  sprintf( hClient->szBuf, "RETR %.255s", psz);
  iRet = DoCommand( hClient, hClient->szBuf);
  if ( iRet < 200 && iRet >= 100)
    {
    iRet = OpenDataConnection( hClient, &s);
    if ( 0 == iRet)
      {
      ReadData( hClient, s, pfn, pv, GEN_TIMEOUT);
      shutdown( s, SD_SEND);
      iRet = GetResponse( hClient, GEN_TIMEOUT);
      }
    }

  if ( INVALID_SOCKET != s)
    closesocket( s);

  return iRet;
  }


/*
 * Put a file
 */
SWS_EXTERN( int, FtpPut(
  SWS_FtpClientH hClient,             /* IN, client handle */
  SWS_PTR( const char) psz,           /* IN: target filename on server */
  SWS_FNPTR( SWS_FFtpPut) pfn,        /* IN: callback */
  SWS_PTR( void) pv                   /* IN: callback context */
)){
  int iRet;
  SOCKET s;

  if ( !IsValidClient( hClient))
    return EINVAL;
  if ( INVALID_SOCKET == hClient->s)
    return EINVAL;

  iRet = NewDataSocket( hClient, &s);
  if ( iRet)
    return iRet;

  sprintf( hClient->szBuf, "STOR %.255s", psz);
  iRet = DoCommand( hClient, hClient->szBuf);
  if ( iRet < 200 && iRet >= 100)
    {
    iRet = OpenDataConnection( hClient, &s);
    if ( 0 == iRet)
      {
      WriteData( hClient, s, pfn, pv);
      shutdown( s, SD_SEND);
      iRet = GetResponse( hClient, GEN_TIMEOUT);
      }
    }

  if ( INVALID_SOCKET != s)
    closesocket( s);

  return iRet;
  }


/*
 * Rename file
 */
SWS_EXTERN( int, FtpRen(
  SWS_FtpClientH hClient,             /* IN, client handle */
  SWS_PTR( const char) pszOld,        /* IN: old filename on server */
  SWS_PTR( const char) pszNew         /* IN: new filename on server */
)){
  int iRet;

  if ( !IsValidClient( hClient))
    return EINVAL;
  if ( INVALID_SOCKET == hClient->s)
    return EINVAL;

  sprintf( hClient->szBuf, "RNFR %.255s", pszOld);
  iRet = DoCommand( hClient, hClient->szBuf);
  if ( 350 == iRet)
    {
    sprintf( hClient->szBuf, "RNTO %.255s", pszNew);
    iRet = DoCommand( hClient, hClient->szBuf);
    }

  return iRet;
  }


/*
 * Delete file
 */
SWS_EXTERN( int, FtpDel(
  SWS_FtpClientH hClient,             /* IN, client handle */
  SWS_PTR( const char) psz            /* IN: filename on server */
)){
  if ( !IsValidClient( hClient))
    return EINVAL;
  if ( INVALID_SOCKET == hClient->s)
    return EINVAL;

  sprintf( hClient->szBuf, "DELE %.255s", psz);
  return DoCommand( hClient, hClient->szBuf);
  }


/*
 * Directory maintenance
 */
SWS_EXTERN( int, FtpCwd(
  SWS_FtpClientH hClient,             /* IN, client handle */
  SWS_PTR( const char) psz            /* IN: path */
)){
  if ( !IsValidClient( hClient))
    return EINVAL;
  if ( INVALID_SOCKET == hClient->s)
    return EINVAL;

  sprintf( hClient->szBuf, "CWD%s%.255s", *psz ? " " : "", psz);
  return DoCommand( hClient, hClient->szBuf);
  }


SWS_EXTERN( int, FtpCdup(
  SWS_FtpClientH hClient              /* IN, client handle */
)){
  if ( !IsValidClient( hClient))
    return EINVAL;
  if ( INVALID_SOCKET == hClient->s)
    return EINVAL;

  return DoCommand( hClient, "CDUP");
  }


SWS_EXTERN( int, FtpPwd(
  SWS_FtpClientH hClient,             /* IN, client handle */
  int len,
  SWS_PTR( char) psz                  /* OUT: path */
)){
  int iRet;

  if ( !IsValidClient( hClient))
    return EINVAL;
  if ( INVALID_SOCKET == hClient->s)
    return EINVAL;
  if ( len > 0 && NULL == psz)
    return EINVAL;

  iRet = DoCommand( hClient, "PWD");
  if ( 257 == iRet && len > 0)
    {
    char szFmt[ 16];
    sprintf( szFmt, "%%*u \"%%%u[^\"]", len);
    *psz = cNul;
    sscanf( hClient->szBuf, szFmt, psz);
    }

  return iRet;
  }


SWS_EXTERN( int, FtpMkd(
  SWS_FtpClientH hClient,             /* IN, client handle */
  SWS_PTR( const char) psz            /* IN: path */
)){
  if ( !IsValidClient( hClient))
    return EINVAL;
  if ( INVALID_SOCKET == hClient->s)
    return EINVAL;

  sprintf( hClient->szBuf, "MKD%s%.255s", *psz ? " " : "", psz);
  return DoCommand( hClient, hClient->szBuf);
  }


SWS_EXTERN( int, FtpRmd(
  SWS_FtpClientH hClient,             /* IN, client handle */
  SWS_PTR( const char) psz            /* IN: path */
)){
  if ( !IsValidClient( hClient))
    return EINVAL;
  if ( INVALID_SOCKET == hClient->s)
    return EINVAL;

  sprintf( hClient->szBuf, "RMD%s%.255s", *psz ? " " : "", psz);
  return DoCommand( hClient, hClient->szBuf);
  }


/*
 * Directory listing
 */
SWS_EXTERN( int, FtpList(
  SWS_FtpClientH hClient,             /* IN, client handle */
  SWS_PTR( const char) psz,           /* IN: filespec */
  SWS_FNPTR( SWS_FFtpGet) pfn,        /* IN: data callback */
  SWS_PTR( void) pv                   /* IN: callback context */
)){
  int iRet;
  SOCKET s;

  if ( !IsValidClient( hClient))
    return EINVAL;
  if ( INVALID_SOCKET == hClient->s)
    return EINVAL;

  iRet = NewDataSocket( hClient, &s);
  if ( iRet)
    return iRet;

  sprintf( hClient->szBuf, "LIST%s%.255s", *psz ? " " : "", psz);
  iRet = DoCommand( hClient, hClient->szBuf);
  if ( iRet < 200 && iRet >= 100)
    {
    iRet = OpenDataConnection( hClient, &s);
    if ( 0 == iRet)
      {
      ReadData(
        hClient, s, pfn, pv, GEN_TIMEOUT);
      iRet = GetResponse( hClient, GEN_TIMEOUT);
      }
    }

  if ( INVALID_SOCKET != s)
    closesocket( s);

  return iRet;
  }


/*
 * File listing
 */
SWS_EXTERN( int, FtpNlst(
  SWS_FtpClientH hClient,             /* IN, client handle */
  SWS_PTR( const char) psz,           /* IN: filespec */
  SWS_FNPTR( SWS_FFtpGet) pfn,        /* IN: data callback */
  SWS_PTR( void) pv                   /* IN: callback context */
)){
  int iRet;
  SOCKET s;

  if ( !IsValidClient( hClient))
    return EINVAL;
  if ( INVALID_SOCKET == hClient->s)
    return EINVAL;

  iRet = NewDataSocket( hClient, &s);
  if ( iRet)
    return iRet;

  sprintf( hClient->szBuf, "NLST%s%.255s", *psz ? " " : "", psz);
  iRet = DoCommand( hClient, hClient->szBuf);
  if ( iRet < 200 && iRet >= 100)
    {
    iRet = OpenDataConnection( hClient, &s);
    if ( 0 == iRet)
      {
      ReadData(
        hClient, s, pfn, pv, GEN_TIMEOUT);
      iRet = GetResponse( hClient, GEN_TIMEOUT);
      }
    }

  if ( INVALID_SOCKET != s)
    closesocket( s);

  return iRet;
  }


/*
 * File status
 */
SWS_EXTERN( int, FtpStat(
  SWS_FtpClientH hClient,             /* IN, client handle */
  SWS_PTR( const char) psz            /* IN: filespec */
)){
  if ( !IsValidClient( hClient))
    return EINVAL;
  if ( INVALID_SOCKET == hClient->s)
    return EINVAL;

  sprintf( hClient->szBuf, "STAT%s%.255s", *psz ? " " : "", psz);
  return DoCommand( hClient, hClient->szBuf);
  }


/***************************************************************************
 * Local Functions
 ***************************************************************************/

/*
 * Log output
 */
int LogPrintf( SWS_FtpClientH hClient, const char* pszFormat, ...)
  {
  int iRet;
  va_list va;
  char szBuf[ 128];

  va_start( va, pszFormat);

  if ( hClient->pfnResponse)
    {
    iRet = vsprintf( szBuf, pszFormat, va);
    (*hClient->pfnResponse)( hClient->pvResponse, hClient, szBuf, iRet);
    }


  iRet = 0;
  if ( NULL != hClient->logfile)
    {
    time_t const t = time( NULL);

    strftime( szBuf, sizeof(szBuf), "%Y/%m/%d %H:%M:%S", localtime( &t) );
    iRet += fprintf( hClient->logfile, "%s FTP: ", szBuf);
    iRet += vfprintf( hClient->logfile, pszFormat, va);
    }

  va_end( va);
  return iRet;
  }


/*
 * Issue a command
 */
static int DoCommand(
  SWS_FtpClientH hClient,
  const char* pszCmd
) {
  char buf[ 128];

  if ( hClient->iVerbosity >= 0)
    LogPrintf( hClient, "%s; ", pszCmd);

  /* Flush control input stream */
  setsockasync( hClient->s, 1);
  recv( hClient->s, buf, sizeof( buf), 0);
  setsockasync( hClient->s, 0);

  if ( SOCKET_ERROR == RawWrite( hClient->s, pszCmd, strlen( pszCmd) )
    || SOCKET_ERROR == RawWrite( hClient->s, CRLF, sizeof( CRLF) - 1)
  ) {
    LogPrintf( hClient, "Connection closed by server\n");
    return SOCKET_ERROR;
    }

  return GetResponse( hClient, GEN_TIMEOUT);
  }


/*
 * Get a server response
 */
static int GetResponse(
  SWS_FtpClientH hClient,            /* IN, client handle */
  unsigned timeout
) {
  time_t const t = time( NULL) + timeout;
  int eol = 1;
  int iResponse = -1;

  hClient->szBuf[0] = cNul;

  /* Until timeout */
  while ( time( NULL) <= t )
    {
    char c;
    int len;

    len = ReadLine( hClient, hClient->szBuf, sizeof(hClient->szBuf), timeout );
    if ( SOCKET_ERROR == len)
      {
      LogPrintf( hClient, "No data during read\n");
      iResponse = ETIMEDOUT;
      break;
      }

    if ( hClient->iVerbosity >= 0)
      LogPrintf( hClient, "%s", hClient->szBuf);

    if ( !eol)
      {
      /* Skip remainder of part line */
      if ( len > 0 && '\n' == hClient->szBuf[ len - 1])
        eol = 1;
      }
    else
      {
      int iCode;

      if ( len > 0 && '\n' == hClient->szBuf[ len - 1] )
        eol = 1;
      else
        eol = 0;

      /* Check for response code */
      c = ' ';
      if ( sscanf( hClient->szBuf, "%d%c", &iCode, &c) >= 1)
        {
        if ( ' ' == c)
          {
          if ( -1 == iResponse)
            {
            /* Single line response */
            iResponse = iCode;
            break;
            }
          if ( iCode == iResponse)
            {
            /* End of multi-line response */
            break;
            }
          }
        else if ( '-' == c)
          {
          /* Part of multi-line response */
          if ( -1 == iResponse)
            iResponse = iCode;
          }
        }
      }
    }

  return iResponse;
  }


/*
 * Create data socket
 */
int NewDataSocket(
  SWS_FtpClientH hClient,
  SOCKET* psData                      /* OUT: Data socket */
) {
  int iRet;
  SOCKET sData;

  assert( NULL != psData);
  *psData = INVALID_SOCKET;

  /* Get a socket */
  sData = socket( PF_INET, SOCK_STREAM, 0);
  if ( INVALID_SOCKET == sData)
    {
    iRet = GetLastError();
    LogPrintf( hClient, "%d Aborted - unable to create data socket\n", iRet);
    return iRet;
    }

  /* Passive mode? */
  if ( hClient->isPasv)
    {
    /* Yes, tell server, Reply 227= Entering passive mode */
    iRet = DoCommand( hClient, "PASV");
    if ( 227 == iRet)
      {
      unsigned u[ 6];

      /* Parse the address returned */
      if ( 6 == sscanf(
          hClient->szBuf,
          "%*u %*[^(]( %u , %u , %u , %u , %u , %u",
          u, u+1, u+2, u+3, u+4, u+5
        )
      ) {
        /* Init data port address */
        hClient->dataAddr.sin_family = AF_INET;
        hClient->dataAddr.sin_port = htons( (u_short)(u[4] * 256 + u[5]) );
        hClient->dataAddr.sin_addr.s_addr = htonl(
          ((u_long)u[0] << 24)
          | ((u_long)u[1] << 16)
          | ((u_long)u[2] << 8)
          | ((u_long)u[3])
        );
        iRet = 0;
        }
      else
        {
        iRet = EINVAL;
        LogPrintf( hClient, "Aborted - unrecognised reply\n");
        }
      }
    }
  else
    {
    /* Normal data connection */
    u_long const host = ntohl( hClient->dataAddr.sin_addr.s_addr);
    u_short port = ntohs( hClient->dataAddr.sin_port);
    u_short const port0 = port;

#if 0
    /* Enable address reuse */
    iRet = 1;
    setsockopt(
      sData,
      SOL_SOCKET, SO_REUSEADDR,
      (char*)&iRet, sizeof(iRet)
    );
#endif

    do
      {
      /* Incr data port for each connection */
      if ( 0 == port++)
        port = 1024;

      hClient->dataAddr.sin_port = htons( port);

      iRet = bind(
        sData,
        (struct sockaddr*)&hClient->dataAddr,
        sizeof( hClient->dataAddr)
      );
      }
    while ( iRet && port0 != port);

    if ( iRet)
      {
      iRet = GetLastError();
      LogPrintf(
        hClient, "%d Aborted - failed to bind socket to data port\n", iRet);
      }
    else if ( listen( sData, 1))
      {
      iRet = GetLastError();
      LogPrintf( hClient, "%d Aborted - listen failed\n", iRet);
      }
    else
      {
      /* Tell server our listening port address */
      sprintf( hClient->szBuf, "PORT %u,%u,%u,%u,%u,%u",
        (unsigned)((host >> 24) & 0xff),
        (unsigned)((host >> 16) & 0xff),
        (unsigned)((host >> 8) & 0xff),
        (unsigned)(host & 0xff),
        (port >> 8) & 0xff,
        port & 0xff
      );
      iRet = DoCommand( hClient, hClient->szBuf);
      if ( 200 == iRet)
        iRet = 0;
      }
    }

  if ( 0 == iRet)
    *psData = sData;
  else
    closesocket( sData);

  return iRet;
  }


/*
 * Make a data connection
 */
int OpenDataConnection(
  SWS_FtpClientH hClient,
  SOCKET* psData                      /* I/O Data connection */
) {
  int iRet;
  SOCKET sData;

  assert( NULL != psData);
  sData = *psData;
  assert( INVALID_SOCKET != sData);
  *psData = INVALID_SOCKET;

  if ( hClient->isPasv)
    {
    /* Passive mode so connect to server */
    iRet = ConnectAsync(
      sData, (struct sockaddr*)&hClient->dataAddr, sizeof(hClient->dataAddr));
    if ( 0 != iRet)
      {
      LogPrintf( hClient, "%d Failed to make PASV data connection\n", iRet);
      }
    }
  else
    {
    /* Normal mode so wait for connection from server */
    SOCKET s = AcceptAsync( sData, NULL, NULL, CONN_TIMEOUT );
    if ( INVALID_SOCKET != s)
      {
      /* Success */
      iRet = 0;
      }
    else
      {
      iRet = ETIMEDOUT;
      LogPrintf(
        hClient, "%d Failed to accept incoming data connection\n", iRet);
      }

    closesocket( sData);
    sData = s;
    }

  if ( INVALID_SOCKET != sData)
    {
    /* Disable Nagle */
    iRet = 1;
    setsockopt(
      sData,
      IPPROTO_TCP, TCP_NODELAY,
      (char*)&iRet, sizeof(iRet)
    );

    *psData = sData;
    iRet = 0;
    }

  return iRet;
  }


/*
 * Read data stream
 */
static int ReadData(
  SWS_FtpClientH hClient,            /* IN, client handle */
  SOCKET s,
  SWS_FNPTR( SWS_FFtpGet) pfn,       /* IN: callback */
  SWS_PTR( void) pv,                 /* IN: callback context */
  unsigned timeout
) {
  int iRet;

  do
    {
    /* Get a buffer */
    iRet = RawRead(
      s,
      hClient->szBuf, sizeof( hClient->szBuf),
      timeout
    );
    switch ( iRet)
      {
    case SOCKET_ERROR:
      LogPrintf( hClient, "Connection closed during read\n");
      return SOCKET_ERROR;

    case 0:
      return 0;
      }

    iRet = (*pfn)( pv, hClient, hClient->szBuf, iRet);
    }
  while ( 0 == iRet);

  return iRet;
  }


/*
 * Write data stream
 */
static int WriteData(
  SWS_FtpClientH hClient,            /* IN, client handle */
  SOCKET s,
  SWS_FNPTR( SWS_FFtpPut) pfn,       /* IN: callback */
  SWS_PTR( void) pv                  /* IN: callback context */
) {
  int iRet;

  for (;;)
    {
    /* Ask client to fill data buffer */
    iRet = (*pfn)( pv, hClient, hClient->szBuf, sizeof(hClient->szBuf) );
    if ( iRet <= 0)
      break;

    iRet = RawWrite(
      s,
      hClient->szBuf, iRet
    );
    if ( SOCKET_ERROR == iRet)
      {
      LogPrintf( hClient, "Connection closed during write\n");
      break;
      }
    }

  return iRet;
  }


/*
 * Read a line
 */
static int ReadLine(
  SWS_FtpClientH hClient,            /* IN, client handle */
  char* pszBuf,
  size_t size,
  unsigned timeout
) {
  unsigned char c;
  size_t count = 0;
  int iRet = 0;
  int gotCR = 0;

  while (
    count < size
    && SOCKET_ERROR != (iRet = RawRead( hClient->s, &c, 1, timeout) )
  ) {
    if ( 0 == iRet)
      return SOCKET_ERROR;

    if ( gotCR)
      {
      gotCR = 0;
      switch ( c)
        {
      case 0:
      case 10:
        pszBuf[ count++ ] = '\n';
        if ( count < size)
          pszBuf[ count] = '\0';
        return count;

      default:
        pszBuf[ count = 0] = (char)c;
        break;
        }
      }
    else switch ( c)
      {
    default:
      if ( /*isprint( c) &&*/ count + 1 <= size)
        pszBuf[ count++] = (char)c;
      break;

    case 10:
      pszBuf[ count++ ] = '\n';
      if ( count < size)
        pszBuf[ count] = '\0';
      return count;

    case 13:
      gotCR = 1;
      break;
      }
    }

  return iRet;
  }


/*
 * Non-blocking read data
 */
static int RawRead(
  SOCKET s,
  void* pv,
  size_t size,
  unsigned timeout
) {
  time_t const t = time( NULL) + timeout;

  /* Until timeout */
  do
    {
    fd_set set;
    int iRet;
    struct timeval tval;

    FD_ZERO( &set);
    FD_SET( s, &set);

    /* Set timeout for select */
    tval.tv_usec = 333 * 1000UL;
    tval.tv_sec = 0;

    /* Wait for a receive event or timeout */
    iRet = select( FD_SETSIZE, &set, NULL, NULL, &tval);
    if ( 0 == iRet)
      continue; /* Timeout */

    if ( SOCKET_ERROR == iRet)
      return SOCKET_ERROR;

    /* Process receive event */
    if ( FD_ISSET( s, &set))
      {
      iRet = recv( s, (char*)pv, size, 0);
      if ( SOCKET_ERROR == iRet)
        {
        int err = GetLastError();
        switch ( err)
          {
        case EWOULDBLOCK:
#if EAGAIN != EWOULDBLOCK
        case EAGAIN:
#endif
          continue;
        case ECONNRESET:
        case ECONNABORTED:
        case ENOTCONN:
        default:
          break;
          }
        }
      return iRet;
      }
    }
  while ( time( NULL) <= t );

  return SOCKET_ERROR;
  }


/*
 * Write raw data
 */
static int RawWrite(
  SOCKET s,
  const void* pv,
  size_t size
) {
  int iRet;

  iRet = send( s, (const char*)pv, size, MSG_NOSIGNAL);
  if ( SOCKET_ERROR == iRet)
    {
    int err = GetLastError();
    switch ( err)
      {
    case EWOULDBLOCK:
#if EAGAIN != EWOULDBLOCK
    case EAGAIN:
#endif
      /* Win98se sometimes returns EWOULDBLOCK even though select()
      * previously indicated socket is ready to be written */
      return 0;
    case ECONNRESET:
    case ECONNABORTED:
    case ETIMEDOUT:
    default:
      break;
      }
    }

  return iRet;
  }


/*
 * Non-blocking connect
 */
static int ConnectAsync( SOCKET s, struct sockaddr* pAddr, int len)
  {
  int iRet = 0;

  /* Enable non-blocking mode */
  setsockasync( s, 1);

  if ( connect( s, pAddr, len)
    && (EWOULDBLOCK == (iRet = GetLastError()) || EINPROGRESS == iRet || EAGAIN == iRet)
  ) {
    int count;

    for ( count = 0; ; ++count)
      {
      fd_set setw, sete;
      struct timeval tval;

      FD_ZERO( &setw);
      FD_SET( s, &setw);
      FD_ZERO( &sete);
      FD_SET( s, &sete);

      /* Set timeout for select */
      tval.tv_usec = 333 * 1000UL;
      tval.tv_sec = 0;

      /* Wait for a connect, error or timeout */
      iRet = select( FD_SETSIZE, NULL, &setw, &sete, &tval);
      if ( 0 == iRet)
        {
        /* Timeout */
        continue;
        }
      else if ( SOCKET_ERROR == iRet)
        {
        iRet = GetLastError();
        break;
        }
      else if ( FD_ISSET( s, &sete) || FD_ISSET( s, &setw))
        {
        /* Get error */
        socklen_t n = sizeof iRet;
        getsockopt( s, SOL_SOCKET, SO_ERROR, (char*)&iRet, &n);
        break;
        }
      }
    }

  /* Disable non-blocking mode */
  setsockasync( s, 0);

  return iRet;
  }


/*
 * Non-blocking accept
 */
static SOCKET AcceptAsync(
  SOCKET s,
  struct sockaddr* pAddr,
  socklen_t* pLen,
  unsigned timeout
) {
  SOCKET sAccept = INVALID_SOCKET;
  time_t const t = time( NULL) + timeout;

  /* Until timeout */
  while ( time( NULL) <= t )
    {
    fd_set setr;
    struct timeval tval;

    FD_ZERO( &setr);
    FD_SET( s, &setr);

    /* Set timeout for select */
    tval.tv_usec = 333 * 1000UL;
    tval.tv_sec = 0;

    /* Wait for a connect, error or timeout */
    if ( 0 == select( FD_SETSIZE, &setr, NULL, NULL, &tval))
      {
      /* Timeout */
      continue;
      }

    if ( FD_ISSET( s, &setr))
      {
      sAccept = accept( s, pAddr, pLen);
      break;
      }
    }

  return sAccept;
  }

/* End of file */
