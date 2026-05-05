/****************************************************************************
 *
 * File name   : packet.h
 * Function    : Packet driver.
 * Project     : SwsSock
 * Authors     : Lawrence Rust
 * Systems     : ANSI C
 * Version     : 1.0
 * Release Date:
 *
 ****************************************************************************
 * File History
 * ------------
 * 30-Apr-01, LVR, v1.00 - Created.
 *
 *
 * Comments:
 * --------
 * This file contains the external interface to the packet driver
 *
 ****************************************************************************/

#ifndef SWS_PACKET_H
#define SWS_PACKET_H 0x0100 /* Version no. BCD */

/* Dependencies */

/* Constants */
#define SWS_PACKET_IDENT "PacketDriverInterface" /* + "1".. */

/*
 * Functions
 */
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Install the driver */
extern int SWS_PacketInstall( void);  /* Returns !0 on error */

/* Remove the driver */
extern void SWS_PacketCleanup( void); /* Returns !0 on error */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* ndef SWS_PACKET_H */
/* End of file */
