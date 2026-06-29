/****************************************************************************
 *
 * File name   : sws_rip.h
 * Function    : Public API for the RIP client
 * Project     : SwsSock
 * Authors     : Lawrence Rust
 * Systems     : ANSI C
 * Version     : 1.0
 *
 ****************************************************************************
 * File History
 * ------------
 * 06-Jan-03, LVR, v1.00 - Created.
 *
 * Comments:
 * --------
 * This file contains the external interface to the RIP module
 *
 ****************************************************************************/

#ifndef SWS_RIP_H
#define SWS_RIP_H 0x0100 /* Version no. BCD */

/* Dependencies */
#include <stddef.h>   /* size_t */
#include "sws_host.h" /* Basic types */


/* Constants */

#define SWS_RIP_SERVER 520 /* RIP server */

/* RIP message from RFC2453 page 20

       0                   1                   2                   3
       0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      |  command (1)  |  version (1)  |       must be zero (2)        |
      +---------------+---------------+-------------------------------+
      |                                                               |
      ~                         RIP Entry (20)                        ~
      |                                                               |
      +---------------+---------------+---------------+---------------+

   There may be between 1 and 25 (inclusive) RIP entries.  A RIP-1 entry
   has the following format:

       0                   1                   2                   3
       0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
      | address family identifier (2) |      must be zero (2)         |
      +-------------------------------+-------------------------------+
      |                        IPv4 address (4)                       |
      +---------------------------------------------------------------+
      |                        must be zero (4)                       |
      +---------------------------------------------------------------+
      |                        must be zero (4)                       |
      +---------------------------------------------------------------+
      |                           metric (4)                          |
      +---------------------------------------------------------------+

   p31: The format for the 20-octet route entry (RTE) for RIP-2 is:

    0                   1                   2                   3 3
    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   | Address Family Identifier (2) |        Route Tag (2)          |
   +-------------------------------+-------------------------------+
   |                         IP Address (4)                        |
   +---------------------------------------------------------------+
   |                         Subnet Mask (4)                       |
   +---------------------------------------------------------------+
   |                         Next Hop (4)                          |
   +---------------------------------------------------------------+
   |                         Metric (4)                            |
   +---------------------------------------------------------------+
*/

/* RIP header offsets */
#define SWS_RIP_HDR_CMD  0 /* ui8 */
#define SWS_RIP_HDR_VER  1 /* ui8 */
#define SWS_RIP_HDR_MBZ  2 /* ui16 */
#define SWS_RIP_HDR_LEN  4

/* RIP commands */
#define SWS_RIP_CMD_REQUEST  1
#define SWS_RIP_CMD_RESPONSE 2

/* RIP RTE offsets */
#define SWS_RIP_RTE_AF       0 /* ui16, typically AF_INET (2) */
#define SWS_RIP_RTE_TAG      2 /* ui16 RIP-2 */
#define SWS_RIP_RTE_ADDR     4 /* ui32 */
#define SWS_RIP_RTE_MASK     8 /* ui32 RIP-2 */
#define SWS_RIP_RTE_NEXT    12 /* ui32 RIP-2 */
#define SWS_RIP_RTE_METRIC  16 /* ui32 1..16 */
#define SWS_RIP_RTE_LEN     20

/* RIP-2 multicast group */
#define SWS_RIP_MULTICAST_GROUP 0xe0000009L


/* Types */


/*
 * Functions
 */
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* ndef SWS_RIP_H */
/* End of file */
