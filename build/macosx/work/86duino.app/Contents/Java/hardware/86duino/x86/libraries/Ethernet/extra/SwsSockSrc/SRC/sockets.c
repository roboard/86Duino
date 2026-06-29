/****************************************************************************
 *
 * File name   : sockets.c
 * Function    : Upper edge socket functions.
 * Project     : SwsSock
 * Authors     : Lawrence Rust
 * Systems     : ANSI C.
 *
 ****************************************************************************
 *
 * Created by Lawrence Rust, Software Systems Consultants
 * lvr@softsystem.co.uk. Tel/Fax +33 5 49 72 79 63
 *
 * 02-Apr-01, LVR, Created.
 * 20-Dec-01, LVR, SWS_accept copies control data.
 * 02-Jan-02, LVR, SWS_send returns bytes sent in non-blocking mode
 * 03-Jan-02, LVR, Fix SWS_sendto as per SWS_send
 * 06-Feb-02, LVR, SWS_connect leaves state as calling if non-blocking
 * 17-Nov-02, LVR, SWS_connect doesn't reject bound sockets.
 * 25-Nov-02, LVR, Added SWS_TransportSetHostname.
 * 29-Nov-02, LVR, SWS_TransportEventSet( Error) sets socket error code.
 * 04-Dec-02, LVR, Added DNS code.
 *                 SWS_getsockname accepts kStateListening & kStateCalling
 * 10-Dec-02, LVR, SWS_TransportEventWait sets TxReady if PlatformWaitEvent
 *                 returns an error from hTxReadyEvent.
 * 30-Sep-03, LVR, Fix SWS_close with non-zero linger timeout (s->refs not
 *                 decremented unless linger timeout exceeded).
 * 12-Oct-04, LVR, Fixed re-initialisation.by initing all static data
 * 11-Jun-05, LVR, Added getproto/serv/byname/number
 * 11-Aug-06, LVR, Change all sockaddr sizes from int to socklen_t
 * 20-Jun-09, LVR, Change SWS_SO_SND/RCVTIMEO from int* to SWS_timeval*
 * 20-Jul-09, LVR, Added SWS_strnicmp
 * 12-Mar-10, LVR, Moved SWS_stricmp & SWS_strnicmp to sws_str.c
 *
 *
 * Comments:
 * --------
 * This file implements the uppper edge sockets API and the lower edge
 * protocol provider API.
 *
 ****************************************************************************
 */

/* Compilation options */
#include "config.h"

#ifndef OPT_GETXBYY
#define OPT_GETXBYY 1 /* !0 for getprotbyname etc */
#endif

#ifndef OPT_MAX_HOSTS     /* Max no. address entries per struct SWS_hostent */
#if OPT_DNS
#define OPT_MAX_HOSTS 16
#else
#define OPT_MAX_HOSTS (OPT_DATALINK_ENTRIES +2)
#endif
#endif

#ifndef OPT_MAX_ALIASES   /* Max no. alias entries per struct SWS_hostent */
#if OPT_DNS
#define OPT_MAX_ALIASES 16
#else
#define OPT_MAX_ALIASES 1
#endif
#endif

#ifndef OPT_MAX_BACKLOG   /* Max no. sockets in listen queue */
#define OPT_MAX_BACKLOG 32
#endif


/* Exports */
#include "sws_sock.h"    /* sockets API */
#include "sws_tprt.h"   /* transport interface */

/* Imports */
#include <stddef.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>

#include "sws_timr.h"
#include "sws_dlnk.h"
#include "sws_msgq.h"
#include "sws_dns.h"
#include "sws_str.h"
#include "sws_ip.h"
#if OPT_DATALINK_P2P
#include "sws_p2p.h"
#endif
#include "ip.h"
#include "platform.h"

#include "debug.h"


/*
 * Macros & Constants
 */

typedef enum EState
  {
  kStateClosed = 0,
  kStateOpening,
  kStateOpen,
  kStateBound,
  kStateListening,
  kStateCalling,
  kStateConnected,
  kStateClosing
  } EState;

#define TVAL2TICKS(ptv) ((ptv)->tv_sec * SWS_TIMER_TICKS + ((ptv)->tv_usec * SWS_TIMER_TICKS) / 1000000UL)
#define TICKS2TVAL(ptv,t) (ptv)->tv_sec = (t) / SWS_TIMER_TICKS, (ptv)->tv_usec = (((t) % SWS_TIMER_TICKS) * 1000000UL) / SWS_TIMER_TICKS

#if 1000 == SWS_TIMER_TICKS
# define MS2TICKS( m) (m)
#else
# define MS2TICKS( m) (((m) * SWS_TIMER_TICKS + 500) / 1000)
#endif

enum
  {
  kMaxHost = 256 /* Max host name */
  };


/*
 * Local data types
 */
typedef struct STransport STransport;

/* Socket instance */
typedef struct SWS_Socket
  {
  ui32 uMagic;
#define SOCKET_MAGIC SWS_MAKEFOURCC( 's','o','c','k')

  struct SWS_Socket* pNext;           /* Free list */
  STransport* pTransport;             /* -> Transport provider */
  EState eState;                      /* Current state */
  int error;                          /* Last error */
  Atomic_t refs;

  /* Control data */
  struct SWS_linger linger;
  SWS_u_long sndtime;
  SWS_u_long rcvtime;
  int isNonBlocking;
  int shutdown;

  /* Pennding connections */
  int listenBacklog;                  /* Max no. pending connections */
  Atomic_t calls;                     /* No of pending connections */
  SWS_MsgqH hBacklogQ;                /* Q of pending connections */
  struct SWS_Socket* pListener;       /* Socket to accept the call */

  /* Events */
  EventH hRxReadyEvent;
  EventH hRxOobEvent;
  EventH hTxReadyEvent;
  EventH hCloseEvent;

  SWS_VARRAY( char, data);            /* VARIABLE LENGTH data for transport */
  } SWS_Socket;

/* Transport provider registration data */
struct STransport
  {
  struct STransport* pNext;           /* Linked list */

  ui16 uVersion;
  ui8 uAddressFamily;                 /* SWS_AF_xxx type */
  ui8 uSocketType;                    /* SWS_SOCK_xxx type */
  ui8 uProtocol;                      /* SWS_IPPROTO_xxx type */
  ui8 reserved1;
  SWS_PTR( const char) pszName;

  /* Dispatch table */
  SWS_PTR( void) pvContext;           /* Context for pfnOpen */
  SWS_FNPTR( SWS_FTransportOpen) pfnOpen;
  SWS_FNPTR( SWS_FTransportClose) pfnClose;
  SWS_FNPTR( SWS_FTransportBind) pfnBind;
  SWS_FNPTR( SWS_FTransportListen) pfnListen;
  SWS_FNPTR( SWS_FTransportConnect) pfnConnect;
  SWS_FNPTR( SWS_FTransportShutdown) pfnShutdown;
  SWS_FNPTR( SWS_FTransportSendTo) pfnSendTo;
  SWS_FNPTR( SWS_FTransportRecvFrom) pfnRecvFrom;
  SWS_FNPTR( SWS_FTransportGetInfo) pfnGetInfo;
  SWS_FNPTR( SWS_FTransportSetInfo) pfnSetInfo;
  SWS_FNPTR( SWS_FTransportCleanup) pfnCleanup;
  SWS_FNPTR( SWS_FTransportGetSockName) pfnGetSockName;
  SWS_FNPTR( SWS_FTransportGetPeerName) pfnGetPeerName;

  /* Socket pool */
  SWS_Socket* pFreeSockets;           /* Free socket list */
  size_t socketSize;                  /* sizeof(SWS_Socket) + transport data */
  size_t sockets;                     /* No. sockets */
  SWS_VARRAY( char, socketArray);     /* VARIABLE LENGTH SWS_Socket array */
  };

/* SocketGet context */
typedef struct SSocketInfo
  {
  STransport* pTransport;             /* IN */
  SWS_Socket* s;                      /* OUT */
  } SSocketInfo;


/*
 * Prototypes
 */
static SWS_FASTCALLDEF( void, SocketCloseAck( SWS_SOCKET));
static FPlatformCallback SocketGet;
static FPlatformCallback SocketPut;
static SWS_FASTCALLDEF( int, IsValidSocket( SWS_SOCKET));


/*
 * Module data
 */

/* Configuration */
static char s_szHostname[ kMaxHost]; /* The hostname */

/* Operation */
static int s_inited;                /* Library is inited */
static STransport* s_transportList; /* Transport module list */
static EventH s_hFalseEvent;
static const char kszLocalHost[] = "localhost";
static ui32 s_ulDnsPriServer, s_ulDnsSecServer;

#if OPT_DNS && OPT_GETXBYY
/* Known protocols */
static const struct SProtocol
  {
  SWS_PTR( const char) pszName;
  short nProtocol;
  } s_protocols[]
= {
  { "icmp",  SWS_IPPROTO_ICMP },
  { "igmp",  SWS_IPPROTO_IGMP },
  { "ggp",   SWS_IPPROTO_GGP },   /* RFC823 */
  { "ip",    SWS_IPPROTO_IPENC }, /* RFC2003 */
  { "st",    SWS_IPPROTO_ST },    /* RFC1190 */
  { "tcp",   SWS_IPPROTO_TCP },
  { "udp",   SWS_IPPROTO_UDP },
  { "rdp",   SWS_IPPROTO_RDP },
  { "sip",   SWS_IPPROTO_IPV6 },  /* RFC2460 */
  { "raw",   SWS_IPPROTO_RAW }
  };
static struct SWS_protoent s_proto;
static SWS_PTR( const char) s_szProtoAliases[ 2];

/* Known services */
static const struct SService
  {
  SWS_PTR( const char) pszName;
  short nPort;
  SWS_PTR( const char) pszProtocol;
  } s_services[]
= {
  { "echo",       SWS_IPPORT_ECHO,      "tcp" },
  { "echo",       SWS_IPPORT_ECHO,      "udp" },
  { "discard",    SWS_IPPORT_DISCARD,   "tcp" },
  { "discard",    SWS_IPPORT_DISCARD,   "udp" },
  { "systat",     SWS_IPPORT_SYSTAT,    "tcp" },
  { "daytime",    SWS_IPPORT_DAYTIME,   "tcp" },
  { "ftp",        SWS_IPPORT_FTP,       "tcp" },
  { "telnet",     SWS_IPPORT_TELNET,    "tcp" },
  { "smtp",       SWS_IPPORT_SMTP,      "tcp" },
  { "time",       SWS_IPPORT_TIMESERVER, "tcp" },
  { "nameserver", SWS_IPPORT_NAMESERVER, "tcp" },
  { "nicname",    SWS_IPPORT_WHOIS,     "tcp" },
  { "tftp",       SWS_IPPORT_TFTP,      "udp" }
  };
static struct SWS_servent s_serv;
static SWS_PTR( const char) s_szServAliases[ 2];
#endif


/*
 * Global data
 */
#if SWS_IPV6
SWS_EXPORTDATADEF( const struct SWS_in6_addr, SWS_in6addr_any) = SWS_IN6ADDR_ANY_INIT;
SWS_EXPORTDATADEF( const struct SWS_in6_addr, SWS_in6addr_loopback) = SWS_IN6ADDR_LOOPBACK_INIT;
#endif


/*
 * Initialise this module
 * Single threaded
 */
int SWS_TransportInit( void)
  {
  TRACE( 9, ("SWS_TransportInit()\n"));
  ASSERT( NULL == s_transportList);
  ASSERT( PlatformIsCritical());

  /* Init static data */
  s_inited = 0;
  s_transportList = NULL;
  s_ulDnsPriServer = PlatformPropertyGetUint( PKEY_DNS_SERVER, SWS_INADDR_ANY );
  s_ulDnsSecServer = PlatformPropertyGetUint( PKEY_DNS_SERVER "2", SWS_INADDR_ANY );

  /* Read config data */
  PlatformPropertyGetString(
    PKEY_DNS_HOSTNAME, "", s_szHostname, sizeof( s_szHostname));

  /* Create an event to wait on for sleeping */
  s_hFalseEvent = PlatformCreateEvent();
  if ( NULL == s_hFalseEvent)
    {
    TRACE( 0, ("!!! SWS_TransportInit() Failed to create event\n"));
    return SWS_ENOMEM;
    }

  s_inited = 1;

  return 0;
  }


/*
 * Finalise this module
 * Single threaded
 */
void SWS_TransportCleanup( void)
  {
  TRACE( 9, ("SWS_TransportCleanup()\n"));
  ASSERT( PlatformIsCritical());

  if ( s_inited)
    {
    STransport* pTransport;

    /* Free alias list */
    SWS_gethostbyname( kszLocalHost);

    /* Cleanup of transports */
    while ( NULL != (pTransport = s_transportList))
      {
      unsigned u;
      char* p;

      /* Close all sockets */
      p = pTransport->socketArray;
      for ( u = 0; u < pTransport->sockets; ++u)
        {
        if ( kStateClosed != ((SWS_Socket*)p)->eState)
          {
#ifndef NDEBUG
          if ( kStateClosing != ((SWS_Socket*)p)->eState)
            {
            TRACE( 1, ("*** SWS_TransportCleanup() SAP %p protocol %u left open\n",
              ((SWS_Socket*)p)->data, pTransport->uProtocol));
            }
#endif

          (pTransport->pfnClose)( ((SWS_Socket*)p)->data, 1);
          SocketCloseAck( (SWS_Socket*) p);
          }

        p += pTransport->socketSize;
        }

      /* Call the transport's cleanup handler */
      if ( pTransport->pfnCleanup)
        (*pTransport->pfnCleanup)( pTransport->pvContext);

      s_transportList = pTransport->pNext;
      free ( pTransport);
      }

    if ( NULL != s_hFalseEvent)
      PlatformDestroyEvent( s_hFalseEvent);

    s_inited = 0;
    }
  }


/*
 * Validate a socket
 */
static SWS_FASTCALL( int, IsValidSocket( SWS_SOCKET s))
  {
  return SWS_INVALID_SOCKET != s
    && SOCKET_MAGIC == s->uMagic;
  }


/*
 * Return to user
 */
static INLINE( void, Idle( void))
  {
  if ( !PlatformIsCritical())
    PlatformIdle();
  }
#define API_RETURN( _e) {Idle(); return _e;}
#define API_RETURNS( _s) API_RETURN( _s)
#define API_RETURNH( _p) API_RETURN( _p)


/****************************************************************************
 * Upper edge socket interface
 ****************************************************************************/

/*
 * Get last error
 */
SWS_EXPORT( int, SWS_GetLastError( void))
  {
  TRACE( 9, ("SWS_GetLastError()\n"));
  return errno;
  }


/*
 * Set last error
 */
SWS_EXPORT( int, SWS_SetLastError( int err))
  {
  TRACE( 9, ("SWS_SetLastError(%d)\n",err));
  return errno = err;
  }
#define SetLastError( e) errno = (e)


/*
 * Create a socket
 */
SWS_EXPORT( SWS_SOCKET, SWS_socket(
  int family,
  int type,
  int protocol
)){
  STransport* pTransport;
  SWS_Socket* s;
  int err = SWS_EAFNOSUPPORT;
  SSocketInfo info;

  TRACE( 9, ("SWS_socket(%d,%d,%d)\n", family,type,protocol));
  ASSERT( s_inited);
  if ( !s_inited)
    {
    SetLastError( SWS_EACCES);
    return SWS_INVALID_SOCKET;
    }

  /* Find a matching transport */
  for ( pTransport = s_transportList;
    NULL != pTransport;
    pTransport = pTransport->pNext
  ) {
    if ( SWS_AF_UNSPEC == family
      || family == pTransport->uAddressFamily
    ) {
      /* Address family matched */
      if ( 0 == type
        || type == pTransport->uSocketType
      ) {
        /* Socket type matched */
        if ( 0 == protocol
          || protocol == pTransport->uProtocol
          || 0 == pTransport->uProtocol
        ) {
          /* Protocol matched */
          err = 0;
          break;
          }
        err = SWS_EPROTONOSUPPORT;
        }

      if ( SWS_EAFNOSUPPORT == err)
        err = SWS_ESOCKTNOSUPPORT;
      }
    }

  if ( err)
    {
    TRACE( 1, ("*** SWS_socket() family:%d type:%d protocol:%d not matched)\n",
      family,type,protocol));
    SetLastError( err);
    API_RETURNS( SWS_INVALID_SOCKET);
    }

  /* Allocate a socket */
  info.pTransport = pTransport;
  err = PlatformSynchronise( &SocketGet, &info);
  if ( err)
    {
    SetLastError( err);
    API_RETURNS( SWS_INVALID_SOCKET);
    }
  s = info.s;
  ASSERT( IsValidSocket( s));
  ASSERT( s->pTransport == pTransport);

  /* Call the transport's open function */
  s->eState = kStateOpening;
  ASSERT( pTransport->pfnOpen);
  err = (*pTransport->pfnOpen)( pTransport->pvContext, s->data, type, protocol);
  if ( err)
    {
    /* Release resources */
    s->eState = kStateClosing;
    SocketCloseAck( s);

    SetLastError( s->error = err);
    API_RETURNS( SWS_INVALID_SOCKET);
    }

  /* Mark socket open */
  s->eState = kStateOpen;

  SetLastError( 0);
  API_RETURNS( s);
  }

/*
 * Allocate a socket from the free list
 * Synchronised
 */
static SWS_CB( int, SocketGet( SWS_PTR( void) pv))
  {
  SWS_PTR( SSocketInfo) const pInfo = (SWS_PTR(SSocketInfo))pv;
  int err;
  STransport* pTransport;
  SWS_Socket* s;

  TRACE( 9, ("SocketGet(%p)\n", pv));
  ASSERT( NULL != pInfo);
  ASSERT( PlatformIsCritical());

  pTransport = pInfo->pTransport;
  ASSERT( NULL != pTransport);

  /* Get first unused socket */
  s = pTransport->pFreeSockets;
  if ( NULL == s)
    {
    TRACE( 2, ("SocketGet() None available\n"));
    return SWS_EMFILE;
    }

  ASSERT( s->pTransport == pTransport);
  ASSERT( kStateClosed == s->eState);
  pTransport->pFreeSockets = s->pNext;
  s->pNext = NULL;
  s->pListener = NULL;

  /* Init socket */
  s->uMagic = SOCKET_MAGIC;
  s->error = 0;
  s->linger.l_onoff = 0;
  s->linger.l_linger = 0;
  s->isNonBlocking = 0;
  s->sndtime = (SWS_u_long)(300.0 * SWS_TIMER_TICKS); /* Secs -> ticks */
  s->rcvtime = WAIT_FOREVER;
  s->shutdown = 0;
  s->listenBacklog = 0;
  s->hBacklogQ = NULL;
  s->hRxReadyEvent = NULL;
  s->hRxOobEvent = NULL;
  s->hTxReadyEvent = NULL;
  s->hCloseEvent = NULL;
  AtomicWrite( &s->refs, 1);
  AtomicWrite( &s->calls, 0);

  /* Allocate resources */
  err = 0;
  do
    {
    s->hRxReadyEvent = PlatformCreateEvent();
    if ( NULL == s->hRxReadyEvent)
      {
      err = SWS_ENOBUFS;
      break;
      }

    s->hRxOobEvent = PlatformCreateEvent();
    if ( NULL == s->hRxOobEvent)
      {
      err = SWS_ENOBUFS;
      break;
      }

    s->hTxReadyEvent = PlatformCreateEvent();
    if ( NULL == s->hTxReadyEvent)
      {
      err = SWS_ENOBUFS;
      break;
      }

    s->hCloseEvent = PlatformCreateEvent();
    if ( NULL == s->hCloseEvent)
      {
      err = SWS_ENOBUFS;
      break;
      }
    }
  while (0);

  if ( err)
    {
    /* Release resources */
    if ( NULL != s->hCloseEvent)
      PlatformDestroyEvent( s->hCloseEvent);
    if ( NULL != s->hTxReadyEvent)
      PlatformDestroyEvent( s->hTxReadyEvent);
    if ( NULL != s->hRxOobEvent)
      PlatformDestroyEvent( s->hRxOobEvent);
    if ( NULL != s->hRxReadyEvent)
      PlatformDestroyEvent( s->hRxReadyEvent);

    s->eState = kStateClosed;
    SocketPut( s);
    return err;
    }

  pInfo->s = s;
  return 0;
  }


/*
 * Close a socket
 */
SWS_EXPORT( int, SWS_close( SWS_SOCKET s))
  {
  int err, reset;

  TRACE( 9, ("SWS_close(%p)\n", s));
  ASSERT( s_inited);
  ASSERT( IsValidSocket( s));
  ASSERT( NULL != s && NULL != s->pTransport);

  if ( !s_inited)
    {
    SetLastError( SWS_EACCES);
    return SWS_SOCKET_ERROR;
    }

  if ( !IsValidSocket( s) )
    {
    SetLastError( SWS_ENOTSOCK);
    API_RETURN( SWS_SOCKET_ERROR);
    }

  switch ( s->eState)
    {
  case kStateOpen:
  case kStateBound:
  case kStateListening:
  case kStateCalling:
  case kStateConnected:
    break;

  default:
    TRACE( 1, ("*** SWS_close() not permitted\n"));
    SetLastError( s->error = SWS_EACCES);
    API_RETURN( SWS_SOCKET_ERROR);
    }

  /* Mark socket as closing */
  AtomicIncrement( &s->refs);
  s->eState = kStateClosing;
  reset = s->linger.l_onoff && 0 == s->linger.l_linger;

  /* Call the transport's close */
  PlatformSetEvent( s->hCloseEvent, 0);
  ASSERT( s->pTransport->pfnClose);
  err = (*s->pTransport->pfnClose)( s->data, reset);
  if ( 0 == err)
    {
    /* Immediate completion */
    AtomicDecrement( &s->refs);
    }
  else if ( -1 != err)
    {
    /* Reset the connection */
    (*s->pTransport->pfnClose)( s->data, 1);
    AtomicDecrement( &s->refs);
    }
  else if ( s->linger.l_onoff)
    {
    /* Wait for async close */
    ui32 ticks;

    ASSERT( s->linger.l_linger);
    if ( s->isNonBlocking)
      ticks = WAIT_NEVER;
    else
      ticks = SWS_TIMER_TICKS * s->linger.l_linger;

    err = (int)PlatformWaitEvent( s->hCloseEvent, ticks);
    switch ( err)
      {
    case 1:
    case SWS_ENOTCONN:
    case SWS_ECONNRESET:
      /* Normal closure */
      err = 0;
      break;

    case 0:
      /* Timed out */
      if ( s->isNonBlocking)
        {
        err = SWS_EWOULDBLOCK;
        }
      else
        {
        /* Reset the connection */
        (*s->pTransport->pfnClose)( s->data, 1);
        err = SWS_ECONNRESET;
        AtomicDecrement( &s->refs);
        }
      break;

    default:
      break;
      }
    }

  SocketCloseAck( s);

  if ( err)
    {
    SetLastError( err);
    API_RETURN( SWS_SOCKET_ERROR);
    }

  SetLastError( 0);
  API_RETURN( 0);
  }


/*
 * Complete a pending close
 */
static SWS_FASTCALL( void, SocketCloseAck(
  SWS_SOCKET s
)){
  TRACE( 9, ("SocketCloseAck(%p)\n",s));
  ASSERT( NULL != s);
  ASSERT( SOCKET_MAGIC == s->uMagic);
  ASSERT( NULL != s && NULL != s->pTransport);

  if ( 0 == AtomicDecrement( &s->refs))
    {
    s->eState = kStateClosed;

    if ( NULL != s->pListener
      && SOCKET_MAGIC == s->pListener->uMagic
    ) {
      /* Decrement the listener's call backlog */
      AtomicDecrement( &s->pListener->calls);
      s->pListener = NULL;
      }

    if ( NULL != s->hBacklogQ)
      {
      SWS_SOCKET sAccept;

      /* Reset unaccepted connections */
      while ( !SWS_MsgqGet( s->hBacklogQ, sizeof( sAccept), &sAccept))
        {
        /* The unaccepted connection may nave been destroyed during shutdown */
        if ( SOCKET_MAGIC == sAccept->uMagic)
          {
          (*sAccept->pTransport->pfnClose)( sAccept->data, 1);
          SocketCloseAck( sAccept);
          }
        AtomicDecrement( &s->calls);
        }

      SWS_MsgqDestroy( s->hBacklogQ);
      s->hBacklogQ = NULL;
      }

    if ( NULL != s->hCloseEvent)
      {
      PlatformDestroyEvent( s->hCloseEvent);
      s->hCloseEvent = NULL;
      }

    if ( NULL != s->hTxReadyEvent)
      {
      PlatformDestroyEvent( s->hTxReadyEvent);
      s->hTxReadyEvent = NULL;
      }

    if ( NULL != s->hRxOobEvent)
      {
      PlatformDestroyEvent( s->hRxOobEvent);
      s->hRxOobEvent = NULL;
      }

    if ( NULL != s->hRxReadyEvent)
      {
      PlatformDestroyEvent( s->hRxReadyEvent);
      s->hRxReadyEvent = NULL;
      }

    /* Free the socket */
    PlatformSynchronise( &SocketPut, s);
    }
  }


/*
 * Place a socket on the free list
 * Synchronised
 */
static SWS_CB( int, SocketPut( SWS_PTR( void) pv))
  {
  SWS_PTR( SWS_Socket) s = (SWS_PTR(SWS_Socket))pv;
  ASSERT( NULL != s && NULL != s->pTransport);
  ASSERT( kStateClosed == s->eState);
  ASSERT( PlatformIsCritical());

  /* Add to head of free socket list */
  s->uMagic = 0;
  s->pNext = s->pTransport->pFreeSockets;
  s->pTransport->pFreeSockets = s;

  return 0;
  }


/*
 * Bind a socket to a local name
 */
SWS_EXPORT( int, SWS_bind(
  SWS_SOCKET s,
  SWS_PTR( const struct SWS_sockaddr) pName,
  SWS_socklen_t namelen
)){
  int err;

  TRACE( 9, ("SWS_bind(%p,%p,%d)\n", s,pName,(int)namelen));
  ASSERT( s_inited);
  ASSERT( IsValidSocket( s));
  ASSERT( NULL != s && NULL != s->pTransport);
  ASSERT( NULL != pName || 0 == namelen);

  if ( !s_inited)
    {
    SetLastError( SWS_EACCES);
    return SWS_SOCKET_ERROR;
    }

  if ( !IsValidSocket( s) )
    {
    SetLastError( SWS_ENOTSOCK);
    API_RETURN( SWS_SOCKET_ERROR);
    }

  switch ( s->eState)
    {
  case kStateOpen:
  case kStateBound:
    break;

  default:
    TRACE( 1, ("*** SWS_bind() not permitted\n"));
    SetLastError( s->error = SWS_EINVAL);
    API_RETURN( SWS_SOCKET_ERROR);
    }

  /* Call the transport's bind */
  ASSERT( s->pTransport->pfnBind);
  err = (*s->pTransport->pfnBind)( s->data, pName, namelen);
  if ( err)
    {
    SetLastError( s->error = err);
    API_RETURN( SWS_SOCKET_ERROR);
    }

  /* Mark socket as connected */
  s->eState = kStateBound;

  SetLastError( 0);
  API_RETURN( 0);
  }


/*
 * Start listening on a socket
 */
SWS_EXPORT( int, SWS_listen( SWS_SOCKET s, int backlog))
  {
  int err;

  TRACE( 9, ("SWS_listen(%p,%d)\n", s,backlog));
  ASSERT( s_inited);
  ASSERT( IsValidSocket( s));
  ASSERT( NULL != s && NULL != s->pTransport);

  if ( !s_inited)
    {
    SetLastError( SWS_EACCES);
    return SWS_SOCKET_ERROR;
    }

  if ( !IsValidSocket( s) )
    {
    SetLastError( SWS_ENOTSOCK);
    API_RETURN( SWS_SOCKET_ERROR);
    }

  switch ( s->eState)
    {
  case kStateBound:
  case kStateListening:
    break;

  case kStateConnected:
    TRACE( 1, ("*** SWS_listen() connected\n"));
    SetLastError( s->error = SWS_EISCONN);
    API_RETURN( SWS_SOCKET_ERROR);

  default:
    TRACE( 1, ("*** SWS_listen() not permitted\n"));
    SetLastError( s->error = SWS_EINVAL);
    API_RETURN( SWS_SOCKET_ERROR);
    }

  /* Ensure backlog is reasonable */
  if ( backlog > OPT_MAX_BACKLOG)
    backlog = OPT_MAX_BACKLOG;
  else if ( backlog < 1)
    backlog = 1;

  if ( NULL == s->hBacklogQ)
    {
    s->hBacklogQ = SWS_MsgqCreate( OPT_MAX_BACKLOG, sizeof( SWS_SOCKET));
    if ( NULL == s->hBacklogQ)
      {
      SetLastError( s->error = SWS_ENOBUFS);
      API_RETURN( SWS_SOCKET_ERROR);
      }
    }
  s->listenBacklog = backlog;

  /* Discard excess pending connections */
  while ( SWS_MsgqCount( s->hBacklogQ) > (size_t)backlog)
    {
    SWS_SOCKET sTemp;
    VERIFY( !SWS_MsgqGet( s->hBacklogQ, sizeof( sTemp), &sTemp));
    ASSERT( SOCKET_MAGIC == sTemp->uMagic);

    /* Reset the connection */
    (*sTemp->pTransport->pfnClose)( sTemp->data, 1);
    SocketCloseAck( sTemp);
    }

  /* Start the transport listening for connections */
  s->eState = kStateListening;
  ASSERT( s->pTransport->pfnListen);
  err = (*s->pTransport->pfnListen)( s->data);
  if ( err)
    {
    s->eState = kStateBound;
    SetLastError( s->error = err);
    API_RETURN( SWS_SOCKET_ERROR);
    }

  SetLastError( 0);
  API_RETURN( 0);
  }


/*
 * Accept an incoming call
 */
SWS_EXPORT( SWS_SOCKET, SWS_accept(
  SWS_SOCKET s,
  SWS_PTR( struct SWS_sockaddr) pName,  /* OUT */
  SWS_PTR( SWS_socklen_t) pNamelen      /* I/O */
)){
  i32 err;
  SWS_SOCKET sAccept;

  TRACE( 9, ("SWS_accept(%p,%p,%p)\n", s,pName,pNamelen));
  ASSERT( s_inited);
  ASSERT( IsValidSocket( s));
  ASSERT( NULL != s && NULL != s->pTransport);

  if ( !s_inited)
    {
    SetLastError( SWS_EACCES);
    return SWS_INVALID_SOCKET;
    }

  if ( !IsValidSocket( s) )
    {
    SetLastError( SWS_ENOTSOCK);
    API_RETURNS( SWS_INVALID_SOCKET);
    }

  if ( NULL != pNamelen && *pNamelen <= 0)
    {
    SetLastError( s->error = SWS_EINVAL);
    API_RETURNS( SWS_INVALID_SOCKET);
    }

  switch ( s->eState)
    {
  case kStateListening:
    break;

  default:
    TRACE( 1, ("*** SWS_accept() not listening\n"));
    SetLastError( s->error = SWS_EINVAL);
    API_RETURNS( SWS_INVALID_SOCKET);
    }

  /* Wait for incoming connection */
  err = PlatformWaitEvent(
    s->hRxReadyEvent, s->isNonBlocking ? WAIT_NEVER : WAIT_FOREVER);
  if ( 1 != err)
    {
    SetLastError( s->error =
      (err ? (int)err : s->isNonBlocking ? SWS_EWOULDBLOCK : SWS_ETIMEDOUT));
    API_RETURNS( SWS_INVALID_SOCKET);
    }

  /* Get the oldest pending connection */
  VERIFY( !SWS_MsgqGet( s->hBacklogQ, sizeof( sAccept), &sAccept));
  ASSERT( kStateConnected == sAccept->eState);
  ASSERT( AtomicRead( &s->calls));
  AtomicDecrement( &s->calls);

  /* Copy the control data */
  sAccept->linger = s->linger;
  sAccept->sndtime = s->sndtime;
  sAccept->rcvtime = s->rcvtime;
  sAccept->isNonBlocking = s->isNonBlocking;
  /* NB not shutdown */

  if ( SWS_MsgqCount( s->hBacklogQ) > 0)
    PlatformSetEvent( s->hRxReadyEvent, 1);

  if ( NULL != pNamelen && NULL != pName)
    VERIFY( !SWS_getpeername( sAccept, pName, pNamelen));

  SetLastError( 0);
  API_RETURNS( sAccept);
  }


/*
 * Bind a socket to a remote name
 */
SWS_EXPORT( int, SWS_connect(
  SWS_SOCKET s,
  SWS_PTR( const struct SWS_sockaddr) pName,
  SWS_socklen_t namelen
)){
  int err;
  EState state;

  TRACE( 9, ("SWS_connect(%p,%p,%d)\n", s,pName,(int)namelen));
  ASSERT( s_inited);
  ASSERT( IsValidSocket( s));
  ASSERT( NULL != s && NULL != s->pTransport);
  ASSERT( NULL != pName || 0 == namelen);

  if ( !s_inited)
    {
    SetLastError( SWS_EACCES);
    return SWS_SOCKET_ERROR;
    }

  if ( !IsValidSocket( s) )
    {
    SetLastError( SWS_ENOTSOCK);
    API_RETURN( SWS_SOCKET_ERROR);
    }

  switch ( s->eState)
    {
  case kStateOpen:
  case kStateBound:
    break;

  case kStateListening:
    TRACE( 1, ("*** SWS_connect() already listening\n"));
    SetLastError( s->error = SWS_EINVAL);
    API_RETURN( SWS_SOCKET_ERROR);

  case kStateCalling:
    /* This is normal when polling a non-blocking socket for a connect */
    TRACE( 2, ("SWS_connect() already connecting\n"));
    SetLastError( s->error = SWS_EALREADY);
    API_RETURN( SWS_SOCKET_ERROR);

  case kStateConnected:
    if ( SWS_SOCK_STREAM == s->pTransport->uSocketType)
      {
      /* Connection oriented streams (TCP) cannot be re-connected.
       * But this is normal when polling a non-blocking socket for a connect.
       */
      TRACE( 2, ("SWS_connect() already connected\n"));
      SetLastError( s->error = SWS_EISCONN);
      API_RETURN( SWS_SOCKET_ERROR);
      }
    /* Connectionless (UDP or raw) sockets can be re-connected */
    break;

  default:
    TRACE( 1, ("*** SWS_connect() not permitted\n"));
    SetLastError( s->error = SWS_EACCES);
    API_RETURN( SWS_SOCKET_ERROR);
    }

  state = s->eState;
  s->eState = kStateCalling;

  /* Call the transport's connect */
  ASSERT( s->pTransport->pfnConnect);
  err = (*s->pTransport->pfnConnect)( s->data, pName, namelen);
  if ( 0 == err)
    {
    s->eState = kStateConnected;
    }
  else if ( -1 != err)
    {
    SetLastError( s->error = err);
    s->eState = state;
    API_RETURN( SWS_SOCKET_ERROR);
    }
  else
    {
    /* Wait for async connect */
    err = (int)PlatformWaitEvent(
      s->hTxReadyEvent, s->isNonBlocking ? WAIT_NEVER : WAIT_FOREVER);
    if ( 1 != err)
      {
      /* Not connected */
      if ( 0 == err)
        err = SWS_EWOULDBLOCK;
      else
        s->eState = state;

      SetLastError( s->error = err);
      API_RETURN( SWS_SOCKET_ERROR);
      }

    /* Mark socket ready to send */
    PlatformSetEvent( s->hTxReadyEvent, 1);
    }

  SetLastError( 0);
  API_RETURN( 0);
  }


/*
 * Receive data
 */
SWS_EXPORT( int, SWS_recv(
  SWS_SOCKET s,
  SWS_PTR( void) pBuf,
  unsigned len,
  unsigned flags
)){
  int iRet;

  TRACE( 9, ("SWS_recv(%p,%p,%u,%u)\n", s,pBuf,len,flags));
  ASSERT( s_inited);
  ASSERT( IsValidSocket( s));
  ASSERT( NULL != s && NULL != s->pTransport);
  ASSERT( NULL != pBuf || 0 == len);

  if ( !s_inited)
    {
    SetLastError( SWS_EACCES);
    return SWS_SOCKET_ERROR;
    }

  if ( !IsValidSocket( s) )
    {
    SetLastError( SWS_ENOTSOCK);
    API_RETURN( SWS_SOCKET_ERROR);
    }

  if ( NULL == pBuf && 0 != len)
    {
    SetLastError( s->error = SWS_EFAULT);
    API_RETURN( SWS_SOCKET_ERROR);
    }

  switch ( s->eState)
    {
  case kStateOpen:
    TRACE( 1, ("*** SWS_recv() not bound\n"));
    SetLastError( s->error = SWS_EINVAL);
    API_RETURN( SWS_SOCKET_ERROR);

  case kStateConnected:
  case kStateBound:
    break;

  default:
    TRACE( 1, ("*** SWS_recv() not permitted\n"));
    SetLastError( s->error = SWS_ENOTCONN);
    API_RETURN( SWS_SOCKET_ERROR);
    }

  if ( s->shutdown & SWS_SD_RECEIVE)
    {
    SetLastError( s->error = SWS_ESHUTDOWN);
    API_RETURN( SWS_SOCKET_ERROR);
    }

  /* Call the transport's recv */
  ASSERT( s->pTransport->pfnRecvFrom);
  iRet = (*s->pTransport->pfnRecvFrom)( s->data, pBuf, &len, flags, NULL, NULL);
  if ( iRet)
    {
    SetLastError( s->error = iRet);
    API_RETURN( SWS_SOCKET_ERROR);
    }

  SetLastError( 0);
  API_RETURN( (int)len);
  }


SWS_EXPORT( int, SWS_recvfrom(
  SWS_SOCKET s,
  SWS_PTR( void) pBuf,
  unsigned len,
  unsigned flags,
  SWS_PTR( struct SWS_sockaddr) pFrom,
  SWS_PTR( SWS_socklen_t) pFromlen
)){
  int iRet;

  TRACE( 9, ("SWS_recvfrom(%p,%p,%u,%u,%p,%p)\n",
    s,pBuf,len,flags,pFrom,pFromlen));
  ASSERT( s_inited);
  ASSERT( IsValidSocket( s));
  ASSERT( NULL != s && NULL != s->pTransport);
  ASSERT( NULL != pBuf || 0 == len);

  if ( !s_inited)
    {
    SetLastError( SWS_EACCES);
    return SWS_SOCKET_ERROR;
    }

  if ( !IsValidSocket( s) )
    {
    SetLastError( SWS_ENOTSOCK);
    API_RETURN( SWS_SOCKET_ERROR);
    }

  if ( len > 0 && NULL == pBuf)
    {
    SetLastError( s->error = SWS_EFAULT);
    API_RETURN( SWS_SOCKET_ERROR);
    }

  if ( NULL != pFrom)
    {
    if ( NULL == pFromlen)
      {
      TRACE( 1, ("*** SWS_recvfrom() Invalid from length ptr\n"));
      SetLastError( s->error = SWS_EFAULT);
      API_RETURN( SWS_SOCKET_ERROR);
      }

    if ( *pFromlen <= 0)
      {
      TRACE( 1, ("*** SWS_recvfrom() Invalid from length\n"));
      SetLastError( s->error = SWS_EINVAL);
      API_RETURN( SWS_SOCKET_ERROR);
      }
    }

  switch ( s->eState)
    {
  case kStateOpen:
    if ( NULL == pFrom)
      {
      TRACE( 1, ("*** SWS_recvfrom() not bound\n"));
      SetLastError( s->error = SWS_EFAULT);
      API_RETURN( SWS_SOCKET_ERROR);
      }
    break;

  case kStateConnected:
  case kStateBound:
    break;

  default:
    TRACE( 1, ("*** SWS_recvfrom() not permitted\n"));
    SetLastError( s->error = SWS_ENOTCONN);
    API_RETURN( SWS_SOCKET_ERROR);
    }

  if ( s->shutdown & SWS_SD_RECEIVE)
    {
    SetLastError( s->error = SWS_ESHUTDOWN);
    API_RETURN( SWS_SOCKET_ERROR);
    }

  /* Call the transport's recv */
  ASSERT( s->pTransport->pfnRecvFrom);
  iRet = (*s->pTransport->pfnRecvFrom)(
    s->data, pBuf, &len, flags, pFrom, pFromlen);
  if ( iRet)
    {
    SetLastError( s->error = iRet);
    API_RETURN( SWS_SOCKET_ERROR);
    }

  SetLastError( 0);
  API_RETURN( (int)len);
  }


/*
 * Send data
 */
SWS_EXPORT( int, SWS_send(
  SWS_SOCKET s,
  SWS_PTR( const void) pBuf,
  unsigned len,
  unsigned flags
)){
  int iRet;

  TRACE( 9, ("SWS_send(%p,%p,%u,%u)\n", s,pBuf,len,flags));
  ASSERT( s_inited);
  ASSERT( IsValidSocket( s));
  ASSERT( NULL != s && NULL != s->pTransport);
  ASSERT( NULL != pBuf || 0 == len);

  if ( !s_inited)
    {
    SetLastError( SWS_EACCES);
    return SWS_SOCKET_ERROR;
    }

  if ( !IsValidSocket( s) )
    {
    SetLastError( SWS_ENOTSOCK);
    API_RETURN( SWS_SOCKET_ERROR);
    }

  if ( len > 0 && NULL == pBuf)
    {
    SetLastError( s->error = SWS_EFAULT);
    API_RETURN( SWS_SOCKET_ERROR);
    }

  switch ( s->eState)
    {
  case kStateConnected:
    break;

  case kStateOpen:
    TRACE( 1, ("*** SWS_send() not bound\n"));
    SetLastError( s->error = SWS_EINVAL);
    API_RETURN( SWS_SOCKET_ERROR);

  default:
    TRACE( 1, ("*** SWS_send() not permitted\n"));
    SetLastError( s->error = SWS_ENOTCONN);
    API_RETURN( SWS_SOCKET_ERROR);
    }

  if ( s->shutdown & SWS_SD_SEND)
    {
    SetLastError( s->error = SWS_ESHUTDOWN);
    API_RETURN( SWS_SOCKET_ERROR);
    }

  /* Call the transport's send */
  ASSERT( s->pTransport->pfnSendTo);
  iRet = (*s->pTransport->pfnSendTo)( s->data, pBuf, &len, flags, NULL, 0);
  if ( iRet)
    {
    /* EWOULDBLOCK is not an error if some data has been sent */
    if ( SWS_EWOULDBLOCK != iRet || 0 == len)
      {
      SetLastError( s->error = iRet);
      API_RETURN( SWS_SOCKET_ERROR);
      }
    }

  SetLastError( 0);
  API_RETURN( (int)len);
  }


SWS_EXPORT( int, SWS_sendto(
  SWS_SOCKET s,
  SWS_PTR( const void) pBuf,
  unsigned len,
  unsigned flags,
  SWS_PTR( const struct SWS_sockaddr) to,
  SWS_socklen_t tolen
)){
  int iRet;

  TRACE( 9, ("SWS_sendto(%p,%p,%u,%u,%p,%d)\n",
    s,pBuf,len,flags,to,(int)tolen));
  ASSERT( s_inited);
  ASSERT( IsValidSocket( s));
  ASSERT( NULL != s && NULL != s->pTransport);
  ASSERT( NULL != pBuf || 0 == len);

  if ( !s_inited)
    {
    SetLastError( SWS_EACCES);
    return SWS_SOCKET_ERROR;
    }

  if ( !IsValidSocket( s) )
    {
    SetLastError( SWS_ENOTSOCK);
    API_RETURN( SWS_SOCKET_ERROR);
    }

  if ( len > 0 && NULL == pBuf)
    {
    SetLastError( s->error = SWS_EFAULT);
    API_RETURN( SWS_SOCKET_ERROR);
    }

  if ( tolen <= 0)
    {
    TRACE( 1, ("*** SWS_sendto() Invalid to length\n"));
    SetLastError( s->error = SWS_EINVAL);
    API_RETURN( SWS_SOCKET_ERROR);
    }

  switch ( s->eState)
    {
  case kStateOpen:
    if ( NULL == to)
      {
      TRACE( 1, ("*** SWS_sendto() not connected\n"));
      SetLastError( s->error = SWS_EFAULT);
      API_RETURN( SWS_SOCKET_ERROR);
      }
    break;

  case kStateConnected:
  case kStateBound:
    break;

  default:
    TRACE( 1, ("*** SWS_sendto() not permitted\n"));
    SetLastError( s->error = SWS_ENOTCONN);
    API_RETURN( SWS_SOCKET_ERROR);
    }

  if ( s->shutdown & SWS_SD_SEND)
    {
    SetLastError( s->error = SWS_ESHUTDOWN);
    API_RETURN( SWS_SOCKET_ERROR);
    }

  /* Call the transport's send */
  ASSERT( s->pTransport->pfnSendTo);
  iRet = (*s->pTransport->pfnSendTo)( s->data, pBuf, &len, flags, to, tolen);
  if ( iRet)
    {
    /* EWOULDBLOCK is not an error if some data has been sent */
    if ( SWS_EWOULDBLOCK != iRet || 0 == len)
      {
      SetLastError( s->error = iRet);
      API_RETURN( SWS_SOCKET_ERROR);
      }
    }

  /* Connectionless sockets are now bound */
  if ( kStateOpen == s->eState && NULL != to)
    s->eState = kStateBound;

  SetLastError( 0);
  API_RETURN( (int)len);
  }


/*
 * Get the address of the local host
 */
SWS_EXPORT( int, SWS_getsockname(
  SWS_SOCKET s,
  SWS_PTR( struct SWS_sockaddr) pName,
  SWS_PTR( SWS_socklen_t) pLen
)){
  int iRet;

  TRACE( 9, ("SWS_getsockname(%p,%p,%p)\n",s,pName,pLen));
  ASSERT( s_inited);
  ASSERT( IsValidSocket( s));
  ASSERT( NULL != s && NULL != s->pTransport);
  ASSERT( NULL != pName);
  ASSERT( NULL != pLen);

  if ( !s_inited)
    {
    SetLastError( SWS_EACCES);
    return SWS_SOCKET_ERROR;
    }

  if ( !IsValidSocket( s) )
    {
    SetLastError( SWS_ENOTSOCK);
    API_RETURN( SWS_SOCKET_ERROR);
    }

  switch ( s->eState)
    {
  case kStateOpen:
  case kStateBound:
  case kStateListening:
  case kStateCalling:
  case kStateConnected:
    break;

  default:
    TRACE( 1, ("*** SWS_getsockname() not permitted\n"));
    SetLastError( s->error = SWS_EINVAL);
    API_RETURN( SWS_SOCKET_ERROR);
    }

  ASSERT( s->pTransport->pfnGetSockName);
  iRet = (*s->pTransport->pfnGetSockName)( s->data, pName, pLen);
  if ( iRet)
    {
    SetLastError( s->error = iRet);
    API_RETURN( SWS_SOCKET_ERROR);
    }

  SetLastError( 0);
  API_RETURN( 0);
  }


/*
 * Get the address of the remote host
 */
SWS_EXPORT( int, SWS_getpeername(
  SWS_SOCKET s,
  SWS_PTR( struct SWS_sockaddr) pName,
  SWS_PTR( SWS_socklen_t) pLen
)){
  int iRet;

  TRACE( 9, ("SWS_getpeername(%p,%p,%p)\n",s,pName,pLen));
  ASSERT( s_inited);
  ASSERT( IsValidSocket( s));
  ASSERT( NULL != s && NULL != s->pTransport);
  ASSERT( NULL != pName);
  ASSERT( NULL != pLen);

  if ( !s_inited)
    {
    SetLastError( SWS_EACCES);
    return SWS_SOCKET_ERROR;
    }

  if ( !IsValidSocket( s) )
    {
    SetLastError( SWS_ENOTSOCK);
    API_RETURN( SWS_SOCKET_ERROR);
    }

  switch ( s->eState)
    {
  case kStateConnected:
    break;

  default:
    TRACE( 1, ("*** SWS_getpeername() not connected\n"));
    SetLastError( s->error = SWS_ENOTCONN);
    API_RETURN( SWS_SOCKET_ERROR);
    }

  ASSERT( s->pTransport->pfnGetPeerName);
  iRet = (*s->pTransport->pfnGetPeerName)( s->data, pName, pLen);
  if ( iRet)
    {
    SetLastError( s->error = iRet);
    API_RETURN( SWS_SOCKET_ERROR);
    }

  SetLastError( 0);
  API_RETURN( 0);
  }


/*
 * Get socket options
 */
SWS_EXPORT( int, SWS_getsockopt(
  SWS_SOCKET s,
  unsigned level,
  unsigned opt,
  SWS_PTR( void) pVal,
  SWS_PTR( int) pLen
)){
  int err;

  TRACE( 9, ("SWS_getsockopt(%p,%u,%u,%p,%p)\n",
    s,level,opt,pVal,pLen));
  ASSERT( s_inited);
  ASSERT( IsValidSocket( s));
  ASSERT( NULL != s && NULL != s->pTransport);
  ASSERT( NULL != pVal);
  ASSERT( NULL != pLen);

  if ( !s_inited)
    {
    SetLastError( SWS_EACCES);
    return SWS_SOCKET_ERROR;
    }

  if ( !IsValidSocket( s) )
    {
    SetLastError( SWS_ENOTSOCK);
    API_RETURN( SWS_SOCKET_ERROR);
    }

  if ( NULL == pVal)
    {
    TRACE( 1, ("***SWS_getsockopt() Invalid output buffer\n"));
    SetLastError( s->error = SWS_EFAULT);
    API_RETURN( SWS_SOCKET_ERROR);
    }
  if ( NULL == pLen || *pLen < 0)
    {
    TRACE( 1, ("***SWS_getsockopt() Invalid buffer length\n"));
    SetLastError( s->error = SWS_EFAULT);
    API_RETURN( SWS_SOCKET_ERROR);
    }

  switch ( level)
    {
  case SWS_SOL_SOCKET:
    switch ( opt)
      {
    case SWS_SO_DEBUG:
      if ( *pLen != sizeof(int))
        {
        SetLastError( s->error = SWS_EFAULT);
        API_RETURN( SWS_SOCKET_ERROR);
        }
#if DEBUG_TRACE_ENABLE
      if ( DEBUG_TRACE_LEVEL < 3)
        *(SWS_PTR(int))pVal = 0;
      else
        *(SWS_PTR(int))pVal = 1;
#else
      *(SWS_PTR(int))pVal = 0;
#endif
      SetLastError( 0);
      API_RETURN( 0);

    case SWS_SO_ACCEPTCONN:
      if ( *pLen != sizeof(int))
        {
        SetLastError( s->error = SWS_EFAULT);
        API_RETURN( SWS_SOCKET_ERROR);
        }

      *(SWS_PTR(int))pVal = kStateListening == s->eState;
      break;

    case SWS_SO_ERROR:
      if ( *pLen != sizeof(int))
        {
        SetLastError( s->error = SWS_EFAULT);
        API_RETURN( SWS_SOCKET_ERROR);
        }

      *(SWS_PTR(int))pVal = s->error;
      s->error = 0;
      break;

    case SWS_SO_LINGER:
      if ( *pLen != sizeof( struct SWS_linger))
        {
        SetLastError( s->error = SWS_EFAULT);
        API_RETURN( SWS_SOCKET_ERROR);
        }
      *(SWS_PTR(struct SWS_linger))pVal = s->linger;
      SetLastError( 0);
      API_RETURN( 0);

    case SWS_SO_DONTLINGER:
      if ( *pLen != sizeof(int))
        {
        SetLastError( s->error = SWS_EFAULT);
        API_RETURN( SWS_SOCKET_ERROR);
        }
      *(SWS_PTR(int))pVal = 0 == s->linger.l_onoff;
      SetLastError( 0);
      API_RETURN( 0);

    case SWS_SO_SNDTIMEO:
      if ( *pLen != sizeof(struct SWS_timeval))
        {
        SetLastError( s->error = SWS_EFAULT);
        API_RETURN( SWS_SOCKET_ERROR);
        }
      TICKS2TVAL( (SWS_PTR(struct SWS_timeval))pVal, s->sndtime);
      SetLastError( 0);
      API_RETURN( 0);

    case SWS_SO_RCVTIMEO:
      if ( *pLen != sizeof(struct SWS_timeval))
        {
        SetLastError( s->error = SWS_EFAULT);
        API_RETURN( SWS_SOCKET_ERROR);
        }
      TICKS2TVAL( (SWS_PTR(struct SWS_timeval))pVal, s->rcvtime);
      SetLastError( 0);
      API_RETURN( 0);

    case SWS_SO_TYPE:
      if ( *pLen != sizeof(int))
        {
        SetLastError( s->error = SWS_EFAULT);
        API_RETURN( SWS_SOCKET_ERROR);
        }
      *(SWS_PTR(int))pVal = s->pTransport->uSocketType;
      SetLastError( 0);
      API_RETURN( 0);

    /*case SWS_SO_SNDBUF:*/
    /*case SWS_SO_RCVBUF:*/
    /*case SWS_SO_SNDLOWAT:*/
    /*case SWS_SO_RCVLOWAT:*/
    default:
      /* Pass down to transport provider */
      break;
      }
    break;

  case SWS_SOL_IP:
  case SWS_SOL_TCP:
  case SWS_SOL_UDP:
    if ( SWS_AF_INET != s->pTransport->uAddressFamily)
      {
      SetLastError( s->error = SWS_EINVAL);
      API_RETURN( SWS_SOCKET_ERROR);
      }
    break;

  default:
    TRACE( 1, ("***SWS_getsockopt() Invalid level\n"));
    SetLastError( s->error = SWS_EINVAL);
    API_RETURN( SWS_SOCKET_ERROR);
    }

  /* Call the transport's get option */
  ASSERT( s->pTransport->pfnGetInfo);
  err = (*s->pTransport->pfnGetInfo)( s->data, level, opt, pVal, pLen);
  if ( err)
    {
    SetLastError( s->error = err);
    API_RETURN( SWS_SOCKET_ERROR);
    }

  SetLastError( 0);
  API_RETURN( 0);
  }


/*
 * Set socket options
 */
SWS_EXPORT( int, SWS_setsockopt(
  SWS_SOCKET s,
  unsigned level,
  unsigned opt,
  SWS_PTR( const void) pVal,
  int len
)){
  int err;

  TRACE( 9, ("SWS_setsockopt(%p,%u,%u,%p,%d)\n",
    s,level,opt,pVal,len));
  ASSERT( s_inited);
  ASSERT( IsValidSocket( s));
  ASSERT( NULL != s && NULL != s->pTransport);
  ASSERT( NULL != pVal);
  ASSERT( len > 0);

  if ( !s_inited)
    {
    SetLastError( SWS_EACCES);
    return SWS_SOCKET_ERROR;
    }

  if ( !IsValidSocket( s) )
    {
    SetLastError( SWS_ENOTSOCK);
    API_RETURN( SWS_SOCKET_ERROR);
    }

  if ( NULL == pVal || len <= 0)
    {
    TRACE( 1, ("*** SWS_setsockopt() Invalid input buffer\n"));
    SetLastError( s->error = SWS_EFAULT);
    API_RETURN( SWS_SOCKET_ERROR);
    }

  switch ( level)
    {
  case SWS_SOL_SOCKET:
    switch ( opt)
      {
    case SWS_SO_DEBUG:
      if ( len != sizeof(int))
        {
        SetLastError( s->error = SWS_EFAULT);
        API_RETURN( SWS_SOCKET_ERROR);
        }
#if DEBUG_TRACE_ENABLE
      if ( *(SWS_PTR(const int))pVal)
        {
        if ( DEBUG_TRACE_LEVEL < 3)
          DEBUG_SET_TRACE_LEVEL( 3);
        }
      else if ( DEBUG_TRACE_LEVEL > 1)
        DEBUG_SET_TRACE_LEVEL( 1);
#endif
      SetLastError( 0);
      API_RETURN( 0);

    case SWS_SO_LINGER:
      if ( len != sizeof( struct SWS_linger))
        {
        SetLastError( s->error = SWS_EFAULT);
        API_RETURN( SWS_SOCKET_ERROR);
        }
      s->linger = *(SWS_PTR(const struct SWS_linger))pVal;
      SetLastError( 0);
      API_RETURN( 0);

    case SWS_SO_DONTLINGER:
      if ( len != sizeof(int))
        {
        SetLastError( s->error = SWS_EFAULT);
        API_RETURN( SWS_SOCKET_ERROR);
        }
      s->linger.l_onoff = (SWS_u_short)(*(SWS_PTR(const int))pVal == 0);
      SetLastError( 0);
      API_RETURN( 0);

    case SWS_SO_SNDTIMEO:
      if ( len != sizeof(struct SWS_timeval))
        {
        SetLastError( s->error = SWS_EFAULT);
        API_RETURN( SWS_SOCKET_ERROR);
        }
      s->sndtime = TVAL2TICKS( (SWS_PTR(const struct SWS_timeval))pVal);
      SetLastError( 0);
      API_RETURN( 0);

    case SWS_SO_RCVTIMEO:
      if ( len != sizeof(struct SWS_timeval))
        {
        SetLastError( s->error = SWS_EFAULT);
        API_RETURN( SWS_SOCKET_ERROR);
        }
      s->rcvtime = TVAL2TICKS( (SWS_PTR(const struct SWS_timeval))pVal);
      SetLastError( 0);
      API_RETURN( 0);

    case SWS_SO_ACCEPTCONN:
    case SWS_SO_ERROR:
    case SWS_SO_TYPE:
      SetLastError( s->error = SWS_EOPNOTSUPP);
      API_RETURN( SWS_SOCKET_ERROR);

    /*case SWS_SO_SNDBUF:*/
    /*case SWS_SO_RCVBUF:*/
    /*case SWS_SO_SNDLOWAT:*/
    /*case SWS_SO_RCVLOWAT:*/
    default:
      break;
      }
    break;

  case SWS_SOL_IP:
  case SWS_SOL_TCP:
  case SWS_SOL_UDP:
    if ( SWS_AF_INET != s->pTransport->uAddressFamily)
      {
      SetLastError( s->error = SWS_EINVAL);
      API_RETURN( SWS_SOCKET_ERROR);
      }
    break;

  default:
    TRACE( 1, ("***SWS_setsockopt() Invalid level\n"));
    SetLastError( s->error = SWS_EINVAL);
    API_RETURN( SWS_SOCKET_ERROR);
    }

  /* Call the transport's set option */
  ASSERT( s->pTransport->pfnSetInfo);
  err = (*s->pTransport->pfnSetInfo)( s->data, level, opt, pVal, len);
  if ( err)
    {
    SetLastError( s->error = err);
    API_RETURN( SWS_SOCKET_ERROR);
    }

  SetLastError( 0);
  API_RETURN( 0);
  }


/*
 * Special features
 */
SWS_EXPORT( int, SWS_ioctl(
  SWS_SOCKET s,
  SWS_u_long cmd,
  SWS_PTR( SWS_u_long) pArgs
)){
  int err;
  int len;

  TRACE( 9, ("SWS_ioctl(%p,%lu,%p)\n", s,cmd,pArgs));
  ASSERT( s_inited);
  ASSERT( IsValidSocket( s));
  ASSERT( NULL != s && NULL != s->pTransport);

  if ( !s_inited)
    {
    SetLastError( SWS_EACCES);
    return SWS_SOCKET_ERROR;
    }

  if ( !IsValidSocket( s) )
    {
    SetLastError( SWS_ENOTSOCK);
    API_RETURN( SWS_SOCKET_ERROR);
    }

  switch ( cmd)
    {
  case SWS_FIONBIO: /* set/clear non-blocking i/o */
    s->isNonBlocking = (int)*(SWS_PTR(SWS_u_long))pArgs;
    err = 0;
    break;

  case SWS_FIONREAD: /* get # bytes to read */
    len = sizeof(SWS_u_long);
    ASSERT( s->pTransport->pfnGetInfo);
    err = (*s->pTransport->pfnGetInfo)(
      s->data, SWS_SOL_IOCTL, SWS_IOCTL_FIONREAD, pArgs, &len);
    break;

  default:
    SetLastError( s->error = SWS_EINVAL);
    API_RETURN( SWS_SOCKET_ERROR);
    }

  if ( err)
    {
    SetLastError( s->error = err);
    API_RETURN( SWS_SOCKET_ERROR);
    }

  SetLastError( 0);
  API_RETURN( 0);
  }


/*
 * Wait for socket event
 */
SWS_EXPORT( int, SWS_select(
  SWS_PTR( SWS_fd_set) pRead,
  SWS_PTR( SWS_fd_set) pWrite,
  SWS_PTR( SWS_fd_set) pExcept,
  SWS_PTR( const struct SWS_timeval) pTimeout
)){
  ui32 ticks, uTimeout;
  int iRet, isFirst;
  SWS_u_int read, write, except;

  TRACE( 9, ("SWS_select(%p,%p,%p,%p)\n", pRead, pWrite, pExcept, pTimeout));
  ASSERT( s_inited);

  if ( !s_inited)
    {
    SetLastError( SWS_EACCES);
    return SWS_SOCKET_ERROR;
    }

  if ( NULL != pTimeout)
    {
    /* Set a timeout */
    if ( pTimeout->tv_sec >= 0
      && (SWS_u_long)pTimeout->tv_sec < ULONG_MAX / SWS_TIMER_TICKS
    ) {
      ticks = (pTimeout->tv_usec * SWS_TIMER_TICKS) / 1000000UL
        + pTimeout->tv_sec * SWS_TIMER_TICKS;
      if ( 0 == ticks)
        ticks = 1;

      uTimeout = ticks + PlatformTime();
      }
    else
      {
      TRACE( 1, ("*** SWS_select() Timeout too big\n"));
      SetLastError( SWS_EINVAL);
      API_RETURN( SWS_SOCKET_ERROR);
      }
    }
  else
    {
    /* Infinite */
    uTimeout = ticks = WAIT_FOREVER;
    }

  iRet = 0;
  read = write = except = 0;

  isFirst = 1;
  do
    {
    SWS_u_int i;
    SWS_SOCKET s;

    if ( isFirst)
      {
      isFirst = 0;
      PlatformWaitAnyEvent( WAIT_NEVER);
      }
    else
      {
      PlatformWaitAnyEvent( ticks);
      }

    /* Any read events? (RxData, listen ready, or closed/reset/terminated) */
    if ( NULL != pRead)
      {
      ASSERT( pRead->fd_count <= pRead->fd_limit);

      for ( i = 0; i < pRead->fd_count; ++i)
        {
        s = pRead->fd_array[ i];
        ASSERT( NULL != s);
        ASSERT( SOCKET_MAGIC == s->uMagic);

        if ( s->shutdown & SWS_SD_RECEIVE)
          continue;

        if ( PlatformGetEvent( s->hRxReadyEvent) )
          pRead->fd_array[ read++] = s;
        }

      iRet += read;
      }

    /* Any write events? (TxReady or connect complete) */
    if ( NULL != pWrite)
      {
      ASSERT( pWrite->fd_count <= pWrite->fd_limit);

      for ( i = 0; i < pWrite->fd_count; ++i)
        {
        s = pWrite->fd_array[ i];
        ASSERT( NULL != s);
        ASSERT( SOCKET_MAGIC == s->uMagic);

        if ( s->shutdown & SWS_SD_SEND)
          continue;

        if ( 1 == PlatformGetEvent( s->hTxReadyEvent) )
          pWrite->fd_array[ write++] = s;
        }

      iRet += write;
      }

    /* Connect failed or OOB data? */
    if ( NULL != pExcept)
      {
      ASSERT( pExcept->fd_count <= pExcept->fd_limit);

      for ( i = 0; i < pExcept->fd_count; ++i)
        {
        s = pExcept->fd_array[ i];
        ASSERT( NULL != s);
        ASSERT( SOCKET_MAGIC == s->uMagic);

        if ( PlatformGetEvent( s->hTxReadyEvent) > 1
          || PlatformGetEvent( s->hRxOobEvent) == 1
        ) {
          pExcept->fd_array[ except++] = s;
          }
        }

      iRet += except;
      }
    }
  while ( !iRet && (NULL == pTimeout || !SWS_TimerLE( uTimeout, PlatformTime())) );

  if ( NULL != pRead)
    pRead->fd_count = read;
  if ( NULL != pWrite)
    pWrite->fd_count = write;
  if ( NULL != pExcept)
    pExcept->fd_count = except;

  return iRet;
  }


/*
 * Select socket array manipulators
 */
SWS_EXPORT( int, SWS_FdIsSet( SWS_SOCKET s, SWS_PTR( SWS_fd_set) set ))
  {
  SWS_u_int i;

  ASSERT( NULL == s || SOCKET_MAGIC == s->uMagic);
  ASSERT( NULL != set);
  ASSERT( set->fd_count <= set->fd_limit);

  if ( NULL != set && IsValidSocket( s))
    {
    for ( i = 0; i < set->fd_count; ++i)
      {
      if ( set->fd_array[ i] == s)
        return 1;
      }
    }
  return 0;
  }

SWS_EXPORT( void, SWS_FdClr( SWS_SOCKET s, SWS_PTR( SWS_fd_set) set))
  {
  SWS_u_int i;

  ASSERT( NULL == s || SOCKET_MAGIC == s->uMagic);
  ASSERT( NULL != set);
  ASSERT( set->fd_count <= set->fd_limit);

  if ( NULL != set && IsValidSocket( s))
    {
    for ( i = 0; i < set->fd_count; ++i)
      {
      if ( set->fd_array[ i] == s)
        {
        while ( i < set->fd_count - 1)
          {
          set->fd_array[ i] = set->fd_array[ i + 1];
          ++i;
          }

        set->fd_count--;
        break;
        }
      }
    }
  }

SWS_EXPORT( void, SWS_FdSet( SWS_SOCKET s, SWS_PTR( SWS_fd_set) set ))
  {
  SWS_u_int i;

  ASSERT( NULL == s || SOCKET_MAGIC == s->uMagic);
  ASSERT( NULL != set);
  ASSERT( set->fd_count <= set->fd_limit);

  if ( NULL != set && IsValidSocket( s))
    {
    for ( i = 0; i < set->fd_count; ++i)
      {
      if ( set->fd_array[ i] == s)
        break;
      }

    if ( i == set->fd_count && i < set->fd_limit)
      {
      set->fd_array[ i] = s;
      set->fd_count++;
      }
    }
  }


/*
 * Shutdown a socket
 */
SWS_EXPORT( int, SWS_shutdown(
  SWS_SOCKET s,
  int how
)){
  int err;

  TRACE( 9, ("SWS_shutdown(%p,%d)\n",s,how));
  ASSERT( s_inited);
  ASSERT( IsValidSocket( s));
  ASSERT( NULL != s && NULL != s->pTransport);

  if ( !s_inited)
    {
    SetLastError( SWS_EACCES);
    return SWS_SOCKET_ERROR;
    }

  if ( !IsValidSocket( s) )
    {
    SetLastError( SWS_ENOTSOCK);
    API_RETURN( SWS_SOCKET_ERROR);
    }

  ASSERT( s->pTransport->pfnShutdown);
  err = (*s->pTransport->pfnShutdown)( s->data, how);
  if ( err)
    {
    SetLastError( s->error = err);
    API_RETURN( SWS_SOCKET_ERROR);
    }

  s->shutdown = how;

  SetLastError( 0);
  API_RETURN( 0);
  }


/*
 * Network <=> host conversions
 */
SWS_EXPORT( SWS_u_long, SWS_htonl( SWS_u_long v))
  {
#if SWS_BIG_ENDIAN
  return v;
#else
  return SWAP32(v);
#endif
  }

SWS_EXPORT( SWS_u_short, SWS_htons( SWS_u_short v))
  {
#if SWS_BIG_ENDIAN
  return v;
#else
  return SWAP16(v);
#endif
  }

SWS_EXPORT( SWS_u_long, SWS_ntohl( SWS_u_long v))
  {
#if SWS_BIG_ENDIAN
  return v;
#else
  return SWAP32(v);
#endif
  }

SWS_EXPORT( SWS_u_short, SWS_ntohs( SWS_u_short v))
  {
#if SWS_BIG_ENDIAN
  return v;
#else
  return SWAP16(v);
#endif
  }


/*
 * Internet address <=> text conversions
 */
SWS_EXPORT( SWS_in_addr_t, SWS_inet_addr(
  SWS_PTR( const char) psz
)){
  int fields, n;
  long v1, v2, v3, v4;
  ui32 v, uAddr;

  TRACE( 9, ("SWS_inet_addr(%s)\n", psz));
  ASSERT( NULL != psz);

  if ( NULL == psz)
    {
    TRACE( 1, ("*** SWS_inet_addr() NULL buffer\n"));
    SetLastError( SWS_EFAULT);
    return SWS_INADDR_NONE;
    }

  n = 0;
  fields = sscanf( psz, "%li%n.%li%n.%li%n.%li%n",
    &v1, &n, &v2, &n, &v3, &n, &v4, &n);

  switch ( fields)
    {
  case 1:
    v = (ui32)v1;
    break;
  case 2: /* Class A */
    if ( v1 > 0xff || v1 < 0
      || v2 > 0xffffffL || v2 < 0
    ) {
      TRACE( 9, ("SWS_inet_addr() out of range\n"));
      return SWS_INADDR_NONE;
      }
    v = (v1 << 24) | v2;
    break;
  case 3: /* Class B */
    if ( v1 > 0xff || v1 < 0
      || v2 > 0xff || v2 < 0
      || v3 > 0xffff || v3 < 0
    ) {
      TRACE( 9, ("SWS_inet_addr() out of range\n"));
      return SWS_INADDR_NONE;
      }
    v = (v1 << 24) | (v2 << 16) | v3;
    break;
  case 4:
    if ( v1 > 0xff || v1 < 0
      || v2 > 0xff || v2 < 0
      || v3 > 0xff || v3 < 0
      || v4 > 0xff || v4 < 0
    ) {
      TRACE( 9, ("SWS_inet_addr() out of range\n"));
      return SWS_INADDR_NONE;
      }
    v = (v1 << 24) | (v2 << 16) | (v3 << 8) | v4;
    break;
  default:
    TRACE( 9, ("SWS_inet_addr() not a number\n"));
    return SWS_INADDR_NONE;
    }

  if ( (size_t)n != strlen( psz))
    {
    TRACE( 9, ("SWS_inet_addr() format\n"));
    return SWS_INADDR_NONE;
    }

  NetWrite32( &uAddr, v);
  TRACE( 9, ("SWS_inet_addr() %08lx\n", uAddr));
  return uAddr;
  }


/*
 * Internet address <=> text conversions
 */
SWS_EXPORT( SWS_PTR( const char), SWS_inet_ntoa(
   struct SWS_in_addr addr            /* IN: IP address in network byte order */
)){
  static char s_szBuffer[16];
  TRACE( 9, ("SWS_inet_ntoa(%08lx)\n", addr.SWS_s_addr));

  sprintf( s_szBuffer, "%u.%u.%u.%u",
    addr.S_un.S_un_b.s_b1,
    addr.S_un.S_un_b.s_b2,
    addr.S_un.S_un_b.s_b3,
    addr.S_un.S_un_b.s_b4
  );

  TRACE( 9, ("SWS_inet_ntoa() %s\n", s_szBuffer));
  return s_szBuffer;
  }


/****************************************************************************
 * DNS support
 ****************************************************************************/

/*
 * Get the primary DNS server
 */
SWS_EXPORT( ui32, SWS_DnsGetPriServer( void))
  {
  TRACE( 9, ("SWS_DnsGetPriServer()\n"));
  return s_ulDnsPriServer;
  }


/*
 * Get the secondary DNS server
 */
SWS_EXPORT( ui32, SWS_DnsGetSecServer( void))
  {
  TRACE( 9, ("SWS_DnsGetSecServer()\n"));
  return s_ulDnsSecServer;
  }


#if OPT_DNS
/*
 * Set the primary/secondary DNS server
 */
SWS_EXPORT( int, SWS_DnsSetPriServer(
  ui32 server                         /* IN: IP address */
)){
  TRACE( 9, ("SWS_DnsSetPriServer(%08lx)\n",server));

  if ( SWS_INADDR_NONE != server)
    {
    s_ulDnsPriServer = server;
    return 0;
    }

  return SWS_EINVAL;
  }

SWS_EXPORT( int, SWS_DnsSetSecServer(
  ui32 server                         /* IN: IP address */
)){
  TRACE( 9, ("SWS_DnsSetSecServer(%08lx)\n",server));

  if ( SWS_INADDR_NONE != server)
    {
    s_ulDnsSecServer = server;
    return 0;
    }

  return SWS_EINVAL;
  }


/*
 * Set the hostname
 */
SWS_EXPORT( int, SWS_DnsSetHostname( /* Returns !0 on error */
  SWS_PTR( const char) pszHost,      /* IN: -> name */
  SWS_PTR( const char) pszDomain     /* IN: -> name */
)){
  TRACE( 9, ("SWS_DnsSetHostname('%s','%s')\n",pszHost,pszDomain));
  ASSERT( NULL != pszHost);

  strncpy( s_szHostname, pszHost, sizeof( s_szHostname) - 1 );
  if ( NULL != pszDomain && *pszDomain)
    {
    strcat( s_szHostname, ".");
    strcat( s_szHostname, pszDomain);
    }

  return 0;
  }
#endif


/*
 * Get the hostname
 */
SWS_EXPORT( int, SWS_gethostname(
  SWS_PTR( char) psz,
  int len
)){
  TRACE( 9, ("SWS_gethostname(%p,%d)\n", psz,len));
  ASSERT( s_inited);

  if ( !s_inited)
    {
    SetLastError( SWS_EACCES);
    return SWS_SOCKET_ERROR;
    }

  if ( NULL == psz)
    {
    TRACE( 1, ("*** SWS_gethostname() NULL buffer\n"));
    SetLastError( SWS_EFAULT);
    API_RETURN( SWS_SOCKET_ERROR);
    }

  if ( len < 1 + (int)strlen( s_szHostname))
    {
    TRACE( 1, ("*** SWS_gethostname() buffer too small\n"));
    SetLastError( SWS_EINVAL);
    API_RETURN( SWS_SOCKET_ERROR);
    }

  strcpy( psz, s_szHostname);
  TRACE( 3, ("SWS_gethostname() '%s'\n", psz));

  SetLastError( 0);
  API_RETURN( 0);
  }


/*
 * DNS address lookup
 */
SWS_EXPORT( SWS_PTR( struct SWS_hostent), SWS_gethostbyaddr(
  SWS_PTR( const char) pName,
  int len,
  int family
)){
  ui32 ipaddress;

  TRACE( 9, ("SWS_gethostbyaddr(%p,%d,%d)\n", pName,len,family));
  ASSERT( s_inited);
  ASSERT( NULL != pName);

  if ( !s_inited)
    {
    SetLastError( SWS_EACCES);
    return NULL;
    }

  if ( NULL == pName)
    {
    TRACE( 1, ("*** SWS_gethostbyaddr() NULL buffer\n"));
    SetLastError( SWS_EFAULT);
    API_RETURNH( NULL);
    }

  if ( SWS_AF_INET != family)
    {
    TRACE( 1, ("*** SWS_gethostbyaddr() unsupported family\n"));
    SetLastError( SWS_EAFNOSUPPORT);
    API_RETURNH( NULL);
    }

  if ( (size_t)len < sizeof( SWS_u_long) )
    {
    TRACE( 1, ("*** SWS_gethostbyaddr() buffer too small\n"));
    SetLastError( SWS_EFAULT);
    API_RETURNH( NULL);
    }

  NetRead32( ipaddress, pName);
  if ( SWS_INADDR_LOOPBACK == ipaddress)
    {
    /* Loopback */
    return SWS_gethostbyname( kszLocalHost);
    }
  else
    {
    int i;
    SWS_PTR( struct SWS_hostent) pHost;

    /* Does it match one of this host's addresses? */
    pHost = SWS_gethostbyname( s_szHostname);
    if ( NULL != pHost)
      for ( i = 0; NULL != pHost->h_addr_list[i]; ++i)
        {
        if ( ipaddress == *(SWS_PTR( ui32)) pHost->h_addr_list[i] )
          {
          /* It's this host */
          SetLastError( 0);
          API_RETURNH( pHost);
          }
        }

#if OPT_DNS
    {
    SWS_DnsQHandle hQuery;
    SWS_SDnsQRecord rec;
    char szHost[ kMaxHost];
    int found;

    /* Is it a private or special address - RFC 1918 & 3330 */
    if ( 0x00000000L == (0xf0000000L & ipaddress) /* 0.0.0.0      - 0.255.255.255 */
#if 0 /* Private addresses can have private names */
      || 0x0a000000L == (0xff000000L & ipaddress) /* 10.0.0.0     - 10.255.255.255 */
      || 0xa9fe0000L == (0xffff0000L & ipaddress) /* 169.254.0.0  - 169.254.255.255 */
      || 0xac100000L == (0xfff00000L & ipaddress) /* 172.16.0.0   - 172.31.255.255 */
      || 0xc0a80000L == (0xffff0000L & ipaddress) /* 192.168.0.0  - 192.168.255.255 */
#endif
      || SWS_IN_MULTICAST( ipaddress)
      || 0xf0000000L == (0xf0000000L & ipaddress) /* Experimental */
    ) {
      SetLastError( SWS_EHOST_NOT_FOUND);
      API_RETURNH( NULL);
      }

    /* Convert IP address to IN-ADDR.ARPA domain for query */
    sprintf( szHost, "%u.%u.%u.%u.IN-ADDR.ARPA",
      (unsigned)(ipaddress & 0xff),
      (unsigned)((ipaddress >> 8) & 0xff),
      (unsigned)((ipaddress >> 16) & 0xff),
      (unsigned)((ipaddress >> 24) & 0xff)
    );

#if 0 && OPT_DATALINK_P2P
    /* Dial on demand if no DNS */
    if ( SWS_INADDR_ANY == SWS_DnsGetPriServer())
      SWS_P2pDialOnDemand( SWS_INADDR_ANY);
#endif

    /* Ask DNS to convert address to host name */
    hQuery = SWS_DnsOpenQuery(
      SWS_DNS_OP_QUERY | SWS_DNS_RD_MASK,
      szHost,
      SWS_DNS_TYPE_PTR,
      SWS_DNS_CLASS_IN,
      SWS_DnsGetPriServer(), /* Preferred server */
      SWS_DNS_SERVER
    );
    if ( NULL == hQuery)
      {
      SetLastError( SWS_ENO_RECOVERY);
      API_RETURNH( NULL);
      }

    found = 0;
    while ( !SWS_DnsQueryRead( hQuery, &rec))
      {
      if ( SWS_kDnsAnswer == rec.eRecordType
        && SWS_DNS_CLASS_IN == rec.rclass
        && SWS_DNS_TYPE_PTR == rec.rtype
      ) {
        /* Get the hostname */
        SWS_DnsQueryName( szHost, sizeof( szHost), hQuery, rec.dataIndex);
        found = 1;
        break;
        }
      }

    if ( !found)
      {
      switch ( SWS_DnsQueryRcode( hQuery) & SWS_DNS_RCODE_MASK)
        {
      case SWS_DNS_RC_GOOD:
        SetLastError( SWS_ENO_DATA);
        break;
      case SWS_DNS_RC_NAMERR:
        SetLastError( SWS_EHOST_NOT_FOUND);
        break;
      default:
        SetLastError( SWS_ETRY_AGAIN);
        break;
        }
      }

    SWS_DnsCloseQuery( hQuery);

    if ( found)
      {
      /* Convert the host name to a hostent */
#if 0 /* This incurs an extra DNS op and may sometimes fail */
      return SWS_gethostbyname( szHost);
#else
      static struct SWS_hostent host;
      static SWS_in_addr_t addr[ 1];
      static SWS_PTR( SWS_in_addr_t) list[ 2] = { addr, NULL };
      static SWS_PTR( char) s_szAliases[ 1];
      static char s_szHost[ kMaxHost];

      host.h_name = strncpy( s_szHost, szHost, sizeof s_szHost);
      addr[ 0] = SWS_htonl( ipaddress);
      host.h_aliases = (SWS_PTR( SWS_PTR( const char)))s_szAliases;
      host.h_addrtype = SWS_AF_INET;
      host.h_length = sizeof( struct SWS_in_addr);
      host.h_addr_list = (SWS_PTR( SWS_PTR( char)))list;
      API_RETURNH( &host);
#endif
      }
    }
#else
    SetLastError( SWS_ENETDOWN);
#endif
    }

  API_RETURNH( NULL);
  }


/*
 * DNS name lookup
 */
SWS_EXPORT( SWS_PTR( struct SWS_hostent), SWS_gethostbyname(
  SWS_PTR( const char) psz
)){
  static struct SWS_hostent host;
  static SWS_in_addr_t addr[ OPT_MAX_HOSTS];
  static SWS_in_addr_t* list[ OPT_MAX_HOSTS];
  static SWS_PTR( char) s_szAliases[ OPT_MAX_ALIASES];
  unsigned indx;
  unsigned link, count;
  ui32 uVal;
  size_t size;

  TRACE( 9, ("SWS_gethostbyname(%s)\n", psz));
  ASSERT( s_inited);
  ASSERT( NULL != psz);

  if ( !s_inited)
    {
    SetLastError( SWS_EACCES);
    return NULL;
    }

  if ( NULL == psz)
    {
    TRACE( 1, ("*** SWS_gethostbyname() NULL ptr\n"));
    SetLastError( SWS_EFAULT);
    API_RETURNH( NULL);
    }

#if OPT_DNS
  for (
    indx = 0;
    indx < lengthof( s_szAliases) && NULL != s_szAliases[ indx];
    ++indx
  ) {
    free( s_szAliases[ indx] );
    s_szAliases[ indx] = NULL;
    }
#endif

  if ( !SWS_stricmp( psz, kszLocalHost))
    {
    /* Loopback */
    host.h_name = kszLocalHost;
    addr[ 0] = SWS_htonl( SWS_INADDR_LOOPBACK);
    list[ 0] = &addr[ 0];
    list[ 1] = NULL;
    }
  else if ( !SWS_stricmp( psz, s_szHostname))
    {
    /* This host */
    host.h_name = s_szHostname;

    /* Enumerate all datalinks */
    count = SWS_DatalinkCount();
    ASSERT( count <= (int)lengthof( addr));
    indx = 0;
    for ( link = 0; link < count; ++link)
      {
      uVal = SWS_INADDR_NONE;
      size = sizeof( uVal);

      /* Get IP address for datalink */
      SWS_DatalinkGetProperty(
        link,
        SWS_kDatalinkProperty_Address,
        &uVal,
        &size
      );
      if ( SWS_INADDR_NONE == uVal)
        continue; /* Invalid */
      if ( SWS_INADDR_LOOPBACK == uVal && indx > 0)
        continue;
      if ( SWS_INADDR_ANY == uVal)
        continue; /* Not yet configured */

      if ( 1 == indx && SWS_ntohl( addr[ 0]) == SWS_INADDR_LOOPBACK)
        indx = 0;

      addr[ indx] = SWS_htonl( uVal);
      list[ indx] = &addr[ indx];
      ++indx;
      }

    if ( 0 == indx)
      API_RETURNH( NULL);
    list[ indx] = NULL;
    }
  /* Does it match an entry in hosts? */
  else if ( SWS_INADDR_NONE != (uVal = PlatformPropertyGetUint(
      "hosts", psz, SWS_INADDR_NONE))
  ) {
    host.h_name = psz;
    addr[ 0] = SWS_htonl( uVal);
    list[ 0] = &addr[ 0];
    list[ 1] = NULL;
    }
#if 0
#define kszGateway "gateway"
  else if ( !SWS_stricmp( psz, kszGateway))
    {
    SWS_PTR( SWS_SRoute) pRoute;

    /* Default gateway */
    host.h_name = kszGateway;
    indx = 0;

    pRoute = SWS_RouteFind( SWS_INADDR_ANY, SWS_INADDR_ANY, SWS_IP_TOS_NORM);
    if ( NULL != pRoute)
      {
      uVal = SWS_RouteNextHop( pRoute);
      if ( SWS_INADDR_ANY != uVal)
        {
        addr[ indx] = SWS_htonl( uVal);
        list[ indx] = &addr[ indx];
        ++indx;
        }
      }

    if ( 0 == indx)
      API_RETURN( NULL);
    list[ indx] = NULL;
    }
#endif
#if 0
#define kszDns "dns"
  else if ( !SWS_stricmp( psz, kszDns))
    {
    /* DNS server */
    host.h_name = kszDns;
    indx = 0;

    uVal = SWS_DnsGetPriServer();
    if ( SWS_INADDR_ANY != uVal)
      {
      addr[ indx] = SWS_htonl( uVal);
      list[ indx] = &addr[ indx];
      ++indx;
      }

    uVal = SWS_DnsGetSecServer();
    if ( SWS_INADDR_ANY != uVal)
      {
      addr[ indx] = SWS_htonl( uVal);
      list[ indx] = &addr[ indx];
      ++indx;
      }

    if ( 0 == indx)
      API_RETURN( NULL);
    list[ indx] = NULL;
    }
#endif
#if OPT_DNS
  else
    {
    SWS_DnsQHandle hQuery;
    SWS_SDnsQRecord rec;
    static char s_szHost[ kMaxHost];
    unsigned alias = 0;

#if OPT_DATALINK_P2P
    /* Dial on demand if no DNS */
    if ( SWS_INADDR_ANY == SWS_DnsGetPriServer())
      SWS_P2pDialOnDemand( SWS_INADDR_ANY);
#endif

    /* Query name server */
    hQuery = SWS_DnsOpenQuery(
      SWS_DNS_OP_QUERY | SWS_DNS_RD_MASK,
      psz,
      SWS_DNS_TYPE_A,
      SWS_DNS_CLASS_IN,
      SWS_DnsGetPriServer(), /* Preferred server */
      SWS_DNS_SERVER
    );
    if ( NULL == hQuery)
      {
      SetLastError( SWS_ENO_RECOVERY);
      API_RETURNH( NULL);
      }

    indx = 0;
    while ( !SWS_DnsQueryRead( hQuery, &rec))
      {
      if ( SWS_kDnsAnswer == rec.eRecordType
        && SWS_DNS_CLASS_IN == rec.rclass
      ) {
        if ( SWS_DNS_TYPE_A == rec.rtype
          && indx < lengthof( list)
        ) {
          /* Get the host name & IP address */
          SWS_DnsQueryName( s_szHost, sizeof( s_szHost), hQuery, rec.nameIndex);
          SWS_DnsQueryData(
            &addr[ indx], sizeof( addr[ indx]), hQuery, rec.dataIndex);

          list[ indx] = &addr[ indx];
          ++indx;
          }
        else if ( SWS_DNS_TYPE_CNAME == rec.rtype
          && alias < lengthof( s_szAliases)
        ) {
          char* pszAlias = (char*)malloc( kMaxHost);
          if ( NULL != pszAlias)
            {
            SWS_DnsQueryName( pszAlias, kMaxHost, hQuery, rec.nameIndex);
            s_szAliases[ alias++] = pszAlias;
            }
          }
        }
      }

    if ( 0 == indx)
      {
      switch ( SWS_DnsQueryRcode( hQuery) & SWS_DNS_RCODE_MASK)
        {
      case SWS_DNS_RC_GOOD:
        SetLastError( SWS_ENO_DATA);
        break;
      case SWS_DNS_RC_NAMERR:
        SetLastError( SWS_EHOST_NOT_FOUND);
        break;
      default:
        SetLastError( SWS_ETRY_AGAIN);
        break;
        }
      }

    SWS_DnsCloseQuery( hQuery);

    if ( 0 == indx)
      API_RETURNH( NULL);

    list[ indx] = NULL;
    host.h_name = s_szHost;
    s_szAliases[ alias] = NULL;
    }
#else
  else
    {
    SetLastError( SWS_ENETDOWN);
    API_RETURN( NULL);
    }
#endif

  host.h_aliases = (SWS_PTR( SWS_PTR( const char)))s_szAliases;
  host.h_addrtype = SWS_AF_INET;
  host.h_length = sizeof( struct SWS_in_addr);
  host.h_addr_list = (SWS_PTR( SWS_PTR( char)))list;

  SetLastError( 0);
  API_RETURNH( &host);
  }


#if OPT_GETXBYY
/*
 * Protocol name lookup
 */
SWS_EXPORT( SWS_PTR( struct SWS_protoent), SWS_getprotobyname(
  SWS_PTR( const char) psz
)){
#if OPT_DNS
  const STransport* pTransport;
  const struct SProtocol* pProtocol;
  int i;
#else
  (void)psz;
#endif

  TRACE( 9, ("SWS_getprotobyname(%s)\n", psz));
  ASSERT( s_inited);
  ASSERT( NULL != psz);

  if ( !s_inited)
    {
    SetLastError( SWS_EACCES);
    return NULL;
    }

#if OPT_DNS
  /* Look in list of known protocols */
  for ( pProtocol = &s_protocols[ i = 0];
    i < (int)lengthof( s_protocols);
    ++i, ++pProtocol
  ) {
    if ( !SWS_stricmp( psz, pProtocol->pszName))
      {
      s_proto.p_name = pProtocol->pszName;
      s_szProtoAliases[ 0] = pProtocol->pszName;
      s_szProtoAliases[ 1] = NULL;
      s_proto.p_aliases = s_szProtoAliases;
      s_proto.p_proto = pProtocol->nProtocol;

      SetLastError( 0);
      API_RETURN( &s_proto);
      }
    }

  /* Look in list of registered transports */
  for ( pTransport = s_transportList;
    NULL != pTransport;
    pTransport = pTransport->pNext
  ) {
    /* Socket type matched */
    if ( !SWS_stricmp( psz, pTransport->pszName))
      {
      s_proto.p_name = pTransport->pszName;
      s_szProtoAliases[ 0] = pTransport->pszName;
      s_szProtoAliases[ 1] = NULL;
      s_proto.p_aliases = s_szProtoAliases;
      s_proto.p_proto = pTransport->uProtocol;

      SetLastError( 0);
      API_RETURN( &s_proto);
      }
    }
#endif

  SetLastError( SWS_EHOST_NOT_FOUND); /* Authoritative */
  API_RETURN( NULL);
  }


SWS_EXPORT( SWS_PTR( struct SWS_protoent), SWS_getprotobynumber( int iProto))
  {
#if OPT_DNS
  const STransport* pTransport;
  const struct SProtocol* pProtocol;
  int i;
#else
  (void)iProto;
#endif

  TRACE( 9, ("SWS_getprotobynumber(%d)\n", iProto));
  ASSERT( s_inited);

  if ( !s_inited)
    {
    SetLastError( SWS_EACCES);
    return NULL;
    }

#if OPT_DNS
  /* Look in list of known protocols */
  for ( pProtocol = &s_protocols[ i = 0];
    i < (int)lengthof( s_protocols);
    ++i, ++pProtocol
  ) {
    if ( iProto == pProtocol->nProtocol)
      {
      s_proto.p_name = pProtocol->pszName;
      s_szProtoAliases[ 0] = pProtocol->pszName;
      s_szProtoAliases[ 1] = NULL;
      s_proto.p_aliases = s_szProtoAliases;
      s_proto.p_proto = pProtocol->nProtocol;

      SetLastError( 0);
      API_RETURN( &s_proto);
      }
    }

  /* Look in list of registered transports */
  for ( pTransport = s_transportList;
    NULL != pTransport;
    pTransport = pTransport->pNext
  ) {
    /* Socket type matched */
    if ( iProto == (int)pTransport->uProtocol)
      {
      s_proto.p_name = pTransport->pszName;
      s_szProtoAliases[ 0] = pTransport->pszName;
      s_szProtoAliases[ 1] = NULL;
      s_proto.p_aliases = s_szProtoAliases;
      s_proto.p_proto = pTransport->uProtocol;

      SetLastError( 0);
      API_RETURN( &s_proto);
      }
    }
#endif

  SetLastError( SWS_EHOST_NOT_FOUND); /* Authoritative */
  API_RETURN( NULL);
  }


/*
 * Service name lookup
 */
SWS_EXPORT( SWS_PTR( struct SWS_servent), SWS_getservbyname(
  SWS_PTR( const char) pszService,
  SWS_PTR( const char) pszProto
)){
#if OPT_DNS
  const struct SService* pService;
  int i;
#else
  (void)pszService;
  (void)pszProto;
#endif

  TRACE( 1, ("*** SWS_getservbyname(%s,%p)\n", pszService,pszProto));
  ASSERT( s_inited);
  ASSERT( NULL != pszService);

  if ( !s_inited)
    {
    SetLastError( SWS_EACCES);
    return NULL;
    }

#if OPT_DNS
  /* Look in list of known services */
  for ( pService = &s_services[ i = 0];
    i < (int)lengthof( s_services);
    ++i, ++pService
  ) {
    if ( !SWS_stricmp( pszService, pService->pszName)
      && (NULL == pszProto || !SWS_stricmp( pszProto, pService->pszProtocol))
    ) {
      s_serv.s_name = pService->pszName;
      s_szServAliases[ 0] = pService->pszName;
      s_szServAliases[ 1] = NULL;
      s_serv.s_aliases = s_szServAliases;
      s_serv.s_port = (short) SWS_htons( (SWS_u_short)pService->nPort);
      s_serv.s_proto = pService->pszProtocol;

      SetLastError( 0);
      API_RETURN( &s_serv);
      }
    }
#endif

  SetLastError( SWS_ETRY_AGAIN); /* Non-authoritative */
  API_RETURN( NULL);
  }


SWS_EXPORT( SWS_PTR( struct SWS_servent), SWS_getservbyport(
  int iPort, /* Network order */
  SWS_PTR( const char) pszProto
)){
#if OPT_DNS
  const struct SService* pService;
  int i;
#else
  (void)iPort;
  (void)pszProto;
#endif

  TRACE( 1, ("*** SWS_getservbyport(%d,%p)\n", iPort,pszProto));
  ASSERT( s_inited);
  if ( !s_inited)
    {
    SetLastError( SWS_EACCES);
    return NULL;
    }

#if OPT_DNS
  /* Look in list of known services */
  iPort = SWS_ntohs( (SWS_u_short)iPort);
  for ( pService = &s_services[ i = 0];
    i < (int)lengthof( s_services);
    ++i, ++pService
  ) {
    if ( pService->nPort == iPort
      && (NULL == pszProto || !SWS_stricmp( pszProto, pService->pszProtocol))
    ) {
      s_serv.s_name = pService->pszName;
      s_szServAliases[ 0] = pService->pszName;
      s_szServAliases[ 1] = NULL;
      s_serv.s_aliases = s_szServAliases;
      s_serv.s_port = (short) SWS_htons( (SWS_u_short) pService->nPort);
      s_serv.s_proto = pService->pszProtocol;

      SetLastError( 0);
      API_RETURN( &s_serv);
      }
    }
#endif

  SetLastError( SWS_ETRY_AGAIN); /* Non-authoritative */
  API_RETURN( NULL);
  }
#endif /* OPT_GETXBYY */


/****************************************************************************
 * Time
 ****************************************************************************/

/*
 * Idle time processing
 */
SWS_EXPORT( void, SWS_SockSleep( SWS_u_long millisecs))
  {
  TRACE( 9, ("SWS_SockSleep(%lu)\n", millisecs));

  if ( !s_inited)
    return;

  if ( !PlatformIsCritical())
    {
    switch ( millisecs)
      {
    case 0:
      PlatformIdle();
      break;
    case 1:
      PlatformWaitEvent( s_hFalseEvent, WAIT_MIN);
      break;
    default:
      PlatformWaitEvent( s_hFalseEvent, MS2TICKS( millisecs));
      break;
      }
    }
  }


/*
 * Monotonic millisecond timer
 */
SWS_EXPORT( SWS_u_long, SWS_SockTicks( void))
  {
  TRACE( 9, ("SWS_SockTicks()\n"));

#if 1
  return PlatformTime();
#else
  API_RETURN( PlatformTime());
#endif
  }


/****************************************************************************
 * Lower edge transport interface
 ****************************************************************************/

/*
 * Register a transport provider
 */
SWS_EXPORT( int, SWS_TransportRegister(  /* Returns !0 on error */
  SWS_PTR( const SWS_TransportDesc) pDesc, /* IN: Transport descriptor */
  SWS_PTR( void) pvContext,           /* IN: Client context */
  unsigned saps,                      /* IN: No. SAP's */
  size_t sapSize                      /* IN: Sizeof each SAP */
)){
  STransport* pTransport;
  char* s;
  size_t socketSize;
  ui32 size;

  TRACE( 9, ("SWS_TransportRegister(%p,%p,%u,%u)\n",
    pDesc,pvContext,saps,sapSize));
  ASSERT( NULL != pDesc);
  ASSERT( saps > 0);
  ASSERT( PlatformIsCritical());

  if ( sizeof(*pDesc) > pDesc->uSize)
    {
    TRACE( 0, ("!!!SWS_TransportRegister descriptor size too small\n"));
    return SWS_EINVAL;
    }

  if ( SWS_TRANSPORT_VERSION > pDesc->uVersion)
    {
    TRACE( 0, ("!!!SWS_TransportRegister version too old\n"));
    return SWS_EINVAL;
    }

  socketSize = sizeof( SWS_Socket) + sapSize;
  size = sizeof( STransport) + (ui32)saps * (ui32)socketSize;
#if SWS_SIZE_MAX < ULONG_MAX
  if ( size > SWS_SIZE_MAX)
    {
    TRACE( 0, ("!!!SWS_TransportRegister %u SAPs of %u bytes too big\n",
      saps, (unsigned)sapSize));
    return SWS_ENOMEM;
    }
#endif

  /* Allocate transport data */
  pTransport = (STransport*)malloc( (size_t)size);
  if ( NULL == pTransport)
    {
    TRACE( 0, ("!!! SWS_TransportRegister() Out of memory\n"));
    return SWS_ENOMEM;
    }

  pTransport->uVersion = pDesc->uVersion;
  pTransport->uAddressFamily = pDesc->uAddressFamily;
  pTransport->uSocketType = pDesc->uSocketType;
  pTransport->uProtocol = pDesc->uProtocol;
  pTransport->pszName = pDesc->pszName;

  pTransport->pvContext = pvContext;

  ASSERT( pDesc->pfnOpen);
  pTransport->pfnOpen = pDesc->pfnOpen;
  ASSERT( pDesc->pfnClose);
  pTransport->pfnClose = pDesc->pfnClose;
  ASSERT( pDesc->pfnBind);
  pTransport->pfnBind = pDesc->pfnBind;
  ASSERT( pDesc->pfnListen);
  pTransport->pfnListen = pDesc->pfnListen;
  ASSERT( pDesc->pfnConnect);
  pTransport->pfnConnect = pDesc->pfnConnect;
  ASSERT( pDesc->pfnShutdown);
  pTransport->pfnShutdown = pDesc->pfnShutdown;
  ASSERT( pDesc->pfnSendTo);
  pTransport->pfnSendTo = pDesc->pfnSendTo;
  ASSERT( pDesc->pfnRecvFrom);
  pTransport->pfnRecvFrom = pDesc->pfnRecvFrom;
  ASSERT( pDesc->pfnGetInfo);
  pTransport->pfnGetInfo = pDesc->pfnGetInfo;
  ASSERT( pDesc->pfnSetInfo);
  pTransport->pfnSetInfo = pDesc->pfnSetInfo;
  pTransport->pfnCleanup = pDesc->pfnCleanup;
  ASSERT( pDesc->pfnGetSockName);
  pTransport->pfnGetSockName = pDesc->pfnGetSockName;
  ASSERT( pDesc->pfnGetPeerName);
  pTransport->pfnGetPeerName = pDesc->pfnGetPeerName;

  pTransport->pFreeSockets = NULL;
  pTransport->sockets = saps;
  pTransport->socketSize = socketSize;

  /* Create socket free list */
  for (
    s = pTransport->socketArray;
    saps;
    --saps, s += pTransport->socketSize
  ) {
    /* Add to free list */
    ((SWS_Socket*)s)->pTransport = pTransport;
    ((SWS_Socket*)s)->eState = kStateClosed;
    SocketPut( (SWS_Socket*)s);
    }

  /* Add to transport list */
  pTransport->pNext = s_transportList;
  s_transportList = pTransport;

  return 0;
  }


/*
 * Incoming call from a listening SAP
 */
SWS_EXPORT( SWS_PTR( void), SWS_TransportCall(
  SWS_PTR( void) pvSap,
  SWS_PTR( int) pErr
)){
  SWS_SOCKET s;
  SSocketInfo info;
  int err;

  TRACE( 9, ("SWS_TransportCall(%p,%p)\n",pvSap,pErr));
  ASSERT( NULL != pvSap);
  ASSERT( NULL != pErr);

  s = (SWS_SOCKET)SWS_CONTAINER( pvSap, SWS_Socket, data);
  ASSERT( SOCKET_MAGIC == s->uMagic);
  ASSERT( NULL != s->pTransport);
  ASSERT( kStateListening == s->eState);

  if ( !IsValidSocket( s) )
    return NULL;

  if ( kStateListening != s->eState)
    {
    *pErr = SWS_EINVAL;
    return NULL;
    }

  if ( AtomicIncrement( &s->calls) > s->listenBacklog)
    {
    TRACE( 1, ("*** SWS_TransportCall() Too many pending connects\n"));
    AtomicDecrement( &s->calls);
    *pErr = SWS_ETOOMANYREFS;
    return NULL;
    }

  /* Create a new socket */
  info.pTransport = s->pTransport;
  err = PlatformSynchronise( &SocketGet, &info);
  if ( err)
    {
    AtomicDecrement( &s->calls);
    *pErr = SWS_EMFILE; /* No free sockets */
    return NULL;
    }
  ASSERT( NULL != info.s);

  /* Mark socket as calling */
  info.s->eState = kStateCalling;
  info.s->pListener = s;

  *pErr = 0;
  return info.s->data;
  }


/*
 * Set/clear an event
 */
SWS_EXPORT( void, SWS_TransportEventSet(
  SWS_PTR( void) pvSap,               /* IN: -> SAP data */
  SWS_ETransportEvent eEvent,         /* IN: Event type */
  i32 iInfo                           /* IN: Event info */
)){
  SWS_SOCKET s, sListener;

  TRACE( 9, ("SWS_TransportEventSet(%p,%d,%ld)\n",pvSap,eEvent,iInfo));
  ASSERT( NULL != pvSap);

  s = (SWS_SOCKET)SWS_CONTAINER( pvSap, SWS_Socket, data);
  ASSERT( SOCKET_MAGIC == s->uMagic);
  ASSERT( NULL != s->pTransport);

  if ( !IsValidSocket( s) )
    return;

  switch ( eEvent)
    {
  case SWS_kTransportEventRxReady:
    PlatformSetEvent( s->hRxReadyEvent, iInfo);
    break;

  case SWS_kTransportEventOob:
    PlatformSetEvent( s->hRxOobEvent, iInfo);
    break;

  case SWS_kTransportEventTxReady:
    PlatformSetEvent( s->hTxReadyEvent, iInfo);
    break;

  case SWS_kTransportEventConnect:
    /* Change call to connected */
    ASSERT( kStateCalling == s->eState);
    s->eState = kStateConnected;
    s->error = 0;
    PlatformSetEvent( s->hTxReadyEvent, iInfo);

    sListener = s->pListener;
    s->pListener = NULL;
    if ( NULL != sListener
      && SOCKET_MAGIC == sListener->uMagic /* In case listener was closed */
      && kStateListening == sListener->eState
    ) {
      ASSERT( NULL != sListener->pTransport);

      /* Add to pending accept queue */
      TRACE( 2, ("SWS_TransportEventSet(%p) Call for listener %p\n",
        pvSap, sListener));
      VERIFY( !SWS_MsgqPut( sListener->hBacklogQ, sizeof( s), &s));
      PlatformSetEvent( sListener->hRxReadyEvent, iInfo);
      }
    break;

  case SWS_kTransportEventError:
    /* Abort any pending operations */
    s->error = (int)iInfo;
    PlatformSetEvent( s->hRxReadyEvent, iInfo);
    PlatformSetEvent( s->hTxReadyEvent, iInfo);
    PlatformSetEvent( s->hCloseEvent, iInfo);
    break;

  case SWS_kTransportEventClose:
    PlatformSetEvent( s->hCloseEvent, iInfo);
    SocketCloseAck( s);
    break;

  default:
    TRACE( 0, ("!!!SWS_TransportEventSet Unknown event %d\n",eEvent));
    break;
    }
  }


/*
 * Wait for an event
 */
SWS_EXPORT( int, SWS_TransportEventWait(  /* Returns !0 on error */
  SWS_PTR( void) pvSap,               /* IN: -> SAP data */
  SWS_ETransportEvent eEvent          /* IN: Event type */
)){
  SWS_SOCKET s;
  i32 iRet;
  int isNonBlocking;

  TRACE( 9, ("SWS_TransportEventWait(%p,%d)\n",pvSap,eEvent));
  ASSERT( NULL != pvSap);

  s = (SWS_SOCKET)SWS_CONTAINER( pvSap, SWS_Socket, data);
  ASSERT( SOCKET_MAGIC == s->uMagic);
  ASSERT( NULL != s->pTransport);

  if ( !IsValidSocket( s) )
    return SWS_EBADF;

  isNonBlocking = s->isNonBlocking;
  switch ( eEvent)
    {
  case SWS_kTransportEventRxReady:
    iRet = PlatformWaitEvent(
      s->hRxReadyEvent, isNonBlocking ? WAIT_NEVER : s->rcvtime);
    break;

  case SWS_kTransportEventOob:
    iRet = PlatformWaitEvent(
      s->hRxOobEvent, isNonBlocking ? WAIT_NEVER : s->rcvtime);
    break;

  case SWS_kTransportEventTxReady:
    iRet = PlatformWaitEvent(
      s->hTxReadyEvent, isNonBlocking ? WAIT_NEVER : s->sndtime);

    /* Set TxReady after an error */
    if ( 1 != iRet && 0 != iRet)
      PlatformSetEvent( s->hTxReadyEvent, 1);
    break;

  case SWS_kTransportEventConnect:
    iRet = PlatformWaitEvent(
      s->hTxReadyEvent, isNonBlocking ? WAIT_NEVER : WAIT_FOREVER);
    break;

  default:
    TRACE( 0, ("!!!SWS_TransportEventWait Unknown event %d\n",eEvent));
    return SWS_EINVAL;
    }

  return iRet ? (int)iRet : isNonBlocking ? SWS_EWOULDBLOCK : SWS_ETIMEDOUT;
  }

/* End of file */
