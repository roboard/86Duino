/****************************************************************************
 *
 * File name   : config.h
 * Function    : sockets library configuration file.
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
 * 07-Jun-01, LVR, v1.01 - Added DOS & Win32 sections
 * 05-Oct-01, LVR, v1.02 - Added VRP options
 * 23-Oct-01, LVR, v1.03 - Changed OPT_DATALINK_SLIP to 1
 * 30-Jan-02, LVR, v1.04 - Added OPT_TCP_PREALLOCATE_BUFFERS
 * 06-Nov-02, LVR, v1.05 - Added PKEY_TCP_PREALLOC
 * 02-Dec-02, LVR, v1.06 - Added OPT_DNS & PKEY_DNS_SERVER
 * 16-Dec-02, LVR, v1.07 - Added OPT_TCP_TTL & PKEY_TCP_TTL
 * 17-Dec-02, LVR, v1.08 - Added OPT_THREADS
 * 18-Dec-02, LVR, v1.09 - Added OPT_IP_ROUTES
 * 23-Dec-02, LVR, v1.10 - Added OPT_IP_MULTICAST & OPT_PROTO_IGMP
 * 30-Dec-02, LVR, v1.11 - Added OPT_IP_OPTIONS
 * 24-Jan-03, LVR, v1.12 - Increased OPT_BUFFER_SMALL_SIZE to 80
 * 06-Apr-03, LVR, v1.13 - Increase OPT_DPCS 32->64 and OPT_TCP_RTO_MIN 100->200
 * 31-Aug-03, LVR, v1.14 - Added Loopback & PPP sections
 * 03-Oct-03, LVR, v1.15 - Added min/max UDP/TCP ports
 * 19-Oct-05, LVR, v1.16 - Added OPT_ARP
 * 10-Jun-05, LVR, v1.17 - Added PKEY_IP_ROUTED
 * 03-Aug-05, LVR, v1.18 - Added P2P and PPP
 * 05-Sep-05, LVR, v1.19 - Added more P2P and PPP configuration
 * 27-Sep-05, LVR, v1.20 - Increased OPT_DPCS, OPT_ENET_TXQLEN && OPT_IP_TXQLEN
 *
 *
 * Comments:
 * --------
 * This file contains compilation & configuration options
 *
 ****************************************************************************/

#ifndef CONFIG_H
#define CONFIG_H 0x0120 /* Version no. BCD */

/*
 * Compile configuration
 */
#ifndef OPT_DEMO
#define OPT_DEMO 0
#endif

/* Builtin protocols */
#ifndef OPT_PROTO_IPRAW
#define OPT_PROTO_IPRAW 0
#endif
#ifndef OPT_PROTO_UDP
#define OPT_PROTO_UDP 0
#endif
#ifndef OPT_PROTO_TCP
#define OPT_PROTO_TCP 1
#endif
#ifndef OPT_PROTO_ENTRIES
#define OPT_PROTO_ENTRIES (OPT_PROTO_UDP + OPT_PROTO_TCP + OPT_RAWIP_SOCKETS)
#endif

/* Builtin data link providers */
#ifndef OPT_DATALINK_ETHERNET
#define OPT_DATALINK_ETHERNET 2 /* Max no. Ethernet adapters */
#endif
#ifndef OPT_DATALINK_P2P
#define OPT_DATALINK_P2P 0      /* Max no. Point to Point links */
#endif
#ifndef OPT_DATALINK_LOOPBACK
#define OPT_DATALINK_LOOPBACK 0
#endif
#ifndef OPT_DATALINK_ENTRIES
#define OPT_DATALINK_ENTRIES (OPT_DATALINK_LOOPBACK + OPT_DATALINK_ETHERNET + OPT_DATALINK_P2P)
#endif

/* Builtin services */
#ifndef OPT_DHCP
#define OPT_DHCP 0 /* 1= DHCP client for unconfigured datalinks, 2= All datalinks */
#endif
#if OPT_DHCP && !OPT_PROTO_UDP
#error The UDP protocol is required for the DHCP client
#endif

#ifndef OPT_DNS
#define OPT_DNS 0 /* 1= DNS client */
#endif
#if OPT_DNS && !OPT_PROTO_UDP
#error The UDP protocol is required for the DNS client
#endif

/* Threading model */
#define OPT_THREADS_NONE   0 /* No threads */
#define OPT_THREADS_COOP   1 /* Coroutines using setjmp */
#define OPT_THREADS_NATIVE 2 /* Platform native threads */

#ifndef OPT_THREADS
#define OPT_THREADS OPT_THREADS_NONE
#endif

/* Buffer pool */
#define OPT_BUFFER_LARGE_SIZE 1514 /* Normal IP is 576, max Ethernet is 1514 */
#define OPT_BUFFER_SMALL_SIZE 80 /* Smallest datagram 20 bytes, Ethernet min 60 */
#define OPT_BUFFER_LARGE_COUNT 32
#define OPT_BUFFER_SMALL_COUNT 90
#define OPT_BUFFER_HANDLES (OPT_BUFFER_LARGE_COUNT + OPT_BUFFER_SMALL_COUNT)

/* Scheduling */
#define OPT_TIMERS (8 \
  + (OPT_DHCP ? OPT_DATALINK_ENTRIES : 0) \
  + (OPT_ARP ? OPT_DATALINK_ETHERNET : 0) \
  + (4 * OPT_DATALINK_P2P) \
  + (OPT_DATALINK_P2P ? 3 * OPT_PPP : 0) \
  + (OPT_PROTO_IGMP ? 1 : 0) \
  + (1 * OPT_IP_REASSEMBLY_BUFFERS) \
  + (3 * OPT_TCP_SOCKETS) \
  ) /* Timer pool size */
#define OPT_EVENTS (1 \
  + 4 * OPT_SOCKETS \
  ) /* Event pool size */
#define OPT_DPCS 64 /* Max no. pending deferred procedure calls */

/* Ethernet */
#define OPT_ENET_8023 0 /* Tx frame format. 0= RFC-894, !0= IEEE 802.3 (RFC-1042) */
#define OPT_ENET_TXQLEN 64 /* Max no. buffers in Tx queue (at least OPT_IP_TXQLEN) */

#ifndef OPT_ARP
#define OPT_ARP 1
#endif
#define OPT_ARP_ENTRIES 16 /* ARP cache size */
#define OPT_ARP_TIMEOUT 120 /* ARP cache timeout in secs */
#define OPT_ARP_RESPONSE 2000 /* ARP response timeout in millisecs */

/* P2P */
#ifndef OPT_P2P_VJCOMP
#define OPT_P2P_VJCOMP 1  /* !0 enables VJ compressed headers */
#endif

/* SLIP */
#ifndef OPT_SLIP
#define OPT_SLIP 1 /* !0 adds SLIP to available P2P protocols */
#endif
#define OPT_SLIP_TXBUFFER 2048 /* Tx buffer size (at least 2*1006+2) */
#define OPT_SLIP_RXBUFFER 2048 /* Rx buffer size (at least 2*1006+2) */

/* PPP */
#ifndef OPT_PPP
#define OPT_PPP 1 /* !0 adds PPP to available P2P protocols */
#endif
#define OPT_PPP_TXBUFFER 3072 /* Tx buffer size (at least 2*1508+2) */
#define OPT_PPP_RXBUFFER 3072 /* Rx buffer size (at least 2*1508+2) */
#define OPT_PPP_RETRY_TERM 2 /* TERM-REQ retries, RFC 1661 p24 */
#define OPT_PPP_RETRY_CFG 10 /* CFG-REQ retries, RFC 1661 p24 */
#define OPT_PPP_RETRY_NAK 5 /* NAKs until REJ, RFC 1661 p25 */
#define OPT_PPP_RETRY_PERIOD 3000 /* Retry interval in ms, RFC 1661 p24 */

/* IP parameters */
#define OPT_IP_TTL 64 /* Time To Live RFC 1700 says 64 */

#ifndef OPT_IP_REASSEMBLY_BUFFERS
#define OPT_IP_REASSEMBLY_BUFFERS 0 /* Max datagrams pending reassembly 0= disable */
#endif
#define OPT_IP_REASSEMBLY_TIMEOUT 60 /* 60 secs as per RFC1122 p57 */
#define OPT_IP_REASSEMBLY_MAX 0 /* Max size datagram that can be reassembled, 0= no limit */

#define OPT_IP_MAX_FRAGMENT 0 /* Max Tx IP fragment length (>= 68), 0= MTU for subnet */
#define OPT_IP_TXQLEN 64 /* Max no. buffers in Tx queue, at least enough for TCP send window */
#if OPT_ENET_TXQLEN < OPT_IP_TXQLEN
#error OPT_ENET_TXQLEN must be >= OPT_IP_TXQLEN
#endif

#ifndef OPT_IP_ROUTES
#define OPT_IP_ROUTES 0 /* No. entries in route cache, 0= disabled */
#endif
#if OPT_IP_ROUTES && OPT_IP_ROUTES < 6 * OPT_DATALINK_ENTRIES
#error OPT_IP_ROUTES must be >= 6 * OPT_DATALINK_ENTRIES
#endif

#ifndef OPT_IP_OPTIONS
#define OPT_IP_OPTIONS 0 /* !0 to process received RECORD_ROUTE && TIMESTAMP options */
#endif

#ifndef OPT_IP_DISCARD_NOPROTO
#define OPT_IP_DISCARD_NOPROTO 1 /* !0 to ignore datagrams for unknown protocols */
#endif

#ifndef OPT_IP_MULTICAST
#define OPT_IP_MULTICAST 0 /* !0 to enable multicast */
#endif

#ifndef OPT_IP_ROUTED
#define OPT_IP_ROUTED 0 /* !0 to enable routing */
#endif

/* IGMP */
#ifndef OPT_PROTO_IGMP
#define OPT_PROTO_IGMP 0 /* IGMP version, 0= disabled */
#endif

/* ICMP */
#ifndef OPT_PROTO_ICMP
#define OPT_PROTO_ICMP 0
#endif
#ifndef OPT_ICMP_ECHO_SERVER
#define OPT_ICMP_ECHO_SERVER 0 /* !0 to include echo server (ping) */
#endif

/* RAWIP */
#if OPT_PROTO_IPRAW
#define OPT_RAWIP_SOCKETS 4 /* Socket pool size */
#define OPT_RAWIP_BUFFERS 4 /* No. datagrams buffered per socket */
#else
#define OPT_RAWIP_SOCKETS 0
#endif

/* UDP */
#if OPT_PROTO_UDP
#define OPT_UDP_SOCKETS 4 /* Socket pool size */
#define OPT_UDP_BUFFERS 4 /* Max no. datagrams buffered per socket */
#define OPT_UDP_CHECKSUM 1 /* 0= disable checksum, 1= verify checksum, 2= require checksum */
#ifndef OPT_UDP_DISCARD_NOPORT
#define OPT_UDP_DISCARD_NOPORT 0 /* !0 to ignore datagrams for closed ports */
#endif
#ifndef OPT_UDP_MIN_PORT
#define OPT_UDP_MIN_PORT 1024  /* 1st auto allocated port no. */
#endif
#ifndef OPT_UDP_MAX_PORT
#define OPT_UDP_MAX_PORT 5000  /* Max auto allocated port no. */
#endif
#else
#define OPT_UDP_SOCKETS 0
#endif

/* TCP */
#if OPT_PROTO_TCP
#define OPT_TCP_SOCKETS 8 /* Socket pool size */
#ifndef OPT_TCP_PREALLOCATE_BUFFERS
#define OPT_TCP_PREALLOCATE_BUFFERS 1 /* !0 = pre-allocate send & receive buffers */
#endif
#ifndef OPT_TCP_RTO_QUEUE_LEN
#define OPT_TCP_RTO_QUEUE_LEN 16 /* Entries in the round trip time estimation buffer */
#endif
#define OPT_TCP_RTO 3000 /* Initial retransmit timeout in millisecs */
#define OPT_TCP_RTO_MIN 200 /* Min retransmit timeout in ms */
#define OPT_TCP_RTO_MAX (240*1000UL) /* Max retransmit timeout (2x MSL as per RFC1122 p96) */
#define OPT_TCP_SYN_FAIL (180*1000UL) /* SYN fail timeout (RFC1122 p101 3 mins) */
#define OPT_TCP_SEG_FAIL (100*1000UL) /* Data fail timeout (RFC1122 p101 100s) */
#define OPT_TCP_TIMEWAIT (240*1000UL) /* Time wait timeout (2x MSL), 0= none */
#define OPT_TCP_ACKWAIT 20 /* Deferred ACK delay 0= none */
#define OPT_TCP_MSS_MAX 0 /* Max MSS, 0= use MTU from IP */
#define OPT_TCP_TTL 60 /* Default TTL */
#ifndef OPT_TCP_DISCARD_NOPORT
#define OPT_TCP_DISCARD_NOPORT 1 /* !0 to ignore datagrams for closed ports */
#endif
#ifndef OPT_TCP_MIN_PORT
#define OPT_TCP_MIN_PORT 1024U  /* 1st auto allocated port no. */
#endif
#ifndef OPT_TCP_MAX_PORT
#define OPT_TCP_MAX_PORT 5000U  /* Max auto allocated port no. */
#endif

/* NB for platforms with 16-bit size_t and OPT_TCP_PREALLOCATE_BUFFERS is defined
 * OPT_TCP_SOCKETS * (OPT_TCP_RCVBUF + OPT_TCP_SNDBUF) < 64K */
#define OPT_TCP_RCVBUF 1024 /* Default receive buffer size */
#define OPT_TCP_SNDBUF 1024 /* Default send buffer size */

#else
#define OPT_TCP_SOCKETS 0
#endif


/* Max no. open sockets */
#define OPT_SOCKETS (OPT_RAWIP_SOCKETS + OPT_UDP_SOCKETS + OPT_TCP_SOCKETS)


/*
 * Runtime configuration property keys
 * Config files have a format:
 * [section]
 * value=
 */
#ifndef OPT_LOGFILE
#define OPT_LOGFILE "sws_sock.log" /* default log filename, stderr, stdout or empty for none */
#endif

/* Logging */
#define PSECT_LOG           "log"
#define PKEY_LOG_TRACE              PSECT_LOG, "trace"    /* Log trace level */
#define PKEY_LOG_FILE               PSECT_LOG, "file"     /* Log trace output filename */
#define PKEY_LOG_IPDUMP             PSECT_LOG, "pdump"    /* IP packet dumper detail, 0= off */
#define PKEY_LOG_ENDUMP             PSECT_LOG, "edump"    /* Ethernet packet dumper detail, 0= off */
/* Scheduling */
#define PSECT_TIMER         "timer"
#define PKEY_TIMER_COUNT            PSECT_TIMER, "count"
#define PSECT_EVENT         "event"
#define PKEY_EVENT_COUNT            PSECT_EVENT, "count"
#define PSECT_DPC           "dpc"
#define PKEY_DPC_COUNT              PSECT_DPC, "count"
/* Buffer pool */
#define PSECT_BUFFER        "buffer"
#define PKEY_BUFFER_HANDLES         PSECT_BUFFER, "handles"
#define PKEY_BUFFER_LARGE_COUNT     PSECT_BUFFER, "large_count"
#define PKEY_BUFFER_LARGE_SIZE      PSECT_BUFFER, "large_size"
#define PKEY_BUFFER_SMALL_COUNT     PSECT_BUFFER, "small_count"
#define PKEY_BUFFER_SMALL_SIZE      PSECT_BUFFER, "small_size"
/* Loopback */
#define PSECT_LOOPBACK      "loopback"
#define PKEY_LOOPBACK_ENABLE        PSECT_LOOPBACK, "enable" /* Enable loopback */
/* Ethernet */
#define PSECT_ENET          "enet"
#define PKEY_ENET_802P3             PSECT_ENET, "802p3"   /* Frame format, 0= RFC894, !0= IEEE 802.3 (RFC1042) */
#define PKEY_ENET_TXQ               PSECT_ENET, "txq"     /* Buffers in send queue */
#define PKEY_ENET_SELF              PSECT_ENET, "sendself"/* Transmit packets addressed to self */
#define PSECT_ARP           "arp"
#define PKEY_ARP_COUNT              PSECT_ARP, "count"    /* ARP cache size */
#define PKEY_ARP_TIMEOUT            PSECT_ARP, "timeout"  /* ARP cache timeout in secs */
#define PKEY_ARP_RESPONSE           PSECT_ARP, "response" /* ARP response timeout in millisecs */
/* P2P */
#define PSECT_P2P           "p2p"
#define PSECT_P2P_USERS     "p2p_users"
/* SLIP */
#define PSECT_SLIP          "slip"
#define PKEY_SLIP_TXBUFFER          PSECT_SLIP, "txbuffer"/* Tx buffer size */
#define PKEY_SLIP_RXBUFFER          PSECT_SLIP, "rxbuffer"/* Rx buffer size */
/* PPP */
#define PSECT_PPP           "ppp"
#define PKEY_PPP_TXBUFFER           PSECT_PPP,  "txbuffer"/* Tx buffer size */
#define PKEY_PPP_RXBUFFER           PSECT_PPP,  "rxbuffer"/* Rx buffer size */
#define PKEY_PPP_RXBUFFER           PSECT_PPP,  "rxbuffer"/* Rx buffer size */
#define PKEY_PPP_RETRY_TERM         PSECT_PPP,  "retryterm" /* TERM-REQ retries */
#define PKEY_PPP_RETRY_CFG          PSECT_PPP,  "retrycfg" /* CFG-REQ retries */
#define PKEY_PPP_RETRY_NAK          PSECT_PPP,  "retrynak" /* NAK's until REJ */
#define PKEY_PPP_RETRY_PERIOD       PSECT_PPP,  "retryms"  /* Retry interval */
/* IP */
#define PSECT_IP            "ip"
#define PKEY_IP_REASSEMBLY          PSECT_IP, "reassembly"      /* Max no. datagrams pending reassembly */
#define PKEY_IP_REASSEMBLY_TIMEOUT  PSECT_IP, "reassembly_timeout" /* Reassembly timeout */
#define PKEY_IP_REASSEMBLY_MAX      PSECT_IP, "reassembly_max" /* Max size datagram that can be reassembled */
#define PKEY_IP_MAXFRAG             PSECT_IP, "maxfrag"        /* Max Tx IP fragment */
#define PKEY_IP_TTL                 PSECT_IP, "ttl"            /* Time To Live */
#define PKEY_IP_TXQ                 PSECT_IP, "txq"            /* Buffers in send queue */
#define PKEY_IP_NOPROTO             PSECT_IP, "noproto"        /* 0= send ICMP_PROT_UNREACH else discard */
#define PKEY_IP_ROUTES              PSECT_IP, "route_cache"    /* Route cache size */
#define IP_USER_ROUTE                         "route"          /* User route1..: dst mask next adapter */
#define PKEY_IP_ROUTED              PSECT_IP, "routed"         /* Enable routing */
/* IGMP */
#define PSECT_IGMP          "igmp"
#define PKEY_IGMP_VERSION           PSECT_IGMP, "version"  /* 0= disable IGMP */
/* ICMP */
#define PSECT_ICMP          "icmp"
#define PKEY_ICMP_ECHO_SERVER       PSECT_ICMP, "echo_server"  /* 0= disable echo server */
/* RAWIP */
#define PSECT_RAWIP         "rawip"
#define PKEY_RAWIP_SOCKETS          PSECT_RAWIP, "sockets"
#define PKEY_RAWIP_BUFFERS          PSECT_RAWIP, "buffers" /* Max no. datagrams buffered per socket */
/* UDP */
#define PSECT_UDP           "udp"
#define PKEY_UDP_SOCKETS            PSECT_UDP, "sockets"
#define PKEY_UDP_BUFFERS            PSECT_UDP, "buffers"  /* Max no. datagrams buffered per socket */
#define PKEY_UDP_CHECKSUM           PSECT_UDP, "checksum" /* 0= disable checksum, 1= verify checksum, 2= require checksum */
#define PKEY_UDP_NOPORT             PSECT_UDP, "noport"   /* 0= send ICMP_NOPORT else discard */
#define PKEY_UDP_LOPORT             PSECT_UDP, "loport"   /* Min port number */
#define PKEY_UDP_HIPORT             PSECT_UDP, "hiport"   /* Max port number */
/* TCP */
#define PSECT_TCP           "tcp"
#define PKEY_TCP_SOCKETS            PSECT_TCP, "sockets"
#define PKEY_TCP_RTO                PSECT_TCP, "rto"      /* Initial retransmit timeout */
#define PKEY_TCP_RTO_MIN            PSECT_TCP, "rtomin"
#define PKEY_TCP_RTO_MAX            PSECT_TCP, "rtomax"
#define PKEY_TCP_SYN_FAIL           PSECT_TCP, "synfail"  /* Connect timeout */
#define PKEY_TCP_SEG_FAIL           PSECT_TCP, "segfail"  /* Data timeout */
#define PKEY_TCP_TIMEWAIT           PSECT_TCP, "timewait" /* Delay in timewait state */
#define PKEY_TCP_ACKWAIT            PSECT_TCP, "ackwait"  /* Max ack deferrment */
#define PKEY_TCP_RCVBUF             PSECT_TCP, "rcvbuf"
#define PKEY_TCP_SNDBUF             PSECT_TCP, "sndbuf"
#define PKEY_TCP_MSS_MAX            PSECT_TCP, "mss"      /* Default max segment size */
#define PKEY_TCP_PREALLOC           PSECT_TCP, "prealloc" /* !0 to pre-allocate tx & rx buffers */
#define PKEY_TCP_NOPORT             PSECT_TCP, "noport"   /* 0= send RST if closed else discard */
#define PKEY_TCP_TTL                PSECT_TCP, "ttl"      /* Default TTL */
#define PKEY_TCP_LOPORT             PSECT_TCP, "loport"   /* Min port number */
#define PKEY_TCP_HIPORT             PSECT_TCP, "hiport"   /* Max port number */
/* DNS client */
#define PSECT_DNS           "dns"
#define PKEY_DNS_HOSTNAME           PSECT_DNS, "hostname"
#define PKEY_DNS_SERVER             PSECT_DNS, "server"

/* DHCP client */
#define PSECT_DHCP          "dhcp"
#define PKEY_DHCP_ENABLE            PSECT_DHCP, "enable"

/* DOS platform options */
#define PSECT_DOS           "dos"
#define PKEY_DOS_DRIVER             PSECT_DOS, "driver"   /* Packet driver */
#define PKEY_DOS_STACK              PSECT_DOS, "stack"    /* Default thread stack size */
#define PKEY_DOS_YIELD              PSECT_DOS, "yield"    /* Call kbhit if idle in PlatformIdle */

/* DJGPP platform options */
#define PSECT_DJGPP         "djgpp"
#define PKEY_DJGPP_DRIVER           PSECT_DJGPP, "driver" /* Packet driver */
#define PKEY_DJGPP_STACK            PSECT_DJGPP, "stack"  /* Default thread stack size */
#define PKEY_DJGPP_YIELD            PSECT_DJGPP, "yield"  /* Call dpmi_yield in PlatformIdle */

/* WIN32 platform options */
#define PSECT_WIN32         "win32"
#define PKEY_WIN32_STACK            PSECT_WIN32, "stack"  /* Default thread stack size */

/* VRP platform options */
#define PSECT_VRP           "vrp"
#define PKEY_VRP_STACK              PSECT_VRP, "stack"    /* Default thread stack size */

/* Enet adapter config. In the section named by the link provider's ident string */
#define PVAL_IPADDRESS                         "ipaddress"
#define PVAL_IPSUBNET                          "ipsubnet"
#define PVAL_IPGATEWAY                         "ipgateway"
#define PVAL_DHCP                              "dhcp"

/* P2P adapter config. In the section named by the link provider's ident string */
#define PVAL_P2P_MODEM                         "modem"    /* null or hayes */
#define PVAL_P2P_MODEMINIT                     "modeminit"
#define PVAL_P2P_RINGS                         "rings"
#define PVAL_P2P_CONNECT                       "connect"  /* 'Dial on startup' connection  */
#define PVAL_P2P_DEMAND                        "demand"   /* 'Dial on demand' connection */

/* P2P connection info. In the section named by PVAL_P2P_CONNECT or PVAL_P2P_DEMAND */
#define PVAL_P2P_PROTOCOL                      "protocol" /* ppp or slip */
#define PVAL_P2P_AUTH                          "auth"     /* none, pap or chap */
#define PVAL_P2P_DIAL                          "dial"     /* client dial string */
#define PVAL_P2P_LOGIN                         "login"    /* !0 for client login */
#define PVAL_P2P_TIMEOUT                       "timeout"  /* Connection timeout, milliseconds */
#define PVAL_P2P_IDLE                          "idle"     /* Idle disconnect timeout, seconds */
#define PVAL_P2P_VJSLOTS                       "vjslots"  /* VJ TCP/IP header compression slots */
#define PVAL_P2P_USER                          "username" /* client username */
#define PVAL_P2P_PASSWD                        "password" /* client password */
/* Also PVAL_IPADDRESS, PVAL_IPSUBNET & PVAL_IPGATEWAY */

#endif /* ndef CONFIG_H */
/* End of file */
