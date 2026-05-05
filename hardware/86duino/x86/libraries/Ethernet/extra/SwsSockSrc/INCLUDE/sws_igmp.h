/****************************************************************************
 *
 * File name   : sws_igmp.h
 * Function    : IGMP module public interface
 * Project     : SwsSock
 * Authors     : Lawrence Rust
 * Systems     : ANSI C
 * Version     : 1.0
 * Release Date:
 *
 ****************************************************************************
 * File History
 * ------------
 * 21-Dec-02, LVR, v1.00 - Created.
 *
 *
 * Comments:
 * --------
 * This file contains the private interface to the IGMP module
 *
 ****************************************************************************/

#ifndef SWS_IGMP_H
#define SWS_IGMP_H 0x0100 /* Version no. BCD */

/* Dependencies */
#include <stddef.h>   /* size_t */
#include "sws_host.h" /* Basic types */
#include "sws_buf.h"


/*
 * Constants
 */
#ifdef SWS_SOCK_TUNNEL
#define SWS_IGMP_PROTOCOL (SWS_IPPROTO_IGMP + 128) /* For testing with raw sockets */
#else
#define SWS_IGMP_PROTOCOL SWS_IPPROTO_IGMP
#endif

/* Example IGMPv2 Header from RFC2236, page 2

    0                   1                   2                   3
    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |      Type     | Max Resp Time |           Checksum            |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |                         Group Address                         |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*/

/* IGMP header offsets */
#define SWS_IGMP_TYPE  0 /* ui8 */
#define SWS_IGMP_TIME  1 /* ui8 */
#define SWS_IGMP_CSUM  2 /* ui16 */
#define SWS_IGMP_GROUP 4 /* ui32 */

#define SWS_IGMP_HDR   8 /* Minimum IGMP header length */

/* IGMP message types */
#define SWS_IGMP_QUERY   0x11    /* Host Membership Query */
#define SWS_IGMP_REPORT  0x12    /* V1 Host Membership Report */
#define SWS_IGMP_REPORT2 0x16    /* V2 Host Membership Report */
#define SWS_IGMP_LEAVE   0x17    /* V2 Leave group */
#define SWS_IGMP_REPORT3 0x22    /* V3 Host Membership Report RFC3376 */


struct SWS_IpOptions;


/*
 * Functions
 */
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Send an IGMP datagram */
SWS_EXPORT( int, SWS_IgmpSend(        /* Returns !0 on error */
  ui32,                               /* IN: Source IP address or SWS_INADDR_ANY */
  ui32,                               /* IN: Destination IP address, NOT SWS_INADDR_ANY */
  unsigned,                           /* IN: IGMP type */
  unsigned,                           /* IN: Max response time */
  ui32                                /* IN: Group address */
));


/* Process a received IGMP datagram */
SWS_EXPORT( int, SWS_IgmpRecv(        /* Returns !0 if handled */
  SWS_BufferH,                        /* IN: IP packet including header */
  size_t,                             /* IN: IP header length including options */
  ui32,                               /* IN: src IP address */
  ui32,                               /* IN: multicast address */
  ui32                                /* IN: specific dst IP address */
));


/* Initialise the module */
extern int SWS_IgmpInit( void);       /* Returns !0 on error */

/* Finalise the module */
extern void SWS_IgmpFinal( void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* ndef SWS_IGMP_H */
/* End of file */
