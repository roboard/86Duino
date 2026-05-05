/****************************************************************************
 *
 * File name   : sws_tprt.h
 * Function    : Transport modules public interface
 * Project     : SwsSock
 * Authors     : Lawrence Rust
 * Systems     : ANSI C
 * Version     : 1.0
 * Release Date:
 *
 ****************************************************************************
 * File History
 * ------------
 * 09-Apr-01, LVR, v1.00 - Created.
 * 12-Apr-05, LVR, v1.02 - Added.pszName to SWS_TransportDesc
 *
 *
 * Comments:
 * --------
 * This file contains the public interface to the IP module
 *
 ****************************************************************************/

#ifndef SWS_TPRT_H
#define SWS_TPRT_H 0x0102 /* Version no. BCD */

/* Dependencies */
#include "sws_host.h" /* Basic types */


/* Forward declarations */
struct SWS_Socket;
struct SWS_sockaddr;


/* Down calls */
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef SWS_CB( int, SWS_FTransportOpen(
  SWS_PTR( void),                     /* IN: Client context to SWS_TransportRegister */
  SWS_PTR( void),                     /* IN: -> SAP data */
  int,                                /* IN: socket type */
  int                                 /* IN: protocol */
));
typedef SWS_CB( int, SWS_FTransportClose( /* Returns -1 if pending */
  SWS_PTR( void),                     /* IN: -> SAP data */
  int                                 /* IN: 0= linger, !0= reset*/
));
typedef SWS_CB( int, SWS_FTransportBind(
  SWS_PTR( void),                     /* IN: -> SAP data */
  SWS_PTR( const struct SWS_sockaddr),/* IN: Name */
  int                                 /* IN: namelen */
));
typedef SWS_CB( int, SWS_FTransportListen(
  SWS_PTR( void)                      /* IN: -> SAP data */
));
typedef SWS_CB( int, SWS_FTransportConnect(
  SWS_PTR( void),                     /* IN: -> SAP data */
  SWS_PTR( const struct SWS_sockaddr),/* IN: Name */
  int                                 /* IN: namelen */
));
typedef SWS_CB( int, SWS_FTransportShutdown(
  SWS_PTR( void),                     /* IN: -> SAP data */
  int                                 /* IN: how, SWS_SD_xxx */
));
typedef SWS_CB( int, SWS_FTransportSendTo( /* Returns bytes sent, -1 on error */
  SWS_PTR( void),                     /* IN: -> SAP data */
  SWS_PTR( const void),               /* IN: -> data to send */
  SWS_PTR( unsigned),                 /* I/O: data len */
  unsigned,                           /* IN: flags, SWS_MSG_xxx */
  SWS_PTR( const struct SWS_sockaddr),/* IN: Optional name */
  int                                 /* IN: namelen */
));
typedef SWS_CB( int, SWS_FTransportRecvFrom( /* Returns bytes received, -1 on error */
  SWS_PTR( void),                     /* IN: -> SAP data */
  SWS_PTR( void),                     /* OUT: data received */
  SWS_PTR( unsigned),                 /* I/O: data len */
  unsigned,                           /* IN: flags, SWS_MSG_xxx */
  SWS_PTR( struct SWS_sockaddr),      /* OUT: Optional name */
  SWS_PTR( int)                       /* IN/OUT: namelen */
));
typedef SWS_CB( int, SWS_FTransportGetInfo(
  SWS_PTR( void),                     /* IN: -> SAP data */
  unsigned,                           /* IN: level SWS_SOL_xxx */
  unsigned,                           /* IN: option */
  SWS_PTR( void),                     /* OUT: -> value */
  SWS_PTR( int)                       /* I/O: length of value */
));
typedef SWS_CB( int, SWS_FTransportSetInfo(
  SWS_PTR( void),                     /* IN: -> SAP data */
  unsigned,                           /* IN: level SWS_SOL_xxx */
  unsigned,                           /* IN: option */
  SWS_PTR( const void),               /* IN: -> value */
  int                                 /* IN: length of value */
));
typedef SWS_CB( int, SWS_FTransportGetSockName(
  SWS_PTR( void),                     /* IN: -> SAP data */
  SWS_PTR( struct SWS_sockaddr),      /* OUT: name */
  SWS_PTR( int)                       /* IN/OUT: namelen */
));
typedef SWS_CB( int, SWS_FTransportGetPeerName(
  SWS_PTR( void),                     /* IN: -> SAP data */
  SWS_PTR( struct SWS_sockaddr),      /* OUT: name */
  SWS_PTR( int)                       /* IN/OUT: namelen */
));
typedef SWS_CB( void, SWS_FTransportCleanup(
  SWS_PTR( void)                      /* IN: Client context to SWS_TransportRegister */
));

#ifdef __cplusplus
}
#endif /* __cplusplus */


/*
 * Types
 */

/* Transport descriptor */
typedef struct SWS_TransportDesc
  {
  ui16 uSize;                         /* Set to sizeof(SWS_TransportDesc) */
  ui16 uVersion;                      /* Set to SWS_TRANSPORT_VERSION */
#define SWS_TRANSPORT_VERSION 0x0103

  ui8 uAddressFamily;                 /* SWS_AF_xxx type */
  ui8 uSocketType;                    /* SWS_SOCK_xxx type */
  ui8 uProtocol;                      /* SWS_IPPROTO_xxx type */
  ui8 dummy;
  SWS_PTR( const char) pszName;       /* Protocol name */

  /* Dispatch table */
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
  /* Add new down calls below */
  } SWS_TransportDesc;

/* Socket events */
typedef enum SWS_ETransportEvent
  {
  SWS_kTransportEventError = 0,       /* Serious error */
  SWS_kTransportEventRxReady,         /* Rx data available */
  SWS_kTransportEventTxReady,         /* Tx ready */
  SWS_kTransportEventConnect,         /* Connection complete */
  SWS_kTransportEventClose,           /* Pending close completion */
  SWS_kTransportEventOob              /* OOB data available */
  } SWS_ETransportEvent;


/*
 * Functions
 */
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Register a transport */
SWS_EXPORT( int, SWS_TransportRegister( /* Returns !0 on error */
  SWS_PTR( const SWS_TransportDesc),  /* IN: Transport descriptor */
  SWS_PTR( void),                     /* IN: Client context */
  unsigned,                           /* IN: No. SAP's */
  size_t                              /* IN: Sizeof each SAP */
));

/* Set/clear an event */
SWS_EXPORT( void, SWS_TransportEventSet(
  SWS_PTR( void),                     /* IN: -> SAP data */
  SWS_ETransportEvent,                /* IN: Event type */
  i32                                 /* IN: Event info */
));

/* Wait for an event */
SWS_EXPORT( int, SWS_TransportEventWait(  /* Returns !0 on error */
  SWS_PTR( void),                     /* IN: -> SAP data */
  SWS_ETransportEvent                 /* IN: Event type */
));

/* Incoming call from a listening SAP */
SWS_EXPORT( SWS_PTR( void), SWS_TransportCall( /* Returns new SAP, NULL on error */
  SWS_PTR( void),                     /* IN: -> listening SAP */
  SWS_PTR( int)                       /* OUT: error code */
));


/* Initialisation & cleanup */
extern int SWS_TransportInit( void);
extern void SWS_TransportCleanup( void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* ndef SWS_TPRT_H */
/* End of file */
