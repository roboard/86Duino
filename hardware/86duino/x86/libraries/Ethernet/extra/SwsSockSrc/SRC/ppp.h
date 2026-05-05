/****************************************************************************
 *
 * File name   : ppp.h
 * Function    : API to the PPP protocol module
 * Project     : SwsSock
 * Authors     : Lawrence Rust
 * Systems     : ANSI C
 *
 ****************************************************************************
 * File History
 * ------------
 * 03-Aug-05, LVR, v1.00 - Created.
 *
 * Comments:
 * --------
 * This file contains the external interface to the PPP protocol module
 *
 ****************************************************************************/

#ifndef SWS_PPP_PROTO_H
#define SWS_PPP_PROTO_H 0x0100 /* Version no. BCD */

/* Dependencies */
#include "sws_host.h" /* Basic types */
#include "sws_dlnk.h"
#include "p2p.h"


/* Types */


/*
 * Functions
 */
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Bind the PPP protocol to a P2P adapter */
extern int SWS_PppBind(               /* Returns !0 on error */
  unsigned,                           /* IN: P2P adapter */
  SWS_PTR( const SWS_DatalinkCallbacks),/* IN: datalink up calls */
  SWS_PTR( const SWS_P2pProtocolDn),  /* IN: Adapter down calls */
  SWS_PTR( SWS_P2pProtocol)           /* OUT: Callbacks */
);

/* Initialise this module */
extern int SWS_PppInit( void);       /* Returns !0 on error */

/* Finalise this module */
extern void SWS_PppCleanup( void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* ndef SWS_PPP_PROTO_H */
/* End of file */
