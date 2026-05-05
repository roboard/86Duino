/****************************************************************************
 *
 * File name   : loopback.h
 * Function    : Loopback data link provider external interface.
 * Project     : SwsSock
 * Authors     : Lawrence Rust
 * Systems     : ANSI C
 * Version     : 1.0
 * Release Date:
 *
 ****************************************************************************
 * File History
 * ------------
 * 02-Apr-01, LVR, v1.00 - Created.
 *
 *
 * Comments:
 * --------
 * This file contains the external interface to the loopback module
 *
 ****************************************************************************/

#ifndef SWS_LOOPBACK_H
#define SWS_LOOPBACK_H 0x0100 /* Version no. BCD */

/* Dependencies */


/*
 * Functions
 */
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Initialise the module */
extern int SWS_LoopbackInit( void);    /* Returns !0 on error */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* ndef SWS_LOOPBACK_H */
/* End of file */
