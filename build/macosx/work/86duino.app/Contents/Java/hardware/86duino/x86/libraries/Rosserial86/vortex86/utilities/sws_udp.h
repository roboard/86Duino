/****************************************************************************
 *
 * File name   : udp.h
 * Function    : UDP transport protocol module public interface.
 * Project     : SwsSock
 * Authors     : Lawrence Rust
 * Systems     : ANSI C
 * Version     : 1.0
 * Release Date:
 *
 ****************************************************************************
 * File History
 * ------------
 * 19-Apr-01, LVR, v1.00 - Created.
 *
 *
 * Comments:
 * --------
 * This file contains the private interface to the UDP module
 *
 ****************************************************************************/

#ifndef SWS_UDP_H
#define SWS_UDP_H 0x0100 /* Version no. BCD */

/* Dependencies */
#include "sws_host.h"

/* Forward refs */


/* Macros */
#ifdef SWS_SOCK_TUNNEL
#define SWS_UDP_PROTOCOL (SWS_IPPROTO_UDP + 128) /* For testing with raw sockets */
#else
#define SWS_UDP_PROTOCOL SWS_IPPROTO_UDP
#endif

/* UDP header from RFC768 page 1

                  0      7 8     15 16    23 24    31
                 +--------+--------+--------+--------+
                 |     Source      |   Destination   |
                 |      Port       |      Port       |
                 +--------+--------+--------+--------+
                 |                 |                 |
                 |     Length      |    Checksum     |
                 +--------+--------+--------+--------+
                 |
                 |          data octets ...
                 +---------------- ...
*/

/* UDP header offsets */
#define SWS_UDP_SRC 0
#define SWS_UDP_DST 2
#define SWS_UDP_LEN 4
#define SWS_UDP_SUM 6
#define SWS_UDP_HDR 8 /* Header length */


/*
 * Functions
 */
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Initialise the module */
extern int SWS_UdpInit( void);        /* Returns !0 on error */

/* Finalise the module */
extern void SWS_UdpCleanup( void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* ndef SWS_UDP_H */
/* End of file */
