/*****************************************************************************
 *
 * File name   : tftp.c
 * Function    : TFTP client (RFC 1350)
 * Authors     : Lawrence Rust
 * Systems     : ANSI C, BSD sockets
 *
 *****************************************************************************
 * File History
 * ------------
 *
 * 30-Aug-03, LVR, Created.
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
#include "tftp.h"
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
#include <assert.h>

/* Platform */
#include "sws_net.h"

#include "tftp.h"


/***************************************************************************
 * Macros & constants
 ***************************************************************************/

#ifndef TFTP_TIMEOUT
#define TFTP_TIMEOUT 2 /* seconds */
#endif
#ifndef TFTP_RETRIES
#define TFTP_RETRIES 3
#endif

#define TFTP_BLOCK_SIZE 512

/* tftp opcodes */
#define TFTP_RRQ  1
#define TFTP_WRQ  2
#define TFTP_DATA 3
#define TFTP_ACK  4
#define TFTP_ERR  5

/* tftp error codes */
#define ERR_UNDEFINED      0
#define ERR_FILE_NOT_FOUND 1
#define ERR_ACCESS         2
#define ERR_DISK_FULL      3
#define ERR_ILLEGAL_OP     4
#define ERR_UNKNOWN_TID    5
#define ERR_FILE_EXISTS    6
#define ERR_BAD_USER       7


/***************************************************************************
 * Data types
 ***************************************************************************/

typedef struct STftp
  {
  struct sockaddr host;
  SOCKET s;
  u_short uBlock;
  int rxData;
  unsigned char bufRx[ 4 + TFTP_BLOCK_SIZE];
  unsigned char bufTx[ 4 + TFTP_BLOCK_SIZE];
  } STftp;


/***************************************************************************
 * Local function prototypes
 ***************************************************************************/

int SendAck( STftp*, size_t);
static int RawRead( SOCKET, void*, size_t, unsigned, struct sockaddr*);


/***************************************************************************
 * Module data
 ***************************************************************************/

extern sig_atomic_t volatile g_exit;


/***************************************************************************
 * Functions
 ***************************************************************************/

/*
 * Send a file
 */
int TftpPut(                          /* Returns !0 on error */
  struct sockaddr* pSock,             /* IN: tftp server */
  FILE* srcFile,                      /* IN: source file */
  const char* pszDst,                 /* IN: destination */
  const char* pszMode,                /* IN: mode */
  int iVerbosity                      /* IN: stderr verbosity */
) {
  int iRet;
  size_t size;
  STftp conn;
  unsigned char* p;

  assert( NULL != srcFile);
  assert( NULL != pszDst);
  assert( NULL != pSock);

  /* Server port specifed? */
  if ( AF_INET == pSock->sa_family
    && 0 == ((struct sockaddr_in*)pSock)->sin_port
  ) {
    /* No, set default */
    ((struct sockaddr_in*)pSock)->sin_port = htons( TFTP_SERVER);
    }

  /* Create a socket */
  conn.s = socket( pSock->sa_family, SOCK_DGRAM, 0);
  if ( INVALID_SOCKET == conn.s)
    return GetLastError();

  if ( iVerbosity > 0)
    {
    fprintf( stdout, "Requesting tftp service at %s:%u...",
      inet_ntoa( ((struct sockaddr_in*)pSock)->sin_addr),
      ntohs( ((struct sockaddr_in*)pSock)->sin_port)
    ), fflush( stdout);
    }

  /* Format a WRQ packet */
  p = &conn.bufTx[0];
  *(u_short*)p = htons( TFTP_WRQ);
  p += 2;
  strcpy( (char*)p, pszDst);
  p += 1 + strlen( pszDst);
  strcpy( (char*)p, pszMode);
  p += 1 + strlen( pszMode);

  /* Send request */
  conn.host = *pSock;
  conn.uBlock = 0;
  iRet = SendAck( &conn, p - conn.bufTx);
  if ( iRet)
    {
    closesocket( conn.s);
    return iRet;
    }

  if ( iVerbosity > 0)
    fputs( "\nSending", stdout), fflush( stdout);

  /* Exchange packets only from ack'ing host/port */
  iRet = connect( conn.s, &conn.host, sizeof( conn.host) );
  assert( !iRet);

  /* Send loop */
  do
    {
    if ( iVerbosity > 0)
      fputs( ".", stdout), fflush( stdout);

    /* Read the next block of data */
    ++conn.uBlock;
    size = fread( &conn.bufTx[4], 1, TFTP_BLOCK_SIZE, srcFile);

    /* Send a data packet */
    *(u_short*)&conn.bufTx[0] = htons( TFTP_DATA);
    *(u_short*)&conn.bufTx[2] = htons( (u_short)conn.uBlock);
    iRet = SendAck( &conn, size + 4);
    if ( iRet)
      {
      static const char kszMsg[] = "Timeout";

      /* Send error */
      *(u_short*)&conn.bufTx[0] = htons( TFTP_ERR);
      *(u_short*)&conn.bufTx[2] = htons( ERR_UNDEFINED);
      strcpy( (char*)&conn.bufTx[4], kszMsg);
      sendto(
        conn.s,
        (const char*)conn.bufTx, 4 + sizeof(kszMsg),
        0,
        &conn.host, sizeof(conn.host)
      );
      break;
      }
    }
  while ( TFTP_BLOCK_SIZE == size);

  if ( iVerbosity > 0)
    fputs( "\n", stdout);

  /* Disconnect server */
  closesocket( conn.s);
  return iRet;
  }


/*
 * Receive a file
 */
int TftpGet(                          /* Returns !0 on error */
  struct sockaddr* pSock,             /* IN: tftp server */
  const char* pszSrc,                 /* IN: source */
  FILE* dstFile,                      /* IN: destination file */
  const char* pszMode,                /* IN: mode */
  int iVerbosity                      /* IN: stderr verbosity */
) {
  int iRet;
  STftp conn;
  unsigned char* p;

  assert( NULL != dstFile);
  assert( NULL != pszSrc);
  assert( NULL != pSock);

  /* Server port specifed? */
  if ( AF_INET == pSock->sa_family
    && 0 == ((struct sockaddr_in*)pSock)->sin_port
  ) {
    /* No, set default */
    ((struct sockaddr_in*)pSock)->sin_port = htons( TFTP_SERVER);
    }

  /* Create a socket */
  conn.s = socket( pSock->sa_family, SOCK_DGRAM, 0);
  if ( INVALID_SOCKET == conn.s)
    return GetLastError();

  if ( iVerbosity > 0)
    {
    fprintf( stdout, "Requesting tftp service at %s:%u...",
      inet_ntoa( ((struct sockaddr_in*)pSock)->sin_addr),
      ntohs( ((struct sockaddr_in*)pSock)->sin_port)
    ), fflush( stdout);
    }

  /* Format a RRQ packet */
  p = &conn.bufTx[0];
  *(u_short*)p = htons( TFTP_RRQ);
  p += 2;
  strcpy( (char*)p, pszSrc);
  p += 1 + strlen( pszSrc);
  strcpy( (char*)p, pszMode);
  p += 1 + strlen( pszMode);

  /* Send request */
  conn.host = *pSock;
  conn.uBlock = 1;
  iRet = SendAck( &conn, p - conn.bufTx);
  if ( iRet)
    {
    closesocket( conn.s);
    return iRet;
    }

  if ( iVerbosity > 0)
    fputs( "\nReceiving", stdout), fflush( stdout);

  /* Exchange packets only from ack'ing host/port */
  iRet = connect( conn.s, &conn.host, sizeof( conn.host) );
  assert( !iRet);

  while ( conn.rxData >= 4 + TFTP_BLOCK_SIZE)
    {
    if ( iVerbosity > 0)
      fputs( ".", stdout), fflush( stdout);

    fwrite( &conn.bufRx[4], 1, conn.rxData - 4, dstFile);

    /* Ack the data block and get next */
    *(u_short*)&conn.bufTx[0] = htons( TFTP_ACK);
    *(u_short*)&conn.bufTx[2] = htons( (u_short)conn.uBlock);
    ++conn.uBlock;
    iRet = SendAck( &conn, 4);
    if ( iRet)
      {
      static const char kszMsg[] = "Timeout";

      /* Send error */
      *(u_short*)&conn.bufTx[0] = htons( TFTP_ERR);
      *(u_short*)&conn.bufTx[2] = htons( ERR_UNDEFINED);
      strcpy( (char*)&conn.bufTx[4], kszMsg);
      sendto(
        conn.s,
        (const char*)conn.bufTx, 4 + sizeof(kszMsg),
        0,
        &conn.host, sizeof(conn.host)
      );
      break;
      }
    }

  if ( 0 == iRet)
    {
    /* Write the final block */
    if ( conn.rxData > 4)
      fwrite( &conn.bufRx[4], 1, conn.rxData - 4, dstFile);
    if ( iVerbosity > 0)
      fputs( "\n", stdout);
    }

  /* Disconnect server */
  closesocket( conn.s);
  return iRet;
  }


/*
 * Send a packet with retries
 */
int SendAck(
  STftp* pConn,
  size_t size
) {
  int iRet;
  unsigned uResends = 0;

retransmit:
  iRet = sendto(
    pConn->s,
    (const char*)pConn->bufTx, size,
    0,
    &pConn->host, sizeof(pConn->host)
  );
  if ( SOCKET_ERROR == iRet)
    {
    iRet = GetLastError();
    switch ( iRet)
      {
    case ECONNRESET:
    case ENETUNREACH:
    case EHOSTUNREACH:
      fprintf( stderr, "\nServer unreachable\n" );
      break;
    default:
      fprintf( stderr, "\nError (%d) while sending: %s\n", iRet, strerror( iRet));
      break;
      }
    return iRet;
    }

  /* Get response */
  for (;;)
    {
    unsigned uOp;

    iRet = RawRead(
      pConn->s,
      pConn->bufRx, sizeof(pConn->bufRx),
      TFTP_TIMEOUT,
      &pConn->host
    );
    if ( SOCKET_ERROR == iRet)
      {
      iRet = GetLastError();
      switch ( iRet)
        {
      case ECONNRESET:
      case ENETUNREACH:
      case EHOSTUNREACH:
        fprintf( stderr, "\nServer unreachable\n" );
        return iRet;
      case EWOULDBLOCK:
#if EAGAIN != EWOULDBLOCK
      case EAGAIN:
#endif
        continue;
      default:
        fprintf( stderr, "t" );
        break;
        }

      if ( ++uResends < TFTP_RETRIES)
        goto retransmit;

      fputs( "\nToo many retries\n", stderr);
      return ETIMEDOUT;
      }

    if ( iRet < 4)
      continue;

    /* Get opcode */
    uOp = ntohs( ((u_short*)pConn->bufRx)[0]);
    if ( TFTP_ACK == uOp || TFTP_DATA == uOp)
      {
      /* Check block ack'd */
      if ( pConn->uBlock == ntohs( *(u_short*)&pConn->bufRx[2]) )
        {
        /* Good reply */
        pConn->rxData = iRet;
        iRet = 0;
        break;
        }
      fputs( "x", stderr);
      }
    else if ( TFTP_ERR == uOp)
      {
      iRet =  ntohs( *(u_short*)&pConn->bufRx[2] );
      fprintf( stderr, "\nERROR %d: %s\n", iRet, &pConn->bufRx[4]);
      return iRet ? iRet : -1;
      }
    else
      {
      fprintf( stderr, "\nUnexpected op code(%u) received\n", uOp );
      }
    }

  return iRet;
  }



/*
 * Non-blocking read data
 */
int RawRead(
  SOCKET s,
  void* pv,
  size_t size,
  unsigned timeout,
  struct sockaddr* pSock
) {
  time_t const t = time( NULL) + timeout;

  /* Until SIGINT or timeout */
  while ( !g_exit && time( NULL) <= t )
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
      socklen_t len = pSock ? sizeof(*pSock) : 0;
      return recvfrom( s, (char*)pv, size, 0, pSock, &len);
      }
    }

  return SOCKET_ERROR;
  }

/* End of file */
