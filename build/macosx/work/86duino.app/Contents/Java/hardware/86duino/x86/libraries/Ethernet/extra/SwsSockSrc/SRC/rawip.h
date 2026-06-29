/****************************************************************************
 *
 * File name   : rawip.h
 * Function    : SOCK_RAW transport protocol module external interface.
 * Project     : SwsSock
 * Authors     : Lawrence Rust
 * Systems     : ANSI C
 * Version     : 1.0
 * Release Date:
 *
 ****************************************************************************
 * File History
 * ------------
 * 10-Apr-01, LVR, v1.00 - Created.
 *
 *
 * Comments:
 * --------
 * This file contains the external interface to the raw IP module
 *
 ****************************************************************************/

#ifndef SWS_RAWIP_H
#define SWS_RAWIP_H 0x0100 /* Version no. BCD */

/* Dependencies */
#include "sws_host.h"

/* Forward refs */


/*
 * Functions
 */
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Initialise the module */
extern int SWS_RawipInit( void);      /* Returns !0 on error */

/* Finalise the module */
extern void SWS_RawipCleanup( void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* ndef SWS_RAWIP_H */
/* End of file */
