/*****************************************************************************
 *
 * File name   : dhcp.c
 * Function    : DHCP client RFC 2131 and 951 (BOOTP)
 * Authors     : Lawrence Rust
 * Systems     : ANSI C
 * Version     : 1.0
 * Release Date: 03-Feb-03
 *
 *****************************************************************************
 * File History
 * ------------
 *
 * 22-Nov-02, LVR, Created.
 *
 *
 * Comments
 * --------
 *
 *****************************************************************************/

/* Compilation options */
#include "config.h"

#ifndef OPT_DHCP_PARAMS
#define OPT_DHCP_PARAMS 1 /* !0 to specify params wanted */
#endif


/* Exports */
#include "sws_dhcp.h"

#if OPT_DHCP
#if !OPT_PROTO_UDP
#error DHCP requires the UDP protocol
#endif

/* Imports */

/* ANSI C library */
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "debug.h"
#include "platform.h"

#include "sws_sock.h"
#include "sws_dlnk.h" /* SWS_DatalinkGetProperty */
#include "sws_timr.h"
#include "sws_ip.h"
#if OPT_PROTO_ICMP
#include "sws_icmp.h"
#endif
#include "sws_dns.h"

#define GetLastError() SWS_GetLastError()


/***************************************************************************
 * Macros & constants
 ***************************************************************************/

#define DHCP_TIMEOUT 4 /* Initial retry period in secs. RFC 2131 p24 */
#define DHCP_MAX_TIMEOUT 32 /* Seconds */
#define DHCP_POLL_INTERVAL 5 /* ms */

typedef enum EState
  {
  kStateError,
  kStateInitReboot,
  kStateRebooting,
  kStateInit,
  kStateSelecting,
  kStateRequesting,
  kStateBound,
  kStateRenewing,
  kStateRebinding
  } EState;


/***************************************************************************
 * Data types
 ***************************************************************************/

typedef struct SDhcpClient SDhcpClient;

typedef void FDhcpState( SDhcpClient*);

struct SDhcpClient
  {
  SDhcpClient* pNext; /* Linked list */

  int iface;
  EState eState;
  FDhcpState* pfnState;
  int iComplete;

  SWS_SOCKET s;
  SWS_TimerH hTimer;
  unsigned timeout;
  ui32 tTimeout;

  ui32 xid;
  ui16 secs;
  ui32 serverIpAddress;
  ui32 myIpAddress, subnetMask;
  ui32 gatewayIpAddress, dnsIpAddress;
  ui32 tRequest, tRenew, tRebind, tExpire;
  char szHost[ 32];
  char szDomain[ 160];

  unsigned char buf[ 576];
  };


/***************************************************************************
 * Local function prototypes
 ***************************************************************************/

static SDhcpClient* DhcpFind( int);
static void DhcpDestroy( SDhcpClient*);
static void DhcpConfigure( SDhcpClient*);
static void DhcpUnconfigure( SDhcpClient*);
static int DhcpRelease( SDhcpClient*);
static int DhcpOpenSocket( SDhcpClient*);
static int DhcpSetTimeout( SDhcpClient*);
static int DhcpMessage( SDhcpClient*, ui8, size_t*);

static FDhcpState DhcpDiscover;
static FDhcpState DhcpSelecting;
static FDhcpState DhcpRequest;
static FDhcpState DhcpRequesting;
static FDhcpState DhcpBound;
static FDhcpState DhcpRenewing;

static SWS_FTimer DhcpTimeout;


/***************************************************************************
 * Module data
 ***************************************************************************/

/* Configuration */
static int s_enable;

static SDhcpClient* s_pHead;


/***************************************************************************
 * Functions
 ***************************************************************************/

/*
 * Initialise this module
 */
int SWS_DhcpInit( void)               /* Returns !0 on error */
  {
  TRACE( 9, ("SWS_DhcpInit()\n"));

  s_pHead = NULL;

  /* Read config data */
  s_enable = (int)PlatformPropertyGetUint( PKEY_DHCP_ENABLE, OPT_DHCP );
  if ( s_enable > 0)
    {
    unsigned link, count = SWS_DatalinkCount();

    srand( (unsigned)time( NULL) );

    for ( link = 0; link < count; ++link)
      {
      ui32 ulVal;
      ui8 ucVal;
      size_t size;
      int iRet;

      /* Get IP address for datalink */
      size = sizeof( ulVal);
      iRet = SWS_DatalinkGetProperty(
        link,
        SWS_kDatalinkProperty_Address,
        &ulVal,
        &size
      );
      if ( 0 == iRet) switch ( ulVal)
        {
      default:
        /* Preserve configuration unless enable >= 2 */
        if ( s_enable < 2)
          break;
        /* Fall thru */
      case SWS_INADDR_ANY:
        /* Is DHCP enabled for this i/f? */
        size = sizeof( ucVal);
        iRet = SWS_DatalinkGetProperty(
          link,
          SWS_kDatalinkProperty_DHCP,
          &ucVal,
          &size
        );
        if ( 0 == iRet && ucVal)
          SWS_DhcpRenew( link);
        break;

      case SWS_INADDR_LOOPBACK:
        break;
        }
      }
    }

  return 0;
  }


/*
 * Finalise this module
 */
void SWS_DhcpCleanup( void)
  {
  TRACE( 9, ("SWS_DhcpCleanup()\n"));

  while ( NULL != s_pHead)
    {
    SDhcpClient* pClient = s_pHead;

    DhcpRelease( pClient);
    SWS_SockSleep( 50); /* To permit ARP reply */

    s_pHead = s_pHead->pNext;
    DhcpDestroy( pClient);
    }
  }


/***************************************************************************
 * Exported functions
 ***************************************************************************/

/*
 * Renew or obtain a DHCP lease
 */
SWS_EXPORT( int, SWS_DhcpRenew( int iface))
  {
  SDhcpClient* pClient;
  int iRet;

  TRACE( 9, ("SWS_DhcpRenew(%d)\n", iface));

  /* Is there an existing DHCP client for this interface? */
  pClient = DhcpFind( iface);
  if ( NULL == pClient)
    {
    /* No, then create one */
    size_t size;
    char* psz;

    /* Allocate instance data */
    pClient = (SDhcpClient*) calloc( 1, sizeof( *pClient) );
    if ( NULL == pClient)
      {
      TRACE( 0, ("!!! SWS_DhcpRenew() Out of memory\n"));
      return SWS_ENOMEM;
      }

    pClient->iface = iface;
    SWS_gethostname( pClient->szHost, sizeof( pClient->szHost) );
    psz = strchr( pClient->szHost, '.');
    if ( NULL != psz)
      *psz = '\0';
    pClient->serverIpAddress = SWS_INADDR_ANY;
    pClient->gatewayIpAddress = SWS_INADDR_NONE;

    /* Get default IP address */
    size = sizeof( pClient->myIpAddress);
    iRet = SWS_DatalinkGetProperty(
      iface,
      SWS_kDatalinkProperty_Address,
      &pClient->myIpAddress,
      &size
    );
    if ( iRet)
      {
      TRACE( 1, ("*** SWS_DhcpRenew() No such interface\n"));
      DhcpDestroy( pClient);
      return iRet;
      }

    /* Allocate a timer */
    pClient->hTimer = SWS_TimerAlloc( &DhcpTimeout, pClient);
    if ( NULL == pClient->hTimer)
      {
      TRACE( 1, ("*** SWS_DhcpRenew() SWS_TimerAlloc failed\n"));
      DhcpDestroy( pClient);
      return SWS_EMFILE;
      }

    /* Open a network socket */
    if ( SWS_INVALID_SOCKET == pClient->s)
      {
      iRet = DhcpOpenSocket( pClient);
      if ( iRet)
        {
        DhcpDestroy( pClient);
        return iRet;
        }
      }
    }
  else
    {
    /* Release current lease */
    DhcpRelease( pClient);
    }

  /* Set request ID */
  pClient->xid = SWS_SockTicks();
  pClient->timeout = 0;
  pClient->iComplete = -1;

  if ( SWS_INADDR_ANY == pClient->myIpAddress)
    {
    /* Start with a DHCP DISCOVER message */
    pClient->eState = kStateInit;
    DhcpDiscover( pClient);
    }
  else
    {
    /* Start with a DHCP REQUEST message */
    pClient->eState = kStateInitReboot;
    DhcpRequest( pClient);
    }

  /* Wait for DHCP to complete */
  while ( -1 == pClient->iComplete)
    SWS_SockSleep( 0);

  iRet = pClient->iComplete;
  if ( 0 == iRet)
    {
    /* Add to list of DHCP clients */
    pClient->pNext = s_pHead;
    s_pHead = pClient;
    }
  else
    {
    DhcpDestroy( pClient);
    }

  return iRet;
  }



/*
 * Release the lease for a datalink
 */
SWS_EXPORT( int, SWS_DhcpRelease( int iface))
  {
  SDhcpClient* pClient;

  TRACE( 9, ("SWS_DhcpRelease(%d)\n", iface));

  pClient = DhcpFind( iface);
  if ( NULL == pClient)
    {
    TRACE( 1, ("*** SWS_DhcpRelease(%d) No client with that ID\n", iface));
    return SWS_EINVAL;
    }

  return DhcpRelease( pClient);
  }


/***************************************************************************
 * Internal DHCP state functions
 ***************************************************************************/

/*
 * Send a DHCP discover request
 */
void DhcpDiscover( SDhcpClient* pClient)
  {
  int iRet;
  size_t offset;
  struct SWS_sockaddr_in from;
  ui32 ticks = SWS_SockTicks();

  TRACE( 9, ("DhcpDiscover(%p)\n", pClient));

  switch ( pClient->eState)
    {
  case kStateRebooting:
  case kStateInit:
  case kStateRequesting:
  case kStateBound:
  case kStateRenewing:
  case kStateRebinding:
    pClient->eState = kStateSelecting;
    pClient->pfnState = &DhcpSelecting;
    break;

  case kStateSelecting:
    break;

  default:
    TRACE( 1, ("*** DhcpDiscover() unexpected state %d\n", (int)pClient->eState));
    pClient->iComplete = SWS_EINVAL;
    return;
    }

  if ( SWS_INVALID_SOCKET == pClient->s
    && 0 != (iRet = DhcpOpenSocket( pClient))
  ) {
    pClient->iComplete = iRet;
    return;
    }

  iRet = DhcpSetTimeout( pClient);
  if ( iRet)
    {
    TRACE( 1, ("*** DhcpDiscover(%p) timed out\n", pClient));
    pClient->iComplete = iRet;
    return;
    }
  TRACE( 2, ("DhcpDiscover(%p) timeout %u\n", pClient, pClient->timeout));
  pClient->tTimeout = ticks + (ui32)pClient->timeout * SWS_TIMER_TICKS;
  pClient->secs = (ui16)((ticks - pClient->xid + 500) / 1000);

  /* Discard any previous offer */
  pClient->serverIpAddress = SWS_INADDR_ANY;
  pClient->dnsIpAddress = SWS_INADDR_ANY;
  pClient->gatewayIpAddress = SWS_INADDR_NONE;
  pClient->szDomain[ 0] = '\0';

  /* Format a DHCP message */
  iRet = DhcpMessage( pClient, SWS_DHCP_MSGTYPE_DHCPDISCOVER, &offset);
  if ( iRet)
    {
    pClient->iComplete = iRet;
    return;
    }

  SWS_POKE8( pClient->buf, offset, SWS_DHCP_OPT_MAX_SIZE);
  ++offset;
  SWS_POKE8( pClient->buf, offset, 2); /* length */
  ++offset;
  SWS_POKE16( pClient->buf, offset, sizeof(pClient->buf) );
  offset += 2;

  if ( SWS_INADDR_ANY != pClient->myIpAddress)
    {
    SWS_POKE8( pClient->buf, offset, SWS_DHCP_OPT_IP_ADDRESS);
    ++offset;
    SWS_POKE8( pClient->buf, offset, 4); /* length */
    ++offset;
    SWS_POKE32( pClient->buf, offset, pClient->myIpAddress);
    offset += 4;
    }

  if ( pClient->szHost[0] )
    {
    /* Send host name */
    size_t size = strlen( pClient->szHost);
    SWS_POKE8( pClient->buf, offset, SWS_DHCP_OPT_HOST_NAME);
    ++offset;
    SWS_POKE8( pClient->buf, offset, (ui8)size); /* length */
    ++offset;
    memcpy( pClient->buf + offset, pClient->szHost, size);
    offset += size;
    }

  SWS_POKE8( pClient->buf, offset, SWS_DHCP_OPT_END);
  ++offset;
  ASSERT( offset <= sizeof( pClient->buf) );

  /* Broadcast message to DHCP server */
  from.sin_family = SWS_AF_INET;
  from.sin_addr.SWS_s_addr = SWS_htonl( SWS_INADDR_BROADCAST);
  from.sin_port = SWS_htons( SWS_DHCP_SERVER);
  iRet = SWS_sendto( pClient->s, pClient->buf, offset, SWS_MSG_DONTROUTE,
    (struct SWS_sockaddr*)&from, sizeof(from) );
  if ( SWS_SOCKET_ERROR == iRet)
    {
    iRet = GetLastError();
    TRACE( 1, ("*** DhcpDiscover() sendto failed %d\n", iRet));
    pClient->iComplete = iRet;
    return;
    }
  ASSERT( offset == (unsigned)iRet);

  /* Wait for an offer reply */
  ASSERT( &DhcpSelecting == pClient->pfnState);
  SWS_TimerStart( pClient->hTimer, DHCP_POLL_INTERVAL);
  }


/*
 * Wait for a DHCP offer message
 */
void DhcpSelecting( SDhcpClient* pClient)
  {
  SWS_u_long bytes;
  ui32 ticks;

  TRACE( 9, ("DhcpSelecting(%p)\n", pClient));

  switch ( pClient->eState)
    {
  case kStateSelecting:
    break;
  default:
    TRACE( 1, ("*** DhcpSelecting() unexpected state %d\n", (int)pClient->eState));
    pClient->iComplete = SWS_EINVAL;
    return;
    }

  /* Poll for data */
  while ( 0 == SWS_ioctl( pClient->s, SWS_FIONREAD, &bytes)
    && bytes > 0
  ) {
    /* Message received */
    int iRet = SWS_recv( pClient->s, pClient->buf, sizeof( pClient->buf), 0);

    if ( SWS_SOCKET_ERROR != iRet
      && (unsigned)iRet >= SWS_DHCP_OPTS
      && SWS_DHCP_OP_BOOTREPLY == pClient->buf[ SWS_DHCP_OP]
    ) {
      ui32 ulVal;

      /* Verify it's a reply to the request */
      SWS_PEEK32( ulVal, pClient->buf, SWS_DHCP_XID);
      if ( pClient->xid == ulVal )
        {
        /* Get assigned address */
        SWS_PEEK32( pClient->myIpAddress, pClient->buf, SWS_DHCP_YIADDR);
        ASSERT( SWS_INADDR_ANY != pClient->myIpAddress);
        SWS_PEEK32( pClient->serverIpAddress, pClient->buf, SWS_DHCP_SIADDR);

        TRACE( 2, ("DhcpSelecting(%p) myIP:%08lx Server:%08lx\n",
          pClient, pClient->myIpAddress, pClient->serverIpAddress ));

        if ( (unsigned)iRet >= SWS_DHCP_OPTS + 7
          && SWS_DHCP_OP_BOOTREPLY == pClient->buf[ SWS_DHCP_OP]
          && SWS_DHCP_COOKIE1 == pClient->buf[ SWS_DHCP_OPTS + 0]
          && SWS_DHCP_COOKIE2 == pClient->buf[ SWS_DHCP_OPTS + 1]
          && SWS_DHCP_COOKIE3 == pClient->buf[ SWS_DHCP_OPTS + 2]
          && SWS_DHCP_COOKIE4 == pClient->buf[ SWS_DHCP_OPTS + 3]
          && SWS_DHCP_OPT_MSGTYPE == pClient->buf[ SWS_DHCP_OPTS + 4]
          && 1 == pClient->buf[ SWS_DHCP_OPTS + 5]
          && SWS_DHCP_MSGTYPE_DHCPOFFER == pClient->buf[ SWS_DHCP_OPTS + 6]
        ) {
          /* Scan options */
          size_t offset = SWS_DHCP_OPTS + 7;
          while ( offset < (unsigned)iRet)
            {
            unsigned type, len;

            /* Get option and length */
            SWS_PEEK8( type, pClient->buf, offset);
            ++offset;

            if ( SWS_DHCP_OPT_END == type)
              break;

            switch ( type)
              {
            case SWS_DHCP_OPT_NOP:
              len = 0;
              break;

            case SWS_DHCP_OPT_SUBNET_MASK:
              SWS_PEEK8( len, pClient->buf, offset);
              ++offset;
              SWS_PEEK32( pClient->subnetMask, pClient->buf, offset);
              TRACE( 2, ("DhcpSelecting(%p) subnet:%08lx\n", pClient, pClient->subnetMask));
              break;

            case SWS_DHCP_OPT_ROUTER:
              SWS_PEEK8( len, pClient->buf, offset);
              ++offset;
              SWS_PEEK32( pClient->gatewayIpAddress, pClient->buf, offset);
              TRACE( 2, ("DhcpSelecting(%p) router:%08lx\n", pClient, pClient->gatewayIpAddress));
              break;

            case SWS_DHCP_OPT_HOST_NAME:
              SWS_PEEK8( len, pClient->buf, offset);
              ++offset;
              TRACE( 2, ("DhcpSelecting(%p) hostname'%.*s'\n", pClient, (int)len, pClient->buf + offset));
              strncpy( pClient->szHost, (char*)pClient->buf + offset, len);
              pClient->szHost[ len] = '\0';
              break;

            case SWS_DHCP_OPT_DOMAIN_NAME:
              SWS_PEEK8( len, pClient->buf, offset);
              ++offset;
              TRACE( 2, ("DhcpSelecting(%p) domain name'%.*s'\n", pClient, (int)len, pClient->buf + offset));
              strncpy( pClient->szDomain, (char*)pClient->buf + offset, len);
              pClient->szDomain[ len] = '\0';
              break;

            case SWS_DHCP_OPT_SERVERID:
              SWS_PEEK8( len, pClient->buf, offset);
              ++offset;
              SWS_PEEK32( pClient->serverIpAddress, pClient->buf, offset);
              TRACE( 2, ("DhcpSelecting(%p) server:%08lx\n", pClient, pClient->serverIpAddress));
              break;

            case SWS_DHCP_OPT_DNS:
              SWS_PEEK8( len, pClient->buf, offset);
              ++offset;
              SWS_PEEK32( pClient->dnsIpAddress, pClient->buf, offset);
              TRACE( 2, ("DhcpSelecting(%p) DNS:%08lx\n", pClient, pClient->dnsIpAddress));
              break;

            case SWS_DHCP_OPT_NAME_SERVER:
            default:
              SWS_PEEK8( len, pClient->buf, offset);
              ++offset;
              break;
              }

            offset += len;
            }

          /* Send DHCPREQUEST */
          pClient->timeout = 0;
          DhcpRequest( pClient);
          return;
          }
        else
          {
          TRACE( 2, ("DhcpSelecting() Using BOOTP server\n"));

          DhcpConfigure( pClient);
          pClient->eState = kStateInit;
          pClient->iComplete = 0;
          return;
          }
        }
      }
    }

  /* Exceeded recv timeout? */
  ticks = SWS_SockTicks();
  if ( !SWS_TimerLE( ticks, pClient->tTimeout) )
    {
    /* Yes, retry the DHCPDISCOVER */
    TRACE( 3, ("DhcpSelecting(%p) timeout\n", pClient));
    DhcpDiscover( pClient);
    return;
    }

  /* Wait some more */
  ASSERT( &DhcpSelecting == pClient->pfnState);
  SWS_TimerStart( pClient->hTimer, DHCP_POLL_INTERVAL);
  }


/*
 * Send a DHCP request message
 */
void DhcpRequest( SDhcpClient* pClient)
  {
  size_t offset;
  struct SWS_sockaddr_in from;
  int iRet;
  ui32 ticks = SWS_SockTicks();

  TRACE( 9, ("DhcpRequest(%p)\n", pClient));

  switch ( pClient->eState)
    {
  case kStateSelecting:
    pClient->eState = kStateRequesting;
    pClient->pfnState = &DhcpRequesting;
    break;

  case kStateInitReboot:
    pClient->eState = kStateRebooting;
    pClient->pfnState = &DhcpRequesting;
    break;

  case kStateRequesting:
  case kStateRebooting:
    break;

  default:
    TRACE( 1, ("*** DhcpRequest() unexpected state %d\n", (int)pClient->eState));
    pClient->iComplete = SWS_EINVAL;
    return;
    }

  iRet = DhcpSetTimeout( pClient);
  if ( iRet)
    {
    TRACE( 1, ("*** DhcpRequest(%p) timed out\n", pClient));
    pClient->iComplete = iRet;
    return;
    }
  TRACE( 2, ("DhcpRequest(%p) timeout %u\n", pClient, pClient->timeout));
  pClient->tTimeout = ticks + (ui32)pClient->timeout * SWS_TIMER_TICKS;

  /* Format a DHCP message */
  iRet = DhcpMessage( pClient, SWS_DHCP_MSGTYPE_DHCPREQUEST, &offset);
  if ( iRet)
    {
    pClient->iComplete = iRet;
    return;
    }

  SWS_POKE8( pClient->buf, offset, SWS_DHCP_OPT_MAX_SIZE);
  ++offset;
  SWS_POKE8( pClient->buf, offset, 2); /* length */
  ++offset;
  SWS_POKE16( pClient->buf, offset, sizeof(pClient->buf) );
  offset += 2;

  ASSERT( SWS_INADDR_ANY != pClient->myIpAddress);
  SWS_POKE8( pClient->buf, offset, SWS_DHCP_OPT_IP_ADDRESS);
  ++offset;
  SWS_POKE8( pClient->buf, offset, 4); /* length */
  ++offset;
  SWS_POKE32( pClient->buf, offset, pClient->myIpAddress);
  offset += 4;

  if ( SWS_INADDR_ANY != pClient->serverIpAddress)
    {
    SWS_POKE8( pClient->buf, offset, SWS_DHCP_OPT_SERVERID);
    ++offset;
    SWS_POKE8( pClient->buf, offset, 4); /* length */
    ++offset;
    SWS_POKE32( pClient->buf, offset, pClient->serverIpAddress);
    offset += 4;
    }

  if ( pClient->szHost[0] )
    {
    /* Send host name */
    size_t size = strlen( pClient->szHost);
    SWS_POKE8( pClient->buf, offset, SWS_DHCP_OPT_HOST_NAME);
    ++offset;
    SWS_POKE8( pClient->buf, offset, (ui8)size); /* length */
    ++offset;
    memcpy( pClient->buf + offset, pClient->szHost, size);
    offset += size;
    }

  SWS_POKE8( pClient->buf, offset, SWS_DHCP_OPT_END);
  ++offset;
  ASSERT( offset <= sizeof( pClient->buf) );

  pClient->tRequest = (ui32)time( NULL);

  /* Broadcast message to DHCP server */
  from.sin_family = SWS_AF_INET;
  from.sin_addr.SWS_s_addr = SWS_htonl( SWS_INADDR_BROADCAST);
  from.sin_port = SWS_htons( SWS_DHCP_SERVER);
  iRet = SWS_sendto( pClient->s, pClient->buf, offset, SWS_MSG_DONTROUTE,
    (struct SWS_sockaddr*)&from, sizeof(from) );
  if ( SWS_SOCKET_ERROR == iRet)
    {
    iRet = GetLastError();
    TRACE( 1, ("*** DhcpRequest() sendto failed %d\n", iRet));
    pClient->iComplete = iRet;
    return;
    }
  ASSERT( offset == (unsigned)iRet);

  /* Wait for a DHCPACK or DHCPNAK reply */
  ASSERT( &DhcpRequesting == pClient->pfnState);
  SWS_TimerStart( pClient->hTimer, DHCP_POLL_INTERVAL);
  }


/*
 * Wait for a reply to a DHCPREQUEST message
 */
void DhcpRequesting( SDhcpClient* pClient)
  {
  SWS_u_long bytes;
  ui32 ticks;

  TRACE( 9, ("DhcpRequesting(%p)\n", pClient));

  switch ( pClient->eState)
    {
  case kStateRequesting:
  case kStateRebooting:
    break;

  default:
    TRACE( 1, ("*** DhcpRequesting() unexpected state %d\n", (int)pClient->eState));
    pClient->iComplete = SWS_EINVAL;
    return;
    }

  /* Poll for data */
  while ( 0 == SWS_ioctl( pClient->s, SWS_FIONREAD, &bytes)
    && bytes > 0
  ) {
    /* Message received */
    int iRet = SWS_recv( pClient->s, pClient->buf, sizeof( pClient->buf), 0);

    if ( SWS_SOCKET_ERROR != iRet
      && (unsigned)iRet >= SWS_DHCP_OPTS + 7
      && SWS_DHCP_OP_BOOTREPLY == pClient->buf[ SWS_DHCP_OP]
      && SWS_DHCP_COOKIE1 == pClient->buf[ SWS_DHCP_OPTS + 0]
      && SWS_DHCP_COOKIE2 == pClient->buf[ SWS_DHCP_OPTS + 1]
      && SWS_DHCP_COOKIE3 == pClient->buf[ SWS_DHCP_OPTS + 2]
      && SWS_DHCP_COOKIE4 == pClient->buf[ SWS_DHCP_OPTS + 3]
      && SWS_DHCP_OPT_MSGTYPE == pClient->buf[ SWS_DHCP_OPTS + 4]
      && 1 == pClient->buf[ SWS_DHCP_OPTS + 5]
    ) {
      ui32 ulVal;

      /* Verify it's a reply to the request */
      SWS_PEEK32( ulVal, pClient->buf, SWS_DHCP_XID);
      if ( pClient->xid == ulVal )
        {
        unsigned type;
        size_t offset;

        SWS_PEEK8( type, pClient->buf, SWS_DHCP_OPTS + 6);
        switch ( type)
          {
        case SWS_DHCP_MSGTYPE_DHCPACK:
          TRACE( 2, ("DhcpRequesting(%p) ACK\n", pClient));

          DhcpConfigure( pClient);

          /* NB SHOULD ping the client address and if a reply DHCPDECLINE */
#if 0 && OPT_PROTO_ICMP
          SWS_IcmpSend(
            SWS_INADDR_ANY,
            pClient->myIpAddress,
            SWS_ICMP_ECHO, 0, 0,
            NULL,
            0,
            NULL
          );
          ticks = SWS_SockTicks() + (1 * SWS_TIMER_TICKS);
          do
            {
            SWS_BufferH hBuf = SWS_IpRecvIcmp();
            if ( NULL != hBuf)
              {
              DhcpUnconfigure( pClient);
              return DhcpDecline( pClient);
              }
            }
          while ( SWS_TimerLE( SWS_SockTicks(), ticks));
#endif

          pClient->tExpire = pClient->tRenew = pClient->tRebind = 0;

          /* Scan options */
          offset = SWS_DHCP_OPTS + 7;
          while ( offset < (unsigned)iRet)
            {
            unsigned len;

            /* Get option and length */
            SWS_PEEK8( type, pClient->buf, offset);
            ++offset;

            if ( SWS_DHCP_OPT_END == type)
              break;

            switch ( type)
              {
            case SWS_DHCP_OPT_NOP:
              len = 0;
              break;

            case SWS_DHCP_OPT_LEASE_TIME:
              SWS_PEEK8( len, pClient->buf, offset);
              ++offset;
              SWS_PEEK32( ulVal, pClient->buf, offset);
              TRACE( 2, ("DhcpRequesting(%p) Lease:%lu secs\n", pClient, ulVal));
              pClient->tExpire = pClient->tRequest + ulVal;
              if ( 0 == pClient->tRenew)
                pClient->tRenew = pClient->tRequest + ulVal / 2;
              if ( 0 == pClient->tRebind)
                pClient->tRebind = (pClient->tRenew + pClient->tExpire) / 2;
              break;

            case SWS_DHCP_OPT_T1_TIME:
              SWS_PEEK8( len, pClient->buf, offset);
              ++offset;
              SWS_PEEK32( ulVal, pClient->buf, offset);
              TRACE( 2, ("DhcpRequesting(%p) Renew:%lu secs\n", pClient, ulVal));
              pClient->tRenew = pClient->tRequest + ulVal;
              break;

            case SWS_DHCP_OPT_T2_TIME:
              SWS_PEEK8( len, pClient->buf, offset);
              ++offset;
              SWS_PEEK32( ulVal, pClient->buf, offset);
              TRACE( 2, ("DhcpRequesting(%p) Rebind:%lu secs\n", pClient, ulVal));
              pClient->tRebind = pClient->tRequest + ulVal;
              break;

            case SWS_DHCP_OPT_SERVERID:
              SWS_PEEK8( len, pClient->buf, offset);
              ++offset;
              SWS_PEEK32( pClient->serverIpAddress, pClient->buf, offset);
              TRACE( 2, ("DhcpRequesting(%p) server:%08lx\n", pClient, pClient->serverIpAddress));
              break;

            default:
              SWS_PEEK8( len, pClient->buf, offset);
              ++offset;
              break;
              }

            offset += len;
            }

          /* Release the socket */
          pClient->iComplete = 0;
          SWS_close( pClient->s);
          pClient->s = SWS_INVALID_SOCKET;

          DhcpBound( pClient);
          return;

        case SWS_DHCP_MSGTYPE_DHCPNAK:
          TRACE( 2, ("DhcpRequesting(%p) NAK\n", pClient));
          DhcpUnconfigure( pClient);

          /* restart */
          pClient->xid = SWS_SockTicks();
          pClient->timeout = 0;
          DhcpDiscover( pClient);
          return;

        default:
          /* Ignore the message */
          break;
          }
        }
      }
    }

  /* Exceeded recv timeout? */
  ticks = SWS_SockTicks();
  if ( !SWS_TimerLE( ticks, pClient->tTimeout) )
    {
    /* Yes, retry the DHCPREQUEST */
    TRACE( 3, ("DhcpRequesting(%p) timeout\n", pClient));
    DhcpRequest( pClient);
    return;
    }

  /* Wait some more */
  ASSERT( &DhcpRequesting == pClient->pfnState);
  SWS_TimerStart( pClient->hTimer, DHCP_POLL_INTERVAL);
  }


/*
 * Check for lease expiry
 */
void DhcpBound( SDhcpClient* pClient)
  {
  struct SWS_sockaddr_in from;
  size_t offset;
  ui32 t;
  int iRet;

  TRACE( 9, ("DhcpBound(%p)\n", pClient));

  switch ( pClient->eState)
    {
  case kStateRequesting:
  case kStateRebooting:
    pClient->eState = kStateBound;
    pClient->pfnState = &DhcpBound;
    break;

  case kStateRenewing:
  case kStateRebinding:
    break;

  case kStateBound:
    break;

  default:
    TRACE( 1, ("*** DhcpBound() unexpected state %d\n", (int)pClient->eState));
    pClient->iComplete = SWS_EINVAL;
    return;
    }

  t = (ui32)time( NULL);
  if ( t >= pClient->tExpire)
    {
    /* Lease expired, restart */
    TRACE( 2, ("DhcpBound(%p) Lease expired\n", pClient));

    DhcpUnconfigure( pClient);

    pClient->timeout = 0;
    pClient->xid = SWS_SockTicks();
    DhcpDiscover( pClient);
    return;
    }
  else if ( t >= pClient->tRebind)
    {
    /* Rebinding */
    pClient->eState = kStateRebinding;
    pClient->serverIpAddress = SWS_INADDR_BROADCAST;

    t = (pClient->tExpire - t) / 2;
    TRACE( 2, ("DhcpBound(%p) Rebinding timeout %lu\n", pClient, t));
    pClient->tTimeout = SWS_SockTicks() + t * SWS_TIMER_TICKS;
    }
  else if ( t >= pClient->tRenew)
    {
    /* Renewing */
    pClient->eState = kStateRenewing;

    t = (pClient->tRebind - t) / 2;
    TRACE( 2, ("DhcpBound(%p) Renewing timeout %lu\n", pClient, t));
    pClient->tTimeout = SWS_SockTicks() + t * SWS_TIMER_TICKS;
    }
  else
    {
    /* Continue polling for lease expiry */
    t = pClient->tRenew - t;
    if ( t > 1000000UL)
      t = 1000000UL;

    ASSERT( &DhcpBound == pClient->pfnState);
    SWS_TimerStart( pClient->hTimer, t * SWS_TIMER_TICKS);
    return;
    }

  /* Renwe the lease */
  if ( SWS_INVALID_SOCKET == pClient->s)
    {
    iRet = DhcpOpenSocket( pClient);
    if ( iRet)
      {
      /* Try again later */
      SWS_TimerStart( pClient->hTimer, DHCP_POLL_INTERVAL);
      ASSERT( &DhcpBound == pClient->pfnState);
      return;
      }
    }

  /* Format a DHCP message */
  pClient->xid++;
  pClient->secs = 0;
  iRet = DhcpMessage( pClient, SWS_DHCP_MSGTYPE_DHCPREQUEST, &offset);
  if ( iRet)
    {
    pClient->iComplete = iRet;
    return;
    }

  SWS_POKE32( pClient->buf, SWS_DHCP_CIADDR, pClient->myIpAddress);

  SWS_POKE8( pClient->buf, offset, SWS_DHCP_OPT_END);
  ++offset;
  ASSERT( offset <= sizeof( pClient->buf) );

  pClient->tRequest = (ui32)time( NULL);

  /* Send to DHCP server */
  from.sin_family = SWS_AF_INET;
  from.sin_addr.SWS_s_addr = SWS_htonl( pClient->serverIpAddress);
  from.sin_port = SWS_htons( SWS_DHCP_SERVER);
  iRet = SWS_sendto( pClient->s, pClient->buf, offset, 0,
    (struct SWS_sockaddr*)&from, sizeof(from) );
  if ( SWS_SOCKET_ERROR == iRet)
    {
    iRet = GetLastError();
    TRACE( 1, ("*** DhcpBound() sendto failed %d\n", iRet));
    pClient->iComplete = iRet;
    return;
    }
  ASSERT( offset == (unsigned)iRet);

  /* Wait for reply */
  pClient->pfnState = &DhcpRenewing;
  SWS_TimerStart( pClient->hTimer, DHCP_POLL_INTERVAL);
  }


/*
 * Waiting for lease renewal
 */
void DhcpRenewing( SDhcpClient* pClient)
  {
  SWS_u_long bytes;
  ui32 ticks;

  TRACE( 9, ("DhcpRenewing(%p)\n", pClient));

  switch ( pClient->eState)
    {
  case kStateRenewing:
  case kStateRebinding:
    break;

  default:
    TRACE( 1, ("*** DhcpRenewing() unexpected state %d\n", (int)pClient->eState));
    pClient->iComplete = SWS_EINVAL;
    return;
    }

  /* Poll for data */
  while ( 0 == SWS_ioctl( pClient->s, SWS_FIONREAD, &bytes)
    && bytes > 0
  ) {
    /* Message received */
    int iRet = SWS_recv( pClient->s, pClient->buf, sizeof( pClient->buf), 0);

    if ( SWS_SOCKET_ERROR != iRet
      && (unsigned)iRet >= SWS_DHCP_OPTS + 7
      && SWS_DHCP_OP_BOOTREPLY == pClient->buf[ SWS_DHCP_OP]
      && SWS_DHCP_COOKIE1 == pClient->buf[ SWS_DHCP_OPTS + 0]
      && SWS_DHCP_COOKIE2 == pClient->buf[ SWS_DHCP_OPTS + 1]
      && SWS_DHCP_COOKIE3 == pClient->buf[ SWS_DHCP_OPTS + 2]
      && SWS_DHCP_COOKIE4 == pClient->buf[ SWS_DHCP_OPTS + 3]
      && SWS_DHCP_OPT_MSGTYPE == pClient->buf[ SWS_DHCP_OPTS + 4]
      && 1 == pClient->buf[ SWS_DHCP_OPTS + 5]
    ) {
      ui32 ulVal;

      /* Verify it's a reply to the request */
      SWS_PEEK32( ulVal, pClient->buf, SWS_DHCP_XID);
      if ( pClient->xid == ulVal )
        {
        unsigned type;
        size_t offset;

        SWS_PEEK8( type, pClient->buf, SWS_DHCP_OPTS + 6);
        switch ( type)
          {
        case SWS_DHCP_MSGTYPE_DHCPACK:
          TRACE( 2, ("DhcpRenewing(%p) ACK\n", pClient));

          pClient->tExpire = pClient->tRenew = pClient->tRebind = 0;

          /* Scan options */
          offset = SWS_DHCP_OPTS + 7;
          while ( offset < (unsigned)iRet)
            {
            unsigned len;

            /* Get option and length */
            SWS_PEEK8( type, pClient->buf, offset);
            ++offset;

            if ( SWS_DHCP_OPT_END == type)
              break;

            switch ( type)
              {
            case SWS_DHCP_OPT_NOP:
              len = 0;
              break;

            case SWS_DHCP_OPT_LEASE_TIME:
              SWS_PEEK8( len, pClient->buf, offset);
              ++offset;
              SWS_PEEK32( ulVal, pClient->buf, offset);
              pClient->tExpire = pClient->tRequest + ulVal;
              if ( 0 == pClient->tRenew)
                pClient->tRenew = pClient->tRequest + ulVal / 2;
              if ( 0 == pClient->tRebind)
                pClient->tRebind = (pClient->tRenew + pClient->tExpire) / 2;
              break;

            case SWS_DHCP_OPT_T1_TIME:
              SWS_PEEK8( len, pClient->buf, offset);
              ++offset;
              SWS_PEEK32( ulVal, pClient->buf, offset);
              pClient->tRenew = pClient->tRequest + ulVal;
              break;

            case SWS_DHCP_OPT_T2_TIME:
              SWS_PEEK8( len, pClient->buf, offset);
              ++offset;
              SWS_PEEK32( ulVal, pClient->buf, offset);
              pClient->tRebind = pClient->tRequest + ulVal;
              break;

            default:
              SWS_PEEK8( len, pClient->buf, offset);
              ++offset;
              break;
              }

            offset += len;
            }

          DhcpBound( pClient);
          return;

        case SWS_DHCP_MSGTYPE_DHCPNAK:
          TRACE( 2, ("DhcpRenewing(%p) NAK\n", pClient));
          DhcpUnconfigure( pClient);

          /* restart */
          pClient->timeout = 0;
          pClient->xid = SWS_SockTicks();
          DhcpDiscover( pClient);
          return;

        default:
          /* Ignore the message */
          break;
          }
        }
      }
    }

  /* Exceeded recv timeout? */
  ticks = SWS_SockTicks();
  if ( !SWS_TimerLE( ticks, pClient->tTimeout) )
    {
    /* Yes, retry the DHCPREQUEST */
    TRACE( 3, ("DhcpRenewing(%p) timeout\n", pClient));
    DhcpBound( pClient);
    return;
    }

  /* Wait some more */
  SWS_TimerStart( pClient->hTimer, DHCP_POLL_INTERVAL);
  }


/*
 * Release a DHCP lease
 */
int DhcpRelease( SDhcpClient* pClient)
  {
  size_t offset;
  struct SWS_sockaddr_in from;
  int iRet;

  TRACE( 2, ("DhcpRelease(%p)\n", pClient));

  switch ( pClient->eState)
    {
  default:
    /* Nothing to release */
    return 0;

  case kStateRequesting:
  case kStateBound:
  case kStateRenewing:
    break;
    }

  if ( SWS_INVALID_SOCKET == pClient->s)
    {
    iRet = DhcpOpenSocket( pClient);
    if ( iRet)
      return iRet;
    }

  /* Format a DHCP message */
  pClient->secs = 0;
  iRet = DhcpMessage( pClient, SWS_DHCP_MSGTYPE_DHCPRELEASE, &offset);
  if ( iRet)
    return iRet;

  ASSERT( SWS_INADDR_ANY != pClient->myIpAddress);
  SWS_POKE32( pClient->buf, SWS_DHCP_CIADDR, pClient->myIpAddress);

  ASSERT( SWS_INADDR_ANY != pClient->serverIpAddress);
  SWS_POKE8( pClient->buf, offset, SWS_DHCP_OPT_SERVERID);
  ++offset;
  SWS_POKE8( pClient->buf, offset, 4); /* length */
  ++offset;
  SWS_POKE32( pClient->buf, offset, pClient->serverIpAddress);
  offset += 4;

  SWS_POKE8( pClient->buf, offset, SWS_DHCP_OPT_END);
  ++offset;
  ASSERT( offset <= sizeof( pClient->buf) );

  /* Unicast message to DHCP server */
  from.sin_family = SWS_AF_INET;
  from.sin_addr.SWS_s_addr = SWS_htonl( pClient->serverIpAddress);
  from.sin_port = SWS_htons( SWS_DHCP_SERVER);
  iRet = SWS_sendto( pClient->s, pClient->buf, offset, 0,
    (struct SWS_sockaddr*)&from, sizeof(from) );
  if ( SWS_SOCKET_ERROR == iRet)
    {
    iRet = GetLastError();
    TRACE( 1, ("*** DhcpRelease() sendto failed %d\n", iRet));
    }
  else
    {
    ASSERT( offset == (unsigned)iRet);
    }

  DhcpUnconfigure( pClient);

  pClient->eState = kStateInitReboot;
  pClient->pfnState = &DhcpRequest;
  pClient->serverIpAddress = SWS_INADDR_ANY;

  return 0;
  }


/***************************************************************************
 * Support functions
 ***************************************************************************/

/*
 * Find a DHCP client for a given datalink
 */
SDhcpClient* DhcpFind( int iface)
  {
  SDhcpClient* pClient = s_pHead;
  while ( NULL != pClient)
    {
    if ( iface == pClient->iface)
      return pClient;
    }
  return NULL;
  }


/*
 * Destroy a DHCP client
 */
void DhcpDestroy( SDhcpClient* pClient)
  {
  TRACE( 9, ("DhcpDestroy(%p)\n", pClient));
  ASSERT( NULL != pClient);

  if ( SWS_INVALID_SOCKET != pClient->s)
    SWS_close( pClient->s);

  if ( NULL != pClient->hTimer)
    SWS_TimerRelease( pClient->hTimer);

  free( pClient);
  }


/*
 * Open a socket
 */
int DhcpOpenSocket( SDhcpClient* pClient)
  {
  int iRet, val;
  struct SWS_sockaddr_in from;

  TRACE( 9, ("DhcpOpenSocket(%p)\n", pClient));

  pClient->s = SWS_socket( SWS_PF_INET, SWS_SOCK_DGRAM, 0);
  if ( SWS_INVALID_SOCKET == pClient->s)
    {
    iRet = GetLastError();
    TRACE( 1, ("*** DhcpOpenSocket() socket failed %d\n", iRet));
    return iRet;
    }

  /* Enable broadcasts from the socket */
  val = 1;
  iRet = SWS_setsockopt(
    pClient->s,
    SWS_SOL_SOCKET, SWS_SO_BROADCAST,
    (char*)&val, sizeof(val)
  );
  if ( iRet)
    {
    iRet = GetLastError();
    TRACE( 1, ("*** DhcpOpenSocket() setsockopt failed %d\n", iRet));
    SWS_close( pClient->s);
    pClient->s = SWS_INVALID_SOCKET;
    return iRet;
    }

  /* Set the source port */
  from.sin_family = SWS_AF_INET;
  from.sin_addr.SWS_s_addr = SWS_htonl( SWS_INADDR_ANY);
  from.sin_port = SWS_htons( SWS_DHCP_CLIENT);
  iRet = SWS_bind( pClient->s, (struct SWS_sockaddr*)&from, sizeof(from) );
  if ( iRet)
    {
    iRet = GetLastError();
    TRACE( 1, ("*** DhcpOpenSocket() bind failed %d\n", iRet));
    SWS_close( pClient->s);
    pClient->s = SWS_INVALID_SOCKET;
    return iRet;
    }

  return 0;
  }


/*
 * Set IP parameters
 */
void DhcpConfigure( SDhcpClient* pClient)
  {
  int iRet;
#if OPT_DNS
  char szHost[ 256];
#endif

  TRACE( 9, ("DhcpConfigure(%p)\n", pClient));

  /* Set IP address for interface */
  TRACE( 2, ("DhcpConfigure() IP address %08lx\n", pClient->myIpAddress));
  ASSERT( pClient->myIpAddress);
  iRet = SWS_DatalinkSetProperty(
    pClient->iface,
    SWS_kDatalinkProperty_Address,
    &pClient->myIpAddress,
    sizeof( pClient->myIpAddress)
  );
  if ( iRet)
    TRACE( 1, ("*** DhcpConfigure() Failed(%d) to set IP address\n", iRet));

  if ( pClient->subnetMask)
    {
    /* Set subnet mask for interface */
    TRACE( 2, ("DhcpConfigure() Subnet mask %08lx\n", pClient->subnetMask));
    iRet = SWS_DatalinkSetProperty(
      pClient->iface,
      SWS_kDatalinkProperty_SubnetMask,
      &pClient->subnetMask,
      sizeof( pClient->subnetMask)
    );
    if ( iRet)
      TRACE( 1, ("*** DhcpConfigure() Failed(%d) to set subnet mask\n", iRet));
    }

  if ( SWS_INADDR_NONE != pClient->gatewayIpAddress)
    {
    /* Set IP address for interface */
    TRACE( 2, ("DhcpConfigure() Gateway %08lx\n", pClient->gatewayIpAddress));
    iRet = SWS_DatalinkSetProperty(
      pClient->iface,
      SWS_kDatalinkProperty_Gateway,
      &pClient->gatewayIpAddress,
      sizeof( pClient->gatewayIpAddress)
    );
    if ( iRet)
      TRACE( 1, ("*** DhcpConfigure() Failed(%d) to set gateway\n", iRet));
    }

#if OPT_DNS
  /* Set host name */
  if ( pClient->szHost[0]
    && (SWS_gethostname( szHost, sizeof(szHost)) || '0' == szHost[0])
  ) {
    TRACE( 2, ("DhcpConfigure() hostname: %s.%s\n", pClient->szHost, pClient->szDomain));
    SWS_DnsSetHostname( pClient->szHost, pClient->szDomain);
    }

  if ( SWS_INADDR_ANY != pClient->dnsIpAddress)
    {
    TRACE( 2, ("DhcpConfigure() DNS %08lx\n", pClient->dnsIpAddress));
    SWS_DnsSetPriServer( pClient->dnsIpAddress);
    SWS_RouteAdd( pClient->myIpAddress, pClient->dnsIpAddress, ~0UL, pClient->dnsIpAddress);
    }
#endif

  /* Reset the routing table */
  SWS_RouteReset();
  }


/*
 * Force IP parameters to unconfigured state
 */
void DhcpUnconfigure( SDhcpClient* pClient)
  {
  int iRet;
  ui32 const addr = SWS_INADDR_ANY;

  TRACE( 9, ("DhcpUnconfigure(%p)\n", pClient));

  if ( SWS_INVALID_SOCKET != pClient->s)
    {
    /* Close the socket in case bound to existing address */
    SWS_close( pClient->s);
    pClient->s = SWS_INVALID_SOCKET;
    }

  /* Set IP address to unconfigured
   * NB preserve myIpAddress */
  iRet = SWS_DatalinkSetProperty(
    pClient->iface,
    SWS_kDatalinkProperty_Address,
    &addr,
    sizeof( addr)
  );
  if ( iRet)
    TRACE( 1, ("*** DhcpUnconfigure() Failed(%d) to set IP address\n", iRet));

  /* Set subnet mask to default for class */
  pClient->subnetMask = 0;
  iRet = SWS_DatalinkSetProperty(
    pClient->iface,
    SWS_kDatalinkProperty_SubnetMask,
    &pClient->subnetMask,
    sizeof( pClient->subnetMask)
  );
  if ( iRet)
    TRACE( 1, ("*** DhcpUnconfigure() Failed(%d) to set subnet mask\n", iRet));

  /* Reset the gateway */
  pClient->gatewayIpAddress = SWS_INADDR_NONE;
  iRet = SWS_DatalinkSetProperty(
    pClient->iface,
    SWS_kDatalinkProperty_Gateway,
    &addr,
    sizeof( addr)
  );
  if ( iRet)
    TRACE( 1, ("*** DhcpUnconfigure() Failed(%d) to set gateway\n", iRet));

  pClient->dnsIpAddress = SWS_INADDR_ANY;
#if OPT_DNS
  SWS_DnsSetPriServer( pClient->dnsIpAddress);
#endif

  /* Reset the routing table */
  SWS_RouteReset();
  }


/*
 * Calc new retry interval
 */
int DhcpSetTimeout( SDhcpClient* pClient)
  {
  if ( 0 == pClient->timeout) switch ( rand() % 3)
    /* Initial retry period */
    {
  case 0:
    pClient->timeout = DHCP_TIMEOUT - 1;
    break;
  case 1:
    pClient->timeout = DHCP_TIMEOUT;
    break;
  default:
    pClient->timeout = DHCP_TIMEOUT + 1;
    break;
    }
  else if ( pClient->timeout <= DHCP_TIMEOUT + 1) switch ( rand() % 3)
    /* Second retry period */
    {
  case 0:
    pClient->timeout = 2 * DHCP_TIMEOUT - 1;
    break;
  case 1:
    pClient->timeout = 2 * DHCP_TIMEOUT;
    break;
  default:
    pClient->timeout = 2 * DHCP_TIMEOUT + 1;
    break;
    }
  else if ( pClient->timeout < DHCP_MAX_TIMEOUT)
    {
    /* Exponential backoff */
    pClient->timeout *= 2;
    if ( pClient->timeout > DHCP_MAX_TIMEOUT)
      pClient->timeout = DHCP_MAX_TIMEOUT;
    }
  else
    {
    return SWS_ETIMEDOUT;
    }
  return 0;
  }


/*
 * Format DHCP client request message
 */
int DhcpMessage( SDhcpClient* pClient, ui8 type, size_t* pOffset)
  {
  int iRet;
  size_t offset;

  TRACE( 9, ("DhcpMessage(%p,%u,%p)\n", pClient,type,pOffset));

  SWS_POKE8( pClient->buf, SWS_DHCP_OP, SWS_DHCP_OP_BOOTREQUEST);
  SWS_POKE8( pClient->buf, SWS_DHCP_HOPS, 0);
  SWS_POKE32( pClient->buf, SWS_DHCP_XID, pClient->xid);
  SWS_POKE16( pClient->buf, SWS_DHCP_SECS, pClient->secs );
  SWS_POKE16( pClient->buf, SWS_DHCP_FLAGS, 0); /* 1= broadcast bit */
  SWS_POKE32( pClient->buf, SWS_DHCP_CIADDR, 0);
  SWS_POKE32( pClient->buf, SWS_DHCP_YIADDR, 0);
  SWS_POKE32( pClient->buf, SWS_DHCP_SIADDR, 0);
  SWS_POKE32( pClient->buf, SWS_DHCP_GIADDR, 0);

  /* Get h/w type for interface */
  offset = sizeof( ui8);
  iRet = SWS_DatalinkGetProperty(
    pClient->iface,
    SWS_kDatalinkProperty_HwType,
    pClient->buf + SWS_DHCP_HTYPE,
    &offset
  );
  if ( iRet)
    {
    TRACE( 1, ("*** DhcpMessage() Failed(%d) to get h/w type\n", iRet));
    return iRet;
    }

  /* Get h/w address for interface */
  offset = SWS_DHCP_CHADDR_LEN;
  iRet = SWS_DatalinkGetProperty(
    pClient->iface,
    SWS_kDatalinkProperty_HwAddress,
    pClient->buf + SWS_DHCP_CHADDR,
    &offset
  );
  if ( iRet)
    {
    TRACE( 1, ("*** DhcpDiscover() Failed(%d) to get h/w address\n", iRet));
    return iRet;
    }
  SWS_POKE8( pClient->buf, SWS_DHCP_HLEN, (ui8)offset);

  memset( pClient->buf + SWS_DHCP_SNAME, 0, SWS_DHCP_SNAME_LEN);
  memset( pClient->buf + SWS_DHCP_FILE, 0, SWS_DHCP_FILE_LEN);

  /* Standard DHCP options */
  offset = SWS_DHCP_OPTS;
  SWS_POKE8( pClient->buf, offset, SWS_DHCP_COOKIE1);
  ++offset;
  SWS_POKE8( pClient->buf, offset, SWS_DHCP_COOKIE2);
  ++offset;
  SWS_POKE8( pClient->buf, offset, SWS_DHCP_COOKIE3);
  ++offset;
  SWS_POKE8( pClient->buf, offset, SWS_DHCP_COOKIE4);
  ++offset;

  SWS_POKE8( pClient->buf, offset, SWS_DHCP_OPT_MSGTYPE);
  ++offset;
  SWS_POKE8( pClient->buf, offset, 1); /* length */
  ++offset;
  SWS_POKE8( pClient->buf, offset, type);
  ++offset;

#if OPT_DHCP_PARAMS
  if ( SWS_DHCP_MSGTYPE_DHCPDISCOVER == type
    || SWS_DHCP_MSGTYPE_DHCPREQUEST == type
  ) {
    /* Want the following parameters */
    SWS_POKE8( pClient->buf, offset, SWS_DHCP_OPT_PARAMETERS);
    ++offset;
    SWS_POKE8( pClient->buf, offset, 5); /* length */
    ++offset;
    SWS_POKE8( pClient->buf, offset, SWS_DHCP_OPT_SUBNET_MASK);
    ++offset;
    SWS_POKE8( pClient->buf, offset, SWS_DHCP_OPT_ROUTER);
    ++offset;
    SWS_POKE8( pClient->buf, offset, SWS_DHCP_OPT_DNS);
    ++offset;
    SWS_POKE8( pClient->buf, offset, SWS_DHCP_OPT_HOST_NAME);
    ++offset;
    SWS_POKE8( pClient->buf, offset, SWS_DHCP_OPT_DOMAIN_NAME);
    ++offset;
    }
#endif

  *pOffset = offset;
  return 0;
  }


/*
 * Timer callback
 */
SWS_CB( void, DhcpTimeout( SWS_TimerH hTimer, SWS_PTR( void) pv) )
  {
  SDhcpClient* pClient = (SDhcpClient*)pv;
  (void)hTimer;

  TRACE( 9, ("DhcpTimeout(%p,%p)\n", hTimer, pv));

  ASSERT( NULL != pClient);
  ASSERT( pClient->pfnState);
  (*pClient->pfnState)( pClient);
  }

#else
char SWS_Dhcp;
#endif /* OPT_DHCP */
/* End of file */
