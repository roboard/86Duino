/****************************************************************************
 *
 * File name   : ip.c
 * Function    : Internet Protocol module
 * Project     : SwsSock
 * Authors     : Lawrence Rust
 * Systems     : ANSI C.
 *
 ****************************************************************************
 *
 * Created by Lawrence Rust, Software Systems Consultants
 * lvr@softsystem.co.uk. Tel/Fax +33 5 49 72 79 63
 *
 * 03-Apr-01, LVR, Created.
 * 11-Oct-01, LVR, Fix TRACE warnings for ARM SDT 2.51.
 *                 Remove read of platform setting IP address etc from
 *                   SWS_DatalinkRegister.
 * 17-Oct-01, LVR, IpSendRdy notifies all protocols of more Tx resources
 * 18-Oct-01, LVR, More checks after TxQ full in SWS_IpSend
 * 18-Dec-01, LVR, IpRecvInd uses gleaned address for uHostIp
 *                 IpRoute sends direct if no route or gateway
 * 07-Feb-02, LVR, SWS_IpDeregisterProtocol zeroes released slot in s_protocolArray
 *                 SWS_IpGetSrc rejects adapters with SWS_INADDR_ANY
 * 31-Oct-02, LVR, Added SWS_kErrorTxNoRoute.
 * 08-Nov-02, LVR, IpRoute sets MTU to sub-net MTU if routed via gateway.
 * 11-Nov-02, LVR, Added call to SWS_IcmpInit.
 * 09-Dec-02, LVR, Added OPT_IP_TXENABLE to allow disabling of Tx code
 * 20-Dec-02, LVR, Added routing table.
 * 22-Dec-02, LVR, Added IGMP and multicast.
 * 12-Oct-04, LVR, Fixed re-initialisation.by initing all static data
 * 09-Feb-05, LVR, RouteAdd doesn't find interfacves with no IP address
 * 05-May-05, LVR, IpFindProtocol matches 0 protocol to permit raw socket
 *                 to receive ICMP packets.
 * 10-Jun-05, LVR, Add OPT_IP_ROUTED and routing code
 * 03-Aug-05, LVR, Add DatalinkStatus, IpLinkStatus & RouteRemoveInterface
 * 26-Jan-10, LVR, InterfaceSetAddress & InterfaceSubnet set the subnet mask
 *                 in the adapter if a default for the IP is used.
 *
 * Comments:
 * --------
 * This file implements the Internet Protocol as described in RFC 791
 *
 ****************************************************************************
 */
/* Compilation options */
#include "config.h"

#ifndef OPT_IP_TXENABLE
#define OPT_IP_TXENABLE 1 /* 0= disable transmit */
#endif

#if OPT_IP_ROUTES
# ifndef OPT_IP_ROUTE_HASH
#  define OPT_IP_ROUTE_HASH 11 /* No. routing table hash entries */
# endif
#endif


/* Exports */
#include "sws_ip.h"
#include "sws_dlnk.h"
#include "ip.h"

/* Imports */
#include "platform.h"
#include <stddef.h>
#include <limits.h>
#include <string.h>

#include "sws_buf.h"
#include "sws_timr.h"
#include "sws_msgq.h"
#include "sws_sock.h"
#include "sws_gef.h"
#if OPT_DATALINK_P2P
#include "sws_p2p.h"
#endif
#if OPT_PROTO_ICMP
#include "sws_icmp.h"
#endif
#if OPT_PROTO_IGMP
#include "sws_igmp.h"
#endif
#if OPT_DATALINK_LOOPBACK
#include "loopback.h"
#endif
#if OPT_IP_ROUTES
#include "deque.h"
#endif
#include "platform.h"

#include "debug.h"


/*
 * Constants
 */
COMPILATION_REQUIRES( 8 == CHAR_BIT); /* For IP header manipulation */
COMPILATION_REQUIRES( sizeof(ui16) == 2); /* For IP header manipulation */
COMPILATION_REQUIRES( sizeof(ui32) == 4); /* For IP header manipulation */
COMPILATION_REQUIRES( OPT_TIMERS >= OPT_IP_REASSEMBLY_BUFFERS);

#define kMulticastGroup    0xe0000000UL
#define kGroupAllHosts     0xe0000001UL /* All hosts group */
#define kGroupAllRouters   0xe0000002UL /* All routers group */
#define kExperimentalGroup 0xf0000000UL


/*
 * Local data types
 */
#if OPT_IP_MULTICAST
/* Multicast group data */
typedef struct SGroup
  {
  int refs;                           /* Ref count per group */
#if OPT_PROTO_IGMP
  SWS_TimerH hTimer;                  /* Query timeout */
  int iReportSent;
#endif
  } SGroup;
#endif

/* Data link interface registration info */
typedef struct SInterface
  {
  ui32 uMagic;
#define INTERFACE_MAGIC SWS_MAKEFOURCC( 'i','f','c','e')
  ui16 uVersion;

  /* Dispatch table */
  SWS_PTR( void) pvContext;           /* Context for dispatch fn */
  SWS_FNPTR( SWS_FDatalinkSend) pfnSend; /* Required */
  SWS_FNPTR( SWS_FDatalinkGetProperty) pfnGetProperty; /* Required */
  SWS_FNPTR( SWS_FDatalinkSetProperty) pfnSetProperty; /* Optional */
  SWS_FNPTR( SWS_FDatalinkShutdown) pfnShutdown; /* Optional */
  SWS_FNPTR( SWS_FDatalinkSetMulticast) pfnSetMulticast; /* Optional */

  /* Cached property values */
  int isLinkUp;
  char szIdent[64];
  ui32 uIpAddr;                       /* Interface's IP address */
  ui32 uSubnetMask;
  ui32 uGatewayAddr;                  /* Interface's default gateway */
  size_t uMtu;                        /* Interface's MTU */

  /* Operational data */
  SWS_MsgqH hTxMsgQ;                  /* Send buffer Q */
  int iTxBusy;                        /* Tx is busy */
  int iTxFull;                        /* TxMsgQ is full */

#if OPT_PROTO_IGMP
  int iIgmpVersion;
#endif

#if OPT_IP_MULTICAST
  size_t groups;                      /* No. active multicast groups */
  SGroup group[ SWS_IP_MAX_MEMBERSHIPS]; /* Ref count per group */
  ui32 multicastList[ SWS_IP_MAX_MEMBERSHIPS]; /* Array of member multicast groups */
#endif
  } SInterface;

/* Protocol registration info */
typedef struct SProtocol
  {
  ui32 uMagic;
#define PROTOCOL_MAGIC SWS_MAKEFOURCC( 'p','r','t','c')
  ui16 uVersion;

  ui8 uProtocol;                      /* SWS_IPPROTO_xxx */
  ui8 reserved1;

  /* Dispatch table */
  SWS_PTR( void) pvContext;           /* Context for dispatch fn */
  SWS_FNPTR( SWS_FIpRecvInd) pfnRecvInd; /* Required */
#if OPT_PROTO_ICMP
  SWS_FNPTR( SWS_FIpIcmpInd) pfnIcmpInd; /* Optional */
#endif
  SWS_FNPTR( SWS_FIpSendInd) pfnSendInd; /* Optional */
  } SProtocol;


/* IpRecvInd callback context info */
typedef struct SRecvInfo
  {
  SInterface* pInterface;
  SWS_BufferH hBuffer;
  int broadcast;
  } SRecvInfo;

/* Protocol deregistration info */
typedef struct SProtocolInfo
  {
  SWS_PTR( const SWS_IpProtocolDesc) pDesc;
  SWS_PTR( void) pvContext;
  } SProtocolInfo;

#if OPT_IP_ROUTES
/* Routing table entry */
struct SWS_SRoute
  {
  SDeque link;        /* LRU list, newest first */
  ui32 dst;           /* Destinatio IP address */
  ui32 netMask;       /* Destination network address mask */
  ui32 nextHop;       /* IP address for next hop */
  SInterface* pInterface;
  ui32 timeLastUsed;
  struct {
    unsigned sticky;
    } flags;
  };

/* Convert an SDeque* to the containing SWS_SRoute* */
#define DEQ2ROUTE( p) SWS_CONTAINER( p, SWS_SRoute, link)
#endif

/* Fragmentation info */
typedef struct SFragInfo
  {
  size_t sent;
  SWS_BufferH hBuffer;
  SWS_SIpRouteInfo route;
  ui32 dst;
  ui16 id;
  char done;
  char df;
  char hdrincl;
#if OPT_IP_MULTICAST
  char multicast_loopback;
#endif
  ui8 tos;
  ui8 ttl;
  ui8 protocol;
  ui8 uOptLen;
  ui8 optBuf[ SWS_IPHDR_MAXLEN - SWS_IPHDR_OPTIONS];
  } SFragInfo;

#if OPT_IP_REASSEMBLY_BUFFERS
/* A reassembly buffer */
typedef struct SReassembly
  {
  struct SReassembly *pNext, *pPrev;  /* Doubly linked list */

  SWS_TimerH hTimer;                  /* Reassembly timeout */
  SWS_BufferH hBuffer;                /* Buffer containing sequence of holes */
  unsigned length;                    /* Size of datagram, 0 if unknown */
  int iFragZero;                      /* !0 if fragment zero received */
  ui16 hdr;                           /* Header length */
  ui16 holes;                         /* Buffer offset of first hole */

  /* Fragment matching info */
  ui32 src;
  ui32 dst;
  ui16 ident;
  ui8 protocol;
  } SReassembly;

/* Reassembly 'buffer hole' descriptor */
typedef struct SHole
  {
  ui16 size;                          /* Size of hole */
  ui16 next, prev;                    /* Buffer offset of next/prev hole */
  } SHole;
COMPILATION_REQUIRES( sizeof(SHole) <= SWS_IP_MIN_FRAG);

#endif /* OPT_IP_REASSEMBLY_BUFFERS*/


/*
 * Prototypes
 */
static int IpRoute( ui32, ui32, unsigned, SWS_SIpRouteInfo*);
#if OPT_IP_TXENABLE
static SWS_FASTCALLDEF( int, IpSendFragment( SInterface*, SFragInfo*));
#endif

static SWS_FASTCALLDEF( SInterface*, IpFindInterface( ui32 const));
static SWS_FASTCALLDEF( ui32, InterfaceAddress( SInterface* const));
#if !OPT_DHCP
static SWS_FASTCALLDEF( int, InterfaceSetAddress( SInterface* const, ui32));
#endif
static SWS_FASTCALLDEF( ui32, InterfaceSubnet( SInterface* const));
static SWS_FASTCALLDEF( ui32, InterfaceGateway( SInterface* const));
static INLINE( size_t, InterfaceMtu( SInterface* const));

#if OPT_IP_OPTIONS
static void IpTimestamp( void*, unsigned, ui32);
#endif

#if OPT_IP_REASSEMBLY_BUFFERS
static SReassembly* ReassemblyFind( ui32, ui32, unsigned, unsigned);
static SReassembly* ReassemblyGet( ui32, ui32, unsigned, unsigned);
static SWS_FASTCALLDEF( void, ReassemblyPut( SReassembly*));
static int ReassemblyAddFragment( SReassembly*, SWS_BufferH, unsigned, unsigned, int, unsigned);
static SWS_FASTCALLDEF( int, ReassemblyIsComplete( SReassembly*));
static SWS_FTimer ReassemblyTimeout;
#endif

#if OPT_PROTO_ENTRIES
static SProtocol* IpFindProtocol( unsigned const, SProtocol*, int);
static FPlatformCallback IpDeregisterProtocolSync;
static FPlatformCallback IpRegisterProtocolSync;
#endif

#if OPT_IP_TXENABLE
static FPlatformCallback IpSendSync;
#endif
static FPlatformCallback IpRecvInd;
static FPlatformCallback IpLinkStatus;
static FPlatformDpc IpSendRdy;
static int IpRecvBuiltin( unsigned,SWS_BufferH,size_t,ui32,ui32,ui32);

static SWS_FDatalinkSendRdy DatalinkSendRdy;
static SWS_FDatalinkRecvInd DatalinkRecvInd;
static SWS_FDatalinkStatus DatalinkStatus;

#if OPT_IP_ROUTES
static void RouteInit( void);
static SWS_SRoute* RouteAdd( SInterface*, ui32, ui32, unsigned, ui32, unsigned);
static void RouteAddInterface( SInterface*);
static void RouteRemoveInterface( SInterface*);
static SWS_FASTCALLDEF( SDeque*, RouteHashTable( ui32));
static FPlatformCallback IpRouteResetSync;

typedef enum { kMatchBest, kMatchExact, kMatchAny } EMatch;
static SWS_SRoute* RouteFind( ui32 src, ui32 dst, unsigned tos, EMatch mode);
#define RouteFindExact( src, dst, tos) RouteFind( src, dst, tos, kMatchExact)
#define RouteFindBest( src, dst, tos) RouteFind( src, dst, tos, kMatchBest)
#endif /*OPT_IP_ROUTES*/

#if OPT_IP_ROUTED
static int IpForward( SWS_BufferH, ui32, ui32);
#else
#define IpForward(h,src,dst) 0
#endif

#if OPT_IP_MULTICAST
static int IpGroupFind( SInterface*, ui32);
static int IpGroupAdd( SInterface*, ui32);
static int IpGroupRemove( SInterface*, SGroup*);

#if OPT_PROTO_IGMP
static SWS_FTimer IgmpTimeout;
static SWS_FASTCALLDEF( void, IgmpStartQueryTimer( SGroup*, unsigned tenths));
#endif
#endif


/*
 * Module data
 */
/* Configuration */
static ui8    s_ucDefaultTTL;         /* Default Time To Live */
#if OPT_IP_REASSEMBLY_BUFFERS
static ui8    s_uMinReassemblyTimeout;  /* Fragmented datagram reassembly timeout */
static ui16   s_uReassemblyMax;       /* Max size datagram that can be reassembled */
static size_t s_uReassemblyQLen;      /* Max no. fragmented datagrams */
#endif
static ui16   s_fragment;             /* Datagrams larger will be fragmented, 0= no limit */
static size_t s_uTxQLen, s_uTxQHi, s_uTxQLo;
#if !OPT_IP_DISCARD_NOPROTO
static int    s_ignore_noproto;       /* !0 to silently drop packets for unknown protocols */
#endif
#if OPT_PROTO_IGMP
static int    s_iIgmpVersion;
#endif
#if OPT_IP_ROUTED
static int    s_iRouted;              /* !0 to route incoming packets */
#endif


/* Routing callback */
static SWS_FNPTR( SWS_FIpRouter) s_pfnRoute;
static SWS_PTR( void) s_pvRouteCtx;

/* Stats */
static Atomic_t s_aTxPackets;         /* Total packets sent */
static Atomic_t s_aTxBytes;           /* Total bytes sent */
static Atomic_t s_aTxErrors;          /* Datalink send errors */
static Atomic_t s_aTxDropped;         /* No Tx packets dropped (TxQ full) */
static Atomic_t s_aRxPackets;         /* Total packets received */
static Atomic_t s_aRxBytes;           /* Total bytes received */
static Atomic_t s_aRxErrors;          /* No. packets received with bad IP header */
#if OPT_IP_REASSEMBLY_BUFFERS
static Atomic_t s_aRxDropped;         /* No Rx packets dropped (no buffers) */
#endif
static Atomic_t s_aRxNoProtocol;      /* No Rx packets discarded (no protocol) */

#if OPT_IP_REASSEMBLY_BUFFERS
/* Reassembly buffers */
static SReassembly* s_pReassembling;  /* Pending list */
static SReassembly* s_pReassemblyList; /* Free list */
static SReassembly* s_pReassemblyBuffers; /* Allocated memory */
#endif

/* Data link interfaces */
static SInterface s_interfaceArray[ OPT_DATALINK_ENTRIES ? OPT_DATALINK_ENTRIES : 1];
static unsigned s_uInterfaces;

#if OPT_PROTO_ENTRIES
/* Protocols */
static SProtocol s_protocolArray[ OPT_PROTO_ENTRIES];
static unsigned s_uProtocols;
#endif

#if OPT_IP_ROUTES
static size_t s_uRouteCacheSize;      /* Routing table size */
static SWS_SRoute* s_pRouteCache;    /* -> routing table */
static SDeque s_routeFree;            /* Free list */
static SDeque s_routeUsed[ OPT_IP_ROUTE_HASH]; /* Active route hash table */
#endif


/****************************************************************************
 * IP (network) layer
 ****************************************************************************/

/*
 * Initialise this module
 * Single threaded
 */
int SWS_IpInit( void)                 /* Returns !0 on error */
  {
#if OPT_IP_REASSEMBLY_BUFFERS
  size_t u;
  SReassembly* pReassembly;
#endif

  TRACE( 9, ("SWS_IpInit()\n"));
  ASSERT( PlatformIsCritical());

  /* Init static data */
#if OPT_IP_REASSEMBLY_BUFFERS
  s_pReassemblyList = s_pReassembling = NULL;
#endif
  s_uInterfaces = 0;
  memset( s_interfaceArray, 0, sizeof( s_interfaceArray) );
#if OPT_PROTO_ENTRIES
  s_uProtocols = 0;
  memset( s_protocolArray, 0, sizeof( s_protocolArray) );
#endif
#if OPT_IP_ROUTES
  s_pRouteCache = NULL;
#endif
  s_pfnRoute = 0;
  s_pvRouteCtx = NULL;

  /* Read config data */
  s_ucDefaultTTL = (ui8)PlatformPropertyGetUint( PKEY_IP_TTL, OPT_IP_TTL );
#if OPT_IP_REASSEMBLY_BUFFERS
  s_uMinReassemblyTimeout = (ui8)PlatformPropertyGetUint(
    PKEY_IP_REASSEMBLY_TIMEOUT, OPT_IP_REASSEMBLY_TIMEOUT );
  s_uReassemblyQLen = (size_t)PlatformPropertyGetUint(
    PKEY_IP_REASSEMBLY, OPT_IP_REASSEMBLY_BUFFERS );
  s_uReassemblyMax = (ui16)PlatformPropertyGetUint(
    PKEY_IP_REASSEMBLY_MAX, OPT_IP_REASSEMBLY_MAX );
#endif
  s_fragment = (ui16)PlatformPropertyGetUint(
    PKEY_IP_MAXFRAG, OPT_IP_MAX_FRAGMENT );
  if ( 0 == s_fragment || s_fragment >= SWS_IP_MAXLEN)
    s_fragment = SWS_IP_MAXLEN;
  else if ( s_fragment < SWS_IP_MIN_FRAG + SWS_IPHDR_MINLEN)
    s_fragment = SWS_IP_MIN_FRAG + SWS_IPHDR_MINLEN;

  s_uTxQLen = (size_t)PlatformPropertyGetUint( PKEY_IP_TXQ, OPT_IP_TXQLEN );
  s_uTxQHi = (2 * s_uTxQLen) / 3;
  s_uTxQLo = s_uTxQLen / 3;
#if !OPT_IP_DISCARD_NOPROTO
  s_ignore_noproto = (int)PlatformPropertyGetUint(
    PKEY_IP_NOPROTO, OPT_IP_DISCARD_NOPROTO );
#endif
#if OPT_PROTO_IGMP
  s_iIgmpVersion = (int)PlatformPropertyGetUint(
    PKEY_IGMP_VERSION, OPT_PROTO_IGMP );
#endif
#if OPT_IP_ROUTED
  s_iRouted = (int)PlatformPropertyGetUint( PKEY_IP_ROUTED, 0);
#endif

#if OPT_IP_REASSEMBLY_BUFFERS
  /* Allocate reassembly buffers */
  s_pReassemblyBuffers = (SReassembly*)calloc(
    s_uReassemblyQLen,
    sizeof(SReassembly)
  );
  if ( NULL == s_pReassemblyBuffers)
    {
    TRACE( 0, ("!!! SWS_IpInit() Out of memory for reassembly buffers\n"));
    return SWS_ENOMEM;
    }

  /* Create reassembly buffer free list */
  for (
    pReassembly = s_pReassemblyBuffers, u = 0;
    u < s_uReassemblyQLen;
    ++u, ++pReassembly
  ) {
    /* Add to free list */
    pReassembly->pNext = s_pReassemblyList;
    s_pReassemblyList = pReassembly;
    }
#endif

#if OPT_IP_ROUTES
  s_uRouteCacheSize = (size_t)PlatformPropertyGetUint(
    PKEY_IP_ROUTES, OPT_IP_ROUTES );
  if ( 0 == s_uRouteCacheSize)
    s_uRouteCacheSize = OPT_IP_ROUTES;

  /* Allocate the route cache */
  ASSERT( NULL == s_pRouteCache);
  s_pRouteCache = (SWS_SRoute*)calloc( s_uRouteCacheSize, sizeof(SWS_SRoute) );
  if ( NULL == s_pRouteCache)
    {
    TRACE( 0, ("!!! SWS_IpInit() Out of memory for route cache\n"));
#if OPT_IP_REASSEMBLY_BUFFERS
    free( s_pReassemblyBuffers);
    s_pReassemblyBuffers = NULL;
#endif
    return SWS_ENOMEM;
    }

  RouteInit();
#endif

#if OPT_PROTO_ICMP
  /* Initialise the ICMP module */
  SWS_IcmpInit();
#endif

#if OPT_PROTO_IGMP
  /* Initialise the IGMP module */
  SWS_IgmpInit();
#endif

#if OPT_DATALINK_LOOPBACK
  /* Initialise the loopback interface */
  SWS_LoopbackInit();
#endif

  return 0;
  }


/*
 * Finalise this module
 * Single threaded
 */
void SWS_IpCleanup( void)
  {
  SInterface* pInterface;
  unsigned u;

  TRACE( 9, ("SWS_IpCleanup()\n"));
  ASSERT( PlatformIsCritical());

  /* Shutdown datalink interfaces */
  for (
    pInterface = &s_interfaceArray[ u = 0];
    u < s_uInterfaces;
    ++u, ++pInterface
  ) {
    ASSERT( INTERFACE_MAGIC == pInterface->uMagic);

#if OPT_IP_MULTICAST && OPT_PROTO_IGMP
    {
    unsigned i;
    for ( i = 0; i < pInterface->groups; ++i)
      {
      /* Dispose of group resources */
      if ( NULL != pInterface->group[ i].hTimer)
        SWS_TimerRelease( pInterface->group[ i].hTimer);
      }
    }
#endif

    if ( pInterface->pfnShutdown)
      (*pInterface->pfnShutdown)( pInterface->pvContext);

    pInterface->uMagic = 0;

    if ( NULL != pInterface->hTxMsgQ)
      {
      SFragInfo frag;
      while ( !SWS_MsgqGet( pInterface->hTxMsgQ, sizeof(frag), &frag))
        SWS_BufferRelease( frag.hBuffer);

      SWS_MsgqDestroy( pInterface->hTxMsgQ);
      pInterface->hTxMsgQ = NULL;
      }
    }
  s_uInterfaces = 0;

#if OPT_IP_ROUTES
  /* Dispose of the route cache */
  if ( NULL != s_pRouteCache)
    {
    free( s_pRouteCache);
    s_pRouteCache = NULL;
    }
#endif

#if OPT_IP_REASSEMBLY_BUFFERS
  /* Discard datagrams pending reassembly */
  while ( NULL != s_pReassembling)
    ReassemblyPut( s_pReassembling);

  /* Release reassembly heap */
  if ( NULL != s_pReassemblyBuffers)
    {
    free( s_pReassemblyBuffers);
    s_pReassemblyBuffers = NULL;
    }
#endif

#if OPT_PROTO_IGMP
  /* Finalise the IGMP module */
  SWS_IgmpFinal();
#endif

#if OPT_PROTO_ICMP
  /* Finalise the ICMP module */
  SWS_IcmpFinal();
#endif

#if OPT_IP_REASSEMBLY_BUFFERS
#define FMT_RASFAIL " Reassembly failures:%u"
#define VAL_RASFAIL , AtomicRead( &s_aRxDropped)
#else
#define FMT_RASFAIL
#define VAL_RASFAIL
#endif

  TRACE( 1, ("IP: TxPackets:%u TxBytes:%u TxErrors:%u TxDropped:%u"
    " RxPackets:%u RxBytes:%u RxErrors:%u RxNoProtocol:%u" FMT_RASFAIL "\n",
    AtomicRead( &s_aTxPackets), AtomicRead( &s_aTxBytes),
    AtomicRead( &s_aTxErrors), AtomicRead( &s_aTxDropped),
    AtomicRead( &s_aRxPackets), AtomicRead( &s_aRxBytes),
    AtomicRead( &s_aRxErrors), AtomicRead( &s_aRxNoProtocol) VAL_RASFAIL
  ));
  }


#if OPT_PROTO_ENTRIES
/*
 * Search for a given protocol
 * Single threaded
 */
static SProtocol* IpFindProtocol(
  unsigned const uProtocol,
  SProtocol* pProtocol,
  int isWild
) {
  TRACE( 9, ("IpFindProtocol(%u,%p,%d)\n", uProtocol,pProtocol,isWild));
  ASSERT( PlatformIsCritical());

  if ( NULL == pProtocol)
    pProtocol = &s_protocolArray[0];
  else
    ++pProtocol;

  while ( pProtocol - s_protocolArray < (ptrdiff_t)s_uProtocols)
    {
    ASSERT( PROTOCOL_MAGIC == pProtocol->uMagic);
    if ( uProtocol == pProtocol->uProtocol)
      return pProtocol;
    if ( isWild) switch ( pProtocol->uProtocol)
      {
    case SWS_IPPROTO_IP:
    case SWS_IPPROTO_RAW:
      return pProtocol;
    default:
      break;
      }

    ++pProtocol;
    }

  return NULL;
  }


/*
 * Register a protocol
 */
SWS_EXPORT( int, SWS_IpRegisterProtocol( /* Returns !0 on error */
  SWS_PTR( const SWS_IpProtocolDesc) pDesc, /* IN: Transport descriptor */
  SWS_PTR( void) pvContext            /* IN: Client context */
)){
  SProtocolInfo info;

  TRACE( 9, ("SWS_IpRegisterProtocol(%p,%p)\n", pDesc, pvContext));
  ASSERT( NULL != pDesc);
  ASSERT( pDesc->pfnRecvInd);

  if ( sizeof(*pDesc) > pDesc->uSize)
    {
    TRACE( 0, ("!!!SWS_IpRegisterProtocol descriptor size too small\n"));
    return SWS_EINVAL;
    }

  if ( SWS_PROTOCOL_VERSION != pDesc->uVersion)
    {
    TRACE( 0, ("!!!SWS_IpRegisterProtocol version incompatible\n"));
    return SWS_EINVAL;
    }

  if ( s_uProtocols >= lengthof( s_protocolArray) )
    {
    TRACE( 0, ("!!!SWS_IpRegisterProtocol too many data links\n"));
    return SWS_ETOOMANYREFS;
    }

  info.pDesc = pDesc;
  info.pvContext = pvContext;
  return PlatformSynchronise( &IpRegisterProtocolSync, &info);
  }

static SWS_CB( int, IpRegisterProtocolSync( SWS_PTR( void) pv ))
  {
  SProtocolInfo* const pInfo = (SProtocolInfo*)pv;
  SProtocol* pProtocol;

  /* Add to protocol list */
  ASSERT( 0 == s_protocolArray[ s_uProtocols].uMagic);
  pProtocol = &s_protocolArray[ s_uProtocols++];

  pProtocol->uVersion = pInfo->pDesc->uVersion;
  pProtocol->uProtocol = pInfo->pDesc->uProtocol;
  pProtocol->pfnRecvInd = pInfo->pDesc->pfnRecvInd;
#if OPT_PROTO_ICMP
  pProtocol->pfnIcmpInd = pInfo->pDesc->pfnIcmpInd;
#endif
  pProtocol->pfnSendInd = pInfo->pDesc->pfnSendInd;
  pProtocol->pvContext = pInfo->pvContext;
  pProtocol->uMagic = PROTOCOL_MAGIC;

  return 0;
  }


/*
 * De-register a protocol
 */
SWS_EXPORT( int, SWS_IpDeregisterProtocol( /* Returns !0 on error */
  SWS_PTR( const SWS_IpProtocolDesc) pDesc, /* IN: Transport descriptor */
  SWS_PTR( void) pvContext            /* IN: Client context */
)){
  SProtocolInfo info;

  TRACE( 9, ("SWS_IpDeregisterProtocol(%p,%p)\n", pDesc, pvContext));
  ASSERT( NULL != pDesc);
  ASSERT( pDesc->pfnRecvInd);

  if ( sizeof(*pDesc) > pDesc->uSize)
    {
    TRACE( 0, ("!!!SWS_IpDeregisterProtocol descriptor size too small\n"));
    return SWS_EINVAL;
    }

  if ( SWS_PROTOCOL_VERSION != pDesc->uVersion)
    {
    TRACE( 0, ("!!!SWS_IpDeregisterProtocol version incompatible\n"));
    return SWS_EINVAL;
    }

  info.pDesc = pDesc;
  info.pvContext = pvContext;
  return PlatformSynchronise( &IpDeregisterProtocolSync, &info);
  }

static SWS_CB( int, IpDeregisterProtocolSync( SWS_PTR( void) pv ))
  {
  SProtocol* pProtocol;
  SProtocolInfo* const pInfo = (SProtocolInfo*)pv;

  /* Locate registration */
  for (
    pProtocol = IpFindProtocol( pInfo->pDesc->uProtocol, NULL, 0);
    NULL != pProtocol;
    pProtocol = IpFindProtocol( pInfo->pDesc->uProtocol, pProtocol, 0)
  ) {
    if ( pProtocol->pvContext == pInfo->pvContext
      && pProtocol->pfnRecvInd == pInfo->pDesc->pfnRecvInd
#if OPT_PROTO_ICMP
      && pProtocol->pfnIcmpInd == pInfo->pDesc->pfnIcmpInd
#endif
    ) {
      /* Move later protocols down */
      while ( &pProtocol[1] - s_protocolArray < (ptrdiff_t)s_uProtocols)
        {
        pProtocol[0] = pProtocol[1];
        ++pProtocol;
        }

      --s_uProtocols;
      memset( &s_protocolArray[ s_uProtocols], 0, sizeof( s_protocolArray[0]));

      return 0;
      }
    }

  TRACE( 1, ("***SWS_IpDeregisterProtocol not known\n"));
  return SWS_EINVAL;
  }
#endif /* OPT_PROTO_ENTRIES */


/*
 * Get/Set SWS_SOL_IP options
 */
SWS_EXPORT( int, SWS_IpSetOption(
  SWS_PTR( SWS_IpOptions) pIpOpts,    /* OUT: -> option data */
  int opt,                            /* IN: option SWS_IP_xxx */
  SWS_PTR( const void) pVal,          /* IN: -> value */
  int len                             /* IN: length of value */
)){
  TRACE( 9, ("SWS_IpSetOption(%p,%d,%p,%d)\n", pIpOpts,opt,pVal,len));
  ASSERT( NULL != pIpOpts);
  ASSERT( NULL != pVal);
  ASSERT( len >= 0);

  switch ( opt)
    {
  case SWS_IP_OPTIONS:
    if ( len > (int)sizeof( pIpOpts->opts))
      return SWS_EFAULT;

    memcpy( pIpOpts->opts, pVal, (size_t)len);
    pIpOpts->uOptLen = (ui8)len;
    break;

  case SWS_IP_HDRINCL:
    pIpOpts->flags.hdrincl = 0 != *(SWS_PTR(const int))pVal;
    break;

  case SWS_IP_TOS:
    pIpOpts->tos = (ui8)*(SWS_PTR(const int))pVal;
    break;

  case SWS_IP_TTL:
    pIpOpts->ttl = (ui8)*(SWS_PTR(const int))pVal;
    break;

  case SWS_IP_DONTFRAGMENT:
    pIpOpts->flags.df = 0 != *(SWS_PTR(const int))pVal;
    break;

#if OPT_IP_MULTICAST
  case SWS_IP_MULTICAST_IF:
    pIpOpts->multicast_if = SWS_ntohl(
      ((SWS_PTR( const struct SWS_in_addr)) pVal)->SWS_s_addr);
    break;

  case SWS_IP_MULTICAST_TTL:
    pIpOpts->multicast_ttl = *(SWS_PTR(const SWS_u_char))pVal;
    break;

  case SWS_IP_MULTICAST_LOOP:
    pIpOpts->flags.multicast_loop = *(SWS_PTR(const SWS_u_char))pVal;
    break;
#endif

  default:
    return SWS_EOPNOTSUPP;
    }

  return 0;
  }


SWS_EXPORT( int, SWS_IpGetOption(
  SWS_PTR( const SWS_IpOptions) pIpOpts, /* IN: -> option data */
  int opt,                            /* IN: option SWS_IP_xxx */
  SWS_PTR( void) pVal,                /* OUT: -> value */
  SWS_PTR( int) pLen                  /* I/O: length of value */
)){
  TRACE( 9, ("SWS_IpGetOption(%p,%d,%p,%p)\n", pIpOpts,opt,pVal,pLen));
  ASSERT( NULL != pIpOpts);
  ASSERT( NULL != pVal);
  ASSERT( NULL != pLen);

  switch ( opt)
    {
  case SWS_IP_OPTIONS:
    if ( *pLen < pIpOpts->uOptLen)
      return SWS_EFAULT;

    memcpy( pVal, pIpOpts->opts, pIpOpts->uOptLen);
    *pLen = (int)(pIpOpts->uOptLen);
    break;

  case SWS_IP_HDRINCL:
    if ( *pLen < (int)sizeof(int))
      return SWS_EFAULT;

    *(SWS_PTR(int))pVal = pIpOpts->flags.hdrincl;
    *pLen = (int)sizeof(int);
    break;

  case SWS_IP_TOS:
    if ( *pLen < (int)sizeof(int))
      return SWS_EFAULT;

    *(SWS_PTR(int))pVal = pIpOpts->tos;
    *pLen = (int)sizeof(int);
    break;

  case SWS_IP_TTL:
    if ( *pLen < (int)sizeof(int))
      return SWS_EFAULT;

    *(SWS_PTR(int))pVal = pIpOpts->ttl;
    *pLen = (int)sizeof(int);
    break;

  case SWS_IP_DONTFRAGMENT:
    if ( *pLen < (int)sizeof(int))
      return SWS_EFAULT;

    *(SWS_PTR(int))pVal = pIpOpts->flags.df;
    *pLen = (int)sizeof(int);
    break;

#if OPT_IP_MULTICAST
  case SWS_IP_MULTICAST_IF:
    if ( *pLen < (int)sizeof( struct SWS_in_addr))
      return SWS_EFAULT;

    ((SWS_PTR( struct SWS_in_addr))pVal)->SWS_s_addr = SWS_htonl(
      pIpOpts->multicast_if);
    *pLen = (int)sizeof( struct SWS_in_addr);
    break;

  case SWS_IP_MULTICAST_TTL:
    if ( *pLen < (SWS_u_char)sizeof(SWS_u_char))
      return SWS_EFAULT;

    *(SWS_PTR(SWS_u_char))pVal = pIpOpts->multicast_ttl;
    *pLen = (SWS_u_char)sizeof(int);
    break;

  case SWS_IP_MULTICAST_LOOP:
    if ( *pLen < (SWS_u_char)sizeof(SWS_u_char))
      return SWS_EFAULT;

    *(SWS_PTR(SWS_u_char))pVal = (SWS_u_char)pIpOpts->flags.multicast_loop;
    *pLen = (SWS_u_char)sizeof(int);
    break;
#endif

  default:
    return SWS_EOPNOTSUPP;
    }

  return 0;
  }


/*
 * Get best source IP address to route to destination
 */
SWS_EXPORT( ui32, SWS_IpGetSrc(       /* Returns 0 on error */
  ui32 dst,                           /* IN: Remote IP address */
  unsigned tos                        /* IN: Type Of Service - IPTOS_xxx */
)){
  SWS_SIpRouteInfo route;

  TRACE( 9, ("SWS_IpGetSrc(%08lx,%u)\n", dst, tos));

  if ( IpRoute( SWS_INADDR_ANY, dst, tos & 0xff, &route))
    return SWS_INADDR_NONE;

  /* Reject adapter with an unset IP address unless client requests this */
  if ( SWS_INADDR_ANY == route.src && !(tos & 0x8000) )
    return SWS_INADDR_NONE;

  return route.src;
  }


/*
 * Get max transport-layer message sizes
 */
SWS_EXPORT( size_t, SWS_IpGetMaxSizes( /* Returns max Tx size */
  ui32 src,                           /* IN: src IP address */
  ui32 dst,                           /* IN: dst IP address */
  unsigned tos,                       /* IN: Type Of Service - IPTOS_xxx */
  SWS_PTR( size_t) puMaxRx            /* OUT: Max Rx size, maybe NULL */
)){
  SWS_SIpRouteInfo route;

  TRACE( 9, ("SWS_IpGetMaxSizes(%08lx,%08lx,%u,%p)\n", dst,src,tos,puMaxRx));

  if ( IpRoute( src, dst, tos, &route))
    return 0;

  if ( NULL != puMaxRx)
    {
    /* Return the maximum message size that can be received
     * IP datagrams are limited to 0ffff bytes */
#if OPT_IP_REASSEMBLY_BUFFERS
    if ( s_uReassemblyMax > 0)
      *puMaxRx = s_uReassemblyMax - SWS_IPHDR_MINLEN;
    else
      *puMaxRx = SWS_IP_MAXLEN - SWS_IPHDR_MINLEN;
#else
    *puMaxRx = route.mtu - SWS_IPHDR_MINLEN;
#endif
    }

  if ( s_fragment < route.mtu)
    route.mtu = s_fragment;

  /* Return the maximum transport-layer message size INCLUDING options */
  return route.mtu - SWS_IPHDR_MINLEN;
  }


/*
 * 16-bit checksum
 * >>>> NB This routine is critical to performance <<<<
 */
#if 0
static INLINE( ui32, IpChecksum(
  SWS_PTR( const ui16) punData,
  size_t words
)){
  ui32 sum = 0;

  ASSERT( !((ptrdiff_t)punData & 1));

  while ( words--)
    sum += *(punData++);

  return sum;
  }
#else

#define IpAddChecksum( _p, _w, _t) do { \
  register SWS_PTR( const SWS_PACKED( ui16)) _punData = _p;\
  register size_t _words = _w;\
  while ( _words--) \
    _t += *(_punData++); \
  } while ( 0)

#endif


/*
 * Calc 1's complement 16-bit checksum over a buffer.
 * This is endian independent (surprisingly) if the checksum
 * is written into an IP packet in host endian form.
 * See RFC 1071 p3
 *
 * >>>> NB This routine is critical to performance <<<<
 */
SWS_EXPORT( unsigned, SWS_IpChecksum(
  SWS_BufferH hBuffer,
  SWS_size_t uOffset,
  SWS_size_t uLen,
  ui32 uSum
)){
  SWS_PTR( ui8) p;
  int swap = 0;

  TRACE( 9, ("SWS_IpChecksum(%p,%lu,%lu,%lu)\n",
    hBuffer, (ui32)uOffset, (ui32)uLen, uSum));
  ASSERT( NULL != hBuffer);

  while ( uLen)
    {
    SWS_size_t uFrag;

    /* Get next fragment */
    p = (SWS_PTR( ui8)) SWS_BufferGetFragment( hBuffer, uOffset, &uFrag);
    if ( NULL == p)
      {
      TRACE( 0, ("!!! SWS_IpChecksum() length too big\n"));
      return 1;
      }

    if ( uFrag <= uLen)
      uLen -= uFrag;
    else
      {
      uFrag = uLen;
      uLen = 0;
      }
    uOffset += uFrag;

    /* Align fragment start */
    if ( swap)
      {
      swap = 0;
#if SWS_BIG_ENDIAN
      uSum += *(p++);
#else
      uSum += ((ui32)(*(p++))) << 8; /* Little endian */
#endif
      --uFrag;
      }

    /* Sum whole 16-bit words */
#if 0
    uSum += IpChecksum( (SWS_PTR( SWS_PACKED( ui16)))p, (size_t)(uFrag / sizeof(ui16)) );
#else
    IpAddChecksum( (SWS_PTR( SWS_PACKED( ui16)))p, (size_t)(uFrag / sizeof(ui16)), uSum );
#endif

    /* Deal with trailing byte */
    if ( (sizeof(ui16) - 1) & uFrag)
      {
      swap = 1;
#if SWS_BIG_ENDIAN
      uSum += p[ uFrag - 1] << 8;
#else
      uSum += p[ uFrag - 1]; /* Little endian */
#endif
      }
    }

  /* Deal with end around carries in high order 16-bits */
  while ( uSum >= (1UL<<16))
    uSum = (uSum & 0xffff) + (uSum >> 16);

  return ~(unsigned)uSum & 0xffff;
  }


/*
 * Calc UDP/TCP pseudo header checksum
 */
SWS_EXPORT( ui32, SWS_IpPseudoHdr(
  ui32 src,
  ui32 dst,
  unsigned protocol,
  size_t len
)){
  ui32 uSum;

  uSum = (src >> 16) + (src & 0xffff)
    + (dst >> 16) + (dst & 0xffff)
    + protocol + len;

  /* Deal with end around carries in high order 16-bits */
  while ( uSum >= (1UL<<16))
    uSum = (uSum & 0xffff) + (uSum >> 16);

#if SWS_BIG_ENDIAN
  return uSum;
#else
  return SWAP16( uSum);
#endif
  }


#if OPT_IP_MULTICAST
/*
 * Join a multicast group
 */
SWS_EXPORT( int, SWS_IpGroupJoin(
  ui32 group,                         /* IN: group */
  ui32 src                            /* IN: src adapter or SWS_INADDR_ANY */
)){
  int iRet;
  SWS_SIpRouteInfo info;
  SInterface* pInterface;

  TRACE( 9, ("SWS_IpGroupJoin(%08lx,%08lx)\n", group,src));

  /* Validate group */
  if ( !SWS_IN_MULTICAST( group) )
    {
    TRACE( 1, ("*** SWS_IpGroupJoin() invalid group %08lx\n", group));
    return SWS_EINVAL;
    }

  /* Validate src */
  iRet = IpRoute( src, group, SWS_IP_TOS_DEFAULT, &info);
  if ( iRet)
    {
    TRACE( 1, ("*** SWS_IpGroupJoin() invalid src interface %08lx\n", src));
    return SWS_EADDRNOTAVAIL;
    }

  pInterface = IpFindInterface( info.src);
  ASSERT( NULL != pInterface);

  /* Add the all-hosts group */
  if ( IpGroupFind( pInterface, kGroupAllHosts) < 0)
    IpGroupAdd( pInterface, kGroupAllHosts);

  /* Add group to adapter's multicast list */
  return IpGroupAdd( pInterface, group);
  }


/*
 * Leave a multicast group
 */
SWS_EXPORT( int, SWS_IpGroupLeave(
  ui32 group,                         /* IN: group */
  ui32 src                            /* IN: src adapter or SWS_INADDR_ANY */
)){
  SInterface* pInterface;
  SWS_SIpRouteInfo info;
  int i;

  TRACE( 9, ("SWS_IpGroupLeave(%08lx,%08lx)\n", group,src));

  i = IpRoute( src, group, SWS_IP_TOS_DEFAULT, &info);
  if ( i)
    {
    TRACE( 1, ("*** SWS_IpGroupLeave() invalid src interface %08lx\n", src));
    return SWS_EINVAL;
    }

  pInterface = IpFindInterface( info.src);
  ASSERT( NULL != pInterface);

  i = IpGroupFind( pInterface, group);
  if ( i < 0)
    {
    TRACE( 1, ("*** SWS_IpGroupLeave() no such group %08lx\n", group));
    return SWS_EINVAL;
    }

  return IpGroupRemove( pInterface, &pInterface->group[ i] );
  }


/*
 * Find a multicast group
 */
int IpGroupFind(
  SInterface* pInterface,
  ui32 group                          /* IN: group */
) {
  size_t i;

  TRACE( 9, ("IpGroupFind(%p,%08lx)\n", pInterface,group));
  ASSERT( pInterface);

  for ( i = 0; i < pInterface->groups; ++i)
    {
    if ( pInterface->multicastList[ i] == group)
      {
      ASSERT( pInterface->group[ i].refs > 0);
      return (int)i;
      }
    }

  /* Not found */
  return -1;
  }


/*
 * Add a multicast group
 */
int IpGroupAdd(
  SInterface* pInterface,
  ui32 group                          /* IN: group */
) {
  int iRet;
  SGroup* pGroup;

  TRACE( 9, ("IpGroupAdd(%p,%08lx)\n", pInterface,group));

  iRet = IpGroupFind( pInterface, group);
  if ( iRet >= 0)
    {
    ++(pInterface->group[ iRet].refs);
    return 0;
    }

  if ( pInterface->groups >= lengthof( pInterface->multicastList) )
    {
    TRACE( 1, ("*** IpGroupAdd() too many groups\n"));
    return SWS_ENOMEM;
    }

  pGroup = &pInterface->group[ pInterface->groups];

  pGroup->refs = 1;
#if OPT_PROTO_IGMP
  pGroup->hTimer = SWS_TimerAlloc( &IgmpTimeout, pInterface);
  pGroup->iReportSent = 0;
#endif

  /* Add to adapter's multicast list */
  pInterface->multicastList[ pInterface->groups++] = group;

  if ( pInterface->pfnSetMulticast)
    {
    /* Update the adapter's multicast list */
    iRet = (*pInterface->pfnSetMulticast)(
      pInterface->pvContext,
      pInterface->groups,
      pInterface->multicastList
    );
    }
  else
    {
    iRet = 0;
    }

  if ( iRet)
    {
    --(pInterface->groups);
    }
#if OPT_PROTO_IGMP
  else if ( s_iIgmpVersion > 0
    && kGroupAllHosts != group
  ) {
    /* Start the timer to ensure one retry */
    if ( NULL != pGroup->hTimer)
      SWS_TimerStart( pGroup->hTimer, 10 * SWS_TIMER_TICKS );

    pGroup->iReportSent = 1;

    SWS_IgmpSend(
      InterfaceAddress( pInterface),
      group,
      pInterface->iIgmpVersion >= 2 ? SWS_IGMP_REPORT2 :SWS_IGMP_REPORT,
      0,
      group
    );
    }
#endif

  return iRet;
  }


/*
 * Remove a multicast group
 */
int IpGroupRemove(
  SInterface* pInterface,
  SGroup* pGroup                      /* IN: group */
) {
  int g;

  TRACE( 9, ("IpGroupRemove(%p,%p)\n", pInterface,pGroup));
  ASSERT( NULL != pInterface);
  ASSERT( pGroup >= pInterface->group);
  ASSERT( pGroup < pInterface->group + pInterface->groups);
  ASSERT( pGroup->refs > 0);

  /* Decrement reference count */
  if ( --(pGroup->refs) > 0)
    return 0;

  g = (int)(pGroup - pInterface->group);

#if OPT_PROTO_IGMP
  /* Send a leave if this was the last reporting host */
  if ( pInterface->iIgmpVersion >= 2
    && pGroup->iReportSent
    && kGroupAllHosts != pInterface->multicastList[ g]
  ) {
    SWS_IgmpSend(
      InterfaceAddress( pInterface),
      kGroupAllRouters,
      SWS_IGMP_LEAVE,
      0,
      pInterface->multicastList[ g]
    );
    }

  if ( NULL != pGroup->hTimer)
    SWS_TimerRelease( pGroup->hTimer);
#endif

  if ( g < (int)--(pInterface->groups))
    {
    /* Remove the group from adapter's multicast list */
    memmove(
      &pInterface->multicastList[ g],
      &pInterface->multicastList[ 1 + g],
      (pInterface->groups - g) * sizeof( pInterface->multicastList[ 0])
    );

    /* Delete the group */
    memmove(
      pGroup,
      pGroup + 1,
      (pInterface->groups - g) * sizeof( *pGroup)
    );
    }

  /* Update the adapter's multicast list */
  if ( !pInterface->pfnSetMulticast)
    return 0;

  return (*pInterface->pfnSetMulticast)(
    pInterface->pvContext,
    pInterface->groups,
    pInterface->multicastList
  );
  }

#endif /* OPT_IP_MULTICAST */


/*
 * Send an IP datagram with options
 */
SWS_EXPORT( int, SWS_IpSend(          /* Returns !0 omn error */
  ui32 src,                           /* IN: Src IP address, 0= default */
  ui32 dst,                           /* IN: Dst IP address, (NOT 0) */
  unsigned protocol,                  /* IN: Protocol IPPROTO_xxx, 0= IP */
  SWS_BufferH hBuffer,                /* IN: Datagram content */
  unsigned id,                        /* IN: Ident, 0= don't care */
  SWS_PTR( SWS_IpOptions) pIpOpts     /* IN: IP option data */
)){
#if OPT_IP_TXENABLE
  static ui16 s_lastID;               /* Last datagram ID */
  SFragInfo frag;
  int iRet;
  SWS_size_t uSize;
#else
  (void)src;
  (void)dst;
  (void)protocol;
  (void)hBuffer;
  (void)id;
  (void)pIpOpts;
#endif

  TRACE( 9, ("SWS_IpSend(%08lx,%08lx,%u,%p,%u,%p)\n", src,dst,protocol,hBuffer,id,pIpOpts));
  ASSERT( NULL != hBuffer);

#if OPT_IP_TXENABLE
  /* Check IP options */
  if ( NULL != pIpOpts)
    {
    frag.tos = pIpOpts->tos;
    switch ( frag.tos & 0x1f) /* Ignore precedence */
      {
    case SWS_IPTOS_LOWDELAY:
    case SWS_IPTOS_THROUGHPUT:
    case SWS_IPTOS_RELIABILITY:
    case SWS_IPTOS_COST:
    case SWS_IPTOS_ROUTINE:
      break;

    default:
      TRACE( 1, ("*** SWS_IpSend() invalid TOS %u\n", frag.tos));
      /*return SWS_EINVAL;*/
      break;
      }

#if OPT_IP_MULTICAST
    if ( SWS_IN_MULTICAST( dst))
      {
      frag.ttl = pIpOpts->multicast_ttl;
      if ( 0 == frag.ttl)
        frag.ttl = SWS_IP_DEFAULT_MULTICAST_TTL;

      frag.multicast_loopback = (char)pIpOpts->flags.multicast_loop;
      }
    else
#endif
      {
      frag.ttl = pIpOpts->ttl;
      if ( 0 == frag.ttl)
        frag.ttl = s_ucDefaultTTL;
      }
    frag.df = (char)pIpOpts->flags.df;
    frag.hdrincl = (char)pIpOpts->flags.hdrincl;
    ASSERT( sizeof( frag.optBuf) >= pIpOpts->uOptLen);
    memcpy( frag.optBuf, pIpOpts->opts, pIpOpts->uOptLen);
    frag.uOptLen = pIpOpts->uOptLen;
    }
  else
    {
    /* Use defaults */
#if OPT_IP_MULTICAST
    if ( SWS_IN_MULTICAST( dst))
      {
      frag.ttl = SWS_IP_DEFAULT_MULTICAST_TTL;
      frag.multicast_loopback = SWS_IP_DEFAULT_MULTICAST_LOOP;
      }
    else
#endif
      {
      frag.ttl = s_ucDefaultTTL;
      }

    frag.tos = SWS_IP_TOS_DEFAULT;
    frag.df = 0;
    frag.hdrincl = 0;
    frag.uOptLen = 0;
    }

  uSize = SWS_BufferSize( hBuffer);
#if SWS_SIZE_MAX > SWS_IP_MAXLEN
  if ( (frag.hdrincl && uSize > SWS_IP_MAXLEN)
    || (!frag.hdrincl && SWS_IPHDR_MINLEN + frag.uOptLen + uSize > SWS_IP_MAXLEN)
  ) {
    TRACE( 1, ("*** SWS_IpSend() datagram %u too big\n", frag.uOptLen + uSize));
    return SWS_EMSGSIZE;
    }
#endif
  frag.hBuffer = hBuffer;

  /* Setup defaulted params */
  if ( SWS_IP_IDENT_DEFAULT == id)
    frag.id = ++s_lastID;
  else
    frag.id = (ui16)id;

  frag.protocol = (ui8)protocol;

  if ( SWS_INADDR_ANY == dst)
    {
    TRACE( 1, ("*** SWS_IpSend() dst is INADDR_ANY\n"));
    return SWS_EADDRNOTAVAIL;
    }

  /* Martian address filtering */
  /* BUGBUG Combining these 2 tests with && breaks MSVC1.52c */
  if ( dst >= kExperimentalGroup)
    {
    if ( SWS_INADDR_BROADCAST != dst)
      {
      TRACE( 1, ("*** SWS_IpSend() invalid dst %08lx\n", dst));
      return SWS_EADDRNOTAVAIL;
      }
    }

  /* Route the datagram */
#if OPT_IP_MULTICAST
  if ( SWS_INADDR_ANY == src
    && SWS_IN_MULTICAST( dst)
    && NULL != pIpOpts
  ) {
    src = pIpOpts->multicast_if;
    }
#endif

  frag.dst = dst;
  iRet = IpRoute( src, dst, frag.tos, &frag.route);
  if ( iRet)
    {
    AtomicIncrement( &s_aTxDropped);
    SWS_GefRaise( SWS_kGefIpError, hBuffer, SWS_kErrorTxNoRoute, "no route");
    return iRet;
    }

  /* Set max packet size */
  if ( s_fragment < frag.route.mtu)
    frag.route.mtu = s_fragment;

  frag.sent = 0;
  frag.done = 0;

  /* Send the datagram */
  return PlatformSynchronise( &IpSendSync, &frag);
#else
  return 0;
#endif
  }

#if OPT_IP_TXENABLE
/* Synchronised callback from SWS_IpSend */
static SWS_CB( int, IpSendSync(
  SWS_PTR( void) pvContext
)){
  SWS_PTR( SFragInfo) pFrag = (SWS_PTR( SFragInfo))pvContext;
  SInterface* pInterface;
  int iRet;

  TRACE( 9, ("IpSendSync(%p)\n", pvContext));
  ASSERT( NULL != pvContext);
  ASSERT( PlatformIsCritical());

  pInterface = IpFindInterface( pFrag->route.src);
  ASSERT( NULL != pInterface);

  /* Push onto send Q */
  SWS_BufferAddRef( pFrag->hBuffer);
  iRet = SWS_MsgqPut( pInterface->hTxMsgQ, sizeof(*pFrag), pFrag);
  if ( 0 == iRet)
    {
    size_t const pending = SWS_MsgqCount( pInterface->hTxMsgQ);
    if ( pending > s_uTxQHi)
      {
      pInterface->iTxFull = 1;
      iRet = -1;
      }

    /* Is the interface busy? */
    if ( !pInterface->iTxBusy)
      {
      /* No, send the packet now */
      IpSendRdy( pInterface, NULL, 0);
      }
    }
  else
    {
    /* TxQ full */
    TRACE( 1, ("*** IpSendSync() %08lx TxQ full\n", pFrag->route.src));

    ASSERT( pInterface->iTxBusy);
    ASSERT( pInterface->iTxFull);

    AtomicIncrement( &s_aTxDropped);
    SWS_GefRaise( SWS_kGefIpError, pFrag->hBuffer, SWS_kErrorTxDropped, "TxQ full");

    SWS_BufferRelease( pFrag->hBuffer);
    }

  return iRet;
  }


/*
 * Send the next fragment of a datagram
 */
static SWS_FASTCALL( int, IpSendFragment(
  SInterface* pInterface,
  SFragInfo* pFrag
)){
  int iRet;
  size_t tosend, data;
  SWS_BufferH hbTemp;
  SWS_size_t uLen;

  TRACE( 9, ("IpSendFragment(%p,%p)\n", pInterface, pFrag));
  ASSERT( NULL != pInterface);
  ASSERT( INTERFACE_MAGIC == pInterface->uMagic);
  ASSERT( NULL != pFrag);
  ASSERT( PlatformIsCritical());

  uLen = SWS_BufferSize( pFrag->hBuffer);
  ASSERT( uLen > pFrag->sent);
  tosend = (size_t)(uLen - pFrag->sent);

  /* IPPROTO_RAW implies IP_HDRINCL */
  if ( !pFrag->hdrincl
    && SWS_IPPROTO_RAW != pFrag->protocol
  ) {
    size_t hdr;
    unsigned offset, opts;
    SWS_PTR( void) pvIpHdr;
    ui8* pOptions;
    ui8 optBuf[ SWS_IPHDR_MAXLEN - SWS_IPHDR_OPTIONS + 8];

#if OPT_IP_OPTIONS
    opts = 0;
    offset = 0;
    while ( offset < pFrag->uOptLen)
      {
      unsigned opt, len;

      /* Get option kind */
      SWS_PEEK8( opt, pFrag->optBuf, offset);

      /* Decode option */
      if ( SWS_IP_OPT_END == opt)
        break;

      if  ( SWS_IP_OPT_NOP == opt)
        {
        ++offset;
        }
      else
        {
        SWS_PEEK8( len, pFrag->optBuf, offset + 1);

        if ( (SWS_IP_OPT_COPIED & opt)
          || 0 == pFrag->sent /* 1st fragment contains all caller supplied options */
        ) {
          memcpy( &optBuf[ opts], pFrag->optBuf + offset, len);

          switch ( opt)
            {
          case SWS_IP_OPT_TIMESTAMP:
            /* RFC1122 3.2.1.8: Timestamp option is OPTIONAL
             * The originating host MUST record a timestamp in a
             * Timestamp option whose Internet address fields are
             * not pre-specified or whose first pre-specified
             * address is the host's interface address
             */
            /* If ptr is at beginning then timestamp */
            if ( 5 == optBuf[ opts + 2] )
              IpTimestamp( &optBuf[ opts], offset, pFrag->route.src);
            break;

          default:
            break;
            }

          opts += len;
          }

        offset += len;
        }
      }

    if ( opts > 0)
      {
      /* Ensure end marker present */
      if ( opts < SWS_IPHDR_OPTIONS
        && SWS_IP_OPT_END != optBuf[ opts - 1]
      ) {
        SWS_POKE8( optBuf, opts, SWS_IP_OPT_END);
        ++opts;
        }
      pOptions = optBuf;
      }
    else
      {
      pOptions = NULL;
      }
#else
    if ( 0 != (opts = pFrag->uOptLen))
      pOptions = (ui8*) memcpy( optBuf, pFrag->optBuf, opts);
    else
      pOptions = NULL;
#endif

    hdr = SWS_IPHDR_MINLEN;
    hdr += ROUNDUP( opts, 4); /* Round up to 32-bits */
    ASSERT( (hdr & 3) == 0);
    if ( hdr > 0xf * 4)
      {
      TRACE( 1, ("*** IpSendFragment() header too big\n"));
      pFrag->done = 1;
      return SWS_EMSGSIZE;
      }

    /* Is fragment size > MTU? */
    if ( hdr + tosend <= pFrag->route.mtu)
      {
      /* No, it's the last or only fragment */
      ASSERT( pFrag->sent + tosend == uLen);
      data = tosend;

      if ( pFrag->sent > 0)
        VERIFY( !SWS_BufferRemoveFront( pFrag->hBuffer, pFrag->sent));

      SWS_BufferAddRef( pFrag->hBuffer);
      hbTemp = pFrag->hBuffer;

      /* Prepend IP header */
      iRet = SWS_BufferAppendFront( hbTemp, hdr);
      if ( iRet)
        return iRet;

      pFrag->done = 1;
      }
    else if ( pFrag->df)
      {
      TRACE( 1, ("*** IpSendFragment() datagram can't be fragmented\n"));
      pFrag->done = 1;
      return SWS_EMSGSIZE;
      }
    /* Fragment the datagram */
    else
      {
      /* Calc data bytes this packet.
       * Bytes must be divisible by 8 except in last fragment */
      data = (unsigned)(-SWS_IP_MIN_FRAG & (pFrag->route.mtu - hdr)); /* 8-byte blocks */
      ASSERT( data < tosend);
      ASSERT( data + hdr <= pFrag->route.mtu);

      hbTemp = SWS_BufferGet( data + hdr, SWS_kBufferPrefix);
      if ( NULL == hbTemp)
        return SWS_ENOBUFS;

      /* Copy fragment to new buffer */
      VERIFY( !SWS_BufferCopy( hbTemp, hdr, pFrag->hBuffer, pFrag->sent, data));
      }

    /* Calc starting offset of data in this fragment */
    ASSERT( 0 == (pFrag->sent % SWS_IP_MIN_FRAG)); /* Offset must be multiple of 8 bytes */
    offset = pFrag->sent / SWS_IP_MIN_FRAG; /* Offset is in units of 8 */

    /* Set MF if not last frag */
    if ( data < tosend)
      {
      offset |= SWS_IP_FLAG_MF; /* MoreFragments */
      ASSERT( 0 == (data % SWS_IP_MIN_FRAG)); /* Fragment must be multiple of 8 bytes */
      }
    if ( pFrag->df)
      offset |= SWS_IP_FLAG_DF; /* DontFragment */

    pvIpHdr = SWS_BufferGetFragment( hbTemp, 0, &uLen);
    ASSERT( NULL != pvIpHdr);
    ASSERT( uLen >= hdr);

    /* Fill in the IP header */
    SWS_POKE8( pvIpHdr, SWS_IPHDR_VER_IHL, (ui8)((SWS_IPVERSION << 4) | (hdr / 4)) );
    SWS_POKE8( pvIpHdr, SWS_IPHDR_TOS, pFrag->tos);
    SWS_POKE16( pvIpHdr, SWS_IPHDR_TOTAL, (ui16)(data + hdr) );
    SWS_POKE16( pvIpHdr, SWS_IPHDR_ID, pFrag->id );
    SWS_POKE16( pvIpHdr, SWS_IPHDR_FLAGS_OFFSET, (ui16)offset );
    SWS_POKE8( pvIpHdr, SWS_IPHDR_TTL, pFrag->ttl );
    SWS_POKE8( pvIpHdr, SWS_IPHDR_PROTOCOL, pFrag->protocol );
    SWS_POKE16( pvIpHdr, SWS_IPHDR_CHECKSUM, 0 );
    SWS_POKE32( pvIpHdr, SWS_IPHDR_SRC, pFrag->route.src );
    SWS_POKE32( pvIpHdr, SWS_IPHDR_DST, pFrag->dst );

    /* Append the IP options */
    if ( opts)
      {
      unsigned uPad;

      ASSERT( hdr >= SWS_IPHDR_MINLEN + opts);
      memcpy( (SWS_PTR(char))pvIpHdr + SWS_IPHDR_OPTIONS, pOptions, opts);

      /* Pad with zeroes */
      uPad = hdr - (SWS_IPHDR_MINLEN + opts);
      if ( uPad)
        memset( (SWS_PTR(char))pvIpHdr + SWS_IPHDR_OPTIONS + opts, 0, uPad);
      }

    /* Calc. hdr checksum */
    SWS_POKECSUM( pvIpHdr, SWS_IPHDR_CHECKSUM, SWS_IpChecksum( hbTemp, 0, hdr, 0));
    /*ASSERT( !SWS_IpChecksum( hbTemp, 0, hdr, 0));*/
    }
  else
    {
    SWS_BufferAddRef( pFrag->hBuffer);
    hbTemp = pFrag->hBuffer;
    data = tosend;
    pFrag->done = 1;
    }

  /* Call event filter */
  AtomicIncrement( &s_aTxPackets);
  AtomicAdd( &s_aTxBytes, (int)SWS_BufferSize( hbTemp));
  if ( 0 == (iRet = SWS_GefRaise( SWS_kGefIpTx, hbTemp, pFrag->route.nextHop)) )
    {
#if OPT_IP_MULTICAST
    /* Loopback the packet if it's a multicast to a member group */
    if ( SWS_IN_MULTICAST( pFrag->route.nextHop)
      && kGroupAllHosts != pFrag->route.nextHop
      && pFrag->multicast_loopback
      && IpGroupFind( pInterface, pFrag->route.nextHop) >= 0
    ) {
      SWS_BufferH hb = SWS_BufferDup( hbTemp, 0);
      if ( NULL != hb)
        {
        DatalinkRecvInd( pInterface, hb, 0);
        SWS_BufferRelease( hb);
        }
      }
#endif

    /* Call the interface's 'send buffer' function */
    ASSERT( pInterface->pfnSend);
    iRet = (*pInterface->pfnSend)(
      pInterface->pvContext,
      hbTemp,
      pFrag->route.nextHop,
      pFrag->tos
    );
    if ( iRet > 0)
      {
      AtomicIncrement( &s_aTxErrors);
      SWS_GefRaise( SWS_kGefIpError, NULL, SWS_kErrorTxFailed, "datalink send failed", iRet);
      }
    }
  else
    {
    TRACE( 4, ("IpSendFragment() filtered %d\n", iRet));
    iRet = 0;
    }

  SWS_BufferRelease( hbTemp);
  pFrag->sent += data;

  return iRet;
  }
#endif /* OPT_IP_TXENABLE */


/****************************************************************************
 * Upcalls from datalink interface
 ****************************************************************************/

/*
 * DPC scheduled by DatalinkSendRdy
 */
static SWS_CB( void, IpSendRdy(
  SWS_PTR( void) pvContext,           /* IN: Client context */
  SWS_PTR( void) pvData,              /* IN: */
  ui32 uLen                           /* IN: */
)){
#if OPT_IP_TXENABLE
  SInterface* const pInterface = (SInterface*)pvContext;
  SFragInfo frag;
#else
  (void)pvContext;
#endif
  (void)pvData;
  (void)uLen;

  TRACE( 5, ("IpSendRdy(%p,%p,%lu)\n",pvContext,pvData,uLen));
#if OPT_IP_TXENABLE
  ASSERT( NULL != pInterface);
  ASSERT( INTERFACE_MAGIC == pInterface->uMagic);
  ASSERT( PlatformIsCritical());

  if ( INTERFACE_MAGIC != pInterface->uMagic)
    return;

  pInterface->iTxBusy = 0;

  /* Until no more fragments to send */
  while ( !SWS_MsgqGet( pInterface->hTxMsgQ, sizeof(frag), &frag))
    {
    int iRet;

    pInterface->iTxBusy = 1;
    iRet = IpSendFragment( pInterface, &frag);

    if ( !frag.done )
      {
      /* More fragments to go so replace at end of send Q.
       * This enables nested sends or multiple streams to share */
      VERIFY( !SWS_MsgqPut( pInterface->hTxMsgQ, sizeof(frag), &frag));
      }
    else
      {
      /* Fragment complete so release buffer */
      SWS_BufferRelease( frag.hBuffer);
      }

    if ( -1 == iRet)
      return; /* Wait for DatalinkSendRdy callback */

    pInterface->iTxBusy = 0;
    }

  /* Did a previous SWS_IpSend return pending because of a hi TxMsgQ? */
  if ( pInterface->iTxFull
    && SWS_MsgqCount( pInterface->hTxMsgQ) <= s_uTxQLo
  ) {
    /* Yes */
    pInterface->iTxFull = 0;

#if OPT_PROTO_ENTRIES
    {
    /* Notify all protocols of more Tx resources */
    SProtocol* pProtocol;
    unsigned u;
    for (
      pProtocol = &s_protocolArray[ u = 0];
      u < s_uProtocols;
      ++u, ++pProtocol
    ) {
      if ( pProtocol->pfnSendInd)
        (*pProtocol->pfnSendInd)( pProtocol->pvContext);
      }
    }
#endif
    }
#endif /* OPT_IP_TXENABLE */
  }


/*
 * Receive a packet callback
 * Single threaded
 */
static SWS_CB( int, IpRecvInd(
  SWS_PTR( void) pvContext
)){
  SRecvInfo* const pInfo = (SRecvInfo*)pvContext;
  SInterface* pInterface;
  SWS_BufferH hBuffer;
  SWS_PTR(void) pvIpHdr;
  SWS_size_t uLen;
  unsigned hdr, uVersion, offset, total, protocol;
#if OPT_PROTO_ENTRIES
  unsigned tos;
#endif
  ui32 src, dst, uHostIp, uSubnetMask;
#if OPT_IP_ROUTED || OPT_IP_OPTIONS
  ui32 uNextIp = SWS_INADDR_NONE;
#endif
#if OPT_IP_REASSEMBLY_BUFFERS
  SReassembly* pReassembly = NULL;
#endif
  int i;

  TRACE( 9, ("IpRecvInd(%p)\n", pvContext));
  ASSERT( NULL != pvContext);
  ASSERT( PlatformIsCritical());

  pInterface = pInfo->pInterface;
  ASSERT( NULL != pInterface);
  ASSERT( INTERFACE_MAGIC == pInterface->uMagic);

  if ( 0 == s_uInterfaces || INTERFACE_MAGIC != pInterface->uMagic)
    return SWS_EBADF;

  hBuffer = pInfo->hBuffer;
  ASSERT( NULL != hBuffer);

  /* Call the event filter */
  AtomicIncrement( &s_aRxPackets);
  AtomicAdd( &s_aRxBytes, (int)SWS_BufferSize( hBuffer));
  if ( SWS_GefRaise( SWS_kGefIpRx, hBuffer, pInfo->broadcast))
    {
    TRACE( 4, ("IpRecvInd() filtered\n"));
    return 0;
    }

  /* Get -> IP header */
  pvIpHdr = SWS_BufferGetFragment( hBuffer, 0, &uLen);
  if ( NULL == pvIpHdr
    || uLen < SWS_IPHDR_MINLEN
  ) {
    TRACE( 1, ("*** IpRecvInd() datagram too small (%u bytes)\n", uLen));
    AtomicIncrement( &s_aRxErrors);
    SWS_GefRaise(
      SWS_kGefIpError, hBuffer, SWS_kErrorRxBadHdr, "packet too small");
    return SWS_EINVAL;
    }

  /* Verify IP header */
  SWS_PEEK8( hdr, pvIpHdr, SWS_IPHDR_VER_IHL);
  uVersion = hdr >> 4;
  if ( SWS_IPVERSION != uVersion)
    {
    TRACE( 1, ("*** IpRecvInd() invalid IP version %u\n", uVersion));
    AtomicIncrement( &s_aRxErrors);
    SWS_GefRaise(
      SWS_kGefIpError, hBuffer, SWS_kErrorRxBadHdr, "invalid IP version");
    return SWS_EINVAL;
    }

  hdr = 4 * (hdr & 0xf);
  if ( hdr < SWS_IPHDR_MINLEN)
    {
    TRACE( 1, ("*** IpRecvInd() IHL too small (%u bytes)\n", hdr));
    AtomicIncrement( &s_aRxErrors);
    SWS_GefRaise(
      SWS_kGefIpError, hBuffer, SWS_kErrorRxBadHdr, "header too small");
    return SWS_EINVAL;
    }

  uLen = SWS_BufferSize( hBuffer);
  if ( hdr > uLen)
    {
    TRACE( 1, ("*** IpRecvInd() header truncated: received %u of %u\n", uLen, hdr));
    AtomicIncrement( &s_aRxErrors);
    SWS_GefRaise(
      SWS_kGefIpError, hBuffer, SWS_kErrorRxBadHdr, "header truncated");
    return SWS_EINVAL;
    }

  if ( SWS_IpChecksum( hBuffer, 0, hdr, 0))
    {
    TRACE( 1, ("*** IpRecvInd() Invalid checksum\n"));
    AtomicIncrement( &s_aRxErrors);
    SWS_GefRaise(
      SWS_kGefIpError, hBuffer, SWS_kErrorRxBadHdr, "bad checksum");

    /* NB Win2k calcs bad csum on packets > 576 bytes sent by SOCK_RAW.
     * It attempts to fragment them but the subsequent fragments never arrive. */
    return SWS_EINVAL;
    }

  /* Verify datagram is correctly framed */
  SWS_PEEK16( total, pvIpHdr, SWS_IPHDR_TOTAL );
  if ( total > uLen)
    {
    TRACE( 1, ("*** IpRecvInd() Framing error: received %u, IP total %u\n",
      uLen, total));

    AtomicIncrement( &s_aRxErrors);
    SWS_GefRaise(
      SWS_kGefIpError, hBuffer, SWS_kErrorRxBadHdr, "truncated fragment");
    return SWS_EINVAL;
    }
  else if ( total < uLen)
    {
    /* Remove trailing padding e.g. short Ethernet packets or odd lengths */
    TRACE( 4, ("IpRecvInd() removed %u trailing bytes\n", uLen - total));
    VERIFY( !SWS_BufferRemoveBack( hBuffer, uLen - total));
    }

#if OPT_PROTO_ENTRIES
  SWS_PEEK8( tos, pvIpHdr, SWS_IPHDR_TOS);
#endif

  /* Determine the destination host */
  uSubnetMask = InterfaceSubnet( pInterface);
  SWS_PEEK32( dst, pvIpHdr, SWS_IPHDR_DST);
  uHostIp = InterfaceAddress( pInterface);
  if ( dst != uHostIp
    && uHostIp != SWS_INADDR_LOOPBACK
  ) {
    /* Not us, is it an IP multi-cast? */
    if ( SWS_IN_MULTICAST( dst))
      {
#if OPT_IP_MULTICAST
      if ( IpGroupFind( pInterface, dst) < 0)
        {
        TRACE( 1, ("*** IpRecvInd() multicast to %08lx\n", dst));
        return SWS_EHOSTUNREACH;
        }
#else
      TRACE( 1, ("*** IpRecvInd() multicast to %08lx\n", dst));
      return SWS_EHOSTUNREACH;
#endif
      }
    /* Is it a link-layer broadcast? */
    else if ( pInfo->broadcast)
      {
      /* A host SHOULD silently discard a datagram that is received via
       * a link-layer broadcast (see Section 2.4) but does not specify
       * an IP multicast or broadcast destination address
       */
      if ( SWS_INADDR_BROADCAST != dst
        && (SWS_INADDR_BROADCAST & ~uSubnetMask) != (dst & ~uSubnetMask)
        && 0 != (dst & ~uSubnetMask) /* For BSD 4.2 */
      ) {
        TRACE( 3, ("IpRecvInd() invalid datalink broadcast to %08lx\n", dst));

        SWS_GefRaise(
          SWS_kGefIpError, hBuffer, SWS_kErrorRxBadHost, "invalid datalink broadcast");
        return SWS_EHOSTUNREACH;
        }
      }
    /* Is it an IP broadcast? */
    else if ( SWS_INADDR_BROADCAST != dst
      && (0 == ~uSubnetMask
      || ((SWS_INADDR_BROADCAST & ~uSubnetMask) != (dst & ~uSubnetMask)
         && 0 != (dst & ~uSubnetMask)) /* For BSD 4.2 */
      )
    ) {
      /* Not an IP broadcast
       * Do we have an assigned addrss? */
      if ( SWS_INADDR_ANY != uHostIp)
        {
        /* Yes, it's not to us. Is it from us? */
        SWS_PEEK32( src, pvIpHdr, SWS_IPHDR_SRC);
        if ( src == uHostIp)
          {
          TRACE( 4, ("IpRecvInd() Unexpected loopback\n"));
          return 0;
          }
        else
#if OPT_IP_ROUTED
        if ( s_iRouted)
          {
          /* Forward to dst */
          uNextIp = dst;
          }
        else
#endif
          {
          TRACE( 3, ("IpRecvInd() packet intended for %08lx\n", dst));

          SWS_GefRaise( SWS_kGefIpError, hBuffer, SWS_kErrorRxBadHost, "misrouted packet");
          return SWS_EHOSTUNREACH;
          }
        }
#if !OPT_DHCP
      else if ( 0 == uSubnetMask)
        {
        /* Discovered (gleaned) this interface's IP address */
        TRACE( 2, ("IpRecvInd() Discovered IP address %08lx for interface %p\n",dst,pInterface));

        uHostIp = dst;
        InterfaceSetAddress( pInterface, dst);
        SWS_GefRaise(
          SWS_kGefIpError, hBuffer, SWS_kErrorIpGleaned, "IP address gleaned");
        }
#endif
      }
    /* Handle broadcasts as if they're directed at this host */
    }

#if OPT_IP_OPTIONS
  /* Process IP options.
   * NB Processing options is not a requirement for RFC791 & RFC1122
   * All IP options (except NOP or END-OF-LIST) received in
   * datagrams MUST be passed to the transport layer
   */
  offset = SWS_IPHDR_MINLEN;
  while ( offset < hdr)
    {
    unsigned opt, len, ptr;

    /* Get option kind */
    SWS_PEEK8( opt, pvIpHdr, offset);

    /* Decode option */
    if ( SWS_IP_OPT_END == opt)
      break;

    switch ( opt)
      {
    case SWS_IP_OPT_NOP:
      ++offset;
      break;

    case SWS_IP_OPT_LOOSE_ROUTE:
    case SWS_IP_OPT_STRICT_ROUTE:
      SWS_PEEK8( len, pvIpHdr, offset + 1);
      SWS_PEEK8( ptr, pvIpHdr, offset + 2); /* pointer */
      if ( ptr < len)
        {
        /* Update the source route */
        SWS_PEEK32( uNextIp, pvIpHdr, offset + ptr - 1);
        SWS_POKE32( pvIpHdr, offset + ptr - 1, uHostIp);
        SWS_POKE8( pvIpHdr, offset + 2, (ui8)(ptr + 4));

        /* A strict route requires dst to be directly reachable */
        if ( SWS_IP_OPT_STRICT_ROUTE == opt
          && (uNextIp & uSubnetMask) != (uHostIp & uSubnetMask)
        ) {
#if OPT_PROTO_ICMP
          /*  Send ICMP source route failed */
          SWS_PEEK32( src, pvIpHdr, SWS_IPHDR_SRC);
          SWS_IcmpSend(
            uHostIp, src,
            SWS_ICMP_DEST_UNREACH, SWS_ICMP_ROUTE_FAIL, 0,
            hBuffer,
            SWS_IP_IDENT_DEFAULT,
            NULL
          );
#endif
          return SWS_EHOSTUNREACH;
          }
        }

      offset += len;
      break;

    case SWS_IP_OPT_TIMESTAMP:
      /* RFC1122 3.2.1.8: Timestamp option is OPTIONAL
       * The destination host MUST (if possible) add the
       * current timestamp to a Timestamp option before
       * passing the option to the transport layer or to
       * ICMP for processing
       */
      IpTimestamp( pvIpHdr, offset, uHostIp);

      SWS_PEEK8( len, pvIpHdr, offset + 1);
      offset += len;
      break;

    case SWS_IP_OPT_RECORD_ROUTE:
      SWS_PEEK8( len, pvIpHdr, offset + 1);
      SWS_PEEK8( ptr, pvIpHdr, offset + 2); /* pointer */
      if ( ptr < len)
        {
        /* Append this host */
        SWS_POKE32( pvIpHdr, offset + ptr - 1, uHostIp);
        SWS_POKE8( pvIpHdr, offset + 2, (ui8)(ptr + 4) );
        }

      offset += len;
      break;

    default:
      SWS_PEEK8( len, pvIpHdr, offset + 1);
      offset += len;
      break;
      }
    }
#endif /* OPT_IP_OPTIONS */

#if OPT_IP_ROUTED || OPT_IP_OPTIONS
  /* Is the packet to be forwarded? */
  if ( SWS_INADDR_NONE != uNextIp)
    return IpForward( hBuffer, uHostIp, uNextIp);
#endif

  /* Receive locally */
  SWS_PEEK16( offset, pvIpHdr, SWS_IPHDR_FLAGS_OFFSET );
  SWS_PEEK8( protocol, pvIpHdr, SWS_IPHDR_PROTOCOL );
  SWS_PEEK32( src, pvIpHdr, SWS_IPHDR_SRC );

  /* Is the packet fragmented? */
  if ( (SWS_IP_OFFSET_MASK | SWS_IP_FLAG_MF) & offset) /* Mask off DF & reserved MSB */
    {
    /* Yes, reassembly required */
#if OPT_IP_REASSEMBLY_BUFFERS
    unsigned ttl, ident;
    int iRet;
    int const mf = (offset & SWS_IP_FLAG_MF) ? 1 : 0;

    offset = (offset & SWS_IP_OFFSET_MASK) << 3;

    SWS_PEEK8( ttl, pvIpHdr, SWS_IPHDR_TTL );
    SWS_PEEK16( ident, pvIpHdr, SWS_IPHDR_ID );

    /* Search the pending reassembly buffers for a match */
    pReassembly = ReassemblyFind( src, dst, ident, protocol);
    if ( NULL != pReassembly)
      {
      TRACE( 3, ("IpRecvInd() match frag %08lx=>%08lx proto:%u id:%u off:%u len:%u\n",
        src, dst, protocol, ident, 8 * (offset & SWS_IP_OFFSET_MASK), total));
      }
    else
      {
      TRACE( 3, ("IpRecvInd() new fragment src:%08lx dst:%08lx proto:%u id:%u len:%u\n",
        src, dst, protocol, ident, total));

      /* Allocate a reassembly buffer */
      pReassembly = ReassemblyGet( src, dst, ident, protocol);
      if ( NULL == pReassembly)
        {
        /* Discard a pending reassembly */
        TRACE( 2, ("IpRecvInd() Out of reassembly buffers, discarding\n"));
        AtomicIncrement( &s_aRxDropped);
        SWS_GefRaise(
          SWS_kGefIpError, hBuffer, SWS_kErrorRxDropped, "out of reassembly buffers");

        pReassembly = s_pReassembling;
        ASSERT( NULL != s_pReassembling);
#if OPT_PROTO_ICMP
        if ( pReassembly->iFragZero
          && !pInfo->broadcast
        ) {
          /*  Send ICMP time exceeded code 1 (reassembly) */
          SWS_IcmpSend(
            uHostIp, pReassembly->src,
            SWS_ICMP_TIME_EXCEED, SWS_ICMP_FRAG_EXCEED, 0,
            pReassembly->hBuffer,
            SWS_IP_IDENT_DEFAULT,
            NULL
          );
          }
#endif
        ReassemblyPut( pReassembly);
        pReassembly = ReassemblyGet( src, dst, ident, protocol);
        ASSERT( NULL != pReassembly);
        }

      /* Ensure ttl is at least some minimum (RFC1122 p57) */
      if ( ttl < s_uMinReassemblyTimeout)
        ttl = s_uMinReassemblyTimeout;
      }

    /* Add the fragment */
    iRet = ReassemblyAddFragment(
      pReassembly,
      hBuffer,
      hdr,
      offset,
      mf,
      ttl
    );
    if ( iRet)
      {
      /* Out of reassembly buffer space */
      AtomicIncrement( &s_aRxDropped);
      SWS_GefRaise(
        SWS_kGefIpError, hBuffer, SWS_kErrorRxDropped, "no buffer for fragment");

#if OPT_PROTO_ICMP
      if ( pReassembly->iFragZero
        && NULL != pReassembly->hBuffer
        && !pInfo->broadcast
      ) {
        /*  Send source quench */
        SWS_IcmpSend(
          uHostIp, src,
          SWS_ICMP_QUENCH, 0, 0,
          pReassembly->hBuffer,
          SWS_IP_IDENT_DEFAULT,
          NULL
        );
        }
#endif

      /* Discard the reassembly buffer */
      ReassemblyPut( pReassembly);
      return iRet;
      }

    if ( !ReassemblyIsComplete( pReassembly))
      return 0; /* More fragments required */

    /* Reassembly is complete */
    hBuffer = pReassembly->hBuffer;
    ASSERT( pReassembly->length == (ui16)SWS_BufferSize( hBuffer));

    pvIpHdr = SWS_BufferGetFragment( hBuffer, 0, &uLen);
    ASSERT( NULL != pvIpHdr);
    ASSERT( uLen >= SWS_IPHDR_MINLEN);

    SWS_PEEK8( hdr, pvIpHdr, SWS_IPHDR_VER_IHL);
    hdr = 4 * (hdr & 0xf);
#else
    SWS_PEEK16( hdr, pvIpHdr, SWS_IPHDR_ID );
    TRACE( 1, ("*** IpRecvInd() discarding fragment src:%08lx dst:%08lx proto:%u id:%u len:%u\n",
      src, dst, protocol, hdr, total));
    return SWS_EMSGSIZE;
#endif /* OPT_IP_REASSEMBLY_BUFFERS */
    }

#if OPT_PROTO_ICMP
  /* Send SourceQuench if buffer space low */
  if ( SWS_BufferAvailable() < SWS_BufferTotal() / 5
    && !pInfo->broadcast
  ) {
    SWS_BufferH hb;
    TRACE( 2, ("IpRecvInd() Buffers low, sending src quench to %08lx\n", src));

    hb = SWS_BufferDup( hBuffer, hdr + 8);
    if ( NULL != hb)
      {
      SWS_IcmpSend(
        uHostIp, src,
        SWS_ICMP_QUENCH, 0, 0,
        hb,
        SWS_IP_IDENT_DEFAULT,
        NULL
      );
      SWS_BufferRelease( hb);
      }
    }
#endif

  i = 0;
#if OPT_PROTO_ENTRIES
  /* Pass to matching protocol(s) */
  {
  SProtocol* pProtocol;
  for (
    pProtocol = IpFindProtocol( protocol, NULL, 0);
    NULL != pProtocol;
    pProtocol = IpFindProtocol( protocol, pProtocol, 0)
  ) {
    ASSERT( pProtocol->pfnRecvInd);
    (*pProtocol->pfnRecvInd)(
      pProtocol->pvContext,
      hBuffer, hdr,
      src, dst, uHostIp,
      tos
    );
    ++i;
    }
  }
#endif

  /* Pass to buitin protocols */
  i += IpRecvBuiltin( protocol, hBuffer, hdr, src, dst, uHostIp);
  if ( 0 == i)
    {
    TRACE( 3, ("IpRecvInd() protocol %u unreachable\n", protocol));
    ASSERT( SWS_IPPROTO_ICMP != protocol);
#if defined OPT_PROTO_ICMP && defined SWS_SOCK_TUNNEL
    ASSERT( SWS_ICMP_PROTOCOL != protocol);
#endif

    AtomicIncrement( &s_aRxNoProtocol);
    SWS_GefRaise(
      SWS_kGefIpError, hBuffer, SWS_kErrorRxProtocol, "protocol not known");

#if !OPT_IP_DISCARD_NOPROTO && OPT_PROTO_ICMP
    /* Send ICMP protocol unreachable if datagram destination specified this host */
    if ( !s_ignore_noproto
      && !pInfo->broadcast
      && dst == uHostIp
    ) {
      /* Send an ICMP 'protocol unknown' */
      /* NB May modify the buffer since no protocol has a ref */
      SWS_IcmpSend(
        uHostIp, src,
        SWS_ICMP_DEST_UNREACH,
        SWS_ICMP_PROT_UNREACH,
        0 /*ident&seq*/,
        hBuffer,
        0,
        NULL
      );
      }
#endif
    }

#if OPT_IP_REASSEMBLY_BUFFERS
  /* Release reassembly buffer */
  if ( NULL != pReassembly)
    ReassemblyPut( pReassembly);
#endif

  return 0;
  }


#if OPT_IP_ROUTED
/*
 * Route a datagram
 * See Requirements for IP Version 4 Routers (RFC1812)
 */
int IpForward( SWS_BufferH hBuffer, ui32 uHostIp, ui32 dst)
  {
  SWS_PTR(void) pvIpHdr;
  SWS_size_t uLen;
  ui8 tos, ttl, pcol;
  ui16 offset;
#if OPT_PROTO_ICMP
  ui8 code,type;
  ui32 data;
#endif
  SWS_SIpRouteInfo route;

  TRACE( 9, ("IpForward(%p,%08lx,%08lx)\n", hBuffer, uHostIp, dst));
  ASSERT( NULL != hBuffer);
  ASSERT( PlatformIsCritical());

  pvIpHdr = SWS_BufferGetFragment( hBuffer, 0, &uLen);
  ASSERT( NULL != pvIpHdr && uLen >= SWS_IPHDR_MINLEN);

  SWS_PEEK16( offset, pvIpHdr, SWS_IPHDR_FLAGS_OFFSET );
  SWS_PEEK8( tos, pvIpHdr, SWS_IPHDR_TOS);
  SWS_PEEK8( ttl, pvIpHdr, SWS_IPHDR_TTL);

  /* Check TTL > 1 */
  if ( ttl <= 1)
    {
    /* Timed out */
#if OPT_PROTO_ICMP
    code = SWS_ICMP_TIME_EXCEED;
    type = SWS_ICMP_TTL_EXCEED;
    data = 0;
#endif
    }
  /* Route possible?  */
  else if ( IpRoute( SWS_INADDR_ANY, dst, tos, &route))
    {
    /* No, destination unreachable */
#if OPT_PROTO_ICMP
    code = SWS_ICMP_DEST_UNREACH;
    type = SWS_ICMP_HOST_UNREACH;
    data = 0;
#endif
    }
  /* Don't send back over same i/f */
  else if ( route.src == uHostIp)
    {
    /* Redirect */
#if OPT_PROTO_ICMP
    code = SWS_ICMP_REDIRECT;
    type = SWS_ICMP_REDR_HOST;
    data = route.nextHop;
#endif
    }
  /* Ensure fragmentation OK */
  else if ( (offset & SWS_IP_FLAG_DF)
    && route.mtu < SWS_BufferSize( hBuffer)
  ) {
#if OPT_PROTO_ICMP
    code = SWS_ICMP_DEST_UNREACH;
    type = SWS_ICMP_FRAG_NEEDED;
    data = 0;
#endif
    }
  else
    {
    SWS_IpOptions opts;
    unsigned hdr;

    /* Decrement TTL */
    SWS_POKE8( pvIpHdr, SWS_IPHDR_TTL, (ui8)(ttl - 1));

    /* Set dst */
    SWS_POKE32( pvIpHdr, SWS_IPHDR_DST, dst);

    SWS_PEEK8( hdr, pvIpHdr, SWS_IPHDR_VER_IHL);
    hdr = 4 * (hdr & 0xf);
    ASSERT( hdr >= SWS_IPHDR_MINLEN);

    /* Update hdr checksum */
    SWS_POKE16( pvIpHdr, SWS_IPHDR_CHECKSUM, 0 );
    SWS_POKECSUM( pvIpHdr, SWS_IPHDR_CHECKSUM, SWS_IpChecksum( hBuffer, 0, hdr, 0) );

    /* Send it on */
    memset( &opts, 0, sizeof( opts));
    opts.flags.hdrincl = 1;
    SWS_PEEK8( pcol, pvIpHdr, SWS_IPHDR_PROTOCOL );
    return SWS_IpSend( route.src, dst, pcol, hBuffer, SWS_IP_IDENT_DEFAULT, &opts);
    }

#if OPT_PROTO_ICMP
  SWS_PEEK8( pcol, pvIpHdr, SWS_IPHDR_PROTOCOL );
  if ( SWS_IPPROTO_ICMP != pcol)
    {
    /*  Send ICMP */
    SWS_PEEK32( dst, pvIpHdr, SWS_IPHDR_SRC);
    SWS_IcmpSend(
      uHostIp, dst,
      code, type, data,
      hBuffer,
      SWS_IP_IDENT_DEFAULT,
      NULL
    );
    }
#endif
  return SWS_EHOSTUNREACH;
  }
#endif


/*
 * Check if protocol is builtin
 */
static int IpRecvBuiltin(
  unsigned protocol,
  SWS_BufferH hBuffer,
  size_t hdr,
  ui32 src,
  ui32 dst,
  ui32 uHostIp
) {
#if !OPT_PROTO_IGMP
  (void)dst;
#endif
#if !OPT_PROTO_ICMP
  (void)hBuffer;
  (void)hdr;
  (void)src;
  (void)uHostIp;
#endif

  switch ( protocol)
    {
#if OPT_PROTO_IGMP
  case SWS_IGMP_PROTOCOL:
#ifdef SWS_SOCK_TUNNEL
  case SWS_IPPROTO_IGMP:
#endif
    if ( s_iIgmpVersion > 0)
      SWS_IgmpRecv( hBuffer, hdr, src, dst, uHostIp);
    return 1;
#endif

  case SWS_IPPROTO_ICMP:
#if OPT_PROTO_ICMP
#ifdef SWS_SOCK_TUNNEL
  case SWS_ICMP_PROTOCOL:
#endif
    SWS_IcmpRecv( hBuffer, hdr, src, dst, uHostIp);
#endif
    return 1; /* Handled */

  default:
    break;
    }

  return 0; /* Not handled */
  }


#if OPT_PROTO_ICMP
/*
 * Handle ICMP error messages
 */
void SWS_IpIcmpError(
  SWS_BufferH hBuffer,
  size_t uHdrLen,
  ui32 src,
  ui32 dst,                           /* IN: specific adapter IP address */
  unsigned type,
  unsigned code,
  ui32 data
) {
  SWS_PTR( void) pvHdr;
  SWS_size_t uLen;
  unsigned protocol;

  TRACE( 9, ("SWS_IpIcmpError(%p,%u,%08lx,%08lx,%u,%u,%lu)\n",
    hBuffer,uHdrLen,src,dst,type,code,data));
  ASSERT( NULL != hBuffer);
  ASSERT( PlatformIsCritical());

  /* Get -> IP header */
  pvHdr = SWS_BufferGetFragment( hBuffer, uHdrLen, &uLen);
  if ( NULL == pvHdr || uLen < SWS_IPHDR_MINLEN)
    {
    TRACE( 1, ("***SWS_IpIcmpError() truncated IP header\n"));
    return;
    }

  SWS_PEEK8( protocol, pvHdr, SWS_IPHDR_PROTOCOL);
  SWS_GefRaise( SWS_kGefIcmpError, hBuffer, protocol, type, code, data);

  TRACE( 2, ("SWS_IpIcmpError() src %08lx i/f %08lx prtcl %u, type %u, code %u\n",
    src,dst,protocol,type,code));

#if OPT_PROTO_ENTRIES
  {
  SProtocol* pProtocol;
  unsigned hdr;

  SWS_PEEK8( hdr, pvHdr, SWS_IPHDR_VER_IHL);
  hdr = 4 * (hdr & 0xf);

  SWS_PEEK32( src, pvHdr, SWS_IPHDR_SRC);
  SWS_PEEK32( dst, pvHdr, SWS_IPHDR_DST);

  /* Find the original protocol */
  for (
    pProtocol = IpFindProtocol( protocol, NULL, 0);
    NULL != pProtocol;
    pProtocol = IpFindProtocol( protocol, pProtocol, 0)
  ) {
    /* Pass error up to protocol handler */
    (*pProtocol->pfnIcmpInd)(
      pProtocol->pvContext,
      hBuffer, uHdrLen + hdr,
      src, dst,
      type, code, data
    );
    }
  }
#else
  (void)src;
  (void)dst;
#endif
  }


/*
 * Process a received ICMP redirect message
 */
void SWS_IpIcmpRedirect(
  SWS_BufferH hBuffer,
  size_t uHdrLen,
  ui32 src,
  ui32 dst,
  unsigned code,
  ui32 gateway
) {
  TRACE( 2, ("SWS_IpIcmpRedirect(%p,%u,%08lx,%08lx,%u,%08lx)\n",
    hBuffer,uHdrLen,src,dst,code,gateway));

#if OPT_IP_ROUTES && !OPT_DEMO
  {
  SWS_PTR( void) pvHdr;
  SWS_size_t uLen;
  ui32 host;
  unsigned tos;
  SWS_SIpRouteInfo info;
  (void)code;

  /* Get the intended recipient's IP address */
  pvHdr = SWS_BufferGetFragment( hBuffer, uHdrLen, &uLen);
  ASSERT( NULL != pvHdr && uLen >= SWS_IPHDR_MINLEN);
  SWS_PEEK32( host, pvHdr, SWS_IPHDR_DST);
  SWS_PEEK8( tos, pvHdr, SWS_IPHDR_TOS);

  /* RFC 1122 3.2.2.2
   * A Redirect message SHOULD be silently discarded if the new
   * gateway address it specifies is not on the same connected
   * (sub-) net through which the Redirect arrived [INTRO:2,
   * Appendix A], or if the source of the Redirect is not the
   * current first-hop gateway for the specified destination (see
   * Section 3.3.1).
   */
  if ( 0 != IpRoute( dst, host, tos, &info))
    {
    TRACE( 1, ("*** SWS_IpIcmpRedirect() gateway %08lx not local\n", host));
    }
  else if ( info.nextHop != src
    && info.nextHop != info.src /* Accept from any if gateway= src */
  ) {
    TRACE( 1, ("*** SWS_IpIcmpRedirect() redirect not from gateway %08lx\n", info.nextHop));
    }
  else
    {
    /* Update the routing table */
    RouteAdd( IpFindInterface( dst), host, ~0UL, tos, gateway, 0);
    }
  }
#else
  (void)hBuffer;
  (void)uHdrLen;
  (void)src;
  (void)dst;
  (void)code;
  (void)gateway;
#endif
  }
#endif /*OPT_PROTO_ICMP*/


#if OPT_IP_OPTIONS
/*
 * Update IP timestamp option
 */
static void IpTimestamp( void* pv, unsigned offset, ui32 uHostIp)
  {
  unsigned opt, len, ptr;

  SWS_PEEK8( len, pv, offset + 1);
  SWS_PEEK8( ptr, pv, offset + 2); /* pointer */
  if ( ptr < len)
    {
    ui32 addr, ticks;

    ticks = PlatformTime();

    SWS_PEEK8( opt, pv, offset + 3); /* flags */
    switch ( opt & 0xf)
      {
    case 0:
      SWS_POKE32( pv, offset + ptr - 1, ticks );
      ptr += 4;
      break;
    case 1:
      SWS_POKE32( pv, offset + ptr - 1, uHostIp);
      SWS_POKE32( pv, offset + ptr + 3, ticks );
      ptr += 8;
      break;
    case 3:
      SWS_PEEK32( addr, pv, offset + ptr - 1);
      if ( addr == uHostIp)
        {
        SWS_POKE32( pv, offset + ptr + 3, ticks );
        ptr += 8;
        }
      break;
    default:
      break;
      }

    SWS_POKE8( pv, offset + 2, (ui8)ptr);
    }
  else
    {
    /* Bump overflows */
    SWS_PEEK8( opt, pv, offset + 3); /* flags */
    opt += 1<<4;
    if ( opt <= 255)
      SWS_POKE8( pv, offset + 3, (ui8)opt);
    }
  }
#endif


/****************************************************************************
 * Routing
 ****************************************************************************/

/*
 * Install routing filter
 */
SWS_EXPORT( int, SWS_IpSetRouteFilter( /* !0 = error */
  SWS_FNPTR( SWS_FIpRouter) pfn,      /* IN: -> Filter callback */
  SWS_PTR( void) pv
)){
  TRACE( 9, ("SWS_IpSetRouteFilter(%p,%p)\n",pfn,pv));

  if ( s_pfnRoute && pfn)
    return SWS_EACCES;

  s_pfnRoute = 0;
  s_pvRouteCtx = pv;
  s_pfnRoute = pfn;

  return 0;
  }


/*
 * Get interface & next hop for a given route
 */
static int IpRoute(                   /* Returns !0 on error */
  ui32 src,                           /* IN: Local IP address */
  ui32 dst,                           /* IN: Remote IP address */
  unsigned tos,                       /* IN: Type Of Service - IPTOS_xxx */
  SWS_SIpRouteInfo* pInfo             /* OUT: Interface/NextHop/MTU */
) {
#if OPT_IP_ROUTES
  const SWS_SRoute* pRoute;
#else
  SInterface* pInterface = NULL;
  unsigned uInterface;
  ui32 uSubnetMask;
#if !OPT_DEMO
  ui32 gateway;
#endif
  (void)tos;
#endif

  TRACE( 9, ("IpRoute(%lx,%lx,%u,%p)\n", src,dst,tos,pInfo));
  ASSERT( NULL != pInfo);

  /* Client routing callback */
  if ( s_pfnRoute && 0 == (*s_pfnRoute)( s_pvRouteCtx, src, dst, tos, pInfo))
    return 0;

#if OPT_DATALINK_P2P
  {
  int iRetries = 0;
retry:
#endif

#if OPT_IP_ROUTES
  /* Is there an established route? */
  pRoute = RouteFindBest( src, dst, tos);
  if ( NULL != pRoute)
    {
    /* Yes */
    pInfo->src = InterfaceAddress( pRoute->pInterface);
    pInfo->mtu = InterfaceMtu( pRoute->pInterface);

    if ( SWS_INADDR_LOOPBACK == pRoute->nextHop)
      {
      /* Loop this back */
      pInfo->nextHop = dst;
      if ( SWS_INADDR_ANY == src)
        pInfo->src = dst;
      }
    else if ( SWS_INADDR_ANY == pRoute->nextHop)
      {
      /* Local sub-net */
      if ( SWS_INADDR_ANY == (dst & pRoute->netMask))
        {
        pInfo->nextHop = (dst & ~pRoute->netMask) | (pInfo->src & pRoute->netMask);
        }
      else
        {
        pInfo->nextHop = dst;
        }
      }
    else
      {
      /* Explicit route */
      pInfo->nextHop = pRoute->nextHop;

      /* Is it a default gateway? */
      if ( SWS_INADDR_ANY == pRoute->netMask)
        {
        /* Yes, add an explicit route */
        RouteAdd(
          pRoute->pInterface,
          dst,
          ~0UL,
          SWS_IP_TOS_DEFAULT,
          pRoute->nextHop, 0
        );
        }
      }

    /* TODO: Dead gateway detection RFC1122 (STD3) p51 & RFC816 */
    return 0;
    }

  /* No route, check valid src */
  if ( SWS_INADDR_ANY != src
    && NULL == IpFindInterface( src)
  ) {
    TRACE( 1, ("*** IpRoute() No interface with address %08lx\n", src));
    return SWS_EADDRNOTAVAIL;
    }

#else

  pInfo->src = SWS_INADDR_NONE;
  pInfo->nextHop = SWS_INADDR_ANY;
  pInfo->mtu = 0;

  if ( SWS_INADDR_ANY != src)
    {
    /* Check valid src */
    pInterface = IpFindInterface( src);
    if ( NULL == pInterface)
      {
      TRACE( 1, ("*** IpRoute() No interface with address %08lx\n", src));
      return SWS_EADDRNOTAVAIL;
      }
    pInfo->src = src;
    }

  /* Check for loopback dst */
  if ( (dst & SWS_IN_CLASSA_NET) == (SWS_INADDR_LOOPBACK & SWS_IN_CLASSA_NET))
    {
    /* Sending to loopback */
    pInterface = IpFindInterface( SWS_INADDR_LOOPBACK);
    if ( NULL == pInterface)
      {
      TRACE( 1, ("*** IpRoute() No loopback interface\n"));
      return SWS_EADDRNOTAVAIL;
      }

    pInfo->nextHop = SWS_INADDR_LOOPBACK;
    pInfo->src = InterfaceAddress( pInterface);
    pInfo->mtu = InterfaceMtu( pInterface);
    return 0;
    }

  /* Local adapter unspecified? */
  if ( SWS_INADDR_ANY != src)
    {
    /* Yes, use standard routing */
    ASSERT( NULL != pInterface);

    /* Get subnet mask */
    uSubnetMask = InterfaceSubnet( pInterface);

    /* On local sub-net? */
    if ( (uSubnetMask & src) == (uSubnetMask & dst) )
      {
      if ( (~uSubnetMask & dst) == (~uSubnetMask & SWS_INADDR_BROADCAST))
        {
        /* Local sub-net broadcast */
        pInfo->nextHop = SWS_INADDR_BROADCAST;
        }
      else
        {
        /* Host on local sub-net */
        pInfo->nextHop = dst;
        }

      /* Use mtu for the local subnet */
      pInfo->mtu = InterfaceMtu( pInterface);
      return 0;
      }
    /* Limited broadcast? */
    else if ( dst == SWS_INADDR_BROADCAST)
      {
      pInfo->nextHop = dst;

      /* Use mtu for the local subnet */
      pInfo->mtu = InterfaceMtu( pInterface);
      return 0;
      }
    /* Multicast group? */
    else if ( SWS_IN_MULTICAST( dst))
      {
      pInfo->nextHop = dst;

      /* Use mtu for the local subnet */
      pInfo->mtu = InterfaceMtu( pInterface);
      return 0;
      }
#if !OPT_DEMO
    else
      {
      /* Send to default gateway */
      gateway = InterfaceGateway( pInterface);
      switch ( gateway)
        {
      default:
        /* Gateway valid */
        pInfo->nextHop = gateway;
#if 0
        pInfo->mtu = SWS_IP_MIN_MTU;
#else
        pInfo->mtu = InterfaceMtu( pInterface);
#endif
        return 0;

      case SWS_INADDR_NONE:
        /* No gateway */
        break;

      case SWS_INADDR_ANY:
        /* Gateway unset. Hope for proxy arp */
        pInfo->nextHop = dst;
        pInfo->mtu = SWS_IP_MIN_MTU;
        return 0;
        }
      }
#endif
    /* No route or gateway */
    }
  else
    {
    ui32 uIpAddr;

    /* Find an interface on same subnet as dst */
    for (
      pInterface = &s_interfaceArray[ uInterface = 0];
      uInterface < s_uInterfaces;
      ++pInterface, ++uInterface
    ) {
      /* Get the IP address */
      uIpAddr = InterfaceAddress( pInterface);

      if ( SWS_INADDR_LOOPBACK == uIpAddr)
        continue; /* dst net isn't loopback so ignore */

      if ( SWS_INADDR_BROADCAST == dst)
        {
        /* limited broadcast */
        pInfo->nextHop = dst;
        pInfo->src = uIpAddr;
        pInfo->mtu = InterfaceMtu( pInterface);
        return 0;
        }

      if ( SWS_INADDR_ANY == uIpAddr)
        continue; /* IP address not set */

      if ( SWS_IN_MULTICAST( dst))
        {
        /* Multicast group */
        pInfo->nextHop = dst;
        pInfo->src = uIpAddr;

        /* Use mtu for the local subnet */
        pInfo->mtu = InterfaceMtu( pInterface);
        return 0;
        }

      /* Get subnet mask */
      uSubnetMask = InterfaceSubnet( pInterface);

      if ( 0 == (uSubnetMask & dst))
        {
        /* Dst on this net e.g. 0.0.x.y */
        pInfo->nextHop = dst | (uSubnetMask & uIpAddr);
        pInfo->src = uIpAddr;
        pInfo->mtu = InterfaceMtu( pInterface);
        return 0;
        }

      if ( (uSubnetMask & uIpAddr) == (uSubnetMask & dst) )
        {
        /* Local subnet */
        pInfo->nextHop = dst;
        pInfo->src = uIpAddr;
        pInfo->mtu = InterfaceMtu( pInterface);
        return 0;
        }
      }

#if !OPT_DEMO
    /* Not local subnet so select 1st adapter with a gateway */
    for (
      pInterface = &s_interfaceArray[ uInterface = 0];
      uInterface < s_uInterfaces;
      ++pInterface, ++uInterface
    ) {
      uIpAddr = InterfaceAddress( pInterface);
      if ( SWS_INADDR_LOOPBACK == uIpAddr)
        continue; /* Loopback adapter */

      if ( SWS_INADDR_ANY == uIpAddr)
        {
        /* Select unconfigured interface in case attempting bootp etc */
        pInfo->nextHop = dst;
        pInfo->src = SWS_INADDR_ANY;
        pInfo->mtu = SWS_IP_MIN_MTU;
        return 0;
        }

      gateway = InterfaceGateway( pInterface);
      switch ( gateway)
        {
      case SWS_INADDR_NONE:
        /* No gateway */
        break;

      case SWS_INADDR_ANY:
        /* Gateway unset. Hope for proxy arp */
        pInfo->nextHop = dst;
        pInfo->src = uIpAddr;
        pInfo->mtu = SWS_IP_MIN_MTU;
        return 0;

      default:
        /* Gateway valid */
        pInfo->nextHop = gateway;
        pInfo->src = uIpAddr;
#if 0
        pInfo->mtu = SWS_IP_MIN_MTU;
#else
        pInfo->mtu = InterfaceMtu( pInterface);
#endif
        return 0;
        }
      }
#endif

#if OPT_DATALINK_LOOPBACK
    /* If nothing else use loopback */
    if ( SWS_INADDR_ANY == dst
      && SWS_INADDR_ANY == src
    ) {
      pInterface = IpFindInterface( SWS_INADDR_LOOPBACK);
      if ( NULL != pInterface)
        {
        pInfo->nextHop = SWS_INADDR_LOOPBACK;
        pInfo->src = SWS_INADDR_LOOPBACK;
        pInfo->mtu = InterfaceMtu( pInterface);
        return 0;
        }
      }
#endif
    /* No route or gateway */
    }

#endif /* !OPT_IP_ROUTES */

#if OPT_DATALINK_P2P
  /* If this is the 1st attempt try dial on demand */
  if ( 1 == ++iRetries
    && SWS_INADDR_ANY == src
    && SWS_INADDR_ANY != dst
    && 0 == SWS_P2pDialOnDemand( dst)
  ) {
    goto retry;
    }
  }
#endif

  TRACE( 1, ("*** IpRoute() No route from %08lx to %08lx\n", src,dst));
  return SWS_ENETUNREACH;
  }


#if OPT_IP_ROUTES
/*
 * Rebuild the route cache
 * See RFC1122 (STD3) p48 for discussion on routing tables
 */
void RouteInit( void)
  {
  size_t u;
  SInterface* pInterface;

  TRACE( 9, ("RouteInit()\n"));
  ASSERT( NULL != s_pRouteCache);
  ASSERT( PlatformIsCritical());

  /* Empty the route table */
  for ( u = 0; u < lengthof( s_routeUsed); ++u)
    DEQUE_INIT( &s_routeUsed[ u]);

  /* Setup the free list */
  DEQUE_INIT( &s_routeFree);
  for ( u = 0; u < s_uRouteCacheSize; ++u)
    {
    SWS_SRoute* const pRoute = &s_pRouteCache[ u];
    DEQUE_INIT( &pRoute->link);
    pRoute->dst = SWS_INADDR_NONE;
    pRoute->nextHop = SWS_INADDR_NONE;
    pRoute->netMask = SWS_INADDR_NONE;
    pRoute->pInterface = NULL;
    SWS_DequePrepend( &s_routeFree, &pRoute->link);
    }

  /* Add standard routes for each interface */
  u = s_uInterfaces;
  for (
    pInterface = &s_interfaceArray[ 0];
    u > 0;
    ++pInterface, --u
  ) {
    RouteAddInterface( pInterface);
    }
  }


/*
 * Add standard routes for an interface
 */
void RouteAddInterface( SInterface* pInterface)
  {
  ui32 src, netMask, gateway;
  unsigned u;

  TRACE( 9, ("RouteAddInterface(%p)\n", pInterface));
  ASSERT( NULL != pInterface);
  ASSERT( INTERFACE_MAGIC == pInterface->uMagic);

  netMask = InterfaceSubnet( pInterface);
  src = InterfaceAddress( pInterface);

  if ( SWS_INADDR_LOOPBACK != src)
    {
    /* Limited broadcast */
    RouteAdd( pInterface, SWS_INADDR_BROADCAST, ~0UL, SWS_IP_TOS_DEFAULT, SWS_INADDR_BROADCAST, 1);
    ASSERT( RouteFindExact( SWS_INADDR_ANY, SWS_INADDR_BROADCAST, SWS_IP_TOS_DEFAULT));

    /* Multicast */
    RouteAdd( pInterface, kMulticastGroup, SWS_IN_CLASSD_NET, SWS_IP_TOS_DEFAULT, SWS_INADDR_ANY, 1);
    ASSERT( RouteFindBest( SWS_INADDR_ANY, kMulticastGroup + 0x010101, SWS_IP_TOS_DEFAULT));

    if ( SWS_INADDR_ANY != src)
      {
      /* sub-net broadcast */
      RouteAdd( pInterface, src | ~netMask, ~0UL, SWS_IP_TOS_DEFAULT, SWS_INADDR_BROADCAST, 1);
      ASSERT( RouteFindExact( src, src | ~netMask, SWS_IP_TOS_DEFAULT));

#if OPT_DATALINK_LOOPBACK
      {
      SInterface* pIf = IpFindInterface( SWS_INADDR_LOOPBACK);
      if ( NULL != pIf)
        {
        /* From loopback to this interface */
        RouteAdd( pIf, src, ~0UL, SWS_IP_TOS_DEFAULT, SWS_INADDR_LOOPBACK, 1);
        ASSERT( RouteFindExact( SWS_INADDR_LOOPBACK, src, SWS_IP_TOS_DEFAULT));
        }
      }
#endif

#if 0 /* This net i.e. 0.x.y.z is obsolete */
      if ( ~0UL != netMask)
        {
        /* Dst on this net e.g. 0.x.y.z */
        RouteAdd( pInterface,
          src & 0x00ffffffL & netMask,
          netMask,
          SWS_IP_TOS_DEFAULT,
          SWS_INADDR_ANY, 1
        );
        ASSERT( RouteFindBest( src, 0x00000001, SWS_IP_TOS_DEFAULT));
        }
#endif
      }
    }

  if ( SWS_INADDR_ANY != src && ~0UL != netMask)
    {
    /* sub-net unicast */
    RouteAdd(
      pInterface, src & netMask, netMask, SWS_IP_TOS_DEFAULT, SWS_INADDR_ANY, 1);
    ASSERT( RouteFindBest( src, src + 1, SWS_IP_TOS_DEFAULT));
    }

  gateway = InterfaceGateway( pInterface);
  if ( SWS_INADDR_NONE != gateway)
    {
    /* The default gateway */
    if ( SWS_INADDR_ANY != gateway)
      {
      RouteAdd( pInterface, SWS_INADDR_ANY, 0, SWS_IP_TOS_DEFAULT, gateway, 1);
      }
    else
      {
      /* Gateway unset. Hope for proxy arp */
      RouteAdd(
        pInterface, SWS_INADDR_ANY, 0, SWS_IP_TOS_DEFAULT, SWS_INADDR_ANY, 1);
      }

    ASSERT( RouteFindBest( src, SWS_INADDR_ANY, SWS_IP_TOS_DEFAULT));
    }

  /* Add user routes */
  for ( u = 1; ; ++u)
    {
    char szBuf[ 96];
    char szAdapter[ 32];
    unsigned dst[4], msk[4], nxt[4];
    int i;

    sprintf ( szAdapter, IP_USER_ROUTE "%u", u);
    if ( 0 == PlatformPropertyGetString( PSECT_IP, szAdapter, "", szBuf, sizeof( szBuf) ))
      break;

    i = sscanf( szBuf, "%u.%u.%u.%u %u.%u.%u.%u %u.%u.%u.%u %31s",
      &dst[0], &dst[1], &dst[2], &dst[3],
      &msk[0], &msk[1], &msk[2], &msk[3],
      &nxt[0], &nxt[1], &nxt[2], &nxt[3],
      szAdapter
    );
    if ( 13 == i
      && !strcmp( szAdapter, pInterface->szIdent)
    ) {
      RouteAdd(
        pInterface,
        ((ui32)dst[0] << 24) | ((ui32)dst[1] << 16) | ((ui32)dst[2] << 8) | dst[3],
        ((ui32)msk[0] << 24) | ((ui32)msk[1] << 16) | ((ui32)msk[2] << 8) | msk[3],
        SWS_IP_TOS_DEFAULT,
        ((ui32)nxt[0] << 24) | ((ui32)nxt[1] << 16) | ((ui32)nxt[2] << 8) | nxt[3],
        1
      );
      }
    }
  }


/*
 * Remove all routes for an interface
 */
void RouteRemoveInterface( SInterface* pInterface)
  {
  SWS_PTR( SWS_SRoute) pRoute;

  TRACE( 9, ("RouteRemoveInterface(%p)\n", pInterface));
  ASSERT( NULL != pInterface);

  for (
    pRoute = SWS_RouteBegin();
    NULL != pRoute;
    pRoute = SWS_RouteNext( pRoute)
  ) {
    if ( pInterface == pRoute->pInterface
      || (SWS_INADDR_ANY != pInterface->uIpAddr && pInterface->uIpAddr == pRoute->dst)
    ) {
      SWS_RouteDelete( pRoute);
      }
    }
  }


/*
 * Hash destination address to route table list
 */
SWS_FASTCALL( SDeque*, RouteHashTable( ui32 addr))
  {
  unsigned shift;

  if ( SWS_IN_CLASSA( addr) )
    shift = SWS_IN_CLASSA_NSHIFT;
  else if ( SWS_IN_CLASSB( addr) )
    shift = 20;
  else if ( SWS_IN_CLASSC( addr) )
    shift = 20; /* NB not SWS_IN_CLASSC_NSHIFT as this is too variable */
  else if ( SWS_IN_CLASSD( addr) )
    shift = SWS_IN_CLASSD_NSHIFT;
  else
    return &s_routeUsed[ 0];

  return &s_routeUsed[ (size_t)(addr >> shift) % lengthof( s_routeUsed) ];
  }


/*
 * Search the routing table
 */
SWS_SRoute* RouteFind(
  ui32 src,
  ui32 dst,
  unsigned tos,
  EMatch mode
) {
  SDeque* const pHead = RouteHashTable( kMatchAny != mode ? dst : SWS_INADDR_ANY);
  SDeque* pItem;
  SWS_SRoute* pBestRoute = NULL;
  (void)tos;

  TRACE( 9, ("RouteFind(%08lx,%08lx,%u,%d)\n", src,dst,tos,mode));

  /* Search the route table, LRU first */
  for (
    pItem = DEQUE_FIRST( pHead);
    pItem != pHead;
    pItem = DEQUE_FIRST( pItem)
  ) {
    SWS_SRoute* const pRoute = DEQ2ROUTE( pItem);
    ASSERT( NULL != pRoute->pInterface);

    /* Matching src? */
    if ( SWS_INADDR_ANY == src
      || InterfaceAddress( pRoute->pInterface) == src
    ) {
      /* Yes, compare dst */
      if ( dst == pRoute->dst)
        {
        /* Exact match */
        pBestRoute = pRoute;
        break;
        }

      /* Does network/subnet match? */
      if ( kMatchExact != mode
        && (dst & pRoute->netMask) == pRoute->dst
        && (SWS_INADDR_ANY != pRoute->netMask
          /* Only consider default routes for hosts on class A, B or C nets */
          || dst < kMulticastGroup )
      ) {
        /* Yes, check if better match (longer netmask) then previous */
#       define IPDIST(i1,i2) ((i1) >= (i2) ? (i1) - (i2) : (i2) - (i1))
        if ( NULL == pBestRoute
          || pRoute->netMask > pBestRoute->netMask
          || (pRoute->netMask == pBestRoute->netMask
            && IPDIST( pRoute->nextHop, dst) < IPDIST( pBestRoute->nextHop, dst)
            )
        ) {
          pBestRoute = pRoute;
          }
        }
      }
    }

  /* Route found? */
  if ( NULL != pBestRoute)
    {
    /* Yes, move to front of LRU list */
    if ( &pBestRoute->link != DEQUE_FIRST( pHead))
      {
      SWS_DequeRemove( &pBestRoute->link);
      SWS_DequePrepend( pHead, &pBestRoute->link);
      }
    pBestRoute->timeLastUsed = PlatformTime();
    }
  /* No, is a best match required? */
  else if ( kMatchBest == mode)
    {
#if !OPT_DEMO
    /* Yes, try again including default routes */
    pBestRoute = RouteFind( src, dst, tos, kMatchAny);
#endif

#if OPT_DATALINK_LOOPBACK
    /* If nothing else use loopback */
    if ( NULL == pBestRoute
      && SWS_INADDR_ANY == dst
      && SWS_INADDR_ANY == src
    ) {
      pBestRoute = RouteFind( SWS_INADDR_LOOPBACK, SWS_INADDR_LOOPBACK, tos, kMatchBest);
      }
#endif
    }

  return pBestRoute;
  }


/*
 * Add a route to the table
 */
SWS_SRoute* RouteAdd(
  SInterface* pInterface,
  ui32 dst,
  ui32 netMask,
  unsigned tos,
  ui32 nextHop,
  unsigned sticky
) {
  SDeque* pItem;
  SWS_SRoute* pRoute;
  ui32 src;

  TRACE( 2, ("RouteAdd(%p,%08lx,%08lx,%u,%08lx,%#x)\n", pInterface,dst,netMask,tos,nextHop,sticky));
  ASSERT( NULL != pInterface);

  /* Check if route already present */
  src = InterfaceAddress( pInterface);
  if ( SWS_INADDR_ANY != src
    && NULL != (pRoute = (SWS_SRoute*) RouteFindExact( src, dst, tos))
    && pRoute->netMask == netMask
  ) {
    ASSERT( pRoute->pInterface == pInterface);
    ASSERT( pRoute->dst == dst);

    /* Update the next hop address */
    pRoute->nextHop = nextHop;

    return pRoute;
    }

  /* Get an unused route */
  pItem = DEQUE_FIRST( &s_routeFree);
  if ( &s_routeFree == pItem)
    {
    /* Recycle oldest non-sticky entry */
    size_t i;

    pRoute = NULL;

    for ( i = 0; i < lengthof( s_routeUsed); ++i)
      {
      SDeque* const pHead = &s_routeUsed[ i];
      for (
        pItem = DEQUE_LAST( pHead);
        pItem != pHead;
        pItem = DEQUE_LAST( pItem)
      ) {
        SWS_SRoute* p = DEQ2ROUTE( pItem);
        if ( !p->flags.sticky)
          {
          if ( NULL == pRoute
            || p->timeLastUsed < pRoute->timeLastUsed
          ) {
            pRoute = p;
            }
          }
        }
      }

    if ( NULL == pRoute)
      {
      TRACE( 1, ("*** RouteAdd() No free routes\n"));
      return NULL;
      }
    pItem = &pRoute->link;
    }
  SWS_DequeRemove( pItem);

  /* Save routing info */
  pRoute = DEQ2ROUTE( pItem);
  pRoute->dst = dst & netMask;
  pRoute->netMask = netMask;
  pRoute->nextHop = nextHop;
  pRoute->pInterface = pInterface;
  pRoute->flags.sticky = sticky;
  pRoute->timeLastUsed = PlatformTime();

  /* Add to routing table */
  SWS_DequePrepend( RouteHashTable( pRoute->dst), pItem);

  return pRoute;
  }
#endif /* OPT_IP_ROUTES */


/*****************************************************************************
 * Routing table ops
 ****************************************************************************/

/*
 * Reset the routing table
 */
SWS_EXPORT( void, SWS_RouteReset( void))
  {
  TRACE( 9, ("SWS_RouteReset()\n"));

#if OPT_IP_ROUTES
  PlatformSynchronise( &IpRouteResetSync, NULL);
#endif
  }

#if OPT_IP_ROUTES
SWS_CB( int, IpRouteResetSync(
  SWS_PTR( void) pvContext            /* IN: Client context */
)){
  (void)pvContext;

  TRACE( 9, ("IpRouteResetDpc()\n"));

  RouteInit();
  return 0;
  }
#endif


/*
 * Find the best route
 */
SWS_EXPORT( SWS_PTR( SWS_SRoute), SWS_RouteFind(
  ui32 src,                          /* IN: src ip or SWS_INADDR_ANY */
  ui32 dst,                          /* IN: dst ip */
  unsigned tos                       /* IN: Type Of Service - SWS_IPTOS_xxx */
)){
  TRACE( 9, ("SWS_RouteFind(%08lx,%08lx,%u)\n",src,dst,tos));

#if OPT_IP_ROUTES
  return RouteFindBest( src, dst, tos);
#else
  (void)src;
  (void)dst;
  (void)tos;
  return NULL;
#endif
  }


/*
 * Start iteration thru the routing table
 */
SWS_EXPORT( SWS_PTR( SWS_SRoute), SWS_RouteBegin( void))
  {
  TRACE( 9, ("SWS_RouteBegin()\n"));

#if OPT_IP_ROUTES
  {
  SWS_SRoute* pRoute = s_pRouteCache;
  size_t u = s_uRouteCacheSize;

  while ( u-- > 0)
    {
    /* Route in use? */
    if ( NULL != pRoute->pInterface)
      return pRoute; /* Yes, start here */

    ++pRoute;
    }
  }
#endif

  return NULL;
  }


/*
 * Get next entry in routing table
 */
SWS_EXPORT( SWS_PTR( SWS_SRoute), SWS_RouteNext(
  SWS_PTR( SWS_SRoute) pRoute      /* IN: current route */
)){
  TRACE( 9, ("SWS_RouteNext(%p)\n",pRoute));
  ASSERT( NULL != pRoute);

#if OPT_IP_ROUTES
  {
  size_t u = s_uRouteCacheSize - (size_t)(pRoute - (SWS_PTR( SWS_SRoute))s_pRouteCache);

  while ( u-- > 1)
    {
    ++pRoute;

    /* Route in use? */
    if ( NULL != pRoute->pInterface)
      return pRoute; /* Yes */
    }
  }
#else
  (void)pRoute;
#endif

  return NULL;
  }


/*
 * Get routing info
 */
SWS_EXPORT( ui32, SWS_RouteSrc(
  SWS_PTR( const SWS_SRoute) pRoute  /* IN: route */
)){
#if OPT_IP_ROUTES
  ASSERT( NULL != pRoute);
  return InterfaceAddress( pRoute->pInterface);
#else
  (void)pRoute;
  return SWS_INADDR_NONE;
#endif
  }

SWS_EXPORT( ui32, SWS_RouteDst(
  SWS_PTR( const SWS_SRoute) pRoute  /* IN: route */
)){
#if OPT_IP_ROUTES
  ASSERT( NULL != pRoute);
  return pRoute->dst;
#else
  (void)pRoute;
  return SWS_INADDR_NONE;
#endif
  }

SWS_EXPORT( ui32, SWS_RouteMask(
  SWS_PTR( const SWS_SRoute) pRoute  /* IN: route */
)){
#if OPT_IP_ROUTES
  ASSERT( NULL != pRoute);
  return pRoute->netMask;
#else
  (void)pRoute;
  return SWS_INADDR_NONE;
#endif
  }

SWS_EXPORT( ui32, SWS_RouteNextHop(
  SWS_PTR( const SWS_SRoute) pRoute  /* IN: route */
)){
#if OPT_IP_ROUTES
  ASSERT( NULL != pRoute);
  return pRoute->nextHop;
#else
  (void)pRoute;
  return SWS_INADDR_NONE;
#endif
  }


/*
 * Add/remove a route
 */
SWS_EXPORT( int, SWS_RouteDelete(
  SWS_PTR( SWS_SRoute) pRoute        /* IN: route */
)){
  TRACE( 9, ("SWS_RouteDelete(%p)\n",pRoute));

#if OPT_IP_ROUTES
  ASSERT( NULL != pRoute);

  SWS_DequeRemove( &pRoute->link);
  pRoute->dst = SWS_INADDR_NONE;
  pRoute->nextHop = SWS_INADDR_NONE;
  pRoute->netMask = SWS_INADDR_NONE;
  pRoute->pInterface = NULL;
  SWS_DequeAppend( &s_routeFree, &pRoute->link);

  return 0;
#else
  (void)pRoute;
  return SWS_EINVAL;
#endif
  }

SWS_EXPORT( SWS_PTR( SWS_SRoute), SWS_RouteAdd(
  ui32 src,                           /* IN: src */
  ui32 dst,                           /* IN: dst */
  ui32 netMask,                       /* IN: subnet mask */
  ui32 nextHop                        /* IN: next hop */
)){
  TRACE( 9, ("SWS_RouteAdd(%08lx,%08lx,%08lx,%08lx)\n",src,dst,netMask,nextHop));

#if OPT_IP_ROUTES
  {
  SInterface* pInterface = IpFindInterface( src);
  if ( NULL == pInterface)
    return NULL;

  return RouteAdd( pInterface, dst, netMask, SWS_IP_TOS_NORM, nextHop, 0);
  }
#else
  (void)src;
  (void)dst;
  (void)netMask;
  (void)nextHop;
  return NULL;
#endif
  }


#if OPT_IP_REASSEMBLY_BUFFERS
/****************************************************************************
 * Reassembly buffer management
 ****************************************************************************/

/*
 * Find an IP datagram awaiting reassembly
 * Single threaded
 */
static SReassembly* ReassemblyFind(
  ui32 src,
  ui32 dst,
  unsigned ident,
  unsigned protocol
) {
  SReassembly* pReassembly;

  ASSERT( PlatformIsCritical());

  for (
    pReassembly = s_pReassembling;
    NULL != pReassembly;
    pReassembly = pReassembly->pNext
  ) {
    ASSERT( NULL == pReassembly->pNext
      || pReassembly == pReassembly->pNext->pPrev);

    if ( pReassembly->src == src
      && pReassembly->dst == dst
      && pReassembly->ident == ident
      && pReassembly->protocol == protocol
    ) {
      /* Matched */
      break;
      }
    }
  return pReassembly;
  }


/*
 * Allocate a reassembly buffer for an IP datagram
 * Single threaded
 */
static SReassembly* ReassemblyGet(
  ui32 src,
  ui32 dst,
  unsigned ident,
  unsigned protocol
) {
  SReassembly* pReassembly;

  TRACE( 9, ("ReassemblyGet(%08lx,%08lx,%u,%u)\n", src,dst,ident,protocol));
  ASSERT( NULL == ReassemblyFind(src, dst, ident, protocol));
  ASSERT( PlatformIsCritical());

  /* Remove reassembly buffer from free list */
  pReassembly = s_pReassemblyList;
  if ( NULL != pReassembly)
    {
    s_pReassemblyList = pReassembly->pNext;

    /* Allocate a timer */
    pReassembly->hTimer = SWS_TimerAlloc( &ReassemblyTimeout, pReassembly);
    if ( NULL == pReassembly->hTimer)
      {
      s_pReassemblyList = pReassembly;
      return NULL;
      }

    /* Init reassembly resources */
    pReassembly->holes = 0;
    pReassembly->length = 0;
    pReassembly->hdr = SWS_IPHDR_MINLEN;
    pReassembly->hBuffer = NULL;
    pReassembly->iFragZero = 0;

    pReassembly->src = src;
    pReassembly->dst = dst;
    pReassembly->ident = (ui16)ident;
    pReassembly->protocol = (ui8)protocol;

    /* Add to pending list */
    pReassembly->pPrev = NULL;
    pReassembly->pNext = s_pReassembling;
    if ( NULL != s_pReassembling)
      s_pReassembling->pPrev = pReassembly;
    s_pReassembling = pReassembly;
    }

  return pReassembly;
  }


/*
 * Release a reassembly buffer
 * Single threaded
 */
static SWS_FASTCALL( void, ReassemblyPut( SReassembly* pReassembly))
  {
  TRACE( 9, ("ReassemblyPut(%p)\n", pReassembly));
  ASSERT( NULL != pReassembly);
  ASSERT( PlatformIsCritical());

  /* Stop the timer */
  if ( NULL != pReassembly->hTimer)
    SWS_TimerStop( pReassembly->hTimer);

  /* Unsplice from pending list */
  if ( NULL != pReassembly->pNext)
    pReassembly->pNext->pPrev = pReassembly->pPrev;

  if ( NULL == pReassembly->pPrev)
    {
    ASSERT( s_pReassembling == pReassembly);
    s_pReassembling = pReassembly->pNext;
    }
  else
    {
    ASSERT( s_pReassembling != pReassembly);
    pReassembly->pPrev->pNext = pReassembly->pNext;
    }

  /* Release buffer */
  if ( NULL != pReassembly->hBuffer)
    {
    SWS_BufferRelease( pReassembly->hBuffer);
    pReassembly->hBuffer = NULL;
    }

  /* Release timer */
  if ( NULL != pReassembly->hTimer)
    {
    SWS_TimerRelease( pReassembly->hTimer);
    pReassembly->hTimer = NULL;
    }

  /* Add to free list */
  pReassembly->pPrev = NULL;
  pReassembly->pNext = s_pReassemblyList;
  s_pReassemblyList = pReassembly;
  }


/*
 * Hole manipulators
 */
static INLINE( void, HoleSet(
  SWS_BufferH hBuffer,
  SWS_size_t indx,
  const SHole* pHole
)){
  VERIFY( !SWS_BufferCopyFromMemory(
    hBuffer,
    indx,
    pHole,
    sizeof( *pHole)
  ));
  }

static INLINE( void, HoleSetSize(
  SWS_BufferH hBuffer,
  SWS_size_t indx,
  ui16 val
)){
  VERIFY( !SWS_BufferCopyFromMemory(
    hBuffer,
    indx + offsetof( SHole, size),
    &val,
    sizeof( val)
  ));
  }

static INLINE( void, HoleSetNext(
  SWS_BufferH hBuffer,
  SWS_size_t indx,
  ui16 val
)){
  VERIFY( !SWS_BufferCopyFromMemory(
    hBuffer,
    indx + offsetof( SHole, next),
    &val,
    sizeof( val)
  ));
  }

static INLINE( void, HoleSetPrev(
  SWS_BufferH hBuffer,
  SWS_size_t indx,
  ui16 val
)){
  VERIFY( !SWS_BufferCopyFromMemory(
    hBuffer,
    indx + offsetof( SHole, prev),
    &val,
    sizeof( val)
  ));
  }

static INLINE( void, HoleGet(
  SWS_BufferH hBuffer,
  SWS_size_t indx,
  SHole* pHole
)){
  VERIFY( !SWS_BufferCopyToMemory(
    pHole,
    hBuffer,
    indx,
    sizeof( *pHole)
  ));
  }

#if 0 /* Not used */
static INLINE( ui16, HoleGetSize(
  SWS_BufferH hBuffer,
  SWS_size_t indx
)){
  ui16 val;
  VERIFY( !SWS_BufferCopyToMemory(
    &val,
    hBuffer,
    indx + offsetof( SHole, size),
    sizeof( val)
  ));
  return val;
  }

static INLINE( ui16, HoleGetNext(
  SWS_BufferH hBuffer,
  SWS_size_t indx
)){
  ui16 val;
  VERIFY( !SWS_BufferCopyToMemory(
    &val,
    hBuffer,
    indx + offsetof( SHole, next),
    sizeof( val)
  ));
  return val;
  }

static INLINE( ui16, HoleGetPrev(
  SWS_BufferH hBuffer,
  SWS_size_t indx
)){
  ui16 val;
  VERIFY( !SWS_BufferCopyToMemory(
    &val,
    hBuffer,
    indx + offsetof( SHole, prev),
    sizeof( val)
  ));
  return val;
  }
#endif


/*
 * Add a fragment to a part assembled datagram
 * Uses Clark's algorithm (RFC815)
 * Single threaded
 */
static int ReassemblyAddFragment(
  SReassembly* pReassembly,           /* IN: Reassembly buffer */
  SWS_BufferH hBuffer,                /* IN: IP fragment to add */
  unsigned hdr,                       /* IN: IP header length */
  unsigned offset,                    /* IN: Starting offset in buffer */
  int mf,                             /* IN: More fragments */
  unsigned ttl                        /* IN: TimeToLive in secs */
) {
  int iRet;
  unsigned total, indx;
  SWS_size_t uLen, uSize;
  ui32 ticks;
  SHole hole;

  TRACE( 9, ("ReassemblyAddFragment(%p,%p,%u,%u,%d,%u)\n",
    pReassembly,hBuffer,hdr,offset,mf,ttl));
  ASSERT( NULL != pReassembly);
  ASSERT( PlatformIsCritical());

  /* Calc size of data in new fragment */
  uSize = SWS_BufferSize( hBuffer);
  ASSERT( !mf || uSize >= SWS_IP_MIN_FRAG + hdr); /* RFC791: Min frag is 8 bytes */
#if SWS_SIZE_MAX > SWS_IP_MAXLEN
  ASSERT( uSize <= SWS_IP_MAXLEN);
#endif

  /* Calc buffer size required */
  total = (unsigned)(offset + (uSize - hdr));
  if ( s_uReassemblyMax && total > s_uReassemblyMax)
    {
    TRACE( 1, ("*** ReassemblyAddFragment() fragment too large\n"));
    return SWS_ENOBUFS;
    }

  /* Don't tie up all the buffers */
  if ( SWS_BufferAvailable() < total + 0x2000 )
    {
    TRACE( 1, ("*** ReassemblyAddFragment() commitment too large\n"));
    return SWS_ENOBUFS;
    }

  if ( NULL == pReassembly->hBuffer)
    {
    /* Get a buffer */
    pReassembly->hBuffer = SWS_BufferGet(
      pReassembly->hdr + total,
      (SWS_EBufferMode)(mf ? SWS_kBufferSuffix : SWS_kBufferPrefix)
    );
    if ( NULL == pReassembly->hBuffer)
      return SWS_ENOBUFS;

    /* Create initial hole */
    ASSERT( 0 == pReassembly->holes);
    hole.size = (ui16)total;
    hole.prev = 0;
    hole.next = 0;

    HoleSet( pReassembly->hBuffer, pReassembly->hdr, &hole);
    pReassembly->holes = pReassembly->hdr;
    }
  else
    {
    /* Check if total > buffer allocated */
    SWS_size_t uAllocated = SWS_BufferSize( pReassembly->hBuffer);
    if ( total + pReassembly->hdr > uAllocated)
      {
      /* Extend the buffer */
      SWS_size_t uExtra = total + pReassembly->hdr - uAllocated;
      iRet = SWS_BufferAppendBack(
        pReassembly->hBuffer,
        uExtra > sizeof( SHole) ? uExtra : sizeof( SHole)
      );
      if ( iRet)
        return iRet;

      /* Create hole */
      HoleSetSize( pReassembly->hBuffer, uAllocated, (ui16)uExtra);
      HoleSetPrev( pReassembly->hBuffer, uAllocated, 0);

      /* Append to hole list */
      HoleSetNext( pReassembly->hBuffer, uAllocated, pReassembly->holes);
      if ( pReassembly->holes)
        HoleSetPrev( pReassembly->hBuffer, pReassembly->holes, (ui16)uAllocated);

      pReassembly->holes = (ui16)uAllocated;
      }
    }

  /* If 1st fragment prepend IP header to buffer */
  if ( 0 == offset)
    {
    if ( hdr > pReassembly->hdr)
      {
      iRet = SWS_BufferAppendFront(
        pReassembly->hBuffer,
        hdr - pReassembly->hdr
      );
      if ( iRet)
        return iRet;

      pReassembly->hdr = (ui16)hdr;
      }
    else if ( hdr < pReassembly->hdr)
      {
      iRet = SWS_BufferRemoveFront(
        pReassembly->hBuffer,
        pReassembly->hdr - hdr
      );
      if ( iRet)
        return iRet;

      pReassembly->hdr = (ui16)hdr;
      }

    /* Save the header */
    VERIFY( !SWS_BufferCopy( pReassembly->hBuffer, 0, hBuffer, 0, hdr));
    pReassembly->iFragZero = 1;
    }
  offset += pReassembly->hdr;
  total += pReassembly->hdr;

  /* Walk the list of holes */
  for (
    indx = pReassembly->holes;
    indx;
    indx = hole.next
  ) {
    SHole temp;

    HoleGet( pReassembly->hBuffer, indx, &hole);

    if ( total <= indx)
      {
      /* fragment lies completely before the hole */
      continue;
      }

    if ( offset >= indx + hole.size)
      {
      /* fragment lies completely after the hole */
      continue;
      }

    if ( offset <= indx)
      {
      /* Fragment straddles start of hole */
      if ( total < indx + hole.size)
        {
        /* Move the hole */
        if ( hole.prev)
          HoleSetNext( pReassembly->hBuffer, hole.prev, (ui16)total);
        else
          pReassembly->holes = (ui16)total;

        if ( hole.next)
          HoleSetPrev( pReassembly->hBuffer, hole.next, (ui16)total);

        /* Get new hole position */
        temp.size = (ui16)(hole.size - (total - indx));
        temp.next = hole.next;
        temp.prev = hole.prev;
        HoleSet( pReassembly->hBuffer, total, &temp);
        }
      else
        {
        /* Hole is filled so remove it */
        if ( hole.prev)
          HoleSetNext( pReassembly->hBuffer, hole.prev, hole.next);
        else
          pReassembly->holes = hole.next;

        if ( hole.next)
          HoleSetPrev( pReassembly->hBuffer, hole.next, hole.prev);
        }
      }
    /* Fragment starts within the hole */
    else if ( total >= indx + hole.size)
      {
      /* Fragment extends beyond end of hole so reduce size of hole */
      HoleSetSize( pReassembly->hBuffer, indx, (ui16)(offset - indx) );
      }
    else
      {
      /* Fragment splits this hole into 2 */
      temp.size = (ui16)(hole.size - (total - indx));
      temp.prev = (ui16)indx;
      temp.next = hole.next;
      HoleSet( pReassembly->hBuffer, total, &temp);

      if ( hole.next)
        HoleSetPrev( pReassembly->hBuffer, hole.next, (ui16)total );

      hole.next = (ui16)total;
      hole.size = (ui16)(offset - indx);
      HoleSet( pReassembly->hBuffer, indx, &hole);
      }
    } /* Next hole */

  if ( !mf)
    {
    /* Last fragment so set length of datagram */
    uLen = SWS_BufferSize( pReassembly->hBuffer);
    ASSERT( uLen >= total);
    pReassembly->length = (ui16)total;
    if ( uLen > total)
      SWS_BufferRemoveBack( pReassembly->hBuffer, uLen - total);
    }

  /* Copy fragment to reassembly buffer */
  VERIFY( !SWS_BufferCopy(
    pReassembly->hBuffer,
    offset,
    hBuffer,
    hdr,
    uSize - hdr
  ));

  /* ttl = max(ttl, timerem) */
  ticks = (SWS_TimerRemaining( pReassembly->hTimer)
    + SWS_TIMER_TICKS/2) / SWS_TIMER_TICKS;
  if ( (ui32)ttl > ticks)
    SWS_TimerStart( pReassembly->hTimer, (ui32)ttl * SWS_TIMER_TICKS);

  return 0;
  }


/*
 * Test if reassembly is complete
 */
static SWS_FASTCALL( int, ReassemblyIsComplete( SReassembly* pReassembly))
  {
  TRACE( 9, ("ReassemblyAddFragment(%p)\n", pReassembly));
  ASSERT( NULL != pReassembly);

  return pReassembly->length && !pReassembly->holes;
  }


/*
 * Handle reassembly timeout
 * Single threaded
 */
static SWS_CB( void, ReassemblyTimeout(
  SWS_TimerH hTimer,                  /* IN: Timer that expired */
  SWS_PTR( void) pvContext            /* IN: User context */
)){
  SReassembly* const pReassembly = (SReassembly*)pvContext;

  TRACE( 9, ("ReassemblyTimeout(%p,%p)\n", hTimer,pvContext));
  ASSERT( PlatformIsCritical());
  (void)hTimer;

#if OPT_PROTO_ICMP
  if ( pReassembly->iFragZero
    && !SWS_IN_MULTICAST( pReassembly->dst)
  ) {
    /*  Send ICMP time exceeded code 1 (reassembly) */
    SWS_IcmpSend(
      SWS_INADDR_ANY, pReassembly->src,
      SWS_ICMP_TIME_EXCEED, SWS_ICMP_FRAG_EXCEED, 0,
      pReassembly->hBuffer,
      SWS_IP_IDENT_DEFAULT,
      NULL
    );
    }
#endif

  /* Dispose of the reassembly buffer */
  ReassemblyPut( pReassembly);
  }

#endif /* OPT_IP_REASSEMBLY_BUFFERS */


/****************************************************************************
 * Datalink interface
 ****************************************************************************/

/*
 * Register a data link interface
 */
SWS_EXPORT( int, SWS_DatalinkRegister( /* Returns !0 on error */
  SWS_PTR( const SWS_DatalinkDesc) pDesc,  /* IN: Data link descriptor */
  SWS_PTR( void) pvContext,           /* IN: Client context */
  SWS_PTR( SWS_DatalinkCallbacks) pUp /* OUT: Up calls */
)){
  SInterface* pInterface;
  size_t size;

  TRACE( 9, ("SWS_DatalinkRegister(%p,%p,%p)\n", pDesc, pvContext, pUp));
  ASSERT( NULL != pDesc);
  ASSERT( pDesc->pfnSend);
  ASSERT( pDesc->pfnGetProperty);
  ASSERT( NULL != pUp);

  if ( sizeof(*pDesc) > pDesc->uSize)
    {
    TRACE( 0, ("!!!SWS_DatalinkRegister descriptor size too small\n"));
    return SWS_EINVAL;
    }

  if ( SWS_DATALINK_VERSION != pDesc->uVersion)
    {
    TRACE( 0, ("!!!SWS_DatalinkRegister version incompatible\n"));
    return SWS_EINVAL;
    }

  if ( s_uInterfaces >= lengthof( s_interfaceArray) )
    {
    TRACE( 0, ("!!!SWS_DatalinkRegister too many data links\n"));
    return SWS_ETOOMANYREFS;
    }

  /* Add to interface list */
  pInterface = &s_interfaceArray[ s_uInterfaces++];
  memset( pInterface, 0, sizeof( *pInterface) );

  /* Allocate resources */
  pInterface->hTxMsgQ = SWS_MsgqCreate( s_uTxQLen, sizeof( SFragInfo));
  if ( NULL == pInterface->hTxMsgQ)
    {
    --s_uInterfaces;
    TRACE( 0, ("!!!SWS_DatalinkRegister failed to create MsgQ len %u\n", s_uTxQLen));
    return SWS_ENOBUFS;
    }
  pInterface->iTxBusy = 0;

  /* Save registration info */
  pInterface->pfnSend = pDesc->pfnSend;
  pInterface->pfnGetProperty = pDesc->pfnGetProperty;
  pInterface->pfnSetProperty = pDesc->pfnSetProperty;
  pInterface->pfnShutdown = pDesc->pfnShutdown;
  pInterface->pfnSetMulticast = pDesc->pfnSetMulticast;
  pInterface->pvContext = pvContext;
  pInterface->uVersion = pDesc->uVersion;

  pInterface->uMtu = 0;
  pInterface->uIpAddr = SWS_INADDR_ANY;
  pInterface->uSubnetMask = 0;
  pInterface->uGatewayAddr = SWS_INADDR_NONE;
  pInterface->isLinkUp = -1;

#if OPT_IP_MULTICAST
  pInterface->groups = 0;
#endif
#if OPT_PROTO_IGMP
  pInterface->iIgmpVersion = s_iIgmpVersion;
#endif

  pInterface->uMagic = INTERFACE_MAGIC;

  /* Fetch the interface's ident string */
  size = sizeof( pInterface->szIdent);
  if ( (*pInterface->pfnGetProperty)(
    pInterface->pvContext,
    SWS_kDatalinkProperty_Ident,
    pInterface->szIdent,
    &size
  )){
    sprintf( pInterface->szIdent, "Adapter%u", s_uInterfaces);
    }

  /* Get the adapter's IP address, subnet mask & gateway */
  (void)InterfaceAddress( pInterface);
  (void)InterfaceSubnet( pInterface);
  (void)InterfaceGateway( pInterface);

  TRACE( 2, ("SWS_DatalinkRegister %s: %u.%u.%u.%u\n",
    pInterface->szIdent,
    (unsigned)((pInterface->uIpAddr >> 24) & 0xff),
    (unsigned)((pInterface->uIpAddr >> 16) & 0xff),
    (unsigned)((pInterface->uIpAddr >> 8) & 0xff),
    (unsigned)(pInterface->uIpAddr & 0xff)
  ));

  /* Return 'up' calls */
  pUp->pvContext = pInterface;
  pUp->pfnSendRdy = &DatalinkSendRdy;
  pUp->pfnRecvInd = &DatalinkRecvInd;
  pUp->pfnLinkStatus = &DatalinkStatus;

#if OPT_IP_ROUTES
  /* Add standard routes */
  RouteAddInterface( pInterface);
#endif

  return 0;
  }


/*
 * Count no. data links
 */
SWS_EXPORT( unsigned, SWS_DatalinkCount( void)) /* Returns no. links */
  {
  TRACE( 9, ("SWS_DatalinkCount()\n"));
  return s_uInterfaces;
  }


/*
 * Find a data link
 */
SWS_EXPORT( int, SWS_DatalinkFind(    /* Returns interface ID, -ve if not found */
  SWS_PTR( const char) psz            /* IN: Ident */
)){
  unsigned uInterface;
  SInterface* pInterface;

  TRACE( 9, ("SWS_DatalinkFind(%s)\n",psz));
  ASSERT( NULL != psz);

  for (
    pInterface = &s_interfaceArray[ uInterface = 0];
    uInterface < s_uInterfaces;
    ++uInterface, ++pInterface
  ) {
    char szBuf[256];
    size_t uLen;

    ASSERT( INTERFACE_MAGIC == pInterface->uMagic);
    ASSERT( pInterface->pfnGetProperty);

    uLen = sizeof( szBuf);
    if ( !(*pInterface->pfnGetProperty)(
      pInterface->pvContext,
      SWS_kDatalinkProperty_Ident,
      szBuf,
      &uLen
    )){
      if ( !strcmp( psz, szBuf))
        return (int)uInterface;
      }
    }

  return -1;
  }


/*
 * Get a data link property
 */
SWS_EXPORT( int, SWS_DatalinkGetProperty( /* Returns !0 on error */
  unsigned id,                        /* IN: ID, 0..count-1 */
  int type,                           /* IN: Property, SWS_kDatalinkProperty_xxx */
  SWS_PTR( void) pVal,                /* OUT: Value */
  SWS_PTR( size_t) pLen               /* I/O: Buffer size in, data size out */
)){
  SInterface* pInterface;

  TRACE( 9, ("SWS_DatalinkGetProperty(%u,%d,%p,%p)\n",id,type,pVal,pLen));
  ASSERT( NULL != pVal);
  ASSERT( NULL != pLen);

  if ( id >= s_uInterfaces)
    return SWS_EINVAL;

  pInterface = &s_interfaceArray[ id];
  ASSERT( INTERFACE_MAGIC == pInterface->uMagic);

  ASSERT( pInterface->pfnGetProperty);
  return (*pInterface->pfnGetProperty)(
    pInterface->pvContext,
    type,
    pVal,
    pLen
  );
  }


/*
 * Set a data link property
 */
SWS_EXPORT( int, SWS_DatalinkSetProperty( /* Returns !0 on error */
  unsigned id,                        /* IN: ID, 0..count-1 */
  int type,                           /* IN: Property, SWS_kDatalinkProperty_xxx */
  SWS_PTR( const void) pVal,          /* OUT: Value */
  size_t len                          /* IN: Size of value */
)){
  SInterface* pInterface;
  int iRet;

  TRACE( 9, ("SWS_DatalinkSetProperty(%u,%d,%p,%u)\n",id,type,pVal,len));
  ASSERT( NULL != pVal);

  if ( id >= s_uInterfaces)
    return SWS_EINVAL;

  pInterface = &s_interfaceArray[ id];
  ASSERT( INTERFACE_MAGIC == pInterface->uMagic);

  if ( !pInterface->pfnSetProperty)
    return SWS_EACCES;

  iRet = (*pInterface->pfnSetProperty)(
    pInterface->pvContext,
    type,
    pVal,
    len
  );
  /* Flush cached values */
  if ( !iRet) switch ( type)
    {
  case SWS_kDatalinkProperty_Address:
    pInterface->uIpAddr = SWS_INADDR_ANY;
    break;
  case SWS_kDatalinkProperty_SubnetMask:
    pInterface->uSubnetMask = 0;
    break;
  case SWS_kDatalinkProperty_Gateway:
    pInterface->uGatewayAddr = SWS_INADDR_NONE;
    break;
  case SWS_kDatalinkProperty_MTU:
    pInterface->uMtu = 0;
    break;
    }

  return iRet;
  }


/*
 * Get a data link state
 */
SWS_EXPORT( int, SWS_DatalinkIsUp(    /* Returns !0 if up */
  unsigned id                         /* IN: ID, 0..count-1 */
)){
  SInterface* pInterface;
  TRACE( 9, ("SWS_DatalinkIsUp(%u)\n",id));

  if ( id >= s_uInterfaces)
    return 0;

  pInterface = &s_interfaceArray[ id];
  ASSERT( INTERFACE_MAGIC == pInterface->uMagic);
  return pInterface->isLinkUp;
  }


/*
 * Datalink status change
 * This is an asynchronous up call from the data link
 */
typedef struct SLinkInfo
  {
  SInterface* pInterface;
  int up;
  } SLinkInfo;

static SWS_CB( void, DatalinkStatus(
  SWS_PTR( void) pvContext,           /* IN: Client context */
  int up
)){
  SLinkInfo info;

  ASSERT( NULL != pvContext);
  ASSERT( INTERFACE_MAGIC == ((SWS_PTR(SInterface))pvContext)->uMagic);

  info.pInterface = (SInterface*)pvContext;
  info.up = 0 != up;

  PlatformSynchronise( &IpLinkStatus, &info);
  }

static SWS_CB( int, IpLinkStatus(
  SWS_PTR( void) pvContext
)){
  SLinkInfo* const pInfo = (SLinkInfo*)pvContext;
  SInterface* const pInterface = pInfo->pInterface;

  TRACE( 9, ("IpLinkStatus(%p)\n",pvContext));

  if ( pInterface->isLinkUp != pInfo->up)
    {
    pInterface->isLinkUp = pInfo->up;

#if OPT_IP_ROUTES
    RouteRemoveInterface( pInterface);
#endif

    /* Invalidate cached datalink info */
    pInterface->uIpAddr = SWS_INADDR_ANY;
    pInterface->uSubnetMask = 0;
    pInterface->uGatewayAddr = SWS_INADDR_NONE;
    pInterface->uMtu = 0;

    if ( pInterface->isLinkUp)
      {
      (void)InterfaceAddress( pInterface);
      (void)InterfaceSubnet( pInterface);
      (void)InterfaceGateway( pInterface);
      (void)InterfaceMtu( pInterface);

      TRACE( 2, ("IpLinkStatus %s up: %u.%u.%u.%u\n",
        pInterface->szIdent,
        (unsigned)((pInterface->uIpAddr >> 24) & 0xff),
        (unsigned)((pInterface->uIpAddr >> 16) & 0xff),
        (unsigned)((pInterface->uIpAddr >> 8) & 0xff),
        (unsigned)(pInterface->uIpAddr & 0xff)
      ));
      }
    else
      {
      TRACE( 2, ("IpLinkStatus() %s down\n",pInterface->szIdent));
      }

#if OPT_IP_ROUTES
    if ( pInterface->isLinkUp)
      RouteAddInterface( pInterface);
#endif
    }

  return 0;
  }


/*
 * 'Send buffer' completed
 * This is an asynchronous up call from the data link
 */
static SWS_CB( void, DatalinkSendRdy(
  SWS_PTR( void) pvContext            /* IN: Client context */
)){
  ASSERT( NULL != pvContext);
  ASSERT( INTERFACE_MAGIC == ((SWS_PTR(SInterface))pvContext)->uMagic);

  if ( PlatformDpc( &IpSendRdy, pvContext, NULL, 0))
    {
    /* Out of DPC's so do callback immediately.
     * NB This may be unsafe for SWS_MsgqGet */
    TRACE( 1, ("*** DatalinkSendRdy(%p) Out of DPCs\n", pvContext));
    IpSendRdy( pvContext, NULL, 0);
    }
  }


/*
 * Receive an IP packet
 * This is an asynchronous up call from the data link
 */
static SWS_CB( void, DatalinkRecvInd(
  SWS_PTR( void) pvContext,           /* IN: Client context */
  SWS_BufferH hBuffer,                /* IN: Packet received */
  int broadcast
)){
  SRecvInfo info;

  ASSERT( NULL != pvContext);
  ASSERT( INTERFACE_MAGIC == ((SWS_PTR(SInterface))pvContext)->uMagic);
  ASSERT( NULL != hBuffer);

  info.pInterface = (SInterface*)pvContext;
  info.hBuffer = hBuffer;
  info.broadcast = broadcast;

  PlatformSynchronise( &IpRecvInd, &info);
  }


/*
 * Search for an interface with a given IP address
 */
static SWS_FASTCALL( SInterface*, IpFindInterface( ui32 const src))
  {
  unsigned uInterface;
  SInterface* pInterface;

  TRACE( 9, ("IpFindInterface(%08lx)\n", src));

  for (
    pInterface = &s_interfaceArray[ uInterface = 0];
    uInterface < s_uInterfaces;
    ++uInterface, ++pInterface
  ) {
    ASSERT( INTERFACE_MAGIC == pInterface->uMagic);
    if ( src == InterfaceAddress( pInterface))
      return pInterface;
    }

  return NULL;
  }


/*
 * Get interface's IP address
 */
static SWS_FASTCALL( ui32, InterfaceAddress( SInterface* const pInterface))
  {
  TRACE( 9, ("InterfaceAddress(%p)\n", pInterface));
  ASSERT( NULL != pInterface);
  ASSERT( INTERFACE_MAGIC == pInterface->uMagic);

  if ( !pInterface->isLinkUp)
    return SWS_INADDR_ANY;

  if ( SWS_INADDR_ANY == pInterface->uIpAddr)
    {
    size_t uLen = sizeof( pInterface->uIpAddr);
    ASSERT( pInterface->pfnGetProperty);
    VERIFY( !(*pInterface->pfnGetProperty)(
      pInterface->pvContext,
      SWS_kDatalinkProperty_Address,
      &pInterface->uIpAddr,
      &uLen
    ));
    ASSERT( uLen == sizeof(pInterface->uIpAddr));
    if ( SWS_INADDR_NONE == pInterface->uIpAddr)
      pInterface->uIpAddr = SWS_INADDR_ANY;

    ASSERT( pInterface->uIpAddr < kMulticastGroup); /* Not class D, E or broadcast */
    }
  return pInterface->uIpAddr;
  }


#if !OPT_DHCP
/*
 * Set interface's IP address
 */
static SWS_FASTCALL( int, InterfaceSetAddress(
  SInterface* const pInterface,
  ui32 addr
)){
  TRACE( 9, ("InterfaceSetAddress(%p,%08lx)\n", pInterface, addr));
  ASSERT( NULL != pInterface);
  ASSERT( INTERFACE_MAGIC == pInterface->uMagic);

#if OPT_IP_ROUTES
  RouteRemoveInterface( pInterface);
#endif

  pInterface->uIpAddr = addr;

  if ( 0 == pInterface->uSubnetMask
    && SWS_INADDR_ANY != addr
    && SWS_INADDR_NONE != addr
  ) {
    if ( SWS_IN_CLASSA( addr))
      pInterface->uSubnetMask = SWS_IN_CLASSA_NET;
    else if ( SWS_IN_CLASSB( addr))
      pInterface->uSubnetMask = SWS_IN_CLASSB_NET;
    else if ( SWS_IN_CLASSC( addr))
      pInterface->uSubnetMask = SWS_IN_CLASSC_NET;
    }

#if OPT_IP_ROUTES
  RouteAddInterface( pInterface);
#endif

  if ( !pInterface->pfnSetProperty)
    return SWS_EACCES; /* Permission denied */

  if ( pInterface->uSubnetMask)
    (void)(*pInterface->pfnSetProperty)(
      pInterface->pvContext,
      SWS_kDatalinkProperty_SubnetMask,
      &pInterface->uSubnetMask,
      sizeof pInterface->uSubnetMask
    );
  return (*pInterface->pfnSetProperty)(
    pInterface->pvContext,
    SWS_kDatalinkProperty_Address,
    &addr,
    sizeof( addr)
  );
  }
#endif


/*
 * Get interface's subnet mask
 */
static SWS_FASTCALL( ui32, InterfaceSubnet( SInterface* const pInterface))
  {
  TRACE( 9, ("InterfaceSubnet(%p)\n", pInterface));
  ASSERT( NULL != pInterface);
  ASSERT( INTERFACE_MAGIC == pInterface->uMagic);

  if ( !pInterface->isLinkUp)
    return 0;

  if ( 0 == pInterface->uSubnetMask)
    {
    size_t uLen = sizeof( pInterface->uSubnetMask);
    ASSERT( pInterface->pfnGetProperty);
    VERIFY( !(*pInterface->pfnGetProperty)(
      pInterface->pvContext,
      SWS_kDatalinkProperty_SubnetMask,
      &pInterface->uSubnetMask,
      &uLen
    ));
    ASSERT( uLen == sizeof(pInterface->uSubnetMask));
    if ( 0 == pInterface->uSubnetMask)
      {
      /* Use default subnet for class */
      ui32 uIpAddr = InterfaceAddress( pInterface);
      if ( SWS_INADDR_ANY != uIpAddr
        && SWS_INADDR_NONE != uIpAddr
      ) {
        if ( SWS_IN_CLASSA( uIpAddr))
          pInterface->uSubnetMask = SWS_IN_CLASSA_NET;
        else if ( SWS_IN_CLASSB( uIpAddr))
          pInterface->uSubnetMask = SWS_IN_CLASSB_NET;
        else if ( SWS_IN_CLASSC( uIpAddr))
          pInterface->uSubnetMask = SWS_IN_CLASSC_NET;

        if ( pInterface->uSubnetMask && pInterface->pfnSetProperty)
          (void)(*pInterface->pfnSetProperty)(
            pInterface->pvContext,
            SWS_kDatalinkProperty_SubnetMask,
            &pInterface->uSubnetMask,
            sizeof pInterface->uSubnetMask
          );
        }
      }
    }
  return pInterface->uSubnetMask;
  }


/*
 * Get interface's default gateway
 */
static SWS_FASTCALL( ui32, InterfaceGateway( SInterface* const pInterface))
  {
  ASSERT( NULL != pInterface);
  ASSERT( INTERFACE_MAGIC == pInterface->uMagic);

  if ( !pInterface->isLinkUp)
    return SWS_INADDR_NONE;

  if ( SWS_INADDR_NONE == pInterface->uGatewayAddr)
    {
    size_t uLen = sizeof( pInterface->uGatewayAddr);
    ASSERT( pInterface->pfnGetProperty);
    VERIFY( !(*pInterface->pfnGetProperty)(
      pInterface->pvContext,
      SWS_kDatalinkProperty_Gateway,
      &pInterface->uGatewayAddr,
      &uLen
    ));
    ASSERT( uLen == sizeof(pInterface->uGatewayAddr));
    }
  return pInterface->uGatewayAddr;
  }


/*
 * Get interface's MTU
 */
INLINE( size_t, InterfaceMtu( SInterface* const pInterface))
  {
  ASSERT( NULL != pInterface);
  ASSERT( INTERFACE_MAGIC == pInterface->uMagic);

  if ( 0 == pInterface->uMtu)
    {
    size_t uLen = sizeof( pInterface->uMtu);
    ASSERT( pInterface->pfnGetProperty);
    VERIFY( !(*pInterface->pfnGetProperty)(
      pInterface->pvContext,
      SWS_kDatalinkProperty_MTU,
      &pInterface->uMtu,
      &uLen
    ));
    ASSERT( uLen == sizeof(pInterface->uMtu));
    ASSERT( pInterface->uMtu >= SWS_IP_MIN_MTU);
    }
  return pInterface->uMtu;
  }


/****************************************************************************
 * IGMP support
 ****************************************************************************/

#if OPT_PROTO_IGMP
/*
 * Handle IGMP query
 */
void SWS_IpIgmpQuery(
  ui32 src,
  ui32 group,
  unsigned tenths
) {
#if OPT_IP_MULTICAST
  SInterface* pInterface;
#endif

  TRACE( 9, ("SWS_IpIgmpQuery(%08lx,%08lx,%u)\n", src,group,tenths));

#if OPT_IP_MULTICAST
  if ( 0 == s_iIgmpVersion)
    return;

  if ( kGroupAllHosts == group)
    return;

  pInterface = IpFindInterface( src);
  if ( NULL != pInterface)
    {
    int i;
    SGroup* pGroup;

    if ( 0 == tenths)
      {
      /* IGMPv1. Assume 10 seconds */
      tenths = 100;
      pInterface->iIgmpVersion = 1;
      }

    if ( 0 == group)
      {
      /* General query */
      for ( i = 0; i < (int)pInterface->groups; ++i)
        {
        if ( pInterface->multicastList[ i] != kGroupAllHosts)
          {
          pGroup = &pInterface->group[ i];
          if ( NULL != pGroup->hTimer)
            IgmpStartQueryTimer( pGroup, tenths);
          }
        }
      }
    else
      {
      i = IpGroupFind( pInterface, group);
      if ( i >= 0)
        {
        pGroup = &pInterface->group[ i];
        if ( NULL != pGroup->hTimer)
          IgmpStartQueryTimer( pGroup, tenths);
        }
      }
    }

#else
  (void)src;
  (void)group;
  (void)tenths;
#endif
  }


#if OPT_IP_MULTICAST
/*
 * Start query delay
 */
SWS_FASTCALL( void, IgmpStartQueryTimer(
  SGroup* pGroup,
  unsigned tenths
)){
  ui32 ticks, tCurrent;

  /* random 0..t/10 seconds */
  ticks = (ui32)(1 + (rand() % tenths)) * (SWS_TIMER_TICKS / 10);
  tCurrent = SWS_TimerRemaining( pGroup->hTimer);

  if ( 0 == tCurrent || ticks < tCurrent )
    SWS_TimerStart( pGroup->hTimer, ticks );
  }
#endif


/*
 * Handle IGMP report
 */
void SWS_IpIgmpReport(
  ui32 src,
  ui32 group
) {
#if OPT_IP_MULTICAST
  SInterface* pInterface;
#endif

  TRACE( 9, ("SWS_IpIgmpReport(%08lx,%08lx)\n", src,group));

#if OPT_IP_MULTICAST
  if ( 0 == s_iIgmpVersion)
    return;

  if ( kGroupAllHosts == group)
    return;

  pInterface = IpFindInterface( src);
  if ( NULL != pInterface)
    {
    int i;

    i = IpGroupFind( pInterface, group);
    if ( i >= 0)
      {
      SGroup* const pGroup = &pInterface->group[ i];
      if ( NULL != pGroup->hTimer)
        {
        /* Cancel report timer */
        SWS_TimerStop( pGroup->hTimer);
        pGroup->iReportSent = 0;
        }
      }
    }

#else
  (void)src;
  (void)group;
#endif
  }


#if OPT_IP_MULTICAST
/*
 * Report timer callback
 */
SWS_CB( void, IgmpTimeout( SWS_TimerH hTimer, SWS_PTR( void) pv ))
  {
  SInterface* const pInterface = (SInterface*)pv;
  unsigned i;

  TRACE( 9, ("IgmpTimeout(%p,%p)\n", hTimer,pv));

  /* Find group by matching timer handle */
  for ( i = 0; i < pInterface->groups; ++i)
    {
    SGroup* const pGroup = &pInterface->group[ i];

    if ( pGroup->hTimer == hTimer)
      {
      ui32 group = pInterface->multicastList[ i];

      /* Send report */
      pGroup->iReportSent = 1;

      SWS_IgmpSend(
        InterfaceAddress( pInterface),
        group,
        pInterface->iIgmpVersion >= 2 ? SWS_IGMP_REPORT2 :SWS_IGMP_REPORT,
        0,
        group
      );
      break;
      }
    }

  }
#endif
#endif /* OPT_PROTO_IGMP */

/* End of file */
