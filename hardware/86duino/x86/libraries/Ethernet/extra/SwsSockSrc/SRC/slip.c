/****************************************************************************
 *
 * File name   : slip.c
 * Function    : SLIP data link
 * Project     : SwsSock
 * Authors     : Lawrence Rust
 * Systems     : ANSI C.
 *
 ****************************************************************************
 *
 * Created by Lawrence Rust, Software Systems Consultants
 * lvr@softsystem.co.uk. Tel/Fax +33 5 49 72 79 63
 *
 * 23-Oct-01, LVR, Created.
 * 06-Feb-02, LVR, Prevent missing end marker.from overruning rx buffer.
 * 22-Mar-02, LVR, Fix SlipSend to prevent callback sending data twice.
 * 26-Nov-02, LVR, Added SWS_kDatalinkProperty_HwAddress to EnetGetProperty.
 * 12-Oct-04, LVR, Fixed re-initialisation.by initing all static data
 * 11-Jun-05, LVR, Don't loopback self addressed data if subnet is ADDR_NONE
 * 17-Jun-05, LVR, Added sending init string in SWS_SlipAddAdapter
 * 16-Aug-05, LVR, Changed to P2P protocol
 *
 * Comments:
 * --------
 * This file implements a SLIP (RFC 1055) data link
 *
 ****************************************************************************
 */
/* Compilation options */
#include "config.h"

/* Exports */
#include "slip.h"

#if OPT_SLIP && OPT_DATALINK_P2P
/* Imports */
#include <stddef.h>
#include <string.h>
#include <ctype.h>

#include "sws_sock.h"  /* for SWS_INADDR_ANY and error codes */
#include "sws_buf.h"
#include "sws_p2p.h"
#include "sws_gef.h"

#include "platform.h"

#include "debug.h"


/*
 * Macros & Constants
 */

/* SLIP special character codes from RFC1055 */
enum
  {
  kSlipEnd    = 0300,                 /* End of packet, 0xc0 */
  kSlipEsc    = 0333,                 /* Byte stuffing, 0xdb */
  kSlipEscEnd = 0334,                 /* kSlipEsc, kSlipEscEnd == kSlipEnd */
  kSlipEscEsc = 0335                  /* kSlipEsc, kSlipEscEsc == kSlipEsc */
  };

enum
  {
  kSlipMaxFragment = 1006,            /* largest IP packet to transmit */

  /* In the worst case each byte will be escaped
   * together with a kSlipEnd to begin and end */
  kSlipMaxPacket = 2 * kSlipMaxFragment + 2
  };


/*
 * Local data types
 */

/* Adapter instance data */
typedef struct SSlipAdapter
  {
  ui32 uMagic;
#define ADAPTER_MAGIC SWS_MAKEFOURCC( 's','l','i','p')

  unsigned uP2pAdapter;               /* ID of assoc P2P adapter */
  SWS_P2pProtocolDn dn;               /* Adapter down call table */
  SWS_DatalinkCallbacks up;           /* Datalink up call table */
  int isUp;                           /* Adapter is up */

#if OPT_P2P_VJCOMP
  SWS_SP2pVjCompH hTxVjComp;          /* VJ compressor instance */
  SWS_SP2pVjCompH hRxVjComp;          /* VJ decompressor instance */
  int isVjSent;
#endif

  /* Tx buffer */
  unsigned txSize;
  void* pTxBuffer;
  volatile unsigned txWrite, txRead;  /* Write/read cursors */
  volatile unsigned uTxPending;       /* No. bytes pending in output device */
  volatile int iTxBusy;               /* SlipSendIp is pending */

  /* Get free space in Tx buffer */
#define SlipTxFree( pAdapter) \
  (unsigned)((pAdapter->txWrite >= pAdapter->txRead) \
  ? ((pAdapter->txSize - 1) - pAdapter->txWrite + pAdapter->txRead) \
  : ((pAdapter->txRead - pAdapter->txWrite) - 1))

  /* Get contiguous used space in Tx buffer */
#define SlipTxPending( pAdapter) \
  (unsigned)((pAdapter->txWrite < pAdapter->txRead) \
  ? (pAdapter->txSize - pAdapter->txRead) \
  : (pAdapter->txWrite - pAdapter->txRead))

  /* Rx buffer */
  unsigned rxSize;
  void* pRxBuffer;
  volatile unsigned rxWrite, rxRead;  /* Write/read cursors */
  volatile int iRxPending;
  volatile int iRxBusy;               /* SlipRecvIndDpc pending */

  /* Get contiguous free space in Rx buffer */
#define SlipRxFree( pAdapter) \
  (unsigned)((pAdapter->rxWrite < pAdapter->rxRead) \
  ? ((pAdapter->rxRead - pAdapter->rxWrite) - 1) \
  : (pAdapter->rxRead > 0) \
  ? (pAdapter->rxSize - pAdapter->rxWrite) \
  : ((pAdapter->rxSize - 1) - pAdapter->rxWrite))

  /* Get no. pending Rx bytes */
#define SlipRxPending( pAdapter) \
  (unsigned)((pAdapter->rxWrite < pAdapter->rxRead) \
  ? ((pAdapter->rxSize - pAdapter->rxRead) + pAdapter->rxWrite) \
  : (pAdapter->rxWrite - pAdapter->rxRead))

  /* The current input frame */
  SWS_BufferH hBuffer;
  SWS_size_t sizeBuf;
  SWS_PTR( ui8) pucBuf;
  unsigned uReceived;
  int iEsc;
  } SSlipAdapter;


/*
 * Prototypes
 */
#define IS_SLIP_ADAPTER( a) (NULL != (a) && ADAPTER_MAGIC == (a)->uMagic)
static SWS_FASTCALLDEF( void, SlipDestroyInstance( SSlipAdapter*));

/* Tx/Rx */
static SWS_FASTCALLDEF( int, SlipSendRaw( SSlipAdapter* const, SWS_BufferH));
static SWS_FASTCALLDEF( int, SlipTransmit( SSlipAdapter* const, unsigned));
static SWS_FASTCALLDEF( int, SlipReceive( SSlipAdapter* const));

/* P2P downcalls */
static SWS_FP2pProtoRelease SlipRelease;
static SWS_FP2pProtoSend SlipSendIp;
static SWS_FP2pProtoGetProperty SlipGetProperty;
static SWS_FP2pProtoLinkInd SlipLinkInd;

/* Adapter up calls */
static SWS_FP2pSendAck SlipSendAck;
static SWS_FP2pRecvInd SlipRecvInd;

static FPlatformDpc SlipRecvIndDpc;


/*
 * Module data
 */

/* Configuration */
static unsigned s_txSize;
static unsigned s_rxSize;

/* Stats */
static Atomic_t s_aTxPackets, s_aTxBytes;
static Atomic_t s_aTxErrors;          /* Driver send errors */
static Atomic_t s_aRxPackets, s_aRxBytes;
static Atomic_t s_aRxErrors;          /* Rx errors indicated */
static Atomic_t s_aRxNoBuffers;       /* Received packets dropped by driver */


/*
 * Initialise this module
 */
extern int SWS_SlipInit( void)        /* Returns !0 on error */
  {
  TRACE( 9, ("SWS_SlipInit()\n"));
  ASSERT( PlatformIsCritical());

  /* Init static data */
  AtomicWrite( &s_aTxPackets, 0);
  AtomicWrite( &s_aTxBytes, 0);
  AtomicWrite( &s_aTxErrors, 0);
  AtomicWrite( &s_aRxPackets, 0);
  AtomicWrite( &s_aRxBytes, 0);
  AtomicWrite( &s_aRxErrors, 0);
  AtomicWrite( &s_aRxNoBuffers, 0);

  /* Read config data */
  s_txSize = (unsigned)PlatformPropertyGetUint(
    PKEY_SLIP_TXBUFFER, OPT_SLIP_TXBUFFER );
  if ( s_txSize < kSlipMaxPacket)
    s_txSize = kSlipMaxPacket;

  s_rxSize = (unsigned)PlatformPropertyGetUint(
    PKEY_SLIP_RXBUFFER, OPT_SLIP_RXBUFFER );
  if ( s_rxSize < kSlipMaxPacket)
    s_rxSize = kSlipMaxPacket;

  return 0;
  }


/*
 * Finalise this module
 */
extern void SWS_SlipCleanup( void)
  {
  TRACE( 9, ("SWS_SlipCleanup()\n"));
  ASSERT( PlatformIsCritical());

  TRACE( 1, ("SLIP: TxPackets:%u TxBytes:%u TxErrors:%u"
    " RxPackets:%u RxBytes:%u RxErrors:%u RxNoBuffers:%u\n",
    AtomicRead( &s_aTxPackets), AtomicRead( &s_aTxBytes),
    AtomicRead( &s_aTxErrors),
    AtomicRead( &s_aRxPackets), AtomicRead( &s_aRxBytes),
    AtomicRead( &s_aRxErrors), AtomicRead( &s_aRxNoBuffers)
  ));
  }


/*
 * Bind the SLIP protocol to a P2P adapter
 */
extern int SWS_SlipBind(              /* Returns !0 on error */
  unsigned uAdapter,                  /* IN: P2P Adapter ID */
  SWS_PTR( const SWS_DatalinkCallbacks) pUp, /* IN: Up calls */
  SWS_PTR( const SWS_P2pProtocolDn) pDn, /* IN: Down calls */
  SWS_PTR( SWS_P2pProtocol) pOut         /* OUT: Callbacks */
) {
  SSlipAdapter* pAdapter;

  TRACE( 9, ("SWS_SlipBind(%u,%p,%p,%p)\n",
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
  pAdapter = (SSlipAdapter*) malloc( sizeof(*pAdapter) + 2 + s_rxSize + s_txSize);
  if ( NULL == pAdapter)
    {
    TRACE( 0, ("!!!SWS_SlipBind too many adapters\n"));
    return SWS_ENOMEM;
    }

  memset( pAdapter, 0, sizeof(*pAdapter));

  pAdapter->pRxBuffer = pAdapter + 1;
  pAdapter->pTxBuffer = (char*)pAdapter->pRxBuffer + s_rxSize + 1;
  pAdapter->rxSize = 1 + s_rxSize;
  pAdapter->txSize = 1 + s_txSize;
  ASSERT( SlipTxFree( pAdapter) == s_txSize);
  ASSERT( SlipRxFree( pAdapter) == s_rxSize);

  pAdapter->hBuffer = NULL;

  /* Save context info */
  pAdapter->uP2pAdapter = uAdapter;
  pAdapter->dn = *pDn;
  pAdapter->up = *pUp;

#if OPT_P2P_VJCOMP
  {
  ui8 uSlots = SWS_P2pVjSlots( uAdapter);
  switch ( uSlots)
    {
  case 0:
    break;

  case 1:
    pAdapter->hTxVjComp = SWS_P2pVjOpen( 15, 0);
    pAdapter->hRxVjComp = SWS_P2pVjOpen( 15, 0);
    pAdapter->isVjSent = 0;
    break;

  default:
    pAdapter->hTxVjComp = SWS_P2pVjOpen( uSlots, 0);
    pAdapter->hRxVjComp = SWS_P2pVjOpen( uSlots, 0);
    pAdapter->isVjSent = 1;
    break;
    }
  }
#endif

  pAdapter->uMagic = ADAPTER_MAGIC;

  /* Callbacks */
  pOut->pvContext = pAdapter;
  pOut->pfnRelease = &SlipRelease;
  pOut->pfnSend = &SlipSendIp;
  pOut->pfnSendAck = &SlipSendAck;
  pOut->pfnRecvInd = &SlipRecvInd;
  pOut->pfnLinkInd = &SlipLinkInd;
  pOut->pfnGetProperty = &SlipGetProperty;
  pOut->pfnSetProperty = 0;

  return 0;
  }


/*
 * Destroy an adapter
 */
static SWS_FASTCALL( void, SlipDestroyInstance( SSlipAdapter* pAdapter))
  {
  TRACE( 9, ("SlipDestroyInstance(%p)\n", pAdapter));
  ASSERT( pAdapter);

  if ( NULL != pAdapter->hBuffer)
    {
    SWS_BufferRelease( pAdapter->hBuffer);
    pAdapter->hBuffer = NULL;
    }

#if OPT_P2P_VJCOMP
  if ( NULL != pAdapter->hTxVjComp)
    SWS_P2pVjClose( pAdapter->hTxVjComp);

  if ( NULL != pAdapter->hRxVjComp)
    SWS_P2pVjClose( pAdapter->hRxVjComp);
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
SWS_CB( void, SlipRelease(
  void* pvContext
)){
  SSlipAdapter* const pAdapter = (SSlipAdapter*)pvContext;

  TRACE( 9, ("SlipRelease(%p)\n", pvContext));
  ASSERT( IS_SLIP_ADAPTER( pAdapter));

  SlipDestroyInstance( pAdapter);
  }


/*
 * Get an interface property
 */
static SWS_CB( int, SlipGetProperty(
  SWS_PTR( void) pvContext,
  int type,
  SWS_PTR( void) pvBuf,
  SWS_PTR( size_t) puLen
)){
  (void)pvContext;

  TRACE( 9, ("SlipGetProperty(%p,%d,%p,%p)\n", pvContext,type,pvBuf,puLen));
  ASSERT( NULL != pvBuf);
  ASSERT( NULL != puLen);

  switch ( type)
    {
  default:
    /* Not implemented */
    TRACE( 3, ("SlipGetProperty %d not implemented\n", type));
    return SWS_EBADF;

  case SWS_kDatalinkProperty_MTU:
    if ( *puLen < sizeof(size_t))
      {
      TRACE( 0, ("!!!SlipGetProperty buffer too small\n"));
      return SWS_EINVAL;
      }

    *(SWS_PTR(size_t))pvBuf = kSlipMaxFragment;
    *puLen = sizeof(size_t);
    break;
    }

  return 0;
  }


/*
 * Send an IP packet
 * Single threaded
 */
static SWS_CB( int, SlipSendIp(
  void* pvContext,
  SWS_BufferH hBuffer
)){
  SSlipAdapter* const pAdapter = (SSlipAdapter*)pvContext;
  int iRet;

  TRACE( 9, ("SlipSendIp(%p,%p)\n", pvContext, hBuffer));
  ASSERT( NULL != hBuffer);
  ASSERT( IS_SLIP_ADAPTER( pAdapter));
  ASSERT( PlatformIsCritical());

  if ( pAdapter->isUp > 0)
    {
#if OPT_P2P_VJCOMP
    /* Sending VJ compressed headers? */
    if ( NULL != pAdapter->hTxVjComp
      && pAdapter->isVjSent
    ) {
      SWS_PTR( ui8) pHdr;
      SWS_size_t len;
      SWS_EP2pVjType type;

      /* Yes, compress it */
      type = SWS_P2pVjCompress( pAdapter->hTxVjComp, hBuffer);

      /* Encode packet type as per RFC 1144 p43 */
      pHdr = (SWS_PTR( ui8)) SWS_BufferGetFragment( hBuffer, 0, &len);
      *pHdr |= (ui8)type;
      }
#endif

    iRet = SlipSendRaw( pAdapter, hBuffer);
    if ( iRet <= 0)
      {
      /* Don't report immediate completion since this causes
       * excessive buffering and increased latencies */
#if 0
      /* Is there room for another packet in the tx buffer? */
      if ( SlipTxFree( pAdapter) >= kSlipMaxPacket)
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
          ASSERT( SlipTxFree( pAdapter) >= kSlipMaxPacket);
          pAdapter->iTxBusy = 0;
          iRet = 0;
          }
        }
      }
    }
  else
    {
    TRACE( 1, ("*** SlipSendIp() link down\n"));
    iRet = SWS_ENETDOWN;
    }

  return iRet;
  }


/****************************************************************************
 * Up calls from adapter
 ****************************************************************************/

/*
 * SWS_FSlipSend async completion
 * !!! CAUTION IRQ TIME !!!
 */
static SWS_CB( void, SlipSendAck(
  SWS_PTR( void) pvContext,           /* IN: Client context */
  int iErr                            /* IN: !0 if error */
)){
  SSlipAdapter* const pAdapter = (SSlipAdapter*)pvContext;
  unsigned txRead;

  TRACE( 10, ("SlipSendAck(%p,%d)\n", pvContext, iErr));
  ASSERT( NULL != pAdapter);

  if ( !IS_SLIP_ADAPTER( pAdapter))
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
    unsigned const count = SlipTxPending( pAdapter);
    if ( 0 == count)
      break;

    if ( SlipTransmit( pAdapter, count))
      break;
    }

  /* Did SlipTransmit returned a pending status? */
  if ( pAdapter->iTxBusy)
    {
    /* Yes, then is there space for another packet? */
    if ( SlipTxFree( pAdapter) >= kSlipMaxPacket)
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
static SWS_CB( void, SlipRecvInd(
  SWS_PTR( void) pvContext,           /* IN: Client context */
  size_t uLen,                        /* IN: Data size */
  int iErr                            /* IN: Final error code, 0= no error */
)){
  SSlipAdapter* const pAdapter = (SSlipAdapter*)pvContext;
  unsigned u;

  TRACE( 10, ("SlipRecvInd(%p,%u,%d)\n", pvContext,uLen,iErr));
  ASSERT( NULL != pAdapter);

  if ( !IS_SLIP_ADAPTER( pAdapter))
    return;

  ASSERT( pAdapter->iRxPending > 0);
  pAdapter->iRxPending = 0;

  if ( iErr)
    {
    TRACE( 10, ("SlipRecvInd(%p) error %d\n", pAdapter, iErr));
    AtomicIncrement( &s_aRxErrors);

#if OPT_P2P_VJCOMP
    if ( NULL != pAdapter->hRxVjComp)
      SWS_P2pVjDecompress( pAdapter->hRxVjComp, SWS_kP2pVjTypeError, NULL);
#endif
    }

  AtomicAdd( &s_aRxBytes, (int)uLen);

  /* Update the receive buffer write cursor */
  u = pAdapter->rxWrite + uLen;
  if ( u >= pAdapter->rxSize)
    u -= pAdapter->rxSize;
  pAdapter->rxWrite = u;

  if ( pAdapter->isUp > 0)
    {
    /* Queue more Rx buffers */
    do
      {
      iErr = SlipReceive( pAdapter);
      if ( iErr)
        break;
      }
    while ( !pAdapter->iRxPending);
    }
  }


/*
 * Adapter link status change callback
 */
static SWS_CB( void, SlipLinkInd(
  SWS_PTR( void) pvContext,
  int iLinkUp
)){
  SSlipAdapter* const pAdapter = (SSlipAdapter*)pvContext;

  TRACE( 9, ("SlipLinkInd(%p,%d)\n", pvContext,iLinkUp));

  if ( IS_SLIP_ADAPTER( pAdapter))
    {
    switch ( iLinkUp)
      {
    case SWS_kP2pLinkGoingDown:
    case SWS_kP2pLinkDown:
      pAdapter->isUp = 0;

      /* Cancel pending Rx */
      if ( pAdapter->iRxPending)
        {
        size_t size = 0;
        (*pAdapter->dn.pfnRecv)( pAdapter->dn.pvContext, NULL, &size, -1, 0);
        pAdapter->iRxPending = 0;
        }

      /* Tell P2P that adapter can be closed */
      SWS_P2pLinkFinished( pAdapter->uP2pAdapter);
      break;

    case SWS_kP2pLinkUp:
      pAdapter->isUp = 1;

      /* Link up so post an Rx buffer */
      if ( !pAdapter->iRxPending)
        {
        pAdapter->rxRead = pAdapter->rxWrite = 0;
        (void) SlipReceive( pAdapter);
        }
      break;
      }

    /* Pass the indication up to the datalink */
    (*pAdapter->up.pfnLinkStatus)( pAdapter->up.pvContext, pAdapter->isUp);
    }
  }


/****************************************************************************
 * Transmit support
 ****************************************************************************/

/*
 * Append a byte to the Tx ring buffer
 */
static SWS_FASTCALL( void, SlipPut( SSlipAdapter* const pAdapter, ui8 data))
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
static SWS_FASTCALL( int, SlipTransmit(
  SSlipAdapter* const pAdapter,
  unsigned count
)){
  int iRet;

  TRACE( 9, ("SlipTransmit(%p,%u)\n", pAdapter,count));

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
        TRACE( 2, ("SlipTransmit(%p) error %d\n", pAdapter, iRet));
        AtomicIncrement( &s_aTxErrors);
        SWS_GefRaise( SWS_kGefSlipError, "Tx error");

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
static INLINE( int, SlipGet( SSlipAdapter* const pAdapter))
  {
  int data;
  unsigned u = pAdapter->rxRead;

  if ( u == pAdapter->rxWrite)
    return -1;

  data = ((ui8*)pAdapter->pRxBuffer)[ u];
  if ( ++u >= pAdapter->rxSize)
    u = 0;

  pAdapter->rxRead = u;
  return data;
  }


/*
 * Request input device for data
 * !!! CAUTION IRQ TIME !!!
 */
static SWS_FASTCALL( int, SlipReceive(
  SSlipAdapter* const pAdapter
)){
  int iRet;
  size_t count;

  TRACE( 10, ("SlipReceive(%p)\n", pAdapter));
  ASSERT( IS_SLIP_ADAPTER( pAdapter));
  ASSERT( !pAdapter->iRxPending);

  /* Get the available receive buffer space */
  count = SlipRxFree( pAdapter);
  if ( count > 0)
    {
    ASSERT( pAdapter->rxWrite + count <= pAdapter->rxSize);
    if ( count > kSlipMaxPacket)
      count = kSlipMaxPacket;

    /* Request device to receive */
    pAdapter->iRxPending = 1;

    ASSERT( pAdapter->dn.pfnRecv);
    iRet = (*pAdapter->dn.pfnRecv)(
      pAdapter->dn.pvContext,
      (ui8*)pAdapter->pRxBuffer + pAdapter->rxWrite,
      &count,
      kSlipEnd,
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
        {
        AtomicIncrement( &s_aRxErrors);
        SWS_GefRaise( SWS_kGefSlipError, "Rx error");
        }
      }
    }
  else
    {
    TRACE( 1, ("*** SlipReceive() RxBuffer overrun\n"));
    AtomicIncrement( &s_aRxNoBuffers);
    SWS_GefRaise( SWS_kGefSlipError, "RxBuffer overrun");

#if OPT_P2P_VJCOMP
    if ( NULL != pAdapter->hRxVjComp)
      SWS_P2pVjDecompress( pAdapter->hRxVjComp, SWS_kP2pVjTypeError, NULL);
#endif

    iRet = SWS_ENOBUFS;
    }

  /* Request a DPC if rx data pending */
  if ( pAdapter->rxWrite != pAdapter->rxRead
    && !pAdapter->iRxBusy
  ) {
    pAdapter->iRxBusy = 1;

    /* Request a DPC */
    if ( PlatformDpc( &SlipRecvIndDpc, pAdapter, NULL, 0) )
      {
      /* Q full */
      pAdapter->iRxBusy = 0;
      }
    }

  return iRet;
  }


/****************************************************************************
 * SLIP framing
 ****************************************************************************/

/*
 * Send a SLIP packet
 */
static SWS_FASTCALL( int, SlipSendRaw(
  SSlipAdapter* const pAdapter,
  SWS_BufferH hBuffer                 /* IN: Buffer to send (no header) */
)){
  int iRet;
  SWS_size_t uLen, uSrc;

  /* Call event filter */
  if ( 0 != (iRet = SWS_GefRaise( SWS_kGefSlipTx, hBuffer)))
    {
    TRACE( 3, ("SlipSendRaw() filtered %d\n", iRet));
    return iRet;
    }

  /* Ensure space for worst case output */
  uLen = SWS_BufferSize( hBuffer);
  if ( 2 * uLen + 2 > SlipTxFree( pAdapter) )
    {
    TRACE( 1, ("*** SlipSendIp() packet too big\n"));
    SWS_GefRaise( SWS_kGefSlipError, "Tx too big");
    return SWS_EMSGSIZE;
    }

  /* Karn (RFC1055): Send an initial END character to flush out any data
   * that may have accumulated in the receiver due to line noise */
  SlipPut( pAdapter, kSlipEnd);

  /* Process the buffer */
  uSrc = 0;
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

    /* Translate esc & end codes */
    while ( uLenSrc--)
      {
      ui8 data = *pSrc++;
      switch ( data)
        {
      default:
        SlipPut( pAdapter, data);
        break;
      case kSlipEnd:
        SlipPut( pAdapter, kSlipEsc);
        SlipPut( pAdapter, kSlipEscEnd);
        break;
      case kSlipEsc:
        SlipPut( pAdapter, kSlipEsc);
        SlipPut( pAdapter, kSlipEscEsc);
        break;
        }
      }
    }

  /* Append END */
  SlipPut( pAdapter, kSlipEnd);

  /* Send it */
  AtomicIncrement( &s_aTxPackets);
  iRet = 0;
  do
    {
    /* Get count of tx bytes pending */
    unsigned const count = SlipTxPending( pAdapter);
    if ( 0 == count)
      break; /* All sent */

    iRet = SlipTransmit( pAdapter, count);
    }
  /* Rpt until error or pending(busy) */
  while ( 0 == iRet);

  return iRet;
  }


/*
 * Received data indication - deferred procedure callback
 */
static SWS_CB( void, SlipRecvIndDpc(
  SWS_PTR( void) pvContext,           /* IN: Client context */
  SWS_PTR( void) pvData,              /* IN: */
  ui32 uLen                           /* IN: */
)){
  SSlipAdapter* const pAdapter = (SSlipAdapter*)pvContext;
  int data;
  (void)pvData;
  (void)uLen;

  TRACE( 5, ("SlipRecvIndDpc(%p,%p,%lu)\n",pvContext,pvData,uLen));
  ASSERT( IS_SLIP_ADAPTER( pAdapter));
  ASSERT( PlatformIsCritical());

  if ( !IS_SLIP_ADAPTER( pAdapter))
    return;

  ASSERT( pAdapter->iRxBusy);

  /* Scan the received data, translate escaped chars and store in output buffer */
  while ( (data = SlipGet( pAdapter)) >= 0)
    {
    if ( pAdapter->iEsc)
      {
      pAdapter->iEsc = 0;
      switch ( data)
        {
      case kSlipEscEsc:
        data = kSlipEsc;
        break;
      case kSlipEscEnd:
        data = kSlipEnd;
        break;
      default:
        TRACE( 1, ("*** SlipRecvIndDpc() Protocol violation %d\n",data));
        /* Just store the byte */
        break;
        }
      }
    else if ( kSlipEsc == data)
      {
      pAdapter->iEsc = 1;
      continue;
      }
    else if ( kSlipEnd == data)
      {
      /* Ignore empty packets caused by prefixing packet with END */
      if ( pAdapter->uReceived > 1)
        {
        SWS_BufferH hBuffer;
        SWS_size_t len;

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
        pAdapter->sizeBuf = 0;

#if OPT_P2P_VJCOMP
        if ( NULL != pAdapter->hRxVjComp)
          {
          SWS_EP2pVjType type;
          SWS_PTR( ui8) pHdr;

          /* Decode packet type as per RFC 1144 p43 */
          pHdr = (SWS_PTR( ui8)) SWS_BufferGetFragment( hBuffer, 0, &len);
          if ( SWS_kP2pVjTypeCompressed & *pHdr)
            {
            type = SWS_kP2pVjTypeCompressed;
            }
          else if ( *pHdr >= SWS_kP2pVjTypeUncompressed)
            {
            type = SWS_kP2pVjTypeUncompressed;
            *pHdr &= ~0x30;
            pAdapter->isVjSent = 1;
            }
          else
            type = SWS_kP2pVjTypeIP;

          /* Decompress */
          SWS_P2pVjDecompress( pAdapter->hRxVjComp, type, hBuffer);
          }
#endif

        /* Call event filter */
        if ( 0 == SWS_GefRaise( SWS_kGefSlipRx, hBuffer))
          {
          /* Pass frame up to IP layer */
          (*pAdapter->up.pfnRecvInd)(
            pAdapter->up.pvContext,
            hBuffer,
            0 /* broadcast */
          );
          }
        else
          {
          TRACE( 3, ("SlipRecvIndDpc() filtered\n"));
          }

        SWS_BufferRelease( hBuffer);
        }
      else
        {
        /* Discard runt frames */
        pAdapter->uReceived = 0;
        pAdapter->sizeBuf = 0;
        }

      continue;
      }

    /* Get an output buffer if required */
    if ( NULL == pAdapter->hBuffer)
      {
      pAdapter->hBuffer = SWS_BufferGet(
        kSlipMaxFragment, SWS_kBufferDefault);
      if ( NULL == pAdapter->hBuffer)
        {
        AtomicIncrement( &s_aRxNoBuffers);
        SWS_GefRaise( SWS_kGefSlipError, "Rx no buffer");

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
      /* No, get the next fragment */
      pAdapter->pucBuf = (SWS_PTR( ui8))SWS_BufferGetFragment(
        pAdapter->hBuffer,
        pAdapter->uReceived,
        &pAdapter->sizeBuf
      );
      if ( NULL == pAdapter->pucBuf)
        {
        /* Frame too big - kSlipEnd marker lost */
        TRACE( 2, ("SlipRecvIndDpc() Buffer overrun\n"));
        AtomicIncrement( &s_aRxErrors);
        SWS_GefRaise( SWS_kGefSlipError, "Rx frame too big");

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
  while ( pAdapter->isUp > 0 && !pAdapter->iRxPending)
    {
    if ( SlipReceive( pAdapter))
      break;
    }
  }

#else
char SWS_Slip;
#endif /*OPT_SLIP && OPT_DATALINK_P2P*/
/* End of file */
