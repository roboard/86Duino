/****************************************************************************
 *
 * File name   : slip.h
 * Function    : Public API for serial adapters
 * Project     : SwsSock
 * Authors     : Lawrence Rust
 * Systems     : ANSI C
 *
 ****************************************************************************
 * File History
 * ------------
 * 23-Oct-01, LVR, v1.00 - Created.
 * 03-Aug-05, LVR, v1.01 - P2P version.
 *
 * Comments:
 * --------
 * This file contains the external interface to the SLIP module
 *
 ****************************************************************************/

#ifndef SWS_SLIP_H
#define SWS_SLIP_H 0x0101 /* Version no. BCD */

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

/* Bind the SLIP protocol to a P2P adapter */
extern int SWS_SlipBind(              /* Returns !0 on error */
  unsigned,                           /* IN: P2P adapter */
  SWS_PTR( const SWS_DatalinkCallbacks),/* IN: datalink up calls */
  SWS_PTR( const SWS_P2pProtocolDn),  /* IN: Adapter down calls */
  SWS_PTR( SWS_P2pProtocol)           /* OUT: Callbacks */
);

/* Initialise the SLIP module */
extern int SWS_SlipInit( void);       /* Returns !0 on error */

/* Finalise the SLIP module */
extern void SWS_SlipCleanup( void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* ndef SWS_SLIP_H */
/* End of file */
