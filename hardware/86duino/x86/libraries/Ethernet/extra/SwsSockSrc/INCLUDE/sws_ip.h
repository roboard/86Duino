/****************************************************************************
 *
 * File name   : sws_ip.h
 * Function    : IP module public interface
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
 * 17-Oct-01, LVR, v1.01 - Added SWS_FIpSendInd
 * 31-Oct-02, LVR, v1.02 - Added SWS_kErrorTxNoRoute
 * 25-Nov-02, LVR, v1.03 - Move ICMP definitions to sws_icmp.h
 * 19-Dec-02, LVR, v1.04 - Added routing functions
 * 22-Dec-02, LVR, v1.05 - Added multicast data to SWS_IpOptions
 * 21-Jun-05, LVR, v1.06 - Added SWS_IpSetRouteFilter
 *
 *
 * Comments:
 * --------
 * This file contains the public interface to the IP module
 *
 ****************************************************************************/

#ifndef SWS_IP_H
#define SWS_IP_H 0x0106 /* Version no. BCD */

/* Dependencies */
#include <stddef.h>   /* size_t */
#include "sws_host.h" /* Basic types */
#include "sws_buf.h"


/*
 * Macros & Constants
 */

/* Example Internet Datagram Header from RFC791, page 11, figure 4

    0                   1                   2                   3
    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |Version|  IHL  |Type of Service|          Total Length         |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |         Identification        |Flags|      Fragment Offset    |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |  Time to Live |    Protocol   |         Header Checksum       |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |                       Source Address                          |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |                    Destination Address                        |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |                    Options                    |    Padding    |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*/

/* IP header offsets */
#define SWS_IPHDR_VER_IHL      0  /* ui8, 4:4.  IHL is in 32 bit words */
#define SWS_IPHDR_TOS          1  /* ui8 */
#define SWS_IPHDR_TOTAL        2  /* ui16 */
#define SWS_IPHDR_ID           4  /* ui16 */
#define SWS_IPHDR_FLAGS_OFFSET 6  /* ui16 3:13.  Offset in units of 8 bytes */
#define SWS_IPHDR_TTL          8  /* ui8 */
#define SWS_IPHDR_PROTOCOL     9  /* ui8 */
#define SWS_IPHDR_CHECKSUM    10  /* ui16 */
#define SWS_IPHDR_SRC         12  /* ui32 */
#define SWS_IPHDR_DST         16  /* ui32 */
#define SWS_IPHDR_OPTIONS     20  /* ui32 padded, variable length */

#define SWS_IPHDR_MINLEN SWS_IPHDR_OPTIONS
#define SWS_IPHDR_MAXLEN 60       /* IHL max= 0xf 32-bit words */
#define SWS_IPHDR_MAXOPTS (SWS_IPHDR_MAXLEN - SWS_IPHDR_OPTIONS)

#define SWS_IPVERSION 4
#define SWS_IP_MAXLEN 0xffff

#define SWS_IP_MIN_MTU 576        /* Min MTU for any path - RFC791 minimum */
#define SWS_IP_MIN_FRAG 8         /* Data bytes in IP fragment RFC791 minimum */

#define SWS_IP_OFFSET_MASK 0x1fff

/* Flag bits */
#define SWS_IP_FLAG_MF   0x2000   /* 0= last fragment */
#define SWS_IP_FLAG_DF   0x4000   /* 0= may fragment */
#define SWS_IP_FLAG_RESV 0x8000   /* MBZ */

/* IP options. See http://www.iana.org/assignments/ip-parameters */
#define SWS_IP_OPT_COPIED       0x80 /* If set the option is copied into fragments */
#define SWS_IP_OPT_CLASS2       0x40 /* Class 2. If set the option is for debugging and measurement */
#define SWS_IP_OPT_END          0x00 /* RFC791, len 1 */
#define SWS_IP_OPT_NOP          0x01 /* RFC791, len 1 */
#define SWS_IP_OPT_SECURITY     0x82 /* Security */
#define SWS_IP_OPT_LOOSE_ROUTE  0x83 /* Loose route */
#define SWS_IP_OPT_TIMESTAMP    0x44 /* Timestamp */
#define SWS_IP_OPT_ESEC         0x85 /* Extended Security RFC1108 */
#define SWS_IP_OPT_RECORD_ROUTE 0x07 /* Record route */
#define SWS_IP_OPT_SID          0x88 /* Stream Identifier, len 4 */
#define SWS_IP_OPT_STRICT_ROUTE 0x89 /* Strict route */
#define SWS_IP_OPT_EXP_MEASURE  0x0a /* Experimental Measurement */
#define SWS_IP_OPT_MTU_PROBE    0x0b /* MTU Probe RFC1191 obsolete */
#define SWS_IP_OPT_MTU_REPLY    0x0c /* MTU Reply RFC1191 obsolete */
#define SWS_IP_OPT_EIP          0x91 /* Extended Internet Protocol RFC1385 */
#define SWS_IP_OPT_TRACEROUTE   0x52 /* Traceroute RFC1393 */
#define SWS_IP_OPT_ROUTER_ALERT 0x94 /* Router alert */

/* TOS precedence from RFC791 pll */
#define SWS_IP_PREC_MASK        0xe0
#define SWS_IP_PREC_NETC        0xe0 /* Network control */
#define SWS_IP_PREC_INWC        0xc0 /* Internetwork control */
#define SWS_IP_PREC_CRIT        0xa0 /* CRITIC/ECP */
#define SWS_IP_PREC_FLASHOR     0x80 /* Flash override */
#define SWS_IP_PREC_FLASH       0x60 /* Flash */
#define SWS_IP_PREC_IMM         0x40 /* Immediate */
#define SWS_IP_PREC_PRI         0x20 /* Priority */
#define SWS_IP_PREC_ROUTINE     0x00 /* Routine */

/* TOS values */
#define SWS_IP_TOS_MASK         0x1F
#define SWS_IP_TOS_DELAY        0x10 /* Low delay */
#define SWS_IP_TOS_THRU         0x08 /* High throughput */
#define SWS_IP_TOS_REL          0x04 /* High reliability */
#define SWS_IP_TOS_COST         0x02 /* Low cost RFC1349 */
#define SWS_IP_TOS_NORM         0x00 /* Normal service */

/* SWS_kGefIpError codes */
typedef enum SWS_EIpError
  {
  SWS_kErrorTxDropped = 1U,           /* Out of Tx buffers */
  SWS_kErrorTxFailed,                 /* Data link error + int error code */
  SWS_kErrorRxDropped,                /* Out of buffers */
  SWS_kErrorRxBadHdr,                 /* Bad IP header */
  SWS_kErrorRxBadHost,                /* IP destination != host address */
  SWS_kErrorRxProtocol,               /* No protocol handler */
  SWS_kErrorTxNoRoute,                /* Not possible to route packet */
  SWS_kErrorIpGleaned                 /* IP address gleaned */
  } SWS_EIpError;

/* Default values for SWS_IpSend */
#define SWS_IP_TTL_DEFAULT 0
#define SWS_IP_TOS_DEFAULT 0
#define SWS_IP_IDENT_DEFAULT 0
#define SWS_IP_MAY_FRAGMENT 0
#define SWS_IP_DONT_FRAGMENT 1


/*
 * Prototypes
 */
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Up calls from IP to protocol handler */

/* Indication of a received IP datagram */
typedef SWS_CB( void, SWS_FIpRecvInd(
  SWS_PTR( void),                     /* IN: Client context */
  SWS_BufferH,                        /* IN: IP datagram */
  size_t,                             /* IN: IP header length including options */
  ui32,                               /* IN: src IP address */
  ui32,                               /* IN: dst IP address */
  ui32,                               /* IN: host IP address */
  unsigned                            /* IN: TOS */
));
/* Indication of a received ICMP message */
typedef SWS_CB( void, SWS_FIpIcmpInd(
  SWS_PTR( void),                     /* IN: Client context */
  SWS_BufferH,                        /* IN: IP datagram - DO NOT MODIFY */
  size_t,                             /* IN: IP header length including options */
  ui32,                               /* IN: src IP address in original datagram */
  ui32,                               /* IN: dst IP address in original datagram */
  unsigned,                           /* IN: ICMP type */
  unsigned,                           /* IN: ICMP code */
  ui32                                /* IN: ICMP optional data (identifier & sequence) */
));
/* Indication of send resources */
typedef SWS_CB( void, SWS_FIpSendInd(
  SWS_PTR( void)                      /* IN: Client context */
));

#ifdef __cplusplus
}
#endif /* __cplusplus */


/*
 * Types
 */
/* Protocol descriptor */
typedef struct SWS_IpProtocolDesc
  {
  ui16 uSize;                         /* Set to sizeof(SWS_IpProtocolDesc) */
  ui16 uVersion;                      /* Set to SWS_PROTOCOL_VERSION */
#define SWS_PROTOCOL_VERSION 0x0100

  ui8 uProtocol;                      /* SWS_IPPROTO_xxx */
  ui8 dummy;

  /* Up call table */
  SWS_FNPTR( SWS_FIpRecvInd) pfnRecvInd;  /* Required */
  SWS_FNPTR( SWS_FIpIcmpInd) pfnIcmpInd;  /* Optional */
  SWS_FNPTR( SWS_FIpSendInd) pfnSendInd;  /* Optional */
  } SWS_IpProtocolDesc;

/* SWS_SOL_IP options */
typedef struct SWS_IpOptions
  {
  struct
    {
    unsigned df : 1;                  /* Dont fragment */
    unsigned hdrincl : 1;             /* Sent data includeds IP header */
    unsigned multicast_loop : 1;      /* Loopback multicasts */
    } flags;

  ui8 ttl;                            /* Time To Live */
  ui8 tos;                            /* Type Of Service */

  ui32 multicast_if;
  ui8 multicast_ttl;                  /* Multicast TTL */
  ui8 uResv1;

  ui8 uOptLen;                        /* Length of IP header option list */
  ui8 opts[ SWS_IPHDR_MAXOPTS];       /* IP header options, SWS_IPOPT_xxx */
  char resv[ 13];                     /* 64 bytes total */
  } SWS_IpOptions;


/*
 * Functions
 */
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Register a protocol */
SWS_EXPORT( int, SWS_IpRegisterProtocol( /* Returns !0 on error */
  SWS_PTR( const SWS_IpProtocolDesc), /* IN: Transport descriptor */
  SWS_PTR( void)                      /* IN: Client context */
));

/* Deregister a protocol */
SWS_EXPORT( int, SWS_IpDeregisterProtocol( /* Returns !0 on error */
  SWS_PTR( const SWS_IpProtocolDesc), /* IN: Transport descriptor */
  SWS_PTR( void)                      /* IN: Client context */
));


/* Get/Set SWS_SOL_IP options */
SWS_EXPORT( int, SWS_IpSetOption(
  SWS_PTR( SWS_IpOptions),            /* OUT: -> option data */
  int,                                /* IN: option SWS_IP_xxx */
  SWS_PTR( const void),               /* IN: -> value */
  int                                 /* IN: length of value */
));
SWS_EXPORT( int, SWS_IpGetOption(
  SWS_PTR( const SWS_IpOptions),      /* IN: -> option data */
  int,                                /* IN: option SWS_IP_xxx */
  SWS_PTR( void),                     /* OUT: -> value */
  SWS_PTR( int)                       /* I/O: length of value */
));

/* Send an IP datagram */
SWS_EXPORT( int, SWS_IpSend(          /* Returns !0 on error */
  ui32,                               /* IN: Source IP address or SWS_INADDR_ANY */
  ui32,                               /* IN: Destination IP address, NOT SWS_INADDR_ANY */
  unsigned,                           /* IN: Protocol SWS_IPPROTO_xxx */
  SWS_BufferH,                        /* IN: Datagram content !!!MODIFIED!!! */
  unsigned,                           /* IN: Ident, 0= don't care */
  SWS_PTR( SWS_IpOptions)             /* IN: IP option data, maybe NULL */
));

/* Get best source IP address to route to destination */
SWS_EXPORT( ui32, SWS_IpGetSrc(       /* Returns source IP address, 0 on error */
  ui32,                               /* IN: Remote IP address */
  unsigned                            /* IN: Type Of Service - SWS_IPTOS_xxx */
));

/* Get max datagram sizes */
SWS_EXPORT( size_t, SWS_IpGetMaxSizes(  /* Returns max Tx data size, 0= no route */
  ui32,                               /* IN: src IP address */
  ui32,                               /* IN: dst IP address */
  unsigned,                           /* IN: Type Of Service - SWS_IPTOS_xxx */
  SWS_PTR( size_t)                    /* OUT: Max Rx size (0= unlimited), maybe NULL */
));

/* Calc 1's complement 16-bit checksum over a buffer */
SWS_EXPORT( unsigned, SWS_IpChecksum(
  SWS_BufferH,                        /* IN: Buffer to checksum */
  SWS_size_t,                         /* IN: Starting offset */
  SWS_size_t,                         /* IN: Length */
  ui32                                /* IN: Initial sum */
));

/* Calc UDP/TCP pseudo header checksum */
SWS_EXPORT( ui32, SWS_IpPseudoHdr(
  ui32,                               /* IN: src IP address */
  ui32,                               /* IN: dst IP address */
  unsigned,                           /* IN: Protocol */
  size_t                              /* IN: Length of data */
));


/* Multicast */
SWS_EXPORT( int, SWS_IpGroupJoin(
  ui32,                               /* IN: group */
  ui32                                /* IN: src interface */
));
SWS_EXPORT( int, SWS_IpGroupLeave(
  ui32,                               /* IN: group */
  ui32                                /* IN: src interface */
));


/*****************************************************************************
 * Routing table ops
 ****************************************************************************/

/* Opaque route handle */
typedef struct SWS_SRoute SWS_SRoute;

/* Routing info */
typedef struct SWS_SIpRouteInfo
  {
  ui32 src;                           /* IP address of interface to send from */
  ui32 nextHop;                       /* IP address of next host in route */
  size_t mtu;                         /* Path MTU */
  } SWS_SIpRouteInfo;


/* Add a route */
SWS_EXPORT( SWS_PTR( SWS_SRoute), SWS_RouteAdd(
  ui32,                               /* IN: src */
  ui32,                               /* IN: dst */
  ui32,                               /* IN: subnet mask */
  ui32                                /* IN: next hop */
));
/* Remove a route */
SWS_EXPORT( int, SWS_RouteDelete(
  SWS_PTR( SWS_SRoute)                 /* IN: route */
));

/* Reset the routing table */
SWS_EXPORT( void, SWS_RouteReset( void));

/* Find the best route */
SWS_EXPORT( SWS_PTR( SWS_SRoute), SWS_RouteFind(
  ui32,                               /* IN: src ip or SWS_INADDR_ANY */
  ui32,                               /* IN: dst ip */
  unsigned                            /* IN: Type Of Service - SWS_IPTOS_xxx */
));

/* Start iteration thru the routing table */
SWS_EXPORT( SWS_PTR( SWS_SRoute), SWS_RouteBegin( void));

/* Get next entry in routing table */
SWS_EXPORT( SWS_PTR( SWS_SRoute), SWS_RouteNext(
  SWS_PTR( SWS_SRoute)               /* IN: current route */
));

/* Get routing info */
SWS_EXPORT( ui32, SWS_RouteSrc(
  SWS_PTR( const SWS_SRoute)         /* IN: route */
));
SWS_EXPORT( ui32, SWS_RouteDst(
  SWS_PTR( const SWS_SRoute)         /* IN: route */
));
SWS_EXPORT( ui32, SWS_RouteMask(
  SWS_PTR( const SWS_SRoute)         /* IN: route */
));
SWS_EXPORT( ui32, SWS_RouteNextHop(
  SWS_PTR( const SWS_SRoute)         /* IN: route */
));

/* Packet routing callback */
typedef SWS_CB( int, SWS_FIpRouter(   /* Return 0 if route provided */
  SWS_PTR( void),                     /* IN: Client context */
  ui32,                               /* IN: src IP address */
  ui32,                               /* IN: dst IP address */
  unsigned,                           /* IN: TOS */
  SWS_PTR( SWS_SIpRouteInfo)          /* OUT: route */
));

/* Install a routing filter */
SWS_EXPORT( int, SWS_IpSetRouteFilter( /* !0 = error */
  SWS_FNPTR( SWS_FIpRouter),          /* IN: -> Filter callback */
  SWS_PTR( void)                      /* IN: Callback context */
));

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* ndef SWS_IP_H */
/* End of file */
