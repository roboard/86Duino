/****************************************************************************
 *
 * File name   : sws_ntp.h
 * Function    : Public API for the NTP client
 * Project     : SwsSock
 * Authors     : Lawrence Rust
 * Systems     : ANSI C
 * Version     : 1.0
 *
 ****************************************************************************
 * File History
 * ------------
 * 27-Feb-03, LVR, v1.00 - Created.
 *
 * Comments:
 * --------
 * This file contains the external interface to the NTP client module
 *
 ****************************************************************************/

#ifndef SWS_NTP_H
#define SWS_NTP_H 0x0100 /* Version no. BCD */

/* Dependencies */
#include <stddef.h>   /* size_t */
#include "sws_host.h" /* Basic types */


/* Constants */

#define SWS_NTP_SERVER 123 /* UDP */

/* NTP versoon 1 message from RFC1059 page 44

    0                   1                   2                   3
    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |LI | VN  | MODE|    Stratum    |      Poll     |   Precision   |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |                     Synchronizing Distance                    |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |                     Estimated Drift Rate                      |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |                  Reference Clock Identifier                   |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |                                                               |
   |                 Reference Timestamp (64 bits)                 |
   |                                                               |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |                                                               |
   |                 Originate Timestamp (64 bits)                 |
   |                                                               |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |                                                               |
   |                  Receive Timestamp (64 bits)                  |
   |                                                               |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |                                                               |
   |                  Transmit Timestamp (64 bits)                 |
   |                                                               |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

*/

/* NTP message offsets */
#define SWS_NTP_HDR_MODE      0
#define SWS_NTP_HDR_STRATUM   1
#define SWS_NTP_HDR_POLL      2
#define SWS_NTP_HDR_PRECISION 3
#define SWS_NTP_HDR_DISTANCE  4
#define SWS_NTP_HDR_DRIFT     8
#define SWS_NTP_HDR_REFID    12
#define SWS_NTP_HDR_REFTIME  16
#define SWS_NTP_HDR_ORIGTIME 24
#define SWS_NTP_HDR_RECVTIME 32
#define SWS_NTP_HDR_SENDTIME 40
#define SWS_NTP_HDR_LEN      48U

/* Leap Indicator (LI) */
#define SWS_NTP_LI_SHIFT    6
#define SWS_NTP_LI_MASK     (3<<SWS_NTP_LI_SHIFT)
#define SWS_NTP_LI_NONE     (0<<SWS_NTP_LI_SHIFT)
#define SWS_NTP_LI_PLUS1    (1<<SWS_NTP_LI_SHIFT)
#define SWS_NTP_LI_MINUS1   (2<<SWS_NTP_LI_SHIFT)
#define SWS_NTP_LI_ALARM    (3<<SWS_NTP_LI_SHIFT)

/* Version Number (VN) */
#define SWS_NTP_VN_SHIFT    3
#define SWS_NTP_VN_MASK     (7<<SWS_NTP_VN_SHIFT)
#define SWS_NTP_VN_1        (1<<SWS_NTP_VN_SHIFT)
#define SWS_NTP_VN_2        (2<<SWS_NTP_VN_SHIFT)
#define SWS_NTP_VN_3        (3<<SWS_NTP_VN_SHIFT)

/* Mode (v2 and later) */
#define SWS_NTP_MODE_SHIFT  0
#define SWS_NTP_MODE_MASK   (7<<SWS_NTP_MODE_SHIFT)
#define SWS_NTP_MODE_NONE   (0<<SWS_NTP_MODE_SHIFT)
#define SWS_NTP_MODE_ACTIVE (1<<SWS_NTP_MODE_SHIFT)
#define SWS_NTP_MODE_PASV   (2<<SWS_NTP_MODE_SHIFT)
#define SWS_NTP_MODE_CLIENT (3<<SWS_NTP_MODE_SHIFT)
#define SWS_NTP_MODE_SERVER (4<<SWS_NTP_MODE_SHIFT)
#define SWS_NTP_MODE_BCAST  (5<<SWS_NTP_MODE_SHIFT)
#define SWS_NTP_MODE_CTRL   (6<<SWS_NTP_MODE_SHIFT)
#define SWS_NTP_MODE_PRIV   (7<<SWS_NTP_MODE_SHIFT)

/* Stratum */
#define SWS_NTP_STRATUM_NOME 0
#define SWS_NTP_STRATUM_PRI  1
#define SWS_NTP_STRATUM_SEC  2



/* Types */
struct sockaddr;

/* Fixed point time */
typedef struct SWS_SNtpTime
  {
  ui32 secs;          /* Seconds since 0:00 on Jan 1st 1900 */
  ui32 fract;         /* 2**-32 seconds */
  } SWS_SNtpTime;

/* NTP record */
typedef struct SWS_SNtpRecord
  {
  unsigned uVersion;  /* NTP reply version */
  unsigned uLeap;     /* Leap second indicator */
  unsigned uStratum;  /* NTP server stratum */
  int iPrecision;     /* Precision in seconds to the nearest power of 2 */
  ui32 uError;        /* FP est. error in seconds when time last set. FP between b15 and 16 */
  i32 iDrift;         /* FP est. drift rate. FP left of the high-order bit */
  ui32 addrRef;       /* IP address of reference if uStratum >= 2 */
  SWS_SNtpTime tNet;  /* NTP time when reply received corrected for time of flight */
  SWS_SNtpTime tSet;  /* Time of last adjustment */
  SWS_SNtpTime tSys;  /* System time when reply received */
  SWS_SNtpTime tRtt;  /* Round trip time excluding server processing */
  } SWS_SNtpRecord;


/*
 * Functions
 */
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Set the hostname */
extern int SWS_NtpQuery(              /* Returns !0 on error */
  SWS_PTR( SWS_SNtpRecord),           /* OUT: */
  SWS_PTR( struct sockaddr),          /* IN: -> server */
  size_t                              /* IN: sockaddr size */
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* ndef SWS_NTP_H */
/* End of file */
