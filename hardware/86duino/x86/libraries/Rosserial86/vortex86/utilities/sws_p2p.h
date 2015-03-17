/****************************************************************************
 *
 * File name   : sws_p2p.h
 * Function    : Public API for Point to Point adapters
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
 * This file contains the external interface to the P2P link manager
 *
 ****************************************************************************/

#ifndef SWS_P2P_H
#define SWS_P2P_H 0x0100 /* Version no. BCD */

/* Dependencies */
#include <stddef.h>   /* size_t */
#include "sws_host.h" /* Basic types */
#include "sws_buf.h"


/*
 * Macros and constants
 */

/* Datalink Properties */
enum
  {
  SWS_kP2pProperty_Modem = 0x4000,    /* ui8 (r/o) 0= none, 1= dialup */
  SWS_kP2pProperty_Baud               /* ui32 (r/o) */
  };

/* Adapter state */
enum SWS_EP2pState
  {
  SWS_kP2pStateClosed,
  SWS_kP2pStateListening,
  SWS_kP2pStateCalling,
  SWS_kP2pStateLogin,
  SWS_kP2pStateConnected
  };

/* Protocols */
enum SWS_EP2pProtocol
  {
  SWS_kP2pProtocolSlip,
  SWS_kP2pProtocolPpp
  };

/* Authentication */
enum SWS_EP2pAuth
  {
  SWS_kP2pAuthNone,
  SWS_kP2pAuthPap,
  SWS_kP2pAuthChap
  };

/* SWS_P2pClient/Server callback events */
enum SWS_EP2pEvent
  {
  SWS_kP2pEventIdle,
  SWS_kP2pEventConnect,               /* Data -> ui32 baud */
  SWS_kP2pEventLogin,                 /* Data -> SWS_SP2pLogin */
  SWS_kP2pEventDisconnect,
  SWS_kP2pEventError,                 /* Data -> int modem response */
  SWS_kP2pEventClose
  };


/*
 * Types
 */

/* SWS_SP2pClient/Server event callback */
typedef SWS_CB( int, SWS_FP2pEvent(
  SWS_PTR( void),                     /* IN: Client context */
  unsigned,                           /* IN: Adapter */
  int,                                /* IN: SWS_EP2pEvent */
  size_t,                             /* IN: Size of event data */
  SWS_PTR( void)                      /* I/O: Event data */
));

/* Event data passed to SWS_FP2pEvent/SWS_kP2pEventLogin */
typedef struct SWS_SP2pLogin
  {
  SWS_PTR( const char) pszRx;         /* IN: text received, NULL on timeout */
  char szTx[ 80];                     /* OUT: text to send */
  ui32 uTimeout;                      /* OUT: timeout, millisecs */
  } SWS_SP2pLogin;


/* User account info */
typedef struct SWS_SP2pAcct
  {
  char szName[ 32];
  char szPassword[ 32];
  } SWS_SP2pAcct;

/* Client/server session properties */
typedef struct SWS_SP2pSession
  {
  i16 protocol;                       /* SWS_EP2pProtocol */
  i16 auth;                           /* SWS_EP2pAuth */

  char szServer[ 32];                 /* Modem dial string */
  ui32 uConnectTimeout;               /* Connection setup timeout in milliseconds, 0= none */
  ui16 uIdleTimeout;                  /* Connection idle timeout in seconds, 0= none */
  ui16 uCompSlots;                    /* VJ TCP/IP header compression slots */

  SWS_FNPTR( SWS_FP2pEvent) pfnEvent; /* Event callback */
  SWS_PTR( void) pvEvent;             /* Context for calback */

  SWS_SP2pAcct user;

  ui32 uIpAddress;                    /* Valid IP address or SWS_INADDR_ANY */
  ui32 uSubnetMask;                   /* Valid subnet, 0= default for IP class */
  ui32 uGateway;                      /* Valid IP address or SWS_INADDR_ANY */
  } SWS_SP2pSession;


/*
 * Functions
 */
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Get the no. P2P adapters */
SWS_EXPORT( unsigned, SWS_P2pCount( void));

/* Get an adapter's datalink ID */
SWS_EXPORT( SWS_PTR( const char), SWS_P2pGetIdent(
  unsigned                            /* IN: 0..SWS_P2pCount() -1 */
));

/* Get an adapter's connection state */
SWS_EXPORT( int, SWS_P2pGetState(     /* Returns SWS_EP2pState */
  unsigned                            /* IN: 0..SWS_P2pCount() -1 */
));


/* Start a session */
SWS_EXPORT( int, SWS_P2pStart(        /* Returns !0 on error */
  unsigned,                           /* IN: 0..SWS_P2pCount() -1 */
  SWS_PTR( const SWS_SP2pSession)     /* IN: Session properties */
));

/* Start a link from config file settings */
SWS_EXPORT( int, SWS_P2pStartLink( /* Returns !0 on error */
  unsigned,                           /* IN: 0..SWS_P2pCount() -1 */
  SWS_PTR( const char)                /* IN: Link name */
));

/* Stop a session */
SWS_EXPORT( int, SWS_P2pStop(         /* Returns !0 on error */
  unsigned                            /* IN: 0..SWS_P2pCount() -1 */
));

/* Start dial on demand adapters */
SWS_EXPORT( int, SWS_P2pDialOnDemand( /* Returns 0 if connection established */
  ui32                                /* IN: IP address to reach */
));


/* Get/set session accout holder */
SWS_EXPORT( int, SWS_P2pGetAcct(      /* Returns !0 on error */
  unsigned,                           /* IN: 0..SWS_P2pCount() -1 */
  SWS_PTR( SWS_SP2pAcct)              /* OUT: Logged in user */
));
SWS_EXPORT( int, SWS_P2pSetAcct(      /* Returns !0 on error */
  unsigned,                           /* IN: 0..SWS_P2pCount() -1 */
  SWS_PTR( SWS_SP2pAcct)              /* I/O: szName in, szPassword out */
));

/* Session IP setup */
SWS_EXPORT( ui32, SWS_P2pGetIpAddress(
  unsigned                            /* IN: 0..SWS_P2pCount() -1 */
));
SWS_EXPORT( ui32, SWS_P2pGetSubnetMask(
  unsigned                            /* IN: 0..SWS_P2pCount() -1 */
));
SWS_EXPORT( ui32, SWS_P2pGetGateway(
  unsigned                            /* IN: 0..SWS_P2pCount() -1 */
));

/* Session authentication protocol */
SWS_EXPORT( ui16, SWS_P2pGetAuth(     /* Returns 0= none */
  unsigned                            /* IN: 0..SWS_P2pCount() -1 */
));

/*
 * Van Jacobsen header compression RFC 1144
 */
struct SWS_SP2pVjComp;
typedef SWS_PTR( struct SWS_SP2pVjComp) SWS_SP2pVjCompH;

/* Types of compressed packets */
typedef enum SWS_EP2pVjType
  {
  /* NB type values assigned to aid SLIP framing */
  SWS_kP2pVjTypeIP           = 0x40,  /* Native IP */
  SWS_kP2pVjTypeUncompressed = 0x70,  /* Uncompressed TCP */
  SWS_kP2pVjTypeCompressed   = 0x80,  /* Compressed TCP */
  SWS_kP2pVjTypeError        = 0x00   /* Not a type but an error indication */
  } SWS_EP2pVjType;

/* Get VJ compression slots for the session */
SWS_EXPORT( ui8, SWS_P2pVjSlots(      /* 0= disabled */
  unsigned                            /* IN: 0..SWS_P2pCount() -1 */
));

/* Open a compressor */
SWS_EXPORT( SWS_SP2pVjCompH, SWS_P2pVjOpen(
  ui8,                                /* IN: MaxSlotId */
  ui8                                 /* IN: CompSlotId */
));

/* Close a compressor */
SWS_EXPORT( void, SWS_P2pVjClose(
  SWS_SP2pVjCompH                     /* IN: Compressor context */
));

/* Compress an IP packet */
SWS_EXPORT( SWS_EP2pVjType, SWS_P2pVjCompress(
  SWS_SP2pVjCompH,                    /* IN: Compressor context */
  SWS_BufferH                         /* I/O */
));

/* Decompress a packet */
SWS_EXPORT( int, SWS_P2pVjDecompress(
  SWS_SP2pVjCompH,                    /* IN: Compressor context */
  SWS_EP2pVjType,                     /* IN: Type */
  SWS_BufferH                         /* I/O */
));

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* ndef SWS_P2P_H */
/* End of file */
