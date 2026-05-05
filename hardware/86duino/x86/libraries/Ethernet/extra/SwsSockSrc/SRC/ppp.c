/****************************************************************************
 *
 * File name   : ppp.c
 * Function    : PPP data link
 * Project     : SwsSock
 * Authors     : Lawrence Rust
 * Systems     : ANSI C.
 *
 ****************************************************************************
 *
 * Created by Lawrence Rust, Software Systems Consultants
 * lvr@softsystem.co.uk. Tel/Fax +33 5 49 72 79 63
 *
 * 14-May-09, LVR, SendEchoReply sends cookie
 * 03-Aug-05, LVR, Created.
 *
 * Comments:
 * --------
 * This file implements PPP (RFC 1661) with HDLC framing (RFC 1662)
 *
 ****************************************************************************
 */

/* Compilation options */
#include "config.h"

#ifndef OPT_PPP_AUTH
#define OPT_PPP_AUTH 1    /* !0 to enable PAP (RFC 1334) & CHAP (RFC 1994) */
#endif
#ifndef OPT_PPP_MSCHAP
#define OPT_PPP_MSCHAP 0  /* !0 for CHAP (RFC 2433) authentication */
#endif
#ifndef OPT_PPP_MSCHAP2   /* TODO implement */
#define OPT_PPP_MSCHAP2 0 /* !0 for CHAP (RFC 2759) authentication */
#endif

#ifndef OPT_PPP_IDENT
#define OPT_PPP_IDENT 0   /* !0 to send LCP ident RFC1570 packet */
#endif

#ifndef OPT_PPP_CCP       /* TODO implement */
#define OPT_PPP_CCP 0     /* !0 to negotiate RFC 1962 compression */
#endif


/* Exports */
#include "sws_ppp.h"
#include "ppp.h"

#if OPT_PPP && OPT_DATALINK_P2P
/* Imports */
#include <stddef.h>
#include <string.h>

#include "sws_sock.h"  /* for SWS_INADDR_ANY and error codes */
#include "sws_p2p.h"
#include "sws_buf.h"
#include "sws_timr.h"
#include "sws_ip.h" /* SWS_RouteAdd */
#include "sws_gef.h"
#if OPT_PPP_AUTH
#include "sws_md5.h"
#endif
#include "platform.h"

#include "debug.h"


/*
 * Macros & Constants
 */

/* HDLC-like Framing -  RFC 1662 */
#define kHdlcFlag     ((ui8)0x7e)  /* HDLC flag, start and end of packet */
#define kHdlcEsc      ((ui8)0x7d)  /* HDLC escape */
#define kHdlcAddress  ((ui8)0xff)  /* HDLC address, 0xff= all stations */
#define kHdlcControl  ((ui8)0x03)  /* HDLC control, 3= unnumberd information */

#define HDLC_ESC( c) ((ui8)((c) ^ 0x20))

#define PPPINITFCS16    0xffff  /* Initial FCS value */
#define PPPGOODFCS16    0xf0b8  /* Good final FCS value */

enum
  {
  kHdlcOverhead = 8U,               /* addr:1, ctrl:1, pcol:2, fcs:4 */
  kPppMaxFragment = 1500U,          /* largest PPP packet to transmit */

  /* In the worst case each byte will be escaped
   * together with an HDLC start & end flag */
  kPppMaxPacket = 2U * (kPppMaxFragment + kHdlcOverhead) + 2U
  };
COMPILATION_REQUIRES( kPppMaxFragment + kHdlcOverhead <= OPT_BUFFER_LARGE_SIZE);


/*
 * Local data types
 */

/* LCP/NCP automaton state */
typedef enum EState
  {
  kStateInitial = 0,
  kStateStarting = 1,
  kStateClosed = 2,
  kStateStopped = 3,
  kStateClosing = 4,
  kStateStopping = 5,
  kStateReqSent = 6,
  kStateAckRcvd = 7,
  kStateAckSent = 8,
  kStateOpened = 9
  } EState;

/* Automaton events */
typedef enum EEvent
  {
  kEventUp,     /* lower layer is Up */
  kEventDown,   /* lower layer is Down */
  kEventOpen,   /* administrative Open */
  kEventClose,  /* administrative Close */
  kEventTO_P,   /* Timeout with counter > 0 */
  kEventTO_M,   /* Timeout with counter expired */
  kEventRCR_P,  /* Receive-Configure-Request (Good) */
  kEventRCR_M,  /* Receive-Configure-Request (Bad) */
  kEventRCA,    /* Receive-Configure-Ack */
  kEventRCN,    /* Receive-Configure-Nak/Rej */
  kEventRTR,    /* Receive-Terminate-Request */
  kEventRTA,    /* Receive-Terminate-Ack */
  kEventRUC,    /* Receive-Unknown-Code */
  kEventRXJ_P,  /* Receive-Code-Reject (permitted) */
                /* Receive-Protocol-Reject */
  kEventRXJ_M,  /* Receive-Code-Reject (catastrophic) */
                /* Receive-Protocol-Reject */
  kEventRXR     /* Receive-Echo-Request */
                /* Receive-Echo-Reply */
                /* Receive-Discard-Request */
  } EEvent;

struct SAutomaton;
typedef SWS_FASTCALLDEF( void, FAutomaton( struct SAutomaton*));
typedef SWS_FASTCALLDEF( void, FAutomatonCfg( struct SAutomaton*, SWS_BufferH ));
typedef void FAutomatonRsp( struct SAutomaton*, unsigned, unsigned, unsigned, SWS_PTR( const void) );
typedef int FAutomatonReq( struct SAutomaton*, unsigned, unsigned, SWS_PTR( void) );

/* Option negotiation FSS automaton */
typedef struct SAutomaton
  {
  /* 'virtual' fn's */
  FAutomaton* pfnReset;               /* Reset */
  FAutomaton* pfnUp;                  /* This layer up */
  FAutomaton* pfnDown;                /* This layer down */
  FAutomaton* pfnStarted;             /* This layer started */
  FAutomaton* pfnFinished;            /* This layer finished */
  FAutomatonCfg* pfnConfigAdd;        /* Add CONFIGURE_REQ options */
  FAutomatonRsp* pfnConfigRsp;        /* Parse CONFIGURE_ACK/Nak/Rej options */
  FAutomatonReq* pfnConfigReq;        /* Parse CONFIGURE_REQ options */

  struct SPppAdapter* pAdapter;
  EState eState;
  SWS_TimerH hRestartTimer;
  int iRestartCnt;
  int iCfgFailures;
  ui16 protocol;
  ui8 uCfgId, uTermId, uRejId;
  } SAutomaton;


/* Adapter instance data */
typedef struct SPppAdapter
  {
  ui32 uMagic;
#define ADAPTER_MAGIC SWS_MAKEFOURCC( 'p','p','p','1')

  unsigned uP2pAdapter;               /* ID of assoc P2P adapter */
  SWS_P2pProtocolDn dn;               /* Adapter down call table */
  SWS_DatalinkCallbacks up;           /* Datalink up call table */
  int isUp;                           /* Adapter is up */

  /* Protocol automata */
  SAutomaton automLcp;
#if OPT_PPP_AUTH
  SAutomaton automAuth;
#endif
  SAutomaton automIpcp;

  /* LCP options */
  int isACFC;                         /* Send compressed address & control field */
  int isPFC;                          /* Send compressed protocol field */
  ui32 uTxACCM, uRxACCM;
  ui32 uTxMagic, uRxMagic;
  ui16 uMtu, uMru;

#if OPT_PPP_AUTH
  /* Auth options */
  ui16 uRxAuth;                       /* Auth protocol required of peer */
  ui16 uRxAuthState, uTxAuthState;
#endif

  /* IPCP config */
  ui32 uIpAddress;
  ui32 uGatewayAddr;                  /* Adapter's default gateway */
#if OPT_DNS
  ui32 uPriDns, uSecDns;
#endif

#if OPT_P2P_VJCOMP
  SWS_SP2pVjCompH hTxVjComp;          /* VJ compressor instance */
  SWS_SP2pVjCompH hRxVjComp;          /* VJ decompressor instance */
  ui8 uRxMaxSlotId, uRxCompSlotId;    /* VJ decompressor settings */
#endif

#if OPT_PPP_AUTH
  /* CHAP options */
  ui8 uChapAlgo;                      /* 5= MD5, 128 = MS-CHAP, 129 = MS-CHAP2 */
  SWS_MD5Digest challenge;
#endif

  /* Tx buffer */
  unsigned txSize;
  void* pTxBuffer;
  volatile unsigned txWrite, txRead;  /* Write/read cursors */
  volatile unsigned uTxPending;       /* No. bytes pending in output device */
  volatile int iTxBusy;               /* PppSendIp is pending */

  /* Get free space in Tx buffer */
#define PppTxFree( pAdapter) \
  (unsigned)((pAdapter->txWrite >= pAdapter->txRead) \
  ? ((pAdapter->txSize - 1) - pAdapter->txWrite + pAdapter->txRead) \
  : ((pAdapter->txRead - pAdapter->txWrite) - 1))

  /* Get contiguous used space in Tx buffer */
#define PppTxPending( pAdapter) \
  (unsigned)((pAdapter->txWrite < pAdapter->txRead) \
  ? (pAdapter->txSize - pAdapter->txRead) \
  : (pAdapter->txWrite - pAdapter->txRead))

  /* Rx buffer */
  unsigned rxSize;
  void* pRxBuffer;
  volatile unsigned rxWrite, rxRead;  /* Write/read cursors */
  volatile int iRxPending;
  volatile int iRxBusy;               /* PppRecvIndDpc pending */
  int iRxEnable;

  /* Get contiguous free space in Rx buffer */
#define PppRxFree( pAdapter) \
  (unsigned)((pAdapter->rxWrite < pAdapter->rxRead) \
  ? ((pAdapter->rxRead - pAdapter->rxWrite) - 1) \
  : (pAdapter->rxRead > 0) \
  ? (pAdapter->rxSize - pAdapter->rxWrite) \
  : ((pAdapter->rxSize - 1) - pAdapter->rxWrite))

  /* Get no. pending Rx bytes */
#define PppRxPending( pAdapter) \
  (unsigned)((pAdapter->rxWrite < pAdapter->rxRead) \
  ? ((pAdapter->rxSize - pAdapter->rxRead) + pAdapter->rxWrite) \
  : (pAdapter->rxWrite - pAdapter->rxRead))

  /* The current input frame */
  SWS_BufferH hBuffer;
  SWS_size_t sizeBuf;
  SWS_PTR( ui8) pucBuf;
  unsigned uReceived;
  int iEsc;
  } SPppAdapter;


/*
 * Prototypes
 */
#define IS_PPP_ADAPTER( a) (NULL != (a) && ADAPTER_MAGIC == (a)->uMagic)
static SWS_FASTCALLDEF( void, PppDestroyInstance( SPppAdapter*));

/* Tx/Rx */
static int PppSendRaw( SPppAdapter* const, SWS_BufferH, ui16);
static SWS_FASTCALLDEF( int, PppTransmit( SPppAdapter* const, unsigned));
static SWS_FASTCALLDEF( int, PppReceive( SPppAdapter* const));

static void PppRxUnknown( SPppAdapter*, SWS_BufferH, ui16);
#if OPT_PPP_AUTH
static SWS_FASTCALLDEF( void, PppRxChap( SPppAdapter*, SWS_BufferH));
static SWS_FASTCALLDEF( void, PppRxPap( SPppAdapter*, SWS_BufferH));
#endif
#if OPT_PPP_CCP
static SWS_FASTCALLDEF( void, PppRxCcp( SPppAdapter*, SWS_BufferH));
static SWS_FASTCALLDEF( void, CcpCfg( SPppAdapter*));
#endif

static SWS_FASTCALLDEF( ui16, PppFcs16Buffer( SWS_BufferH, unsigned));

/* Automata */
static int AutomatonCtor( SAutomaton*, ui16, SPppAdapter*);
static SWS_FASTCALLDEF( void, AutomatonReset( SAutomaton*));
static SWS_FASTCALLDEF( void, AutomatonDtor( SAutomaton*));
static SWS_FASTCALLDEF( void, AutomatonRx( SAutomaton*, SWS_BufferH));
static void AutomatonEvent( SAutomaton*, EEvent, SWS_BufferH);
#define AutomatonSend( p, h) PppSendRaw( (p)->pAdapter, h, (p)->protocol)
static int AutomatonCfgReq( SAutomaton*, SWS_BufferH, unsigned);
static void AutomatonCfgRsp( SAutomaton*, unsigned, SWS_BufferH, unsigned);
static SWS_FTimer AutomatonTimeout;

static FAutomaton LcpReset;
static FAutomaton LcpUp;
static FAutomaton LcpDown;
static FAutomaton LcpFinished;
static FAutomaton LcpStarted;
static FAutomatonCfg LcpCfg;
static FAutomatonRsp LcpRsp;
static FAutomatonReq LcpReq;
static FPlatformDpc LcpFinishedDpc;
#if OPT_PPP_IDENT
static SWS_FASTCALL( SWS_BufferH, LcpIdent( SAutomaton*, const char*));
#endif

static FAutomaton IpcpReset;
static FAutomaton IpcpUp;
static FAutomaton IpcpDown;
static FAutomaton IpcpFinished;
static FAutomaton IpcpStarted;
static FAutomatonCfg IpcpCfg;
static FAutomatonRsp IpcpRsp;
static FAutomatonReq IpcpReq;

#if OPT_PPP_AUTH
static FAutomaton AuthReset;
static FAutomaton AuthUp;
static FAutomaton AuthDown;
static FAutomaton AuthFinished;
static FAutomatonCfg AuthCfg;

static void PapCfg( SAutomaton*, SWS_BufferH);
static ui8 PppReqPap( SPppAdapter*, SWS_PTR( const void), unsigned);
static void ChapCfg( SAutomaton*, SWS_BufferH);
static SWS_FASTCALL( ui8, ChapResponse( SAutomaton*, SWS_BufferH));
#endif

/* P2P downcalls */
static SWS_FP2pProtoRelease PppRelease;
static SWS_FP2pProtoSend PppSendIp;
static SWS_FP2pProtoGetProperty PppGetProperty;
static SWS_FP2pProtoSetProperty PppSetProperty;
static SWS_FP2pProtoLinkInd PppLinkInd;
static FPlatformCallback PppOnLinkUp;
static FPlatformCallback PppOnLinkDown;
static FPlatformCallback PppOnLinkShutdown;

/* Adapter up calls */
static SWS_FP2pSendAck PppSendAck;
static SWS_FP2pRecvInd PppRecvInd;
static FPlatformDpc PppRecvIndDpc;


/*
 * Module data
 */

/* Configuration */
static unsigned s_txSize, s_rxSize;   /* tx/rx buffer size */

/* LCP configuration */
static int s_iMaxTerminate;
static int s_iMaxConfigure;
static int s_iMaxFailure;
static ui32 s_uRestartPeriod;         /* Restart period ticks */

/* Stats */
static Atomic_t s_aTxPackets, s_aTxBytes;
static Atomic_t s_aTxErrors;          /* Driver send errors */
static Atomic_t s_aTxNoBuffers;       /* No buffers in send */
static Atomic_t s_aRxPackets, s_aRxBytes;
static Atomic_t s_aRxErrors;          /* Rx errors indicated */
static Atomic_t s_aRxNoBuffers;       /* Received packets dropped by driver */
static Atomic_t s_aRxFraming;         /* Bad FCS, frame too big */


/****************************************************************************
 * External functions
 ****************************************************************************/

/*
 * Initialise this module
 */
extern int SWS_PppInit( void)         /* Returns !0 on error */
  {
  TRACE( 9, ("SWS_PppInit()\n"));
  ASSERT( PlatformIsCritical());

  /* Init static data */
  AtomicWrite( &s_aTxPackets, 0);
  AtomicWrite( &s_aTxBytes, 0);
  AtomicWrite( &s_aTxErrors, 0);
  AtomicWrite( &s_aTxNoBuffers, 0);
  AtomicWrite( &s_aRxPackets, 0);
  AtomicWrite( &s_aRxBytes, 0);
  AtomicWrite( &s_aRxErrors, 0);
  AtomicWrite( &s_aRxNoBuffers, 0);
  AtomicWrite( &s_aRxFraming, 0);

  /* Read config data */
  s_txSize = (unsigned)PlatformPropertyGetUint(
    PKEY_PPP_TXBUFFER, OPT_PPP_TXBUFFER );
  if ( s_txSize < kPppMaxPacket)
    s_txSize = kPppMaxPacket;

  s_rxSize = (unsigned)PlatformPropertyGetUint(
    PKEY_PPP_RXBUFFER, OPT_PPP_RXBUFFER );
  if ( s_rxSize < kPppMaxPacket)
    s_rxSize = kPppMaxPacket;

  s_iMaxTerminate = (int)PlatformPropertyGetUint(
    PKEY_PPP_RETRY_TERM, OPT_PPP_RETRY_TERM); /* RFC 1661 p24 */
  s_iMaxConfigure = (int)PlatformPropertyGetUint(
    PKEY_PPP_RETRY_CFG, OPT_PPP_RETRY_CFG); /* RFC 1661 p24 */
  s_iMaxFailure = (int)PlatformPropertyGetUint(
    PKEY_PPP_RETRY_NAK, OPT_PPP_RETRY_NAK); /* RFC 1661 p25 */
  s_uRestartPeriod = PlatformPropertyGetUint(
    PKEY_PPP_RETRY_PERIOD, OPT_PPP_RETRY_PERIOD); /* RFC 1661 p24 */
#if 1000 != SWS_TIMER_TICKS
  s_uRestartPeriod = (s_uRestartPeriod * SWS_TIMER_TICKS) / 1000;
#endif

  return 0;
  }


/*
 * Finalise this module
 */
extern void SWS_PppCleanup( void)
  {
  TRACE( 9, ("SWS_PppCleanup()\n"));
  ASSERT( PlatformIsCritical());

  TRACE( 1, ("PPP: TxPackets:%u TxBytes:%u TxErrors:%u"
    " RxPackets:%u RxBytes:%u RxErrors:%u RxNoBuffers:%u RxFraming:%u\n",
    AtomicRead( &s_aTxPackets), AtomicRead( &s_aTxBytes),
    AtomicRead( &s_aTxErrors),
    AtomicRead( &s_aRxPackets), AtomicRead( &s_aRxBytes),
    AtomicRead( &s_aRxErrors), AtomicRead( &s_aRxNoBuffers),
    AtomicRead( &s_aRxFraming)
  ));
  }


/*
 * Bind protocol to an adapter
 */
extern int SWS_PppBind(               /* Returns !0 on error */
  unsigned uAdapter,                  /* IN: P2P Adapter ID */
  SWS_PTR( const SWS_DatalinkCallbacks) pUp, /* IN: Up calls */
  SWS_PTR( const SWS_P2pProtocolDn) pDn, /* IN: Down calls */
  SWS_PTR( SWS_P2pProtocol) pOut         /* OUT: Callbacks */
) {
  SPppAdapter* pAdapter;

  TRACE( 9, ("SWS_PppBind(%u,%p,%p,%p)\n",
    uAdapter, pUp,pDn,pOut));
  ASSERT( NULL != pUp);
  ASSERT( pUp->pfnSendRdy);
  ASSERT( pUp->pfnRecvInd);
  ASSERT( pUp->pfnLinkStatus);
  ASSERT( NULL != pDn);
  ASSERT( pDn->pfnSend);
  ASSERT( pDn->pfnRecv);
  ASSERT( NULL != pOut);

  /* Alloc instance and rx/tx buffers */
  pAdapter = (SPppAdapter*) malloc( sizeof(*pAdapter) + 2 + s_rxSize + s_txSize);
  if ( NULL == pAdapter)
    {
    TRACE( 0, ("!!! SWS_PppBind too many adapters\n"));
    return SWS_ENOMEM;
    }
  memset( pAdapter, 0, sizeof(*pAdapter));

  pAdapter->pRxBuffer = pAdapter + 1;
  pAdapter->pTxBuffer = (char*)pAdapter->pRxBuffer + s_rxSize + 1;
  pAdapter->rxSize = 1 + s_rxSize;
  pAdapter->txSize = 1 + s_txSize;
  ASSERT( PppTxFree( pAdapter) == s_txSize);
  ASSERT( PppRxFree( pAdapter) == s_rxSize);

  pAdapter->hBuffer = NULL;

  /* Save context info */
  pAdapter->uP2pAdapter = uAdapter;
  pAdapter->dn = *pDn;
  pAdapter->up = *pUp;

  pAdapter->uMagic = ADAPTER_MAGIC;

  pAdapter->automLcp.pfnReset = &LcpReset;
  pAdapter->automLcp.pfnUp = &LcpUp;
  pAdapter->automLcp.pfnDown = &LcpDown;
  pAdapter->automLcp.pfnStarted = &LcpStarted;
  pAdapter->automLcp.pfnFinished = &LcpFinished;
  pAdapter->automLcp.pfnConfigAdd = &LcpCfg;
  pAdapter->automLcp.pfnConfigRsp = &LcpRsp;
  pAdapter->automLcp.pfnConfigReq = &LcpReq;

  pAdapter->automIpcp.pfnReset = &IpcpReset;
  pAdapter->automIpcp.pfnUp = &IpcpUp;
  pAdapter->automIpcp.pfnDown = &IpcpDown;
  pAdapter->automIpcp.pfnStarted = &IpcpStarted;
  pAdapter->automIpcp.pfnFinished = &IpcpFinished;
  pAdapter->automIpcp.pfnConfigAdd = &IpcpCfg;
  pAdapter->automIpcp.pfnConfigRsp = &IpcpRsp;
  pAdapter->automIpcp.pfnConfigReq = &IpcpReq;

#if OPT_PPP_AUTH
  pAdapter->automAuth.pfnReset = &AuthReset;
  pAdapter->automAuth.pfnUp = &AuthUp;
  pAdapter->automAuth.pfnDown = &AuthDown;
  pAdapter->automAuth.pfnStarted = 0;
  pAdapter->automAuth.pfnFinished = &AuthFinished;
  pAdapter->automAuth.pfnConfigAdd = 0;
  pAdapter->automAuth.pfnConfigRsp = 0;
  pAdapter->automAuth.pfnConfigReq = 0;
#endif

  if ( AutomatonCtor( &pAdapter->automLcp, SWS_PPP_LCP, pAdapter)
    || AutomatonCtor( &pAdapter->automIpcp, SWS_PPP_IPCP, pAdapter)
#if OPT_PPP_AUTH
    || AutomatonCtor( &pAdapter->automAuth, 0, pAdapter)
#endif
  ) {
    PppDestroyInstance( pAdapter);
    return SWS_ENOMEM;
    }

  /* Callbacks */
  pOut->pvContext = pAdapter;
  pOut->pfnRelease = &PppRelease;
  pOut->pfnSend = &PppSendIp;
  pOut->pfnSendAck = &PppSendAck;
  pOut->pfnRecvInd = &PppRecvInd;
  pOut->pfnLinkInd = &PppLinkInd;
  pOut->pfnGetProperty = &PppGetProperty;
  pOut->pfnSetProperty = &PppSetProperty;

  /* Admin open */
  AutomatonEvent( &pAdapter->automIpcp, kEventOpen, NULL);

  return 0;
  }


/*
 * Destroy instance
 */
static SWS_FASTCALL( void, PppDestroyInstance( SPppAdapter* pAdapter))
  {
  TRACE( 2, ("PppDestroyInstance(%p)\n", pAdapter));
  ASSERT( pAdapter);

  AutomatonDtor( &pAdapter->automLcp);
  AutomatonDtor( &pAdapter->automIpcp);
#if OPT_PPP_AUTH
  AutomatonDtor( &pAdapter->automAuth);
#endif

  if ( NULL != pAdapter->hBuffer)
    {
    SWS_BufferRelease( pAdapter->hBuffer);
    pAdapter->hBuffer = NULL;
    }

#if OPT_P2P_VJCOMP
  if ( NULL != pAdapter->hRxVjComp)
    SWS_P2pVjClose( pAdapter->hRxVjComp);

  if ( NULL != pAdapter->hTxVjComp)
    SWS_P2pVjClose( pAdapter->hTxVjComp);
#endif

  memset( pAdapter, 0, sizeof( *pAdapter));;
  free( pAdapter);
  }


/****************************************************************************
 * Down calls from P2P
 ****************************************************************************/

/*
 * Unbind protocol
 */
static SWS_CB( void, PppRelease(
  void* pvContext
)){
  SPppAdapter* const pAdapter = (SPppAdapter*)pvContext;

  TRACE( 9, ("PppRelease(%p)\n", pvContext));
  ASSERT( IS_PPP_ADAPTER( pAdapter));

  /* Admin close */
  AutomatonEvent( &pAdapter->automIpcp, kEventClose, NULL);

  pAdapter->iRxEnable = 0;
  if ( pAdapter->dn.pfnRecv && pAdapter->iRxPending)
    {
    /* Cancel pending Rx */
    (*pAdapter->dn.pfnRecv)( pAdapter->dn.pvContext, NULL, NULL, -1, 0);
    pAdapter->iRxPending = 0;
    }

  PppDestroyInstance( pAdapter);
  }


/*
 * Get an interface property
 */
static SWS_CB( int, PppGetProperty(
  SWS_PTR( void) pvContext,
  int type,
  SWS_PTR( void) pvBuf,
  SWS_PTR( size_t) puLen
)){
  SPppAdapter* const pAdapter = (SPppAdapter*)pvContext;

  TRACE( 9, ("PppGetProperty(%p,%d,%p,%p)\n", pvContext,type,pvBuf,puLen));
  ASSERT( IS_PPP_ADAPTER( pAdapter));
  ASSERT( NULL != pvBuf);
  ASSERT( NULL != puLen);

  switch ( type)
    {
  default:
    /* Not implemented */
    TRACE( 3, ("PppGetProperty %d not implemented\n", type));
    return SWS_EBADF;

  case SWS_kDatalinkProperty_Address:
    if ( *puLen < sizeof(ui32))
      {
      TRACE( 0, ("!!! PppGetProperty buffer too small\n"));
      return SWS_EINVAL;
      }

    *(SWS_PTR(ui32))pvBuf = pAdapter->uIpAddress;
    *puLen = sizeof(ui32);
    break;

  case SWS_kDatalinkProperty_Gateway:
    if ( *puLen < sizeof(ui32))
      {
      TRACE( 0, ("!!! PppGetProperty buffer too small\n"));
      return SWS_EINVAL;
      }

    *(SWS_PTR(ui32))pvBuf = pAdapter->uGatewayAddr;
    *puLen = sizeof(ui32);
    break;

  case SWS_kDatalinkProperty_MTU:
    if ( *puLen < sizeof(size_t))
      {
      TRACE( 0, ("!!! PppGetProperty buffer too small\n"));
      return SWS_EINVAL;
      }

    *(SWS_PTR(size_t))pvBuf = pAdapter->uMru;
    *puLen = sizeof(size_t);
    break;
    }

  return 0;
  }


/*
 * Set an interface property
 */
static SWS_CB( int, PppSetProperty(
  SWS_PTR( void) pvContext,
  int type,
  SWS_PTR( const void) pvBuf,
  size_t len
)){
  SPppAdapter* const pAdapter = (SPppAdapter*)pvContext;

  TRACE( 9, ("PppSetProperty(%p,%d,%p,%u)\n", pvContext,type,pvBuf,len));
  ASSERT( IS_PPP_ADAPTER( pAdapter));
  ASSERT( NULL != pvBuf);

  switch ( type)
    {
  default:
    /* Not implemented */
    TRACE( 3, ("PppSetProperty %d not implemented\n", type));
    return SWS_EBADF;

  case SWS_kDatalinkProperty_Address:
    if ( len < sizeof(ui32))
      {
      TRACE( 0, ("!!! PppSetProperty buffer too small\n"));
      return SWS_EINVAL;
      }

    if ( pAdapter->isUp)
      {
      TRACE( 1, ("*** PppSetProperty Address while link up\n"));
      return SWS_EACCES;
      }

    pAdapter->uIpAddress = *(SWS_PTR(const ui32))pvBuf;
    break;
    }

  return 0;
  }


/*
 * Send an IP packet
 * Single threaded
 */
static SWS_CB( int, PppSendIp(
  void* pvContext,
  SWS_BufferH hBuffer
)){
  SPppAdapter* const pAdapter = (SPppAdapter*)pvContext;
  int iRet;

  TRACE( 9, ("PppSendIp(%p,%p)\n", pvContext, hBuffer));
  ASSERT( NULL != hBuffer);
  ASSERT( IS_PPP_ADAPTER( pAdapter));
  ASSERT( PlatformIsCritical());

  if ( pAdapter->isUp > 0)
    {
    ui16 protocol = SWS_PPP_IP;

#if OPT_P2P_VJCOMP
    /* VJ header compression negotiated? */
    if ( NULL != pAdapter->hTxVjComp)
      {
      /* Yes, compress the packet header */
      switch ( SWS_P2pVjCompress( pAdapter->hTxVjComp, hBuffer))
        {
      case SWS_kP2pVjTypeError:
      case SWS_kP2pVjTypeIP:
        break;

      case SWS_kP2pVjTypeUncompressed:
        protocol = SWS_PPP_VJU;
        break;

      case SWS_kP2pVjTypeCompressed:
        protocol = SWS_PPP_VJ;
        break;
        }
      }
#endif

    iRet = PppSendRaw( pAdapter, hBuffer, protocol);
    if ( iRet <= 0)
      {
      /* Don't report immediate completion since this causes
       * excessive buffering and increased latencies */
#if 0
      /* Is there room for another packet in the tx buffer? */
      if ( PppTxFree( pAdapter) >= kPppMaxPacket)
        {
        /* Yes */
        iRet = 0;
        }
      else
#endif
        {
        /* No room so get PppSendAck to check */
        pAdapter->iTxBusy = 1;
        iRet = -1;

        /* Verify callback is pending */
        if ( 0 == pAdapter->uTxPending && pAdapter->iTxBusy)
          {
          /* Callback occurred between PppTxFree and setting iTxBusy */
          ASSERT( PppTxFree( pAdapter) >= kPppMaxPacket);
          pAdapter->iTxBusy = 0;
          iRet = 0;
          }
        }
      }
    }
  else
    {
    TRACE( 0, ("!!! PppSendIp() link down\n"));
    iRet = SWS_ENETDOWN;
    }

  return iRet;
  }


/****************************************************************************
 * Up calls from adapter
 ****************************************************************************/

/*
 * Adapter send async completion
 * !!! CAUTION IRQ TIME !!!
 */
static SWS_CB( void, PppSendAck(
  SWS_PTR( void) pvContext,           /* IN: Client context */
  int iErr                            /* IN: !0 if error */
)){
  SPppAdapter* const pAdapter = (SPppAdapter*)pvContext;
  unsigned txRead;

  TRACE( 10, ("PppSendAck(%p,%d)\n", pvContext, iErr));

  if ( !IS_PPP_ADAPTER( pAdapter))
    return;

  if ( iErr)
    AtomicIncrement( &s_aTxErrors);

  txRead = pAdapter->uTxPending;
  ASSERT( txRead > 0);

  /* Update the tx buffer output cursor */
  txRead += pAdapter->txRead;
  ASSERT( txRead <= pAdapter->txSize);
  if ( txRead >= pAdapter->txSize)
    txRead = 0;
  pAdapter->txRead = txRead;

  pAdapter->uTxPending = 0;

  /* Send any remaining tx data */
  for ( ;;)
    {
    unsigned const count = PppTxPending( pAdapter);
    if ( 0 == count)
      break;

    if ( PppTransmit( pAdapter, count))
      break;
    }

  /* Did PppSendIp return a pending status? */
  if ( pAdapter->iTxBusy)
    {
    /* Yes, then is there space for another packet? */
    if ( PppTxFree( pAdapter) >= kPppMaxPacket)
      {
      /* Yes */
      pAdapter->iTxBusy = 0;

      /* Issue a 'TxReady' indication up to the IP layer */
      ASSERT( pAdapter->up.pfnSendRdy);
      (*pAdapter->up.pfnSendRdy)( pAdapter->up.pvContext);
      }
    else
      ASSERT( pAdapter->uTxPending > 0);
    }
  }


/*
 * Received data indication
 * !!! CAUTION IRQ TIME !!!
 */
static SWS_CB( void, PppRecvInd(
  SWS_PTR( void) pvContext,           /* IN: Client context */
  size_t uLen,                        /* IN: Data size */
  int iErr                            /* IN: Final error code, 0= no error */
)){
  SPppAdapter* const pAdapter = (SPppAdapter*)pvContext;

  TRACE( 10, ("PppRecvInd(%p,%u,%d)\n", pvContext,uLen,iErr));

  if ( IS_PPP_ADAPTER( pAdapter)
    && pAdapter->iRxPending > 0
  ) {
    unsigned u;

    pAdapter->iRxPending = 0;

    if ( iErr)
      {
      TRACE( 10, ("PppRecvInd(%p) error %d\n", pAdapter, iErr));
      AtomicIncrement( &s_aRxErrors);
      }

    AtomicAdd( &s_aRxBytes, (int)uLen);

    /* Update the receive buffer write cursor */
    u = pAdapter->rxWrite + uLen;
    if ( u >= pAdapter->rxSize)
      u -= pAdapter->rxSize;
    pAdapter->rxWrite = u;

    if ( pAdapter->iRxEnable)
      {
      /* Queue more Rx buffers */
      do
        {
        iErr = PppReceive( pAdapter);
        if ( iErr)
          break;
        }
      while ( !pAdapter->iRxPending);
      }
    }
  }


/*
 * Adapter link status change callback
 */
static SWS_CB( void, PppLinkInd(
  SWS_PTR( void) pvContext,
  int iLinkUp
)){
  SPppAdapter* const pAdapter = (SPppAdapter*)pvContext;

  TRACE( 9, ("PppLinkInd(%p,%d)\n", pvContext,iLinkUp));

  if ( IS_PPP_ADAPTER( pAdapter))
    {
    switch ( iLinkUp)
      {
    case SWS_kP2pLinkUp:
      PlatformSynchronise( &PppOnLinkUp, pAdapter);
      break;

    case SWS_kP2pLinkGoingDown:
      PlatformSynchronise( &PppOnLinkShutdown, pAdapter);
      break;

    case SWS_kP2pLinkDown:
      PlatformSynchronise( &PppOnLinkDown, pAdapter);
      break;
      }
    }
  }

/* Link up */
static SWS_CB( int, PppOnLinkUp( SWS_PTR( void) pv))
  {
  SPppAdapter* const pAdapter = (SPppAdapter*)pv;

  TRACE( 3, ("PppOnLinkUp(%p)\n", pv));
  ASSERT( IS_PPP_ADAPTER( pAdapter));
  ASSERT( PlatformIsCritical());

  AutomatonEvent( &pAdapter->automLcp, kEventOpen, NULL);

  if ( pAdapter->automLcp.eState > kStateStarting)
    AutomatonEvent( &pAdapter->automLcp, kEventDown, NULL);

  AutomatonEvent( &pAdapter->automLcp, kEventUp, NULL);

  return 0;
  }

/* Handle unexpected link shutdown */
static SWS_CB( int, PppOnLinkDown( SWS_PTR( void) pv))
  {
  SPppAdapter* const pAdapter = (SPppAdapter*)pv;

  TRACE( 3, ("PppOnLinkDown(%p)\n", pv));
  ASSERT( IS_PPP_ADAPTER( pAdapter));
  ASSERT( PlatformIsCritical());

  switch ( pAdapter->automLcp.eState)
    {
  case kStateInitial:
    break;

  case kStateStarting:
    AutomatonEvent( &pAdapter->automLcp, kEventClose, NULL);
    break;

  case kStateClosing:
  case kStateClosed:
    AutomatonEvent( &pAdapter->automLcp, kEventDown, NULL);
    break;

  case kStateStopped:
  case kStateStopping:
  case kStateReqSent:
  case kStateAckRcvd:
  case kStateAckSent:
  case kStateOpened:
    AutomatonEvent( &pAdapter->automLcp, kEventDown, NULL);
    AutomatonEvent( &pAdapter->automLcp, kEventClose, NULL);
    break;
    }

  /* NB pAdapter may be invalid if protocol released */
  return 0;
  }

/* Orderly shutdown link */
static SWS_CB( int, PppOnLinkShutdown( SWS_PTR( void) pv))
  {
  SPppAdapter* const pAdapter = (SPppAdapter*)pv;

  TRACE( 3, ("PppOnLinkShutdown(%p)\n", pv));
  ASSERT( IS_PPP_ADAPTER( pAdapter));
  ASSERT( PlatformIsCritical());

  /* Spin while link is torn down */
  for ( ;;)
    {
    switch ( pAdapter->automLcp.eState)
      {
    case kStateClosed:
      AutomatonEvent( &pAdapter->automLcp, kEventDown, NULL);
      /* NB pAdapter may be invalid if protocol released */
      return 0;

    case kStateInitial:
      return 0;

    case kStateClosing:
      break;

    case kStateStarting:
    case kStateStopped:
    case kStateStopping:
    case kStateReqSent:
    case kStateAckRcvd:
    case kStateAckSent:
    case kStateOpened:
      AutomatonEvent( &pAdapter->automLcp, kEventClose, NULL);
      break;
      }

    PlatformIdle();
    }
  }


/****************************************************************************
 * Transmit support
 ****************************************************************************/

/*
 * Append a byte to the Tx ring buffer
 */
static SWS_FASTCALL( void, PppPut( SPppAdapter* const pAdapter, ui8 data))
  {
  unsigned u = pAdapter->txWrite;

  ((ui8*)pAdapter->pTxBuffer)[ u] = data;
  if ( ++u >= pAdapter->txSize)
    u = 0;

  ASSERT( pAdapter->txRead != u);
  pAdapter->txWrite = u;
  }


/*
 * Send buffered data to the output device
 */
static SWS_FASTCALL( int, PppTransmit(
  SPppAdapter* const pAdapter,
  unsigned count
)){
  int iRet;

  TRACE( 9, ("PppTransmit(%p,%u)\n", pAdapter,count));
  ASSERT( IS_PPP_ADAPTER( pAdapter));

  if ( 0 == count)
    iRet = 0;
  else if ( pAdapter->uTxPending > 0)
    iRet = -1;
  else
    {
    /* For smooth buffer management limit count */
    size_t const lim = pAdapter->txSize / 4;
    if ( count > lim)
      count = lim;

    AtomicAdd( &s_aTxBytes, (int)count);
    pAdapter->uTxPending = count;

    /* Send to driver */
    iRet = (*pAdapter->dn.pfnSend)(
      pAdapter->dn.pvContext,
      (ui8*)pAdapter->pTxBuffer + pAdapter->txRead,
      count
    );
    if ( -1 != iRet)
      {
      /* Immediate completion */
      /* Update the tx buffer output cursor */
      count += pAdapter->txRead;
      ASSERT( count <= pAdapter->txSize);
      if ( count >= pAdapter->txSize)
        count = 0;
      pAdapter->txRead = count;

      pAdapter->uTxPending = 0;

      if ( iRet)
        {
        TRACE( 2, ("PppTransmit(%p) error %d\n", pAdapter, iRet));
        AtomicIncrement( &s_aTxErrors);

        /* Tx error so discard all unsent tx data */
        pAdapter->txRead = pAdapter->txWrite;
        }
      }
    }

  return iRet;
  }


/****************************************************************************
 * Receive support
 ****************************************************************************/

/*
 * Fetch a byte from the Rx ring buffer
 */
static INLINE( int, PppGet( SPppAdapter* const pAdapter))
  {
  int data;
  unsigned u = pAdapter->rxRead;

  if ( u == pAdapter->rxWrite)
    return -1;

  data = ((ui8*)pAdapter->pRxBuffer)[ u];
  if ( ++u >= pAdapter->rxSize)
    u = 0;

  pAdapter->rxRead = u;

  TRACE( 5, ("PppGet() %#x\n", data));
  return data;
  }


/*
 * Request input device for data
 * !!! CAUTION IRQ TIME !!!
 */
static SWS_FASTCALL( int, PppReceive(
  SPppAdapter* const pAdapter
)){
  int iRet;
  size_t count;

  TRACE( 10, ("PppReceive(%p)\n", pAdapter));
  ASSERT( IS_PPP_ADAPTER( pAdapter));
  ASSERT( !pAdapter->iRxPending);
  ASSERT( pAdapter->iRxEnable);

  /* Get the available receive buffer space */
  count = PppRxFree( pAdapter);
  if ( count > 0)
    {
    ASSERT( pAdapter->rxWrite + count <= pAdapter->rxSize);
    if ( count > kPppMaxPacket)
      count = kPppMaxPacket;

    /* Request device to receive */
    pAdapter->iRxPending = 1;

    iRet = (*pAdapter->dn.pfnRecv)(
      pAdapter->dn.pvContext,
      (ui8*)pAdapter->pRxBuffer + pAdapter->rxWrite,
      &count,
      kHdlcFlag,
      0
    );
    if ( -1 != iRet)
      {
      unsigned u;

      /* Immediate completion */
      pAdapter->iRxPending = 0;

      /* Update the receive buffer write cursor */
      u = (unsigned)count + pAdapter->rxWrite;
      if ( u >= pAdapter->rxSize)
        u -= pAdapter->rxSize;
      pAdapter->rxWrite = u;

      if ( iRet)
        AtomicIncrement( &s_aRxErrors);
      }
    }
  else
    {
    TRACE( 1, ("*** PppReceive() RxBuffer overrun\n"));
    AtomicIncrement( &s_aRxNoBuffers);
    iRet = SWS_ENOBUFS;
    }

  /* Request a DPC if rx data pending */
  if ( pAdapter->rxWrite != pAdapter->rxRead
    && !pAdapter->iRxBusy
  ) {
    pAdapter->iRxBusy = 1;

    /* Request a DPC */
    if ( PlatformDpc( &PppRecvIndDpc, pAdapter, NULL, 0) )
      {
      /* Q full */
      pAdapter->iRxBusy = 0;
      }
    }

  return iRet;
  }


/*****************************************************************************
 * HDLC framing
 ****************************************************************************/

/*
 * Calculate Frame Check Sequence
 * See RFC 1662.p19
 * >>>> NB This routine is critical to performance <<<<
 */
static ui16 PppFcs16(
  SWS_PTR( const ui8) pSrc,
  SWS_size_t uLen,
  unsigned fcs
) {
  /* FCS lookup table from RFC 1662.p19 */
  static const ui16 s_fcstab[ 256] = {
    0x0000, 0x1189, 0x2312, 0x329b, 0x4624, 0x57ad, 0x6536, 0x74bf,
    0x8c48, 0x9dc1, 0xaf5a, 0xbed3, 0xca6c, 0xdbe5, 0xe97e, 0xf8f7,
    0x1081, 0x0108, 0x3393, 0x221a, 0x56a5, 0x472c, 0x75b7, 0x643e,
    0x9cc9, 0x8d40, 0xbfdb, 0xae52, 0xdaed, 0xcb64, 0xf9ff, 0xe876,
    0x2102, 0x308b, 0x0210, 0x1399, 0x6726, 0x76af, 0x4434, 0x55bd,
    0xad4a, 0xbcc3, 0x8e58, 0x9fd1, 0xeb6e, 0xfae7, 0xc87c, 0xd9f5,
    0x3183, 0x200a, 0x1291, 0x0318, 0x77a7, 0x662e, 0x54b5, 0x453c,
    0xbdcb, 0xac42, 0x9ed9, 0x8f50, 0xfbef, 0xea66, 0xd8fd, 0xc974,
    0x4204, 0x538d, 0x6116, 0x709f, 0x0420, 0x15a9, 0x2732, 0x36bb,
    0xce4c, 0xdfc5, 0xed5e, 0xfcd7, 0x8868, 0x99e1, 0xab7a, 0xbaf3,
    0x5285, 0x430c, 0x7197, 0x601e, 0x14a1, 0x0528, 0x37b3, 0x263a,
    0xdecd, 0xcf44, 0xfddf, 0xec56, 0x98e9, 0x8960, 0xbbfb, 0xaa72,
    0x6306, 0x728f, 0x4014, 0x519d, 0x2522, 0x34ab, 0x0630, 0x17b9,
    0xef4e, 0xfec7, 0xcc5c, 0xddd5, 0xa96a, 0xb8e3, 0x8a78, 0x9bf1,
    0x7387, 0x620e, 0x5095, 0x411c, 0x35a3, 0x242a, 0x16b1, 0x0738,
    0xffcf, 0xee46, 0xdcdd, 0xcd54, 0xb9eb, 0xa862, 0x9af9, 0x8b70,
    0x8408, 0x9581, 0xa71a, 0xb693, 0xc22c, 0xd3a5, 0xe13e, 0xf0b7,
    0x0840, 0x19c9, 0x2b52, 0x3adb, 0x4e64, 0x5fed, 0x6d76, 0x7cff,
    0x9489, 0x8500, 0xb79b, 0xa612, 0xd2ad, 0xc324, 0xf1bf, 0xe036,
    0x18c1, 0x0948, 0x3bd3, 0x2a5a, 0x5ee5, 0x4f6c, 0x7df7, 0x6c7e,
    0xa50a, 0xb483, 0x8618, 0x9791, 0xe32e, 0xf2a7, 0xc03c, 0xd1b5,
    0x2942, 0x38cb, 0x0a50, 0x1bd9, 0x6f66, 0x7eef, 0x4c74, 0x5dfd,
    0xb58b, 0xa402, 0x9699, 0x8710, 0xf3af, 0xe226, 0xd0bd, 0xc134,
    0x39c3, 0x284a, 0x1ad1, 0x0b58, 0x7fe7, 0x6e6e, 0x5cf5, 0x4d7c,
    0xc60c, 0xd785, 0xe51e, 0xf497, 0x8028, 0x91a1, 0xa33a, 0xb2b3,
    0x4a44, 0x5bcd, 0x6956, 0x78df, 0x0c60, 0x1de9, 0x2f72, 0x3efb,
    0xd68d, 0xc704, 0xf59f, 0xe416, 0x90a9, 0x8120, 0xb3bb, 0xa232,
    0x5ac5, 0x4b4c, 0x79d7, 0x685e, 0x1ce1, 0x0d68, 0x3ff3, 0x2e7a,
    0xe70e, 0xf687, 0xc41c, 0xd595, 0xa12a, 0xb0a3, 0x8238, 0x93b1,
    0x6b46, 0x7acf, 0x4854, 0x59dd, 0x2d62, 0x3ceb, 0x0e70, 0x1ff9,
    0xf78f, 0xe606, 0xd49d, 0xc514, 0xb1ab, 0xa022, 0x92b9, 0x8330,
    0x7bc7, 0x6a4e, 0x58d5, 0x495c, 0x3de3, 0x2c6a, 0x1ef1, 0x0f78
  };

  while ( uLen--)
    fcs = (fcs >> 8) ^ s_fcstab[ (fcs ^ *pSrc++) & 0xff];

  return (ui16)fcs;
  }


/*
 * Calculate Frame Check Sequence over a SWS_BufferH
 */
static SWS_FASTCALL( ui16, PppFcs16Buffer( SWS_BufferH hBuffer, unsigned fcs))
  {
  SWS_size_t uLen, uSrc;

  uSrc = 0;
  uLen = SWS_BufferSize( hBuffer);
  while ( uLen)
    {
    SWS_PTR( const ui8) pSrc;
    SWS_size_t uLenSrc;

    /* Get next fragment */
    pSrc = (SWS_PTR( const ui8)) SWS_BufferGetFragment(
      hBuffer, uSrc, &uLenSrc);
    ASSERT( NULL != pSrc);

    uSrc += uLenSrc;
    ASSERT( uLenSrc <= uLen);
    uLen -= uLenSrc;

    fcs = PppFcs16( pSrc, uLenSrc, fcs);
    }

  return (ui16)fcs;
  }


/*
 * Send a PPP packet
 */
static int PppSendRaw(
  SPppAdapter* const pAdapter,
  SWS_BufferH hBuffer,                /* IN: Buffer to send (no header) */
  ui16 protocol                       /* IN: Protocol ID */
) {
  int iRet;
  SWS_size_t uLen, uSrc;
  SWS_PTR( void) pvHdr;
  ui16 fcs;

  TRACE( 9, ("PppSendRaw(%p,%p,%#x)\n", pAdapter, hBuffer, protocol));
  ASSERT( IS_PPP_ADAPTER( pAdapter));
  ASSERT( NULL != hBuffer);
  ASSERT( PlatformIsCritical());

  /* Ensure packet is <= max size */
  uLen = SWS_BufferSize( hBuffer);
  if ( 2 * uLen + 2 * kHdlcOverhead + 2 > PppTxFree( pAdapter) )
    {
    TRACE( 0, ("!!! PppSendRaw() packet too big\n"));
    return SWS_EMSGSIZE;
    }

  /* Call event filter */
  if ( 0 != (iRet = SWS_GefRaise( SWS_kGefPppTx, (unsigned)protocol, hBuffer)))
    {
    TRACE( 3, ("PppSendRaw() filtered %d\n", iRet));
    return iRet;
    }

  /* Prepend protocol ID */
  if ( pAdapter->isPFC > 0 && protocol <= 0xff)
    uLen = 1;
  else
    uLen = 2;

  iRet = SWS_BufferAppendFront( hBuffer, uLen );
  if ( iRet)
    return iRet;

  pvHdr = SWS_BufferGetFragment( hBuffer, 0, &uLen);
  ASSERT( NULL != pvHdr);

  if ( pAdapter->isPFC > 0 && protocol <= 0xff)
    {
    ASSERT( uLen >= 1);
    SWS_POKE8( pvHdr, 0, (ui8)protocol);
    }
  else
    {
    ASSERT( uLen >= 2);
    SWS_POKE16( pvHdr, 0, (ui16)protocol);
    }

  /* Prepend HDLC header */
  if ( pAdapter->isACFC <= 0
    || SWS_PPP_LCP == protocol
  ) {
    iRet = SWS_BufferAppendFront( hBuffer, 2 );
    if ( iRet)
      return iRet;

    pvHdr = SWS_BufferGetFragment( hBuffer, 0, &uLen);
    ASSERT( NULL != pvHdr);
    ASSERT( uLen >= 2);

    SWS_POKE8( pvHdr, 0, kHdlcAddress);
    SWS_POKE8( pvHdr, 1, kHdlcControl);
    }
  uLen = SWS_BufferSize( hBuffer);

  /* Calc FCS */
  fcs = PppFcs16Buffer( hBuffer, PPPINITFCS16);

  /* Append FCS */
  iRet = SWS_BufferAppendBack( hBuffer, 2 );
  if ( iRet)
    return iRet;

  pvHdr = SWS_BufferGetFragment( hBuffer, uLen, &uSrc);
  ASSERT( NULL != pvHdr);

  /* NB FCS is stored least significant byte first */
  fcs ^= 0xffff;
  SWS_POKE8( pvHdr, 0, (ui8)(fcs & 0xff));
  pvHdr = SWS_BufferGetFragment( hBuffer, uLen + 1, &uSrc);
  ASSERT( NULL != pvHdr);
  SWS_POKE8( pvHdr, 0, (ui8)(fcs >> 8));

  /* Start with HDLC flag */
  PppPut( pAdapter, kHdlcFlag);

  /* Process the buffer */
  uSrc = 0;
  uLen = SWS_BufferSize( hBuffer);
  while ( uLen)
    {
    SWS_PTR( const ui8) pSrc;
    SWS_size_t uLenSrc;

    pSrc = (SWS_PTR( const ui8)) SWS_BufferGetFragment(
      hBuffer, uSrc, &uLenSrc);
    ASSERT( NULL != pSrc);

    uSrc += uLenSrc;
    ASSERT( uLenSrc <= uLen);
    uLen -= uLenSrc;

    /* Translate flag & esc codes */
    while ( uLenSrc--)
      {
      ui8 data = *pSrc++;
      switch ( data)
        {
      case 0x00:
      case 0x01:
      case 0x02:
      case 0x03:
      case 0x04:
      case 0x05:
      case 0x06:
      case 0x07:
      case 0x08:
      case 0x09:
      case 0x0a:
      case 0x0b:
      case 0x0c:
      case 0x0d:
      case 0x0e:
      case 0x0f:
      case 0x10:
      case 0x11:
      case 0x12:
      case 0x13:
      case 0x14:
      case 0x15:
      case 0x16:
      case 0x17:
      case 0x18:
      case 0x19:
      case 0x1a:
      case 0x1b:
      case 0x1c:
      case 0x1d:
      case 0x1e:
      case 0x1f:
        if ( SWS_PPP_LCP == protocol
          || (pAdapter->uTxACCM & (1 << data))
        ) {
          PppPut( pAdapter, kHdlcEsc);
          PppPut( pAdapter, HDLC_ESC( data));
          }
        else
          PppPut( pAdapter, data);
        break;

      case kHdlcFlag:
        PppPut( pAdapter, kHdlcEsc);
        PppPut( pAdapter, HDLC_ESC( kHdlcFlag));
        break;
      case kHdlcEsc:
        PppPut( pAdapter, kHdlcEsc);
        PppPut( pAdapter, HDLC_ESC( kHdlcEsc));
        break;

      default:
        PppPut( pAdapter, data);
        break;
        }
      }
    }

  /* End with HDLC flag */
  PppPut( pAdapter, kHdlcFlag);

  /* Send it */
  AtomicIncrement( &s_aTxPackets);
  iRet = 0;
  do
    {
    /* Get count of tx bytes pending */
    unsigned const count = PppTxPending( pAdapter);
    if ( 0 == count)
      break; /* All sent */

    iRet = PppTransmit( pAdapter, count);
    }
  /* Rpt until error or pending(busy) */
  while ( 0 == iRet);

  return iRet;
  }


/*
 * Received data indication - deferred procedure callback
 */
static SWS_CB( void, PppRecvIndDpc(
  SWS_PTR( void) pvContext,           /* IN: Client context */
  SWS_PTR( void) pvData,              /* IN: */
  ui32 uLen                           /* IN: */
)){
  SPppAdapter* const pAdapter = (SPppAdapter*)pvContext;
  int data;
  (void)pvData;
  (void)uLen;

  TRACE( 5, ("PppRecvIndDpc(%p,%p,%lu)\n",pvContext,pvData,uLen));
  ASSERT( PlatformIsCritical());

  if ( !IS_PPP_ADAPTER( pAdapter))
    return;

  ASSERT( pAdapter->iRxBusy);

  /* For each received byte */
  while ( (data = PppGet( pAdapter)) >= 0)
    {
    /* Look for flags */
    if ( kHdlcFlag == data)
      {
      /* Ignore empty packets caused by back to back flags */
      if ( pAdapter->uReceived > 0
        && !pAdapter->iEsc
      ) {
        SWS_BufferH hBuffer;
        SWS_size_t len;
        ui16 fcs;

        AtomicIncrement( &s_aRxPackets);

        ASSERT( NULL != pAdapter->hBuffer);
        hBuffer = pAdapter->hBuffer;
        pAdapter->hBuffer = NULL;

        len = SWS_BufferSize( hBuffer);
        if ( len > pAdapter->uReceived)
          SWS_BufferRemoveBack( hBuffer, len - pAdapter->uReceived);
        else
          ASSERT( !(len < pAdapter->uReceived));

        pAdapter->uReceived = 0;

        /* Verify the FCS */
        fcs = PppFcs16Buffer( hBuffer, PPPINITFCS16);
        if ( PPPGOODFCS16 == fcs)
          {
          SWS_PTR( const ui8) pHdr;
          ui16 pcol;
          unsigned u;

          /* Exclude the FCS */
          SWS_BufferRemoveBack( hBuffer, 2);

          /* Get header */
          pHdr = (SWS_PTR( const ui8)) SWS_BufferGetFragment(
            hBuffer, 0, &len);
          ASSERT( NULL != pHdr);
          ASSERT( len >= 2);

          len = 0;
          if ( kHdlcAddress == pHdr[0] && kHdlcControl == pHdr[1])
            {
            /* Drop address & control fields */
            pHdr += 2;
            len += 2;
            }

          /* Read protocol using ISO 3309 extension mechanism */
          pcol = 0;
          do
            {
            ++len;
            u = *pHdr++;
            pcol = (ui16)((pcol << 8) | u);
            }
          while ( !(1 & u));

          /* Drop the HDLC header */
          SWS_BufferRemoveFront( hBuffer, len);

          /* Call event filter */
          if ( 0 != SWS_GefRaise( SWS_kGefPppRx, (unsigned)pcol, hBuffer))
            {
            TRACE( 3, ("PppRecvIndDpc() filtered\n"));
            }
          /* Protocol specific action */
          else switch ( pcol)
            {
          case SWS_PPP_IP:
            /* Pass frame up to IP layer */
            (*pAdapter->up.pfnRecvInd)(
              pAdapter->up.pvContext,
              hBuffer, 0
            );
            break;

          case SWS_PPP_LCP:
            AutomatonRx( &pAdapter->automLcp, hBuffer);
            break;

          case SWS_PPP_IPCP:
            AutomatonRx( &pAdapter->automIpcp, hBuffer);
            break;

#if OPT_PPP_AUTH
          case SWS_PPP_CHAP:
            PppRxChap( pAdapter, hBuffer);
            break;

          case SWS_PPP_PAP:
            PppRxPap( pAdapter, hBuffer);
            break;
#endif

#if OPT_PPP_CCP
          case SWS_PPP_CCP:
            PppRxCcp( pAdapter, hBuffer);
            break;
#endif

#if OPT_P2P_VJCOMP
          case SWS_PPP_VJ:
          case SWS_PPP_VJU:
            if ( NULL != pAdapter->hRxVjComp)
              {
              if ( 0 == SWS_P2pVjDecompress(
                pAdapter->hRxVjComp,
                SWS_PPP_VJ == pcol ? SWS_kP2pVjTypeCompressed : SWS_kP2pVjTypeUncompressed,
                hBuffer
              )){
                /* Pass frame up to IP layer */
                (*pAdapter->up.pfnRecvInd)(
                  pAdapter->up.pvContext,
                  hBuffer, 0
                );
                }
              }
            else
              PppRxUnknown( pAdapter, hBuffer, pcol);
            break;
#endif

          default:
            PppRxUnknown( pAdapter, hBuffer, pcol);
            break;
            }
          }
        else
          {
          TRACE( 1, ("*** PppRecvIndDpc() bad FCS %lu bytes\n",
             (ui32)SWS_BufferSize( hBuffer) ));

          AtomicIncrement( &s_aRxFraming);
          SWS_GefRaise( SWS_kGefPppError, "Bad FCS");

#if OPT_P2P_VJCOMP
          if ( NULL != pAdapter->hRxVjComp)
            SWS_P2pVjDecompress( pAdapter->hRxVjComp, SWS_kP2pVjTypeError, NULL);
#endif
          }

        SWS_BufferRelease( hBuffer);
        }

      pAdapter->sizeBuf = 0;
      pAdapter->iEsc = 0;
      continue;
      }
    /* Translate escaped data */
    else if ( pAdapter->iEsc)
      {
      pAdapter->iEsc = 0;
      data = HDLC_ESC( data);
      }
    /* Look for escape char */
    else if ( kHdlcEsc == data)
      {
      pAdapter->iEsc = 1;
      continue;
      }
    /* Ignore unescaped data within the receiving ACCM */
    else if ( data < 0x20
      && ((1 << data) & pAdapter->uRxACCM)
    ) {
      TRACE( 1, ("*** PppRecvIndDpc() unescaped %#x\n", data));
      SWS_GefRaise( SWS_kGefPppError, "Unescaped data");
      continue;
      }

    /* Output buffer assigned? */
    if ( NULL == pAdapter->hBuffer)
      {
      /* No, get an output buffer */
      pAdapter->hBuffer = SWS_BufferGet(
        pAdapter->uMtu + kHdlcOverhead, SWS_kBufferDefault);
      if ( NULL == pAdapter->hBuffer)
        {
        AtomicIncrement( &s_aRxNoBuffers);

        /* Flush the rx buffer */
        pAdapter->rxRead = pAdapter->rxWrite;
        break;
        }

      pAdapter->uReceived = 0;
      pAdapter->sizeBuf = 0;
      }

    /* More room in current fragment? */
    if ( 0 == pAdapter->sizeBuf)
      {
      /* No, get the next buffer fragment */
      pAdapter->pucBuf = (SWS_PTR( ui8))SWS_BufferGetFragment(
        pAdapter->hBuffer,
        pAdapter->uReceived,
        &pAdapter->sizeBuf
      );
      if ( NULL == pAdapter->pucBuf)
        {
        /* Frame too big - kHdlcFlag marker lost */
        TRACE( 2, ("PppRecvIndDpc() Buffer overrun\n"));

        AtomicIncrement( &s_aRxFraming);
        SWS_GefRaise( SWS_kGefPppError, "Rx buffer overrun");

#if OPT_P2P_VJCOMP
        if ( NULL != pAdapter->hRxVjComp)
          SWS_P2pVjDecompress( pAdapter->hRxVjComp, SWS_kP2pVjTypeError, NULL);
#endif

        /* Dump it */
        pAdapter->uReceived = 0;
        pAdapter->sizeBuf = 0;
        continue;
        }
      }

    /* Store the byte in the output buffer */
    ASSERT( NULL != pAdapter->pucBuf);
    *(pAdapter->pucBuf++) = (ui8)data;
    ASSERT( pAdapter->sizeBuf > 0);
    pAdapter->sizeBuf--;
    pAdapter->uReceived++;
    }

  pAdapter->iRxBusy = 0;

  /* Queue more Rx buffers */
  while ( pAdapter->iRxEnable && !pAdapter->iRxPending)
    {
    if ( PppReceive( pAdapter))
      break;
    }
  }


/*
 * Receive packet for unsupported protocols
 */
static void PppRxUnknown(
  SPppAdapter* pAdapter,
  SWS_BufferH hBuffer,
  ui16 pcol
) {
  TRACE( 9, ("PppRxUnknown(%p,%p,%u)\n",pAdapter,hBuffer,pcol));
  ASSERT( IS_PPP_ADAPTER( pAdapter));

  /* Send protocol reject */
  if ( 0 == SWS_BufferAppendFront( hBuffer, 6))
    {
    SWS_PTR( void) pvHdr;
    SWS_size_t uLen;

    pvHdr = SWS_BufferGetFragment( hBuffer, 0, &uLen);
    ASSERT( NULL != pvHdr);
    ASSERT( uLen >= 6);

    SWS_POKE8( pvHdr, SWS_PPP_HDR_CODE, SWS_PPP_PROTOCOL_REJ);
    if ( 0 == pAdapter->automLcp.uRejId)
      pAdapter->automLcp.uRejId = pAdapter->automLcp.uCfgId;
    SWS_POKE8( pvHdr, SWS_PPP_HDR_ID, ++pAdapter->automLcp.uRejId);
    uLen = SWS_BufferSize( hBuffer);
    SWS_POKE16( pvHdr, SWS_PPP_HDR_LEN, (ui16)uLen);
    SWS_POKE16( pvHdr, SWS_PPP_HDR_LEN + 2, pcol);

    (void)PppSendRaw( pAdapter, hBuffer, SWS_PPP_LCP);
    }
  }


#if OPT_PPP_CCP
/*****************************************************************************
 * CCP protocol
 ****************************************************************************/
/*
 * Parse received CCP packets, RFC 1962
 */
static SWS_FASTCALL( void, PppRxCcp(
  SPppAdapter* pAdapter,
  SWS_BufferH hBuffer
)){
  SWS_PTR( void) pvHdr;
  SWS_size_t uLen;

  TRACE( 9, ("PppRxCcp(%p,%p)\n",pAdapter,hBuffer));
  ASSERT( IS_PPP_ADAPTER( pAdapter));

  pvHdr = SWS_BufferGetFragment( hBuffer, 0, &uLen);
  if ( NULL != pvHdr
    && uLen >= SWS_PPP_HDR_DATA
  ) {
    ui8 code, id;
    ui16 len;

    SWS_PEEK8( code, pvHdr, SWS_PPP_HDR_CODE);
    SWS_PEEK8( id, pvHdr, SWS_PPP_HDR_ID);
    SWS_PEEK16( len, pvHdr, SWS_PPP_HDR_LEN);

    switch ( code)
      {
    case SWS_PPP_CONFIGURE_REQ:
      SWS_POKE8( pvHdr, SWS_PPP_HDR_CODE, SWS_PPP_CONFIGURE_ACK);
      PppSendRaw( pAdapter, hBuffer, SWS_PPP_CCP);
      break;

    case SWS_PPP_CONFIGURE_ACK:
      AutomatonEvent( &pAdapter->automIpcp, kEventUp, NULL);
      break;

    case SWS_PPP_CONFIGURE_NAK:
    case SWS_PPP_CONFIGURE_REJ:
      break;

    default:
      break;
      }
    }
  }


static SWS_FASTCALLDEF( void, CcpCfg( SPppAdapter* pAdapter))
  {
  SWS_BufferH hBuffer;

  TRACE( 9, ("CcpCfg(%p)\n",pAdapter));
  ASSERT( IS_PPP_ADAPTER( pAdapter));

  hBuffer = SWS_BufferGet(
    SWS_PPP_HDR_DATA + SWS_PPP_OPT_DATA + 4, SWS_kBufferDefault);
  if ( NULL != hBuffer)
    {
    SWS_PTR( void) pvHdr;
    SWS_size_t uLen;

    /* Fill in header */
    pvHdr = SWS_BufferGetFragment( hBuffer, 0, &uLen);
    ASSERT( NULL != pvHdr);
    SWS_POKE8( pvHdr, SWS_PPP_HDR_CODE, SWS_PPP_CONFIGURE_REQ);
    SWS_POKE8( pvHdr, SWS_PPP_HDR_ID, 1);
    uLen = SWS_BufferSize( hBuffer);
    SWS_POKE16( pvHdr, SWS_PPP_HDR_LEN, (ui16)uLen);

    /* Option data */
    SWS_POKE8( pvHdr, SWS_PPP_HDR_DATA + SWS_PPP_OPT_TYPE, 18); /* 18= MSFT-PPC */
    SWS_POKE8( pvHdr, SWS_PPP_HDR_DATA + SWS_PPP_OPT_LEN, SWS_PPP_OPT_DATA + 4);
    SWS_POKE32( pvHdr, SWS_PPP_HDR_DATA + SWS_PPP_OPT_DATA, 1);

    PppSendRaw( pAdapter, hBuffer, SWS_PPP_CCP);
    SWS_BufferRelease( hBuffer);
    }
  }
#endif


#if OPT_PPP_AUTH
/*****************************************************************************
 * CHAP protocol
 ****************************************************************************/
/*
 * Parse received CHAP packets, RFC 1994
 */
static SWS_FASTCALL( void, PppRxChap(
  SPppAdapter* pAdapter,
  SWS_BufferH hBuffer
)){
  SAutomaton* const p = &pAdapter->automAuth;
  SWS_PTR( void) pvHdr;
  SWS_size_t uLen;

  TRACE( 9, ("PppRxChap(%p,%p)\n",pAdapter,hBuffer));
  ASSERT( IS_PPP_ADAPTER( pAdapter));

  p->protocol = SWS_PPP_CHAP;

  pvHdr = SWS_BufferGetFragment( hBuffer, 0, &uLen);
  if ( NULL != pvHdr
    && uLen >= SWS_PPP_HDR_DATA
  ) {
    ui8 code, id, size;
    ui16 len;
    SWS_SP2pAcct user;
    size_t sizeUser;

    SWS_PEEK8( code, pvHdr, SWS_PPP_HDR_CODE);
    SWS_PEEK8( id, pvHdr, SWS_PPP_HDR_ID);
    SWS_PEEK16( len, pvHdr, SWS_PPP_HDR_LEN);

    switch ( code)
      {
    case SWS_PPP_CHAP_CHALLENGE:
      SWS_PEEK8( size, pvHdr, SWS_PPP_HDR_DATA);
      if ( len < (ui16)(SWS_PPP_HDR_DATA + 1U + size))
        {
        TRACE( 1, ("*** PppRxChap() Challenge too short\n"));
        break;
        }

      if ( 5 == pAdapter->uChapAlgo)
        {
        /* MD5 */
        SWS_MD5 md5;

        SWS_POKE8( pvHdr, SWS_PPP_HDR_CODE, SWS_PPP_CHAP_RESPONSE);
        SWS_POKE8( pvHdr, SWS_PPP_HDR_DATA, sizeof(SWS_MD5Digest));
        len = (ui16)(len - (SWS_PPP_HDR_DATA + 1U + sizeof(SWS_MD5Digest)));

        /* Get username & password */
        VERIFY( !SWS_P2pGetAcct( pAdapter->uP2pAdapter, &user));

        /* Calc response */
        SWS_MD5Init( &md5);
        SWS_MD5Update( &md5, &id, sizeof(id) );
        SWS_MD5Update( &md5, user.szPassword, strlen( user.szPassword) );
        SWS_MD5Update(
          &md5, (SWS_PTR( char))pvHdr + SWS_PPP_HDR_DATA + 1, size);
        SWS_MD5Final(
          &md5,
          (SWS_MD5Digest*)((SWS_PTR( char))pvHdr + SWS_PPP_HDR_DATA + 1)
        );

        sizeUser = strlen( user.szName);
        SWS_POKE16(
          pvHdr, SWS_PPP_HDR_LEN,
          (ui16)(SWS_PPP_HDR_DATA + 1 + sizeof(SWS_MD5Digest) + sizeUser)
        );

        /* Append username */
        SWS_BufferRemoveBack( hBuffer, len);
        SWS_BufferAppendBack( hBuffer, sizeUser);
        SWS_BufferCopyFromMemory(
          hBuffer, SWS_PPP_HDR_DATA + 1 + sizeof(SWS_MD5Digest), user.szName, sizeUser);
        }
#if OPT_PPP_MSCHAP
      else if ( 128 == pAdapter->uChapAlgo)
        {
        ui8 buf[ 24];
        enum { kRspSize = 24 + 24 + 1 };

        /* MS-CHAP RFC 2433 */
        if ( 8 != size)
          {
          TRACE( 1, ("*** PppRxChap() Incorrect size for MS-CHAP challenge\n"));
          break;
          }

        /* Get username & password */
        VERIFY( !SWS_P2pGetAcct( pAdapter->uP2pAdapter, &user));
        sizeUser = strlen( user.szName);

        SWS_POKE8( pvHdr, SWS_PPP_HDR_CODE, SWS_PPP_CHAP_RESPONSE);
        SWS_POKE16(
          pvHdr, SWS_PPP_HDR_LEN,
          (ui16)(SWS_PPP_HDR_DATA + 1 + kRspSize + sizeUser)
        );
        SWS_POKE8( pvHdr, SWS_PPP_HDR_DATA, kRspSize);

        SWS_BufferRemoveBack( hBuffer, size);
        SWS_BufferAppendBack( hBuffer, kRspSize + sizeUser);

        /* LmChallengeResponse */
        memset( buf, 0, sizeof( buf));
        SWS_BufferCopyFromMemory(
          hBuffer, SWS_PPP_HDR_DATA + 1, buf, sizeof( buf));

        /* NtChallengeResponse */
        SWS_BufferCopyFromMemory(
          hBuffer, SWS_PPP_HDR_DATA + 1 + 24, buf, sizeof( buf));

        /* "Use Windows NT compatible challenge response" flag */
        pvHdr = SWS_BufferGetFragment(
          hBuffer, SWS_PPP_HDR_DATA + 1 + 48, &uLen);
        SWS_POKE8( pvHdr, 0, 0);

        /* Append username */
        SWS_BufferCopyFromMemory(
          hBuffer, SWS_PPP_HDR_DATA + 1 + kRspSize, user.szName, sizeUser);
        }
#endif
#if OPT_PPP_MSCHAP2
      else if ( 129 == pAdapter->uChapAlgo)
        {
        /* RFC 2759) */
        {int TODO_MSCHAP2_Challenge;}
        TRACE( 1, ("*** PppRxChap() TODO MSCHAP2 challenge\n"));
        break;
        }
#endif
      else
        {
        TRACE( 0, ("!!! PppRxChap() Unknow challenge algorithm\n"));
        break;
        }

      switch ( p->eState)
        {
      case kStateInitial:
      case kStateStarting:
        TRACE( 1, ("*** PppRxChap() REQ while starting\n"));
        break;

      case kStateOpened:
        AutomatonSend( p, hBuffer);
        break;

      default:
        AutomatonEvent( p, kEventRCR_P, hBuffer);
        break;
        }
      break;

    case SWS_PPP_CHAP_RESPONSE:
      if ( id == p->uCfgId)
        {
        /* Validate response */
        code = ChapResponse( p, hBuffer);

        SWS_POKE8( pvHdr, SWS_PPP_HDR_CODE, code);
        SWS_POKE16( pvHdr, SWS_PPP_HDR_LEN, SWS_PPP_HDR_DATA);
        uLen = SWS_BufferSize( hBuffer);
        if ( uLen > SWS_PPP_HDR_DATA)
          SWS_BufferRemoveBack( hBuffer, uLen - SWS_PPP_HDR_DATA);

        /* Send response */
        AutomatonSend( p, hBuffer);

        if ( SWS_PPP_CHAP_SUCCESS == code)
          {
          pAdapter->uRxAuthState = 0;
          if ( 0 == pAdapter->uTxAuthState)
            AutomatonEvent( p, kEventRCA, hBuffer);
          else if ( SWS_PPP_PAP == pAdapter->uTxAuthState)
            AutomatonEvent( p, kEventRCN, hBuffer);
          }
        else if ( ++p->iCfgFailures < s_iMaxFailure)
          AutomatonEvent( p, kEventRCN, hBuffer);
        else
          AutomatonEvent( p, kEventRXJ_M, hBuffer);
        }
      else
        TRACE( 1, ("*** PppRxChap(%p) response mismatched id %u\n",p,id));
      break;

    case SWS_PPP_CHAP_SUCCESS:
      switch ( p->eState)
        {
      case kStateInitial:
      case kStateStarting:
        TRACE( 1, ("*** PppRxChap() SUCCESS while starting\n"));
        break;

      default:
        pAdapter->uTxAuthState = 0;
        if ( 0 == pAdapter->uRxAuthState)
          AutomatonEvent( p, kEventRCA, hBuffer);
        break;
        }
      break;

    case SWS_PPP_CHAP_FAILURE:
      switch ( p->eState)
        {
      case kStateInitial:
      case kStateStarting:
        TRACE( 1, ("*** PppRxChap() FAILURE while starting\n"));
        break;

      default:
        if ( ++p->iCfgFailures < s_iMaxFailure)
          AutomatonEvent( p, kEventRCN, hBuffer);
        else
          AutomatonEvent( p, kEventRXJ_M, hBuffer);
        break;
        }
      break;

    default:
      TRACE( 1, ("*** PppRxChap() Unexpected code %u\n",code));
      break;
      }
    }
  }


/*
 * Construct a CHAP challenge
 */
static void ChapCfg( SAutomaton* pAutomaton, SWS_BufferH hBuffer)
  {
  SPppAdapter* const pAdapter = pAutomaton->pAdapter;
  SWS_size_t const uHdr = SWS_BufferSize( hBuffer);
  size_t size, sizeName;
  char szBuf[ 64];

  TRACE( 9, ("ChapCfg(%p,%p)\n",pAutomaton,hBuffer));
  ASSERT( NULL != pAutomaton);
  ASSERT( IS_PPP_ADAPTER( pAdapter));

  /* An autheticator doesn't receive Auth-Req's so assume ack'd */
  pAutomaton->eState = kStateAckSent;

  /* Get hostname */
  SWS_gethostname( szBuf, sizeof( szBuf));
  sizeName = strlen( szBuf);
  size = 1 + sizeof( pAdapter->challenge) + sizeName;

  /* Append space for request */
  if ( !SWS_BufferAppendBack( hBuffer, size))
    {
    SWS_PTR( void) pvHdr;
    SWS_size_t uReq;
    SWS_MD5 md5;
    ui32 const uTicks = PlatformTime();

    /* Get ptr to request */
    pvHdr = SWS_BufferGetFragment( hBuffer, uHdr, &uReq);
    ASSERT( NULL != pvHdr);
    ASSERT( uReq >= size);

    /* Calc. new challenge */
    SWS_MD5Init( &md5);
    SWS_MD5Update( &md5, &pAutomaton->uCfgId, sizeof(pAutomaton->uCfgId) );
    SWS_MD5Update( &md5, &uTicks, sizeof(uTicks) );
    SWS_MD5Final( &md5, &pAdapter->challenge);

     /* Append challenge */
    SWS_POKE8( pvHdr, 0, (ui8)sizeof( pAdapter->challenge));
    SWS_BufferCopyFromMemory(
      hBuffer, uHdr + 1, &pAdapter->challenge, sizeof( pAdapter->challenge));

    /* Append name */
    SWS_BufferCopyFromMemory(
      hBuffer, uHdr + 1 + sizeof( pAdapter->challenge), szBuf, sizeName);
    }
  }


/*
 * Validate a CHAP response
 */
static SWS_FASTCALL( ui8, ChapResponse(
  SAutomaton* pAutomaton,
  SWS_BufferH hBuffer
)){
  SPppAdapter* const pAdapter = pAutomaton->pAdapter;
  SWS_PTR( void) pvHdr;
  SWS_size_t uLen;
  ui16 len;
  ui8 id, size;
  SWS_SP2pAcct acct;
  SWS_MD5 md5;
  SWS_MD5Digest digest;

  TRACE( 9, ("ChapResponse(%p,%p)\n",pAutomaton,hBuffer));
  ASSERT( NULL != pAutomaton);
  ASSERT( IS_PPP_ADAPTER( pAdapter));

  pvHdr = SWS_BufferGetFragment( hBuffer, 0, &uLen);
  if ( NULL == pvHdr
    || uLen < SWS_PPP_HDR_DATA + 1 + sizeof(SWS_MD5Digest)
  ) {
    TRACE( 1, ("*** ChapResponse() buffer too small\n"));
    return SWS_PPP_CHAP_FAILURE;
    }

  SWS_PEEK8( id, pvHdr, SWS_PPP_HDR_ID);
  SWS_PEEK16( len, pvHdr, SWS_PPP_HDR_LEN);
  if ( len < SWS_PPP_HDR_DATA + 1 + sizeof(SWS_MD5Digest))
    {
    TRACE( 1, ("*** ChapResponse() length too small\n"));
    return SWS_PPP_CHAP_FAILURE;
    }

  len -= SWS_PPP_HDR_DATA + 1 + sizeof(SWS_MD5Digest);
  if ( len >= sizeof(acct.szName))
    {
    TRACE( 1, ("*** ChapResponse() name too long\n"));
    return SWS_PPP_CHAP_FAILURE;
    }

  SWS_PEEK8( size, pvHdr, SWS_PPP_HDR_DATA);
  if ( size != sizeof(SWS_MD5Digest))
    {
    TRACE( 1, ("*** ChapResponse() incorrect value size\n"));
    return SWS_PPP_CHAP_FAILURE;
    }

  /* Get acct name */
  memcpy( acct.szName, (const char*)pvHdr + SWS_PPP_HDR_DATA + 1 + size, len);
  acct.szName[ len] = 0;

  /* Get acct info */
  if ( SWS_P2pSetAcct( pAdapter->uP2pAdapter, &acct))
    {
    TRACE( 1, ("*** ChapResponse() no such user %s\n", acct.szName));
    return SWS_PPP_CHAP_FAILURE;
    }

  /* Calc digest for acct password */
  SWS_MD5Init( &md5);
  SWS_MD5Update( &md5, &id, sizeof(id) );
  SWS_MD5Update( &md5, acct.szPassword, strlen( acct.szPassword) );
  SWS_MD5Update( &md5, &pAdapter->challenge, sizeof(pAdapter->challenge));
  SWS_MD5Final( &md5, &digest);

  /* Compare to response */
  if ( memcmp(
      &digest, (const char*)pvHdr + SWS_PPP_HDR_DATA + 1, sizeof(digest)
    )
  ) {
    TRACE( 1, ("*** ChapResponse() incorrect response\n"));
    return SWS_PPP_CHAP_FAILURE;
    }

  return SWS_PPP_CHAP_SUCCESS;
  }
#endif /* OPT_PPP_AUTH */


#if OPT_PPP_AUTH
/*****************************************************************************
 * PAP protocol
 ****************************************************************************/
/*
 * Parse received PAP packets, RFC 1334
 */
static SWS_FASTCALL( void, PppRxPap(
  SPppAdapter* pAdapter,
  SWS_BufferH hBuffer
)){
  SAutomaton* const p = &pAdapter->automAuth;
  SWS_PTR( void) pvHdr;
  SWS_size_t uLen;

  TRACE( 9, ("PppRxPap(%p,%p)\n",pAdapter,hBuffer));
  ASSERT( IS_PPP_ADAPTER( pAdapter));

  p->protocol = SWS_PPP_PAP;

  pvHdr = SWS_BufferGetFragment( hBuffer, 0, &uLen);
  if ( NULL != pvHdr
    && uLen >= SWS_PPP_HDR_DATA
  ) {
    ui8 code, id;
    ui16 len;

    SWS_PEEK8( code, pvHdr, SWS_PPP_HDR_CODE);
    SWS_PEEK8( id, pvHdr, SWS_PPP_HDR_ID);
    SWS_PEEK16( len, pvHdr, SWS_PPP_HDR_LEN);

    switch ( code)
      {
    case SWS_PPP_PAP_REQ: /* Authenticate-Request */
      switch ( p->eState)
        {
      case kStateInitial:
      case kStateStarting:
        TRACE( 1, ("*** PppRxPap() REQ while starting\n"));
        break;

      case kStateOpened:
        break;

      case kStateReqSent:
        /* Treat this an an Ack */
        if ( 0 == pAdapter->uTxAuthState && 0 == pAdapter->uRxAuthState)
          AutomatonEvent( p, kEventRCA, hBuffer);

        /* Fall thru */
      default:
        id = PppReqPap( pAdapter, pvHdr, len);
        SWS_POKE8( pvHdr, SWS_PPP_HDR_CODE, id);
        if ( SWS_PPP_PAP_ACK == id)
          {
          pAdapter->uRxAuthState = 0;
          if ( 0 == pAdapter->uTxAuthState)
            AutomatonEvent( p, kEventRCA, hBuffer);
          AutomatonEvent( p, kEventRCR_P, hBuffer);
          }
        else if ( ++p->iCfgFailures < s_iMaxFailure)
          {
          uLen = SWS_BufferSize( hBuffer);
          if ( uLen > SWS_PPP_HDR_DATA)
            SWS_BufferRemoveBack( hBuffer, uLen - SWS_PPP_HDR_DATA);

          AutomatonSend( p, hBuffer);
          }
        else
          AutomatonEvent( p, kEventRXJ_M, hBuffer);
        break;
        }
      break;

    case SWS_PPP_PAP_ACK:
      if ( id == p->uCfgId)
        {
        pAdapter->uTxAuthState = 0;
        if ( 0 == pAdapter->uRxAuthState)
          AutomatonEvent( p, kEventRCA, hBuffer);
        else if ( SWS_PPP_CHAP == pAdapter->uRxAuthState)
          AutomatonEvent( p, kEventRCN, hBuffer);
        }
      else
        TRACE( 1, ("*** PppRxPap(%p) Ack mismatched id %u\n",p,id));
      break;

    case SWS_PPP_PAP_NAK:
      if ( id == p->uCfgId)
        AutomatonEvent( p, kEventRCN, hBuffer);
      else
        TRACE( 1, ("*** PppRxPap(%p) Nak mismatched id %u\n",p,id));
      break;

    default:
      TRACE( 1, ("*** PppRxPap() Unexpected code %u\n",code));
      break;
      }
    }
  }


/*
 * Validate PAP Authenticate-Request
 */
static ui8 PppReqPap(
  SPppAdapter* pAdapter,
  SWS_PTR( const void) pvHdr,
  unsigned len
) {
  unsigned uPeer, uPasswd;
  const char *pPasswd;
  SWS_SP2pAcct acct;

  TRACE( 9, ( "PppReqPap(%p,%p,%u)\n",pAdapter,pvHdr,len));
  ASSERT( NULL != pvHdr);

  if ( len < SWS_PPP_HDR_DATA + 1)
    return SWS_PPP_PAP_NAK;

  /* Peer-ID Length */
  SWS_PEEK8( uPeer, pvHdr, SWS_PPP_HDR_DATA);
  if ( len < SWS_PPP_HDR_DATA + 1 + uPeer)
    return SWS_PPP_PAP_NAK;

  if ( uPeer >= sizeof(acct.szName))
    return SWS_PPP_PAP_NAK;

  memcpy( acct.szName, (const char*)pvHdr + SWS_PPP_HDR_DATA + 1, uPeer);
  acct.szName[ uPeer] = 0;

  /* Passwd-Length */
  SWS_PEEK8( uPasswd, pvHdr, SWS_PPP_HDR_DATA + 1 + uPeer);
  if ( len < SWS_PPP_HDR_DATA + 2 + uPeer + uPasswd)
    return SWS_PPP_PAP_NAK;

  /* Passwd */
  pPasswd = (const char*)pvHdr + SWS_PPP_HDR_DATA + 2 + uPeer;

  /* Verify username */
  if ( SWS_P2pSetAcct( pAdapter->uP2pAdapter, &acct))
    return SWS_PPP_PAP_NAK;

  if ( strncmp( acct.szPassword, pPasswd, uPasswd))
    return SWS_PPP_PAP_NAK;

  return SWS_PPP_PAP_ACK;
  }


/*
 * Construct PAP (RFC 1334) Authenticate-Request
 */
static void PapCfg( SAutomaton* pAutomaton, SWS_BufferH hBuffer)
  {
  SPppAdapter* const pAdapter = pAutomaton->pAdapter;
  SWS_size_t uHdr;
  size_t size, sizeUser, sizePass;
  SWS_SP2pAcct user;

  TRACE( 9, ("PapCfg(%p,%p)\n",pAutomaton,hBuffer));
  ASSERT( NULL != pAutomaton);
  ASSERT( IS_PPP_ADAPTER( pAdapter));

  /* A client doesn't receive Auth-Req's so assume ack'd */
  pAutomaton->eState = kStateAckSent;

  uHdr = SWS_BufferSize( hBuffer);

  /* Get username & password */
  VERIFY( !SWS_P2pGetAcct( pAdapter->uP2pAdapter, &user));
  sizeUser = strlen( user.szName);
  sizePass = strlen( user.szPassword);
  size = 2 + sizeUser + sizePass;

  /* Append space for request */
  if ( !SWS_BufferAppendBack( hBuffer, size))
    {
    SWS_PTR( void) pvHdr;
    SWS_size_t uReq;

    /* Get ptr to request */
    pvHdr = SWS_BufferGetFragment( hBuffer, uHdr, &uReq);
    ASSERT( NULL != pvHdr);
    ASSERT( uReq >= size);

    /* Append username */
    SWS_POKE8( pvHdr, 0, (ui8)sizeUser);
    SWS_BufferCopyFromMemory( hBuffer, uHdr + 1, user.szName, sizeUser);

    /* Append password */
    SWS_POKE8( pvHdr, 1 + sizeUser, (ui8)sizePass);
    SWS_BufferCopyFromMemory(
      hBuffer, uHdr + 2 + sizeUser, user.szPassword, sizePass);
    }
  }
#endif /*OPT_PPP_AUTH*/


#if OPT_PPP_AUTH
/****************************************************************************
 * Authentication automaton
 ****************************************************************************/
/*
 * Reset
 */
static SWS_FASTCALL( void, AuthReset( SAutomaton* p))
  {
  SPppAdapter* const pAdapter = p->pAdapter;

  TRACE( 9, ("AuthReset(%p)\n",p));
  ASSERT( NULL != p);
  ASSERT( IS_PPP_ADAPTER( pAdapter));

  AutomatonReset( p);

  p->protocol = 0;
  p->pfnConfigAdd = 0;
  pAdapter->uRxAuth = SWS_P2pGetAuth( pAdapter->uP2pAdapter);
  pAdapter->uTxAuthState = 0;
  pAdapter->uRxAuthState = 0;
  }


/*
 * This layer up
 */
static SWS_FASTCALL( void, AuthUp( SAutomaton* p))
  {
  SPppAdapter* const pAdapter = p->pAdapter;

  TRACE( 2, ("AuthUp(%p)\n",p));
  ASSERT( NULL != p);
  ASSERT( IS_PPP_ADAPTER( pAdapter));
  ASSERT( p->protocol);

  AutomatonEvent( &pAdapter->automIpcp, kEventUp, NULL);
  }


/*
 * This layer down
 */
static SWS_FASTCALL( void, AuthDown( SAutomaton* p))
  {
  SPppAdapter* const pAdapter = p->pAdapter;

  TRACE( 2, ("AuthDown(%p)\n",p));
  ASSERT( NULL != p);
  ASSERT( IS_PPP_ADAPTER( pAdapter));

  p->protocol = 0;
  p->pfnConfigAdd = 0;
  pAdapter->uRxAuth = SWS_P2pGetAuth( pAdapter->uP2pAdapter);
  pAdapter->uTxAuthState = 0;
  pAdapter->uRxAuthState = 0;

  if ( pAdapter->automIpcp.eState > kStateStarting)
    AutomatonEvent( &pAdapter->automIpcp, kEventDown, NULL);
  }


/*
 * This layer finished
 */
static SWS_FASTCALL( void, AuthFinished( SAutomaton* p))
  {
  SPppAdapter* const pAdapter = p->pAdapter;

  TRACE( 2, ("AuthFinished(%p)\n",p));
  ASSERT( NULL != p);
  ASSERT( IS_PPP_ADAPTER( pAdapter));

  AuthReset( p);
  AutomatonEvent( &pAdapter->automLcp, kEventClose, NULL);
  }


static SWS_FASTCALL( void, AuthCfg( SAutomaton* p, SWS_BufferH hBuffer))
  {
  SPppAdapter* const pAdapter = p->pAdapter;

  if ( SWS_PPP_CHAP == pAdapter->uRxAuthState)
    {
    p->protocol = SWS_PPP_CHAP;
    ChapCfg( p, hBuffer);
    }
  else if ( SWS_PPP_PAP == pAdapter->uTxAuthState)
    {
    p->protocol = SWS_PPP_PAP;
    PapCfg( p, hBuffer);
    }
  }
#endif /*OPT_PPP_AUTH*/


/*****************************************************************************
 * LCP automaton
 ****************************************************************************/
/*
 * Reset LCP state
 */
static SWS_FASTCALL( void, LcpReset( SAutomaton* p))
  {
  SPppAdapter* const pAdapter = p->pAdapter;

  TRACE( 9, ("LcpReset(%p)\n",p));
  ASSERT( NULL != p);
  ASSERT( IS_PPP_ADAPTER( pAdapter));

  AutomatonReset( p);

  pAdapter->isACFC = -1;
  pAdapter->isPFC = -1;
  pAdapter->uTxACCM = ~0UL;
  pAdapter->uRxACCM = ~0UL;
  pAdapter->uRxMagic = 0;
  pAdapter->uTxMagic = PlatformTime();
  pAdapter->uMtu = pAdapter->uMru = kPppMaxFragment;
  }


/*
 * This layer up
 */
static SWS_FASTCALL( void, LcpUp( SAutomaton* p))
  {
  SPppAdapter* const pAdapter = p->pAdapter;

  TRACE( 2, ("LcpUp(%p)\n",p));
  ASSERT( NULL != p);
  ASSERT( IS_PPP_ADAPTER( pAdapter));

#if OPT_PPP_IDENT /* RFC1570 */
  {
  SWS_BufferH hBuffer;

  /* Get ident packet */
  hBuffer = LcpIdent( p, "MSRASV5.00");
  if ( NULL != hBuffer)
    {
    (void)AutomatonSend( p, hBuffer);
    SWS_BufferRelease( hBuffer);
    }

  hBuffer = LcpIdent( p, "MSRAS-0-SOFTSYS");
  if ( NULL != hBuffer)
    {
    (void)AutomatonSend( p, hBuffer);
    SWS_BufferRelease( hBuffer);
    }
  }
#endif

#if OPT_PPP_CCP /* TODO */
  CcpCfg( pAdapter);
#endif
#if OPT_PPP_AUTH
  if ( pAdapter->automAuth.protocol)
    AutomatonEvent( &pAdapter->automAuth, kEventUp, NULL);
  else
#endif
    AutomatonEvent( &pAdapter->automIpcp, kEventUp, NULL);
  }


#if OPT_PPP_IDENT /* RFC1570 */
/*
 * Construct an LCP ident packet
 */
static SWS_FASTCALL( SWS_BufferH, LcpIdent( SAutomaton* p, const char* psz))
  {
  SPppAdapter* const pAdapter = p->pAdapter;
  SWS_BufferH hBuffer;
  size_t const len = strlen( psz);

  TRACE( 9, ("LcpIdent(%p,%s)\n",p,psz));
  ASSERT( IS_PPP_ADAPTER( pAdapter));

  hBuffer = SWS_BufferGet( SWS_PPP_HDR_DATA + 4 + len, SWS_kBufferDefault);
  if ( NULL != hBuffer)
    {
    SWS_PTR( void) pvHdr;
    SWS_size_t uLen;

    /* Fill in header */
    pvHdr = SWS_BufferGetFragment( hBuffer, 0, &uLen);
    ASSERT( NULL != pvHdr);
    SWS_POKE8( pvHdr, SWS_PPP_HDR_CODE, SWS_PPP_IDENT);
    SWS_POKE8( pvHdr, SWS_PPP_HDR_ID, ++p->uCfgId);
    uLen = SWS_BufferSize( hBuffer);
    SWS_POKE16( pvHdr, SWS_PPP_HDR_LEN, (ui16)uLen);

    /* Ident body */
    SWS_POKE32( pvHdr, SWS_PPP_HDR_DATA, pAdapter->uTxMagic);
    SWS_BufferCopyFromMemory(
      hBuffer, SWS_PPP_HDR_DATA + 4, psz, len);
    }

  return hBuffer;
  }
#endif


/*
 * This layer down
 */
static SWS_FASTCALL( void, LcpDown( SAutomaton* p))
  {
  SPppAdapter* const pAdapter = p->pAdapter;

  TRACE( 2, ("LcpDown(%p)\n",p));
  ASSERT( NULL != p);
  ASSERT( IS_PPP_ADAPTER( pAdapter));

#if OPT_PPP_AUTH
  if ( pAdapter->automAuth.eState > kStateStarting)
    AutomatonEvent( &pAdapter->automAuth, kEventDown, NULL);
  else
#endif
  if ( pAdapter->automIpcp.eState > kStateStarting)
    AutomatonEvent( &pAdapter->automIpcp, kEventDown, NULL);
  }


/*
 * This layer started
 */
static SWS_FASTCALL( void, LcpStarted( SAutomaton* p))
  {
  SPppAdapter* const pAdapter = p->pAdapter;

  TRACE( 2, ("LcpStarted(%p)\n",p));
  ASSERT( NULL != p);
  ASSERT( IS_PPP_ADAPTER( pAdapter));

  if ( !pAdapter->iRxEnable)
    {
    pAdapter->iRxEnable = 1;
    /* Post an Rx buffer */
    pAdapter->rxRead = pAdapter->rxWrite = 0;
    (void) PppReceive( pAdapter);
    }
  }


/*
 * This layer finished
 */
static SWS_FASTCALL( void, LcpFinished( SAutomaton* p))
  {
  SPppAdapter* const pAdapter = p->pAdapter;

  TRACE( 2, ("LcpFinished(%p)\n",p));
  ASSERT( NULL != p);
  ASSERT( IS_PPP_ADAPTER( pAdapter));

  /* Cancel pending Rx */
  pAdapter->iRxEnable = 0;
  (*pAdapter->dn.pfnRecv)( pAdapter->dn.pvContext, NULL, NULL, -1, 0);
  pAdapter->iRxPending = 0;

  LcpReset( p);

  /* Request DPC to tell P2P that adapter can be closed */
  if  (PlatformDpc( &LcpFinishedDpc, pAdapter, NULL, 0))
    {
    TRACE( 1, ("*** LcpFinished(%p) Out of DPCs\n", p));
    LcpFinishedDpc( pAdapter, NULL, 0);
    }
  }

static SWS_CB( void, LcpFinishedDpc(
  SWS_PTR( void) pvContext,           /* IN: Client context */
  SWS_PTR( void) pvData,              /* IN: */
  ui32 uLen                           /* IN: */
)){
  SPppAdapter* const pAdapter = (SPppAdapter*)pvContext;
  (void)pvData;
  (void)uLen;

  TRACE( 5, ("LcpFinishedDpc(%p)\n",pvContext));

  if ( IS_PPP_ADAPTER( pAdapter))
    SWS_P2pLinkFinished( pAdapter->uP2pAdapter);
  /* NB pAdapter may be invalid now */
  }


/*
 * Add LCP configuration options
 */
static SWS_FASTCALL( void, LcpCfg( SAutomaton* pAutomaton, SWS_BufferH hBuffer))
  {
  SPppAdapter* const pAdapter = pAutomaton->pAdapter;
  char buf[ 2* (SWS_PPP_OPT_DATA +4)
    + 2* SWS_PPP_OPT_DATA
    + (SWS_PPP_OPT_DATA +3)], *p = buf;

  TRACE( 9, ("LcpCfg(%p,%p)\n",pAutomaton,hBuffer));
  ASSERT( NULL != pAutomaton);
  ASSERT( IS_PPP_ADAPTER( pAdapter));

  if ( ~0UL == pAdapter->uRxACCM)
    {
    SWS_POKE8( p, SWS_PPP_OPT_TYPE, SWS_PPP_LCP_ACCM);
    SWS_POKE8( p, SWS_PPP_OPT_LEN, SWS_PPP_OPT_DATA + 4);
    SWS_POKE32( p, SWS_PPP_OPT_DATA, 0); /* Desired ACCM */
    p += SWS_PPP_OPT_DATA + 4;
    }

  if ( pAdapter->uTxMagic)
    {
    SWS_POKE8( p, SWS_PPP_OPT_TYPE, SWS_PPP_LCP_MAGIC);
    SWS_POKE8( p, SWS_PPP_OPT_LEN, SWS_PPP_OPT_DATA + 4);
    SWS_POKE32( p, SWS_PPP_OPT_DATA, pAdapter->uTxMagic);
    p += SWS_PPP_OPT_DATA + 4;
    }

  if ( -1 == pAdapter->isPFC)
    {
    SWS_POKE8( p, SWS_PPP_OPT_TYPE, SWS_PPP_LCP_PFC);
    SWS_POKE8( p, SWS_PPP_OPT_LEN, SWS_PPP_OPT_DATA);
    p += SWS_PPP_OPT_DATA;
    }

  if ( -1 == pAdapter->isACFC)
    {
    SWS_POKE8( p, SWS_PPP_OPT_TYPE, SWS_PPP_LCP_ACFC);
    SWS_POKE8( p, SWS_PPP_OPT_LEN, SWS_PPP_OPT_DATA);
    p += SWS_PPP_OPT_DATA;
    }

#if OPT_PPP_AUTH
  /* Peer authentication required? */
  switch ( pAdapter->uRxAuth)
    {
  case 0:
    break;

  case SWS_PPP_CHAP:
    SWS_POKE8( p, SWS_PPP_OPT_TYPE, SWS_PPP_LCP_AUTH);
    SWS_POKE8( p, SWS_PPP_OPT_LEN, SWS_PPP_OPT_DATA + 3);
    SWS_POKE16( p, SWS_PPP_OPT_DATA, SWS_PPP_CHAP);
    SWS_POKE8( p, SWS_PPP_OPT_DATA + 2, 5); /* MD5 */
    p += SWS_PPP_OPT_DATA + 3;
    break;

  case SWS_PPP_PAP:
    SWS_POKE8( p, SWS_PPP_OPT_TYPE, SWS_PPP_LCP_AUTH);
    SWS_POKE8( p, SWS_PPP_OPT_LEN, SWS_PPP_OPT_DATA + 2);
    SWS_POKE16( p, SWS_PPP_OPT_DATA, SWS_PPP_PAP);
    p += SWS_PPP_OPT_DATA + 2;
    break;
    }
#endif

  ASSERT( (size_t)(p - buf) <= sizeof(buf));
  if ( p > buf)
    {
    SWS_size_t const uSize = (SWS_size_t)(p - buf);
    SWS_size_t const uHdr = SWS_BufferSize( hBuffer);

    if ( !SWS_BufferAppendBack( hBuffer, uSize))
      SWS_BufferCopyFromMemory( hBuffer, uHdr, buf, uSize);
    }
  }


/*
 * Parse LCP CONFIGURE_ACK/Nak/Rej
 */
static void LcpRsp(
  SAutomaton* p,
  unsigned code,
  unsigned type,
  unsigned len,
  SWS_PTR( const void) pvOpt
) {
  SPppAdapter* const pAdapter = p->pAdapter;
  ui32 magic;

  TRACE( 9, ("LcpRsp(%p,%u,%u,%u,%p)\n",p,code,type,len,pvOpt));
  ASSERT( NULL != p);
  ASSERT( IS_PPP_ADAPTER( pAdapter));
  ASSERT( NULL != pvOpt);

  switch ( type)
    {
  case SWS_PPP_LCP_ACCM:
    switch ( code)
      {
    case SWS_PPP_CONFIGURE_REJ:
      AutomatonEvent( p, kEventRXJ_M, NULL);
      break;

    case SWS_PPP_CONFIGURE_ACK:
    case SWS_PPP_CONFIGURE_NAK:
      if ( len >= SWS_PPP_OPT_DATA + 4)
        {
        SWS_PEEK32( magic, pvOpt, SWS_PPP_OPT_DATA);
        pAdapter->uRxACCM = magic;
        }
      break;
      }
    break;

  case SWS_PPP_LCP_MAGIC:
    switch ( code)
      {
    case SWS_PPP_CONFIGURE_REJ:
      pAdapter->uTxMagic = 0;
      break;

    case SWS_PPP_CONFIGURE_ACK:
      break;

    case SWS_PPP_CONFIGURE_NAK:
      if ( len >= SWS_PPP_OPT_DATA + 4)
        {
        SWS_PEEK32( magic, pvOpt, SWS_PPP_OPT_DATA);
        /* Possible loopback? */
        if ( pAdapter->uRxMagic == magic)
          {
          TRACE( 1, ("*** LcpRsp() ?loopback\n"));
          pAdapter->uTxMagic = PlatformTime(); /* Yes */
          }
        }
      break;
      }
    break;

  case SWS_PPP_LCP_PFC:
    pAdapter->isPFC = SWS_PPP_CONFIGURE_ACK == code;
    break;

  case SWS_PPP_LCP_ACFC:
    pAdapter->isACFC = SWS_PPP_CONFIGURE_ACK == code;
    break;

#if OPT_PPP_AUTH
  case SWS_PPP_LCP_AUTH:
    switch ( code)
      {
    case SWS_PPP_CONFIGURE_REJ:
      /* Auth is required but rejected - fatal */
      AutomatonEvent( p, kEventRXJ_M, NULL);
      break;

    case SWS_PPP_CONFIGURE_ACK:
      pAdapter->automAuth.protocol = pAdapter->uRxAuth;
      pAdapter->uRxAuthState = pAdapter->uRxAuth;
      if ( SWS_PPP_CHAP == pAdapter->uRxAuth)
        pAdapter->automAuth.pfnConfigAdd = &AuthCfg;
      break;

    case SWS_PPP_CONFIGURE_NAK:
      if ( len >= SWS_PPP_OPT_DATA + 2)
        {
        ui16 val;

        SWS_PEEK16( val, pvOpt, SWS_PPP_OPT_DATA);
        switch ( val)
          {
        case SWS_PPP_CHAP:
          if ( len >= SWS_PPP_OPT_DATA + 3)
            {
            SWS_PEEK8( val, pvOpt, SWS_PPP_OPT_DATA + 2);
            switch ( val)
              {
            case 5: /* MD5 */
              pAdapter->uRxAuth = SWS_PPP_CHAP;
              break;

            default:
              pAdapter->uRxAuth = (ui16)(SWS_PPP_CHAP == pAdapter->uRxAuth ? SWS_PPP_PAP : SWS_PPP_CHAP);
              break;
              }
            }
          break;

        case SWS_PPP_PAP:
          pAdapter->uRxAuth = SWS_PPP_PAP;
          break;
          }
        }
      break;
      }
    break;
#endif

  default:
    TRACE( 1, ("*** LcpRsp() Unexpected type %u code %u \n", type, code));
    break;
    }
  }


/*
 * Verify LCP CONFIGURE_REQ options
 */
static int LcpReq(
  SAutomaton* p,
  unsigned type,
  unsigned len,
  SWS_PTR( void) pv
) {
  SPppAdapter* const pAdapter = p->pAdapter;
  int action;
  ui16 data;
  ui32 magic;

  TRACE( 9, ("LcpReq(%p,%u,%u,%p)\n",p,type,len,pv));
  ASSERT( NULL != p);
  ASSERT( NULL != pv);
  ASSERT( IS_PPP_ADAPTER( pAdapter));
  (void)pAdapter;

  /* Assume type is OK */
  action = SWS_PPP_CONFIGURE_ACK;

  switch ( type)
    {
  default:
    /* Reject unrecognised options */
    TRACE( 1, ("*** LcpReq() unknown %u\n", type));
    action = SWS_PPP_CONFIGURE_REJ;
    break;

  case SWS_PPP_LCP_MRU:
    if ( len >= 2 + SWS_PPP_OPT_DATA)
      {
      SWS_PEEK16( data, pv, SWS_PPP_OPT_DATA);
      if ( data < kPppMaxFragment)
        {
        SWS_POKE16( pv, SWS_PPP_OPT_DATA, kPppMaxFragment);
        action = SWS_PPP_CONFIGURE_NAK;
        }
      }
    else
      action = SWS_PPP_CONFIGURE_REJ;
    break;

  case SWS_PPP_LCP_ACCM:
    if ( len >= 4 + SWS_PPP_OPT_DATA)
      {
      SWS_PEEK32( magic, pv, SWS_PPP_OPT_DATA);
      p->pAdapter->uTxACCM = magic;
      }
    else
      action = SWS_PPP_CONFIGURE_REJ;
    break;

#if OPT_PPP_AUTH
  case SWS_PPP_LCP_AUTH:
    if ( len >= 2 + SWS_PPP_OPT_DATA)
      {
      SWS_PEEK16( data, pv, SWS_PPP_OPT_DATA);
      switch ( data)
        {
      case SWS_PPP_PAP:
        pAdapter->automAuth.protocol = SWS_PPP_PAP;
        pAdapter->uTxAuthState = SWS_PPP_PAP;
        pAdapter->automAuth.pfnConfigAdd = &AuthCfg;
        break;

      case SWS_PPP_CHAP:
        /* Get algorithm */
        SWS_PEEK8( data, pv, SWS_PPP_OPT_DATA + 2);
        if ( 5 == data
#if OPT_PPP_MSCHAP
          || 128 == data /* MS-CHAP RFC 2433 */
#endif
#if OPT_PPP_MSCHAP2
          || 129 == data /* MS-CHAP2 RFC 2759 */
#endif
        ) {
          pAdapter->uChapAlgo = (ui8)data;
          pAdapter->automAuth.protocol = SWS_PPP_CHAP;
          pAdapter->uTxAuthState = SWS_PPP_CHAP;
          break;
          }

        /* Want MD5 */
        SWS_POKE8( pv, SWS_PPP_OPT_DATA + 2, 5);
        /* Fall thru */
      default:
        /* Want CHAP */
        SWS_POKE16( pv, SWS_PPP_OPT_DATA, SWS_PPP_CHAP);
        action = SWS_PPP_CONFIGURE_NAK;
        break;
        }
      }
    else
      action = SWS_PPP_CONFIGURE_REJ;
    break;
#endif

  case SWS_PPP_LCP_MAGIC:
    /* Loopback? */
    if ( len >= 4 + SWS_PPP_OPT_DATA)
      {
      SWS_PEEK32( magic, pv, SWS_PPP_OPT_DATA);
      if ( p->pAdapter->uTxMagic != magic)
        {
        /* No */
        p->pAdapter->uRxMagic = magic;
        }
      else
        {
        /* Possible loopback */
        TRACE( 1, ("*** LcpReq() ?loopback\n"));
        p->pAdapter->uRxMagic = PlatformTime();
        SWS_POKE32( pv, SWS_PPP_OPT_DATA, p->pAdapter->uRxMagic);
        action = SWS_PPP_CONFIGURE_NAK;
        }
      }
    else
      action = SWS_PPP_CONFIGURE_REJ;
    break;

  case SWS_PPP_LCP_PFC:
  case SWS_PPP_LCP_ACFC:
    break;

#if 0 /* Multilink options */
  case SWS_PPP_LCP_ML_MRRU: /* RFC1717 */
  case SWS_PPP_LCP_ML_SSNH: /* RFC1717 */
  case SWS_PPP_LCP_ML_EPD: /* RFC1717 */
  case SWS_PPP_LCP_BACP: /* Link Discriminator for BACP RFC 2125 */
    break;
#endif
    }

  return action;
  }


/*****************************************************************************
 * IPCP automaton
 ****************************************************************************/
/*
 * Reset IPCP state
 */
static SWS_FASTCALL( void, IpcpReset( SAutomaton* pAutomaton))
  {
  SPppAdapter* const pAdapter = pAutomaton->pAdapter;

  TRACE( 9, ("IpcpReset(%p)\n",pAutomaton));
  ASSERT( NULL != pAutomaton);
  ASSERT( IS_PPP_ADAPTER( pAdapter));

  if ( pAutomaton->eState <= kStateStopping)
    AutomatonReset( pAutomaton);

#if OPT_DNS
  pAdapter->uPriDns = SWS_INADDR_ANY;
  pAdapter->uSecDns = SWS_INADDR_ANY;
#endif
  pAdapter->uIpAddress = SWS_P2pGetIpAddress( pAdapter->uP2pAdapter);
  pAdapter->uGatewayAddr = SWS_P2pGetGateway( pAdapter->uP2pAdapter);

#if OPT_P2P_VJCOMP
  if ( NULL != pAdapter->hTxVjComp)
    {
    SWS_P2pVjClose( pAdapter->hTxVjComp);
    pAdapter->hTxVjComp = NULL;
    }

  if ( NULL != pAdapter->hRxVjComp)
    {
    SWS_P2pVjClose( pAdapter->hRxVjComp);
    pAdapter->hRxVjComp = NULL;
    }

  pAdapter->uRxCompSlotId = 1;
  pAdapter->uRxMaxSlotId = SWS_P2pVjSlots( pAdapter->uP2pAdapter);
#endif
  }


/*
 * This layer up
 */
static SWS_FASTCALL( void, IpcpUp( SAutomaton* p))
  {
  SPppAdapter* const pAdapter = p->pAdapter;

  TRACE( 2, ("IpcpUp(%p)\n",p));
  ASSERT( NULL != p);
  ASSERT( IS_PPP_ADAPTER( pAdapter));

  /* Notify the datalink that we're up */
  pAdapter->isUp = 1;
  (*pAdapter->up.pfnLinkStatus)( pAdapter->up.pvContext, 1);

#if OPT_DNS && OPT_IP_ROUTES
  /* Add an explicit route to each DNS server */
  if ( SWS_INADDR_ANY != pAdapter->uPriDns
    && SWS_INADDR_NONE != pAdapter->uPriDns
  ) {
    SWS_DnsSetPriServer( pAdapter->uPriDns);
    SWS_RouteAdd(
      pAdapter->uIpAddress, pAdapter->uPriDns, ~0UL, pAdapter->uPriDns);
    }

  if ( SWS_INADDR_ANY != pAdapter->uSecDns
    && SWS_INADDR_NONE != pAdapter->uSecDns
  ) {
    SWS_DnsSetSecServer( pAdapter->uSecDns);
    SWS_RouteAdd(
      pAdapter->uIpAddress, pAdapter->uSecDns, ~0UL, pAdapter->uSecDns);
    }
#endif
  }


/*
 * This layer down
 */
static SWS_FASTCALL( void, IpcpDown( SAutomaton* p))
  {
  SPppAdapter* const pAdapter = p->pAdapter;

  TRACE( 2, ("IpcpDown(%p)\n",p));
  ASSERT( NULL != p);
  ASSERT( IS_PPP_ADAPTER( pAdapter));

  /* Notify the datalink that we're down */
  (*pAdapter->up.pfnLinkStatus)( pAdapter->up.pvContext, 0);
  pAdapter->isUp = 0;

  IpcpReset( p);
  }


/*
 * This layer started
 */
static SWS_FASTCALL( void, IpcpStarted( SAutomaton* p))
  {
  SPppAdapter* const pAdapter = p->pAdapter;

  TRACE( 2, ("IpcpStarted(%p)\n",p));
  ASSERT( NULL != p);
  ASSERT( IS_PPP_ADAPTER( pAdapter));
  (void)pAdapter;

  /* Open all lower protocols, except LCP */
#if OPT_PPP_AUTH
  AutomatonEvent( &pAdapter->automAuth, kEventOpen, NULL);
#endif
#if OPT_PPP_CCP
  {int TODO_OpenCCP;}
#endif
  }


/*
 * This layer finished
 */
static SWS_FASTCALL( void, IpcpFinished( SAutomaton* p))
  {
  SPppAdapter* const pAdapter = p->pAdapter;

  TRACE( 2, ("IpcpFinished(%p)\n",p));
  ASSERT( NULL != p);
  ASSERT( IS_PPP_ADAPTER( pAdapter));
  (void)pAdapter;

  IpcpReset( p);

  /* Close all lower protocols, except LCP */
#if OPT_PPP_AUTH
  AutomatonEvent( &pAdapter->automAuth, kEventClose, NULL);
#endif
#if OPT_PPP_CCP
  {int TODO_CloseCCP;}
#endif
  }


/*
 * IPCP configuration-request options
 */
static SWS_FASTCALL( void, IpcpCfg( SAutomaton* pAutomaton, SWS_BufferH hBuffer))
  {
  SPppAdapter* const pAdapter = pAutomaton->pAdapter;
  char buf[ 4 * (SWS_PPP_OPT_DATA +4)], *p = buf;

  TRACE( 9, ("IpcpCfg(%p,%p)\n",pAutomaton,hBuffer));
  ASSERT( NULL != pAutomaton);
  ASSERT( IS_PPP_ADAPTER( pAdapter));

#if OPT_P2P_VJCOMP
  if ( pAdapter->uRxMaxSlotId > 0
    && NULL == pAdapter->hRxVjComp
  ) {
    pAdapter->hRxVjComp = SWS_P2pVjOpen(
      pAdapter->uRxMaxSlotId, pAdapter->uRxCompSlotId);
    }

  if ( NULL != pAdapter->hRxVjComp)
    {
    SWS_POKE8( p, SWS_PPP_OPT_TYPE, SWS_PPP_IPCP_IPCOMP);
    SWS_POKE8( p, SWS_PPP_OPT_LEN, SWS_PPP_OPT_DATA + 4);
    SWS_POKE16( p, SWS_PPP_OPT_DATA, SWS_PPP_VJ);
    SWS_POKE8( p, SWS_PPP_OPT_DATA + 2, pAdapter->uRxMaxSlotId);
    SWS_POKE8( p, SWS_PPP_OPT_DATA + 3, pAdapter->uRxCompSlotId);
    p += SWS_PPP_OPT_DATA + 4;
    }
#endif

  if ( SWS_INADDR_NONE != pAdapter->uIpAddress)
    {
    SWS_POKE8( p, SWS_PPP_OPT_TYPE, SWS_PPP_IPCP_IPADDR);
    SWS_POKE8( p, SWS_PPP_OPT_LEN, SWS_PPP_OPT_DATA + 4);
    SWS_POKE32( p, SWS_PPP_OPT_DATA, pAdapter->uIpAddress);
    p += SWS_PPP_OPT_DATA + 4;
    }

#if OPT_DNS
  if ( SWS_INADDR_NONE != pAdapter->uPriDns)
    {
    SWS_POKE8( p, SWS_PPP_OPT_TYPE, SWS_PPP_IPCP_PRIDNS);
    SWS_POKE8( p, SWS_PPP_OPT_LEN, SWS_PPP_OPT_DATA + 4);
    SWS_POKE32( p, SWS_PPP_OPT_DATA, pAdapter->uPriDns);
    p += SWS_PPP_OPT_DATA + 4;
    }

  if ( SWS_INADDR_NONE != pAdapter->uSecDns)
    {
    SWS_POKE8( p, SWS_PPP_OPT_TYPE, SWS_PPP_IPCP_SECDNS);
    SWS_POKE8( p, SWS_PPP_OPT_LEN, SWS_PPP_OPT_DATA + 4);
    SWS_POKE32( p, SWS_PPP_OPT_DATA, pAdapter->uSecDns);
    p += SWS_PPP_OPT_DATA + 4;
    }
#endif

  ASSERT( (size_t)(p - buf) <= sizeof(buf));
  if ( p > buf)
    {
    SWS_size_t const uSize = (SWS_size_t)(p - buf);
    SWS_size_t const uHdr = SWS_BufferSize( hBuffer);

    if ( !SWS_BufferAppendBack( hBuffer, uSize))
      SWS_BufferCopyFromMemory( hBuffer, uHdr, buf, uSize);
    }
  }


/*
 * Parse IPCP CONFIGURE_ACK/Nak/Rej options
 */
static void IpcpRsp(
  SAutomaton* p,
  unsigned code,
  unsigned type,
  unsigned len,
  SWS_PTR( const void) pvOpt
) {
  SPppAdapter* const pAdapter = p->pAdapter;
  ui32 magic;

  TRACE( 9, ("IpcpRsp(%p,%u,%u,%u,%p)\n",p,code,type,len,pvOpt));
  ASSERT( NULL != p);
  ASSERT( IS_PPP_ADAPTER( pAdapter));
  ASSERT( NULL != pvOpt);

  switch ( type)
    {
#if OPT_P2P_VJCOMP
  case SWS_PPP_IPCP_IPCOMP:
    switch ( code)
      {
    case SWS_PPP_CONFIGURE_ACK:
      break;

    case SWS_PPP_CONFIGURE_NAK:
      if ( NULL != pAdapter->hRxVjComp)
        {
        SWS_P2pVjClose( pAdapter->hRxVjComp);
        pAdapter->hRxVjComp = NULL;
        }

      if ( len >= SWS_PPP_OPT_DATA + 2)
        {
        SWS_PEEK16( magic, pvOpt, SWS_PPP_OPT_DATA);

        if ( SWS_PPP_VJ == magic
          && len >= SWS_PPP_OPT_DATA + 4
        ) {
          SWS_PEEK8( pAdapter->uRxMaxSlotId, pvOpt, SWS_PPP_OPT_DATA +2);
          SWS_PEEK8( pAdapter->uRxCompSlotId, pvOpt, SWS_PPP_OPT_DATA +3);
          }
        else
          pAdapter->uRxMaxSlotId = 0;
        }
      break;

    case SWS_PPP_CONFIGURE_REJ:
      if ( NULL != pAdapter->hRxVjComp)
        {
        SWS_P2pVjClose( pAdapter->hRxVjComp);
        pAdapter->hRxVjComp = NULL;
        }

      pAdapter->uRxMaxSlotId = 0;
      break;
      }
    break;
#endif

  case SWS_PPP_IPCP_IPADDR:
    switch ( code)
      {
    case SWS_PPP_CONFIGURE_ACK:
      break;

    case SWS_PPP_CONFIGURE_NAK:
      if ( len >= SWS_PPP_OPT_DATA + 4)
        {
        SWS_PEEK32( magic, pvOpt, SWS_PPP_OPT_DATA);
        pAdapter->uIpAddress = magic;
        }
      break;

    case SWS_PPP_CONFIGURE_REJ:
      pAdapter->uIpAddress = SWS_INADDR_NONE;
      break;
      }
    break;

#if OPT_DNS
  case SWS_PPP_IPCP_PRIDNS:
    switch ( code)
      {
    case SWS_PPP_CONFIGURE_ACK:
      break;

    case SWS_PPP_CONFIGURE_NAK:
      if ( len >= SWS_PPP_OPT_DATA + 4)
        {
        SWS_PEEK32( magic, pvOpt, SWS_PPP_OPT_DATA);
        pAdapter->uPriDns = magic;
        }
      break;

    case SWS_PPP_CONFIGURE_REJ:
      pAdapter->uPriDns = SWS_INADDR_NONE;
      break;
      }
    break;

  case SWS_PPP_IPCP_SECDNS:
    switch ( code)
      {
    case SWS_PPP_CONFIGURE_ACK:
      break;

    case SWS_PPP_CONFIGURE_NAK:
      if ( len >= SWS_PPP_OPT_DATA + 4)
        {
        SWS_PEEK32( magic, pvOpt, SWS_PPP_OPT_DATA);
        pAdapter->uSecDns = magic;
        }
      break;

    case SWS_PPP_CONFIGURE_REJ:
      pAdapter->uSecDns = SWS_INADDR_NONE;
      break;
      }
    break;
#endif

  default:
    TRACE( 1, ("*** IpcpRsp() unknown %u\n", type));
    break;
    }
  }


/*
 * Verify IPCP ConfigureReq options
 */
static int IpcpReq(
  SAutomaton* p,
  unsigned type,
  unsigned len,
  SWS_PTR( void) pv
) {
  SPppAdapter* const pAdapter = p->pAdapter;
  int action;
  ui32 magic;

  TRACE( 9, ( "IpcpReq(%p,%u,%u,%p)\n",p,type,len,pv));
  ASSERT( NULL != p);
  ASSERT( NULL != pv);
  ASSERT( IS_PPP_ADAPTER( pAdapter));

  /* Default to ack'ing the option */
  action = SWS_PPP_CONFIGURE_ACK;

  switch ( type)
    {
  default:
    /* Reject unrecognised options */
    TRACE( 1, ("*** IpcpReq() type %u rej\n", type));
    action = SWS_PPP_CONFIGURE_REJ;
    break;

  case SWS_PPP_IPCP_IPCOMP:
#if OPT_P2P_VJCOMP
    if ( len >= 2 + SWS_PPP_OPT_DATA)
      {
      ui16 data;

      if ( NULL != pAdapter->hTxVjComp)
        {
        SWS_P2pVjClose( pAdapter->hTxVjComp);
        pAdapter->hTxVjComp = NULL;
        }

      SWS_PEEK16( data, pv, SWS_PPP_OPT_DATA);
      switch ( data)
        {
      case SWS_PPP_VJ: /* Van Jacobson Compressed TCP/IP */
        if ( len >= 4 + SWS_PPP_OPT_DATA)
          {
          ui8 uMaxSlotId, uCompSlotId, uSlots;

          SWS_PEEK8( uMaxSlotId, pv, SWS_PPP_OPT_DATA + 2);
          SWS_PEEK8( uCompSlotId, pv, SWS_PPP_OPT_DATA + 3);

          uSlots = SWS_P2pVjSlots( pAdapter->uP2pAdapter);
          if ( uSlots > 0)
            {
            if ( uSlots < uMaxSlotId)
              uMaxSlotId = uSlots;

            /* Open a VJ compressor */
            pAdapter->hTxVjComp = SWS_P2pVjOpen( uMaxSlotId, uCompSlotId);
            }
          }
        else
          action = SWS_PPP_CONFIGURE_REJ;
        break;

      default:
        TRACE( 2, ("IpcpReq() IPCOMP type %#x\n", data));
        SWS_POKE16( pv, SWS_PPP_OPT_DATA, SWS_PPP_VJ);
        action = SWS_PPP_CONFIGURE_NAK;
        break;
        }
      }
    else
      action = SWS_PPP_CONFIGURE_REJ;
#endif
    break;

  case SWS_PPP_IPCP_IPADDR:
    if ( len >= 4 + SWS_PPP_OPT_DATA)
      {
      SWS_PEEK32( magic, pv, SWS_PPP_OPT_DATA);
      if ( SWS_INADDR_NONE != magic
        && SWS_INADDR_ANY != magic
      ) {
        pAdapter->uGatewayAddr = magic;
        }
      else if ( SWS_INADDR_NONE != pAdapter->uGatewayAddr
        && SWS_INADDR_ANY != pAdapter->uGatewayAddr
      ) {
        SWS_POKE32( pv, SWS_PPP_OPT_DATA, pAdapter->uGatewayAddr);
        action = SWS_PPP_CONFIGURE_NAK;
        }
      else
        {
        TRACE( 1, ("*** IpcpReq() IPADDR but gateway unset\n"));
        action = SWS_PPP_CONFIGURE_REJ;
        }
      }
    else
      action = SWS_PPP_CONFIGURE_REJ;
    break;

#if OPT_DNS
  case SWS_PPP_IPCP_PRIDNS:
    if ( len >= 4 + SWS_PPP_OPT_DATA)
      {
      SWS_PEEK32( magic, pv, SWS_PPP_OPT_DATA);
      if ( SWS_INADDR_NONE == magic
        || SWS_INADDR_ANY == magic
      ) {
        magic = SWS_DnsGetPriServer();
        if ( SWS_INADDR_ANY != magic)
          {
          SWS_POKE32( pv, SWS_PPP_OPT_DATA, magic);
          action = SWS_PPP_CONFIGURE_NAK;
          }
        else
          action = SWS_PPP_CONFIGURE_REJ;
        }
      }
    else
      action = SWS_PPP_CONFIGURE_REJ;
    break;

  case SWS_PPP_IPCP_SECDNS:
    if ( len >= 4 + SWS_PPP_OPT_DATA)
      {
      SWS_PEEK32( magic, pv, SWS_PPP_OPT_DATA);
      if ( SWS_INADDR_NONE == magic
        || SWS_INADDR_ANY == magic
      ) {
        magic = SWS_DnsGetSecServer();
        if ( SWS_INADDR_ANY != magic)
          {
          SWS_POKE32( pv, SWS_PPP_OPT_DATA, magic);
          action = SWS_PPP_CONFIGURE_NAK;
          }
        else
          action = SWS_PPP_CONFIGURE_REJ;
        }
      }
    else
      action = SWS_PPP_CONFIGURE_REJ;
    break;
#endif
    }

  return action;
  }


/*****************************************************************************
 * Automata
 ****************************************************************************/
/*
 * Init
 */
static int AutomatonCtor( SAutomaton* p, ui16 protocol, SPppAdapter* pAdapter)
  {
  TRACE( 9, ("AutomatonCtor(%p,%#x,%p)\n",p,protocol,pAdapter));
  ASSERT( NULL != p);
  ASSERT( p->pfnReset);

  p->pAdapter = pAdapter;
  p->protocol = protocol;
  p->eState = kStateInitial;
  p->iRestartCnt = 0;

  p->hRestartTimer = SWS_TimerAlloc( &AutomatonTimeout, p);
  if ( NULL == p->hRestartTimer)
    return SWS_ENOMEM;

  (*p->pfnReset)( p);

  return 0;
  }


/*
 * Final
 */
static SWS_FASTCALL( void, AutomatonDtor( SAutomaton* p))
  {
  TRACE( 9, ("AutomatonDtor(%p)\n",p));

  if ( NULL != p->hRestartTimer)
    {
    SWS_TimerRelease( p->hRestartTimer);
    p->hRestartTimer = NULL;
    }
  }


/*
 * Reset
 */
static SWS_FASTCALL( void, AutomatonReset( SAutomaton* p))
  {
  TRACE( 9, ("AutomatonReset(%p)\n",p));
  ASSERT( NULL != p);

  p->iCfgFailures = 0;
  p->uCfgId = 0;
  p->uTermId = 0;
  p->uRejId = 0;
  }


/*
 * Timer expiry callback
 */
static SWS_CB( void, AutomatonTimeout(
  SWS_TimerH hTimer,                  /* IN: Timer that expired */
  SWS_PTR( void) pv                   /* IN: User context */
)){
  SAutomaton* const p = (SAutomaton*)pv;
  (void)hTimer;

  TRACE( 3, ("AutomatonTimeout(%p)\n",pv));
  ASSERT( NULL != p);
  ASSERT( PlatformIsCritical());

  if ( IS_PPP_ADAPTER( p->pAdapter))
    AutomatonEvent( p, p->iRestartCnt > 0 ? kEventTO_P : kEventTO_M, NULL);
  }


/*
 * Receive a packet
 */
static SWS_FASTCALL( void, AutomatonRx( SAutomaton* p, SWS_BufferH hBuffer))
  {
  SWS_PTR( void) pvHdr;
  SWS_size_t uLen;

  TRACE( 9, ("AutomatonRx(%p,%p)\n",p,hBuffer));
  ASSERT( NULL != p);

  switch ( p->eState)
    {
  case kStateInitial:
  case kStateStarting:
    TRACE( 1, ("***AutomatonRx(%#x) unexpected packet\n",p->protocol));
    return;

  default:
    break;
    }

  pvHdr = SWS_BufferGetFragment( hBuffer, 0, &uLen);
  if ( NULL != pvHdr
    && uLen >= SWS_PPP_HDR_DATA
  ) {
    ui8 code, id;
    ui16 len;

    SWS_PEEK8( code, pvHdr, SWS_PPP_HDR_CODE);
    SWS_PEEK8( id, pvHdr, SWS_PPP_HDR_ID);
    SWS_PEEK16( len, pvHdr, SWS_PPP_HDR_LEN);

    switch ( code)
      {
    case SWS_PPP_CONFIGURE_REQ:
      if ( SWS_PPP_CONFIGURE_ACK == AutomatonCfgReq( p, hBuffer, len) )
        AutomatonEvent( p, kEventRCR_P, hBuffer);
      else
        AutomatonEvent( p, kEventRCR_M, hBuffer);
      break;

    case SWS_PPP_CONFIGURE_ACK:
      if ( id == p->uCfgId)
        {
        AutomatonCfgRsp( p, code, hBuffer, len);
        AutomatonEvent( p, kEventRCA, hBuffer);
        }
      else
        TRACE( 1, ("*** AutomatonRx(%p) Ack mismatched id %u\n",p,id));
      break;

    case SWS_PPP_CONFIGURE_NAK:
    case SWS_PPP_CONFIGURE_REJ:
      if ( id == p->uCfgId)
        {
        AutomatonCfgRsp( p, code, hBuffer, len);
        AutomatonEvent( p, kEventRCN, hBuffer);
        }
      else
        TRACE( 1, ("*** AutomatonRx(%p) Nak/Rej mismatched id %u\n",p,id));
      break;

    case SWS_PPP_TERMINATE_REQ:
      AutomatonEvent( p, kEventRTR, hBuffer);
      break;

    case SWS_PPP_TERMINATE_ACK:
      if ( id == p->uTermId)
        AutomatonEvent( p, kEventRTA, hBuffer);
      else
        TRACE( 1, ("*** AutomatonRx(%p) TermAck mismatched id %u\n",p,id));
      break;

    case SWS_PPP_CODE_REJ:
      SWS_PEEK8( code, pvHdr, SWS_PPP_HDR_DATA);
      TRACE( 1, ("*** AutomatonRx(%p) code %u rejected\n",p,code));
      if ( code <= SWS_PPP_DISCARD_REQ)
        AutomatonEvent( p, kEventRXJ_M, hBuffer);
      else
        AutomatonEvent( p, kEventRXJ_P, hBuffer);
      break;

    case SWS_PPP_PROTOCOL_REJ:
      if ( SWS_PPP_LCP == p->protocol)
        {
        SWS_PEEK16( len, pvHdr, SWS_PPP_HDR_DATA);
        switch ( len)
          {
        case SWS_PPP_LCP:
          TRACE( 1, ("*** AutomatonRx(%p) LCP rejected\n",p));
          AutomatonEvent( &p->pAdapter->automLcp, kEventRXJ_M, hBuffer);
          break;

        case SWS_PPP_IPCP:
          TRACE( 1, ("*** AutomatonRx(%p) IPCP rejected\n",p));
          AutomatonEvent( &p->pAdapter->automIpcp, kEventRXJ_M, hBuffer);
          break;

        default:
          AutomatonEvent( p, kEventRXJ_P, hBuffer);
          break;
          }
        }
      break;

    case SWS_PPP_ECHO_REQ:
      AutomatonEvent( p, kEventRXR, hBuffer);
      break;

    case SWS_PPP_ECHO_REPLY:
      break;
    case SWS_PPP_DISCARD_REQ:
      break;
    case SWS_PPP_IDENT: /* RFC1570 */
      break;

    default:
      TRACE( 1, ("*** AutomatonRx(%p) code %u\n",p,code));
      AutomatonEvent( p, kEventRUC, hBuffer);
      break;
      }
    }
  }


/*
 * Validate Configure-Request options
 */
static int AutomatonCfgReq(
  SAutomaton* p,
  SWS_BufferH hBuffer,
  unsigned uLength
) {
  SWS_PTR( void) pv;
  SWS_size_t uSize;
  unsigned uOffset, uRspLen;
  ui8 len, action;

  TRACE( 9, ("AutomatonCfgReq(%p,%p,%u)\n",p,hBuffer,uLength));
  ASSERT( NULL != p);
  ASSERT( p->pfnConfigReq);

  uRspLen = uLength;
  action = SWS_PPP_CONFIGURE_ACK;

  /* Scan all options */
  for (
    uOffset = SWS_PPP_HDR_DATA;
    uOffset + SWS_PPP_OPT_DATA <= uLength;
    uOffset += len
  ) {
    ui8 type;

    pv = SWS_BufferGetFragment( hBuffer, uOffset, &uSize);
    if ( NULL == pv || uSize < SWS_PPP_OPT_DATA)
      break;

    SWS_PEEK8( type, pv, SWS_PPP_OPT_TYPE);
    SWS_PEEK8( len, pv, SWS_PPP_OPT_LEN);
    if ( len < SWS_PPP_OPT_DATA)
      break;
    if ( len > uSize)
      continue;

    switch ( (*p->pfnConfigReq)( p, type, len, pv))
      {
    case SWS_PPP_CONFIGURE_ACK:
      /* Option OK */
      break;

    case SWS_PPP_CONFIGURE_NAK:
      /* Option OK but value out of range */
      switch ( action)
        {
      case SWS_PPP_CONFIGURE_ACK:
        action = SWS_PPP_CONFIGURE_NAK;
        uRspLen = SWS_PPP_HDR_DATA;
        /* Fall thru */
      case SWS_PPP_CONFIGURE_NAK:
        SWS_BufferCopyFromMemory( hBuffer, uRspLen, pv, len);
        uRspLen += len;
        break;

      case SWS_PPP_CONFIGURE_REJ:
        break;
        }
      break;

    case SWS_PPP_CONFIGURE_REJ:
      /* Option unknown */
      switch ( action)
        {
      case SWS_PPP_CONFIGURE_ACK:
      case SWS_PPP_CONFIGURE_NAK:
        action = SWS_PPP_CONFIGURE_REJ;
        uRspLen = SWS_PPP_HDR_DATA;
        /* Fall thru */
      case SWS_PPP_CONFIGURE_REJ:
        SWS_BufferCopyFromMemory( hBuffer, uRspLen, pv, len);
        uRspLen += len;
        break;
        }
      break;
      }
    }

  pv = SWS_BufferGetFragment( hBuffer, 0, &uSize);
  ASSERT( NULL != pv);
  ASSERT( uSize >= SWS_PPP_HDR_DATA);

  SWS_POKE8( pv, SWS_PPP_HDR_CODE, action);
  SWS_POKE16( pv, SWS_PPP_HDR_LEN, (ui16)uRspLen);

  uSize = SWS_BufferSize( hBuffer);
  if ( uRspLen < uSize)
    SWS_BufferRemoveBack( hBuffer, uSize - uRspLen);

  return action;
  }


/*
 * Process Configure-Ack/Nak/Rej options
 */
static void AutomatonCfgRsp(
  SAutomaton* p,
  unsigned code,
  SWS_BufferH hBuffer,
  unsigned uLength
) {
  unsigned uOffset;
  ui8 len;

  TRACE( 9, ("AutomatonCfgRsp(%p,%u,%p,%u)\n",p,code,hBuffer,uLength));
  ASSERT( NULL != p);
  ASSERT( p->pfnConfigRsp);

  /* Scan all options */
  for (
    uOffset = SWS_PPP_HDR_DATA;
    uOffset + SWS_PPP_OPT_DATA <= uLength;
    uOffset += len
  ) {
    SWS_PTR( void) pv;
    SWS_size_t uSize;
    ui8 type;

    pv = SWS_BufferGetFragment( hBuffer, uOffset, &uSize);
    if ( NULL == pv || uSize < SWS_PPP_OPT_DATA)
      break;

    SWS_PEEK8( type, pv, SWS_PPP_OPT_TYPE);
    SWS_PEEK8( len, pv, SWS_PPP_OPT_LEN);
    if ( len < SWS_PPP_OPT_DATA)
      break;
    if ( len > uSize)
      continue;

    (*p->pfnConfigRsp)( p, code, type, len, pv);
    }
  }


/*
      This action indicates to the upper layers that the automaton is
      entering the Opened state.

      Typically, this action is used by the LCP to signal the Up event
      to a NCP, Authentication Protocol, or Link Quality Protocol, or
      MAY be used by a NCP to indicate that the link is available for
      its network layer traffic.
*/
#define tlu(p) if ( (p)->pfnUp) (*(p)->pfnUp)( p)


/*
      This action indicates to the upper layers that the automaton is
      leaving the Opened state.

      Typically, this action is used by the LCP to signal the Down event
      to a NCP, Authentication Protocol, or Link Quality Protocol, or
      MAY be used by a NCP to indicate that the link is no longer
      available for its network layer traffic.
*/
#define tld(p) if ( (p)->pfnDown) (*(p)->pfnDown)( p)


/*
      This action indicates to the lower layers that the automaton is
      entering the Starting state, and the lower layer is needed for the
      link.  The lower layer SHOULD respond with an Up event when the
      lower layer is available.

      This results of this action are highly implementation dependent.
*/
#define tls(p) if ( (p)->pfnStarted) (*(p)->pfnStarted)( p)


/*
      This action indicates to the lower layers that the automaton is
      entering the Initial, Closed or Stopped states, and the lower
      layer is no longer needed for the link.  The lower layer SHOULD
      respond with a Down event when the lower layer has terminated.

      Typically, this action MAY be used by the LCP to advance to the
      Link Dead phase, or MAY be used by a NCP to indicate to the LCP
      that the link may terminate when there are no other NCPs open.

      This results of this action are highly implementation dependent.
*/
#define tlf(p) if ( (p)->pfnFinished) (*(p)->pfnFinished)( p); \
  else (*(p)->pfnReset)( p)


/*
      This action sets the Restart counter to the appropriate value
      (Max-Terminate or Max-Configure).  The counter is decremented for
      each transmission, including the first.

      Implementation Note:

         In addition to setting the Restart counter, the implementation
         MUST set the timeout period to the initial value when Restart
         timer backoff is used.
*/
static SWS_FASTCALL( void, InitializeRestartCount( SAutomaton* pAutomaton))
  {
  TRACE( 9, ("InitializeRestartCount(%#x)\n",pAutomaton->protocol));
  ASSERT( NULL != pAutomaton);

  switch ( pAutomaton->eState)
    {
  case kStateClosing:
  case kStateStopping:
    pAutomaton->iRestartCnt = s_iMaxTerminate;
    break;
  default:
    pAutomaton->iRestartCnt = s_iMaxConfigure;
    break;
    }
  }
#define irc(p) InitializeRestartCount(p)


/*
      This action sets the Restart counter to zero.

      Implementation Note:

         This action enables the FSA to pause before proceeding to the
         desired final state, allowing traffic to be processed by the
         peer.  In addition to zeroing the Restart counter, the
         implementation MUST set the timeout period to an appropriate
         value.
*/
static SWS_FASTCALL( void, ZeroRestartCount( SAutomaton* pAutomaton))
  {
  TRACE( 9, ("ZeroRestartCount(%#x)\n",pAutomaton->protocol));
  ASSERT( NULL != pAutomaton);

  pAutomaton->iRestartCnt = 0;
  SWS_TimerStart( pAutomaton->hRestartTimer, s_uRestartPeriod);
  }
#define zrc(p) ZeroRestartCount(p)


/*
      A Configure-Request packet is transmitted.  This indicates the
      desire to open a connection with a specified set of Configuration
      Options.  The Restart timer is started when the Configure-Request
      packet is transmitted, to guard against packet loss.  The Restart
      counter is decremented each time a Configure-Request is sent.
*/
static SWS_FASTCALL( void, SendConfigureRequest( SAutomaton* pAutomaton))
  {
  TRACE( 9, ("SendConfigureRequest(%#x)\n",pAutomaton->protocol));
  ASSERT( NULL != pAutomaton);

  ASSERT( pAutomaton->iRestartCnt > 0);
  pAutomaton->iRestartCnt--;

  if ( pAutomaton->pfnConfigAdd)
    {
    SWS_BufferH hBuffer;
    SWS_PTR( void) pvHdr;
    SWS_size_t uLen;
    const ui16 kSize = SWS_PPP_HDR_DATA;

    /* Get a buffer for header */
    hBuffer = SWS_BufferGet( kSize, SWS_kBufferDefault);
    if ( NULL == hBuffer)
      {
      AtomicIncrement( &s_aTxNoBuffers);

      /* Retry on timeout */
      SWS_TimerStart( pAutomaton->hRestartTimer, s_uRestartPeriod);
      return;
      }

    /* Append protocol specific data */
    (*pAutomaton->pfnConfigAdd)( pAutomaton, hBuffer);

    /* Fill in header */
    pvHdr = SWS_BufferGetFragment( hBuffer, 0, &uLen);
    ASSERT( NULL != pvHdr);
    ASSERT( uLen >= kSize);

    SWS_POKE8( pvHdr, SWS_PPP_HDR_CODE, SWS_PPP_CONFIGURE_REQ);
    SWS_POKE8( pvHdr, SWS_PPP_HDR_ID, ++pAutomaton->uCfgId);
    uLen = SWS_BufferSize( hBuffer);
    SWS_POKE16( pvHdr, SWS_PPP_HDR_LEN, (ui16)uLen);

    (void)AutomatonSend( pAutomaton, hBuffer);
    SWS_BufferRelease( hBuffer);
    }

  /* Start restart timer */
  SWS_TimerStart( pAutomaton->hRestartTimer, s_uRestartPeriod);
  }
#define scr(p) SendConfigureRequest(p)


/*
      A Configure-Ack packet is transmitted.  This acknowledges the
      reception of a Configure-Request packet with an acceptable set of
      Configuration Options.
*/
static SWS_FASTCALL( void, SendConfigureAck(
  SAutomaton* pAutomaton,
  SWS_BufferH hBuffer
)){
  TRACE( 9, ("SendConfigureAck(%#x)\n",pAutomaton->protocol));
  ASSERT( NULL != pAutomaton);

  pAutomaton->iCfgFailures = 0;

  (void)AutomatonSend( pAutomaton, hBuffer);
  }
#define sca(p,h) SendConfigureAck(p,h)


/*
      A Configure-Nak or Configure-Reject packet is transmitted, as
      appropriate.  This negative response reports the reception of a
      Configure-Request packet with an unacceptable set of Configuration
      Options.

      Configure-Nak packets are used to refuse a Configuration Option
      value, and to suggest a new, acceptable value.  Configure-Reject
      packets are used to refuse all negotiation about a Configuration
      Option, typically because it is not recognized or implemented.
      The use of Configure-Nak versus Configure-Reject is more fully
      described in the chapter on LCP Packet Formats.
*/
static SWS_FASTCALL( void, SendConfigureNak(
  SAutomaton* pAutomaton,
  SWS_BufferH hBuffer
)){
  TRACE( 9, ("SendConfigureNak(%#x)\n",pAutomaton->protocol));
  ASSERT( NULL != pAutomaton);

  if ( pAutomaton->iCfgFailures < s_iMaxFailure)
    {
    pAutomaton->iCfgFailures++;
    }
  else
    {
    /* Force Configure-Reject */
    SWS_PTR( void) pvHdr;
    SWS_size_t uLen;

    pvHdr = SWS_BufferGetFragment( hBuffer, 0, &uLen);
    ASSERT( NULL != pvHdr);
    ASSERT( uLen >= SWS_PPP_HDR_DATA);

    SWS_POKE8( pvHdr, SWS_PPP_HDR_CODE, SWS_PPP_CONFIGURE_REJ);
    }

  (void)AutomatonSend( pAutomaton, hBuffer);
  }
#define scn(p,h) SendConfigureNak(p,h)


/*
      A Terminate-Request packet is transmitted.  This indicates the
      desire to close a connection.  The Restart timer is started when
      the Terminate-Request packet is transmitted, to guard against
      packet loss.  The Restart counter is decremented each time a
      Terminate-Request is sent.
*/
static SWS_FASTCALL( void, SendTerminateRequest( SAutomaton* pAutomaton))
  {
  SWS_BufferH hBuffer;
  SWS_PTR( void) pvHdr;
  SWS_size_t uLen;
  const ui16 kSize = SWS_PPP_HDR_DATA;

  TRACE( 3, ("SendTerminateRequest(%#x)\n",pAutomaton->protocol));
  ASSERT( NULL != pAutomaton);

  ASSERT( pAutomaton->iRestartCnt > 0);
  pAutomaton->iRestartCnt--;

  /* Get a buffer */
  hBuffer = SWS_BufferGet( kSize, SWS_kBufferDefault);
  if ( NULL == hBuffer)
    {
    AtomicIncrement( &s_aTxNoBuffers);

    /* Retry on timeout */
    SWS_TimerStart( pAutomaton->hRestartTimer, s_uRestartPeriod);
    return;
    }

  pvHdr = SWS_BufferGetFragment( hBuffer, 0, &uLen);
  ASSERT( NULL != pvHdr);
  ASSERT( uLen >= kSize);

  SWS_POKE8( pvHdr, SWS_PPP_HDR_CODE, SWS_PPP_TERMINATE_REQ);
  if ( 0 == pAutomaton->uTermId)
    pAutomaton->uTermId = pAutomaton->uCfgId;
  SWS_POKE8( pvHdr, SWS_PPP_HDR_ID, ++pAutomaton->uTermId);
  SWS_POKE16( pvHdr, SWS_PPP_HDR_LEN, kSize);

  (void)AutomatonSend( pAutomaton, hBuffer);

  /* Start restart timer */
  SWS_TimerStart( pAutomaton->hRestartTimer, s_uRestartPeriod);

  SWS_BufferRelease( hBuffer);
  }
#define str(p) SendTerminateRequest(p)


/*
      A Terminate-Ack packet is transmitted.  This acknowledges the
      reception of a Terminate-Request packet or otherwise serves to
      synchronize the automatons.
*/
static SWS_FASTCALL( void, SendTerminateAck(
  SAutomaton* pAutomaton,
  SWS_BufferH hBuffer
)){
  SWS_PTR( void) pvHdr;
  SWS_size_t uLen;

  TRACE( 9, ("SendTerminateAck(%#x)\n",pAutomaton->protocol));
  ASSERT( NULL != pAutomaton);

  pvHdr = SWS_BufferGetFragment( hBuffer, 0, &uLen);
  ASSERT( NULL != pvHdr);
  ASSERT( uLen >= SWS_PPP_HDR_DATA);

  SWS_POKE8( pvHdr, SWS_PPP_HDR_CODE, SWS_PPP_TERMINATE_ACK);

  (void)AutomatonSend( pAutomaton, hBuffer);
  }
#define sta(p,h) SendTerminateAck(p, h)


/*
      A Code-Reject packet is transmitted.  This indicates the reception
      of an unknown type of packet.
*/
static SWS_FASTCALL( void, SendCodeReject(
  SAutomaton* pAutomaton,
  SWS_BufferH hBuffer
)){
  TRACE( 9, ("SendCodeReject(%#x)\n",pAutomaton->protocol));
  ASSERT( NULL != pAutomaton);

  if ( 0 == SWS_BufferAppendFront( hBuffer, SWS_PPP_HDR_DATA))
    {
    SWS_PTR( void) pvHdr;
    SWS_size_t uLen;

    pvHdr = SWS_BufferGetFragment( hBuffer, 0, &uLen);
    ASSERT( NULL != pvHdr);
    ASSERT( uLen >= SWS_PPP_HDR_DATA);

    SWS_POKE8( pvHdr, SWS_PPP_HDR_CODE, SWS_PPP_CODE_REJ);
    SWS_POKE8( pvHdr, SWS_PPP_HDR_ID, ++pAutomaton->uRejId);
    uLen = SWS_BufferSize( hBuffer);
    SWS_POKE16( pvHdr, SWS_PPP_HDR_LEN, (ui16)uLen);

    (void)AutomatonSend( pAutomaton, hBuffer);
    }
  }
#define scj(p,h) SendCodeReject(p,h)


/*
      An Echo-Reply packet is transmitted.  This acknowledges the
      reception of an Echo-Request packet.
*/
static SWS_FASTCALL( void, SendEchoReply(
  SAutomaton* pAutomaton,
  SWS_BufferH hBuffer
)){
  SWS_PTR( void) pvHdr;
  SWS_size_t uLen;
  ui8 type;

  TRACE( 9, ("SendEchoReply(%#x)\n",pAutomaton->protocol));
  ASSERT( NULL != pAutomaton);

  pvHdr = SWS_BufferGetFragment( hBuffer, 0, &uLen);
  ASSERT( NULL != pvHdr);
  ASSERT( uLen >= SWS_PPP_HDR_DATA);

  SWS_PEEK8( type, pvHdr, SWS_PPP_OPT_TYPE);
  if ( SWS_PPP_ECHO_REQ == type)
    {
    SWS_POKE8( pvHdr, SWS_PPP_HDR_CODE, SWS_PPP_ECHO_REPLY);
    SWS_POKE32( pvHdr, SWS_PPP_HDR_DATA, pAutomaton->pAdapter->uTxMagic);
    (void)AutomatonSend( pAutomaton, hBuffer);
    }
  }
#define ser(p,h) SendEchoReply(p,h)


/*
 * State transition
 */
static SWS_FASTCALL( void, AutomatonNewState(
  SAutomaton* pAutomaton,
  EState eState
)){
  TRACE( 9, ("AutomatonNewState(%p,%d)\n", pAutomaton,eState));

  if ( eState != pAutomaton->eState)
    {
    TRACE( 4, ("AutomatonNewState(%#x) %d -> %d\n",
      pAutomaton->protocol, pAutomaton->eState, eState));

    switch ( eState)
      {
    case kStateInitial:
    case kStateStarting:
    case kStateClosed:
    case kStateStopped:
    case kStateOpened:
      SWS_TimerStop( pAutomaton->hRestartTimer);
      break;

    case kStateClosing:
    case kStateStopping:
    case kStateReqSent:
    case kStateAckRcvd:
    case kStateAckSent:
      break;
      }

    pAutomaton->eState = eState;
    }
  }
#define NEXT( p, s) AutomatonNewState( p, (EState)(s))
#define ILLEGAL() ASSERT( !"Illegal event")


/*
 * LCP/NCP finite state automaton
 */
void AutomatonEvent(
  SAutomaton* pAutomaton,
  EEvent eEvent,
  SWS_BufferH hBuffer
) {
  TRACE( 3, ("AutomatonEvent(%#x,%d,%p)\n",
    pAutomaton->protocol,eEvent,hBuffer));
  ASSERT( NULL != pAutomaton);

  switch ( pAutomaton->eState)
    {
  case kStateInitial:
    switch ( eEvent)
      {
    case kEventUp:
      NEXT( pAutomaton, 2);
      break;
    case kEventDown:
      ILLEGAL();
      break;
    case kEventOpen:
      NEXT( pAutomaton, 1);
      tls( pAutomaton);
      break;
    case kEventClose:
      NEXT( pAutomaton, 0);
      break;
    case kEventTO_P:
    case kEventTO_M:
    case kEventRCR_P:
    case kEventRCR_M:
    case kEventRCA:
    case kEventRCN:
    case kEventRTR:
    case kEventRTA:
    case kEventRUC:
    case kEventRXJ_P:
    case kEventRXJ_M:
    case kEventRXR:
      ILLEGAL();
      break;
      }
    break;

  case kStateStarting:
    switch ( eEvent)
      {
    case kEventUp:
      NEXT( pAutomaton, 6);
      irc( pAutomaton);
#if 1
      /* Send config request after link stable */
      SWS_TimerStart( pAutomaton->hRestartTimer, 50);
#else
      scr( pAutomaton);
#endif
      break;
    case kEventDown:
      ILLEGAL();
      break;
    case kEventOpen:
      NEXT( pAutomaton, 1);
      break;
    case kEventClose:
      NEXT( pAutomaton, 0);
      tlf( pAutomaton);
      break;
    case kEventTO_P:
    case kEventTO_M:
    case kEventRCR_P:
    case kEventRCR_M:
    case kEventRCA:
    case kEventRCN:
    case kEventRTR:
    case kEventRTA:
    case kEventRUC:
    case kEventRXJ_P:
    case kEventRXJ_M:
    case kEventRXR:
      ILLEGAL();
      break;
      }
    break;

  case kStateClosed:
    switch ( eEvent)
      {
    case kEventUp:
      ILLEGAL();
      break;
    case kEventDown:
      NEXT( pAutomaton, 0);
      break;
    case kEventOpen:
      NEXT( pAutomaton, 6);
      irc( pAutomaton);
      scr( pAutomaton);
      break;
    case kEventClose:
      NEXT( pAutomaton, 2);
      break;
    case kEventTO_P:
    case kEventTO_M:
      ILLEGAL();
      break;
    case kEventRCR_P:
    case kEventRCR_M:
    case kEventRCA:
    case kEventRCN:
    case kEventRTR:
      NEXT( pAutomaton, 2);
      sta( pAutomaton, hBuffer);
      break;
    case kEventRTA:
      NEXT( pAutomaton, 2);
      break;
    case kEventRUC:
      NEXT( pAutomaton, 2);
      scj( pAutomaton, hBuffer);
      break;
    case kEventRXJ_P:
      NEXT( pAutomaton, 2);
      break;
    case kEventRXJ_M:
      NEXT( pAutomaton, 2);
      tlf( pAutomaton);
      break;
    case kEventRXR:
      NEXT( pAutomaton, 2);
      break;
      }
    break;

  case kStateStopped:
    switch ( eEvent)
      {
    case kEventUp:
      ILLEGAL();
      break;
    case kEventDown:
      NEXT( pAutomaton, 1);
      tls( pAutomaton);
      break;
    case kEventOpen:
#if 0 /* Restart option - RFC 1661 p17 */
      AutomatonEvent( pAutomaton, kEventDown);
      AutomatonEvent( pAutomaton, kEventUp);
#else
      NEXT( pAutomaton, 3);
#endif
      break;
    case kEventClose:
      NEXT( pAutomaton, 2);
      break;
    case kEventTO_P:
    case kEventTO_M:
      ILLEGAL();
      break;
    case kEventRCR_P:
      NEXT( pAutomaton, 8);
      irc( pAutomaton);
      scr( pAutomaton);
      sca( pAutomaton, hBuffer);
      break;
    case kEventRCR_M:
      NEXT( pAutomaton, 6);
      irc( pAutomaton);
      scr( pAutomaton);
      scn( pAutomaton, hBuffer);
      break;
    case kEventRCA:
    case kEventRCN:
    case kEventRTR:
      NEXT( pAutomaton, 3);
      sta( pAutomaton, hBuffer);
      break;
    case kEventRTA:
      NEXT( pAutomaton, 3);
      break;
    case kEventRUC:
      NEXT( pAutomaton, 3);
      scj( pAutomaton, hBuffer);
      break;
    case kEventRXJ_P:
      NEXT( pAutomaton, 3);
      break;
    case kEventRXJ_M:
      NEXT( pAutomaton, 3);
      tlf( pAutomaton);
      break;
    case kEventRXR:
      NEXT( pAutomaton, 3);
      break;
      }
    break;

  case kStateClosing:
    switch ( eEvent)
      {
    case kEventUp:
      ILLEGAL();
      break;
    case kEventDown:
      NEXT( pAutomaton, 0);
      break;
    case kEventOpen:
#if 0 /* Restart option - RFC 1661 p17 */
      AutomatonEvent( pAutomaton, kEventDown);
      AutomatonEvent( pAutomaton, kEventUp);
#else
      NEXT( pAutomaton, 5);
#endif
      break;
    case kEventClose:
      NEXT( pAutomaton, 4);
      break;
    case kEventTO_P:
      NEXT( pAutomaton, 4);
      str( pAutomaton);
      break;
    case kEventTO_M:
      NEXT( pAutomaton, 2);
      tlf( pAutomaton);
      break;
    case kEventRCR_P:
    case kEventRCR_M:
    case kEventRCA:
    case kEventRCN:
      NEXT( pAutomaton, 4);
      break;
    case kEventRTR:
      NEXT( pAutomaton, 4);
      sta( pAutomaton, hBuffer);
      break;
    case kEventRTA:
      NEXT( pAutomaton, 2);
      tlf( pAutomaton);
      break;
    case kEventRUC:
      NEXT( pAutomaton, 4);
      scj( pAutomaton, hBuffer);
      break;
    case kEventRXJ_P:
      NEXT( pAutomaton, 4);
      break;
    case kEventRXJ_M:
      NEXT( pAutomaton, 2);
      tlf( pAutomaton);
      break;
    case kEventRXR:
      NEXT( pAutomaton, 4);
      break;
      }
    break;

  case kStateStopping:
    switch ( eEvent)
      {
    case kEventUp:
      ILLEGAL();
      break;
    case kEventDown:
      NEXT( pAutomaton, 1);
      break;
    case kEventOpen:
#if 0 /* Restart option - RFC 1661 p17 */
      AutomatonEvent( pAutomaton, kEventDown);
      AutomatonEvent( pAutomaton, kEventUp);
#else
      NEXT( pAutomaton, 5);
#endif
      break;
    case kEventClose:
      NEXT( pAutomaton, 4);
      break;
    case kEventTO_P:
      NEXT( pAutomaton, 5);
      str( pAutomaton);
      break;
    case kEventTO_M:
      NEXT( pAutomaton, 3);
      tlf( pAutomaton);
      break;
    case kEventRCR_P:
    case kEventRCR_M:
    case kEventRCA:
    case kEventRCN:
      NEXT( pAutomaton, 5);
      break;
    case kEventRTR:
      NEXT( pAutomaton, 5);
      sta( pAutomaton, hBuffer);
      break;
    case kEventRTA:
      NEXT( pAutomaton, 3);
      tlf( pAutomaton);
      break;
    case kEventRUC:
      NEXT( pAutomaton, 5);
      scj( pAutomaton, hBuffer);
      break;
    case kEventRXJ_P:
      NEXT( pAutomaton, 5);
      break;
    case kEventRXJ_M:
      NEXT( pAutomaton, 3);
      tlf( pAutomaton);
      break;
    case kEventRXR:
      NEXT( pAutomaton, 5);
      break;
      }
    break;

  case kStateReqSent:
    switch ( eEvent)
      {
    case kEventUp:
      ILLEGAL();
      break;
    case kEventDown:
      NEXT( pAutomaton, 1);
      break;
    case kEventOpen:
      NEXT( pAutomaton, 6);
      break;
    case kEventClose:
      NEXT( pAutomaton, 4);
      irc( pAutomaton);
      str( pAutomaton);
      break;
    case kEventTO_P:
      NEXT( pAutomaton, 6);
      scr( pAutomaton);
      break;
    case kEventTO_M:
#if 0 /* Passive option - RFC 1661 p 15 */
      NEXT( pAutomaton, kStateReqSent);
#else
      NEXT( pAutomaton, 3);
      tlf( pAutomaton);
#endif
      break;
    case kEventRCR_P:
      NEXT( pAutomaton, 8);
      sca( pAutomaton, hBuffer);
      break;
    case kEventRCR_M:
      NEXT( pAutomaton, 6);
      scn( pAutomaton, hBuffer);
      break;
    case kEventRCA:
      NEXT( pAutomaton, 7);
      irc( pAutomaton);
      break;
    case kEventRCN:
      NEXT( pAutomaton, 6);
      irc( pAutomaton);
      scr( pAutomaton);
      break;
    case kEventRTR:
      NEXT( pAutomaton, 6);
      sta( pAutomaton, hBuffer);
      break;
    case kEventRTA:
      NEXT( pAutomaton, 6);
      break;
    case kEventRUC:
      NEXT( pAutomaton, 6);
      scj( pAutomaton, hBuffer);
      break;
    case kEventRXJ_P:
      NEXT( pAutomaton, 6);
      break;
    case kEventRXJ_M:
      NEXT( pAutomaton, 3);
      tlf( pAutomaton);
      break;
    case kEventRXR:
      NEXT( pAutomaton, 6);
      break;
      }
    break;

  case kStateAckRcvd:
    switch ( eEvent)
      {
    case kEventUp:
      ILLEGAL();
      break;
    case kEventDown:
      NEXT( pAutomaton, 1);
      break;
    case kEventOpen:
      NEXT( pAutomaton, 7);
      break;
    case kEventClose:
      NEXT( pAutomaton, 4);
      irc( pAutomaton);
      str( pAutomaton);
      break;
    case kEventTO_P:
      NEXT( pAutomaton, 6);
      scr( pAutomaton);
      break;
    case kEventTO_M:
#if 0 /* Passive option - RFC 1661 p 15 */
      NEXT( pAutomaton, kStateAckRcvd);
#else
      NEXT( pAutomaton, 3);
      tlf( pAutomaton);
#endif
      break;
    case kEventRCR_P:
      NEXT( pAutomaton, 9);
      sca( pAutomaton, hBuffer);
      tlu( pAutomaton);
      break;
    case kEventRCR_M:
      NEXT( pAutomaton, 7);
      scn( pAutomaton, hBuffer);
      break;
    case kEventRCA:
    case kEventRCN:
      NEXT( pAutomaton, 6);
      scr( pAutomaton);
      /* Crossed connection? - RFC 1661 p19 */
      TRACE( 1, ("*** PppOnEvent() Crossed connection\n"));
      break;
    case kEventRTR:
      NEXT( pAutomaton, 6);
      sta( pAutomaton, hBuffer);
      break;
    case kEventRTA:
      NEXT( pAutomaton, 6);
      break;
    case kEventRUC:
      NEXT( pAutomaton, 7);
      scj( pAutomaton, hBuffer);
      break;
    case kEventRXJ_P:
      NEXT( pAutomaton, 6);
      break;
    case kEventRXJ_M:
      NEXT( pAutomaton, 3);
      tlf( pAutomaton);
      break;
    case kEventRXR:
      NEXT( pAutomaton, 7);
      break;
      }
    break;

  case kStateAckSent:
    switch ( eEvent)
      {
    case kEventUp:
      ILLEGAL();
      break;
    case kEventDown:
      NEXT( pAutomaton, 1);
      break;
    case kEventOpen:
      NEXT( pAutomaton, 8);
      break;
    case kEventClose:
      NEXT( pAutomaton, 4);
      irc( pAutomaton);
      str( pAutomaton);
      break;
    case kEventTO_P:
      NEXT( pAutomaton, 8);
      scr( pAutomaton);
      break;
    case kEventTO_M:
#if 0 /* Passive option - RFC 1661 p 15 */
      NEXT( pAutomaton, kStateAckSent);
#else
      NEXT( pAutomaton, 3);
      tlf( pAutomaton);
#endif
      break;
    case kEventRCR_P:
      NEXT( pAutomaton, 8);
      sca( pAutomaton, hBuffer);
      break;
    case kEventRCR_M:
      NEXT( pAutomaton, 6);
      scn( pAutomaton, hBuffer);
      break;
    case kEventRCA:
      NEXT( pAutomaton, 9);
      irc( pAutomaton);
      tlu( pAutomaton);
      break;
    case kEventRCN:
      NEXT( pAutomaton, 8);
      irc( pAutomaton);
      scr( pAutomaton);
      break;
    case kEventRTR:
      NEXT( pAutomaton, 6);
      sta( pAutomaton, hBuffer);
      break;
    case kEventRTA:
      NEXT( pAutomaton, 8);
      break;
    case kEventRUC:
      NEXT( pAutomaton, 8);
      scj( pAutomaton, hBuffer);
      break;
    case kEventRXJ_P:
      NEXT( pAutomaton, 8);
      break;
    case kEventRXJ_M:
      NEXT( pAutomaton, 3);
      tlf( pAutomaton);
      break;
    case kEventRXR:
      NEXT( pAutomaton, 8);
      break;
      }
    break;

  case kStateOpened:
    switch ( eEvent)
      {
    case kEventUp:
      ILLEGAL();
      break;
    case kEventDown:
      NEXT( pAutomaton, 1);
      tld( pAutomaton);
      break;
    case kEventOpen:
#if 0 /* Restart option - RFC 1661 p17 */
      AutomatonEvent( pAutomaton, kEventDown);
      AutomatonEvent( pAutomaton, kEventUp);
#else
      NEXT( pAutomaton, 9);
#endif
      break;
    case kEventClose:
      NEXT( pAutomaton, 4);
      tld( pAutomaton);
      irc( pAutomaton);
      str( pAutomaton);
      break;
    case kEventTO_P:
    case kEventTO_M:
      ILLEGAL();
      break;
    case kEventRCR_P:
      NEXT( pAutomaton, 8);
      tld( pAutomaton);
      scr( pAutomaton);
      sca( pAutomaton, hBuffer);
      break;
    case kEventRCR_M:
      NEXT( pAutomaton, 6);
      tld( pAutomaton);
      scr( pAutomaton);
      scn( pAutomaton, hBuffer);
      break;
    case kEventRCA:
    case kEventRCN:
      NEXT( pAutomaton, 6);
      tld( pAutomaton);
      scr( pAutomaton);
      /* Crossed connection? - RFC 1661 p19 */
      TRACE( 1, ("*** PppOnEvent() Crossed connection\n"));
      break;
    case kEventRTR:
      NEXT( pAutomaton, 5);
      tld( pAutomaton);
      zrc( pAutomaton);
      sta( pAutomaton, hBuffer);
      break;
    case kEventRTA:
      NEXT( pAutomaton, 6);
      tld( pAutomaton);
      scr( pAutomaton);
      break;
    case kEventRUC:
      NEXT( pAutomaton, 9);
      scj( pAutomaton, hBuffer);
      break;
    case kEventRXJ_P:
      NEXT( pAutomaton, 9);
      break;
    case kEventRXJ_M:
      NEXT( pAutomaton, 5);
      tld( pAutomaton);
      irc( pAutomaton);
      str( pAutomaton);
      break;
    case kEventRXR:
      NEXT( pAutomaton, 9);
      ser( pAutomaton, hBuffer);
      break;
      }
    }
  }

#else
char SWS_Ppp;
#endif /* OPT_PPP && OPT_DATALINK_P2P */
/* End of file */
