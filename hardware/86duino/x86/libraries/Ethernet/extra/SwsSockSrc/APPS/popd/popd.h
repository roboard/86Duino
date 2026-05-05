/****************************************************************************
 *
 * File name   : popd.h
 * Function    : POP3 server.
 * Project     : SwsSock
 * Authors     : Lawrence Rust
 * Systems     : ANSI C
 *
 ****************************************************************************
 * File History
 * ------------
 * 26-Jan-03, LVR, v1.00 - Created.
 * 30-May-04, LVR, v1.01 - Added PopdSetLog.
 * 15-Jul-09, LVR, v1.02 - Added SMTPD_PORT
 * 07-Aug-09, LVR, v1.03
 * 23-Dec-09, LVR, v1.04
 * 09-Mar-10, LVR, v1.05
 *
 *
 * Comments:
 * --------
 * This file defines the interface to the pop3 server
 *
 ****************************************************************************/

#ifndef POPD_H
#define POPD_H 0x0105 /* Version no. BCD */

#include <stdio.h>

#define POPD_VER_MAJOR 1
#define POPD_VER_MINOR 0.5

#define POPD_PORT 110 /* The default server port */


/*
 * Functions
 */
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* The pop server */
extern int Popd(
  const void*,                        /* IN: -> struct sockaddr_in, local address */
  int,                                /* IN: Verbosity */
  FILE*                               /* IN: Log file, NULL= none */
);

extern void PopdSetLog(
  int,                                /* IN: Verbosity */
  FILE*                               /* IN: Log file, NULL= none */
);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* ndef POPD_H */
/* End of file */

