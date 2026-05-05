/****************************************************************************
 *
 * File name   : ws2.c
 * Function    : Winsock2 emulation functions.
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
 * This file contains Winsock2 compatible entrypoints
 *
 ****************************************************************************
 */

/* Compilation options */

#include "config.h"


/* Exports */

/* Imports */
/* ANSI C library */
#include <stddef.h>
#include <string.h>

/* Win32 platform */
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

#if defined __MWERKS__
# pragma ANSI_strict reset
#elif defined _MSC_VER
# pragma warning ( default : 4201)
#endif

/* SwsSock */
#include "sws_sock.h"
#include "debug.h"


/*
 * Macros & constants
 */

#ifdef __cplusplus
#define EXTERNC extern "C"
#else
#define EXTERNC extern
#endif


/*
 * Local data types
 */


/*
 * Prototypes
 */


/*
 * Module data
 */

/****************************************************************************
 * Winsock2 compatibility
 ****************************************************************************/

/*
 * WSAStartup
 */
EXTERNC __declspec( dllexport) int WSAAPI SWS_WSAStartup(
  IN WORD wVersionRequested,
  OUT LPWSADATA lpWSAData
) {
  (void)wVersionRequested;

  TRACE( 2, ("SWS_WSAStartup()\n"));

  if ( SWS_SockStartup( 0, NULL))
    return WSASYSNOTREADY;

  lpWSAData->wVersion = 2;
  lpWSAData->wHighVersion = 2;
  strcpy( lpWSAData->szDescription, "sws_sock");
  strcpy( lpWSAData->szSystemStatus, "");
  lpWSAData->iMaxSockets = OPT_SOCKETS;
  lpWSAData->iMaxUdpDg = 512;
  lpWSAData->lpVendorInfo = NULL;

  return SWS_SockStartup( 0, NULL) ? WSASYSNOTREADY : 0;
  }


/*
 * WSACleanup
 */
EXTERNC __declspec( dllexport) int WSAAPI SWS_WSACleanup( void)
  {
  TRACE( 2, ("SWS_WSACleanup()\n"));
  SWS_SockCleanup();
  return 0;
  }


/*
 * Error code translation
 */
EXTERNC __declspec( dllexport) int WSAAPI SWS_WSAGetLastError( void)
  {
  int err;

  TRACE( 2, ("SWS_WSAGetLastError()\n"));

#define GET_WSAERR( e) case SWS_##e : return WSA##e
#define GET_WSAERRE( e) case SWS_E##e : return WSA##e

  err = SWS_GetLastError();
  switch ( err)
    {
  case 0: return ERROR_SUCCESS;
  GET_WSAERR( EINTR);
  GET_WSAERR( EBADF);
  case SWS_ENOMEM: return WSA_NOT_ENOUGH_MEMORY;
  GET_WSAERR( EACCES);
  GET_WSAERR( EFAULT);
  case SWS_ENODEV: return ERROR_FILE_NOT_FOUND;
  GET_WSAERR( EINVAL);
  GET_WSAERR( EMFILE);
  GET_WSAERR( EWOULDBLOCK);
  GET_WSAERR( EINPROGRESS);
  GET_WSAERR( EALREADY);
  GET_WSAERR( ENOTSOCK);
  GET_WSAERR( EDESTADDRREQ);
  GET_WSAERR( EMSGSIZE);
  GET_WSAERR( EPROTOTYPE);
  GET_WSAERR( ENOPROTOOPT);
  GET_WSAERR( EPROTONOSUPPORT);
  GET_WSAERR( ESOCKTNOSUPPORT);
  GET_WSAERR( EOPNOTSUPP);
  GET_WSAERR( EPFNOSUPPORT);
  GET_WSAERR( EAFNOSUPPORT);
  GET_WSAERR( EADDRINUSE);
  GET_WSAERR( EADDRNOTAVAIL);
  GET_WSAERR( ENETDOWN);
  GET_WSAERR( ENETUNREACH);
  GET_WSAERR( ENETRESET);
  GET_WSAERR( ECONNABORTED);
  GET_WSAERR( ECONNRESET);
  GET_WSAERR( ENOBUFS);
  GET_WSAERR( EISCONN);
  GET_WSAERR( ENOTCONN);
  GET_WSAERR( ESHUTDOWN);
  GET_WSAERR( ETOOMANYREFS);
  GET_WSAERR( ETIMEDOUT);
  GET_WSAERR( ECONNREFUSED);
  GET_WSAERR( ELOOP);
  GET_WSAERR( ENAMETOOLONG);
  GET_WSAERR( EHOSTDOWN);
  GET_WSAERR( EHOSTUNREACH);
  GET_WSAERR( ENOTEMPTY);
  GET_WSAERR( EPROCLIM);
  GET_WSAERR( EUSERS);
  GET_WSAERR( EDQUOT);
  GET_WSAERR( ESTALE);
  GET_WSAERR( EREMOTE);
  GET_WSAERRE( HOST_NOT_FOUND);
  GET_WSAERRE( TRY_AGAIN);
  GET_WSAERRE( NO_RECOVERY);
  GET_WSAERRE( NO_DATA);

  default:
    TRACE( 0, ("!!!SWS_WSAGetLastError %d", err));
    return ERROR_NOT_SUPPORTED;
    }
  }


EXTERNC __declspec( dllexport) void WSAAPI SWS_WSASetLastError( int err)
  {
  TRACE( 2, ("SWS_WSASetLastError(%d)\n",err));

#define SET_SWSERR( e) case WSA##e : SWS_SetLastError( SWS_##e); break
#define SET_SWSERRE( e) case WSA##e : SWS_SetLastError( SWS_E##e); break

  switch ( err)
    {
  case ERROR_SUCCESS: SWS_SetLastError( 0); break;
  SET_SWSERR( EINTR);
  SET_SWSERR( EBADF);
  case WSA_NOT_ENOUGH_MEMORY: SWS_SetLastError( SWS_ENOMEM); break;
  SET_SWSERR( EACCES);
  SET_SWSERR( EFAULT);
  case ERROR_FILE_NOT_FOUND: SWS_SetLastError( SWS_ENODEV); break;
  SET_SWSERR( EINVAL);
  SET_SWSERR( EMFILE);
  SET_SWSERR( EWOULDBLOCK);
  SET_SWSERR( EINPROGRESS);
  SET_SWSERR( EALREADY);
  SET_SWSERR( ENOTSOCK);
  SET_SWSERR( EDESTADDRREQ);
  SET_SWSERR( EMSGSIZE);
  SET_SWSERR( EPROTOTYPE);
  SET_SWSERR( ENOPROTOOPT);
  SET_SWSERR( EPROTONOSUPPORT);
  SET_SWSERR( ESOCKTNOSUPPORT);
  SET_SWSERR( EOPNOTSUPP);
  SET_SWSERR( EPFNOSUPPORT);
  SET_SWSERR( EAFNOSUPPORT);
  SET_SWSERR( EADDRINUSE);
  SET_SWSERR( EADDRNOTAVAIL);
  SET_SWSERR( ENETDOWN);
  SET_SWSERR( ENETUNREACH);
  SET_SWSERR( ENETRESET);
  SET_SWSERR( ECONNABORTED);
  SET_SWSERR( ECONNRESET);
  SET_SWSERR( ENOBUFS);
  SET_SWSERR( EISCONN);
  SET_SWSERR( ENOTCONN);
  SET_SWSERR( ESHUTDOWN);
  SET_SWSERR( ETOOMANYREFS);
  SET_SWSERR( ETIMEDOUT);
  SET_SWSERR( ECONNREFUSED);
  SET_SWSERR( ELOOP);
  SET_SWSERR( ENAMETOOLONG);
  SET_SWSERR( EHOSTDOWN);
  SET_SWSERR( EHOSTUNREACH);
  SET_SWSERR( ENOTEMPTY);
  SET_SWSERR( EPROCLIM);
  SET_SWSERR( EUSERS);
  SET_SWSERR( EDQUOT);
  SET_SWSERR( ESTALE);
  SET_SWSERR( EREMOTE);
  SET_SWSERRE( HOST_NOT_FOUND);
  SET_SWSERRE( TRY_AGAIN);
  SET_SWSERRE( NO_RECOVERY);
  SET_SWSERRE( NO_DATA);

  default:
    SWS_SetLastError( 1);
    break;
    }
  }


/*
 * Winsock2 shutdown()
 */
EXTERNC __declspec( dllexport) int WSAAPI WSAshutdown(
  IN SOCKET s,
  IN int how
) {
  TRACE( 9, ("WSAshutdown(%p,%d)\n",s,how));

  switch ( how)
    {
  case SD_SEND:    how = SWS_SD_SEND;    break;
  case SD_RECEIVE: how = SWS_SD_RECEIVE; break;
  case SD_BOTH:    how = SWS_SD_BOTH;    break;
    }
  return SWS_shutdown( (SWS_SOCKET)s, how);
  }


/*
 * Winsock2 select()
 */
EXTERNC __declspec( dllexport) int WSAAPI WSAselect(
  IN int nfds,
  IN OUT fd_set FAR * readfds,
  IN OUT fd_set FAR * writefds,
  IN OUT fd_set FAR *exceptfds,
  IN const struct timeval FAR * timeout
) {
  SWS_fd_set rd, wr, ex;
  u_int i;
  struct SWS_timeval t;
  int iRet;
  (void)nfds;

  TRACE( 9, ("WSAselect()\n"));

  if ( readfds)
    {
    rd.fd_limit = SWS_FD_SETSIZE;
    rd.fd_count = readfds->fd_count;
    for ( i = 0; i < rd.fd_count; ++i)
      rd.fd_array[ i] = (SWS_SOCKET)readfds->fd_array[ i];
    }
  else
    SWS_FdZero( &rd);

  if ( writefds)
    {
    wr.fd_limit = SWS_FD_SETSIZE;
    wr.fd_count = writefds->fd_count;
    for ( i = 0; i < rd.fd_count; ++i)
      wr.fd_array[ i] = (SWS_SOCKET)writefds->fd_array[ i];
    }
  else
    SWS_FdZero( &wr);

  if ( exceptfds)
    {
    ex.fd_limit = SWS_FD_SETSIZE;
    ex.fd_count = exceptfds->fd_count;
    for ( i = 0; i < rd.fd_count; ++i)
      ex.fd_array[ i] = (SWS_SOCKET)exceptfds->fd_array[ i];
    }
  else
    SWS_FdZero( &ex);

  if ( timeout)
    {
    t.tv_sec = timeout->tv_sec;
    t.tv_usec = timeout->tv_usec;
    }

  iRet = SWS_select( &rd, &wr, &ex, timeout ? &t : NULL);

  if ( exceptfds)
    {
    exceptfds->fd_count = ex.fd_count;
    for ( i = 0; i < ex.fd_count; ++i)
      exceptfds->fd_array[ i] = (SOCKET)ex.fd_array[ i];
    }

  if ( writefds)
    {
    writefds->fd_count = wr.fd_count;
    for ( i = 0; i < wr.fd_count; ++i)
      writefds->fd_array[ i] = (SOCKET)wr.fd_array[ i];
    }

  if ( readfds)
    {
    readfds->fd_count = rd.fd_count;
    for ( i = 0; i < rd.fd_count; ++i)
      readfds->fd_array[ i] = (SOCKET)rd.fd_array[ i];
    }

  return iRet;
  }


/*
 * __WSAFDIsSet
 */
EXTERNC __declspec( dllexport) int PASCAL FAR WSAFdIsSet( SWS_SOCKET s, fd_set FAR * set)
  {
  u_int i;

  TRACE( 9, ("WSAFdIsSet(%p,%p)\n",s,set));

  if ( NULL != set)
    {
    for ( i = 0; i < set->fd_count && i < FD_SETSIZE; ++i)
      {
      if ( (SWS_SOCKET)set->fd_array[ i] == s)
        return 1;
      }
    }
  return 0;
  }


EXTERNC __declspec( dllexport) SOCKET WSAAPI SWS_WSASocket(
  IN int af,
  IN int type,
  IN int protocol,
  IN LPWSAPROTOCOL_INFO lpProtocolInfo,
  IN GROUP g,
  IN DWORD dwFlags
) {
  TRACE( 9, ("SWS_WSASocket()\n"));
  (void)g;
  (void)lpProtocolInfo;

  if ( WSA_FLAG_OVERLAPPED & dwFlags)
    {
    TRACE( 1, ("*** SWS_WSASocket() overlapped I/O not supported\n"));
    SWS_SetLastError( SWS_EAFNOSUPPORT);
    return INVALID_SOCKET;
    }

  if ( NULL != lpProtocolInfo)
    TRACE( 2, ("SWS_WSASocket() protocol info ignored\n"));

  return (SOCKET) SWS_socket( af, type, protocol);
  }


/*
 * WSARecv
 */
EXTERNC __declspec( dllexport) int WSAAPI SWS_WSARecv(
  IN SOCKET s,
  IN OUT LPWSABUF lpBuffers,
  IN DWORD dwBufferCount,
  OUT LPDWORD lpNumberOfBytesRecvd,
  IN OUT LPDWORD lpFlags,
  IN LPWSAOVERLAPPED lpOverlapped,
  IN LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine
) {
  int iLen;
  (void)dwBufferCount;
  (void)lpOverlapped;
  (void)lpCompletionRoutine;

  TRACE( 9, ("SWS_WSARecv()\n"));
  ASSERT( NULL != lpBuffers);
  ASSERT( dwBufferCount > 0);
  ASSERT( NULL != lpNumberOfBytesRecvd);
  ASSERT( NULL != lpFlags);

  iLen = SWS_recv( (SWS_SOCKET)s, lpBuffers->buf, lpBuffers->len, *lpFlags);
  if ( SWS_SOCKET_ERROR != iLen)
    {
    *lpNumberOfBytesRecvd = iLen;
    return 0;
    }

  return SOCKET_ERROR;
  }


/*
 * WSARecvFrom
 */
EXTERNC __declspec( dllexport) int WSAAPI SWS_WSARecvFrom(
  IN SOCKET s,
  IN OUT LPWSABUF lpBuffers,
  IN DWORD dwBufferCount,
  OUT LPDWORD lpNumberOfBytesRecvd,
  IN OUT LPDWORD lpFlags,
  OUT struct sockaddr FAR * lpFrom,
  IN OUT LPINT lpFromlen,
  IN LPWSAOVERLAPPED lpOverlapped,
  IN LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine
) {
  int iLen;
  (void)dwBufferCount;
  (void)lpOverlapped;
  (void)lpCompletionRoutine;

  TRACE( 9, ("SWS_WSARecvFrom()\n"));
  ASSERT( NULL != lpBuffers);
  ASSERT( dwBufferCount > 0);
  ASSERT( NULL != lpNumberOfBytesRecvd);
  ASSERT( NULL != lpFlags);
  ASSERT( NULL != lpFrom);
  ASSERT( NULL != lpFromlen);

  iLen = SWS_recvfrom(
    (SWS_SOCKET)s,
    lpBuffers->buf,
    lpBuffers->len,
    *lpFlags,
    (SWS_PTR( struct SWS_sockaddr)) lpFrom,
    lpFromlen
  );
  if ( SWS_SOCKET_ERROR != iLen)
    {
    *lpNumberOfBytesRecvd = iLen;
    return 0;
    }

  return SOCKET_ERROR;
  }


EXTERNC __declspec( dllexport) int WSAAPI SWS_WSAIoctl(
  IN SOCKET s,
  IN DWORD dwIoControlCode,
  IN LPVOID lpvInBuffer,
  IN DWORD cbInBuffer,
  OUT LPVOID lpvOutBuffer,
  IN DWORD cbOutBuffer,
  OUT LPDWORD lpcbBytesReturned,
  IN LPWSAOVERLAPPED lpOverlapped,
  IN LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine
) {
  (void)s;
  (void)dwIoControlCode;
  (void)lpvInBuffer;
  (void)cbInBuffer;
  (void)lpvOutBuffer;
  (void)cbOutBuffer;
  (void)lpcbBytesReturned;
  (void)lpOverlapped;
  (void)lpCompletionRoutine;

  TRACE( 1, ("*** SWS_WSAIoctl()\n"));

  SWS_SetLastError( SWS_EINVAL);
  return SOCKET_ERROR;
  }


/*
 * WSAJoinLeaf to support mcast
 */
EXTERNC __declspec( dllexport) SOCKET WSAAPI SWS_WSAJoinLeaf(
  IN SOCKET s,
  IN const struct sockaddr FAR * name,
  IN int namelen,
  IN LPWSABUF lpCallerData,
  OUT LPWSABUF lpCalleeData,
  IN LPQOS lpSQOS,
  IN LPQOS lpGQOS,
  IN DWORD dwFlags
) {
  struct SWS_ip_mreq mreq;
  struct SWS_sockaddr_in from;
  (void)lpCallerData;
  (void)lpCalleeData;
  (void)lpSQOS;
  (void)lpGQOS;
  (void)dwFlags;

  TRACE( 9, ("SWS_WSAJoinLeaf()\n"));

  if ( namelen != sizeof( struct SWS_sockaddr_in))
    {
    SWS_SetLastError( SWS_EFAULT);
    return INVALID_SOCKET;
    }

  mreq.imr_multiaddr = ((SWS_PTR( struct SWS_sockaddr_in))name)->sin_addr;
  mreq.imr_interface.s_addr = INADDR_ANY;
  if ( SWS_setsockopt( (SWS_SOCKET)s, SWS_IPPROTO_IP, SWS_IP_ADD_MEMBERSHIP, &mreq, sizeof( mreq) ))
    return INVALID_SOCKET;

  /* Select the port to receive on */
  from = *(SWS_PTR( struct SWS_sockaddr_in))name;
  from.sin_family = AF_INET;
  from.sin_addr.s_addr = mreq.imr_interface.s_addr;
  if ( SWS_bind( (SWS_SOCKET)s, (SWS_PTR( struct SWS_sockaddr))&from, sizeof(from)))
    return INVALID_SOCKET;

  return s;
  }


EXTERNC __declspec( dllexport) INT WSAAPI SWS_WSAAddressToStringA(
  IN     LPSOCKADDR          lpsaAddress,
  IN     DWORD               dwAddressLength,
  IN     LPWSAPROTOCOL_INFOA lpProtocolInfo,
  IN OUT LPSTR             lpszAddressString,
  IN OUT LPDWORD             lpdwAddressStringLength
) {
  (void)lpsaAddress;
  (void)dwAddressLength;
  (void)lpProtocolInfo;
  (void)lpszAddressString;
  (void)lpdwAddressStringLength;

  TRACE( 1, ("*** SWS_WSAAddressToStringA()\n"));

  SWS_SetLastError( SWS_ENOBUFS);
  return SOCKET_ERROR;
  }


EXTERNC __declspec( dllexport) INT WSAAPI SWS_WSAAddressToStringW(
  IN     LPSOCKADDR          lpsaAddress,
  IN     DWORD               dwAddressLength,
  IN     LPWSAPROTOCOL_INFOW lpProtocolInfo,
  IN OUT LPWSTR             lpszAddressString,
  IN OUT LPDWORD             lpdwAddressStringLength
) {
  (void)lpsaAddress;
  (void)dwAddressLength;
  (void)lpProtocolInfo;
  (void)lpszAddressString;
  (void)lpdwAddressStringLength;

  TRACE( 1, ("*** SWS_WSAAddressToStringW()\n"));

  SWS_SetLastError( SWS_ENOBUFS);
  return SOCKET_ERROR;
  }


EXTERNC __declspec( dllexport) INT WSAAPI SWS_WSAStringToAddressW(
  IN     LPWSTR              AddressString,
  IN     INT                 AddressFamily,
  IN     LPWSAPROTOCOL_INFOW lpProtocolInfo,
  OUT    LPSOCKADDR          lpAddress,
  IN OUT LPINT               lpAddressLength
) {
  (void)AddressString;
  (void)AddressFamily;
  (void)lpProtocolInfo;
  (void)lpAddress;
  (void)lpAddressLength;

  TRACE( 1, ("*** SWS_WSAStringToAddressW()\n"));

  SWS_SetLastError( SWS_ENOMEM);
  return SOCKET_ERROR;
  }


EXTERNC __declspec( dllexport) INT WSAAPI SWS_WSALookupServiceBeginW(
  IN  LPWSAQUERYSETW lpqsRestrictions,
  IN  DWORD          dwControlFlags,
  OUT LPHANDLE       lphLookup
) {
  (void)lpqsRestrictions;
  (void)dwControlFlags;
  (void)lphLookup;

  TRACE( 1, ("*** SWS_WSALookupServiceBeginW()\n"));

  SWS_SetLastError( SWS_ENOMEM);
  return SOCKET_ERROR;
  }


EXTERNC __declspec( dllexport) INT WSAAPI SWS_WSALookupServiceEnd(
  IN HANDLE  hLookup
) {
  (void)hLookup;

  TRACE( 1, ("*** SWS_WSALookupServiceEnd()\n"));

  SWS_SetLastError( SWS_ENOMEM);
  return SOCKET_ERROR;
  }


EXTERNC __declspec( dllexport) INT WSAAPI SWS_WSALookupServiceNextW(
  IN     HANDLE           hLookup,
  IN     DWORD            dwControlFlags,
  IN OUT LPDWORD          lpdwBufferLength,
  OUT    LPWSAQUERYSETW   lpqsResults
) {
  (void)hLookup;
  (void)dwControlFlags;
  (void)lpdwBufferLength;
  (void)lpqsResults;

  TRACE( 1, ("*** SWS_WSALookupServiceNextW()\n"));

  SWS_SetLastError( SWS_ENOMEM);
  return SOCKET_ERROR;
  }

/* End of file */
