/****************************************************************************
 *
 * File name   : packetx.c
 * Function    : PF_PACKET/SOCK_RAW ethernet datalink
 * Project     : SwsSock
 * Authors     : Lawrence Rust
 * Systems     : ANSI C, Linux 2.2+
 *
 ****************************************************************************
 *
 * Created by Lawrence Rust, Software Systems Consultants
 * lvr@softsystem.co.uk
 *
 * 13-Aug-09, LVR, Created.
 *
 * Comments:
 * --------
 * This file implements a PF_PACKET/SOCK_RAW userspace protocol.
 *
 ****************************************************************************
 */

/* Compilation options */
#ifndef _GNU_SOURCE
#define _GNU_SOURCE     /* For F_SETSIG */
#endif

#include "config.h"

/* Exports */
#include "packetx.h"

#if OPT_DATALINK_ETHERNET

/* Imports */
/* ANSI C library */
#include <stddef.h>
#include <string.h>   /* memcpy */
#include <ctype.h>
#include <errno.h>
#include <signal.h>

/* POSIX/BSD */
#include <unistd.h>       /* close usleep, gethostname */
#include <fcntl.h>        /* fcntl */

/* BSD sockets */
#include <sys/socket.h>   /* socket, accept... */
#include <sys/ioctl.h>    /* ioctl */
#include <arpa/inet.h>    /* htonl etc and inet_addr */

/* Linux platform */
#if __GLIBC__ < 2 || (__GLIBC__ == 2 &&  __GLIBC_MINOR__ < 1)
/* Packet sockets pre glibc 2.1 */
# include <asm/types.h>
# include <linux/if_packet.h>
# include <linux/if_ether.h>  /* The L2 protocols */
#else
/* Packet sockets post glibc 2.1 */
# include <netpacket/packet.h>
# include <net/ethernet.h> /* the L2 protocols */
#endif
#include <net/if_arp.h>
#include <net/if.h>

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
#include "deque.h"    /* Lists */
#include "debug.h"    /* TRACE, ASSERT & VERIFY */


/*
 * Macros & Constants
 */

/* Configurable vales */
#define PKEY_SIGNAL "signal"  /* Numeric */
#ifndef PACKETX_SIGNAL
#define PACKETX_SIGNAL SIGIO
#endif

#define PKEY_PACKETX_DISABLE   "disable"
#define PKEY_PACKETX_RXBUFFERS "rxbuffers"
#ifndef OPT_PACKETX_RXBUFFERS
#define OPT_PACKETX_RXBUFFERS 32
#endif

/* Get a ptr to the start of a struct given a ptr to a field within it */
#ifndef SWS_CONTAINER
#define SWS_CONTAINER( ptr, type, field) \
  (type*)( (char*)(ptr) - offsetof( type, field) )
#endif

/* glibc  2.1  does  not  have  a  define  for  SOL_PACKET */
#ifndef SOL_PACKET
#define SOL_PACKET 263
#endif

#define MAX_MCASTS 16


/*
 * Local data types
 */

/* Packet driver instance */
typedef struct SPacketDriver
  {
  ui32 uMagic;
#define PACKET_MAGIC 'pktX'

  /* netdevice index and name */
  int ifindex;
  char szName[ IFNAMSIZ];

  struct SPacketDriver* pNext;        /* List of drivers */
  SWS_EnetCallbacks upcalls;          /* Up-call info */

  SWS_EnetMacAddress mac;             /* Cached MAC address */
  int macAddrIsValid;                 /* !0 if cache is valid */

#if OPT_IP_MULTICAST
  int iMulticasts;
  SWS_EnetMacAddress mcast[ MAX_MCASTS];
#endif

  unsigned char txBuffer[ ETH_FRAME_LEN];
  } SPacketDriver;
COMPILATION_REQUIRES( sizeof( SWS_EnetMacAddress) == ETH_ALEN);

/* Receive buffer */
typedef struct SBuffer
  {
  SDeque list;
#define LIST2BUFFER( p) SWS_CONTAINER( p, SBuffer, list)
  unsigned char buf[ ETH_FRAME_LEN];
  } SBuffer;


/*
 * Prototypes
 */
static SWS_FEnetSend PacketXSend;
static SWS_FEnetRecvAck PacketXRecvAck;
static SWS_FEnetGetMacAddr PacketXGetMacAddr;
static SWS_FEnetShutdown PacketXShutdown;
static SWS_FEnetIoctl PacketXIoctl;

static int AddPacketInterface( int, const char []);
static void FreePacketInterface( SPacketDriver*);
static int SetPromiscuity( SPacketDriver* const,unsigned mode);
#if OPT_IP_MULTICAST
static int SetMulticastList( SPacketDriver* const, SWS_PTR( void), size_t);
#endif
static void io_signal( int iSig);


/*
 * Module data
 */
/* Configuration */
static int s_iSignal;

/* Operation */
static sighandler_t s_pfnSignal;
static int s_sockpkt;                 /* packet socket */
static SPacketDriver* s_pPacketDrivers; /* List of interfaces */
static SBuffer* s_pBuffers;           /* Rx buffers */
static SDeque s_qRxBufs;              /* Rx buffer free list */

/* Statistics */
static unsigned long s_ulTxPackets, s_ulTxErrors, s_ulTxBytes;
static unsigned long s_ulRxPackets, s_ulRxErrors, s_ulRxBytes;

/* Ethernet device descriptor */
static char s_szIdent[ sizeof SWS_PACKETX_IDENT + IFNAMSIZ ];
static SWS_EnetDevice const s_packet =
  {
  sizeof s_packet,
  SWS_ENET_VERSION,
  &s_szIdent[ 0],
  &PacketXSend,
  &PacketXRecvAck,
  &PacketXGetMacAddr,
  &PacketXShutdown,
  &PacketXIoctl
  };


/*
 * Initialise the driver
 * Single threaded
 */
int SWS_PacketXInstall( void)          /* Returns !0 on error */
  {
  int i, iAdapters;
  unsigned u, uBufs;

  TRACE( 9, ("SWS_PacketXInstall()\n"));
  ASSERT( PlatformIsCritical());

  /* Get configuration data */
  if ( PlatformPropertyGetUint( SWS_PACKETX_IDENT, PKEY_PACKETX_DISABLE, 0))
    {
    TRACE( 2, ("SWS_PacketXInstall() Disabled\n"));
    return SWS_ENETDOWN;
    }

  s_iSignal = PlatformPropertyGetUint( SWS_PACKETX_IDENT, PKEY_SIGNAL, PACKETX_SIGNAL );
#ifndef F_SETSIG
  {int WARN_F_SETSIG_NotDefined;}
  if ( SIGIO != s_iSignal)
    {
    TRACE( 1, ("*** SWS_PacketXInstall() Signal must be SIGIO."
      " Recompile with F_SETSIG support\n"));
    return SWS_ENETDOWN;
    }
#endif

  /* Install async i/o signal handler */
  s_pfnSignal =  signal( s_iSignal, &io_signal);
  if ( SIG_ERR == s_pfnSignal)
    {
    TRACE( 1, ("*** SWS_PacketXInstall() signal(%d) failed: %s\n",
      s_iSignal, strerror( errno)));
    return SWS_ENETDOWN;
    }

  uBufs = PlatformPropertyGetUint(
    SWS_PACKETX_IDENT, PKEY_PACKETX_RXBUFFERS, OPT_PACKETX_RXBUFFERS );

  /* Allocate buffer pool */
  s_pBuffers = (SBuffer*) malloc( uBufs * sizeof( SBuffer));
  if ( NULL == s_pBuffers)
    {
    TRACE( 0, ("!!! SWS_PacketXInstall() Out of memory for RxQ\n"));
    signal( s_iSignal, s_pfnSignal);
    return SWS_ENETDOWN;
    }

  /* Create free buffer list */
  DEQUE_INIT( &s_qRxBufs);
  for ( u = 0; u < uBufs; ++u)
    {
    DEQUE_INIT( &s_pBuffers[ u].list);
    SWS_DequeAppend( &s_qRxBufs, &s_pBuffers[ u].list);
    }

  s_sockpkt = socket( PF_PACKET, SOCK_RAW, htons( ETH_P_ALL));
  if ( -1 == s_sockpkt)
    {
    TRACE( 1, ("*** SWS_PacketXInstall() packet socket failed: %s\n", strerror( errno)));
    free( s_pBuffers), s_pBuffers = NULL;
    signal( s_iSignal, s_pfnSignal);
    return SWS_ENETDOWN;
    }

  /* Search for physical interfaces */
  iAdapters = 0;
  for ( i = 1; i < 256 && iAdapters < OPT_DATALINK_ETHERNET; ++i)
    {
    struct ifreq req;

    /* Get interface's name */
    req.ifr_ifindex = i;
    if ( -1 == ioctl( s_sockpkt, SIOCGIFNAME, &req))
      continue;

    /* Get interface's flags */
    if ( -1 == ioctl( s_sockpkt, SIOCGIFFLAGS, &req))
      {
      TRACE( 1, ("*** SWS_PacketXInstall() %s ioctl(SIOCGIFFLAGS): %s\n",
        req.ifr_name, strerror( errno) ));
      continue;
      }
    if ( !(req.ifr_flags & IFF_UP) )
      {
      TRACE( 2, ("SWS_PacketXInstall() %s is not up\n", req.ifr_name));
      continue;
      }
    if ( !(req.ifr_flags & IFF_RUNNING) )
      {
      TRACE( 2, ("SWS_PacketXInstall() %s is not running\n", req.ifr_name));
      continue;
      }
#if 0
    if ( req.ifr_flags & IFF_LOOPBACK)
      continue;
#endif

    /* Get hardware address */
    if ( -1 == ioctl( s_sockpkt, SIOCGIFHWADDR, &req))
      {
      TRACE( 1, ("*** SWS_PacketXInstall() %s ioctl(SIOCGIFHWADDR): %s\n",
        req.ifr_name, strerror( errno) ));
      continue;
      }
    if ( ARPHRD_ETHER != req.ifr_hwaddr.sa_family)
      {
      TRACE( 2, ("SWS_PacketXInstall() %s is not Ethernet (%#x)\n",
        req.ifr_name, req.ifr_hwaddr.sa_family));
      continue;
      }

    TRACE( 2, ("SWS_PacketXInstall() iface #%d = %s, Hwaddr %02x-%02x-%02x-%02x-%02x-%02x\n",
      i, req.ifr_name,
      (unsigned char)req.ifr_hwaddr.sa_data[0],
      (unsigned char)req.ifr_hwaddr.sa_data[1],
      (unsigned char)req.ifr_hwaddr.sa_data[2],
      (unsigned char)req.ifr_hwaddr.sa_data[3],
      (unsigned char)req.ifr_hwaddr.sa_data[4],
      (unsigned char)req.ifr_hwaddr.sa_data[5]
    ));
    if ( !AddPacketInterface( i, req.ifr_name))
      ++iAdapters;
    }

  if ( 0 == iAdapters)
    {
    TRACE( 1, ("*** SWS_PacketXInstall() no adapters found\n"));
    close( s_sockpkt), s_sockpkt = -1;
    free( s_pBuffers), s_pBuffers = NULL;
    signal( s_iSignal, s_pfnSignal);
    return SWS_ENETDOWN;
    }

  /* Request signal on tx/rx events */
  if ( -1 == fcntl( s_sockpkt, F_SETFL, O_ASYNC | fcntl( s_sockpkt, F_GETFL)) )
    TRACE( 1, ("*** SWS_PacketXInstall() fcntl F_SETFL, O_ASYNC failed: %s\n", strerror( errno)));
#ifdef F_SETSIG
  else if ( SIGIO != s_iSignal && -1 == fcntl( s_sockpkt, F_SETSIG, s_iSignal))
    TRACE( 1, ("*** SWS_PacketXInstall() fcntl F_SETSIG %d failed: %s\n", s_iSignal, strerror( errno)));
#endif
  else if ( -1 == fcntl( s_sockpkt, F_SETOWN, getpid()))
    TRACE( 1, ("*** SWS_PacketXInstall() fcntl F_SETOWN failed: %s\n", strerror( errno)));

#if OPT_DNS
  /* DNS server set? */
  if ( (i = 1, SWS_INADDR_ANY == SWS_DnsGetPriServer())
    || (i = 2, SWS_INADDR_ANY == SWS_DnsGetSecServer())
  ) {
    /* No, get default DNS server */
    FILE* file = fopen( "/etc/resolv.conf", "r");
    if ( NULL != file)
      {
      char szBuf[ 160];

      /* Read the file one line at a time */
      while ( NULL != fgets( szBuf, sizeof szBuf, file))
        {
        /* Search for a line: nameserver w.x.y.z */
        static const char kszTag[] = "nameserver";
        if ( 0 == strncasecmp( kszTag, szBuf, sizeof kszTag -1))
          {
          ui32 val;
          const char* psz = szBuf + sizeof kszTag -1;
          while ( isspace( *psz))
            ++psz;

          val = inet_addr( psz);
          if ( INADDR_NONE != val)
            {
            val = SWS_ntohl( val);
            if ( 1 == i++)
              {
              SWS_DnsSetPriServer( val);
              }
            else if ( val != SWS_DnsGetPriServer())
              {
              SWS_DnsSetSecServer( val);
              break;
              }
            }
          }
        }

      fclose( file);
      }
    }
#endif

  return 0;
  }


/*
 * Finalise the driver
 * Single threaded
 */
void SWS_PacketXCleanup( void)
  {
  SPacketDriver* pPacket;

  TRACE( 9, ("SWS_PacketXCleanup()\n"));
  ASSERT( PlatformIsCritical());

  /* All the drivers should have been shutdown by now */
  while ( NULL != (pPacket = s_pPacketDrivers))
    {
    s_pPacketDrivers = pPacket->pNext;
    FreePacketInterface( pPacket);
    }

  close( s_sockpkt), s_sockpkt = -1;
  DEQUE_INIT( &s_qRxBufs);
  free( s_pBuffers), s_pBuffers = NULL;
  signal( s_iSignal, s_pfnSignal);

  TRACE( 1, ("PcktX: TxPackets:%lu TxBytes:%lu TxErrors:%lu"
    " RxPackets:%lu RxBytes:%lu RxErrors:%lu\n",
    s_ulTxPackets, s_ulTxBytes, s_ulTxErrors,
    s_ulRxPackets, s_ulRxBytes, s_ulRxErrors
  ));
  }


/*
 * Create an SwsSock interface for a packet driver
 */
int AddPacketInterface( int ifindex, const char szName[])
  {
  SPacketDriver* pPacket;
  int iRet;

  TRACE( 9, ("AddPacketInterface(%s)\n", szName));

  /* Create adapter name */
  sprintf( s_szIdent, SWS_PACKETX_IDENT "%s", szName);

  /* Is the adapter disabled? */
  if ( PlatformPropertyGetUint( s_szIdent, PKEY_PACKETX_DISABLE, 0))
    {
    TRACE( 2, ("AddPacketInterface() %s disabled\n", s_szIdent));
    return SWS_ENODEV;
    }

  /* Allocate instance data */
  pPacket = (SPacketDriver*)calloc( 1, sizeof( SPacketDriver));
  if ( NULL == pPacket)
    {
    TRACE( 0, ("!!! AddPacketInterface() Out of memory for instance\n"));
    return SWS_ENOMEM;
    }

  pPacket->uMagic = PACKET_MAGIC;
  pPacket->ifindex = ifindex;
  strcpy( pPacket->szName, szName);

  /* Add to list of drivers */
  pPacket->pNext = s_pPacketDrivers;
  s_pPacketDrivers = pPacket;

  /* Register with SwsSock as an Ethernet driver */
  pPacket->upcalls.uSize = sizeof pPacket->upcalls;
  iRet = SWS_EnetAddAdapter( &s_packet, pPacket, &pPacket->upcalls);
  if ( iRet)
    {
    s_pPacketDrivers = pPacket->pNext;
    FreePacketInterface( pPacket);
    return iRet;
    }
  ASSERT( pPacket->upcalls.pfnSendAck);
  ASSERT( pPacket->upcalls.pfnRecvInd);

  return 0;
  }


/*
 * Callback from Ethernet module to shutdown an adapter
 */
static SWS_CB( void, PacketXShutdown(
  SWS_PTR( void) pvContext            /* IN: Client context */
)){
  SPacketDriver* pPacket, *pPrev;;

  TRACE( 9, ("PacketXShutdown(%p)\n", pvContext));
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
      FreePacketInterface( pPacket);
      return;
      }

    pPrev = pPacket;
    }

  TRACE( 1, ("*** PacketXShutdown() Client %p not found)\n", pvContext));
  }


/*
 * Release packet driver resources
 */
void FreePacketInterface( SPacketDriver* pPacket)
  {
  TRACE( 9, ("FreePacketInterface(%p)\n", pPacket));
  ASSERT( NULL != pPacket);

  free( pPacket);
  }


/*
 * Transmit a buffer, possibly fragmented
 * Single threaded
 */
SWS_CB( int, PacketXSend(             /* Return !0 on error, -1 if pending */
  SWS_PTR( void) pvContext,           /* IN: Driver's context */
  SWS_BufferH hBuffer                 /* IN: Buffer to send (owned by caller) */
)){
  SPacketDriver* const pPacket = (SPacketDriver*)pvContext;
  SWS_size_t len;
  struct sockaddr_ll sa;

  TRACE( 9, ("PacketXSend(%p,%p)\n",pvContext,hBuffer));
  ASSERT( NULL != pPacket);
  ASSERT( PACKET_MAGIC == pPacket->uMagic);

  len = SWS_BufferSize( hBuffer);
  ASSERT( len >= ETH_ZLEN); /* Min Ethernet frame */

  if ( len > sizeof pPacket->txBuffer)
    {
    TRACE( 1, ("*** PacketXSend packet too big\n"));
    return SWS_EINVAL;
    }

  /* Defragment buffer */
  SWS_BufferCopyToMemory( pPacket->txBuffer, hBuffer, 0, len);

  /* Specify destination addr
  * NB Packets aren't looped back so can't send to host
  */
  memset( &sa, 0, sizeof sa);
  sa.sll_family = AF_PACKET;
  sa.sll_ifindex = pPacket->ifindex;
#if 0
  sa.sll_halen = sizeof pPacket->mac.addr;
  memcpy( sa.sll_addr, pPacket->mac.addr, sizeof pPacket->mac.addr);
#elif 1
  sa.sll_halen = ETH_ALEN;
  memcpy( sa.sll_addr, pPacket->txBuffer, ETH_ALEN);
#endif

  if ( -1 == sendto( s_sockpkt, pPacket->txBuffer, len, 0, (struct sockaddr*)&sa, sizeof sa))
    {
    TRACE( 1, ("*** PacketXSend sendto failed: %s\n", strerror( errno)));
    ++s_ulTxErrors;
    return SWS_BASEERR + 1000;
    }

  ++s_ulTxPackets;
  s_ulTxBytes += len;

  return 0;
  }


/*
 * IO ready signal handler
 */
static void io_signal( int iSig)
  {
  for ( ;; )
    {
    fd_set set;
    struct timeval tval;
    SDeque* pQ;

    FD_ZERO( &set);
    FD_SET( s_sockpkt, &set);

    /* Set timeout for select */
    tval.tv_usec = 0;
    tval.tv_sec = 0;

    /* Wait for a receive event or timeout */
    if ( 0 >= select( FD_SETSIZE, &set, NULL, NULL, &tval))
      break; /* Timeout or error */

    /* Get a buffer */
    pQ = DEQUE_FIRST( &s_qRxBufs);
    if ( pQ != &s_qRxBufs)
      {
      SBuffer* pBuffer = LIST2BUFFER( SWS_DequeRemove( pQ));
      struct sockaddr_ll sa;
      socklen_t slen = sizeof sa;
      int len;
      int isPending = 0;

      /* Receive the message */
      sa.sll_family = AF_PACKET;
      len = recvfrom( s_sockpkt, pBuffer->buf, sizeof pBuffer->buf,
        0, (struct sockaddr*)&sa, &slen);
      if ( -1 != len)
        {
        SPacketDriver* pPacket;

        ++s_ulRxPackets;
        s_ulRxBytes += len;

        /* Search for a matching interface
         * NB Packets from all interfaces are received so we may not
         * find a matching interface - this is to be expected.
         */
        for ( pPacket = s_pPacketDrivers; NULL != pPacket;  pPacket = pPacket->pNext)
          {
          if ( pPacket->ifindex == sa.sll_ifindex)
            {
            /* Make a receive indication upcall */
            if ( pPacket->upcalls.pfnRecvInd
              && 0 == (*pPacket->upcalls.pfnRecvInd)(
                pPacket->upcalls.pvContext, pBuffer->buf, len)
            ) {
              isPending = 1;
              }
            break;
            }
          }
        }
      else if ( errno != EAGAIN)
        {
        ++s_ulRxErrors;
        TRACE( 1, ("*** io_signal recvfrom failed: %s\n", strerror( errno)));
        }

      if ( !isPending)
        SWS_DequeAppend( &s_qRxBufs, &pBuffer->list);
      }
    else
      {
      TRACE( 2, ("io_signal out of buffers\n"));
      break;
      }
    }

  if ( SIG_DFL != s_pfnSignal)
    (*s_pfnSignal)( iSig);
  signal( iSig, &io_signal);
  }


/*
 * Receive upcall complete.  Buffer is returned
 * Single threaded
 */
SWS_CB( void, PacketXRecvAck(
  SWS_PTR( void) pvContext,           /* IN: Driver's context */
  SWS_PTR( void) pvData,              /* IN: Buffer */
  size_t len                          /* IN: Buffer size */
)){
  SBuffer* const pBuffer = SWS_CONTAINER( pvData, SBuffer, buf);
  (void)pvContext;
  (void)len;

  TRACE( 9, ("PacketXRecvAck(%p,%p,%u)\n",pvContext,pvData,(unsigned)len));

  /* Buffer is now ready to be re-used */
  SWS_DequeAppend( &s_qRxBufs, &pBuffer->list);
  }


/*
 * Get the adapter's MAC address
 */
SWS_CB( int, PacketXGetMacAddr(        /* Return !0 on error */
  SWS_PTR( void) pvContext,           /* IN: Driver's context */
  SWS_PTR( SWS_EnetMacAddress) pMac   /* OUT: Mac address */
)){
  SPacketDriver* const pPacket = (SPacketDriver*)pvContext;

  TRACE( 9, ("PacketXGetMacAddr(%p,%p)\n",pvContext,pMac));
  ASSERT( NULL != pPacket);
  ASSERT( PACKET_MAGIC == pPacket->uMagic);
  ASSERT( NULL != pMac);

  if ( !pPacket->macAddrIsValid)
    {
    struct ifreq req;

    /* Get hardware address */
    strcpy( req.ifr_name, pPacket->szName);
    if ( -1 == ioctl( s_sockpkt, SIOCGIFHWADDR, &req))
      {
      TRACE( 1, ("*** PacketXGetMacAddr ioctl error %s\n", strerror( errno)));
      return 1;
      }
    if ( ARPHRD_ETHER != req.ifr_hwaddr.sa_family)
      {
      TRACE( 1, ("*** PacketXGetMacAddr unknown arp %d\n", req.ifr_hwaddr.sa_family));
      return 1;
      }

    memcpy( pPacket->mac.addr, req.ifr_hwaddr.sa_data, sizeof pPacket->mac.addr);

    /* Cache the address for later */
    pPacket->macAddrIsValid = 1;
    }

  *pMac = pPacket->mac;

  return 0;
  }


/*
 * Handle private ioctls
 */
SWS_CB( int, PacketXIoctl(             /* Return !0 on error */
  SWS_PTR( void) pvContext,           /* IN: Driver's context */
  unsigned uType,                     /* IN: Type */
  SWS_PTR( void) pvData,              /* I/O: -> data */
  SWS_PTR( size_t) pSize              /* I/O: data size */
)){
  SPacketDriver* const pPacket = (SPacketDriver*)pvContext;

  TRACE( 9, ("PacketXIoctl(%p,%u,%p,%p)\n",pvContext,uType,pvData,pSize));
  ASSERT( NULL != pPacket);
  ASSERT( PACKET_MAGIC == pPacket->uMagic);

  switch ( uType)
    {
  default:
    TRACE( 3, ("PacketXIoctl %u not implemented\n", uType));
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
    }
  }


/*
 * Set receive promiscuity
 */
static int SetPromiscuity(
  SPacketDriver* const pPacket,
  unsigned mode
) {
  struct packet_mreq mr;

  TRACE( 9, ("SetPromiscuity(%p,%u)\n",pPacket,mode));
  ASSERT( NULL != pPacket);
  ASSERT( PACKET_MAGIC == pPacket->uMagic);

  mr.mr_ifindex = pPacket->ifindex;
  mr.mr_type = PACKET_MR_PROMISC;
  mr.mr_alen = ETH_ALEN;
  if ( -1 == setsockopt( s_sockpkt, SOL_PACKET,
      mode ? PACKET_ADD_MEMBERSHIP : PACKET_DROP_MEMBERSHIP,
      &mr, sizeof mr
    )
  ) {
    TRACE( 1, ("*** SetPromiscuity setsockopt error %s\n", strerror( errno)));
    return SWS_EINVAL;
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
  struct packet_mreq mr;
  int i;

  TRACE( 9, ("SetMulticastList(%p,%p,%u)\n",pPacket,pv,size));
  ASSERT( NULL != pPacket);
  ASSERT( PACKET_MAGIC == pPacket->uMagic);

  if ( size > lengthof ( pPacket->mcast) * sizeof pPacket->mcast[ 0])
    {
    TRACE( 1, ("*** SetMulticastList too many entries\n"));
    return SWS_EINVAL;
    }

  /* Drop each member of existing list */
  for ( i = pPacket->iMulticasts - 1; i >= 0; --i)
    {
    mr.mr_ifindex = pPacket->ifindex;
    mr.mr_type = PACKET_MR_MULTICAST;
    mr.mr_alen = sizeof pPacket->mcast[ i];
    memcpy( mr.mr_address, &pPacket->mcast[ i], sizeof pPacket->mcast[ i]);
    if ( -1 == setsockopt( s_sockpkt, SOL_PACKET, PACKET_DROP_MEMBERSHIP, &mr, sizeof mr))
      TRACE( 1, ("*** SetMulticastList setsockopt drop error %s\n", strerror( errno)));
    pPacket->iMulticasts--;
    }
  ASSERT( 0 == pPacket->iMulticasts);

  /* Add each member of new list */
  for ( i = 0; (size_t)i < size; i += sizeof pPacket->mcast[ 0])
    {
    memcpy( &pPacket->mcast[ i], pv, sizeof pPacket->mcast[ i]);
    pv = (SWS_PTR(char))pv + sizeof( SWS_EnetMacAddress);
    mr.mr_ifindex = pPacket->ifindex;
    mr.mr_type = PACKET_MR_MULTICAST;
    mr.mr_alen = sizeof pPacket->mcast[ i];
    memcpy( mr.mr_address, &pPacket->mcast[ i], sizeof pPacket->mcast[ i]);
    if ( -1 == setsockopt( s_sockpkt, SOL_PACKET, PACKET_ADD_MEMBERSHIP, &mr, sizeof mr))
      {
      TRACE( 1, ("*** SetMulticastList setsockopt add error %s\n", strerror( errno)));
      return SWS_EINVAL;
      }
    pPacket->iMulticasts++;
    }

  return 0;
  }
#endif

#endif /* OPT_DATALINK_ETHERNET */
/* End of file */
