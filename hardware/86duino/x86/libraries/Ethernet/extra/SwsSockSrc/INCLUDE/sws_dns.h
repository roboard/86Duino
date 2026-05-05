/****************************************************************************
 *
 * File name   : sws_dns.h
 * Function    : Public API for the DNS client
 * Project     : SwsSock
 * Authors     : Lawrence Rust
 * Systems     : ANSI C
 * Version     : 1.0
 *
 ****************************************************************************
 * File History
 * ------------
 * 30-Nov-02, LVR, v1.00 - Created.
 * 29-Jun-05, LVR, v1.01 - Removed SWS_DnsSetHostname & SWS_DnsGet/SetServer
 * 27-Jun-09, LVR, v1.02 - Added SWS_DNS_NB
 *
 * Comments:
 * --------
 * This file contains the external interface to the DNS client module
 *
 ****************************************************************************/

#ifndef SWS_DNS_H
#define SWS_DNS_H 0x0102 /* Version no. BCD */

/* Dependencies */
#include <stddef.h>   /* size_t */
#include "sws_host.h" /* Basic types */


/* Constants */

#define SWS_DNS_SERVER 53 /* UDP & TCP */
#define SWS_DNS_NB 137 /* NetBIOS name server (UDP & TCP) */

/* DNS message from RFC1035 page 25

    +---------------------+
    |        Header       |
    +---------------------+
    |       Question      | the question for the name server
    +---------------------+
    |        Answer       | RRs answering the question
    +---------------------+
    |      Authority      | RRs pointing toward an authority
    +---------------------+
    |      Additional     | RRs holding additional information
    +---------------------+

DNS message header from RFC1035 p26

                                    1  1  1  1  1  1
      0  1  2  3  4  5  6  7  8  9  0  1  2  3  4  5
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                      ID                       |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |QR|   Opcode  |AA|TC|RD|RA|   Z    |   RCODE   |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                    QDCOUNT                    |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                    ANCOUNT                    |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                    NSCOUNT                    |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                    ARCOUNT                    |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+

DNS question from RFC1035 p28

                                    1  1  1  1  1  1
      0  1  2  3  4  5  6  7  8  9  0  1  2  3  4  5
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                                               |
    /                     QNAME                     /
    /                                               /
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                     QTYPE                     |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                     QCLASS                    |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+

DNS resource record (RR) from RFC1035 page 29

                                    1  1  1  1  1  1
      0  1  2  3  4  5  6  7  8  9  0  1  2  3  4  5
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                                               |
    /                                               /
    /                      NAME                     /
    |                                               |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                      TYPE                     |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                     CLASS                     |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                      TTL                      |
    |                                               |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
    |                   RDLENGTH                    |
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--|
    /                     RDATA                     /
    /                                               /
    +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+

* UDP messages are 512 octets or less
* TCP messages are prefixed with a 16-bit length
*/

/* DNS message offsets */
#define SWS_DNS_HDR_ID       0
#define SWS_DNS_HDR_OP       2
#define SWS_DNS_HDR_QDCOUNT  4  /* No. question sections */
#define SWS_DNS_HDR_ANCOUNT  6  /* No. answer sections */
#define SWS_DNS_HDR_NSCOUNT  8  /* No. authority sections */
#define SWS_DNS_HDR_ARCOUNT  10 /* No. additional sections */
#define SWS_DNS_HDR_LEN      12

/* DNS opcodes */
#define SWS_DNS_OP_QUERY   (0<<11)
#define SWS_DNS_OP_IQUERY  (1<<11) /* Deprecated in RFC 3425, Nov 2002 */
#define SWS_DNS_OP_CQUERYM (2<<11)
/* NetBIOS (RFC 1001, 1002 (STD19)) extended opcodes */
#define SWS_DNS_OP_CQUERYU (3<<11)
#define SWS_DNS_OP_REGIST  (5<<11)
#define SWS_DNS_OP_RELEASE (6<<11)
#define SWS_DNS_OP_WACK    (7<<11)
#define SWS_DNS_OP_REFRESH (9<<11)

#define SWS_DNS_QR_MASK     (1<<15) /* 0= Query !0= Reply */
#define SWS_DNS_OPCODE_MASK (0xf<<11)
#define SWS_DNS_AA_MASK     (1<<10) /* R: Authoritative answer */
#define SWS_DNS_TC_MASK     (1<<9) /* Message truncated */
#define SWS_DNS_RD_MASK     (1<<8) /* Q: Recursion desired */
#define SWS_DNS_RA_MASK     (1<<7) /* R: Recursion available */
#define SWS_DNS_Z_MASK      (0x7<<4)
#define SWS_DNS_BB_MASK     (1<<4) /* Q: Netbios broadcast */
#define SWS_DNS_RCODE_MASK  0x000f /* R: response code */

/* Reply codes */
#define SWS_DNS_RC_GOOD    0
#define SWS_DNS_RC_FORMAT  1 /* NS unable to interpret query */
#define SWS_DNS_RC_SFAIL   2 /* NS failure */
#define SWS_DNS_RC_NAMERR  3 /* Name error (from authoritative NS) */
#define SWS_DNS_RC_NOTIMP  4 /* Not implemented */
#define SWS_DNS_RC_REFUSED 5 /* Policy refusal */
/* Extras */
#define SWS_DNS_RC_NWERR   14 /* Network problem */
#define SWS_DNS_RC_NOREPLY 15 /* No reply from name server */

/* DNS question offsets */
#define SWS_DNS_QTYPE        0
#define SWS_DNS_QCLASS       2
#define SWS_DNS_QLEN         4

/* DNS RR offsets */
#define SWS_DNS_RR_TYPE      0
#define SWS_DNS_RR_CLASS     2
#define SWS_DNS_RR_TTL       4
#define SWS_DNS_RR_RDLEN     8
#define SWS_DNS_RR_RDATA    10 /* + RDLEN */

/* DNS TYPES RF1035 p12 */
#define SWS_DNS_TYPE_A       1 /* Host address */
#define SWS_DNS_TYPE_NS      2 /* Authoritative name server */
#define SWS_DNS_TYPE_MD      3 /* Mail destination, MX preferred */
#define SWS_DNS_TYPE_MF      4 /* Mail forwarder, MX preferred */
#define SWS_DNS_TYPE_CNAME   5 /* Canonical name */
#define SWS_DNS_TYPE_SOA     6 /* Start Of Authority zone */
#define SWS_DNS_TYPE_MB      7 /* Mailbox domain name */
#define SWS_DNS_TYPE_MG      8 /* Mail group name */
#define SWS_DNS_TYPE_MR      9 /* Mail rename domain */
#define SWS_DNS_TYPE_NULL   10 /* Null RR */
#define SWS_DNS_TYPE_WKS    11 /* Well known service */
#define SWS_DNS_TYPE_PTR    12 /* Domain name pointer */
#define SWS_DNS_TYPE_HINFO  13 /* Host info */
#define SWS_DNS_TYPE_MINFO  14 /* Mailbox info */
#define SWS_DNS_TYPE_MX     15 /* Mail exchange */
#define SWS_DNS_TYPE_TXT    16 /* Text strings */
/* DNS QTYPES RF1035 p13 */
#define SWS_DNS_QTYPE_AXFR  252 /* Transfer entire zone */
#define SWS_DNS_QTYPE_MAILB 253 /* mailbox-related records */
#define SWS_DNS_QTYPE_MAILA 254 /* Mail agents, MX preferred */
#define SWS_DNS_QTYPE_STAR  255 /* All records */
/* NetBIOS (RFC 1001, 1002 (STD19)) QTYPES */
#define SWS_DNS_QTYPE_NB     0x20
#define SWS_DNS_QTYPE_NBSTAT 0x21 /* Node status */

/* DNS CLASS RF1035 p13 */
#define SWS_DNS_CLASS_IN     1 /* Internet class */
#define SWS_DNS_CLASS_CS     2 /* CSNET class (obsolete) */
#define SWS_DNS_CLASS_CH     3 /* CHAOS class */
#define SWS_DNS_CLASS_HS     4 /* Hesiod class */
/* DNS QCLASS RF1035 p13 */
#define SWS_DNS_QCLASS_STAR 255 /* Any class */


/* Types */

/* DNS query handle */
typedef SWS_PTR( struct SWS_DnsSQuery) SWS_DnsQHandle;

/* DNS record type */
typedef enum SWS_EDnsRecordType
  {
  SWS_kDnsQuestion,
  SWS_kDnsAnswer,
  SWS_kDnsAuthority,
  SWS_kDnsAdditional
  } SWS_EDnsRecordType;

/* DNS record */
typedef struct SWS_SDnsQRecord
  {
  SWS_EDnsRecordType eRecordType;
  ui16 nameIndex;
  ui16 rtype;
  ui16 rclass;
  ui32 ttl;
  ui16 rdlen;
  ui16 dataIndex;
  } SWS_SDnsQRecord;


/*
 * Functions
 */
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* General DNS query */
SWS_EXTERN( SWS_DnsQHandle, SWS_DnsOpenQuery(
  unsigned,                           /* IN, DNS opcode */
  SWS_PTR( const char),               /* IN: Query text */
  unsigned,                           /* IN: Query type */
  unsigned,                           /* IN: Query class */
  ui32,                               /* IN: Preferred server or INADDR_ANY */
  unsigned                            /* IN: Port. 0= default */
));

SWS_EXTERN( int, SWS_DnsCloseQuery(
  SWS_DnsQHandle                      /* IN, DNS query handle */
));

SWS_EXTERN( int, SWS_DnsQueryRcode(   /* Returns response code */
  SWS_DnsQHandle                      /* IN, DNS query handle */
));

/* Get next record */
SWS_EXTERN( int, SWS_DnsQueryRead(
  SWS_DnsQHandle,                     /* IN, DNS query handle */
  SWS_PTR( SWS_SDnsQRecord)           /* OUT Next resource record */
));

/* Reset to first record */
SWS_EXTERN( int, SWS_DnsQueryRewind(
  SWS_DnsQHandle                      /* IN, DNS query handle */
));

/* Get record name */
SWS_EXTERN( unsigned, SWS_DnsQueryName( /* Returns index to following data */
  SWS_PTR( char),                     /* OUT: ASCIIZ name */
  size_t,                             /* IN: buffer size */
  SWS_DnsQHandle,                     /* IN, DNS query handle */
  unsigned                            /* IN: Start index */
));

/* Get record data */
SWS_EXTERN( void, SWS_DnsQueryData(
  SWS_PTR( void),                     /* OUT: data buffer */
  size_t,                             /* IN: buffer size */
  SWS_DnsQHandle,                     /* IN, DNS query handle */
  unsigned                            /* IN: Start index */
));

/* Get default DNS server address */
SWS_EXTERN( ui32, SWS_DnsDefaultServer( void));

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* ndef SWS_DNS_H */
/* End of file */
