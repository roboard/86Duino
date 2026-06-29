/****************************************************************************
 *
 * File name   : syslogd.h
 * Function    : syslog (RFC 3164) server.
 * Authors     : Lawrence Rust
 * Systems     : ANSI C
 *
 ****************************************************************************
 * File History
 * ------------
 * 24-Jun-03, LVR, v1.00 - Created
 * 13-Jul-09, LVR, v1.01 - Added SYSLOGD_PORT
 *
 *
 * Comments:
 * --------
 * This file defines the interface to the syslog server
 *
 ****************************************************************************/

#ifndef SYSLOGD_H
#define SYSLOGD_H 0x0101 /* Version no. BCD */

#include <stdio.h>

#define SYSLOGD_VER_MAJOR 1
#define SYSLOGD_VER_MINOR 0.1

#define SYSLOGD_PORT 514 /* The syslog server port */

struct sockaddr;

/*
 * Functions
 */
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* The syslog server */
extern int Syslogd(
  struct sockaddr*,                   /* IN: -> local address */
  int,                                /* IN: Verbosity */
  FILE*                               /* IN: Log file, NULL= none */
);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* ndef SYSLOGD_H */
/* End of file */

