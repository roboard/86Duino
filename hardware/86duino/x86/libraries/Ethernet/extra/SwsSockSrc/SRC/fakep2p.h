/****************************************************************************
 *
 * File name   : fakep2p.h
 * Function    : Fake P2P driver external interface.
 * Project     : SwsSock
 * Authors     : Lawrence Rust
 * Systems     : ANSI C
 *
 ****************************************************************************
 * File History
 * ------------
 * 23-Oct-01, LVR, v1.00 - Created.
 * 21-Aug-05, LVR, v1.01 - Changed name from fakeslip to fakep2p
 *
 *
 * Comments:
 * --------
 * This file contains the external interface to the fake P2P driver
 *
 ****************************************************************************/

#ifndef SWS_FAKEP2P_H
#define SWS_FAKEP2P_H 0x0101 /* Version no. BCD */

/* Dependencies */

/* Constants */
#define SWS_FAKEP2P_IDENT "FakeP2p"

/*
 * Functions
 */
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Install the driver */
extern int SWS_FakeP2pInstall( void);    /* Returns !0 on error */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* ndef SWS_FAKEP2P_H */
/* End of file */
