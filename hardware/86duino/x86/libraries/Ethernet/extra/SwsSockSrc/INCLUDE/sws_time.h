/****************************************************************************
 *
 * File name   : sws_time.h
 * Function    : Public API to the time module
 * Project     : SwsSock
 * Authors     : Lawrence Rust
 * Systems     : ANSI C
 *
 ****************************************************************************
 * File History
 * ------------
 * 12-Mar-10, LVR, v1.00 - Created.
 *
 * Comments:
 * --------
 * This file contains the external interface to the time module
 *
 ****************************************************************************/

#ifndef SWS_TIME_H
#define SWS_TIME_H 0x0100 /* Version no. BCD */

/* Dependencies */
#include <time.h> /* time_t */

#include "sws_host.h" /* Basic types */

/* Types */

/*
 * Functions
 */
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/** Convert a MIME (RFC 822) format date to a UTC time_t */
SWS_EXTERN( time_t, SWS_TimeFromStr( SWS_PTR( const char)));

/** Convert a UTC time_t to MIME (RFC 822) format */
SWS_EXTERN( SWS_PTR( char), SWS_TimeToStr( /* Returns the string */
  SWS_PTR( char),                     /* OUT date as string >= 30 bytes */
  SWS_size_t,                         /* IN: buffer size */
  time_t                              /* IN: UTC time */
));

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* ndef SWS_TIME_H */
/* End of file */
