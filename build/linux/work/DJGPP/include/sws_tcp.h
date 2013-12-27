/****************************************************************************
 *
 * File name   : tcp.h
 * Function    : TCP transport protocol module public interface.
 * Project     : SwsSock
 * Authors     : Lawrence Rust
 * Systems     : ANSI C
 * Version     : 1.0
 * Release Date:
 *
 ****************************************************************************
 * File History
 * ------------
 * 14-May-01, LVR, v1.00 - Created.
 * 06-Jan-05, LVR, v1.01 - Added.SWS_TcpEnum
 * 05-Jul-09, LVR, v1.02 - Added.SWS_TCP_ECEF & SWS_TCP_CWRF
 *
 *
 * Comments:
 * --------
 * This file contains the private interface to the TCP module
 *
 ****************************************************************************/

#ifndef SWS_TCP_H
#define SWS_TCP_H 0x0102 /* Version no. BCD */

/* Dependencies */
#include "sws_host.h"

/* Forward refs */


/* Macros */
#ifdef SWS_SOCK_TUNNEL
#define SWS_TCP_PROTOCOL (SWS_IPPROTO_TCP + 128) /* For testing with raw sockets */
#else
#define SWS_TCP_PROTOCOL SWS_IPPROTO_TCP
#endif

/* TCP Header Format from RFC 793 p15

    0                   1                   2                   3
    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |          Source Port          |       Destination Port        |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |                        Sequence Number                        |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |                    Acknowledgment Number                      |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |  Data |           |U|A|P|R|S|F|                               |
   | Offset| Reserved  |R|C|S|S|Y|I|            Window             |
   |       |           |G|K|H|T|N|N|                               |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |           Checksum            |         Urgent Pointer        |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |                    Options                    |    Padding    |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |                             data                              |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*/

/* TCP header offsets */
#define SWS_TCP_SRC 0   /* ui16 */
#define SWS_TCP_DST 2   /* ui16 */
#define SWS_TCP_SEQ 4   /* ui32 */
#define SWS_TCP_ACK 8   /* ui32 */
#define SWS_TCP_FLG 12  /* ui16 */
#define SWS_TCP_WND 14  /* ui16 */
#define SWS_TCP_SUM 16  /* ui16 */
#define SWS_TCP_URG 18  /* ui16 */
#define SWS_TCP_OPT 20  /* ui32 padded, variable length */

#define SWS_TCP_MINLEN SWS_TCP_OPT
#define SWS_TCP_MAXLEN 60

#define SWS_TCP_FINF (1<<0)
#define SWS_TCP_SYNF (1<<1)
#define SWS_TCP_RSTF (1<<2)
#define SWS_TCP_PSHF (1<<3)
#define SWS_TCP_ACKF (1<<4)
#define SWS_TCP_URGF (1<<5)
/*http://www.iana.org/assignments/tcp-header-flags

   RFC 3168 defines two of the six bits from the Reserved field to be
   used for ECN, as follows:

     0   1   2   3   4   5   6   7   8   9  10  11  12  13  14  15
   +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
   |               |               | C | E | U | A | P | R | S | F |
   | Header Length |    Reserved   | W | C | R | C | S | S | Y | I |
   |               |               | R | E | G | K | H | T | N | N |
   +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
*/
#define SWS_TCP_ECEF (1<<6) /* ECN-Echo */
#define SWS_TCP_CWRF (1<<7) /* Congestion Window Reduced */
#define SWS_TCP_MBZF (0xf<<8)

/* TCP options */
#define SWS_TCP_OPT_END 0 /* RFC793, len 1 */
#define SWS_TCP_OPT_NOP 1 /* RFC793, len 1 */
#define SWS_TCP_OPT_MSS 2 /* RFC793 */
#define SWS_TCP_OPT_MSS_SIZE 4
#define SWS_TCP_OPT_WS 3  /* RFC1323 */
#define SWS_TCP_OPT_WS_SIZE 3
#define SWS_TCP_OPT_SACKP 4 /* RFC2018 (obsoletes 1072) */
#define SWS_TCP_OPT_SACKP_SIZE 2
#define SWS_TCP_OPT_SACK 5  /* RFC2018 (obsoletes 1072) */
#define SWS_TCP_OPT_ECHO 6  /* RFC1072 */
#define SWS_TCP_OPT_ECHOR 7 /* RFC1072 */
#define SWS_TCP_OPT_TSOPT 8 /* RFC1323 */
#define SWS_TCP_OPT_POCP 9  /* RFC1693 */
#define SWS_TCP_OPT_POSP 10 /* RFC1693 */
#define SWS_TCP_OPT_CC 11
#define SWS_TCP_OPT_CCNEW 12
#define SWS_TCP_OPT_CCECHO 13
#define SWS_TCP_OPT_CSUMREQ 14  /* RFC1146 */
#define SWS_TCP_OPT_CSUMDATA 15 /* RFC1146 */
#define SWS_TCP_OPT_SKEETER 16
#define SWS_TCP_OPT_BUBBA 17
#define SWS_TCP_OPT_TRAIL 18


/*
 * Types
 */
typedef struct SWS_STcpConn
  {
  int iState;
  ui32 localHost;                     /* Host form */
  ui16 localPort;                     /* Host form */
  ui32 remoteHost;                    /* Host form */
  ui16 remotePort;                    /* Host form */
  } SWS_STcpConn;

/* SWS_TcpEnum callback */
typedef SWS_CB( int, SWS_FTcpEnum(    /* Return !0 to stop enumeration */
  SWS_PTR( void),                     /* IN: Context to SWS_TcpEnum */
  SWS_PTR( const SWS_STcpConn)        /* IN: End point info */
));


/*
 * Functions
 */
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Enumerate TCP connections */
SWS_EXPORT( int, SWS_TcpEnum(         /* Returns 0 or value from Callback */
  SWS_FNPTR( SWS_FTcpEnum),           /* IN: Callback */
  SWS_PTR( void)                      /* IN: Callback context */
));

/* Initialise the module */
extern int SWS_TcpInit( void);        /* Returns !0 on error */

/* Finalise the module */
extern void SWS_TcpCleanup( void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* ndef SWS_TCP_H */
/* End of file */
