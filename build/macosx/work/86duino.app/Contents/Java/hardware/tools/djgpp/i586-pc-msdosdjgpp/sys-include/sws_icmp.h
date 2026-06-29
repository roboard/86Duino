/****************************************************************************
 *
 * File name   : icmp.h
 * Function    : ICMP module public interface
 * Project     : SwsSock
 * Authors     : Lawrence Rust
 * Systems     : ANSI C
 * Version     : 1.0
 * Release Date:
 *
 ****************************************************************************
 * File History
 * ------------
 * 03-Apr-01, LVR, v1.00 - Created.
 * 11-Nov-02, LVR, v1.01 - Added SWS_IcmpInit
 * 25-Nov-02, LVR, v1.03 - Move all ICMP definitions from sws_ip.h
 * 16-Aug-09, LVR, v1.04 - Add dst IP to SWS_IcmpRecv
 *
 *
 * Comments:
 * --------
 * This file contains the private interface to the ICMP module
 *
 ****************************************************************************/

#ifndef SWS_ICMP_H
#define SWS_ICMP_H 0x0104 /* Version no. BCD */

/* Dependencies */
#include <stddef.h>   /* size_t */
#include "sws_host.h" /* Basic types */
#include "sws_buf.h"


/*
 * Constants
 */
#ifdef SWS_SOCK_TUNNEL
#define SWS_ICMP_PROTOCOL (SWS_IPPROTO_ICMP + 128) /* For testing with raw sockets */
#else
#define SWS_ICMP_PROTOCOL SWS_IPPROTO_ICMP
#endif

/* Example ICMP Header from RFC792, page 4

    0                   1                   2                   3
    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |     Type      |     Code      |          Checksum             |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |           Identifier          |        Sequence Number        |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |     Data ...
   +-+-+-+-+-
*/

/* ICMP header offsets */
#define SWS_ICMP_TYPE 0 /* ui8 */
#define SWS_ICMP_CODE 1 /* ui8 */
#define SWS_ICMP_CSUM 2 /* ui16 */
#define SWS_ICMP_DATA 4 /* ui32 */

#define SWS_ICMP_HDR 8                /* Minimum ICMP header length */

/* ICMP message types */
#define SWS_ICMP_ECHO_REPLY     0    /* Echo Reply */
                                     /* 1,2 unassigned */
#define SWS_ICMP_DEST_UNREACH   3    /* Destination Unreachable */
#define SWS_ICMP_QUENCH         4    /* Source Quench */
#define SWS_ICMP_REDIRECT       5    /* Redirect */
                                     /* 6= Alternate Host Address */
                                     /* 7= unassigned */
#define SWS_ICMP_ECHO           8    /* Echo Request */
#define SWS_ICMP_ROUTER_ADVERT  9    /* Router Advertisement RFC 1256 */
#define SWS_ICMP_ROUTER_SOLICIT 10   /* Router Solicitation RFC 1256 */
#define SWS_ICMP_TIME_EXCEED    11   /* Time-to-live Exceeded */
#define SWS_ICMP_PARAM_PROB     12   /* Parameter Problem */
#define SWS_ICMP_TIMESTAMP      13   /* Timestamp */
#define SWS_ICMP_TIME_REPLY     14   /* Timestamp Reply */
#define SWS_ICMP_INFO_RQST      15   /* Information Request */
#define SWS_ICMP_INFO_REPLY     16   /* Information Reply */
#define SWS_ICMP_ADDR_MASK      17   /* Address mask request RFC 950 */
#define SWS_ICMP_ADDR_MASK_REPLY 18  /* Address mask reply RFC 950 */
#define SWS_ICMP_IPSP           19   /* Problem with secured packet */
                                     /* 20..29 Robustness Experiment */
#define SWS_ICMP_TRACEROUTE     30   /* Traceroute RFC1393 */
#define SWS_ICMP_CONVERT_ERR    31   /* Datagram Conversion Error RFC1475 */
                                     /* 32..40 see http://www.iana.org/assignments/icmp-parameters */
#define SWS_ICMP_TYPES          41   /* 41.255 Reserved */

/* ICMP Destination Unreachable codes */
#define SWS_ICMP_NET_UNREACH    0    /* Net unreachable */
#define SWS_ICMP_HOST_UNREACH   1    /* Host unreachable */
#define SWS_ICMP_PROT_UNREACH   2    /* Protocol unreachable */
#define SWS_ICMP_PORT_UNREACH   3    /* Port unreachable */
#define SWS_ICMP_FRAG_NEEDED    4    /* Fragmentation needed and DF set */
#define SWS_ICMP_ROUTE_FAIL     5    /* Source route failed */
#define SWS_ICMP_NET_UNKNOWN    6    /* Destination Network Unknown */
#define SWS_ICMP_HOST_UNKNOWN   7    /* Destination Host Unknown */
#define SWS_ICMP_HOST_ISOLATED  8    /* Source Host Isolated */
#define SWS_ICMP_NET_PROHIB     9    /* Destination Network is Administratively Prohibited */
#define SWS_ICMP_HOST_PROHIB   10    /* Destination Host is Administratively Prohibited */
#define SWS_ICMP_NET_UNREACH_TOS 11  /* Destination Network Unreachable for Type of Service */
#define SWS_ICMP_HOST_UNREACH_TOS 12 /* Destination Host Unreachable for Type of Service */
#define SWS_ICMP_ADMIN_PROHIB  13    /* Administrativly prohibited RFC1812,*/
#define SWS_ICMP_PREC_VIOL     14    /* Host Precedence Violation RFC1812,*/
#define SWS_ICMP_PREC_CUTOFF   15    /* Precedence cutoff in effect RFC1812,*/

/* ICMP Time Exceeded codes */
#define SWS_ICMP_TTL_EXCEED     0    /* Time-to-live exceeded */
#define SWS_ICMP_FRAG_EXCEED    1    /* Fragment reassembly time exceeded */

/* ICMP Redirect message codes */
#define SWS_ICMP_REDR_NET       0    /* Redirect for the network */
#define SWS_ICMP_REDR_HOST      1    /* Redirect for the host */
#define SWS_ICMP_REDR_TOS_NET   2    /* Redirect for Type of Service & network */
#define SWS_ICMP_REDR_TOS_HOST  3    /* Redirect for Type of Service & host */

/* ICMP Bad security packet codes */
#define SWS_ICMP_IPSP_SPI_UNKNOWN   0
#define SWS_ICMP_IPSP_AUTH_FAIL     1
#define SWS_ICMP_IPSP_DECRYPT_FAIL  2


struct SWS_IpOptions;


/*
 * Functions
 */
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Send an ICMP datagram */
SWS_EXPORT( int, SWS_IcmpSend(       /* Returns !0 on error */
  ui32,                               /* IN: Source IP address or SWS_INADDR_ANY */
  ui32,                               /* IN: Destination IP address, NOT SWS_INADDR_ANY */
  unsigned,                           /* IN: ICMP type */
  unsigned,                           /* IN: ICMP code */
  ui32,                               /* IN: ICMP optional data (identifier & sequence) */
  SWS_BufferH,                        /* IN: Datagram content !!!MODIFIED!!!, NULL = none */
  unsigned,                           /* IN: IP ident, 0= don't care */
  SWS_PTR( struct SWS_IpOptions)      /* IN: IP option data, maybe NULL */
));

/* Process an ICMP datagram */
SWS_EXPORT( int, SWS_IcmpRecv(        /* Returns !0 if handled */
  SWS_BufferH,                        /* IN: IP packet including header */
  size_t,                             /* IN: IP header length including options */
  ui32,                               /* IN: src IP address */
  ui32,                               /* IN: dst IP address */
  ui32                                /* IN: specific dst IP address */
));


/* Initialise the module */
extern int SWS_IcmpInit( void);       /* Returns !0 on error */

/* Finalise the module */
extern void SWS_IcmpFinal( void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* ndef SWS_ICMP_H */
/* End of file */
