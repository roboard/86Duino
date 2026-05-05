/****************************************************************************
 *
 * File name   : smtpd.h
 * Function    : SMTP server.
 * Project     : SwsSock
 * Authors     : Lawrence Rust
 * Systems     : ANSI C
 *
 ****************************************************************************
 * File History
 * ------------
 * 20-Jan-03, LVR, v1.00 - Created.
 * 15-Jul-09, LVR, v1.01 - Added SMTPD_PORT
 * 07-Aug-09, LVR, v1.02
 * 22-Nov-09, LVR, v1.03
 * 23-Dec-09, LVR, v1.04
 * 29-Jan-10, LVR, v1.05
 * 11-Feb-11, LVR, v1.06
 *
 *
 * Comments:
 * --------
 * This file defines the interface to the smtp server
 *
 ****************************************************************************/

#ifndef SMTPD_H
#define SMTPD_H 0x0106 /* Version no. BCD */

#include <stdio.h>

#define SMTPD_VER_MAJOR 1
#define SMTPD_VER_MINOR 0.6

#define SMTPD_PORT 25 /* The default server port */

/* A mailbox record */
typedef struct SMailbox
  {
  char szMailbox[ 256]; /* User name */
  char szInbox[ 256]; /* Path to inbox */
  unsigned long ulQuota;
  } SMailbox;


/*
 * Functions
 */
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* The smtp server */
extern int Smtpd(
  const void*,                        /* IN: -> struct sockaddr_in, local address */
  int,                                /* IN: Verbosity */
  FILE*                               /* IN: Log file, NULL= none */
);

/* Get a mailbox record given a username */
extern int SmtpMailboxRecord(         /* !0 if successful */
  SMailbox*,                          /* OUT: Best match */
  const char*                         /* IN: Username or mailbox name */
);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* ndef SMTPD_H */
/* End of file */

