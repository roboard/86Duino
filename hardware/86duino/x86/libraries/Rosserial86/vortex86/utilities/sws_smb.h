/****************************************************************************
 *
 * File name   : sws_smb.h
 * Function    : SMB/CIFS definitions
 * Authors     : Lawrence Rust
 * Systems     : ANSI C
 * Version     : 1.0
 *
 ****************************************************************************
 * File History
 * ------------
 * 09-Dec-02, LVR, v1.00 - Created.
 *
 * Comments:
 * --------
 * This file contains common SMB/CIFS definitions
 *
 ****************************************************************************/

#ifndef SWS_SMB_H
#define SWS_SMB_H 0x0100 /* Version no. BCD */

/* Dependencies */


/* Constants */

#define SWS_NBSS 139 /* NetBIOS session server (TCP) */
#define SWS_CIFS 445 /* CIFS/SMB server (TCP) */

/* SESSION MESSAGE PACKET from RFC1002 p32

                        1 1 1 1 1 1 1 1 1 1 2 2 2 2 2 2 2 2 2 2 3 3
    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |      TYPE     |     FLAGS     |            LENGTH             |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |                                                               |
   /                                                               /
   /                           USER_DATA                           /
   /                                                               /
   |                                                               |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*/

/* Session Packet Types from RFC1002 p30 */

#define SWS_NBSS_MESSAGE    0x00
#define SWS_NBSS_REQUEST    0x81
#define SWS_NBSS_ACK        0x82
#define SWS_NBSS_NAK        0x83
#define SWS_NBSS_RETARGET   0x84
#define SWS_NBSS_KEEP_ALIVE 0x85

/* SMB header from ftp://ftp.microsoft.com/developr/drg/CIFS/COREP.TXT
 * NB all data in SMB header is accessed little-endian
 */
/* SMB offsets */
#define SWS_SMB_IDF        0 /* contains 0xFF,'SMB' */
#define SWS_SMB_COM        4 /* command code */
#define SWS_SMB_RCLS       5 /* error class */
#define SWS_SMB_REH        6 /* reserved for future */
#define SWS_SMB_ERR        7 /* error code */
#define SWS_SMB_FLG        9 /* flags */
#define SWS_SMB_FLG2      10 /* flags */
#define SWS_SMB_RES       12 /* reserved for future */
#define SWS_SMB_TID       24 /* authenticated resource identifier */
#define SWS_SMB_PID       26 /* caller's process id */
#define SWS_SMB_UID       28 /* authenticated user id */
#define SWS_SMB_MID       30 /* multiplex id */
#define SWS_SMB_WCT       32 /* count of 16-bit words that follow */
#define SWS_SMB_VWV       33 /* variable number of 16-bit words */
#define SWS_SMB_WRD( _n) (SWS_SMB_VWV + 2* (_n))
#define SWS_SMB_BYT( _n) (SWS_SMB_VWV + (_n))
#define SWS_SMB_BCC( _p) (SWS_SMB_VWV + 2* SWS_SMB_GET_BYTE( _p, SWS_SMB_WCT)) /* count of bytes that follow */
#define SWS_SMB_BUF( _p) (SWS_SMB_BCC( _p) + 2) /* variable number of bytes */
#define SWS_SMB_LEN( _p) (SWS_SMB_BUF( _p) + SWS_SMB_GET_WORD( _p, SWS_SMB_BCC( _p)))

/* FLG bits */
#define SWS_SMB_FLG_LOCK_READ         (1U<<0) /* Lock/Read and Write/Unlock support */
#define SWS_SMB_FLG_SEND_NO_ACK       (1U<<1)
#define SWS_SMB_FLG_RESERVED1         (1U<<2)
#define SWS_SMB_FLG_CASE_INSENSITIVE  (1U<<3)
#define SWS_SMB_FLG_CANNONICAL        (1U<<4)
#define SWS_SMB_FLG_OPLOCK            (1U<<5)
#define SWS_SMB_FLG_NOTIFY            (1U<<6)
#define SWS_SMB_FLG_REPLY             (1U<<7) /* Set in server -> client reply */

/* FLG2 bits */
#define SWS_SMB_FLG2_LONG_NAMES       (1U<< 0)
#define SWS_SMB_FLG2_EXT_ATTRIBS      (1U<< 2)
#define SWS_SMB_FLG2_DFS              (1U<<12)
#define SWS_SMB_FLG2_PAGING           (1U<<13)
#define SWS_SMB_FLG2_NTSTATUS         (1U<<14)
#define SWS_SMB_FLG2_UNICODE          (1U<<15)

#define SWS_SMB_ISVALID( _p) (\
  (0xff == SWS_SMB_GET_BYTE( _p,       SWS_SMB_IDF) )\
  && ('S' == SWS_SMB_GET_BYTE( _p, 1 + SWS_SMB_IDF) )\
  && ('M' == SWS_SMB_GET_BYTE( _p, 2 + SWS_SMB_IDF) )\
  && ('B' == SWS_SMB_GET_BYTE( _p, 3 + SWS_SMB_IDF) )\
)

/* Accessors */
#define SWS_SMB_GET_BYTE( _p, _n) ((const unsigned char*)(_p))[_n]
#define SWS_SMB_SET_BYTE( _p, _n, _v) (((unsigned char*)(_p))[_n] = (unsigned char)(_v))
#define SWS_SMB_GET_WORD( _p, _n) (unsigned short)(\
            SWS_SMB_GET_BYTE( _p,  _n) \
  + 0x100 * SWS_SMB_GET_BYTE( _p, (_n) + 1) \
)
#define SWS_SMB_SET_WORD( _p, _n, _v) (\
  SWS_SMB_SET_BYTE( _p,  _n,      (_v) % 0x100), \
  SWS_SMB_SET_BYTE( _p, (_n) + 1, (_v) / 0x100) \
)
#define SWS_SMB_GET_DWORD( _p, _n) (\
                  SWS_SMB_GET_BYTE( _p,      _n ) \
  + 0x100       * SWS_SMB_GET_BYTE( _p, 1 + (_n)) \
  + 0x10000UL   * SWS_SMB_GET_BYTE( _p, 2 + (_n)) \
  + 0x1000000UL * SWS_SMB_GET_BYTE( _p, 3 + (_n)) \
)
#define SWS_SMB_SET_DWORD( _p, _n, _v) (\
  SWS_SMB_SET_BYTE( _p,  _n,      (_v) % 0x100), \
  SWS_SMB_SET_BYTE( _p, (_n) + 1, 0xff & ((_v) / 0x100)), \
  SWS_SMB_SET_BYTE( _p, (_n) + 2, 0xff & ((_v) / 0x10000UL)), \
  SWS_SMB_SET_BYTE( _p, (_n) + 3, (_v) / 0x1000000UL) \
)

/* Get/set word parameter */
#define SWS_SMB_GET_WPARAM( _v, _p, _n)\
  (_v = SWS_SMB_GET_WORD( _p, SWS_SMB_WRD(_n)))
#define SWS_SMB_SET_WPARAM( _p, _n, _v)\
  SWS_SMB_SET_WORD( _p, SWS_SMB_WRD(_n), _v)

/* Protocol levels */
#define SWS_SMB_PROTOCOL_CORE  0 /* Core */
#define SWS_SMB_PROTOCOL_COREP 1 /* Core+ */
#define SWS_SMB_PROTOCOL_LM10  2 /* Extended 1.0 */
#define SWS_SMB_PROTOCOL_MSNW  3 /* Extended 1.0 */
#define SWS_SMB_PROTOCOL_LM12  4 /* Extended 2.0 */
#define SWS_SMB_PROTOCOL_LM21  5
#define SWS_SMB_PROTOCOL_NTLM012 6

#ifndef SWS_SMB_PROTOCOL
#define SWS_SMB_PROTOCOL SWS_SMB_PROTOCOL_NTLM012
#endif

/* Core commands from ftp://ftp.microsoft.com/developr/drg/CIFS/SMB.TXT p66 */
#define SWS_SMBmkdir      0x00   /* create directory */
#define SWS_SMBrmdir      0x01   /* delete directory */
#define SWS_SMBopen       0x02   /* open file */
#define SWS_SMBcreate     0x03   /* create file */
#define SWS_SMBclose      0x04   /* close file */
#define SWS_SMBflush      0x05   /* flush file */
#define SWS_SMBunlink     0x06   /* delete file */
#define SWS_SMBrename     0x07   /* rename file */
#define SWS_SMBgetatr     0x08   /* get file attributes */
#define SWS_SMBsetatr     0x09   /* set file attributes */
#define SWS_SMBread       0x0A   /* read from file */
#define SWS_SMBwrite      0x0B   /* write to file */
#define SWS_SMBlock       0x0C   /* lock byte range */
#define SWS_SMBunlock     0x0D   /* unlock byte range */
#define SWS_SMBctemp      0x0E   /* create temporary file (reserved) */
#define SWS_SMBmknew      0x0F   /* make new file */
#define SWS_SMBchkpth     0x10   /* check directory path */
#define SWS_SMBexit       0x11   /* process exit */
#define SWS_SMBlseek      0x12   /* seek */

#define SWS_SMBtcon       0x70   /* tree connect */
#define SWS_SMBtdis       0x71   /* tree disconnect */
#define SWS_SMBnegprot    0x72   /* negotiate protocol */

#define SWS_SMBdskattr    0x80   /* get disk attributes */
#define SWS_SMBsearch     0x81   /* search directory */

#define SWS_SMBsplopen    0xC0   /* open print spool file */
#define SWS_SMBsplwr      0xC1   /* write to print spool file */
#define SWS_SMBsplclose   0xC2   /* close print spool file */
#define SWS_SMBsplretq    0xC3   /* return print queue */

#define SWS_SMBsends      0xD0   /* send single block message */
#define SWS_SMBsendb      0xD1   /* send broadcast message */
#define SWS_SMBfwdname    0xD2   /* forward user name */
#define SWS_SMBcancelf    0xD3   /* cancel forward */
#define SWS_SMBgetmac     0xD4   /* get machine name */
#define SWS_SMBsendstrt   0xD5   /* send start of multi-block message */
#define SWS_SMBsendend    0xD6   /* send end of multi-block message */
#define SWS_SMBsendtxt    0xD7   /* send text of multi-block message */

/* MICROSOFT NETWORKS 1.03 core+ protocol requests */
#if SWS_SMB_PROTOCOL >= SWS_SMB_PROTOCOL_COREP
#define SWS_SMBlockread      0x13   /* lock then read data */
#define SWS_SMBwriteunlock   0x14   /* write then unlock data */
#define SWS_SMBreadBraw      0x1A   /* read block raw */
#define SWS_SMBwriteBraw     0x1D   /* write block raw */
#endif /* SWS_SMB_PROTOCOL >= SWS_SMB_PROTOCOL_COREP */

/* MICROSOFT NETWORKS 3.0 & LANMAN 1.0 extended protocol requests */
#if SWS_SMB_PROTOCOL >= SWS_SMB_PROTOCOL_LM10
#define SWS_SMBreadBmpx      0x1B   /* read block multiplexed */
#define SWS_SMBreadBs        0x1C   /* read block (secondary response) */
#define SWS_SMBwriteBmpx     0x1E   /* write block multiplexed */
#define SWS_SMBwriteBs       0x1F   /* write block (secondary request) */
#define SWS_SMBwriteC        0x20   /* write complete response */
                                    /* 0x21 reserved */
#define SWS_SMBsetattrE      0x22   /* set file attributes expanded */
#define SWS_SMBgetattrE      0x23   /* get file attributes expanded */
#define SWS_SMBlockingX      0x24   /* lock/unlock byte ranges and X */
#define SWS_SMBtrans         0x25   /* transaction - name, bytes in/out */
#define SWS_SMBtranss        0x26   /* transaction (secondary request/response) */
#define SWS_SMBioctl         0x27   /* IOCTL */
#define SWS_SMBioctls        0x28   /* IOCTL  (secondary request/response) */
#define SWS_SMBcopy          0x29   /* copy */
#define SWS_SMBmove          0x2A   /* move */
#define SWS_SMBecho          0x2B   /* echo */
#define SWS_SMBwriteclose    0x2C   /* Write and Close */
#define SWS_SMBopenX         0x2D   /* open and X */
#define SWS_SMBreadX         0x2E   /* read and X */
#define SWS_SMBwriteX        0x2F   /* write and X */

#define SWS_SMBsesssetupX    0x73   /* Session Set Up & X (including User Logon) */
#define SWS_SMBtconX         0x75   /* tree connect and X */

#define SWS_SMBffirst        0x82   /* find first */
#define SWS_SMBfunique       0x83   /* find unique */
#define SWS_SMBfclose        0x84   /* find close */
#define SWS_SMBinvalid       0xFE   /* invalid command */
#define SWS_SMBnoX           0xFF   /* No following AndX command */

/* trans subcommands */
#define SWS_SMB_TRANS_SetNmPHandState 0x01
#define SWS_SMB_TRANS_RawReadNmPipe   0x11
#define SWS_SMB_TRANS_QNmPHandState   0x21
#define SWS_SMB_TRANS_QNmPipeInfo     0x22
#define SWS_SMB_TRANS_PeekNmPipe      0x23
#define SWS_SMB_TRANS_TransactNmPipe  0x26
#define SWS_SMB_TRANS_RawWriteNmPipe  0x31
#define SWS_SMB_TRANS_WaitNamedPipe   0x53
#define SWS_SMB_TRANS_CallNamedPipe   0x54

/* RAP commands. See: Protocols for X/Open PC Interworking: SMB, Version 2 p275 */
#define SWS_SMB_RAP_NetShareEnum      0
#define SWS_SMB_RAP_NetServerGetInfo 13
#define SWS_SMB_RAP_NetUserEnum      53
#define SWS_SMB_RAP_NetUserGetInfo   56
#define SWS_SMB_RAP_NetServerEnum2  104
#endif /* SWS_SMB_PROTOCOL >= SWS_SMB_PROTOCOL_LM10 */

#if SWS_SMB_PROTOCOL >= SWS_SMB_PROTOCOL_LM12
#define SWS_SMBtrans2        0x32   /* transaction2 */
#define SWS_SMBtrans2s       0x33   /* transaction2 (secondary request/response) */
#define SWS_SMBfclose2       0x34   /* find close */
#define SWS_SMBsesslogoffX   0x74   /* Session Logoff & X */

/* trans2 subcommands */
#define SWS_SMB_TRANS2_OPEN2                  0x00
#define SWS_SMB_TRANS2_FIND_FIRST2            0x01
#define SWS_SMB_TRANS2_FIND_NEXT2             0x02
#define SWS_SMB_TRANS2_QUERY_FS_INFORMATION   0x03
#define SWS_SMB_TRANS2_QUERY_PATH_INFORMATION 0x05
#define SWS_SMB_TRANS2_SET_PATH_INFORMATION   0x06
#define SWS_SMB_TRANS2_QUERY_FILE_INFORMATION 0x07
#define SWS_SMB_TRANS2_SET_FILE_INFORMATION   0x08
#define SWS_SMB_TRANS2_FSCTL                  0x09
#define SWS_SMB_TRANS2_IOCTL2                 0x0a
#define SWS_SMB_TRANS2_NOTIFY_FIRST           0x0b
#define SWS_SMB_TRANS2_FIND_NOTIFY_NEXT       0x0c
#define SWS_SMB_TRANS2_CREATE_DIRECTORY       0x0d
#define SWS_SMB_TRANS2_SESSION_SETUP          0x0e
#endif /* SWS_SMB_PROTOCOL >= SWS_SMB_PROTOCOL_LM12 */

/* NT LAN manager 0.12 */
#if SWS_SMB_PROTOCOL >= SWS_SMB_PROTOCOL_NTLM012
#define SWS_SMBntcreateX     0xA2   /* NT create and X */
#endif

/* Data buffer formats */
#define SWS_SMB_FORMAT_DATA    0x01 /* Followed by 16-bit byte count */
#define SWS_SMB_FORMAT_DIALECT 0x02 /* ASCIIZ */
#define SWS_SMB_FORMAT_PATH    0x03 /* ASCIIZ */
#define SWS_SMB_FORMAT_ASCII   0x04 /* ASCIIZ */
#define SWS_SMB_FORMAT_VAR     0x05 /* ? */

/* Error classes from smbpub.doc p102 */
#define SWS_SMB_RCLS_SUCCESS 0x00 /* The request was successful */
#define SWS_SMB_RCLS_ERRDOS  0x01 /* Error from DOS */
#define SWS_SMB_RCLS_ERRSRV  0x02 /* Error from server network file manager */
#define SWS_SMB_RCLS_ERRHRD  0x03 /* hardware error */
#define SWS_SMB_RCLS_ERRXOS  0x04 /* reserved */
#define SWS_SMB_RCLS_ERRMX1  0xe1 /* reserved */
#define SWS_SMB_RCLS_ERRMX2  0xe2 /* reserved */
#define SWS_SMB_RCLS_ERRMX3  0xe3 /* reserved */
#define SWS_SMB_RCLS_ERRCMD  0xff /* Command was not in the "SMB" format */

/* Error codes from smbpub.doc p102 */
/* SWS_SMB_RCLS_SUCCESS codes */
#define SWS_SMB_SUCCESS         0 /* The request was successful */
#define SWS_SMB_BUFFERED     0x54 /* Messaging */
#define SWS_SMB_LOGGED       0x55 /* Messaging */
#define SWS_SMB_DISPLAYED    0x56 /* Messaging */

/* SWS_SMB_RCLS_ERRDOS codes */
#define SWS_SMB_ERRDOS_BADFUNC  0x0001 /* Invalid function */
#define SWS_SMB_ERRDOS_BADFILE  0x0002 /* File not found */
#define SWS_SMB_ERRDOS_BADPATH  0x0003 /* Directory not found */
#define SWS_SMB_ERRDOS_NOFIDX   0x0004 /* Too many open files */
#define SWS_SMB_ERRDOS_ACCESS   0x0005 /* Access denied */

/* SWS_SMB_RCLS_ERRSRV codes */
#define SWS_SMB_ERRSRV_ERR      0x0001 /* Non-specific error code */
#define SWS_SMB_ERRSRV_BADPW    0x0002 /* Bad password */
#define SWS_SMB_ERRSRV_ACCESS   0x0004 /* The client does not have the necessary access rights */
#define SWS_SMB_ERRSRV_NOSUPPORT 0xffff /* Function not supported */

/* SWS_SMB_RCLS_ERRHRD codes */
#define SWS_SMB_ERRHRD_NOWRITE  0x0001 /* Attempt to write on write-protected media */
#define SWS_SMB_ERRHRD_GENERAL  0x001f /* General failure */

#endif /* ndef SWS_SMB_H */
/* End of file */
