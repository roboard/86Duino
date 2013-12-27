/****************************************************************************
 *
 * File name   : sws_dump.h
 * Function    : Packet dumper.
 * Project     : SwsSock
 * Authors     : Lawrence Rust
 * Systems     : ANSI C
 *
 ****************************************************************************
 *
 * Created by Lawrence Rust, Software Systems Consultants
 * lvr@softsystem.co.uk. Tel/Fax +33 5 49 72 79 63
 *
 * File History
 * ------------
 * 29-Oct-02, LVR, v1.00 - Created.
 *
 *
 * Comments:
 * --------
 * This file implements the packet dumper
 *
 ****************************************************************************/

#ifndef SWS_DUMP_H
#define SWS_DUMP_H 0x0100 /* Version no. BCD */

/* Dependencies */
#include <stdio.h>    /* FILE */
#include "sws_gef.h"


/*
 * Functions
 */
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Setup the packet dumper */
SWS_EXTERN( int, SWS_DumpSetup(       /* Returns !0 if on */
  int,                                /* IN: Verbosity 0= off, 1= normal, 2= more etc */
  FILE*,                              /* IN: Destination, NULL= stderr */
  FILE*                               /* IN: Secondary destination, NULL= none */
));

/* Install this Global Event Filter for packet dumping */
SWS_EXTERN( int, SWS_DumpPacket(
  SWS_PTR( void),                     /* IN: callback context */
  int,                                /* IN: SWS_IpEvent */
  va_list
));

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* ndef SWS_DUMP_H */
/* End of file */
