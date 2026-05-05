/****************************************************************************
 *
 * File name   : recvmail.h
 * Function    : POP3 client.
 * Project     : SwsSock
 * Authors     : Lawrence Rust
 * Systems     : ANSI C
 * Version     : 1.0
 * Release Date:
 *
 ****************************************************************************
 * File History
 * ------------
 * 13-Feb-04, LVR, v1.00 - Created.
 * 08-Feb-10, LVR, v1.01 - RecvMail reads input from stdin (not input file)
 *
 *
 * Comments:
 * --------
 * This file defines the interface to the POP3 client
 *
 ****************************************************************************/

#ifndef RECVMAIL_H
#define RECVMAIL_H

#include <stdio.h>

#define RECVMAIL_VER_MAJOR 1
#define RECVMAIL_VER_MINOR 0.1

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

extern int RecvMail(
  int,                        /* stdin isaScript, 0= interactive */
  const void*,                /* -> server sockaddr_in */
  FILE*,                      /* Logfile, NULL= none, stdout for interactive */
  int                         /* verbosity */
);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* ndef RECVMAIL_H */
/* End of file */
