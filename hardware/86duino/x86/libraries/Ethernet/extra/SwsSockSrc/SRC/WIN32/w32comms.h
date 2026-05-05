/****************************************************************************
 *
 * File name   : w32comms.h
 * Function    : Win32 serial driver external interface.
 * Project     : SWSSOCK
 * Authors     : Lawrence Rust
 * Systems     : ANSI C
 *
 ****************************************************************************
 * File History
 * ------------
 * 24-Oct-01, LVR, v1.00 - Created.
 * 16-Jun-05, LVR, v1.01 - Added.SWS_Win32CommsCleanup
 *
 *
 * Comments:
 * --------
 * This file contains the external interface to the Win32 serial driver
 *
 ****************************************************************************/

#ifndef SWS_W32COMMS_H
#define SWS_W32COMMS_H 0x0100 /* Version no. BCD */

/* Dependencies */

/* Constants */
#define SWS_W32COMMS_IDENT "Win32Comms" /* + "1".. */

/*
 * Functions
 */
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Install the driver */
extern int SWS_Win32CommsInstall( void);    /* Returns !0 on error */

/* Remove the driver */
extern void SWS_Win32CommsCleanup( void); /* Returns !0 on error */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* ndef SWS_W32COMMS_H */
/* End of file */
