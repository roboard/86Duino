/****************************************************************************
 *
 * File name   : fakenic.h
 * Function    : Fake Ethernet driver external interface.
 * Project     : SwsSock
 * Authors     : Lawrence Rust
 * Systems     : ANSI C
 * Version     : 1.0
 * Release Date:
 *
 ****************************************************************************
 * File History
 * ------------
 * 26-Apr-01, LVR, v1.00 - Created.
 *
 *
 * Comments:
 * --------
 * This file contains the external interface to the fake Ethernet driver
 *
 ****************************************************************************/

#ifndef SWS_FAKENIC_H
#define SWS_FAKENIC_H 0x0100 /* Version no. BCD */

/* Dependencies */

/* Constants */
#define SWS_FAKE_IDENT "FakeEthernet"

/*
 * Functions
 */
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Install the driver */
extern int SWS_FakeInstall( void);    /* Returns !0 on error */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* ndef SWS_FAKENIC_H */
/* End of file */
