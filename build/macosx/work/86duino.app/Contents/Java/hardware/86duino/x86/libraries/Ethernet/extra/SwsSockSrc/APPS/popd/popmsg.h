/****************************************************************************
 *
 * File name   : popmsg.h
 * Function    : POP3 message server.
 * Authors     : Lawrence Rust
 * Systems     : ANSI C
 *
 ****************************************************************************
 * File History
 * ------------
 * 23-Mar-04, LVR, v1.00 - Created.
 * 08-Nov-04, LVR, v1.01 - Add PopMsgUidl.
 *
 *
 * Comments:
 * --------
 * This file defines the interface to the pop3 message server
 *
 ****************************************************************************/

#ifndef POPMSG_H
#define POPMSG_H 0x0101 /* Version no. BCD */

/* Dependencies */
#include <stdio.h>


/*
 * Constants
 */
/* Function error codes */
enum {
  kPopMsgErrNone = 0,
  kPopMsgErrParam,
  kPopMsgErrNoMem,
  kPopMsgErrBadUser,
  kPopMsgErrMbxLocked,
  kPopMsgErrFailed
};

/* Message callback events */
enum EPopMsgNotify {
  kPopMsgNotifyProgress,
  kPopMsgNotifyConfirm,
  kPopMsgNotifyPost,
  kPopMsgNotifyMsg
  };


/*
 * Types
 */
typedef struct SPopUser SPopUser;
typedef struct SPopMsg SPopMsg;
struct SMimeInfo;


/*
 * Functions
 */
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Verify user credentials */
extern int PopMsgCreateUser(            /* Returns 0= success */
  SPopUser**,                           /* OUT: user identifier */
  const char*,                          /* IN: User name */
  const char*,                          /* IN: Password */
  int,                                  /* IN: Logfile verbosity */
  FILE*                                 /* IN: Logfile or NULL */
);

/* Get text for last error */
extern const char* PopMsgLastError( void);

/* Destroy user */
extern void PopMsgDestroyUser(
  SPopUser*                             /* IN: User ID */
);


/* Get user name */
extern const char* PopMsgUserName(
  SPopUser*                             /* IN: User ID */
);

/* Get message count */
extern unsigned PopMsgCount(
  SPopUser*                             /* IN: User ID */
);

/* Get total size of messages */
extern unsigned long PopTotalSize(
  SPopUser*                             /* IN: User ID */
);

/* Get message count confirmed */
extern unsigned PopMsgCountConfirmed(
  SPopUser*                             /* IN: User ID */
);

/* Get total size of messages confirmed */
extern unsigned long PopTotalSizeConfirmed(
  SPopUser*                             /* IN: User ID */
);

/* Get message size */
extern unsigned long PopMsgSize(
  SPopUser*,                            /* IN: User ID */
  unsigned                              /* IN: Message identifier, 1..PopMsgCount */
);

/* Get message unique ID */
extern const char* PopMsgUidl(          /* Returns 0= success */
  SPopUser*,                            /* IN: User ID */
  unsigned                              /* IN: Message identifier, 1..PopMsgCount */
);


/* Open a message for reading = fopen */
extern SPopMsg* PopMsgOpen(             /* Returns NULL on error */
  SPopUser*,                            /* IN: User ID */
  unsigned                              /* IN: Message identifier, 1..PopMsgCount */
);

/* Read from an open message = fgets */
extern char* PopMsgRead(
  char*,                                /* OUT: data */
  int,                                  /* IN: buffer size */
  SPopMsg*                              /* IN */
);

/* Close an open message = fclose */
extern int PopMsgClose(                 /* Returns 0= success */
  SPopUser*,                            /* IN: User ID */
  SPopMsg*                              /* IN */
);


/* Delete a message = remove */
extern int PopMsgDel(                   /* Returns 0= success */
  SPopUser*,                            /* IN: User ID */
  unsigned                              /* IN: Message identifier, 1..PopMsgCount */
);

/* Undelete a message */
extern int PopMsgUndelete(              /* Returns 0= success */
  SPopUser*,                            /* IN: User ID */
  unsigned                              /* IN: Message identifier, 1..PopMsgCount */
);

/* Is message deleted? */
extern int PopMsgIsDeleted(             /* Returns !0 if deleted */
  SPopUser*,                            /* IN: User ID */
  unsigned                              /* IN: Message identifier, 1..PopMsgCount */
);

/* Commit all deletions */
extern int PopMsgCommit(
  SPopUser*                             /* IN: User ID */
);


/* Get message header */
extern const char* PopMsgGetHdr(        /* Returns NULL on error */
  SPopUser*,                            /* IN: User ID */
  unsigned                              /* IN: Message identifier, 1..PopMsgCount */
);

/* Get header MIME info */
extern int PopMsgGetMimeInfo(           /* Returns 0= no error */
  SPopUser*,                            /* IN: User ID */
  unsigned,                             /* IN: Message identifier, 1..PopMsgCount */
  struct SMimeInfo*                     /* OUT */
);


/* Get reason for message acceptance/rejection */
extern const char* PopMsgGetRule(       /* Returns NULL if not rejected */
  SPopUser*,                            /* IN: User ID */
  unsigned                              /* IN: Message identifier, 1..PopMsgCount */
);

/* Get spam index */
extern double PopMsgSpamIndex(          /* 0..100, -ve on error */
  SPopUser*,                            /* IN: User ID */
  unsigned                              /* IN: Message identifier, 1..PopMsgCount */
);

/* Is message acceptance to be confirmed */
extern int PopMsgWantsConfirmation(
  SPopUser*,                            /* IN: User ID */
  unsigned                              /* IN: Message identifier, 1..PopMsgCount */
);


/* Message client callback */
typedef int FPopMsgCallback(
  enum EPopMsgNotify,                   /* IN: Reason */
  void*,                                /* IN: Client callback context */
  SPopUser*,                            /* IN: User ID */
  unsigned                              /* IN  Message identifier */
);

/* Install message callback */
extern FPopMsgCallback* PopMsgSetCallback(/* Returns old filter */
  FPopMsgCallback*,                     /* IN: Callback function */
  void*                                 /* IN: Client callback context */
);

/* Get bytes of message downloaded so far during kPopMsgNotifyMsg event */
extern unsigned long PopMsgBytesSoFar(
  SPopUser*,                            /* IN: User ID */
  unsigned                              /* IN: Message identifier, 1..PopMsgCount */
);


/* Block a sender */
extern int PopMsgBlockSender(
  SPopUser*,                            /* IN: User ID */
  const char*                           /* IN: Sender's mailbox */
);

/* Block a subject */
extern int PopMsgBlockSubject(
  SPopUser*,                            /* IN: User ID */
  const char*                           /* IN: Subject */
);

/* Block a host */
extern int PopMsgBlockHost(
  SPopUser*,                            /* IN: User ID */
  unsigned long,                        /* IN: IP address, host form */
  unsigned                              /* IN: CIDR subnet (1..32) */
);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* ndef POPMSG_H */
/* End of file */

