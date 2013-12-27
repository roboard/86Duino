/****************************************************************************
 *
 * File name   : sws_dmpp.h
 * Function    : P2P/PPP packet dumper.
 * Project     : SwsSock
 * Authors     : Lawrence Rust
 * Systems     : ANSI C
 *
 ****************************************************************************
 * File History
 * ------------
 * 25-Oct-05, LVR, v1.00 - Created.
 *
 *
 * Comments:
 * --------
 * This file contains the public interface to the P2P/PPP packet dumper
 *
 ****************************************************************************/

#ifndef SWS_DMPP_H
#define SWS_DMPP_H 0x0100 /* Version no. BCD */

/* Dependencies */
#include <stdio.h>    /* FILE */
#include <stdarg.h>   /* va_list */
#include "sws_host.h" /* Basic types */


/*
 * Functions
 */
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Setup the packet dumper */
SWS_EXTERN( void, SWS_DmpPppSetup(
  int                                 /* IN: Verbosity 0= off, 1= normal, 2= more etc */
));
SWS_EXTERN( void, SWS_DmpP2pSetup(
  int                                 /* IN: Verbosity 0= off, 1= normal, 2= more etc */
));

/* GEF handler */
SWS_EXTERN( int, SWS_DmpPpp(          /* Returns chars written */
  SWS_PTR( char),                     /* OUT: text */
  int,                                /* IN: SWS_EGefEvent */
  va_list va                          /* IN: */
));
SWS_EXTERN( int, SWS_DmpP2p(          /* Returns chars written */
  SWS_PTR( char),                     /* OUT: text */
  int,                                /* IN: SWS_EGefEvent */
  va_list va                          /* IN: */
));

/* Dump a packet */
SWS_EXTERN( int, SWS_DmpPppPacket(    /* Returns space used in buffer */
  SWS_PTR( char),                     /* OUT: Text */
  ui16,                               /* IN: Protocol */
  SWS_PTR( const void),               /* IN: -> packet */
  SWS_size_t                          /* IN: packet len */
));

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* ndef SWS_DMPP_H */
/* End of file */
