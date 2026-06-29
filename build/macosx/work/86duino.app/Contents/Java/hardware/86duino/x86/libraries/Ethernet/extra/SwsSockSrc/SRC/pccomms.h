/****************************************************************************
 *
 * File name   : pccomms.h
 * Function    : PC serial comms driver external interface.
 * Project     : SwsSock
 * Authors     : Lawrence Rust
 * Systems     : ANSI C
 * Version     : 1.0
 * Release Date:
 *
 ****************************************************************************
 * File History
 * ------------
 * 07-Jan-03, LVR, v1.00 - Created.
 *
 *
 * Comments:
 * --------
 * This file contains the external interface to the PC comms driver
 *
 ****************************************************************************/

#ifndef SWS_PCCOMMS_H
#define SWS_PCCOMMS_H 0x0100 /* Version no. BCD */

/* Dependencies */

/* Constants */
#define SWS_PCCOMMS_IDENT "PcComms"

/*
 * Functions
 */
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Install the driver */
extern int SWS_PcCommsInstall( void);    /* Returns !0 on error */

/* Finalise the driver */
extern void SWS_PcCommsCleanup( void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* ndef SWS_PCCOMMS_H */
/* End of file */
