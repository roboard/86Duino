/****************************************************************************
 *
 * File name   : raw32.h
 * Function    : SOCK_RAW/IPPROTO_RAW winsock datalink.
 * Project     : SwsSock
 * Authors     : Lawrence Rust
 * Systems     : ANSI C
 * Version     : 1.0
 * Release Date:
 *
 ****************************************************************************
 * File History
 * ------------
 * 12-Jun-05, LVR, v1.00 - Created.
 *
 *
 * Comments:
 * --------
 * This file contains the external interface to the SOCK_RAW datalink
 *
 ****************************************************************************/

#ifndef SWS_RAW32_H
#define SWS_RAW32_H 0x0100 /* Version no. BCD */

/* Dependencies */

/* Constants */

/*
 * Functions
 */
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Install the driver */
extern int RawIpInstall( void);

/* Remove the driver */
extern void RawIpPreCleanup( void);
extern void RawIpCleanup( void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* ndef SWS_RAW32_H */
/* End of file */
