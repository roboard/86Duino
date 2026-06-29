/****************************************************************************
 *
 * File name   : raw32.c
 * Function    : SOCK_RAW/IPPROTO_RAW winsock datalink
 * Project     : SwsSock
 * Authors     : Lawrence Rust
 * Systems     : ANSI C, Win32 (MSVC).
 *
 ****************************************************************************
 *
 * Created by Lawrence Rust, Software Systems Consultants
 * lvr@softsystem.co.uk. Tel/Fax +33 5 49 72 79 63
 *
 * 11-Jun-05, LVR, Created.
 *
 * Comments:
 * --------
 * This file implements a SOCK_RAW/IPPROTO_RAW datalink
 *
 ****************************************************************************
 */

/* Compilation options */

#include "config.h"


/* Exports */
#include "raw32.h"

/* Imports */
/* ANSI C library */
#include <stddef.h>
#include <string.h>

#if OPT_RAW_SOCK

/* Win32 platform */
#include <malloc.h>
#if defined __MWERKS__
# pragma ANSI_strict off
#elif defined _MSC_VER
# pragma warning ( disable : 4201) /* nonstandard extension used : nameless struct/union */
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <winsock2.h>
#if defined _MSC_VER
#pragma comment( lib, "ws2_32.lib")
#endif
#include <mstcpip.h> /* WSAIoctl - SIO_RCVALL */
#include <ws2tcpip.h> /* WSAIoctl - SIO_RCVALL */
#include <process.h> /* _beginthread */

#if defined __MWERKS__
# pragma ANSI_strict reset
#elif defined _MSC_VER
# pragma warning ( default : 4201)
#endif


/* SwsSock */
#include "sws_dlnk.h"
#include "sws_ip.h"
#include "sws_sock.h"

#include "platform.h"
#include "debug.h"


/*
 * Macros & constants
 */
#define kszIdent "winsock" /* Adapter ident string */


/*
 * Local data types
 */


/*
 * Prototypes
 */
static SWS_FDatalinkGetProperty Win32GetProperty;
static SWS_FDatalinkSetProperty Win32SetProperty;
static SWS_FDatalinkSend Win32Send;

static void __cdecl RawIpRecv( void*);


/*
 * Module data
 */
/* Configuration */
static ui32 s_uIpAddress = INADDR_ANY;
static ui32 s_uSubnetMask; /* 0= Default for IP address class */
static ui32 s_uGatewayAddress = SWS_INADDR_NONE;

/* Data link descriptor */
static SWS_DatalinkDesc const s_raw = {
  sizeof(s_raw),
  SWS_DATALINK_VERSION,
  &Win32Send,
  &Win32GetProperty,
  &Win32SetProperty,
  0, /* SWS_FDatalinkShutdown */
  0  /* SWS_FDatalinkSetMulticast */
};

/* Up calls */
static SWS_DatalinkCallbacks s_upcalls;

/* Operational data */
static SOCKET s_rawSocket;
static size_t s_sizeBuffer; /* Buffer size */
static void* s_pBuffer;     /* -> Tx/Rx buffer */
static unsigned long s_ulReceiver; /* Receiver thread handle */
static int s_iReceiverExit;


/****************************************************************************
 * RAW winsock datalink
 ****************************************************************************/

/*
 * Initialise the RAW_IP datalink
 */
int RawIpInstall( void)
  {
  WSADATA wsaData;
  BOOL bVal;
  int iErr, iLen;
  unsigned uVal;
  struct hostent* pHost;
  char szBuf[ 512];

  TRACE( 9, ("RawIpInstall()\n"));
  ASSERT( PlatformIsCritical());

  iErr = WSAStartup( MAKEWORD( 2, 2 ), &wsaData);
  if ( iErr)
    {
    TRACE( 1, ("*** RawIpInstall() WSAStartup failed %#lx\n", iErr));
    return iErr;
    }

  /* Create a raw socket */
  s_rawSocket = socket( PF_INET, SOCK_RAW, IPPROTO_IP);
  if ( INVALID_SOCKET == s_rawSocket)
    {
    iErr = WSAGetLastError();
    TRACE( 1, ("*** RawIpInstall() Failed to create socket (%d)\n", iErr));
    return iErr;
    }

  /* The app provides the IP header */
  /* OK on Win2k. Fails on Win98SE */
  bVal = TRUE;
  iErr = setsockopt( s_rawSocket, IPPROTO_IP, IP_HDRINCL, (char*)&bVal, sizeof(bVal));
  if ( SOCKET_ERROR == iErr)
    {
    iErr = WSAGetLastError();
    TRACE( 1, ("*** RawIpInstall setsockopt(IP_HDRINCL) failed(%d)\n", iErr));
    }

  /* Enable broadcasts */
  bVal = TRUE;
  iErr = setsockopt( s_rawSocket, SOL_SOCKET, SO_BROADCAST, (char*)&bVal, sizeof(bVal));
  if ( SOCKET_ERROR == iErr)
    {
    iErr = WSAGetLastError();
    TRACE( 1, ("*** RawIpInstall setsockopt(SO_BROADCAST) failed(%d)\n", iErr));
    }

  /* Get local IP address.  This can take ages on Win98 */
  if ( SOCKET_ERROR == gethostname( szBuf, sizeof( szBuf)))
    {
    iErr = WSAGetLastError();
    TRACE( 1, ("*** RawIpInstall gethostname failed(%d)\n", iErr));
    return iErr;
    }
  pHost = gethostbyname( szBuf);
  ASSERT( NULL != pHost);
  s_uIpAddress = ntohl( *(ui32*)pHost->h_addr);

  /* Get max message size */
  iLen = (int)sizeof(uVal);
  iErr = getsockopt( s_rawSocket, SOL_SOCKET, SO_MAX_MSG_SIZE, (char*)&uVal, &iLen);
  if ( SOCKET_ERROR == iErr)
    {
    iErr = WSAGetLastError();
    TRACE( 1, ("***RawIpInstall getsockopt failed(%d)\n", iErr));
    return iErr;
    }
  if ( uVal > 65535)
    uVal = 65535;

  /* Allocate i/o buffer */
  s_pBuffer = malloc( uVal);
  if ( NULL == s_pBuffer)
    {
    TRACE( 1, ("***RawIpInstall out of memory for buffer\n"));
    return SWS_ENOMEM;
    }
  s_sizeBuffer = uVal;

  /* Register the data link */
  s_upcalls.uSize = sizeof(s_upcalls);
  iErr = SWS_DatalinkRegister( &s_raw, NULL, &s_upcalls);
  if ( iErr)
    return iErr;

  ASSERT( s_upcalls.pfnSendRdy);
  ASSERT( s_upcalls.pfnRecvInd);

  /* Start the receiver */
  s_ulReceiver = _beginthread( &RawIpRecv, 0, NULL);

  return 0;
  }


/*
 * Cleanup RAW_IP datalink
 */
void RawIpPreCleanup( void)
  {
  TRACE( 9, ("RawIpPreCleanup()\n"));
  ASSERT( !PlatformIsCritical());

  /* Signal the receiver thread to terminate */
  s_iReceiverExit = 1;

  /* Wait for the receiver thread to terminate */
  switch ( WaitForSingleObject( (HANDLE)s_ulReceiver, 1000))
    {
  case WAIT_OBJECT_0:
    break;

  case WAIT_TIMEOUT:
    /* This will occur if the thread is waiting for the critical section */
    TRACE( 2, ("RawIpPreCleanup() WaitForSingleObject timed out\n"));

    /* Resolve the deadlock by killing the thread */
    if ( !TerminateThread( (HANDLE)s_ulReceiver, 1))
      {
      TRACE( 1, ("*** RawIpPreCleanup() TerminateThread failed %lu\n",
        GetLastError() ));
      }
    break;

  case WAIT_FAILED:
  default:
    /* This normally mean that the thread has exited and the handle is invalid */
    TRACE( 3, ("RawIpPreCleanup() WaitForSingleObject failed %lu\n",
      GetLastError() ));
    break;
    }
  }


/*
 * Cleanup RAW_IP datalink
 */
void RawIpCleanup( void)
  {
  TRACE( 9, ("RawIpCleanup()\n"));
  ASSERT( PlatformIsCritical());

  closesocket( s_rawSocket);
  s_rawSocket = INVALID_SOCKET;
  WSACleanup();

  if ( NULL != s_pBuffer)
    {
    free( s_pBuffer);
    s_pBuffer = NULL;
    }
  }


/*
 * Get an interface property
 */
SWS_CB( int, Win32GetProperty(
  void* pvContext,
  unsigned uType,
  void* pvBuf,
  size_t* puLen
)){
  TRACE( 9, ("Win32GetProperty(%p,%lu,%p,%p)\n", pvContext,uType,pvBuf,puLen));
  (void)pvContext;
  ASSERT( NULL != pvBuf);
  ASSERT( NULL != puLen);

  switch ( uType)
    {
  default:
    /* Not implemented */
    TRACE( 3, ("Win32GetProperty %lu not implemented\n", uType));
    return SWS_EBADF;

  case SWS_kDatalinkProperty_Address:
    if ( *puLen < sizeof(ui32))
      {
      TRACE( 0, ("!!!Win32GetProperty buffer too small\n"));
      return SWS_EINVAL;
      }

    *(SWS_PTR(ui32))pvBuf = s_uIpAddress;
    *puLen = sizeof(ui32);
    break;

  case SWS_kDatalinkProperty_SubnetMask:
    if ( *puLen < sizeof(ui32))
      {
      TRACE( 0, ("!!!Win32GetProperty buffer too small\n"));
      return SWS_EINVAL;
      }

    *(SWS_PTR(ui32))pvBuf = s_uSubnetMask;
    *puLen = sizeof(ui32);
    break;

  case SWS_kDatalinkProperty_MTU:
    if ( *puLen < sizeof(size_t))
      {
      TRACE( 0, ("!!!Win32GetProperty buffer too small\n"));
      return SWS_EINVAL;
      }

    *(SWS_PTR(size_t))pvBuf = s_sizeBuffer; /* > 576 ? 576 : s_sizeBuffer;*/
    *puLen = sizeof(size_t);
    break;

  case SWS_kDatalinkProperty_Gateway:
    if ( *puLen < sizeof(ui32))
      {
      TRACE( 0, ("!!!Win32GetProperty buffer too small\n"));
      return SWS_EINVAL;
      }

    if ( SWS_INADDR_NONE == s_uGatewayAddress)
      s_uGatewayAddress = PlatformPropertyGetUint( PSECT_WIN32, PVAL_IPGATEWAY, SWS_INADDR_NONE );

    *(SWS_PTR(ui32))pvBuf = s_uGatewayAddress;
    *puLen = sizeof(ui32);
    break;

  case SWS_kDatalinkProperty_Ident:
    if ( *puLen < sizeof(kszIdent))
      {
      TRACE( 0, ("!!!Win32GetProperty buffer too small\n"));
      return SWS_EINVAL;
      }

    memcpy( pvBuf, kszIdent, sizeof(kszIdent));
    *puLen = sizeof(kszIdent);
    break;

  case SWS_kDatalinkProperty_HwType:
    if ( *puLen < sizeof( ui8) )
      {
      TRACE( 0, ("!!!EnetGetProperty buffer too small\n"));
      return SWS_EINVAL;
      }

    *(SWS_PTR(ui8))pvBuf = 1; /* 1= Ethernet */
    *puLen = sizeof( ui8);
    break;

  case SWS_kDatalinkProperty_HwAddress:
    {
    /* Fake MAC address */
    static const ui8 kMacAddr[] = {
      0x00, 0xD0, 0xB7, 0xC7, 0x76, 0x75
    };
    if ( *puLen < sizeof(kMacAddr))
      {
      TRACE( 0, ("!!!Win32GetProperty buffer too small\n"));
      return SWS_EINVAL;
      }

    memcpy( pvBuf, kMacAddr, sizeof(kMacAddr));
    *puLen = sizeof(kMacAddr);
    }
    break;
    }

  return 0;
  }


/*
 * Set an interface property
 */
SWS_CB( int, Win32SetProperty(
  SWS_PTR( void) pvContext,
  unsigned uType,
  SWS_PTR( const void) pvBuf,
  size_t len
)){
  (void)pvContext;

  TRACE( 9, ("Win32SetProperty(%p,%lu,%p,%u)\n", pvContext,uType,pvBuf,len));
  ASSERT( NULL != pvBuf);

  switch ( uType)
    {
  default:
    /* Not implemented */
    TRACE( 3, ("Win32SetProperty %lu not implemented\n", uType));
    return SWS_EBADF;

  case SWS_kDatalinkProperty_Address:
    if ( len < sizeof(ui32))
      {
      TRACE( 0, ("!!!Win32SetProperty buffer too small\n"));
      return SWS_EINVAL;
      }

    s_uIpAddress = *(SWS_PTR(const ui32))pvBuf;
    break;

  case SWS_kDatalinkProperty_SubnetMask:
    if ( len < sizeof(ui32))
      {
      TRACE( 0, ("!!!Win32SetProperty buffer too small\n"));
      return SWS_EINVAL;
      }

    s_uSubnetMask = *(SWS_PTR(const ui32))pvBuf;
    break;

  case SWS_kDatalinkProperty_Gateway:
    if ( len < sizeof(ui32))
      {
      TRACE( 0, ("!!!Win32SetProperty buffer too small\n"));
      return SWS_EINVAL;
      }

    s_uGatewayAddress = *(SWS_PTR(const ui32))pvBuf;
    break;
    }

  return 0;
  }


/*
 * Send a raw IP packet
 */
SWS_CB( int, Win32Send(
  void* pvContext,
  SWS_BufferH hBuffer,
  ui32 dst,
  unsigned tos
)){
  int iRet;
  (void)pvContext;
  (void)tos;

  TRACE( 9, ("Win32Send(%p,%p,%lu,%u)\n", pvContext, hBuffer, dst, tos));
  ASSERT( NULL != hBuffer);

  if ( dst == s_uIpAddress)
    {
    /* Sending to this adapter so loopback the data */
    ASSERT( s_upcalls.pfnRecvInd);
    (*s_upcalls.pfnRecvInd)( s_upcalls.pvContext, hBuffer, 0);
    iRet = 0;
    }
  else
    {
    size_t len;
    struct sockaddr_in to;

    if ( SWS_BufferSize( hBuffer) > 576)
      TRACE( 1, ("*** Win32Send() packet > 576 bytes may be corrupted by win2k\n"));

    /* Copy data to linear buffer */
    len = (size_t)SWS_BufferSize( hBuffer);
    ASSERT( len <= s_sizeBuffer );
    VERIFY( !SWS_BufferCopyToMemory( s_pBuffer, hBuffer, 0, len));

    memset( &to, 0, sizeof( to) );
    to.sin_family = AF_INET;
    to.sin_addr.s_addr = htonl( dst);
    to.sin_port = htons( 0);

    iRet = sendto(
      s_rawSocket,
      (const char*)s_pBuffer,
      len,
      MSG_DONTROUTE,
      (struct sockaddr*)&to, sizeof(to)
    );
    if ( SOCKET_ERROR != iRet)
      iRet = 0;
    else
      {
      /* NB this error occurs when sending the 2nd part of a fragmented datagram */
      iRet = WSAGetLastError();
      TRACE( 1, ("*** Win32Send() send failed(%d)\n", iRet));
      }
    }

  return iRet;
  }


/*
 * Receiver thread
 */
void __cdecl RawIpRecv( void* pv)
  {
  void* pBuffer;
  SOCKET s;
  struct sockaddr_in local;
  int iErr, iLen;
  unsigned uVal;
  (void)pv;

  /* Create a socket */
  s = socket( PF_INET, SOCK_RAW, IPPROTO_IP);
  if ( INVALID_SOCKET == s)
    {
    iErr = WSAGetLastError();
    TRACE( 1, ("*** RawIpRecv() Failed to create socket (%d)\n", iErr));
    return;
    }

  /* Send receive timeout */
  iLen = 250; /* milli-secs */
  iErr = setsockopt(
    s,
    SOL_SOCKET, SO_RCVTIMEO,
    (char*)&iLen, sizeof(iLen)
  );
  if ( iErr)
    {
    iErr = WSAGetLastError();
    TRACE( 1, ("*** RawIpRecv() setsockopt SO_RCVTIMEO failed (%d)\n", iErr));
    }

  /* bind to default IP address */
  memset( &local, 0, sizeof( local) );
  local.sin_family = AF_INET;
  local.sin_addr.s_addr = htonl( s_uIpAddress);
  local.sin_port = htons( 0);
  iErr = bind( s, (struct sockaddr*)&local, sizeof(local) );
  if ( iErr)
    {
    iErr = WSAGetLastError();
    TRACE( 1, ("*** RawIpRecv() Failed to bind socket (%d)\n", iErr));
    closesocket( s);
    return;
    }
#if 0
  {
  BOOL bVal;
  DWORD dwCount;

  /* Receive all packets. This requires admin privilege. Fails on Win98SE */
  /* Works on Win2k. Fails on Win98SE - don't see ICMP ping requests */
  bVal = TRUE;
  iErr = WSAIoctl(
    s,
    SIO_RCVALL,
    &bVal, sizeof(bVal), /* in buf */
    NULL, 0, /* out buf */
    &dwCount,
    NULL, 0
  );
  if ( iErr)
    {
    iErr = WSAGetLastError();
    TRACE( 1, ("*** RawIpRecv() WSAIoctl failed (%d)\n", iErr));
    }
  }
#endif

  /* Get max message size */
  iLen = (int)sizeof(uVal);
  iErr = getsockopt( s, SOL_SOCKET, SO_MAX_MSG_SIZE, (char*)&uVal, &iLen);
  if ( SOCKET_ERROR == iErr)
    {
    iErr = WSAGetLastError();
    TRACE( 1, ("***RawIpRecv getsockopt failed(%d)\n", iErr));
    closesocket( s);
    return;
    }
  if ( uVal > 65535)
    uVal = 65535;

  pBuffer = _alloca( uVal);

  if ( NULL == pBuffer)
    {
    TRACE( 1, ("*** RawIpRecv out of memory for buffer\n"));
    closesocket( s);
    return;
    }

  /* Set receive buffer size */
  iErr = setsockopt(
    s,
    SOL_SOCKET, SO_RCVBUF,
    (char*)&uVal, sizeof(uVal)
  );
  if ( iErr)
    {
    iErr = WSAGetLastError();
    TRACE( 1, ("*** RawIpRecv() setsockopt SO_RCVBUF failed (%d)\n", iErr));
    }

  while ( !s_iReceiverExit)
    {
    struct sockaddr_in from;
    int len, iRet;

    /* Recv a raw datagram */
    len = sizeof( from);
    iRet = recvfrom(
      s,
      (char*)pBuffer,
      uVal,
      0,
      (struct sockaddr*)&from, &len
    );
    if ( SOCKET_ERROR == iRet)
      {
      iRet = GetLastError();
      if ( WSAETIMEDOUT == iRet)
        continue;

#if 0      /* This normally means winsock has been terminated (WSAEINTR) */
      if ( WSAEINTR != iRet)
#endif
        TRACE( 1, ("*** RawIpRecv recvfrom failed(%d)\n", iRet ));

      /* Exit */
      break;
      }
    else if ( ntohl( from.sin_addr.s_addr) == s_uIpAddress)
      {
      /* Winsock2 loops back all packets sent */
      TRACE( 3, ("RawIpRecv loopbacked packet\n"));
      }
    else
      {
      SWS_BufferH hBuffer;
      ui8* pIpHdr;

      /* NB RAW socket data is always received with an IP header */
      pIpHdr = (ui8*)pBuffer;
      if ( 0 == pIpHdr[9])
        {
        /* Win98 adds this extra header */
        len = 4 * (pIpHdr[0] & 0xf);
        pIpHdr += len;
        iRet -= len;
        }

      /* Wrap the data in a 'shadow' buffer */
      hBuffer = SWS_BufferGetShadow( (SWS_size_t)iRet, SWS_kBufferPrefix, pIpHdr);
      if ( NULL != hBuffer)
        {
        /* Pass received data up to IP layer */
         ASSERT( s_upcalls.pfnRecvInd);
        (*s_upcalls.pfnRecvInd)( s_upcalls.pvContext, hBuffer, 0);

        /* Release buffer - copies the data if still referenced */
        SWS_BufferReleaseShadow( hBuffer);
        }
      }
    }
  TRACE( 2, ("RawIpRecv terminating\n" ));
  }


#if 0
/*
 * Polled packet receiver
 */
static void Win32Recv( void)
  {
  int iRet;
  u_long ulCount;

  for ( ;;)
    {
    /* Check for data */
    iRet = ioctlsocket( s_rawSocket, FIONREAD, &ulCount);
    if ( SOCKET_ERROR == iRet)
      {
      iRet = WSAGetLastError();
      TRACE( 1, ("***Win32Recv ioctlsocket failed(%d)\n", iRet));
      break;
      }
    else if ( !ulCount)
      {
      /* No data */
      break;
      }
    else
      {
      struct sockaddr_in from;
      int len;

      /* Recv the data */
      len = sizeof( from);
      iRet = recvfrom(
        s_rawSocket,
        (char*)s_pBuffer,
        s_sizeBuffer,
        0,
        (struct sockaddr*)&from, &len
      );
      if ( SOCKET_ERROR == iRet)
        {
        iRet = WSAGetLastError();
        TRACE( 1, ("***Win32Recv recvfrom failed(%d)\n", iRet));
        }
      else if ( ntohl( from.sin_addr.s_addr) == s_uIpAddress)
        {
        /* Winsock2 loops back all packets sent */
        TRACE( 3, ("Win32Recv loopbacked packet\n"));
        }
      else
        {
        SWS_BufferH hBuffer;

        PlatformBeginCritical();

        /* Wrap the data in a 'shadow' buffer */
        hBuffer = SWS_BufferGetShadow( (SWS_size_t)iRet, SWS_kBufferPrefix, s_pBuffer);
        if ( NULL != hBuffer)
          {
          /* Pass received data up to IP layer */
           ASSERT( s_upcalls.pfnRecvInd);
          (*s_upcalls.pfnRecvInd)( s_upcalls.pvContext, hBuffer, 0);

          /* Release buffer - copies the data if still referenced */
          SWS_BufferReleaseShadow( hBuffer);
          }

        PlatformEndCritical();
        }
      }
    }
  }
#endif
#endif /*OPT_RAW_SOCK*/

/* End of file */
