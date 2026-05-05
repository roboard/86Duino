/****************************************************************************
 *
 * File name   : echopd.h
 * Function    : echo/discard server.
 * Project     : SwsSock
 * Authors     : Lawrence Rust
 * Systems     : ANSI C
 *
 ****************************************************************************
 * File History
 * ------------
 * 05-Mar-03, LVR, v1.00 - Created.
 * 15-Jul-09, LVR, v2.01 - Added ECHOD_PORT & DSCRD_PORT
 *
 *
 * Comments:
 * --------
 * This file defines the interface to the echo server
 *
 ****************************************************************************/

#ifndef ECHOD_H
#define ECHOD_H 0x0201 /* Version no. BCD */

#include <stdio.h>

#define ECHOD_VER_MAJOR 2
#define ECHOD_VER_MINOR 0.1

#define ECHOD_PORT 7 /* The default server port */
#define DSCRD_PORT 9 /* The default server port */

struct sockaddr;

/*
 * Functions
 */
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* The echo/discard server */
extern int Echod(
  struct sockaddr*,                   /* IN: -> struct sockaddr_in, local address */
  int,                                /* IN: 0= discard */
  int,                                /* IN: Verbosity */
  FILE*                               /* IN: Log file, NULL= none */
);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* ndef ECHOD_H */
/* End of file */

