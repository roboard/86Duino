/****************************************************************************
 *
 * File name   : sws_enet.h
 * Function    : Ethernet adapter public interface
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
 * 23-Apr-01, LVR, v1.01 - Revised for packet driver
 * 04-May-01, LVR, v1.02 - Added shutdown callback
 * 09-May-01, LVR, v1.03 - Added ioctl callback
 * 16-May-01, LVR, v1.04 - Revised definition of SWS_EnetMacAddress
 * 03-Oct-01, LVR, v1.05 - Add SWS_PACKED to SWS_EnetMacAddress for ARM SDT 2.51 .
 * 18-Oct-01, LVR, v1.06 - SWS_FEnetRecvInd returns !0 if packet dropped .
 * 29-Oct-02, LVR, v1.07 - Add SWS_EnetSetFilter .
 * 10-Jun-05, LVR, v1.08 - Add SWS_kEnetEventError .
 *
 * Comments:
 * --------
 * This file contains the public interface to the Ethernet module
 *
 ****************************************************************************/

#ifndef SWS_ENET_H
#define SWS_ENET_H 0x0108 /* Version no. BCD */

/* Dependencies */
#include <stddef.h>   /* size_t */
#include "sws_host.h" /* Basic types */
#include "sws_buf.h"  /* Buffers */


/*
 * Macros & Constants
 */

/* SWS_FEnetIoctl types */
enum
  {
  SWS_kEnetIoctl_Promiscuity = 0x1000,  /* ui32 (w/o) !0 to receive all packets */
  SWS_kEnetIoctl_MulticastList          /* SWS_EnetMacAddress[] (w/o) */
  };


/*
 * Types
 */
/* Ethernet MAC address */
typedef SWS_PACKED( struct) SWS_EnetMacAddress
  {
  ui8 addr[6];                        /* A MAC address */
  } SWS_EnetMacAddress;


/* Prototypes */
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Down calls */

/* Transmit a buffer, possibly fragmented */
typedef SWS_CB( int, SWS_FEnetSend(   /* Return !0 on error, -1 if pending */
  SWS_PTR( void),                     /* IN: Driver's context */
  SWS_BufferH                         /* IN: Buffer to send (owned by caller) */
));

/* Receive complete.  Buffer is returned to driver */
typedef SWS_CB( void, SWS_FEnetRecvAck(
  SWS_PTR( void),                     /* IN: Driver's context */
  SWS_PTR( void),                     /* IN: Buffer */
  size_t                              /* IN: Buffer size */
));

/* Get the adapter's MAC address */
typedef SWS_CB( int, SWS_FEnetGetMacAddr( /* Return !0 on error */
  SWS_PTR( void),                     /* IN: Driver's context */
  SWS_PTR( SWS_EnetMacAddress)        /* OUT: Mac address */
));

/* Shutdown the adapter */
typedef SWS_CB( void, SWS_FEnetShutdown(
  SWS_PTR( void)                      /* IN: Client context */
));

/* Generic ioctl */
typedef SWS_CB( int, SWS_FEnetIoctl(  /* Returns !0 on error */
  SWS_PTR( void),                     /* IN: Client context */
  unsigned,                           /* IN: Type */
  SWS_PTR( void),                     /* I/O: -> data */
  SWS_PTR( size_t)                    /* I/O: data size */
));

/* Up calls - May be called in ANY context including IRQ */

/* Pending send complete.  Driver is ready for more data */
typedef SWS_CB( void, SWS_FEnetSendAck(
  SWS_PTR( void),                     /* IN: Up call context */
  int                                 /* IN: Final error code, 0= no error */
));

/* Receive data.  Buffer is returned in SWS_FEnetRecvAck */
typedef SWS_CB( int, SWS_FEnetRecvInd( /* Returns !0 if packet dropped */
  SWS_PTR( void),                     /* IN: Up call context */
  SWS_PTR( void),                     /* IN: Received data buffer */
  size_t                              /* IN: Buffer size */
));

#ifdef __cplusplus
}
#endif /* __cplusplus */


/* Ethernet device descriptor */
typedef struct SWS_EnetDevice
  {
  ui16 uSize;                         /* Set to sizeof(SWS_EnetDevice) */
  ui16 uVersion;                      /* Set to SWS_ENET_VERSION */
#define SWS_ENET_VERSION 0x102

  SWS_PTR( const char) pszIdent;      /* Ident string used for config & SWS_DatalinkFind */

  /* Down call table */
  SWS_FNPTR( SWS_FEnetSend) pfnSend;          /* Required */
  SWS_FNPTR( SWS_FEnetRecvAck) pfnRecvAck;    /* Required */
  SWS_FNPTR( SWS_FEnetGetMacAddr) pfnGetMacAddr; /* Required */
  SWS_FNPTR( SWS_FEnetShutdown) pfnShutdown;  /* 0 if not implemented */
  SWS_FNPTR( SWS_FEnetIoctl) pfnIoctl;        /* 0 if not implemented */
  } SWS_EnetDevice;


/* Up call table */
typedef struct SWS_EnetCallbacks
  {
  ui32 uSize;                         /* Set to sizeof(SWS_EnetCallbacks) */
  SWS_PTR( void) pvContext;           /* Context for up calls */

  SWS_FNPTR( SWS_FEnetSendAck) pfnSendAck;
  SWS_FNPTR( SWS_FEnetRecvInd) pfnRecvInd;
  } SWS_EnetCallbacks;


/*
 * Functions
 */
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*
 * Register an Ethernet adapter with the SwsSock stack
 */
SWS_EXPORT( int, SWS_EnetAddAdapter(  /* Returns !0 on error */
  SWS_PTR( const SWS_EnetDevice),     /* IN: Device descriptor */
  SWS_PTR( void),                     /* IN: Driver's context for down calls */
  SWS_PTR( SWS_EnetCallbacks)         /* OUT: Up calls */
));


/* Initialise the Ethernet manager */
extern int SWS_EnetInit( void);       /* Returns !0 on error */

/* Finalise the Ethernet manager */
extern void SWS_EnetCleanup( void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* ndef SWS_ENET_H */
/* End of file */
