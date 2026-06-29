/****************************************************************************
 *
 * File name   : udp.c
 * Function    : UDP transport protocol
 * Project     : SwsSock
 * Authors     : Lawrence Rust
 * Systems     : ANSI C.
 *
 ****************************************************************************
 *
 * Created by Lawrence Rust, Software Systems Consultants
 * lvr@softsystem.co.uk. Tel/Fax +33 5 49 72 79 63
 *
 * 19-Apr-01, LVR, Created.
 * 03-Oct-01, LVR, Fix TRACE warnings for ARM SDT 2.51 .
 * 17-Oct-01, LVR, Added UdpSendInd to handle SWS_ENOBUFS in UdpSendTo
 * 07-Feb-02, LVR, UdpSendTo binds the SAP IP address if unset
 * 08-Nov-02, LVR, UdpIcmpInd sets TxReady event for ICMP unreachable indications
 * 24-Nov-02, LVR, UdpSendTo will use an adapter with an unset IP address if the
 *                 SWS_MSG_DONTROUTE flag is set (for DHCP).
 * 28-Dec-02, LVR, Added multicast.
 * 03-Oct-03, LVR, Added config for lo/hi auto port
 * 12-Oct-04, LVR, Fixed re-initialisation.by initing all static data
 * 11-Jun-05, LVR, UdpConnect selects localHost if none bound.
 * 25-Jun-05, LVR, GetSock/PeerName return error if local/remote host unset
 *
 * Comments:
 * --------
 * This file implements the UDP transport
 *
 ****************************************************************************
 */

/* Compilation options */
#include "config.h"

/* Exports */
#include "sws_udp.h"

#if OPT_PROTO_UDP
/* Imports */
#include <stddef.h>
#include <string.h>

#include "sws_buf.h"
#include "sws_tprt.h"
#include "sws_ip.h"
#include "sws_icmp.h"
#include "sws_timr.h"
#include "sws_sock.h"  /* SOCK_DGRAM */
#include "sws_gef.h"

#include "platform.h"

#include "debug.h"


/*
 * Macros & Constants
 */


/*
 * Local data types
 */
/* Data stored with each received datagram */
typedef struct SUdpMessage
  {
  SWS_BufferH hBuffer;
  ui32 remoteHost;
  ui16 remotePort;
  ui16 header;                        /* Header length */
  } SUdpMessage;

#if OPT_IP_MULTICAST
/* Multicast group */
typedef struct SGroupMember
  {
  ui32 addr, src;
  } SGroupMember;
#endif

/* A UDP Service Access Point */
typedef struct SUdpSap
  {
  struct SUdpSap* pNext;              /* List of bound SAP's */

  ui32 localHost;
  ui32 remoteHost;
  ui16 localPort;
  ui16 remotePort;

  /* Options */
  ui16 flags;
  int noChecksum;
  SWS_IpOptions ipOptions;
  size_t SndBuf;                      /* Size of send buffer */
  size_t RcvBuf;                      /* Size of receive buffer */

#if OPT_IP_MULTICAST
  size_t groups;                      /* No. active multicast groups */
  SGroupMember group[ SWS_IP_MAX_MEMBERSHIPS];
#endif

  /* Receive ring buffer */
  Atomic_t read, write, count;
  SWS_VARRAY( SUdpMessage, message);  /* VARIABLE LENGTH */
  } SUdpSap;

/* Data used by UdpBindSync to bind a SAP */
typedef struct SBindInfo
  {
  SUdpSap* pUdp;
  unsigned port;
  ui32 host;
  } SBindInfo;


/*
 * Prototypes
 */
static int UdpBindSap( SUdpSap*, ui32, unsigned);
static SUdpSap* UdpFindSap( unsigned, unsigned, ui32 host, ui32 remote);

static SWS_FTransportOpen UdpOpen;
static SWS_FTransportClose UdpClose;
static SWS_FTransportBind UdpBind;
static SWS_FTransportListen UdpListen;
static SWS_FTransportConnect UdpConnect;
static SWS_FTransportShutdown UdpShutdown;
static SWS_FTransportSendTo UdpSendTo;
static SWS_FTransportRecvFrom UdpRecvFrom;
static SWS_FTransportGetInfo UdpGetInfo;
static SWS_FTransportSetInfo UdpSetInfo;
static SWS_FTransportCleanup UdpCleanup;
static SWS_FTransportGetPeerName UdpGetPeerName;
static SWS_FTransportGetSockName UdpGetSockName;

static SWS_FIpRecvInd UdpRecvInd;
#if OPT_PROTO_ICMP
static SWS_FIpIcmpInd UdpIcmpInd;
#endif
static SWS_FIpSendInd UdpSendInd;

static FPlatformCallback UdpBindSync;
static FPlatformCallback UdpUnbindSync;

static void UdpRecvPacket( SUdpSap*, SWS_BufferH, size_t, unsigned, ui32, ui16);

#if OPT_IP_MULTICAST
static int UdpGroupJoin( SUdpSap* const, ui32, ui32);
static int UdpGroupLeave( SUdpSap* const, ui32, ui32);
static int UdpGroupFind( SUdpSap* const, ui32 , ui32);
#endif


/*
 * Module data
 */
/* Configuration */
static size_t s_uUdpSaps;
static size_t s_uUdpBuffers;
static unsigned s_uChecksum;
#if !OPT_UDP_DISCARD_NOPORT
static int s_ignore_noport;           /* !0 to silently drop packets for closed ports */
#endif
static ui16 s_loPort, s_hiPort;       /* Min/max auto port */

/* Stats */
static Atomic_t s_aTxDatagrams, s_aTxBytes;
static Atomic_t s_aRxDatagrams, s_aRxBytes;
static Atomic_t s_aRxErrors;          /* Datagrams received with errors */
static Atomic_t s_aRxNoPort;          /* Datagrams received with no matching port */
static Atomic_t s_aRxOverflow;        /* Rx Buffer overflows */
#if OPT_PROTO_ICMP
static Atomic_t s_aIcmpErrors;        /* ICMP errors received */
#endif

/* Bound SAP's */
static SUdpSap* volatile s_pBoundUdps; /* List of bound SAPs */
static ui16 s_port;

/* UDP transport registration data */
static const SWS_TransportDesc s_transportUdp = {
  sizeof( s_transportUdp),
  SWS_TRANSPORT_VERSION,
  SWS_AF_INET,
  SWS_SOCK_DGRAM,
  SWS_IPPROTO_UDP,
  0,
  "udp",
  &UdpOpen,
  &UdpClose,
  &UdpBind,
  &UdpListen,
  &UdpConnect,
  &UdpShutdown,
  &UdpSendTo,
  &UdpRecvFrom,
  &UdpGetInfo,
  &UdpSetInfo,
  &UdpCleanup,
  &UdpGetSockName,
  &UdpGetPeerName
};
static int s_bRegTransport;

/* UDP protocol registration data */
static const SWS_IpProtocolDesc s_protocolUdp = {
  sizeof( s_protocolUdp),
  SWS_PROTOCOL_VERSION,
  SWS_UDP_PROTOCOL,
  0,
  &UdpRecvInd,
#if OPT_PROTO_ICMP
  &UdpIcmpInd,
#else
  0,
#endif
  &UdpSendInd
};
static int s_bRegProtocol;


/*
 * Initialise this module
 * Single threaded
 */
int SWS_UdpInit( void)                /* Returns !0 on error */
  {
  int iErr;

  TRACE( 9, ("SWS_UdpInit()\n"));
  ASSERT( PlatformIsCritical());
  ASSERT( NULL == s_pBoundUdps);
  ASSERT( !s_bRegTransport);
  ASSERT( !s_bRegProtocol);

  /* Init static data */
  s_pBoundUdps = NULL;
  s_port = 0;

  s_uUdpSaps = (size_t)PlatformPropertyGetUint( PKEY_UDP_SOCKETS, OPT_UDP_SOCKETS );
  if ( 0 == s_uUdpSaps)
    return 0;

  s_uUdpBuffers = (size_t)(1 + PlatformPropertyGetUint( PKEY_UDP_BUFFERS, OPT_UDP_BUFFERS ));
  s_uChecksum = (unsigned)PlatformPropertyGetUint( PKEY_UDP_CHECKSUM, OPT_UDP_CHECKSUM );
#if !OPT_UDP_DISCARD_NOPORT
  s_ignore_noport = (int)PlatformPropertyGetUint( PKEY_UDP_NOPORT, OPT_UDP_DISCARD_NOPORT );
#endif
  s_port = s_loPort = (ui16)PlatformPropertyGetUint( PKEY_UDP_LOPORT, OPT_UDP_MIN_PORT );
  s_hiPort = (ui16)PlatformPropertyGetUint( PKEY_UDP_HIPORT, OPT_UDP_MAX_PORT );
  if ( s_loPort > s_hiPort)
    return 0;

  /* Register the UDP protocol */
  iErr = SWS_IpRegisterProtocol( &s_protocolUdp, NULL);
  if ( !iErr)
    {
    s_bRegProtocol = 1;

    /* Register the UDP transport */
    iErr = SWS_TransportRegister(
      &s_transportUdp,
      NULL,
      s_uUdpSaps,
      sizeof( SUdpSap) + s_uUdpBuffers * sizeof( SUdpMessage)
    );
    if ( !iErr)
      s_bRegTransport = 1;
    else
      {
      VERIFY( !SWS_IpDeregisterProtocol( &s_protocolUdp, NULL));
      s_bRegProtocol = 0;
      }
    }

  return iErr;
  }


/*
 * Finalise this module
 * Single threaded
 */
void SWS_UdpCleanup( void)
  {
  TRACE( 9, ("SWS_UdpCleanup()\n"));
  ASSERT( PlatformIsCritical());

  ASSERT( !s_bRegTransport);
  ASSERT( !s_bRegProtocol);

#if OPT_PROTO_ICMP
# define ICMP_ERRORS AtomicRead( &s_aIcmpErrors)
#else
# define ICMP_ERRORS 0
#endif

  TRACE( 1, ("UDP: TxDatagrams:%u TxBytes:%u IcmpErrors:%u"
    " RxDatagrams:%u RxBytes:%u RxErrors:%u RxOverflows:%u RxNoPort:%u\n",
    AtomicRead( &s_aTxDatagrams), AtomicRead( &s_aTxBytes),
    ICMP_ERRORS, AtomicRead( &s_aRxDatagrams),
    AtomicRead( &s_aRxBytes), AtomicRead( &s_aRxErrors),
    AtomicRead( &s_aRxOverflow), AtomicRead( &s_aRxNoPort)
  ));
  }


/****************************************************************************
 * Downcalls from SwsSock
 ****************************************************************************/

/*
 * Transport layer is closing
 * Single threaded
 */
static SWS_CB( void, UdpCleanup( SWS_PTR( void) pvContext))
  {
  (void)pvContext;

  TRACE( 9, ("UdpCleanup(%p)\n", pvContext));
  ASSERT( PlatformIsCritical());

  while ( NULL != s_pBoundUdps)
    UdpClose( s_pBoundUdps, 1);

  s_bRegTransport = 0;

  if ( s_bRegProtocol)
    {
    VERIFY( !SWS_IpDeregisterProtocol( &s_protocolUdp, NULL));
    s_bRegProtocol = 0;
    }
  }


/*
 * Open a Service Access Point
 */
SWS_CB( int, UdpOpen(
  SWS_PTR( void) pvContext,
  SWS_PTR( void) pvSap,               /* IN: -> SAP data */
  int type,                           /* IN: socket type */
  int protocol                        /* IN: protocol */
)){
  SUdpSap* const pUdp = (SUdpSap*)pvSap;
  (void)pvContext;

  TRACE( 9, ("UdpOpen(%p,%p,%d,%d)\n", pvContext, pvSap, type, protocol));
  ASSERT( NULL != pUdp);
  (void)type;
  (void)protocol;

  /* Initialise the SAP */
  memset( pUdp, 0, sizeof( *pUdp));
  pUdp->pNext = NULL;
  pUdp->localHost = SWS_INADDR_ANY;
  pUdp->remoteHost = SWS_INADDR_ANY;
  pUdp->RcvBuf = pUdp->SndBuf = (size_t)SWS_BufferAvailable();
#if 0
  pUdp->localPort = 0;
  pUdp->remotePort = 0;
  pUdp->flags = 0;
  pUdp->noChecksum = 0;
#if OPT_IP_MULTICAST
  pUdp->groups = 0;
#endif
  AtomicWrite( &pUdp->read, 0);
  AtomicWrite( &pUdp->write, 0);
  AtomicWrite( &pUdp->count, 0);
#endif
  pUdp->ipOptions.flags.multicast_loop = SWS_IP_DEFAULT_MULTICAST_LOOP;
  pUdp->ipOptions.multicast_ttl = SWS_IP_DEFAULT_MULTICAST_TTL;

  SWS_TransportEventSet( pUdp, SWS_kTransportEventTxReady, 1);

  return 0;
  }


/*
 * Close a Service Access Point
 */
SWS_CB( int, UdpClose(
  SWS_PTR( void) pvSap,
  int reset
)){
  SUdpSap* const pUdp = (SUdpSap*)pvSap;
  int read;
  (void)reset;

  TRACE( 9, ("UdpClose(%p,%d)\n", pvSap,reset));
  ASSERT( NULL != pUdp);

#if OPT_IP_MULTICAST
  while ( pUdp->groups > 0)
    {
    SGroupMember* pGroup = &pUdp->group[ pUdp->groups - 1];
    UdpGroupLeave( pUdp, pGroup->addr, pGroup->src);
    }
#endif

  /* Unlink from bound list */
  PlatformSynchronise( &UdpUnbindSync, pUdp);
  pUdp->pNext = NULL;

  /* Discard buffered data */
  read = AtomicRead( &pUdp->read);
  while ( read != AtomicRead( &pUdp->write))
    {
    SWS_BufferRelease( pUdp->message[ read].hBuffer );
    if ( ++read >= (int)s_uUdpBuffers)
      read = 0;

    AtomicWrite( &pUdp->read, read);
    }

  return 0;
  }

SWS_CB( int, UdpUnbindSync( SWS_PTR( void) pv))
  {
  SUdpSap* pPrev;

  ASSERT( PlatformIsCritical());

  pPrev = s_pBoundUdps;
  if ( pPrev == pv)
    {
    s_pBoundUdps = ((SUdpSap*)pv)->pNext;
    }
  else
    {
    for ( ; NULL != pPrev; pPrev = pPrev->pNext)
      {
      if ( pPrev->pNext == pv)
        {
        pPrev->pNext = ((SUdpSap*)pv)->pNext;
        break;
        }
      }
    }

  return 0;
  }


/*
 * Associate a Service Access Point with a local address & port
 */
SWS_CB( int, UdpBind(
  SWS_PTR( void) pvSap,               /* IN: IN: -> SAP */
  SWS_PTR( const struct SWS_sockaddr) pName,
  int namelen
)){
  SUdpSap* const pUdp = (SUdpSap*)pvSap;
  ui32 localHost;
  unsigned localPort;

  TRACE( 9, ("UdpBind(%p,%p,%d)\n", pvSap, pName, namelen));
  ASSERT( NULL != pUdp);
  ASSERT( NULL != pName);
  ASSERT( 0 == pUdp->localPort); /* sws_sock prevents duplicate binds */

  if ( SWS_AF_INET != pName->sa_family)
    {
    TRACE( 1, ("*** UdpBind() unsupported address type\n"));
    return SWS_EAFNOSUPPORT;
    }

  if ( namelen != sizeof( struct SWS_sockaddr_in))
    {
    TRACE( 1, ("*** UdpBind() namelen %d incorrect\n", namelen));
    return SWS_EINVAL;
    }

  /* Ensure local address is valid */
  NetRead32( localHost, &((SWS_PTR(const struct SWS_sockaddr_in))pName)->sin_addr.SWS_s_addr);
  if ( 0 == SWS_IpGetMaxSizes(
      localHost, localHost, pUdp->ipOptions.tos, NULL)
  ) {
    TRACE( 1, ("*** UdpBind() invalid local host\n"));
    return SWS_EADDRNOTAVAIL;
    }

  NetRead16( localPort, &((SWS_PTR(const struct SWS_sockaddr_in))pName)->sin_port);
  return UdpBindSap( pUdp, localHost, localPort);
  }


/*
 * Wait for an incoming connection request
 */
SWS_CB( int, UdpListen( SWS_PTR(
  void) pvSap
)){
  TRACE( 9, ("UdpListen(%p)\n", pvSap));
  ASSERT( NULL != pvSap);
  (void)pvSap;

  /* Not relevant to UDP */
  return SWS_EOPNOTSUPP;
  }


/*
 * Associate a Service Access Point with a remote address & port
 */
SWS_CB( int, UdpConnect(
  SWS_PTR( void) pvSap,               /* IN: IN: -> SAP */
  SWS_PTR( const struct SWS_sockaddr) pName,
  int namelen
)){
  ui32 remoteHost;
  SUdpSap* const pUdp = (SUdpSap*)pvSap;

  TRACE( 9, ("UdpConnect(%p,%p,%d)\n", pvSap, pName, namelen));
  ASSERT( NULL != pUdp);
  ASSERT( NULL != pName);

  if ( namelen != sizeof( struct SWS_sockaddr_in))
    {
    TRACE( 1, ("*** UdpConnect() namelen %d incorrect\n", namelen));
    return SWS_EINVAL;
    }

  if ( SWS_AF_INET != pName->sa_family)
    {
    TRACE( 1, ("*** UdpConnect() incorrect address type\n"));
    return SWS_EINVAL;
    }

  NetRead32(
    remoteHost,
    &((SWS_PTR(const struct SWS_sockaddr_in))pName)->sin_addr.SWS_s_addr
  );
  if ( SWS_INADDR_ANY != remoteHost)
    {
    ui32 localHost = pUdp->localHost;
    if ( SWS_INADDR_ANY == localHost)
      {
      /* Select an adapter */
      unsigned tos = pUdp->ipOptions.tos;
      if ( (pUdp->flags & SWS_SO_DONTROUTE))
        tos |= 0x8000; /* Include interfaces with unset IP address */

      localHost = SWS_IpGetSrc( remoteHost, tos);
      if ( SWS_INADDR_NONE == localHost)
        {
        TRACE( 1, ("*** UdpConnect() %08lx not reachable\n", remoteHost));
        return SWS_ENETUNREACH;
        }

      /* Implicit bind */
      pUdp->localHost = localHost;
      }
    /* Ensure address is reachable */
    else if ( 0 == SWS_IpGetMaxSizes(
        localHost, remoteHost, pUdp->ipOptions.tos, NULL)
    ) {
      TRACE( 1, ("*** UdpConnect() %08lx not reachable\n", remoteHost));
      return SWS_ENETUNREACH;
      }
    }

  /* Set remote port & address */
  pUdp->remoteHost = remoteHost;
  NetRead16( pUdp->remotePort, &((SWS_PTR(const struct SWS_sockaddr_in))pName)->sin_port);

  return 0;
  }


/*
 * Break an existing connection
 */
SWS_CB( int, UdpShutdown( SWS_PTR( void) pvSap, int how ))
  {
  (void)pvSap;
  (void)how;

  TRACE( 9, ("UdpDisconnect(%p,%d)\n", pvSap,how));
  ASSERT( NULL != pvSap);

  return 0;
  }


/*
 * Send data to remote connection
 */
SWS_CB( int, UdpSendTo(
  SWS_PTR( void) pvSap,               /* IN: IN: -> SAP */
  SWS_PTR( const void) pBuf,          /* IN: -> data to send */
  SWS_PTR( unsigned) pLen,            /* I/O: data len */
  unsigned flags,                     /* IN: flags, SWS_MSG_xxx */
  SWS_PTR( const struct SWS_sockaddr) pName,/* IN: Optional name */
  int namelen                         /* IN: namelen */
)){
  SUdpSap* const pUdp = (SUdpSap*)pvSap;
  SWS_BufferH hBuffer;
  SWS_PTR( void) pvHdr;
  int err;
  unsigned len;
  SWS_size_t uLen;
  ui32 remoteHost, localHost;
  unsigned remotePort;

  TRACE( 9, ("UdpSendTo(%p,%p,%p,%u,%p,%d)\n",
    pvSap, pBuf, pLen, flags, pName, namelen));
  ASSERT( NULL != pvSap);
  ASSERT( NULL != pLen);
  ASSERT( NULL != pBuf || 0 == *pLen);

  len = *pLen;
  *pLen = 0;

  /* Validate params */
  if ( len > SWS_IP_MAXLEN - SWS_IPHDR_MINLEN - SWS_UDP_HDR)
    {
    TRACE( 1, ("*** UdpSendTo() Length too big\n"));
    return SWS_EMSGSIZE;
    }

  /* NB ignore SWS_MSG_DONTROUTE */
  if ( flags & ~SWS_MSG_DONTROUTE)
    {
    TRACE( 1, ("*** UdpSendTo() unsupported flags\n"));
    return SWS_EINVAL;
    }

  if ( NULL != pName)
    {
    /* Check remote port & address */
    if ( SWS_AF_INET != pName->sa_family)
      {
      TRACE( 1, ("*** UdpSendTo() unsupported address type\n"));
      return SWS_EAFNOSUPPORT;
      }

    if ( namelen != sizeof( struct SWS_sockaddr_in))
      {
      TRACE( 1, ("*** UdpSendTo() namelen %d incorrect\n", namelen));
      return SWS_EINVAL;
      }

    NetRead16( remotePort, &((SWS_PTR(const struct SWS_sockaddr_in))pName)->sin_port);
    NetRead32( remoteHost, &((SWS_PTR(const struct SWS_sockaddr_in))pName)->sin_addr.SWS_s_addr);
    }
  else
    {
    /* Use previous connection */
    remotePort = pUdp->remotePort;
    remoteHost = pUdp->remoteHost;
    }

  if ( 0 == remotePort || SWS_INADDR_ANY == remoteHost)
    {
    TRACE( 1, ("*** UdpSendTo() invalid remote address or port\n"));
    return SWS_EADDRNOTAVAIL;
    }

  if ( SWS_INADDR_BROADCAST == remoteHost && !(pUdp->flags & SWS_SO_BROADCAST) )
    {
    TRACE( 1, ("*** UdpSendTo() broadcast not enabled\n"));
    return SWS_EACCES;
    }

  if ( len > pUdp->SndBuf)
    {
    TRACE( 1, ("*** UdpSendTo() datagram bigger than allocated buffer\n"));
    return SWS_ENOBUFS;
    }

  /* Bind this SAP to a local port */
  if ( 0 == pUdp->localPort)
    {
    err = UdpBindSap( pUdp, SWS_INADDR_ANY, 0);
    if ( err)
      return err;
    }

  localHost = pUdp->localHost;
  if ( SWS_INADDR_ANY == localHost)
    {
    /* Select an adapter */
    unsigned tos = pUdp->ipOptions.tos;
    if ( (flags & SWS_MSG_DONTROUTE) || (pUdp->flags & SWS_SO_DONTROUTE))
      tos |= 0x8000; /* Include interfaces with unset IP address */

    localHost = SWS_IpGetSrc( remoteHost, tos);
    if ( SWS_INADDR_NONE == localHost)
      {
      TRACE( 1, ("*** UdpSendTo() %08lx not reachable\n", remoteHost));
      return SWS_ENETUNREACH;
      }

    /* Implicit bind */
    pUdp->localHost = localHost;
    }

  /* Wait for buffer space */
  if ( SWS_BufferTotal() < len + 0x2000)
    {
    TRACE( 1, ("*** UdpSendTo() Too big\n"));
    return SWS_ENOBUFS;
    }
  while ( SWS_BufferAvailable() < len + 0x2000)
    PlatformIdle();

  /* Get a 'shadow' buffer for the data */
  hBuffer = SWS_BufferGetShadow(
    len,
    SWS_kBufferPrefix,
    (SWS_PTR(void))pBuf
  );
  if ( NULL == hBuffer)
    {
    TRACE( 1, ("*** UdpSendTo() Out of buffers\n"));
    return SWS_ENOBUFS;
    }

  /* Append UDP header */
  if ( SWS_BufferAppendFront( hBuffer, SWS_UDP_HDR))
    {
    TRACE( 1, ("*** UdpSendTo() Out of buffers\n"));
    VERIFY( !SWS_BufferRelease( hBuffer));
    return SWS_ENOBUFS;
    }

  /* Get -> UDP header */
  pvHdr = SWS_BufferGetFragment( hBuffer, 0, &uLen);
  ASSERT( NULL != pvHdr);
  ASSERT( uLen >= SWS_UDP_HDR);

  /* Fill in the UDP header */
  SWS_POKE16( pvHdr, SWS_UDP_SRC, pUdp->localPort);
  SWS_POKE16( pvHdr, SWS_UDP_DST, (ui16)remotePort);
  len += SWS_UDP_HDR;
  SWS_POKE16( pvHdr, SWS_UDP_LEN, (ui16)len);
  SWS_POKE16( pvHdr, SWS_UDP_SUM, 0);

  /* UDP checksum */
  if ( !pUdp->noChecksum)
    {
    ui32 uHsum = SWS_IpPseudoHdr( localHost, remoteHost, SWS_UDP_PROTOCOL, len);
    unsigned uSum = SWS_IpChecksum( hBuffer, 0, len, uHsum);
    if ( 0 == uSum)
      uSum = 0xffff;

    SWS_POKECSUM( pvHdr, SWS_UDP_SUM, uSum);
    /*ASSERT( !SWS_IpChecksum( hBuffer, 0, len, uHsum));*/
    }

  /* Repeat until sent */
  for ( ;;)
    {
    /* Wait until Tx resources available */
    err = SWS_TransportEventWait( pUdp, SWS_kTransportEventTxReady);
    if ( 1 != err)
      {
      VERIFY( !SWS_BufferReleaseShadow( hBuffer));
      return err;
      }

    err = SWS_IpSend(
      localHost,
      remoteHost,
      SWS_UDP_PROTOCOL,
      hBuffer,
      SWS_IP_IDENT_DEFAULT,
      &pUdp->ipOptions
    );
    /* Out of Tx resources? */
    if ( err >= 0)
      {
      /* No */
      SWS_TransportEventSet( pUdp, SWS_kTransportEventTxReady, 1);
      break;
      }
    }

  AtomicIncrement( &s_aTxDatagrams);
  AtomicAdd( &s_aTxBytes, (int)len);
  *pLen = len - SWS_UDP_HDR;

  /* Release buffer but copy user data if still referenced */
  SWS_BufferReleaseShadow( hBuffer);

  return err;
  }


/*
 * Get data on a connection
 */
SWS_CB( int, UdpRecvFrom(
  SWS_PTR( void) pvSap,               /* IN: IN: -> SAP */
  SWS_PTR( void) pBuf,                /* OUT: Data */
  SWS_PTR( unsigned) pLen,            /* I/O: buffer len */
  unsigned flags,                     /* IN: flags, SWS_MSG_xxx */
  SWS_PTR( struct SWS_sockaddr) pName, /* OUT: Optional name */
  SWS_PTR( int) pNamelen              /* I/O: namelen */
)){
  SUdpSap* const pUdp = (SUdpSap*)pvSap;
  SUdpMessage msg;
  int read;
  int iRet;
  unsigned len;
  SWS_size_t uLen;

  TRACE( 9, ("UdpRecvFrom(%p,%p,%p,%u,%p,%p)\n",
    pvSap,pBuf,pLen,flags,pName,pNamelen));
  ASSERT( NULL != pvSap);
  ASSERT( NULL != pLen);
  ASSERT( NULL != pBuf || 0 == *pLen);

  len = *pLen;
  *pLen = 0;

  if ( NULL != pName)
    {
    if ( NULL == pNamelen)
      {
      TRACE( 1, ("*** UdpRecvFrom() NULL namelen\n"));
      return SWS_EFAULT;
      }

    if ( *pNamelen < (int)sizeof( struct SWS_sockaddr_in))
      {
      TRACE( 1, ("*** UdpRecvFrom() namelen %d too small\n", *pNamelen));
      return SWS_EINVAL;
      }
    }

  if ( ~SWS_MSG_PEEK & flags)
    {
    TRACE( 1, ("*** UdpRecvFrom() unsupported flags\n"));
    return SWS_EINVAL;
    }

  /* Check for Rx data */
  if ( SWS_MSG_PEEK & flags)
    {
    if ( (read = AtomicRead( &pUdp->read)) == AtomicRead( &pUdp->write))
      return 0;
    }
  else do
    {
    /* Wait until Rx data available */
    iRet = SWS_TransportEventWait( pUdp, SWS_kTransportEventRxReady);
    if ( 1 != iRet)
      return iRet;
    }
  while ( (read = AtomicRead( &pUdp->read)) == AtomicRead( &pUdp->write));

  /* Get the oldest message */
  msg = pUdp->message[ read];
  if ( !(SWS_MSG_PEEK & flags))
    {
    /* Update the read ptr */
    if ( ++read >= (int)s_uUdpBuffers)
      read = 0;
    AtomicWrite( &pUdp->read, read);
    }

  /* Calc amount of data to receive */
  uLen = SWS_BufferSize( msg.hBuffer);
  ASSERT( uLen >= msg.header);
  uLen -= msg.header;
  if ( len >= uLen)
    {
    /* Recv all datagram */
    *pLen = (unsigned)uLen;
    iRet = 0;
    }
  else if ( SWS_MSG_PEEK & flags )
    {
    /* Just peeking - return length available */
    *pLen = (unsigned)uLen;
    uLen = (SWS_size_t)len;
    iRet = 0;
    }
  else
    {
    /* Data lost */
    TRACE( 1, ("*** UdpRecvFrom() %u byte buffer too small (wanted %u)\n", len, uLen));
    uLen = (SWS_size_t)len;
    iRet = SWS_EMSGSIZE;
    }
  SWS_BufferCopyToMemory( pBuf, msg.hBuffer, msg.header, uLen);
  ASSERT( (unsigned)AtomicRead( &pUdp->count) >= uLen);
  AtomicAdd( &pUdp->count, -(int)uLen);

  if ( !(SWS_MSG_PEEK & flags) )
    SWS_BufferRelease( msg.hBuffer );

  if ( AtomicRead( &pUdp->read) != AtomicRead( &pUdp->write) )
    {
    /* There's still data remaining in the Rx queue */
    SWS_TransportEventSet( pUdp, SWS_kTransportEventRxReady, 1);
    }

  if ( NULL != pName)
    {
    /* Return source of datagram */
    memset( pName, 0, sizeof(struct SWS_sockaddr_in));
    pName->sa_family = SWS_AF_INET;
    NetWrite16( &((SWS_PTR(struct SWS_sockaddr_in))pName)->sin_port, msg.remotePort );
    NetWrite32( &((SWS_PTR(struct SWS_sockaddr_in))pName)->sin_addr.SWS_s_addr, msg.remoteHost);
    *pNamelen = sizeof(struct SWS_sockaddr_in);
    }

  return iRet;
  }


/*
 * Get info
 */
SWS_CB( int, UdpGetInfo(
  SWS_PTR( void) pvSap,               /* IN: -> SAP data */
  unsigned level,                     /* IN: level SWS_SOL_xxx */
  unsigned opt,                       /* IN: option */
  SWS_PTR( void) pVal,                /* OUT: -> value */
  SWS_PTR( int) pLen                  /* I/O: length of value */
)){
  SUdpSap* const pUdp = (SUdpSap*)pvSap;
  size_t size;
  int read;

  TRACE( 9, ("UdpGetInfo(%p,%u,%u,%p,%p)\n", pvSap,level,opt,pVal,pLen));
  ASSERT( NULL != pUdp);
  ASSERT( NULL != pVal);
  ASSERT( NULL != pLen);
  ASSERT( *pLen > 0);

  switch ( level)
    {
  case SWS_SOL_SOCKET:
    switch ( opt)
      {
    case SWS_SO_REUSEADDR:
    case SWS_SO_DONTROUTE:
    case SWS_SO_BROADCAST:
    case SWS_SO_USELOOPBACK:
      if ( *pLen != sizeof(int))
        return SWS_EFAULT;

      *(SWS_PTR(int))pVal = pUdp->flags & opt;
      break;

    case SWS_SO_MAX_MSG_SIZE:
      if ( *pLen != sizeof(unsigned))
        return SWS_EFAULT;

      /* Get max Tx datagram */
      size = SWS_IpGetMaxSizes(
        pUdp->localHost,
        SWS_INADDR_ANY != pUdp->remoteHost ? pUdp->remoteHost : pUdp->localHost,
        pUdp->ipOptions.tos, NULL
      );

      /* Deduct UDP header */
      if ( size >= SWS_UDP_HDR)
        size -= SWS_UDP_HDR;

      *(SWS_PTR(unsigned))pVal = (unsigned)size;
      break;

    case SWS_SO_RCVBUF:
      if ( *pLen != sizeof(unsigned))
        return SWS_EFAULT;

      *(SWS_PTR(unsigned))pVal = (unsigned)pUdp->RcvBuf;
      break;

    case SWS_SO_SNDBUF:
      if ( *pLen != sizeof(unsigned))
        return SWS_EFAULT;

      *(SWS_PTR(unsigned))pVal = (unsigned)pUdp->SndBuf;
      break;

    default:
      return SWS_EOPNOTSUPP;
      }
    break;

  case SWS_SOL_IP:
    if ( SWS_IP_HDRINCL == opt)
      return SWS_EOPNOTSUPP;

    return SWS_IpGetOption( &pUdp->ipOptions, opt, pVal, pLen);

  case SWS_SOL_UDP:
    switch ( opt)
      {
    case SWS_UDP_NOCHECKSUM:
      if ( *pLen < (int)sizeof(int))
        return SWS_EFAULT;

      *pLen = (int)sizeof(int);
      *(SWS_PTR(int))pVal = pUdp->noChecksum;
      break;

    default:
      return SWS_ENOPROTOOPT;
      }
    break;

  case SWS_SOL_IOCTL:
    switch ( opt)
      {
    default:
      return SWS_EOPNOTSUPP;

    case SWS_IOCTL_FIONREAD:
      if ( *pLen != sizeof(SWS_u_long))
        return SWS_EFAULT;

      /* Get size of first datagram */
      read = AtomicRead( &pUdp->read);
      if ( AtomicRead( &pUdp->write) == read)
        *(SWS_PTR(SWS_u_long))pVal = 0;
      else
        *(SWS_PTR(SWS_u_long))pVal = (SWS_u_long)SWS_BufferSize( pUdp->message[ read].hBuffer);
      break;
      }
    break;

  default:
    return SWS_EINVAL;
    }

  return 0;
  }


/*
 * Set info
 */
SWS_CB( int, UdpSetInfo(
  SWS_PTR( void) pvSap,               /* IN: -> SAP data */
  unsigned level,                     /* IN: level SWS_SOL_xxx */
  unsigned opt,                       /* IN: option */
  SWS_PTR( const void) pVal,          /* IN: -> value */
  int len                             /* IN: length of value */
)){
  SUdpSap* const pUdp = (SUdpSap*)pvSap;
#if OPT_IP_MULTICAST
  SWS_PTR( const struct SWS_ip_mreq) pMreq;
#endif
  size_t size;

  TRACE( 9, ("UdpSetInfo(%p,%u,%u,%p,%d)\n", pvSap,level,opt,pVal,len));
  ASSERT( NULL != pUdp);
  ASSERT( NULL != pVal);
  ASSERT( len > 0);

  switch ( level)
    {
  case SWS_SOL_SOCKET:
    switch ( opt)
      {
    case SWS_SO_REUSEADDR:
    case SWS_SO_DONTROUTE:
    case SWS_SO_BROADCAST:
    case SWS_SO_USELOOPBACK:
      if ( len != sizeof(int))
        return SWS_EFAULT;

      if ( *(SWS_PTR(const int))pVal)
        pUdp->flags |= opt;
      else
        pUdp->flags &= ~opt;
      break;

    case SWS_SO_SNDBUF:
      if ( len != sizeof(int))
        return SWS_EFAULT;

      size = (size_t)*(SWS_PTR(const unsigned int))pVal;
      if ( size < SWS_IP_MIN_MTU)
        size = SWS_IP_MIN_MTU;
#if SWS_SIZE_MAX > SWS_IP_MAXLEN
      else if ( size > SWS_IP_MAXLEN)
        size = SWS_IP_MAXLEN;
#endif

      pUdp->SndBuf = size;
      break;

    case SWS_SO_RCVBUF:
      /* Resize the receive buffer */
      if ( len != sizeof(int))
        return SWS_EFAULT;

      size = (size_t)*(SWS_PTR(const unsigned int))pVal;
      if ( size < SWS_IP_MIN_MTU)
        size = SWS_IP_MIN_MTU;
#if SWS_SIZE_MAX > SWS_IP_MAXLEN
      else if ( size > SWS_IP_MAXLEN)
        size = SWS_IP_MAXLEN;
#endif

      pUdp->RcvBuf = size;
      break;

    default:
      return SWS_EOPNOTSUPP;
      }
    break;

  case SWS_SOL_IP:
    switch ( opt)
      {
    case SWS_IP_HDRINCL:
      return SWS_EOPNOTSUPP;

#if OPT_IP_MULTICAST
    case SWS_IP_ADD_MEMBERSHIP:
      pMreq = (SWS_PTR(const struct SWS_ip_mreq))pVal;
      return UdpGroupJoin(
        pUdp,
        SWS_ntohl( pMreq->imr_multiaddr.SWS_s_addr),
        SWS_ntohl( pMreq->imr_interface.SWS_s_addr)
      );

    case SWS_IP_DROP_MEMBERSHIP:
      pMreq = (SWS_PTR(const struct SWS_ip_mreq))pVal;
      return UdpGroupLeave(
        pUdp,
        SWS_ntohl( pMreq->imr_multiaddr.SWS_s_addr),
        SWS_ntohl( pMreq->imr_interface.SWS_s_addr)
      );
#endif

    default:
      break;
      }
    return SWS_IpSetOption( &pUdp->ipOptions, opt, pVal, len);

  case SWS_SOL_UDP:
    switch ( opt)
      {
    case SWS_UDP_NOCHECKSUM:
      if ( len != sizeof(int))
        return SWS_EFAULT;

      pUdp->noChecksum = 0 != *(SWS_PTR(const int))pVal;
      break;

    default:
      return SWS_ENOPROTOOPT;
      }
    break;

  default:
    return SWS_EINVAL;
    }

  return 0;
  }


/*
 * Get the address of the local host
 */
static SWS_CB( int, UdpGetSockName(
  SWS_PTR( void) pvSap,               /* IN: -> SAP data */
  SWS_PTR( struct SWS_sockaddr) pName,/* OUT: name */
  SWS_PTR( int) pLen                  /* IN/OUT: namelen */
)){
  SUdpSap* const pUdp = (SUdpSap*)pvSap;

  TRACE( 9, ("UdpGetSockName(%p,%p,%p)\n", pvSap,pName,pLen));
  ASSERT( NULL != pUdp);
  ASSERT( NULL != pName);
  ASSERT( NULL != pLen);

  if ( NULL == pLen || *pLen < (int)sizeof( struct SWS_sockaddr_in))
    return SWS_EFAULT;

#if 1
  if ( SWS_INADDR_ANY == pUdp->localHost)
    return SWS_EINVAL;
#endif

  ((SWS_PTR(struct SWS_sockaddr_in))pName)->sin_family = SWS_AF_INET;
  NetWrite16( &((SWS_PTR(struct SWS_sockaddr_in))pName)->sin_port, pUdp->localPort);
  NetWrite32( &((SWS_PTR(struct SWS_sockaddr_in))pName)->sin_addr.SWS_s_addr, pUdp->localHost);

  *pLen = sizeof( struct SWS_sockaddr_in);

  return 0;
  }


/*
 * Get the address of the remote host
 */
static SWS_CB( int, UdpGetPeerName(
  SWS_PTR( void) pvSap,               /* IN: -> SAP data */
  SWS_PTR( struct SWS_sockaddr) pName,/* OUT: name */
  SWS_PTR( int) pLen                  /* IN/OUT: namelen */
)){
  SUdpSap* const pUdp = (SUdpSap*)pvSap;

  TRACE( 9, ("UdpGetPeerName(%p,%p,%p)\n", pvSap,pName,pLen));
  ASSERT( NULL != pUdp);
  ASSERT( NULL != pName);
  ASSERT( NULL != pLen);

  if ( NULL == pLen || *pLen < (int)sizeof( struct SWS_sockaddr_in))
    return SWS_EFAULT;

  if ( SWS_INADDR_ANY == pUdp->remoteHost)
    return SWS_ENOTCONN;

  ((SWS_PTR(struct SWS_sockaddr_in))pName)->sin_family = SWS_AF_INET;
  NetWrite16( &((SWS_PTR(struct SWS_sockaddr_in))pName)->sin_port, pUdp->remotePort);
  NetWrite32( &((SWS_PTR(struct SWS_sockaddr_in))pName)->sin_addr.SWS_s_addr, pUdp->remoteHost);

  *pLen = sizeof( struct SWS_sockaddr_in);

  return 0;
  }


/****************************************************************************
 * Upcalls from the IP layer
 ****************************************************************************/

/*
 * IP Datagram received indication
 * Single threaded
 */
SWS_CB( void, UdpRecvInd(
  SWS_PTR( void) pvContext,
  SWS_BufferH hBuffer,
  size_t hdrLen,
  ui32 src,                           /* IP src address */
  ui32 dst,                           /* IP dst address */
  ui32 spec,                          /* IP address of receiving adapter */
  unsigned tos
)){
  SUdpSap* pUdp;
  SWS_PTR( void) pvHdr;
  SWS_size_t uLen;
  unsigned len, sum;
  ui16 localPort, remotePort;
  (void)pvContext;
  (void)tos;

  TRACE( 9, ("UdpRecvInd(%p,%p,%u,%lu,%lu,%lu,%u)\n",
    pvContext,hBuffer,hdrLen,src,dst,spec,tos));
  ASSERT( PlatformIsCritical());
  ASSERT( NULL != hBuffer);

  AtomicIncrement( &s_aRxDatagrams);

  /* Get UDP header */
  pvHdr = SWS_BufferGetFragment( hBuffer, hdrLen, &uLen);
  if ( NULL == pvHdr || uLen < SWS_UDP_HDR)
    {
    TRACE( 1, ("*** UdpRecvInd() Truncated header\n"));
    AtomicIncrement( &s_aRxErrors);
    SWS_GefRaise( SWS_kGefUdpError, "Header too short");
    return;
    }

  /* Ensure buffer length is correct */
  uLen = SWS_BufferSize( hBuffer);
  ASSERT( uLen >= hdrLen + SWS_UDP_HDR);
  uLen -= hdrLen;
  SWS_PEEK16( len, pvHdr, SWS_UDP_LEN);
  if ( uLen < (SWS_size_t)len)
    {
    TRACE( 1, ("*** UdpRecvInd() Truncated data\n"));
    AtomicIncrement( &s_aRxErrors);
    SWS_GefRaise( SWS_kGefUdpError, "Truncated data");
    return;
    }
  ASSERT( len >= SWS_UDP_HDR);

  SWS_PEEK16( sum, pvHdr, SWS_UDP_SUM);

  switch ( s_uChecksum)
    {
  case 2:
    if ( 0 == sum)
      {
      TRACE( 1, ("*** UdpRecvInd() Discarding datagram with no checksum\n"));
      AtomicIncrement( &s_aRxErrors);
      SWS_GefRaise( SWS_kGefUdpError, "No checksum");
      return;
      }
    /* Fall thru */

  default:
    if ( sum)
      {
      /* Verify checksum */
      ui32 uSum = SWS_IpPseudoHdr( src, dst, SWS_UDP_PROTOCOL, len);
      if ( 0 != SWS_IpChecksum( hBuffer, hdrLen, len, uSum))
        {
        TRACE( 1, ("*** UdpRecvInd() Bad checksum\n"));
        AtomicIncrement( &s_aRxErrors);
        SWS_GefRaise( SWS_kGefUdpError, "Bad checksum");
        return;
        }
      }
    break;

  case 0:
    /* Ignore checksum */
    break;
    }

  AtomicAdd( &s_aRxBytes, (int)len);
  len -= SWS_UDP_HDR;
  hdrLen += SWS_UDP_HDR;

  /* Get end point */
  SWS_PEEK16( localPort, pvHdr, SWS_UDP_DST);
  SWS_PEEK16( remotePort, pvHdr, SWS_UDP_SRC);

#if OPT_IP_MULTICAST
  /* Multicast? */
  if ( SWS_IN_CLASSD( dst))
    {
    /* Yes, offer to all matching saps */
    for ( pUdp = s_pBoundUdps; NULL != pUdp; pUdp = pUdp->pNext)
      {
      if ( UdpGroupFind( pUdp, dst, spec))
        {
        if ( localPort == pUdp->localPort)
          {
          if ( remotePort == pUdp->remotePort
            || 0 == pUdp->remotePort
          ) {
            if ( spec == pUdp->localHost
              || SWS_INADDR_ANY == pUdp->localHost
            ) {
              if ( src == pUdp->remoteHost
                || SWS_INADDR_ANY == pUdp->remoteHost
              ) {
                UdpRecvPacket( pUdp, hBuffer, hdrLen, len, src, remotePort);
                }
              }
            }
          }
        }
      }

    return;
    }
  else
#endif
  /* Match with bound SAP */
  if ( NULL == (pUdp = UdpFindSap( localPort, remotePort, spec, src))
    && NULL == (pUdp = UdpFindSap( localPort, 0, spec, SWS_INADDR_NONE))
    && NULL == (pUdp = UdpFindSap( localPort, 0, SWS_INADDR_ANY, SWS_INADDR_NONE))
  ) {
    TRACE( 2, ("UdpRecvInd() Port %u not known\n", localPort));
    AtomicIncrement( &s_aRxNoPort);

#if !OPT_UDP_DISCARD_NOPORT && OPT_PROTO_ICMP
    if ( !s_ignore_noport && dst == spec)
      {
      /* Send an ICMP port not reachable */
      SWS_BufferH hb = SWS_BufferDup( hBuffer, hdrLen);
      if ( NULL != hb)
        {
        SWS_IcmpSend(
          spec, src,
          SWS_ICMP_DEST_UNREACH,
          SWS_ICMP_PORT_UNREACH,
          0 /*ident&seq*/,
          hb,
          SWS_IP_IDENT_DEFAULT,
          NULL
        );
        SWS_BufferRelease( hb);
        }
      }
#endif

    return;
    }

  UdpRecvPacket( pUdp, hBuffer, hdrLen, len, src, remotePort);
  }


#if OPT_PROTO_ICMP
/*
 * ICMP message received indication
 * Single threaded
 */
SWS_CB( void, UdpIcmpInd(
  SWS_PTR( void) pvContext,           /* IN: Client context */
  SWS_BufferH hBuffer,                /* IN: IP datagram */
  size_t hdrLen,                      /* IN: IP header length including options */
  ui32 src,                           /* IN: src IP address in original datagram */
  ui32 dst,                           /* IN: dst IP address in original datagram */
  unsigned type,                      /* IN: ICMP type */
  unsigned code,                      /* IN: ICMP code */
  ui32 data                           /* IN: ICMP optional data (identifier & sequence) */
)){
  SWS_PTR( void) pvHdr;
  SUdpSap* pUdp;
  SWS_size_t uLen;
#if DEBUG_TRACE_ENABLE
  ui32 localHost;
#endif
  unsigned localPort, remotePort;
  (void)data;
  (void)code;
  (void)dst;
  (void)pvContext;

  TRACE( 9, ("UdpIcmpInd(%p,%p,%u,%08lx,%08lx,%u,%u,%lu)\n",
    pvContext,hBuffer,hdrLen,src,dst,type,code,data));
  ASSERT( PlatformIsCritical());
  ASSERT( NULL != hBuffer);

  pvHdr = SWS_BufferGetFragment( hBuffer, hdrLen, &uLen);
  ASSERT( NULL != pvHdr);
  if ( uLen < SWS_UDP_HDR)
    return;

  /* Match with bound SAP */
  SWS_PEEK16( localPort, pvHdr, SWS_UDP_SRC);
  SWS_PEEK16( remotePort, pvHdr, SWS_UDP_DST);
  if ( NULL == (pUdp = UdpFindSap( localPort, remotePort, src, dst))
    && NULL == (pUdp = UdpFindSap( localPort, 0, src, SWS_INADDR_NONE))
    && NULL == (pUdp = UdpFindSap( localPort, 0, SWS_INADDR_ANY, SWS_INADDR_NONE))
  ) {
#if DEBUG_TRACE_ENABLE
    localHost = SWS_INADDR_NONE;
#endif
    }
  else
    {
#if DEBUG_TRACE_ENABLE
    localHost = pUdp->localHost;
#endif
    }

  switch ( type)
    {
  case SWS_ICMP_DEST_UNREACH:
    TRACE( 2, ("UdpIcmpInd() Host %08lx port %u unreachable(%u) from %08lx(%08lx) port %u\n",
      dst, remotePort, code, localHost, src, localPort));
    AtomicIncrement( &s_aIcmpErrors);

    /* Indicate that the remote port is not reachable */
    if ( NULL != pUdp)
      {
      switch ( code)
        {
      case SWS_ICMP_NET_UNREACH:
        SWS_TransportEventSet( pUdp, SWS_kTransportEventTxReady, SWS_ENETUNREACH);
        break;
      case SWS_ICMP_HOST_UNREACH:
        SWS_TransportEventSet( pUdp, SWS_kTransportEventTxReady, SWS_EHOSTUNREACH);
        break;
      case SWS_ICMP_PROT_UNREACH:
      case SWS_ICMP_PORT_UNREACH:
      default:
        SWS_TransportEventSet( pUdp, SWS_kTransportEventError, SWS_ECONNRESET);
        break;
        }
      }
    break;

  case SWS_ICMP_QUENCH:
    TRACE( 2, ("UdpIcmpInd() Quench from %08lx port %u for %08lx(%08lx) port %u\n",
      dst, remotePort, localHost, src, localPort));
    if ( NULL != pUdp)
      SWS_TransportEventSet( pUdp, SWS_kTransportEventTxReady, SWS_ENOBUFS);
    break;

  case SWS_ICMP_TIME_EXCEED:
    TRACE( 2, ("UdpIcmpInd() Time exceeded(%u) from %08lx port %u for %08lx(%08lx) port %u\n",
      code, dst, remotePort, localHost, src, localPort));
    AtomicIncrement( &s_aIcmpErrors);
    break;

  case SWS_ICMP_PARAM_PROB:
    TRACE( 2, ("UdpIcmpInd() Param prob(%u) from %08lx port %u for %08lx(%08lx) port %u\n",
      code, dst, remotePort, localHost, src, localPort));
    AtomicIncrement( &s_aIcmpErrors);
    break;
    }
  }
#endif


/*
 * Send resources available
 * Single threaded
 */
SWS_CB( void, UdpSendInd(
  SWS_PTR( void) pvContext
)){
  SUdpSap* pUdp;
  (void)pvContext;

  TRACE( 9, ("UdpSendInd(%p)\n", pvContext));
  ASSERT( PlatformIsCritical());

  for ( pUdp = s_pBoundUdps; NULL != pUdp; pUdp = pUdp->pNext)
    {
    SWS_TransportEventSet( pUdp, SWS_kTransportEventTxReady, 1);
    }
  }


/****************************************************************************
 * SAP support
 ****************************************************************************/

/*
 * Bind a SAP to a local port.
 */
static int UdpBindSap( SUdpSap* pUdp, ui32 host, unsigned port)
  {
  SBindInfo bind;

  TRACE( 9, ("UdpBindSap(%p,%lu,%u)\n",pUdp,host,port));
  ASSERT( NULL != pUdp);
  ASSERT( 0 == pUdp->localPort);

  bind.pUdp = pUdp;
  bind.host = host;
  bind.port = port;

  /* Attach to list of bound SAPs */
  return PlatformSynchronise( &UdpBindSync, &bind);
  }

SWS_CB( int, UdpBindSync( SWS_PTR( void) pv))
  {
  SBindInfo* const pBind = (SBindInfo*)pv;
  SUdpSap* const pUdp = pBind->pUdp;
  unsigned port = pBind->port;

  ASSERT( PlatformIsCritical());

  if ( 0 == port)
    {
    /* Allocate a port number */
    unsigned u;

    port = s_port;
    for ( u = 0; NULL != UdpFindSap( port, 0, pBind->host, SWS_INADDR_NONE); ++u)
      {
      if ( u > (unsigned)(s_hiPort - s_loPort))
        {
        TRACE( 1, ("*** UdpBindSync() All ports in use\n"));
        return SWS_ENOBUFS;
        }

      if ( ++port > s_hiPort)
        port = s_loPort;
      else if ( port < s_loPort)
        port = s_loPort;
      }
    s_port = (ui16)port;
    }
  else
    {
    /* Check if port/host exist */
    SUdpSap* pExist = UdpFindSap( port, 0, pBind->host, SWS_INADDR_NONE);
    if ( NULL != pExist
      && !(pExist->flags & SWS_SO_REUSEADDR)
    ) {
      TRACE( 1, ("*** UdpBindSync() Port in use\n"));
      return SWS_EADDRINUSE;
      }
    }

  /* Save local port & address */
  pUdp->localHost = pBind->host;
  pUdp->localPort = (ui16)port;

  /* Attach to list of bound SAPs */
  pUdp->pNext = s_pBoundUdps;
  s_pBoundUdps = pUdp;

  return 0;
  }


/*
 * Find a SAP bound to a given local port.
 * Single threaded
 */
static SUdpSap* UdpFindSap(
  unsigned localPort,
  unsigned remotePort,
  ui32 host,
  ui32 remote
) {
  SUdpSap* pUdp;

  TRACE( 9, ("UdpFindSap(%u,%u,%08lx,%08lx)\n",localPort,remotePort,host,remote));
  ASSERT( PlatformIsCritical());

  /* A hash table would be faster */
  for ( pUdp = s_pBoundUdps; NULL != pUdp; pUdp = pUdp->pNext)
    {
    if ( localPort == pUdp->localPort
      && (remotePort == pUdp->remotePort
        || 0 == remotePort)
      && (pUdp->localHost == host
        || SWS_INADDR_NONE == host)
      && (pUdp->remoteHost == remote
        || SWS_INADDR_NONE == remote)
    ) {
      break;
      }
    }
  return pUdp;
  }


/*
 * Receive a datagram
 * Single threaded
 */
static void UdpRecvPacket(
  SUdpSap* pUdp,
  SWS_BufferH hBuffer,
  size_t hdrLen,
  unsigned len,
  ui32 src,                           /* IP src address */
  ui16 remotePort
) {
  SUdpMessage* pMsg;
  int read, write;

  if ( AtomicRead( &pUdp->count) + len > pUdp->RcvBuf)
    {
    TRACE( 2, ("UdpRecvPacket() RxBuffer size exceeded\n"));
    AtomicIncrement( &s_aRxOverflow);
    return;
    }

  AtomicAdd( &pUdp->count, (int)len);

  /* TODO: DirectCopyToWaitingReceiveBuffer */

  /* Add message to ring buffer */
  pMsg = &pUdp->message[ write = AtomicRead( &pUdp->write) ];
  pMsg->hBuffer = hBuffer;
  pMsg->header = (ui16)hdrLen;
  pMsg->remoteHost = src;
  pMsg->remotePort = remotePort;

  if ( ++write >= (int)s_uUdpBuffers)
    write = 0;

  if ( (read = AtomicRead( &pUdp->read)) == write)
    {
    SWS_BufferH h;

    /* Overflow */
    TRACE( 2, ("UdpRecvPacket(%p) Overflow\n", pUdp));

    /* Discard the oldest datagram */
    h = pUdp->message[ read].hBuffer;
    if ( ++read >= (int)s_uUdpBuffers)
      read = 0;
    AtomicWrite( &pUdp->read, read);

    SWS_BufferRelease( h);
    AtomicIncrement( &s_aRxOverflow);
    }
  AtomicWrite( &pUdp->write, write);

  TRACE( 3, ("UdpRecvPacket(%p) port %u %u bytes\n", pUdp, pUdp->localPort, len));
  SWS_BufferAddRef( hBuffer);

  /* Indicate Rx data available */
  SWS_TransportEventSet( pUdp, SWS_kTransportEventRxReady, 1);
  }


#if OPT_IP_MULTICAST
/*
 * Join a multicast group
 */
static int UdpGroupJoin(
  SUdpSap* const pUdp,
  ui32 group,
  ui32 src
) {
  int iRet;
  SGroupMember* pGroup;

  TRACE( 9, ("UdpGroupJoin(%p,%08lx,%08lx)\n", pUdp,group,src));
  ASSERT( NULL != pUdp);

  iRet = UdpGroupFind( pUdp, group, src);
  if ( iRet >= 0)
    return 0;

  if ( pUdp->groups >= lengthof( pUdp->group) )
    {
    TRACE( 1, ("*** UdpGroupAdd() too many groups\n"));
    return SWS_ENOMEM;
    }

  if ( SWS_INADDR_ANY == src)
    {
    int len = sizeof( src);
    SWS_IpGetOption( &pUdp->ipOptions, SWS_IP_MULTICAST_IF, &src, &len);
    }

  pGroup = &pUdp->group[ pUdp->groups++];
  pGroup->addr = group;
  pGroup->src = src;

  iRet = SWS_IpGroupJoin( group, src);
  if ( iRet)
    --(pUdp->groups);

  return iRet;
  }


/*
 * Leave a multicast group
 */
static int UdpGroupLeave(
  SUdpSap* const pUdp,
  ui32 group,
  ui32 src
) {
  int iRet;

  TRACE( 9, ("UdpGroupLeave(%p,%08lx,%08lx)\n", pUdp,group,src));
  ASSERT( NULL != pUdp);

  iRet = UdpGroupFind( pUdp, group, src);
  if ( iRet < 0)
    return SWS_EINVAL;

  src = pUdp->group[ iRet].src;

  if ( iRet < (int)--(pUdp->groups))
    {
    /* Delete the group */
    memmove(
      &pUdp->group[ iRet],
      &pUdp->group[ iRet + 1],
      (pUdp->groups - iRet) * sizeof( pUdp->group[0])
    );
    }

  return SWS_IpGroupLeave( group, src);
  }


/*
 * Find a multicast group
 */
static int UdpGroupFind(
  SUdpSap* const pUdp,
  ui32 group,                         /* IN: group */
  ui32 src
) {
  size_t i;
  SGroupMember* pGroup;

  TRACE( 9, ("UdpGroupFind(%p,%08lx)\n", pUdp,group));
  ASSERT( NULL != pUdp);

  for ( pGroup = &pUdp->group[ i = 0]; i < pUdp->groups; ++i, ++pGroup)
    {
    if ( pGroup->addr == group
      && (pGroup->src == src || SWS_INADDR_ANY == src)
    ) {
      return (int)i;
      }
    }

  /* Not found */
  return -1;
  }
#endif /* OPT_IP_MULTICAST */

#else
char SWS_Udp;
#endif /* OPT_PROTO_UDP */
/* End of file */
