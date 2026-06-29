/****************************************************************************
 *
 * File name   : psxcomms.c
 * Function    : Posix serial comms driver
 * Project     : SWSSOCK
 * Authors     : Lawrence Rust
 * Systems     : ANSI C.
 *
 ****************************************************************************
 *
 * Created by Lawrence Rust, Software Systems Consultants, Cambridge, UK.
 * lvr@softsystem.co.uk. Tel/Fax +44 1223 862391
 *
 * 20-Aug-09, LVR, Created.
 *
 * Comments:
 * --------
 * This file implements the Posix serial comms driver
 *
 ****************************************************************************
 */

/* Compilation options */
#ifndef _BSD_SOURCE
#define _BSD_SOURCE     /* For CRTSCTS in termios.h */
#endif
#ifndef _GNU_SOURCE
#define _GNU_SOURCE     /* For F_SETSIG */
#endif

#include "config.h"   /* Build options */

#ifndef OPT_SERIAL_ADAPTERS
#define OPT_SERIAL_ADAPTERS 4 /* Max no. serial,ports */
#endif

/* Exports */
#include "psxcomms.h"

#if OPT_DATALINK_P2P
/* Imports */
#include <stddef.h>
#include <string.h>   /* memcpy, strcmp */
#include <errno.h>
#include <signal.h>

/* POSIX */
#include <unistd.h>     /* open, close, usleep */
#include <fcntl.h>      /* O_RDWR, O_NONBLOCK, O_NOCTTY */
#include <termios.h>    /* tcgetattr etc */
#include <sys/ioctl.h>  /* ioctl */

/* Swssock */
#include "sws_sock.h" /* SWS_ENOBUFS */
#include "sws_buf.h"  /* Buffer operations */
#include "p2p.h"      /* P2P driver registration */
#include "platform.h"

#include "debug.h"    /* TRACE, ASSERT & VERIFY */


/*
 * Macros & Constants
 */

/* Configurable vales */
#define PKEY_SIGNAL "signal"  /* Numeric */
#ifndef PSXCOMMS_SIGNAL
#define PSXCOMMS_SIGNAL SIGIO
#endif

#define PKEY_PORT   "port"    /* ASCSIIZ device name */

#define PKEY_BAUD   "baud"    /* Numeric */
#ifndef PSXCOMMS_BAUD
#define PSXCOMMS_BAUD 38400
#endif

#define PKEY_BITS   "bits"    /* Numeric */
#ifndef PSXCOMMS_BITS
#define PSXCOMMS_BITS 8
#endif

#define PKEY_PARITY "parity"  /* ASCIIZ odd, even, none, mark, space */
#ifndef PSXCOMMS_PARITY
#define PSXCOMMS_PARITY "none"
#endif

#define PKEY_STOP   "stop"    /* Numeric */
#ifndef PSXCOMMS_STOP
#define PSXCOMMS_STOP 1
#endif

#define PKEY_ERROR  "error"   /* ASCIIZ discard, nul, ignore */
#ifndef PSXCOMMS_ERROR
#define PSXCOMMS_ERROR "discard"
#endif

#define PKEY_RTSCTS "rtscts"  /* Numeric, boolean */
#ifndef PSXCOMMS_RTSCTS
#define PSXCOMMS_RTSCTS 0
#endif

#define PKEY_DTR "dtr"        /* Numeric, boolean */
#ifndef PSXCOMMS_DTR
#define PSXCOMMS_DTR 1
#endif

#define PKEY_RTS "rts"        /* Numeric, boolean */
#ifndef PSXCOMMS_RTS
#define PSXCOMMS_RTS 0
#endif


/*
 * Local data types
 */
/* Port instance */
typedef struct SPosixComms
  {
  ui32 uMagic;
#define PSXCOMMS_MAGIC 'pscm'

  struct SPosixComms* pNext;            /* List of drivers */

  /* Configuration */
  ui32 uBaud;

  /* Operation */
  int fd;                             /* Handle to comms device */
  struct termios tio;                 /* Current port settings */
  SWS_P2pCallbacks upcalls;           /* Up call info */

  Atomic_t aTx;
  SWS_PTR( const void) pvTx;          /* -> Tx buffer */
  size_t sizeTxWritten;               /* Tx buffer written */

  Atomic_t aRx;
  SWS_PTR( void) pvRx;                /* -> Rx buffer */
  int iEnd;
  } SPosixComms;


/*
 * Prototypes
 */
static int AddSerialPort( int);
static void FreeSerialPort( SPosixComms*);
static int CommsWrite( SPosixComms*, const void* pvData, size_t len);
static int CommsRecv( SPosixComms*, void* pvData, size_t len, int iEnd);
static void CommsSignal( int);
#ifdef TIOCMGET
static int CommsOnline( SPosixComms*, int iOnLine);
static unsigned CommsStatus( SPosixComms* pAdapter);
#endif

static SWS_FP2pSend PosixCommsSend;
static SWS_FP2pRecv PosixCommsRecv;
static SWS_FP2pShutdown PosixCommsShutdown;
static SWS_FP2pIoctl PosixCommsIoctl;


/*
 * Module data
 */
/* Configuration */
static int s_iSignal;

/* Operation */
static SPosixComms* s_pAdapters; /* List of interfaces */
#ifdef __CYGWIN__
typedef void (*sighandler_t)( int);
#endif
static sighandler_t s_pfnSignal;

/* P2P device descriptor */
static char s_szIdent[] = SWS_PSXCOMMS_IDENT "01";
static const SWS_P2pDevice s_PosixComms =
  {
  sizeof( SWS_P2pDevice),
  SWS_P2P_VERSION,
  s_szIdent,
  &PosixCommsSend,
  &PosixCommsRecv,
  &PosixCommsShutdown,
  &PosixCommsIoctl
  };


/*
 * Initialise the driver
 */
int SWS_PosixCommsInstall( void)           /* Returns !0 on error */
  {
  int i, iAdapters;

  TRACE( 9, ("SWS_PosixCommsInstall()\n"));
  ASSERT( PlatformIsCritical());

  /* Configuration */
#ifdef F_SETSIG
  s_iSignal = PlatformPropertyGetUint(
    SWS_PSXCOMMS_IDENT, PKEY_SIGNAL, PSXCOMMS_SIGNAL );
#else
  s_iSignal = SIGIO;
#endif

  /* Install async i/o signal handler */
  s_pfnSignal =  signal( s_iSignal, &CommsSignal);
  if ( SIG_ERR == s_pfnSignal)
    {
    TRACE( 1, ("*** SWS_PosixCommsInstall() signal(%d) failed: %s\n",
      s_iSignal, strerror( errno)));
    return SWS_ENETDOWN;
    }

  /* Search for serial ports */
  iAdapters = 0;
  for ( i = 0; i < OPT_SERIAL_ADAPTERS && iAdapters < OPT_DATALINK_P2P; ++i)
    {
    if ( !AddSerialPort( i))
      ++iAdapters;
    }

  if ( 0 == iAdapters)
    {
    TRACE( 1, ("*** SWS_PosixCommsInstall() no adapters found\n"));
    signal( s_iSignal, s_pfnSignal);
    return SWS_ENETDOWN;
    }
  return 0;
  }


/*
 * Finalise the driver
 * Single threaded
 */
void SWS_PosixCommsCleanup( void)
  {
  SPosixComms* pAdapter;

  TRACE( 9, ("SWS_PosixCommsCleanup()\n"));
  ASSERT( PlatformIsCritical());

  /* All the drivers should have been shutdown by now */
  while ( NULL != (pAdapter = s_pAdapters))
    {
    s_pAdapters = pAdapter->pNext;
    FreeSerialPort( pAdapter);
    }
  signal( s_iSignal, s_pfnSignal);
  }


/*
 * Convert baud rate to enumerated type
 */
struct BaudRates { unsigned long ulBaud; speed_t speed; };
static const struct BaudRates BaudRateTab[] = {
  {     50UL, B50     },
  {     75UL, B75     },
  {    110UL, B110    },
  {    150UL, B150    },
  {    300UL, B300    },
  {    600UL, B600    },
  {   1200UL, B1200   },
  {   2400UL, B2400   },
  {   4800UL, B4800   },
  {   9600UL, B9600   },
  {  19200UL, B19200  },
  {  38400UL, B38400  }
#ifdef B57600
 ,{  57600UL, B57600  }
#endif
#ifdef B115200
 ,{ 115200UL, B115200 }
#endif
#ifdef B230400
 ,{ 230400UL, B230400 }
#endif
#ifdef B460800
 ,{ 460800UL, B460800 }
#endif
#ifdef B500000
 ,{ 500000UL, B500000 }
#endif
#ifdef B576000
 ,{ 576000UL, B576000 }
#endif
#ifdef B1000000
 ,{1000000UL, B1000000 }
#endif
#ifdef B1152000
 ,{1152000UL, B1152000 }
#endif
#ifdef B1500000
 ,{1500000UL, B1500000 }
#endif
#ifdef B2000000
 ,{2000000UL, B2000000 }
#endif
#ifdef B2500000
 ,{2500000UL, B2500000 }
#endif
#ifdef B3000000
 ,{3000000UL, B3000000 }
#endif
#ifdef B3500000
 ,{3500000UL, B3500000 }
#endif
#ifdef B4000000
 ,{4000000UL, B4000000 }
#endif
  };

static speed_t Baud2Speed( unsigned long ulBaud)
  {
  unsigned i;
  for ( i = 0; i < lengthof( BaudRateTab); i++)
    {
    unsigned long b = BaudRateTab[i].ulBaud;
    unsigned long d = b > ulBaud ? b - ulBaud : ulBaud - b;
    unsigned long err;
# define MAX_BERR 666 /* 6.66 % */
    if ( d < ulBaud
      && d < b
      && (err = ((d * 10000) / ulBaud)) <= MAX_BERR /* 100=1%, 5% => 0.5 bit in 10 */
    ) {
      return BaudRateTab[i].speed;
      }
    if ( b > ulBaud)
      break;
    }
  return B0; /* No matching standard rate */
  }


/*
 * Create an SwsSock interface for a serial port
 */
int AddSerialPort( int iPort)
  {
  SPosixComms* pAdapter;
  int iRet;
  char szBuf[64];

  TRACE( 9, ("AddSerialPort(%d)\n",iPort));

  sprintf( s_szIdent, SWS_PSXCOMMS_IDENT "%d", 1 + iPort);

  /* Get comms port name */
  iRet = PlatformPropertyGetString( s_szIdent, PKEY_PORT, "",  szBuf, sizeof szBuf);
  if ( 0 == iRet)
    {
    TRACE( 2, ("AddSerialPort() %s disabled\n", s_szIdent));
    return SWS_ENODEV;
    }
  TRACE( 2, ("AddSerialPort() %s = %s\n", s_szIdent, szBuf));

  /* Allocate instance data */
  pAdapter = (SPosixComms*) calloc( 1, sizeof(SPosixComms));
  if ( NULL == pAdapter)
    {
    TRACE( 0, ("!!! AddSerialPort(%d) Out of memory for instance\n", iPort));
    return SWS_ENOMEM;
    }

  /* Open the device */
  pAdapter->fd = open( szBuf, O_RDWR | O_NONBLOCK | O_NOCTTY);
  if ( -1 == pAdapter->fd)
    {
    int err = errno;
    TRACE( 2, ("AddSerialPort() open '%s' failed: %s\n", szBuf, strerror( errno) ));
    free( pAdapter);
    return ENODEV == err ? SWS_ENODEV : SWS_EACCES;
    }

  pAdapter->uMagic = PSXCOMMS_MAGIC;

  /* Set comms parameters */
  for ( ;;)
    {
    speed_t speed;
    ui32 bits, rtscts;
    tcflag_t cflag, iflag, mask;

    /* Get default port settings */
    if ( tcgetattr( pAdapter->fd, &pAdapter->tio) < 0)
      {
      TRACE( 0, ("!!! AddSerialPort(%d) tcgetattr failed: %s\n", iPort, strerror( errno) ));
      iRet = SWS_EBADF;
      break;
      }

    /* Baud rate selection */
    pAdapter->uBaud = PlatformPropertyGetUint( s_szIdent, PKEY_BAUD, PSXCOMMS_BAUD );
    if ( B0 == (speed = Baud2Speed( pAdapter->uBaud)))
      {
      TRACE( 0, ("!!! AddSerialPort() unsupported baud: %lu\n", (unsigned long)pAdapter->uBaud));
      iRet = SWS_EINVAL;
      break;
      }

    /* Initial port settings */
    pAdapter->tio.c_iflag = IGNBRK | IGNPAR;
    pAdapter->tio.c_oflag = 0;
    pAdapter->tio.c_cflag |= CREAD | HUPCL | CLOCAL;
    pAdapter->tio.c_lflag = 0;
    /* On reads, block until at least one char available */
    pAdapter->tio.c_cc[VMIN] = 1;
    pAdapter->tio.c_cc[VTIME] = 0;
    pAdapter->tio.c_cc[VSTART] = 17; /* Ctrl-Q */
    pAdapter->tio.c_cc[VSTOP] = 19; /* Ctrl-S */

    iflag = pAdapter->tio.c_iflag;
    cflag = pAdapter->tio.c_cflag;

    /* Data bits selection */
    mask = ~CSIZE;
    bits = PlatformPropertyGetUint( s_szIdent, PKEY_BITS, PSXCOMMS_BITS );
    switch ( bits)
      {
    case 8:
      cflag &= mask;
      cflag |= CS8;
      break;
    case 7:
      cflag &= mask;
      cflag |= CS7;
      break;
    case 6:
      cflag &= mask;
      cflag |= CS6;
      break;
    case 5:
      cflag &= mask;
      cflag |= CS5;
      break;
    default:
      TRACE( 0, ("!!! AddSerialPort() Invalid data bits %u\n", bits ));
      break;
      }

    /* Parity selection */
#ifdef CMSPAR
    mask = ~(PARENB | PARODD | CMSPAR);
#else
    mask = ~(PARENB | PARODD);
#endif
    PlatformPropertyGetString( s_szIdent, PKEY_PARITY, PSXCOMMS_PARITY, szBuf, sizeof szBuf);
    if ( !strcasecmp( szBuf, "none") )
      {
      cflag &= mask;
      }
    else if ( !strcasecmp( szBuf, "odd") )
      {
      cflag &= mask;
      cflag |= (PARENB | PARODD);
      }
    else if ( !strcasecmp( szBuf, "even") )
      {
      cflag &= mask;
      cflag |= PARENB;
      }
#ifdef CMSPAR
    else if ( !strcasecmp( szBuf, "mark") )
      {
      cflag &= mask;
      cflag |= (PARENB | PARODD | CMSPAR);
      }
    else if ( !strcasecmp( szBuf, "space") )
      {
      cflag &= mask;
      cflag |= (PARENB | CMSPAR);
      }
#endif
    else
      TRACE( 0, ("!!! AddSerialPort() Invalid parity '%s'\n", szBuf ));

    /* Stop bits selection */
    bits = PlatformPropertyGetUint( s_szIdent, PKEY_STOP, PSXCOMMS_STOP );
    switch ( bits)
      {
    case 2:
      cflag |= CSTOPB;
      break;
    case 1:
      cflag &= ~CSTOPB;
      break;
    default:
      TRACE( 0, ("!!! AddSerialPort() Invalid stop bits %u\n", bits ));
      break;
      }

    /* Error handling */
    mask = ~(INPCK | IGNPAR);
    PlatformPropertyGetString( s_szIdent, PKEY_ERROR, PSXCOMMS_ERROR, szBuf, sizeof szBuf);
    if ( !strcasecmp( szBuf, "discard") )
      {
      iflag &= mask;
      iflag |= IGNPAR | INPCK;
      }
    else if ( !strcasecmp( szBuf, "nul") )
      {
      iflag &= mask;
      iflag |= INPCK;
      }
    else if ( !strcasecmp( szBuf, "ignore") )
      {
      iflag &= mask;
      }
    else
      TRACE( 0, ("!!! AddSerialPort() Invalid error mode '%s'\n", szBuf ));

#ifdef CRTSXOFF
    cflag &= ~CRTSXOFF;
#endif
    rtscts = PlatformPropertyGetUint( s_szIdent, PKEY_RTSCTS, PSXCOMMS_RTSCTS );
    if ( rtscts)
#ifdef CRTSCTS
      cflag |= CRTSCTS; /* RTS/CTS */
    else
      cflag &= ~CRTSCTS;
#else
      TRACE( 0, ("!!! AddSerialPort() RTS/CTS not supported\n"));
#endif

    /* Commit changes */
    pAdapter->tio.c_cflag = cflag;
    pAdapter->tio.c_iflag = iflag;
    if ( cfsetispeed( &pAdapter->tio, speed) < 0)
      {
      TRACE( 0, ("!!! AddSerialPort() cfsetispeed failed: %s\n", strerror( errno) ));
      iRet = SWS_EINVAL;
      break;
      }
    if ( cfsetospeed( &pAdapter->tio, speed) < 0)
      {
      TRACE( 0, ("!!! AddSerialPort() cfsetospeed failed: %s\n", strerror( errno) ));
      iRet = SWS_EINVAL;
      break;
      }

    /* Set the attributes */
    if ( (iRet = tcsetattr( pAdapter->fd, TCSANOW, &pAdapter->tio)) < 0)
      {
      TRACE( 0, ("!!! AddSerialPort() tcsetattr failed: %s\n", strerror( errno) ));
      iRet = SWS_EINVAL;
      break;
      }
    /* Verify baud set OK */
    else if ( (iRet = tcgetattr( pAdapter->fd, &pAdapter->tio)) < 0)
      {
      TRACE( 0, ("!!! AddSerialPort() tcgetattr failed: %s\n", strerror( errno) ));
      iRet = SWS_EINVAL;
      break;
      }
    else if ( cfgetispeed( &pAdapter->tio) != speed
      || cfgetospeed( &pAdapter->tio) != speed
    ) {
      TRACE( 0, ("!!! AddSerialPort() cfget(io)speed failed: %s\n", strerror( errno) ));
      iRet = SWS_EINVAL;
      break;
      }

#ifdef TIOCMBIS
    if ( PlatformPropertyGetUint( s_szIdent, PKEY_DTR, PSXCOMMS_DTR))
      mask = 1;
    else
      mask = 0;

    bits = TIOCM_DTR;
    if ( ioctl( pAdapter->fd, mask ? TIOCMBIS : TIOCMBIC, &bits) < 0)
      {
      iRet = errno;
      TRACE( 0, ("!!! AddSerialPort() ioctl(TIOCM_DTR) failed: %s\n", strerror( errno)));
      break;
      }

    if ( !rtscts)
      {
      if ( PlatformPropertyGetUint( s_szIdent, PKEY_RTS, PSXCOMMS_RTS))
        mask = 1;
      else
        mask = 0;

      bits = TIOCM_RTS;
      if ( ioctl( pAdapter->fd, mask ? TIOCMBIS : TIOCMBIC, &bits) < 0)
        {
        iRet = errno;
        TRACE( 0, ("!!! AddSerialPort() ioctl(TIOCM_RTS) failed: %s\n", strerror( errno)));
        break;
        }
      }
#endif

    /* Request signal on tx/rx events */
#ifdef O_ASYNC
    if ( -1 == fcntl( pAdapter->fd, F_SETFL, O_ASYNC | fcntl( pAdapter->fd, F_GETFL)) )
      TRACE( 1, ("*** AddSerialPort() fcntl F_SETFL, O_ASYNC failed: %s\n", strerror( errno)));
#elif defined FIOASYNC
    if ( iRet = 1, -1 == ioctl( pAdapter->fd, FIOASYNC, &iRet) )
      TRACE( 1, ("*** AddSerialPort() ioctl FIOASYNC failed: %s\n", strerror( errno)));
#else
    {int WARN_AsyncIORequired;}
    TRACE( 0, ("!!! AddSerialPort() AsyncIO required\n"));
#endif
#ifdef F_SETSIG
    if ( SIGIO != s_iSignal && -1 == fcntl( pAdapter->fd, F_SETSIG, s_iSignal))
      TRACE( 1, ("*** AddSerialPort() fcntl F_SETSIG %d failed: %s\n", s_iSignal, strerror( errno)));
#endif
#ifdef F_SETOWN
    if ( -1 == fcntl( pAdapter->fd, F_SETOWN, getpid()))
      TRACE( 1, ("*** AddSerialPort() fcntl F_SETOWN failed: %s\n", strerror( errno)));
#endif

    /* Add to list of drivers */
    pAdapter->pNext = s_pAdapters;
    s_pAdapters = pAdapter;

    /* Register a P2P adapter */
    iRet = SWS_P2pAddAdapter( &s_PosixComms, pAdapter, &pAdapter->upcalls);
    if ( iRet)
      {
      s_pAdapters = pAdapter->pNext;
      break;
      }

    ASSERT( pAdapter->upcalls.pfnSendAck);
    ASSERT( pAdapter->upcalls.pfnRecvInd);
    ASSERT( pAdapter->upcalls.pfnStatInd);

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
static SWS_CB( void, PosixCommsShutdown(
  SWS_PTR( void) pvContext            /* IN: Client context */
)){
  SPosixComms* pAdapter, *pPrev;;

  TRACE( 9, ("PosixCommsShutdown(%p)\n", pvContext));
  ASSERT( NULL != pvContext);

  /* Search for the driver */
  pPrev = NULL;
  for ( pAdapter = s_pAdapters; NULL != pAdapter; pAdapter = pAdapter->pNext)
    {
    if ( (SPosixComms*)pvContext == pAdapter)
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

  TRACE( 1, ("*** PosixCommsShutdown() Client %p not found)\n", pvContext));
  }


/*
 * Release serial driver resources
 */
void FreeSerialPort( SPosixComms* pAdapter)
  {
  int fd;

  TRACE( 9, ("FreeSerialPort(%p)\n",pAdapter));

  /* Release resources */
  fd = pAdapter->fd;
  if ( -1 != fd)
    {
    pAdapter->fd = -1;

    /* Close the device */
    close( fd);
    }

  free( pAdapter);
  }


/*
 * Transmit a buffer
 * Single threaded
 * May be re-entered from SWS_FP2pSendAck upcall
 */
SWS_CB( int, PosixCommsSend(          /* Return !0 on error, -1 if pending */
  SWS_PTR( void) pvContext,           /* IN: Driver's context */
  SWS_PTR( const void) pvData,        /* IN: Data to send */
  size_t len                          /* IN: Byte count */
)){
  SPosixComms* const pAdapter = (SPosixComms*)pvContext;
  int n;

  TRACE( 9, ("PosixCommsSend(%p,%p,%u)\n",pvContext,pvData,len));
  ASSERT( NULL != pAdapter);
  ASSERT( PSXCOMMS_MAGIC == pAdapter->uMagic);
  ASSERT( NULL != pvData);
  ASSERT( len > 0);
  /*ASSERT( PlatformIsCritical()); Only valid if not in callback */

  if ( -1 == pAdapter->fd)
    return SWS_EBADF; /* In PosixCommsShutdown */

  if ( 0 == len)
    return 0;

  n = CommsWrite( pAdapter, pvData, len);
  if ( -1 == n)
    return SWS_EBADF;
  else if ( (size_t)n == len)
    return 0;

  pAdapter->pvTx = pvData;
  pAdapter->sizeTxWritten = (size_t)n;
  len -= (size_t)n;
  if ( AtomicXchg( &pAdapter->aTx, len))
    {
    /* Serious error - there is already a buffer queued */
    TRACE( 1, ("*** PosixCommsSend is busy\n"));
    return -(EBUSY);
    }

  return -1;
  }

int CommsWrite( SPosixComms* pAdapter, const void* pvData, size_t len)
  {
  int n;

  TRACE( 9, ("CommsWrite(%p,%p,%u)\n", pAdapter, pvData, (unsigned)len));

  n = write( pAdapter->fd, pvData, len);
  if ( -1 == n)
    {
    if ( EAGAIN == errno)
      return 0;

    TRACE( 1, ("*** CommsWrite write failed: %s\n", strerror( errno)));
    }
  return n;
  }


/*
 * Start receiving.
 * Single threaded
 * May be re-entered from SWS_FP2pRecvInd upcall
 */
SWS_CB( int, PosixCommsRecv(          /* Return !0 on error, -1 if pending */
  SWS_PTR( void) pvContext,           /* IN: Driver's context */
  SWS_PTR( void) pvData,              /* IN: Buffer */
  SWS_PTR( size_t) pLen,              /* I/O: Buffer size */
  int iEnd,                           /* IN: Delimiter, -ve if none */
  ui32 uTimeout                       /* IN: Timeout, 0= none, 1= immediate */
)){
  SPosixComms* const pAdapter = (SPosixComms*)pvContext;
  size_t len;
  int n;

  TRACE( 9, ("PosixCommsRecv(%p,%p,%p,%d,%lu)\n",pvContext,pvData,pLen,iEnd,uTimeout));
  ASSERT( NULL != pAdapter);
  ASSERT( PSXCOMMS_MAGIC == pAdapter->uMagic);
  /*ASSERT( PlatformIsCritical()); Only valid if not in callback */

  if ( -1 == pAdapter->fd)
    return SWS_EBADF; /* In PosixCommsShutdown */

  if ( NULL == pvData)
    {
    AtomicWrite( &pAdapter->aRx, 0);
    pAdapter->pvRx = NULL;
    return 0;
    }

  ASSERT( NULL != pLen);
  if ( 0 == (len = *pLen))
    return 0;

  n = CommsRecv( pAdapter, pvData, len, iEnd);
  switch ( n)
    {
  case -1:
    return SWS_EBADF;
  case 0:
    break;
  default:
    *pLen = (size_t)n;
    return 0;
    }

  switch ( uTimeout)
    {
  case WAIT_NEVER:
    *pLen = 0;
    return 0;
  case WAIT_FOREVER:
    break;
  default:
    return SWS_EINVAL; /* NB timeout not implemented */
    }

  pAdapter->iEnd = iEnd;
  pAdapter->pvRx = pvData;
  if ( AtomicXchg( &pAdapter->aRx, len))
    {
    /* Serious error - there is already a buffer queued */
    TRACE( 1, ("*** PosixCommsRecv is busy\n"));
    return -(EBUSY);
    }

  return -1;
  }

int CommsRecv( SPosixComms* pAdapter, void* pvData, size_t len, int iEnd)
  {
  int recvd = 0;

  TRACE( 9, ("CommsRecv(%p,%p,%u,%d)\n", pAdapter, pvData, (unsigned)len, iEnd));

  do
    {
    int n = read( pAdapter->fd, pvData, iEnd < 0 ? len : 1);
    if ( -1 == n)
      {
      if ( EAGAIN != errno)
        {
        TRACE( 1, ("*** CommsRecv read failed: %s\n", strerror( errno)));
        recvd = -1;
        }
      break;
      }
    else if ( 0 == n)
      break;

    recvd += n;
    if ( iEnd < 0)
      break;
    if ( (unsigned)iEnd == *(unsigned char*)pvData)
      break;
    pvData = (char*)pvData + n;
    len -= (size_t)n;
    }
  while ( len);

  if ( recvd > 0)
    TRACE( 4, ("CommsRecv(%p,,%u,%d) read %d\n", pAdapter, len, iEnd, recvd));

  return recvd;
  }


/*
 * IO ready signal handler
 */
void CommsSignal( int iSig)
  {
  int isBusy;

  TRACE( 10, ("CommsSignal\n"));

  do
    {
    SPosixComms* pAdapter;

    isBusy = 0;
    for ( pAdapter = s_pAdapters; NULL != pAdapter; pAdapter = pAdapter->pNext)
      {
      size_t len;
      int n, err;

      if ( (len = (size_t) AtomicRead( &pAdapter->aTx))
        && (n = CommsWrite( pAdapter,
          (const char*)pAdapter->pvTx + pAdapter->sizeTxWritten, len))
      ) {
        if ( -1 != n)
          {
          pAdapter->sizeTxWritten += (size_t)n;
          len = AtomicAdd( &pAdapter->aTx, -n);
          err = 0;
          }
        else
          {
          len = 0;
          AtomicWrite( &pAdapter->aTx, 0);
          err = SWS_EBADF;
          }

        if ( 0 == len)
          {
          /* Indicate transmit complete */
          ASSERT( pAdapter->upcalls.pfnSendAck);
          (*pAdapter->upcalls.pfnSendAck)( pAdapter->upcalls.pvContext, err);
          }
        isBusy = 1;
        }

      if ( (len = (size_t) AtomicRead( &pAdapter->aRx))
        && (n = CommsRecv( pAdapter, pAdapter->pvRx, len, pAdapter->iEnd))
      ) {
        if ( -1 != n)
          len = (size_t)n, err = 0;
        else
          len = 0, err = SWS_EBADF;

        AtomicWrite( &pAdapter->aRx, 0);

        /* Indicate receive data/error */
        ASSERT( pAdapter->upcalls.pfnRecvInd);
        (*pAdapter->upcalls.pfnRecvInd)( pAdapter->upcalls.pvContext, len, err);
        isBusy = 1;
        }
      }
    }
  while ( isBusy);

  if ( SIG_DFL != s_pfnSignal)
    (*s_pfnSignal)( iSig);
  signal( iSig, &CommsSignal);
  }


/*
 * Handle private ioctls
 */
SWS_CB( int, PosixCommsIoctl(           /* Return !0 on error */
  SWS_PTR( void) pvContext,           /* IN: Driver's context */
  unsigned uType,                     /* IN: Type */
  SWS_PTR( void) pvData,              /* I/O: -> data */
  SWS_PTR( size_t) pSize              /* I/O: data size */
)){
  SPosixComms* const pAdapter = (SPosixComms*)pvContext;

  TRACE( 9, ("PosixCommsIoctl(%p,%u,%p,%p)\n",pvContext,uType,pvData,pSize));
  ASSERT( NULL != pAdapter);
  ASSERT( PSXCOMMS_MAGIC == pAdapter->uMagic);

  switch ( uType)
    {
  default:
    TRACE( 3, ("PosixCommsIoctl %u not implemented\n", uType));
    return SWS_EBADF;

  case SWS_kP2pIoctl_GetBaud:
    if ( *pSize != sizeof(ui32))
      return SWS_EINVAL;

    *(SWS_PTR( ui32))pvData = pAdapter->uBaud;
    return 0;

#ifdef TIOCMGET
  case SWS_kP2pIoctl_SetOnLine:
    /* En/dis/able ready/online state */
    if ( *pSize != sizeof(ui32))
      return SWS_EINVAL;

    return CommsOnline( pAdapter, *((SWS_PTR( const ui32)) pvData) ? 1 : 0);

  case SWS_kP2pIoctl_GetModemStatus:
    if ( *pSize != sizeof(ui8))
      return SWS_EINVAL;

    *(SWS_PTR( ui8))pvData = (ui8)CommsStatus( pAdapter);
    return 0;
#endif
    }
  }


#ifdef TIOCMGET
/*
 * En/dis/able online state
 */
int CommsOnline( SPosixComms* pAdapter, int iOnLine)
  {
  int bits = TIOCM_DTR;

  TRACE( 9, ("CommsOnline(%p,%d)\n",pAdapter,iOnLine));
  ASSERT( NULL != pAdapter);
  ASSERT( PSXCOMMS_MAGIC == pAdapter->uMagic);

  if ( ioctl( pAdapter->fd, iOnLine ? TIOCMBIS : TIOCMBIC, &bits) < 0)
    {
    TRACE( 1, ("*** CommsOnline() ioctl(TIOCM_DTR) failed: %s\n", strerror( errno)));
    return SWS_EINVAL;
    }
  return 0;
  }


/*
 * Get modem status
 */
unsigned CommsStatus( SPosixComms* pAdapter)
  {
  int bits;
  unsigned status = 0;

  TRACE( 9, ("CommsStatus(%p)\n",pAdapter));
  ASSERT( NULL != pAdapter);
  ASSERT( PSXCOMMS_MAGIC == pAdapter->uMagic);

  /* Get modem status */
  if ( ioctl( pAdapter->fd, TIOCMGET, &bits) < 0)
    {
    TRACE( 1, ("*** CommsStatus() ioctl(TIOCMGET) failed: %s\n", strerror( errno)));
    return 0;
    }

  if ( TIOCM_CD & bits)
    status |= SWS_P2P_MODEM_DCD;
  if ( TIOCM_RI & bits)
    status |= SWS_P2P_MODEM_RNG;
  if ( TIOCM_DSR & bits)
    status |= SWS_P2P_MODEM_DSR;

  return status;
  }
#endif

#endif /*OPT_DATALINK_P2P*/
/* End of file */
