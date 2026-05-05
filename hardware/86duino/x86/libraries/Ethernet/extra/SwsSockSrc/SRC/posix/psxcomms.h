/****************************************************************************
 *
 * File name   : psxcomms.h
 * Function    : Posix serial driver external interface.
 * Project     : SWSSOCK
 * Authors     : Lawrence Rust
 * Systems     : ANSI C
 *
 ****************************************************************************
 * File History
 * ------------
 * 20-Aug-09, LVR, v1.00 - Created.
 *
 *
 * Comments:
 * --------
 * This file contains the external interface to the Posix serial driver
 *
 ****************************************************************************/

#ifndef SWS_PSXCOMMS_H
#define SWS_PSXCOMMS_H 0x0100 /* Version no. BCD */

/* Dependencies */

/* Constants */
#define SWS_PSXCOMMS_IDENT "PosixComms" /* + "1".. */

/*
 * Functions
 */
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Install the driver */
extern int SWS_PosixCommsInstall( void);    /* Returns !0 on error */

/* Remove the driver */
extern void SWS_PosixCommsCleanup( void); /* Returns !0 on error */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* ndef SWS_PSXCOMMS_H */
/* End of file */
