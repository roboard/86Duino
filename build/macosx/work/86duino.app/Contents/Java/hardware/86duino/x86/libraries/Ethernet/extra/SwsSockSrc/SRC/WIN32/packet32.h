/****************************************************************************
 *
 * File name   : packet32.h
 * Function    : Win32 sws_sock packet driver.
 * Project     : SwsSock
 * Authors     : Lawrence Rust
 * Systems     : ANSI C
 * Version     : 1.0
 * Release Date:
 *
 ****************************************************************************
 * File History
 * ------------
 * 27-Mar-03, LVR, v1.00 - Created.
 *
 *
 * Comments:
 * --------
 * This file contains the external interface to the packet driver
 *
 ****************************************************************************/

#ifndef SWS_PACKET32_H
#define SWS_PACKET32_H 0x0100 /* Version no. BCD */

/* Dependencies */

/* Constants */
#define SWS_PACKET32_IDENT "Packet32DriverInterface" /* + "1".. */
#define SWS_PACKET32WAN_IDENT "Packet32WanInterface" /* + "1".. */

/*
 * Functions
 */
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Install the driver */
extern int SWS_Packet32Install( void);  /* Returns !0 on error */

/* Remove the driver */
extern void SWS_Packet32Cleanup( void); /* Returns !0 on error */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* ndef SWS_PACKET32_H */
/* End of file */
