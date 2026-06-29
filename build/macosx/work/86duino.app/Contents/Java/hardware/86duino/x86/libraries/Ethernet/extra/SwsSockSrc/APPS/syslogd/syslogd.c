/*****************************************************************************
 *
 * File name   : syslogd.c
 * Function    : syslog (RFC 3164) server
 * Authors     : Lawrence Rust
 * Systems     : ANSI C, sockets
 *
 *****************************************************************************
 * File History
 * ------------
 *
 * 24-Jun-03, LVR, Created.
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
#if !SWS_USE_SOCKLIB
#include "syslogd.h"
#endif

/* Imports */

/* ANSI C library */
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>
#include <time.h>
#if _MSC_VER >= 1000
#include <crtdbg.h>
#endif

#include "sws_net.h"

#include "syslogd.h"


/***************************************************************************
 * Macros & constants
 ***************************************************************************/

/* Length of an array - sorely missing from stddef.h */
#ifndef lengthof
#define lengthof( _a) ((size_t)( sizeof(_a)/sizeof((_a)[0]) ))
#endif

#define kszPrefix "SYSLOGD: "

#define POLL_TIMEOUT    333U /* select timeout in millisecs */
#define MAX_PACKET      1024 /* Max syslog packet */

enum
  {
  kSeverities = 24,           /* No. severities */
  kFacilities = 8             /* No. facilities */
  };
static const char* const kszSeverity[ kSeverities] = {
  "kernel",     /* 0 */
  "user",       /* 1 */
  "mail",       /* 2 */
  "system",     /* 3 */
  "auth(4)",    /* 4 */
  "syslogd",    /* 5 */
  "lpt",        /* 6 */
  "news",       /* 7 */
  "uucp",       /* 8 */
  "clock(9)",   /* 9 */
  "auth(10)",   /* 10 */
  "ftp",        /* 11 */
  "ntp",        /* 12 */
  "log audit",  /* 13 */
  "log alert",  /* 14 */
  "clock(15)",  /* 15 */
  "local(0)",   /* 16 */
  "local(1)",   /* 17 */
  "local(2)",   /* 18 */
  "local(3)",   /* 19 */
  "local(4)",   /* 20 */
  "local(5)",   /* 21 */
  "local(6)",   /* 22 */
  "local(7)"    /* 23 */
};
static const char* const kszFacility[ kFacilities] = {
  "panic",    /* 0 */
  "alert",    /* 1 */
  "critical", /* 2 */
  "error",    /* 3 */
  "warning",  /* 4 */
  "notice",   /* 5 */
  "info",     /* 6 */
  "debug"     /* 7 */
};


/***************************************************************************
 * Data types
 ***************************************************************************/

typedef struct SConnection
  {
  SOCKET s;                     /* client connection */
  u_short port;                 /* Server port */
  struct sockaddr_in client;    /* client's address */
  time_t tRequest;

  /* Logging */
  int iVerbosity;
  FILE* logfile;

  void* pvBuf;
  size_t sizeBuf;
  } SConnection;


/***************************************************************************
 * Local function prototypes
 ***************************************************************************/

static void SessionInput( SConnection*);
static int ReadRaw( SConnection*, void*, size_t);


/***************************************************************************
 * Module data
 ***************************************************************************/

/* Operation */
extern sig_atomic_t volatile g_exit;


/***************************************************************************
 * Functions
 ***************************************************************************/

/*
 * Syslog server
 */
extern int Syslogd(
  struct sockaddr* pHost,
  int iVerbosity,
  FILE* logfile
) {
  int iRet = 0;
  SOCKET sDgrm = INVALID_SOCKET;
  void* pvBuf;
  SConnection conn;
  char szTime[ 32];

  /* Allocate a common buffer */
  pvBuf = malloc( MAX_PACKET + 1);
  if ( NULL == pvBuf)
    {
    fprintf( stderr, kszPrefix "Out of memory for buffer\n");
    return 1;
    }

  if ( AF_UNSPEC == pHost->sa_family)
    pHost->sa_family = AF_INET;

  /* Server port specifed? */
  if ( AF_INET == pHost->sa_family
    && 0 == ((struct sockaddr_in*)pHost)->sin_port
  ) {
    /* No, set default */
    ((struct sockaddr_in*)pHost)->sin_port = htons( SYSLOGD_PORT );
    }

  /* Open server sockets */
  sDgrm = socket( pHost->sa_family, SOCK_DGRAM, 0);
  if ( INVALID_SOCKET == sDgrm)
    {
    iRet = GetLastError();
    fprintf( stderr, kszPrefix "Failed (%d) to create socket: %s\n",
      iRet, strerror( iRet));
    goto error;
    }

  /* Select the port to receive on */
  if ( bind( sDgrm, pHost, sizeof(*pHost)) )
    {
    iRet = GetLastError();
    fprintf( stderr, kszPrefix "Failed (%d) to bind to port %u: %s\n",
      iRet, ntohs( ((struct sockaddr_in*)pHost)->sin_port), strerror( iRet));
    goto error;
    }

  conn.s = sDgrm;
  conn.client.sin_family = AF_UNSPEC;
  conn.logfile = logfile;
  conn.iVerbosity = iVerbosity;
  conn.pvBuf = pvBuf;
  conn.sizeBuf = MAX_PACKET;
  conn.port = ntohs( ((struct sockaddr_in*)pHost)->sin_port);

  if ( NULL != logfile && iVerbosity > 0)
    {
    time_t t = time( NULL);
    strftime( szTime, sizeof szTime,
      "%Y/%m/%d %H:%M:%S", localtime( &t) );
    fprintf( logfile, "%s " kszPrefix "%s:%u LISTENING\n",
      szTime,
      inet_ntoa( ((const struct sockaddr_in*)pHost)->sin_addr),
      ntohs( ((const struct sockaddr_in*)pHost)->sin_port)
    );
    }

  /* The server loop */
  while ( !g_exit)
    {
    fd_set setRd;
    struct timeval tval;

    FD_ZERO( &setRd);
    FD_SET( sDgrm, &setRd);

    /* Set timeout for select */
    tval.tv_usec = (POLL_TIMEOUT % 1000) * 1000UL;
    tval.tv_sec = POLL_TIMEOUT / 1000;

    /* Wait for receive event */
    iRet = select( FD_SETSIZE, &setRd, NULL, NULL, &tval);
    if ( SOCKET_ERROR == iRet)
      {
      iRet = GetLastError();
      fprintf(
        stderr, kszPrefix "Select failed (%d): %s\n", iRet, strerror( iRet));
      continue;
      }

    if ( FD_ISSET( sDgrm, &setRd))
      {
      /* Receive a datagram */
      conn.tRequest = time( NULL);
      SessionInput( &conn);
      }
    }

error:
  if ( INVALID_SOCKET != sDgrm)
    closesocket( sDgrm);

  if ( NULL != pvBuf)
    free( pvBuf);

  return iRet;
  }


/*
 * Session input ready
 */
void SessionInput( SConnection* pConn)
  {
  int iRet;

  /* Input a line */
  iRet = ReadRaw( pConn, pConn->pvBuf, pConn->sizeBuf);
  if ( SOCKET_ERROR != iRet)
    {
    if ( NULL != pConn->logfile)
      {
      unsigned uPri, uDay, uHour, uMin, uSec;
      char c, *psz;
      int n;
      char szBuf[ 32];

      /* Log time and source */
      strftime( szBuf, sizeof szBuf, "%Y/%m/%d %H:%M:%S",
        localtime( &pConn->tRequest) );
      fprintf( pConn->logfile, "%s " kszPrefix "%s",
        szBuf,
        inet_ntoa( pConn->client.sin_addr)
      );

      /* Parse the PRI */
      psz = (char*)pConn->pvBuf;
      psz[ iRet] = '\0';
      if ( 2 <= sscanf( psz, "<%u%c%n", &uPri, &c, &n)
        && '>' == c
        && uPri < kSeverities * kFacilities
      ) {
        /* Valid PRI */
        unsigned uSev, uFac;

        uSev = uPri / 8;
        uFac = uPri - 8 * uSev;
        fprintf( pConn->logfile, " %s %s",
          kszSeverity[ uSev],
          kszFacility[ uFac]
        );

        /* Skip PRI */
        psz += n;
        iRet -= n;

        /* Test for timestamp */
        if ( 5 <= sscanf(
            psz, "%3s %u %u:%u:%u%n", szBuf, &uDay, &uHour, &uMin, &uSec, &n
          )
        ) {
          ++n; /* Skip following space */
          psz += n;
          iRet -= n;
          }

        /* Skip hostname */
        while ( iRet > 0 && ' ' != *psz)
          {
          ++psz;
          --iRet;
          }

        /* Skip following space */
        ++psz;
        --iRet;
        }

      /* Display header & content */
      fprintf( pConn->logfile, " %.*s\n", iRet, psz);
      }
    }
  }


/****************************************************************************
 * Network I/O
 ****************************************************************************/

/*
 * Read raw data from the client
 */
static int ReadRaw(
  SConnection* pConn,
  void* pv,
  size_t size
) {
  int iRet;
  socklen_t len;

  len = sizeof( pConn->client);
  iRet = recvfrom( pConn->s, (char*)pv, size, 0,
    (struct sockaddr*)&pConn->client, &len
  );

  if ( SOCKET_ERROR == iRet)
    {
    int err = GetLastError();
    if ( EWOULDBLOCK == err || EAGAIN == err)
      {
      return 0;
      }
    else if ( pConn->iVerbosity >= 0)
      {
      FILE* file = NULL != pConn->logfile ? pConn->logfile : stderr;
      char szTime[ 32];

      strftime( szTime, sizeof szTime, "%Y/%m/%d %H:%M:%S",
        localtime( &pConn->tRequest) );
      fprintf( file, "%s " kszPrefix "%s:%u ",
        szTime,
        inet_ntoa( pConn->client.sin_addr),
        ntohs( pConn->client.sin_port)
      );
      switch ( err)
        {
      case ECONNRESET:
        fputs( "Connection reset in read\n", file);
        break;
      case ECONNABORTED:
        fputs( "Connection aborted in read\n", file);
        break;
      case ENOTCONN:
        fputs( "Disconnected in read\n", file);
        break;
      default:
        fprintf( file, "Receive failed (%d): %s\n", err, strerror( err));
        break;
        }
      }
    }
  else if ( 0 == iRet)
    {
    /* Graceful disconnect */
    iRet = SOCKET_ERROR;
    }

  return iRet;
  }

/* End of file */
