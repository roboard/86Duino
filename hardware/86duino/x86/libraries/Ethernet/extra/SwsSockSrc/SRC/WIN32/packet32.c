/****************************************************************************
 *
 * File name   : packet32.c
 * Function    : SwsSock packet driver
 * Project     : SwsSock
 * Authors     : Lawrence Rust
 * Systems     : ANSI C Win32
 *
 ****************************************************************************
 *
 * Created by Lawrence Rust, Software Systems Consultants
 * lvr@softsystem.co.uk. Tel/Fax +33 5 49 72 79 63
 *
 * 27-Mar-03, LVR, Created.
 *
 * Comments:
 * --------
 * This file interfaces the SwsSock NDIS protocol driver to the SwsSock stack
 *
 ****************************************************************************
 */

/* VC6 remote debugging:
 *
 * To the custom build page of the project settings add:
 *
 * Commands:
 * %comspec% /c copy $(InputPath) \\Armstrong\DRIVEC\users\lvr
 *
 * Output:
 * \\Armstrong\DRIVEC\users\lvr\httpd.exe
 *
 * To the debug page (general category) add:
 *
 * Working directory:
 * \\Armstrong\DRIVEC\users\lvr
 *
 * Remote path:
 * \\Armstrong\DRIVEC\users\lvr\httpd.exe
 */

/* Compilation options */
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0400 /* For QueueUserAPC */
#endif

#include "config.h"
#if OPT_DATALINK_ETHERNET

#ifndef OPT_ASYNC_SEND
#define OPT_ASYNC_SEND 1 /* !0 to use async output */
#endif

#ifndef OPT_PACKET_ADAPTERS
#define OPT_PACKET_ADAPTERS 4
#endif


/* Exports */
#include "packet32.h"

/* Imports */
#include <stddef.h>
#include <string.h>   /* memcpy */
#include <errno.h>

/* Win32 */
#if defined __MWERKS__
# pragma ANSI_strict off
#elif defined _MSC_VER
# pragma warning ( disable : 4201) /* nonstandard extension used : nameless struct/union */
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <process.h> /* _beginthreadex */

#if OPT_DNS
#include <iphlpapi.h> /* GetNetworkParams - DNS server */
#ifdef _MSC_VER
#pragma comment( lib, "iphlpapi.lib") /* For GetNetworkParams */
#pragma comment( lib, "delayimp.lib") /* Required for delay loading */
#pragma comment( linker, "/delayload:iphlpapi.dll") /* Delayload iphlpapi.dll */
#endif
#endif

/* Private interface to the NDIS protocol driver */
#include "ioctls.h"

#if defined __MWERKS__
# pragma ANSI_strict reset
#elif defined _MSC_VER
# pragma comment( lib, "advapi32.lib") /* For OpenSCManager */
# pragma warning ( default : 4201)
#endif

/* SwsSock */
#include "sws_enet.h" /* Ethernet driver registration */
#include "sws_dlnk.h" /* SWS_kDatalinkProperty_Promiscuity */
#include "sws_buf.h"  /* Buffer operations */
#include "sws_msgq.h" /* Message queues */
#include "sws_sock.h" /* Error codes */
#if OPT_DNS
#include "sws_dlnk.h"
#include "sws_ip.h"
#endif
#include "platform.h" /* PlatformPropertyGetUint */
#include "debug.h"    /* TRACE, ASSERT & VERIFY */


/*
 * Macros & Constants
 */

/* Configurable vales */
#define PKEY_PACKET_DISABLE "disable"

#define PKEY_PACKET_RXBUFFERS "rxbuffers"
#ifndef OPT_PACKET_RXBUFFERS
#define OPT_PACKET_RXBUFFERS 48
#endif

#if OPT_ASYNC_SEND
#define PKEY_PACKET_TXBUFFERS "txbuffers"
#ifndef OPT_PACKET_TXBUFFERS
#define OPT_PACKET_TXBUFFERS 16
#endif
#endif

/* Protocol driver service name */
#define SWS_SOCK_SERVICE IOCTL_DEVICE

/* Get a ptr to the start of a struct given a ptr to a field within it */
#ifndef SWS_CONTAINER
#define SWS_CONTAINER( ptr, type, field) \
  (type*)( (char*)(ptr) - offsetof( type, field) )
#endif

#define MAX_FRAME_SIZE 1514 /* Ethernet MTU=1500 (+14 byte MAC hdr), no FCS */


/*
 * Local data types
 */

/* Buffer */
typedef struct SBuffer
  {
  OVERLAPPED overlap;
  char buffer[ MAX_FRAME_SIZE];
  } SBuffer;

/* Packet driver instance */
typedef struct SPacketDriver
  {
  ui32 uMagic;
#define PACKET_MAGIC 'pkt2'

  struct SPacketDriver* pNext;        /* List of drivers */
  SWS_EnetCallbacks upcalls;          /* Up-call info */

  HANDLE hDevice;                     /* SwsVpkt protocol driver */

  unsigned long ulReceiver;           /* Receiver thread handle */
  int iReceiverExit;

  SWS_EnetMacAddress mac;             /* Cached MAC address */
  int macAddrIsValid;                 /* !0 if cache is valid */

#if OPT_ASYNC_SEND
  LONG lTxAvailable;
  size_t txNext;
#else
  int isTxPending;
  SBuffer txBuffer;
#endif

  SWS_VARRAY( SBuffer, bufferVLA);    /* VLA of Tx & Rx buffers */
  } SPacketDriver;


/*
 * Prototypes
 */
static SWS_FEnetSend PacketSend;
static SWS_FEnetRecvAck PacketRecvAck;
static SWS_FEnetGetMacAddr PacketGetMacAddr;
static SWS_FEnetShutdown PacketShutdown;
static SWS_FEnetIoctl PacketIoctl;

static int AddPacketDriver( int);
static void FreePacketDriver( SPacketDriver*);
static int SetPromiscuity( SPacketDriver* const,unsigned mode);
#if OPT_IP_MULTICAST
static int SetMulticastList( SPacketDriver* const, SWS_PTR( void), size_t);
#endif
static BOOL OverlappedDeviceIoControl( HANDLE,DWORD,LPVOID,DWORD,LPVOID,DWORD,LPDWORD);
static BOOL StartAService( const char* pszService);

static unsigned __stdcall Receiver( void*);
static BOOL QueueBuffer( SPacketDriver*, SBuffer*);
static VOID WINAPI ReceiveComplete( DWORD, DWORD, LPOVERLAPPED);
static VOID WINAPI ReceiverApc( ULONG_PTR);
#if OPT_ASYNC_SEND
static VOID WINAPI SendComplete( DWORD, DWORD, LPOVERLAPPED);
#endif


/*
 * Module data
 */
/* Configuration */
static size_t s_rxBufferCount;
#if OPT_ASYNC_SEND
static size_t s_txBufferCount;
#endif

/* Operation */
static SPacketDriver* s_pPacketDrivers; /* List of interfaces */
static int s_iPacketAdapters, s_iWanAdapters;

/* Statistics */
static unsigned long s_ulTxPackets, s_ulTxErrors, s_ulTxBytes;
static volatile unsigned long s_ulRxPackets, s_ulRxErrors, s_ulRxBytes;

/* Ethernet device descriptor */
static char s_szIdent[] = SWS_PACKET32_IDENT "01";
static SWS_EnetDevice const s_packet =
  {
  sizeof( s_packet),
  SWS_ENET_VERSION,
  &s_szIdent[ 0],
  &PacketSend,
  &PacketRecvAck,
  &PacketGetMacAddr,
  &PacketShutdown,
  &PacketIoctl
  };


/*
 * Initialise the driver
 * Single threaded
 */
int SWS_Packet32Install( void)          /* Returns !0 on error */
  {
  int i, iAdapters;

  TRACE( 9, ("SWS_Packet32Install()\n"));
  ASSERT( PlatformIsCritical());

  s_iPacketAdapters = s_iWanAdapters = 0;

  /* Get configuration data */
  if ( PlatformPropertyGetUint( SWS_PACKET32_IDENT, PKEY_PACKET_DISABLE, 0))
    {
    TRACE( 2, ("SWS_Packet32Install() Disabled\n"));
    return SWS_ENETDOWN;
    }

  s_rxBufferCount = (size_t)PlatformPropertyGetUint(
    SWS_PACKET32_IDENT, PKEY_PACKET_RXBUFFERS, OPT_PACKET_RXBUFFERS );
#if OPT_ASYNC_SEND
  s_txBufferCount = (size_t)PlatformPropertyGetUint(
    SWS_PACKET32_IDENT, PKEY_PACKET_TXBUFFERS, OPT_PACKET_TXBUFFERS );
#endif

  /* Start the sws_sock protocol driver */
  StartAService( SWS_SOCK_SERVICE);

  /* Search for packet drivers */
  iAdapters = 0;
  for ( i = 0; i < OPT_PACKET_ADAPTERS && iAdapters < OPT_DATALINK_ETHERNET; ++i)
    {
    int err = AddPacketDriver( i);
    if ( !err)
      ++iAdapters;
    }

  if ( 0 == iAdapters)
    {
    TRACE( 1, ("*** SWS_Packet32Install() no adapters found\n"));
    return SWS_ENETDOWN;
    }

#if OPT_DNS
  /* DNS server set? */
  if ( (i = 1, SWS_INADDR_ANY == SWS_DnsGetPriServer())
    || (i = 2, SWS_INADDR_ANY == SWS_DnsGetSecServer())
  ) {
    /* No, get Win32 DNS server */
    UINT const mode = SetErrorMode( SEM_FAILCRITICALERRORS);

#ifdef _MSC_VER
    /* Catch DLL not found exception if Iphlpapi.lib is delay loaded */
    __try
      {
#endif
      char buf[ sizeof( FIXED_INFO) + 0x100];
      PFIXED_INFO pInfo = (PFIXED_INFO)buf;
      ULONG size = sizeof( buf);

      /* NB This can take a few seconds on a slow (<=200MHz) PC */
      if ( ERROR_SUCCESS == GetNetworkParams( pInfo, &size) )
        {
        /* Use last DNS server in list (most recently added) */
        PIP_ADDR_STRING pAddr;
        ui32 val;

        if ( NULL != pInfo->CurrentDnsServer)
          pAddr = pInfo->CurrentDnsServer;
        else
          {
          pAddr = &pInfo->DnsServerList;
          while ( NULL != pAddr->Next)
            pAddr = pAddr->Next;
          }

        val = SWS_inet_addr( pAddr->IpAddress.String);
        if ( SWS_INADDR_NONE != val)
          {
          val = SWS_ntohl( val);
          if ( 1 == i)
            SWS_DnsSetPriServer( val);
          else
            SWS_DnsSetSecServer( val);
#if 0
          /* Add an explicit route to the name server */
          i = SWS_DatalinkFind( SWS_PACKET32_IDENT "1");
          if ( i >= 0)
            {
            ui32 ip;
            size_t size = sizeof( ip);
            if ( !SWS_DatalinkGetProperty(
                (unsigned)i,
                SWS_kDatalinkProperty_Address,
                &ip,
                &size
              )
            ) {
              SWS_RouteAdd( ip, val, ~0UL, val);
              }
            }
#endif
          }
        }
#ifdef _MSC_VER
      }
    __except( EXCEPTION_EXECUTE_HANDLER)
      {
      }
#endif

    SetErrorMode( mode);
    }
#endif

  return 0;
  }


/*
 * Finalise the driver
 * Single threaded
 */
void SWS_Packet32Cleanup( void)
  {
  SPacketDriver* pPacket;

  TRACE( 9, ("SWS_Packet32Cleanup()\n"));
  ASSERT( PlatformIsCritical());

  /* All the drivers should have been shutdown by now */
  while ( NULL != (pPacket = s_pPacketDrivers))
    {
    s_pPacketDrivers = pPacket->pNext;
    FreePacketDriver( pPacket);
    }

  TRACE( 1, ("Pckt32: TxPackets:%lu TxBytes:%lu TxErrors:%lu RxPackets:%lu RxBytes:%lu RxErrors:%lu\n",
    s_ulTxPackets, s_ulTxBytes, s_ulTxErrors,
    s_ulRxPackets, s_ulRxBytes, s_ulRxErrors
  ));
  }


/*
 * Create an SwsSock interface for a packet driver
 */
int AddPacketDriver( int if_num)
  {
  SPacketDriver* pPacket;
  char szName[ sizeof( "\\\\.\\" IOCTL_DEVICE) + 4];
  int iRet;
  unsigned uThreadID;
  IOCTL_INFO info;
  DWORD dwOutputByteCount;
  size_t size;

  TRACE( 9, ("AddPacketDriver(%d)\n", if_num));

  /* Allocate instance data */
  size = sizeof( SPacketDriver) + s_rxBufferCount * sizeof( SBuffer);
#if OPT_ASYNC_SEND
  size += s_txBufferCount * sizeof( SBuffer);
#endif
  pPacket = (SPacketDriver*)calloc( 1, size);
  if ( NULL == pPacket)
    {
    TRACE( 0, ("!!! AddPacketDriver() Out of memory for instance\n"));
    return SWS_ENOMEM;
    }

  pPacket->uMagic = PACKET_MAGIC;
  pPacket->hDevice = INVALID_HANDLE_VALUE;

#if OPT_ASYNC_SEND
  /* Max no. pending transmits */
  pPacket->lTxAvailable = (LONG)s_txBufferCount;
#else
  /* Create overlapped Tx done event */
  pPacket->txBuffer.overlap.hEvent = CreateEvent( NULL, TRUE, FALSE, NULL);
  if ( NULL == pPacket->txBuffer.overlap.hEvent)
    {
    TRACE( 1, ("*** AddPacketDriver() CreateEvent failed(%#lx)\n",GetLastError() ));
    FreePacketDriver( pPacket);
    return SWS_ENOMEM;
    }
#endif

  /* Open SwsVpkt driver */
  sprintf( szName, "\\\\.\\" IOCTL_DEVICE "%d", if_num);
  pPacket->hDevice = CreateFile(
    szName,
    GENERIC_READ | GENERIC_WRITE,
    (1 * FILE_SHARE_READ) | (1 * FILE_SHARE_WRITE),
    NULL, /* Security */
    OPEN_EXISTING,
    (1 * FILE_FLAG_OVERLAPPED) | FILE_ATTRIBUTE_NORMAL,
    NULL /* Template file */
  );
  if ( INVALID_HANDLE_VALUE == pPacket->hDevice)
    {
    DWORD dwErr = GetLastError();
    TRACE( 2, ("AddPacketDriver() CreateFile %s failed(%#lx)\n",
      szName,dwErr));
    FreePacketDriver( pPacket);
    return ERROR_FILE_NOT_FOUND == dwErr ? SWS_ENODEV : SWS_EACCES;
    }

  /* Get adapter type */
  info.bWan = FALSE;
  OverlappedDeviceIoControl(
    pPacket->hDevice,               /* Device handle */
    (DWORD)IOCTL_GETINFO,           /* IOCTL code */
    NULL, 0,                        /* -> input buffer & size */
    &info,                          /* -> output buffer */
    sizeof( info),                  /* input buffer size */
    &dwOutputByteCount              /* Bytes returned */
  );

  /* Create adapter name */
  if ( info.bWan)
    sprintf( s_szIdent, SWS_PACKET32WAN_IDENT "%d", ++s_iWanAdapters);
  else
    sprintf( s_szIdent, SWS_PACKET32_IDENT "%d", ++s_iPacketAdapters);

#if defined _DEBUG && defined IOCTL_GETDESC
  {
  char szDesc[ 256];

  /* Get adapter description */
  if ( OverlappedDeviceIoControl(
    pPacket->hDevice,               /* Device handle */
    (DWORD)IOCTL_GETDESC,           /* IOCTL code */
    NULL, 0,                        /* -> input buffer & size */
    szDesc,                         /* -> output buffer */
    sizeof( szDesc),                /* input buffer size */
    &dwOutputByteCount              /* Bytes returned */
  )){
    TRACE( 2, ("AddPacketDriver() %s: %s\n", s_szIdent,szDesc));
    }
  }
#endif

  /* Is the adapter disabled? */
  if ( PlatformPropertyGetUint( s_szIdent, PKEY_PACKET_DISABLE, 0))
    {
    TRACE( 2, ("AddPacketDriver() %s disabled\n", s_szIdent));
    FreePacketDriver( pPacket);
    return SWS_ENODEV;
    }

  /* Start the receiver */
  pPacket->ulReceiver = _beginthreadex(
    NULL, /* Security */
    0,    /* Stack size */
    &Receiver,
    pPacket,
    CREATE_SUSPENDED,
    &uThreadID
  );
  if ( 0 == pPacket->ulReceiver)
    {
    TRACE( 1, ("*** AddPacketDriver() _beginthread failed(%lx)\n",errno ));
    FreePacketDriver( pPacket);
    return errno;
    }
#if 1
  SetThreadPriority( (HANDLE)pPacket->ulReceiver, THREAD_PRIORITY_ABOVE_NORMAL);
#endif

  /* Register with SwsSock as an Ethernet driver */
  pPacket->upcalls.uSize = sizeof( pPacket->upcalls);
  iRet = SWS_EnetAddAdapter( &s_packet, pPacket, &pPacket->upcalls);
  if ( iRet)
    {
    FreePacketDriver( pPacket);
    return iRet;
    }
  ASSERT( pPacket->upcalls.pfnSendAck);
  ASSERT( pPacket->upcalls.pfnRecvInd);

  /* Allow the thread to run once the callbacks are in place */
  ResumeThread( (HANDLE)pPacket->ulReceiver);

  /* Add to list of drivers */
  pPacket->pNext = s_pPacketDrivers;
  s_pPacketDrivers = pPacket;

  return 0;
  }


/*
 * Start a service
 */
BOOL StartAService( const char* pszService)
  {
  SC_HANDLE schSCManager;
  SC_HANDLE schService;
  SERVICE_STATUS status;
  BOOL bRet = TRUE;

  TRACE( 9, ("StartAService(%s)\n", pszService));
  ASSERT( NULL != pszService);

  schSCManager = OpenSCManager( NULL, NULL, SC_MANAGER_CONNECT);
  if ( NULL == schSCManager)
    {
    TRACE( 1, ("*** StartAService() OpenSCManager error %lu\n", GetLastError() ));
    return FALSE;
    }

  schService = OpenService( schSCManager, pszService, SERVICE_START | SERVICE_QUERY_STATUS);
  if ( NULL != schService)
    {
    if ( StartService( schService, 0, NULL))
      {
      /* Wait until started */
      DWORD dwMaxWait = 3000, dwInterval = 100;
      for(;;)
        {
        if ( !QueryServiceStatus( schService, &status))
          {
          TRACE( 2, ("StartAService() Can't query the '%s' service, error %lu\n",
            pszService, GetLastError() ));
          bRet = FALSE;
          break;
          }
        if ( SERVICE_RUNNING == status.dwCurrentState)
          {
          TRACE( 2, ("StartAService(%s) started\n", pszService));
          SleepEx( dwInterval, TRUE);
          break;
          }

        if ( dwMaxWait < dwInterval)
          {
          TRACE( 2, ("StartAService(%s) timed out starting\n", pszService));
          bRet = FALSE;
          break;
          }
        dwMaxWait -= dwInterval;

        SleepEx( dwInterval, TRUE);
        }
      }
    else
      {
      /* Find the reason the service wasn't started */
      DWORD dwError = GetLastError();
      /* It's ok if the service was already running */
      if ( ERROR_SERVICE_ALREADY_RUNNING != dwError)
        {
        TRACE( 1, ("*** StartAService() Can't start the '%s' service, error %lu\n",
          pszService, dwError ));
        bRet = FALSE;
        }
      }

    CloseServiceHandle( schService);
    }
  else if ( NULL != (schService = OpenService( schSCManager, pszService, SERVICE_QUERY_STATUS)) )
    {
    bRet = QueryServiceStatus( schService, &status);
    if ( bRet)
      {
      bRet = SERVICE_RUNNING == status.dwCurrentState;
      }
    else
      {
      TRACE( 2, ("StartAService() Can't query the '%s' service, error %lu\n",
        pszService, GetLastError() ));
      }

    CloseServiceHandle( schService);
    }
  else
    {
    /* Probably ERROR_ACCESS_DENIED - user doesn't have administrative privilege */
    TRACE( 2, ("StartAService() Can't open the '%s' service, error %lu\n",
      pszService, GetLastError() ));
    bRet = FALSE;
    }

  CloseServiceHandle( schSCManager);
  return bRet;
  }


/*
 * Callback from Ethernet module to shutdown an adapter
 */
static SWS_CB( void, PacketShutdown(
  SWS_PTR( void) pvContext            /* IN: Client context */
)){
  SPacketDriver* pPacket, *pPrev;;

  TRACE( 9, ("PacketShutdown(%p)\n", pvContext));
  ASSERT( NULL != pvContext);

  /* Search for the driver */
  pPrev = NULL;
  for ( pPacket = s_pPacketDrivers; NULL != pPacket; pPacket = pPacket->pNext)
    {
    if ( (SPacketDriver*)pvContext == pPacket)
      {
      /* Unsplice it */
      if ( NULL != pPrev)
        pPrev->pNext = pPacket->pNext;
      else
        s_pPacketDrivers = pPacket->pNext;

      /* Shut it down */
      FreePacketDriver( pPacket);
      return;
      }

    pPrev = pPacket;
    }

  TRACE( 1, ("*** PacketShutdown() Client %p not found)\n", pvContext));
  }


/*
 * Release packet driver resources
 */
void FreePacketDriver( SPacketDriver* pPacket)
  {
  TRACE( 9, ("FreePacketDriver(%p)\n", pPacket));
  ASSERT( NULL != pPacket);

  if ( pPacket->ulReceiver)
    {
    /* Signal the receiver thread to exit */
    pPacket->iReceiverExit = 1;
    switch ( WaitForSingleObject( (HANDLE)pPacket->ulReceiver, 1000))
      {
    case WAIT_OBJECT_0:
      break;

    case WAIT_TIMEOUT:
      /* This will occur if the thread is waiting for the critical section */
      TRACE( 2, ("FreePacketDriver() WaitForSingleObject timed out\n"));

      /* Resolve the deadlock by killing the thread */
      if ( !TerminateThread( (HANDLE)pPacket->ulReceiver, 1))
        {
        TRACE( 1, ("*** FreePacketDriver() TerminateThread failed %lu\n",
          GetLastError() ));
        }
      break;

    case WAIT_FAILED:
    default:
      /* This normally mean that the thread has exited and the handle is invalid */
      TRACE( 3, ("FreePacketDriver() WaitForSingleObject failed %lu\n",
        GetLastError() ));
      break;
      }
    }

  if ( INVALID_HANDLE_VALUE != pPacket->hDevice)
    {
    /* Cancel all pending sends */
    CancelIo( pPacket->hDevice);
    SleepEx( 10, TRUE);

    CloseHandle( pPacket->hDevice);
    pPacket->hDevice = INVALID_HANDLE_VALUE;
    }

#if !OPT_ASYNC_SEND
  if ( NULL != pPacket->txBuffer.overlap.hEvent)
    CloseHandle( pPacket->txBuffer.overlap.hEvent);
#endif

  free( pPacket);
  }


/*
 * Transmit a buffer, possibly fragmented
 * Single threaded
 */
SWS_CB( int, PacketSend(              /* Return !0 on error, -1 if pending */
  SWS_PTR( void) pvContext,           /* IN: Driver's context */
  SWS_BufferH hBuffer                 /* IN: Buffer to send (owned by caller) */
)){
  SPacketDriver* const pPacket = (SPacketDriver*)pvContext;
  SWS_size_t len;

  TRACE( 9, ("PacketSend(%p,%p)\n",pvContext,hBuffer));
  ASSERT( NULL != pPacket);
  ASSERT( PACKET_MAGIC == pPacket->uMagic);

  len = SWS_BufferSize( hBuffer);
  ASSERT( len >= 60 /* Min Ethernet frame */);

#if OPT_ASYNC_SEND
  {
  SBuffer* pBuffer;
  int iRet;

  if ( len > sizeof(pPacket->bufferVLA[0].buffer) )
    {
    TRACE( 1, ("*** PacketSend packet too big\n"));
    return SWS_EINVAL;
    }

  if ( pPacket->lTxAvailable <= 0)
    {
    TRACE( 1, ("*** PacketSend out of buffers\n"));
    return SWS_ENOBUFS;
    }

  /* Return 'pending' if last buffer */
  iRet = InterlockedDecrement( &pPacket->lTxAvailable) <= 0 ? -1 : 0;

  /* Get next available buffer */
  pBuffer = &pPacket->bufferVLA[ s_rxBufferCount + pPacket->txNext];
  if ( ++(pPacket->txNext) >= s_txBufferCount)
    pPacket->txNext = 0;

  pBuffer->overlap.Offset = 0;
  pBuffer->overlap.OffsetHigh = 0;
  pBuffer->overlap.hEvent = pPacket;

  /* Defragment client buffer */
  SWS_BufferCopyToMemory( pBuffer->buffer, hBuffer, 0, len);

  /* Send and forget */
  if ( !WriteFileEx(
      pPacket->hDevice,
      pBuffer->buffer,
      len,
      &pBuffer->overlap,
      &SendComplete
    )
  ) {
    DWORD const dwErr = GetLastError();
    SendComplete( dwErr, 0, &pBuffer->overlap);
    return ERROR_NO_SYSTEM_RESOURCES == dwErr ? SWS_ENOBUFS : SWS_ENETDOWN;
    }

  return iRet;
  }
#else
  {
  BOOL bResult;
  DWORD dwWritten;

  if ( len > sizeof(pPacket->txBuffer.buffer) )
    {
    TRACE( 1, ("*** PacketSend packet too big\n"));
    return SWS_EINVAL;
    }

  if ( pPacket->isTxPending)
    {
    /* Wait for completion */
    bResult = GetOverlappedResult(
      pPacket->hDevice, &pPacket->txBuffer.overlap, &dwWritten, TRUE);
    if ( !bResult)
      {
      TRACE( 1, ("*** PacketSend GetOverlappedResult error %#lx\n",GetLastError() ));
      ++s_ulTxErrors;
      }

    pPacket->isTxPending = 0;
    }

  /* Defragment buffer */
  SWS_BufferCopyToMemory( pPacket->txBuffer.buffer, hBuffer, 0, len);

  /* Request the driver to send the frame */
  pPacket->txBuffer.overlap.Offset = 0;
  pPacket->txBuffer.overlap.OffsetHigh = 0;
  bResult = WriteFile(
    pPacket->hDevice,
    pPacket->txBuffer.buffer,
    len,
    &dwWritten,
    &pPacket->txBuffer.overlap
   );
  if ( !bResult)
    {
    DWORD dwErr = GetLastError();

    if ( ERROR_IO_PENDING == dwErr)
      {
      /* Write is pending */
      pPacket->isTxPending = 1;
      }
    else
      {
      TRACE( 1, ("*** PacketSend WriteFile error %#lx\n",dwErr));
      ++s_ulTxErrors;
      return SWS_BASEERR + 1000;
      }
    }

  ++s_ulTxPackets;
  s_ulTxBytes += len;

  return 0;
  }
#endif
  }


#if OPT_ASYNC_SEND
/*
 * WriteFileEx completion routine
 */
static VOID WINAPI SendComplete(
  DWORD dwErr,                        /* completion code */
  DWORD dwBytes,                      /* number of bytes transferred */
  LPOVERLAPPED lpOverlapped           /* I/O information buffer */
) {
  SPacketDriver* const pPacket = (SPacketDriver*)lpOverlapped->hEvent;

  TRACE( 9, ("SendComplete(%#lx,%lu,%p)\n",dwErr,dwBytes,lpOverlapped));
  ASSERT( NULL != lpOverlapped);
  ASSERT( NULL != pPacket);
  ASSERT( PACKET_MAGIC == pPacket->uMagic);

  switch ( dwErr)
    {
  case 0:
    ++s_ulTxPackets;
    s_ulTxBytes += dwBytes;
    break;

  case ERROR_OPERATION_ABORTED:
    TRACE( 4, ("SendComplete() aborted)\n"));
    break;

  case ERROR_NO_SYSTEM_RESOURCES:
    /* This can happen if driver has too many pending receives */
    TRACE( 2, ("SendComplete() Out of resources)\n"));
    ++s_ulTxErrors;
    break;

  default:
    TRACE( 2, ("SendComplete() error %lu)\n",dwErr));
    ++s_ulTxErrors;
    break;
    }

  if ( 1 == InterlockedIncrement( &pPacket->lTxAvailable))
    (*pPacket->upcalls.pfnSendAck)( pPacket->upcalls.pvContext, 0);
  }
#endif


/*
 * Receiver thread
 */
static unsigned __stdcall Receiver( void* pv)
  {
  SPacketDriver* const pPacket = (SPacketDriver*)pv;
  size_t u;

  TRACE( 3, ("Receiver(%p) starting\n",pPacket ));

  /* Queue all receive buffers */
  for ( u = 0; u < s_rxBufferCount; ++u)
    QueueBuffer( pPacket, &pPacket->bufferVLA[ u]);

  /* Dispatch completion callbacks and APC's until exit */
  while ( !pPacket->iReceiverExit)
    SleepEx( 100, TRUE);

  /* Cancel all pending reads */
  CancelIo( pPacket->hDevice);
  SleepEx( 10, TRUE);

  TRACE( 3, ("Receiver(%p) exiting\n",pPacket ));
  return 0;
  }


/*
 * Post a receive buffer
 */
static BOOL QueueBuffer(
  SPacketDriver* pPacket,
  SBuffer* pBuffer
) {
  TRACE( 9, ("QueueBuffer(%p,%p)\n",pPacket,pBuffer));
  ASSERT( NULL != pPacket);
  ASSERT( PACKET_MAGIC == pPacket->uMagic);
  ASSERT( NULL != pBuffer);

  ZeroMemory( &pBuffer->overlap, sizeof(pBuffer->overlap) );
  pBuffer->overlap.hEvent = (HANDLE)pPacket;

  if ( !ReadFileEx(
      pPacket->hDevice,
      pBuffer->buffer,
      sizeof( pBuffer->buffer),
      &pBuffer->overlap,
      &ReceiveComplete
    )
  ) {
    DWORD dwErr = GetLastError();
    switch ( dwErr)
      {
    case ERROR_OPERATION_ABORTED:
      break;

    default:
      TRACE( 1, ("*** QueueBuffer ReadFileEx error %#08lx\n",dwErr ));
      ++s_ulRxErrors;

      /* Retry later */
      VERIFY( QueueUserAPC( &ReceiverApc, (HANDLE)pPacket->ulReceiver, (ULONG_PTR)pBuffer));
      break;
      }
    return FALSE;
    }

  return TRUE;
  }


/*
 * ReadFileEx completion routine
 */
static VOID WINAPI ReceiveComplete(
  DWORD dwErr,                        /* completion code */
  DWORD dwBytes,                      /* number of bytes transferred */
  LPOVERLAPPED lpOverlapped           /* I/O information buffer */
) {
  SBuffer* const pBuffer = SWS_CONTAINER( lpOverlapped, SBuffer, overlap);
  SPacketDriver* pPacket;
  int iPending;

  TRACE( 9, ("ReceiveComplete(%#lx,%lu,%p)\n",dwErr,dwBytes,lpOverlapped));
  ASSERT( NULL != lpOverlapped);

  pPacket = (SPacketDriver*)pBuffer->overlap.hEvent;
  ASSERT( NULL != pPacket);
  ASSERT( PACKET_MAGIC == pPacket->uMagic);

  switch ( dwErr)
    {
  case 0:
    ++s_ulRxPackets;
    s_ulRxBytes += dwBytes;

    /* Make a receive indication upcall */
    if ( pPacket->upcalls.pfnRecvInd)
      {
      iPending = 0 == (*pPacket->upcalls.pfnRecvInd)(
        pPacket->upcalls.pvContext,
        pBuffer->buffer,
        dwBytes
      );
      }
    else
      {
      iPending = 0;
      }
    break;

  case ERROR_OPERATION_ABORTED:
    iPending = 0;
    break;

  default:
    TRACE( 2, ("ReceiveComplete() error %lu)\n",dwErr));
    ++s_ulRxErrors;
    iPending = 0;
    break;
    }

  /* Is the buffer pending upcall completion? */
  if ( !iPending && !pPacket->iReceiverExit)
    {
    /* No, so resubmit it */
    QueueBuffer( pPacket, pBuffer);
    SleepEx( 0, FALSE); /* Ensure DPCs are run */
    }
  }


/*
 * Receive upcall complete.  Buffer is returned
 * Single threaded
 */
SWS_CB( void, PacketRecvAck(
  SWS_PTR( void) pvContext,           /* IN: Driver's context */
  SWS_PTR( void) pvData,              /* IN: Buffer */
  size_t len                          /* IN: Buffer size */
)){
  SPacketDriver* const pPacket = (SPacketDriver*)pvContext;
  SBuffer* const pBuffer = SWS_CONTAINER( pvData, SBuffer, buffer);
  (void)len;

  TRACE( 9, ("PacketRecvAck(%p,%p,%u)\n",pvContext,pvData,(unsigned)len));
  ASSERT( NULL != pPacket);
  ASSERT( PACKET_MAGIC == pPacket->uMagic);

  /* Buffer is now ready to be re-used */
  if ( !pPacket->iReceiverExit)
    VERIFY( QueueUserAPC( &ReceiverApc, (HANDLE)pPacket->ulReceiver, (ULONG_PTR)pBuffer));
  }


/*
 * Receiver APC
 */
static VOID WINAPI ReceiverApc( ULONG_PTR ulContext)
  {
  SBuffer* const pBuffer = (SBuffer*)ulContext;
  SPacketDriver* pPacket;

  TRACE( 9, ("ReceiverApc(%p)\n",pBuffer));
  ASSERT( NULL != pBuffer);

  pPacket = (SPacketDriver*)pBuffer->overlap.hEvent;
  ASSERT( NULL != pPacket);
  ASSERT( PACKET_MAGIC == pPacket->uMagic);

  /* Resubmit the buffer if still running */
  if ( !pPacket->iReceiverExit)
    QueueBuffer( pPacket, pBuffer);
  }


/*
 * Get the adapter's MAC address
 */
SWS_CB( int, PacketGetMacAddr(        /* Return !0 on error */
  SWS_PTR( void) pvContext,           /* IN: Driver's context */
  SWS_PTR( SWS_EnetMacAddress) pMac   /* OUT: Mac address */
)){
  SPacketDriver* const pPacket = (SPacketDriver*)pvContext;

  TRACE( 9, ("PacketGetMacAddr(%p,%p)\n",pvContext,pMac));
  ASSERT( NULL != pPacket);
  ASSERT( PACKET_MAGIC == pPacket->uMagic);
  ASSERT( NULL != pMac);

  if ( !pPacket->macAddrIsValid)
    {
    BOOL bResult;
    DWORD dwOutputByteCount;

    /* Get the device type etc */
    bResult = OverlappedDeviceIoControl(
      pPacket->hDevice,               /* Device handle */
      (DWORD)IOCTL_GETMACADDR,        /* IOCTL code */
      NULL, 0,                        /* -> input buffer & size */
      &pPacket->mac.addr,             /* -> output buffer */
      sizeof( pPacket->mac.addr),     /* output buffer size */
      &dwOutputByteCount              /* Bytes returned */
    );
    if ( !bResult)
      {
      TRACE( 1, ("*** PacketGetMacAddr DeviceIoControl error %#lx\n",GetLastError() ));
      return 1;
      }

    /* Cache the address for later */
    pPacket->macAddrIsValid = 1;
    }

  *pMac = pPacket->mac;

  return 0;
  }


/*
 * Handle private ioctls
 */
SWS_CB( int, PacketIoctl(             /* Return !0 on error */
  SWS_PTR( void) pvContext,           /* IN: Driver's context */
  unsigned uType,                     /* IN: Type */
  SWS_PTR( void) pvData,              /* I/O: -> data */
  SWS_PTR( size_t) pSize              /* I/O: data size */
)){
  SPacketDriver* const pPacket = (SPacketDriver*)pvContext;

  TRACE( 9, ("PacketIoctl(%p,%u,%p,%p)\n",pvContext,uType,pvData,pSize));
  ASSERT( NULL != pPacket);
  ASSERT( PACKET_MAGIC == pPacket->uMagic);

  switch ( uType)
    {
  default:
    TRACE( 3, ("PacketIoctl %u not implemented\n", uType));
    return SWS_EBADF;

  case SWS_kEnetIoctl_Promiscuity:
    /* En/dis/able promiscuous mode */
    if ( *pSize != sizeof(ui32))
      return SWS_EINVAL;

    return SetPromiscuity( pPacket, *((SWS_PTR( ui32)) pvData) ? 1 : 0);

#if OPT_IP_MULTICAST
  case SWS_kEnetIoctl_MulticastList:
    /* Set multicast list */
    return SetMulticastList( pPacket, pvData, *pSize);
#endif

#if OPT_DEMO
  case SWS_kDatalinkProperty_Private + 0x1234:
    if ( *pSize != sizeof(ui32))
      return SWS_EINVAL;

    *(SWS_PTR( ui32))pvData = (*(SWS_PTR( ui32))pvData) ^ 0xDEC0DEA5L;
    return 0;
#endif
    }
  }


/*
 * Set receive promiscuity
 */
static int SetPromiscuity(
  SPacketDriver* const pPacket,
  unsigned mode
) {
  BOOL bResult;
  DWORD dwOutputByteCount;
  BYTE bMode;

  TRACE( 9, ("SetPromiscuity(%p,%u)\n",pPacket,mode));
  ASSERT( NULL != pPacket);
  ASSERT( PACKET_MAGIC == pPacket->uMagic);

  bMode = (BYTE)mode;
  bResult = OverlappedDeviceIoControl(
    pPacket->hDevice,               /* Device handle */
    (DWORD)IOCTL_SETPROMISCUITY,    /* IOCTL code */
    &bMode,                         /* -> input buffer */
    sizeof( bMode),                 /* input buffer size */
    NULL, 0,                        /* -> output buffer & size */
    &dwOutputByteCount              /* Bytes returned */
  );
  if ( !bResult)
    {
    TRACE( 1, ("*** SetPromiscuity DeviceIoControl error %#lx\n",GetLastError() ));
    return 1;
    }

  return 0;
  }


#if OPT_IP_MULTICAST
/*
 * Set multicast list
 */
static int SetMulticastList(
  SPacketDriver* const pPacket,
  SWS_PTR( void) pv,
  size_t size
) {
  BOOL bResult;
  DWORD dwOutputByteCount;

  TRACE( 9, ("SetMulticastList(%p,%p,%u)\n",pPacket,pv,size));
  ASSERT( NULL != pPacket);
  ASSERT( PACKET_MAGIC == pPacket->uMagic);

  bResult = OverlappedDeviceIoControl(
    pPacket->hDevice,               /* Device handle */
    (DWORD)IOCTL_SETMULTICASTLIST,  /* IOCTL code */
    pv,                             /* -> input buffer */
    size,                           /* input buffer size */
    NULL, 0,                        /* -> output buffer & size */
    &dwOutputByteCount              /* Bytes returned */
  );
  if ( !bResult)
    {
    TRACE( 1, ("*** SetMulticastList DeviceIoControl error %#lx\n",GetLastError() ));
    return SWS_EINVAL;
    }

#if 0
  {
  void* pvList = _alloca( size = 6 * 64);
  bResult = OverlappedDeviceIoControl(
    pPacket->hDevice,               /* Device handle */
    (DWORD)IOCTL_GETMULTICASTLIST,  /* IOCTL code */
    NULL, 0,                        /* -> input buffer & size */
    pvList, size,                   /* -> output buffer & size */
    &dwOutputByteCount              /* Bytes returned */
  );
  if ( !bResult)
    TRACE( 1, ("*** GetMulticastList DeviceIoControl error %#lx\n",GetLastError() ));
  }
#endif

  return 0;
  }
#endif


/*
 * Make a synchronous DeviceIoControl call to a handle opened using FILE_FLAG_OVERLAPPED
 */
static BOOL OverlappedDeviceIoControl(
  HANDLE hDevice,
  DWORD dwIoControlCode,
  LPVOID lpInBuffer,
  DWORD nInBufferSize,
  LPVOID lpOutBuffer,
  DWORD nOutBufferSize,
  LPDWORD lpBytesReturned
) {
  BOOL bResult;
  HANDLE hEvent;
  OVERLAPPED overlap;

  /* Create a manual reset event for overlapped wait */
  hEvent = CreateEvent( NULL, TRUE, FALSE, NULL);
  if ( NULL == hEvent)
    return FALSE;

  ZeroMemory( &overlap, sizeof( overlap) );
  overlap.hEvent = hEvent;

  bResult = DeviceIoControl(
    hDevice,
    dwIoControlCode,
    lpInBuffer,
    nInBufferSize,
    lpOutBuffer,
    nOutBufferSize,
    lpBytesReturned,
    &overlap
  );
  if ( !bResult
    && ERROR_IO_PENDING == GetLastError()
  ) {
    /* Wait for completion */
    bResult = GetOverlappedResult( hDevice, &overlap, lpBytesReturned, TRUE);
    }

  CloseHandle( hEvent);

  return bResult;
  }

#endif /* OPT_DATALINK_ETHERNET */
/* End of file */
