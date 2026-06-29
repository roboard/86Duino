/****************************************************************************
 *
 * File name   : tftp.h
 * Function    : Public API for the TFTP client
 * Authors     : Lawrence Rust
 * Systems     : ANSI C
 * Version     : 1.0
 *
 ****************************************************************************
 * File History
 * ------------
 * 30-Aug-03, LVR, v1.00 - Created.
 *
 * Comments:
 * --------
 * This file contains the external interface to the TFTP client module
 *
 ****************************************************************************/

#ifndef TFTP_H
#define TFTP_H 0x0100 /* Version no. BCD */

/* Dependencies */
#include <stdio.h>   /* FILE */


/* Constants */
#define TFTP_VER_MAJOR 1
#define TFTP_VER_MINOR 00

#define TFTP_SERVER 69 /* UDP (RFC 1350) */

/* Transfer modes */
#define TFTP_TEXT  "netascii"
#define TFTP_OCTET "octet"
#define TFTP_MAIL  "mail"


/* Types */
struct sockaddr;


/*
 * Functions
 */
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

extern int TftpPut(                   /* Returns !0 on error */
  struct sockaddr*,                   /* IN: tftp server */
  FILE*,                              /* IN: source file */
  const char*,                        /* IN: destination filename */
  const char*,                        /* IN: Transfer mode (TFTP_TEXT, TFTP_OCTET etc) */
  int                                 /* IN: stderr verbosity */
);

extern int TftpGet(                   /* Returns !0 on error */
  struct sockaddr*,                   /* IN: tftp server */
  const char*,                        /* IN: source filename */
  FILE*,                              /* IN: destination file */
  const char*,                        /* IN: Transfer mode (TFTP_TEXT, TFTP_OCTET etc) */
  int                                 /* IN: stderr verbosity */
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* ndef TFTP_H */
/* End of file */
