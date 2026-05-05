/****************************************************************************
 *
 * File name   : sws_upnp.h
 * Function    : Public API for the Universal Plug'n'Play client
 * Project     : SwsSock
 * Authors     : Lawrence Rust
 * Systems     : ANSI C
 * Version     : 1.0
 *
 ****************************************************************************
 * File History
 * ------------
 * 29-Aug-09, LVR, v1.00 - Created.
 *
 * Comments:
 * --------
 * This file contains the external interface to the client module
 *
 ****************************************************************************/

#ifndef SWS_UPNP_H
#define SWS_UPNP_H 0x0100 /* Version no. BCD */

/* Dependencies */
#include "stddef.h" /* size_t */
#include "sws_host.h" /* Basic types */


/* Constants */
#define SWS_UPNP_URLLEN 128 /* Max len of device and control URL's inc nul */
#define SWS_UPNP_ADDRLEN 32 /* Max len of IP address inc nul */

/* Types */
struct SWS_UpnpPortMap
  {
  char szRemote[ SWS_UPNP_ADDRLEN];   /* Remote IP address */
  unsigned uExtern;                   /* External port, 0= all 1..65535 */
  char szProto[ 8];                   /* Protocol TCP, UDP */
  unsigned uIntern;                   /* Internal port 1..65535 */
  char szClient[ SWS_UPNP_ADDRLEN];   /* Internal IP address */
  unsigned uEnabled;                  /* Boolean */
  unsigned uLease;                    /* Lease time in seconds. 0= static */
  char szInfo[ 32];                   /* Description */
  };

struct SWS_UpnpConnStatus
  {
  char szStatus[ 32];                 /* Connection status */
  char szError[ 32];                  /* Last connection error ERROR_xxx */
  unsigned long ulUptime;             /* Connection uptime seconds */
  };

struct SWS_UpnpLinkStatus
  {
  char szType[ 32];                   /* Access type */
  unsigned long ulUp;                 /* Max layer 1 upstream bit rate */
  unsigned long ulDown;               /* Max layer 1 downstream bit rate */
  char szStatus[ 32];                 /* Link status */

  unsigned long ulBytesSent;          /* Bytes sent */
  unsigned long ulBytesRecvd;         /* Bytes received */
  unsigned long ulPacketsSent;        /* Packets sent */
  unsigned long ulPacketsRecvd;       /* Packets recvd */
  };


/*
 * Functions
 */
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/** Locate a UPnP device */
SWS_EXTERN( int, SWS_UpnpLocateDevice( /* Returns !0 on error */
  const char [],                      /* IN: Device type */
  char [ SWS_UPNP_URLLEN]             /* OUT: Device URL */
));

/** Get the control URL for a UPnP service */
SWS_EXTERN( int, SWS_UpnpGetControl(
  const char [],                      /* IN: Device type */
  const char [],                      /* IN: Service name */
  char szURL[ SWS_UPNP_URLLEN]        /* OUT: Control URL */
));

/** Send a SOAP request to a UPnP device/service */
SWS_EXTERN( int, SWS_UpnpSoapAction(  /* Returns !0 on error */
  const char [],                      /* IN: Device type */
  const char [],                      /* IN: Service name */
  const char [],                      /* IN: Action name */
  const char [],                      /* IN: SOAP params */
  char [], size_t                     /* OUT: SOAP response */
));

/*
 * Internet Gateway Device operations
 */

/** Map an external port */
SWS_EXTERN( int, SWS_UpnpAddPortMap(  /* Returns !0 on error */
  const char [],                      /* IN: Protocol, UDP, TCP */
  unsigned,                           /* IN: Internal port 1..65535 */
  const char [],                      /* IN: Internal IP address in dotted form */
  unsigned,                           /* IN: External port, 0= all */
  const char []                       /* IN: Description, maybe NULL */
));

/** Unmap an external port */
#define SWS_UpnpDelPortMap( prtcl, ext)\
  SWS_UpnpAddPortMap( prtcl, 0, NULL, ext, NULL)

/** Get a port mapping */
SWS_EXTERN( int, SWS_UpnpGetPortMap(  /* Returns !0 on error */
  unsigned,                           /* IN: Index 0.. */
  SWS_PTR( struct SWS_UpnpPortMap)    /* OUT */
));

/** Get the IGD's external IP address */
SWS_EXTERN( int, SWS_UpnpGetExtIP(    /* Returns !0 on error */
  char [ SWS_UPNP_ADDRLEN]            /* OUT: IP address */
));

/** Get connection status */
SWS_EXTERN( int, SWS_UpnpGetConnStatus(  /* Returns !0 on error */
  SWS_PTR( struct SWS_UpnpConnStatus)    /* OUT */
));

/** Get link status */
SWS_EXTERN( int, SWS_UpnpGetLinkStatus(  /* Returns !0 on error */
  SWS_PTR( struct SWS_UpnpLinkStatus)    /* OUT */
));

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* ndef SWS_UPNP_H */
/* End of file */
