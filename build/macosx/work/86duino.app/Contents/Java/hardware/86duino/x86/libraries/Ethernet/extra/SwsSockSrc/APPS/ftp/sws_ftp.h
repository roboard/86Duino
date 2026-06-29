/****************************************************************************
 *
 * File name   : sws_ftp.h
 * Function    : Public API to the FTP client.
 * Project     : SwsSock
 * Authors     : Lawrence Rust
 * Systems     : ANSI C
 *
 ****************************************************************************
 * File History
 * ------------
 * 13-Feb-04, LVR, v1.00 - Created.
 *
 * Comments:
 * --------
 * This file defines the interface to the FTP client
 *
 ****************************************************************************/

#ifndef SWS_FTP_H
#define SWS_FTP_H 0x0100 /* Version no. BCD */

/* Dependencies */
#include <stdio.h>
#include "sws_host.h" /* Basic types */


/* Types */

/* FTP client handle */
typedef SWS_PTR( struct SWS_FtpClient) SWS_FtpClientH;

/* Transfer callbacks */
typedef SWS_CB( int, SWS_FFtpGet(     /* Return 0 to continue */
  SWS_PTR( void),                     /* IN: user context */
  SWS_FtpClientH,                     /* IN, client handle */
  SWS_PTR( const void),               /* IN: -> data buffer */
  int                                 /* IN: buffer size */
));
typedef SWS_CB( int, SWS_FFtpPut(     /* Return no. bytes, 0= done, -ve abort */
  SWS_PTR( void),                     /* IN: user context */
  SWS_FtpClientH,                     /* IN, client handle */
  SWS_PTR( void),                     /* OUT: -> data buffer */
  int                                 /* IN: buffer size */
));


/*
 * Functions
 */
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Create/destroy client */
SWS_EXTERN( SWS_FtpClientH, FtpClientCreate(
  FILE*,                              /* IN: log file, NULL= none */
  int                                 /* IN: log verbosity */
));
SWS_EXTERN( void, FtpClientDestroy(
  SWS_FtpClientH                      /* IN, client handle */
));

/* Set command output callback */
SWS_EXTERN( int, FtpSetCallback(
  SWS_FtpClientH,                     /* IN, client handle */
  SWS_FNPTR( SWS_FFtpGet),            /* IN: callback */
  SWS_PTR( void)                      /* IN: callback context */
));

/* Connect/disconnect server */
SWS_EXTERN( int, FtpConnect(
  SWS_FtpClientH,                     /* IN, client handle */
  SWS_PTR( const char),               /* IN: FTP server hostname */
  unsigned                            /* IN: FTP server port, 0= default */
));
SWS_EXTERN( int, FtpDisconnect(
  SWS_FtpClientH                      /* IN, client handle */
));

/* Access control */
SWS_EXTERN( int, FtpNoop(
  SWS_FtpClientH                      /* IN, client handle */
));
SWS_EXTERN( int, FtpUser(
  SWS_FtpClientH,                     /* IN, client handle */
  SWS_PTR( const char)                /* IN: username */
));
SWS_EXTERN( int, FtpPass(
  SWS_FtpClientH,                     /* IN, client handle */
  SWS_PTR( const char)                /* IN: password */
));
SWS_EXTERN( int, FtpAcct(
  SWS_FtpClientH,                     /* IN, client handle */
  SWS_PTR( const char)                /* IN: account name */
));
SWS_EXTERN( int, FtpQuit(
  SWS_FtpClientH                      /* IN, client handle */
));

/* Session attributes */
SWS_EXTERN( int, FtpPasv(
  SWS_FtpClientH,                     /* IN, client handle */
  int                                 /* IN: !0 for passive mode */
));
SWS_EXTERN( int, FtpType(
  SWS_FtpClientH,                     /* IN, client handle */
  int                                 /* IN: 0= ASCII, 1= binary (default) */
));

/* Info */
SWS_EXTERN( int, FtpSyst(
  SWS_FtpClientH,                     /* IN, client handle */
  int,                                /* IN: buffer size */
  SWS_PTR( char)                      /* OUT: buffer for info */
));
SWS_EXTERN( int, FtpHelp(
  SWS_FtpClientH,                     /* IN, client handle */
  SWS_PTR( const char)                /* IN: text */
));
SWS_EXTERN( int, FtpCmd(
  SWS_FtpClientH,                     /* IN, client handle */
  SWS_PTR( const char)                /* IN: remote command */
));

/* File transfer */
SWS_EXTERN( int, FtpGet(
  SWS_FtpClientH,                     /* IN, client handle */
  SWS_PTR( const char),               /* IN: source filename on server */
  SWS_FNPTR( SWS_FFtpGet),            /* IN: data callback */
  SWS_PTR( void)                      /* IN: callback context */
));
SWS_EXTERN( int, FtpPut(
  SWS_FtpClientH,                     /* IN, client handle */
  SWS_PTR( const char),               /* IN: target filename on server */
  SWS_FNPTR( SWS_FFtpPut),            /* IN: data callback */
  SWS_PTR( void)                      /* IN: callback context */
));

/* File maintenance */
SWS_EXTERN( int, FtpRen(
  SWS_FtpClientH,                     /* IN, client handle */
  SWS_PTR( const char),               /* IN: old filename on server */
  SWS_PTR( const char)                /* IN: new filename on server */
));
SWS_EXTERN( int, FtpDel(
  SWS_FtpClientH,                     /* IN, client handle */
  SWS_PTR( const char)                /* IN: filename on server */
));

/* Directory maintenance */
SWS_EXTERN( int, FtpCwd(
  SWS_FtpClientH,                     /* IN, client handle */
  SWS_PTR( const char)                /* IN: path */
));
SWS_EXTERN( int, FtpCdup(
  SWS_FtpClientH                      /* IN, client handle */
));
SWS_EXTERN( int, FtpPwd(
  SWS_FtpClientH,                     /* IN, client handle */
  int,                                /* IN: buffer size */
  SWS_PTR( char)                      /* OUT: buffer for name */
));
SWS_EXTERN( int, FtpMkd(
  SWS_FtpClientH,                     /* IN, client handle */
  SWS_PTR( const char)                /* IN: path */
));
SWS_EXTERN( int, FtpRmd(
  SWS_FtpClientH,                     /* IN, client handle */
  SWS_PTR( const char)                /* IN: path */
));

/* Listings */
SWS_EXTERN( int, FtpList(
  SWS_FtpClientH,                     /* IN, client handle */
  SWS_PTR( const char),               /* IN: filespec */
  SWS_FNPTR( SWS_FFtpGet),            /* IN: data callback */
  SWS_PTR( void)                      /* IN: callback context */
));
SWS_EXTERN( int, FtpNlst(
  SWS_FtpClientH,                     /* IN, client handle */
  SWS_PTR( const char),               /* IN: filespec */
  SWS_FNPTR( SWS_FFtpGet),            /* IN: data callback */
  SWS_PTR( void)                      /* IN: callback context */
));
SWS_EXTERN( int, FtpStat(
  SWS_FtpClientH,                     /* IN, client handle */
  SWS_PTR( const char)                /* IN: filespec */
));

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* ndef RECVMAIL_H */
/* End of file */
