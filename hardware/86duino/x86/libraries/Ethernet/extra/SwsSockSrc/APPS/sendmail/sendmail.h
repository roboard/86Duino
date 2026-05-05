/****************************************************************************
 *
 * File name   : sendmail.h
 * Function    : SMTP client.
 * Authors     : Lawrence Rust
 * Systems     : ANSI C, sockets
 * Version     : 1.0
 *
 ****************************************************************************
 * File History
 * ------------
 * 29-Nov-02, LVR, v1.00 - Created.
 * 15-Nov-09, LVR, v1.01 - Added Login
 *
 *
 * Comments:
 * --------
 * This file defines the interface to the SMTP client
 *
 ****************************************************************************/

#ifndef SENDMAIL_H
#define SENDMAIL_H

#define SENDMAIL_VER_MAJOR 1
#define SENDMAIL_VER_MINOR 0.1

/* Dependencies */
#include <stdio.h>  /* FILE */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

extern int SendMail(
  FILE*,                      /* File to post */
  int,                        /* IsInteractive */
  int,                        /* Count of recipients */
  const char* const [],       /* Array of recipients, cc: or bcc: */
  const char*,                /* From, empty= none, NULL= use mail/from cfg entry */
  const char*,                /* Reply-To, NULL/empty= none */
  const char*,                /* Subject. NULL= no envelope, empty= none */
  const char*,                /* SMTP server, NULL= default, empty= use MX for each recipient */
  unsigned short,             /* SMTP port */
  const char*,                /* Name for HELO, NULL= gethostname, empty= use IP address*/
  const char*,                /* Auth plain login, NULL= none, usr:pwd | base64 */
  FILE*,                      /* Logfile, NULL= none */
  int                         /* Stderr verbosity, 1= normal, 0= none */
);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* ndef SENDMAIL_H */
/* End of file */
