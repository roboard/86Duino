/****************************************************************************
 *
 * File name   : rawip.c
 * Function    : Raw sockets transport protocol
 * Project     : SwsSock
 * Authors     : Lawrence Rust
 * Systems     : ANSI C.
 *
 ****************************************************************************
 *
 * Created by Lawrence Rust, Software Systems Consultants
 * lvr@softsystem.co.uk. Tel/Fax +33 5 49 72 79 63
 *
 * 24-Apr-01, LVR, Created.
 * 03-Oct-01, LVR, Fix TRACE warnings for ARM SDT 2.51 .
 * 17-Oct-01, LVR, Added RawipSendInd to handle SWS_ENOBUFS in RawipSendTo
 * 07-Feb-02, LVR, RawipSendTo allows adapters with SWS_INADDR_ANY for DHCP etc
 * 04-Dec-02, LVR, RawipIcmpInd delivers datagram to sockets doing ICMP
 * 11-Jun-05, LVR, RawipConnect selects localHost if none bound.
 * 25-Jun-05, LVR, GetSock/PeerName return error if local/remote host unset
 *
 * Comments:
 * --------
 * This file implements the raw sockets transport
 *
 ****************************************************************************
 */

/* Compilation options */
#include "config.h"

/* Exports */
#include "rawip.h"

#if OPT_PROTO_IPRAW
/* Imports */
#include <stddef.h>
#include <string.h>

#include "sws_buf.h"
#include "sws_tprt.h"
#include "sws_ip.h"
#include "sws_icmp.h"
#include "sws_sock.h"  /* SWS_SOCK_RAW */
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
typedef struct SRawipMessage
  {
  SWS_BufferH hBuffer;
  ui32 remoteHost;
  } SRawipMessage;

#if OPT_IP_MULTICAST
/* Multicast group */
typedef struct SGroupMember
  {
  ui32 addr, src;
  } SGroupMember;
#endif

/* A SOCK_RAW Service Access Point */
typedef struct SRawipSap
  {
  ui32 localHost;
  ui32 remoteHost;
  ui8 protocol;

  ui16 flags;
  SWS_IpOptions ipOptions;
  size_t SndBuf;                      /* Size of send buffer */
  size_t RcvBuf;                      /* Size of receive buffer */

#if OPT_IP_MULTICAST
  size_t groups;                      /* No. active multicast groups */
  SGroupMember group[ SWS_IP_MAX_MEMBERSHIPS];
#endif

  /* Receive ring buffer */
  Atomic_t read, write, count;
  SWS_VARRAY( SRawipMessage, message); /* VARIABLE LENGTH */
  } SRawipSap;


/*
 * Prototypes
 */
static SWS_FTransportOpen RawipOpen;
static SWS_FTransportClose RawipClose;
static SWS_FTransportBind RawipBind;
static SWS_FTransportListen RawipListen;
static SWS_FTransportConnect RawipConnect;
static SWS_FTransportShutdown RawipShutdown;
static SWS_FTransportSendTo RawipSendTo;
static SWS_FTransportRecvFrom RawipRecvFrom;
static SWS_FTransportGetInfo RawipGetInfo;
static SWS_FTransportSetInfo RawipSetInfo;
static SWS_FTransportCleanup RawipCleanup;
static SWS_FTransportGetPeerName RawipGetPeerName;
static SWS_FTransportGetSockName RawipGetSockName;

static SWS_FIpRecvInd RawipRecvInd;
#if OPT_PROTO_ICMP
static SWS_FIpIcmpInd RawipIcmpInd;
#endif
static SWS_FIpSendInd RawipSendInd;

static FPlatformCallback RawipCloseSync;

#if OPT_IP_MULTICAST
static int RawipGroupJoin( SRawipSap* const, ui32, ui32);
static int RawipGroupLeave( SRawipSap* const, ui32, ui32);
static int RawipGroupFind( SRawipSap* const, ui32 , ui32);
#endif


/*
 * Module data
 */
/* Configuration */
static size_t s_uRawipSaps;
static size_t s_uRawipBuffers;

/* Stats */
static Atomic_t s_aTxDatagrams, s_aTxBytes;
static Atomic_t s_aRxDatagrams, s_aRxBytes;
static Atomic_t s_aRxOverflow;        /* Rx Buffer overflows */
#if OPT_PROTO_ICMP || DEBUG_TRACE_ENABLE
static Atomic_t s_aIcmpErrors;        /* ICMP errors received */
#endif

/* SOCK_RAW transport registration data */
static const SWS_TransportDesc s_transportRawip = {
  sizeof( s_transportRawip),
  SWS_TRANSPORT_VERSION,
  SWS_AF_INET,
  SWS_SOCK_RAW,
  0, /* Any protocol */
  0, /* dummy */
  "raw",
  &RawipOpen,
  &RawipClose,
  &RawipBind,
  &RawipListen,
  &RawipConnect,
  &RawipShutdown,
  &RawipSendTo,
  &RawipRecvFrom,
  &RawipGetInfo,
  &RawipSetInfo,
  &RawipCleanup,
  &RawipGetSockName,
  &RawipGetPeerName
};
static int s_bRegTransport;

/* RAWIP protocol registration data */
static SWS_IpProtocolDesc s_protocolRawip = {
  sizeof( s_protocolRawip),
  SWS_PROTOCOL_VERSION,
  0, /* Filled in by RawipOpen & RawipClose */
  0,
  &RawipRecvInd,
#if OPT_PROTO_ICMP
  &RawipIcmpInd,
#else
  0,
#endif
  &RawipSendInd
};
static int s_iProtocols;


/*
 * Initialise this module
 * Single threaded
 */
int SWS_RawipInit( void)              /* Returns !0 on error */
  {
  int iRet;

  TRACE( 9, ("SWS_RawipInit()\n"));
  ASSERT( PlatformIsCritical());
  ASSERT( !s_bRegTransport);
  ASSERT( !s_iProtocols);

  s_uRawipSaps = (size_t)PlatformPropertyGetUint(
    PKEY_RAWIP_SOCKETS, OPT_RAWIP_SOCKETS );
  if ( 0 == s_uRawipSaps)
    return 0;

  s_uRawipBuffers = (size_t)(1 + PlatformPropertyGetUint(
    PKEY_RAWIP_BUFFERS, OPT_RAWIP_BUFFERS ));

  /* Register the transport */
  iRet = SWS_TransportRegister(
    &s_transportRawip,
    NULL,
    s_uRawipSaps,
    sizeof( SRawipSap) + s_uRawipBuffers * sizeof( SRawipMessage)
  );
  s_bRegTransport = 0 == iRet;

  return iRet;
  }


/*
 * Finalise this module
 * Single threaded
 */
void SWS_RawipCleanup( void)
  {
  TRACE( 9, ("SWS_RawipCleanup()\n"));
  ASSERT( PlatformIsCritical());

  ASSERT( !s_bRegTransport);
  ASSERT( !s_iProtocols);

  TRACE( 1, ("RAW: TxDatagrams:%u TxBytes:%u IcmpErrors:%u"
    " RxDatagrams:%u RxBytes:%u RxOverflows:%u\n",
    AtomicRead( &s_aTxDatagrams), AtomicRead( &s_aTxBytes),
    AtomicRead( &s_aIcmpErrors), AtomicRead( &s_aRxDatagrams),
    AtomicRead( &s_aRxBytes), AtomicRead( &s_aRxOverflow)
  ));
  }


/****************************************************************************
 * Downcalls from SwsSock
 ****************************************************************************/

/*
 * Transport layer is closing
 * Single threaded
 */
static SWS_CB( void, RawipCleanup( SWS_PTR( void) pvContext))
  {
  (void)pvContext;

  TRACE( 9, ("RawipCleanup(%p)\n", pvContext));
  ASSERT( PlatformIsCritical());

  ASSERT( !s_iProtocols);
  s_bRegTransport = 0;
  }


/*
 * Open a Service Access Point
 */
SWS_CB( int, RawipOpen(
  SWS_PTR( void) pvContext,
  SWS_PTR( void) pvSap,               /* IN: -> SAP data */
  int type,                           /* IN: socket type */
  int protocol                        /* IN: protocol */
)){
  SRawipSap* const pRawip = (SRawipSap*)pvSap;
  int iRet;
  (void)pvContext;
  (void)type;

  TRACE( 9, ("RawipOpen(%p,%p,%d,%d)\n", pvContext, pvSap, type, protocol));
  ASSERT( NULL != pRawip);

  if ( protocol < SWS_IPPROTO_IP || protocol >= SWS_IPPROTO_MAX)
    {
    TRACE( 1, ("***RawipOpen() Invalid protocol %d\n", protocol));
    return SWS_EINVAL;
    }

  /* Initialise the SAP */
  memset( pRawip, 0, sizeof( *pRawip));
  pRawip->localHost = SWS_INADDR_ANY;
  pRawip->remoteHost = SWS_INADDR_ANY;
  pRawip->RcvBuf = pRawip->SndBuf = (size_t)SWS_BufferAvailable();
  pRawip->protocol = (ui8)protocol;
#if 0
  pRawip->flags = 0;
#if OPT_IP_MULTICAST
  pRawip->groups = 0;
#endif
  AtomicWrite( &pRawip->read, 0);
  AtomicWrite( &pRawip->write, 0);
  AtomicWrite( &pRawip->count, 0);
  memset( &pRawip->ipOptions, 0, sizeof( pRawip->ipOptions));
#endif
  pRawip->ipOptions.flags.multicast_loop = SWS_IP_DEFAULT_MULTICAST_LOOP;
  pRawip->ipOptions.multicast_ttl = SWS_IP_DEFAULT_MULTICAST_TTL;

  ++s_iProtocols;

  /* Register the protocol */
  s_protocolRawip.uProtocol = (ui8)protocol;
  iRet = SWS_IpRegisterProtocol( &s_protocolRawip, pRawip);
  if ( iRet)
    --s_iProtocols;

  SWS_TransportEventSet( pRawip, SWS_kTransportEventTxReady, 1);

  return iRet;
  }


/*
 * Close a Service Access Point
 */
SWS_CB( int, RawipClose(
  SWS_PTR( void) pvSap,
  int reset
)){
  SRawipSap* const pRawip = (SRawipSap*)pvSap;
  int read;
  (void)reset;

  TRACE( 9, ("RawipClose(%p,%d)\n", pvSap,reset));
  ASSERT( NULL != pRawip);

#if OPT_IP_MULTICAST
  while ( pRawip->groups > 0)
    {
    SGroupMember* pGroup = &pRawip->group[ pRawip->groups - 1];
    RawipGroupLeave( pRawip, pGroup->addr, pGroup->src);
    }
#endif

  /* Deregister the protocol */
  PlatformSynchronise( &RawipCloseSync, pRawip);

  /* Discard buffered data */
  read = AtomicRead( &pRawip->read);
  while ( read != AtomicRead( &pRawip->write))
    {
    SWS_BufferRelease( pRawip->message[ read].hBuffer );
    if ( ++read >= (int)s_uRawipBuffers)
      read = 0;

    AtomicWrite( &pRawip->read, read);
    }

  return 0;
  }

SWS_CB( int, RawipCloseSync( SWS_PTR( void) pv ))
  {
  SRawipSap* const pRawip = (SRawipSap*)pv;

  ASSERT( PlatformIsCritical());

  ASSERT( s_iProtocols > 0);
  --s_iProtocols;

  /* De-register the protocol */
  s_protocolRawip.uProtocol = pRawip->protocol;
  return SWS_IpDeregisterProtocol( &s_protocolRawip, pRawip);
  }


/*
 * Associate a Service Access Point with a local address & port
 */
SWS_CB( int, RawipBind(
  SWS_PTR( void) pvSap,               /* IN: IN: -> SAP */
  SWS_PTR( const struct SWS_sockaddr) pName,
  int namelen
)){
  SRawipSap* const pRawip = (SRawipSap*)pvSap;
  ui32 localHost;

  TRACE( 9, ("RawipBind(%p,%p,%d)\n", pvSap, pName, namelen));
  ASSERT( NULL != pRawip);
  ASSERT( NULL != pName);

  if ( SWS_AF_INET != pName->sa_family)
    {
    TRACE( 1, ("*** RawipBind() unsupported address type\n"));
    return SWS_EAFNOSUPPORT;
    }

  if ( namelen != sizeof( struct SWS_sockaddr_in))
    {
    TRACE( 1, ("*** RawipBind() namelen %d incorrect\n", namelen));
    return SWS_EINVAL;
    }

  /* Ensure local address is valid */
  NetRead32(
    localHost, &((SWS_PTR(const struct SWS_sockaddr_in))pName)->sin_addr.SWS_s_addr);
  if ( 0 == SWS_IpGetMaxSizes(
      localHost, localHost, pRawip->ipOptions.tos, NULL)
  ) {
    TRACE( 1, ("*** RawipBind() invalid local host\n"));
    return SWS_EADDRNOTAVAIL;
    }

  pRawip->localHost = localHost;

  return 0;
  }


/*
 * Wait for an incoming connection request
 */
SWS_CB( int, RawipListen(
  SWS_PTR( void) pvSap
)){
  TRACE( 9, ("RawipListen(%p)\n", pvSap));
  ASSERT( NULL != pvSap);
  (void)pvSap;

  /* Not relevant to RAW SAP's */
  return SWS_EOPNOTSUPP;
  }


/*
 * Associate a Service Access Point with a remote address & port
 */
SWS_CB( int, RawipConnect(
  SWS_PTR( void) pvSap,               /* IN: IN: -> SAP */
  SWS_PTR( const struct SWS_sockaddr) pName,
  int namelen
)){
  SRawipSap* const pRawip = (SRawipSap*)pvSap;
  ui32 remoteHost;

  TRACE( 9, ("RawipConnect(%p,%p,%d)\n", pvSap, pName, namelen));
  ASSERT( NULL != pRawip);
  ASSERT( NULL != pName);

  if ( namelen != sizeof( struct SWS_sockaddr_in))
    {
    TRACE( 1, ("*** RawipConnect() namelen %d incorrect\n", namelen));
    return SWS_EINVAL;
    }

  if ( SWS_AF_INET != pName->sa_family)
    {
    TRACE( 1, ("*** RawipConnect() incorrect address type\n"));
    return SWS_EINVAL;
    }

  NetRead32(
    remoteHost,
    &((SWS_PTR(const struct SWS_sockaddr_in))pName)->sin_addr.SWS_s_addr
  );
  if ( SWS_INADDR_ANY != remoteHost)
    {
    ui32 localHost = pRawip->localHost;
    if ( SWS_INADDR_ANY == localHost)
      {
      /* Select an adapter */
      unsigned tos = pRawip->ipOptions.tos;
      if ( (pRawip->flags & SWS_SO_DONTROUTE))
        tos |= 0x8000; /* Include interfaces with unset IP address */

      localHost = SWS_IpGetSrc( remoteHost, tos);
      if ( SWS_INADDR_NONE == localHost)
        {
        TRACE( 1, ("*** RawipConnect() %08lx not reachable\n", remoteHost));
        return SWS_ENETUNREACH;
        }

      /* Implicit bind */
      pRawip->localHost = localHost;
      }
    /* Ensure address is reachable */
    else if ( 0 == SWS_IpGetMaxSizes(
        localHost, remoteHost, pRawip->ipOptions.tos, NULL
      )
    ) {
      TRACE( 1, ("*** RawipConnect() %08lx not reachable\n", remoteHost));
      return SWS_ENETUNREACH;
      }
    }

  /* Set remote host address */
  pRawip->remoteHost = remoteHost;

  return 0;
  }


/*
 * Break an existing connection
 */
SWS_CB( int, RawipShutdown( SWS_PTR( void) pvSap, int how ))
  {
  (void)pvSap;
  (void)how;

  TRACE( 9, ("RawipDisconnect(%p,%d)\n", pvSap,how));
  ASSERT( NULL != pvSap);

  return 0;
  }


/*
 * Send data to remote connection
 */
SWS_CB( int, RawipSendTo(
  SWS_PTR( void) pvSap,               /* IN: IN: -> SAP */
  SWS_PTR( const void) pBuf,          /* IN: -> data to send */
  SWS_PTR( unsigned) pLen,            /* I/O: data len */
  unsigned flags,                     /* IN: flags, SWS_MSG_xxx */
  SWS_PTR( const struct SWS_sockaddr) pName,/* IN: Optional name */
  int namelen                         /* IN: namelen */
)){
  SRawipSap* const pRawip = (SRawipSap*)pvSap;
  SWS_BufferH hBuffer;
  int err;
  unsigned len;
  ui32 remoteHost, localHost;

  TRACE( 9, ("RawipSendTo(%p,%p,%p,%u,%p,%d)\n",
    pvSap, pBuf, pLen, flags, pName, namelen));
  ASSERT( NULL != pvSap);
  ASSERT( NULL != pLen);
  ASSERT( NULL != pBuf || 0 == *pLen);

  len = *pLen;
  *pLen = 0;

  /* Validate params */
  if ( len > SWS_IP_MAXLEN - SWS_IPHDR_MINLEN)
    {
    TRACE( 1, ("*** RawipSendTo() Length too big\n"));
    return SWS_EMSGSIZE;
    }

  /* NB ignore SWS_MSG_DONTROUTE */
  if ( flags & ~SWS_MSG_DONTROUTE)
    {
    TRACE( 1, ("*** RawipSendTo() unsupported flags\n"));
    return SWS_EINVAL;
    }

  if ( NULL != pName)
    {
    /* Check remote address */
    if ( SWS_AF_INET != pName->sa_family)
      {
      TRACE( 1, ("*** RawipSendTo() unsupported address type\n"));
      return SWS_EAFNOSUPPORT;
      }

    if ( namelen != sizeof( struct SWS_sockaddr_in))
      {
      TRACE( 1, ("*** RawipSendTo() namelen %d incorrect\n", namelen));
      return SWS_EINVAL;
      }

    NetRead32( remoteHost,
      &((SWS_PTR(const struct SWS_sockaddr_in))pName)->sin_addr.SWS_s_addr);
    }
  else
    {
    /* Use previous connection */
    remoteHost = pRawip->remoteHost;
    }

  if ( SWS_INADDR_ANY == remoteHost)
    {
    TRACE( 1, ("*** RawipSendTo() invalid remote address\n"));
    return SWS_EADDRNOTAVAIL;
    }

  if ( SWS_INADDR_BROADCAST == remoteHost && !(pRawip->flags & SWS_SO_BROADCAST) )
    {
    TRACE( 1, ("*** RawipSendTo() broadcast not enabled\n"));
    return SWS_EACCES;
    }

  if ( len > pRawip->SndBuf)
    {
    TRACE( 1, ("*** RawipSendTo() datagram bigger than allocated buffer\n"));
    return SWS_ENOBUFS;
    }

  localHost = pRawip->localHost;
  if ( SWS_INADDR_ANY == localHost)
    {
    /* Select an adapter */
    unsigned tos = pRawip->ipOptions.tos;
    if ( (flags & SWS_MSG_DONTROUTE) || (pRawip->flags & SWS_SO_DONTROUTE))
      tos |= 0x8000; /* Include interfaces with unset IP address */

    localHost = SWS_IpGetSrc( remoteHost, tos);
    if ( SWS_INADDR_NONE == localHost)
      {
      TRACE( 1, ("*** RawipSendTo() %08lx not reachable\n", remoteHost));
      return SWS_ENETUNREACH;
      }

    /* Implicit bind */
    pRawip->localHost = localHost;
    }

  /* Wait for buffer space */
  if ( SWS_BufferTotal() < len + 0x2000)
    {
    TRACE( 1, ("*** RawipSendTo() Too big\n"));
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
    TRACE( 1, ("***RawipSendTo() Out of buffers\n"));
    return SWS_ENOBUFS;
    }

  /* Repeat until sent */
  for(;;)
    {
    /* Wait until Tx resources available */
    err = SWS_TransportEventWait( pRawip, SWS_kTransportEventTxReady);
    if ( 1 != err)
      {
      VERIFY( !SWS_BufferReleaseShadow( hBuffer));
      return err;
      }

    err = SWS_IpSend(
      localHost,
      remoteHost,
      pRawip->protocol,
      hBuffer,
      SWS_IP_IDENT_DEFAULT,
      &pRawip->ipOptions
    );
    /* Out of Tx resources? */
    if ( err >= 0)
      {
      /* No */
      SWS_TransportEventSet( pRawip, SWS_kTransportEventTxReady, 1);
      break;
      }
    }

  AtomicIncrement( &s_aTxDatagrams);
  AtomicAdd( &s_aTxBytes, (int)len);
  *pLen = len;

  /* Release buffer but copy user data if still referenced */
  SWS_BufferReleaseShadow( hBuffer);

  return err;
  }


/*
 * Get data on a connection
 */
SWS_CB( int, RawipRecvFrom(
  SWS_PTR( void) pvSap,               /* IN: IN: -> SAP */
  SWS_PTR( void) pBuf,                /* OUT: Data */
  SWS_PTR( unsigned) pLen,            /* I/O: buffer len */
  unsigned flags,                     /* IN: flags, SWS_MSG_xxx */
  SWS_PTR( struct SWS_sockaddr) pName, /* OUT: Optional name */
  SWS_PTR( int) pNamelen              /* I/O: namelen */
)){
  SRawipSap* const pRawip = (SRawipSap*)pvSap;
  SRawipMessage msg;
  int read;
  int iRet;
  unsigned len;
  SWS_size_t uLen;

  TRACE( 9, ("RawipRecvFrom(%p,%p,%p,%u,%p,%p)\n",
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
      TRACE( 1, ("*** RawipRecvFrom() NULL namelen\n"));
      return SWS_EFAULT;
      }

    if ( *pNamelen < (int)sizeof( struct SWS_sockaddr_in))
      {
      TRACE( 1, ("*** RawipRecvFrom() namelen %d too small\n", *pNamelen));
      return SWS_EINVAL;
      }
    }

  if ( ~SWS_MSG_PEEK & flags)
    {
    TRACE( 1, ("*** RawipRecvFrom() unsupported flags\n"));
    return SWS_EINVAL;
    }

  /* Check for Rx data */
  if ( SWS_MSG_PEEK & flags)
    {
    if ( (read = AtomicRead( &pRawip->read)) == AtomicRead( &pRawip->write))
      return 0;
    }
  else do
    {
    /* Wait until Rx data available */
    int err = SWS_TransportEventWait( pRawip, SWS_kTransportEventRxReady);
    if ( 1 != err)
      return err;
    }
  while ( (read = AtomicRead( &pRawip->read)) == AtomicRead( &pRawip->write));

  /* Get the oldest message */
  msg = pRawip->message[ read];
  if ( !(SWS_MSG_PEEK & flags))
    {
    /* Update the read ptr */
    if ( ++read >= (int)s_uRawipBuffers)
      read = 0;
    AtomicWrite( &pRawip->read, read);
    }

  /* Calc amount of data to receive */
  uLen = SWS_BufferSize( msg.hBuffer);
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
    TRACE( 1, ("*** RawipRecvFrom() %u byte buffer too small (wanted %u)\n",
      len, uLen));
    uLen = (SWS_size_t)len;
    iRet = SWS_EMSGSIZE;
    }
  SWS_BufferCopyToMemory( pBuf, msg.hBuffer, 0, uLen);
  ASSERT( (SWS_size_t)AtomicRead( &pRawip->count) >= uLen);
  AtomicAdd( &pRawip->count, -(int)uLen);

  if ( !(SWS_MSG_PEEK & flags) )
    SWS_BufferRelease( msg.hBuffer );

  if ( AtomicRead( &pRawip->read) != AtomicRead( &pRawip->write) )
    {
    /* There's still data remaining in the Rx queue */
    SWS_TransportEventSet( pRawip, SWS_kTransportEventRxReady, 1);
    }

  if ( NULL != pName)
    {
    /* Return source of datagram */
    memset( pName, 0, sizeof(struct SWS_sockaddr_in));
    pName->sa_family = SWS_AF_INET;
    NetWrite16( &((SWS_PTR(struct SWS_sockaddr_in))pName)->sin_port, 0 );
    NetWrite32( &((SWS_PTR(struct SWS_sockaddr_in))pName)->sin_addr.SWS_s_addr,
      msg.remoteHost);
    *pNamelen = sizeof(struct SWS_sockaddr_in);
    }

  return iRet;
  }


/*
 * Get info
 */
SWS_CB( int, RawipGetInfo(
  SWS_PTR( void) pvSap,               /* IN: -> SAP data */
  unsigned level,                     /* IN: level SWS_SOL_xxx */
  unsigned opt,                       /* IN: option */
  SWS_PTR( void) pVal,                /* OUT: -> value */
  SWS_PTR( int) pLen                  /* I/O: length of value */
)){
  SRawipSap* const pRawip = (SRawipSap*)pvSap;
  size_t size;
  int read;

  TRACE( 9, ("RawipGetInfo(%p,%u,%u,%p,%p)\n", pvSap,level,opt,pVal,pLen));
  ASSERT( NULL != pRawip);
  ASSERT( NULL != pVal);
  ASSERT( NULL != pLen);
  ASSERT( *pLen > 0);

  switch ( level)
    {
  case SWS_SOL_SOCKET:
    switch ( opt)
      {
    /*case SWS_SO_REUSEADDR:*/
    case SWS_SO_DONTROUTE:
    case SWS_SO_BROADCAST:
    case SWS_SO_USELOOPBACK:
      if ( *pLen != sizeof(int))
        return SWS_EFAULT;

      *(SWS_PTR(int))pVal = (int)(pRawip->flags & opt);
      break;

    case SWS_SO_MAX_MSG_SIZE:
      if ( *pLen != sizeof(unsigned))
        return SWS_EFAULT;

      /* Get max Tx datagram */
      size = SWS_IpGetMaxSizes(
        pRawip->localHost,
        SWS_INADDR_ANY != pRawip->remoteHost ? pRawip->remoteHost : pRawip->localHost,
        pRawip->ipOptions.tos, NULL
      );

      *(SWS_PTR(unsigned))pVal = (unsigned)size;
      break;

    case SWS_SO_RCVBUF:
      if ( *pLen != sizeof(unsigned))
        return SWS_EFAULT;

      *(SWS_PTR(unsigned))pVal = (unsigned)pRawip->RcvBuf;
      break;

    case SWS_SO_SNDBUF:
      if ( *pLen != sizeof(unsigned))
        return SWS_EFAULT;

      *(SWS_PTR(unsigned))pVal = (unsigned)pRawip->SndBuf;
      break;

    default:
      return SWS_EOPNOTSUPP;
      }
    break;

  case SWS_SOL_IP:
    return SWS_IpGetOption( &pRawip->ipOptions, opt, pVal, pLen);

  case SWS_SOL_IOCTL:
    switch ( opt)
      {
    default:
      return SWS_EOPNOTSUPP;

    case SWS_IOCTL_FIONREAD:
      if ( *pLen != sizeof(SWS_u_long))
        return SWS_EFAULT;

      /* Get size of first datagram */
      read = AtomicRead( &pRawip->read);
      if ( AtomicRead( &pRawip->write) == read)
        *(SWS_PTR(SWS_u_long))pVal = 0;
      else
        *(SWS_PTR(SWS_u_long))pVal =
          (SWS_u_long)SWS_BufferSize( pRawip->message[ read].hBuffer);
      break;
      }
    break;

  default:
    return SWS_EOPNOTSUPP;
    }

  return 0;
  }


/*
 * Set info
 */
SWS_CB( int, RawipSetInfo(
  SWS_PTR( void) pvSap,               /* IN: -> SAP data */
  unsigned level,                     /* IN: level SWS_SOL_xxx */
  unsigned opt,                       /* IN: option */
  SWS_PTR( const void) pVal,          /* IN: -> value */
  int len                             /* IN: length of value */
)){
  SRawipSap* const pRawip = (SRawipSap*)pvSap;
#if OPT_IP_MULTICAST
  SWS_PTR( const struct SWS_ip_mreq) pMreq;
#endif
  size_t size;

  TRACE( 9, ("RawipSetInfo(%p,%u,%u,%p,%d)\n", pvSap,level,opt,pVal,len));
  ASSERT( NULL != pRawip);
  ASSERT( NULL != pVal);
  ASSERT( len > 0);

  switch ( level)
    {
  case SWS_SOL_SOCKET:
    switch ( opt)
      {
    /*case SWS_SO_REUSEADDR:*/
    case SWS_SO_DONTROUTE:
    case SWS_SO_BROADCAST:
    case SWS_SO_USELOOPBACK:
      if ( len != sizeof(int))
        return SWS_EFAULT;

      if ( *(SWS_PTR(const int))pVal)
        pRawip->flags |= opt;
      else
        pRawip->flags &= ~opt;
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

      pRawip->SndBuf = size;
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

      pRawip->RcvBuf = size;
      break;

    default:
      return SWS_EOPNOTSUPP;
      }
    break;

  case SWS_SOL_IP:
#if OPT_IP_MULTICAST
    switch ( opt)
      {
    case SWS_IP_ADD_MEMBERSHIP:
      pMreq = (SWS_PTR(const struct SWS_ip_mreq))pVal;
      return RawipGroupJoin(
        pRawip,
        SWS_ntohl( pMreq->imr_multiaddr.SWS_s_addr),
        SWS_ntohl( pMreq->imr_interface.SWS_s_addr)
      );

    case SWS_IP_DROP_MEMBERSHIP:
      pMreq = (SWS_PTR(const struct SWS_ip_mreq))pVal;
      return RawipGroupLeave(
        pRawip,
        SWS_ntohl( pMreq->imr_multiaddr.SWS_s_addr),
        SWS_ntohl( pMreq->imr_interface.SWS_s_addr)
      );

    default:
      break;
      }
#endif
    return SWS_IpSetOption( &pRawip->ipOptions, opt, pVal, len);

  default:
    return SWS_EOPNOTSUPP;
    }

  return 0;
  }


/*
 * Get the address of the local host
 */
static SWS_CB( int, RawipGetSockName(
  SWS_PTR( void) pvSap,               /* IN: -> SAP data */
  SWS_PTR( struct SWS_sockaddr) pName,/* OUT: name */
  SWS_PTR( int) pLen                  /* IN/OUT: namelen */
)){
  SRawipSap* const pRawip = (SRawipSap*)pvSap;

  TRACE( 9, ("RawipGetSockName(%p,%p,%p)\n", pvSap,pName,pLen));
  ASSERT( NULL != pRawip);
  ASSERT( NULL != pName);
  ASSERT( NULL != pLen);

  if ( NULL == pLen || *pLen < (int)sizeof( struct SWS_sockaddr_in))
    return SWS_EFAULT;

#if 1
  if ( SWS_INADDR_ANY == pRawip->localHost)
    return SWS_EINVAL;
#endif

  ((SWS_PTR(struct SWS_sockaddr_in))pName)->sin_family = SWS_AF_INET;
  ((SWS_PTR(struct SWS_sockaddr_in))pName)->sin_port = 0;
  NetWrite32( &((SWS_PTR(struct SWS_sockaddr_in))pName)->sin_addr.SWS_s_addr,
    pRawip->localHost);

  *pLen = sizeof( struct SWS_sockaddr_in);

  return 0;
  }


/*
 * Get the address of the remote host
 */
static SWS_CB( int, RawipGetPeerName(
  SWS_PTR( void) pvSap,               /* IN: -> SAP data */
  SWS_PTR( struct SWS_sockaddr) pName,/* OUT: name */
  SWS_PTR( int) pLen                  /* IN/OUT: namelen */
)){
  SRawipSap* const pRawip = (SRawipSap*)pvSap;

  TRACE( 9, ("RawipGetPeerName(%p,%p,%p)\n", pvSap,pName,pLen));
  ASSERT( NULL != pRawip);
  ASSERT( NULL != pName);
  ASSERT( NULL != pLen);

  if ( NULL == pLen || *pLen < (int)sizeof( struct SWS_sockaddr_in))
    return SWS_EFAULT;

  if ( SWS_INADDR_ANY == pRawip->remoteHost)
    return SWS_ENOTCONN;

  ((SWS_PTR(struct SWS_sockaddr_in))pName)->sin_family = SWS_AF_INET;
  ((SWS_PTR(struct SWS_sockaddr_in))pName)->sin_port = 0;
  NetWrite32( &((SWS_PTR(struct SWS_sockaddr_in))pName)->sin_addr.SWS_s_addr,
    pRawip->remoteHost);

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
SWS_CB( void, RawipRecvInd(
  SWS_PTR( void) pvContext,
  SWS_BufferH hBuffer,
  size_t hdrLen,
  ui32 src,                           /* IP src address */
  ui32 dst,                           /* IP dst address */
  ui32 spec,                          /* IP address of receiving adapter */
  unsigned tos
)){
  SRawipSap* const pRawip = (SRawipSap*)pvContext;
  SRawipMessage* pMsg;
  SWS_size_t uLen;
  int read, write;
  (void)hdrLen;
  (void)dst;
  (void)spec;
  (void)tos;

  TRACE( 9, ("RawipRecvInd(%p,%p,%u,%08lx,%08lx,%08lx,%u)\n",
    pvContext,hBuffer,hdrLen,src,dst,spec,tos));
  ASSERT( PlatformIsCritical());
  ASSERT( NULL != pRawip);
  ASSERT( NULL != hBuffer);

  if ( SWS_INADDR_ANY != pRawip->localHost
    && spec !=  pRawip->localHost
  ) {
    /* Bound to another adapter */
    return;
    }

#if OPT_IP_MULTICAST
  /* Multicast? */
  if ( SWS_IN_CLASSD( dst))
    {
    if ( RawipGroupFind( pRawip, dst, spec) < 0)
      {
      /* Not in the group */
      return;
      }
    }
  else
#endif
  if ( SWS_INADDR_ANY != pRawip->remoteHost
    && src != pRawip->remoteHost
  ) {
    /* Connected to a different host */
    return;
    }

  AtomicIncrement( &s_aRxDatagrams);

  /* NB include the header for all raw sockets */
  uLen = SWS_BufferSize( hBuffer);
  AtomicAdd( &s_aRxBytes, (int)uLen);

  if ( AtomicRead( &pRawip->count) + uLen > pRawip->RcvBuf)
    {
    TRACE( 1, ("*** RawipRecvInd() RxBuffer size exceeded\n"));
    AtomicIncrement( &s_aRxOverflow);
    return;
    }

  /* TODO: DirectCopyToWaitingReceiveBuffer */

  /* Add message to ring buffer */
  pMsg = &pRawip->message[ write = AtomicRead( &pRawip->write) ];
  pMsg->hBuffer = hBuffer;
  pMsg->remoteHost = src;

  if ( ++write >= (int)s_uRawipBuffers)
    write = 0;

  if ( (read = AtomicRead( &pRawip->read)) == write)
    {
    SWS_BufferH h;

    /* Overflow */
    TRACE( 2, ("RawipRecvInd() Overflow\n"));

    /* Discard the oldest datagram */
    h = pRawip->message[ read].hBuffer;
    if ( ++read >= (int)s_uRawipBuffers)
      read = 0;
    AtomicWrite( &pRawip->read, read);

    SWS_BufferRelease( h);
    AtomicIncrement( &s_aRxOverflow);
    }

  AtomicAdd( &pRawip->count, (int)uLen);
  AtomicWrite( &pRawip->write, write);

  SWS_BufferAddRef( hBuffer);

  /* Signal Rx data available */
  SWS_TransportEventSet( pRawip, SWS_kTransportEventRxReady, 1);
  }


#if OPT_PROTO_ICMP
/*
 * ICMP message received indication
 * Single threaded
 */
SWS_CB( void, RawipIcmpInd(
  SWS_PTR( void) pvContext,           /* IN: Client context */
  SWS_BufferH hBuffer,                /* IN: IP datagram - do not modify */
  size_t hdrLen,                      /* IN: IP header length including options */
  ui32 src,                           /* IN: src IP address in original datagram */
  ui32 dst,                           /* IN: dst IP address in original datagram */
  unsigned type,                      /* IN: ICMP type */
  unsigned code,                      /* IN: ICMP code */
  ui32 data                           /* IN: ICMP optional data (identifier & sequence) */
)){
  SRawipSap* const pRawip = (SRawipSap*)pvContext;
  (void)hBuffer;
  (void)hdrLen;
  (void)src;
  (void)dst;
  (void)data;

  TRACE( 9, ("RawipIcmpInd(%p,%p,%u,%08lx,%08lx,%u,%u,%lu)\n",
    pvContext,hBuffer,hdrLen,src,dst,type,code,data));
  ASSERT( PlatformIsCritical());
  ASSERT( NULL != pvContext);
  ASSERT( NULL != hBuffer);

  switch ( type)
    {
  case SWS_ICMP_DEST_UNREACH:
    TRACE( 2, ("RawipIcmpInd() Host %08lx unreachable(%u) from %08lx(%08lx)\n",
      dst, code, ((SWS_PTR(SRawipSap))pvContext)->localHost, src));
    AtomicIncrement( &s_aIcmpErrors);

    /* Indicate that the remote port is not reachable */
    switch ( code)
      {
    case SWS_ICMP_NET_UNREACH:
      SWS_TransportEventSet( pRawip, SWS_kTransportEventTxReady, SWS_ENETUNREACH);
      break;
    case SWS_ICMP_HOST_UNREACH:
      SWS_TransportEventSet( pRawip, SWS_kTransportEventTxReady, SWS_EHOSTUNREACH);
      break;
    case SWS_ICMP_PROT_UNREACH:
    case SWS_ICMP_PORT_UNREACH:
    default:
      SWS_TransportEventSet( pRawip, SWS_kTransportEventError, SWS_ECONNRESET);
      break;
      }
    break;

  case SWS_ICMP_QUENCH:
    TRACE( 2, ("RawipIcmpInd() Quench from %08lx for %08lx(%08lx)\n",
      dst, ((SWS_PTR(SRawipSap))pvContext)->localHost, src));
    break;

  case SWS_ICMP_TIME_EXCEED:
    TRACE( 2, ("RawipIcmpInd() Time exceeded(%u) from %08lx for %08lx(%08lx)\n",
      code, dst, ((SWS_PTR(SRawipSap))pvContext)->localHost, src));
    AtomicIncrement( &s_aIcmpErrors);
    break;

  case SWS_ICMP_PARAM_PROB:
    TRACE( 2, ("RawipIcmpInd() Param prob(%u) from %08lx for %08lx(%08lx)\n",
      code, dst, ((SWS_PTR(SRawipSap))pvContext)->localHost, src));
    AtomicIncrement( &s_aIcmpErrors);
    break;
    }
  }
#endif


/*
 * Send resources available
 * Single threaded
 */
SWS_CB( void, RawipSendInd(
  SWS_PTR( void) pvContext
)){
  SRawipSap* const pRawip = (SRawipSap*)pvContext;

  TRACE( 9, ("RawipSendInd(%p)\n", pvContext));
  ASSERT( PlatformIsCritical());

  SWS_TransportEventSet( pRawip, SWS_kTransportEventTxReady, 1);
  }


#if OPT_IP_MULTICAST
/*
 * Join a multicast group
 */
static int RawipGroupJoin(
  SRawipSap* const pRawip,
  ui32 group,
  ui32 src
) {
  int iRet;
  SGroupMember* pGroup;

  TRACE( 9, ("RawipGroupJoin(%p,%08lx,%08lx)\n", pRawip,group,src));
  ASSERT( NULL != pRawip);

  iRet = RawipGroupFind( pRawip, group, src);
  if ( iRet >= 0)
    return 0;

  if ( pRawip->groups >= lengthof( pRawip->group) )
    {
    TRACE( 1, ("*** RawipGroupJoin() too many groups\n"));
    return SWS_ENOMEM;
    }

  if ( SWS_INADDR_ANY == src)
    {
    int len = sizeof( src);
    SWS_IpGetOption( &pRawip->ipOptions, SWS_IP_MULTICAST_IF, &src, &len);
    }

  pGroup = &pRawip->group[ pRawip->groups++];
  pGroup->addr = group;
  pGroup->src = src;

  iRet = SWS_IpGroupJoin( group, src);
  if ( iRet)
    --(pRawip->groups);

  return iRet;
  }


/*
 * Leave a multicast group
 */
static int RawipGroupLeave(
  SRawipSap* const pRawip,
  ui32 group,
  ui32 src
) {
  int iRet;

  TRACE( 9, ("RawipGroupLeave(%p,%08lx,%08lx)\n", pRawip,group,src));
  ASSERT( NULL != pRawip);

  iRet = RawipGroupFind( pRawip, group, src);
  if ( iRet < 0)
    return SWS_EINVAL;

  src = pRawip->group[ iRet].src;

  if ( iRet < (int)--(pRawip->groups))
    {
    /* Delete the group */
    memmove(
      &pRawip->group[ iRet],
      &pRawip->group[ iRet + 1],
      (pRawip->groups - iRet) * sizeof( pRawip->group[0])
    );
    }

  return SWS_IpGroupLeave( group, src);
  }


/*
 * Find a multicast group
 */
static int RawipGroupFind(
  SRawipSap* const pRawip,
  ui32 group,                         /* IN: group */
  ui32 src
) {
  size_t i;
  SGroupMember* pGroup;

  TRACE( 9, ("RawipGroupFind(%p,%08lx)\n", pRawip,group));
  ASSERT( NULL != pRawip);

  for ( pGroup = &pRawip->group[ i = 0]; i < pRawip->groups; ++i, ++pGroup)
    {
    if ( pGroup->addr == group
      && (SWS_INADDR_ANY == src || SWS_INADDR_ANY == pGroup->src || pGroup->src == src)
    ) {
      return (int)i;
      }
    }

  /* Not found */
  return -1;
  }
#endif /* OPT_IP_MULTICAST */

#else
char SWS_Rawip;
#endif /* OPT_PROTO_IPRAW */
/* End of file */
