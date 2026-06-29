/****************************************************************************
 *
 * File name   : ftpcli.h
 * Function    : FTP command line interpreter.
 * Project     : SwsSock
 * Authors     : Lawrence Rust
 * Systems     : ANSI C
 *
 ****************************************************************************
 * File History
 * ------------
 * 09-Jul-05, LVR, v1.00 - Created.
 * 17-Mar-10, LVR, v1.01 - Replace inputfile with isScript
 *
 *
 * Comments:
 * --------
 * This file defines the interface to the POP3 client
 *
 ****************************************************************************/

#ifndef FTPCLI_H
#define FTPCLI_H

#include <stdio.h>

#define FTPCLI_VER_MAJOR 1
#define FTPCLI_VER_MINOR 0.1

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

extern int FtpCli(
  int,                        /* 0= interactive */
  const char*,                /* Server hostname */
  unsigned,                   /* Server port */
  FILE*,                      /* Logfile, NULL= none */
  int                         /* verbosity */
);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* ndef FTPCLI_H */
/* End of file */
