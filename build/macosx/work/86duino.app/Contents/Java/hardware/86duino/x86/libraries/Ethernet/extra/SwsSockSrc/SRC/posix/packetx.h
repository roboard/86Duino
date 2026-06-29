/****************************************************************************
 *
 * File name   : packetx.h
 * Function    : PF_PACKET/SOCK_RAW ethernet datalink
 * Project     : SwsSock
 * Authors     : Lawrence Rust
 * Systems     : ANSI C
 *
 ****************************************************************************
 * File History
 * ------------
 * 13-Aug-09, LVR, Created.
 *
 *
 * Comments:
 * --------
 * This file contains the external interface to the packet driver
 *
 ****************************************************************************/

#ifndef SWS_PACKETX_H
#define SWS_PACKETX_H 0x0100 /* Version no. BCD */

/* Dependencies */

/* Constants */
#define SWS_PACKETX_IDENT "PacketX" /* + ifname */

/*
 * Functions
 */
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Install the driver */
extern int SWS_PacketXInstall( void);  /* Returns !0 on error */

/* Remove the driver */
extern void SWS_PacketXCleanup( void); /* Returns !0 on error */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* ndef SWS_PACKETX_H */
/* End of file */
