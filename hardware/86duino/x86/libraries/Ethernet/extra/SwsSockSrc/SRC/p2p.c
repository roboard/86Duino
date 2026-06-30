/****************************************************************************
 *
 * File name   : p2p.c
 * Function    : Point to point link manager
 * Project     : SwsSock
 * Authors     : Lawrence Rust
 * Systems     : ANSI C.
 *
 ****************************************************************************
 *
 * Created by Lawrence Rust, Software Systems Consultants
 * lvr@softsystem.co.uk. Tel/Fax +33 5 49 72 79 63
 *
 * 18-Jul-05, LVR, Created.
 *
 * Comments:
 * --------
 * This file implements the P2P link manager supporting PPP & SLIP
 *
 ****************************************************************************
 */

/* Compilation options */
#include "config.h"

/* Exports */
#include "sws_p2p.h"
#include "p2p.h"

#if OPT_DATALINK_P2P
/* Imports */
#include <stddef.h>
#include <string.h>
#include <ctype.h>

#include "sws_sock.h"  /* for SWS_INADDR_ANY and error codes */
#include "sws_str.h"   /* SWS_stricmp */
#include "sws_dlnk.h"  /* SWS_DatalinkFind & SWS_DatalinkSetProperty */
#include "sws_timr.h"
#include "sws_gef.h"

#if OPT_SLIP
#include "slip.h"
#endif
#if OPT_PPP
#include "sws_ppp.h"
#include "ppp.h"
#endif

#if OPT_P2P_VJCOMP
#include "sws_ip.h"
#include "sws_tcp.h"
#include "deque.h"
#endif

#include "platform.h"
#include "debug.h"


/*
 * Macros & Constants
 */

/* Connection configuration */
#define kszProtocolSlip   "slip"
#define kszProtocolPpp    "ppp"

/* Authentication protocol */
#define kszAuthNone       "none"
#define kszAuthPap        "pap"
#define kszAuthChap       "chap"

/* Modems supported */
#define kszModemNull      "null"
#define kszModemHayes     "hayes"
#define kRings            2
#define kszModemMsDirect  "msdirect"
typedef enum EModem {
  kModemNone,
  kModemHayes,
  kModemMsDirect
} EModem;

/* Adapter state */
typedef enum EState {
  kStateClosed,
  kStateOpen,
  kStateCalling,
  kStateLogin,
  kStateConnected,
  kStateListening,
  kStateRinging,
  kStateAccepted,
  kStateClosing
} EState;

#define kCR               '\x0d'
#define kszCR             "\x0d"

/* Modem command strings */
#define kModemEsc         "+++" /* Set esc char with S2=43 */
#define kModemCmdPrefix   "AT"
#define kModemCmdSuffix   kszCR
#define kModemReset       "Z"
#define kModemFactorySet0 "&F0"
#define kModemEchoOff     "E0"
#define kModemTerseResp   "Q0V0"
#define kModemVerboseResp "Q0V1"
#define kModemCompressOn  "S46=138"
#define kModemCompressOff "S46=136"
#define kModemErrorForce  "\N2"
#define kModemErrorOn     "&Q5"
#define kModemErrorOff    "&Q6"
#define kModemFlowOff     "&K0"
#define kModemFlowHard    "&K3"
#define kModemFlowSoft    "&K4"
#define kModemDcdOff      "&C0"
#define kModemDcdOn       "&C1"
#define kModemDtrIgnore   "&D0"
#define kModemDtrAttn     "&D1"
#define kModemDtrHangup   "&D2"
#define kModemDtrReset    "&D3"
#define kModemNegoOff     "W0S95=0"
#define kModemNegoOn      "W1S95=47"
#define kModemNegoOffDce  "W2S95=1"
#define kModemExtendOff   "X0"
#define kModemExtendSpeed "X1"
#define kModemExtendDial  "X2"
#define kModemExtendBusy  "X3"
#define kModemExtendAll   "X4"
#define kModemModnITU     "B0"
#define kModemModnBell    "B1"
#define kModemSpeakerOff  "M0"
#define kModemSpeakerDial "M1"
#define kModemSpeakerOn   "M2"
#define kModemSpeakerSet  "M3"
#define kModemVolumeOff   "L0"
#define kModemVolumeLow   "L1"
#define kModemVolumeMed   "L2"
#define kModemVolumeHi    "L3"
#define kModemAutoAnswer  "S0="
#define kModemAutoAnswerOff kModemAutoAnswer "0"
#define kModemDial        "D"
#define kModemAnswer      "A"
#define kModemHangup      "H"

#define kModemInit kModemFactorySet0 kModemEchoOff kModemVerboseResp
#define kModemUsrInit \
  kModemDtrHangup kModemFlowHard kModemDcdOn \
  kModemAutoAnswerOff \
  kModemExtendAll kModemNegoOn \
  kModemVolumeLow kModemSpeakerDial \
  kModemModnITU kModemErrorOn kModemCompressOn

#if 1000 == SWS_TIMER_TICKS
#define MS2TICKS(m) ((m) ? (ui32)(m) : WAIT_NEVER)
#else
#define MS2TICKS(m) ((m) ? ((ui32)(m) * SWS_TIMER_TICKS) / 1000 : WAIT_NEVER)
#endif

#define kConnectTimeout   60000UL /* Call connect timeout, millisecs */
#define kMsModemTimeout1    300 /* MsDirect initial modem retry */
#define kMsModemTimeout2   1000 /* MsDirect subsequent modem retry */
#define kModemCmdTimeout   1500 /* Modem command timeout, millisecs */
#define kModemGuardTime    1100 /* Modem guard time, millisecs */
#define kWriteTimeout      1000 /* Adapter write timeout, millisecs */
#define kHayesIdleTimeout   500 /* Rx idle, millisecs */
#define kRxIdleTimeout       50 /* Rx idle, millisecs */
#define kDcdDebounce         20 /* Millisecs */
#define kDsrDebounce         20 /* Millisecs */


/*
 * Local data types
 */

/* Adapter instance data */
typedef struct SWS_SP2pAdapter
  {
  ui32 uMagic;
#define ADAPTER_MAGIC SWS_MAKEFOURCC( 'p','2','p','0')

  /* Adapter dispatch table */
  SWS_FNPTR( SWS_FP2pSend) pfnSend;
  SWS_FNPTR( SWS_FP2pRecv) pfnRecv;
  SWS_FNPTR( SWS_FP2pShutdown) pfnShutdown;
  SWS_FNPTR( SWS_FP2pIoctl) pfnIoctl;
  SWS_PTR( void) pvContext;           /* Context for dispatch fns */

  /* IP datalink upcalls */
  SWS_DatalinkCallbacks dlink;

  /* P2P protocol dispatch table */
  SWS_P2pProtocol proto;

  /* Datalink properties */
  char szIdent[ 32];                  /* Adapter ident string */
  ui32 uHwAddress;

  /* Modem properties */
  EModem eModem;
  char szModemInit[ 80];
  unsigned uRings;

  /* Adapter status */
  volatile int iAdapterCarrier, iAdapterRing, iAdapterReady;

  /* Resources */
  SWS_TimerH hDcdDebounceTimer;
  SWS_TimerH hDsrDebounceTimer;
  SWS_TimerH hRxIdleTimer;
  SWS_TimerH hLoginTimer;

  /* Session properties */
  SWS_SP2pAcct user;
  SWS_FNPTR( SWS_FP2pEvent) pfnEvent; /* Client callback */
  SWS_PTR( void) pvEvent;
  ui32 uIpAddress;
  ui32 uSubnetMask;
  ui32 uGatewayAddr;                  /* Adapter's default gateway */
  ui32 uIdleTimeout;                  /* Link idle timeout */
  ui16 auth;                          /* SWS_PPP_PAP/CHAP etc */
#if OPT_P2P_VJCOMP
  ui16 uCompSlots;                    /* VJ compression slots */
#endif

  /* Session state */
  EState eState;                      /* Connection state */
  ui32 ulBaud;                        /* Connection speed */
  int iModemResponse;                 /* Last modem response code */

  /* Adapter I/O pending */
  volatile int iTxPending, iRxPending;

  /* Rx buffer */
  int isRxEnabled;
  Atomic_t aRxDpc;
  volatile unsigned rxWrite, rxRead;  /* Write/read cursors */
  unsigned char cRxBuffer[ 128];
  } SWS_SP2pAdapter;


/*
 * Prototypes
 */
static SWS_FASTCALLDEF( void, P2pFreeAdapter( SWS_SP2pAdapter*));

static SWS_FASTCALLDEF( int, P2pBind( SWS_SP2pAdapter*, ui32));
static SWS_FASTCALLDEF( void, P2pRelease( SWS_SP2pAdapter*));

static int P2pCall( SWS_SP2pAdapter*, SWS_PTR( const char), ui32);
static SWS_FASTCALLDEF( int, P2pListen( SWS_SP2pAdapter*, ui32));
static SWS_FASTCALLDEF( int, P2pClose( SWS_SP2pAdapter*));
static SWS_FASTCALLDEF( int, P2pHangup( SWS_SP2pAdapter*));
static SWS_FASTCALLDEF( int, P2pSession(
  SWS_PTR( const char), SWS_PTR( SWS_SP2pSession) ));

static SWS_FASTCALLDEF( void, P2pOnConnected( SWS_SP2pAdapter*));
static SWS_FASTCALLDEF( int, P2pOnLogin( SWS_SP2pAdapter*, const char*));
static SWS_FASTCALLDEF( void, P2pOnLinkUp( SWS_SP2pAdapter*));
static SWS_FASTCALLDEF( void, P2pOnLinkDown( SWS_SP2pAdapter*));
static SWS_FASTCALLDEF( void, P2pOnRing( SWS_SP2pAdapter*));
static SWS_FASTCALLDEF( void, P2pOnError( SWS_SP2pAdapter*, int));

static SWS_FASTCALLDEF( int, P2pModemInit( SWS_SP2pAdapter*));
static int P2pModemCmd( SWS_SP2pAdapter*, const char*, ui32);
static SWS_FASTCALLDEF( int, P2pModemResponse( SWS_SP2pAdapter*, const char*));
static SWS_FASTCALLDEF( int, P2pModemRsp2Err( int));

static SWS_FASTCALLDEF( int, P2pAdapterWrite( SWS_SP2pAdapter*, const char*));
static SWS_FASTCALLDEF( size_t, P2pAdapterPeek( SWS_SP2pAdapter*, int));
static size_t P2pAdapterRead( SWS_SP2pAdapter*, char*, size_t, int);
static SWS_FASTCALLDEF( int, P2pAdapterStatus( SWS_SP2pAdapter*));
static SWS_FASTCALLDEF( int, P2pAdapterOnline( SWS_SP2pAdapter*, int));

static SWS_FASTCALLDEF( void, P2pRxEnable( SWS_SP2pAdapter*, int));
static SWS_FASTCALLDEF( void, P2pRxData( SWS_SP2pAdapter*, size_t));
static SWS_FASTCALLDEF( void, P2pRecvReady( SWS_SP2pAdapter*, int));

static SWS_FASTCALLDEF( ui32, GetIp( SWS_PTR( const char)));

/* Datalink down calls */
static SWS_FDatalinkSend P2pSend;
static SWS_FDatalinkGetProperty P2pGetProperty;
static SWS_FDatalinkSetProperty P2pSetProperty;

/* Adapter up calls */
static SWS_FP2pSendAck P2pSendAck;
static SWS_FP2pRecvInd P2pRecvInd;
static SWS_FP2pStatInd P2pStatInd;

/* Callbacks */
static FPlatformCallback P2pOpen;
static SWS_FTimer DcdDebounceTimeout;
static SWS_FTimer DsrDebounceTimeout;
static SWS_FTimer RxIdleTimeout;
static SWS_FTimer LoginTimeout;
static FPlatformDpc P2pRecvIndDpc;
static FPlatformDpc P2pStatIndDpc;
static SWS_FP2pEvent P2pClientLogin;
static SWS_FP2pEvent P2pEvent;

/* Get contiguous free space in Rx buffer */
#define P2pRxFree( pAdapter) \
  (unsigned)((pAdapter->rxWrite < pAdapter->rxRead) \
  ? ((pAdapter->rxRead - pAdapter->rxWrite) - 1) \
  : (pAdapter->rxRead > 0) \
  ? (sizeof( pAdapter->cRxBuffer) - pAdapter->rxWrite) \
  : ((sizeof( pAdapter->cRxBuffer) - 1) - pAdapter->rxWrite))

/* Get no. pending Rx bytes */
#define P2pRxPending( pAdapter) \
  (unsigned)((pAdapter->rxWrite < pAdapter->rxRead) \
  ? ((sizeof( pAdapter->cRxBuffer) - pAdapter->rxRead) + pAdapter->rxWrite) \
  : (pAdapter->rxWrite - pAdapter->rxRead))

#define P2pRxFlush( pAdapter) \
  ((pAdapter)->rxRead = (pAdapter)->rxWrite)

/* Validate adapter */
#define IS_P2P_ADAPTER( a) ( (a) >= s_adapterArray \
  && ((a) - s_adapterArray) < (ptrdiff_t)s_uAdapters \
  && ADAPTER_MAGIC == (a)->uMagic )

/* Get adapter index */
#define INDEX( a) ((unsigned)((a) - s_adapterArray))

/* Send an event to client */
#define EVENT( a, e, u, d) ((a)->pfnEvent ? (*(a)->pfnEvent)( \
  (a)->pvEvent, INDEX( a), (e), (u), (d)) : 0)


/*
 * Module data
 */

/* Configuration */

/* Adapter list */
static SWS_SP2pAdapter s_adapterArray[ OPT_DATALINK_P2P];
static unsigned s_uAdapters;

/* Data link descriptor */
static const SWS_DatalinkDesc s_p2p = {
  sizeof( SWS_DatalinkDesc),
  SWS_DATALINK_VERSION,
  &P2pSend,
  &P2pGetProperty,
  &P2pSetProperty,
  0, /* SWS_FDatalinkShutdown */
  0  /* SWS_FDatalinkSetMulticast */
};


/****************************************************************************
 * External non-client functions
 ****************************************************************************/

/*
 * Initialise this module
 */
extern int SWS_P2pInit( void)
  {
  TRACE( 9, ("SWS_P2pInit()\n"));
  ASSERT( PlatformIsCritical());

  /* Init static data */
  s_uAdapters = 0;
  memset( s_adapterArray, 0, sizeof(s_adapterArray) );

  /* Init protocols */
#if OPT_SLIP
  SWS_SlipInit();
#endif
#if OPT_PPP
  SWS_PppInit();
#endif

  return 0;
  }


/*
 * Finalise this module
 */
extern void SWS_P2pCleanup( void)
  {
  unsigned u;
  SWS_SP2pAdapter* pAdapter;

  TRACE( 9, ("SWS_P2pCleanup()\n"));
  ASSERT( PlatformIsCritical());

  /* Shutdown all adapters */
  for (
    pAdapter = &s_adapterArray[ u = 0];
    u < s_uAdapters;
    ++u, ++pAdapter
  ) {
    if ( ADAPTER_MAGIC == pAdapter->uMagic)
      {
      P2pClose( pAdapter);

      if ( pAdapter->pfnShutdown)
        (*pAdapter->pfnShutdown)( pAdapter->pvContext);

      P2pFreeAdapter( pAdapter);
      }
    }
  s_uAdapters = 0;

  /* Finalise protocols */
#if OPT_PPP
  SWS_PppCleanup();
#endif
#if OPT_SLIP
  SWS_SlipCleanup();
#endif
  }


/*
 * Tear down a link
 */
extern int SWS_P2pLinkFinished(
  unsigned uAdapter                   /* IN: 0..SWS_P2pCount() -1 */
) {
  SWS_SP2pAdapter* pAdapter;

  TRACE( 9, ("SWS_P2pLinkFinished(%u)\n", uAdapter));

  if ( uAdapter >= s_uAdapters)
    {
    TRACE( 0, ("!!! SWS_P2pLinkFinished no such adapter %u\n", uAdapter));
    return SWS_EINVAL;
    }
  pAdapter = &s_adapterArray[ uAdapter];
  ASSERT( ADAPTER_MAGIC == pAdapter->uMagic);

  TRACE( 2, ("SWS_P2pLinkFinished(%s)\n", pAdapter->szIdent));

  switch( pAdapter->eState)
    {
  case kStateClosed:
    return SWS_EACCES;

  case kStateOpen:
    return 0;

  case kStateClosing:
    return SWS_EACCES;

  case kStateCalling:
  case kStateLogin:
  case kStateConnected:
    P2pClose( pAdapter);
    return 0;

  case kStateListening:
    return 0;

  case kStateRinging:
  case kStateAccepted:
    P2pHangup( pAdapter);
    P2pListen( pAdapter, WAIT_NEVER);
    return 0;
    }

  return SWS_EACCES;
  }


/****************************************************************************
 * External client functions
 ****************************************************************************/

/*
 * Register a P2P adapter
 */
SWS_EXPORT( int, SWS_P2pAddAdapter(   /* Returns !0 on error */
  SWS_PTR( const SWS_P2pDevice) pDesc,/* IN: Device descriptor */
  SWS_PTR( void) pvContext,           /* IN: Context for dispatch functions */
  SWS_PTR( SWS_P2pCallbacks) pUp      /* OUT: Up calls */
)){
  SWS_SP2pAdapter* pAdapter;
  char szBuf[ 32];
  int iRet;

  TRACE( 9, ("SWS_P2pAddAdapter(%p,%p,%p)\n", pDesc,pvContext,pUp));
  ASSERT( NULL != pDesc);
  ASSERT( pDesc->pfnSend);
  ASSERT( pDesc->pfnRecv);
  ASSERT( NULL != pUp);

  if ( NULL == pDesc || NULL == pUp)
    return SWS_EFAULT;

  if ( sizeof(*pDesc) > pDesc->uSize)
    {
    TRACE( 0, ("!!! SWS_P2pAddAdapter descriptor size too small\n"));
    return SWS_EINVAL;
    }

  if ( SWS_P2P_VERSION != pDesc->uVersion)
    {
    TRACE( 0, ("!!! SWS_P2pAddAdapter version mismatch\n"));
    return SWS_EINVAL;
    }

  /* Add to adapter list */
  if ( s_uAdapters >= lengthof( s_adapterArray) )
    {
    TRACE( 0, ("!!! SWS_P2pAddAdapter too many adapters\n"));
    return SWS_ETOOMANYREFS;
    }
  pAdapter = &s_adapterArray[ s_uAdapters++];

  /* Init instance */
  memset( pAdapter, 0, sizeof( *pAdapter));
  strncpy( pAdapter->szIdent, pDesc->pszIdent, sizeof( pAdapter->szIdent) -1);

  pAdapter->pfnSend = pDesc->pfnSend;
  pAdapter->pfnRecv = pDesc->pfnRecv;
  pAdapter->pfnShutdown = pDesc->pfnShutdown;
  pAdapter->pfnIoctl = pDesc->pfnIoctl;
  pAdapter->pvContext = pvContext;

  pAdapter->rxRead = pAdapter->rxWrite = 0;
  pAdapter->eState = kStateClosed;
  AtomicWrite( &pAdapter->aRxDpc, 0);

  /* Alloc resources */
  pAdapter->hDcdDebounceTimer = SWS_TimerAlloc( &DcdDebounceTimeout, pAdapter);
  if ( NULL == pAdapter->hDcdDebounceTimer)
    {
    P2pFreeAdapter( pAdapter);
    --s_uAdapters;
    return SWS_ENOMEM;
    }

  pAdapter->hDsrDebounceTimer = SWS_TimerAlloc( &DsrDebounceTimeout, pAdapter);
  if ( NULL == pAdapter->hDsrDebounceTimer)
    {
    P2pFreeAdapter( pAdapter);
    --s_uAdapters;
    return SWS_ENOMEM;
    }

  pAdapter->hLoginTimer = SWS_TimerAlloc( &LoginTimeout, pAdapter);
  if ( NULL == pAdapter->hLoginTimer)
    {
    P2pFreeAdapter( pAdapter);
    --s_uAdapters;
    return SWS_ENOMEM;
    }

  pAdapter->hRxIdleTimer = SWS_TimerAlloc( &RxIdleTimeout, pAdapter);
  if ( NULL == pAdapter->hRxIdleTimer)
    {
    P2pFreeAdapter( pAdapter);
    --s_uAdapters;
    return SWS_ENOMEM;
    }

  /* Get type of modem */
  PlatformPropertyGetString(
    pAdapter->szIdent, PVAL_P2P_MODEM, "", szBuf, sizeof(szBuf) -1);
  if ( !SWS_stricmp( szBuf, "")
    || !SWS_stricmp( szBuf, kszModemNull)
  ) {
    pAdapter->eModem = kModemNone;
    }
  else if ( !SWS_stricmp( szBuf, kszModemHayes) )
    {
    pAdapter->eModem = kModemHayes;

    PlatformPropertyGetString(
      pAdapter->szIdent, PVAL_P2P_MODEMINIT, kModemUsrInit,
      pAdapter->szModemInit, sizeof(pAdapter->szModemInit) -1
    );

    pAdapter->uRings = (unsigned)PlatformPropertyGetUint(
      pAdapter->szIdent, PVAL_P2P_RINGS, kRings);
    }
  else if ( !SWS_stricmp( szBuf, kszModemMsDirect) )
    {
    pAdapter->eModem = kModemMsDirect;
    }
  else
    {
    TRACE( 0, ("!!! SWS_P2pAddAdapter bad modem type '%s'\n", szBuf));
    P2pFreeAdapter( pAdapter);
    --s_uAdapters;
    return SWS_EPROTOTYPE;
    }

  /* Init IP properties */
  pAdapter->uHwAddress = PlatformTime();  /* Random h/w address */
  pAdapter->uIpAddress = SWS_INADDR_ANY;  /* Unset */
  pAdapter->uSubnetMask = ~0UL;           /* None */
  pAdapter->uGatewayAddr = SWS_INADDR_NONE; /* None */

  pAdapter->uMagic = ADAPTER_MAGIC;

  /* Save adapter 'Up' calls */
  pUp->pvContext = pAdapter;
  pUp->pfnSendAck = &P2pSendAck;
  pUp->pfnRecvInd = &P2pRecvInd;
  pUp->pfnStatInd = &P2pStatInd;

  /* Register this datalink */
  iRet = SWS_DatalinkRegister( &s_p2p, pAdapter, &pAdapter->dlink);
  if ( iRet)
    {
    /* Cleanup on error */
    P2pFreeAdapter( pAdapter);
    --s_uAdapters;
    return iRet;
    }

  ASSERT( pAdapter->dlink.pfnSendRdy);
  ASSERT( pAdapter->dlink.pfnRecvInd);
  ASSERT( pAdapter->dlink.pfnLinkStatus);

  /* Indicate this link is down */
  (*pAdapter->dlink.pfnLinkStatus)( pAdapter->dlink.pvContext, 0);

  /* Get adapter status */
  P2pAdapterStatus( pAdapter);

  /* Is this a permanent connection? */
  if ( PlatformPropertyGetString(
    pAdapter->szIdent,
    PVAL_P2P_CONNECT, "",
    szBuf, sizeof(szBuf) -1
  )){
    /* Yes, start it */
    SWS_P2pStartLink( INDEX( pAdapter), szBuf);
    }

  return 0;
  }


/*
 * Destroy an adapter
 */
static SWS_FASTCALL( void, P2pFreeAdapter( SWS_SP2pAdapter* pAdapter))
  {
  TRACE( 9, ("P2pFreeAdapter(%p)\n", pAdapter));
  ASSERT( pAdapter);

  /* Release resources */
  if ( pAdapter->hDsrDebounceTimer)
    SWS_TimerRelease( pAdapter->hDsrDebounceTimer);

  if ( pAdapter->hDcdDebounceTimer)
    SWS_TimerRelease( pAdapter->hDcdDebounceTimer);

  if ( pAdapter->hRxIdleTimer)
    SWS_TimerRelease( pAdapter->hRxIdleTimer);

  if ( pAdapter->hLoginTimer)
    SWS_TimerRelease( pAdapter->hLoginTimer);

  memset( pAdapter, 0, sizeof( *pAdapter));
  }


/*
 * Get no. P2P adapters
 */
SWS_EXPORT( unsigned, SWS_P2pCount( void))
  {
  TRACE( 9, ("SWS_P2pCount()\n"));
  return s_uAdapters;
  }


/*
 * Get adapter name
 */
SWS_EXPORT( SWS_PTR( const char), SWS_P2pGetIdent(
  unsigned uAdapter                   /* IN: 0..SWS_P2pCount() -1 */
)){
  SWS_SP2pAdapter* pAdapter;

  TRACE( 9, ("SWS_P2pGetIdent(%u)\n", uAdapter));

  if ( uAdapter >= s_uAdapters)
    {
    TRACE( 0, ("!!! SWS_P2pGetIdent no such adapter %u\n", uAdapter));
    return NULL;
    }
  pAdapter = &s_adapterArray[ uAdapter];
  ASSERT( ADAPTER_MAGIC == pAdapter->uMagic);

  return pAdapter->szIdent;
  }


/*
 * Get an adapter's state
 */
SWS_EXPORT( int, SWS_P2pGetState(     /* Returns SWS_EP2pState */
  unsigned uAdapter                   /* IN: 0..SWS_P2pCount() -1 */
)){
  SWS_SP2pAdapter* pAdapter;

  TRACE( 9, ("SWS_P2pGetState(%u)\n", uAdapter));

  if ( uAdapter >= s_uAdapters)
    {
    TRACE( 0, ("!!! SWS_P2pGetState no such adapter %u\n", uAdapter));
    return -1;
    }
  pAdapter = &s_adapterArray[ uAdapter];
  ASSERT( ADAPTER_MAGIC == pAdapter->uMagic);

  switch( pAdapter->eState)
    {
  case kStateClosed:
  case kStateOpen:
  case kStateClosing:
    return SWS_kP2pStateClosed;

  case kStateCalling:
    return SWS_kP2pStateCalling;

  case kStateLogin:
    return SWS_kP2pStateLogin;

  case kStateConnected:
    return SWS_kP2pStateConnected;

  case kStateListening:
  case kStateRinging:
    return SWS_kP2pStateListening;

  case kStateAccepted:
    return SWS_kP2pStateConnected;
    }

  return SWS_kP2pStateClosed;
  }


/*
 * Start a client/server session
 */
SWS_EXPORT( int, SWS_P2pStart(        /* Returns !0 on error */
  unsigned uAdapter,                  /* IN: 0..SWS_P2pCount() -1 */
  SWS_PTR( const SWS_SP2pSession) pSess /* IN: Session properties */
)){
  SWS_SP2pAdapter* pAdapter;
  int iRet;

  TRACE( 9, ("SWS_P2pStart(%u,%p)\n", uAdapter,pSess));
  ASSERT( NULL != pSess);

  if ( NULL == pSess)
    return SWS_EFAULT;

  if ( uAdapter >= s_uAdapters)
    {
    TRACE( 0, ("!!! SWS_P2pStart no such adapter %u\n", uAdapter));
    return SWS_EINVAL;
    }
  pAdapter = &s_adapterArray[ uAdapter];
  ASSERT( ADAPTER_MAGIC == pAdapter->uMagic);

  if ( SWS_INADDR_NONE != pSess->uIpAddress
    && !SWS_IN_CLASSC( pSess->uIpAddress)
    && !SWS_IN_CLASSB( pSess->uIpAddress)
    && !SWS_IN_CLASSA( pSess->uIpAddress)
  ) {
    TRACE( 0, ("!!! SWS_P2pStart invalid IP address %#lx\n",
      pSess->uIpAddress));
    return SWS_EINVAL;
    }

  /* Claim the adapter */
  iRet = PlatformSynchronise( &P2pOpen, pAdapter);
  if ( iRet)
    return iRet;

  /* Save session info */
  pAdapter->user = pSess->user;
  if ( SWS_INADDR_NONE != pSess->uIpAddress)
    pAdapter->uIpAddress = pSess->uIpAddress;
  else
    pAdapter->uIpAddress = SWS_INADDR_ANY;
  pAdapter->uSubnetMask = pSess->uSubnetMask;
  pAdapter->uGatewayAddr = pSess->uGateway;
  pAdapter->uIdleTimeout = (ui32)pSess->uIdleTimeout * SWS_TIMER_TICKS;
#if OPT_P2P_VJCOMP
  pAdapter->uCompSlots = (ui16)(pSess->uCompSlots
    ? pSess->uCompSlots : SWS_kP2pProtocolPpp == pSess->protocol ? 15U : 1U);
#endif

  switch ( pSess->auth)
    {
  case SWS_kP2pAuthNone:
    pAdapter->auth = 0;
    break;
#if OPT_PPP
  case SWS_kP2pAuthPap:
    pAdapter->auth = SWS_PPP_PAP;
    break;
  case SWS_kP2pAuthChap:
    pAdapter->auth = SWS_PPP_CHAP;
    break;
#endif
  default:
    TRACE( 0, ("!!! SWS_P2pStart invalid authenication %u\n",
      pSess->auth));
    return SWS_EINVAL;
    }

  pAdapter->pfnEvent = pSess->pfnEvent ? pSess->pfnEvent : &P2pEvent;
  pAdapter->pvEvent = pSess->pvEvent;

  /* P2pBind the protocol */
  iRet = P2pBind( pAdapter, pSess->protocol);
  if ( iRet)
    {
    pAdapter->eState = kStateClosed;
    pAdapter->pfnEvent = 0;
    pAdapter->uIpAddress = SWS_INADDR_ANY;  /* Unset */
    pAdapter->uSubnetMask = ~0UL;           /* None */
    pAdapter->uGatewayAddr = SWS_INADDR_NONE; /* None */
    return iRet;
    }

  /* Dialup client? */
  if ( pSess->szServer[0] )
    {
    /* Yes, start the modem calling */
    iRet = P2pCall( pAdapter, pSess->szServer, MS2TICKS( pSess->uConnectTimeout));
    }
  else
    {
    /* Start the modem listening */
    iRet = P2pListen( pAdapter, MS2TICKS( pSess->uConnectTimeout));
    }

  if ( iRet > 0)
    {
    pAdapter->pfnEvent = &P2pEvent;
    P2pClose( pAdapter);
    }

  return iRet;
  }


/*
 * Start a named session
 */
SWS_EXPORT( int, SWS_P2pStartLink( /* Returns !0 on error */
  unsigned uAdapter,                  /* IN: 0..SWS_P2pCount() -1 */
  SWS_PTR( const char) pszSess        /* IN: Session */
)){
  SWS_SP2pSession session;
  int iRet;

  TRACE( 9, ("SWS_P2pStartLink(%u,%s)\n", uAdapter,pszSess));
  ASSERT( NULL != pszSess);

  if ( uAdapter >= s_uAdapters)
    {
    TRACE( 0, ("!!! SWS_P2pStartLink no such adapter %u\n", uAdapter));
    return SWS_EINVAL;
    }

  if ( NULL == pszSess)
    return SWS_EFAULT;

  /* Get session setup */
  iRet = P2pSession( pszSess, &session);
  if ( iRet)
    return iRet;

  TRACE( 2, ("SWS_P2pStartLink %s\n", pszSess));
  return SWS_P2pStart( uAdapter, &session);
  }


/*
 * Stop a client/server
 */
SWS_EXPORT( int, SWS_P2pStop(         /* Returns !0 on error */
  unsigned uAdapter                   /* IN: 0..SWS_P2pCount() -1 */
)){
  SWS_SP2pAdapter* pAdapter;

  TRACE( 9, ("SWS_P2pStop(%u)\n",uAdapter));

  if ( uAdapter >= s_uAdapters)
    {
    TRACE( 0, ("!!! SWS_P2pStop no such adapter %u\n", uAdapter));
    return SWS_EINVAL;
    }
  pAdapter = &s_adapterArray[ uAdapter];
  ASSERT( ADAPTER_MAGIC == pAdapter->uMagic);

  /* Close the current connection */
  return P2pClose( pAdapter);
  }


/*
 * Startup a dial on demand adapter
 */
SWS_EXPORT( int, SWS_P2pDialOnDemand( /* Returns 0 if connection established */
  ui32 uDst                           /* IN: IP address to reach */
)){
  SWS_SP2pAdapter* pAdapter;
  unsigned uAdapter;
  int iRet = SWS_ENODEV;
  (void)uDst;

  TRACE( 9, ("SWS_P2pDialOnDemand(%lu)\n",uDst));

  for ( pAdapter = &s_adapterArray[ uAdapter = 0];
    uAdapter < s_uAdapters;
    ++uAdapter, ++pAdapter
  ) {
    char szBuf[ 32];

    /* Is this a 'dial on demand' connection? */
    if ( PlatformPropertyGetString(
        pAdapter->szIdent,
        PVAL_P2P_DEMAND, "",
        szBuf, sizeof(szBuf) -1
      )
    ) {
      SWS_SP2pSession session;

      /* Get session setup */
      if ( 0 == P2pSession( szBuf, &session))
        {
        /* TODO: Compare gateway with uDst */

        if ( 0 == session.uConnectTimeout)
          session.uConnectTimeout = kConnectTimeout;

        TRACE( 2, ("SWS_P2pDialOnDemand() %s => %s\n",
          pAdapter->szIdent, szBuf));

        if ( 0 == SWS_P2pStart( uAdapter, &session))
          iRet = 0;
        }
      }
    }

  return iRet;
  }


/*
 * Get session user info
 */
SWS_EXPORT( int, SWS_P2pGetAcct(      /* Returns !0 on error */
  unsigned uAdapter,                  /* IN: 0..SWS_P2pCount() -1 */
  SWS_PTR( SWS_SP2pAcct) pUsr         /* OUT: Logged in user */
)){
  SWS_SP2pAdapter* pAdapter;

  TRACE( 9, ("SWS_P2pGetAcct(%u,%p)\n",uAdapter,pUsr));
  ASSERT( NULL != pUsr);

  if ( NULL == pUsr)
    return SWS_EFAULT;

  if ( uAdapter >= s_uAdapters)
    {
    TRACE( 0, ("!!! SWS_P2pGetAcct no such adapter %u\n", uAdapter));
    return SWS_EINVAL;
    }
  pAdapter = &s_adapterArray[ uAdapter];
  ASSERT( ADAPTER_MAGIC == pAdapter->uMagic);

  switch( pAdapter->eState)
    {
  case kStateClosed:
  case kStateClosing:
    TRACE( 1, ("*** SWS_P2pGetAcct(%u) not connected\n",uAdapter));
    return SWS_ENOTCONN;

  case kStateOpen:
  case kStateListening:
  case kStateRinging:
  case kStateCalling:
  case kStateLogin:
  case kStateConnected:
  case kStateAccepted:
    *pUsr = pAdapter->user;
    break;
    }

  return 0;
  }


/*
 * Update session user info
 */
SWS_EXPORT( int, SWS_P2pSetAcct(      /* Returns !0 on error */
  unsigned uAdapter,                  /* IN: 0..SWS_P2pCount() -1 */
  SWS_PTR( SWS_SP2pAcct) pUsr         /* I/O */
)){
  SWS_SP2pAdapter* pAdapter;

  TRACE( 9, ("SWS_P2pSetAcct(%u,%p)\n",uAdapter,pUsr));
  ASSERT( NULL != pUsr);

  if ( NULL == pUsr)
    return SWS_EFAULT;

  if ( uAdapter >= s_uAdapters)
    {
    TRACE( 0, ("!!! SWS_P2pSetAcct no such adapter %u\n", uAdapter));
    return SWS_EINVAL;
    }
  pAdapter = &s_adapterArray[ uAdapter];
  ASSERT( ADAPTER_MAGIC == pAdapter->uMagic);

  if ( '\0' == pUsr->szName[0])
    strcpy( pUsr->szName, "guest");

  PlatformPropertyGetString(
    PSECT_P2P_USERS, pUsr->szName, " ",
    pUsr->szPassword, sizeof(pUsr->szPassword) -1
  );
  if ( strcmp( pUsr->szPassword, " "))
    {
    TRACE( 2, ("SWS_P2pSetAcct(%s) %s\n",pAdapter->szIdent, pUsr->szName));
    pAdapter->user = *pUsr;
    return 0;
    }

  return SWS_EINVAL;
  }


/*
 * Get IP address, subnet mask & gateway
 */
SWS_EXPORT( ui32, SWS_P2pGetIpAddress(
  unsigned uAdapter                   /* IN: 0..SWS_P2pCount() -1 */
)){
  SWS_SP2pAdapter* pAdapter;

  TRACE( 9, ("SWS_P2pGetIpAddress(%u)\n",uAdapter));

  if ( uAdapter >= s_uAdapters)
    {
    TRACE( 0, ("!!! SWS_P2pGetIpAddress no such adapter %u\n", uAdapter));
    return SWS_INADDR_NONE;
    }
  pAdapter = &s_adapterArray[ uAdapter];
  ASSERT( ADAPTER_MAGIC == pAdapter->uMagic);

  return pAdapter->uIpAddress;
  }


SWS_EXPORT( ui32, SWS_P2pGetSubnetMask(
  unsigned uAdapter                   /* IN: 0..SWS_P2pCount() -1 */
)){
  SWS_SP2pAdapter* pAdapter;

  TRACE( 9, ("SWS_P2pGetSubnetMask(%u)\n",uAdapter));

  if ( uAdapter >= s_uAdapters)
    {
    TRACE( 0, ("!!! SWS_P2pGetSubnetMask no such adapter %u\n", uAdapter));
    return 0;
    }
  pAdapter = &s_adapterArray[ uAdapter];
  ASSERT( ADAPTER_MAGIC == pAdapter->uMagic);

  return pAdapter->uSubnetMask;
  }


SWS_EXPORT( ui32, SWS_P2pGetGateway(
  unsigned uAdapter                   /* IN: 0..SWS_P2pCount() -1 */
)){
  SWS_SP2pAdapter* pAdapter;

  TRACE( 9, ("SWS_P2pGetGateway(%u)\n",uAdapter));

  if ( uAdapter >= s_uAdapters)
    {
    TRACE( 0, ("!!! SWS_P2pGetGateway no such adapter %u\n", uAdapter));
    return SWS_INADDR_NONE;
    }
  pAdapter = &s_adapterArray[ uAdapter];
  ASSERT( ADAPTER_MAGIC == pAdapter->uMagic);

  return pAdapter->uGatewayAddr;
  }


/*
 * Get authentication protocol
 */
SWS_EXPORT( ui16, SWS_P2pGetAuth(     /* Returns 0= none */
  unsigned uAdapter                   /* IN: 0..SWS_P2pCount() -1 */
)){
  SWS_SP2pAdapter* pAdapter;

  TRACE( 9, ("SWS_P2pGetAuth(%u)\n",uAdapter));

  if ( uAdapter >= s_uAdapters)
    {
    TRACE( 0, ("!!! SWS_P2pGetAuth no such adapter %u\n", uAdapter));
    return 0;
    }
  pAdapter = &s_adapterArray[ uAdapter];
  ASSERT( ADAPTER_MAGIC == pAdapter->uMagic);

  return pAdapter->auth;
  }


#if OPT_P2P_VJCOMP
/*****************************************************************************
 * Van Jacobsen TCP/IP header compression RFC 1144
 ****************************************************************************/

/* Change flags */
#define VJ_NEW_C 0x40 /* New compression slot */
#define VJ_NEW_I 0x20 /* IP id */
#define VJ_NEW_P 0x10 /* TCP push */
#define VJ_NEW_S 0x08 /* Seq */
#define VJ_NEW_A 0x04 /* Ack no. */
#define VJ_NEW_W 0x02 /* Window */
#define VJ_NEW_U 0x01 /* Urgent ptr */

#define VJ_SPECIAL_I (VJ_NEW_S | VJ_NEW_W | VJ_NEW_U) /* Interactive echo */
#define VJ_SPECIAL_D (VJ_NEW_S | VJ_NEW_A | VJ_NEW_W | VJ_NEW_U) /* Uni data */
#define VJ_SPECIAL_MASK (VJ_NEW_S | VJ_NEW_A | VJ_NEW_W | VJ_NEW_U)

/* Compression slot */
typedef struct SVjSlot
  {
  SDeque link;                        /* LRU list */
  unsigned uHdrLen;                   /* Header length */
  char hdr[ SWS_IPHDR_MAXLEN + SWS_TCP_MAXLEN]; /* Max len TCP/IP header */
  } SVjSlot;

/* Convert an SDeque* to the containing SVjSlot* */
#define DEQ2SLOT( p) SWS_CONTAINER( p, SVjSlot, link)


/* (De)Compressor instance */
typedef struct SWS_SP2pVjComp
  {
  ui32 uMagic;
#define VJCOMP_MAGIC SWS_MAKEFOURCC( 'v','j','c','p')

  ui8 uMaxSlotId, uCompSlotId;

  /* Decomp */
  int isResyncing;

  /* Comp */
  SDeque lru;                         /* LRU list head */
  unsigned uLastSlot;                 /* Last used slot id */

  SVjSlot slot[ 1];                   /* VLA: 0..uMaxSlotId */
  } SWS_SP2pVjComp;


/*
 * Encode a value
 */
static unsigned VJ_ENCODE(
  ui16 uVal,
  SWS_PTR( void) pvHdr,
  unsigned uIndex
) {
  if ( uVal > 0 && uVal <= 255)
    {
    SWS_POKE8( pvHdr, uIndex, (ui8)uVal);
    ++uIndex;
    }
  else
    {
    SWS_POKE8( pvHdr, uIndex, 0);
    ++uIndex;
    SWS_POKE16( pvHdr, uIndex, uVal);
    uIndex += 2;
    }

  return uIndex;
  }


/*
 * Decode a compressed value
 */
static SWS_FASTCALL( unsigned, VJ_DECODE(
  SWS_PTR( const void) pvHdr,
  unsigned* puIndex
)){
  unsigned u, uIndex = *puIndex;

  SWS_PEEK8( u, pvHdr, uIndex);
  ++uIndex;
  if ( 0 == u)
    {
    SWS_PEEK16( u, pvHdr, uIndex);
    uIndex += 2;
    }

  *puIndex = uIndex;
  return u;
  }


/*
 * Get VJ compression slots
 */
SWS_EXPORT( ui8, SWS_P2pVjSlots(      /* 0= disabled */
  unsigned uAdapter                   /* IN: 0..SWS_P2pCount() -1 */
)){
  SWS_SP2pAdapter* pAdapter;

  TRACE( 9, ("SWS_P2pVjSlots(%u)\n",uAdapter));

  if ( uAdapter >= s_uAdapters)
    {
    TRACE( 0, ("!!! SWS_P2pVjSlots no such adapter %u\n", uAdapter));
    return 0;
    }
  pAdapter = &s_adapterArray[ uAdapter];
  ASSERT( ADAPTER_MAGIC == pAdapter->uMagic);

#if OPT_P2P_VJCOMP
  return (ui8)(pAdapter->uCompSlots <= 255 ? pAdapter->uCompSlots : 0);
#else
  return 0;
#endif
  }


/*
 * Create a (de)compressor
 */
SWS_EXPORT( SWS_SP2pVjCompH, SWS_P2pVjOpen(
  ui8 uMaxSlotId,                     /* IN: Max compression slot index */
  ui8 uCompSlotId                     /* IN: !0 = Compress slot index */
)){
  SWS_SP2pVjComp* pComp;

  TRACE( 9, ("SWS_P2pVjOpen(%u,%u)\n", uMaxSlotId, uCompSlotId));

  pComp = (SWS_SP2pVjComp*) malloc(
    sizeof(*pComp) + uMaxSlotId * sizeof(pComp->slot));
  if ( NULL != pComp)
    {
    unsigned u;
    SVjSlot* pSlot;

    pComp->uMagic = VJCOMP_MAGIC;
    pComp->uMaxSlotId = uMaxSlotId;
    pComp->uCompSlotId = uCompSlotId;
    pComp->isResyncing = 0;
    DEQUE_INIT( &pComp->lru);
    pComp->uLastSlot = ~0U;

    /* Init the slots */
    for (
      pSlot = &pComp->slot[ u = 0];
      u <= uMaxSlotId;
      ++u, ++pSlot
    ) {
      DEQUE_INIT( &pSlot->link);
      SWS_DequeAppend( &pComp->lru, &pSlot->link);
      pSlot->uHdrLen = 0;
      }
    }

  return pComp;
  }


/*
 * Close a compressor
 */
SWS_EXPORT( void, SWS_P2pVjClose(
  SWS_SP2pVjCompH hComp               /* IN: Compressor context */
)){
  TRACE( 9, ("SWS_P2pVjClose(%p)\n",hComp));

  if ( NULL != hComp)
    {
    ASSERT( VJCOMP_MAGIC == hComp->uMagic);
    hComp->uMagic = ~VJCOMP_MAGIC;

    free( hComp);
    }
  }


/*
 * Compress an IP packet
 */
SWS_EXPORT( SWS_EP2pVjType, SWS_P2pVjCompress(
  SWS_SP2pVjCompH hComp,              /* IN: Compressor context */
  SWS_BufferH hBuf
)){
  SWS_PTR( void) pvHdr;
  SVjSlot* pSlot;
  SDeque* pItem;
  unsigned u, uIpHdrLen, uHdrLen, uSrc, uDst;
  ui32 ulVal, ulSrc, ulDst;
  SWS_size_t len;

  TRACE( 9, ("SWS_P2pVjCompress(%p,%p)\n",hComp,hBuf));
  ASSERT( NULL != hComp && VJCOMP_MAGIC == hComp->uMagic);

  /* Don't compress short frames */
  len = SWS_BufferSize( hBuf);
  if ( len < SWS_IPHDR_MINLEN + SWS_TCP_MINLEN)
    return SWS_kP2pVjTypeIP;

  /* Get IP header */
  pvHdr = SWS_BufferGetFragment( hBuf, 0, &len);
  if ( len < SWS_IPHDR_MINLEN + SWS_TCP_MINLEN)
    return SWS_kP2pVjTypeIP;

  /* Don't compress an IP fragment */
  SWS_PEEK16( u, pvHdr, SWS_IPHDR_FLAGS_OFFSET);
  if ( u & (SWS_IP_OFFSET_MASK | SWS_IP_FLAG_MF))
    return SWS_kP2pVjTypeIP;

  /* Only compress TCP */
  SWS_PEEK8( u, pvHdr, SWS_IPHDR_PROTOCOL);
  if ( SWS_IPPROTO_TCP != u)
    return SWS_kP2pVjTypeIP;

  SWS_PEEK8( uIpHdrLen, pvHdr, SWS_IPHDR_VER_IHL);
  uIpHdrLen = (uIpHdrLen & 0xf) * 4;
  SWS_PEEK16( u, pvHdr, uIpHdrLen + SWS_TCP_FLG);
  uHdrLen = uIpHdrLen + 4 * (u >> 12);

  /* All header must be accessible */
  if ( uHdrLen > len)
    return SWS_kP2pVjTypeIP;

  /* Are the TCP flags OK for compressing */
  SWS_PEEK16( u, pvHdr, uIpHdrLen + SWS_TCP_FLG);
  if ( SWS_TCP_ACKF != (u & (SWS_TCP_SYNF | SWS_TCP_FINF | SWS_TCP_RSTF | SWS_TCP_ACKF)) )
    return SWS_kP2pVjTypeIP;

  SWS_PEEK32( ulSrc, pvHdr, SWS_IPHDR_SRC);
  SWS_PEEK32( ulDst, pvHdr, SWS_IPHDR_DST);
  SWS_PEEK16( uSrc, pvHdr, uIpHdrLen + SWS_TCP_SRC);
  SWS_PEEK16( uDst, pvHdr, uIpHdrLen + SWS_TCP_DST);

  /* Search for matching slot in LRU order */
  pSlot = NULL;
  for (
    pItem = DEQUE_FIRST( &hComp->lru);
    pItem != &hComp->lru;
    pItem = DEQUE_FIRST( pItem)
  ) {
    pSlot = DEQ2SLOT( pItem);

    if ( 0 == pSlot->uHdrLen)
      break; /* Unused slot */

    /* Matching header lengths */
    if ( uHdrLen == pSlot->uHdrLen)
      {
      /* Match src & dst IP addresses */
      SWS_PEEK32( ulVal, pSlot->hdr, SWS_IPHDR_SRC);
      if ( ulVal == ulSrc)
        {
        SWS_PEEK32( ulVal, pSlot->hdr, SWS_IPHDR_DST);
        if ( ulVal == ulDst)
          {
          /* Match src & dst ports */
          SWS_PEEK16( u, pSlot->hdr, uIpHdrLen + SWS_TCP_SRC);
          if ( u == uSrc)
            {
            SWS_PEEK16( u, pSlot->hdr, uIpHdrLen + SWS_TCP_DST);
            if ( u == uDst)
              break; /* matched */
            }
          }
        }
      }
    }

  /* Slot found? */
  if ( pItem == &hComp->lru)
    {
    /* No, re-use oldest */
    pItem = DEQUE_LAST( &hComp->lru);
    pSlot = DEQ2SLOT( pItem);
    pSlot->uHdrLen = 0;
    }

  /* Move item to head of LRU list */
  if ( pItem != DEQUE_FIRST( &hComp->lru))
    {
    SWS_DequeRemove( pItem);
    SWS_DequePrepend( &hComp->lru, pItem);
    }

  /* Match found? */
  if ( pSlot->uHdrLen > 0) do
    {
    /* Yes, check for compressible changes */
    unsigned uChanges, uHdrOffset;
    ui8 hdr[ 16];
    ui32 ulDeltaS, ulDeltaA;

    /* IP version, IHL & TOS */
    SWS_PEEK16( uSrc, pSlot->hdr, SWS_IPHDR_VER_IHL);
    SWS_PEEK16( uDst, pvHdr, SWS_IPHDR_VER_IHL);
    if ( uSrc != uDst)
      break;

    /* Flags & fragment offset */
    SWS_PEEK16( uSrc, pSlot->hdr, SWS_IPHDR_FLAGS_OFFSET);
    SWS_PEEK16( uDst, pvHdr, SWS_IPHDR_FLAGS_OFFSET);
    if ( uSrc != uDst)
      break;

    /* TTL & protocol */
    SWS_PEEK16( uSrc, pSlot->hdr, SWS_IPHDR_TTL);
    SWS_PEEK16( uDst, pvHdr, SWS_IPHDR_TTL);
    if ( uSrc != uDst)
      break;

    /* IP options */
    if ( uIpHdrLen > SWS_IPHDR_OPTIONS
      && memcmp(
        (ui8*)pSlot->hdr + SWS_IPHDR_OPTIONS,
        (ui8*)pvHdr + SWS_IPHDR_OPTIONS,
        uIpHdrLen - SWS_IPHDR_OPTIONS
      )
    ) {
      break;
      }

    /* TCP options */
    if ( uHdrLen - uIpHdrLen > SWS_TCP_OPT
      && memcmp(
        (ui8*)pSlot->hdr + uIpHdrLen + SWS_TCP_OPT,
        (ui8*)pvHdr + uIpHdrLen + SWS_TCP_OPT,
        uHdrLen - uIpHdrLen - SWS_TCP_OPT
      )
    ) {
      break;
      }

    /* Detect & encode changes */
    uChanges = 0;
    uHdrOffset = 0;

    SWS_PEEK16( uSrc, pvHdr, uIpHdrLen + SWS_TCP_URG);
    SWS_PEEK16( u, pvHdr, uIpHdrLen + SWS_TCP_FLG);
    if ( u & SWS_TCP_URGF)
      {
      uHdrOffset = VJ_ENCODE( (ui16)uSrc, hdr, uHdrOffset);
      uChanges |= VJ_NEW_U;
      }
    else
      {
      SWS_PEEK16( u, pSlot->hdr, uIpHdrLen + SWS_TCP_URG);
      if ( u != uSrc)
        {
        /* !! URG changed but flag not set */
        break;
        }
      }

    SWS_PEEK16( u, pvHdr, uIpHdrLen + SWS_TCP_WND);
    SWS_PEEK16( uSrc, pSlot->hdr, uIpHdrLen + SWS_TCP_WND);
    u -= uSrc;
    if ( u != 0)
      {
      uHdrOffset = VJ_ENCODE( (ui16)u, hdr, uHdrOffset);
      uChanges |= VJ_NEW_W;
      }

    SWS_PEEK32( ulDeltaA, pvHdr, uIpHdrLen + SWS_TCP_ACK);
    SWS_PEEK32( ulSrc, pSlot->hdr, uIpHdrLen + SWS_TCP_ACK);
    ulDeltaA -= ulSrc;
    if ( ulDeltaA > 0xffff)
      break;
    if ( ulDeltaA != 0)
      {
      uHdrOffset = VJ_ENCODE( (ui16)ulDeltaA, hdr, uHdrOffset);
      uChanges |= VJ_NEW_A;
      }

    SWS_PEEK32( ulDeltaS, pvHdr, uIpHdrLen + SWS_TCP_SEQ);
    SWS_PEEK32( ulSrc, pSlot->hdr, uIpHdrLen + SWS_TCP_SEQ);
    ulDeltaS -= ulSrc;
    if ( ulDeltaS > 0xffff)
      break;
    if ( ulDeltaS != 0)
      {
      uHdrOffset = VJ_ENCODE( (ui16)ulDeltaS, hdr, uHdrOffset);
      uChanges |= VJ_NEW_S;
      }

    switch ( uChanges)
      {
    case 0:
      /* If the packet contains data and last didn't
       * then it's probably normal, otherwise it's
       * possibly a retransmit in which case send
       * it uncompressed */
      SWS_PEEK16( u, pvHdr, SWS_IPHDR_TOTAL);
      SWS_PEEK16( uSrc, pSlot->hdr, SWS_IPHDR_TOTAL);
      if ( u == uSrc || uSrc != uHdrLen)
        continue;
      break;

    case VJ_SPECIAL_D:
    case VJ_SPECIAL_I:
      /* Special case flags value so send uncompressed */
      continue;

    case (VJ_NEW_S | VJ_NEW_A):
      if ( ulDeltaS == ulDeltaA)
        {
        SWS_PEEK16( uSrc, pSlot->hdr, SWS_IPHDR_TOTAL);
        if ( ulDeltaS == uSrc - uHdrLen)
          {
          /* Special case for interactive traffic */
          uChanges = VJ_SPECIAL_I;
          uHdrOffset = 0;
          }
        }
      break;

    case VJ_NEW_S:
      SWS_PEEK16( uSrc, pSlot->hdr, SWS_IPHDR_TOTAL);
      if ( ulDeltaS == uSrc - uHdrLen)
        {
        /* Special case for data xfer */
        uChanges = VJ_SPECIAL_D;
        uHdrOffset = 0;
        }
      break;

    default:
      break;
      }

    /* IP header ID */
    SWS_PEEK16( uSrc, pSlot->hdr, SWS_IPHDR_ID);
    SWS_PEEK16( u, pvHdr, SWS_IPHDR_ID);
    u -= uSrc;
    if ( 1 != u)
      {
      uHdrOffset = VJ_ENCODE( (ui16)u, hdr, uHdrOffset);
      uChanges |= VJ_NEW_I;
      }

    /* TCP PSH bit */
    SWS_PEEK16( u, pvHdr, uIpHdrLen + SWS_TCP_FLG);
    if ( u & SWS_TCP_PSHF)
      uChanges |= VJ_NEW_P;

    /* Get TCP checksum */
    SWS_PEEK16( uSrc, pvHdr, uIpHdrLen + SWS_TCP_SUM);

    /* Save new header in slot */
    ASSERT( pSlot->uHdrLen == uHdrLen);
    memcpy( pSlot->hdr, pvHdr, uHdrLen);

    /* Need to send the compression slot ID? */
    u  = (unsigned)(pSlot - hComp->slot);
    if ( 0 == hComp->uCompSlotId
      || hComp->uLastSlot != u
    ) {
      /* Yes */
      hComp->uLastSlot = u;

      /* Discard original header leaving space for compressed one */
      ASSERT( uHdrLen >= uHdrOffset + 4);
      SWS_BufferRemoveFront( hBuf, uHdrLen - uHdrOffset - 4);
      pvHdr = SWS_BufferGetFragment( hBuf, 0, &len);
      ASSERT( len >= 4);

      /* Prepend compressed header */
      uChanges |= VJ_NEW_C;
      SWS_POKE8( pvHdr, 0, (ui8)uChanges);
      SWS_POKE8( pvHdr, 1, (ui8)u);
      SWS_POKE16( pvHdr, 2, (ui16)uSrc); /* TCP csum */
      SWS_BufferCopyFromMemory( hBuf, 4, hdr, uHdrOffset);
      }
    else
      {
      /* Discard original header leaving space for compressed one */
      ASSERT( uHdrLen >= uHdrOffset + 3);
      SWS_BufferRemoveFront( hBuf, uHdrLen - uHdrOffset - 3);
      pvHdr = SWS_BufferGetFragment( hBuf, 0, &len);
      ASSERT( len >= 3);

      /* Prepend compressed header */
      SWS_POKE8( pvHdr, 0, (ui8)uChanges);
      SWS_POKE16( pvHdr, 1, (ui16)uSrc); /* TCP csum */
      SWS_BufferCopyFromMemory( hBuf, 3, hdr, uHdrOffset);
      }

    return SWS_kP2pVjTypeCompressed;
    }
  while ( 0);

  /* Not compressible so save new header */
  ASSERT( uHdrLen <= sizeof( pSlot->hdr));
  memcpy( pSlot->hdr, pvHdr, uHdrLen);
  pSlot->uHdrLen = uHdrLen;

  /* Send new slot ID */
  u = (unsigned)(pSlot - hComp->slot);
  hComp->uLastSlot = u;
  SWS_POKE8( pvHdr, SWS_IPHDR_PROTOCOL, (ui8)u);

  /* Send 'uncompressed' type */
  return SWS_kP2pVjTypeUncompressed;
  }


/*
 * Decompress an IP packet
 */
SWS_EXPORT( int, SWS_P2pVjDecompress(
  SWS_SP2pVjCompH hComp,              /* IN: Compressor context */
  SWS_EP2pVjType eType,               /* IN: Type */
  SWS_BufferH hBuf
)){
  SWS_PTR( void) pvHdr;
  SVjSlot* pSlot;
  unsigned u, uIpHdrLen, uChanges, uHdrOffset, uVal;
  ui32 ulVal;
  SWS_size_t len;
  int iRet;

  TRACE( 9, ("SWS_P2pVjDecompress(%p,%d,%p)\n",hComp,eType,hBuf));
  ASSERT( NULL != hComp && VJCOMP_MAGIC == hComp->uMagic);

  switch ( eType)
    {
  default:
    TRACE( 1, ("*** SWS_P2pVjDecompress() Bad type\n"));

  case SWS_kP2pVjTypeError:
    hComp->isResyncing = 1;
    return SWS_EINVAL;

  case SWS_kP2pVjTypeIP:
    return 0;

  case SWS_kP2pVjTypeUncompressed:
    /* Get IP header */
    pvHdr = SWS_BufferGetFragment( hBuf, 0, &len);

    /* Get compression slot */
    SWS_PEEK8( u, pvHdr, SWS_IPHDR_PROTOCOL);
    if ( u > hComp->uMaxSlotId)
      {
      TRACE( 1, ("*** SWS_P2pVjDecompress() Invalid new slot\n"));
      hComp->isResyncing = 1;
      return SWS_EINVAL;
      }

    hComp->uLastSlot = u;
    pSlot = &hComp->slot[ u];
    hComp->isResyncing = 0;

    /* Restore IP protocol field */
    SWS_POKE8( pvHdr, SWS_IPHDR_PROTOCOL, SWS_IPPROTO_TCP);

    /* Calc combined TCP/IP header len */
    SWS_PEEK8( uIpHdrLen, pvHdr, SWS_IPHDR_VER_IHL);
    uIpHdrLen = (uIpHdrLen & 0xf) * 4;
    SWS_PEEK16( u, pvHdr, uIpHdrLen + SWS_TCP_FLG);
    uIpHdrLen += 4 * (u >> 12);

    /* Save TCP/IP header */
    ASSERT( uIpHdrLen <= sizeof( pSlot->hdr));
    memcpy( pSlot->hdr, pvHdr, uIpHdrLen);
    pSlot->uHdrLen = uIpHdrLen;

    return 0;

  case SWS_kP2pVjTypeCompressed:
    pvHdr = SWS_BufferGetFragment( hBuf, 0, &len);
    break;
    }

  /* This is a compressed packet */
  uHdrOffset = 1;
  SWS_PEEK8( uChanges, pvHdr, 0);
  if ( uChanges & VJ_NEW_C)
    {
    /* Get state index */
    SWS_PEEK8( u, pvHdr, uHdrOffset);
    if ( u > hComp->uMaxSlotId)
      {
      TRACE( 1, ("*** SWS_P2pVjDecompress() Invalid slot\n"));
      hComp->isResyncing = 1;
      return SWS_EINVAL;
      }

    ++uHdrOffset;
    hComp->isResyncing = 0;
    hComp->uLastSlot = u;
    }
  else if ( hComp->isResyncing)
    {
    TRACE( 1, ("*** SWS_P2pVjDecompress() Resyncing after error\n"));
    return SWS_EINVAL;
    }

  ASSERT( hComp->uLastSlot <= hComp->uMaxSlotId);
  pSlot = &hComp->slot[ hComp->uLastSlot];

  SWS_PEEK8( uIpHdrLen, pSlot->hdr, SWS_IPHDR_VER_IHL);
  uIpHdrLen = (uIpHdrLen & 0xf) * 4;

  /* Update TCP checksum */
  SWS_PEEK16( u, pvHdr, uHdrOffset);
  uHdrOffset += 2;
  SWS_POKE16( pSlot->hdr, uIpHdrLen + SWS_TCP_SUM, (ui16)u);

  /* Get PSH flag */
  SWS_PEEK16( u, pSlot->hdr, uIpHdrLen + SWS_TCP_FLG);
  if ( uChanges & VJ_NEW_P)
    u |= SWS_TCP_PSHF;
  else
    u &= ~SWS_TCP_PSHF;
  SWS_POKE16( pSlot->hdr, uIpHdrLen + SWS_TCP_FLG, (ui16)u);

  switch ( uChanges & VJ_SPECIAL_MASK)
    {
  case VJ_SPECIAL_I:
    SWS_PEEK16( u, pSlot->hdr, SWS_IPHDR_TOTAL);
    u -= pSlot->uHdrLen;

    SWS_PEEK32( ulVal, pSlot->hdr, uIpHdrLen + SWS_TCP_SEQ);
    ulVal += u;
    SWS_POKE32( pSlot->hdr, uIpHdrLen + SWS_TCP_SEQ, ulVal);

    SWS_PEEK32( ulVal, pSlot->hdr, uIpHdrLen + SWS_TCP_ACK);
    ulVal += u;
    SWS_POKE32( pSlot->hdr, uIpHdrLen + SWS_TCP_ACK, ulVal);
    break;

  case VJ_SPECIAL_D:
    SWS_PEEK16( u, pSlot->hdr, SWS_IPHDR_TOTAL);
    u -= pSlot->uHdrLen;
    SWS_PEEK32( ulVal, pSlot->hdr, uIpHdrLen + SWS_TCP_SEQ);
    ulVal += u;
    SWS_POKE32( pSlot->hdr, uIpHdrLen + SWS_TCP_SEQ, ulVal);
    break;

  default:
    SWS_PEEK16( u, pSlot->hdr, uIpHdrLen + SWS_TCP_FLG);
    if ( uChanges & VJ_NEW_U)
      {
      SWS_PEEK16( uVal, pSlot->hdr, uIpHdrLen + SWS_TCP_URG);
      uVal += VJ_DECODE( pvHdr, &uHdrOffset);
      SWS_POKE16( pSlot->hdr, uIpHdrLen + SWS_TCP_URG, (ui16)uVal);
      u |= SWS_TCP_URGF;
      }
    else
      u &= ~SWS_TCP_URGF;
    SWS_POKE16( pSlot->hdr, uIpHdrLen + SWS_TCP_FLG, (ui16)u);

    if ( uChanges & VJ_NEW_W)
      {
      SWS_PEEK16( uVal, pSlot->hdr, uIpHdrLen + SWS_TCP_WND);
      uVal += VJ_DECODE( pvHdr, &uHdrOffset);
      SWS_POKE16( pSlot->hdr, uIpHdrLen + SWS_TCP_WND, (ui16)uVal);
      }

    if ( uChanges & VJ_NEW_A)
      {
      SWS_PEEK32( ulVal, pSlot->hdr, uIpHdrLen + SWS_TCP_ACK);
      ulVal += VJ_DECODE( pvHdr, &uHdrOffset);
      SWS_POKE32( pSlot->hdr, uIpHdrLen + SWS_TCP_ACK, ulVal);
      }

    if ( uChanges & VJ_NEW_S)
      {
      SWS_PEEK32( ulVal, pSlot->hdr, uIpHdrLen + SWS_TCP_SEQ);
      ulVal += VJ_DECODE( pvHdr, &uHdrOffset);
      SWS_POKE32( pSlot->hdr, uIpHdrLen + SWS_TCP_SEQ, ulVal);
      }
    break;
    }

  SWS_PEEK16( uVal, pSlot->hdr, SWS_IPHDR_ID);
  if ( uChanges & VJ_NEW_I)
    uVal += VJ_DECODE( pvHdr, &uHdrOffset);
  else
    ++uVal;
  SWS_POKE16( pSlot->hdr, SWS_IPHDR_ID, (ui16)uVal);

  /* uHdrOffset is now the offset to new data */
  len = SWS_BufferSize( hBuf);
  ASSERT( len >= uHdrOffset);

  /* Write IP packet length */
  uVal = pSlot->uHdrLen + ((unsigned)len - uHdrOffset);
  SWS_POKE16( pSlot->hdr, SWS_IPHDR_TOTAL, (ui16)uVal);

  /* Remove compressed header */
  iRet = SWS_BufferRemoveFront( hBuf, uHdrOffset);

  /* Prepend decompressed header */
  if ( 0 == iRet)
    iRet = SWS_BufferAppendFront( hBuf, pSlot->uHdrLen);
  if ( 0 == iRet)
    {
    SWS_BufferCopyFromMemory( hBuf, 0, pSlot->hdr, pSlot->uHdrLen);

    /* Recalc IP hdr csum */
    pvHdr = SWS_BufferGetFragment( hBuf, 0, &len);
    SWS_POKECSUM( pvHdr, SWS_IPHDR_CHECKSUM, 0);
    SWS_POKECSUM(
      pvHdr, SWS_IPHDR_CHECKSUM, SWS_IpChecksum( hBuf, 0, uIpHdrLen, 0));
    /*ASSERT( !SWS_IpChecksum( hBuf, 0, uIpHdrLen, 0));*/
    }

  return iRet;
  }

#endif /* OPT_P2P_VJCOMP */


/****************************************************************************
 * Datalink down calls
 ****************************************************************************/

/*
 * Send an IP packet
 * Single threaded
 */
SWS_CB( int, P2pSend(                 /* Return !0 on error, -1 if pending */
  void* pvContext,
  SWS_BufferH hBuffer,
  ui32 dst,                           /* IN: IP address of next hop */
  unsigned tos                        /* IN: Type Of Service SWS_IPTOS_xxx */
)){
  SWS_SP2pAdapter* const pAdapter = (SWS_SP2pAdapter*)pvContext;
  int iRet;
  (void)tos;

  TRACE( 9, ("P2pSendIp(%p,%p,%08lx,%u)\n", pvContext, hBuffer, dst, tos));
  ASSERT( NULL != hBuffer);
  ASSERT( IS_P2P_ADAPTER( pAdapter));
  ASSERT( SWS_INADDR_ANY != dst);
  ASSERT( PlatformIsCritical());

  /* Is the destination this adapter? */
  if ( dst == pAdapter->uIpAddress
    && dst != pAdapter->uGatewayAddr
  ) {
    /* Yes, loopback the data */
    (*pAdapter->dlink.pfnRecvInd)(
      pAdapter->dlink.pvContext,
      hBuffer,
      0
    );
    iRet = 0;
    }
  else if ( pAdapter->proto.pfnSend)
    {
    /* Restart link idle timer */
    if ( pAdapter->uIdleTimeout > 0)
      SWS_TimerStart( pAdapter->hLoginTimer, pAdapter->uIdleTimeout);

    iRet = (*pAdapter->proto.pfnSend)( pAdapter->proto.pvContext, hBuffer);
    }
  else
    {
    TRACE( 0, ("!!! P2pSendIp(%p) protocol not bound\n", pvContext));
    iRet = SWS_ENETDOWN;
    }

  return iRet;
  }


/*
 * Get an interface property
 */
static SWS_CB( int, P2pGetProperty(
  SWS_PTR( void) pvContext,
  int type,
  SWS_PTR( void) pvBuf,
  SWS_PTR( size_t) puLen
)){
  SWS_SP2pAdapter* const pAdapter = (SWS_SP2pAdapter*)pvContext;
  size_t size;

  TRACE( 9, ("P2pGetProperty(%p,%d,%p,%p)\n", pvContext,type,pvBuf,puLen));
  ASSERT( IS_P2P_ADAPTER( pAdapter));
  ASSERT( NULL != pvBuf);
  ASSERT( NULL != puLen);

  /* Offer to protocol */
  if ( pAdapter->proto.pfnGetProperty
    && 0 == (*pAdapter->proto.pfnGetProperty)(
      pAdapter->proto.pvContext, type, pvBuf, puLen
    )
  ) {
    return 0;
    }

  switch ( type)
    {
  default:
    /* Pass down to adapter */
    if ( type >= SWS_kDatalinkProperty_Private && pAdapter->pfnIoctl)
      return (*pAdapter->pfnIoctl)( pAdapter->pvContext, type, pvBuf, puLen);

    /* Not implemented */
    TRACE( 3, ("P2pGetProperty %d not implemented\n", type));
    return SWS_EBADF;

  case SWS_kDatalinkProperty_Address:
    if ( *puLen < sizeof(ui32))
      {
      TRACE( 0, ("!!! P2pGetProperty buffer too small\n"));
      return SWS_EINVAL;
      }

    *(SWS_PTR(ui32))pvBuf = pAdapter->uIpAddress;
    *puLen = sizeof(ui32);
    break;

  case SWS_kDatalinkProperty_SubnetMask:
    if ( *puLen < sizeof(ui32))
      {
      TRACE( 0, ("!!! P2pGetProperty buffer too small\n"));
      return SWS_EINVAL;
      }

    *(SWS_PTR(ui32))pvBuf = pAdapter->uSubnetMask;
    *puLen = sizeof(ui32);
    break;

  case SWS_kDatalinkProperty_Gateway:
    if ( *puLen < sizeof(ui32))
      {
      TRACE( 0, ("!!! P2pGetProperty buffer too small\n"));
      return SWS_EINVAL;
      }

    *(SWS_PTR(ui32))pvBuf = pAdapter->uGatewayAddr;
    *puLen = sizeof(ui32);
    break;

  case SWS_kDatalinkProperty_MTU:
    if ( *puLen < sizeof(size_t))
      {
      TRACE( 0, ("!!! P2pGetProperty buffer too small\n"));
      return SWS_EINVAL;
      }

    *(SWS_PTR(size_t))pvBuf = 576; /* 576= min MTU for any path - RFC791 */
    *puLen = sizeof(size_t);
    break;

  case SWS_kDatalinkProperty_Ident:
    size = strlen( pAdapter->szIdent);
    if ( *puLen <= size)
      {
      TRACE( 0, ("!!! P2pGetProperty buffer too small\n"));
      return SWS_EINVAL;
      }

    strcpy( (SWS_PTR(char))pvBuf, pAdapter->szIdent);
    *puLen = size;
    break;

  case SWS_kDatalinkProperty_HwType:
    size = sizeof( ui8);
    if ( *puLen < size)
      {
      TRACE( 0, ("!!! P2pGetProperty buffer too small\n"));
      return SWS_EINVAL;
      }

    *(SWS_PTR(ui8))pvBuf = 20; /* Serial line, RFC 1700 p164 */
    *puLen = size;
    break;

  case SWS_kDatalinkProperty_HwAddress:
    size = sizeof( pAdapter->uHwAddress);
    if ( *puLen < size)
      {
      TRACE( 0, ("!!! P2pGetProperty buffer too small\n"));
      return SWS_EINVAL;
      }

    memcpy( pvBuf, &pAdapter->uHwAddress, size);
    *puLen = size;
    break;

  case SWS_kDatalinkProperty_DHCP:
    if ( *puLen < sizeof(ui8))
      {
      TRACE( 0, ("!!! P2pGetProperty buffer too small\n"));
      return SWS_EINVAL;
      }

    *(SWS_PTR(ui8))pvBuf = (ui8)(SWS_INADDR_ANY == pAdapter->uIpAddress
      && 0 == pAdapter->uSubnetMask);
    *puLen = sizeof(ui8);
    break;

  case SWS_kP2pProperty_Modem:
    if ( *puLen < sizeof(ui8))
      {
      TRACE( 0, ("!!! P2pGetProperty buffer too small\n"));
      return SWS_EINVAL;
      }

    switch ( pAdapter->eModem)
      {
    case kModemNone:
    case kModemMsDirect:
      *(SWS_PTR(ui8))pvBuf = 0;
      break;
    case kModemHayes:
      *(SWS_PTR(ui8))pvBuf = 1;
      break;
      }
    *puLen = sizeof(ui8);
    break;

  case SWS_kP2pProperty_Baud:
    if ( *puLen < sizeof(ui32))
      {
      TRACE( 0, ("!!! P2pGetProperty buffer too small\n"));
      return SWS_EINVAL;
      }

    if ( 0 == pAdapter->ulBaud
      && pAdapter->pfnIoctl
    ) {
      (*pAdapter->pfnIoctl)(
        pAdapter->pvContext, SWS_kP2pIoctl_GetBaud, &pAdapter->ulBaud, puLen);
      }

    *(SWS_PTR(ui32))pvBuf = pAdapter->ulBaud;
    *puLen = sizeof(ui32);
    break;
    }

  return 0;
  }


/*
 * Set an interface property
 */
static SWS_CB( int, P2pSetProperty(
  SWS_PTR( void) pvContext,
  int type,
  SWS_PTR( const void) pvBuf,
  size_t len
)){
  SWS_SP2pAdapter* const pAdapter = (SWS_SP2pAdapter*)pvContext;

  TRACE( 9, ("P2pSetProperty(%p,%d,%p,%u)\n", pvContext,type,pvBuf,len));
  ASSERT( IS_P2P_ADAPTER( pAdapter));
  ASSERT( NULL != pvBuf);

  /* Offer to protocol */
  if ( pAdapter->proto.pfnSetProperty
    && 0 == (*pAdapter->proto.pfnSetProperty)(
      pAdapter->proto.pvContext, type, pvBuf, len
    )
  ) {
    return 0;
    }

  switch ( type)
    {
  default:
    /* Pass down to adapter */
    if ( type >= SWS_kDatalinkProperty_Private && pAdapter->pfnIoctl)
      return (*pAdapter->pfnIoctl)(
        pAdapter->pvContext, type, (SWS_PTR( void))pvBuf, &len);

    /* Not implemented */
    TRACE( 3, ("P2pSetProperty %d not implemented\n", type));
    return SWS_EBADF;

  case SWS_kDatalinkProperty_Address:
    if ( len < sizeof(ui32))
      {
      TRACE( 0, ("!!! P2pSetProperty buffer too small\n"));
      return SWS_EINVAL;
      }

    pAdapter->uIpAddress = *(SWS_PTR(const ui32))pvBuf;
    break;

  case SWS_kDatalinkProperty_Gateway:
    if ( len < sizeof(ui32))
      {
      TRACE( 0, ("!!! P2pSetProperty buffer too small\n"));
      return SWS_EINVAL;
      }

    pAdapter->uGatewayAddr = *(SWS_PTR(const ui32))pvBuf;
    break;

  case SWS_kDatalinkProperty_SubnetMask:
    if ( len < sizeof(ui32))
      {
      TRACE( 0, ("!!! P2pSetProperty buffer too small\n"));
      return SWS_EINVAL;
      }

    pAdapter->uSubnetMask = *(SWS_PTR(const ui32))pvBuf;
    break;
    }

  return 0;
  }


/****************************************************************************
 * Internal functions
 ****************************************************************************/

/*
 * Open an adapter
 * Single-threaded
 */
SWS_CB( int, P2pOpen( SWS_PTR( void) pv))
  {
  SWS_SP2pAdapter* const pAdapter = (SWS_SP2pAdapter*)pv;

  TRACE( 9, ("P2pOpen(%p)\n", pv));
  ASSERT( IS_P2P_ADAPTER( pAdapter));
  ASSERT( PlatformIsCritical());

  switch ( pAdapter->eState)
    {
  case kStateClosed:
    break;

  case kStateOpen:
    TRACE( 1, ("*** P2pOpen adapter is open\n"));
    return SWS_EALREADY;

  case kStateCalling:
    TRACE( 1, ("*** P2pOpen adapter is connecting\n"));
    return SWS_EALREADY;

  case kStateLogin:
    TRACE( 1, ("*** P2pOpen adapter is logging in\n"));
    return SWS_EALREADY;

  case kStateConnected:
  case kStateAccepted:
    TRACE( 1, ("*** P2pOpen adapter is connected\n"));
    return SWS_EISCONN;

  case kStateListening:
  case kStateRinging:
    TRACE( 1, ("*** P2pOpen adapter is listening\n"));
    return SWS_EALREADY;

  case kStateClosing:
    TRACE( 1, ("*** P2pOpen adapter is closing\n"));
    return SWS_EALREADY;

  default:
    TRACE( 1, ("*** P2pOpen adapter is in use\n"));
    return SWS_EINVAL;
    }

  pAdapter->eState = kStateOpen;
  pAdapter->ulBaud = 0;

  TRACE( 3, ("P2pOpen(%p) %s opened\n", pAdapter,pAdapter->szIdent));

  if ( pAdapter->pfnIoctl)
    {
    size_t size;

    /* Get default baud rate */
    size = sizeof(pAdapter->ulBaud);
    (*pAdapter->pfnIoctl)(
      pAdapter->pvContext, SWS_kP2pIoctl_GetBaud, &pAdapter->ulBaud, &size);
    }

  return 0;
  }


/*
 * Close a link
 */
static SWS_FASTCALL( int, P2pClose( SWS_SP2pAdapter* pAdapter))
  {
  TRACE( 3, ("P2pClose() %s\n", pAdapter->szIdent));

  SWS_TimerStop( pAdapter->hLoginTimer);

  /* Hang up */
  P2pHangup( pAdapter);

  /* Disable receive */
  P2pRxEnable( pAdapter, 0);

  /* Go offline */
  P2pAdapterOnline( pAdapter, 0);

  /* Tell client that session is finished */
  (void)EVENT( pAdapter, SWS_kP2pEventClose, 0, NULL);
  pAdapter->pfnEvent = &P2pEvent;
  pAdapter->pvEvent = NULL;

  /* Release the bound protocol */
  P2pRelease( pAdapter);

  pAdapter->uIpAddress = SWS_INADDR_ANY;  /* Unset */
  pAdapter->uSubnetMask = ~0UL;           /* None */
  pAdapter->uGatewayAddr = SWS_INADDR_NONE; /* None */

  pAdapter->eState = kStateClosed;
  TRACE( 3, ("P2pClose() %s closed\n", pAdapter->szIdent));

  return 0;
  }


/*
 * Bind to a protocol
 */
static SWS_FASTCALL( int, P2pBind( SWS_SP2pAdapter* pAdapter, ui32 protocol))
  {
  SWS_P2pProtocolDn dn;
  int iRet;

  TRACE( 9, ("P2pBind(%p,%#lx)\n", pAdapter,protocol));
  ASSERT( !pAdapter->proto.pfnRelease);

  /* Init callbacks from protocol to adapter */
  dn.pvContext = pAdapter->pvContext;
  dn.pfnSend = pAdapter->pfnSend;
  dn.pfnRecv = pAdapter->pfnRecv;

  memset( &pAdapter->proto, 0, sizeof( pAdapter->proto) );

  switch ( protocol)
    {
#if OPT_SLIP
  case SWS_kP2pProtocolSlip:
    TRACE( 3, ("P2pBind(%s) SLIP\n", pAdapter->szIdent));
    iRet = SWS_SlipBind(
      INDEX( pAdapter), &pAdapter->dlink, &dn, &pAdapter->proto);
    break;
#endif

#if OPT_PPP
  case SWS_kP2pProtocolPpp:
    TRACE( 3, ("P2pBind(%s) PPP\n", pAdapter->szIdent));
    iRet = SWS_PppBind(
      INDEX( pAdapter), &pAdapter->dlink, &dn, &pAdapter->proto);
    break;
#endif

  default:
    TRACE( 0, ("!!! P2pBind() no such protocol %#lx\n", protocol));
    iRet = SWS_EPROTONOSUPPORT;
    break;
    }

  if ( 0 == iRet)
    {
    ASSERT( pAdapter->proto.pfnRelease);
    ASSERT( pAdapter->proto.pfnSend);
    ASSERT( pAdapter->proto.pfnSendAck);
    ASSERT( pAdapter->proto.pfnRecvInd);
    ASSERT( pAdapter->proto.pfnLinkInd);
    }

  return iRet;
  }


/*
 * Release a protocol
 */
static SWS_FASTCALL( void, P2pRelease( SWS_SP2pAdapter* pAdapter))
  {
  TRACE( 9, ("P2pRelease(%p)\n", pAdapter));

  if ( pAdapter->proto.pfnRelease)
    (*pAdapter->proto.pfnRelease)( pAdapter->proto.pvContext);

  memset( &pAdapter->proto, 0, sizeof(pAdapter->proto) );
  }


/*
 * Initiate outgoing connection
 */
static int P2pCall(
  SWS_SP2pAdapter* pAdapter,
  SWS_PTR( const char) psz,
  ui32 uWait /* Ticks */
) {
  int iRet = SWS_ENODEV;
  char szBuf[ 80];

  TRACE( 9, ("P2pCall(%p,%s,%lu)\n", pAdapter,psz,uWait));
  ASSERT( kStateOpen == pAdapter->eState);

  switch ( pAdapter->eModem)
    {
  case kModemNone:
    if ( NULL != psz && '\0' != *psz)
      TRACE( 1, ("*** P2pCall(%s) superfluous dial string\n",
        pAdapter->szIdent));

    pAdapter->eState = kStateCalling;
    pAdapter->iModemResponse = 0;

    /* Go online */
    P2pAdapterOnline( pAdapter, 1);

    /* Refresh adapter status */
    P2pAdapterStatus( pAdapter);
    if ( pAdapter->iAdapterReady)
      {
      TRACE( 3, ("P2pCall(%s) adapter ready\n", pAdapter->szIdent));
      P2pOnConnected( pAdapter);
      iRet = 0;
      }
    else
      {
      TRACE( 3, ("P2pCall(%s) adapter not ready, waiting %lu ms\n",
        pAdapter->szIdent, uWait));
      iRet = -1;
      }
    break;

  case kModemMsDirect:
    pAdapter->eState = kStateCalling;

    /* Go online */
    P2pAdapterOnline( pAdapter, 1);

    /* Enable receive data */
    P2pRxEnable( pAdapter, 1);

    iRet = P2pAdapterWrite( pAdapter, "CLIENT");
    /* NB use a short initial timeout in case no initial reply */
    SWS_TimerStart( pAdapter->hLoginTimer, MS2TICKS( kMsModemTimeout1));
    break;

  case kModemHayes:
    if ( NULL == psz)
      {
      TRACE( 0, ("!!! P2pCall(%s) missing dial string\n", pAdapter->szIdent));
      return SWS_EINVAL;
      }

    /* Go online */
    P2pAdapterOnline( pAdapter, 1);

    /* Enable receive data */
    P2pRxEnable( pAdapter, 1);

    /* Reset modem */
    iRet = P2pModemInit( pAdapter);
    if ( iRet)
      return iRet;

    TRACE( 2, ("P2pCall(%s) => %s\n", pAdapter->szIdent, psz));
    pAdapter->eState = kStateCalling;

    /* Dial */
    sprintf( szBuf, kModemDial "%.64s", psz);
    iRet = P2pModemCmd( pAdapter, szBuf, WAIT_NEVER);
    break;
    }

  if ( iRet <= 0)
    {
    ui32 uLimit = PlatformTime() + uWait;
    int const iLink = SWS_DatalinkFind( pAdapter->szIdent);
    ASSERT( iLink >= 0);

    /* Wait for connection, login and link setup */
    iRet = 0;
    while ( kStateCalling == pAdapter->eState
      || kStateLogin == pAdapter->eState
      || (kStateConnected == pAdapter->eState
        && !SWS_DatalinkIsUp( (unsigned)iLink)
      )
    ) {
      if ( WAIT_NEVER == uWait)
        return -1;

      if ( WAIT_FOREVER != uWait
        && SWS_TimerLT( uLimit, PlatformTime())
      ) {
        TRACE( 1, ("*** P2pCall(%p) timed out\n", pAdapter));
        return SWS_ETIMEDOUT;
        }

      iRet = EVENT( pAdapter, SWS_kP2pEventIdle, 0, NULL);
      if ( iRet > 0)
        {
        TRACE( 1, ("*** P2pCall(%p) client idle abort\n", pAdapter));
        return iRet;
        }
      }

    iRet = pAdapter->iModemResponse;
    if ( 0 == iRet && kStateConnected != pAdapter->eState)
      iRet = SWS_ECONNREFUSED; /* Login failed */

    if ( iRet)
      TRACE( 1, ("*** P2pCall(%s) failed %d\n", pAdapter->szIdent, iRet));
    }
  else
    {
    pAdapter->eState = kStateOpen;
    }

  return iRet;
  }


/*
 * Listen for incoming connection
 */
static SWS_FASTCALL( int, P2pListen(
  SWS_SP2pAdapter* pAdapter,
  ui32 uWait /* Ticks */
)){
  int iRet = SWS_ENODEV;
  char szBuf[ 80];

  TRACE( 9, ("P2pListen(%p,%lu)\n", pAdapter,uWait));
  ASSERT( kStateOpen == pAdapter->eState);

  switch ( pAdapter->eModem)
    {
  case kModemNone:
    pAdapter->eState = kStateListening;
    pAdapter->iModemResponse = 0;

    /* Go online */
    P2pAdapterOnline( pAdapter, 1);

    /* Refresh adapter status */
    P2pAdapterStatus( pAdapter);
    if ( pAdapter->iAdapterReady)
      {
      TRACE( 3, ("P2pListen(%s) adapter ready\n", pAdapter->szIdent));
      P2pOnConnected( pAdapter);
      iRet = 0;
      }
    else
      {
      TRACE( 3, ("P2pListen(%s) adapter not ready, waiting %lu ms\n",
        pAdapter->szIdent, uWait));
      iRet = -1;
      }
    break;

  case kModemMsDirect:
    pAdapter->eState = kStateListening;

    /* Go online */
    P2pAdapterOnline( pAdapter, 1);

    /* Enable receive data */
    P2pRxEnable( pAdapter, 1);
    iRet = 0;
    break;

  case kModemHayes:
    /* Go online */
    P2pAdapterOnline( pAdapter, 1);

    /* Enable receive data */
    P2pRxEnable( pAdapter, 1);

    /* Init modem */
    iRet = P2pModemInit( pAdapter);
    if ( iRet)
      return iRet;

    TRACE( 2, ("P2pListen(%s) listening\n", pAdapter->szIdent));
    pAdapter->eState = kStateListening;

    /* Setup (auto)answer */
    sprintf( szBuf, kModemAutoAnswer "%u", pAdapter->uRings);
    iRet = P2pModemCmd( pAdapter, szBuf, MS2TICKS( kModemCmdTimeout));
    break;
    }

  if ( iRet <= 0)
    {
    ui32 uLimit = PlatformTime() + uWait;
    int const iLink = SWS_DatalinkFind( pAdapter->szIdent);
    ASSERT( iLink >= 0);

    /* Wait for connection, login and link setup */
    iRet = 0;
    while ( kStateListening == pAdapter->eState
      || kStateRinging == pAdapter->eState
      || (kStateAccepted == pAdapter->eState
        && !SWS_DatalinkIsUp( (unsigned)iLink)
      )
    ) {
      if ( WAIT_NEVER == uWait)
        return -1;

      if ( WAIT_FOREVER != uWait
        && SWS_TimerLT( uLimit, PlatformTime())
      ) {
        TRACE( 1, ("*** P2pListen(%p) timed out\n", pAdapter));
        return SWS_ETIMEDOUT;
        }

      iRet = EVENT( pAdapter, SWS_kP2pEventIdle, 0, NULL);
      if ( iRet > 0)
        {
        TRACE( 1, ("*** P2pListen(%p) client idle abort\n", pAdapter));
        return iRet;
        }
      }

    iRet = pAdapter->iModemResponse;
    if ( iRet)
      TRACE( 1, ("*** P2pListen(%s) failed %d\n", pAdapter->szIdent, iRet));

    if ( 0 == iRet && kStateAccepted != pAdapter->eState)
      iRet = SWS_ECONNREFUSED; /* Login failed */
    }
  else
    {
    pAdapter->eState = kStateOpen;
    }

  return iRet;
  }


/*
 * Hangup a connection
 */
static SWS_FASTCALL( int, P2pHangup( SWS_SP2pAdapter* pAdapter))
  {
  int iErr = 0;
  EState eState;

  TRACE( 9, ("P2pHangup() %p\n", pAdapter));

  SWS_TimerStop( pAdapter->hLoginTimer);

  eState = pAdapter->eState;
  switch ( eState)
    {
  case kStateClosed:
  case kStateOpen:
    return 0;

  case kStateClosing:
    /* Prevent recursion */
    TRACE( 3, ("P2pHangup(%s) already closing\n", pAdapter->szIdent));
    return SWS_EACCES;

  case kStateCalling:
  case kStateListening:
  case kStateRinging:
    pAdapter->eState = kStateClosing;
    break;

  case kStateLogin:
    pAdapter->eState = kStateClosing;
    (void)EVENT( pAdapter, SWS_kP2pEventDisconnect, 0, NULL);
    break;

  case kStateConnected:
  case kStateAccepted:
    TRACE( 2, ("P2pHangup() shutdown %s\n", pAdapter->szIdent));
    pAdapter->eState = kStateClosing;
    (void)EVENT( pAdapter, SWS_kP2pEventDisconnect, 0, NULL);

    /* Request protocol to shutdown link */
    ASSERT( pAdapter->proto.pfnLinkInd);
    (*pAdapter->proto.pfnLinkInd)(
      pAdapter->proto.pvContext, SWS_kP2pLinkGoingDown);
    break;
    }

  TRACE( 2, ("P2pHangup() closing %s\n", pAdapter->szIdent));
  switch ( pAdapter->eModem)
    {
  case kModemNone:
  case kModemMsDirect:
    /* Disable receive data */
    P2pRxEnable( pAdapter, 0);
    break;

  case kModemHayes:
    P2pRxEnable( pAdapter, 1);
    pAdapter->iModemResponse = -1;

    /* Attempt h/w hangup */
    iErr = P2pAdapterOnline( pAdapter, 0);
    if ( !iErr
      && pAdapter->iAdapterCarrier
      && pAdapter->iAdapterReady
    ) {
      ui32 const uLimit = PlatformTime() + kModemGuardTime;

      /* Wait for modem response */
      while ( -1 == (iErr = pAdapter->iModemResponse))
        {
        if ( SWS_TimerLT( uLimit, PlatformTime()) )
          {
          TRACE( 1, ("*** P2pHangup(%p) timed out\n", pAdapter));
          iErr = SWS_ETIMEDOUT;
          break;
          }

        (void)EVENT( pAdapter, SWS_kP2pEventIdle, 0, NULL);
        }

      /* Accept NO CARRIER as no error */
      if ( SWS_ESHUTDOWN == iErr)
        iErr = 0;
      }
    else if ( kStateListening == eState)
      {
      iErr = 0;
      }
    else
      {
      /* Wait for modem guard time */
      TRACE( 3, ("P2pHangup() waiting for guard time\n"));
      PlatformSleep( kModemGuardTime);
      iErr = -1;
      }

    if ( iErr)
      {
      /* Enter command mode */
      TRACE( 3, ("P2pHangup() send modem esc\n"));
      P2pAdapterWrite( pAdapter, kModemEsc);

      /* Guard time */
      TRACE( 3, ("P2pHangup() waiting for guard time\n"));
      PlatformSleep( kModemGuardTime);

      /* Hang up modem */
      TRACE( 3, ("P2pHangup() send modem hangup\n"));
      iErr = P2pModemCmd( pAdapter, kModemHangup, MS2TICKS( kModemCmdTimeout));
      }

    /* Disable answer */
    P2pModemCmd( pAdapter, kModemAutoAnswer "0", MS2TICKS( kModemCmdTimeout));
    break;
    }

  pAdapter->eState = kStateOpen;

  return iErr;
  }


/*
 * Get named session info
 */
SWS_FASTCALL( int, P2pSession(            /* Returns !0 on error */
  SWS_PTR( const char) pszSess,       /* IN: Session */
  SWS_PTR( SWS_SP2pSession) pSess     /* OUT */
)){
  char szBuf[ 32];

  TRACE( 9, ("P2pSession(%s,%p)\n", pszSess,pSess));
  ASSERT( NULL != pszSess);
  ASSERT( NULL != pSess);

  /* Get connection protocol */
  PlatformPropertyGetString(
    pszSess,
    PVAL_P2P_PROTOCOL, "",
    szBuf, sizeof(szBuf) -1
  );
  if ( !SWS_stricmp( szBuf, kszProtocolSlip))
    {
    pSess->protocol = SWS_kP2pProtocolSlip;
    }
  else if ( !SWS_stricmp( szBuf, kszProtocolPpp))
    {
    pSess->protocol = SWS_kP2pProtocolPpp;
    }
  else if ( !SWS_stricmp( szBuf, ""))
    {
    TRACE( 0, ("!!! P2pSession(%s) protocol not specified\n", pszSess));
    return SWS_ENODEV;
    }
  else
    {
    TRACE( 0, ("!!! P2pSession(%s) no such protocol %s\n",
      pszSess, szBuf));
    return SWS_EINVAL;
    }

  /* Get authentication protocol */
  PlatformPropertyGetString(
    pszSess,
    PVAL_P2P_AUTH, "",
    szBuf, sizeof(szBuf) -1
  );
  if ( !SWS_stricmp( szBuf, kszAuthNone)
    || !SWS_stricmp( szBuf, "")
  ) {
    pSess->auth = SWS_kP2pAuthNone;
    }
  else if ( !SWS_stricmp( szBuf, kszAuthPap))
    {
    pSess->auth = SWS_kP2pAuthPap;
    }
  else if ( !SWS_stricmp( szBuf, kszAuthChap))
    {
    pSess->auth = SWS_kP2pAuthChap;
    }
  else
    {
    TRACE( 0, ("!!! P2pSession(%s) no such authentication protocol %s\n",
      pszSess, szBuf));
    return SWS_EINVAL;
    }

  /* Dial string */
  PlatformPropertyGetString(
    pszSess, PVAL_P2P_DIAL, "",
    pSess->szServer, sizeof(pSess->szServer) -1
  );

  /* Connection timeout */
  pSess->uConnectTimeout = PlatformPropertyGetUint(
    pszSess,
    PVAL_P2P_TIMEOUT,
    pSess->szServer[0] ? kConnectTimeout : 0
  );

  /* Idle timeout */
  pSess->uIdleTimeout = (ui16)PlatformPropertyGetUint(
    pszSess, PVAL_P2P_IDLE, 0);

  /* VJ header compression */
  pSess->uCompSlots = (ui16)PlatformPropertyGetUint(
    pszSess,
    PVAL_P2P_VJSLOTS,
    0
  );

  /* Login */
  pSess->pvEvent = NULL;
  if ( PlatformPropertyGetUint( pszSess, PVAL_P2P_LOGIN, 0))
    pSess->pfnEvent = &P2pClientLogin;
  else
    pSess->pfnEvent = 0;

  /* Get connection username & password */
  PlatformPropertyGetString(
    pszSess,
    PVAL_P2P_USER, "",
    pSess->user.szName, sizeof(pSess->user.szName) -1
  );
  PlatformPropertyGetString(
    pszSess,
    PVAL_P2P_PASSWD, "",
    pSess->user.szPassword, sizeof(pSess->user.szPassword) -1
  );

  /* Get connection IP settings */
  pSess->uIpAddress = PlatformPropertyGetUint(
    pszSess, PVAL_IPADDRESS, SWS_INADDR_ANY);
  pSess->uSubnetMask = PlatformPropertyGetUint(
    pszSess, PVAL_IPSUBNET, ~0UL);
  pSess->uGateway = PlatformPropertyGetUint(
    pszSess, PVAL_IPGATEWAY, SWS_INADDR_ANY);

  return 0;
  }


/****************************************************************************
 * Event handlers
 ****************************************************************************/

/*
 * Call connected notification
 */
static SWS_FASTCALL( void, P2pOnConnected( SWS_SP2pAdapter* pAdapter))
  {
  TRACE( 9, ("P2pOnConnected(%p)\n", pAdapter));

  SWS_TimerStop( pAdapter->hLoginTimer);

  switch ( pAdapter->eState)
    {
  case kStateCalling:
    TRACE( 2, ("P2pOnConnected(%p) calling\n", pAdapter));
    pAdapter->eState = kStateLogin;

    /* Client callback */
    (void)EVENT(
      pAdapter, SWS_kP2pEventConnect,
      sizeof(pAdapter->ulBaud), &pAdapter->ulBaud
    );

    /* Process any login */
    P2pOnLogin( pAdapter, "");
    return;

  case kStateListening:
  case kStateRinging:
    TRACE( 2, ("P2pOnConnected(%p) listening\n", pAdapter));
    pAdapter->eState = kStateAccepted;

    /* Client callback */
    (void)EVENT(
      pAdapter, SWS_kP2pEventConnect,
      sizeof(pAdapter->ulBaud), &pAdapter->ulBaud
    );
    break;

  default:
    return;
    }

  P2pOnLinkUp( pAdapter);
  }


/*
 * Process login event
 */
static SWS_FASTCALL( int, P2pOnLogin(
  SWS_SP2pAdapter* pAdapter,
  const char* psz
)){
  int iRet;
  SWS_SP2pLogin info;
  size_t size;

  TRACE( 9, ("P2pOnLogin(%p,%p)\n", pAdapter,psz));

  switch ( pAdapter->eState)
    {
  case kStateLogin:
    break;

  default:
    TRACE( 1, ("*** P2pOnLogin(%p) unexpected\n", pAdapter));
    return SWS_EINVAL;
    }

  /* Send client login event */
  info.szTx[0] = '\0';
  info.pszRx = psz;
  info.uTimeout = SWS_TimerStop( pAdapter->hLoginTimer);
  iRet = EVENT( pAdapter, SWS_kP2pEventLogin, sizeof( info), &info);
  switch ( iRet)
    {
  case 0:
    TRACE( 2, ("P2pOnLogin(%p) login complete\n", pAdapter));
    pAdapter->eState = kStateConnected;
    P2pOnLinkUp( pAdapter);
    break;

  case -1:
    /* Awaiting more input */
    size = strlen( info.szTx);
    if ( size > 0)
      {
      TRACE( 2, ("P2pOnLogin() write %u '%s'\n", size,info.szTx));
      P2pAdapterWrite( pAdapter, info.szTx);
      }

    /* Start idle timeout */
    if ( 0 != info.uTimeout)
      SWS_TimerStart( pAdapter->hLoginTimer, info.uTimeout);

    iRet = 0;
    break;

  default:
    TRACE( 2, ("P2pOnLogin() client abort %d\n", iRet));
    P2pClose( pAdapter);
    break;
    }

  return iRet;
  }


/*
 * Link up event (OnConnected, OnLogin)
 * Transfer input to protocol
 */
static SWS_FASTCALL( void, P2pOnLinkUp( SWS_SP2pAdapter* pAdapter))
  {
  TRACE( 9, ("P2pOnLinkUp(%p)\n", pAdapter));

  switch ( pAdapter->eState)
    {
  case kStateClosed:
  case kStateOpen:
  case kStateCalling:
  case kStateLogin:
  case kStateListening:
  case kStateRinging:
  case kStateClosing:
    ASSERT( !"Invalid P2pOnLinkUp");
    break;

  case kStateConnected:
  case kStateAccepted:
    TRACE( 2, ("P2pOnLinkUp(%s)\n", pAdapter->szIdent));

    /* Disable receive */
    P2pRxEnable( pAdapter, 0);

    /* Notify protocol that link is up */
    ASSERT( pAdapter->proto.pfnLinkInd);
    (*pAdapter->proto.pfnLinkInd)( pAdapter->proto.pvContext, SWS_kP2pLinkUp);

    /* Start link idle timer */
    if ( pAdapter->uIdleTimeout > 0)
      SWS_TimerStart( pAdapter->hLoginTimer, pAdapter->uIdleTimeout);
    break;
    }
  }


/*
 * Link down event (no carrier, loss of DCD or DSR and error)
 */
static SWS_FASTCALL( void, P2pOnLinkDown( SWS_SP2pAdapter* pAdapter))
  {
  TRACE( 9, ("P2pOnLinkDown(%p)\n", pAdapter));

  SWS_TimerStop( pAdapter->hLoginTimer);

  switch ( pAdapter->eState)
    {
  case kStateClosed:
  case kStateClosing:
  case kStateOpen:
    break;

  case kStateConnected:
    TRACE( 2, ("P2pOnLinkDown(%s)\n", pAdapter->szIdent));
    pAdapter->eState = kStateCalling;

    SWS_GefRaise( SWS_kGefP2pError, "Link down");

    /* Notify protocol that link has gone down */
    ASSERT( pAdapter->proto.pfnLinkInd);
    (*pAdapter->proto.pfnLinkInd)(
      pAdapter->proto.pvContext, SWS_kP2pLinkDown);

    /* Fall thru */

  case kStateCalling:
  case kStateLogin:
    P2pHangup( pAdapter);
    break;

  case kStateListening:
    break;

  case kStateRinging:
    P2pHangup( pAdapter);
    P2pListen( pAdapter, WAIT_NEVER);
    break;

  case kStateAccepted:
    TRACE( 2, ("P2pOnLinkDown(%s)\n", pAdapter->szIdent));
    pAdapter->eState = kStateListening;

    SWS_GefRaise( SWS_kGefP2pError, "Link down");

    /* Notify protocol that link has gone down */
    ASSERT( pAdapter->proto.pfnLinkInd);
    (*pAdapter->proto.pfnLinkInd)(
      pAdapter->proto.pvContext, SWS_kP2pLinkDown);

    P2pHangup( pAdapter);
    P2pListen( pAdapter, WAIT_NEVER);
    break;
    }
  }


/*
 * Incoming call (ring) notification
 */
static SWS_FASTCALL( void, P2pOnRing( SWS_SP2pAdapter* pAdapter))
  {
  TRACE( 9, ("P2pOnRing(%p)\n", pAdapter));

  switch ( pAdapter->eState)
    {
  case kStateListening:
    pAdapter->eState = kStateRinging;
    if ( 0 == pAdapter->uRings)
      {
      /* Answer. NB no response until 'connect' or 'no carrier' */
      if ( P2pModemCmd( pAdapter, kModemAnswer, WAIT_NEVER) > 0)
        pAdapter->eState = kStateListening;
      }
    break;

  default:
    break;
    }
  }


/*
 * Handle modem error response
 */
static SWS_FASTCALL( void, P2pOnError(
  SWS_SP2pAdapter* pAdapter,
  int iStatus
)){
  TRACE( 9, ("P2pOnError(%p,%d)\n", pAdapter,iStatus));
  TRACE( 2, ("P2pOnError(%s,%d)\n", pAdapter->szIdent,iStatus));

  iStatus = P2pModemRsp2Err( iStatus);
  (void)EVENT( pAdapter, SWS_kP2pEventError, sizeof(iStatus), &iStatus);

  switch ( pAdapter->eState)
    {
  case kStateClosed:
  case kStateClosing:
  case kStateOpen:
    break;

  case kStateCalling:
    pAdapter->eState = kStateOpen;
    break;

  case kStateLogin:
  case kStateConnected:
    P2pOnLinkDown( pAdapter);
    break;

  case kStateListening:
    break;

  case kStateRinging:
    pAdapter->eState = kStateListening;
    break;

  case kStateAccepted:
    P2pOnLinkDown( pAdapter);
    break;
    }
  }


/****************************************************************************
 * Hayes modem support
 ****************************************************************************/

/*
 * Initialise the modem command
 */
static SWS_FASTCALL( int, P2pModemInit( SWS_SP2pAdapter* pAdapter))
  {
  int iRet;

  TRACE( 9, ("P2pModemInit(%p)\n", pAdapter));
  ASSERT( kModemHayes == pAdapter->eModem);

  /* Reset modem */
  iRet = P2pModemCmd( pAdapter, kModemReset, MS2TICKS( kModemCmdTimeout));
  if ( iRet > 0)
    return iRet;

  /* Configure modem */
  iRet = P2pModemCmd( pAdapter, kModemInit, MS2TICKS( kModemCmdTimeout));
  if ( iRet > 0)
    return iRet;

  if ( pAdapter->szModemInit[0])
    iRet = P2pModemCmd( pAdapter, pAdapter->szModemInit, MS2TICKS( kModemCmdTimeout));

  return iRet;
  }


/*
 * Issue a modem command
 */
static int P2pModemCmd(
  SWS_SP2pAdapter* pAdapter,
  const char* psz,
  ui32 uTimeout /* Ticks */
) {
  int iRet;
  ui32 uLimit;
  char szBuf[ 80 + sizeof( kModemCmdPrefix kModemCmdSuffix)];

  TRACE( 9, ("P2pModemCmd(%p,%s,%lu)\n", pAdapter,psz,uTimeout));
  ASSERT( kModemHayes == pAdapter->eModem);
  ASSERT( NULL != psz);

  sprintf( szBuf, kModemCmdPrefix "%.80s" kModemCmdSuffix, psz);

  /* Flush input */
  P2pRxFlush( pAdapter);
  pAdapter->iModemResponse = -1;

  /* Send command string */
  TRACE( 2, ("P2pModemCmd(%p) send %s\n", pAdapter,szBuf));
  iRet = P2pAdapterWrite( pAdapter, szBuf);
  if ( iRet)
    return iRet;

  /* Wait for response */
  uLimit = PlatformTime() + uTimeout;
  while ( -1 == (iRet = pAdapter->iModemResponse))
    {
    if ( WAIT_NEVER == uTimeout)
      {
      TRACE( 3, ("P2pModemCmd(%p) nowait reply pending\n", pAdapter));
      break;
      }

    if ( WAIT_FOREVER != uTimeout
      && SWS_TimerLT( uLimit, PlatformTime())
    ) {
      TRACE( 1, ("*** P2pModemCmd(%p) timed out\n", pAdapter));
      iRet = SWS_ETIMEDOUT;
      SWS_GefRaise( SWS_kGefP2pError, "Modem not responding");
      break;
      }

    iRet = EVENT( pAdapter, SWS_kP2pEventIdle, 0, NULL);
    if ( iRet > 0)
      {
      TRACE( 1, ("*** P2pModemCmd(%p) client idle abort\n", pAdapter));
      break;
      }
    }

  if ( pAdapter->iModemResponse > 0)
    TRACE( 1, ("*** P2pModemCmd() Response %d\n", pAdapter->iModemResponse));

  return iRet;
  }


/*
 * Interpret modem response
 */
static SWS_FASTCALL( int, P2pModemResponse(
  SWS_SP2pAdapter* pAdapter,
  const char* pszStatus
)){
  int iStatus;

  TRACE( 9, ("P2pModemResponse(%p,%s)\n", pAdapter,pszStatus));
  ASSERT( kModemHayes == pAdapter->eModem);
  ASSERT( NULL != pszStatus);

  if ( 1 != sscanf( pszStatus, "%d", &iStatus))
    {
    char szInfo[ 16];
    unsigned long ulBaud;

    /* Modem response is terminated by CR */
    if ( NULL == strstr( pszStatus, kModemCmdSuffix))
      {
      TRACE( 1, ("*** P2pModemResponse(%p) no CR in '%s'\n",
        pAdapter, pszStatus));
      return SWS_EINVAL;
      }

    if ( !strcmp( pszStatus, "OK" kModemCmdSuffix))
      iStatus = 0;
    else if ( !strcmp( pszStatus, "ERROR" kModemCmdSuffix))
      iStatus = 4;
    else if ( !strcmp( pszStatus, "RING" kModemCmdSuffix))
      iStatus = 2;
    else if ( !strcmp( pszStatus, "NO CARRIER" kModemCmdSuffix))
      iStatus = 3;
    else if ( !strcmp( pszStatus, "NO DIALTONE" kModemCmdSuffix))
      iStatus = 6;
    else if ( !strcmp( pszStatus, "BUSY" kModemCmdSuffix))
      iStatus = 7;
    else if ( !strcmp( pszStatus, "NO ANSWER" kModemCmdSuffix))
      iStatus = 8;
    else if ( !strcmp( pszStatus, "CONNECT" kModemCmdSuffix))
      iStatus = 1;
    else if ( 1 <= sscanf( pszStatus, "CONNECT %lu/%15s", &ulBaud, szInfo))
      {
      TRACE( 2, ("P2pModemResponse(%p) %s\n", pAdapter, pszStatus));
      pAdapter->ulBaud = ulBaud;
      P2pOnConnected( pAdapter);
      return 0;
      }
    else if ( 1 == sscanf( pszStatus, "CARRIER %lu", &ulBaud)
      || 1 == sscanf( pszStatus, "PROTOCOL: %15s", szInfo)
      || 1 == sscanf( pszStatus, "COMPRESSION: %15s", szInfo)
    ) {
      TRACE( 2, ("P2pModemResponse(%p) %s\n", pAdapter, pszStatus));
      return -1;
      }
    else if ( 0 == strncmp(
        pszStatus, kModemCmdPrefix, sizeof( kModemCmdPrefix) -1
      )
    ) {
      TRACE( 3, ("P2pModemResponse(%p) Cmd echo %s\n", pAdapter, pszStatus));
      return -1;
      }
    else
      {
      TRACE( 1, ("*** P2pModemResponse(%p) Unknown '%s'\n",
        pAdapter, pszStatus));
      return SWS_EINVAL;
      }
    }

  switch ( iStatus)
    {
  case 0:  /* OK */
    TRACE( 2, ("P2pModemResponse(%p) OK\n", pAdapter));
    break;

  case 2:  /* RING */
    TRACE( 2, ("P2pModemResponse(%p) RING\n", pAdapter));
    P2pOnRing( pAdapter);
    iStatus = -1;
    break;

  case 3:  /* NO CARRIER (hungup) */
    TRACE( 2, ("P2pModemResponse(%p) NO CARRIER\n", pAdapter));
    P2pOnLinkDown( pAdapter);
    break;

  case 4:  /* ERROR */
    TRACE( 2, ("P2pModemResponse(%p) ERROR\n", pAdapter));
    P2pOnError( pAdapter, iStatus);
    break;

  case 6:  /* NO DIALTONE */
    TRACE( 2, ("P2pModemResponse(%p) NO DIALTONE\n", pAdapter));
    P2pOnError( pAdapter, iStatus);
    break;

  case 7:  /* BUSY */
    TRACE( 2, ("P2pModemResponse(%p) BUSY\n", pAdapter));
    P2pOnError( pAdapter, iStatus);
    break;

  case 8:  /* NO ANSWER */
    TRACE( 2, ("P2pModemResponse(%p) NO ANSWER\n", pAdapter));
    P2pOnError( pAdapter, iStatus);
    break;

  case 1:   /* CONNECT */
  case 5:   /* CONNECT 1200 */
  case 9:   /* CONNECT 600 */
  case 10:  /* CONNECT 2400 */
  case 11:  /* CONNECT 4800 */
  case 12:  /* CONNECT 9600 */
  case 13:  /* CONNECT 14400 */
  case 14:  /* CONNECT 19200 */
  case 15:  /* CONNECT 16800 */
  case 16:  /* CONNECT 19200 */
  case 17:  /* CONNECT 38400 */
  case 18:  /* CONNECT 57600 */
  case 19:  /* CONNECT 115200 */
  case 20:
  case 21:
  case 22:  /* CONNECT 1200TX/75RX */
  case 23:  /* CONNECT 1200RX/75TX */
  case 24:  /* CONNECT 7200 */
  case 25:
  case 26:  /* CONNECT 1200TX/75RX */
  case 27:  /* CONNECT 1200RX/75TX */
  case 28:  /* CONNECT 38400 */
  case 29:
  case 30:
  case 31:
  case 32:  /* CONNECT 28800 */
  case 33:  /* CONNECT 115200 */
  case 34:
  case 35:
  case 36:
  case 37:
  case 38:
  case 39:
  case 59:  /* CONNECT 16800 */
  case 61:  /* CONNECT 21600 */
  case 62:  /* CONNECT 24000 */
  case 63:  /* CONNECT 26400 */
  case 64:  /* CONNECT 28800 */
  /*case 84:  * CONNECT 33600 */
  case 91:  /* CONNECT 31200 */
  case 165: /* CONNECT 32000 */
  case 166: /* CONNECT 34000 */
  case 167: /* CONNECT 36000 */
  case 168: /* CONNECT 38000 */
  case 169: /* CONNECT 40000 */
  case 170: /* CONNECT 42000 */
  case 171: /* CONNECT 44000 */
  case 172: /* CONNECT 46000 */
  case 173: /* CONNECT 48000 */
  case 174: /* CONNECT 50000 */
  case 175: /* CONNECT 52000 */
  case 176: /* CONNECT 54000 */
  case 177: /* CONNECT 56000 */
    TRACE( 2, ("P2pModemResponse(%p) CONNECT %d\n", pAdapter,iStatus));
    P2pOnConnected( pAdapter);
    iStatus = 0;
    break;

  case 40:
  case 42:
  case 43:
  case 46:
  case 47:
  case 48:
  case 49:
  case 50:
  case 51:
  case 52:
  case 53:
  case 54:
  case 55:
  case 56:
  case 57:
  case 58: /* CARRIER: 28800 */
  case 78:
  case 79:
  case 150:
  case 151:
  case 152:
  case 153:
  case 154:
  case 155:
  case 156:
  case 157:
  case 158:
  case 159:
  case 160:
  case 161:
  case 162:
    TRACE( 2, ("P2pModemResponse(%p) CARRIER %d\n", pAdapter,iStatus));
    iStatus = -1;
    break;

  case 66:  /* COMPRESSION: MNP5 */
  case 67:  /* COMPRESSION: V42bis */
  case 69:  /* COMPRESSION: NONE */
    TRACE( 2, ("P2pModemResponse(%p) COMPRESSION: %d\n", pAdapter, iStatus));
    iStatus = -1;
    break;

  case 70: /* PROTOCOL: NONE */
  case 77: /* PROTOCOL: LAPM */
  case 80:
  case 84: /* PROTOCOL: ALT */
    TRACE( 2, ("P2pModemResponse(%p) PROTOCOL: %d\n", pAdapter, iStatus));
    iStatus = -1;
    break;

  default:
    TRACE( 2, ("P2pModemResponse(%p) %d\n", pAdapter,iStatus));
    iStatus = 0;
    break;
    }

  return iStatus;
  }


/*
 * Map Hayes modem response code to SwsSock error
 */
SWS_FASTCALL( int, P2pModemRsp2Err( int iErr))
  {
  TRACE( 9, ("P2pModemRsp2Err(%d)\n", iErr));
  switch ( iErr)
    {
  case -1:
    break;

  case 0:  /* OK */
  case 1:  /* CONNECT */
  case 2:  /* RING */
  case 5:  /* CONNECT 1200 */
    return 0;

  case 3:  /* NO CARRIER (hungup) */
    return SWS_ESHUTDOWN;

  case 4:  /* ERROR */
    return SWS_EINVAL;

  case 6:  /* NO DIALTONE */
    return SWS_ENETDOWN;

  case 7:  /* BUSY */
    return SWS_EHOSTUNREACH;

  case 8:  /* NO ANSWER */
    return SWS_EHOSTDOWN;

  default:
    if ( iErr < SWS_BASEERR)
      iErr = 0;
    break;
    }

  return iErr;
  }


/****************************************************************************
 * P2P adapter read/write
 ****************************************************************************/

/*
 * Write a string to the adapter
 */
static SWS_FASTCALL( int, P2pAdapterWrite(
  SWS_SP2pAdapter* pAdapter,
  const char* psz
)){
  int iRet;
  ui32 uLimit, uLen;

  TRACE( 9, ("P2pAdapterWrite(%p,%s)\n", pAdapter,psz));
  ASSERT( NULL != psz);

  uLen = strlen( psz);
  iRet = SWS_GefRaise( SWS_kGefP2pTx, uLen, (SWS_PTR( const void))psz);
  if ( 0 != iRet)
    return iRet;

  /* Wait for Tx ready */
  uLimit = PlatformTime() + kWriteTimeout;
  while ( pAdapter->iTxPending)
    {
    if ( SWS_TimerLT( uLimit, PlatformTime()) )
      {
      TRACE( 0, ("!!! P2pAdapterWrite(%p) timed out\n", pAdapter));
      return SWS_ETIMEDOUT;
      }

    iRet = EVENT( pAdapter, SWS_kP2pEventIdle, 0, NULL);
    if ( iRet)
      {
      TRACE( 1, ("*** P2pAdapterWrite(%p) client idle abort\n", pAdapter));
      return iRet;
      }
    }

  ASSERT( 0 == pAdapter->iTxPending);
  pAdapter->iTxPending = 1;

  /* Send string */
  iRet = (*pAdapter->pfnSend)( pAdapter->pvContext, psz, (size_t)uLen);
  if ( -1 != iRet)
    pAdapter->iTxPending = 0;
  else
    iRet = 0;

  return iRet;
  }


/*
 * Test if char is present in read ahead
 */
static SWS_FASTCALL( size_t, P2pAdapterPeek(
  SWS_SP2pAdapter* pAdapter,
  int iEol
)){
  unsigned u;
  size_t size;

  TRACE( 9, ("P2pAdapterPeek(%p,%d)\n",pAdapter,iEol));

  size = 0;
  u = pAdapter->rxRead;
  while ( u != pAdapter->rxWrite)
    {
    ++size;
    if ( pAdapter->cRxBuffer[ u] == iEol)
      return size;

    if ( ++u >= sizeof( pAdapter->cRxBuffer))
      u -= sizeof( pAdapter->cRxBuffer);
    }

  return 0;
  }


/*
 * Read a string
 */
static size_t P2pAdapterRead(
  SWS_SP2pAdapter* pAdapter,
  char* psz,
  size_t size,
  int iEol
) {
  char* const pszStart = psz;
  size_t sizeRead = 0;
  unsigned u = pAdapter->rxRead;

  TRACE( 9, ("P2pAdapterRead(%p,%p,%u,%d)\n",
    pAdapter,psz,size,iEol));
  ASSERT( NULL != psz);

  while ( size > 0 && u != pAdapter->rxWrite)
    {
    unsigned char c = pAdapter->cRxBuffer[ u];

    if ( ++u >= sizeof( pAdapter->cRxBuffer))
      u -= sizeof( pAdapter->cRxBuffer);

    if ( iEol >= 0
      && c == iEol
    ) {
      /* Ignore leading occurrences of delimiter */
      if ( sizeRead)
        {
        *(psz++) = c;
        ++sizeRead;
        --size;
        iEol = -1;
        break;
        }
      }
    else if ( isprint( c))
      {
      *(psz++) = c;
      ++sizeRead;
      --size;
      }
    }

  if ( size > 0)
    *psz = '\0';

  /* If EOL not found then do nothing */
  if ( iEol < 0)
    pAdapter->rxRead = u;
  else
    sizeRead = 0;

  if ( sizeRead > 0)
    {
    if ( SWS_GefRaise( SWS_kGefP2pRx, (ui32)sizeRead, (SWS_PTR( const void))pszStart))
      sizeRead = 0;
    }

  return sizeRead;
  }


/*
 * Set adepter online state
 */
static SWS_FASTCALL( int, P2pAdapterOnline(
  SWS_SP2pAdapter* pAdapter, int iOnline
)){
  int iRet;
  size_t size;
  ui32 u;

  TRACE( 9, ("P2pAdapterOnline(%p,%d)\n", pAdapter,iOnline));

  if ( !pAdapter->pfnIoctl)
    return SWS_EACCES;

  u = iOnline ? 1 : 0;
  size = sizeof( u);
  iRet = (*pAdapter->pfnIoctl)(
    pAdapter->pvContext, SWS_kP2pIoctl_SetOnLine, &u, &size);

  if ( 0 == iRet)
    TRACE( 3, ("P2pAdapterOnline(%p) %d\n", pAdapter,iOnline));

  return iRet;
  }


/*
 * Get adepter status
 */
static SWS_FASTCALL( int, P2pAdapterStatus( SWS_SP2pAdapter* pAdapter))
  {
  int iRet;
  size_t size;
  ui8 uModem;

  TRACE( 9, ("P2pAdapterStatus(%p)\n", pAdapter));

  if ( !pAdapter->pfnIoctl)
    return SWS_EACCES;

  size = sizeof( uModem);
  iRet = (*pAdapter->pfnIoctl)(
    pAdapter->pvContext, SWS_kP2pIoctl_GetModemStatus, &uModem, &size);
  if ( iRet)
    return iRet;

  iRet = (uModem & SWS_P2P_MODEM_DCD) ? 1 : 0;
  if ( iRet != pAdapter->iAdapterCarrier)
    {
    TRACE( 3, ("P2pAdapterStatus CD %d\n", iRet));
    pAdapter->iAdapterCarrier = iRet;
    }

  iRet = (uModem & SWS_P2P_MODEM_RNG) ? 1 : 0;
  if ( iRet != pAdapter->iAdapterRing)
    {
    TRACE( 3, ("P2pAdapterStatus RI %d\n", iRet));
    pAdapter->iAdapterRing = iRet;
    }

  iRet = (uModem & SWS_P2P_MODEM_DSR) ? 1 : 0;
  if ( iRet != pAdapter->iAdapterReady)
    {
    TRACE( 3, ("P2pAdapterStatus DSR %d\n", iRet));
    pAdapter->iAdapterReady = iRet;
    }

  return 0;
  }


/*
 * En/diable receive
 */
static SWS_FASTCALL( void, P2pRxEnable( SWS_SP2pAdapter* pAdapter, int iEna))
  {
  TRACE( 9, ("P2pRxEnable(%p,%d)\n", pAdapter,iEna));

  if ( iEna)
    {
    if ( !pAdapter->isRxEnabled)
      {
      TRACE( 3, ("P2pRxEnable(%p) on\n", pAdapter));

      pAdapter->rxRead = pAdapter->rxWrite = 0;
      AtomicWrite( &pAdapter->aRxDpc, 0);

      /* Post an rx buffer */
      pAdapter->isRxEnabled = 1;
      P2pRxData( pAdapter, 0);
      }
    }
  else
    {
    if ( pAdapter->isRxEnabled)
      {
      size_t size = 0;

      TRACE( 3, ("P2pRxEnable(%p) off\n", pAdapter));

      SWS_TimerStop( pAdapter->hRxIdleTimer);

      /* Abort pending rx buffer */
      pAdapter->isRxEnabled = 0;
      (*pAdapter->pfnRecv)(
        pAdapter->pvContext, NULL, &size, -1, WAIT_FOREVER);
      pAdapter->iRxPending = 0;
      }
    }
  }


/*
 * Update rx buffer write cursor
 * !!!CAUTION!!! arbitrary context
 */
static SWS_FASTCALL( void, P2pRxData( SWS_SP2pAdapter* pAdapter, size_t size))
  {
  int iErr;
  unsigned u = pAdapter->rxWrite;

  do
    {
    if ( size > 0)
      {
      /* Update buffer write cursor */
      u += size;
      if ( u >= sizeof( pAdapter->cRxBuffer))
        u -= sizeof( pAdapter->cRxBuffer);

      pAdapter->rxWrite = u;

      /* Process input in DPC */
      if ( 0 == AtomicXchg( &pAdapter->aRxDpc, 1))
        {
        if ( PlatformDpc( &P2pRecvIndDpc, pAdapter, NULL, 0))
          AtomicWrite( &pAdapter->aRxDpc, 0);
        }
      }

    if ( !pAdapter->isRxEnabled)
      break;

    /* Calc. buffer free space */
    size = P2pRxFree( pAdapter);
    if ( 0 == size)
      {
      /* RxBuffer overrun - discard all */
      u = P2pRxFlush( pAdapter);
      size = P2pRxFree( pAdapter);
      }

    switch ( pAdapter->eModem)
      {
    case kModemHayes:
      iErr = kCR;
      break;

    case kModemNone:
    case kModemMsDirect:
    default:
      size = 1;
      iErr = -1;
      break;
      }

    /* Post another read request */
    pAdapter->iRxPending = 1;
    iErr = (*pAdapter->pfnRecv)(
      pAdapter->pvContext,
      pAdapter->cRxBuffer + u,
      &size,
      iErr,
      WAIT_FOREVER
    );
    if ( -1 == iErr)
      break;

    pAdapter->iRxPending = 0;
    }
  while ( 0 == iErr);
  }


/****************************************************************************
 * Up calls from P2P adapter
 ****************************************************************************/

/*
 * Pended send complete
 * !!!CAUTION!!! arbitrary context
 */
static SWS_CB( void, P2pSendAck(
  SWS_PTR( void) pv,                  /* IN: Up call context */
  int iErr                            /* IN: Final error code, 0= no error */
)){
  SWS_SP2pAdapter* const pAdapter = (SWS_SP2pAdapter*)pv;

  if ( IS_P2P_ADAPTER( pAdapter))
    {
    if ( pAdapter->iTxPending)
      pAdapter->iTxPending = 0;
    else if ( pAdapter->proto.pfnSendAck)
      (*pAdapter->proto.pfnSendAck)( pAdapter->proto.pvContext, iErr);
    }
  }


/*
 * Pending receive complete
 * !!!CAUTION!!! arbitrary context
 */
SWS_CB( void, P2pRecvInd(
  SWS_PTR( void) pv,                  /* IN: Up call context */
  size_t size,                        /* IN: No. bytes received */
  int iErr                            /* IN: Final error code, 0= no error */
)){
  SWS_SP2pAdapter* const pAdapter = (SWS_SP2pAdapter*)pv;

  if ( IS_P2P_ADAPTER( pAdapter))
    {
    if ( pAdapter->iRxPending)
      {
      pAdapter->iRxPending = 0;
      /* Update rx buffer write cursor */
      P2pRxData( pAdapter, size);
      }
    else if ( pAdapter->proto.pfnRecvInd)
      (*pAdapter->proto.pfnRecvInd)( pAdapter->proto.pvContext, size, iErr);
    }
  }


/*
 * Adapter status change
 * !!!CAUTION!!! arbitrary context
 */
SWS_CB( void, P2pStatInd(
  SWS_PTR( void) pv,                  /* IN: Up call context */
  int iEvent,                         /* IN: SWS_EP2pStatus */
  int iData
)){
  SWS_SP2pAdapter* const pAdapter = (SWS_SP2pAdapter*)pv;

  if ( IS_P2P_ADAPTER( pAdapter))
    {
    switch ( iEvent)
      {
    case SWS_kP2pStatusCD:
      pAdapter->iAdapterCarrier = iData;
      break;
    case SWS_kP2pStatusRI:
      pAdapter->iAdapterRing = iData;
      break;
    case SWS_kP2pStatusDSR:
      pAdapter->iAdapterReady = iData;
      break;
    default:
      return;
      }

    /* Missing the DPC is not grossly serious */
    (void) PlatformDpc( &P2pStatIndDpc, pAdapter, NULL, iEvent);
    }
  }


/****************************************************************************
 * Callbacks
 ****************************************************************************/

/*
 * DPC to handle Rx input
 */
static SWS_CB( void, P2pRecvIndDpc(
  SWS_PTR( void) pv1,
  SWS_PTR( void) pv2,
  ui32 val
)){
  SWS_SP2pAdapter* const pAdapter = (SWS_SP2pAdapter*)pv1;
  (void)pv2;
  (void)val;

  TRACE( 5, ("P2pRecvIndDpc(%p)\n", pAdapter));
  ASSERT( PlatformIsCritical());

  if ( IS_P2P_ADAPTER( pAdapter))
    {
    AtomicWrite( &pAdapter->aRxDpc, 0);

    if ( kModemHayes != pAdapter->eModem)
      SWS_TimerStart( pAdapter->hRxIdleTimer, MS2TICKS( kRxIdleTimeout));
    /* Is CR present in input? */
    else if ( P2pAdapterPeek( pAdapter, kCR))
      P2pRecvReady( pAdapter, kCR);
    /* No CR, wait for input idle */
    else
      SWS_TimerStart( pAdapter->hRxIdleTimer, MS2TICKS( kHayesIdleTimeout));
    }
  }


/*
 * Rx Input ready
 */
static SWS_FASTCALL( void, P2pRecvReady(
  SWS_SP2pAdapter* pAdapter,
  int iEol
)){
  char szBuf[ 80];

  TRACE( 9, ("P2pRecvReady(%p)\n",pAdapter));

  while ( 0 < P2pAdapterRead( pAdapter, szBuf, sizeof( szBuf), iEol) )
    {
    int iRet = SWS_BASEERR;

    /* Is it a modem response */
    switch ( pAdapter->eModem)
      {
    case kModemNone:
      break;

    case kModemHayes:
      iRet = P2pModemResponse( pAdapter, szBuf);
      if ( iRet < SWS_BASEERR)
        {
        if ( -1 == pAdapter->iModemResponse)
          pAdapter->iModemResponse = P2pModemRsp2Err( iRet);
        }
      break;

    case kModemMsDirect:
      TRACE( 2, ("P2pRecvReady() msdirect read '%s'\n",szBuf));

      switch ( pAdapter->eState)
        {
      case kStateCalling:
        if ( NULL != strstr( szBuf, "CLIENTSERVER"))
          P2pOnConnected( pAdapter);
        return;

      case kStateListening:
        if ( NULL != strstr( szBuf, "CLIENT"))
          {
          TRACE( 2, ("P2pRecvReady(%s) msdirect send CLIENTSERVER\n", pAdapter->szIdent));
          P2pAdapterWrite( pAdapter, "CLIENTSERVER");
          P2pOnConnected( pAdapter);
          }
        return;

      default:
        break;
        }
      break;
      }

    /* Login? */
    if ( iRet >= SWS_BASEERR && kStateLogin == pAdapter->eState)
      P2pOnLogin( pAdapter, szBuf);
    }

  if ( P2pRxPending( pAdapter) > 0)
    {
    if ( kModemHayes == pAdapter->eModem)
      SWS_TimerStart( pAdapter->hRxIdleTimer, MS2TICKS( kHayesIdleTimeout));
    else
      SWS_TimerStart( pAdapter->hRxIdleTimer, MS2TICKS( kRxIdleTimeout));
    }
  }


/*
 * Rx idle timer callback
 */
static SWS_CB( void, RxIdleTimeout(
  SWS_TimerH hTimer,                  /* IN: Timer that expired */
  SWS_PTR( void) pv                   /* IN: User context */
)){
  SWS_SP2pAdapter* const pAdapter = (SWS_SP2pAdapter*)pv;
  (void)hTimer;

  TRACE( 9, ("RxIdleTimeout(%p)\n", pv));

  if ( IS_P2P_ADAPTER( pAdapter))
    P2pRecvReady( pAdapter, -1);
  }


/*
 * Login timer callback
 */
static SWS_CB( void, LoginTimeout(
  SWS_TimerH hTimer,                  /* IN: Timer that expired */
  SWS_PTR( void) pv                   /* IN: User context */
)){
  SWS_SP2pAdapter* const pAdapter = (SWS_SP2pAdapter*)pv;
  (void)hTimer;

  TRACE( 9, ("LoginTimeout(%p)\n", pAdapter));

  if ( IS_P2P_ADAPTER( pAdapter))
    {
    switch ( pAdapter->eState)
      {
    case kStateClosed:
    case kStateOpen:
    case kStateRinging:
    case kStateClosing:
    case kStateListening:
      break;

    case kStateCalling:
      switch ( pAdapter->eModem)
        {
      case kModemMsDirect:
        TRACE( 2, ("LoginTimeout(%s) msdirect send CLIENT\n", pAdapter->szIdent));
        P2pAdapterWrite( pAdapter, "CLIENT");
        SWS_TimerStart( hTimer, MS2TICKS( kMsModemTimeout2));
        break;

      case kModemNone:
      case kModemHayes:
        break;
        }
      break;

    case kStateLogin:
      TRACE( 2, ("LoginTimeout(%s)\n", pAdapter->szIdent));
      P2pOnLogin( pAdapter, NULL);
      break;

    case kStateConnected:
      TRACE( 2, ("LinkIdleTimeout closing %s\n", pAdapter->szIdent));
      P2pClose( pAdapter);
      break;

    case kStateAccepted:
      TRACE( 2, ("LinkIdleTimeout hangup %s\n", pAdapter->szIdent));
      P2pHangup( pAdapter);
      P2pListen( pAdapter, WAIT_NEVER);
      break;
      }
    }
  }


/*
 * DPC to handle adapter status change
 */
static SWS_CB( void, P2pStatIndDpc(
  SWS_PTR( void) pv1,
  SWS_PTR( void) pv2,
  ui32 val
)){
  SWS_SP2pAdapter* const pAdapter = (SWS_SP2pAdapter*)pv1;
  (void)pv2;

  TRACE( 5, ("P2pStatIndDpc(%p)\n", pAdapter));
  ASSERT( PlatformIsCritical());

  if ( IS_P2P_ADAPTER( pAdapter))
    {
    switch ( val)
      {
    case SWS_kP2pStatusCD:
      SWS_TimerStart( pAdapter->hDcdDebounceTimer, MS2TICKS( kDcdDebounce));
      break;

    case SWS_kP2pStatusRI:
      TRACE( 2, ("P2pStatIndDpc(%s) RI %d\n",
        pAdapter->szIdent, pAdapter->iAdapterRing));

      if ( pAdapter->iAdapterRing)
        P2pOnRing( pAdapter);
      break;

    case SWS_kP2pStatusDSR:
      SWS_TimerStart( pAdapter->hDsrDebounceTimer, MS2TICKS( kDsrDebounce));
      break;
      }
    }
  }

/* DCD debounce timeout */
static SWS_CB( void, DcdDebounceTimeout(
  SWS_TimerH hTimer,                  /* IN: Timer that expired */
  SWS_PTR( void) pv                   /* IN: User context */
)){
  SWS_SP2pAdapter* const pAdapter = (SWS_SP2pAdapter*)pv;
  (void)hTimer;

  TRACE( 9, ("DcdDebounceTimeout(%p)\n", pAdapter));

  if ( IS_P2P_ADAPTER( pAdapter))
    {
    TRACE( 2, ("DcdDebounceTimeout(%s) CD %d\n",
      pAdapter->szIdent, pAdapter->iAdapterCarrier));

    if ( !pAdapter->iAdapterCarrier)
      P2pOnLinkDown( pAdapter);
    }
  }

/* DSR debounce timeout */
static SWS_CB( void, DsrDebounceTimeout(
  SWS_TimerH hTimer,                  /* IN: Timer that expired */
  SWS_PTR( void) pv                   /* IN: User context */
)){
  SWS_SP2pAdapter* const pAdapter = (SWS_SP2pAdapter*)pv;
  (void)hTimer;

  TRACE( 9, ("DsrDebounceTimeout(%p)\n", pAdapter));

  if ( IS_P2P_ADAPTER( pAdapter))
    {
    TRACE( 2, ("DsrDebounceTimeout(%s) DSR %d\n",
      pAdapter->szIdent, pAdapter->iAdapterReady));

    if ( pAdapter->iAdapterReady)
      {
      if ( kModemNone == pAdapter->eModem)
        P2pOnConnected( pAdapter);
      }
    else
      P2pOnLinkDown( pAdapter);
    }
  }


/*
 * SWS_P2pStart callback
 */
static SWS_CB( int, P2pClientLogin(
  SWS_PTR( void) pvContext,           /* IN: Client context */
  unsigned uAdapter,                  /* IN: Adapter */
  int iEvent,                         /* IN: SWS_EP2pEvent */
  size_t size,                        /* IN: Size of event data */
  SWS_PTR( void) pvData               /* I/O: Event data */
)){
  enum { kLoginTimeout = 3000, kLinkIdleTimeout = 500}; /* millisecs */
  SWS_PTR( SWS_SP2pLogin) pInfo;
  SWS_SP2pAcct user;
  ui32 uAddr;
  int iLink;
  unsigned u;
  const char* psz;
  (void)size;
  (void)pvContext;

  TRACE( 9, ("P2pClientLogin(%p,%u,%d,%u,%p)\n",
    pvContext,uAdapter,iEvent,size,pvData));

  switch ( iEvent)
    {
  default:
    break;

  case SWS_kP2pEventIdle:
    PlatformIdle();
    return 0;

  case SWS_kP2pEventLogin:
    ASSERT( size >= sizeof( *pInfo) && NULL != pvData);
    pInfo = (SWS_PTR( SWS_SP2pLogin))pvData;

    /* Timeout? */
    if ( NULL == pInfo->pszRx)
      return 0; /* Yes, connection established */

    if ( 0 == strlen( pInfo->pszRx) )
      {
      /* Prompt for login */
      strcpy( pInfo->szTx, kszCR kszCR);
      pInfo->uTimeout = kLoginTimeout;
      }
    else if ( !SWS_stricmp( pInfo->pszRx, "Login: "))
      {
      VERIFY( !SWS_P2pGetAcct( uAdapter, &user));

      /* Send user name */
      strcpy( pInfo->szTx, user.szName);
      strcat( pInfo->szTx, kszCR);

      pInfo->uTimeout = kLoginTimeout;
      }
    else if ( !SWS_stricmp( pInfo->pszRx, "Password: "))
      {
      VERIFY( !SWS_P2pGetAcct( uAdapter, &user));

      /* Send password */
      strcpy( pInfo->szTx, user.szPassword);
      strcat( pInfo->szTx, kszCR);

      pInfo->uTimeout = kLinkIdleTimeout;
      }
    else if ( NULL != (psz = strstr( pInfo->pszRx, "address"))
      && SWS_INADDR_NONE != (uAddr = GetIp( psz))
    ) {
      TRACE( 2, ("P2pClientLogin() IP address %08lx\n",uAddr));

      iLink = SWS_DatalinkFind( SWS_P2pGetIdent( uAdapter));
      ASSERT( iLink >= 0);

      /* Set IP address */
      SWS_DatalinkSetProperty(
        (unsigned)iLink, SWS_kDatalinkProperty_Address, &uAddr, sizeof( uAddr) );

      pInfo->uTimeout = kLinkIdleTimeout;
      }
    else if ( NULL != (psz = strstr( pInfo->pszRx, "MTU"))
      && 1 == sscanf( psz, "%*[^0123456789]%u", &u)
    ) {
      size_t mtu;

      TRACE( 2, ("P2pClientLogin() MTU %u\n",(unsigned)u));

      iLink = SWS_DatalinkFind( SWS_P2pGetIdent( uAdapter));
      ASSERT( iLink >= 0);

      /* Set link MTU */
      mtu = u;
      SWS_DatalinkSetProperty(
        (unsigned)iLink, SWS_kDatalinkProperty_MTU, &mtu, sizeof( mtu) );

      pInfo->uTimeout = kLinkIdleTimeout;
      }

    return -1; /* Connection pending */
    }

  return SWS_EINVAL;
  }


/*
 * SWS_P2pStart callback
 */
static SWS_CB( int, P2pEvent(
  SWS_PTR( void) pvContext,           /* IN: Client context */
  unsigned uAdapter,                  /* IN: Adapter */
  int iEvent,                         /* IN: SWS_EP2pEvent */
  size_t size,                        /* IN: Size of event data */
  SWS_PTR( void) pvData               /* I/O: Event data */
)){
  (void)pvContext;
  (void)uAdapter;
  (void)size;
  (void)pvData;

  TRACE( 9, ("P2pEvent(%p,%u,%d,%u,%p)\n",
    pvContext,uAdapter,iEvent,size,pvData));

  switch ( iEvent)
    {
  default:
    break;

  case SWS_kP2pEventLogin:
    ASSERT( size >= sizeof( SWS_SP2pLogin) && NULL != pvData);
    return 0; /* Connection established */

  case SWS_kP2pEventIdle:
    PlatformIdle();
    return 0;
    }

  return SWS_EINVAL;
  }


/*
 * Search for IP address in string
 */
static SWS_FASTCALL( ui32, GetIp( SWS_PTR( const char) psz))
  {
  while ( NULL != psz)
    {
    unsigned u1, u2, u3, u4;

    if ( 4 == sscanf( psz, " %u.%u.%u.%u", &u1, &u2, &u3, &u4))
      {
      return u4
        | ((ui32)u3 << 8)
        | ((ui32)u2 << 16)
        | ((ui32)u1 << 24);
      }

    psz = strpbrk( psz + 1, "0123456789");
    }

  return SWS_INADDR_NONE;
  }

#else
char SWS_P2p;
#endif /* OPT_DATALINK_P2P */
/* End of file */
