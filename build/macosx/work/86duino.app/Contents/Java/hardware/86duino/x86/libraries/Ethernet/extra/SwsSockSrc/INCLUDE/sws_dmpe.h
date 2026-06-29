/****************************************************************************
 *
 * File name   : sws_dmpe.h
 * Function    : Ethernet packet dumper.
 * Project     : SwsSock
 * Authors     : Lawrence Rust
 * Systems     : ANSI C
 *
 ****************************************************************************
 * File History
 * ------------
 * 29-Oct-02, LVR, v1.00 - Created.
 *
 *
 * Comments:
 * --------
 * This file contains the public interface to the Ethernet packet dumper
 *
 ****************************************************************************/

#ifndef SWS_DMPE_H
#define SWS_DMPE_H 0x0100 /* Version no. BCD */

/* Dependencies */
#include <stdarg.h>   /* va_list */
#include "sws_host.h" /* Basic types */


/*
 * Functions
 */
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Setup the packet dumper */
SWS_EXTERN( void, SWS_DmpEnetSetup(
  int                                 /* IN: Verbosity 0= off, 1= normal, 2= more etc */
));

/* GEF handler */
SWS_EXTERN( int, SWS_DmpEnet(         /* Returns chars written */
  SWS_PTR( char),                     /* OUT: text */
  int,                                /* IN: SWS_EGefEvent */
  va_list va                          /* IN: */
));

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* ndef SWS_DMPE_H */
/* End of file */
