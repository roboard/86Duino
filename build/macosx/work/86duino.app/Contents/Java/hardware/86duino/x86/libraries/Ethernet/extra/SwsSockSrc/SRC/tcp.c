/****************************************************************************
 *
 * File name   : tcp.c
 * Function    : TCP transport protocol
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
 * 07-Jun-01, LVR, In TcpOnRead cast RcvWnd to size_t.
 * 03-Oct-01, LVR, Fix TRACE warnings for ARM SDT 2.51 .
 * 18-Oct-01, LVR, Fix assertion failure in TcpRecvSeg
 *                 Added TcpSendInd to handle SWS_ENOBUFS in TcpOnWrite
 * 11-Dec-01, LVR, TcpBindSync doesn't fail with SWS_EADDRINUSE if conflicting
 *                 TCB is in 'timed wait'.
 * 30-Jan-02, LVR, Added OPT_TCP_PREALLOCATE_BUFFERS compilation option
 * 13-Feb-02, LVR, Remove check on packet len in TcpSendTo
 * 09-Apr-02, LVR, Disable ZeroWinTimer if zeroWinTimout is 0
 * 24-Apr-02, LVR, Fix off by 1 error in calcn of receive window size.
 *                 Fix 0 remote port in RST from Listen state.
 * 15-May-02, LVR, Improve error recovery: re-send all unack'd data after
 *                 ack received for re-transmitted segment.
 * 04-Nov-02, LVR, TcpOnReceive: reject an incoming connection request in
 *                 kStateListen if TcpOnCreate fails (e.g. no buffers).
 * 06-Nov-02, LVR, TcpRecvInd sends RST if no matching port.
 *                 Made OPT_TCP_PREALLOCATE_BUFFERS a runtime config option
 * 08-Nov-02, LVR, TcpRecvSeg always sets wantAck.  Fixes bug where no ack
 *                 was sent for single received segment if no Tx data.
 *                 TcpRecvAck correctly handles ACK to pending data & FIN.
 * 13-Nov-02, LVR, TcpSendPending accounts for seq no. assigned to FIN
 * 19-Nov-02, LVR, TcpOnReceive correctly sets RcvAckd when a SYN is received
 *                 in kStateSynSent and so cause TcpSendSeg to send an ACK.
 * 21-Nov-02, LVR, TcpOnRead tests TCB state before each wait. TcpNewState
 *                 sets SWS_kTransportEventRxReady for kStateCloseWait.
 * 09-Dec-02, LVR, TcpOnReceive sends RST to correct remote port if no TCB's.
 *                 Fix TcpOnError so that a RST received during SynReceived
 *                 causes the TCB to be destroyed.
 * 13-Dec-02, LVR, TcpSendSeg forces ACK if FIN was deferred..
 * 30-Sep-03, LVR, TcpRecvSeg calls TcpSendSeg before closing TCB if
 *                 rxDisable set.  TcpNewState => kStateFinWait2 doesn't set
 *                 SWS_ECONNRESET to allow close to wait for FIN.
 * 03-Oct-03, LVR, Added config for lo/hi auto port
 * 12-Oct-04, LVR, Fixed re-initialisation.by initing all static data
 * 06-Jan-05, LVR, Added.SWS_TcpEnum
 * 24-May-05, LVR, Add LAND attack detection to TcpOnReceive
 * 25-Jun-05, LVR, GetSock/PeerName return error if local/remote host unset
 * 14-Jul-05, LVR, TcpOnReceive accepts FIN if sent with data
 * 06-Jul-09, LVR, TcpTerminateTimedWait searches FinWait2 as well.
 * 05-Aug-09, LVR, Add beginnings of SACK.
 *
 *
 * Comments:
 * --------
 * This file implements the TCP transport
 *
 ****************************************************************************
 */

/* Compilation options */
#if defined _MSC_VER && _MSC_VER < 1000
 /* MSVC1 has problems with some of the INLINE fn's */
# define INLINE(_ret,_f) _ret _f
#endif

#include "config.h"

#ifndef OPT_TCP_SACKP
#define OPT_TCP_SACKP 0 /* !0 to send SACKP option with SYN */
#endif

/* Exports */
#include "sws_tcp.h"

#if OPT_PROTO_TCP
/* Imports */
#include <stddef.h>
#include <string.h>
#include <limits.h>

#include "sws_buf.h"
#include "sws_tprt.h"
#include "sws_ip.h"
#if OPT_PROTO_ICMP
#include "sws_icmp.h"
#endif
#include "sws_timr.h"
#include "sws_sock.h"  /* SWS_SOCK_STREAM */
#include "sws_gef.h"

#include "platform.h"

#include "debug.h"


/*
 * Macros & Constants
 */

/* TcpSendSeg flags */
#define FORCE_SNDF  0x100   /* Don't defer ACK */
#define REPEAT_SNDF 0x200   /* Retransmission ~*/

/* Sequence number operators */
#define SeqDelta( w1,w2) ((i32)(w1) - (i32)(w2))
#define SeqLE( w1,w2) ( SeqDelta( w1,w2) <= 0)
#define SeqLT( w1,w2) ( SeqDelta( w1,w2) < 0)

/* Transmission Control Block  state */
enum ETcbState
  {
  kStateClosed = 0,
  kStateListen,       /* waiting for a connection */
  kStateSynSent,      /* Connect request sent */
  kStateSynReceived,  /* Connect request received */
  kStateSynBoth,      /* Connect request sent & received */
  kStateEstablished,  /* Connection ok */
  kStateFinWait1,     /* Waiting for remote close */
  kStateFinWait2,     /* Waiting for remote close */
  kStateCloseWait,    /* Waiting for local close */
  kStateClosing,      /* Waiting for remote close ack */
  kStateLastAck,      /* Waiting for remote close ack */
  kStateTimeWait      /* Waiting for 2x MSL */
  };
#if DEBUG_TRACE_ENABLE
static const char* const kszStates[] =
  {
  "Closed",
  "Listen",
  "SynSent",
  "SynReceived",
  "SynBoth",
  "Established",
  "FinWait1",
  "FinWait2",
  "CloseWait",
  "Closing",
  "LastAck",
  "TimeWait"
  };
COMPILATION_REQUIRES( kStateClosed == 0);
COMPILATION_REQUIRES( kStateListen == 1);
COMPILATION_REQUIRES( kStateSynSent == 2);
COMPILATION_REQUIRES( kStateSynReceived == 3);
COMPILATION_REQUIRES( kStateSynBoth == 4);
COMPILATION_REQUIRES( kStateEstablished == 5);
COMPILATION_REQUIRES( kStateFinWait1 == 6);
COMPILATION_REQUIRES( kStateFinWait2 == 7);
COMPILATION_REQUIRES( kStateCloseWait == 8);
COMPILATION_REQUIRES( kStateClosing == 9);
COMPILATION_REQUIRES( kStateLastAck == 10);
COMPILATION_REQUIRES( kStateTimeWait == 11);
COMPILATION_REQUIRES( lengthof(kszStates) == 1 + kStateTimeWait);
#endif

/* TCB events:
 *
 * TcpOnCreate
 * TcpOnDestroy
 * TcpOnBind
 * TcpOnListen
 * TcpOnConnect
 * TcpOnWrite
 * TcpOnRead
 * TcpOnClose
 * TcpOnReceive
 * TcpOnError
 * TcpOnIcmpError
 * TcpOnRetransmitTimeout
 * TcpOnAckTimeout
 * TcpOnZeroWinTimeout
 */


/*
 * Local data types
 */

/* Segment round trip timing info */
typedef struct SRttInfo
  {
  ui32 seq;                           /* SEQ of last byte in the segment */
  ui32 sent;                          /* Time sent */
  i32 ticks;                          /* RTT */
  } SRttInfo;

/* A TCP Transmission Control Block */
typedef struct STcb
  {
  struct STcb* pNext;                 /* List of bound TCB's */

  /* Connection points */
  ui32 localHost;
  ui32 remoteHost;
  ui16 localPort;
  ui16 remotePort;

  /* get/setsockopt info */
  struct                              /* SWS_SOL_TCP flags */
    {
    unsigned noNagle : 1;             /* Disable Nagle RFC896 (SWS_TCP_NODELAY) */
    unsigned closeQuietly : 1;
    } opt;
  ui16 maxSeg;                        /* Segment size (SWS_TCP_MAXSEG) */
  ui16 soflags;                       /* SWS_SO_xxx flags */
  SWS_IpOptions ipOptions;            /* SWS_SOL_IP types */

  /* Transmission Control Block */
  enum ETcbState eState;

  int sndBufMallocd;                  /* Send buffer is malloc'd */
  void* pvSndBuf;                     /* Send buffer */
  size_t SndBuf;                      /* Size of send buffer */
  size_t SndNew;                      /* Index of first free */
  size_t SndOld;                      /* Index of unack'd data */
  size_t SndUp;                       /* No. bytes of unack'd urgent data */

  ui32 SndUna;                        /* send unacknowledged */
  ui32 SndNxt;                        /* send next */
  ui32 SndLast;                       /* Last sequence sent */
  ui32 SndWnd;                        /* Largest SEQ that can be sent */
  ui32 SndWL1;                        /* SEQ number used for last SndWnd update */
  ui32 SndWL2;                        /* ACK number used for last SndWnd update */
  ui32 SndWndMax;                     /* Largest send window size seen */
  size_t SndMSS;                      /* Maximum Segment Size to send */

  int rcvBufMallocd;                  /* Receive buffer is malloc'd */
  void* pvRcvBuf;                     /* Receive buffer */
  size_t RcvBuf;                      /* Size of receive buffer */
  size_t RcvNew;                      /* Index of first free */
  size_t RcvOld;                      /* Index of first unread data */
  size_t RcvPsh;                      /* Offset into receive buffer of last pushed data */

  ui32 RcvNxt;                        /* receive next */
  ui32 RcvAckd;                       /* Last ACK sent */
  ui32 RcvWnd;                        /* Sizeof receive window */
  unsigned RvcNoAck;                  /* No. received segments not yet ack'd */
  unsigned RcvDupAck;                 /* No. consecutive duplicate acks */

  /* Congestion control - RFC 2581 */
  ui32 cwnd;                          /* Unack'd send SEQ limit */
  ui32 ssthresh;                      /* Slow start threshold */

  /* Jacobson algorithm for RTO estimation ACM SIGCOMM-88
   * see ftp.ee.lbl.gov/papers/congavoid.ps.Z */
#define VJ_SCALEM 8
  ui32 sa;                            /* RTT mean scaled by VJ_SCALEM */
#define VJ_SCALED 4
  ui32 sv;                            /* RTT mean deviation scaled by VJ_SCALED */
  ui32 rto;                           /* retransmit timeout */
  ui32 retransmits;

  SWS_TimerH hRtoTimer;               /* Re-transmit & timed wait timer */
  SWS_TimerH hAckTimer;               /* Deferred ack timeout */
  SWS_TimerH hZeroWinTimer;           /* Zero window probe */
  ui32 zeroWinTimeout;                /* Zero window probe timeout */

  /* Round trip time estimation */
  unsigned rttQHead;
  unsigned rttQTail;
  SRttInfo rttQ[ 1 + OPT_TCP_RTO_QUEUE_LEN];

  /* Various action flags */
  char wantAck;                       /* Need to send an ACK */
  char deferredAck;                   /* Piggbacked ACK pending */
  char deferredFin;                   /* FIN pending after all data sent */
  char isStreaming;                   /* Last segment was full sized so skip Nagle check */
  char dpcPending;                    /* DPC pending for TcpSendSeg */
  char shutdown;
  char rxDisable;                     /* For receiver shutdown */

  /* NB the pre-allocated rcv & snd buffers follow this struct */
  } STcb;

/* Data used by TcpBindSync */
typedef struct SBindInfo
  {
  STcb* pTcb;
  int reuse;
  ui32 host;
  ui16 port;
  } SBindInfo;

/* Data used by TcpSendSync*/
typedef struct SSendInfo
  {
  STcb* pTcb;
  unsigned flags;                     /* TCP control flags */
  } SSendInfo;

/* Data used by TcpOnClose */
enum ECloseMode { ECloseShutdown, ECloseNormal, ECloseReset };
typedef struct SCloseInfo
  {
  STcb* pTcb;
  enum ECloseMode eMode;
  } SCloseInfo;

/* Context for TcpEnum */
struct SEnum
  {
  SWS_FNPTR( SWS_FTcpEnum) pfn;
  SWS_PTR( void) pvCtx;
  };


/*
 * Prototypes
 */
static FPlatformCallback TcpEnum;

/* TCB event handlers */
static SWS_FASTCALLDEF( int, TcpOnError( STcb* const, const int));
static SWS_FASTCALLDEF( void, TcpOnDestroy( STcb* const));
static SWS_FTimer TcpOnRetransmitTimeout;
static SWS_FTimer TcpOnAckTimeout;
static SWS_FTimer TcpOnZeroWinTimeout;
static FPlatformCallback TcpOnClose;

/* TCB support */
static int TcpSendSeg( STcb* const, unsigned, ui32, ui32);
static int TcpRecvSeg(
  STcb* const, SWS_BufferH const,unsigned,unsigned,ui32 const,unsigned const,int const);
static SWS_FASTCALLDEF( void, TcpRecvAck( STcb* const, ui32 const));
static STcb* TcpFindTcb(
  unsigned const, unsigned const, ui32 const, ui32 const);
static SWS_FASTCALLDEF( void, TcpCongestion( STcb* const));
static SWS_FASTCALLDEF( size_t, TcpRcvOptions( SWS_PTR( void), size_t));
static SWS_FASTCALLDEF( int, TcpPushRttQ( STcb* const, ui32));
static SWS_FASTCALLDEF( int, TcpPopRttQ( STcb* const, SRttInfo*));
static SWS_FASTCALLDEF( int, TcpPeekRttQ( STcb* const, SRttInfo*));
static SWS_FASTCALLDEF( void, TcpFlushRttQ( STcb* const));
static SWS_FASTCALLDEF( void, TcpTerminateTimedWait( void));
static SWS_FASTCALLDEF( size_t, TcpCalcSmss( STcb* const));

static FPlatformCallback TcpBindSync;
static FPlatformCallback TcpUnbindSync;
static FPlatformCallback TcpSendSync;
static FPlatformDpc TcpSendDpc;

/* Transport down calls */
static SWS_FTransportOpen TcpOpen;
static SWS_FTransportClose TcpClose;
static SWS_FTransportBind TcpBind;
static SWS_FTransportListen TcpListen;
static SWS_FTransportConnect TcpConnect;
static SWS_FTransportShutdown TcpShutdown;
static SWS_FTransportSendTo TcpSendTo;
static SWS_FTransportRecvFrom TcpRecvFrom;
static SWS_FTransportGetInfo TcpGetInfo;
static SWS_FTransportSetInfo TcpSetInfo;
static SWS_FTransportCleanup TcpCleanup;
static SWS_FTransportGetPeerName TcpGetPeerName;
static SWS_FTransportGetSockName TcpGetSockName;

/* IP upcalls */
static SWS_FIpRecvInd TcpRecvInd;
#if OPT_PROTO_ICMP
static SWS_FIpIcmpInd TcpIcmpInd;
#endif
static SWS_FIpSendInd TcpSendInd;


/*
 * Module data
 */
/* Configuration */
static size_t s_uTcpSaps;             /* No. SAPs */
static ui32 s_rto;                    /* Default retransmit timeout (RTO) */
static ui32 s_rtoMin;                 /* Min RTO */
static ui32 s_rtoMax;                 /* Max RTO - typically 2xMSL */
static ui32 s_synFail;                /* Max RTO for SYN segment */
static ui32 s_segFail;                /* Max RTO for data segment */
static ui32 s_timeWait;               /* TimeWait state delay, typically 2xMSL */
static ui32 s_ackDelay;               /* Max time ACK delayed for */
static size_t s_sndBuf;               /* Retransmit buffer size */
static size_t s_rcvBuf;               /* Receive buffer size */
static ui16 s_mssMax;                 /* Max MSS, 0= use MTU from IP */
static int s_preallocBufs;            /* !0 if tx & rx buffers pre alloc'd */
static int s_ignore_noport;           /* !0 to silently drop packets for closed ports */
static ui8 s_ttl;                     /* Default TTL, 0= IP default */
static ui16 s_loPort, s_hiPort;       /* Min/max auto port */

/* Stats */
static Atomic_t s_aTxDatagrams, s_aTxBytes;
static Atomic_t s_aRxDatagrams, s_aRxBytes;
static Atomic_t s_aTxErrors;          /* IP send errors */
static Atomic_t s_aRetransmits;       /* Datagrams re-transmitted */
static Atomic_t s_aRxErrors;          /* Datagrams received with errors */
static Atomic_t s_aRxNoPort;          /* Datagrams received with no matching port */
static Atomic_t s_aRxOverflow;        /* Rx Buffer overflows */
#if OPT_PROTO_ICMP
static Atomic_t s_aIcmpErrors;        /* ICMP errors received */
#endif

/* Bound TCB's */
static STcb* volatile s_pBoundTcps;   /* List of bound SAPs */
static ui16 s_port;                   /* Next auto port number */

static int s_iTxResources;            /* IP send ready */

/* TCP transport registration data */
static const SWS_TransportDesc s_transportTcp = {
  sizeof( s_transportTcp),
  SWS_TRANSPORT_VERSION,
  SWS_AF_INET,
  SWS_SOCK_STREAM,
  SWS_IPPROTO_TCP,
  0,
  "tcp",
  &TcpOpen,
  &TcpClose,
  &TcpBind,
  &TcpListen,
  &TcpConnect,
  &TcpShutdown,
  &TcpSendTo,
  &TcpRecvFrom,
  &TcpGetInfo,
  &TcpSetInfo,
  &TcpCleanup,
  &TcpGetSockName,
  &TcpGetPeerName
};
static int s_bRegTransport;

/* TCP protocol registration data */
static const SWS_IpProtocolDesc s_protocolTcp = {
  sizeof( s_protocolTcp),
  SWS_PROTOCOL_VERSION,
  SWS_TCP_PROTOCOL,
  0,
  &TcpRecvInd,
#if OPT_PROTO_ICMP
  &TcpIcmpInd,
#else
  0,
#endif
  &TcpSendInd
};
static int s_bRegProtocol;


/*
 * Initialise this module
 * Single threaded
 */
int SWS_TcpInit( void)                /* Returns !0 on error */
  {
  int iErr;

  TRACE( 9, ("SWS_TcpInit()\n"));
  ASSERT( PlatformIsCritical());
  ASSERT( NULL == s_pBoundTcps);
  ASSERT( !s_bRegTransport);
  ASSERT( !s_bRegProtocol);

  /* Init static data */
  s_pBoundTcps = NULL;

  /* Read config data */
  s_uTcpSaps = (size_t)PlatformPropertyGetUint( PKEY_TCP_SOCKETS, OPT_TCP_SOCKETS );
  if ( 0 == s_uTcpSaps)
    return 0;

  s_rto = PlatformPropertyGetUint( PKEY_TCP_RTO, OPT_TCP_RTO );
  s_rtoMin = PlatformPropertyGetUint( PKEY_TCP_RTO_MIN, OPT_TCP_RTO_MIN );
  s_rtoMax = PlatformPropertyGetUint( PKEY_TCP_RTO_MAX, OPT_TCP_RTO_MAX );
  s_synFail = PlatformPropertyGetUint( PKEY_TCP_SYN_FAIL, OPT_TCP_SYN_FAIL );
  s_segFail = PlatformPropertyGetUint( PKEY_TCP_SEG_FAIL, OPT_TCP_SEG_FAIL );
  s_timeWait = PlatformPropertyGetUint( PKEY_TCP_TIMEWAIT, OPT_TCP_TIMEWAIT );
  s_ackDelay = PlatformPropertyGetUint( PKEY_TCP_ACKWAIT, OPT_TCP_ACKWAIT );
  s_rcvBuf = (size_t)PlatformPropertyGetUint(
    PKEY_TCP_RCVBUF, OPT_TCP_RCVBUF );
  s_sndBuf = (size_t)PlatformPropertyGetUint(
    PKEY_TCP_SNDBUF, OPT_TCP_SNDBUF );
  s_mssMax = (ui16)PlatformPropertyGetUint(
    PKEY_TCP_MSS_MAX, OPT_TCP_MSS_MAX );
  s_preallocBufs = (int)PlatformPropertyGetUint(
    PKEY_TCP_PREALLOC, OPT_TCP_PREALLOCATE_BUFFERS );
  s_ignore_noport = (int)PlatformPropertyGetUint(
    PKEY_TCP_NOPORT, OPT_TCP_DISCARD_NOPORT );
  s_ttl = (ui8)PlatformPropertyGetUint( PKEY_TCP_TTL, OPT_TCP_TTL );
  s_port = s_loPort = (ui16)PlatformPropertyGetUint(
    PKEY_TCP_LOPORT, OPT_TCP_MIN_PORT );
  s_hiPort = (ui16)PlatformPropertyGetUint(
    PKEY_TCP_HIPORT, OPT_TCP_MAX_PORT );
  if ( s_loPort > s_hiPort)
    return 0;

  /* Register the TCP protocol */
  iErr = SWS_IpRegisterProtocol( &s_protocolTcp, NULL);
  if ( !iErr)
    {
    ui32 size;

    s_bRegProtocol = 1;
    s_iTxResources = 1;

    size = sizeof( STcb);
    if ( s_preallocBufs)
      {
      size += (ui32)s_sndBuf + (ui32)s_rcvBuf;
#if SWS_SIZE_MAX < ULONG_MAX
      ASSERT( size <= SWS_SIZE_MAX);
#endif
      }

    /* Register the TCP transport */
    iErr = SWS_TransportRegister(
      &s_transportTcp,
      NULL,
      s_uTcpSaps,
      (size_t)size
    );
    if ( !iErr)
      s_bRegTransport = 1;
    else
      {
      VERIFY( !SWS_IpDeregisterProtocol( &s_protocolTcp, NULL));
      s_bRegProtocol = 0;
      }
    }

  return iErr;
  }


/*
 * Finalise this module
 * Single threaded
 */
void SWS_TcpCleanup( void)
  {
  TRACE( 9, ("SWS_TcpCleanup()\n"));
  ASSERT( PlatformIsCritical());

  ASSERT( !s_bRegTransport);
  ASSERT( !s_bRegProtocol);

#if OPT_PROTO_ICMP
# define ICMP_ERRORS AtomicRead( &s_aIcmpErrors)
#else
# define ICMP_ERRORS 0
#endif

  TRACE( 1, ("TCP: TxDatagrams:%u TxBytes:%u TxErrors:%u Retransmits:%u"
    " RxDatagrams:%u RxBytes:%u RxErrors:%u RxOverflows:%u"
    " RxNoPort:%u IcmpErrors:%u\n",
    AtomicRead( &s_aTxDatagrams), AtomicRead( &s_aTxBytes),
    AtomicRead( &s_aTxErrors), AtomicRead( &s_aRetransmits),
    AtomicRead( &s_aRxDatagrams), AtomicRead( &s_aRxBytes),
    AtomicRead( &s_aRxErrors), AtomicRead( &s_aRxOverflow),
    AtomicRead( &s_aRxNoPort), ICMP_ERRORS
  ));
  }


/*
 * Enumerate TCP connections.
 */
SWS_EXPORT( int, SWS_TcpEnum(
  SWS_FNPTR( SWS_FTcpEnum) pfn,       /* IN: Callback */
  SWS_PTR( void) pvCtx                /* IN: Callback context */
)){
  struct SEnum ctx;

  TRACE( 9, ("SWS_TcpEnum(%p,%p)\n", pfn, pvCtx));

  if ( !pfn)
    {
    TRACE( 1, ("*** SWS_TcpEnum() no callback\n"));
    return -1;
    }

  ctx.pfn = pfn;
  ctx.pvCtx = pvCtx;
  return PlatformSynchronise( &TcpEnum, &ctx);
  }

SWS_CB( int, TcpEnum( SWS_PTR( void) pv))
  {
  SWS_FNPTR( SWS_FTcpEnum) const pfn = ((SWS_PTR( const struct SEnum)) pv)->pfn;
  SWS_PTR( void) const pvCtx =((SWS_PTR( const struct SEnum)) pv)->pvCtx;
  const STcb* pTcb;

  TRACE( 9, ("TcpEnum(%p)\n", pv));
  ASSERT( PlatformIsCritical());

  for ( pTcb = s_pBoundTcps; NULL != pTcb; pTcb = pTcb->pNext)
    {
    if ( kStateClosed != pTcb->eState)
      {
      int iRet;
      SWS_STcpConn info;

      info.iState = pTcb->eState;
      info.localHost = pTcb->localHost;
      info.localPort = pTcb->localPort;
      info.remoteHost = pTcb->remoteHost;
      info.remotePort = pTcb->remotePort;

      iRet = (*pfn)( pvCtx, &info);
      if ( iRet)
        return iRet;
      }
    }

  return 0;
  }


/****************************************************************************
 * Inline helpers
 ****************************************************************************/

/*
 * Calc used space in re-transmit buffer
 */
static INLINE( size_t, TcpSendUsed( STcb* const pTcb))
  {
  if ( pTcb->SndNew >= pTcb->SndOld)
    return pTcb->SndNew - pTcb->SndOld;

  return pTcb->SndBuf - pTcb->SndOld + pTcb->SndNew;
  }


/*
 * Convert send sequence no. to re-transmit buffer index
 */
static INLINE( size_t, TcpSendCursor( STcb* const pTcb, const ui32 seq))
  {
  i32 SndCur = SeqDelta( seq, pTcb->SndUna);
  ASSERT( SndCur >= 0);

  SndCur += (i32)pTcb->SndOld;
  if ( SndCur < (i32)pTcb->SndBuf)
    return (size_t)SndCur;

  return (size_t)(SndCur - pTcb->SndBuf);
  }


/*
 * Calc amount of data not yet sent from re-transmit buffer
 */
static INLINE( size_t, TcpSendPending( STcb* const pTcb, const ui32 seq))
  {
#if 1
  size_t used;
  i32 offset;

  offset = SeqDelta( seq, pTcb->SndUna);
  ASSERT( offset >= 0);

  used = TcpSendUsed( pTcb);
  if ( used <= (ui32)offset)
    return 0; /* Happens if seq = FIN */

  return used - (size_t)offset;

#else
  size_t SndCur;

  if ( pTcb->SndNew == pTcb->SndOld)
    return 0;

  SndCur = TcpSendCursor( pTcb, seq);
  if ( pTcb->SndNew >= SndCur)
    return pTcb->SndNew - SndCur;

  return (pTcb->SndBuf - SndCur) + pTcb->SndNew;

#endif
  }


/*
 * Calc no. urgent bytes not yet sent from re-transmit buffer
 */
static INLINE( size_t, TcpSendUrgent( STcb* const pTcb, const ui32 seq))
  {
  size_t sent;

  if ( !pTcb->SndUp)
    return 0;

  sent = TcpSendCursor( pTcb, seq);
  if ( sent >= pTcb->SndOld)
    sent -= pTcb->SndOld;
  else
    sent += pTcb->SndBuf - pTcb->SndOld;

  return pTcb->SndUp > sent ? pTcb->SndUp - sent : 0;
  }


/*
 * Calc contiguous space in re-transmit buffer
 */
static INLINE( size_t, TcpSendAvailable( STcb* const pTcb))
  {
  if ( pTcb->SndOld > pTcb->SndNew)
    return pTcb->SndOld - pTcb->SndNew - 1;

  if ( pTcb->SndOld)
    return pTcb->SndBuf - pTcb->SndNew;

  return pTcb->SndBuf - pTcb->SndNew - 1;
  }


/*
 * Calc amount of data in receive buffer
 */
static INLINE( size_t, TcpRecvPending( STcb* const pTcb))
  {
  if ( pTcb->RcvNew >= pTcb->RcvOld)
    return pTcb->RcvNew - pTcb->RcvOld;

  return (pTcb->RcvBuf - pTcb->RcvOld) + pTcb->RcvNew;
  }


/*
 * Calc amount of free space in receive buffer
 */
static INLINE( size_t, TcpRecvFree( STcb* const pTcb))
  {
  if ( pTcb->RcvOld > pTcb->RcvNew)
    return pTcb->RcvOld - (1 + pTcb->RcvNew);

  return (pTcb->RcvBuf - (1 + pTcb->RcvNew)) + pTcb->RcvOld;
  }


/****************************************************************************
 * TCP state transitions:
 ****************************************************************************/

static SWS_FASTCALL( void, TcpNewState(
  STcb* const pTcb,
  enum ETcbState const eState
)){
  TRACE( 2, ("TcpNewState(%p) %s -> %s\n",
    pTcb, kszStates[ pTcb->eState], kszStates[ eState]));

  switch ( eState)
    {
  case kStateClosed:
    switch ( pTcb->eState)
      {
    case kStateClosed:
      break;

    case kStateListen:
    case kStateSynSent:
    case kStateSynReceived:
    case kStateSynBoth:
    case kStateEstablished:
    case kStateFinWait1:
    case kStateFinWait2:
    case kStateClosing:
    case kStateLastAck:
    case kStateTimeWait:
    case kStateCloseWait:
      SWS_TimerStop( pTcb->hRtoTimer);
      SWS_TimerStop( pTcb->hAckTimer);
      SWS_TimerStop( pTcb->hZeroWinTimer);

      /* Unlink from bound list */
      PlatformSynchronise( &TcpUnbindSync, pTcb);
      pTcb->localPort = 0;
      pTcb->localHost = SWS_INADDR_ANY;
#if 0 /* Preserve endpoints until transport layer destroys the TCB */
      pTcb->remoteHost = SWS_INADDR_ANY;
      pTcb->remotePort = 0;
#endif
      pTcb->SndWnd = 0;
      pTcb->SndWndMax = 0;
      pTcb->SndUp = 0;
      pTcb->deferredAck = 0;
      pTcb->deferredFin = 0;
      pTcb->wantAck = 0;
      pTcb->RcvWnd = 0;
      pTcb->RvcNoAck = 0;
      pTcb->RcvDupAck = 0;

      pTcb->SndNew = pTcb->SndOld = 0;

      pTcb->rto = s_rto;  /* Initial receive timeout */
      pTcb->sa = 0;
      pTcb->sv = s_rto;
      pTcb->retransmits = 0;
      pTcb->rttQTail = pTcb->rttQHead = 0;
      break;
      }

    pTcb->eState = eState;
    break;

  case kStateListen:
    ASSERT( kStateClosed == pTcb->eState);
    pTcb->eState = eState;
    break;

  case kStateSynSent:
    ASSERT( kStateClosed == pTcb->eState);
    ASSERT( TcpFindTcb(
      pTcb->localPort, pTcb->remotePort, pTcb->localHost, pTcb->remoteHost));
    pTcb->eState = eState;
    break;

  case kStateSynBoth:
    ASSERT( kStateSynSent == pTcb->eState);
    ASSERT( TcpFindTcb(
      pTcb->localPort, pTcb->remotePort, pTcb->localHost, pTcb->remoteHost));
    pTcb->eState = eState;
    break;

  case kStateSynReceived:
    ASSERT( kStateClosed == pTcb->eState);
    ASSERT( TcpFindTcb(
      pTcb->localPort, pTcb->remotePort, pTcb->localHost, pTcb->remoteHost));
    pTcb->eState = eState;
    break;

  case kStateEstablished:
    ASSERT( kStateSynSent == pTcb->eState
      || kStateSynBoth == pTcb->eState
      || kStateSynReceived == pTcb->eState
    );
    ASSERT( TcpFindTcb(
      pTcb->localPort, pTcb->remotePort, pTcb->localHost, pTcb->remoteHost));
    pTcb->eState = eState;
    SWS_TransportEventSet( pTcb, SWS_kTransportEventConnect, 1);
    break;

  case kStateFinWait1:
    ASSERT( kStateEstablished == pTcb->eState
      || kStateSynReceived == pTcb->eState);
    pTcb->eState = eState;
    break;

  case kStateFinWait2:
    ASSERT( kStateFinWait1 == pTcb->eState);
    pTcb->eState = eState;
    break;

  case kStateClosing:
    ASSERT( kStateFinWait1 == pTcb->eState);
    pTcb->eState = eState;
    SWS_TransportEventSet( pTcb, SWS_kTransportEventError, SWS_ECONNRESET);
    break;

  case kStateLastAck:
    ASSERT( kStateCloseWait == pTcb->eState);
    pTcb->eState = eState;
    break;

  case kStateCloseWait:
    ASSERT( kStateSynReceived == pTcb->eState
      || kStateSynBoth == pTcb->eState
      || kStateEstablished == pTcb->eState
    );
    pTcb->eState = eState;
    /* Wake waiting receiver to detect state change */
    SWS_TransportEventSet( pTcb, SWS_kTransportEventRxReady, 1);
    break;

  case kStateTimeWait:
    ASSERT( kStateClosing == pTcb->eState
      || kStateFinWait2 == pTcb->eState
    );
    pTcb->eState = eState;

    /* Wakeup all pending reads */
    SWS_TransportEventSet( pTcb, SWS_kTransportEventError, SWS_ENOTCONN);

    if ( !pTcb->shutdown)
      {
      /* Start timeout to close */
      VERIFY( !SWS_TimerStart( pTcb->hRtoTimer, s_timeWait));
      }
    else
      SWS_TimerStop( pTcb->hRtoTimer);
    break;
    }
  }


/****************************************************************************
 * TCP event handlers:
 ****************************************************************************/

/*
 * Create a TCB
 */
static SWS_FASTCALL( int, TcpOnCreate(
  STcb* const pTcb,
  size_t const sndBuf,                /* IN: Send buffer size */
  size_t const rcvBuf                 /* IN: Receive buffer size */
)){
  TRACE( 9, ("TcpOnCreate(%p.%u,%u)\n", pTcb,(unsigned)sndBuf,(unsigned)rcvBuf));
  ASSERT( NULL != pTcb);

  /* Initialise the TCB */
  ASSERT( kStateClosed == pTcb->eState);
  pTcb->pNext = NULL;
  pTcb->remoteHost = SWS_INADDR_ANY;
  pTcb->remotePort = 0;
  pTcb->localHost = SWS_INADDR_ANY;
  pTcb->localPort = 0;

  pTcb->maxSeg = s_mssMax;
  pTcb->opt.noNagle = 0;
  pTcb->opt.closeQuietly = 0;
  pTcb->ipOptions.ttl = s_ttl;
  pTcb->ipOptions.tos = SWS_IP_TOS_DEFAULT;
  pTcb->ipOptions.flags.df = SWS_IP_DONT_FRAGMENT;

  /* TODO: Recv non-inline OOB data */
  pTcb->soflags = SWS_SO_OOBINLINE;

  pTcb->SndWnd = 0;
  pTcb->SndWndMax = 0;
  pTcb->SndUp = 0;
  pTcb->deferredAck = 0;
  pTcb->deferredFin = 0;
  pTcb->RcvWnd = 0;
  pTcb->rxDisable = 0;
  pTcb->RvcNoAck = 0;
  pTcb->RcvDupAck = 0;

  pTcb->SndNew = pTcb->SndOld = 0;
  pTcb->RcvNew = pTcb->RcvOld = 0;

  pTcb->sv = pTcb->rto = s_rto;  /* Initial receive timeout */
  pTcb->sa = 0;
  pTcb->shutdown = 0;
  pTcb->retransmits = 0;
  pTcb->rttQTail = pTcb->rttQHead = 0;

  /* Allocate send/receive ring buffers */
  pTcb->rcvBufMallocd = 0;
  pTcb->sndBufMallocd = 0;

  /* Is requested receive buffer size > that pre-allocated with the TCB? */
  ASSERT( NULL == pTcb->pvRcvBuf);
  pTcb->RcvBuf = s_rcvBuf;
  if ( rcvBuf <= s_rcvBuf
    && s_preallocBufs
  ) {
    /* Yes, use pre-allocated buffer */
    pTcb->pvRcvBuf = pTcb + 1;
    }
  else
    {
    /* Allocate bigger buffer */
    pTcb->pvRcvBuf = malloc( pTcb->RcvBuf = rcvBuf);
    if ( NULL == pTcb->pvRcvBuf)
      {
      TRACE( 1, ("*** TcpOnCreate() Out of memory for receive buffer\n"));
      TcpOnDestroy( pTcb);
      return SWS_ENOBUFS;
      }
    pTcb->rcvBufMallocd = 1;
    }

  /* Is requested send buffer size <= that pre-allocated with the TCB? */
  ASSERT( NULL == pTcb->pvSndBuf);
  pTcb->SndBuf = s_sndBuf;
  if ( pTcb->SndBuf <= s_sndBuf
    && s_preallocBufs
  ) {
    /* Yes, use pre-allocated buffer */
    pTcb->pvSndBuf = (SWS_PTR( char))(pTcb + 1) + s_rcvBuf;
    }
  else
    {
    /* Allocate bigger buffer */
    pTcb->pvSndBuf = malloc( pTcb->SndBuf = sndBuf);
    if ( NULL == pTcb->pvSndBuf)
      {
      TRACE( 1, ("*** TcpOnCreate() Out of memory for send buffer\n"));
      TcpOnDestroy( pTcb);
      return SWS_ENOBUFS;
      }
    pTcb->sndBufMallocd = 1;
    }

  /* Allocate timers */
  ASSERT( NULL == pTcb->hRtoTimer);
  pTcb->hRtoTimer = SWS_TimerAlloc( &TcpOnRetransmitTimeout, pTcb);
  if ( NULL == pTcb->hRtoTimer)
    {
    TcpOnDestroy( pTcb);
    return SWS_ENOBUFS;
    }

  ASSERT( NULL == pTcb->hAckTimer);
  pTcb->hAckTimer = SWS_TimerAlloc( &TcpOnAckTimeout, pTcb);
  if ( NULL == pTcb->hAckTimer)
    {
    TcpOnDestroy( pTcb);
    return SWS_ENOBUFS;
    }

  ASSERT( NULL == pTcb->hZeroWinTimer);
  pTcb->hZeroWinTimer = SWS_TimerAlloc( &TcpOnZeroWinTimeout, pTcb);
  if ( NULL == pTcb->hZeroWinTimer)
    {
    TcpOnDestroy( pTcb);
    return SWS_ENOBUFS;
    }

  return 0;
  }


/*
 * Destroy a TCB
 */
static SWS_FASTCALL( void, TcpOnDestroy(
  STcb* const pTcb
)){
  TRACE( 9, ("TcpOnDestroy(%p)\n", pTcb));
  ASSERT( NULL != pTcb);

  ASSERT( NULL != pTcb);
  ASSERT( kStateClosed == pTcb->eState);

  /* Unlink from bound list */
  PlatformSynchronise( &TcpUnbindSync, pTcb);

  /* Abort all pending ops */
  SWS_TransportEventSet( pTcb, SWS_kTransportEventError, SWS_ENETDOWN);

  /* Release resources */
  if ( NULL != pTcb->hZeroWinTimer)
    {
    SWS_TimerRelease( pTcb->hZeroWinTimer);
    pTcb->hZeroWinTimer = NULL;
    }

  if ( NULL != pTcb->hAckTimer)
    {
    SWS_TimerRelease( pTcb->hAckTimer);
    pTcb->hAckTimer = NULL;
    }

  if ( NULL != pTcb->hRtoTimer)
    {
    SWS_TimerRelease( pTcb->hRtoTimer);
    pTcb->hRtoTimer = NULL;
    }

  /* Release buffers */
  if ( pTcb->sndBufMallocd)
    {
    pTcb->sndBufMallocd = 0;
    free( pTcb->pvSndBuf);
    }
  pTcb->pvSndBuf = NULL;

  if ( pTcb->rcvBufMallocd)
    {
    pTcb->rcvBufMallocd = 0;
    free( pTcb->pvRcvBuf);
    }
  pTcb->pvRcvBuf = NULL;
  }

/* Single threaded callback to unbind a TCB */
static SWS_CB( int, TcpUnbindSync( SWS_PTR( void) pv))
  {
  STcb* const pTcb = (STcb*)pv;
  STcb* pPrev;

  ASSERT( NULL != pTcb);
  ASSERT( PlatformIsCritical());

  pPrev = s_pBoundTcps;
  if ( pPrev == pTcb)
    {
    s_pBoundTcps = pTcb->pNext;
    }
  else
    {
    for ( ; NULL != pPrev; pPrev = pPrev->pNext)
      {
      if ( pPrev->pNext == pTcb)
        {
        pPrev->pNext = pTcb->pNext;
        break;
        }
      }
    }

  pTcb->pNext = NULL;

  return 0;
  }


/*
 * Terminate a TCB
 */
static SWS_FASTCALL( int, TcpOnError( STcb* const pTcb, const int err ))
  {
  TRACE( 2, ("TcpOnError(%p,%d)\n", pTcb,err));
  ASSERT( NULL != pTcb);
  ASSERT( PlatformIsCritical());

  SWS_TimerStop( pTcb->hRtoTimer);
  SWS_TimerStop( pTcb->hAckTimer);
  SWS_TimerStop( pTcb->hZeroWinTimer);

  switch ( pTcb->eState)
    {
  case kStateClosed:
    break;

  case kStateListen:
    break;

  case kStateSynSent:
  case kStateSynBoth:
    TcpNewState( pTcb, kStateClosed);

    /* Abort all pending ops */
    SWS_TransportEventSet( pTcb, SWS_kTransportEventError, err ? err : 1);
    break;

  case kStateSynReceived: /* This is a new TCB so don't listen */
    TcpNewState( pTcb, kStateClosed);

    /* Release resources */
    TcpOnDestroy( pTcb);

    /* Release the TCB */
    SWS_TransportEventSet( pTcb, SWS_kTransportEventClose, err ? err : 1);
    /* !!! pTcb doesn't exist anymore */
    break;

  case kStateEstablished:
  case kStateCloseWait:
    TcpNewState( pTcb, kStateClosed);

    /* Abort all pending ops */
    SWS_TransportEventSet( pTcb, SWS_kTransportEventError, err ? err : 1);
    break;

  case kStateFinWait1:
  case kStateFinWait2:
  case kStateClosing:
  case kStateLastAck:
    /* This is called from TcpOnReceive when FIN is ACK'd */
    TcpNewState( pTcb, kStateClosed);

    if ( !pTcb->shutdown)
      {
      /* Release resources */
      TcpOnDestroy( pTcb);

      /* Signal pending close is now complete */
      if ( err)
        SWS_TransportEventSet( pTcb, SWS_kTransportEventError, err);

      SWS_TransportEventSet( pTcb, SWS_kTransportEventClose, 1);
      /* !!! pTcb doesn't exist anymore */
      }
    break;

  case kStateTimeWait:
    TcpNewState( pTcb, kStateClosed);

    if ( !pTcb->shutdown)
      {
      /* Release resources */
      TcpOnDestroy( pTcb);

      /* Signal pending close is now complete */
      SWS_TransportEventSet( pTcb, SWS_kTransportEventClose, err ? err : 1);
      /* !!! pTcb doesn't exist anymore */
      }
    break;
    }

  return 0;
  }


/*
 * Set local socket ID.
 */
static SWS_FASTCALL( int, TcpOnBind(
  STcb* const pTcb,
  ui32 host,
  unsigned port,
  int reuse
)){
  SBindInfo bind;

  TRACE( 9, ("TcpOnBind(%p,%lx,%u,%d)\n",pTcb,host,port,reuse));
  ASSERT( NULL != pTcb);
  ASSERT( 0 == pTcb->localPort);

  switch ( pTcb->eState)
    {
  case kStateClosed:
    break;

  default:
    TRACE( 1, ("*** TcpOnBind() Illegal operation\n"));
    return SWS_EINVAL;
    }

  /* Validate host */
  if ( 0 == SWS_IpGetMaxSizes( host, host, SWS_IP_TOS_DEFAULT, NULL))
    {
    TRACE( 1, ("*** TcpOnBind() Invalid host\n"));
    return SWS_EADDRNOTAVAIL;
    }

  bind.pTcb = pTcb;
  bind.reuse = reuse;
  bind.host = host;
  bind.port = (ui16)port;

  /* Attach to list of bound SAPs */
  return PlatformSynchronise( &TcpBindSync, &bind);
  }

static SWS_CB( int, TcpBindSync( SWS_PTR( void) pv))
  {
  SBindInfo* const pBind = (SBindInfo*)pv;
  STcb* const pTcb = pBind->pTcb;
  STcb* pTcbOld;
  unsigned port = pBind->port;
  const ui32 host =
    (SWS_INADDR_ANY == pBind->host) ? SWS_INADDR_NONE : pBind->host;

  ASSERT( PlatformIsCritical());

  if ( 0 == port)
    {
    /* Allocate a port number */
    unsigned u;

    port = s_port;
    for ( u = 0; NULL != TcpFindTcb( port, ~0U, host, SWS_INADDR_NONE); ++u)
      {
      if ( u > (unsigned)(s_hiPort - s_loPort))
        {
        TRACE( 1, ("*** TcpBindSync() All ports in use\n"));
        return SWS_ENOBUFS;
        }

      if ( ++port > s_hiPort)
        port = s_loPort;
      else if ( port < s_loPort)
        port = s_loPort;
      }

    s_port = (ui16)port;
    }
  /* Check if port & host are in use */
  else if ( !pBind->reuse
    && NULL != (pTcbOld = TcpFindTcb( port, ~0U, host, SWS_INADDR_NONE)) )
    {
    if ( kStateTimeWait != pTcbOld->eState
      && !(pTcbOld->soflags & SWS_SO_REUSEADDR)
    ) {
      TRACE( 1, ("*** TcpBindSync() Host %lx port %u in use\n", host, port));
      return SWS_EADDRINUSE;
      }
    }

  /* Save local port & address */
  pTcb->localHost = pBind->host;
  pTcb->localPort = (ui16)port;

  /* Attach to list of bound SAPs */
  pTcb->pNext = s_pBoundTcps;
  s_pBoundTcps = pTcb;

  return 0;
  }


/*
 * Start passive listen
 */
static SWS_FASTCALL( int, TcpOnListen( STcb* const pTcb))
  {
  TRACE( 9, ("TcpOnListen(%p)\n", pTcb));
  ASSERT( NULL != pTcb);

  switch ( pTcb->eState)
    {
  case kStateClosed:
    if ( 0 == pTcb->localPort)
      {
      TRACE( 1, ("*** TcpOnListen() Not bound\n"));
      return SWS_EINVAL;
      }
    break;

  case kStateListen:
    return 0;

  default:
    TRACE( 1, ("*** TcpOnListen() Illegal transition\n"));
    return SWS_EACCES;
    }

  /* Start passive listening */
  TcpNewState( pTcb, kStateListen);

  return 0;
  }


/*
 * Start active connection attempt
 */
static SWS_FASTCALL( int, TcpOnConnect(
  STcb* const pTcb,
  ui32 const host,
  unsigned const port
)){
  int iRet;
  SSendInfo info;

  TRACE( 9, ("TcpOnConnect(%p,%08lx,%u)\n", pTcb,host,port));
  ASSERT( NULL != pTcb);
  ASSERT( SWS_INADDR_ANY != host);
  ASSERT( 0 != port);

  switch ( pTcb->eState)
    {
  case kStateClosed:
    if ( SWS_INADDR_ANY == pTcb->localHost)
      {
      /* Bind the TCB to an adapter */
      ui32 localHost = SWS_IpGetSrc( host, pTcb->ipOptions.tos);
      if ( SWS_INADDR_NONE == localHost)
        {
        TRACE( 1, ("*** TcpOnConnect() %08lx not reachable\n", host));
        return SWS_ENETUNREACH;
        }

      pTcb->localHost = localHost;
      }

    if ( 0 == pTcb->localPort)
      {
      /* Bind the TCB to a local port */
      iRet = TcpOnBind( pTcb, pTcb->localHost, 0, 0);
      if ( iRet)
        return iRet;
      }

    pTcb->rxDisable = 0;
    break;

  case kStateListen:
  case kStateSynReceived:
    TRACE( 1, ("*** TcpOnConnect() socket is listening\n"));
    return SWS_EINVAL;

  case kStateSynSent:
  case kStateSynBoth:
    TRACE( 1, ("*** TcpOnConnect() socket is connecting\n"));
    return SWS_EALREADY;

  default:
    TRACE( 1, ("*** TcpOnConnect() socket is connected\n"));
    return SWS_EISCONN;
    }

  ASSERT( SWS_INADDR_ANY != pTcb->localHost);

  /* Set remote port & address */
  pTcb->remoteHost = host;
  pTcb->remotePort = (ui16)port;

  /* Generate initial sequence number from 4 microsecond clock */
  pTcb->SndUna = ((1000000UL / SWS_TIMER_TICKS) / 4) * PlatformTime();
  pTcb->SndLast = pTcb->SndNxt = pTcb->SndUna;
  pTcb->RcvWnd = TcpRecvFree( pTcb);

  TcpNewState( pTcb, kStateSynSent);

  /* Send a SYN */
  info.pTcb = pTcb;
  info.flags = SWS_TCP_SYNF;
  PlatformSynchronise( &TcpSendSync, &info);

  return -1; /* Connection pending */
  }


/*
 * Process a received segment
 */
static SWS_FASTCALL( int, TcpOnReceive(
  STcb* const pTcb,
  ui32 const localHost,
  unsigned const port,
  ui32 const host,
  SWS_BufferH const hBuffer,
  size_t const hdrLen,
  size_t const tcpHdrLen
)){
  SWS_PTR( void) pvHdr;
  SWS_size_t uLen;
  unsigned flags;
  unsigned seglen, urgent = 0;
  int isValid, isPush;
  ui32 window;
  ui32 seq, ack;
  ui32 oldLocalHost;
  size_t mss;

  TRACE( 9, ("TcpOnReceive(%p,%08lx,%u,%08lx,%p,%u,%u)\n",
    pTcb,localHost,port,host,hBuffer,hdrLen,tcpHdrLen));
  ASSERT( PlatformIsCritical());
  ASSERT( NULL != pTcb);
  ASSERT( NULL != hBuffer);

  /* Get segment length */
  uLen = SWS_BufferSize( hBuffer);
  ASSERT( uLen >= hdrLen + tcpHdrLen);
#if SWS_SIZE_MAX > UINT_MAX
  ASSERT( uLen <= UINT_MAX);
#endif
  seglen = (unsigned)(uLen - hdrLen - tcpHdrLen);

  /* TODO: Extract any source route and save it for all replies */

  /* Get TCP header */
  pvHdr = SWS_BufferGetFragment( hBuffer, hdrLen, &uLen);
  ASSERT( NULL != pvHdr);
  ASSERT( uLen >= tcpHdrLen);

  SWS_PEEK32( seq, pvHdr, SWS_TCP_SEQ);
  SWS_PEEK32( ack, pvHdr, SWS_TCP_ACK);
  SWS_PEEK16( flags, pvHdr, SWS_TCP_FLG);
  SWS_PEEK16( window, pvHdr, SWS_TCP_WND);

  TRACE( 3, ("TcpOnReceive(%p) F %04x S %lu A %lu W %lu len %u\n",
    pTcb,flags,seq,ack,window,seglen));

  /* Check for urgent data */
  if ( SWS_TCP_URGF & flags)
    {
    SWS_PEEK16( urgent, pvHdr, SWS_TCP_URG);
    TRACE( 2, ("TcpOnReceive() URG %u\n", urgent));
    }
  isPush = (SWS_TCP_PSHF & flags) ? 1 : 0;

  switch ( pTcb->eState)
    {
  case kStateClosed:
    if ( !(SWS_TCP_RSTF & flags))
      {
      if ( !s_ignore_noport)
        {
        TRACE( 1, ("*** TcpOnReceive() Closed socket, sending RST\n"));
        if ( SWS_TCP_ACKF & flags)
          {
          seq = ack;
          ack = 0;
          flags = SWS_TCP_RSTF;
          }
        else
          {
          ack = seq + seglen;
          seq = 0;
          flags = SWS_TCP_RSTF | SWS_TCP_ACKF;
          }

        /* Send a RST */
        pTcb->remoteHost = host;
        oldLocalHost = pTcb->localHost;
        pTcb->localHost = localHost;

        TcpSendSeg( pTcb, flags, seq, ack);

        pTcb->localHost = oldLocalHost;
        pTcb->remoteHost = SWS_INADDR_ANY;
        }
      }
    else
      {
      TRACE( 1, ("*** TcpOnReceive() Closed socket got RST\n"));
      }
    break;

  case kStateListen:
    if ( SWS_TCP_RSTF & flags)
      {
      TRACE( 1, ("*** TcpOnReceive() Listen: got RST\n"));
      break;
      }

    if ( SWS_TCP_ACKF & flags)
      {
      TRACE( 1, ("*** TcpOnReceive() Listen: unexpected ACK, sending RST\n"));

      if ( !pTcb->opt.closeQuietly)
        {
        pTcb->remotePort = (ui16)port;
        pTcb->remoteHost = host;
        oldLocalHost = pTcb->localHost;
        pTcb->localHost = localHost;

        /* Send a RST */
        TcpSendSeg( pTcb, SWS_TCP_RSTF, ack, 0);

        pTcb->localHost = oldLocalHost;
        pTcb->remoteHost = SWS_INADDR_ANY;
        pTcb->remotePort = 0;
        }
      break;
      }

    if ( SWS_TCP_SYNF & flags)
      {
      /* Incoming connection request */
      STcb* pNewTcb;
      int err;

      /* LAND attack detection */
      if ( localHost == host
        && pTcb->localPort == port
      ) {
        TRACE( 1, ("*** TcpOnReceive() Listen: self SYN\n"));
        break;
        }

      /* Request a new TCB for the connection */
      pNewTcb = (STcb*)SWS_TransportCall( pTcb, &err);
      if ( NULL == pNewTcb
        && SWS_EMFILE == err
      ) {
        /* No free TCB's so terminate the oldest TCB in timed wait */
        TcpTerminateTimedWait();
        pNewTcb = (STcb*)SWS_TransportCall( pTcb, &err);
        }

      if ( NULL != pNewTcb)
        {
        TRACE( 2, ("TcpOnReceive() Listen(%p) created TCB %p\n",
          pTcb, pNewTcb));

        /* Init the new TCB */
        memset( pNewTcb, 0, sizeof( *pNewTcb) );
        if ( TcpOnCreate( pNewTcb, pTcb->SndBuf, pTcb->RcvBuf))
          {
          /* Failed (no buffer space?)
           * Destroy the new TCB (release resources) */
          TcpOnDestroy( pNewTcb);

          /* Release the new TCB */
          SWS_TransportEventSet( pNewTcb, SWS_kTransportEventClose, 1);

          /* Refuse the connection */
          goto refused;
          }

        /* Inherit settings from listening SAP */
        pNewTcb->ipOptions = pTcb->ipOptions;
        pNewTcb->soflags = pTcb->soflags;
        pNewTcb->maxSeg = pTcb->maxSeg;
        pNewTcb->opt = pTcb->opt;

        /* Bind to end points */
        VERIFY( !TcpOnBind( pNewTcb, localHost, pTcb->localPort, 1));
        pNewTcb->remoteHost = host;
        pNewTcb->remotePort = (ui16)port;

        /* Generate initial sequence number from 4 microsecond clock */
        pNewTcb->SndUna = ((1000000UL / SWS_TIMER_TICKS) / 4) * PlatformTime();
        pNewTcb->SndLast = pNewTcb->SndNxt = pNewTcb->SndUna;
        pNewTcb->RcvWnd = TcpRecvFree( pTcb);
        pNewTcb->RcvAckd = seq;
        pNewTcb->RcvNxt = seq + 1;

        pNewTcb->SndMSS = TcpCalcSmss( pNewTcb);
        if ( tcpHdrLen > SWS_TCP_MINLEN)
          {
          /* Check for MSS option */
          mss = TcpRcvOptions( pvHdr, tcpHdrLen);
          if ( mss > 0 && mss < pNewTcb->SndMSS)
            pNewTcb->SndMSS = mss;
          }

        pNewTcb->cwnd = 2 * (ui32)pNewTcb->SndMSS;
        pNewTcb->ssthresh = window;

        pNewTcb->SndWnd = pNewTcb->SndUna + window;
        pNewTcb->SndWndMax = window;
        pNewTcb->SndWL1 = seq;
        pNewTcb->SndWL2 = pNewTcb->SndUna;

        TcpNewState( pNewTcb, kStateSynReceived);

        /* Queue any attached data */
        if ( !TcpRecvSeg(
            pNewTcb,
            hBuffer,
            hdrLen + tcpHdrLen,
            seglen,
            seq + 1,
            urgent,
            isPush
          )
        ) {
          /* Send a SYN + ACK for received SYN */
          TcpSendSeg(
            pNewTcb,
            SWS_TCP_SYNF | SWS_TCP_ACKF,
            pNewTcb->SndUna,
            pNewTcb->RcvNxt
          );
          }
        }
      else
        {
        /* Connection refused */
 refused:
        TRACE( 2, ("TcpOnReceive() SWS_TransportCall refused\n"));

        if ( !pTcb->opt.closeQuietly)
          {
          /* Send a RST so caller gets SWS_ECONNREFUSED */
          pTcb->remotePort = (ui16)port;
          pTcb->remoteHost = host;
          oldLocalHost = pTcb->localHost;
          pTcb->localHost = localHost;

          TcpSendSeg( pTcb, SWS_TCP_RSTF | SWS_TCP_ACKF, 0, seq + seglen);

          pTcb->localHost = oldLocalHost;
          pTcb->remoteHost = SWS_INADDR_ANY;
          pTcb->remotePort = 0;
          }
        }

      break;
      } /* if (SWS_TCP_SYNF & flags) */

    TRACE( 1, ("*** TcpOnReceive() Listen: no SYN or ACK\n"));
    break;

  case kStateSynSent:
    if ( SWS_TCP_ACKF & flags)
      {
      /* Check if ack corresponds to data sent */
      if ( SeqLE( ack, pTcb->SndUna) || !SeqLE( ack, pTcb->SndNxt) )
        {
        if ( !(SWS_TCP_RSTF & flags))
          {
          TRACE( 1,(
            "*** TcpOnReceive() SynSent: Invalid ACK %lu (%lu..%lu) send RST\n",
            ack, pTcb->SndUna, pTcb->SndNxt));

          /* Send a RST */
          TcpSendSeg( pTcb, SWS_TCP_RSTF, ack, 0);
          }
        else
          {
          TRACE( 1, (
            "*** TcpOnReceive() SynSent: Invalid ACK %lu (%lu..%lu) with RST\n",
            ack, pTcb->SndUna, pTcb->SndNxt));
          }
        break;
        }
      /* ACK is valid */
      }

    if ( SWS_TCP_RSTF & flags)
      {
      if ( SWS_TCP_ACKF & flags)
        {
        /* Connection refused (no sockets) */
        TRACE( 2, ("TcpOnReceive() SynSent: got ACK+RST\n"));
        TcpOnError( pTcb, SWS_ECONNREFUSED);
        }
      else
        TRACE( 1, ("*** TcpOnReceive() SynSent: got RST\n"));
      return 0;
      }

    if ( SWS_TCP_SYNF & flags)
      {
      /* Incoming connection */
      pTcb->SndMSS = TcpCalcSmss( pTcb);
      if ( tcpHdrLen > SWS_TCP_MINLEN)
        {
        /* Check for MSS option */
        mss = TcpRcvOptions( pvHdr, tcpHdrLen);
        if ( mss > 0 && mss < pTcb->SndMSS)
          pTcb->SndMSS = mss;
        }

      pTcb->ssthresh = window;
      pTcb->RcvAckd = seq;
      pTcb->RcvNxt = seq + 1;

      if ( SWS_TCP_ACKF & flags)
        {
        /* Got an ACK for the SYN */
        pTcb->cwnd = 2 * (ui32)pTcb->SndMSS;
        pTcb->SndLast = ++(pTcb->SndUna);

        /* Process ack'd data */
        TcpRecvAck( pTcb, ack);

        pTcb->SndWnd = ack + window;
        pTcb->SndWndMax = window;
        pTcb->SndWL1 = seq;
        pTcb->SndWL2 = ack;

        TcpNewState( pTcb, kStateEstablished);

        /* Queue any attached data */
        if ( !TcpRecvSeg(
            pTcb,
            hBuffer,
            hdrLen + tcpHdrLen,
            seglen,
            seq + 1,
            urgent,
            isPush
          )
        ) {
          /* ACK the SYN */
          TcpSendSeg( pTcb, SWS_TCP_ACKF, pTcb->SndNxt, pTcb->RcvNxt);
          }
        }
      else
        {
        /* Simultaneous open */
        pTcb->cwnd = 2 * (ui32)pTcb->SndMSS;

        TcpNewState( pTcb, kStateSynBoth);

        /* Queue any attached data */
        if ( !TcpRecvSeg(
            pTcb,
            hBuffer,
            hdrLen + tcpHdrLen,
            seglen,
            seq + 1,
            urgent,
            isPush
          )
        ) {
          /* ACK the SYN */
          TcpSendSeg(
            pTcb,
            SWS_TCP_SYNF | SWS_TCP_ACKF,
            pTcb->SndUna,
            pTcb->RcvNxt
          );
          }
        }
      break;
      }

    TRACE( 1, ("*** TcpOnReceive() SynSent: segment dropped\n"));
    break;

  case kStateSynReceived:
  case kStateSynBoth:
  case kStateEstablished:
  case kStateFinWait1:
  case kStateFinWait2:
  case kStateCloseWait:
  case kStateClosing:
  case kStateLastAck:
  case kStateTimeWait:
    /* Check if sequence is within receive window */
    if ( seglen)
      {
      if ( pTcb->RcvWnd > 0)
        {
        const ui32 rcvMax = pTcb->RcvNxt + pTcb->RcvWnd;

        if ( SeqLE( pTcb->RcvNxt, seq) && SeqLT( seq, rcvMax) )
          isValid = 1;
        else
          {
          const ui32 seqMax = seq + seglen;

          if ( SeqLT( pTcb->RcvNxt, seqMax) && SeqLE( seqMax, rcvMax) )
            isValid = 1;
          else
            {
            /* This is probably a re-transmit 'cos the ACK was lost */
            TRACE( 2, (
              "TcpOnReceive(%p) Re-transmitted data, SEQ %lu, len %u, RcvNxt %lu, RcvWnd %lu\n",
              pTcb, seq, seglen, pTcb->RcvNxt, pTcb->RcvWnd));
            isValid = 0;
            }
          }
        }
      else
        {
        TRACE( 2, ("TcpOnReceive(%p) Data recvd with 0 window SEQ %lu, len %u\n",
          pTcb,seq, seglen));
        isValid = 0;
        }
      }
    else if ( pTcb->RcvWnd > 0)
      {
      if ( SeqLE( pTcb->RcvNxt, seq)
        && SeqLT( seq, pTcb->RcvNxt + pTcb->RcvWnd)
      ) {
        isValid = 1;
        }
      else
        {
        TRACE( 2, (
          "TcpOnReceive(%p) Duplicate empty SEQ %lu, RcvNxt %lu, RcvWnd %lu\n",
          pTcb, seq, pTcb->RcvNxt, pTcb->RcvWnd));
        isValid = 0;
        }
      }
    else if ( seq == pTcb->RcvNxt)
      {
      isValid = 1;
      }
    else
      {
      TRACE( 2, (
        "TcpOnReceive(%p) Duplicate empty SEQ %lu, RcvNxt %lu, zero RcvWnd\n",
        pTcb, seq, pTcb->RcvNxt));
      isValid = 0;
      }

    if ( !isValid)
      {
      pTcb->RcvDupAck = 0;
      if ( SWS_TCP_RSTF & flags)
        {
        TRACE( 1, ("*** TcpOnReceive(%p) Invalid SEQ with RST\n", pTcb));
        }
      else if ( localHost == host
        && pTcb->localPort == port
      ) {
        TRACE( 1, ("*** TcpOnReceive() self ACK\n"));
        }
      else
        {
        /* ACK the segment */
        TcpSendSeg(
          pTcb,
          FORCE_SNDF | SWS_TCP_ACKF,
          pTcb->SndLast,
          pTcb->RcvNxt
        );
        }
      break;
      }

    if ( SWS_TCP_RSTF & flags)
      {
      /* Abort the connection */
      TRACE( 2, ("TcpOnReceive(%p) %s got RST\n",
        pTcb, kszStates[ pTcb->eState]));
      TcpOnError( pTcb, SWS_ECONNRESET);
      return 0;
      }

    if ( SWS_TCP_SYNF & flags)
      {
      TRACE( 1, ("*** TcpOnReceive(%p) %s Invalid SYN\n",
        pTcb, kszStates[ pTcb->eState]));

      /* Send a RST */
      TcpSendSeg( pTcb, SWS_TCP_RSTF, ack, 0);

      /* Abort the connection */
      TcpOnError( pTcb, SWS_ECONNABORTED);
      return 0;
      }

    if ( tcpHdrLen > SWS_TCP_MINLEN)
      {
      TcpRcvOptions( pvHdr, tcpHdrLen);
#if OPT_TCP_SACKP
      /* TODO Handle SACK missing segments */
#endif
      }

    if ( SWS_TCP_ACKF & flags)
      {
      switch ( pTcb->eState)
        {
      default:
        ASSERT( !"Invalid state");
        break;

      case kStateSynReceived:
      case kStateSynBoth:
        /* Check if ack corresponds to SYN sent */
        if ( SeqLE( ack, pTcb->SndUna) || !SeqLE( ack, pTcb->SndNxt) )
          {
          TRACE( 1, (
            "*** TcpOnReceive(%p) %s: Invalid ACK:%lu (%lu..%lu). Send RST\n",
            pTcb, kszStates[ pTcb->eState], ack, pTcb->SndUna, pTcb->SndNxt));

          /* Send a RST */
          TcpSendSeg( pTcb, SWS_TCP_RSTF, ack, 0);
          return 0;
          }

        /* Got an ACK for the SYN */
        pTcb->SndLast = ++(pTcb->SndUna);

        /* Process ack'd data */
        TcpRecvAck( pTcb, ack);

        pTcb->SndWnd = ack + window;
        if ( window > pTcb->SndWndMax)
          pTcb->SndWndMax = window;
        pTcb->SndWL1 = seq;
        pTcb->SndWL2 = ack;

        TcpNewState( pTcb, kStateEstablished);
        pTcb->RcvDupAck = 0;
        break;

      case kStateEstablished:
      case kStateFinWait1:
      case kStateFinWait2:
      case kStateCloseWait:
      case kStateClosing:
      case kStateLastAck:
      case kStateTimeWait:
        /* Check if ack corresponds to data sent */
        if ( SeqLT( ack, pTcb->SndUna) )
          {
          TRACE( 1, ("*** TcpOnReceive(%p) Duplicate ACK %lu, UNA %lu\n",
            pTcb, ack, pTcb->SndUna));
          pTcb->RcvDupAck = 0;
          }
        else if ( !SeqLE( ack, pTcb->SndNxt) )
          {
          TRACE( 1, ("*** TcpOnReceive(%p) Invalid ACK:%lu (%lu..%lu)\n",
            pTcb, ack, pTcb->SndUna, pTcb->SndNxt));
          pTcb->RcvDupAck = 0;

          /* Send an ACK */
          TcpSendSeg(
            pTcb,
            FORCE_SNDF | SWS_TCP_ACKF,
            pTcb->SndNxt,
            pTcb->RcvNxt
          );
          }
        else
          {
          /* Process ack'd data */
          TcpRecvAck( pTcb, ack);

          if ( ack == pTcb->SndUna
            && 0 == seglen
            && ack + window == pTcb->SndWnd
            && !(SWS_TCP_FINF & flags)
          ) {
            /* This may be a zero window probe */
            pTcb->wantAck = 1;
            }

          /* Update the send window */
          if ( SeqLT( pTcb->SndWL1, seq)
            || (pTcb->SndWL1 == seq && SeqLE( pTcb->SndWL2, ack) )
          ) {
            if ( 0 == window)
              {
              /* Start zero window probes */
              TRACE( 2, ("TcpOnReceive(%p) Starting zero window probes\n",pTcb));
              VERIFY( !SWS_TimerStart(
                pTcb->hZeroWinTimer,
                pTcb->zeroWinTimeout = pTcb->rto
              ));
              }
            /* Stop zero window probes if no deferred data */
            else if ( pTcb->zeroWinTimeout > 0
              && 0 == TcpSendPending( pTcb, pTcb->SndNxt)
            ) {
              TRACE( 2, ("TcpOnReceive(%p) Stop zero window probes\n",pTcb));
              SWS_TimerStop( pTcb->hZeroWinTimer);
              pTcb->zeroWinTimeout = 0;
              }

            if ( ack + window != pTcb->SndWnd)
              {
              pTcb->SndWnd = ack + window;
              pTcb->SndWL1 = seq;
              pTcb->SndWL2 = ack;
              pTcb->wantAck = 1; /* Check if more data can be sent */
              }
            if ( window > pTcb->SndWndMax)
              pTcb->SndWndMax = window;
            }
          }
        break;
        }
      }

    /* Process the segment data */
    switch ( pTcb->eState)
      {
    case kStateSynReceived:
    case kStateSynBoth:
    case kStateEstablished:
    case kStateFinWait1:
    case kStateFinWait2:
      /* Queue attached data */
      TcpRecvSeg(
        pTcb,
        hBuffer,
        hdrLen + tcpHdrLen,
        seglen,
        seq,
        urgent,
        isPush
      );
      break;

    case kStateCloseWait:
    case kStateClosing:
    case kStateLastAck:
    case kStateTimeWait:
      if ( seglen)
        TRACE( 1, ("***TcpOnReceive(%p) %s unexpected data %u bytes\n",
          pTcb, kszStates[ pTcb->eState], seglen));
      break;

    default:
      ASSERT( !"Invalid state");
      break;
      }

    if ( SWS_TCP_ACKF & flags)
      {
      /* Check if FIN is ack'd */
      switch ( pTcb->eState)
        {
      default:
        break;

      case kStateFinWait1:
        if ( !pTcb->deferredFin && ack == pTcb->SndNxt)
          {
          TRACE( 2, ("TcpOnReceive(%p) %s Got ACK for FIN\n",
            pTcb, kszStates[ pTcb->eState]));
          TcpNewState( pTcb, kStateFinWait2);
          }
        break;

      case kStateClosing:
        if ( !pTcb->deferredFin && ack == pTcb->SndNxt)
          {
          TRACE( 2, ("TcpOnReceive(%p) %s Got ACK for FIN\n",
            pTcb, kszStates[ pTcb->eState]));
          TcpNewState( pTcb, kStateTimeWait);
          }
        break;

      case kStateLastAck:
        if ( !pTcb->deferredFin && ack == pTcb->SndNxt)
          {
          TRACE( 2, ("TcpOnReceive(%p) %s Got ACK for FIN\n",
            pTcb, kszStates[ pTcb->eState]));

          if ( pTcb->wantAck)
            {
            pTcb->wantAck = 0;
            TcpSendSeg( pTcb, SWS_TCP_ACKF, pTcb->SndNxt, pTcb->RcvNxt);
            }

          /* Terminate the TCB */
          TcpOnError( pTcb, 0);
          return 0; /* TCB is destroyed */
          }
        break;
        }
      }

    if ( SWS_TCP_FINF & flags)
      {
      /* Procss FIN.  All data should have been received */
      switch ( pTcb->eState)
        {
      case kStateSynReceived:
      case kStateSynBoth:
      case kStateEstablished:
        if ( pTcb->RcvNxt == seq + seglen)
          {
          TRACE( 2, ("TcpOnReceive(%p) %s Got FIN\n",
            pTcb, kszStates[ pTcb->eState]));
          TcpNewState( pTcb, kStateCloseWait);

          /* ACK the FIN */
          pTcb->wantAck = 1;
          ++(pTcb->RcvNxt);
          }
        else
          TRACE( 2, ("TcpOnReceive(%p) %s FIN ignored\n",
            pTcb, kszStates[ pTcb->eState]));
        break;

      case kStateFinWait1:
        if ( pTcb->RcvNxt == seq + seglen)
          {
          TRACE( 2, ("TcpOnReceive(%p) %s Got FIN\n",
            pTcb, kszStates[ pTcb->eState]));
          TcpNewState( pTcb, kStateClosing);

          /* ACK the FIN */
          pTcb->wantAck = 1;
          ++(pTcb->RcvNxt);
          }
        else
          TRACE( 2, ("TcpOnReceive(%p) %s FIN ignored\n",
            pTcb, kszStates[ pTcb->eState]));
        break;

      case kStateFinWait2:
        if ( pTcb->RcvNxt == seq + seglen)
          {
          TRACE( 2, ("TcpOnReceive(%p) %s Got FIN\n",
            pTcb, kszStates[ pTcb->eState]));
          TcpNewState( pTcb, kStateTimeWait);

          /* ACK the FIN */
          pTcb->wantAck = 1;
          ++(pTcb->RcvNxt);
          }
        else
          TRACE( 2, ("TcpOnReceive(%p) %s FIN ignored\n",
            pTcb, kszStates[ pTcb->eState]));
        break;

      case kStateCloseWait:
      case kStateClosing:
      case kStateLastAck:
        TRACE( 1, ("*** TcpOnReceive(%p) %s Unexpected FIN\n",
          pTcb, kszStates[ pTcb->eState]));
        break;

      case kStateTimeWait:
        TRACE( 2, ("TcpOnReceive(%p) %s Got FIN\n",
          pTcb, kszStates[ pTcb->eState]));

        if ( !pTcb->shutdown)
          {
          /* (Re)start the 2xMSL timer */
          VERIFY( !SWS_TimerStart( pTcb->hRtoTimer, s_timeWait));
          }
        break;

      default:
        ASSERT( !"FINF Invalid state");
        break;
        }
      }

    /* Send an ACK if required */
    if ( pTcb->wantAck)
      {
      pTcb->wantAck = 0;
      TcpSendSeg( pTcb, SWS_TCP_ACKF, pTcb->SndLast, pTcb->RcvNxt);
      }
    break;
    }

  return 0;
  }


/*
 * Send data
 */
static SWS_FASTCALL( int, TcpOnWrite(
  STcb* const pTcb,
  SWS_PTR( const void) pBuf,          /* IN: -> data to send */
  SWS_PTR( unsigned) const pLen,      /* I/O: data len */
  unsigned flags                      /* IN: flags, SWS_MSG_xxx */
)){
  unsigned len;
  int err;

  TRACE( 9, ("TcpOnWrite(%p,%p,%p,%u)\n", pTcb,pBuf,pLen,flags));
  ASSERT( NULL != pTcb);
  ASSERT( NULL != pLen);
  ASSERT( NULL != pBuf || 0 == *pLen);

  len = *pLen;
  *pLen = 0;

  switch ( pTcb->eState)
    {
  case kStateClosed:
    return SWS_ECONNABORTED;

  case kStateListen:
  case kStateSynSent:
  case kStateSynReceived:
  case kStateSynBoth:
    TRACE( 1, ("*** TcpOnWrite(%p) Not yet connected\n",pTcb));
    return SWS_ENOTCONN;

  case kStateEstablished:
    break;

  case kStateFinWait1:
  case kStateFinWait2:
  case kStateClosing:
  case kStateLastAck:
  case kStateTimeWait:
    ASSERT( !"*** TcpOnWrite() socket has been closed");
    return SWS_ENOTCONN;

  case kStateCloseWait: /* Remote has closed */
    TRACE( 1, ("*** TcpOnWrite(%p) Connection reset\n",pTcb));
    return SWS_ECONNRESET;
    }

  TRACE( 3, ("TcpOnWrite(%p) %u bytes from %p\n", pTcb, len, pBuf));

  /* Repeat until all copied to re-transmit buffer */
  while ( len > 0)
    {
    size_t available;
    SSendInfo info;

    /* Wait until Tx resources available */
    err = SWS_TransportEventWait( pTcb, SWS_kTransportEventTxReady);
    if ( 1 != err)
      {
      if ( SWS_ENETDOWN == err)
        SWS_TransportEventSet( pTcb, SWS_kTransportEventTxReady, SWS_ENETDOWN);

      return err;
      }

    while ( (available = TcpSendAvailable( pTcb)) > 0)
      {
      unsigned send;

      /* Calc amount to send */
      if ( len > available)
        send = (unsigned)available;
      else
        send = len;

      /* Move to re-transmit buffer */
      memcpy( (char*)pTcb->pvSndBuf + pTcb->SndNew, pBuf, send);

      /* Update buffer cursor */
      available = pTcb->SndNew + send;
      if ( available < pTcb->SndBuf)
        pTcb->SndNew = available;
      else
        pTcb->SndNew = 0;

      /* Is the data urgent? */
      if ( flags & SWS_MSG_OOB)
        {
        ASSERT( !(pTcb->soflags & SWS_SO_OOBINLINE));
        pTcb->SndUp = TcpSendUsed( pTcb);
        }

      pBuf = (SWS_PTR( const char)) pBuf + send;
      *pLen += send;
      len -= send;
      if ( 0 == len)
        break; /* All sent */
      }

    if ( TcpSendAvailable( pTcb) > 0)
      SWS_TransportEventSet( pTcb, SWS_kTransportEventTxReady, 1);

    /* Try to send a segment */
    info.pTcb = pTcb;
    info.flags = SWS_TCP_ACKF;
    PlatformSynchronise( &TcpSendSync, &info);
    }

  TRACE( 3, ("TcpOnWrite(%p) complete\n", pTcb));
  return 0;
  }

/* Callback to send a segment in single threaded mode */
static SWS_CB( int, TcpSendSync( SWS_PTR( void) pv))
  {
  SWS_PTR( const SSendInfo) const pInfo = (SWS_PTR( const SSendInfo))pv;
  STcb* pTcb;

  TRACE( 9, ("TcpSendSync(%p)\n", pv));
  ASSERT( NULL != pInfo);
  ASSERT( PlatformIsCritical());

  pTcb = pInfo->pTcb;
  ASSERT( NULL != pTcb);

  return TcpSendSeg( pTcb, pInfo->flags, pTcb->SndLast, pTcb->RcvNxt);
  }


/*
 * Read data
 */
static SWS_FASTCALL( int, TcpOnRead(
  STcb* const pTcb,
  SWS_PTR( void) pBuf,                /* OUT: -> data received  */
  SWS_PTR( unsigned) const pLen,      /* I/O: data len */
  unsigned flags                      /* IN: flags, SWS_MSG_xxx */
)){
  unsigned len;
  size_t read = 0, push;

  TRACE( 9, ("TcpOnRead(%p,%p,%p,%u)\n", pTcb,pBuf,pLen,flags));
  ASSERT( NULL != pTcb);
  ASSERT( NULL != pLen);
  ASSERT( NULL != pBuf || 0 == *pLen);

  len = *pLen;
  *pLen = 0;

  do
    {
    int err;

    switch ( pTcb->eState)
      {
    case kStateClosed:
      TRACE( 2, ("TcpOnRead(%p) Connection reset\n",pTcb));
      return SWS_ECONNRESET;

    case kStateListen:
    case kStateSynSent:
    case kStateSynReceived:
    case kStateSynBoth:
      TRACE( 1, ("*** TcpOnRead(%p) Not yet connected\n",pTcb));
      return SWS_ENOTCONN;

    case kStateEstablished:
    case kStateFinWait1:
      if ( SWS_MSG_OOB & flags)
        {
        ASSERT( !(pTcb->soflags & SWS_SO_OOBINLINE));

        /* Wait until OOB data available */
        err = SWS_TransportEventWait( pTcb, SWS_kTransportEventOob);
        if ( 1 != err)
          {
          if ( SWS_ENETDOWN == err)
            SWS_TransportEventSet( pTcb, SWS_kTransportEventOob, SWS_ENETDOWN);
          return err;
          }

        /* TODO: Recv non-inline OOB data */
        *pLen = 0;
        return 0;
        }
      /* Check for Rx data */
      else if ( SWS_MSG_PEEK & flags)
        {
        read = TcpRecvPending( pTcb);
        }
      else
        {
        /* Wait until Rx data available */
        err = SWS_TransportEventWait( pTcb, SWS_kTransportEventRxReady);
        if ( 1 != err)
          {
          if ( SWS_ENETDOWN == err)
            SWS_TransportEventSet( pTcb, SWS_kTransportEventRxReady, SWS_ENETDOWN);
          return err;
          }

        read = TcpRecvPending( pTcb);
        }
      break;

    case kStateFinWait2:
    case kStateClosing:
    case kStateTimeWait:
    case kStateCloseWait:
      /* Queued data only */
      read = TcpRecvPending( pTcb);
      if ( !read)
        {
        TRACE( 2, ("TcpOnRead(%p) in %s no more data\n",
          pTcb, kszStates[ pTcb->eState]));
        return 0;
        }
      break;

    case kStateLastAck:
      TRACE( 1, ("*** TcpOnRead(%p) No longer connected\n", pTcb));
      return SWS_ENOTCONN;
      }
    }
  while ( 0 == read && 0 == (SWS_MSG_PEEK & flags) );

  /* If there's 'PSH'd data then just read that */
  if ( pTcb->RcvPsh >= pTcb->RcvOld)
    push = pTcb->RcvPsh - pTcb->RcvOld;
  else
    push = (pTcb->RcvBuf - pTcb->RcvOld) + pTcb->RcvPsh;
  if ( push)
    {
    ASSERT( push <= read);
    if ( push < read)
      read = push;
    }

  /* Calc amount of data to receive */
  if ( len >= read)
    {
    /* Recv all available */
    *pLen = (unsigned)read;
    }
  else if ( SWS_MSG_PEEK & flags )
    {
    /* Just peeking - return length available */
    *pLen = (unsigned)read;
    read = (size_t)len;
    }
  else
    {
    /* Recv all that fits */
    *pLen = len;
    read = (size_t)len;
    }

  /* Copy the data from the receive buffer */
  TRACE( 3, ("TcpOnRead(%p) receive %u bytes to %p\n", pTcb, read, pBuf));
  if ( pTcb->RcvOld <= pTcb->RcvNew)
    {
    memcpy( pBuf, (char*)pTcb->pvRcvBuf + pTcb->RcvOld, read);
    }
  else
    {
    len = (unsigned)(pTcb->RcvBuf - pTcb->RcvOld);
    if ( len > read)
      len = (unsigned)read;

    memcpy( pBuf, (char*)pTcb->pvRcvBuf + pTcb->RcvOld, len);
    memcpy( (SWS_PTR(char))pBuf + len, pTcb->pvRcvBuf, read - len);
    }

  if ( !(SWS_MSG_PEEK & flags) )
    {
    ui32 temp;
    size_t mss, space;

    /* Update the read cursor */
    temp = (ui32)pTcb->RcvOld + read;
    if ( temp >= pTcb->RcvBuf)
      temp -= pTcb->RcvBuf;

    if ( pTcb->RcvOld == pTcb->RcvPsh)
      pTcb->RcvPsh = (size_t)temp;

    pTcb->RcvOld = (size_t)temp;

    /* Receiver silly window avoidance RFC1122 p97 */
    mss = pTcb->RcvBuf / 2;
    if ( mss > pTcb->SndMSS)
      mss = pTcb->SndMSS;

    space = TcpRecvFree( pTcb);
    ASSERT( space >= pTcb->RcvWnd);
    if ( space >= mss + pTcb->RcvWnd)
      {
      SSendInfo info;

      /* Update the receive window */
      mss = (size_t)pTcb->RcvWnd;
      pTcb->RcvWnd = space;

      /* Has the receive window re-opened? */
      /*if ( 0 == mss)*/
      if ( mss < pTcb->SndMSS /* Work around winsock hangup */
        || pTcb->deferredAck
      ) {
        /* Yes, then send the new window now */
        info.pTcb = pTcb;
        info.flags = FORCE_SNDF | SWS_TCP_ACKF;
        PlatformSynchronise( &TcpSendSync, &info);
        }
      /* Defer sending the new window in case it can be piggy-backed
       * NB This reduces traffic but hits throughput
       */
      else
        {
        pTcb->deferredAck = 1;
        VERIFY( !SWS_TimerStart( pTcb->hAckTimer, s_ackDelay));
        }
      }
    }

  if ( TcpRecvPending( pTcb) )
    {
    /* There's still data remaining in the Rx buffer */
    SWS_TransportEventSet( pTcb, SWS_kTransportEventRxReady, 1);
    }

  return 0;
  }


/*
 * User wants to close the TCB
 */
static SWS_CB( int, TcpOnClose( SWS_PTR( void) pv) )
  {
  STcb* const pTcb = ((SWS_PTR( SCloseInfo)) pv)->pTcb;
  enum ECloseMode const eMode = ((SWS_PTR( SCloseInfo)) pv)->eMode;

  TRACE( 9, ("TcpOnClose(%p,%d)\n", pTcb,eMode));
  ASSERT( NULL != pv);
  ASSERT( NULL != pTcb);
  ASSERT( PlatformIsCritical());

  switch ( pTcb->eState)
    {
  default:
    ASSERT( !"Invalid state");
    /* Fall thru */

  case kStateClosed:
  case kStateListen:
  case kStateSynSent:
    if ( ECloseShutdown == eMode)
      return SWS_ENOTCONN;

    TRACE( 2, ("TcpOnClose(%p)\n", pTcb));
    pTcb->shutdown = 0;
    TcpNewState( pTcb, kStateClosed);
    break;

  case kStateSynReceived:
  case kStateSynBoth:
  case kStateEstablished:
    if ( pTcb->opt.closeQuietly)
      {
      pTcb->shutdown = 0;
      pTcb->eState = kStateFinWait2;
      TcpNewState( pTcb, kStateTimeWait);
      }
    else switch ( eMode)
      {
    case ECloseReset:
      /* Send RST */
      TcpSendSeg( pTcb, SWS_TCP_RSTF, pTcb->SndNxt, 0);

      TRACE( 2, ("TcpOnClose(%p) send RST\n", pTcb));
      pTcb->shutdown = 0;
      TcpNewState( pTcb, kStateClosed);

      /* Abort all pending ops */
      SWS_TransportEventSet( pTcb, SWS_kTransportEventError, SWS_ECONNABORTED);
      break;

    case ECloseShutdown:
    case ECloseNormal:
      TRACE( 2, ("TcpOnClose(%p)\n", pTcb));
      pTcb->shutdown = (char)(ECloseShutdown == eMode);
      TcpNewState( pTcb, kStateFinWait1);

      /* Send FIN */
      TcpSendSeg(
        pTcb, SWS_TCP_FINF | SWS_TCP_ACKF, pTcb->SndLast, pTcb->RcvNxt);
      break;
      }
    break;

  case kStateFinWait1:
  case kStateFinWait2:
    switch ( eMode)
      {
    case ECloseReset:
      /* Send RST */
      TcpSendSeg( pTcb, SWS_TCP_RSTF, pTcb->SndNxt, 0);

      TRACE( 2, ("TcpOnClose(%p) send RST\n", pTcb));
      pTcb->shutdown = 0;
      TcpNewState( pTcb, kStateClosed);

      /* Abort all pending ops */
      SWS_TransportEventSet( pTcb, SWS_kTransportEventError, SWS_ECONNABORTED);
      break;

    case ECloseShutdown:
      return SWS_ENOTCONN;

    case ECloseNormal:
      pTcb->shutdown = 0;
      break;
      }
    break;

  case kStateClosing:
  case kStateLastAck:
  case kStateTimeWait:
    switch ( eMode)
      {
    case ECloseReset:
      TRACE( 2, ("TcpOnClose(%p) reset\n", pTcb));
      pTcb->shutdown = 0;
      TcpNewState( pTcb, kStateClosed);

      /* Abort all pending ops */
      SWS_TransportEventSet( pTcb, SWS_kTransportEventError, SWS_ECONNABORTED);
      break;

    case ECloseShutdown:
      return SWS_ENOTCONN;

    case ECloseNormal:
      pTcb->shutdown = 0;
      break;
      }
    break;

  case kStateCloseWait:
    switch ( eMode)
      {
    case ECloseReset:
      /* Send RST */
      TcpSendSeg( pTcb, SWS_TCP_RSTF, pTcb->SndNxt, 0);

      TRACE( 2, ("TcpOnClose(%p) send RST\n", pTcb));
      pTcb->shutdown = 0;
      TcpNewState( pTcb, kStateClosed);

      /* Abort all pending ops */
      SWS_TransportEventSet( pTcb, SWS_kTransportEventError, SWS_ECONNABORTED);
      break;

    case ECloseShutdown:
    case ECloseNormal:
      TRACE( 2, ("TcpOnClose(%p)\n", pTcb));
      pTcb->shutdown = (char)(ECloseShutdown == eMode);
      TcpNewState( pTcb, kStateLastAck);

      /* Send FIN */
      TcpSendSeg( pTcb, SWS_TCP_FINF | SWS_TCP_ACKF, pTcb->SndLast, pTcb->RcvNxt);
      break;
      }
    break;
    }

  if ( kStateClosed != pTcb->eState)
    return -1; /* Pending */

  /* Release resources */
  TcpOnDestroy( pTcb);
  return 0;
  }



#if OPT_PROTO_ICMP
/*
 * Process an ICMP message
 */
static SWS_FASTCALL( int, TcpOnIcmpError(
  STcb* const pTcb,
  int const type,
  int const code
)){
  TRACE( 9, ("TcpOnIcmpError(%p,%d,%d)\n", pTcb, type, code));
  ASSERT( PlatformIsCritical());
  ASSERT( NULL != pTcb);

  switch ( pTcb->eState)
    {
  case kStateClosed:
  case kStateListen:
    break;

  case kStateSynSent:
  case kStateSynBoth:
  case kStateSynReceived:
  case kStateEstablished:
  case kStateFinWait1:
  case kStateFinWait2:
  case kStateClosing:
  case kStateLastAck:
  case kStateCloseWait:
    switch ( type)
      {
    case SWS_ICMP_DEST_UNREACH:
      switch ( code)
        {
      case SWS_ICMP_NET_UNREACH:
      case SWS_ICMP_HOST_UNREACH:
      case SWS_ICMP_ROUTE_FAIL:
        /* Possible temporary net problems, keep retrying until timeout */
        break;

      case SWS_ICMP_PROT_UNREACH:
      case SWS_ICMP_PORT_UNREACH:
      case SWS_ICMP_FRAG_NEEDED:
      default:
        TcpOnError( pTcb, SWS_ECONNRESET);
        break;
        }
      break;

    case SWS_ICMP_TIME_EXCEED:
    case SWS_ICMP_PARAM_PROB:
      /* Possible temporary net problems, keep retrying until timeout */
      break;

    case SWS_ICMP_QUENCH:
      TcpCongestion( pTcb);
      break;
      }
    break;

  case kStateTimeWait:
    if ( !pTcb->shutdown)
      {
      /* Restart the 2x MSL timer */
      VERIFY( !SWS_TimerStart( pTcb->hRtoTimer, s_timeWait));
      }
    break;
    }

  return 0;
  }
#endif


/*
 * Process re-transmit & timeWait timeouts
 */
static SWS_CB( void, TcpOnRetransmitTimeout(
  SWS_TimerH hTimer,                  /* IN: Timer that expired */
  SWS_PTR( void) pvContext            /* IN: User context */
)){
  STcb* const pTcb = (STcb*)pvContext;
#if DEBUG_TRACE_ENABLE
  SRttInfo rtt;
#endif
  (void)hTimer;

  TRACE( 9, ("TcpOnRetransmitTimeout(%p,%p)\n",hTimer,pvContext));
  ASSERT( NULL != pTcb);
  ASSERT( PlatformIsCritical());
  ASSERT( pTcb->hRtoTimer == hTimer);

  switch ( pTcb->eState)
    {
  case kStateClosed:
    break;

  case kStateListen:
    TRACE( 1, ("*** TcpOnRetransmitTimeout(%p) socket is listening\n", pTcb));
    break;

  case kStateSynReceived:
    /* Missing ACK */
    ASSERT( SeqLT( pTcb->SndUna, pTcb->SndNxt));

#if DEBUG_TRACE_ENABLE
    if ( !TcpPeekRttQ( pTcb, &rtt))
      TRACE( 2, ("TcpOnRetransmitTimeout(%p) RTT SEQ %lu sent at %lu\n",
        pTcb, rtt.seq, rtt.sent));
#endif
    TcpFlushRttQ( pTcb);

    /* Exponential backoff */
    TcpCongestion( pTcb);
    pTcb->retransmits++;
    pTcb->rto *= 2;

    /* Is total time expired < timeout */
    if ( (pTcb->rto - (pTcb->rto >> pTcb->retransmits)) < s_synFail)
      {
      TRACE( 1, ("*** TcpOnRetransmitTimeout(%p) in %s new RTO=%lu\n",
        pTcb, kszStates[ pTcb->eState], pTcb->rto));

      AtomicIncrement( &s_aRetransmits);
      SWS_GefRaise( SWS_kGefTcpError, "Retransmit timeout");

      TcpSendSeg(
        pTcb,
        REPEAT_SNDF | SWS_TCP_SYNF | SWS_TCP_ACKF,
        pTcb->SndUna,
        pTcb->RcvNxt
      );
      VERIFY( !SWS_TimerStart( pTcb->hRtoTimer, pTcb->rto));
      }
    else
      {
      TRACE( 1, ("*** TcpOnRetransmitTimeout(%p) in %s, time %lu exceeded\n",
        pTcb, kszStates[ pTcb->eState], pTcb->rto ));
      TcpOnError( pTcb, SWS_ECONNABORTED);
      }
    break;

  case kStateSynBoth:
  case kStateSynSent:
    /* Missing ACK */
    ASSERT( SeqLT( pTcb->SndUna, pTcb->SndNxt));

#if DEBUG_TRACE_ENABLE
    if ( !TcpPeekRttQ( pTcb, &rtt))
      TRACE( 2, ("TcpOnRetransmitTimeout(%p) RTT SEQ %lu sent at %lu\n",
        pTcb, rtt.seq, rtt.sent));
#endif
    TcpFlushRttQ( pTcb);

    /* Exponential backoff */
    TcpCongestion( pTcb);
    pTcb->retransmits++;
    pTcb->rto *= 2;

    /* Is total time expired < timeout */
    if ( (pTcb->rto - (pTcb->rto >> pTcb->retransmits)) < s_synFail)
      {
      unsigned flags;

      TRACE( 1, ("*** TcpOnRetransmitTimeout(%p) in %s new RTO=%lu\n",
        pTcb, kszStates[ pTcb->eState], pTcb->rto));

      AtomicIncrement( &s_aRetransmits);
      SWS_GefRaise( SWS_kGefTcpError, "Retransmit timeout");

      if ( kStateSynBoth == pTcb->eState)
        flags = REPEAT_SNDF | SWS_TCP_SYNF | SWS_TCP_ACKF;
      else
        flags = REPEAT_SNDF | SWS_TCP_SYNF;

      TcpSendSeg( pTcb, flags, pTcb->SndUna, pTcb->RcvNxt);
      VERIFY( !SWS_TimerStart( pTcb->hRtoTimer, pTcb->rto));
      }
    else
      {
      TRACE( 1, ("*** TcpOnRetransmitTimeout(%p) in %s time %lu exceeded\n",
        pTcb, kszStates[ pTcb->eState], pTcb->rto ));
      TcpOnError( pTcb, SWS_ETIMEDOUT);
      }
    break;

  case kStateEstablished:
    if ( SeqLE( pTcb->SndNxt, pTcb->SndUna + 1))
      break; /* All ack'd */

#if DEBUG_TRACE_ENABLE
    if ( !TcpPeekRttQ( pTcb, &rtt))
      TRACE( 2, ("TcpOnRetransmitTimeout(%p) RTT SEQ %lu sent at %lu\n",
        pTcb, rtt.seq, rtt.sent));
#endif
    DEBUG_SET_TRACE_LEVEL( 2);
    TcpFlushRttQ( pTcb);

    /* Exponential backoff */
    TcpCongestion( pTcb);
    pTcb->retransmits++;
    pTcb->rto *= 2;

    /* Is total time expired < timeout */
    if ( (pTcb->rto - (pTcb->rto >> pTcb->retransmits)) < s_segFail)
      {
      TRACE( 1, ("*** TcpOnRetransmitTimeout(%p) in %s %lu..%lu new RTO=%lu\n",
        pTcb, kszStates[ pTcb->eState], pTcb->SndUna, pTcb->SndNxt, pTcb->rto));

      AtomicIncrement( &s_aRetransmits);
      SWS_GefRaise( SWS_kGefTcpError, "Retransmit timeout");

      TcpSendSeg(
        pTcb,
        REPEAT_SNDF | SWS_TCP_ACKF,
        pTcb->SndUna,
        pTcb->RcvNxt
      );
      VERIFY( !SWS_TimerStart( pTcb->hRtoTimer, pTcb->rto));
      }
    else
      {
      TRACE( 1, ("*** TcpOnRetransmitTimeout(%p) in %s time %lu exceeded\n",
        pTcb, kszStates[ pTcb->eState], pTcb->rto));
      TcpOnError( pTcb, SWS_ECONNABORTED);
      }
    break;

  case kStateFinWait1:
  case kStateClosing:
  case kStateLastAck:
    if ( pTcb->SndUna == pTcb->SndNxt)
      break; /* All ack'd */

#if DEBUG_TRACE_ENABLE
    if ( !TcpPeekRttQ( pTcb, &rtt))
      TRACE( 2, ("TcpOnRetransmitTimeout(%p) RTT SEQ %lu sent at %lu\n",
        pTcb, rtt.seq, rtt.sent));
#endif
    TcpFlushRttQ( pTcb);

    /* Exponential backoff */
    TcpCongestion( pTcb);
    pTcb->retransmits++;
    pTcb->rto *= 2;

    /* Is total time expired < timeout */
    if ( (pTcb->rto - (pTcb->rto >> pTcb->retransmits)) < s_segFail)
      {
      TRACE( 1, ("*** TcpOnRetransmitTimeout(%p) in %s new RTO=%lu\n",
        pTcb, kszStates[ pTcb->eState], pTcb->rto));

      AtomicIncrement( &s_aRetransmits);
      SWS_GefRaise( SWS_kGefTcpError, "Retransmit timeout");

      TcpSendSeg(
        pTcb,
        REPEAT_SNDF | SWS_TCP_FINF | SWS_TCP_ACKF,
        pTcb->SndUna,
        pTcb->RcvNxt
      );
      VERIFY( !SWS_TimerStart( pTcb->hRtoTimer, pTcb->rto));
      }
    else
      {
      TRACE( 1, ("*** TcpOnRetransmitTimeout(%p) in %s, time %lu exceeded\n",
        pTcb, kszStates[ pTcb->eState], pTcb->rto ));
      TcpOnError( pTcb, SWS_ECONNABORTED);
      }
    break;

  case kStateFinWait2: /* Waiting for remote FIN */
  case kStateCloseWait: /* Waiting for user close */
    TRACE( 2, ("TcpOnRetransmitTimeout(%p) in %s\n",
      pTcb, kszStates[ pTcb->eState]));
    break;

  case kStateTimeWait:
    TRACE( 2, ("TcpOnRetransmitTimeout(%p) in %s\n",
      pTcb, kszStates[ pTcb->eState]));
    if ( !pTcb->shutdown)
      {
      TcpOnError( pTcb, 0);
      /* !!! pTcb doesn't exist anymore */
      }
    return;
    }
  }


/*
 * Process deferred ack timeout
 */
static SWS_CB( void, TcpOnAckTimeout(
  SWS_TimerH hTimer,                  /* IN: Timer that expired */
  SWS_PTR( void) pvContext            /* IN: User context */
)){
  STcb* const pTcb = (STcb*)pvContext;
  (void)hTimer;

  TRACE( 9, ("TcpOnAckTimeout(%p,%p)\n",hTimer,pvContext));
  ASSERT( NULL != pTcb);
  ASSERT( PlatformIsCritical());
  ASSERT( pTcb->hAckTimer == hTimer);

  if ( pTcb->deferredAck)
    {
    TRACE( 3, ("TcpOnAckTimeout() TCB %p\n",pTcb));
    TcpSendSeg(
      pTcb,
      FORCE_SNDF | SWS_TCP_ACKF,
      pTcb->SndLast,
      pTcb->RcvNxt
    );
    }
  }


/*
 * Probe zero send window
 */
static SWS_CB( void, TcpOnZeroWinTimeout(
  SWS_TimerH hTimer,                  /* IN: Timer that expired */
  SWS_PTR( void) pvContext            /* IN: User context */
)){
  STcb* const pTcb = (STcb*)pvContext;

  TRACE( 9, ("TcpOnZeroWinTimeout(%p,%p)\n",hTimer,pvContext));
  ASSERT( NULL != pTcb);
  ASSERT( PlatformIsCritical());
  ASSERT( pTcb->hZeroWinTimer == hTimer);

  TRACE( 2, ("TcpOnZeroWinTimeout() TCB %p t/out %lu\n",
    pTcb, pTcb->zeroWinTimeout));

  /* Exponentially backoff the probe interval */
  pTcb->zeroWinTimeout *= 2;

  /* Send a window probe */
  TcpSendSeg(
    pTcb,
    FORCE_SNDF | SWS_TCP_ACKF,
    pTcb->SndLast,
    pTcb->RcvNxt
  );

  if ( pTcb->zeroWinTimeout > 0)
    {
    if ( pTcb->SndWnd == pTcb->SndNxt)
      VERIFY( !SWS_TimerStart( hTimer, pTcb->zeroWinTimeout));
    else
      pTcb->zeroWinTimeout = 0;
    }
  }


/****************************************************************************
 * Segment handling
 ****************************************************************************/

/*
 * Send a TCP segment
 * Single threaded
 */
static int TcpSendSeg(                /* Returns !0 on error */
  STcb* const pTcb,                   /* IN: */
  unsigned flags,                     /* IN: flags */
  ui32 seq,                           /* IN: SEQ */
  ui32 ack                            /* IN: ACK */
) {
  SWS_BufferH hBuffer;
  SWS_size_t uLen;
  size_t hdrLen;
  SWS_PTR( void) pvHdr;
  int iRet;
  ui32 uSum;
  size_t data;
  unsigned urgent = 0;

  TRACE( 9, ("TcpSendSeg(%p,%#x,%lu,%lu)\n", pTcb,flags,seq,ack));
  ASSERT( PlatformIsCritical());
  ASSERT( NULL != pTcb);

  ASSERT( SWS_INADDR_ANY != pTcb->remoteHost);
  ASSERT( SWS_INADDR_BROADCAST != pTcb->remoteHost);

  data = 0;
  if ( !(SWS_TCP_RSTF & flags) )
    {
    size_t const pending = TcpSendPending( pTcb, seq);
    i32 window;
    int isStreaming;

    isStreaming = pTcb->isStreaming;
    pTcb->isStreaming = 0;

    /* Calc available send window */
    window = SeqDelta( pTcb->SndWnd, seq);
    if ( window > 0)
      {
      TRACE( 4, ("TcpSendSeg(%p) window:%ld pending:%u unackd:%ld\n",
        pTcb,window,pending, SeqDelta( pTcb->SndLast, pTcb->SndUna)));

      /* Get amount of queued data */
      if ( pending < (ui32)window)
        window = (i32)pending;

      /* Silly window avoidance see RFC1122 p99
       * NB all data is assumed PSH'd */
      if ( (ui32)window >= pTcb->SndMSS)
        {
        /* Send a max sized segment */
        data = pTcb->SndMSS;

        /* Request another segment & disable Nagle on next segment */
        pTcb->isStreaming = (char)(pending >= 2 * data);
        }
      else if ( pending <= (ui32)window
        /* RFC 896 (Nagle - don't send until all ack'd */
        /* NB Don't allow opt.noNagle override except if timeout retry */
        && (isStreaming
          || pTcb->SndUna == pTcb->SndNxt
          || (pTcb->opt.noNagle && (flags & FORCE_SNDF))
        )
      ) {
        /* Send all queued data */
        data = pending;
        }
      else if ( (ui32)window >= pTcb->SndWndMax / 2
        /* RFC 896 (Nagle - don't send until all ack'd */
        && (isStreaming
          || pTcb->SndUna == pTcb->SndNxt
          || pTcb->opt.noNagle
        )
      ) {
        /* Send a window full */
        data = (size_t)window;
        }
      else if ( flags & (FORCE_SNDF | REPEAT_SNDF))
        {
        /* Send a window full */
        data = (size_t)window;
        }
      else if ( pending)
        {
        TRACE( 4, ("TcpSendSeg(%p) %u bytes deferred\n",pTcb,pending));

        /* Start a timeout to retry */
        if ( 0 == pTcb->zeroWinTimeout)
          {
          VERIFY( !SWS_TimerStart(
            pTcb->hZeroWinTimer,
            pTcb->zeroWinTimeout = s_ackDelay
          ));
          }
        }

      if ( data > 0)
        {
        /* Kill deferred data retry timeout */
        SWS_TimerStop( pTcb->hZeroWinTimer);
        pTcb->zeroWinTimeout = 0;

        /* Slow start / congestion avoidance */
        window = SeqDelta( pTcb->SndUna + pTcb->cwnd, seq);
        if ( window <= 0)
          data = 0;
        else if ( (ui32)window < data)
          data = (size_t)window;
        }
      }
    else if ( pending)
      {
      TRACE( 3, ("TcpSendSeg(%p) Window full, %u bytes pending\n",pTcb,pending));
      }

    if ( SWS_TCP_FINF & flags)
      {
      /* Ensure all data is sent before FIN */
      if ( pending > data)
        {
        /* Defer FIN until after all data sent */
        TRACE( 2, ("TcpSendSeg(%p,%#x,%lu) %s, FIN deferred, %u pending\n",
          pTcb,flags,seq,kszStates[ pTcb->eState], pending));
        pTcb->deferredFin = !0;
        flags &= ~SWS_TCP_FINF;
        if ( !flags)
          return -1;
        }
      }
    else if ( data == pending)
      {
      /* Check for deferred FIN */
      if ( pTcb->deferredFin)
        {
        pTcb->deferredFin = 0;

        /* Append FIN */
        flags |= SWS_TCP_FINF | SWS_TCP_ACKF;
        ack = pTcb->RcvNxt;
        TRACE( 2, ("TcpSendSeg(%p,%#x,%lu) %s sending deferred FIN\n",
          pTcb,flags,seq,kszStates[ pTcb->eState]));
        }
      }

    /* Any data? */
    if ( 0 == data)
      {
      /* No data */
      if ( !flags)
        return -1; /* Nothing to send */

      /* Is this a simple ACK only segment?
       * And not the reply to a 2nd unack'd segment? */
      if ( SWS_TCP_ACKF == flags
        && pTcb->RvcNoAck < 2
      ) {
        /* Has the ACK been sent before? */
        if ( ack == pTcb->RcvAckd)
          {
          /* Yes, don't repeat it */
          return -1;
          }
        else
          {
          /* No, then defer the ACK so that it may be piggybacked */
          if ( !pTcb->deferredAck)
            {
            pTcb->deferredAck = 1;
            VERIFY( !SWS_TimerStart( pTcb->hAckTimer, s_ackDelay));
            }

          TRACE( 4, ("TcpSendSeg(%p,%#x,%lu,%lu) ACK deferred\n",
            pTcb,flags,seq,ack));
          return -1;
          }
        }
      }

    /* Check for deferred ACK */
    if ( pTcb->deferredAck)
      {
      SWS_TimerStop( pTcb->hAckTimer);
      pTcb->deferredAck = 0;
      if ( !(SWS_TCP_ACKF & flags))
        {
        flags |= SWS_TCP_ACKF;
        ack = pTcb->RcvNxt;
        }
      TRACE( 4, ("TcpSendSeg(%p,%#x,%lu,%lu) Sending deferred ACK\n",
        pTcb,flags,seq,ack));
      }
    else if ( !flags)
      {
      flags = SWS_TCP_ACKF; /* Always send ACK with data */
      ack = pTcb->RcvNxt;
      }

    /* If segment is not being re-sent then start re-transmit timer */
    if ( !(flags & REPEAT_SNDF) )
      {
      int const special = (flags & (SWS_TCP_SYNF | SWS_TCP_FINF)) ? 1 : 0;
      if ( data || special)
        {
        /* Add the segment to the re-transmit queue */
        TcpPushRttQ( pTcb, seq + data + special);
        }
#ifndef NDEBUG
      /* Trap repeat empty segments
       * NB These may be valid deferred acks, window updates
       * or acks after a missing segment */
      else if ( seq == pTcb->SndNxt
        && !(flags & FORCE_SNDF)
        && (!(flags & SWS_TCP_ACKF) || ack == pTcb->RcvAckd)
      ) {
        TRACE( 3, ("TcpSendSeg(%p) ? Repeat SEQ %lu ACK %lu, FLG %#x\n",
          pTcb,seq,ack,flags));
        }
#endif
      }

    if ( data && data == pending)
      {
      /* All sent so set PSH */
      flags |= SWS_TCP_PSHF;
      }
    } /* endif ( !(SWS_TCP_RSTF & flags) ) */

  if ( !s_iTxResources && !(flags & (FORCE_SNDF | REPEAT_SNDF)) )
    {
    TRACE( 3, ("TcpSendSeg(%p) no Tx resources\n",pTcb));

    if ( SWS_TCP_FINF & flags)
      pTcb->deferredFin = !0;

    return -1; /* Wait for retransmit timeout or TcpSendInd */
    }

  hdrLen = SWS_TCP_MINLEN;
  if ( SWS_TCP_SYNF & flags)
    {
    hdrLen += SWS_TCP_OPT_MSS_SIZE; /* Space for MSS option */
#if OPT_TCP_SACKP
    hdrLen += SWS_TCP_OPT_SACKP_SIZE; /* Space for SACKP option */
#endif
#define OPT_TCP_WS 0
#if OPT_TCP_WS
    hdrLen += SWS_TCP_OPT_WS_SIZE; /* Space for WS option */
#endif
    hdrLen = ROUNDUP( hdrLen, sizeof(ui32)); /* Round up to 32-bits */
    }

  if ( SWS_TCP_ACKF & flags)
    pTcb->RvcNoAck = 0; /* Reset count of segments received since last ack */

  /* Get a buffer for the TCP header */
  ASSERT( 0 == hdrLen % sizeof(ui32));
  hBuffer = SWS_BufferGet( hdrLen, SWS_kBufferPrefix);
  if ( NULL == hBuffer)
    {
    if ( SWS_TCP_FINF & flags)
      pTcb->deferredFin = !0;

    return SWS_ENOBUFS; /* Wait for retransmit timeout */
    }

  if ( data)
    {
    size_t segment;
    const size_t SndCur = TcpSendCursor( pTcb, seq);

    /* Calc size of 1st fragment of send buffer */
    if ( pTcb->SndNew > SndCur)
      segment = pTcb->SndNew - SndCur;
    else
      segment = pTcb->SndBuf - SndCur;

    if ( segment > data)
      segment = data;

    /* Append the 1st data fragment */
    if ( SWS_BufferAppendBackShadow(
      hBuffer, segment, (char*)pTcb->pvSndBuf + SndCur)
    ) {
      SWS_BufferRelease( hBuffer);
      if ( SWS_TCP_FINF & flags)
        pTcb->deferredFin = !0;

      return SWS_ENOBUFS; /* Wait for retransmit timeout */
      }
    if ( data > segment)
      {
      /* Add the 2nd fragment */
      if ( SWS_BufferAppendBackShadow(
        hBuffer, data - segment, (char*)pTcb->pvSndBuf)
      ) {
        SWS_BufferRelease( hBuffer);
        if ( SWS_TCP_FINF & flags)
          pTcb->deferredFin = !0;

        return SWS_ENOBUFS; /* Wait for retransmit timeout */
        }
      }
    ASSERT( SWS_BufferSize( hBuffer) == hdrLen + data);

    urgent = TcpSendUrgent( pTcb, seq);
    if ( urgent)
      {
      if ( urgent > data)
        urgent = data;

      flags |= SWS_TCP_URGF;
      }
    }

  if ( (flags & SWS_TCP_ACKF) && SeqLT( pTcb->RcvAckd, ack) )
    pTcb->RcvAckd = ack;

  if ( !(SWS_TCP_RSTF & flags))
    {
    uSum = seq + data;
    if ( !SeqLT( uSum, pTcb->SndNxt))
      {
      /* Update SndNxt */
      pTcb->SndNxt = uSum;
      if ( SWS_TCP_SYNF & flags)
        ++(pTcb->SndNxt);
      if ( SWS_TCP_FINF & flags)
        ++(pTcb->SndNxt);

      pTcb->SndLast = pTcb->SndNxt;
      pTcb->retransmits = 0;
      }
    else
      pTcb->SndLast = uSum;
    }

  pvHdr = SWS_BufferGetFragment( hBuffer, 0, &uLen);
  ASSERT( NULL != pvHdr);
  ASSERT( uLen >= hdrLen);

  /* Fill in the TCP header */
  SWS_POKE16( pvHdr, SWS_TCP_SRC, pTcb->localPort);
  SWS_POKE16( pvHdr, SWS_TCP_DST, pTcb->remotePort);
  SWS_POKE32( pvHdr, SWS_TCP_SEQ, seq);
  SWS_POKE32( pvHdr, SWS_TCP_ACK, ack);
  SWS_POKE16( pvHdr, SWS_TCP_URG, (ui16)urgent);
  ASSERT( flags & (SWS_TCP_RSTF | SWS_TCP_SYNF | SWS_TCP_FINF | SWS_TCP_ACKF));
  flags &= (SWS_TCP_URGF | SWS_TCP_ACKF | SWS_TCP_PSHF
    | SWS_TCP_RSTF | SWS_TCP_SYNF | SWS_TCP_FINF);
  flags |= (hdrLen / sizeof(ui32)) << 12;
  SWS_POKE16( pvHdr, SWS_TCP_FLG, (ui16)flags);
  if ( pTcb->RcvWnd <= 0xffff)
    SWS_POKE16( pvHdr, SWS_TCP_WND, (ui16)pTcb->RcvWnd);
  else
    SWS_POKE16( pvHdr, SWS_TCP_WND, 0xffff);

  SWS_POKE16( pvHdr, SWS_TCP_SUM, 0);
  if ( SWS_TCP_SYNF & flags)
    {
    unsigned mss;

    /* Add TCP options */
    iRet = SWS_TCP_OPT;

    /* MSS option */
    ASSERT( hdrLen >= (unsigned)iRet + SWS_TCP_OPT_MSS_SIZE);
    SWS_POKE8( pvHdr, iRet++, SWS_TCP_OPT_MSS);
    SWS_POKE8( pvHdr, iRet++, SWS_TCP_OPT_MSS_SIZE);
    mss = TcpCalcSmss( pTcb);
    SWS_POKE16( pvHdr, iRet, (ui16)mss);
    iRet += 2;

#if OPT_TCP_SACKP
    /* SACKP option */
    ASSERT( hdrLen >= (unsigned)iRet + SWS_TCP_OPT_SACKP_SIZE);
    SWS_POKE8( pvHdr, iRet++, SWS_TCP_OPT_SACKP);
    SWS_POKE8( pvHdr, iRet++, SWS_TCP_OPT_SACKP_SIZE);
#endif

#if OPT_TCP_WS
    /* WS option */
    ASSERT( hdrLen >= (unsigned)iRet + SWS_TCP_OPT_WS_SIZE);
    SWS_POKE8( pvHdr, iRet++, SWS_TCP_OPT_WS);
    SWS_POKE8( pvHdr, iRet++, SWS_TCP_OPT_WS_SIZE);
    SWS_POKE8( pvHdr, iRet++, 0);
#endif

    /* Terminate options if trailing space */
    if ( hdrLen > (unsigned)iRet)
      SWS_POKE8( pvHdr, iRet, SWS_TCP_OPT_END);
    }

  /* Calc checksum */
  uLen = SWS_BufferSize( hBuffer);
  uSum = SWS_IpPseudoHdr(
    pTcb->localHost, pTcb->remoteHost, SWS_TCP_PROTOCOL, uLen);
  SWS_POKECSUM( pvHdr, SWS_TCP_SUM, SWS_IpChecksum( hBuffer, 0, uLen, uSum) );
  /*ASSERT( !SWS_IpChecksum( hBuffer, 0, uLen, uSum));*/

  TRACE( 3, ("TcpSendSeg(%p,%#x,%lu,%lu) %u bytes, %s\n",
    pTcb, flags, seq, ack, uLen - hdrLen, kszStates[ pTcb->eState] ));

  /* Send the segment */
  s_iTxResources = 0;

  iRet = SWS_IpSend(
    pTcb->localHost,
    pTcb->remoteHost,
    SWS_TCP_PROTOCOL,
    hBuffer,
    SWS_IP_IDENT_DEFAULT,
    &pTcb->ipOptions
  );
  if ( -1 != iRet)
    s_iTxResources = 1;
  else
    {
    TRACE( 3, ("TcpSendSeg(%p) Awaiting Tx resources\n", pTcb));
    iRet = 0;
    }

  if ( 0 == iRet)
    {
    AtomicIncrement( &s_aTxDatagrams);
    AtomicAdd( &s_aTxBytes, (int)uLen);
    }
  else
    {
    AtomicIncrement( &s_aTxErrors);
    }

  SWS_BufferReleaseShadow( hBuffer);

  /* Queue a DPC if there's more to send */
  if ( pTcb->isStreaming
    && !pTcb->dpcPending
  ) {
    pTcb->dpcPending = 1;
    if ( PlatformDpc( &TcpSendDpc, pTcb, NULL, 0))
      {
      /* Wait for receive seg */
      TRACE( 1, ("*** TcpSendSeg(%p) Out of DPCs\n", pTcb));
      pTcb->dpcPending = 0;
      }
    }

  return iRet;
  }

/* DPC to send next segment */
static SWS_CB( void, TcpSendDpc(
  SWS_PTR( void) pv1,
  SWS_PTR( void) pv2,
  ui32 uParam
)){
  STcb* const pTcb = (STcb*)pv1;
  (void)pv2;
  (void)uParam;

  TRACE( 5, ("TcpSendDpc(%p,%p,%lu)\n",pv1,pv2,uParam));

  if ( s_bRegProtocol)
    {
    pTcb->dpcPending = 0;

    if ( s_iTxResources
      && kStateClosed != pTcb->eState
    ) {
      TcpSendSeg( pTcb, 0, pTcb->SndLast, 0);
      }
    }
  }


/*
 * Buffer a received TCP segment
 * Single threaded
 */
static int TcpRecvSeg(
  STcb* const pTcb,
  SWS_BufferH const hBuffer,
  unsigned hdr,
  unsigned seglen,
  ui32 const seq,
  unsigned const urgent,
  int const isPush
) {
  i32 offset;

  TRACE( 9, ("TcpRecvSeg(%p,%p,%u,%u,%lu,%u,%d)\n",
    pTcb,hBuffer,hdr,seglen,seq,urgent,isPush));
  ASSERT( PlatformIsCritical());

  if ( 0 == seglen)
    return 0;

  /* Reset the connection if Rx disabled & data received */
  if ( pTcb->rxDisable)
    {
    /* Send a RST */
    TRACE( 2, ("TcpRecvSeg(%p) Receive disabled, sending RST\n", pTcb));
    TcpSendSeg( pTcb, SWS_TCP_RSTF, pTcb->SndNxt, 0);
    TcpNewState( pTcb, kStateClosed);

    /* Abort all pending ops */
    SWS_TransportEventSet( pTcb, SWS_kTransportEventError, SWS_ECONNABORTED);

    return SWS_EINVAL;
    }

  /* Empty (ack only) segments are deferred by SendSeg so count
   * the pending acks, NB May not defer more than 1 (RFC1122 p96) */
  pTcb->wantAck = 1; /* Ack the segment */
  ++(pTcb->RvcNoAck);

  /* Match data with LHS of receive window */
  offset = SeqDelta( seq, pTcb->RcvNxt);
  if ( 0 >= offset)
    {
    ui32 temp;
    size_t space;

    ASSERT( offset + (i32)seglen > 0);

    /* Skip data already received */
    if ( offset < 0)
      {
      offset = -offset;
      if ( seglen <= (unsigned)offset)
        return 0; /* Nothing new */

      seglen -= (unsigned)offset;
      hdr += (unsigned)offset;
      }

    if ( pTcb->RcvWnd < seglen)
      {
      TRACE( 1, ("*** TcpRecvSeg(%p) len %u > window %lu\n",
        pTcb, seglen, pTcb->RcvWnd));
      seglen = (unsigned)pTcb->RcvWnd;
      }

    if ( urgent & !(pTcb->soflags & SWS_SO_OOBINLINE))
      {
      TRACE( 2, ("TcpRecvSeg(%p) Urgent len %u\n", pTcb, urgent));
      ASSERT( seglen >= urgent);

      /* TODO: Separate buffer for OOB data */
      SWS_TransportEventSet( pTcb, SWS_kTransportEventOob, 1);

      pTcb->RcvNxt += urgent;
      seglen -= urgent;
      if ( 0 == seglen)
        return 0;
      }

    /* Append data to receive queue */
    TRACE( 3, ("TcpRecvSeg(%p) SEQ %lu len %u\n", pTcb, seq, seglen));

    /* Calc linear free space from RcvNew */
    if ( pTcb->RcvNew >= pTcb->RcvOld)
      {
      space = pTcb->RcvBuf - pTcb->RcvNew;
      if ( 0 == pTcb->RcvOld)
        --space;
      }
    else
      {
      space = pTcb->RcvOld - (1 + pTcb->RcvNew);
      }

    if ( !space)
      {
      TRACE( 1, (
        "*** TcpRecvSeg(%p) Overflow SEQ %lu len %u\n", pTcb, seq, seglen));
      AtomicIncrement( &s_aRxOverflow);
      SWS_GefRaise( SWS_kGefTcpError, "Receive overflow");
      return SWS_ENOBUFS;
      }

    if ( space > seglen)
      space = seglen;

    /* Copy data to RcvNew */
    SWS_BufferCopyToMemory(
      (char*)pTcb->pvRcvBuf + pTcb->RcvNew, hBuffer, hdr, space);

    if ( seglen > space)
      {
      /* Calc linear free space from start of buffer */
      size_t avail, rem;
      rem = seglen - space;

      if ( pTcb->RcvNew < pTcb->RcvOld)
        {
        TRACE( 1, (
          "*** TcpRecvSeg(%p) Overflow SEQ %lu len %u\n", pTcb, seq, seglen));
        AtomicIncrement( &s_aRxOverflow);
        SWS_GefRaise( SWS_kGefTcpError, "Receive overflow");
        return SWS_ENOBUFS;
        }

      avail = pTcb->RcvOld;
      if ( avail > 0)
        --avail;

      if ( avail < rem)
        {
        TRACE( 1, (
          "*** TcpRecvSeg(%p) Overflow SEQ %lu len %u\n", pTcb, seq, seglen));
        AtomicIncrement( &s_aRxOverflow);
        SWS_GefRaise( SWS_kGefTcpError, "Receive overflow");
        return SWS_ENOBUFS;
        }
      else if ( avail > rem)
        {
        avail = rem;
        }

      SWS_BufferCopyToMemory( pTcb->pvRcvBuf, hBuffer, hdr + space, avail);
      }

    pTcb->RcvNxt += seglen;
    ASSERT( pTcb->RcvWnd >= seglen);
    pTcb->RcvWnd -= seglen;

    /* Update the receive buffer write cursor */
    temp = (ui32)pTcb->RcvNew + seglen;
    if ( temp >= pTcb->RcvBuf)
      temp -= pTcb->RcvBuf;

    pTcb->RcvNew = (size_t)temp;
    ASSERT( pTcb->RcvWnd <= TcpRecvFree( pTcb) );

    /* Mark position of pushed segment */
    if ( isPush)
      pTcb->RcvPsh = (size_t)temp;

    /* Indicate Rx ready if PSH'd or no room for another full segment */
    if ( isPush
      || pTcb->RcvWnd < 2 * pTcb->SndMSS
      || pTcb->RcvWnd < pTcb->RcvBuf / 2
    ) {
      SWS_TransportEventSet( pTcb, SWS_kTransportEventRxReady, 1);
      }
    }
  else
    {
    /* TODO: BufferOutOfSequenceData */
#if OPT_TCP_SACKP
    /* TODO Send SACK */
#endif
    TRACE( 1, (
      "*** TcpRecvSeg() missing segment seq=%lu len %u, RcvNxt=%lu\n",
      seq, seglen, pTcb->RcvNxt));
    SWS_GefRaise( SWS_kGefTcpError, "Missing segment");

    pTcb->RvcNoAck = 2; /* Send an immediate ACK to facilitate fast re-transmit */
    }

  return 0;
  }


/*
 * Process a received ACK
 * Single threaded
 */
static SWS_FASTCALL( void, TcpRecvAck(
  STcb* const pTcb,
  ui32 const ack
)){
  SRttInfo info;
  int err;

  TRACE( 9, ("TcpRecvAck(%p,%lu)\n", pTcb,ack));
  ASSERT( PlatformIsCritical());

  /* Match ack with pending segments */
  info.ticks = 0;
  while ( 0 == (err = TcpPeekRttQ( pTcb, &info)) )
    {
    if ( SeqLT( ack, info.seq))
      {
      err = 1;
      break; /* Still pending */
      }

    TcpPopRttQ( pTcb, &info);
    if ( ack == info.seq)
      break;
    }

  /* Karn's algorithm prohibits calculating RTT's from re-transmissions */
  if ( !err
    && info.ticks > 0
    && !pTcb->retransmits
  ) {
    i32 m = info.ticks; /* RTT */
    ui32 rto;

    /* Jacobson's algorithm for smoothing RTT */
    /* see ftp.ee.lbl.gov/papers/congavoid.ps.Z */

    /* 1. Update the average estimator */
    m -= pTcb->sa / VJ_SCALEM;  /* err = m - a */
    pTcb->sa += m;              /* a = a + err/SCALEM */

    /* 2. Update the deviation estimator */
    if ( m < 0)
      m = -m;
    m -= pTcb->sv / VJ_SCALED;  /* d = |err| - v */
    pTcb->sv += m;              /* v = v + d/SCALED */

    /* Is mean reliable? */
    rto = (pTcb->sa / VJ_SCALEM);
    if ( rto > pTcb->sv || pTcb->sv < s_rtoMin)
      {
      /* 3. Update the RTO = a + SCALED x v */
      rto += pTcb->sv;

      /* For variable latency links add more deviation */
      rto += pTcb->sv;

      /* Approach asymtopically */
      pTcb->rto = (pTcb->rto + rto) / 2;

      if ( pTcb->rto < s_rtoMin)
        pTcb->rto = s_rtoMin;
      else if ( pTcb->rto > s_rtoMax)
        pTcb->rto = s_rtoMax;
      }

    TRACE( 4, ("TcpRecvAck(%p) RTO %lu ms, Mean %lu, Dev %lu\n",
      pTcb, pTcb->rto, pTcb->sa / VJ_SCALEM, pTcb->sv / VJ_SCALED));
    }

  if ( !SeqLE( ack, pTcb->SndUna) )
    {
    ui32 uAckd;

    uAckd = pTcb->RcvDupAck;
    pTcb->RcvDupAck = 0;

    /* Update the congestion window as per RFC 2581 */
    if ( uAckd >= 3)
      {
      /* Fast retransmit recovery */
      pTcb->cwnd = pTcb->ssthresh;
      }
    else if ( pTcb->cwnd > pTcb->ssthresh)
      {
      /* Congestion avoidance */
      ui32 inc;

      ASSERT( pTcb->cwnd);
      inc = ((ui32)pTcb->SndMSS * (ui32)pTcb->SndMSS) / pTcb->cwnd;
      if ( inc)
        pTcb->cwnd += inc;
      else
        pTcb->cwnd++;

      TRACE( 4, ("TcpRecvAck() Congestion avoidance new cwnd %lu\n",
        pTcb->cwnd));
      }
    else
      {
      /* Slow start */
      pTcb->cwnd += pTcb->SndMSS;
      TRACE( 4, ("TcpRecvAck() Slow start new cwnd %lu\n", pTcb->cwnd));
      }

    uAckd = (ui32)SeqDelta( ack, pTcb->SndUna);
    if ( uAckd <= TcpSendUsed( pTcb))
      {
      /* Remove ack'd data on re-transmit queue */
      uAckd += pTcb->SndOld;
      if ( uAckd >= pTcb->SndBuf)
        uAckd -= pTcb->SndBuf;
      pTcb->SndOld = (size_t)uAckd;

      /* Update URG data cursor */
      if ( uAckd >= pTcb->SndUp)
        pTcb->SndUp = 0;
      else
        pTcb->SndUp -= (size_t)uAckd;
      }
    else
      {
      /* A FIN has been sent which returns ACK = SndUna + 1 */
      ASSERT( kStateFinWait1 == pTcb->eState
        || kStateFinWait2 == pTcb->eState
        || kStateClosing == pTcb->eState
        || kStateLastAck == pTcb->eState
      );
      pTcb->SndOld = pTcb->SndNew;
      pTcb->SndUp = 0;
      }

    TRACE( 4, ("TcpRecvAck() ACK %lu..%lu\n", pTcb->SndUna, ack));
    pTcb->SndUna = ack;
    if ( SeqLT( pTcb->SndLast, ack))
      pTcb->SndLast = ack;

    if ( !pTcb->retransmits)
      SWS_TransportEventSet( pTcb, SWS_kTransportEventTxReady, 1);
    else
      pTcb->RvcNoAck = 2; /* Don't delay ACK */

    /* Attempt to send another segment */
    pTcb->wantAck = 1;
    }
  else
    {
    /* RFC 2581 fast retransmit after 3 duplicate acks */
    if ( 3 <= ++(pTcb->RcvDupAck) )
      {
      if ( 3 == pTcb->RcvDupAck
        && pTcb->SndUna != pTcb->SndNxt
        && pTcb->SndLast == pTcb->SndNxt
        && !pTcb->retransmits
      ) {
        TRACE( 2, ("TcpRecvAck(%p) Fast retransmit ACK %lu, NXT %lu\n",
          pTcb, ack, pTcb->SndNxt));
        AtomicIncrement( &s_aRetransmits);
        SWS_GefRaise( SWS_kGefTcpError, "Fast retransmit");

        pTcb->SndLast = pTcb->SndUna;
        pTcb->RvcNoAck = 2; /* Don't delay ACK */

        TcpCongestion( pTcb);
        }

      pTcb->cwnd = pTcb->ssthresh + pTcb->RcvDupAck * (ui32)pTcb->SndMSS;

      /* Attempt to re-send a segment */
      pTcb->wantAck = 1;
      }
    else
      {
      TRACE( 3, (
        "TcpRecvAck(%p) Duplicate ACK %lu, NXT %lu\n",
        pTcb, ack, pTcb->SndNxt));
      }
    }
  }


/*
 * Congestion control as per RFC 2581
 */
static SWS_FASTCALL( void, TcpCongestion( STcb* const pTcb))
  {
  ui32 uFlightSize, uHalfFlightSize, twoMSS;

  TRACE( 9, ("TcpCongestion(%p)\n", pTcb));
  ASSERT( NULL != pTcb);

  uFlightSize = (ui32)SeqDelta( pTcb->SndNxt, pTcb->SndUna);
  uHalfFlightSize = uFlightSize / 2;
  twoMSS = 2 * (ui32)pTcb->SndMSS;

  if ( uHalfFlightSize < twoMSS)
    pTcb->ssthresh = twoMSS;
  else
    pTcb->ssthresh = uHalfFlightSize;

  TRACE( 2, ("TcpCongestion(%p) new ssthresh:%lu, flightsize %lu\n",
    pTcb, pTcb->ssthresh, uFlightSize ));

  /* Switch to slow start */
  pTcb->cwnd = twoMSS;
  }


/*
 * TCP options handling
 */
static SWS_FASTCALL( size_t, TcpRcvOptions(
  SWS_PTR( void) pvHdr,
  size_t tcpHdrLen
)){
  size_t mss = 0;
  size_t offset;

  TRACE( 9, ("TcpRcvOptions(%p,%u)\n", pvHdr, tcpHdrLen));
  ASSERT( NULL != pvHdr);

  offset = SWS_TCP_MINLEN;
  while ( offset < tcpHdrLen)
    {
    unsigned opt, len;

    /* Get option kind */
    SWS_PEEK8( opt, pvHdr, offset);

    /* Decode option */
    if ( SWS_TCP_OPT_END == opt)
      break;
    else if ( SWS_TCP_OPT_NOP == opt)
      ++offset;
    else
      {
      SWS_PEEK8( len, pvHdr, offset + 1);
      switch ( opt)
        {
      case SWS_TCP_OPT_MSS:
        SWS_PEEK16( mss, pvHdr, offset + 2);
        TRACE( 3, ("TcpRcvOptions() MSS %u\n", mss));
        break;

      case SWS_TCP_OPT_SACKP:
        TRACE( 2, ("TcpRcvOptions() SACKP\n"));
#if OPT_TCP_SACKP
        /* TODO enable SACK transmit for missing blocks */
#endif
        break;

      case SWS_TCP_OPT_SACK:
        TRACE( 2, ("TcpRcvOptions() SACK len %u\n", len));
#if OPT_TCP_SACKP
        /* TODO fast retransmit missing blocks */
#endif
        break;

      case SWS_TCP_OPT_WS:
        SWS_PEEK8( opt, pvHdr, offset + 2);
        TRACE( 2, ("TcpRcvOptions() WS %u\n", opt));
        /* TODO store window shift */
        break;

      default:
        TRACE( 2, ("TcpRcvOptions() type %u len %u\n", opt, len));
        break;
        }
      offset += len;
      }
    }

  return mss;
  }


/****************************************************************************
 * Downcalls from SwsSock
 ****************************************************************************/

/*
 * Transport layer is closing
 * Single threaded
 */
static SWS_CB( void, TcpCleanup( SWS_PTR( void) pvContext))
  {
  (void)pvContext;

  TRACE( 9, ("TcpCleanup(%p)\n", pvContext));
  ASSERT( PlatformIsCritical());

  /* Close any remaining TCB's */
  while ( NULL != s_pBoundTcps)
    {
    TcpOnError( s_pBoundTcps, SWS_ECONNABORTED);
    TcpClose( s_pBoundTcps, 1);
    }

  s_bRegTransport = 0;

  if ( s_bRegProtocol)
    {
    VERIFY( !SWS_IpDeregisterProtocol( &s_protocolTcp, NULL));
    s_bRegProtocol = 0;
    }
  }


/*
 * Open a Service Access Point
 */
SWS_CB( int, TcpOpen(
  SWS_PTR( void) pvContext,
  SWS_PTR( void) pvSap,               /* IN: -> SAP data */
  int type,                           /* IN: socket type */
  int protocol                        /* IN: protocol */
)){
  (void)pvContext;

  TRACE( 9, ("TcpOpen(%p,%p,%d,%d)\n", pvContext, pvSap, type, protocol));
  ASSERT( NULL != pvSap);
  (void)type;
  (void)protocol;

  memset( pvSap, 0, sizeof( STcb) );
  return TcpOnCreate( (STcb*)pvSap, s_sndBuf, s_rcvBuf);
  }


/*
 * Close a Service Access Point
 */
SWS_CB( int, TcpClose(
  SWS_PTR( void) pvSap,
  int reset
)){
  SCloseInfo close;

  TRACE( 9, ("TcpClose(%p,%d)\n", pvSap,reset));
  ASSERT( NULL != pvSap);

  close.pTcb = (STcb*)pvSap;
  close.eMode = reset ? ECloseReset : ECloseNormal;
  return PlatformSynchronise( &TcpOnClose, &close);
  }


/*
 * Associate a Service Access Point with a local address & port
 */
SWS_CB( int, TcpBind(
  SWS_PTR( void) pvSap,               /* IN: IN: -> SAP */
  SWS_PTR( const struct SWS_sockaddr) pName,
  int namelen
)){
  STcb* const pTcb = (STcb*)pvSap;
  ui32 localHost;
  unsigned localPort;

  TRACE( 9, ("TcpBind(%p,%p,%d)\n", pvSap, pName, namelen));
  ASSERT( NULL != pTcb);
  ASSERT( NULL != pName);
  ASSERT( 0 == pTcb->localPort); /* sws_sock prevents duplicate binds */
  ASSERT( kStateClosed == pTcb->eState);

  if ( SWS_AF_INET != pName->sa_family)
    {
    TRACE( 1, ("*** TcpBind() unsupported address type\n"));
    return SWS_EAFNOSUPPORT;
    }

  if ( namelen != sizeof( struct SWS_sockaddr_in))
    {
    TRACE( 1, ("*** TcpBind() namelen %d incorrect\n", namelen));
    return SWS_EINVAL;
    }

  /* Ensure local address is valid */
  NetRead32(
    localHost,
    &((SWS_PTR(const struct SWS_sockaddr_in))pName)->sin_addr.SWS_s_addr
  );
  if ( 0 == SWS_IpGetMaxSizes( localHost, localHost, 0, NULL))
    {
    TRACE( 1, ("*** TcpBind() invalid local host\n"));
    return SWS_EADDRNOTAVAIL;
    }

  NetRead16(
    localPort, &((SWS_PTR(const struct SWS_sockaddr_in))pName)->sin_port);
  return TcpOnBind( pTcb, localHost, localPort, 0);
  }


/*
 * Wait for an incoming connection request
 */
SWS_CB( int, TcpListen( SWS_PTR( void) pvSap))
  {
  STcb* const pTcb = (STcb*)pvSap;

  TRACE( 9, ("TcpListen(%p)\n", pvSap));
  ASSERT( NULL != pvSap);

  ASSERT( pTcb->localPort); /* sws_sock ensures that the SAP is bound */

  return TcpOnListen( pTcb);
  }


/*
 * Associate a Service Access Point with a remote address & port
 */
SWS_CB( int, TcpConnect(
  SWS_PTR( void) pvSap,               /* IN: IN: -> SAP */
  SWS_PTR( const struct SWS_sockaddr) pName,
  int namelen
)){
  ui32 remoteHost;
  unsigned remotePort;
  STcb* const pTcb = (STcb*)pvSap;

  TRACE( 9, ("TcpConnect(%p,%p,%d)\n", pvSap, pName, namelen));
  ASSERT( NULL != pTcb);
  ASSERT( NULL != pName);

  if ( namelen != sizeof( struct SWS_sockaddr_in))
    {
    TRACE( 1, ("*** TcpConnect() namelen %d incorrect\n", namelen));
    return SWS_EINVAL;
    }

  if ( SWS_AF_INET != pName->sa_family)
    {
    TRACE( 1, ("*** TcpConnect() incorrect address type\n"));
    return SWS_EINVAL;
    }

  /* Ensure address is reachable */
  NetRead32(
    remoteHost,
    &((SWS_PTR(const struct SWS_sockaddr_in))pName)->sin_addr.SWS_s_addr
  );
  if ( 0 == SWS_IpGetMaxSizes( pTcb->localHost, remoteHost, 0, NULL))
    {
    TRACE( 1, ("*** TcpConnect() %08lx not reachable\n", remoteHost));
    return SWS_ENETUNREACH;
    }

  /* Validate remote address */
  NetRead16(
    remotePort, &((SWS_PTR(const struct SWS_sockaddr_in))pName)->sin_port);
  if ( 0 == remotePort || SWS_INADDR_ANY == remoteHost)
    {
    TRACE( 1, ("*** TcpConnect() invalid remote address or port\n"));
    return SWS_EADDRNOTAVAIL;
    }
  if ( SWS_INADDR_BROADCAST == remoteHost)
    {
    TRACE( 1, ("*** TcpConnect() broadcast not permitted\n"));
    return SWS_EACCES;
    }

  return TcpOnConnect( pTcb, remoteHost, remotePort);
  }


/*
 * Break an existing connection
 */
SWS_CB( int, TcpShutdown( SWS_PTR( void) pvSap, int how ))
  {
  STcb* const pTcb = (STcb*)pvSap;
  int iRet;
  SCloseInfo close;

  TRACE( 9, ("TcpShutdown(%p,%d)\n", pvSap,how));
  ASSERT( NULL != pvSap);

  if ( how & SWS_SD_SEND)
    {
    TRACE( 2, ("TcpShutdown(%p,SEND)\n",pTcb));

    close.pTcb = pTcb;
    close.eMode = ECloseShutdown;
    iRet = PlatformSynchronise( &TcpOnClose, &close);
    if ( iRet > 0)
      return iRet;
    }

  if ( how & SWS_SD_RECEIVE)
    {
    /* Disable receive */
    pTcb->rxDisable = 1;

    /* Abort all pending receives */
    SWS_TransportEventSet( pTcb, SWS_kTransportEventRxReady, SWS_ECONNRESET);

    /* If any data remains then reset the connection */
    if ( TcpRecvPending( pTcb))
      {
      TRACE( 2, ("TcpShutdown(%p) RECV data pending\n", pTcb));

      close.pTcb = pTcb;
      close.eMode = ECloseReset;
      PlatformSynchronise( &TcpOnClose, &close);
      }
    }
  else
    {
    /* Enable receives */
    pTcb->rxDisable = 0;
    }

  return 0;
  }


/*
 * Send data to remote connection
 */
SWS_CB( int, TcpSendTo(
  SWS_PTR( void) pvSap,               /* IN: IN: -> SAP */
  SWS_PTR( const void) pBuf,          /* IN: -> data to send */
  SWS_PTR( unsigned) pLen,            /* I/O: data len */
  unsigned flags,                     /* IN: flags, SWS_MSG_xxx */
  SWS_PTR( const struct SWS_sockaddr) pName,/* IN: Optional name */
  int namelen                         /* IN: namelen */
)){
  STcb* const pTcb = (STcb*)pvSap;

  TRACE( 9, ("TcpSendTo(%p,%p,%p,%u,%p,%d)\n",
    pvSap, pBuf, pLen, flags, pName, namelen));
  ASSERT( NULL != pvSap);
  ASSERT( NULL != pLen);
  ASSERT( NULL != pBuf || 0 == *pLen);

  /* Validate params */
  /* NB ignore SWS_MSG_DONTROUTE */
  if ( flags & ~(SWS_MSG_DONTROUTE | SWS_MSG_OOB))
    {
    TRACE( 1, ("*** TcpSendTo() unsupported flags\n"));
    return SWS_EINVAL;
    }

  if ( (flags & SWS_MSG_OOB) && (pTcb->soflags & SWS_SO_OOBINLINE) )
    {
    TRACE( 1, ("*** TcpSendTo() SWS_MSG_OOB specified with SWS_SO_OOBINLINE\n"));
    return SWS_EINVAL;
    }

  if ( NULL != pName)
    {
    /* Check remote address */
    if ( SWS_AF_INET != pName->sa_family)
      {
      TRACE( 1, ("*** TcpSendTo() unsupported address type\n"));
      return SWS_EAFNOSUPPORT;
      }

    if ( namelen != sizeof( struct SWS_sockaddr_in))
      {
      TRACE( 1, ("*** TcpSendTo() namelen %d incorrect\n", namelen));
      return SWS_EINVAL;
      }

    /* Ignore the actual address */
    }

  return TcpOnWrite( pTcb, pBuf, pLen, flags);
  }


/*
 * Get data on a connection
 */
SWS_CB( int, TcpRecvFrom(
  SWS_PTR( void) pvSap,               /* IN: IN: -> SAP */
  SWS_PTR( void) pBuf,                /* OUT: Data */
  SWS_PTR( unsigned) pLen,            /* I/O: buffer len */
  unsigned flags,                     /* IN: flags, SWS_MSG_xxx */
  SWS_PTR( struct SWS_sockaddr) pName, /* OUT: Optional name */
  SWS_PTR( int) pNamelen              /* I/O: namelen */
)){
  STcb* const pTcb = (STcb*)pvSap;

  TRACE( 9, ("TcpRecvFrom(%p,%p,%p,%u,%p,%p)\n",
    pvSap,pBuf,pLen,flags,pName,pNamelen));
  ASSERT( NULL != pvSap);
  ASSERT( NULL != pLen);
  ASSERT( NULL != pBuf || 0 == *pLen);

  if ( NULL != pName)
    {
    if ( NULL == pNamelen)
      {
      TRACE( 1, ("*** TcpRecvFrom() NULL namelen\n"));
      return SWS_EFAULT;
      }

    if ( *pNamelen < (int)sizeof( struct SWS_sockaddr_in))
      {
      TRACE( 1, ("*** TcpRecvFrom() namelen %d too small\n", *pNamelen));
      return SWS_EINVAL;
      }

    memset( pName, 0, sizeof(struct SWS_sockaddr_in));
    pName->sa_family = SWS_AF_INET;
    NetWrite16(
      &((SWS_PTR(struct SWS_sockaddr_in))pName)->sin_port,
      pTcb->remotePort
    );
    NetWrite32(
      &((SWS_PTR(struct SWS_sockaddr_in))pName)->sin_addr.SWS_s_addr,
      pTcb->remoteHost
    );
    *pNamelen = sizeof(struct SWS_sockaddr_in);
    }

  if ( flags & ~(SWS_MSG_PEEK | SWS_MSG_OOB))
    {
    TRACE( 1, ("*** TcpRecvFrom() unsupported flags\n"));
    return SWS_EINVAL;
    }

  if ( (flags & SWS_MSG_OOB) && (pTcb->soflags & SWS_SO_OOBINLINE) )
    {
    TRACE( 1, ("*** TcpRecvFrom() SWS_MSG_OOB specified with SWS_SO_OOBINLINE\n"));
    return SWS_EINVAL;
    }

  return TcpOnRead( pTcb, pBuf, pLen, flags);
  }


/*
 * Get info
 */
SWS_CB( int, TcpGetInfo(
  SWS_PTR( void) pvSap,               /* IN: -> SAP data */
  unsigned level,                     /* IN: level SWS_SOL_xxx */
  unsigned opt,                       /* IN: option */
  SWS_PTR( void) pVal,                /* OUT: -> value */
  SWS_PTR( int) pLen                  /* I/O: length of value */
)){
  STcb* const pTcb = (STcb*)pvSap;
  size_t size;

  TRACE( 9, ("TcpGetInfo(%p,%u,%u,%p,%p)\n", pvSap,level,opt,pVal,pLen));
  ASSERT( NULL != pTcb);
  ASSERT( NULL != pVal);
  ASSERT( NULL != pLen);
  ASSERT( *pLen > 0);

  switch ( level)
    {
  case SWS_SOL_SOCKET:
    switch ( opt)
      {
    /*case SWS_SO_KEEPALIVE:*/
    case SWS_SO_OOBINLINE:
    case SWS_SO_REUSEADDR:
    case SWS_SO_DONTROUTE:
    /*case SWS_SO_BROADCAST:*/
    case SWS_SO_USELOOPBACK:
      if ( *pLen != sizeof(int))
        return SWS_EFAULT;

      *(SWS_PTR(int))pVal = (int)(pTcb->soflags & opt);
      break;

    case SWS_SO_MAX_MSG_SIZE:
      if ( *pLen != sizeof(unsigned))
        return SWS_EFAULT;

      /* Get max Tx datagram */
      size = SWS_IpGetMaxSizes(
        pTcb->localHost,
        SWS_INADDR_ANY != pTcb->remoteHost ? pTcb->remoteHost : pTcb->localHost,
        0, NULL
      );

      /* Deduct TCP header */
      if ( size >= SWS_TCP_MINLEN)
        size -= SWS_TCP_MINLEN;

      *(SWS_PTR(unsigned))pVal = (unsigned)size;
      break;

    case SWS_SO_RCVBUF:
      if ( *pLen != sizeof(unsigned))
        return SWS_EFAULT;

      *(SWS_PTR(unsigned))pVal = (unsigned)pTcb->RcvBuf;
      break;

    case SWS_SO_SNDBUF:
      if ( *pLen != sizeof(unsigned))
        return SWS_EFAULT;

      *(SWS_PTR(unsigned))pVal = (unsigned)pTcb->SndBuf;
      break;

    default:
      return SWS_EOPNOTSUPP;
      }
    break;

  case SWS_SOL_IP:
    /* IP options */
    switch ( opt)
      {
    case SWS_IP_HDRINCL:
    case SWS_IP_MULTICAST_IF:
    case SWS_IP_MULTICAST_TTL:
    case SWS_IP_MULTICAST_LOOP:
    case SWS_IP_ADD_MEMBERSHIP:
    case SWS_IP_DROP_MEMBERSHIP:
      return SWS_EOPNOTSUPP;

    default:
      break;
      }

    return SWS_IpGetOption( &pTcb->ipOptions, opt, pVal, pLen);

  case SWS_SOL_TCP:
    /* TCP options */
    switch ( opt)
      {
    case SWS_TCP_NODELAY:
      if ( *pLen < (int)sizeof(int))
        return SWS_EFAULT;

      *pLen = (int)sizeof(int);
      *(SWS_PTR(int))pVal = pTcb->opt.noNagle;
      break;

    case SWS_TCP_MAXSEG:
      if ( *pLen < (int)sizeof(int))
        return SWS_EFAULT;

      *pLen = (int)sizeof(unsigned);
      *(SWS_PTR(unsigned))pVal = pTcb->maxSeg;
      break;

    default:
      return SWS_EOPNOTSUPP;
      }
    break;

  case SWS_SOL_IOCTL:
    /* sws_sock private */
    switch ( opt)
      {
    default:
      return SWS_EOPNOTSUPP;

    case SWS_IOCTL_FIONREAD:
      if ( *pLen != sizeof(SWS_u_long))
        return SWS_EFAULT;

      /* Get size of receive buffer */
      *(SWS_PTR(SWS_u_long))pVal = (SWS_u_long) TcpRecvPending( pTcb);
      break;
      }
    break;

  default:
    return SWS_ENOPROTOOPT;
    }

  return 0;
  }


/*
 * Set info
 */
SWS_CB( int, TcpSetInfo(
  SWS_PTR( void) pvSap,               /* IN: -> SAP data */
  unsigned level,                     /* IN: level SWS_SOL_xxx */
  unsigned opt,                       /* IN: option */
  SWS_PTR( const void) pVal,          /* IN: -> value */
  int len                             /* IN: length of value */
)){
  STcb* const pTcb = (STcb*)pvSap;
  size_t size;
  void* pv;

  TRACE( 9, ("TcpSetInfo(%p,%u,%u,%p,%d)\n", pvSap,level,opt,pVal,len));
  ASSERT( NULL != pTcb);
  ASSERT( NULL != pVal);
  ASSERT( len > 0);

  switch ( level)
    {
  case SWS_SOL_SOCKET:
    switch ( opt)
      {
    /*case SWS_SO_KEEPALIVE:*/
    case SWS_SO_OOBINLINE:
    case SWS_SO_REUSEADDR:
    case SWS_SO_DONTROUTE:
    /*case SWS_SO_BROADCAST:*/
    case SWS_SO_USELOOPBACK:
      if ( len != sizeof(int))
        return SWS_EFAULT;

      if ( *(SWS_PTR(const int))pVal)
        pTcb->soflags |= opt;
      else
        pTcb->soflags &= ~opt;
      break;

    case SWS_SO_SNDBUF:
      /* Resize the send buffer */
      if ( len != sizeof(int))
        return SWS_EFAULT;

      size = (size_t)*(SWS_PTR(const unsigned int))pVal;
      if ( size < s_sndBuf)
        size = s_sndBuf;

      pv = realloc( pTcb->sndBufMallocd ? pTcb->pvSndBuf : NULL, size);
      if ( NULL == pv)
        {
        TRACE( 1, ("TcpSetInfo(%p) Out of memory for send buffer\n", pvSap));
        return SWS_ENOBUFS;
        }

      pTcb->SndNew = pTcb->SndOld = pTcb->SndUp = 0;
      pTcb->SndBuf = size;
      pTcb->pvSndBuf = pv;
      pTcb->sndBufMallocd = 1;
      break;

    case SWS_SO_RCVBUF:
      /* Resize the receive buffer */
      if ( len != sizeof(int))
        return SWS_EFAULT;

      size = (size_t)*(SWS_PTR(const unsigned int))pVal;
      if ( size < s_rcvBuf)
        size = s_rcvBuf;

      pv = realloc( pTcb->rcvBufMallocd ? pTcb->pvRcvBuf : NULL, size);
      if ( NULL == pv)
        {
        TRACE( 1, ("TcpSetInfo(%p) Out of memory for receive buffer\n", pvSap));
        return SWS_ENOBUFS;
        }

      pTcb->RcvNew = pTcb->RcvOld = pTcb->RcvPsh = 0;
      pTcb->RcvBuf = size;
      pTcb->pvRcvBuf = pv;
      pTcb->rcvBufMallocd = 1;
      break;

    default:
      return SWS_EOPNOTSUPP;
      }
    break;

  case SWS_SOL_IP:
    /* IP options */
    switch ( opt)
      {
    case SWS_IP_HDRINCL:
    case SWS_IP_MULTICAST_IF:
    case SWS_IP_MULTICAST_TTL:
    case SWS_IP_MULTICAST_LOOP:
    case SWS_IP_ADD_MEMBERSHIP:
    case SWS_IP_DROP_MEMBERSHIP:
      return SWS_EOPNOTSUPP;

    default:
      break;
      }
    return SWS_IpSetOption( &pTcb->ipOptions, opt, pVal, len);

  case SWS_SOL_TCP:
    /* TCP options */
    switch ( opt)
      {
    case SWS_TCP_NODELAY:
      if ( len != (int)sizeof(int))
        return SWS_EFAULT;

      pTcb->opt.noNagle = 0 != *(SWS_PTR(const int))pVal;
      break;

    case 0xdeaf:
      if ( len != (int)sizeof(int))
        return SWS_EFAULT;

      pTcb->opt.closeQuietly = (char)(0 != *(SWS_PTR(const int))pVal);
      break;

    case SWS_TCP_MAXSEG:
    default:
      return SWS_EOPNOTSUPP;
      }
    break;

  default:
    return SWS_EOPNOTSUPP;
    }

  return 0;
  }


/*
 * Get the address of the local host
 */
static SWS_CB( int, TcpGetSockName(
  SWS_PTR( void) pvSap,               /* IN: -> SAP data */
  SWS_PTR( struct SWS_sockaddr) pName,/* OUT: name */
  SWS_PTR( int) pLen                  /* IN/OUT: namelen */
)){
  STcb* const pTcb = (STcb*)pvSap;

  TRACE( 9, ("TcpGetSockName(%p,%p,%p)\n", pvSap,pName,pLen));
  ASSERT( NULL != pTcb);
  ASSERT( NULL != pName);
  ASSERT( NULL != pLen);

  if ( NULL == pLen || *pLen < (int)sizeof( struct SWS_sockaddr_in))
    return SWS_EFAULT;

#if 1
  if ( SWS_INADDR_ANY == pTcb->localHost)
    return SWS_EINVAL;
#endif

  ((SWS_PTR(struct SWS_sockaddr_in))pName)->sin_family = SWS_AF_INET;
  NetWrite16(
    &((SWS_PTR(struct SWS_sockaddr_in))pName)->sin_port,
    pTcb->localPort
  );
  NetWrite32(
    &((SWS_PTR(struct SWS_sockaddr_in))pName)->sin_addr.SWS_s_addr,
    pTcb->localHost
  );

  *pLen = sizeof( struct SWS_sockaddr_in);

  return 0;
  }


/*
 * Get the address of the remote host
 */
static SWS_CB( int, TcpGetPeerName(
  SWS_PTR( void) pvSap,               /* IN: -> SAP data */
  SWS_PTR( struct SWS_sockaddr) pName,/* OUT: name */
  SWS_PTR( int) pLen                  /* IN/OUT: namelen */
)){
  STcb* const pTcb = (STcb*)pvSap;

  TRACE( 9, ("TcpGetPeerName(%p,%p,%p)\n", pvSap,pName,pLen));
  ASSERT( NULL != pTcb);
  ASSERT( NULL != pName);
  ASSERT( NULL != pLen);

  if ( NULL == pLen || *pLen < (int)sizeof( struct SWS_sockaddr_in))
    return SWS_EFAULT;

  if ( SWS_INADDR_ANY == pTcb->remoteHost)
    return SWS_ENOTCONN;

  ((SWS_PTR(struct SWS_sockaddr_in))pName)->sin_family = SWS_AF_INET;
  NetWrite16(
    &((SWS_PTR(struct SWS_sockaddr_in))pName)->sin_port,
    pTcb->remotePort
  );
  NetWrite32(
    &((SWS_PTR(struct SWS_sockaddr_in))pName)->sin_addr.SWS_s_addr,
    pTcb->remoteHost
  );

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
SWS_CB( void, TcpRecvInd(
  SWS_PTR( void) pvContext,
  SWS_BufferH hBuffer,
  size_t hdrLen,
  ui32 src,                           /* IP src address */
  ui32 dst,                           /* IP dst address */
  ui32 spec,                          /* IP address of receiving adapter */
  unsigned tos
)){
  STcb* pTcb;
  SWS_PTR( void) pvHdr;
  SWS_size_t uLen, uTcpLen;
  unsigned flags, len, localPort, remotePort;
  ui32 uSum;
  (void)pvContext;
  (void)tos;

  TRACE( 9, ("TcpRecvInd(%p,%p,%u,%lu,%lu,%lu,%u)\n",
    pvContext,hBuffer,hdrLen,src,dst,spec,tos));
  ASSERT( PlatformIsCritical());
  ASSERT( NULL != hBuffer);

  AtomicIncrement( &s_aRxDatagrams);

  uLen = SWS_BufferSize( hBuffer);
  if ( uLen < hdrLen + SWS_TCP_MINLEN)
    {
    TRACE( 1, ("*** TcpRecvInd() No header\n"));
    AtomicIncrement( &s_aRxErrors);
    SWS_GefRaise( SWS_kGefTcpError, "Missing header");
    return;
    }

  /* Get TCP header */
  pvHdr = SWS_BufferGetFragment( hBuffer, hdrLen, &uTcpLen);
  ASSERT( NULL != pvHdr);
  if ( uTcpLen < SWS_TCP_MINLEN)
    {
    TRACE( 1, ("*** TcpRecvInd() Truncated header\n"));
    AtomicIncrement( &s_aRxErrors);
    SWS_GefRaise( SWS_kGefTcpError, "Truncated header");
    return;
    }

  SWS_PEEK16( flags, pvHdr, SWS_TCP_FLG);
  len = sizeof(ui32) * (flags >> 12);
  if ( len < SWS_TCP_MINLEN)
    {
    TRACE( 1, ("*** TcpRecvInd() Invalid length field\n"));
    AtomicIncrement( &s_aRxErrors);
    SWS_GefRaise( SWS_kGefTcpError, "Header too small");
    return;
    }

  /* Ensure buffer length is correct */
  if ( uLen < hdrLen + len)
    {
    TRACE( 1, ("*** TcpRecvInd() Truncated data\n"));
    AtomicIncrement( &s_aRxErrors);
    SWS_GefRaise( SWS_kGefTcpError, "Truncated data");
    return;
    }
  uLen -= hdrLen;

  /* Verify checksum */
  uSum = SWS_IpPseudoHdr( src, dst, SWS_TCP_PROTOCOL, uLen);
  if ( 0 != SWS_IpChecksum( hBuffer, hdrLen, uLen, uSum))
    {
    TRACE( 1, ("*** TcpRecvInd() Bad checksum\n"));
    AtomicIncrement( &s_aRxErrors);
    SWS_GefRaise( SWS_kGefTcpError, "Bad checksum");
    return;
    }
  AtomicAdd( &s_aRxBytes, (int)uLen);

  /* Get end points */
  SWS_PEEK16( localPort, pvHdr, SWS_TCP_DST);
  SWS_PEEK16( remotePort, pvHdr, SWS_TCP_SRC);

  /* Match with bound SAP */
  if ( NULL == (pTcb = TcpFindTcb( localPort, remotePort, spec, src))
    && NULL == (pTcb = TcpFindTcb( localPort, remotePort, SWS_INADDR_ANY, src))
    && NULL == (pTcb = TcpFindTcb( localPort, 0, spec, SWS_INADDR_ANY))
    && NULL == (pTcb = TcpFindTcb( localPort, 0, SWS_INADDR_ANY, SWS_INADDR_ANY))
  ) {
    TRACE( 4, ("TcpRecvInd() %08lx:%u -> %08lx:%u port not known\n",
      src, remotePort,dst,localPort));
    AtomicIncrement( &s_aRxNoPort);

#if !OPT_TCP_DISCARD_NOPORT
    if ( !s_ignore_noport
      && dst == spec
    ) {
#if 1
      /* Is this a RST? */
      if ( !(SWS_TCP_RSTF & flags))
        {
        /* No, send a RST */
        STcb tcb;
        ui32 seq, ack;

        SWS_PEEK32( seq, pvHdr, SWS_TCP_SEQ);
        SWS_PEEK32( ack, pvHdr, SWS_TCP_ACK);
        if ( SWS_TCP_ACKF & flags)
          {
          seq = ack;
          ack = 0;
          flags = SWS_TCP_RSTF;
          }
        else
          {
          ack = seq + (ui32)(uLen - (SWS_size_t) len);
          seq = 0;
          flags = SWS_TCP_RSTF | SWS_TCP_ACKF;
          }

        pTcb = &tcb;
        memset( pTcb, 0, sizeof( *pTcb) );
        pTcb->remotePort = (ui16)remotePort;
        pTcb->localPort = (ui16)localPort;
        pTcb->remoteHost = src;
        pTcb->localHost = spec;

        TcpSendSeg( pTcb, flags, seq, ack);
        }
#elif OPT_PROTO_ICMP
      /* Send an ICMP port not reachable */
      SWS_BufferH hb = SWS_BufferDup( hBuffer, hdrLen + 8);
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
#endif
      }
#endif

    return;
    }

  TcpOnReceive( pTcb, spec, remotePort, src, hBuffer, hdrLen, len);
  }


#if OPT_PROTO_ICMP
/*
 * ICMP message received indication
 * Single threaded
 */
SWS_CB( void, TcpIcmpInd(
  SWS_PTR( void) pvContext,           /* IN: Client context */
  SWS_BufferH hBuffer,                /* IN: IP datagram - do not modify */
  size_t hdrLen,                      /* IN: IP header length including options */
  ui32 src,                           /* IN: src IP address in original datagram */
  ui32 dst,                           /* IN: dst IP address in original datagram */
  unsigned type,                      /* IN: ICMP type */
  unsigned code,                      /* IN: ICMP code */
  ui32 data                           /* IN: ICMP optional data (identifier & sequence) */
)){
  SWS_PTR( void) pvHdr;
  STcb* pTcb;
  SWS_size_t uLen;
  unsigned localPort, remotePort;
  (void)data;
  (void)code;
  (void)dst;
  (void)pvContext;

  TRACE( 9, ("TcpIcmpInd(%p,%p,%u,%08lx,%08lx,%u,%u,%lu)\n",
    pvContext,hBuffer,hdrLen,src,dst,type,code,data));
  ASSERT( PlatformIsCritical());
  ASSERT( NULL != hBuffer);

  pvHdr = SWS_BufferGetFragment( hBuffer, hdrLen, &uLen);
  ASSERT( NULL != pvHdr);
  if ( uLen < 8)
    return;

  SWS_PEEK16( localPort, pvHdr, SWS_TCP_SRC);
  SWS_PEEK16( remotePort, pvHdr, SWS_TCP_DST);

  switch ( type)
    {
  case SWS_ICMP_DEST_UNREACH:
    TRACE( 2, ("TcpIcmpInd() Host %#08lx port %u unreachable(%u) from %08lx port %u\n",
      dst, remotePort, code, src, localPort));
    AtomicIncrement( &s_aIcmpErrors);
    break;

  case SWS_ICMP_QUENCH:
    TRACE( 2, ("TcpIcmpInd() Quench from %08lx port %u for %08lx port %u\n",
      dst, remotePort, src, localPort));
    break;

  case SWS_ICMP_TIME_EXCEED:
    TRACE( 2, ("TcpIcmpInd() Time exceeded(%u) from %08lx port %u for %08lx port %u\n",
      code, dst, remotePort, src, localPort));
    AtomicIncrement( &s_aIcmpErrors);
    break;

  case SWS_ICMP_PARAM_PROB:
    TRACE( 2, ("TcpIcmpInd() Param prob(%u) from %08lx port %u for %08lx port %u\n",
      code, dst, remotePort, src, localPort));
    AtomicIncrement( &s_aIcmpErrors);
    break;
    }

  /* Match with bound SAP */
  if ( NULL != (pTcb = TcpFindTcb( localPort, remotePort, src, dst))
    || NULL != (pTcb = TcpFindTcb( localPort, remotePort, SWS_INADDR_ANY, dst))
    || NULL != (pTcb = TcpFindTcb( localPort, 0, src, SWS_INADDR_ANY))
    || NULL != (pTcb = TcpFindTcb( localPort, 0, SWS_INADDR_ANY, SWS_INADDR_ANY))
  ) {
    TcpOnIcmpError( pTcb, type, code);
    }
  }
#endif


/*
 * Send resources available
 * Single threaded
 */
SWS_CB( void, TcpSendInd(
  SWS_PTR( void) pvContext
)){
  (void)pvContext;

  TRACE( 3, ("TcpSendInd()\n"));
  ASSERT( PlatformIsCritical());

  if ( !s_iTxResources)
    {
    STcb* pTcb;

    s_iTxResources = 1;

    /* Signal all TCB's */
    for (
      pTcb = s_pBoundTcps;
      s_iTxResources && NULL != pTcb;
      pTcb = pTcb->pNext
    ) {
      switch ( pTcb->eState)
        {
      case kStateClosed:
      case kStateListen:
        break;

      case kStateSynSent:
      case kStateSynReceived:
      case kStateSynBoth:
      case kStateEstablished:
      case kStateFinWait1:
      case kStateFinWait2:
      case kStateCloseWait:
      case kStateClosing:
      case kStateLastAck:
        TcpSendSeg( pTcb, 0, pTcb->SndLast, 0);
        break;

      case kStateTimeWait:
        break;
        }
      }
    }
  }


/****************************************************************************
 * TCB support
 ****************************************************************************/

/*
 * Calc. sender's Maximum Segment Size
 */
static SWS_FASTCALL( size_t, TcpCalcSmss( STcb* const pTcb))
  {
  size_t bufSize;
  size_t mtu = SWS_IpGetMaxSizes(
    pTcb->localHost, pTcb->remoteHost, pTcb->ipOptions.tos, NULL);

  /* Allow for IP header, IP options & TCP header (no TCP options) */
  mtu -= SWS_TCP_MINLEN;

  /* Ensure MSS <= half TxBuf size so that given a full TxBuf at least 2
   * segments are sent thereby ensuring 1 ack even if 1st ack is deferred.
   */
  bufSize = pTcb->SndBuf / 2;
  if ( mtu > bufSize)
    mtu = bufSize;

  /* Ensure MSS <= RxBuf */
  bufSize = pTcb->RcvBuf;
  if ( mtu > bufSize)
    mtu = bufSize;

  if ( pTcb->maxSeg && pTcb->maxSeg < mtu)
    mtu = pTcb->maxSeg;

  TRACE( 3, ("TcpCalcSmss(%p) %u\n", pTcb, (unsigned)mtu));
  return mtu;
  }


/*
 * Find a TCB bound to a given local port.
 */
static STcb* TcpFindTcb(
  unsigned const localPort,
  unsigned const remotePort,
  ui32 const host,
  ui32 const remote
) {
  STcb* pTcb;

  TRACE( 9, ("TcpFindTcb(%u,%u,%08lx,%08lx)\n",
    localPort,remotePort,host,remote));

  /* A hash table would be faster */
  for ( pTcb = s_pBoundTcps; NULL != pTcb; pTcb = pTcb->pNext)
    {
    if ( localPort == pTcb->localPort
      && (remotePort == pTcb->remotePort || ~0U == remotePort)
      && (pTcb->localHost == host || SWS_INADDR_NONE == host)
      && (pTcb->remoteHost == remote || SWS_INADDR_NONE == remote)
    ) {
      break;
      }
    }
  return pTcb;
  }


/*
 * Terminate the oldest TCB in timed wait
 */
static SWS_FASTCALL( void, TcpTerminateTimedWait( void))
  {
  STcb* pTcb, *pTcbFound = NULL;

  TRACE( 9, ("TcpTerminateTimedWait()\n"));
  ASSERT( PlatformIsCritical());

  /* Find oldest TCB in timed wait */
  for ( pTcb = s_pBoundTcps; NULL != pTcb; pTcb = pTcb->pNext)
    {
    if ( kStateTimeWait == pTcb->eState && !pTcb->shutdown)
      pTcbFound = pTcb;
    }
  if ( NULL != pTcbFound)
    {
    /* Terminate the TCB */
    SWS_GefRaise( SWS_kGefTcpError, "Terminated TCB in time-wait");
    TcpOnError( pTcbFound, 0);
    }
  else
    {
    /* Find oldest TCB in FinWait2 */
    for ( pTcb = s_pBoundTcps; NULL != pTcb; pTcb = pTcb->pNext)
      {
      if ( kStateFinWait2 == pTcb->eState && !pTcb->shutdown)
        pTcbFound = pTcb;
      }
    if ( NULL != pTcbFound)
      {
      SWS_GefRaise( SWS_kGefTcpError, "Terminated TCB in fin-wait2");
      TcpOnError( pTcbFound, 0);
      }
    }
  }


/*
 * Add an entry to the RTT queue
 */
static SWS_FASTCALL( int, TcpPushRttQ( STcb* const pTcb, ui32 seq))
  {
  SRttInfo* pInfo;
  unsigned head, tail;

  TRACE( 9, ("TcpPushRttQ(%p,%lu)\n",pTcb,seq));
  ASSERT( PlatformIsCritical());

  pInfo = &pTcb->rttQ[ head = pTcb->rttQHead];

  /* Save the re-transmit info */
  pInfo->ticks = (i32)pTcb->rto + (pInfo->sent = PlatformTime());
  pInfo->seq = seq;

  /* Start RTO timer if 1st entry */
  if ( head == (tail = pTcb->rttQTail))
    VERIFY( !SWS_TimerStart( pTcb->hRtoTimer, pTcb->rto));

  /* Bump head cursor */
  if ( ++head >= lengthof( pTcb->rttQ))
    head = 0;

  /* Check if Q full */
  if ( head == tail)
    {
    TRACE( 3, ("TcpPushRttQ(%p) full\n",pTcb));
    return SWS_ENOBUFS;
    }

  pTcb->rttQHead = head;
  return 0;
  }


/*
 * Remove the oldest entry from the RTT queue
 */
static SWS_FASTCALL( int, TcpPopRttQ( STcb* const pTcb, SRttInfo* pInfo))
  {
  unsigned tail;
  i32 ticks;
  ui32 now;

  TRACE( 9, ("TcpPopRttQ(%p,%p)\n",pTcb,pInfo));
  ASSERT( NULL != pInfo);
  ASSERT( PlatformIsCritical());

  if ( (tail = pTcb->rttQTail) == pTcb->rttQHead)
    return SWS_ENOBUFS; /* Empty */

  /* Remove oldest */
  *pInfo = pTcb->rttQ[ tail];
  now = PlatformTime();
  pInfo->ticks = SWS_TimerDelta( now, pInfo->sent ); /* RTT */

  if ( ++tail >= lengthof( pTcb->rttQ) )
    pTcb->rttQTail = tail = 0;
  else
    pTcb->rttQTail = tail;

  if ( tail == pTcb->rttQHead)
    {
    /* Queue is empty */
    if ( pTcb->SndNxt == pInfo->seq)
      {
      /* Stop the RTO 'cos none pending */
      ticks = (i32)SWS_TimerStop( pTcb->hRtoTimer);
      TRACE( 4, ("TcpPopRttQ(%p) RTO stopped with %ld ms left\n", pTcb,ticks));
      }
    else
      {
      /* Probably previous queue overflow.
       * Restart the RTO with the current RTO */
      TRACE( 3, ("TcpPopRttQ(%p) Q empty but data unack'd\n", pTcb));
      VERIFY( !SWS_TimerStart( pTcb->hRtoTimer, pTcb->rto));
      }
    }
  else
    {
    /* Set the RTO for the oldest pending segment.
     * If it's timed out then fire immediately DPC's get run */
    ticks = SWS_TimerDelta( pTcb->rttQ[ tail].ticks, now);

    TRACE( 4, ("TcpPopRttQ(%p) Restart RTO with %ld ms\n", pTcb,ticks));
    VERIFY( !SWS_TimerStart( pTcb->hRtoTimer, ticks > 0 ? ticks : 0));
    }

  return 0;
  }


/*
 * Peek at oldest entry on the RTT queue
 */
static SWS_FASTCALL( int, TcpPeekRttQ( STcb* const pTcb, SRttInfo* pInfo))
  {
  unsigned tail;

  TRACE( 9, ("TcpPeekRttQ(%p,%p)\n",pTcb,pInfo));
  ASSERT( PlatformIsCritical());
  ASSERT( NULL != pInfo);

  if ( (tail = pTcb->rttQTail) == pTcb->rttQHead)
    return SWS_ENOBUFS; /* Queue is empty */

  *pInfo = pTcb->rttQ[ tail];

  return 0;
  }


/*
 * Empty the RTT queue
 */
static SWS_FASTCALL( void, TcpFlushRttQ( STcb* const pTcb))
  {
  pTcb->rttQTail = pTcb->rttQHead;

  /* Stop the RTO 'cos none pending */
  SWS_TimerStop( pTcb->hRtoTimer);
  }

#else
char SWS_Tcp;
#endif /* OPT_PROTO_TCP */
/* End of file */
