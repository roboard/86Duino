/****************************************************************************
 *
 * File name   : w32comms.c
 * Function    : Win32 serial comms driver
 * Project     : SWSSOCK
 * Authors     : Lawrence Rust
 * Systems     : ANSI C.
 *
 ****************************************************************************
 *
 * Created by Lawrence Rust, Software Systems Consultants, Cambridge, UK.
 * lvr@softsystem.co.uk. Tel/Fax +44 1223 862391
 *
 * 24-Oct-01, LVR, Created.
 * 11-Feb-02  LVR, Add InterlockedExchange to W32CommsSend/Recv to prevent the
 *                 comms thrwad from indicating FileWrite/Read ops completed
 *                 synchronously.
 * 25-May-05, LVR, Revised.
 * 16-Jun-05, LVR, Support multiple ports
 * 03-Aug-05, LVR, P2P support
 *
 * Comments:
 * --------
 * This file implements the Win32 serial comms driver
 *
 ****************************************************************************
 */

/* Compilation options */
#ifndef W32COMMS_AUTORTS
#define W32COMMS_AUTORTS 0 /* !0 to enable automatic control of RTS in half duplex */
#endif

#ifndef OPT_SERIAL_ADAPTERS
#define OPT_SERIAL_ADAPTERS 4 /* Max no. serial,ports */
#endif

#include "config.h"   /* Build options */

/* Exports */
#include "w32comms.h"

#if OPT_DATALINK_P2P
/* Imports */
#include <stddef.h>
#include <string.h>   /* memcpy, strcmp */
#include <errno.h>
#include <process.h> /* _beginthreadex */

#include "sws_sock.h" /* SWS_ENOBUFS */
#include "sws_str.h"  /* SWS_stricmp */
#include "sws_buf.h"  /* Buffer operations */
#include "p2p.h"      /* P2P driver registration */
#include "platform.h"

#include "debug.h"    /* TRACE, ASSERT & VERIFY */


/*
 * Macros & Constants
 */

/* Configurable vales */
#define PKEY_PORT   "port"    /* ASCSIIZ device name (no leading \\.\) */
#ifndef W32COMMS_PORT
#define W32COMMS_PORT ""
#endif

#define PKEY_BAUD   "baud"    /* Numeric */
#ifndef W32COMMS_BAUD
#define W32COMMS_BAUD 38400
#endif

#define PKEY_BITS   "bits"    /* Numeric */
#ifndef W32COMMS_BITS
#define W32COMMS_BITS 8
#endif

#define PKEY_PARITY "parity"  /* ASCIIZ odd, even, none, mark, space */
#ifndef W32COMMS_PARITY
#define W32COMMS_PARITY "none"
#endif

#define PKEY_STOP   "stop"    /* Numeric */
#ifndef W32COMMS_STOP
#define W32COMMS_STOP 1
#endif

#define PKEY_DUPLEX "duplex"  /* Numeric, boolean */
#ifndef W32COMMS_DUPLEX
#define W32COMMS_DUPLEX TRUE
#endif

#define PKEY_CTS "cts"        /* Numeric, boolean */
#ifndef W32COMMS_CTS
#define W32COMMS_CTS FALSE
#endif

#define PKEY_DSR "dsr"        /* Numeric, boolean */
#ifndef W32COMMS_DSR
#define W32COMMS_DSR FALSE
#endif

#define PKEY_DTR "dtr"        /* ASCIIZ enable, disable, handshake */
#ifndef W32COMMS_DTR
#define W32COMMS_DTR "enable"
#endif

#define PKEY_RTS "rts"        /* ASCIIZ enable, disable, handshake */
#ifndef W32COMMS_RTS
#define W32COMMS_RTS "enable"
#endif

/*
 * Local data types
 */
/* Port instance */
typedef struct SW32Comms
  {
  ui32 uMagic;
#define W32COMMS_MAGIC 'comm'

  struct SW32Comms* pNext;            /* List of drivers */

  /* Configuration */
  int iPort;
  BOOL bDuplex;

  SWS_P2pCallbacks upcalls;           /* Up call info */

  /* Operation */
  HANDLE hDevice;                     /* Handle to comms device */
  HANDLE hThread;                     /* Comms event handler thread */
  OVERLAPPED txOverlapped;
  LONG lTxPending;
  OVERLAPPED rxOverlapped;
  LONG lRxPending;
  OVERLAPPED evOverlapped;
  LONG lEvPending;
  DWORD dwEvent;
#if !W32COMMS_AUTORTS
  HANDLE hTxEmptyEvent;
#endif
  } SW32Comms;


/*
 * Prototypes
 */
static int AddSerialPort( int);
static void FreeSerialPort( SW32Comms*);

static SWS_FP2pSend W32CommsSend;
static SWS_FASTCALLDEF( int, W32CommsSendComplete( SW32Comms*, int));
static SWS_FP2pRecv W32CommsRecv;
static int W32CommsRecvComplete( SW32Comms*, int, DWORD);
static SWS_FP2pShutdown W32CommsShutdown;
static SWS_FP2pIoctl W32CommsIoctl;
static SWS_FASTCALLDEF( void, W32CommsEvComplete( SW32Comms*, int));
static unsigned __stdcall Win32CommsThread( void*);
static SWS_FASTCALLDEF( int, W32CommsOnline( SW32Comms*, int));
static SWS_FASTCALLDEF( unsigned, W32CommsStatus( SW32Comms*));


/*
 * Module data
 */

/* Operation */
static SW32Comms* s_pAdapters; /* List of interfaces */

/* P2P device descriptor */
static char s_szIdent[] = SWS_W32COMMS_IDENT "01";
static const SWS_P2pDevice s_W32Comms =
  {
  sizeof( SWS_P2pDevice),
  SWS_P2P_VERSION,
  s_szIdent,
  &W32CommsSend,
  &W32CommsRecv,
  &W32CommsShutdown,
  &W32CommsIoctl
  };


/*
 * Initialise the driver
 */
int SWS_Win32CommsInstall()           /* Returns !0 on error */
  {
  int i, iAdapters;

  TRACE( 9, ("SWS_Win32CommsInstall()\n"));
  ASSERT( PlatformIsCritical());

  /* Search for serial ports */
  iAdapters = 0;
  for ( i = 0; i < OPT_SERIAL_ADAPTERS && iAdapters < OPT_DATALINK_P2P; ++i)
    {
    int err = AddSerialPort( i);
    if ( !err)
      ++iAdapters;
    }

  if ( 0 == iAdapters)
    {
    TRACE( 1, ("*** SWS_Win32CommsInstall() no adapters found\n"));
    return SWS_ENETDOWN;
    }
  return 0;
  }


/*
 * Finalise the driver
 * Single threaded
 */
void SWS_Win32CommsCleanup( void)
  {
  SW32Comms* pAdapter;

  TRACE( 9, ("SWS_Win32CommsCleanup()\n"));
  ASSERT( PlatformIsCritical());

  /* All the drivers should have been shutdown by now */
  while ( NULL != (pAdapter = s_pAdapters))
    {
    s_pAdapters = pAdapter->pNext;
    FreeSerialPort( pAdapter);
    }
  }


/*
 * Create an SwsSock interface for a serial port
 */
int AddSerialPort( int iPort)
  {
  SW32Comms* pAdapter;
  int iRet;
  static const char kszPrefix[] = "\\\\.\\";
  char szBuf[64];

  TRACE( 9, ("AddSerialPort(%d)\n",iPort));

  sprintf( s_szIdent, SWS_W32COMMS_IDENT "%d", 1 + iPort);

  /* Get comms port name */
  strcpy( szBuf, kszPrefix);
  iRet = PlatformPropertyGetString(
    s_szIdent,
    PKEY_PORT,
    W32COMMS_PORT,
    szBuf + sizeof(kszPrefix) - 1,
    sizeof( szBuf) - (sizeof(kszPrefix) - 1)
  );
  if ( 0 == iRet)
    {
    TRACE( 2, ("AddSerialPort(%d) skip %s\n", iPort, s_szIdent));
    return SWS_ENODEV;
    }

  /* Allocate instance data */
  pAdapter = (SW32Comms*) calloc( 1, sizeof(SW32Comms));
  if ( NULL == pAdapter)
    {
    TRACE( 0, ("!!! AddSerialPort(%d) Out of memory for instance\n", iPort));
    return SWS_ENOMEM;
    }

  /* Open the device */
  pAdapter->hDevice = CreateFile(
    szBuf,
    GENERIC_READ | GENERIC_WRITE,
    0, /* Share mode MBZ according to MSDN 'Communications Resource Handles' */
    NULL,         /* No security */
    OPEN_EXISTING,
    FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
    NULL
  );
  if ( INVALID_HANDLE_VALUE == pAdapter->hDevice)
    {
    DWORD dwErr = GetLastError();
    TRACE( 2, ("AddSerialPort() CreateFile '%s' failed error %d\n",
      szBuf, dwErr ));
    FreeSerialPort( pAdapter);
    return ERROR_FILE_NOT_FOUND == dwErr ? SWS_ENODEV : SWS_EACCES;
    }

  pAdapter->uMagic = W32COMMS_MAGIC;
  pAdapter->iPort = iPort;

  /* Set comms parameters */
  for ( ;;)
    {
    DCB dcb;
    COMMTIMEOUTS timeout;
    ui32 stop;
    char szParity[16];
    unsigned uThreadID;

    /* Get current state */
    dcb.DCBlength = sizeof( dcb);
    if ( !GetCommState( pAdapter->hDevice, &dcb) )
      {
      TRACE( 0, ("!!! AddSerialPort() GetCommState failed error %d\n",
        GetLastError() ));
      iRet = SWS_EBADF;
      break;
      }

    dcb.fBinary = TRUE;
    dcb.EofChar = 0;
    dcb.fErrorChar = FALSE;
    dcb.fNull = FALSE;
    dcb.fAbortOnError = FALSE;

    /* Disable Xon/Xoff */
    dcb.fOutX = FALSE;
    dcb.fInX = FALSE;

    /* Setup handshake line */
    dcb.fDsrSensitivity = FALSE;
    dcb.fOutxDsrFlow = PlatformPropertyGetUint(
      s_szIdent, PKEY_DSR, W32COMMS_DSR );
    dcb.fOutxCtsFlow = PlatformPropertyGetUint(
      s_szIdent, PKEY_CTS, W32COMMS_CTS );
    PlatformPropertyGetString(
      s_szIdent,
      PKEY_DTR,
      W32COMMS_DTR,
      szBuf,
      sizeof( szBuf)
    );
    if ( !SWS_stricmp( szBuf, "enable"))
      dcb.fDtrControl = DTR_CONTROL_ENABLE;
    else if ( !SWS_stricmp( szBuf, "disable"))
      dcb.fDtrControl = DTR_CONTROL_DISABLE;
    else if ( !SWS_stricmp( szBuf, "handshake"))
      dcb.fDtrControl = DTR_CONTROL_HANDSHAKE;
    else
      TRACE( 0, ("!!! AddSerialPort() Invalid DTR option '%s'\n", szBuf ));

    pAdapter->bDuplex = (BOOL) PlatformPropertyGetUint(
      s_szIdent, PKEY_DUPLEX, W32COMMS_DUPLEX );
    if ( pAdapter->bDuplex)
      {
      /* Full duplex */
      PlatformPropertyGetString(
        s_szIdent,
        PKEY_RTS,
        W32COMMS_RTS,
        szBuf,
        sizeof( szBuf)
      );
      if ( !SWS_stricmp( szBuf, "enable"))
        dcb.fRtsControl = RTS_CONTROL_ENABLE;
      else if ( !SWS_stricmp( szBuf, "disable"))
        dcb.fRtsControl = RTS_CONTROL_DISABLE;
      else if ( !SWS_stricmp( szBuf, "handshake"))
        dcb.fRtsControl = RTS_CONTROL_HANDSHAKE;
      else
        TRACE( 0, ("!!! AddSerialPort() Invalid RTS option '%s'\n", szBuf ));
      }
    else
      {
      /* Half duplex */
#if W32COMMS_AUTORTS
      /* RTS line will be high if bytes are available for transmission.
       * After all buffered bytes have been sent the RTS line will be low */
      dcb.fRtsControl = RTS_CONTROL_TOGGLE;
#else
      dcb.fRtsControl = RTS_CONTROL_DISABLE;
#endif
      }

    /* Set baud rate */
    dcb.BaudRate = PlatformPropertyGetUint(
      s_szIdent, PKEY_BAUD, W32COMMS_BAUD );

    /* Set parity */
    PlatformPropertyGetString(
      s_szIdent,
      PKEY_PARITY,
      W32COMMS_PARITY,
      szParity,
      sizeof( szParity)
    );
    if ( !SWS_stricmp( szParity, "none")
      || !strcmp( szParity, "0")
    ) {
      dcb.fParity = FALSE;
      dcb.Parity = NOPARITY;
      }
    else if ( !SWS_stricmp( szParity, "odd")
      || !strcmp( szParity, "1")
    ) {
      dcb.fParity = TRUE;
      dcb.Parity = ODDPARITY;
      }
    else if ( !SWS_stricmp( szParity, "even")
      || !strcmp( szParity, "2")
    ) {
      dcb.fParity = TRUE;
      dcb.Parity = EVENPARITY;
      }
    else if ( !SWS_stricmp( szParity, "mark"))
      {
      dcb.fParity = TRUE;
      dcb.Parity = MARKPARITY;
      }
    else if ( !SWS_stricmp( szParity, "space"))
      {
      dcb.fParity = TRUE;
      dcb.Parity = SPACEPARITY;
      }
    else
      TRACE( 0, ("!!! AddSerialPort() Invalid parity '%s'\n", szParity ));

    stop = PlatformPropertyGetUint( s_szIdent, PKEY_STOP, W32COMMS_STOP );
    if ( 1 == stop)
      dcb.StopBits = ONESTOPBIT;
    else if ( 2 == stop)
      dcb.StopBits = TWOSTOPBITS;
    else
      TRACE( 0, ("!!! AddSerialPort() Invalid stop bits %lu\n", stop ));

    dcb.ByteSize = (BYTE)PlatformPropertyGetUint(
      s_szIdent, PKEY_BITS, W32COMMS_BITS );

    if ( !SetCommState( pAdapter->hDevice, &dcb))
      {
      TRACE( 0, ("!!! AddSerialPort() SetCommState failed error %d\n",
        GetLastError() ));
      iRet = SWS_EINVAL;
      break;
      }

    /* Set input & output buffer sizes */
    VERIFY( PurgeComm( pAdapter->hDevice, PURGE_RXCLEAR | PURGE_TXCLEAR));
    VERIFY( SetupComm( pAdapter->hDevice, OPT_PPP_RXBUFFER, OPT_PPP_TXBUFFER));

    /* For Rx, use an inter character timeout */
    timeout.ReadIntervalTimeout = 0;
    timeout.ReadTotalTimeoutMultiplier = 0; /* ms per byte, 0= not used */
    timeout.ReadTotalTimeoutConstant = 0; /* millisecs, 0= no timeout ~*/
    /* Disable Tx timeouts */
    timeout.WriteTotalTimeoutMultiplier = 0; /* ms per byte, 0= not used */
    timeout.WriteTotalTimeoutConstant = 0; /* millisecs, 0= no timeout ~*/
    VERIFY( SetCommTimeouts( pAdapter->hDevice, &timeout));

    /* Disable comm events */
    VERIFY( SetCommMask( pAdapter->hDevice, 0));

    /* Allocate tx, rx & ev manual reset events */
    memset( &pAdapter->txOverlapped, 0, sizeof( pAdapter->txOverlapped));
    pAdapter->txOverlapped.hEvent = CreateEvent( NULL, TRUE, FALSE, NULL);
    if ( NULL == pAdapter->txOverlapped.hEvent)
      {
      TRACE( 0, ("!!! AddSerialPort() CreateEvent tx failed error %d\n",
        GetLastError() ));
      iRet = SWS_ENOMEM;
      break;
      }
    memset( &pAdapter->rxOverlapped, 0, sizeof( pAdapter->rxOverlapped));
    pAdapter->rxOverlapped.hEvent = CreateEvent( NULL, TRUE, FALSE, NULL);
    if ( NULL == pAdapter->rxOverlapped.hEvent)
      {
      TRACE( 0, ("!!! AddSerialPort() CreateEvent rx failed error %d\n",
        GetLastError() ));
      iRet = SWS_ENOMEM;
      break;
      }
    memset( &pAdapter->evOverlapped, 0, sizeof( pAdapter->evOverlapped));
    pAdapter->evOverlapped.hEvent = CreateEvent( NULL, TRUE, FALSE, NULL);
    if ( NULL == pAdapter->evOverlapped.hEvent)
      {
      TRACE( 0, ("!!! AddSerialPort() CreateEvent ev failed error %d\n",
        GetLastError() ));
      iRet = SWS_ENOMEM;
      break;
      }
#if !W32COMMS_AUTORTS
    pAdapter->hTxEmptyEvent = CreateEvent( NULL, TRUE, FALSE, NULL);
    if ( NULL == pAdapter->hTxEmptyEvent)
      {
      TRACE( 0, ("!!! AddSerialPort() CreateEvent txempty failed error %d\n",
        GetLastError() ));
      iRet = SWS_ENOMEM;
      break;
      }
#endif

    /* Create the device event handler thread */
    pAdapter->hThread = (HANDLE)_beginthreadex(
      NULL,
      0,
      &Win32CommsThread,
      pAdapter,
      CREATE_SUSPENDED,
      &uThreadID
    );
    if ( 0 == pAdapter->hThread)
      {
      TRACE( 0, ("!!! AddSerialPort() _beginthreadex failed error %d\n", errno ));
      iRet = errno;
      break;
      }

    SetThreadPriority( pAdapter->hThread, THREAD_PRIORITY_HIGHEST);
    ResumeThread( pAdapter->hThread);

    /* Enable comm events */
#if !W32COMMS_AUTORTS
    pAdapter->dwEvent = EV_DSR | EV_RLSD | EV_RING | EV_TXEMPTY;
#else
    pAdapter->dwEvent = EV_DSR | EV_RLSD | EV_RING;
#endif
    VERIFY( SetCommMask( pAdapter->hDevice, pAdapter->dwEvent));
    pAdapter->lEvPending = 1;
    W32CommsEvComplete( pAdapter, 0);

    /* Register a P2P adapter */
    iRet = SWS_P2pAddAdapter( &s_W32Comms, pAdapter, &pAdapter->upcalls);
    if ( iRet)
      break;

    ASSERT( pAdapter->upcalls.pfnSendAck);
    ASSERT( pAdapter->upcalls.pfnRecvInd);
    ASSERT( pAdapter->upcalls.pfnStatInd);

    /* Add to list of drivers */
    pAdapter->pNext = s_pAdapters;
    s_pAdapters = pAdapter;

    return 0;
    }

  ASSERT( iRet);

  /* Release resources on error */
  FreeSerialPort( pAdapter);

  return iRet;
  }


/*
 * Callback from P2P module to shutdown an adapter
 * Single threaded
 */
static SWS_CB( void, W32CommsShutdown(
  SWS_PTR( void) pvContext            /* IN: Client context */
)){
  SW32Comms* pAdapter, *pPrev;;

  TRACE( 9, ("W32CommsShutdown(%p)\n", pvContext));
  ASSERT( NULL != pvContext);

  /* Search for the driver */
  pPrev = NULL;
  for ( pAdapter = s_pAdapters; NULL != pAdapter; pAdapter = pAdapter->pNext)
    {
    if ( (SW32Comms*)pvContext == pAdapter)
      {
      /* Unsplice it */
      if ( NULL != pPrev)
        pPrev->pNext = pAdapter->pNext;
      else
        s_pAdapters = pAdapter->pNext;

      /* Shut it down */
      FreeSerialPort( pAdapter);
      return;
      }

    pPrev = pAdapter;
    }

  TRACE( 1, ("*** W32CommsShutdown() Client %p not found)\n", pvContext));
  }


/*
 * Release serial driver resources
 */
void FreeSerialPort( SW32Comms* pAdapter)
  {
  HANDLE h;

  TRACE( 9, ("FreeSerialPort(%p)\n",pAdapter));

  /* Release resources */
  h = pAdapter->hDevice;
  if ( h
    && INVALID_HANDLE_VALUE != h
  ) {
    /* Indicate to event thread that it should terminate */
    pAdapter->hDevice = INVALID_HANDLE_VALUE;

    /* Wait a short while to drain Tx */
    Sleep( 30);

    /* Cancel all pending I/O */
    PurgeComm( h, PURGE_TXABORT | PURGE_RXABORT);

    /* Close the device */
    CloseHandle( h);
    h = INVALID_HANDLE_VALUE;
    }

#if !W32COMMS_AUTORTS
  if ( NULL != pAdapter->hTxEmptyEvent)
    {
    SetEvent( pAdapter->hTxEmptyEvent);

    CloseHandle( pAdapter->hTxEmptyEvent);
    pAdapter->hTxEmptyEvent = NULL;
    }
#endif

  if ( NULL != pAdapter->evOverlapped.hEvent)
    {
    /* Wake event thread */
    SetEvent( pAdapter->evOverlapped.hEvent);

    CloseHandle( pAdapter->evOverlapped.hEvent);
    pAdapter->evOverlapped.hEvent = NULL;
    }

  if ( NULL != pAdapter->rxOverlapped.hEvent)
    {
    /* Wake event thread */
    SetEvent( pAdapter->rxOverlapped.hEvent);

    CloseHandle( pAdapter->rxOverlapped.hEvent);
    pAdapter->rxOverlapped.hEvent = NULL;
    }

  if ( NULL != pAdapter->txOverlapped.hEvent)
    {
    /* Wake event thread */
    SetEvent( pAdapter->txOverlapped.hEvent);

    CloseHandle( pAdapter->txOverlapped.hEvent);
    pAdapter->txOverlapped.hEvent = NULL;
    }

  if ( pAdapter->hThread)
    {
    /* Wait for event thread to die */
    WaitForSingleObject( pAdapter->hThread, 300);
    CloseHandle( pAdapter->hThread);
    }

  free( pAdapter);
  }


/*
 * Clear any comms errors
 */
static SWS_FASTCALL( DWORD, W32CommsClearErrors( SW32Comms* const pAdapter))
  {
  DWORD dwErrors;

  if ( ClearCommError( pAdapter->hDevice, &dwErrors, NULL))
    {
    if ( dwErrors)
      {
      if ( CE_BREAK & dwErrors)
        {
        /* This happens after switching from Tx to Rx in half duplex mode
        * following a long frame or series of frames are sent. */
        TRACE( 2, ("W32CommsClearErrors() break detected\n"));
        }

      if ( CE_FRAME & dwErrors)
        TRACE( 1, ("*** W32CommsClearErrors() framing error\n"));
      if ( CE_RXPARITY & dwErrors)
        TRACE( 1, ("*** W32CommsClearErrors() parity error\n"));
      if ( CE_OVERRUN & dwErrors)
        TRACE( 1, ("*** W32CommsClearErrors() overrun error\n"));
      if ( CE_RXOVER & dwErrors)
        TRACE( 1, ("*** W32CommsClearErrors() Rx buffer overflow\n"));
      }
    else
      {
      /*TRACE( 1, ("*** W32CommsClearErrors() No error detected?\n"));*/
      }
    }
  else
    {
    TRACE( 1, ("*** W32CommsClearErrors() ClearCommError failed %08lx\n",
      GetLastError() ));
    dwErrors = 0;
    }

  return dwErrors;
  }


/*
 * Transmit a buffer
 * Single threaded
 * May be re-entered from SWS_FP2pSendAck upcall
 */
static SWS_CB( int, W32CommsSend(     /* Return !0 on error, -1 if pending */
  SWS_PTR( void) pvContext,           /* IN: Driver's context */
  SWS_PTR( const void) pvData,        /* IN: Data to send */
  size_t len                          /* IN: Byte count */
)){
  SW32Comms* const pAdapter = (SW32Comms*)pvContext;
  DWORD dwWritten;

  TRACE( 9, ("W32CommsSend(%p,%p,%u)\n",pvContext,pvData,len));
  ASSERT( NULL != pAdapter);
  ASSERT( W32COMMS_MAGIC == pAdapter->uMagic);
  ASSERT( NULL != pvData);
  ASSERT( len > 0);
  /*ASSERT( PlatformIsCritical()); Only valid if not in callback */

  if ( INVALID_HANDLE_VALUE == pAdapter->hDevice)
    return SWS_EBADF; /* In W32CommsShutdown */

#if !W32COMMS_AUTORTS
  if ( !pAdapter->bDuplex)
    {
    ResetEvent( pAdapter->hTxEmptyEvent);

    /* Assert RTS to enable transmitter */
    VERIFY( EscapeCommFunction( pAdapter->hDevice, SETRTS));
    }
#endif

  if ( InterlockedExchange( &pAdapter->lTxPending, 1))
    {
    /* Serious error - there is already a buffer queued */
    TRACE( 1, ("*** W32CommsSend is busy\n"));
    return -(EBUSY);
    }

  ResetEvent( pAdapter->txOverlapped.hEvent);
  if ( !WriteFile( pAdapter->hDevice, pvData, len, &dwWritten, &pAdapter->txOverlapped))
    {
    if ( ERROR_IO_PENDING == GetLastError())
      {
      TRACE( 4, ("W32CommsSend() %u pending\n", len));
      return -1; /* -1 = pending */
      }
    else
      {
      TRACE( 0, ("!!! W32CommsSend WriteFile error %d\n", GetLastError()));
      return W32CommsSendComplete( pAdapter, SWS_EBADF);
      }
    }

  /* On Win98 the overlapped event is signalled even for sync completion
   * so the 1st thread to reset the pending flag indicates op complete */
  if ( !InterlockedExchange( &pAdapter->lTxPending, 0))
    return -1; /* Pending */

  TRACE( 3, ("W32CommsSend() wrote %lu\n",dwWritten));
  return W32CommsSendComplete( pAdapter, 0);
  }


static SWS_FASTCALL( int, W32CommsSendComplete( SW32Comms* pAdapter, int iErr))
  {
  pAdapter->lTxPending = 0;
  ResetEvent( pAdapter->txOverlapped.hEvent);

  if (iErr)
    W32CommsClearErrors( pAdapter);

#if !W32COMMS_AUTORTS
  if ( !pAdapter->bDuplex)
    {
    if ( !iErr)
      {
      VERIFY( FlushFileBuffers( pAdapter->hDevice));

      /* Wait for TxEmpty event */
      WaitForSingleObject( pAdapter->hTxEmptyEvent, INFINITE);
      }

    /* NB WaitCommEvent does not wait for last character! */
    Sleep( 2);

    /* Disable RTS */
    VERIFY( EscapeCommFunction( pAdapter->hDevice, CLRRTS));
    }
#endif

  /* Indicate transmit complete */
  ASSERT( pAdapter->upcalls.pfnSendAck);
  (*pAdapter->upcalls.pfnSendAck)( pAdapter->upcalls.pvContext, iErr);

  return -1;
  }


/*
 * Start receiving.
 * Single threaded
 * May be re-entered from SWS_FP2pRecvInd upcall
 */
static SWS_CB( int, W32CommsRecv(
  SWS_PTR( void) pvContext,           /* IN: Driver's context */
  SWS_PTR( void) pvData,              /* IN: Buffer */
  SWS_PTR( size_t) pLen,              /* I/O: Buffer size */
  int iEnd,                           /* IN: Delimiter, -ve if none */
  ui32 uTimeout                       /* IN: Timeout, 0= none, 1= immediate */
)){
  SW32Comms* const pAdapter = (SW32Comms*)pvContext;
  DWORD dwRead;
  COMMTIMEOUTS timeout;

  TRACE( 9, ("W32CommsRecv(%p,%p,%p,%d,%lu)\n",pvContext,pvData,pLen,iEnd,uTimeout));
  ASSERT( NULL != pAdapter);
  ASSERT( W32COMMS_MAGIC == pAdapter->uMagic);
  /*ASSERT( PlatformIsCritical()); Only valid if not in callback */

  if ( INVALID_HANDLE_VALUE == pAdapter->hDevice)
    return SWS_EBADF; /* In W32CommsShutdown */

  if ( NULL == pvData)
    {
    if ( pAdapter->lRxPending)
      {
      TRACE( 4, ("W32CommsRecv() Aborting\n"));
      PurgeComm( pAdapter->hDevice, PURGE_RXABORT);
      TRACE( 4, ("W32CommsRecv() Aborted\n"));
      }
    return 0;
    }

  ASSERT( NULL != pLen);
  if ( 0 == *pLen)
    return 0;

  if ( InterlockedExchange( &pAdapter->lRxPending, 1))
    {
    /* Serious error - there is already a receive pending */
    TRACE( 1, ("*** W32CommsRecv is busy\n"));
    return -(EBUSY);
    }

  W32CommsClearErrors( pAdapter);

  VERIFY( GetCommTimeouts( pAdapter->hDevice, &timeout));
  if ( WAIT_NEVER == uTimeout)
    {
    /* No wait, just get what's immediately available */
    timeout.ReadIntervalTimeout = MAXDWORD;
    timeout.ReadTotalTimeoutMultiplier = 0;
    timeout.ReadTotalTimeoutConstant = 0;
    }
  else
    {
    /* NB, ReadFile doesn't terminate on a given delimiter (setting dcb.EofChar
     * has no effect) so if a delimiter is given then just return characters
     * as they arrive.  Setting ReadIntervalTimeout to 1 causes ReadFile to
     * terminate after the first block of chars are read from the FIFO */
    timeout.ReadIntervalTimeout = iEnd < 0 ? 0 : 1; /* millisecs */
    timeout.ReadTotalTimeoutMultiplier = 0; /* ms per byte, 0= not used */
    timeout.ReadTotalTimeoutConstant = uTimeout; /* millisecs, 0= no timeout ~*/
    }
  VERIFY( SetCommTimeouts( pAdapter->hDevice, &timeout));

  ResetEvent( pAdapter->rxOverlapped.hEvent);
  if ( !ReadFile( pAdapter->hDevice, pvData, *pLen, &dwRead, &pAdapter->rxOverlapped))
    {
    if ( ERROR_IO_PENDING == GetLastError())
      {
      TRACE( 4, ("W32CommsRecv() ReadFile pending\n"));
      return -1; /* -1 = pending */
      }
    else
      {
      TRACE( 0, ("!!! W32CommsRecv ReadFile error %d\n", GetLastError()));
      return W32CommsRecvComplete( pAdapter, SWS_EBADF, 0);
      }
    }

  /* On Win98 the overlapped event is signalled even for sync completion
   * so the 1st thread to reset the pending flag indicates op complete */
  if ( !InterlockedExchange( &pAdapter->lRxPending, 0))
    return -1; /* Pending */

  TRACE( 3, ("W32CommsRecv() read %lu\n",dwRead));
  return W32CommsRecvComplete( pAdapter, 0, dwRead);
  }


static int W32CommsRecvComplete( SW32Comms* pAdapter, int iErr, DWORD dwLen)
  {
  pAdapter->lRxPending = 0;
  ResetEvent( pAdapter->rxOverlapped.hEvent);

  if (iErr)
    W32CommsClearErrors( pAdapter);

  ASSERT( pAdapter->upcalls.pfnRecvInd);
  (*pAdapter->upcalls.pfnRecvInd)( pAdapter->upcalls.pvContext, dwLen, iErr);

  return -1;
  }


/*
 * Comms event
 */
static SWS_FASTCALL( void, W32CommsEvComplete( SW32Comms* pAdapter, int iErr))
  {
  pAdapter->lEvPending = 0;
  ResetEvent( pAdapter->evOverlapped.hEvent);

  if ( INVALID_HANDLE_VALUE == pAdapter->hDevice)
    return;

  if (iErr)
    W32CommsClearErrors( pAdapter);

  if ( pAdapter->dwEvent
    && pAdapter->upcalls.pfnStatInd
  ) {
    DWORD dwStatus;
    VERIFY( GetCommModemStatus( pAdapter->hDevice, &dwStatus));

    if ( pAdapter->dwEvent & EV_DSR)
      (*pAdapter->upcalls.pfnStatInd)(
        pAdapter->upcalls.pvContext, SWS_kP2pStatusDSR, (dwStatus & MS_DSR_ON) ? 1 : 0);

    if ( pAdapter->dwEvent & EV_RING)
      (*pAdapter->upcalls.pfnStatInd)(
        pAdapter->upcalls.pvContext, SWS_kP2pStatusRI, (dwStatus & MS_RING_ON) ? 1 : 0);

    if ( pAdapter->dwEvent & EV_RLSD)
      (*pAdapter->upcalls.pfnStatInd)(
        pAdapter->upcalls.pvContext, SWS_kP2pStatusCD, (dwStatus & MS_RLSD_ON) ? 1 : 0);

#if !W32COMMS_AUTORTS
    if ( pAdapter->dwEvent & EV_TXEMPTY)
      SetEvent( pAdapter->hTxEmptyEvent);
#endif
    }

  pAdapter->dwEvent = 0;
  pAdapter->lEvPending = 1;
  if ( WaitCommEvent( pAdapter->hDevice, &pAdapter->dwEvent, &pAdapter->evOverlapped))
    {
    pAdapter->lEvPending = 0;
    W32CommsEvComplete( pAdapter, 0);
    }
  else
    {
    DWORD dwErr = GetLastError();
    if ( ERROR_IO_PENDING != dwErr)
      {
      pAdapter->lEvPending = 0;
      TRACE( 0, ("!!! W32CommsEvComplete WaitCommEvent error %d\n", dwErr));
      W32CommsEvComplete( pAdapter, dwErr);
      }
    }
  }


/*
 * Comms event handler
 */
static unsigned __stdcall Win32CommsThread( void* pv)
  {
  SW32Comms* const pAdapter = (SW32Comms*)pv;
  HANDLE handleArray[3];

  TRACE( 9, ("Win32CommsThread(%p)\n",pv));
  ASSERT( NULL != pAdapter);
  ASSERT( W32COMMS_MAGIC == pAdapter->uMagic);

  handleArray[0] = pAdapter->txOverlapped.hEvent;
  handleArray[1] = pAdapter->rxOverlapped.hEvent;
  handleArray[2] = pAdapter->evOverlapped.hEvent;

  for ( ;;)
    {
    int iErr;
    DWORD dwLen;

    /* Wait for a Tx or Rx event */
    if ( WAIT_FAILED == WaitForMultipleObjects( lengthof( handleArray), handleArray, FALSE, INFINITE))
      break;

    /* Check if sutting down */
    if ( !pAdapter->hDevice || INVALID_HANDLE_VALUE == pAdapter->hDevice)
      break;

    /* Is Rx complete */
    switch ( WaitForSingleObject( pAdapter->rxOverlapped.hEvent, 0))
      {
    case WAIT_OBJECT_0:
      /* Reset pending flag */
      if ( !InterlockedExchange( &pAdapter->lRxPending, 0))
        break; /* Already reset by foreground */

      /* Rx op completed async */
      if ( GetOverlappedResult( pAdapter->hDevice, &pAdapter->rxOverlapped, &dwLen, TRUE))
        {
        iErr = 0;
        TRACE( 3, ("Win32CommsThread() %lu received\n",dwLen));
        }
      else
        {
        DWORD dwErr = GetLastError();
        switch ( dwErr)
          {
        case ERROR_OPERATION_ABORTED:
          iErr = 0;
          break;

        default:
          TRACE( 0, ("!!! Win32CommsThread Rx GetOverlappedResult error %d\n",dwErr));
          iErr = SWS_ETIMEDOUT;
          break;
          }

        dwLen = 0;
        }

      W32CommsRecvComplete( pAdapter, iErr, dwLen);
      break;

    case WAIT_TIMEOUT:
      break;

    default:
      return 1;
      }

    /* Is Tx complete */
    switch ( WaitForSingleObject( pAdapter->txOverlapped.hEvent, 0))
      {
    case WAIT_OBJECT_0:
      /* Reset pending flag */
      if ( !InterlockedExchange( &pAdapter->lTxPending, 0))
        break; /* Already reset by foreground */

      /* Tx completed */
      if ( GetOverlappedResult( pAdapter->hDevice, &pAdapter->txOverlapped, &dwLen, TRUE))
        {
        TRACE( 3, ("Win32CommsThread() %lu transmitted\n",dwLen));
        iErr = 0;
        }
      else
        {
        TRACE( 0, ("!!! Win32CommsThread Tx GetOverlappedResult error %d\n",
          GetLastError()));
        iErr = SWS_ETIMEDOUT;
        }

      W32CommsSendComplete( pAdapter, iErr);
      break;

    case WAIT_TIMEOUT:
      break;

    default:
      return 1;
      }

    /* Is Ev complete */
    switch ( WaitForSingleObject( pAdapter->evOverlapped.hEvent, 0))
      {
    case WAIT_OBJECT_0:
      /* Reset pending flag */
      if ( !InterlockedExchange( &pAdapter->lEvPending, 0))
        break; /* Already reset by foreground */

      /* Ev op completed async */
      if ( GetOverlappedResult( pAdapter->hDevice, &pAdapter->evOverlapped, &dwLen, TRUE))
        {
        iErr = 0;
        }
      else
        {
        TRACE( 0, ("!!! Win32CommsThread Ev GetOverlappedResult error %d\n",
          GetLastError()));
        iErr = SWS_ETIMEDOUT;
        }

      W32CommsEvComplete( pAdapter, iErr);
      break;

    case WAIT_TIMEOUT:
      break;

    default:
      return 1;
      }
    }

  return 0;
  }


/*
 * Handle private ioctls
 */
SWS_CB( int, W32CommsIoctl(           /* Return !0 on error */
  SWS_PTR( void) pvContext,           /* IN: Driver's context */
  unsigned uType,                     /* IN: Type */
  SWS_PTR( void) pvData,              /* I/O: -> data */
  SWS_PTR( size_t) pSize              /* I/O: data size */
)){
  SW32Comms* const pAdapter = (SW32Comms*)pvContext;
  DCB dcb;

  TRACE( 9, ("W32CommsIoctl(%p,%u,%p,%p)\n",pvContext,uType,pvData,pSize));
  ASSERT( NULL != pAdapter);
  ASSERT( W32COMMS_MAGIC == pAdapter->uMagic);

  switch ( uType)
    {
  default:
    TRACE( 3, ("W32CommsIoctl %u not implemented\n", uType));
    return SWS_EBADF;

  case SWS_kP2pIoctl_SetOnLine:
    /* En/dis/able ready/online state */
    if ( *pSize != sizeof(ui32))
      return SWS_EINVAL;

    return W32CommsOnline( pAdapter, *((SWS_PTR( const ui32)) pvData) ? 1 : 0);

  case SWS_kP2pIoctl_GetBaud:
    if ( *pSize != sizeof(ui32))
      return SWS_EINVAL;

    dcb.DCBlength = sizeof( dcb);
    if ( !GetCommState( pAdapter->hDevice, &dcb) )
      return SWS_EBADF;

    *(SWS_PTR( ui32))pvData = dcb.BaudRate;
    return 0;

  case SWS_kP2pIoctl_GetModemStatus:
    if ( *pSize != sizeof(ui8))
      return SWS_EINVAL;

    *(SWS_PTR( ui8))pvData = (ui8)W32CommsStatus( pAdapter);
    return 0;
    }
  }


/*
 * En/dis/able online state
 */
SWS_FASTCALL( int, W32CommsOnline( SW32Comms* pAdapter, int iOnLine))
  {
  TRACE( 9, ("W32CommsOnline(%p,%d)\n",pAdapter,iOnLine));
  ASSERT( NULL != pAdapter);
  ASSERT( W32COMMS_MAGIC == pAdapter->uMagic);

  if ( !EscapeCommFunction( pAdapter->hDevice, iOnLine ? SETDTR : CLRDTR))
    {
    TRACE( 1, ("***W32CommsOnline() EscapeCommFunction failed error %d\n",
      GetLastError() ));
    return SWS_EINVAL;
    }

  return 0;
  }


/*
 * Get modem status
 */
SWS_FASTCALL( unsigned, W32CommsStatus( SW32Comms* pAdapter))
  {
  DWORD dwStatus;
  unsigned uStatus;

  TRACE( 9, ("W32CommsStatus(%p)\n",pAdapter));
  ASSERT( NULL != pAdapter);
  ASSERT( W32COMMS_MAGIC == pAdapter->uMagic);

  VERIFY( GetCommModemStatus( pAdapter->hDevice, &dwStatus));

  uStatus = 0;
  if ( dwStatus & MS_RING_ON)
    uStatus |= SWS_P2P_MODEM_RNG;

  if ( dwStatus & MS_DSR_ON)
    uStatus |= SWS_P2P_MODEM_DSR;

  if ( dwStatus & MS_RLSD_ON)
    uStatus |= SWS_P2P_MODEM_DCD;

  return uStatus;
  }

#endif /*OPT_DATALINK_P2P*/
/* End of file */
