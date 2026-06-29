/*****************************************************************************
 *
 * File name   : sws_upnp.c
 * Function    : Universal Plug'N'Play (www.upnp.org) client
 * Authors     : Lawrence Rust
 * Systems     : ANSI C, BSD sockets
 *
 *****************************************************************************
 * File History
 * ------------
 *
 * 24-Aug-09, LVR, Created.
 *
 *
 * Comments
 * --------
 * See http://www.upnp.org/standardizeddcps/igd.asp
 *
 *****************************************************************************/

/* Compilation options */
#ifndef SWS_USE_SOCKLIB
#define SWS_USE_SOCKLIB 0 /* 0= native sockets, !0 for sws_sock */
#endif

#if defined _MSC_VER && _MSC_VER <= 800 /* VC++ 1.5 & earlier */
# pragma warning( disable : 4705)   /* statement has no effect */
#endif

/* Exports */
#include "sws_upnp.h"

/* Imports */

/* ANSI C library */
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <time.h>

#include "sws_net.h"
#include "sws_str.h"


/***************************************************************************
 * Macros & constants
 ***************************************************************************/
/* Expand and stringize a macro name */
#define STR(s) #s
#define STRING(s) STR(s)

/* Compile time assertion
 * Generates 'invalid array dimension' compiler error if condition false
 */
#undef STATIC_ASSERT
#ifdef __GNUC__
# define STATIC_ASSERT(e) extern char _dummy[(e) ? 1 : -1] __attribute__((unused))
#else
# define STATIC_ASSERT(e) extern char _dummy[(e) ? 1 : -1]
#endif

#if DEBUG
#define DBG( x) (x)
#else
#define DBG( x) ((void)0)
#endif

#define HTTP_CONNECT_TIMEOUT 4000
#define HTTP_RECV_TIMEOUT 2000

#define HTTP_SUCCESS 200
#define HTTP_ERROR_INTERN 501

#define HTTP_VER "1.0"
#define CRLF "\x0d\x0a"
#define CR '\x0d'
#define LF '\x0a'

/* See UPnP-arch-DeviceArchitecture-v1 1-20081015 para 1.3.2 */
#define UPNP_PORT 1900
#define UPNP_GROUP "239.255.255.250"
#define UPNP_TTL 2
#define UPNP_MX 1 /* Max secs to delay response 1..5 */

#define XML_HEADER          "<?xml version=\"1.0\"?>"
#define XMLTAG_SERVICETYPE  "<serviceType>"
#define XMLTAG_CONTROLURL   "<controlURL>"

/* UPnP_IGD_LANDevice 1.0 definitions */
#define UPNP_IGD_DEVICE      "urn:schemas-upnp-org:device:InternetGatewayDevice:1"
#define UPNP_WANIPCONNECTION "urn:schemas-upnp-org:service:WANIPConnection:1"
#define UPNP_WANCONFIG "urn:schemas-upnp-org:service:WANCommonInterfaceConfig:1"


/***************************************************************************
 * Data types
 ***************************************************************************/

/***************************************************************************
 * Local function prototypes
 ***************************************************************************/
static int HttpRequest(
  const char [], size_t,
  const struct sockaddr_in* to,
  char body[], size_t
);
static int HttpGetResponse( SOCKET, char buf[], size_t);
static int ReadLine( SOCKET, char buf[], size_t);

/***************************************************************************
 * Module data
 ***************************************************************************/

/***************************************************************************
 * Exported functions
 ***************************************************************************/

/*
 * Map an external port
 * UPnP_IGD_WANIPConnection 1.0
 */
SWS_EXTERN( int, SWS_UpnpAddPortMap(
  const char kszProto[],              /* IN: Protocol, 0= UDP, 1= TCP */
  unsigned intPort,                   /* IN: Local port */
  const char kszIntAddr[],            /* IN: Internal IP address */
  unsigned extPort,                   /* IN: External port */
  const char kszInfo[]
)){
  const char* pszAction;
  char buf[ 512];

  if ( NULL == kszProto)
    return EINVAL;
  else if ( 0 == SWS_stricmp( "TCP", kszProto))
    kszProto = "TCP";
  else if ( 0 == SWS_stricmp( "UDP", kszProto))
    kszProto = "UDP";
  else
    return EINVAL;

  if ( intPort > 65535U)
    return EINVAL;
  if ( extPort > 65535U)
    return EINVAL;

  /* Format SOAP parameters */
  if ( intPort)
    {
    in_addr_t addr;

    /* Validate client IP address */
    if ( NULL == kszIntAddr)
      return EINVAL;
    else if ( '\0' == kszIntAddr[0]
      || INADDR_ANY == (addr = inet_addr( kszIntAddr))
    ) {
      /* Use host's primary IP address */
      char szHost[ 256];
      struct hostent* pHost;

      if ( gethostname( szHost, sizeof szHost))
        return EINVAL;

      pHost = gethostbyname( szHost);
      if ( NULL == pHost)
        return EINVAL;

      kszIntAddr = inet_ntoa( *(const struct in_addr*)pHost->h_addr);
      }
    else if ( INADDR_NONE == addr)
      return EINVAL;

    pszAction = "AddPortMapping";
    sprintf( buf,
      "<NewRemoteHost></NewRemoteHost>"
      "<NewExternalPort>%u</NewExternalPort>"
      "<NewProtocol>%.7s</NewProtocol>"
      "<NewInternalPort>%u</NewInternalPort>"
      "<NewInternalClient>%.31s</NewInternalClient>"
      "<NewEnabled>1</NewEnabled>"
      "<NewPortMappingDescription>%.31s</NewPortMappingDescription>"
      "<NewLeaseDuration>0</NewLeaseDuration>",
      extPort, kszProto, intPort, kszIntAddr,
      (NULL != kszInfo && kszInfo[0]) ? kszInfo : "SWS_UPnP"
    );
    }
  else
    {
    pszAction = "DeletePortMapping";
    sprintf( buf,
      "<NewRemoteHost></NewRemoteHost>"
      "<NewExternalPort>%u</NewExternalPort>"
      "<NewProtocol>%.7s</NewProtocol>",
      extPort, kszProto
    );
    }

  /* Make the SOAP request */
  return SWS_UpnpSoapAction(
    UPNP_IGD_DEVICE, UPNP_WANIPCONNECTION,
    pszAction, buf,
    NULL, 0
  );
  }


/*
 * Get a port mapping
 * UPnP_IGD_WANIPConnection 1.0
 */
SWS_EXTERN( int, SWS_UpnpGetPortMap(  /* Returns !0 on error */
  unsigned idex,                      /* IN: Index */
  SWS_PTR( struct SWS_UpnpPortMap) pMap /* OUT */
)){
  int iRet;
  char buf[ 768], *psz;

  if ( NULL == pMap)
    return EINVAL;

  /* Format SOAP parameters */
  sprintf( buf, "<NewPortMappingIndex>%u</NewPortMappingIndex>", idex);

  /* Make the SOAP request */
  iRet = SWS_UpnpSoapAction(
    UPNP_IGD_DEVICE, UPNP_WANIPCONNECTION,
    "GetGenericPortMappingEntry", buf,
    buf, sizeof buf
  );
  if ( iRet)
    return iRet;

  /* Parse SOAP response */
#define UPNP_NEWREMHOST "<NewRemoteHost>"
  pMap->szRemote[0] = '\0';
  psz = strstr( buf, UPNP_NEWREMHOST);
  if ( NULL != psz)
    {
    STATIC_ASSERT( sizeof pMap->szRemote > 31);
    sscanf( psz, UPNP_NEWREMHOST " %31[^<]", pMap->szRemote);
    }

#define UPNP_NEWEXTPORT "<NewExternalPort>"
  pMap->uExtern = 0;
  psz = strstr( buf, UPNP_NEWEXTPORT);
  if ( NULL != psz)
    sscanf( psz, UPNP_NEWEXTPORT " %u", &pMap->uExtern);

#define UPNP_NEWPROTOCOL "<NewProtocol>"
  pMap->szProto[0] = '\0';
  psz = strstr( buf, UPNP_NEWPROTOCOL);
  if ( NULL != psz)
    {
    STATIC_ASSERT( sizeof pMap->szProto > 7);
    sscanf( psz, UPNP_NEWPROTOCOL " %7[^<]", pMap->szProto);
    }

#define UPNP_NEWINTPORT "<NewInternalPort>"
  pMap->uIntern = 0;
  psz = strstr( buf, UPNP_NEWINTPORT);
  if ( NULL != psz)
    sscanf( psz, UPNP_NEWINTPORT " %u", &pMap->uIntern);

#define UPNP_NEWINTCLIENT "<NewInternalClient>"
  pMap->szClient[0] = '\0';
  psz = strstr( buf, UPNP_NEWINTCLIENT);
  if ( NULL != psz)
    {
    STATIC_ASSERT( sizeof pMap->szClient > 31);
    sscanf( psz, UPNP_NEWINTCLIENT " %31[^<]", pMap->szClient);
    }

#define UPNP_NEWENABLED "<NewEnabled>"
  pMap->uEnabled = 0;
  psz = strstr( buf, UPNP_NEWENABLED);
  if ( NULL != psz)
    sscanf( psz, UPNP_NEWENABLED " %u", &pMap->uEnabled);

#define UPNP_NEWPMDESC "<NewPortMappingDescription>"
  pMap->szInfo[0] = '\0';
  psz = strstr( buf, UPNP_NEWPMDESC);
  if ( NULL != psz)
    {
    STATIC_ASSERT( sizeof pMap->szInfo > 31);
    sscanf( psz, UPNP_NEWPMDESC " %31[^<]", pMap->szInfo);
    }

#define UPNP_NEWLEASE "<NewLeaseDuration>"
  pMap->uLease = 0;
  psz = strstr( buf, UPNP_NEWLEASE);
  if ( NULL != psz)
    sscanf( psz, UPNP_NEWLEASE " %u", &pMap->uLease);

  return 0;
  }


/*
 * Get connection status
 * UPnP_IGD_WANIPConnection 1.0
 */
SWS_EXTERN( int, SWS_UpnpGetConnStatus(  /* Returns !0 on error */
  SWS_PTR( struct SWS_UpnpConnStatus) pInfo /* OUT */
)){
  int iRet;
  char buf[ 512], *psz;

  if ( NULL == pInfo)
    return EINVAL;

  /* Make the SOAP request */
  iRet = SWS_UpnpSoapAction(
    UPNP_IGD_DEVICE, UPNP_WANIPCONNECTION,
    "GetStatusInfo", "",
    buf, sizeof buf
  );
  if ( iRet)
    return iRet;

  /* Parse SOAP response */
#define UPNP_NEWCONNSTAT "<NewConnectionStatus>"
  pInfo->szStatus[0] = '\0';
  psz = strstr( buf, UPNP_NEWCONNSTAT);
  if ( NULL != psz)
    {
    STATIC_ASSERT( sizeof pInfo->szStatus > 31);
    sscanf( psz, UPNP_NEWCONNSTAT " %31[^<]", pInfo->szStatus);
    }

#define UPNP_NEWCONNERR "<NewLastConnectionError>"
  pInfo->szError[0] = '\0';
  psz = strstr( buf, UPNP_NEWCONNERR);
  if ( NULL != psz)
    {
    STATIC_ASSERT( sizeof pInfo->szError > 31);
    sscanf( psz, UPNP_NEWCONNERR " %31[^<]", pInfo->szError);
    }

#define UPNP_NEWUPTIME "<NewUptime>"
  pInfo->ulUptime = 0;
  psz = strstr( buf, UPNP_NEWUPTIME);
  if ( NULL != psz)
    sscanf( psz, UPNP_NEWUPTIME " %lu", &pInfo->ulUptime);

  return 0;
  }


/*
 * Get link status
 */
SWS_EXTERN( int, SWS_UpnpGetLinkStatus(  /* Returns !0 on error */
  SWS_PTR( struct SWS_UpnpLinkStatus) pLink /* OUT */
)){
  int iRet;
  char buf[ 768], *psz;

  if ( NULL == pLink)
    return EINVAL;

  /* Make the SOAP request */
  iRet = SWS_UpnpSoapAction(
    UPNP_IGD_DEVICE, UPNP_WANCONFIG,
    "GetCommonLinkProperties", "",
    buf, sizeof buf
  );
  if ( iRet)
    return iRet;

  /* Parse SOAP response */
#define UPNP_NEWACCESSTYPE "<NewWANAccessType>"
  pLink->szType[0] = '\0';
  psz = strstr( buf, UPNP_NEWACCESSTYPE);
  if ( NULL != psz)
    {
    STATIC_ASSERT( sizeof pLink->szType > 31);
    sscanf( psz, UPNP_NEWACCESSTYPE " %31[^<]", pLink->szType);
    }

#define UPNP_NEWUPSTREAM "<NewLayer1UpstreamMaxBitRate>"
  pLink->ulUp = 0;
  psz = strstr( buf, UPNP_NEWUPSTREAM);
  if ( NULL != psz)
    sscanf( psz, UPNP_NEWUPSTREAM " %lu", &pLink->ulUp);

#define UPNP_NEWDNSTREAM "<NewLayer1DownstreamMaxBitRate>"
  pLink->ulDown = 0;
  psz = strstr( buf, UPNP_NEWDNSTREAM);
  if ( NULL != psz)
    sscanf( psz, UPNP_NEWDNSTREAM " %lu", &pLink->ulDown);

#define UPNP_NEWLINKSTATUS "<NewPhysicalLinkStatus>"
  pLink->szStatus[0] = '\0';
  psz = strstr( buf, UPNP_NEWLINKSTATUS);
  if ( NULL != psz)
    {
    STATIC_ASSERT( sizeof pLink->szStatus > 31);
    sscanf( psz, UPNP_NEWLINKSTATUS " %31[^<]", pLink->szStatus);
    }

  iRet = SWS_UpnpSoapAction(
    UPNP_IGD_DEVICE, UPNP_WANCONFIG,
    "GetTotalBytesSent", "",
    buf, sizeof buf
  );
  if ( iRet)
    return iRet;

#define UPNP_NEWBYTESSENT "<NewTotalBytesSent>"
  pLink->ulBytesSent = 0;
  psz = strstr( buf, UPNP_NEWBYTESSENT);
  if ( NULL != psz)
    sscanf( psz, UPNP_NEWBYTESSENT " %lu", &pLink->ulBytesSent);

  iRet = SWS_UpnpSoapAction(
    UPNP_IGD_DEVICE, UPNP_WANCONFIG,
    "GetTotalBytesReceived", "",
    buf, sizeof buf
  );
  if ( iRet)
    return iRet;

#define UPNP_NEWBYTESRECVD "<NewTotalBytesReceived>"
  pLink->ulBytesRecvd = 0;
  psz = strstr( buf, UPNP_NEWBYTESRECVD);
  if ( NULL != psz)
    sscanf( psz, UPNP_NEWBYTESRECVD " %lu", &pLink->ulBytesRecvd);

  iRet = SWS_UpnpSoapAction(
    UPNP_IGD_DEVICE, UPNP_WANCONFIG,
    "GetTotalPacketsSent", "",
    buf, sizeof buf
  );
  if ( iRet)
    return iRet;

#define UPNP_NEWPACKETSSENT "<NewTotalPacketsSent>"
  pLink->ulPacketsSent = 0;
  psz = strstr( buf, UPNP_NEWPACKETSSENT);
  if ( NULL != psz)
    sscanf( psz, UPNP_NEWPACKETSSENT " %lu", &pLink->ulPacketsSent);

  iRet = SWS_UpnpSoapAction(
    UPNP_IGD_DEVICE, UPNP_WANCONFIG,
    "GetTotalPacketsReceived", "",
    buf, sizeof buf
  );
  if ( iRet)
    return iRet;

#define UPNP_NEWPACKETSRECVD "<NewTotalPacketsReceived>"
  pLink->ulPacketsRecvd = 0;
  psz = strstr( buf, UPNP_NEWPACKETSRECVD);
  if ( NULL != psz)
    sscanf( psz, UPNP_NEWPACKETSRECVD " %lu", &pLink->ulPacketsRecvd);

  return 0;
  }


/*
 * Get external IP address
 * UPnP_IGD_WANIPConnection 1.0
 */
SWS_EXTERN( int, SWS_UpnpGetExtIP(      /* Returns !0 on error */
  char szAddr[ SWS_UPNP_ADDRLEN]
)){
  int iRet;
  char szReply[ 384], *psz;

  /* Make the SOAP request */
  iRet = SWS_UpnpSoapAction(
    UPNP_IGD_DEVICE, UPNP_WANIPCONNECTION,
    "GetExternalIPAddress", "",
    szReply, sizeof szReply
  );
  if ( iRet)
    return iRet;

  /* Parse SOAP response */
#define UPNP_NEWEXTIP "<NewExternalIPAddress>"
  szAddr[ 0] = '\0';
  psz = strstr( szReply, UPNP_NEWEXTIP);
  if ( NULL != psz)
    {
    STATIC_ASSERT( SWS_UPNP_ADDRLEN > 31);
    sscanf( psz, UPNP_NEWEXTIP " %31[^<]", szAddr);
    }
  else
    {
    DBG( fputs( "SWS_UpnpGetExtIP missing IP address\n", stderr));
    return EINVAL;
    }

  return 0;
  }


/*
 * Send a SOAP request to a PnP device/service
 */
SWS_EXTERN( int, SWS_UpnpSoapAction(
  const char kszDevice[],
  const char kszService[],
  const char kszAction[],
  const char kszParams[],
  char szResult[], size_t sizeRes
)){
  int iRet, n, nHead;
  char szControl[ SWS_UPNP_URLLEN], szHost[ SWS_UPNP_ADDRLEN];
  struct sockaddr_in server;
  unsigned u;
  const size_t size = 8192;
  char* buf;

  if ( 0 != (iRet = SWS_UpnpGetControl( kszDevice, kszService, szControl)))
    return iRet;

  if ( 2 > sscanf( szControl, " http://%31[^:]:%u%n", szHost, &u, &nHead))
    {
    STATIC_ASSERT( sizeof szHost > 31);
    DBG( fprintf( stderr, "SWS_UpnpSoapAction invalid control URL: %s\n",
      szControl));
    return EINVAL;
    }

  server.sin_family = AF_INET;
  server.sin_port = htons( (u_short)u);
  /* NB szHost could be a name */
  server.sin_addr.s_addr = inet_addr( szHost);

  /* Alloc a buffer for the SOAP request*/
  buf = (char*) malloc( 2 * size);
  if ( NULL == buf)
    return ENOMEM;
  else do
    {
    char *pszSoap = &buf[ size];

    /* Format the SOAP request */
    n = sprintf( pszSoap,
      XML_HEADER CRLF
      "<s:Envelope"
        " xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\""
        " s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\">"
        "<s:Body>"
          "<u:%.31s xmlns:u=\"%.127s\">"
          "%.7000s"
          "</u:%.31s>"
        "</s:Body>"
      "</s:Envelope>" CRLF,
      kszAction, kszService, kszParams, kszAction
    );

    /* Post the SOAP request */
    n = sprintf( buf,
      "POST %.127s HTTP/" HTTP_VER CRLF
      "HOST: %.31s:%u" CRLF
      "CONTENT-LENGTH: %d" CRLF
      "CONTENT-TYPE: text/xml" CRLF
      "SOAPACTION: \"%.127s#%.31s\"" CRLF
      CRLF
      "%.7500s",
      &szControl[ nHead],
      szHost, u,
      n, kszService, kszAction,
      pszSoap
    );
    iRet = HttpRequest( buf, n, &server, szResult, sizeRes);
    if ( SOCKET_ERROR == iRet)
      {
      iRet = GetLastError();
      break;
      }
    else if ( 0 == iRet)
      {
      DBG( fputs( "SWS_UpnpSoapAction missing SOAP response\n", stderr));
      iRet = EINVAL;
      break;
      }
    else if ( HTTP_SUCCESS != iRet)
      {
      if ( iRet < HTTP_SUCCESS)
        DBG( fprintf( stderr, "SWS_UpnpSoapAction:%s http error %d \n",
          kszAction, iRet));
      break;
      }
    iRet = 0;
    }
  while ( 0);
  free( buf);

  return iRet;
  }


/*
 * Get the control URL for a PnP service
 */
SWS_EXTERN( int, SWS_UpnpGetControl(
  const char kszDevice[],
  const char kszService[],
  char szURL[ SWS_UPNP_URLLEN]        /* OUT: Control URL */
)){
  int iRet, n, nHead;
  char szHost[ SWS_UPNP_ADDRLEN];
  struct sockaddr_in server;
  unsigned u;
  const size_t size = 8192;
  char* buf;
  static char s_szLastDevice[ SWS_UPNP_URLLEN],
    s_szLastService[ SWS_UPNP_URLLEN], s_szLastURL[ SWS_UPNP_URLLEN];
  static time_t s_tValid;

  /* Is device/service cached? */
  if ( 0 == strcmp( s_szLastDevice, kszDevice)
    && 0 == strcmp( s_szLastService, kszService)
    && time( NULL) < s_tValid /* Cache valid timeout */
  ) {
    strcpy( szURL, s_szLastURL);
    return 0;
    }

  if ( 0 != (iRet = SWS_UpnpLocateDevice( kszDevice, szURL)))
    return iRet;

  if ( 2 > sscanf( szURL, " http://%31[^:]:%u%n", szHost, &u, &nHead))
    {
    STATIC_ASSERT( sizeof szHost > 31);
    DBG( fprintf( stderr, "SWS_UpnpGetControl invalid device URL: %s\n", szURL));
    return EINVAL;
    }

  server.sin_family = AF_INET;
  server.sin_port = htons( (u_short)u);
  /* NB szHost could be a name */
  server.sin_addr.s_addr = inet_addr( szHost);

  /* Alloc a buffer for the device description */
  buf = (char*) malloc( size);
  if ( NULL == buf)
    return ENOMEM;
  else do
    {
    char* psz;
    size_t len;

    /* Request the device description from the device URL */
    n = sprintf( buf,
      "GET %.127s HTTP/" HTTP_VER CRLF
      "HOST: %.31s:%u" CRLF
      CRLF,
      &szURL[ nHead],
      szHost, u
    );
    n = HttpRequest( buf, n, &server, buf, size);
    if ( SOCKET_ERROR == n)
      {
      iRet = GetLastError();
      break;
      }
    else if ( 0 == n)
      {
      DBG( fputs( "SWS_UpnpGetControl missing device description\n", stderr));
      iRet = EINVAL;
      break;
      }
    else if ( HTTP_SUCCESS != n)
      {
      DBG( fprintf( stderr, "SWS_UpnpGetControl:%s http error %d\n",
        kszDevice, n));
      break;
      }

    /* Parse XML body */
    if ( strncmp( buf, XML_HEADER, sizeof( XML_HEADER) - 1))
      {
      DBG( fputs( "SWS_UpnpGetControl invalid XML\n", stderr));
      iRet = EINVAL;
      break;
      }

    len = strlen( kszService);
    psz = buf;
    while ( NULL != (psz = strstr( psz, XMLTAG_SERVICETYPE)))
      {
      psz += sizeof( XMLTAG_SERVICETYPE) -1;
      if ( 0 == strncmp( kszService, psz, len))
        break;
      }
    if ( NULL == psz)
      {
      DBG( fprintf( stderr, "SWS_UpnpGetControl no service: %s\n", kszService));
      iRet = EINVAL;
      break;
      }
    else if ( NULL == (psz = strstr( psz, XMLTAG_CONTROLURL))
      || 1 != sscanf( psz, XMLTAG_CONTROLURL " %127[^<]", &szURL[ nHead])
    ) {
      STATIC_ASSERT( SWS_UPNP_URLLEN > 127);
      DBG( fprintf( stderr, "SWS_UpnpGetControl no control for: %s\n",
        kszService));
      iRet = EINVAL;
      break;
      }

    /* Cache the URL */
    strncpy( s_szLastDevice, kszDevice, sizeof s_szLastDevice);
    strncpy( s_szLastService, kszService, sizeof s_szLastService);
    strcpy( s_szLastURL, szURL);
    s_tValid = time( NULL) + 30*60; /* Cache timeout */

    iRet = 0;
    }
  while ( 0);
  free( buf);
  return iRet;
  }


/*
 * Locate a PnP device
 */
SWS_EXTERN( int, SWS_UpnpLocateDevice(
  const char szType[],
  char szURL[ SWS_UPNP_URLLEN]
)){
  int iRet, iTry;
  SOCKET s;
  static char s_szLastType[ SWS_UPNP_URLLEN], s_szLastURL[ SWS_UPNP_URLLEN];
  static time_t s_tValid;

  /* Is device cached? */
  if ( 0 == strcmp( s_szLastType, szType)
    && time( NULL) < s_tValid /* Cache valid timeout */
  ) {
    strcpy( szURL, s_szLastURL);
    return 0;
    }

  /* Open UDP socket */
  s = socket( PF_INET, SOCK_DGRAM, 0);
  if ( INVALID_SOCKET == s)
    {
    DBG( fprintf( stderr, "SWS_UpnpLocateDevice socket failed: %s\n",
      strerror( GetLastError())));
    return SOCKET_ERROR;
    }

#ifdef IP_DONTFRAGMENT
  /* Set the DF flag */
  iRet = 1;
  setsockopt(
    s,
    IPPROTO_IP, IP_DONTFRAGMENT,
    (char*)&iRet, sizeof iRet
  );
#endif

#ifdef IP_TTL
  /* Set the Time To Live */
  iRet = UPNP_TTL;
  setsockopt(
    s,
    IPPROTO_IP, IP_TTL,
    (char*)&iRet, sizeof iRet
  );
#endif

  /* Set a recv timeout */
  { DECL_TIMEOUT( tv, (UPNP_MX * 1000 + 100));
  setsockopt(
    s,
    SOL_SOCKET, SO_RCVTIMEO,
    (char*)&tv, sizeof tv
  ); }

  for ( iTry = 0; iTry < 3; ++iTry)
    {
    struct sockaddr_in group;
    char *psz;
    int n;
    char buf[ 512];

    /* Send an SSDP discovery message for the device */
    n = sprintf( buf,
      "M-SEARCH * HTTP/1.1" CRLF
      "HOST: " UPNP_GROUP ":" STRING( UPNP_PORT) CRLF
      "MAN: \"ssdp:discover\"" CRLF
      "MX: " STRING( UPNP_MX) CRLF /* Max secs to delay response */
      "ST: %.127s" CRLF /* search type */
      CRLF,
      szType
    );
    group.sin_family = AF_INET;
    group.sin_port = htons( UPNP_PORT);
    group.sin_addr.s_addr = inet_addr( UPNP_GROUP);
    if ( SOCKET_ERROR == sendto( s, buf, n, 0,
        (const struct sockaddr*)&group, sizeof group)
    ) {
      DBG( fprintf( stderr, "SWS_UpnpLocateDevice sendto failed: %s\n",
        strerror( GetLastError())));
      iRet = SOCKET_ERROR;
      break;
      }

    /* Get the reply */
    do
      {
      struct sockaddr_in from;
      socklen_t sl = sizeof from;
      n = recvfrom( s, buf, sizeof buf, 0, (struct sockaddr*)&from, &sl);
      if ( SOCKET_ERROR == n)
        break;
      }
    while ( 'M' == buf[0]); /* Probably an echo of our request */
    if ( SOCKET_ERROR == n)
      {
      iRet = GetLastError();
      if ( EAGAIN == iRet || EWOULDBLOCK == iRet)
        continue;
      DBG( fprintf( stderr, "SWS_UpnpLocateDevice recvfrom failed(%d): %s\n",
        iRet, strerror( iRet)));
      iRet = SOCKET_ERROR;
      break;
      }

    /* Verify HTTP response */
    buf[n] = '\0';
    if ( 1 > sscanf( buf, " HTTP/%*u.%*u %d", &n))
      {
      DBG( fputs( "SWS_UpnpLocateDevice invalid SSDP header\n", stderr));
      continue;
      }
    if ( HTTP_SUCCESS != n)
      {
      DBG( fprintf( stderr, "SWS_UpnpLocateDevice SSDP error %d\n", n));
      continue;
      }

    /* Parse the response for a Location URL */
    for ( psz = buf; NULL != psz; psz = strstr( psz, CRLF))
      {
      STATIC_ASSERT( SWS_UPNP_URLLEN > 127);
      while ( isspace( *psz))
        ++psz;
      if ( 0 == SWS_strnicmp( psz, "LOCATION", 8)
        && 1 == sscanf( psz + 8, " : %127s", szURL)
      )
        break;
      }
    if ( NULL != psz)
      {
      /* Cache the location */
      strncpy( s_szLastType, szType, sizeof s_szLastType);
      strcpy( s_szLastURL, szURL);
      s_tValid = time( NULL) + 30*60; /* Cache timeout */

      /* Success */
      iRet = 0;
      break;
      }

    DBG( fputs( "SWS_UpnpLocateDevice No Location field found\n", stderr));
    iRet = EINVAL;
    }

  if ( EAGAIN == iRet || EWOULDBLOCK == iRet || ETIMEDOUT == iRet)
    DBG( fprintf( stderr, "SWS_UpnpLocateDevice:%s not found\n", szType));

  closesocket( s);
  return iRet;
  }


/***************************************************************************
 * Local support functions
 ***************************************************************************/

/*
 * Send an http request and receive response
 */
int HttpRequest(
  const char req[], size_t sizeRq,
  const struct sockaddr_in* psaTo,
  char body[], size_t sizeBody
) {
  SOCKET s;
  int iRet;

  s = socket( PF_INET, SOCK_STREAM, 0);
  if ( INVALID_SOCKET == s)
    {
    DBG( fprintf( stderr, "SWS_UpnpHttpRequest socket failed: %s\n",
      strerror( GetLastError())));
    return SOCKET_ERROR;
    }

  /* Set recv timeout */
  { DECL_TIMEOUT( tv, HTTP_RECV_TIMEOUT);
  setsockopt(
    s,
    SOL_SOCKET, SO_RCVTIMEO,
    (char*)&tv, sizeof tv
  ); }

  /* Set send/connect timeout */
  { DECL_TIMEOUT( tv, HTTP_CONNECT_TIMEOUT);
  setsockopt(
    s,
    SOL_SOCKET, SO_SNDTIMEO,
    (char*)&tv, sizeof tv
  ); }

  /* Connect to server */
  if ( connect( s, (const struct sockaddr*)psaTo, sizeof *psaTo))
    {
    DBG( fprintf( stderr, "SWS_UpnpHttpRequest connect failed: %s\n",
      strerror( GetLastError())));
    iRet = SOCKET_ERROR;
    }
  /* Send the request */
  else if ( SOCKET_ERROR == (iRet = send( s, req, sizeRq, 0)))
    {
    DBG( fprintf( stderr, "SWS_UpnpHttpRequest send failed: %s\n",
      strerror( GetLastError())));
    }
  else if ( sizeRq != (size_t)iRet)
    {
    DBG( fprintf( stderr, "SWS_UpnpHttpRequest disconnect in send\n"));
    iRet = 0;
    }
  else
    {
    /* Get the response */
    iRet = HttpGetResponse( s, body, sizeBody);
    }

  closesocket( s);
  return iRet;
  }


/*
 * Parse an http response
 */
int HttpGetResponse( SOCKET s, char body[], size_t size)
  {
  int n, code;
  unsigned len;
  char buff[ 128];

  if ( size)
    body[0] = '\0';

  n = ReadLine( s, buff, sizeof buff);
  if ( SOCKET_ERROR == n)
    return SOCKET_ERROR;
  if ( 0 == n)
    return 0;

  /* Verify HTTP response header */
  if ( 1 > sscanf( buff, " HTTP/%*u.%*u %d", &code))
    {
    DBG( fputs( "SWS_UpnpHttpGetResponse invalid HTTP header\n", stderr));
    return HTTP_ERROR_INTERN;
    }

  /* Get rest of header */
  len = size ? size - 1 : 0;
  for (;;)
    {
    char* psz;

    n = ReadLine( s, buff, sizeof buff);
    if ( SOCKET_ERROR == n)
      return SOCKET_ERROR;
    if ( 0 == n)
      return code;

    if ( '\0' == buff[0])
      break;

    psz = buff;
    while ( isspace( *psz))
      ++psz;

    if ( size
      && 0 == SWS_strnicmp( psz, "CONTENT-LENGTH", 14)
      && 1 == sscanf( psz + 14, " : %u", &len)
      && len >= size
    ) {
      len = size -1;
      DBG( fputs( "SWS_UpnpHttpGetResponse buffer too small\n", stderr));
      if ( HTTP_SUCCESS == code)
        code = ENOMEM;
      }
    }

  /* Get body */
  while ( len)
    {
    n = recv( s, body, len, 0);
    if ( SOCKET_ERROR == n)
      {
      DBG( fprintf( stderr, "SWS_UpnpHttpRequest recv failed: %s\n",
        strerror( GetLastError())));
      return SOCKET_ERROR;
      }
    if ( 0 == n)
      break;
    body += n;
    *body = '\0';
    len -= n;
    }

  return code;
  }


/*
 * Read a line
 */
int ReadLine( SOCKET s, char buf[], size_t size)
  {
  int iRet, isCR = 0;
  size_t count = 0;

  while ( count + 1 < size)
    {
    unsigned char c;

    iRet = recv( s, (char*)&c, 1, 0);
    if ( SOCKET_ERROR == iRet)
      {
      DBG( fprintf( stderr, "SWS_UpnpReadLine recv failed: %s\n",
        strerror( GetLastError())));
      return SOCKET_ERROR;
      }
    if ( 0 == iRet)
      return 0; /* Disconnect */

    if ( isCR)
      {
      isCR = 0;
      switch ( c)
        {
      case LF:
        goto done;

      default:
        if ( count + 3 <= size )
          {
          buf[ count++] = CR;
          buf[ count++] = (char)c;
          }
        break;
        }
      }
    else switch ( c)
      {
    default:
      if ( count + 2 <= size )
        buf[ count++] = (char)c;
      break;

    case CR:
      isCR = 1;
      break;

    case LF:
      goto done;
      }
    }

done:
  buf[ iRet = count] = '\0';

  return (int)(iRet + 1); /* Including CR */
  }

/* End of file */
