/****************************************************************************
 *
 * File name   : p2p.h
 * Function    : API for Point to Point adapters and protocols
 * Project     : SwsSock
 * Authors     : Lawrence Rust
 * Systems     : ANSI C
 *
 ****************************************************************************
 * File History
 * ------------
 * 18-Jul-05, LVR, v1.00 - Created.
 *
 * Comments:
 * --------
 * This file contains the external interface to the P2P adapter manager
 *
 ****************************************************************************/

#ifndef SWS_P2P_PRIV_H
#define SWS_P2P_PRIV_H 0x0100 /* Version no. BCD */

/* Dependencies */
#include <stddef.h>   /* size_t */
#include "sws_host.h" /* Basic types */
#include "sws_buf.h"


/*
 * Macros and constants
 */

/* SWS_FP2pStatInd up call events */
enum SWS_EP2pStatus {
  SWS_kP2pStatusCD,
  SWS_kP2pStatusRI,
  SWS_kP2pStatusDSR
};

/* SWS_FP2pIoctl types */
enum
  {
  SWS_kP2pIoctl_SetOnLine = 0x1000,   /* ui32 (w/o) !0 to go online */
  SWS_kP2pIoctl_GetBaud,              /* ui32 (r/o) */
  SWS_kP2pIoctl_GetModemStatus        /* ui8 (r/o) SWS_P2P_MODEM_xxx bits */
  };

/* SWS_kP2pIoctl_GetModemStatus bits */
#define SWS_P2P_MODEM_DCD 1
#define SWS_P2P_MODEM_RNG 2
#define SWS_P2P_MODEM_DSR 4

/* SWS_FP2pProtoLinkInd link state */
enum SWS_EP2pLink
  {
  SWS_kP2pLinkGoingDown = -1,
  SWS_kP2pLinkDown,
  SWS_kP2pLinkUp
  };


/*
 * Types
 */

/* Adapter interface */
/*********************/

/* Adapter down calls */

/* Transmit data */
typedef SWS_CB( int, SWS_FP2pSend(    /* Return !0 on error, -1 if pending */
  SWS_PTR( void),                     /* IN: Driver's context */
  SWS_PTR( const void),               /* IN: Data to send */
  size_t                              /* IN: Byte count */
));

/* Post a receive data buffer */
typedef SWS_CB( int, SWS_FP2pRecv(    /* Return !0 on error, -1 if pending */
  SWS_PTR( void),                     /* IN: Driver's context */
  SWS_PTR( void),                     /* IN: Buffer */
  SWS_PTR( size_t),                   /* I/O: Buffer size, bytes received */
  int,                                /* IN: Delimiter, -ve if none */
  ui32                                /* IN: Timeout, 0= none, 1= immediate */
));

/* Shutdown the adapter */
typedef SWS_CB( void, SWS_FP2pShutdown(
  SWS_PTR( void)                      /* IN: Client context */
));

/* Generic ioctl */
typedef SWS_CB( int, SWS_FP2pIoctl(   /* Returns !0 on error */
  SWS_PTR( void),                     /* IN: Client context */
  unsigned,                           /* IN: Type */
  SWS_PTR( void),                     /* I/O: -> data */
  SWS_PTR( size_t)                    /* I/O: data size */
));


/* Adapter up calls - May be called in ANY context including IRQ */

/* Pending send complete.  Driver is ready for more data */
typedef SWS_CB( void, SWS_FP2pSendAck(
  SWS_PTR( void),                     /* IN: Up call context */
  int                                 /* IN: Final error code, 0= no error */
));

/* Pending receive complete */
typedef SWS_CB( void, SWS_FP2pRecvInd(
  SWS_PTR( void),                     /* IN: Up call context */
  size_t,                             /* IN: No. bytes received */
  int                                 /* IN: Final error code, 0= no error */
));

/* Device status indication */
typedef SWS_CB( void, SWS_FP2pStatInd(
  SWS_PTR( void),                     /* IN: Up call context */
  int,                                /* IN: SWS_EP2pStatus */
  int                                 /* IN: New state */
));


/* P2P adapter descriptor */
typedef struct SWS_P2pDevice
  {
  ui16 uSize;                         /* Set to sizeof(SWS_P2pDevice) */
  ui16 uVersion;                      /* Set to SWS_P2P_VERSION */
#define SWS_P2P_VERSION 0x101

  SWS_PTR( const char) pszIdent;      /* Ident string used for config & SWS_DatalinkFind */

  /* Down call table */
  SWS_FNPTR( SWS_FP2pSend) pfnSend;          /* Required */
  SWS_FNPTR( SWS_FP2pRecv) pfnRecv;          /* Required */
  SWS_FNPTR( SWS_FP2pShutdown) pfnShutdown;  /* 0 if not implemented */
  SWS_FNPTR( SWS_FP2pIoctl) pfnIoctl;        /* 0 if not implemented */
  } SWS_P2pDevice;


/* Callbacks from adapter to P3P */
typedef struct SWS_P2pCallbacks
  {
  SWS_PTR( void) pvContext;           /* Context for up calls */

  SWS_FNPTR( SWS_FP2pSendAck) pfnSendAck;
  SWS_FNPTR( SWS_FP2pRecvInd) pfnRecvInd;
  SWS_FNPTR( SWS_FP2pStatInd) pfnStatInd;
  } SWS_P2pCallbacks;


/* Protocol interface */
/**********************/

/* Calls from protocol to adapter */
typedef struct SWS_P2pProtocolDn
  {
  SWS_PTR( void) pvContext;           /* Context for calls */

  SWS_FNPTR( SWS_FP2pSend) pfnSend;
  SWS_FNPTR( SWS_FP2pRecv) pfnRecv;
  } SWS_P2pProtocolDn;


/* Protocol send */
typedef SWS_CB( int, SWS_FP2pProtoSend( /* Return !0 on error, -1 if pending */
  SWS_PTR( void),                     /* IN: Client context */
  SWS_BufferH                         /* IN: Packet to send */
));

/* Protocol link status indication */
typedef SWS_CB( void, SWS_FP2pProtoLinkInd(
  SWS_PTR( void),                     /* IN: Client context */
  int                                 /* IN: SWS_EP2pLink */
));

/* Protocol close */
typedef SWS_CB( void, SWS_FP2pProtoRelease(
  SWS_PTR( void)                      /* IN: Client context */
));

/* Protocol get/set property */
typedef SWS_CB( int, SWS_FP2pProtoGetProperty(
  SWS_PTR( void),                     /* IN: Client context */
  int,                                /* IN: SWS_kDatalinkProperty_xxx */
  SWS_PTR( void),                     /* OUT: Buffer */
  SWS_PTR( size_t)                    /* I/O: Buffer len */
));
typedef SWS_CB( int, SWS_FP2pProtoSetProperty(
  SWS_PTR( void),                     /* IN: Client context */
  int,                                /* IN: SWS_kDatalinkProperty_xxx */
  SWS_PTR( const void),               /* IN: Buffer */
  size_t                              /* IN: Buffer len */
));

/* Protocol bind info */
typedef struct SWS_P2pProtocol
  {
  /* Calls from P2P to protocol */
  SWS_PTR( void) pvContext;           /* Context for calls */

  SWS_FNPTR( SWS_FP2pProtoRelease) pfnRelease;
  SWS_FNPTR( SWS_FP2pProtoSend) pfnSend;
  SWS_FNPTR( SWS_FP2pSendAck) pfnSendAck;
  SWS_FNPTR( SWS_FP2pRecvInd) pfnRecvInd;
  SWS_FNPTR( SWS_FP2pProtoLinkInd) pfnLinkInd;
  SWS_FNPTR( SWS_FP2pProtoGetProperty) pfnGetProperty;
  SWS_FNPTR( SWS_FP2pProtoSetProperty) pfnSetProperty;
  } SWS_P2pProtocol;


/*
 * Functions
 */
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Register a P2P adapter */
SWS_EXPORT( int, SWS_P2pAddAdapter(   /* Returns !0 on error */
  SWS_PTR( const SWS_P2pDevice),      /* IN: Device descriptor */
  SWS_PTR( void),                     /* IN: Driver's context for down calls */
  SWS_PTR( SWS_P2pCallbacks)          /* OUT: Up calls */
));

/* Protocol shutdown - adapter can be taken offline */
extern int SWS_P2pLinkFinished(
  unsigned                            /* IN: Adapter index */
);

/* Initialise this module */
extern int SWS_P2pInit( void);        /* Returns !0 on error */

/* Finalise this module */
extern void SWS_P2pCleanup( void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* ndef SWS_P2P_PRIV_H */
/* End of file */
