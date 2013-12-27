/****************************************************************************
 *
 * File name   : sws_dlnk.h
 * Function    : IP data link providers public interface
 * Project     : SwsSock
 * Authors     : Lawrence Rust
 * Systems     : ANSI C
 * Version     : 1.0
 * Release Date:
 *
 ****************************************************************************
 * File History
 * ------------
 * 09-Apr-01, LVR, v1.00 - Created.
 * 06-Nov-02, LVR, v1.01 - Added.SWS_kDatalinkProperty_Promiscuity
 * 24-Nov-02, LVR, v1.02 - Added.SWS_kDatalinkProperty_HwType/Address
 * 24-Dec-02, LVR, v1.03 - Added.SWS_FDatalinkSetMulticast
 * 29-Jun-05, LVR, v1.04 - Added.SWS_kDatalinkProperty_DHCP
 * 02-Aug-05, LVR, v1.05 - Added.SWS_FDatalinkStatus to SWS_DatalinkCallbacks
 *
 *
 * Comments:
 * --------
 * This file defines the public interface to data link providers (ethernet, PPP etc.)
 *
 ****************************************************************************/

#ifndef SWS_DLNK_H
#define SWS_DLNK_H 0x0105 /* Version no. BCD */

/* Dependencies */
#include <stddef.h>   /* size_t */
#include "sws_host.h" /* Basic types */
#include "sws_buf.h"


/* Get/set property types */
enum
  {
  SWS_kDatalinkProperty_Address = 1,  /* ui32 IP address */
  SWS_kDatalinkProperty_SubnetMask,   /* ui32 subnet mask */
  SWS_kDatalinkProperty_Gateway,      /* ui32 IP address of gateway */
  SWS_kDatalinkProperty_MTU,          /* size_t Max Transmit Unit */
  SWS_kDatalinkProperty_Ident,        /* SWS_PTR(char) for SWS_DatalinkFind */
  SWS_kDatalinkProperty_Promiscuity,  /* ui32 (w/o) !0 to receive all packets */
  SWS_kDatalinkProperty_HwType,       /* ui8 Hardware type (r/o) - RFC 1700 p163 */
  SWS_kDatalinkProperty_HwAddress,    /* Hardware address (r/o) eg 6 byte MAC address */
  SWS_kDatalinkProperty_DHCP,         /* ui8 (r/o) !0 to use DHCP */
  /* Add new properties above */

  /* Datalink specific properties */
  SWS_kDatalinkProperty_Private = 0x4000,
  SWS_kDatalinkProperty_PrivateMax = 0x7fff
  };


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Down calls */

/* Transmit a buffer, maybe fragmented */
typedef SWS_CB( int, SWS_FDatalinkSend( /* Returns !0 on error, -1 if pending */
  SWS_PTR( void),                     /* IN: Client context */
  SWS_BufferH,                        /* IN: Packet to send */
  ui32,                               /* IN: Destination IP address */
  unsigned                            /* IN: TOS and precedence */
));
/* Get/set interface properties */
typedef SWS_CB( int, SWS_FDatalinkGetProperty(
  SWS_PTR( void),                     /* IN: Client context */
  int,                                /* IN: Property, SWS_kDatalinkProperty_xxx */
  SWS_PTR( void),                     /* OUT: Value */
  SWS_PTR( size_t)                    /* I/O: Buffer size in, data size out */
));
typedef SWS_CB( int, SWS_FDatalinkSetProperty(
  SWS_PTR( void),                     /* IN: Client context */
  int,                                /* IN: Property, SWS_kDatalinkProperty_xxx */
  SWS_PTR( const void),               /* IN: -> Value */
  size_t                              /* IN: Size of value */
));
/* Shutdown the interface */
typedef SWS_CB( void, SWS_FDatalinkShutdown(
  SWS_PTR( void)                      /* IN: Client context */
));
/* Set the multicast list */
typedef SWS_CB( int, SWS_FDatalinkSetMulticast(
  SWS_PTR( void),                     /* IN: Client context */
  unsigned,                           /* IN: No. entries */
  SWS_PTR( const ui32)                /* IN: Array of multicast addresses */
));

/* Up calls */

/* Indication of send resources available (after SWS_FDatalinkSend returns -1) */
typedef SWS_CB( void, SWS_FDatalinkSendRdy(
  SWS_PTR( void)                      /* IN: Up call context */
));
/* IP packet receive indication */
typedef SWS_CB( void, SWS_FDatalinkRecvInd(
  SWS_PTR( void),                     /* IN: Up call context */
  SWS_BufferH,                        /* IN: Packet received */
  int                                 /* IN: Packet was broadcast */
));
/* Link status change */
typedef SWS_CB( void, SWS_FDatalinkStatus(
  SWS_PTR( void),                     /* IN: Up call context */
  int                                 /* IN: 0= link down */
));

#ifdef __cplusplus
}
#endif /* __cplusplus */


/*
 * Types
 */

/* Datalink descriptor */
typedef struct SWS_DatalinkDesc
  {
  ui16 uSize;                         /* Set to sizeof(SWS_DatalinkDesc) */
  ui16 uVersion;                      /* Set to SWS_DATALINK_VERSION */
#define SWS_DATALINK_VERSION 0x100

  /* Dispatch table */
  SWS_FNPTR( SWS_FDatalinkSend) pfnSend;  /* Required */
  SWS_FNPTR( SWS_FDatalinkGetProperty) pfnGetProperty; /* Required */
  SWS_FNPTR( SWS_FDatalinkSetProperty) pfnSetProperty; /* Optional, 0 if unimplemented */
  SWS_FNPTR( SWS_FDatalinkShutdown) pfnShutdown; /* Optional, 0 if unimplemented */
  SWS_FNPTR( SWS_FDatalinkSetMulticast) pfnSetMulticast; /* Optional, 0 if unimplemented */
  } SWS_DatalinkDesc;

/* Up call table */
typedef struct SWS_DatalinkCallbacks
  {
  SWS_PTR( void) pvContext;           /* Context for up calls */

  SWS_FNPTR( SWS_FDatalinkSendRdy) pfnSendRdy;
  SWS_FNPTR( SWS_FDatalinkRecvInd) pfnRecvInd;
  SWS_FNPTR( SWS_FDatalinkStatus)  pfnLinkStatus;
  } SWS_DatalinkCallbacks;


/*
 * Functions
 */
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Register a data link interface */
SWS_EXPORT( int, SWS_DatalinkRegister(  /* Returns !0 on error */
  SWS_PTR( const SWS_DatalinkDesc),   /* IN: Datalink descriptor */
  SWS_PTR( void),                     /* IN: Client context */
  SWS_PTR( SWS_DatalinkCallbacks)     /* OUT: Context for up call */
));

/* Count no. data links */
SWS_EXPORT( unsigned, SWS_DatalinkCount( void)); /* Returns no. links */

/* Find a data link */
SWS_EXPORT( int, SWS_DatalinkFind(    /* Returns interface ID, -ve if not found */
  SWS_PTR( const char)                /* IN: Ident */
));

/* Get a data link property */
SWS_EXPORT( int, SWS_DatalinkGetProperty( /* Returns !0 on error */
  unsigned,                           /* IN: ID, 0..SWS_DatalinkCount-1 */
  int,                                /* IN: Property, SWS_kDatalinkProperty_xxx */
  SWS_PTR( void),                     /* OUT: Value */
  SWS_PTR( size_t)                    /* I/O: Buffer size in, data size out */
));

/* Set a data link property */
SWS_EXPORT( int, SWS_DatalinkSetProperty( /* Returns !0 on error */
  unsigned,                           /* IN: ID, 0..SWS_DatalinkCount-1 */
  int,                                /* IN: Property, SWS_kDatalinkProperty_xxx */
  SWS_PTR( const void),               /* IN: -> Value */
  size_t                              /* IN: Size of value */
));

/* Get a data link state */
SWS_EXPORT( int, SWS_DatalinkIsUp(    /* Returns 0= down */
  unsigned                            /* IN: ID, 0..SWS_DatalinkCount-1 */
));

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* ndef SWS_DLNK_H */
/* End of file */
