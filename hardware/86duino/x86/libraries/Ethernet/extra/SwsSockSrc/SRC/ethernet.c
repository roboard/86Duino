/****************************************************************************
 *
 * File name   : ethernet.c
 * Function    : Ethernet family data link driver
 * Project     : SwsSock
 * Authors     : Lawrence Rust
 * Systems     : ANSI C.
 *
 ****************************************************************************
 *
 * Created by Lawrence Rust, Software Systems Consultants
 * lvr@softsystem.co.uk. Tel/Fax +33 5 49 72 79 63
 *
 * 10-Apr-01, LVR, Created.
 * 03-Oct-01, LVR, Fix TRACE warnings for ARM SDT 2.51 .
 *                 Add read of platform setting IP address etc in
 *                   EnetGetProperty.
 * 18-Oct-01, LVR, EnetRecvInd drops Rx packets if DPC Q > 2/3 full .
 * 29-Oct-02, LVR, Add dest address to EnetRecvIndDpc TRACE.
 *                 Add TRACE to EnetRecvIndDpc for unknow pkt type.
 *                 Add TRACE to ArpRecv for ignored pkt.
 *                 Add SWS_EnetSetFilter.
 * 06-Nov-02, LVR, Added SWS_kDatalinkProperty_Promiscuity to EnetSetProperty.
 * 24-Nov-02, LVR, Added SWS_kDatalinkProperty_HwAddress to EnetGetProperty.
 * 05-Dec-02, LVR, Added OPT_ENET_TXENABLE to disable Tx path.
 * 21-Dec-02, LVR, Added multicast support to EnetSend.
 * 12-Oct-04, LVR, Fixed re-initialisation.by initing all static data
 * 22-May-05, LVR, Added OPT_ARP
 * 10-Jun-05, LVR, Issue SWS_kEnetEventError in EnetSendAckDpc
 * 26-Jan-10, LVR, Init uSubnetMask to ~0UL, EnetGetProperty defaults to 0
 *
 * Comments:
 * --------
 * This file contains the Ethernet family data link driver
 *
 ****************************************************************************
 */

/* Compilation options */
#include "config.h"

#ifndef OPT_ENET_TXENABLE
#define OPT_ENET_TXENABLE 1           /* 0= disable transmit */
#endif

#ifndef OPT_ARP
#define OPT_ARP 1                     /* !0= enable ARP */
#endif

/* Exports */
#include "sws_enet.h"

#if OPT_DATALINK_ETHERNET
/* Imports */
#include <stddef.h>
#include <string.h>
#include <limits.h>

#include "sws_sock.h"  /* for SWS_INADDR_LOOPBACK */
#include "sws_dlnk.h"
#include "sws_ip.h"
#include "sws_buf.h"
#include "sws_msgq.h"
#include "sws_timr.h"
#include "sws_gef.h"
#include "platform.h"

#include "debug.h"


/*
 * Macros & Constants
 */

/* Make a Mac address from an IP multicast group address
 * From RFC1112 p6:
 * An IP host group address is mapped to an Ethernet multicast address
 * by placing the low-order 23-bits of the IP address into the low-order
 * 23 bits of the Ethernet multicast address 01-00-5E-00-00-00 (hex).
 */
#define MULTCAST2MAC( pMac, a) \
  (pMac)->addr[ 0] = 0x01, \
  (pMac)->addr[ 1] = 0x00, \
  (pMac)->addr[ 2] = 0x5e, \
  (pMac)->addr[ 3] = (ui8)(((a) >> 16) & 0x7f), \
  (pMac)->addr[ 4] = (ui8)(((a) >> 8) & 0xff), \
  (pMac)->addr[ 5] = (ui8)((a) & 0xff)

enum
  {
  kEthMinPacket = 60,     /* smallest legal size packet, no fcs */
  kEthMaxPacket = 1514,   /* largest legal size packet, no fcs */
  kEthMax802p3Len = 0x5dc /* (1500) Max value for 802.3 length field */
  };

/* Ethernet types from STD2 (RFC1700) p168 or www.iana.org/assignments/ethernet-numbers */
#define kEthIpType   ((ui16)0x800)    /* IPv4 */
#define kEthArpType  ((ui16)0x806)    /* ARP */
#define kEthRarpType ((ui16)0x8035)   /* RARP */
#define kEthSnmpType ((ui16)0x814c)   /* SNMP */

/* MAC header offsets */
COMPILATION_REQUIRES( 8 == CHAR_BIT);
COMPILATION_REQUIRES( sizeof( SWS_EnetMacAddress) == 6);
enum
  {
  kMacDstAddress  = 0,
  kMacSrcAddress  = kMacDstAddress + sizeof( SWS_EnetMacAddress),
  kMacType        = kMacSrcAddress + sizeof( SWS_EnetMacAddress),
  kMacSize        = kMacType + 2
  };

/* 802.3 header offsets */
enum
  {
  /* LLC, 3 bytes */
  kLlcDsap = kMacSize,
  kLlcSsap,
  kLlcControl,

  /* SNAP, 5 bytes */
  kSnapProtocol,
  kSnapType = kSnapProtocol + 4,

  kEth8023Prefix = kSnapType + 2 /* Total 802.3 header size */
  };

/* ARP hardware address types from RFC 1700 p163 */
enum
  {
  kArpHwEthernet = 1
  };

#if OPT_ARP
/* ARP packet field offsets from RFC 826 */
enum
  {
  kArpHrd = 0,        /* Hardware address type */
  kArpPro = 2,        /* Protocol address type  - kEthXxxType */
  kArpHln = 4,        /* Hardware address byte len */
  kArpPln = 5,        /* Protocol address byte len */
  kArpOp  = 6,        /* Opcode */
  kArpHrdSender = 8,  /* sender's h/w address */
  kArpProSender = kArpHrdSender + sizeof( SWS_EnetMacAddress),
  kArpHrdTarget = kArpProSender + sizeof( ui32),
  kArpProTarget = kArpHrdTarget + sizeof( SWS_EnetMacAddress),
  kArpLen = kArpProTarget + sizeof(ui32)
  };

/* ARP opcodes from RFC 826 */
enum
  {
  kArpOpRequest = 1,
  kArpOpReply = 2
  };

/* ARP cache entry state */
typedef enum EArpState
  {
  kArpInvalid = 0,  /* Not in use */
  kArpPending,      /* Unresolved */
  kArpDynamic,      /* Valid with timeout */
  kArpStatic        /* Valid, no timeout */
  } EArpState;
#endif


/*
 * Local data types
 */

/* Adapter instance data */
typedef struct SAdapter
  {
  ui32 uMagic;
#define ADAPTER_MAGIC SWS_MAKEFOURCC( 'e','n','e','t')
  ui16 uVersion;

  /* NIC driver dispatch table */
  SWS_FNPTR( SWS_FEnetSend) pfnSend;
  SWS_FNPTR( SWS_FEnetRecvAck) pfnRecvAck;
  SWS_FNPTR( SWS_FEnetGetMacAddr) pfnGetMacAddr;
  SWS_FNPTR( SWS_FEnetShutdown) pfnShutdown;
  SWS_FNPTR( SWS_FEnetIoctl) pfnIoctl;
  SWS_PTR( void) pvContext;           /* Context for dispatch fns */

  ui32 uIpAddress;
  ui32 uSubnetMask;
  ui32 uGatewayAddr;                  /* Adapter's default gateway */
  char szIdent[ 32];                  /* Ident string */

#if OPT_ENET_TXENABLE
  SWS_MsgqH hTxMsgQ;                  /* Send buffer Q */
  int iTxFull;                        /* TxQ is full */
  int iTxBusy;                        /* Tx is busy */
#endif

  Atomic_t aRxInds;                   /* No. pending Rx DPCs */

  SWS_DatalinkCallbacks callbacks;
  } SAdapter;

#if OPT_ARP
/* An ARP cache entry */
typedef struct SArp
  {
  struct SArp *pNext, *pPrev;         /* Doubly linked LRU list */

  EArpState state;
  ui32 uExpiryTime;
  ui32 uIpAddress;
  SWS_EnetMacAddress mac;

  /* Packet awaiting ARP reply */
  SWS_BufferH hBuffer;
  SWS_TimerH hTimer;
  } SArp;
#endif

#if OPT_ENET_TXENABLE
/* TransmitQ message */
typedef struct STxMsg
  {
  SWS_BufferH hBuffer;
  } STxMsg;
#endif


/*
 * Prototypes
 */
static void EnetRecvIp( SAdapter* const, SWS_PTR( void), size_t, int);

#if OPT_ENET_TXENABLE
static int EnetSendRaw( SAdapter* const, SWS_BufferH, SWS_PTR( const SWS_EnetMacAddress), unsigned);
static FPlatformDpc EnetSendAckDpc;
#endif

#if OPT_ARP
static void ArpRecv( SAdapter* const, SWS_PTR( void), size_t);
static SWS_FASTCALLDEF( SArp*, ArpFind( ui32 ipAddr));
static SArp* ArpAdd( EArpState, ui32, SWS_PTR( const SWS_EnetMacAddress));
static SWS_FASTCALLDEF( SArp*, ArpInvalidate( SArp*));
static SWS_FASTCALLDEF( void, ArpAddBack( SArp*));
static SWS_FASTCALLDEF( void, ArpAddFront( SArp*));
static SWS_FASTCALLDEF( SArp*, ArpRemove( SArp*));
#if OPT_ENET_TXENABLE
static int ArpSend( SAdapter* const, ui32, SWS_BufferH);
static SWS_FTimer ArpTimeout;
#endif
#endif

static SWS_FDatalinkGetProperty EnetGetProperty;
static SWS_FDatalinkSetProperty EnetSetProperty;
static SWS_FDatalinkSend EnetSend;
#if OPT_IP_MULTICAST
static SWS_FDatalinkSetMulticast EnetSetMulticast;
#endif

static SWS_FEnetSendAck EnetSendAck;
static SWS_FEnetRecvInd EnetRecvInd;
static FPlatformDpc EnetRecvIndDpc;


/*
 * Module data
 */
/* MAC broadcast address */
static const SWS_EnetMacAddress kMacBroadcast = {
  {0xff, 0xff, 0xff, 0xff, 0xff, 0xff}
};

/* Configuration */
#if OPT_ENET_TXENABLE
static int s_iSend802p3; /* !0 to send 802.3 frames */
static size_t s_uTxQLen, s_uTxQLo, s_uTxQHi;
#endif
#if OPT_ARP
static size_t s_uArpCacheSize;
static ui32 s_uArpTimeout;
static ui32 s_uArpReplyTime;
#endif
static size_t s_uRxQHi;

/* Adapter list */
static SAdapter s_adapterArray[ OPT_DATALINK_ETHERNET];
static unsigned s_uAdapters;

/* Stats */
#if OPT_ENET_TXENABLE
static Atomic_t s_aTxPackets, s_aTxBytes;
static Atomic_t s_aTxErrors;          /* Driver send errors */
static Atomic_t s_aTxDropped;         /* TxQ full */
#endif
static Atomic_t s_aRxPackets, s_aRxBytes;
static Atomic_t s_aRxErrors;          /* Out of DPC's, undersize packets */
static Atomic_t s_aRxDropped;         /* Received packets dropped by driver */
static Atomic_t s_aNoDpcs;            /* Out of DPCs in EnetSendAck & EnetRecvInd */

#if OPT_ARP
/* ARP LRU list */
static SArp *s_pArpHead, *s_pArpTail;
static SArp* s_pArpCache;

/* Stats */
static Atomic_t s_aArpTx;
static Atomic_t s_aArpRx;
#endif

/* Data link descriptor */
static const SWS_DatalinkDesc s_ethernet = {
  sizeof(s_ethernet),
  SWS_DATALINK_VERSION,
  &EnetSend,
  &EnetGetProperty,
  &EnetSetProperty,
  0, /* SWS_FDatalinkShutdown */
#if OPT_IP_MULTICAST
  &EnetSetMulticast
#else
  0
#endif
};


/*
 * Initialise this module
 */
int SWS_EnetInit( void)               /* Returns !0 on error */
  {
#if OPT_ARP
  unsigned u;
#endif

  TRACE( 9, ("SWS_EnetInit()\n"));
  ASSERT( PlatformIsCritical());

  /* Init static data */
  s_uAdapters = 0;
#if OPT_ARP
  s_pArpHead = s_pArpTail = NULL;
  s_pArpCache = NULL;
#endif

  /* Read config data */
  s_uRxQHi = (2 * (size_t)PlatformPropertyGetUint( PKEY_DPC_COUNT, OPT_DPCS )) / 3;

#if OPT_ENET_TXENABLE
  s_iSend802p3    = (int)PlatformPropertyGetUint(
    PKEY_ENET_802P3, OPT_ENET_8023 );

  s_uTxQLen       = (size_t)PlatformPropertyGetUint(
    PKEY_ENET_TXQ, OPT_ENET_TXQLEN );
  s_uTxQLo = s_uTxQLen / 3;
  s_uTxQHi = (2 * s_uTxQLen) / 3;
#endif

#if OPT_ARP
  s_uArpCacheSize = (size_t)PlatformPropertyGetUint(
    PKEY_ARP_COUNT, OPT_ARP_ENTRIES );

  s_uArpTimeout   = PlatformPropertyGetUint(
    PKEY_ARP_TIMEOUT, OPT_ARP_TIMEOUT ) * SWS_TIMER_TICKS;

  s_uArpReplyTime = (PlatformPropertyGetUint(
    PKEY_ARP_RESPONSE, OPT_ARP_RESPONSE ) * SWS_TIMER_TICKS) / 1000;

  /* Allocate ARP cache */
  ASSERT( NULL == s_pArpCache);
  s_pArpCache = (SArp*)calloc( s_uArpCacheSize, sizeof(SArp) );
  if ( NULL == s_pArpCache)
    {
    TRACE( 0, ("!!! SWS_EnetInit() Out of memory for %u ARP cache entries\n",
      s_uArpCacheSize));
    return SWS_ENOMEM;
    }

  /* Setup the ARP LRU list */
  for ( u = 0; u < s_uArpCacheSize; ++u)
    ArpAddBack( &s_pArpCache[ u]);
#endif

  return 0;
  }


/*
 * Finalise the Ethernet manager
 */
void SWS_EnetCleanup( void)
  {
  unsigned u;
  SAdapter* pAdapter;

  TRACE( 9, ("SWS_EnetCleanup()\n"));
  ASSERT( PlatformIsCritical());

  /* Shutdown all adapters */
  for (
    pAdapter = &s_adapterArray[ u = 0];
    u < s_uAdapters;
    ++u, ++pAdapter
  ) {
    ASSERT( ADAPTER_MAGIC == pAdapter->uMagic);

    if ( pAdapter->pfnShutdown)
      (*pAdapter->pfnShutdown)( pAdapter->pvContext);

    pAdapter->uMagic = 0;

#if OPT_ENET_TXENABLE
    /* Release transmitQ */
    if ( NULL != pAdapter->hTxMsgQ)
      {
      STxMsg msg;
      while ( !SWS_MsgqGet( pAdapter->hTxMsgQ, sizeof(msg), &msg))
        SWS_BufferRelease( msg.hBuffer);

      SWS_MsgqDestroy( pAdapter->hTxMsgQ);
      pAdapter->hTxMsgQ = NULL;
      }
#endif
    }
  s_uAdapters = 0;

#if OPT_ARP
  /* Clean out the ARP cache */
  while ( NULL != s_pArpHead)
    ArpRemove( ArpInvalidate( s_pArpHead));

  /* Release ARP cache */
  if ( NULL != s_pArpCache)
    {
    free( s_pArpCache);
    s_pArpCache = NULL;
    }
#endif

#if OPT_ENET_TXENABLE
  TRACE( 1, ("Enet: TxPackets:%u TxBytes:%u TxErrors:%u TxDropped:%u"
    " RxPackets:%u RxBytes:%u RxErrors:%u RxPacketsLost:%u NoDpcs:%u\n",
    AtomicRead( &s_aTxPackets), AtomicRead( &s_aTxBytes),
    AtomicRead( &s_aTxErrors), AtomicRead( &s_aTxDropped),
    AtomicRead( &s_aRxPackets), AtomicRead( &s_aRxBytes),
    AtomicRead( &s_aRxErrors), AtomicRead( &s_aRxDropped),
    AtomicRead( &s_aNoDpcs)
  ));
#else
  TRACE( 1, ("Enet: RxPackets:%u RxBytes:%u RxErrors:%u RxPacketsLost:%u NoDpcs:%u\n",
    AtomicRead( &s_aRxPackets), AtomicRead( &s_aRxBytes),
    AtomicRead( &s_aRxErrors), AtomicRead( &s_aRxDropped),
    AtomicRead( &s_aNoDpcs)
  ));
#endif
#if OPT_ARP
  TRACE( 1, ("ARP: RxPackets:%u TxPackets:%u\n",
    AtomicRead( &s_aArpRx), AtomicRead( &s_aArpTx) ));
#endif
  }


/*
 * Register an Ethernet adapter with the SwsSock stack
 */
SWS_EXPORT( int, SWS_EnetAddAdapter(  /* Returns !0 on error */
  SWS_PTR( const SWS_EnetDevice) pDesc, /* IN: Device descriptor */
  SWS_PTR( void) pvContext,           /* IN: Context for dispatch functions */
  SWS_PTR( SWS_EnetCallbacks) pUp     /* OUT: Up calls */
)){
  SAdapter* pAdapter;
  int iRet;

  TRACE( 9, ("SWS_EnetAddAdapter(%p,%p,%p)\n", pDesc,pvContext,pUp));
  ASSERT( NULL != pDesc);
  ASSERT( pDesc->pfnSend);
  ASSERT( pDesc->pfnGetMacAddr);
  ASSERT( pDesc->pfnRecvAck);

  if ( sizeof(*pDesc) > pDesc->uSize)
    {
    TRACE( 0, ("!!!SWS_EnetAddAdapter descriptor size too small\n"));
    return SWS_EINVAL;
    }

  if ( sizeof(*pUp) > pUp->uSize)
    {
    TRACE( 0, ("!!!SWS_EnetAddAdapter callback size too small\n"));
    return SWS_EINVAL;
    }

  if ( SWS_ENET_VERSION != pDesc->uVersion)
    {
    TRACE( 0, ("!!!SWS_EnetAddAdapter version mismatch\n"));
    return SWS_EINVAL;
    }

  if ( s_uAdapters >= lengthof( s_adapterArray) )
    {
    TRACE( 0, ("!!!SWS_EnetAddAdapter too many adapters\n"));
    return SWS_ETOOMANYREFS;
    }

#if OPT_DEMO
  {
  ui32 const uNow = PlatformTime();
  ui32 uVal = uNow;
  size_t len = sizeof( uVal);

  if ( !pDesc->pfnIoctl
    || (*pDesc->pfnIoctl)( pvContext, SWS_kDatalinkProperty_Private + 0x1234, &uVal, &len)
    || len != sizeof( uVal)
    || uVal != (uNow ^ 0xDEC0DEA5L)
  ) {
    return SWS_EINVAL;
    }
  }
#endif

  /* Add to adapter list */
  pAdapter = &s_adapterArray[ s_uAdapters++];
  pAdapter->uVersion = pDesc->uVersion;
  pAdapter->pfnSend = pDesc->pfnSend;
  pAdapter->pfnRecvAck = pDesc->pfnRecvAck;
  pAdapter->pfnGetMacAddr = pDesc->pfnGetMacAddr;
  pAdapter->pfnShutdown = pDesc->pfnShutdown;
  pAdapter->pfnIoctl = pDesc->pfnIoctl;
  pAdapter->pvContext = pvContext;
  strncpy( pAdapter->szIdent, pDesc->pszIdent, sizeof( pAdapter->szIdent) - 1);

#if OPT_ENET_TXENABLE
  pAdapter->iTxFull = 0;
  pAdapter->iTxBusy = 0;

  /* Create a transmit messageQ */
  pAdapter->hTxMsgQ = SWS_MsgqCreate( s_uTxQLen, sizeof( STxMsg));
  if ( NULL == pAdapter->hTxMsgQ)
    {
    --s_uAdapters;
    return SWS_ENOBUFS;
    }
#endif

  pAdapter->uIpAddress = SWS_INADDR_NONE; /* Unset */
  pAdapter->uSubnetMask = ~0UL; /* Unset */
  pAdapter->uGatewayAddr = SWS_INADDR_NONE; /* Unset */

  /* Register a data link */
  pAdapter->uMagic = ADAPTER_MAGIC;
  iRet = SWS_DatalinkRegister( &s_ethernet, pAdapter, &pAdapter->callbacks);
  if ( !iRet)
    {
    ASSERT( pAdapter->callbacks.pfnSendRdy);
    ASSERT( pAdapter->callbacks.pfnRecvInd);

    pUp->pvContext = pAdapter;
    pUp->pfnSendAck = &EnetSendAck;
    pUp->pfnRecvInd = &EnetRecvInd;
    }
  else
    {
    /* Cleanup on error */
#if OPT_ENET_TXENABLE
    SWS_MsgqDestroy( pAdapter->hTxMsgQ);
    pAdapter->hTxMsgQ = NULL;
#endif
    pAdapter->uMagic = 0;
    --s_uAdapters;
    }

  return iRet;
  }


/*
 * Get an interface property
 */
static SWS_CB( int, EnetGetProperty(
  SWS_PTR( void) pvContext,
  int type,
  SWS_PTR( void) pvBuf,
  SWS_PTR( size_t) puLen
)){
  SAdapter* const pAdapter = (SAdapter*)pvContext;
  size_t size;

  TRACE( 9, ("EnetGetProperty(%p,%d,%p,%p)\n", pvContext,type,pvBuf,puLen));
  ASSERT( pAdapter - s_adapterArray < (ptrdiff_t)s_uAdapters);
  ASSERT( ADAPTER_MAGIC == pAdapter->uMagic);
  ASSERT( NULL != pvBuf);
  ASSERT( NULL != puLen);

  switch ( type)
    {
  default:
    /* Pass down to adapter */
    if ( type >= SWS_kDatalinkProperty_Private && pAdapter->pfnIoctl)
      return (*pAdapter->pfnIoctl)( pAdapter->pvContext, type, pvBuf, puLen);

    /* Not implemented */
    TRACE( 3, ("EnetGetProperty %d not implemented\n", type));
    return SWS_EBADF;

  case SWS_kDatalinkProperty_Address:
    if ( *puLen < sizeof(ui32))
      {
      TRACE( 0, ("!!!EnetGetProperty buffer too small\n"));
      return SWS_EINVAL;
      }

    if ( SWS_INADDR_NONE == pAdapter->uIpAddress)
      {
      /* Get user configured IP address */
      pAdapter->uIpAddress = PlatformPropertyGetUint(
        pAdapter->szIdent, PVAL_IPADDRESS, SWS_INADDR_ANY);
      }

    *(SWS_PTR(ui32))pvBuf = pAdapter->uIpAddress;
    *puLen = sizeof(ui32);
    break;

  case SWS_kDatalinkProperty_SubnetMask:
    if ( *puLen < sizeof(ui32))
      {
      TRACE( 0, ("!!!EnetGetProperty buffer too small\n"));
      return SWS_EINVAL;
      }

    if ( ~0UL == pAdapter->uSubnetMask)
      {
      /* Get user configured subnet mask */
      pAdapter->uSubnetMask = PlatformPropertyGetUint(
        pAdapter->szIdent, PVAL_IPSUBNET, 0); /* 0= default for address class */
      }

    *(SWS_PTR(ui32))pvBuf = pAdapter->uSubnetMask;
    *puLen = sizeof(ui32);
    break;

  case SWS_kDatalinkProperty_Gateway:
    if ( *puLen < sizeof(ui32))
      {
      TRACE( 0, ("!!!EnetGetProperty buffer too small\n"));
      return SWS_EINVAL;
      }

    if ( SWS_INADDR_NONE == pAdapter->uGatewayAddr)
      {
      /* Get user configured gateway */
      pAdapter->uGatewayAddr = PlatformPropertyGetUint(
        pAdapter->szIdent, PVAL_IPGATEWAY, SWS_INADDR_NONE);
      }

    *(SWS_PTR(ui32))pvBuf = pAdapter->uGatewayAddr;
    *puLen = sizeof(ui32);
    break;

  case SWS_kDatalinkProperty_MTU:
    if ( *puLen < sizeof(size_t))
      {
      TRACE( 0, ("!!!EnetGetProperty buffer too small\n"));
      return SWS_EINVAL;
      }

#if OPT_ENET_TXENABLE
    if ( s_iSend802p3)
      *(SWS_PTR(size_t))pvBuf = kEthMaxPacket - kEth8023Prefix;
    else
#endif
      *(SWS_PTR(size_t))pvBuf = kEthMaxPacket - kMacSize;
    *puLen = sizeof(size_t);
    break;

  case SWS_kDatalinkProperty_Ident:
    size = strlen( pAdapter->szIdent);
    if ( *puLen <= size)
      {
      TRACE( 0, ("!!!EnetGetProperty buffer too small\n"));
      return SWS_EINVAL;
      }

    strcpy( (SWS_PTR(char))pvBuf, pAdapter->szIdent);
    *puLen = size;
    break;

  case SWS_kDatalinkProperty_HwType:
    size = sizeof( ui8);
    if ( *puLen < size)
      {
      TRACE( 0, ("!!!EnetGetProperty buffer too small\n"));
      return SWS_EINVAL;
      }

    *(SWS_PTR(ui8))pvBuf = kArpHwEthernet;
    *puLen = size;
    break;

  case SWS_kDatalinkProperty_HwAddress:
    size = sizeof( SWS_EnetMacAddress);
    if ( *puLen < size)
      {
      TRACE( 0, ("!!!EnetGetProperty buffer too small\n"));
      return SWS_EINVAL;
      }

    *puLen = size;
    ASSERT( pAdapter->pfnGetMacAddr);
    return (*pAdapter->pfnGetMacAddr)( pAdapter->pvContext, (SWS_PTR( SWS_EnetMacAddress)) pvBuf);

  case SWS_kDatalinkProperty_DHCP:
    if ( *puLen < sizeof(ui8))
      {
      TRACE( 0, ("!!!EnetGetProperty buffer too small\n"));
      return SWS_EINVAL;
      }

    *(SWS_PTR(ui8))pvBuf = (ui8)PlatformPropertyGetUint(
      pAdapter->szIdent, PVAL_DHCP, 0 );
    *puLen = sizeof(ui8);
    break;
    }

  return 0;
  }


/*
 * Set an interface property
 */
static SWS_CB( int, EnetSetProperty(
  SWS_PTR( void) pvContext,
  int type,
  SWS_PTR( const void) pvBuf,
  size_t len
)){
  SAdapter* const pAdapter = (SAdapter*)pvContext;
  int iRet = 0;

  TRACE( 9, ("EnetSetProperty(%p,%d,%p,%u)\n", pvContext,type,pvBuf,len));
  ASSERT( pAdapter - s_adapterArray < (ptrdiff_t)s_uAdapters);
  ASSERT( ADAPTER_MAGIC == pAdapter->uMagic);
  ASSERT( NULL != pvBuf);

  switch ( type)
    {
  default:
    /* Pass down to adapter */
    if ( type >= SWS_kDatalinkProperty_Private && pAdapter->pfnIoctl)
      return (*pAdapter->pfnIoctl)( pAdapter->pvContext, type, (SWS_PTR( void))pvBuf, &len);

    /* Not implemented */
    TRACE( 3, ("EnetSetProperty %d not implemented\n", type));
    return SWS_EBADF;

  case SWS_kDatalinkProperty_Address:
    if ( len < sizeof(ui32))
      {
      TRACE( 0, ("!!!EnetSetProperty buffer too small\n"));
      return SWS_EINVAL;
      }

    pAdapter->uIpAddress = *(SWS_PTR(const ui32))pvBuf;
    break;

  case SWS_kDatalinkProperty_Gateway:
    if ( len < sizeof(ui32))
      {
      TRACE( 0, ("!!!EnetSetProperty buffer too small\n"));
      return SWS_EINVAL;
      }

    pAdapter->uGatewayAddr = *(SWS_PTR(const ui32))pvBuf;
    break;

  case SWS_kDatalinkProperty_SubnetMask:
    if ( len < sizeof(ui32))
      {
      TRACE( 0, ("!!!EnetSetProperty buffer too small\n"));
      return SWS_EINVAL;
      }

    pAdapter->uSubnetMask = *(SWS_PTR(const ui32))pvBuf;
    break;

  case SWS_kDatalinkProperty_Promiscuity:
    if ( pAdapter->pfnIoctl)
      iRet = (*pAdapter->pfnIoctl)( pAdapter->pvContext, SWS_kEnetIoctl_Promiscuity, (SWS_PTR( void))pvBuf, &len);
    else
      {
      TRACE( 3, ("EnetSetProperty %d not implemented\n", type));
      return SWS_EBADF;
      }
    break;
    }

  return iRet;
  }


#if OPT_IP_MULTICAST
/*
 * Set the multicast list
 */
SWS_CB( int, EnetSetMulticast(
  SWS_PTR( void) pvContext,           /* IN: Client context */
  unsigned count,                     /* IN: No. entries */
  SWS_PTR( const ui32) pMlist         /* IN: Array of multicast addresses */
)){
  SAdapter* const pAdapter = (SAdapter*)pvContext;
  SWS_EnetMacAddress* pMac;
  size_t u;
  int iRet;

  TRACE( 9, ("EnetSetMulticast(%p,%u,%p)\n", pvContext,count,pMlist));

  if ( count > 0)
    {
    /* Allocate multicast list */
    pMac = (SWS_EnetMacAddress*) malloc( count * sizeof(SWS_EnetMacAddress) );
    if ( NULL == pMac)
      {
      TRACE( 0, ("!!!EnetSetProperty out of memory for multicast list\n"));
      return SWS_ENOMEM;
      }

    for ( u = 0; u < count; ++u)
      {
      MULTCAST2MAC( &pMac[ u], pMlist[ u] );
      }
    }
  else
    {
    pMac = NULL;
    }

  /* Send list to adapter */
  u = count * sizeof( SWS_EnetMacAddress);
  iRet = (*pAdapter->pfnIoctl)(
    pAdapter->pvContext, SWS_kEnetIoctl_MulticastList, (SWS_PTR( void))pMac, &u);

  if ( NULL != pMac)
    free( (void*)pMac);

  return iRet;
  }
#endif


/*
 * Send a packet
 * Single threaded
 */
static SWS_CB( int, EnetSend(
  void* pvContext,
  SWS_BufferH hBuffer,
  ui32 dst,                           /* IN: IP address of next hop */
  unsigned tos                        /* IN: Type Of Service SWS_IPTOS_xxx */
)){
  SAdapter* const pAdapter = (SAdapter*)pvContext;
  int iRet;
  (void)tos;

  TRACE( 9, ("EnetSend(%p,%p,%lx,%u)\n", pvContext, hBuffer, dst, tos));
  ASSERT( NULL != hBuffer);
  ASSERT( pAdapter - s_adapterArray < (ptrdiff_t)s_uAdapters);
  ASSERT( ADAPTER_MAGIC == pAdapter->uMagic);
  ASSERT( SWS_INADDR_ANY != dst);
  ASSERT( PlatformIsCritical());

  /* Is the destination this adapter? */
  if ( dst == pAdapter->uIpAddress
    && dst != pAdapter->uGatewayAddr
  ) {
    /* Yes, loopback the data */
    ASSERT( pAdapter->callbacks.pfnRecvInd);
    (*pAdapter->callbacks.pfnRecvInd)(
      pAdapter->callbacks.pvContext,
      hBuffer,
      0
    );
    iRet = 0;
    }
  else
#if OPT_ENET_TXENABLE
    {
    SWS_EnetMacAddress macDst;

    /* Lookup dst MAC address */
    if ( dst == pAdapter->uIpAddress)
      {
      /* Sending to self */
      ASSERT( pAdapter->pfnGetMacAddr);
      iRet = (*pAdapter->pfnGetMacAddr)( pAdapter->pvContext, &macDst);
      if ( iRet)
        return iRet;
      }
    else if ( (dst & ~pAdapter->uSubnetMask)
      == (SWS_INADDR_BROADCAST & ~pAdapter->uSubnetMask)
    ) {
      /* A broadcast address */
      memcpy( (void*)macDst.addr, (const void*)kMacBroadcast.addr, sizeof( macDst.addr) );
      }
    else if ( SWS_IN_MULTICAST( dst))
      {
      /* A multicast address */
      MULTCAST2MAC( &macDst, dst);
      }
#if OPT_ARP
    else
      {
      SArp* pArp;

      /* ARP only works on the local subnet */
      ASSERT( (pAdapter->uSubnetMask & dst)
        == (pAdapter->uSubnetMask & pAdapter->uIpAddress) );

      /* Lookup dst in the ARP cache */
      pArp = ArpFind( dst);
      if ( NULL != pArp)
        {
        /* Found it.  Is it resolved yet? */
        switch ( pArp->state)
          {
        case kArpPending:
          /* the ARP request is still pending */
#if 0
          /* Host is unreachable */
          ASSERT( NULL != pArp->hBuffer);
          return SWS_EHOSTUNREACH;
#else
          /* Discard older buffer */
          if ( NULL != pArp->hBuffer)
            {
            TRACE( 2, ("EnetSend() Discarding waiting buffer\n"));
            SWS_BufferRelease( pArp->hBuffer);
            AtomicIncrement( &s_aTxDropped);
            }

          pArp->hBuffer = hBuffer;
          SWS_BufferAddRef( hBuffer);
          return 0;
#endif

        case kArpStatic:
        case kArpDynamic:
          /* Valid ARP entry so move to front of LRU list */
          ArpRemove( pArp);
          ArpAddFront( pArp);

          macDst = pArp->mac;
          break;

        default:
          ASSERT( !"EnetSend() ArpFind returned invalid entry");
          return SWS_EHOSTUNREACH;
          }
        }
      else
        {
        /* Issue an ARP request */
        return ArpSend( pAdapter, dst, hBuffer);
        }
      }
#else
    else
      {
      TRACE( 2, ("EnetSend() ARP disabled\n"));
      return SWS_EHOSTUNREACH;
      }
#endif

    /* Send the packet */
    iRet = EnetSendRaw( pAdapter, hBuffer, &macDst, kEthIpType);
    }
#else
    {
    /* Tx disabled so just drop the packet */
    iRet = 0;
    }
#endif

  return iRet;
  }


#if OPT_ENET_TXENABLE
/*
 * Send an ethernet frame
 */
static int EnetSendRaw(
  SAdapter* const pAdapter,
  SWS_BufferH hBuffer,                /* IN: Buffer to send (no header) */
  SWS_PTR( const SWS_EnetMacAddress) pMacDst, /* IN: MAC address of target */
  unsigned type                       /* IN: Type of frame */
) {
  int iRet;
  SWS_PTR( void) pvHdr;
  SWS_size_t uLen;

  TRACE( 9, ("EnetSendRaw(%p,%p,%p,%u)\n", pAdapter, hBuffer, pMacDst,type));
  ASSERT( pAdapter - s_adapterArray < (ptrdiff_t)s_uAdapters);
  ASSERT( ADAPTER_MAGIC == pAdapter->uMagic);
  ASSERT( NULL != hBuffer);
  ASSERT( NULL != pMacDst);
  ASSERT( PlatformIsCritical());

  /* Ensure packet is <= max size */
  uLen = SWS_BufferSize( hBuffer);
  if ( uLen > kEthMaxPacket - kMacSize)
    {
    TRACE( 1, ("*** EnetSendRaw() packet too big\n"));
    return SWS_EMSGSIZE;
    }
  /* Ensure packet is >= min size */
  if ( (kEthMinPacket - kMacSize) > uLen)
    {
    /* Add padding */
    iRet = SWS_BufferAppendBack( hBuffer, (kEthMinPacket - kMacSize) - uLen );
    if ( iRet)
      return iRet;

    uLen = (kEthMinPacket - kMacSize);
    ASSERT( SWS_BufferSize( hBuffer) == uLen);
    }

  if ( s_iSend802p3)
    {
    /* Prepend Ethernet 802.3 header */
    iRet = SWS_BufferAppendFront( hBuffer, kEth8023Prefix );
    if ( iRet)
      return iRet;

    pvHdr = SWS_BufferGetFragment( hBuffer, 0, &uLen);
    ASSERT( NULL != pvHdr);
    ASSERT( uLen >= kEth8023Prefix );

    /* LLC */
    SWS_POKE8( pvHdr, kLlcDsap, 170);
    SWS_POKE8( pvHdr, kLlcSsap, 170);
    SWS_POKE8( pvHdr, kLlcControl, 3);

    /* SNAP */
    SWS_POKE32( pvHdr, kSnapProtocol, 0);
    SWS_POKE16( pvHdr, kSnapType, (ui16)type);

    type = (unsigned)uLen;
    }
  else
    {
    /* Prepend Ethernet II header */
    iRet = SWS_BufferAppendFront( hBuffer, kMacSize );
    if ( iRet)
      return iRet;

    pvHdr = SWS_BufferGetFragment( hBuffer, 0, &uLen);
    ASSERT( NULL != pvHdr);
    ASSERT( uLen >= kMacSize );
    }

  /* Setup MAC header */
  ASSERT( pAdapter->pfnGetMacAddr);
  iRet = (*pAdapter->pfnGetMacAddr)(
    pAdapter->pvContext,
    (SWS_PTR( SWS_EnetMacAddress))((SWS_PTR(char))pvHdr + kMacSrcAddress)
  );
  if ( iRet)
    {
    TRACE( 0, ("!!! EnetSendRaw() failed(%d) to get MAC address\n", iRet));
    return iRet;
    }
  memcpy(
    (SWS_PTR(char))pvHdr + kMacDstAddress,
    (SWS_PTR(const void))pMacDst,
    sizeof( SWS_EnetMacAddress)
  );
  SWS_POKE16( pvHdr, kMacType, (ui16)type);

  /* Call event filter */
  if ( 0 != (iRet = SWS_GefRaise( SWS_kGefEnetTx, hBuffer)))
    {
    TRACE( 4, ("EnetSendRaw() filtered %d\n", iRet));
    }
  /* Is a send currently pending? */
  else if ( !pAdapter->iTxBusy)
    {
    ASSERT( 0 == SWS_MsgqCount( pAdapter->hTxMsgQ));

    /* No, send the packet immediately */
    pAdapter->iTxBusy = 1;

    AtomicIncrement( &s_aTxPackets);
    AtomicAdd( &s_aTxBytes, (int)SWS_BufferSize( hBuffer) );

    /* Pass frame to driver to send */
    ASSERT( pAdapter->pfnSend);
    iRet = (*pAdapter->pfnSend)( pAdapter->pvContext, hBuffer);
    if ( -1 == iRet)
      {
      /* Pending */
      if ( s_uTxQLen > 0)
        iRet = 0;
      else
        pAdapter->iTxFull = 1;
      }
    else
      {
      /* Immediate completion */
      pAdapter->iTxBusy = 0;
      if ( iRet)
        {
        TRACE( 2, ("EnetSendRaw(%p) error %d\n", pAdapter, iRet));
        AtomicIncrement( &s_aTxErrors);
        SWS_GefRaise( SWS_kGefEnetError, iRet, "Tx error");
        }
      }
    }
  else
    {
    /* Queue the packet */
    STxMsg msg;

    TRACE( 3, ("EnetSendRaw(%p) TxBusy\n", pAdapter));
    SWS_BufferAddRef( hBuffer);

    /* Queue the request */
    msg.hBuffer = hBuffer;
    iRet = SWS_MsgqPut( pAdapter->hTxMsgQ, sizeof(msg), &msg);
    if ( iRet)
      {
      /* Output buffer full */
      TRACE( 1, ("*** EnetSendRaw(%p) outputQ full\n", pAdapter));
      ASSERT( pAdapter->iTxFull);
      AtomicIncrement( &s_aTxDropped);
      SWS_BufferRelease( hBuffer); /* Remove ref to buffer */
      }
    else if ( SWS_MsgqCount( pAdapter->hTxMsgQ) >= s_uTxQHi)
      {
      /* Q nearly full */
      TRACE( 3, ("EnetSendRaw(%p) outputQ hiwater\n", pAdapter));
      pAdapter->iTxFull = 1;
      iRet = -1; /* Pending */
      }
    }

  return iRet;
  }
#endif


/*
 * SWS_FEnetSend async completion
 * !!! CAUTION IRQ TIME !!!
 */
static SWS_CB( void, EnetSendAck(
  SWS_PTR( void) pvContext,           /* IN: Client context */
  int iErr                            /* IN: !0 if error */
)){
#if OPT_ENET_TXENABLE
  SAdapter* const pAdapter = (SAdapter*)pvContext;

  if ( ADAPTER_MAGIC == pAdapter->uMagic)
    {
    /* Request a DPC */
    if ( PlatformDpc( &EnetSendAckDpc, pAdapter, NULL, (ui32)iErr))
      {
      /* Out of DPC's so do callback immediately.
       * NB This may be unsafe for SWS_MsgqGet */
      AtomicIncrement( &s_aNoDpcs);
      EnetSendAckDpc( pAdapter, NULL, (ui32)iErr);
      }
    }
#else
  (void)pvContext;
  (void)iErr;
#endif
  }

#if OPT_ENET_TXENABLE
/* SendAck deferred procedure */
static SWS_CB( void, EnetSendAckDpc(
  SWS_PTR( void) pvContext,           /* IN: Client context */
  SWS_PTR( void) pvData,              /* IN: Not used */
  ui32 uErr                           /* IN: !0 if error */
)){
  SAdapter* const pAdapter = (SAdapter*)pvContext;
  STxMsg msg;
  (void)pvData;

  TRACE( 5, ("EnetSendAckDpc(%p,%p,%lu)\n", pvContext, pvData, uErr));
  ASSERT( NULL != pAdapter);
  ASSERT( ADAPTER_MAGIC == pAdapter->uMagic);
  ASSERT( PlatformIsCritical());

  if ( !pAdapter->iTxBusy)
    {
    TRACE( 1, ("*** EnetSendAckDpc(%p) nothing pending\n", pAdapter));
    return;
    }
  pAdapter->iTxBusy = 0;

  if ( uErr)
    {
    TRACE( 1, ("*** EnetSendAckDpc(%p) result %lu\n", pAdapter, uErr));
    AtomicIncrement( &s_aTxErrors);
    SWS_GefRaise( SWS_kGefEnetError, (int)uErr, "Tx ack error");
    }

  /* Send next queued packet */
  while ( !SWS_MsgqGet( pAdapter->hTxMsgQ, sizeof(msg), &msg))
    {
    int iRet;

    pAdapter->iTxBusy = 1;

    AtomicIncrement( &s_aTxPackets);
    AtomicAdd( &s_aTxBytes, (int)SWS_BufferSize( msg.hBuffer));

    /* Call the interface's 'send buffer' function */
    ASSERT( pAdapter->pfnSend);
    iRet = (*pAdapter->pfnSend)( pAdapter->pvContext, msg.hBuffer);
    SWS_BufferRelease( msg.hBuffer);
    if ( -1 == iRet)
      break; /* Send is pending so await ack */

    /* Immediate completion */
    pAdapter->iTxBusy = 0;
    if ( iRet)
      {
      TRACE( 2, ("EnetSendAckDpc(%p) send error %d\n", pAdapter,iRet));
      AtomicIncrement( &s_aTxErrors);
      SWS_GefRaise( SWS_kGefEnetError, iRet, "Tx error");
      }
    }

  if ( pAdapter->iTxFull
    && SWS_MsgqCount( pAdapter->hTxMsgQ) <= s_uTxQLo
  ) {
    pAdapter->iTxFull = 0;
    TRACE( 3, ("EnetSendAckDpc(%p) outputQ lowater\n", pAdapter));

    /* Issue a 'TxReady' indication to the IP layer */
    ASSERT( pAdapter->callbacks.pfnSendRdy);
    (*pAdapter->callbacks.pfnSendRdy)( pAdapter->callbacks.pvContext);
    }
  }
#endif


/*
 * Received data indication
 * !!! CAUTION IRQ TIME !!!
 */
static SWS_CB( int, EnetRecvInd(
  SWS_PTR( void) pvContext,           /* IN: Client context */
  SWS_PTR( void) pvData,              /* IN: Received data */
  size_t uLen                         /* IN: Data size */
)){
  SAdapter* const pAdapter = (SAdapter*)pvContext;

  if ( s_uAdapters > 0
    && ADAPTER_MAGIC == pAdapter->uMagic
  ) {
    if ( 0 != uLen)
      {
      /* Limit DPC's in case of Rx flood */
      if ( (size_t)AtomicIncrement( &pAdapter->aRxInds) <= s_uRxQHi
        /* Request a DPC */
        && !PlatformDpc(
          &EnetRecvIndDpc,
          pAdapter,
          (SWS_PTR( void))pvData,
          uLen
        )
      ) {
        return 0;
        }
      else
        {
       /* DPC queue full so release the data buffer immediately */
        AtomicDecrement( &pAdapter->aRxInds);
        AtomicIncrement( &s_aNoDpcs);
        return SWS_ENOBUFS;
        }
      }
    else
      {
      /* Driver is out of receive buffers */
      AtomicIncrement( &s_aRxDropped);
      return SWS_ENOBUFS;
      }
    }
  else
    return SWS_EBADF;
  }

/* Received data indication - deferred procedure callback */
static SWS_CB( void, EnetRecvIndDpc(
  SWS_PTR( void) pvContext,           /* IN: Client context */
  SWS_PTR( void) pvData,              /* IN: Received data */
  ui32 uLen                           /* IN: Data size */
)){
  SAdapter* const pAdapter = (SAdapter*)pvContext;

  TRACE( 5, ("EnetRecvIndDpc(%p,%p,%lu)\n",pvContext,pvData,uLen));
  ASSERT( NULL != pAdapter);
  ASSERT( ADAPTER_MAGIC == pAdapter->uMagic);
  ASSERT( PlatformIsCritical());

  if ( 0 == s_uAdapters
    || ADAPTER_MAGIC != pAdapter->uMagic
  ) {
    /* Adapter has been destroyed */
    TRACE( 1, ("*** EnetRecvIndDpc() Adapter destroyed\n"));
    return;
    }

  AtomicDecrement( &pAdapter->aRxInds);

  if ( uLen >= kMacSize)
    {
    int broadcast;

    AtomicIncrement( &s_aRxPackets);
    AtomicAdd( &s_aRxBytes, (int)uLen);

    ASSERT( NULL != pvData);
    TRACE( 4, ("EnetRecvIndDpc() %lu bytes type %#04x"
      " %02x-%02x-%02x-%02x-%02x-%02x to %02x-%02x-%02x-%02x-%02x-%02x\n",
      uLen,
      256 * ((SWS_PTR(ui8))pvData)[ kMacType]
        + ((SWS_PTR(ui8))pvData)[ kMacType +1],
      ((SWS_PTR(ui8))pvData)[ kMacSrcAddress +0],
      ((SWS_PTR(ui8))pvData)[ kMacSrcAddress +1],
      ((SWS_PTR(ui8))pvData)[ kMacSrcAddress +2],
      ((SWS_PTR(ui8))pvData)[ kMacSrcAddress +3],
      ((SWS_PTR(ui8))pvData)[ kMacSrcAddress +4],
      ((SWS_PTR(ui8))pvData)[ kMacSrcAddress +5],
      ((SWS_PTR(ui8))pvData)[ kMacDstAddress +0],
      ((SWS_PTR(ui8))pvData)[ kMacDstAddress +1],
      ((SWS_PTR(ui8))pvData)[ kMacDstAddress +2],
      ((SWS_PTR(ui8))pvData)[ kMacDstAddress +3],
      ((SWS_PTR(ui8))pvData)[ kMacDstAddress +4],
      ((SWS_PTR(ui8))pvData)[ kMacDstAddress +5]
    ));

    /* Check for broadcast - look at the multicast bit */
    broadcast = *((SWS_PTR(ui8))pvData) & 1;

    /* Call event filter */
    if ( !SWS_GefRaise( SWS_kGefEnetRx, pvData, uLen, broadcast))
      {
      unsigned type;
      size_t hdr;

      /* Get type of frame */
      SWS_PEEK16( type, pvData, 12);
      if ( type > kEthMax802p3Len)
        {
        hdr = kMacSize;
        }
      else if ( type < kEth8023Prefix)
        {
        hdr = kMacSize;
        }
      else
        {
        /* Ethernet 802.3 framing see RFC 1042 */
        unsigned dsap, ssap, control;
        ui32 protocol;

        /* LLC */
        SWS_PEEK8( dsap, pvData, kLlcDsap);
        SWS_PEEK8( ssap, pvData, kLlcSsap);
        SWS_PEEK8( control, pvData, kLlcControl);

        /* SNAP */
        SWS_PEEK32( protocol, pvData, kSnapProtocol);
        SWS_PEEK16( type, pvData, kSnapType);

        TRACE( 2, ("EnetRecvIndDpc 802.3 dsap:%u ssap:%u ctrl:%u ptcl:%#lx type:%u\n",
          dsap, ssap, control, protocol, type));

        if ( 170 != dsap || 170 != ssap || 3 != control || 0 != protocol)
          type = 0; /* Not an IP data packet */

        hdr = kEth8023Prefix;
        }

      switch ( type)
        {
#if OPT_ARP
      /*case kEthRarpType:*/
      case kEthArpType:
        ArpRecv( pAdapter, (SWS_PTR( char))pvData + hdr, (size_t)uLen - hdr);
        break;
#endif

      case kEthIpType:
        EnetRecvIp(
          pAdapter,
          (SWS_PTR( char))pvData + hdr,
          (size_t)uLen - hdr,
          broadcast
        );
        break;

      default:
        TRACE( 2, ("EnetRecvIndDpc() Type %#x len %u\n",type,(unsigned)uLen));
        break;
        }
      }
    else
      {
      TRACE( 4, ("EnetRecvIndDpc() filtered\n"));
      }
    }
  else
    {
    TRACE( 1, ("*** EnetRecvIndDpc() buffer too small\n"));
    AtomicIncrement( &s_aRxErrors);
    }

  /* Return data buffer to driver */
  ASSERT( pAdapter->pfnRecvAck);
  (*pAdapter->pfnRecvAck)( pAdapter->pvContext, pvData, (size_t)uLen);
  }


/*
 * Process a received IP packet
 */
static void EnetRecvIp(
  SAdapter* const pAdapter,
  SWS_PTR( void) pvData,              /* IN: Received data */
  size_t uLen,                        /* IN: Data size */
  int broadcast                       /* IN: Packet was broadcast */
) {
  SWS_BufferH hBuffer;

  TRACE( 9, ("EnetRecvIp(%p,%p,%u,%d)\n",
    pAdapter, pvData, uLen, broadcast));
  ASSERT( NULL != pAdapter);
  ASSERT( ADAPTER_MAGIC == pAdapter->uMagic);
  ASSERT( NULL != pvData);
  ASSERT( uLen > 0);
  ASSERT( PlatformIsCritical());

  /* Wrap the data in a 'shadow' buffer */
  hBuffer = SWS_BufferGetShadow( uLen, SWS_kBufferPrefix, pvData);
  if ( NULL != hBuffer)
    {
    /* Pass received data up to IP layer */
    ASSERT( pAdapter->callbacks.pfnRecvInd);
    (*pAdapter->callbacks.pfnRecvInd)(
      pAdapter->callbacks.pvContext,
      hBuffer,
      broadcast
    );

    /* Release buffer - copies the data if still referenced */
    SWS_BufferReleaseShadow( hBuffer);
    }
  }


#if OPT_ARP
/****************************************************************************
 * ARP
 ****************************************************************************/

/*
 * Process an ARP packet
 */
static void ArpRecv(
  SAdapter* const pAdapter,
  SWS_PTR( void) pvData,              /* IN: Received data */
  size_t uLen                         /* IN: Data size */
) {
  unsigned w, ln;
  ui32 src, dst;
  SArp* pArp;

  TRACE( 9, ("ArpRecv(%p,%p,%u)\n",pAdapter,pvData,uLen));
  ASSERT( NULL != pAdapter);
  ASSERT( ADAPTER_MAGIC == pAdapter->uMagic);
  ASSERT( NULL != pvData);
  ASSERT( PlatformIsCritical());

  if ( uLen < kArpLen)
    return;

  /* Parse the ARP header */
  SWS_PEEK16( w, pvData, kArpHrd); /* Hardware address space */
  if ( kArpHwEthernet != w)
    {
    TRACE( 1, ("*** ArpRecv unknown HW type %u\n",w));
    return;
    }

  SWS_PEEK16( w, pvData, kArpPro); /* Protocol address space */
  if ( kEthIpType != w)
    {
    TRACE( 1, ("*** ArpRecv unknown protocol type %u\n",w));
    return;
    }

  SWS_PEEK8( ln, pvData, kArpHln); /* Byte len of each hardware address */
  if ( sizeof( SWS_EnetMacAddress) != ln)
    {
    TRACE( 1, ("*** ArpRecv unknown HW len %u\n",ln));
    return;
    }

  SWS_PEEK8( ln, pvData, kArpPln); /* Byte len of each protocol address */
  if ( sizeof( ui32) != ln)
    {
    TRACE( 1, ("*** ArpRecv unknown protocol len %u\n",ln));
    return;
    }

  AtomicIncrement( &s_aArpRx);

  /* Is the sender's IP address in the ARP cache? */
  SWS_PEEK32( src, pvData, kArpProSender); /* Sender's IP address */
  pArp = ArpFind( src);
  if ( NULL != pArp)
    {
    /* Refresh the expiry time */
    pArp->uExpiryTime = PlatformTime() + s_uArpTimeout;

    switch ( pArp->state)
      {
    default:
      ASSERT( !"ArpRecv returned invalid entry");
      break;

    case kArpPending:
      /* Cancel reply timeout */
      if ( NULL != pArp->hTimer)
        {
        SWS_TimerRelease( pArp->hTimer);
        pArp->hTimer = NULL;
        }

      /* Update the sender's MAC address */
      memcpy(
        (SWS_PTR(void))pArp->mac.addr,
        (SWS_PTR( char))pvData + kArpHrdSender,
        sizeof( pArp->mac.addr)
      );
      pArp->state = kArpDynamic;

      TRACE( 2, ("ArpRecv() Resolved %08lx : %02x-%02x-%02x-%02x-%02x-%02x\n",
        pArp->uIpAddress,
        pArp->mac.addr[0],pArp->mac.addr[1],pArp->mac.addr[2],
        pArp->mac.addr[3],pArp->mac.addr[4],pArp->mac.addr[5]
      ));

      /* Is there a packet waiting to send? */
      if ( NULL != pArp->hBuffer)
        {
        /* Yes then send it */
        TRACE( 3, ("ArpRecv() Sending pending buffer\n"));
#if OPT_ENET_TXENABLE
        EnetSendRaw( pAdapter, pArp->hBuffer, &pArp->mac, kEthIpType);
#endif
        SWS_BufferRelease( pArp->hBuffer);
        pArp->hBuffer = NULL;
        }
      break;

    case kArpDynamic:
      /* Update the sender's MAC address */
      memcpy(
        (SWS_PTR( void))pArp->mac.addr,
        (SWS_PTR( char))pvData + kArpHrdSender,
        sizeof( pArp->mac.addr)
      );

      TRACE( 3, ("ArpRecv() Refreshed %08lx : %02x-%02x-%02x-%02x-%02x-%02x\n",
        pArp->uIpAddress,
        pArp->mac.addr[0],pArp->mac.addr[1],pArp->mac.addr[2],
        pArp->mac.addr[3],pArp->mac.addr[4],pArp->mac.addr[5]
      ));
      break;

    case kArpStatic:
      break;
      }
    }

  /* Is the packet for this adapter? */
  SWS_PEEK32( dst, pvData, kArpProTarget);
  if ( dst == pAdapter->uIpAddress)
    {
    /* Yes, it's a request or reply */
    if ( NULL == pArp)
      {
      /* Add to the ARP cache */
      pArp = ArpAdd(
        kArpDynamic,
        src,
        (SWS_PTR( SWS_EnetMacAddress))((SWS_PTR( char))pvData + kArpHrdSender)
      );

      TRACE( 2, ("ArpRecv() New IP %08lx = %02x-%02x-%02x-%02x-%02x-%02x\n",
        pArp->uIpAddress,
        pArp->mac.addr[0],pArp->mac.addr[1],pArp->mac.addr[2],
        pArp->mac.addr[3],pArp->mac.addr[4],pArp->mac.addr[5]
      ));
      }

    /* Check the opcode */
    SWS_PEEK16( w, pvData, kArpOp);
    if ( kArpOpRequest == w)
      {
      /* Reply required */
      SWS_BufferH hBuffer;
      SWS_PTR( SWS_EnetMacAddress) pMacSrc =
        (SWS_PTR( SWS_EnetMacAddress)) ((SWS_PTR( char))pvData + kArpHrdSender);

      TRACE( 2, ("ArpRecv() Request from %02x-%02x-%02x-%02x-%02x-%02x\n",
        pMacSrc->addr[0],pMacSrc->addr[1],pMacSrc->addr[2],
        pMacSrc->addr[3],pMacSrc->addr[4],pMacSrc->addr[5]
      ));

      /* Wrap the data in a 'shadow' buffer */
      hBuffer = SWS_BufferGetShadow( uLen, SWS_kBufferPrefix, pvData);
      if ( NULL != hBuffer)
        {
        /* Move src address to target */
        SWS_POKE32( pvData, kArpProTarget, src);
        memcpy(
          (SWS_PTR( char))pvData + kArpHrdTarget,
          (SWS_PTR( void))pMacSrc,
          sizeof( SWS_EnetMacAddress)
        );

        /* Set src address */
        SWS_POKE32( pvData, kArpProSender, pAdapter->uIpAddress);
        ASSERT( pAdapter->pfnGetMacAddr);
        (*pAdapter->pfnGetMacAddr)( pAdapter->pvContext, pMacSrc);

        /* Send the reply */
        AtomicIncrement( &s_aArpTx);
        SWS_POKE16( pvData, kArpOp, kArpOpReply);
#if OPT_ENET_TXENABLE
        EnetSendRaw(
          pAdapter,
          hBuffer,
          (SWS_PTR( SWS_EnetMacAddress))((SWS_PTR( char))pvData + kArpHrdTarget),
          kEthArpType
        );
#endif

        /* Release buffer - copies the data if still referenced */
        SWS_BufferReleaseShadow( hBuffer);
        }
      }
    }
  else
    {
    /* Not for this IP address */
    SWS_PEEK16( w, pvData, kArpOp); /* Check the opcode */
    TRACE( 3, ("ArpRecv() Discard Op:%u from %08lx for %08lx\n", w, src, dst));
    }
  }


#if OPT_ENET_TXENABLE
/*
 * Send an ARP request
 */
static int ArpSend(
  SAdapter* const pAdapter,           /* IN */
  ui32 ipAddr,                        /* IN: IP address to resolve */
  SWS_BufferH hBuffer                 /* IN: Buffer to send when resolved */
) {
  int iRet;
  SArp* pArp;
  SWS_BufferH hbArp;

  TRACE( 9, ("ArpSend(%p,%08lx,%p)\n", pAdapter, ipAddr, hBuffer));
  ASSERT( NULL != pAdapter);
  ASSERT( ADAPTER_MAGIC == pAdapter->uMagic);
  ASSERT( NULL != hBuffer);
  ASSERT( SWS_INADDR_ANY != ipAddr);
  ASSERT( (ipAddr & ~pAdapter->uSubnetMask) != (SWS_INADDR_BROADCAST & ~pAdapter->uSubnetMask) );
  ASSERT( PlatformIsCritical());

  /* Create an ARP entry */
  pArp = ArpAdd( kArpPending, ipAddr, &kMacBroadcast );

  /* Setup ARP reply timer */
  pArp->hTimer = SWS_TimerAlloc( &ArpTimeout, pArp);
  if ( NULL == pArp->hTimer)
    {
    ArpInvalidate( pArp);
    return SWS_ENOMEM;
    }

  /* Format an ARP packet */
  hbArp = SWS_BufferGet( kArpLen, SWS_kBufferPrefix);
  if ( NULL != hbArp)
    {
    SWS_PTR( void) pvData;
    SWS_size_t uLen;

    pvData = SWS_BufferGetFragment( hbArp, 0, &uLen);
    ASSERT( NULL != pvData);
    ASSERT( uLen == kArpLen );

    SWS_POKE16( pvData, kArpHrd, kArpHwEthernet);
    SWS_POKE16( pvData, kArpPro, kEthIpType);
    SWS_POKE8( pvData, kArpHln, sizeof( SWS_EnetMacAddress));
    SWS_POKE8( pvData, kArpPln, sizeof( ui32));
    SWS_POKE16( pvData, kArpOp, kArpOpRequest);

    (*pAdapter->pfnGetMacAddr)(
      pAdapter->pvContext,
      (SWS_PTR( SWS_EnetMacAddress))((SWS_PTR( char))pvData + kArpHrdSender)
    );
    SWS_POKE32( pvData, kArpProSender, pAdapter->uIpAddress);
    memset( (SWS_PTR( char))pvData + kArpHrdTarget, 0, sizeof( SWS_EnetMacAddress));
    SWS_POKE32( pvData, kArpProTarget, ipAddr);

    /* Save buffer to be sent in ARP cache entry */
    pArp->hBuffer = hBuffer;
    SWS_BufferAddRef( hBuffer);
    SWS_TimerStart( pArp->hTimer, s_uArpReplyTime);

    /* Send the ARP request packet */
    TRACE( 2, ("ArpSend() request %08lx\n", ipAddr));
    AtomicIncrement( &s_aArpTx);
    iRet = EnetSendRaw( pAdapter, hbArp, &kMacBroadcast, kEthArpType);
    if ( -1 == iRet)
      {
      /* Send pending */
      iRet = 0;
      }
    else if ( iRet)
      {
      /* Release resources on failure */
      ArpInvalidate( pArp);
      }

    SWS_BufferRelease( hbArp);
    }
  else
    {
    /* No buffers so release ARP entry */
    ArpInvalidate( pArp);
    iRet = SWS_ENOBUFS;
    }

  return iRet;
  }


/*
 * ARP request timeout
 */
static SWS_CB( void, ArpTimeout(
  SWS_TimerH hTimer,                  /* IN: Timer that expired */
  SWS_PTR( void) pv                   /* IN: User context */
)){
  SArp* const pArp = (SArp*)pv;
  (void)hTimer;

  TRACE( 9, ("ArpTimeout(%p,%p)\n",hTimer,pv));
  ASSERT( NULL != pArp);
  ASSERT( pArp->hTimer == hTimer);

  TRACE( 3, ("ArpTimeout() IP address %08lx\n",pArp->uIpAddress));

  /* Invalidate the ARP entry */
  ArpInvalidate( pArp);
  }
#endif /* OPT_ENET_TXENABLE */


/*
 * Add an IP address to the ARP cache
 */
static SArp* ArpAdd(
  EArpState eState,                   /* IN: type */
  ui32 ipAddr,                        /* IN: IP address */
  SWS_PTR( const SWS_EnetMacAddress) pMac /* IN */
) {
  SArp* pArp;

  TRACE( 9, ("ArpAdd(%08lx,%p)\n",ipAddr,pMac));
  ASSERT( PlatformIsCritical());

  /* Get oldest entry */
  pArp = ArpInvalidate( s_pArpTail);
  ArpRemove( pArp);

  /* Save IP & MAC address */
  pArp->mac = *pMac;
  pArp->uIpAddress = ipAddr;

  /* Set expiry time */
  if ( kArpDynamic == eState)
    pArp->uExpiryTime = PlatformTime() + s_uArpTimeout;
  else if ( kArpPending == eState)
    pArp->uExpiryTime = PlatformTime() + s_uArpReplyTime;

  pArp->state = eState;

  ArpAddFront( pArp);
  return pArp;
  }


/*
 * Lookup an IP address in the ARP cache
 */
static SWS_FASTCALL( SArp*, ArpFind(
  ui32 ipAddr
)){
  SArp* pArp;

  TRACE( 9, ("ArpFind(%08lx)\n",ipAddr));
  ASSERT( PlatformIsCritical());

  pArp = s_pArpHead;
  while ( NULL != pArp)
    {
    if ( ipAddr == pArp->uIpAddress)
      {
      /* Found a match */
      switch ( pArp->state)
        {
      case kArpStatic:
        /* Always valid */
        break;

      case kArpPending:
      case kArpDynamic:
        /* Is it too old? */
        if ( !SWS_TimerLT( pArp->uExpiryTime, PlatformTime()) )
          {
          TRACE( 4, ("ArpFind() found %08lx\n", pArp->uIpAddress));
          }
        else
          {
          /* Yes, so invalidate it and move to back of LRU list */
          TRACE( 2, ("ArpFind() %08lx expired\n", pArp->uIpAddress));
          ArpInvalidate( pArp);
          pArp = NULL;
          }
        break;

      default:
        /* Invalid */
        ArpInvalidate( pArp);
        pArp = NULL;
        break;
        }

      break;
      }

    if ( SWS_INADDR_ANY == pArp->uIpAddress)
      {
      /* End of valid entries */
      pArp = NULL;
      break;
      }

    pArp = pArp->pNext;
    }

  return pArp;
  }


/*
 * Invalidate an ARP entry
 */
static SWS_FASTCALL( SArp*, ArpInvalidate(
  SArp* pArp
)){
  TRACE( 9, ("ArpInvalidate(%p)\n",pArp));
  ASSERT( NULL != pArp);

  ArpRemove( pArp);

  pArp->uIpAddress = SWS_INADDR_ANY;
  pArp->state = kArpInvalid;

  if ( NULL != pArp->hTimer)
    {
    SWS_TimerRelease( pArp->hTimer);
    pArp->hTimer = NULL;
    }
  if ( NULL != pArp->hBuffer)
    {
    SWS_BufferRelease( pArp->hBuffer);
    pArp->hBuffer = NULL;
    }

  ArpAddBack( pArp);
  return pArp;
  }


/*
 * Add an ARP entry to the back of the LRU list
 */
static SWS_FASTCALL( void, ArpAddBack(
  SArp* pArp
)){
  TRACE( 9, ("ArpAddBack(%p)\n",pArp));
  ASSERT( NULL != pArp);
  ASSERT( NULL == pArp->pNext);
  ASSERT( NULL == pArp->pPrev);
  ASSERT( PlatformIsCritical());

  if ( NULL != s_pArpHead)
    {
    ASSERT( NULL != s_pArpTail);
    s_pArpTail->pNext = pArp;
    pArp->pPrev = s_pArpTail;
    s_pArpTail = pArp;
    }
  else
    {
    /* List is empty */
    ASSERT( NULL == s_pArpTail);
    pArp->pPrev = NULL;
    s_pArpHead = pArp;
    s_pArpTail = pArp;
    }
  }


/*
 * Add an ARP entry to the front of the LRU list
 */
static SWS_FASTCALL( void, ArpAddFront(
  SArp* pArp
)){
  TRACE( 9, ("ArpAddFront(%p)\n",pArp));
  ASSERT( NULL != pArp);
  ASSERT( NULL == pArp->pNext);
  ASSERT( NULL == pArp->pPrev);
  ASSERT( PlatformIsCritical());

  if ( NULL != s_pArpHead)
    {
    ASSERT( NULL != s_pArpTail);
    s_pArpHead->pPrev = pArp;
    pArp->pNext = s_pArpHead;
    s_pArpHead = pArp;
    }
  else
    {
    /* List is empty */
    ASSERT( NULL == s_pArpTail);
    pArp->pNext = NULL;
    s_pArpHead = pArp;
    s_pArpTail = pArp;
    }
  }


/*
 * Remove an ARP entry from the LRU list
 */
static SWS_FASTCALL( SArp*, ArpRemove(
  SArp* pArp
)){
  TRACE( 9, ("ArpRemove(%p)\n",pArp));
  ASSERT( NULL != pArp);
  ASSERT( PlatformIsCritical());

  if ( NULL != pArp->pNext)
    {
    pArp->pNext->pPrev = pArp->pPrev;
    }
  else
    {
    /* End of list */
    ASSERT( pArp == s_pArpTail);
    s_pArpTail = pArp->pPrev;
    }

  if ( NULL != pArp->pPrev)
    {
    pArp->pPrev->pNext = pArp->pNext;
    }
  else
    {
    /* Head of list */
    ASSERT( pArp == s_pArpHead);
    s_pArpHead = pArp->pNext;
    }

  pArp->pNext = NULL;
  pArp->pPrev = NULL;
  return pArp;
  }
#endif /* OPT_ARP */

#else
char SWS_Enet;
#endif /*OPT_DATALINK_ETHERNET*/
/* End of file */
