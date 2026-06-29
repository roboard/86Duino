/****************************************************************************
 *
 * File name   : packet.c
 * Function    : Packet driver
 * Project     : SwsSock
 * Authors     : Lawrence Rust
 * Systems     : ANSI C.
 * Version     : See sws_sock.h for overall library version information.
 * Release Date: See sws_sock.h for overall library release information.
 *
 ****************************************************************************
 *
 * Created by Lawrence Rust, Software Systems Consultants
 * lvr@softsystem.co.uk. Tel/Fax +33 5 49 72 79 63
 *
 * 27-Apr-01, LVR, Created.
 * 28-Oct-02, LVR, Add PKEY_RECV_MODE & PKEY_MAC_ADDRESS.
 * 01-Nov-02, LVR, PacketUpcall calls PacketRecvAck on error.
 * 06-Nov-02, LVR, Added PacketIoctl:SWS_kDatalinkProperty_Promiscuity.
 * 22-Dec-02, LVR, Added PacketSetMulticastList & SWS_kDatalinkProperty_MulticastList
 * 12-Sep-05, LVR, Dispose of RxQ and PacketUpcall.
 *
 * Comments:
 * --------
 * This file interfaces a packet driver to the SwsSock stack
 *
 ****************************************************************************
 */

/* Compilation options */
#include "config.h"

#ifndef OPT_PKT_BUFFER_SIZE
#define OPT_PKT_BUFFER_SIZE (60*1024U) /* Receive buffer size */
#endif


/* Exports */
#include "packet.h"

#if OPT_DATALINK_ETHERNET
/* Imports */
#include <stddef.h>
#include <string.h>   /* memcpy */

/* DOS */
#include <dos.h>

/* SwsSock */
#include "sws_enet.h" /* Ethernet driver registration */
#include "sws_dlnk.h" /* SWS_kDatalinkProperty_Promiscuity */
#include "sws_buf.h"  /* Buffer operations */
#include "sws_msgq.h" /* Message queues */
#include "sws_sock.h"  /* Error codes */
#include "platform.h"  /* PlatformPropertyGetString/Int */

#include "debug.h"    /* TRACE, ASSERT & VERIFY */


/*
 * Macros & Constants
 */

/* Configurable vales */
#define PKEY_RECV_MODE "rxmode"
#define PKEY_MAC_ADDRESS "mac_address"
#define PKEY_BUFFER_SIZE "buffer_size"

/* Get a ptr to the start of a struct given a ptr to a field within it */
#ifndef SWS_CONTAINER
#define SWS_CONTAINER( ptr, type, field) \
  (type*)( (char*)(ptr) - offsetof( type, field) )
#endif

#define MAX_FRAME_SIZE 1514 /* Ethernet MTU=1500 (+14 byte MAC hdr), no FCS */

#define C_FLAG 1 /* flags register */

/* Packet driver interface classes */
#define CL_NONE         0
#define CL_ETHERNET     1
#define CL_PRONET_10    2
#define CL_IEEE8025     3
#define CL_OMNINET      4
#define CL_APPLETALK    5
#define CL_SERIAL_LINE  6
#define CL_STARLAN      7
#define CL_ARCNET       8
#define CL_AX25         9
#define CL_KISS         10
#define CL_IEEE8023     11
#define CL_FDDI         12
#define CL_INTERNET_X25 13
#define CL_LANSTAR      14
#define CL_SLFP         15
#define CL_NETROM       16
#define CL_PPP          17
#define CL_QTSO         18
#define NCLASS          19

/* Packet driver interface types (not a complete list) */
#define TC500           1
#define PC2000          10
#define WD8003          14
#define PC8250          15
#define ANYTYPE         0xffff

/* Packet driver function call numbers */
#define DRIVER_INFO        1
#define ACCESS_TYPE        2
#define RELEASE_TYPE       3
#define SEND_PKT           4
#define TERMINATE          5
#define GET_ADDRESS        6
#define RESET_INTERFACE    7
/* Optional high performance functions */
#define GET_PARAMETERS     10
#define AS_SEND_PKT        11
/* Optional extended functions */
#define SET_RCV_MODE       20
#define GET_RCV_MODE       21
#define SET_MULTICAST_LIST 22
#define GET_MULTICAST_LIST 23
#define GET_STATISTICS     24
#define SET_ADDRESS        25

/* Packet driver error return codes. From Appendix C. */
#define BAD_HANDLE     1  /* invalid handle number */
#define NO_CLASS       2  /* no interfaces of specified class found */
#define NO_TYPE        3  /* no interfaces of specified type found */
#define NO_NUMBER      4  /* no interfaces of specified number found */
#define BAD_TYPE       5  /* bad packet type specified */
#define NO_MULTICAST   6  /* this interface does not support multicast */
#define CANT_TERMINATE 7  /* this packet driver cannot terminate */
#define BAD_MODE       8  /* an invalid receiver mode was specified */
#define NO_SPACE       9  /* operation failed because of insufficient space */
#define TYPE_INUSE     10 /* the type had previously been accessed, and not released */
#define BAD_COMMAND    11 /* the command was out of range, or not implemented */
#define CANT_SEND      12 /* the packet couldn't be sent (usually hardware error) */
#define CANT_SET       13 /* hardware address couldn't be changed (> 1 handle open) */
#define BAD_ADDRESS    14 /* hardware address has bad length or format */
#define CANT_RESET     15 /* couldn't reset interface (> 1 handle open) */


/*
 * Local data types
 */

/* Packet driver instance */
typedef struct SPacketDriver
  {
  ui32 uMagic;
#define PACKET_MAGIC 0x3798e47fL

  struct SPacketDriver* pNext;        /* List of drivers */
  int iInterrupt;                     /* INT to call to access driver */
  unsigned int handle;                /* Driver handle */

  SWS_EnetMacAddress mac;             /* Cached MAC address */
  int macAddrIsValid;                 /* !0 if cache is valid */
  unsigned rxmode;

  SWS_EnetCallbacks upcalls;          /* Up-call info */

  char* rxBuffer;                     /* Receive buffer */
  volatile size_t read, write;        /* Read/write cursors into buffer */
  volatile size_t saveWrite;

  char txBuffer[ MAX_FRAME_SIZE];     /* Send buffer */
  } SPacketDriver;

/* Received data message */
typedef struct SPacketMsg
  {
  void* data;                         /* -> data */
  unsigned len;                       /* Length */
  } SPacketMsg;

/* Packet receive callback */
typedef void __far FReceiver( void);


/*
 * Prototypes
 */
static SWS_FEnetSend PacketSend;
static SWS_FEnetRecvAck PacketRecvAck;
static SWS_FEnetGetMacAddr PacketGetMacAddr;
static SWS_FEnetShutdown PacketShutdown;
static SWS_FEnetIoctl PacketIoctl;

static void __interrupt __far PacketRecv();

static int AddPacketDriver( int, unsigned);
static void FreePacketDriver( SPacketDriver*);
static int PacketSetRxMode( SPacketDriver* const,unsigned mode);
#if OPT_IP_MULTICAST
static int PacketSetMulticastList( SPacketDriver* const, void __far *, size_t);
#endif


/*
 * Module data
 */
static size_t s_sizeBuffer;
static SPacketDriver* s_pPacketDrivers; /* List of interfaces */

/* Statistics */
static volatile unsigned s_packets, s_buffers, s_errors, s_noHandle, s_badCmd;

/* Ethernet device descriptor */
static char s_szIdent[ sizeof( SWS_PACKET_IDENT "01") ];
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
int SWS_PacketInstall( void)          /* Returns !0 on error */
  {
  int i, iAdapters;

  TRACE( 9, ("SWS_PacketInstall()\n"));
  ASSERT( PlatformIsCritical());

  s_sizeBuffer = (size_t)PlatformPropertyGetUint(
    SWS_PACKET_IDENT, PKEY_BUFFER_SIZE, OPT_PKT_BUFFER_SIZE );

  /* Search for packet drivers */
  iAdapters = 0;
  for ( i = 0x60; i <= 0x80; ++i)
    {
    char __far * addr;

    /* Get -> INT handler */
    addr = (char __far*)(unsigned long)_dos_getvect( i);
    if ( addr)
      {
      static const char kszIdent[] = "PKT DRVR";

      if ( !_fstrcmp( addr + 3, kszIdent))
        {
        /* Packet driver found */
        int err;
        union REGS regs;
        struct SREGS sregs;
        unsigned if_num;
        char __far* lpszDriver;

        /* Get driver info */
        segread( &sregs);
        regs.h.ah = DRIVER_INFO;
        regs.h.al = 0xff;
        regs.x.bx = 0xffff;
        regs.x.cx = 0;
        regs.x.cflag = 0;
        int86x( i, &regs, &regs, &sregs);
        if ( (C_FLAG & regs.x.cflag) && BAD_HANDLE != regs.h.dh)
          {
          TRACE( 2, ("SWS_PacketInstall() %s error %u\n", kszIdent, regs.h.dh));
          continue;
          }
        if ( regs.h.ch != CL_ETHERNET)
          {
          TRACE( 2, ("SWS_PacketInstall() %s type %u\n", kszIdent, regs.h.ch));
          continue;
          }

        lpszDriver = MK_FP( sregs.ds, regs.x.si);
#if OPT_DEMO
        if ( _fstrcmp( lpszDriver, "SwsVpkt"))
          continue;
#else
        (void)lpszDriver;
#endif

        TRACE( 2, ("SWS_PacketInstall() '%ls' ver %u type %u func %u\n",
          lpszDriver, regs.x.bx, regs.x.dx, regs.h.al));

        /* Add all interfaces (count-1 in CL reg) */
        for ( if_num = 0; if_num <= regs.h.cl; ++if_num)
          {
          err = AddPacketDriver( i, if_num);
          if ( !err)
            ++iAdapters;
          else
            return err;
          }
        }
      }
    }

  if ( 0 == iAdapters)
    {
    TRACE( 1, ("*** SWS_PacketInstall() no adapters found\n"));
    return SWS_ENETDOWN;
    }

  return 0;
  }


/*
 * Finalise the driver
 * Single threaded
 */
void SWS_PacketCleanup( void)
  {
  SPacketDriver* pPacket;

  TRACE( 9, ("SWS_PacketCleanup()\n"));
  ASSERT( PlatformIsCritical());

  /* All the drivers should have been shutdown by now */
  while ( NULL != (pPacket = s_pPacketDrivers))
    {
    s_pPacketDrivers = pPacket->pNext;
    FreePacketDriver( pPacket);
    }

  TRACE( 1, ("PacketDrvr: RxPkts:%u BufReqs:%u Errors:%u NoHandle:%u BadCmd:%u\n",
    s_packets, s_buffers, s_errors, s_noHandle, s_badCmd
  ));
  }


/*
 * Create an SwsSock interface for a packet driver
 */
int AddPacketDriver( int iInterrupt, unsigned if_num)
  {
  static unsigned s_uInterfaces; /* No. packet driver interfaces */
  SPacketDriver* pPacket;
  union REGS regs;
  struct SREGS sregs;
  int iRet;
#ifdef __WATCOMC__
  FReceiver * const pfn = (FReceiver *)&PacketRecv;
#else
  FReceiver __far * const pfn = (FReceiver __far*)&PacketRecv;
#endif

  TRACE( 9, ("AddPacketDriver(%d,%u)\n", iInterrupt,if_num));

  sprintf( s_szIdent, SWS_PACKET_IDENT "%u", ++s_uInterfaces);

  /* Allocate instance data */
  pPacket = (SPacketDriver*)calloc( 1, sizeof(SPacketDriver));
  if ( NULL == pPacket)
    {
    TRACE( 0, ("!!! AddPacketDriver() Out of memory for instance\n"));
    return SWS_ENOMEM;
    }

  pPacket->handle = 0xffff;

  /* Allocate the receive buffer */
  pPacket->rxBuffer = (char*)malloc( s_sizeBuffer);
  if ( NULL == pPacket->rxBuffer)
    {
    TRACE( 1, ("*** AddPacketDriver malloc(%u) RxBuffer failed\n", s_sizeBuffer));
    FreePacketDriver( pPacket);
    return SWS_ENOMEM;
    }

  /* Attach to packet driver */
  segread( &sregs);
#if OPT_DEMO
  {
  void __far* pv;
  regs.h.ah = 0x80;
  regs.h.al = 'S';
  regs.x.bx = 0x1234;
  regs.h.dl = (unsigned char)if_num;
  regs.x.di = FP_OFF( pfn);
  sregs.es = FP_SEG( pfn); /* es:di -> real mode callback */
  regs.x.cx = sprintf( pPacket->txBuffer, "SwsVpkt");
  pv = pPacket->txBuffer;
  regs.x.si = FP_OFF( pv);
  sregs.ds = FP_SEG( pv);
  }
#else
  regs.h.ah = ACCESS_TYPE;
  regs.h.al = CL_ETHERNET;
  regs.x.bx = ANYTYPE;
  regs.h.dl = (unsigned char)if_num;
  regs.x.di = FP_OFF( pfn);
  sregs.es = FP_SEG( pfn); /* es:di -> real mode callback */
  regs.x.si = 0;
  sregs.ds = 0; /* ds:si -> type */
  regs.x.cx = 0; /* typelen, 0= all types */
#endif
  regs.x.cflag = 0;
  int86x( iInterrupt, &regs, &regs, &sregs);
  if ( C_FLAG & regs.x.cflag)
    {
    TRACE( 1, ("*** SWS_PacketInstall() ACCESS_TYPE failed %d\n", regs.h.dh));
    FreePacketDriver( pPacket);
    return 1;
    }
  pPacket->handle = regs.x.ax;
  pPacket->iInterrupt = iInterrupt;
  pPacket->uMagic = PACKET_MAGIC;

#if 0 /* BUGBUG: Stops Netgear FA310TX packet driver from working */
  /* Reset the interface */
  regs.h.ah = RESET_INTERFACE;
  regs.x.bx = pPacket->handle;
  regs.x.cflag = 0;
  int86x( iInterrupt, &regs, &regs, &sregs);
  if ( C_FLAG & regs.x.cflag)
    {
    TRACE( 1, ("*** SWS_PacketInstall() RESET_INTERFACE failed %d\n", regs.h.dh));
    }
#endif

#if 0
  {
  /* Read a user configured MAC address.  This overrides the value in EEPROM
   * BUGBUG: FA310TX packet driver stops all Rx data - must reload driver
   */
  char szBuf[ 32];
  unsigned addr[6];
  if ( 0 != PlatformPropertyGetString( s_szIdent, PKEY_MAC_ADDRESS, "", szBuf, sizeof(szBuf) )
    && 6 == sscanf( szBuf, "%x-%x-%x-%x-%x-%x", addr, addr+1, addr+2, addr+3, addr+4, addr+5)
  ) {
    void __far* p;

    pPacket->mac.addr[0] = (ui8)addr[0];
    pPacket->mac.addr[1] = (ui8)addr[1];
    pPacket->mac.addr[2] = (ui8)addr[2];
    pPacket->mac.addr[3] = (ui8)addr[3];
    pPacket->mac.addr[4] = (ui8)addr[4];
    pPacket->mac.addr[5] = (ui8)addr[5];

    /* Set the MAC address */
    regs.h.ah = SET_ADDRESS; /* Extended function */
    regs.x.bx = pPacket->handle;
    p = pPacket->mac.addr;
    regs.x.di = FP_OFF( p);
    sregs.es = FP_SEG( p); /* es:di -> MAC address */
    regs.x.cx = sizeof( SWS_EnetMacAddress);
    regs.x.cflag = 0;
    int86x( iInterrupt, &regs, &regs, &sregs);
    if ( C_FLAG & regs.x.cflag)
      {
      TRACE( 1, ("*** SWS_PacketInstall SET_ADDRESS error %d\n", regs.h.dh));
      }
    else
      {
      /* Cache the address for later */
      pPacket->macAddrIsValid = 1;
      }
    }
  }
#endif

  /* Set receive mode */
  iRet = (int)PlatformPropertyGetUint( s_szIdent, PKEY_RECV_MODE, 3 );
  if ( PacketSetRxMode( pPacket, iRet))
    pPacket->rxmode = 3;

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

  /* Add to list of drivers */
  pPacket->pNext = s_pPacketDrivers;
  s_pPacketDrivers = pPacket;

  return 0;
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

  pPacket->uMagic = 0;
  if ( 0xffff != pPacket->handle)
    {
    union REGS regs;

    if ( 3 != pPacket->rxmode)
      PacketSetRxMode( pPacket, 3);

    regs.h.ah = RELEASE_TYPE;
    regs.x.bx = pPacket->handle;
    regs.x.cflag = 0;
    int86( pPacket->iInterrupt, &regs, &regs);

    pPacket->handle = 0xffff;
    }

  if ( NULL != pPacket->rxBuffer)
    {
    free( pPacket->rxBuffer);
    pPacket->rxBuffer = NULL;
    }

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
  SWS_size_t len, uLen, uSrc;
  union REGS regs;
  struct SREGS sregs;
  char __far* buffer;

  TRACE( 9, ("PacketSend(%p,%p)\n",pvContext,hBuffer));
  ASSERT( NULL != pPacket);
  ASSERT( PACKET_MAGIC == pPacket->uMagic);

  len = SWS_BufferSize( hBuffer);
  ASSERT( len >= 60 /* Min Ethernet frame */);

  /* Copy the packet data into a linear buffer */
  uSrc = 0;
  uLen = len;
  while ( uLen)
    {
    SWS_PTR( const void) pvSrc;
    SWS_size_t uLenSrc;

    pvSrc = SWS_BufferGetFragment( hBuffer, uSrc, &uLenSrc);
    ASSERT( NULL != pvSrc);

    if ( uLen < uLenSrc)
      uLenSrc = uLen;

    /* Copy to send buffer */
    memcpy( pPacket->txBuffer + uSrc, pvSrc, uLenSrc);
    uSrc += uLenSrc;
    uLen -= uLenSrc;
    }

  /* Request the packet driver to send the frame */
  segread( &sregs);
  regs.h.ah = SEND_PKT;
  buffer = pPacket->txBuffer;
  regs.x.si = FP_OFF( buffer);
  sregs.ds = FP_SEG( buffer); /* ds:si -> send buffer */
  regs.x.cx = len;
  regs.x.cflag = 0;
  int86x( pPacket->iInterrupt, &regs, &regs, &sregs);
  if ( C_FLAG & regs.x.cflag)
    {
    TRACE( 1, ("*** PacketSend driver error %d\n", regs.h.dh));
    return SWS_BASEERR + 1000 + regs.h.dh;
    }

  return 0;
  }


#ifdef __BORLANDC__
#pragma option -N- /* disable stack checking */
#endif
#ifdef _MSC_VER
#pragma check_stack( off)
#endif

/*
 * Receive complete.  Buffer is returned to driver
 * Single threaded
 */
SWS_CB( void, PacketRecvAck(
  SWS_PTR( void) pvContext,           /* IN: Driver's context */
  SWS_PTR( void) pvData,              /* IN: Buffer */
  size_t len                          /* IN: Buffer size */
)){
  SPacketDriver* const pPacket = (SPacketDriver*)pvContext;
  size_t read;

  TRACE( 10, ("PacketRecvAck(%p,%p,%u)\n",pvContext,pvData,(unsigned)len));
  ASSERT( NULL != pPacket);
  ASSERT( PACKET_MAGIC == pPacket->uMagic);
  ASSERT( (char*)pvData == pPacket->rxBuffer + pPacket->read );

  /* Buffer is now ready to be re-used */
  read = (size_t)(((char*)pvData + len) - pPacket->rxBuffer);
  if ( read < s_sizeBuffer)
    pPacket->read = read;
  else
    pPacket->read = 0;
  }


/*
 * Packet driver receive upcall.
 * Called by the packet driver twice for each incoming packet:
 * 1. with ax == 0 to allocate a buffer, returned in es:di
 * 2. with ax == 1 & ds:si -> buffer to signal data available.
 * bx= handle
 * cx= len.
 *
 * CAUTION IRQ TIME
 */
#ifdef _MSC_VER
#pragma warning( disable : 4704)    /* in-line assembler precludes global optimizations */
#endif

static void __interrupt __far PacketRecv(
#ifdef __WATCOMC__
  union INTPACK r
# define regES r.w.es
# define regDS r.w.ds
# define regDI r.w.di
# define regSI r.w.si
# define regBP r.w.bp
# define regSP r.w.sp
# define regBX r.w.bx
# define regDX r.w.dx
# define regCX r.w.cx
# define regAX r.w.ax
#elif defined __BORLANDC__
  unsigned regBP, unsigned regDI, unsigned regSI,
  unsigned regDS, unsigned regES, unsigned regDX,
  unsigned regCX, unsigned regBX, unsigned regAX
#else
  unsigned regES, unsigned regDS,
  unsigned regDI, unsigned regSI,
  unsigned regBP, unsigned regSP,
  unsigned regBX, unsigned regDX,
  unsigned regCX, unsigned regAX
#endif
) {
  SPacketDriver* pPacket;
#if defined __WATCOMC__
#elif defined __BORLANDC__
  (void)regBP;
  (void)regDX;
  (void)regDI;
  (void)regES;
#else
  (void)regBP;
  (void)regDX;
  (void)regSP;
#endif

  /* Ensure irq's are enabled */
  _enable();

  for ( pPacket = s_pPacketDrivers; NULL != pPacket; pPacket = pPacket->pNext)
    {
    if ( pPacket->handle == regBX)
      break;
    }

  if ( NULL != pPacket)
    {
    unsigned len, space;
    char __far * buffer;

    len = regCX;
    switch ( regAX)
      {
    case 0: /* Request for packet buffer */
      ++s_buffers;

      pPacket->saveWrite = pPacket->write;

      /* Calc. space remaining in buffer */
      if ( pPacket->write >= pPacket->read)
        {
        space = s_sizeBuffer - pPacket->write;
        if ( 0 == pPacket->read)
          --space;

        if ( len > space)
          {
          /* Wraparound and try from start of buffer */
          space = pPacket->read;
          if ( space > 0)
            --space;

          if ( len < space)
            pPacket->write = 0;
          }
        }
      else
        {
        space = (pPacket->read - pPacket->write) - 1;
        }

      if ( len <= space)
        {
        /* Calc address of write cursor in the receive buffer */
        buffer = pPacket->rxBuffer + pPacket->write;

        /* Update write cursor */
        pPacket->write += len;
        ASSERT( pPacket->write <= s_sizeBuffer);
        if ( pPacket->write >= s_sizeBuffer)
          pPacket->write = 0;
        }
      else
        {
        /* Out of buffer space */
        buffer = NULL;
        (*pPacket->upcalls.pfnRecvInd)( pPacket->upcalls.pvContext, NULL, 0);
        }
      regDI = FP_OFF( buffer);
      regES = FP_SEG( buffer);
      break;

    case 1: /* Packet ready */
      ++s_packets;

      buffer = (char __far *) MK_FP( regDS, regSI);
      ASSERT( buffer >= pPacket->rxBuffer);
      ASSERT( buffer + len <= pPacket->rxBuffer + s_sizeBuffer);

      if ( len > 0)
        {
        /* Indicate data received */
        if ( 0 < (*pPacket->upcalls.pfnRecvInd)(
            pPacket->upcalls.pvContext, buffer, len)
        ) {
          /* Error so drop the packet and restore the write cursor */
          ++s_errors;

          len += (unsigned)((char*)buffer - pPacket->rxBuffer);
          if ( len >= s_sizeBuffer)
            len = 0;
          if ( len == pPacket->write)
            pPacket->write = pPacket->saveWrite;
          }
        }
      break;

    default:
      ++s_badCmd;
      break;
      }
    }
  else
    {
    ++s_noHandle;
    regES = regDI = 0;
    }

  /* NB MUST retf not iret */
#ifdef __WATCOMC__
  __asm mov sp,bp
  __asm pop ax
  __asm pop ax
  __asm pop es
  __asm pop ds
  __asm popa
  __asm retf
#elif defined __BORLANDC__
  __asm mov sp,bp
  __asm pop bp
  __asm pop di
  __asm pop si
  __asm pop ds
  __asm pop es
  __asm pop dx
  __asm pop cx
  __asm pop bx
  __asm pop ax
  __asm retf /* NB this gives an unknown ins warning but compiles OK */
#elif defined _MSC_VER
  __asm mov sp,bp
  __asm pop es
  __asm pop ds
  __asm popa
  __asm retf
#else
#error Unknown compiler
#endif
  }

#ifdef __BORLANDC__
#pragma option -r.
#pragma option -N.
#endif
#ifdef _MSC_VER
#pragma warning( default : 4704)
#pragma check_stack()
#endif


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
    union REGS regs;
    struct SREGS sregs;
    ui8 __far * p;

    /* Get MAC address from the packet driver */
    segread( &sregs);
    regs.h.ah = GET_ADDRESS;
    regs.x.bx = pPacket->handle;
    p = pPacket->mac.addr;
    regs.x.di = FP_OFF( p);
    sregs.es = FP_SEG( p);
    regs.x.cx = sizeof( pPacket->mac.addr);
    regs.x.cflag = 0;
    int86x( pPacket->iInterrupt, &regs, &regs, &sregs);
    if ( C_FLAG & regs.x.cflag)
      {
      TRACE( 1, ("*** PacketGetMacAddr driver error %d\n", regs.h.dh));
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
  unsigned mode;

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

    mode = *((SWS_PTR( ui32)) pvData) ? 6 : 3;
    if ( 6 == mode || 6 == pPacket->rxmode)
      return PacketSetRxMode( pPacket, mode );

    break;

#if OPT_IP_MULTICAST
  case SWS_kEnetIoctl_MulticastList:
    if ( 0 == *pSize)
      {
      /* Disable multicast */
      if ( 4 == pPacket->rxmode || 5 == pPacket->rxmode)
        PacketSetRxMode( pPacket, 3);
      }
    /* Set multicast list */
    else if ( 0 == PacketSetMulticastList( pPacket, pvData, *pSize))
      {
      /* The FA310TX packet driver (ngrpci v2.21 (980811)) has a bug in
       * enabling multicast reception. However, if mode 5 (all multicast)
       * is selected before mode 4 and a multicast packet is sent then all
       * subsequent multicast packets are received.
       */
      if ( 3 == pPacket->rxmode)
        {
        PacketSetRxMode( pPacket, 5);
        PacketSetRxMode( pPacket, 4);
        }
      else if ( 5 == pPacket->rxmode)
        {
        PacketSetRxMode( pPacket, 4);
        }
      }
    /* Unable to set multicast filter so enable rx all multicast */
    else if ( 3 == pPacket->rxmode || 4 == pPacket->rxmode)
      {
      PacketSetRxMode( pPacket, 5);
      }
    break;
#endif

#if OPT_DEMO
  case SWS_kDatalinkProperty_Private + 0x1234:
    if ( *pSize != sizeof(ui32))
      return SWS_EINVAL;

    *(SWS_PTR( ui32))pvData = (*(SWS_PTR( ui32))pvData) ^ 0xDEC0DEA5L;
    return 0;
#endif
    }

  return 0;
  }


/*
 * Set receive mode
 * 1= turn off receiver
 * 2= only packets sent to this interface
 * 3= mode 2 plus broadcast packets (default)
 * 4= mode 3 plus limited multicast packets
 * 5= mode 3 plus all multicast packets
 * 6= all packets
 *
 * BUGBUG: FA310TX packet driver
 *  mode 1 == 3,
 *  modes 2,4,5 stop all receive data - must reload driver
 *  mode 6 == 1, ie not promiscuous
 */
static int PacketSetRxMode(
  SPacketDriver* const pPacket,
  unsigned mode
) {
  union REGS regs;

  TRACE( 9, ("PacketSetRxMode(%p,%u)\n",pPacket,mode));
  ASSERT( NULL != pPacket);
  ASSERT( PACKET_MAGIC == pPacket->uMagic);

  /* set_rcv_mode - extended function */
  regs.h.ah = SET_RCV_MODE;
  regs.x.bx = pPacket->handle;
  regs.x.cx = mode;
  regs.x.cflag = 0;
  int86( pPacket->iInterrupt, &regs, &regs);
  if ( C_FLAG & regs.x.cflag)
    {
    TRACE( 1, ("*** PacketSetRxMode() SET_RCV_MODE %d failed %d\n", regs.x.cx, regs.h.dh));
    return SWS_EINVAL;
    }

  pPacket->rxmode = mode;
  return 0;
  }


#if OPT_IP_MULTICAST
/*
 * Set multicast list
 */
static int PacketSetMulticastList(
  SPacketDriver* const pPacket,
  void __far * pv,
  size_t size
) {
  union REGS regs;
  struct SREGS sregs;

  TRACE( 9, ("PacketSetMulticastList(%p,%p,%u)\n",pPacket,pv,size));
  ASSERT( NULL != pPacket);
  ASSERT( PACKET_MAGIC == pPacket->uMagic);

  regs.h.ah = SET_MULTICAST_LIST; /* extended function */
  regs.x.bx = pPacket->handle;
  regs.x.cx = size;
  regs.x.di = FP_OFF( pv);
  segread( &sregs);
  sregs.es = FP_SEG( pv);
  regs.x.cflag = 0;
  int86x( pPacket->iInterrupt, &regs, &regs, &sregs);
  if ( C_FLAG & regs.x.cflag)
    {
    TRACE( 1, ("*** PacketSetMulticastList driver error %d\n", regs.h.dh));
    return 1;
    }
  return 0;
  }
#endif
#endif /* OPT_DATALINK_ETHERNET */
/* End of file */
