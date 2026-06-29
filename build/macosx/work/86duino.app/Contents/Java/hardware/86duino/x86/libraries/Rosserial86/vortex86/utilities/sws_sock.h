/****************************************************************************
 *
 * File name   : sws_sock.h
 * Function    : BSD like sockets library.
 * Project     : SwsSock
 * Authors     : Lawrence Rust
 * Systems     : ANSI C
 * Version     : 1.0
 * Release Date:
 *
 ****************************************************************************
 * File History
 * ------------
 * 02-Apr-01, LVR, v1.00 - Created.
 * 11-Nov-02, LVR, v1.01 - Added ENOTCONN.
 * 12-Mar-03, LVR, v1.02 - Added IPv6 address types
 * 11-Jun-05, LVR, v1.03 - Added getproto/serv/byname/number
 * 29-Jun-05, LVR, v1.04 - Added SWS_DnsSetHostname & SWS_DnsGet/SetServer
 * 11-Aug-06, LVR, v1.05 - Change all sockaddr sizes from int to socklen_t
 * 20-Jun-09, LVR, v1.06 - Add SWS_DECL_TO & DECL_TIMEOUT
 * 20-Jul-09, LVR, v1.07 - Added SWS_strnicmp
 * 12-Mar-10, LVR, v1.08 - Moved SWS_stricmp && SWS_strnicmp to sws_str.h
 *
 *
 * Comments:
 * --------
 * This file contains the external interface to the sockets library
 *
 ****************************************************************************/

#ifndef SwsSock_H
#define SwsSock_H 0x0107 /* Version no. BCD */

/* Dependencies */
#include <stddef.h>
#include "sws_host.h"
#include "sws_gef.h"

/* Basic system type definitions, taken from the BSD file sys/types.h.*/
typedef unsigned char   SWS_u_char;
typedef unsigned short  SWS_u_short;
typedef unsigned int    SWS_u_int;
typedef unsigned long   SWS_u_long;


/* socket reference */
struct SWS_Socket;
typedef SWS_PTR( struct SWS_Socket) SWS_SOCKET;

/* This is used instead of -1, since the SOCKET type is a ptr.*/
#define SWS_INVALID_SOCKET  NULL
#define SWS_SOCKET_ERROR    (-1)


/* Structure used in select() call, taken from the BSD file sys/time.h.*/
struct SWS_timeval {
  long tv_sec;  /* seconds */
  long tv_usec; /* and microseconds */
};

/*
 * SWS_FD_SETSIZE may be defined by the user before including
 * this file, but the default here should be >= 64.
 */
#ifndef SWS_FD_SETSIZE
#ifndef FD_SETSIZE
#define SWS_FD_SETSIZE      64
#else
#define SWS_FD_SETSIZE      FD_SETSIZE
#endif
#endif

typedef struct SWS_fd_set {
  SWS_u_int fd_count;                       /* how many are SET? */
  SWS_u_int fd_limit;                       /* array size */
  SWS_SOCKET fd_array[ SWS_FD_SETSIZE]; /* an array of SWS_SOCKETs */
} SWS_fd_set;


/*
 * Commands for SWS_ioctl(),  taken from the BSD file fcntl.h.
 *
 * Ioctl's have the command encoded in the lower word,
 * and the size of any in or out parameters in the upper
 * word.  The high 2 bits of the upper word are used
 * to encode the in/out status of the parameter; for now
 * we restrict parameters to at most 128 bytes.
 */
#define SWS_IOCPARM_MASK    0x7f            /* parameters must be < 128 bytes */
#define SWS_IOC_VOID        0x20000000L     /* no parameters */
#define SWS_IOC_OUT         0x40000000L     /* copy out parameters */
#define SWS_IOC_IN          0x80000000L     /* copy in parameters */
#define SWS_IOC_INOUT       (SWS_IOC_IN|SWS_IOC_OUT)
#define SWS_IO(x,y)         (SWS_IOC_VOID|((x)<<8)|(y))

#define SWS_IOR(x,y,t)     (SWS_IOC_OUT|(((long)sizeof(t)&SWS_IOCPARM_MASK)<<16)|((x)<<8)|(y))
#define SWS_IOW(x,y,t)     (SWS_IOC_IN|(((long)sizeof(t)&SWS_IOCPARM_MASK)<<16)|((x)<<8)|(y))

#define SWS_FIONREAD    SWS_IOR('f', 127, SWS_u_long) /* get # bytes to read */
#define SWS_FIONBIO     SWS_IOW('f', 126, SWS_u_long) /* set/clear non-blocking i/o */
/*#define SWS_FIOASYNC    SWS_IOW('f', 125, SWS_u_long) / set/clear async i/o */

/* Socket I/O Controls */
#define SWS_SIOCSHIWAT  SWS_IOW('s',  0, SWS_u_long)  /* set high watermark */
#define SWS_SIOCGHIWAT  SWS_IOR('s',  1, SWS_u_long)  /* get high watermark */
#define SWS_SIOCSLOWAT  SWS_IOW('s',  2, SWS_u_long)  /* set low watermark */
#define SWS_SIOCGLOWAT  SWS_IOR('s',  3, SWS_u_long)  /* get low watermark */
#define SWS_SIOCATMARK  SWS_IOR('s',  7, SWS_u_long)  /* at oob mark? */


/*
 * Constants and structures defined by the internet system,
 * Per RFC 790, September 1981, taken from the BSD file netinet/in.h.
 */

/* Protocols. see http://www.iana.org/assignments/protocol-numbers */
#define SWS_IPPROTO_IP              0               /* dummy for IP (IP in IP is 4) */
#define SWS_IPPROTO_ICMP            1               /* control message protocol */
#define SWS_IPPROTO_IGMP            2               /* internet group management protocol */
#define SWS_IPPROTO_GGP             3               /* RFC823 */
#define SWS_IPPROTO_IPENC           4               /* IP Encapsulation within IP RFC2003 */
#define SWS_IPPROTO_ST              5               /* Stream RFC1190 */
#define SWS_IPPROTO_TCP             6               /* tcp */
#define SWS_IPPROTO_UDP             17              /* user datagram protocol */
#define SWS_IPPROTO_RDP             27              /* Reliable Data Protocol RFC908 */
#define SWS_IPPROTO_IPV6            41              /* IPv6 RFC2460 */
#define SWS_IPPROTO_IPV6_ROUTE      43              /* IPv6 */
#define SWS_IPPROTO_IPV6_FRAG       44              /* IPv6 */
#define SWS_IPPROTO_IPV6_ICMP       58              /* IPv6 ICMP RFC1883 */
#define SWS_IPPROTO_IPV6_NONXT      59              /* IPv6 RFC1883 */
#define SWS_IPPROTO_IPV6_OPTS       60              /* IPv6 RFC1883 */
#define SWS_IPPROTO_PRIVATE         61              /* Any host internal protocol */
#define SWS_IPPROTO_RAW             255             /* raw IP packet */
#define SWS_IPPROTO_MAX             256

/* Port/socket numbers: http://www.iana.org/assignments/port-numbers */
#define SWS_IPPORT_ECHO             7
#define SWS_IPPORT_DISCARD          9
#define SWS_IPPORT_SYSTAT           11
#define SWS_IPPORT_DAYTIME          13
#define SWS_IPPORT_NETSTAT          15
#define SWS_IPPORT_FTP              21
#define SWS_IPPORT_TELNET           23
#define SWS_IPPORT_SMTP             25
#define SWS_IPPORT_TIMESERVER       37
#define SWS_IPPORT_NAMESERVER       42
#define SWS_IPPORT_WHOIS            43
#define SWS_IPPORT_MTP              57

/* Port/socket numbers: host specific functions */
#define SWS_IPPORT_TFTP             69
#define SWS_IPPORT_RJE              77
#define SWS_IPPORT_FINGER           79
#define SWS_IPPORT_TTYLINK          87
#define SWS_IPPORT_SUPDUP           95

/* UNIX TCP sockets */
#define SWS_IPPORT_EXECSERVER       512
#define SWS_IPPORT_LOGINSERVER      513
#define SWS_IPPORT_CMDSERVER        514
#define SWS_IPPORT_EFSSERVER        520

/* UNIX UDP sockets */
#define SWS_IPPORT_BIFFUDP          512
#define SWS_IPPORT_WHOSERVER        513
#define SWS_IPPORT_ROUTESERVER      520
                                        /* 520+1 also used */
/* Ports < IPPORT_RESERVED are reserved for privileged processes (e.g. root). */
#define SWS_IPPORT_RESERVED         1024


/* IPv4 address */
typedef SWS_u_long SWS_in_addr_t;
struct SWS_in_addr
  {
  union
    {
    struct { SWS_u_char s_b1,s_b2,s_b3,s_b4; } S_un_b;
    struct { SWS_u_short s_w1,s_w2; } S_un_w;
    SWS_in_addr_t S_addr;
    } S_un;
#define SWS_s_addr  S_un.S_addr      /* can be used for most tcp & ip code */
#define SWS_s_host  S_un.S_un_b.s_b2 /* host on imp */
#define SWS_s_net   S_un.S_un_b.s_b1 /* network */
#define SWS_s_imp   S_un.S_un_w.s_w2 /* imp */
#define SWS_s_impno S_un.S_un_b.s_b4 /* imp # */
#define SWS_s_lh    S_un.S_un_b.s_b3 /* logical host */
  };

struct SWS_ip_mreq
  {
  struct SWS_in_addr imr_multiaddr; /* multicast group to join */
  struct SWS_in_addr imr_interface; /* interface to join on    */
  };

#define SWS_INADDR_ANY              ((SWS_in_addr_t)0x00000000L)
#define SWS_INADDR_LOOPBACK         ((SWS_in_addr_t)0x7f000001L)
#define SWS_INADDR_BROADCAST        ((SWS_in_addr_t)0xffffffffL)
#define SWS_INADDR_NONE             ((SWS_in_addr_t)0xffffffffL)

#define SWS_IN_CLASSA(i)            (((SWS_u_long)(i) & 0x80000000L) == 0)
#define SWS_IN_CLASSA_NET           ((SWS_in_addr_t)0xff000000L)
#define SWS_IN_CLASSA_NSHIFT        24
#define SWS_IN_CLASSA_HOST          ((SWS_in_addr_t)0x00ffffffL)
#define SWS_IN_CLASSA_MAX           128

#define SWS_IN_CLASSB(i)            (((SWS_u_long)(i) & 0xc0000000L) == 0x80000000L)
#define SWS_IN_CLASSB_NET           ((SWS_in_addr_t)0xffff0000L)
#define SWS_IN_CLASSB_NSHIFT        16
#define SWS_IN_CLASSB_HOST          ((SWS_in_addr_t)0x0000ffffL)
#define SWS_IN_CLASSB_MAX           65536

#define SWS_IN_CLASSC(i)            (((SWS_u_long)(i) & 0xe0000000L) == 0xc0000000L)
#define SWS_IN_CLASSC_NET           ((SWS_in_addr_t)0xffffff00L)
#define SWS_IN_CLASSC_NSHIFT        8
#define SWS_IN_CLASSC_HOST          ((SWS_in_addr_t)0x000000ffL)

#define SWS_IN_CLASSD(i)            (((SWS_u_long)(i) & 0xf0000000L) == 0xe0000000L)
#define SWS_IN_CLASSD_NET           ((SWS_in_addr_t)0xf0000000L)
#define SWS_IN_CLASSD_NSHIFT        28
#define SWS_IN_CLASSD_HOST          ((SWS_in_addr_t)0x0fffffffL)
#define SWS_IN_MULTICAST(i)         SWS_IN_CLASSD(i)


/* Socket types */
#define SWS_SOCK_STREAM 1       /* stream (connection) socket   */
#define SWS_SOCK_DGRAM  2       /* datagram (conn.less) socket  */
#define SWS_SOCK_RAW    3       /* raw socket           */

/* Address families */
#define SWS_AF_UNSPEC       0               /* unspecified */
#define SWS_AF_UNIX         1               /* local to host (pipes, portals) */
#define SWS_AF_LOCAL        1               /* POSIX name for AF_UNIX */
#define SWS_AF_INET         2               /* internetwork: UDP, TCP, etc. */
#define SWS_AF_IMPLINK      3               /* arpanet imp addresses */
#define SWS_AF_PUP          4               /* pup protocols: e.g. BSP */
#define SWS_AF_CHAOS        5               /* mit CHAOS protocols */
#define SWS_AF_NS           6               /* XEROX NS protocols */
#define SWS_AF_ISO          7               /* ISO protocols */
#define SWS_AF_OSI          SWS_AF_ISO      /* OSI is ISO */
#define SWS_AF_ECMA         8               /* european computer manufacturers */
#define SWS_AF_DATAKIT      9               /* datakit protocols */
#define SWS_AF_CCITT        10              /* CCITT protocols, X.25 etc */
#define SWS_AF_SNA          11              /* IBM SNA */
#define SWS_AF_DECnet       12              /* DECnet */
#define SWS_AF_DLI          13              /* Direct data link interface */
#define SWS_AF_LAT          14              /* LAT */
#define SWS_AF_HYLINK       15              /* NSC Hyperchannel */
#define SWS_AF_APPLETALK    16              /* AppleTalk */
#define SWS_AF_NETBIOS      17              /* NetBios-style addresses */
#define SWS_AF_INET6        23              /* IP version 6 */

#define SWS_AF_MAX          32

/* Protocol families, same as address families for now */
#define SWS_PF_UNSPEC       SWS_AF_UNSPEC
#define SWS_PF_UNIX         SWS_AF_UNIX
#define SWS_PF_LOCAL        SWS_AF_LOCAL
#define SWS_PF_INET         SWS_AF_INET
#define SWS_PF_IMPLINK      SWS_AF_IMPLINK
#define SWS_PF_PUP          SWS_AF_PUP
#define SWS_PF_CHAOS        SWS_AF_CHAOS
#define SWS_PF_NS           SWS_AF_NS
#define SWS_PF_ISO          SWS_AF_ISO
#define SWS_PF_OSI          SWS_AF_OSI
#define SWS_PF_ECMA         SWS_AF_ECMA
#define SWS_PF_DATAKIT      SWS_AF_DATAKIT
#define SWS_PF_CCITT        SWS_AF_CCITT
#define SWS_PF_SNA          SWS_AF_SNA
#define SWS_PF_DECnet       SWS_AF_DECnet
#define SWS_PF_DLI          SWS_AF_DLI
#define SWS_PF_LAT          SWS_AF_LAT
#define SWS_PF_HYLINK       SWS_AF_HYLINK
#define SWS_PF_APPLETALK    SWS_AF_APPLETALK
#define SWS_PF_NETBIOS      SWS_AF_NETBIOS
#define SWS_PF_INET6        SWS_AF_INET6

#define SWS_PF_MAX          SWS_AF_MAX


/* Maximum queue length specificable by listen.  */
#define SWS_SOMAXCONN       0x7fff

/* Flags we can use with send/ and recv. */
#define SWS_MSG_OOB         0x1             /* process out-of-band data */
#define SWS_MSG_PEEK        0x2             /* peek at incoming message */
#define SWS_MSG_DONTROUTE   0x4             /* send without using routing tables */


/* Level number for (get/set)sockopt() to apply to socket itself. */
#define SWS_SOL_SOCKET      0xffff          /* options for socket level */

/* Option flags per-socket */
#define SWS_SO_DEBUG        0x0001          /* turn on debugging info recording */
#define SWS_SO_ACCEPTCONN   0x0002          /* socket has had listen() */
#define SWS_SO_REUSEADDR    0x0004          /* allow local address reuse */
#define SWS_SO_KEEPALIVE    0x0008          /* keep connections alive */
#define SWS_SO_DONTROUTE    0x0010          /* just use interface addresses */
#define SWS_SO_BROADCAST    0x0020          /* permit sending of broadcast msgs */
#define SWS_SO_USELOOPBACK  0x0040          /* bypass hardware when possible */
#define SWS_SO_LINGER       0x0080          /* linger on close if data present */
#define SWS_SO_OOBINLINE    0x0100          /* leave received OOB data in line */

#define SWS_SO_DONTLINGER   (SWS_u_short)(~SWS_SO_LINGER)
#define SWS_SO_EXCLUSIVEADDRUSE ((int)(~SWS_SO_REUSEADDR)) /* disallow local address reuse */

/* Additional options */
#define SWS_SO_SNDBUF       0x1001          /* send buffer size */
#define SWS_SO_RCVBUF       0x1002          /* receive buffer size */
#define SWS_SO_SNDLOWAT     0x1003          /* send low-water mark */
#define SWS_SO_RCVLOWAT     0x1004          /* receive low-water mark */
#define SWS_SO_SNDTIMEO     0x1005          /* send timeout */
#define SWS_SO_RCVTIMEO     0x1006          /* receive timeout */
#define SWS_SO_ERROR        0x1007          /* get error status and clear */
#define SWS_SO_TYPE         0x1008          /* get socket type */

#define SWS_SO_MAX_MSG_SIZE 0x2003          /* maximum message size */

/* Setsockoptions(2) level. Thanks to BSD these must match IPPROTO_xxx */
#define SWS_SOL_IP      0
#define SWS_SOL_TCP     6
#define SWS_SOL_UDP     17
#define SWS_SOL_IOCTL   0xfffe

/* SWS_SOL_IP options */
#define SWS_IP_OPTIONS          1
#define SWS_IP_HDRINCL          2 /* header is included with data */
#define SWS_IP_TOS              3
#define SWS_IP_TTL              4
#define SWS_IP_MULTICAST_IF     9
#define SWS_IP_MULTICAST_TTL    10
#define SWS_IP_MULTICAST_LOOP   11
#define SWS_IP_ADD_MEMBERSHIP   12
#define SWS_IP_DROP_MEMBERSHIP  13
#define SWS_IP_DONTFRAGMENT     14

/* SWS_IP_TOS Type Of Service (1 only) */
#define SWS_IPTOS_LOWDELAY      0x10
#define SWS_IPTOS_THROUGHPUT    0x08
#define SWS_IPTOS_RELIABILITY   0x04
#define SWS_IPTOS_COST          0x02
#define SWS_IPTOS_ROUTINE       0x00

/* SWS_IP_TOS precedence (1 only) */
#define SWS_IPTOS_INWC          0xc0 /* Internetwork control */
#define SWS_IPTOS_CRIT          0xa0 /* CRITIC/ECP */
#define SWS_IPTOS_FLASHOR       0x80 /* Flash override */
#define SWS_IPTOS_FLASH         0x60 /* Flash */
#define SWS_IPTOS_IMM           0x40 /* Immediate */
#define SWS_IPTOS_PRI           0x20 /* Priority */

/* These need to appear somewhere around here */
#define SWS_IP_DEFAULT_MULTICAST_TTL        1
#define SWS_IP_DEFAULT_MULTICAST_LOOP       1
#define SWS_IP_MAX_MEMBERSHIPS              20

/* SWS_IP_OPTIONS types */
#define SWS_IPOPT_COPIED       0x80
#define SWS_IPOPT_END             0  /* End of list, Len 1 */
#define SWS_IPOPT_NOP             1  /* Padding, Len 1 */
#define SWS_IPOPT_SECURITY      130  /* Len 11 */
#define SWS_IPOPT_LOOSE_ROUTE   131  /* Len byte follows */
#define SWS_IPOPT_STRICT_ROUTE  137  /* Len byte follows */
#define SWS_IPOPT_RECORD_ROUTE    7  /* Len byte follows */
#define SWS_IPOPT_STREAMID      136  /* Len 4 */
#define SWS_IPOPT_TIME_STAMP     68  /* Len byte follows */

/* SWS_SOL_TCP options */
#define SWS_TCP_NODELAY   0x0001      /* Nagle RFC896 */
#define SWS_TCP_MAXSEG    2

/* SWS_SOL_IOCTL options */
#define SWS_IOCTL_FIONREAD 1

/* Option to use with [gs]etsockopt at the SWS_SOL_UDP level */
#define SWS_UDP_NOCHECKSUM  1


/* Shutdown types */
#define SWS_SD_RECEIVE    0x01
#define SWS_SD_SEND       0x02
#define SWS_SD_BOTH       0x03


#define SWS_BASEERR       10000

/* Definitions of regular Unix error constants */
#define SWS_EINTR                (SWS_BASEERR+4) /* Blocking call cancelled */
#define SWS_EBADF                (SWS_BASEERR+9)
#define SWS_ENOMEM               (SWS_BASEERR+12)
#define SWS_EACCES               (SWS_BASEERR+13) /* Permission denied */
#define SWS_EFAULT               (SWS_BASEERR+14) /* Bad address */
#define SWS_ENODEV               (SWS_BASEERR+19)
#define SWS_EINVAL               (SWS_BASEERR+22) /* Invalid arg */
#define SWS_EMFILE               (SWS_BASEERR+24) /* Too many open sockets */

/* Definitions of regular Berkeley error constants */
#define SWS_EWOULDBLOCK          (SWS_BASEERR+35)
#define SWS_EINPROGRESS          (SWS_BASEERR+36)
#define SWS_EALREADY             (SWS_BASEERR+37)
#define SWS_ENOTSOCK             (SWS_BASEERR+38)
#define SWS_EDESTADDRREQ         (SWS_BASEERR+39)
#define SWS_EMSGSIZE             (SWS_BASEERR+40)
#define SWS_EPROTOTYPE           (SWS_BASEERR+41)
#define SWS_ENOPROTOOPT          (SWS_BASEERR+42)
#define SWS_EPROTONOSUPPORT      (SWS_BASEERR+43)
#define SWS_ESOCKTNOSUPPORT      (SWS_BASEERR+44)
#define SWS_EOPNOTSUPP           (SWS_BASEERR+45)
#define SWS_EPFNOSUPPORT         (SWS_BASEERR+46)
#define SWS_EAFNOSUPPORT         (SWS_BASEERR+47)
#define SWS_EADDRINUSE           (SWS_BASEERR+48)
#define SWS_EADDRNOTAVAIL        (SWS_BASEERR+49)
#define SWS_ENETDOWN             (SWS_BASEERR+50)
#define SWS_ENETUNREACH          (SWS_BASEERR+51)
#define SWS_ENETRESET            (SWS_BASEERR+52)
#define SWS_ECONNABORTED         (SWS_BASEERR+53)
#define SWS_ECONNRESET           (SWS_BASEERR+54)
#define SWS_ENOBUFS              (SWS_BASEERR+55)
#define SWS_EISCONN              (SWS_BASEERR+56)
#define SWS_ENOTCONN             (SWS_BASEERR+57)
#define SWS_ESHUTDOWN            (SWS_BASEERR+58)
#define SWS_ETOOMANYREFS         (SWS_BASEERR+59)
#define SWS_ETIMEDOUT            (SWS_BASEERR+60)
#define SWS_ECONNREFUSED         (SWS_BASEERR+61)
#define SWS_ELOOP                (SWS_BASEERR+62)
#define SWS_ENAMETOOLONG         (SWS_BASEERR+63)
#define SWS_EHOSTDOWN            (SWS_BASEERR+64)
#define SWS_EHOSTUNREACH         (SWS_BASEERR+65)
#define SWS_ENOTEMPTY            (SWS_BASEERR+66)
#define SWS_EPROCLIM             (SWS_BASEERR+67)
#define SWS_EUSERS               (SWS_BASEERR+68)
#define SWS_EDQUOT               (SWS_BASEERR+69)
#define SWS_ESTALE               (SWS_BASEERR+70)
#define SWS_EREMOTE              (SWS_BASEERR+71)
#define SWS_EHOST_NOT_FOUND      (SWS_BASEERR+72)
#define SWS_ETRY_AGAIN           (SWS_BASEERR+73)
#define SWS_ENO_RECOVERY         (SWS_BASEERR+74)
#define SWS_ENO_DATA             (SWS_BASEERR+75)


/* Generic socket address */
struct SWS_sockaddr
  {
  short        sa_family;     /* address family, AF_xxx */
  SWS_u_char   sa_data[14];   /* 14 bytes of protocol address */
  };

typedef int SWS_socklen_t;

/* Socket address, IPv4 style. */
struct SWS_sockaddr_in
  {
  short               sin_family;
  SWS_u_short         sin_port;
  struct SWS_in_addr  sin_addr;
  SWS_u_char          sin_zero[8]; /* 16 bytes in total */
  };

/* Structure used for manipulating linger option. */
struct SWS_linger
  {
  SWS_u_short l_onoff;                /* option on/off */
  SWS_u_short l_linger;               /* linger time */
  };


/* Name lookup */
struct SWS_hostent
  {
  SWS_PTR( const char)           h_name;      /* official name of host */
  SWS_PTR( SWS_PTR( const char)) h_aliases;   /* alias list */
  short                          h_addrtype;  /* host address type */
  short                          h_length;    /* length of address */
  SWS_PTR( SWS_PTR( char))       h_addr_list; /* list of addresses from name server */
#define SWS_h_addr h_addr_list[0]       /* address, for backward compatiblity */
  };

/* Protocol lookup */
struct SWS_protoent
  {
  SWS_PTR( const char) p_name;                /* official protocol name */
  SWS_PTR( SWS_PTR( const char)) p_aliases;   /* alias list */
  short   p_proto;                            /* protocol # */
  };

/* Server lookup */
struct SWS_servent
  {
  SWS_PTR( const char) s_name;                /* official service name */
  SWS_PTR( SWS_PTR( const char)) s_aliases;   /* alias list */
  short   s_port;                             /* port #, network order */
  SWS_PTR( const char) s_proto;               /* protocol to use */
  };


/*
 * Functions
 */
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

SWS_EXPORT( SWS_SOCKET, SWS_socket( int _family, int _type, int _protocol));

SWS_EXPORT( int, SWS_bind( SWS_SOCKET, SWS_PTR( const struct SWS_sockaddr), SWS_socklen_t _namelen));
SWS_EXPORT( int, SWS_listen( SWS_SOCKET, int _backlog));
SWS_EXPORT( int, SWS_connect( SWS_SOCKET, SWS_PTR( const struct SWS_sockaddr), SWS_socklen_t _namelen));
SWS_EXPORT( SWS_SOCKET, SWS_accept( SWS_SOCKET, SWS_PTR( struct SWS_sockaddr), SWS_PTR( SWS_socklen_t) _addrlen));

SWS_EXPORT( int, SWS_recv( SWS_SOCKET, SWS_PTR( void) _buf, unsigned _len, unsigned _flags));
SWS_EXPORT( int, SWS_recvfrom( SWS_SOCKET, SWS_PTR( void) _buf, unsigned _len, unsigned _flags, SWS_PTR( struct SWS_sockaddr) _from, SWS_PTR( SWS_socklen_t) _fromlen));

SWS_EXPORT( int, SWS_send( SWS_SOCKET, SWS_PTR( const void) _buf, unsigned _len, unsigned _flags));
SWS_EXPORT( int, SWS_sendto( SWS_SOCKET, SWS_PTR( const void) _buf, unsigned _len, unsigned _flags, SWS_PTR( const struct SWS_sockaddr) _to, SWS_socklen_t _tolen));

SWS_EXPORT( int, SWS_getsockname( SWS_SOCKET, SWS_PTR( struct SWS_sockaddr), SWS_PTR( SWS_socklen_t) _namelen ));
SWS_EXPORT( int, SWS_getpeername( SWS_SOCKET, SWS_PTR( struct SWS_sockaddr), SWS_PTR( SWS_socklen_t) _namelen ));

SWS_EXPORT( int, SWS_getsockopt( SWS_SOCKET, unsigned _level, unsigned _opt, SWS_PTR( void) _val, SWS_PTR( int) _len));
SWS_EXPORT( int, SWS_setsockopt( SWS_SOCKET, unsigned _level, unsigned _opt, SWS_PTR( const void) _val, int _len));

SWS_EXPORT( int, SWS_shutdown( SWS_SOCKET, int _how));

SWS_EXPORT( int, SWS_close( SWS_SOCKET));
SWS_EXPORT( int, SWS_ioctl( SWS_SOCKET, SWS_u_long _cmd, SWS_PTR( SWS_u_long) _args ));

SWS_EXPORT( int, SWS_select( SWS_PTR( SWS_fd_set), SWS_PTR( SWS_fd_set), SWS_PTR( SWS_fd_set), SWS_PTR( const struct SWS_timeval) ));
SWS_EXPORT( int, SWS_FdIsSet( SWS_SOCKET, SWS_PTR( SWS_fd_set) ));
SWS_EXPORT( void, SWS_FdClr( SWS_SOCKET, SWS_PTR( SWS_fd_set) ));
SWS_EXPORT( void, SWS_FdSet( SWS_SOCKET, SWS_PTR( SWS_fd_set) ));
#define SWS_FdZero(set) ((set)->fd_count = 0, (set)->fd_limit = SWS_FD_SETSIZE)

SWS_EXPORT( int, SWS_GetLastError( void));
SWS_EXPORT( int, SWS_SetLastError( int));

/* From netdb.h */
SWS_EXPORT( SWS_PTR( struct SWS_hostent), SWS_gethostbyaddr( SWS_PTR( const char), int _len, int _family));
SWS_EXPORT( SWS_PTR( struct SWS_hostent), SWS_gethostbyname( SWS_PTR( const char) ));

SWS_EXPORT( SWS_PTR( struct SWS_protoent), SWS_getprotobyname( SWS_PTR( const char) ));
SWS_EXPORT( SWS_PTR( struct SWS_protoent), SWS_getprotobynumber( int ));

SWS_EXPORT( SWS_PTR( struct SWS_servent), SWS_getservbyname( SWS_PTR( const char), SWS_PTR( const char) ));
SWS_EXPORT( SWS_PTR( struct SWS_servent), SWS_getservbyport( int /* Network order */, SWS_PTR( const char) ));

/* From unistd.h */
SWS_EXPORT( int, SWS_gethostname( SWS_PTR( char), int _namelen));

/* From inet.h */
SWS_EXPORT( SWS_in_addr_t, SWS_inet_addr( SWS_PTR( const char) ));
SWS_EXPORT( SWS_PTR( const char), SWS_inet_ntoa( struct SWS_in_addr));

SWS_EXPORT( SWS_u_long, SWS_htonl( SWS_u_long));
SWS_EXPORT( SWS_u_short, SWS_htons( SWS_u_short));
SWS_EXPORT( SWS_u_long, SWS_ntohl( SWS_u_long));
SWS_EXPORT( SWS_u_short, SWS_ntohs( SWS_u_short));


/*
 * IPv6 RFC2553
 */
#ifndef SWS_IPV6
#define SWS_IPV6 1
#endif
#if SWS_IPV6
/* IPv6 address */
struct SWS_in6_addr
  {
  union
    {
    SWS_u_char  _S6_u8[16];
    SWS_u_short _S6_u16[8];
    SWS_u_long  _S6_u32[4];
    /*uint64_t   _S6_u64[2];*/
    } _S6_un;
  };
#define SWS_S6_un   _S6_un
#define SWS_s6_addr _S6_un._S6_u8
#define SWS_s6_words _S6_un._S6_u16

/* Socket address, IPv6 style. */
struct SWS_sockaddr_in6
  {
  short               sin6_family;    /* AF_INET6 */
  SWS_u_short         sin6_port;      /* transport layer port # */
  SWS_u_long          sin6_flowinfo;  /* IPv6 traffic class & flow info */
  struct SWS_in6_addr sin6_addr;      /* IPv6 address */
  SWS_u_long          sin6_scope_id;  /* set of interfaces for a scope */
  };

#define SWS_SS_MAXSIZE 128      /* Implementation specific max size */
struct SWS_sockaddr_storage
  {
  short       __ss_family;      /* address family */
  char        __ss_pad1[2];
  SWS_u_long  __ss_align;       /* field to force desired structure */
  char        __ss_pad2[ SWS_SS_MAXSIZE - 8]; /* byte pad to achieve desired size, */
};

#define SWS_IN6ADDR_ANY_INIT        {{{0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0}}}
#define SWS_IN6ADDR_LOOPBACK_INIT   {{{0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,1}}}

#define SWS_INET_ADDRSTRLEN  16 /* Max size of string representation of IPv4 address */
#define SWS_INET6_ADDRSTRLEN 46 /* Max size of string representation of IPv6 address */

/* Option to use with [gs]etsockopt at the SWS_IPPROTO_IPV6 level */
#define SWS_IPV6_UNICAST_HOPS       4  /* Set/get IP unicast hop limit */
#define SWS_IPV6_MULTICAST_IF       9  /* Set/get IP multicast interface */
#define SWS_IPV6_MULTICAST_HOPS     10 /* Set/get IP multicast ttl */
#define SWS_IPV6_MULTICAST_LOOP     11 /* Set/get IP multicast loopback */
#define SWS_IPV6_ADD_MEMBERSHIP     12 /* Add an IP group membership */
#define SWS_IPV6_DROP_MEMBERSHIP    13 /* Drop an IP group membership */
#define SWS_IPV6_JOIN_GROUP         SWS_IPV6_ADD_MEMBERSHIP
#define SWS_IPV6_LEAVE_GROUP        SWS_IPV6_DROP_MEMBERSHIP

/* Flags for getnameinfo() */
#define SWS_NI_MAXHOST     1025 /* Max size of a FQDN */
#define SWS_NI_MAXSERV       32 /* Max size of a service name */
#define SWS_NI_NOFQDN      0x01 /* Only return nodename portion for local hosts */
#define SWS_NI_NUMERICHOST 0x02 /* Return numeric form of the host's address */
#define SWS_NI_NAMEREQD    0x04 /* Error if the host's name not in DNS */
#define SWS_NI_NUMERICSERV 0x08 /* Return numeric form of the service (port #) */
#define SWS_NI_DGRAM       0x10 /* Service is a datagram service */

/* Flags used in "hints" argument to getaddrinfo() */
#define SWS_AI_PASSIVE     0x1  /* Socket address will be used in bind() call */
#define SWS_AI_CANONNAME   0x2  /* Return canonical name in first ai_canonname */
#define SWS_AI_NUMERICHOST 0x4  /* Nodename must be a numeric address string */

/* Error codes from getaddrinfo() */
#define SWS_EAI_ADDRFAMILY  SWS_EAFNOSUPPORT
#define SWS_EAI_AGAIN       SWS_ETRY_AGAIN
#define SWS_EAI_BADFLAGS    SWS_EINVAL
#define SWS_EAI_FAIL        SWS_ENO_RECOVERY
#define SWS_EAI_FAMILY      SWS_EAFNOSUPPORT
#define SWS_EAI_MEMORY      SWS_ENOMEM
#define SWS_EAI_NODATA      SWS_ENO_DATA
#define SWS_EAI_NONAME      SWS_EHOST_NOT_FOUND
#define SWS_EAI_SERVICE     SWS_EPROTOTYPE
#define SWS_EAI_SOCKTYPE    SWS_ESOCKTNOSUPPORT

struct SWS_addrinfo
  {
  int ai_flags;                 /* AI_PASSIVE, AI_CANONNAME, AI_NUMERICHOST */
  int ai_family;                /* PF_xxx */
  int ai_socktype;              /* SOCK_xxx */
  int ai_protocol;              /* 0 or IPPROTO_xxx for IPv4 and IPv6 */
  size_t ai_addrlen;            /* Length of ai_addr */
  SWS_PTR( char) ai_canonname;  /* Canonical name for nodename */
  SWS_PTR( struct SWS_sockaddr) ai_addr;  /* Binary address */
  SWS_PTR( struct SWS_addrinfo) ai_next;  /* Next structure in linked list */
  };

#define SWS_IN6_IS_ADDR_LINKLOCAL(/*const struct in6_addr**/a) \
  (0xfe == (a)->SWS_s6_addr[0] && 0x80 == ((a)->SWS_s6_addr[1] & 0xc0))

#define SWS_IN6_IS_ADDR_LOOPBACK(a) \
  (0 == memcmp( a, &SWS_in6addr_loopback, sizeof(SWS_in6addr_loopback) ))

#define SWS_IN6_IS_ADDR_MC_GLOBAL(a) \
  (SWS_IN6_IS_ADDR_MULTICAST(a) && (0xe == ((a)->SWS_s6_addr[1] & 0xf)) )

#define SWS_IN6_IS_ADDR_MC_LINKLOCAL(a) \
  (SWS_IN6_IS_ADDR_MULTICAST(a) && (2 == ((a)->SWS_s6_addr[1] & 0xf)) )

#define SWS_IN6_IS_ADDR_MC_NODELOCAL(a) \
  (SWS_IN6_IS_ADDR_MULTICAST(a) && (1 == ((a)->SWS_s6_addr[1] & 0xf)) )

#define SWS_IN6_IS_ADDR_MC_ORGLOCAL(a) \
  (SWS_IN6_IS_ADDR_MULTICAST(a) && (8 == ((a)->SWS_s6_addr[1] & 0xf)) )

#define SWS_IN6_IS_ADDR_MC_SITELOCAL(a) \
  (SWS_IN6_IS_ADDR_MULTICAST(a) && (5 == ((a)->SWS_s6_addr[1] & 0xf)) )

#define SWS_IN6_IS_ADDR_MULTICAST(a) \
  (0xff == (a)->SWS_s6_addr[0])

#define SWS_IN6_IS_ADDR_SITELOCAL(a) \
  (0xfe == (a)->SWS_s6_addr[0] && 0xc0 == ((a)->SWS_s6_addr[1] & 0xc0) )

#define SWS_IN6_IS_ADDR_UNSPECIFIED(a) \
  (0 == memcmp( a, &SWS_in6addr_any, sizeof(SWS_in6addr_any) ))

#define SWS_IN6_IS_ADDR_V4COMPAT(a) \
  (0 == (a)->SWS_s6_words[0] \
  && 0 == (a)->SWS_s6_words[1] \
  && 0 == (a)->SWS_s6_words[2] \
  && 0 == (a)->SWS_s6_words[3] \
  && 0 == (a)->SWS_s6_words[4] \
  && 0 == (a)->SWS_s6_words[5] \
  && !(0 == (a)->SWS_s6_words[6] \
    && 0 == (a)->SWS_s6_words[14] \
    && (0 == (a)->SWS_s6_words[15] || 1 == (a)->SWS_s6_words[15])) )

#define SWS_IN6_IS_ADDR_V4MAPPED(a) \
  (0 == (a)->SWS_s6_words[0] \
  && 0 == (a)->SWS_s6_words[1] \
  && 0 == (a)->SWS_s6_words[2] \
  && 0 == (a)->SWS_s6_words[3] \
  && 0 == (a)->SWS_s6_words[4] \
  && 0xffff == (a)->SWS_s6_words[5] )

SWS_EXPORT( int, SWS_getaddrinfo( SWS_PTR( const char) _node, SWS_PTR( const char) _serv, SWS_PTR( const struct SWS_addrinfo), SWS_PTR( SWS_PTR( struct SWS_addrinfo)) ));
SWS_EXPORT( void, SWS_freeaddrinfo( SWS_PTR( struct SWS_addrinfo) ));
SWS_EXPORT( int, SWS_getnameinfo( SWS_PTR( const struct SWS_sockaddr), SWS_socklen_t, SWS_PTR( char) host, size_t, SWS_PTR( char) serv, size_t, int flags));
SWS_EXPORT( int, SWS_inet_pton( int af, SWS_PTR( const char) src, SWS_PTR( void) dst));
SWS_EXPORT( SWS_PTR( const char), SWS_inet_ntop( int af, SWS_PTR( const void) src, SWS_PTR( char) dst, size_t));

/* IPv6 initialisers */
SWS_EXPORTDATA( const struct SWS_in6_addr, SWS_in6addr_any);
SWS_EXPORTDATA( const struct SWS_in6_addr, SWS_in6addr_loopback);
#endif


/*
 * DNS
 */

/* Set the hostname */
SWS_EXPORT( int, SWS_DnsSetHostname(  /* Returns !0 on error */
  SWS_PTR( const char),               /* IN: -> hostname */
  SWS_PTR( const char)                /* IN: -> domain or NULL */
));

/* Get/set DNS server address */
SWS_EXPORT( ui32, SWS_DnsGetPriServer( void));
SWS_EXPORT( ui32, SWS_DnsGetSecServer( void));
SWS_EXPORT( int, SWS_DnsSetPriServer( /* Returns !0 on error */
  ui32                                /* IN: IP address */
));
SWS_EXPORT( int, SWS_DnsSetSecServer( /* Returns !0 on error */
  ui32                                /* IN: IP address */
));


/*
 * Utilities
 */

/* Idle time processing */
SWS_EXPORT( void, SWS_SockSleep( SWS_u_long));

/* Monotonic timer/counter */
SWS_EXPORT( SWS_u_long, SWS_SockTicks( void));
#define SWS_TIMER_TICKS 1000 /* Timer ticks per second */

/* Time comparisons that account for wraparound */
#define SWS_TimerDelta( t1,t2) ((long)(t1) - (long)(t2))
#define SWS_TimerLT( t1,t2) ( SWS_TimerDelta( t1,t2) < 0)
#define SWS_TimerLE( t1,t2) ( SWS_TimerDelta( t1,t2) <= 0)

/* Declare & init a timeout in millisecs for SO_SND/RCVTIMEO */
#define SWS_DECL_TO( name, m) \
  struct SWS_timeval name;\
  name.tv_sec = (m) / 1000;\
  name.tv_usec = ((m) % 1000) * 1000


/*
 * Library initialisation
 * ***Must*** be called before any other function
 */
SWS_EXPORT( int, SWS_SockStartup(     /* Returns !0 on error */
  SWS_FNPTR( SWS_FGef),               /* IN: Event filter */
  SWS_PTR( void)                      /* IN: Event filter context */
));

/* Release resources allocated */
SWS_EXPORT( void, SWS_SockCleanup( void));
SWS_EXTERNC( void, SWS_SockExit( void));

#ifdef __cplusplus
}
#endif /* __cplusplus */


/*
 * BSD sockets compatibility macros
 */
#ifdef SWS_SOCK_COMPAT

#define SOCKET SWS_SOCKET
#define INVALID_SOCKET SWS_INVALID_SOCKET
#define SOCKET_ERROR SWS_SOCKET_ERROR

#define u_char SWS_u_char
#define u_short SWS_u_short
#define u_int SWS_u_int
#define u_long SWS_u_long

#define FIONREAD SWS_FIONREAD
#define FIONBIO SWS_FIONBIO

#define IPPROTO_IP SWS_IPPROTO_IP
#define IPPROTO_ICMP SWS_IPPROTO_ICMP
#define IPPROTO_IGMP SWS_IPPROTO_IGMP
#define IPPROTO_TCP SWS_IPPROTO_TCP
#define IPPROTO_UDP SWS_IPPROTO_UDP
#define IPPROTO_RAW SWS_IPPROTO_RAW

#define INADDR_ANY SWS_INADDR_ANY
#define INADDR_LOOPBACK SWS_INADDR_LOOPBACK
#define INADDR_BROADCAST SWS_INADDR_BROADCAST
#define INADDR_NONE SWS_INADDR_NONE

#define SOCK_STREAM SWS_SOCK_STREAM
#define SOCK_DGRAM SWS_SOCK_DGRAM
#define SOCK_RAW SWS_SOCK_RAW

#define AF_UNSPEC SWS_AF_UNSPEC
#define AF_INET SWS_AF_INET
#define PF_INET SWS_PF_INET
#define AF_INET6 SWS_AF_INET6
#define PF_INET6 SWS_PF_INET6

#define SOL_SOCKET SWS_SOL_SOCKET
#define SO_DEBUG SWS_SO_DEBUG
#define SO_ACCEPTCONN SWS_SO_ACCEPTCONN
#define SO_REUSEADDR SWS_SO_REUSEADDR
#define SO_KEEPALIVE SWS_SO_KEEPALIVE
#define SO_DONTROUTE SWS_SO_DONTROUTE
#define SO_BROADCAST SWS_SO_BROADCAST
#define SO_USELOOPBACK SWS_SO_USELOOPBACK
#define SO_LINGER SWS_SO_LINGER
#define SO_OOBINLINE SWS_SO_OOBINLINE
#define SO_DONTLINGER SWS_SO_DONTLINGER
#define SO_SNDBUF SWS_SO_SNDBUF
#define SO_RCVBUF SWS_SO_RCVBUF
#define SO_SNDLOWAT SWS_SO_SNDLOWAT
#define SO_RCVLOWAT SWS_SO_RCVLOWAT
#define SO_SNDTIMEO SWS_SO_SNDTIMEO
#define SO_RCVTIMEO SWS_SO_RCVTIMEO
#define SO_ERROR SWS_SO_ERROR
#define SO_TYPE SWS_SO_TYPE

#define SOL_IP SWS_SOL_IP
#define IP_OPTIONS SWS_IP_OPTIONS
#define IP_HDRINCL SWS_IP_HDRINCL
#define IP_TOS SWS_IP_TOS
#define IP_TTL SWS_IP_TTL
#define IP_MULTICAST_IF SWS_IP_MULTICAST_IF
#define IP_MULTICAST_TTL SWS_IP_MULTICAST_TTL
#define IP_MULTICAST_LOOP SWS_IP_MULTICAST_LOOP
#define IP_ADD_MEMBERSHIP SWS_IP_ADD_MEMBERSHIP
#define IP_DROP_MEMBERSHIP SWS_IP_DROP_MEMBERSHIP
#define IP_DONTFRAGMENT SWS_IP_DONTFRAGMENT

#define MSG_OOB SWS_MSG_OOB
#define MSG_PEEK SWS_MSG_PEEK
#define MSG_DONTROUTE SWS_MSG_DONTROUTE

#define SOL_TCP SWS_SOL_TCP
#define TCP_NODELAY SWS_TCP_NODELAY

#define SOL_UDP SWS_SOL_UDP
#define UDP_NOCHECKSUM SWS_UDP_NOCHECKSUM

#define SOL_IOCTL SWS_SOL_IOCTL
#define IOCTL_FIONREAD SWS_IOCTL_FIONREAD

/* shutdown() Win32 and BSD */
#define SD_RECEIVE SWS_SD_RECEIVE
#define SD_SEND SWS_SD_SEND
#define SD_BOTH SWS_SD_BOTH
/* shutdown() POSIX */
#define SHUT_RD SWS_SD_RECEIVE
#define SHUT_WR SWS_SD_SEND
#define SHUT_RDWR SWS_SD_BOTH

#define SOMAXCONN SWS_SOMAXCONN

#define socket SWS_socket
#define in_addr SWS_in_addr
#define in_addr_t SWS_in_addr_t
#define socklen_t SWS_socklen_t
#define in6_addr SWS_in6_addr
#define ip_mreq SWS_ip_mreq
#define sockaddr SWS_sockaddr
#define sockaddr_in SWS_sockaddr_in
#define s_addr  SWS_s_addr
#define s_host SWS_s_host
#define s_net  SWS_s_net
#define s_imp SWS_s_imp
#define s_impno SWS_s_impno
#define s_lh SWS_s_lh
#define sockaddr_in6 SWS_sockaddr_in6
#define s6_addr  SWS_s6_addr
#define hostent SWS_hostent
#define protoent SWS_protoent
#define servent SWS_servent
#define h_addr SWS_h_addr
#define timeval SWS_timeval
#define DECL_TIMEOUT( n,m) SWS_DECL_TO( n, m)
#define linger SWS_linger
#undef fd_set
#define fd_set SWS_fd_set
#define _fd_set SWS_fd_set
#ifndef FD_SETSIZE
#define FD_SETSIZE SWS_FD_SETSIZE
#endif
#undef FD_ISSET
#define FD_ISSET SWS_FdIsSet
#undef FD_CLR
#define FD_CLR SWS_FdClr
#undef FD_SET
#define FD_SET SWS_FdSet
#undef FD_ZERO
#define FD_ZERO SWS_FdZero

#undef EWOULDBLOCK
#define EWOULDBLOCK SWS_EWOULDBLOCK
#ifndef EAGAIN
#define EAGAIN SWS_EWOULDBLOCK
#endif
#undef EINPROGRESS
#define EINPROGRESS SWS_EINPROGRESS
#undef EALREADY
#define EALREADY SWS_EALREADY
#undef ENOTSOCK
#define ENOTSOCK SWS_ENOTSOCK
#undef EDESTADDRREQ
#define EDESTADDRREQ SWS_EDESTADDRREQ
#undef EMSGSIZE
#define EMSGSIZE SWS_EMSGSIZE
#undef EPROTOTYPE
#define EPROTOTYPE SWS_EPROTOTYPE
#undef ENOPROTOOPT
#define ENOPROTOOPT SWS_ENOPROTOOPT
#undef EPROTONOSUPPORT
#define EPROTONOSUPPORT SWS_EPROTONOSUPPORT
#undef ESOCKTNOSUPPORT
#define ESOCKTNOSUPPORT SWS_ESOCKTNOSUPPORT
#undef EOPNOTSUPP
#define EOPNOTSUPP SWS_EOPNOTSUPP
#undef EPFNOSUPPORT
#define EPFNOSUPPORT SWS_EPFNOSUPPORT
#undef EAFNOSUPPORT
#define EAFNOSUPPORT SWS_EAFNOSUPPORT
#undef EADDRINUSE
#define EADDRINUSE SWS_EADDRINUSE
#undef EADDRNOTAVAIL
#define EADDRNOTAVAIL SWS_EADDRNOTAVAIL
#undef ENETDOWN
#define ENETDOWN SWS_ENETDOWN
#undef ENETUNREACH
#define ENETUNREACH SWS_ENETUNREACH
#undef ENETRESET
#define ENETRESET SWS_ENETRESET
#undef ECONNABORTED
#define ECONNABORTED SWS_ECONNABORTED
#undef ECONNRESET
#define ECONNRESET SWS_ECONNRESET
#undef ENOBUFS
#define ENOBUFS SWS_ENOBUFS
#undef EISCONN
#define EISCONN SWS_EISCONN
#undef ENOTCONN
#define ENOTCONN SWS_ENOTCONN
#undef ESHUTDOWN
#define ESHUTDOWN SWS_ESHUTDOWN
#undef ETOOMANYREFS
#define ETOOMANYREFS SWS_ETOOMANYREFS
#undef ETIMEDOUT
#define ETIMEDOUT SWS_ETIMEDOUT
#undef ECONNREFUSED
#define ECONNREFUSED SWS_ECONNREFUSED
#undef ELOOP
#define ELOOP SWS_ELOOP
#undef ENAMETOOLONG
#define ENAMETOOLONG SWS_ENAMETOOLONG
#undef EHOSTDOWN
#define EHOSTDOWN SWS_EHOSTDOWN
#undef EHOSTUNREACH
#define EHOSTUNREACH SWS_EHOSTUNREACH
#undef ENOTEMPTY
#define ENOTEMPTY SWS_ENOTEMPTY
#undef EPROCLIM
#define EPROCLIM SWS_EPROCLIM
#undef EUSERS
#define EUSERS SWS_EUSERS
#undef EDQUOT
#define EDQUOT SWS_EDQUOT
#undef ESTALE
#define ESTALE SWS_ESTALE
#undef EREMOTE
#define EREMOTE SWS_EREMOTE
#undef EHOST_NOT_FOUND
#define EHOST_NOT_FOUND SWS_EHOST_NOT_FOUND
#undef ETRY_AGAIN
#define ETRY_AGAIN SWS_ETRY_AGAIN
#undef ENO_RECOVERY
#define ENO_RECOVERY SWS_ENO_RECOVERY
#undef ENO_DATA
#define ENO_DATA SWS_ENO_DATA


#define bind SWS_bind
#define listen SWS_listen
#define connect SWS_connect
#define accept SWS_accept
#define recv SWS_recv
#define recvfrom SWS_recvfrom
#define send SWS_send
#define sendto SWS_sendto
#define select(n,r,w,e,t) SWS_select(r,w,e,t)
#define getsockname SWS_getsockname
#define getpeername SWS_getpeername
#define getsockopt SWS_getsockopt
#define setsockopt SWS_setsockopt
#define shutdown SWS_shutdown
/*#define close SWS_close*/
#define closesocket SWS_close
#define ioctlsocket SWS_ioctl
#define gethostbyaddr SWS_gethostbyaddr
#define gethostbyname SWS_gethostbyname
#define gethostname SWS_gethostname
#define getprotobyname SWS_getprotobyname
#define getprotobynumber SWS_getprotobynumber
#define getservbyname SWS_getservbyname
#define getservbyport SWS_getservbyport
#define inet_addr SWS_inet_addr
#define inet_ntoa SWS_inet_ntoa
#define htonl SWS_htonl
#define htons SWS_htons
#define ntohl SWS_ntohl
#define ntohs SWS_ntohs

#ifdef WIN32
/* Winsock compatibility */
#define WSAGetLastError() SWS_GetLastError()
#endif

#endif /* ndef SWS_SOCK_COMPAT */

#endif /* ndef SwsSock_H */
/* End of file */
