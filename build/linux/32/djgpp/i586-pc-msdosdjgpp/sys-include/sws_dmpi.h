/****************************************************************************
 *
 * File name   : sws_dmpi.h
 * Function    : IP packet dumper.
 * Project     : SwsSock
 * Authors     : Lawrence Rust
 * Systems     : ANSI C
 *
 ****************************************************************************
 * File History
 * ------------
 * 02-Apr-01, LVR, v1.00 - Created.
 * 31-Oct-02, LVR, v1.01 - Added secondary destination to SWS_IpDump
 *
 *
 * Comments:
 * --------
 * This file contains the public interface to the IP packet dumper
 *
 ****************************************************************************/

#ifndef SWS_DMPI_H
#define SWS_DMPI_H 0x0101 /* Version no. BCD */

/* Dependencies */
#include <stdarg.h>   /* va_list */

#include "sws_host.h"
#include "sws_buf.h"


/*
 * Functions
 */
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Setup the packet dumper */
SWS_EXTERN( void, SWS_DmpIpSetup(
  int                                 /* IN: Verbosity 0= off, 1= normal, 2= more etc */
));

/* GEF handler */
SWS_EXTERN( int, SWS_DmpIp(           /* Returns chars written */
  SWS_PTR( char),                     /* OUT: text */
  int,                                /* IN: SWS_EGefEvent */
  va_list va                          /* IN: */
));

/* Dump a packet */
SWS_EXTERN( int, SWS_DmpIpPacket(     /* Returns space used in buffer */
  SWS_PTR( char),                     /* OUT: Text */
  SWS_BufferH                         /* IN: packet */
));

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* ndef SWS_DMPI_H */
/* End of file */
