/****************************************************************************
 *
 * File name   : sws_ppp.h
 * Function    : Public API to the PPP module
 * Project     : SwsSock
 * Authors     : Lawrence Rust
 * Systems     : ANSI C
 * Version     : 1.0
 *
 ****************************************************************************
 * File History
 * ------------
 * 11-Aug-05, LVR, v1.00 - Created.
 *
 * Comments:
 * --------
 * This file contains the external interface to the PPP module
 *
 ****************************************************************************/

#ifndef SWS_PPP_H
#define SWS_PPP_H 0x0100 /* Version no. BCD */

/* Dependencies */
#include "sws_host.h" /* Basic types */


/* PPP encapsulated message from RFC 1661 page 4

           +----------+-------------+---------+
           | Protocol | Information | Padding |
           | 8/16 bits|      *      |    *    |
           +----------+-------------+---------+


An LCP packet from RFC 1661 page 26

    0                   1                   2                   3
    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |     Code      |  Identifier   |            Length             |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |    Data ...
   +-+-+-+-+


LCP options from RFC 1661 page 39

    0                   1
    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |     Type      |    Length     |    Data ...
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+


An IPCP packet from RFC 1332 page 6

    0                   1                   2                   3
    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |     Type      |    Length     |     IP-Compression-Protocol   |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |    Data ...
   +-+-+-+-+
*/

/* Constants */

/* PPP protocols from ppp-numberrs.txt @ www.iana.org/numbers.html */
#define SWS_PPP_IP    ((ui16)0x0021)  /* IPv4 */
#define SWS_PPP_AT    ((ui16)0x0029)  /* AppleTalk */
#define SWS_PPP_IPX   ((ui16)0x002b)  /* IPX */
#define SWS_PPP_VJ    ((ui16)0x002d)  /* Van Jacobson Compressed TCP/IP */
#define SWS_PPP_VJU   ((ui16)0x002f)  /* Van Jacobson Uncompressed TCP/IP */

#define SWS_PPP_IPCP  ((ui16)0x8021)  /* IPCP */
#define SWS_PPP_ATCP  ((ui16)0x8029)  /* ATCP */
#define SWS_PPP_IPXCP ((ui16)0x802b)  /* IPXCP */
#define SWS_PPP_NBFCP ((ui16)0x803f)  /* NBFCP */
#define SWS_PPP_IP6CP ((ui16)0x8057)  /* IPV6CP */
#define SWS_PPP_CCP   ((ui16)0x80fd)  /* CCP RFC 1962 */

#define SWS_PPP_LCP   ((ui16)0xc021)  /* LCP */
#define SWS_PPP_PAP   ((ui16)0xc023)  /* Password Authentication Protocol */
#define SWS_PPP_SHIVA ((ui16)0xc027)  /* Shiva Password Authentication Protocol */
#define SWS_PPP_CHAP  ((ui16)0xc223)  /* Challenge Handshake Authentication Protocol */

/* PPP Information header offsets */
#define SWS_PPP_HDR_CODE 0  /* ui8 */
#define SWS_PPP_HDR_ID   1  /* ui8 */
#define SWS_PPP_HDR_LEN  2  /* ui16 */
#define SWS_PPP_HDR_DATA 4  /* variable */

/* LCP/IPCP packet codes, RFC 1661 p27 */
#define SWS_PPP_CONFIGURE_VENDOR 0 /* RFC2153 */
#define SWS_PPP_CONFIGURE_REQ 1
#define SWS_PPP_CONFIGURE_ACK 2
#define SWS_PPP_CONFIGURE_NAK 3
#define SWS_PPP_CONFIGURE_REJ 4
#define SWS_PPP_TERMINATE_REQ 5
#define SWS_PPP_TERMINATE_ACK 6
#define SWS_PPP_CODE_REJ      7
#define SWS_PPP_PROTOCOL_REJ  8
#define SWS_PPP_ECHO_REQ      9
#define SWS_PPP_ECHO_REPLY    10
#define SWS_PPP_DISCARD_REQ   11
#define SWS_PPP_IDENT         12 /* RFC1570 */
#define SWS_PPP_TIMEREM       13 /* RFC1570 */

/* Option header offsets */
#define SWS_PPP_OPT_TYPE 0  /* ui8 */
#define SWS_PPP_OPT_LEN  1  /* ui8 */
#define SWS_PPP_OPT_DATA 2  /* variable */

/* LCP configure options, RFC 1661 p39, RFC 1570 & 2153 */
#define SWS_PPP_LCP_VENDOR    0 /*  RFC2153 */
#define SWS_PPP_LCP_MRU       1
#define SWS_PPP_LCP_ACCM      2
#define SWS_PPP_LCP_AUTH      3
#define SWS_PPP_LCP_QUAL      4
#define SWS_PPP_LCP_MAGIC     5
#define SWS_PPP_LCP_PFC       7
#define SWS_PPP_LCP_ACFC      8
#define SWS_PPP_LCP_FCS       9 /* RFC 1570 */
#define SWS_PPP_LCP_PAD      10 /* RFC 1570 */
#define SWS_PPP_LCP_NUMMODE  11 /* RFC 1663 */
#define SWS_PPP_LCP_CALLBACK 13 /* RFC 1570 */
#define SWS_PPP_LCP_ML_MRRU  17 /* RFC1717 */
#define SWS_PPP_LCP_ML_SSNH  18 /* RFC1717 */
#define SWS_PPP_LCP_ML_EPD   19 /* RFC1717 */
#define SWS_PPP_LCP_BACP     23 /* RFC 2125 */

/* IPCP configure options, RFC 1332 & 1877 */
#define SWS_PPP_IPCP_VENDOR      0
#define SWS_PPP_IPCP_IPADDRS     1
#define SWS_PPP_IPCP_IPCOMP      2
#define SWS_PPP_IPCP_IPADDR      3
#define SWS_PPP_IPCP_PRIDNS    129 /* RFC1877 */
#define SWS_PPP_IPCP_PRINBNS   130 /* RFC1877 */
#define SWS_PPP_IPCP_SECDNS    131 /* RFC1877 */
#define SWS_PPP_IPCP_SECNBNS   132 /* RFC1877 */

/* CHAP packet codes, RFC1994 p6 */
#define SWS_PPP_CHAP_CHALLENGE  1
#define SWS_PPP_CHAP_RESPONSE   2
#define SWS_PPP_CHAP_SUCCESS    3
#define SWS_PPP_CHAP_FAILURE    4

/* PAP packet codes, RFC1334 p5 */
#define SWS_PPP_PAP_REQ         1
#define SWS_PPP_PAP_ACK         2
#define SWS_PPP_PAP_NAK         3

/* CCP packet codes, RFC1962 */
#define SWS_PPP_CCP_RST_REQ    14
#define SWS_PPP_CCP_RST_RPY    15


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

#endif /* ndef SWS_PPP_H */
/* End of file */
