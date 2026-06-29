/****************************************************************************
 *
 * File name   : ip.h
 * Function    : Private interface to the IP module
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
 * 19-Dec-02, LVR, v1.01 - Added code & gateway params to SWS_IpIcmpRedirect
 *
 *
 * Comments:
 * --------
 * This file contains the private interface to the IP module
 *
 ****************************************************************************/

#ifndef SWS_IPPRIV_H
#define SWS_IPPRIV_H 0x0101 /* Version no. BCD */

/* Dependencies */
#include <stddef.h>   /* size_t */
#include "sws_host.h" /* Basic types */
#include "sws_buf.h"


/*
 * Functions
 */
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Initialise the module */
extern int SWS_IpInit( void);         /* Returns !0 on error */

/* Finalise the module */
extern void SWS_IpCleanup( void);

/* Process an ICMP error message */
extern void SWS_IpIcmpError(
  SWS_BufferH,
  size_t uHdrLen,
  ui32 src,
  ui32 dst,
  unsigned type,
  unsigned code,
  ui32 data
);

/* Process an ICMP redirect message */
extern void SWS_IpIcmpRedirect(
  SWS_BufferH,
  size_t uHdrLen,
  ui32 src,
  ui32 dst,
  unsigned code,
  ui32 gateway
);

/* IGMP events */
extern void SWS_IpIgmpQuery(
  ui32 src,
  ui32 group,
  unsigned tenths
);
extern void SWS_IpIgmpReport(
  ui32 src,
  ui32 group
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* ndef SWS_IPPRIV_H */
/* End of file */
