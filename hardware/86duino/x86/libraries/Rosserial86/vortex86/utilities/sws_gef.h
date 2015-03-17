/****************************************************************************
 *
 * File name   : sws_gef.h
 * Function    : Global event filter.
 * Project     : SwsSock
 * Authors     : Lawrence Rust
 * Systems     : ANSI C
 *
 ****************************************************************************
 * File History
 * ------------
 * 28-Aug-05, LVR, v1.00 - Created.
 *
 *
 * Comments:
 * --------
 * This file contains the public interface to the Global Event Filter
 *
 ****************************************************************************/

#ifndef SWS_GEF_H
#define SWS_GEF_H 0x0100 /* Version no. BCD */

/* Dependencies */
#include <stdarg.h>   /* va_list */

#include "sws_host.h" /* Basic types */


/*
 * Macros & Constants
 */

/* Types of events */
enum SWS_EGefEvent
  {
  SWS_kGefNone = 0,
  SWS_kGefCleanup,                    /* SwsSock is closing */

  SWS_kGefIpTx,                       /* SWS_BufferH. ui32 next hop */
  SWS_kGefIpRx,                       /* SWS_BufferH, int isBroadcast */
  SWS_kGefIpError,                    /* SWS_BufferH, int error, SWS_PTR( const char) message */
  SWS_kGefIcmpError,                  /* SWS_BufferH: uint protocol, uint type, uint code, ui32 data */

  SWS_kGefTcpError,                   /* SWS_PTR( const char) message */
  SWS_kGefUdpError,                   /* SWS_PTR( const char) message */
  SWS_kGefRawError,                   /* SWS_PTR( const char) message */

  SWS_kGefEnetTx,                     /* SWS_BufferH */
  SWS_kGefEnetRx,                     /* SWS_PTR( const void), ui32 len, int broadcast */
  SWS_kGefEnetError,                  /* int error, SWS_PTR( const char) message */

  SWS_kGefP2pTx,                      /* ui32 len, SWS_PTR( const void) */
  SWS_kGefP2pRx,                      /* ui32 len, SWS_PTR( const void) */
  SWS_kGefP2pError,                   /* SWS_PTR( const char) message */

  SWS_kGefPppTx,                      /* uint protocol, SWS_BufferH */
  SWS_kGefPppRx,                      /* uint protocol, SWS_BufferH */
  SWS_kGefPppError,                   /* SWS_PTR( const char) message */

  SWS_kGefSlipTx,                     /* SWS_BufferH */
  SWS_kGefSlipRx,                     /* SWS_BufferH */
  SWS_kGefSlipError,                  /* SWS_PTR( const char) message */

  /* Add new events above */
  SWS_kGefCount
  };


/*
 * Data types
 */

/* Filter callback */
typedef SWS_CB( int, SWS_FGef(        /* Return value passed to caller of SWS_GefRaise */
  SWS_PTR( void),                     /* IN: callback context */
  int,                                /* IN: SWS_EGefEvent */
  va_list
));


/*
 * Functions
 */
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Install an event filter */
SWS_EXPORT( SWS_FNPTR( SWS_FGef), SWS_GefInstall( /* Returns previous GEF */
  SWS_FNPTR( SWS_FGef),               /* IN: -> Filter callback */
  SWS_PTR( void)                      /* IN: callback context */
));

/* Signal an event. NB C calling convention required  for ... */
extern int SWS_GefRaise(              /* Returns GEF result or 0 if none */
  int,                                /* IN: SWS_EGefEvent */
  ...                                 /* IN: Depends on event */
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* ndef SWS_GEF_H */
/* End of file */
