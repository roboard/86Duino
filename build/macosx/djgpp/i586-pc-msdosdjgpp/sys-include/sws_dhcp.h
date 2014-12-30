/****************************************************************************
 *
 * File name   : sws_dhcp.h
 * Function    : Public API for the DHCP client
 * Project     : SwsSock
 * Authors     : Lawrence Rust
 * Systems     : ANSI C
 * Version     : 1.0
 *
 ****************************************************************************
 * File History
 * ------------
 * 22-Nov-02, LVR, v1.00 - Created.
 *
 * Comments:
 * --------
 * This file contains the external interface to the DHCP client module
 *
 ****************************************************************************/

#ifndef SWS_DHCP_H
#define SWS_DHCP_H 0x0100 /* Version no. BCD */

/* Dependencies */
#include <stddef.h>   /* size_t */
#include "sws_host.h" /* Basic types */


/* Constants */

#define SWS_DHCP_CLIENT 68 /* BOOTP client */
#define SWS_DHCP_SERVER 67 /* BOOTP server */

/* DHCP message from RFC2131 page 9

   0                   1                   2                   3
   0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |     op (1)    |   htype (1)   |   hlen (1)    |   hops (1)    |
   +---------------+---------------+---------------+---------------+
   |                            xid (4)                            |
   +-------------------------------+-------------------------------+
   |           secs (2)            |           flags (2)           |
   +-------------------------------+-------------------------------+
   |                          ciaddr  (4)                          |
   +---------------------------------------------------------------+
   |                          yiaddr  (4)                          |
   +---------------------------------------------------------------+
   |                          siaddr  (4)                          |
   +---------------------------------------------------------------+
   |                          giaddr  (4)                          |
   +---------------------------------------------------------------+
   |                                                               |
   |                          chaddr  (16)                         |
   |                                                               |
   |                                                               |
   +---------------------------------------------------------------+
   |                                                               |
   |                          sname   (64)                         |
   +---------------------------------------------------------------+
   |                                                               |
   |                          file    (128)                        |
   +---------------------------------------------------------------+
   |                                                               |
   |                          options (variable)                   |
   +---------------------------------------------------------------+
*/

/* DHCP message offsets */
#define SWS_DHCP_OP      0
#define SWS_DHCP_HTYPE   1 /* Hardware address type */
#define SWS_DHCP_HLEN    2
#define SWS_DHCP_HOPS    3
#define SWS_DHCP_XID     4
#define SWS_DHCP_SECS    8
#define SWS_DHCP_FLAGS  10
#define SWS_DHCP_CIADDR 12
#define SWS_DHCP_YIADDR 16
#define SWS_DHCP_SIADDR 20
#define SWS_DHCP_GIADDR 24
#define SWS_DHCP_CHADDR 28
#define SWS_DHCP_CHADDR_LEN 16
#define SWS_DHCP_SNAME  44
#define SWS_DHCP_SNAME_LEN  64
#define SWS_DHCP_FILE  108
#define SWS_DHCP_FILE_LEN  128
#define SWS_DHCP_OPTS  236

#define SWS_DHCP_OP_BOOTREQUEST 1
#define SWS_DHCP_OP_BOOTREPLY   2

#define SWS_DHCP_HTYPE_ETHERNET 1 /* RFC 1700 p163 */
#define SWS_DHCP_HLEN_ETHERNET  6

#define SWS_DHCP_COOKIE1 99
#define SWS_DHCP_COOKIE2 130
#define SWS_DHCP_COOKIE3 83
#define SWS_DHCP_COOKIE4 99

/* BOOTP Vendor Extensions RFC1497 */
#define SWS_DHCP_OPT_END        255 /* Len 1 */
#define SWS_DHCP_OPT_NOP          0 /* Len 1 */
#define SWS_DHCP_OPT_SUBNET_MASK  1
#define SWS_DHCP_OPT_TOFFSET      2
#define SWS_DHCP_OPT_ROUTER       3
#define SWS_DHCP_OPT_TIME_SERVER  4
#define SWS_DHCP_OPT_NAME_SERVER  5 /* IEN-116 Server */
#define SWS_DHCP_OPT_DNS          6 /* DNS Server */
#define SWS_DHCP_OPT_HOST_NAME   12
#define SWS_DHCP_OPT_DOMAIN_NAME 15
#define SWS_DHCP_OPT_IP_TTL      23
#define SWS_DHCP_OPT_MTU         26

/* DHCP specific option types RFC2132 */
#define SWS_DHCP_OPT_IP_ADDRESS  50
#define SWS_DHCP_OPT_LEASE_TIME  51
#define SWS_DHCP_OPT_OVERLOAD    52
#define SWS_DHCP_OPT_MSGTYPE     53
#define SWS_DHCP_OPT_SERVERID    54
#define SWS_DHCP_OPT_PARAMETERS  55
#define SWS_DHCP_OPT_MESSAGE     56
#define SWS_DHCP_OPT_MAX_SIZE    57
#define SWS_DHCP_OPT_T1_TIME     58
#define SWS_DHCP_OPT_T2_TIME     59
#define SWS_DHCP_OPT_CLASSID     60
#define SWS_DHCP_OPT_CLIENTID    61 /* HW type 1= MAC address */

/* codes 128 to 254 (decimal) are reserved for site-specific options */

#define SWS_DHCP_MSGTYPE_DHCPDISCOVER 1
#define SWS_DHCP_MSGTYPE_DHCPOFFER    2
#define SWS_DHCP_MSGTYPE_DHCPREQUEST  3
#define SWS_DHCP_MSGTYPE_DHCPDECLINE  4
#define SWS_DHCP_MSGTYPE_DHCPACK      5
#define SWS_DHCP_MSGTYPE_DHCPNAK      6
#define SWS_DHCP_MSGTYPE_DHCPRELEASE  7

/* Types */


/*
 * Functions
 */
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

SWS_EXPORT( int, SWS_DhcpRenew(
  int                                 /* Datalink ID, */
));

SWS_EXPORT( int, SWS_DhcpRelease(
  int                                 /* Datalink ID, */
));


/* Initialise the DHCP client */
extern int SWS_DhcpInit( void);       /* Returns !0 on error */

/* Finalise the DHCP client */
extern void SWS_DhcpCleanup( void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* ndef SWS_DHCP_H */
/* End of file */
