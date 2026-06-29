/****************************************************************************
 *
 * File name   : identd.h
 * Function    : ident (RFC 1413) server.
 * Authors     : Lawrence Rust
 * Systems     : ANSI C
 *
 ****************************************************************************
 * File History
 * ------------
 * 05-Jan-05, LVR, v1.00 - Created.
 * 15-Jul-09, LVR, v1.01 - Added IDENTD_PORT
 * 24-Dec-09, LVR, v1.02
 *
 *
 * Comments:
 * --------
 * This file defines the interface to the ident server
 *
 ****************************************************************************/

#ifndef IDENTD_H
#define IDENTD_H 0x0102 /* Version no. BCD */

#include <stdio.h>

#define IDENTD_VER_MAJOR 1
#define IDENTD_VER_MINOR 0.2

#define IDENTD_PORT 113 /* The default server port */

struct sockaddr;

/*
 * Functions
 */
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* The ident server */
extern int Identd(
  struct sockaddr*,                   /* IN: -> local address */
  int,                                /* IN: Verbosity */
  FILE*                               /* IN: Log file, NULL= none */
);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* ndef IDENTD_H */
/* End of file */

