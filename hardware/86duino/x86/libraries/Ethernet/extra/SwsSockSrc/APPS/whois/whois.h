/****************************************************************************
 *
 * File name   : whois.h
 * Function    : Public API for the WHOIS/NICNAME client
 * Authors     : Lawrence Rust
 * Systems     : ANSI C
 * Version     : 1.0
 *
 ****************************************************************************
 * File History
 * ------------
 * 03-Mar-03, LVR, v1.00 - Created.
 *
 * Comments:
 * --------
 * This file contains the external interface to the WHOIS client module
 *
 ****************************************************************************/

#ifndef WHOIS_H
#define WHOIS_H 0x0100 /* Version no. BCD */

/* Dependencies */
#include <stdio.h>   /* FILE */


/* Constants */
#define WHOIS_SERVER 43 /* TCP (RFC 954) */

/* Types */
struct sockaddr;


/*
 * Functions
 */
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Query whois registration data for IP address or domain */
extern int WhoisQuery(                /* Returns !0 on error */
  FILE*,                              /* OUT: response */
  const char*,                        /* IN: Query */
  int,                                /* IN: stderr verbosity */
  struct sockaddr*                    /* I/O: whois server */
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* ndef WHOIS_H */
/* End of file */
