/****************************************************************************
 *
 * File name   : sws_rpc.h
 * Function    : DCE/RPC definitions
 * Authors     : Lawrence Rust
 * Systems     : ANSI C
 * Version     : 1.0
 *
 ****************************************************************************
 * File History
 * ------------
 * 12-Aug-03, LVR, v1.00 - Created.
 *
 * Comments:
 * --------
 * This file contains common DCE v1.1 RPC definitions
 *
 * Reference: The Open Group www.opengroup.org
 * Document Number C706 August 1997
 * http://www.opengroup.org/onlinepubs/009629399/toc.htm
 * or http://www.opengroup.org/online-pubs?DOC=009629399&FORM=PDF
 *
 ****************************************************************************/

#ifndef SWS_RPC_H
#define SWS_RPC_H 0x0100 /* Version no. BCD */

/* Dependencies */


/*
 * Constants
 */
#define SWS_RPC_EPMAP 135 /* DCE Endpoint mapper (UDP & TCP) */

/* PDU types */
#define SWS_RPC_PTYPE_REQST  0x00 /* CO/CL */
#define SWS_RPC_PTYPE_PING   0x01 /* CL */
#define SWS_RPC_PTYPE_RESPNS 0x02 /* CO/CL */
#define SWS_RPC_PTYPE_FAULT  0x03 /* CO/CL */
#define SWS_RPC_PTYPE_WORK   0x04 /* CL */
#define SWS_RPC_PTYPE_NOCALL 0x05 /* CL */
#define SWS_RPC_PTYPE_REJ    0x06 /* CL */
#define SWS_RPC_PTYPE_ACK    0x07 /* CL */
#define SWS_RPC_PTYPE_CNCL   0x08 /* CL */
#define SWS_RPC_PTYPE_FACK   0x09 /* CL */
#define SWS_RPC_PTYPE_CANACK 0x0a /* CL */
#define SWS_RPC_PTYPE_BIND   0x0b /* CO */
#define SWS_RPC_PTYPE_BNDACK 0x0c /* CO */
#define SWS_RPC_PTYPE_BNDNAK 0x0d /* CO */
#define SWS_RPC_PTYPE_ALTCTX 0x0e /* CO */
#define SWS_RPC_PTYPE_ACRPLY 0x0f /* CO */
#define SWS_RPC_PTYPE_SHUTDN 0x11 /* CO */
#define SWS_RPC_PTYPE_COCAN  0x12 /* CO */
#define SWS_RPC_PTYPE_ORPHN  0x13 /* CO */

/* NDR types */
#define SWS_RPC_NDR0_ANSI   0U
#define SWS_RPC_NDR0_EBCDIC 1U
#define SWS_RPC_NDR0_BE     0U
#define SWS_RPC_NDR0_LE     1U
#define SWS_RPC_NDR1_IEEE   0U
#define SWS_RPC_NDR1_VAX    1U
#define SWS_RPC_NDR1_CRAY   2U
#define SWS_RPC_NDR1_IBM    3U
#define SWS_RPC_NDR_STD (SWS_RPC_NDR0_ANSI | (SWS_RPC_NDR0_LE << 4) | (SWS_RPC_NDR1_IEEE << 8))

/* Presentation result types */
#define SWS_RPC_PRESULT_ACCEPT          0
#define SWS_RPC_PRESULT_USER_REJECT     1U
#define SWS_RPC_PRESULT_PROVIDER_REJECT 2U

/* Presentation reason types */
#define SWS_RPC_PREASON_UNSPECIFIED     0
#define SWS_RPC_PREASON_ABS_UNSUPP      1U
#define SWS_RPC_PREASON_TFR_UNSUPP      2U
#define SWS_RPC_PREASON_EXCEEDED        3U

/* BindNak reason codes */
#define SWS_RPC_BINDNAK_REASON_NOT_SPECIFIED            0
#define SWS_RPC_BINDNAK_TEMPORARY_CONGESTION            1
#define SWS_RPC_BINDNAK_LOCAL_LIMIT_EXCEEDED            2
#define SWS_RPC_BINDNAK_CALLED_PADDR_UNKNOWN            3 /* not used */
#define SWS_RPC_BINDNAK_PROTOCOL_VERSION_NOT_SUPPORTED  4
#define SWS_RPC_BINDNAK_DEFAULT_CONTEXT_NOT_SUPPORTED   5 /* not used */
#define SWS_RPC_BINDNAK_USER_DATA_NOT_READABLE          6 /* not used */
#define SWS_RPC_BINDNAK_NO_PSAP_AVAILABLE               7 /* not used */

/* Transfer syntax identifiers. This one's for standard NDR described in DCE v1.1 RPC */
#define SWS_RPC_GUID_NDR {0x04,0x5d,0x88,0x8a,0xeb,0x1c,0xc9,0x11,0x9f,0xe8,0x08,0x00,0x2b,0x10,0x48,0x60}
#define SWS_RPC_GUID_NDR_VER 2U


/*
 * Accessors
 */
#define SWS_RPC_GET_BYTE( _v, _p, _n) (_v = ((const unsigned char*)(_p))[_n])
#define SWS_RPC_SET_BYTE( _p, _n, _v) (((unsigned char*)(_p))[_n] = _v)
#define SWS_RPC_GET_WORD( _v, _p, _n) (_v = (unsigned short)(\
  ((const unsigned char*)(_p))[ _n] \
  + 256U * ((const unsigned char*)(_p))[ 1 + (_n)] \
))
#define SWS_RPC_SET_WORD( _p, _n, _v) (\
  ((unsigned char*)(_p))[ _n] = (unsigned char)((_v) % 256U), \
  ((unsigned char*)(_p))[ 1 + (_n)] = (unsigned char)((_v) / 256U) \
)
#define SWS_RPC_GET_DWORD( _v, _p, _n) (_v = (unsigned long)(\
  ((const unsigned char*)(_p))[ _n] \
  + 256U * ((const unsigned char*)(_p))[ 1 + (_n)] \
  + 0x10000UL * ((const unsigned char*)(_p))[ 2 + (_n)] \
  + 0x1000000UL * ((const unsigned char*)(_p))[ 3 + (_n)] \
))
#define SWS_RPC_SET_DWORD( _p, _n, _v) (\
  ((unsigned char*)(_p))[ _n] = (unsigned char)((_v) % 256U), \
  ((unsigned char*)(_p))[ 1 + (_n)] = (unsigned char)(0xff & ((_v) / 256U)), \
  ((unsigned char*)(_p))[ 2 + (_n)] = (unsigned char)(0xff & ((_v) / 0x10000UL)), \
  ((unsigned char*)(_p))[ 3 + (_n)] = (unsigned char)(0xff & ((_v) / 0x1000000UL)) \
)

/* Roundup a value */
#define SWS_RPC_ROUNDUP( _x,_z) ((size_t)((_x) + ((-(ptrdiff_t)(_x)) & ((_z) -1)) ))


/*
 * Types
 */

/* UUID's are 16 bytes long.  Field offsets are: */
#define SWS_RPC_UUID_TLOW   0 /* 00:04 Low field of timestamp */
#define SWS_RPC_UUID_TMID   4 /* 04:02 Mid field of timestamp */
#define SWS_RPC_UUID_THI    6 /* 06:02 Hi field of timestamp and version (MS4 bits) */
#define SWS_RPC_UUID_CLKHI  8 /* 08:01 The high field of the clock sequence multiplexed with the variant (MS2 bits) */
#define SWS_RPC_UUID_CLKLO  9 /* 09:01 The low field of the clock sequence */
#define SWS_RPC_UUID_NODE  10 /* 10:06 The spatially unique node identifier */
#define SWS_RPC_UUID_LEN   16

/* Presentation context element field offsets: */
#define SWS_RPC_PCE_ID         0 /* 00:02 ID  */
#define SWS_RPC_PCE_COUNT      2 /* 02:01 Item count  */
#define SWS_RPC_PCE_ASN        4 /* 04:20 Abstract syntax */
#define SWS_RPC_PCE_SYNLIST   (4 + SWS_RPC_SYN_LEN) /* 24:xx Array of transfer syntaxes */

/* Presentation syntax field offsets: */
#define SWS_RPC_SYN_UUID  0                /* 00:16 UUID */
#define SWS_RPC_SYN_VER   SWS_RPC_UUID_LEN /* 16:04 Version */
#define SWS_RPC_SYN_LEN   (SWS_RPC_UUID_LEN + 4)

/* Presentation result list field offsets: */
#define SWS_RPC_PRL_COUNT  0  /* 00:01 */
#define SWS_RPC_PRL_ARRAY  4  /* 04:xx presentation result element array */

/* Presentation result element field offsets: */
#define SWS_RPC_PR_RESULT  0  /* 00:02 */
#define SWS_RPC_PR_REASON  2  /* 02:02 */
#define SWS_RPC_PR_TFR_SYN 4  /* 04:16 UUID of trasnsfer syntax */


/*
 * Connectionless RPC PDUs
 * http://www.opengroup.org/onlinepubs/009629399/chap12.htm#tagcjh_17_05
 */

/* PDU offsets */
#define SWS_RPC_CL_VER          0 /* 00:01 4LSB RPC version =SWS_RPC_CL_VERSION */
#define SWS_RPC_CL_PTYPE        1 /* 01:01 5LSB packet type */
#define SWS_RPC_CL_FLAGS1       2 /* 02:01 flags */
#define SWS_RPC_CL_FLAGS2       3 /* 03:01 flags */
#define SWS_RPC_CL_DREP         4 /* 04:03 NDR data representation format label */
#define SWS_RPC_CL_SERIALH      7 /* 07:01 high byte of serial number */
#define SWS_RPC_CL_OBJECT       8 /* 08:16 object identifier (Contains the IPID) */
#define SWS_RPC_CL_IFID        24 /* 24:16 interface identifier (IID) */
#define SWS_RPC_CL_ACTID       40 /* 40:16 activity identifier */
#define SWS_RPC_CL_BOOT        56 /* 56:04 server boot time */
#define SWS_RPC_CL_IFVERS      60 /* 60:04 interface version */
#define SWS_RPC_CL_SEQNUM      64 /* 60:04 sequence number */
#define SWS_RPC_CL_OPNUM       68 /* 68:02 operation number */
#define SWS_RPC_CL_IHINT       70 /* 70:02 interface hint */
#define SWS_RPC_CL_AHINT       72 /* 72:02 activity hint */
#define SWS_RPC_CL_DLEN        74 /* 74:02 length of packet body */
#define SWS_RPC_CL_FRAGNUM     76 /* 76:02 fragment number */
#define SWS_RPC_CL_AUTHID      78 /* 78:01 authentication protocol id */
#define SWS_RPC_CL_SERIALL     79 /* 79:01 low byte of serial number */
#define SWS_RPC_CL_HDRLEN      80

#define SWS_RPC_CL_VERSION 4U

/* FLAG1 bits */
#define SWS_RPC_CL_FLAG1_RESERVED_01     0x01 /* Reserved for implementation */
#define SWS_RPC_CL_FLAG1_LASTFRAG        0x02 /* Last in multi PDU transmission */
#define SWS_RPC_CL_FLAG1_FRAG            0x04 /* Part of a multi PDU transmission */
#define SWS_RPC_CL_FLAG1_NOFACK          0x08
#define SWS_RPC_CL_FLAG1_MAYBE           0x10
#define SWS_RPC_CL_FLAG1_IDEMPOTENT      0x20
#define SWS_RPC_CL_FLAG1_BROADCAST       0x40
#define SWS_RPC_CL_FLAG1_RESERVED_80     0x80 /* Reserved for implementation */

/* FLAG2 bits */
#define SWS_RPC_CL_FLAG2_RESERVED_01     0x01 /* Reserved for implementation */
#define SWS_RPC_CL_FLAG2_CANCEL_PENDING  0x02
#define SWS_RPC_CL_FLAG2_RESERVED_04     0x04 /* Reserved for future */
#define SWS_RPC_CL_FLAG2_RESERVED_08     0x08 /* Reserved for future */
#define SWS_RPC_CL_FLAG2_RESERVED_10     0x10 /* Reserved for future */
#define SWS_RPC_CL_FLAG2_RESERVED_20     0x20 /* Reserved for future */
#define SWS_RPC_CL_FLAG2_RESERVED_40     0x40 /* Reserved for future */
#define SWS_RPC_CL_FLAG2_RESERVED_80     0x80 /* Reserved for future */

/*** Request PDU offsets */
#define SWS_RPC_CL_REQ_DATA SWS_RPC_CL_HDRLEN


/*
 * Connection-oriented RPC PDUs
 * http://www.opengroup.org/onlinepubs/009629399/chap12.htm#tagcjh_17_06
 *
 * The common header fields, which appear in all PDU types, are as follows.
 * The comment fields show the exact octet alignment and octet length of
 * each element.
 */

/* PDU offsets */
#define SWS_RPC_CO_VER_MAJOR  0 /* 00:01 RPC version =SWS_RPC_CO_VER_MAJOR */
#define SWS_RPC_CO_VER_MINOR  1 /* 01:01 minor version =SWS_RPC_CO_VER_MINOR */
#define SWS_RPC_CO_PTYPE      2 /* 02:01 packet type */
#define SWS_RPC_CO_PFC_FLAGS  3 /* 03:01 flags (see PFC_... ) */
#define SWS_RPC_CO_PDATA      4 /* 04:04 NDR data representation format label */
#define SWS_RPC_CO_FRAG_LEN   8 /* 08:02 total length of fragment */
#define SWS_RPC_CO_AUTH_LEN  10 /* 10:02 length of auth_value */
#define SWS_RPC_CO_CALL_ID   12 /* 12:04 call identifier */
#define SWS_RPC_CO_HDR_LEN   16

#define SWS_RPC_CO_VMAJOR 5U
#define SWS_RPC_CO_VMINOR 0

/* PFC flags */
#define SWS_RPC_CO_PFC_FIRST_FRAG        0x01 /* First fragment */
#define SWS_RPC_CO_PFC_LAST_FRAG         0x02 /* Last fragment */
#define SWS_RPC_CO_PFC_PENDING_CANCEL    0x04 /* Cancel was pending at sender */
#define SWS_RPC_CO_PFC_RESERVED_1        0x08
#define SWS_RPC_CO_PFC_CONC_MPX          0x10 /* supports concurrent multiplexing
                                            * of a single connection. */
#define SWS_RPC_CO_PFC_DID_NOT_EXECUTE   0x20 /* only meaningful on `fault' packet;
                                            * if true, guaranteed call did not
                                            * execute. */
#define SWS_RPC_CO_PFC_MAYBE             0x40 /* `maybe' call semantics requested */
#define SWS_RPC_CO_PFC_OBJECT_UUID       0x80 /* if true, a non-nil object UUID
                                            * was specified in the handle, and
                                            * is present in the optional object
                                            * field. If false, the object field
                                            * is omitted. */
/*** Bind PDU offsets */
#define SWS_RPC_CO_BIND_MAX_XMIT      SWS_RPC_CO_HDR_LEN  /* 16:02 max transmit frag size, bytes */
#define SWS_RPC_CO_BIND_MAX_RECV (2 + SWS_RPC_CO_HDR_LEN) /* 18:02 max receive frag size, bytes */
#define SWS_RPC_CO_BIND_GROUP_ID (4 + SWS_RPC_CO_HDR_LEN) /* 20:04 incarnation of client-server assoc group */
#define SWS_RPC_CO_BIND_PCOUNT   (8 + SWS_RPC_CO_HDR_LEN) /* 24:01 presentation context list element count */
#define SWS_RPC_CO_BIND_PLIST    (12 + SWS_RPC_CO_HDR_LEN)/* 28:xx presentation context list */
                                                    /* xx:xx optional authentication verifier iff auth_length != 0 */
/*** BindAck PDU offsets */
#define SWS_RPC_CO_BINDACK_MAX_XMIT      SWS_RPC_CO_HDR_LEN  /* 16:02 max transmit frag size, bytes */
#define SWS_RPC_CO_BINDACK_MAX_RECV (2 + SWS_RPC_CO_HDR_LEN) /* 18:02 max receive frag size, bytes */
#define SWS_RPC_CO_BINDACK_GROUP_ID (4 + SWS_RPC_CO_HDR_LEN) /* 20:04 incarnation of client-server assoc group */
#define SWS_RPC_CO_BINDACK_ADDR2    (8 + SWS_RPC_CO_HDR_LEN) /* 24:xx optional secondary address */
                                                       /* xx:xx presentation context result list */
                                                       /* xx:xx optional authentication verifier iff auth_length != 0 */
/*** BindNak PDU offsets */
#define SWS_RPC_CO_BINDNAK_REASON        SWS_RPC_CO_HDR_LEN  /* 16:02 reject reason code */
#define SWS_RPC_CO_BINDNAK_VERSIONS (2 + SWS_RPC_CO_HDR_LEN  /* 18:xx array of protocols supported */

/*** Request PDU offsets */
#define SWS_RPC_CO_REQ_ALLOC          SWS_RPC_CO_HDR_LEN  /* 16:04 allocation hint */
#define SWS_RPC_CO_REQ_CTX_ID    (4 + SWS_RPC_CO_HDR_LEN) /* 20:02 pres context, i.e. data rep */
#define SWS_RPC_CO_REQ_OPNUM     (6 + SWS_RPC_CO_HDR_LEN) /* 22:02 operation */
#define SWS_RPC_CO_REQ_OBJECT    (8 + SWS_RPC_CO_HDR_LEN) /* 24:16 optional object UID iff PFC_OBJECT_UUID */
#define SWS_RPC_CO_REQ_DATA      (8 + SWS_RPC_CO_HDR_LEN) /* Stub data */
                                                    /* xx:xx optional authentication verifier iff auth_length != 0 */
/*** Response PDU offsets */
#define SWS_RPC_CO_RESP_ALLOC         SWS_RPC_CO_HDR_LEN  /* 16:04 allocation hint */
#define SWS_RPC_CO_RESP_CTX_ID   (4 + SWS_RPC_CO_HDR_LEN) /* 20:02 pres context, i.e. data rep */
#define SWS_RPC_CO_RESP_CANCELS  (6 + SWS_RPC_CO_HDR_LEN) /* 22:01 council count */
#define SWS_RPC_CO_RESP_DATA     (8 + SWS_RPC_CO_HDR_LEN) /* Stub data */
                                                    /* xx:xx optional authentication verifier iff auth_length != 0 */

/*
 * M$ DCOM headers
 */

/*** ORPCTHIS */
#define SWS_RPC_ORPCTHIS_VERSION      0 /* 00:04 Version major lo, minor hi */
#define SWS_RPC_ORPCTHIS_FLAGS        4 /* 04:04 SWS_RPC_ORPCF_xxx flags */
#define SWS_RPC_ORPCTHIS_RSRVD        8 /* 08:04 MBZ */
#define SWS_RPC_ORPCTHIS_CID         12 /* 12:16 Causality ID */
#define SWS_RPC_ORPCTHIS_EXTENTS     28 /* 28:04 Unique ptr to EXT_COUNT */

#define SWS_RPC_ORPC_EXTARRAY_COUNT  32 /* 32:04 Actual no. extents */
#define SWS_RPC_ORPC_EXTARRAY_RSRVD  36 /* 36:04 reserved */
#define SWS_RPC_ORPC_EXTARRAY_TAGS   40 /* 40:04*n ROUNDUP(EXT_COUNT,2) unique ptrs to ORPC_EXTENTS */

#define SWS_RPC_ORPC_EXTENT_GUID      0 /* 00:16 Extension GUID */
#define SWS_RPC_ORPC_EXTENT_SIZE     16 /* 16:04 Extension size */
#define SWS_RPC_ORPC_EXTENT_DATA     20 /* 20:xx data rounded to 8-byte boundary */

#define SWS_RPC_DCOM_VER_MAJOR  5U
#define SWS_RPC_DCOM_VER_MINOR  6U
#define SWS_RPC_DCOM_VER_W95    (SWS_RPC_DCOM_VER_MAJOR | (1UL<<16)) /* Win95 and NT4 pre SP3 */
#define SWS_RPC_DCOM_VER_NT4SP3 (SWS_RPC_DCOM_VER_MAJOR | (3UL<<16)) /* NT4SP3 and Win95 OSR1 */
#define SWS_RPC_DCOM_VER_W2K    (SWS_RPC_DCOM_VER_MAJOR | (6UL<<16)) /* Win2000, XP */

#define SWS_RPC_ORPCF_NULL  0
#define SWS_RPC_ORPCF_LOCAL 1U

/*** ORPCTHAT */
#define SWS_RPC_ORPCTHAT_FLAGS     0 /* 00:04 flags */
#define SWS_RPC_ORPCTHAT_UNIQUE    4 /* 04:04 Unique ptr to EXT_COUNT */

/*** OBJREF */
#define SWS_RPC_OBJREF_SIG         0 /* 00:04 'MEOW' */
#define SWS_RPC_OBJREF_FLAGS       4 /* 04:04 flags, SWS_RPC_OBJREFF_xxx */
#define SWS_RPC_OBJREF_IID         8 /* 08:16 GUID */
#define SWS_RPC_OBJREF_VARIANT    24 /* 24:xx */
/* Variant part */
/* standard: STDOBJREF, DUALSTRINGARRAY */
/* handler: STDOBJREF, CLSID of handler, DUALSTRINGARRAY */
/* custom: CLSID of unmarshaller, cbExten:4, size:4, data[size] */

#define SWS_RPC_OBJREF_SIGNATURE 0x574f454d /* 'MEOW' */
#define SWS_RPC_OBJREFF_STD       1
#define SWS_RPC_OBJREFF_HANDLER   2
#define SWS_RPC_OBJREFF_CUSTOM    4

/*** STDOBJREF */
#define SWS_RPC_STDOBJREF_FLAGS  0 /* 00:04 */
#define SWS_RPC_STDOBJREF_REFS   4 /* 04:04 */
#define SWS_RPC_STDOBJREF_OXID   8 /* 08:08 oxid of server with this oid */
#define SWS_RPC_STDOBJREF_OID   16 /* 16:08 oid of object with this ipid */
#define SWS_RPC_STDOBJREF_IPID  24 /* 24:16 ipid of Interface */

#endif /* ndef SWS_RPC_H */
/* End of file */
