/****************************************************************************
 *
 * File name   : sws_str.h
 * Function    : Public API to the strings module
 * Project     : SwsSock
 * Authors     : Lawrence Rust
 * Systems     : ANSI C
 *
 ****************************************************************************
 * File History
 * ------------
 * 12-Mar-10, LVR, v1.00 - Created.
 * 07-Apr-10, LVR, v1.00 - Added SWS_stristr
 *
 * Comments:
 * --------
 * This file contains the external interface to the strings module
 *
 ****************************************************************************/

#ifndef SWS_STR_H
#define SWS_STR_H 0x0101 /* Version no. BCD */

/* Dependencies */
#include "sws_host.h" /* Basic types */

/* Types */

/*
 * Functions
 */
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/** Wild-card name matching - Much smaller than POSIX regcomp */
SWS_EXTERN( int, SWS_StrMatch(        /* Returns 1 if matched */
  SWS_PTR( const char),               /* IN: Pattern may contain * and ? */
  SWS_PTR( const char),               /* IN: string to test */
  int                                 /* IN: !0= case insensitive */
));

/** Case insensitive strcmp */
SWS_EXTERN( int, SWS_stricmp( SWS_PTR( const char), SWS_PTR( const char)));
SWS_EXTERN( int, SWS_strnicmp( SWS_PTR( const char), SWS_PTR( const char), SWS_size_t));

/** Case insensitive strstr */
SWS_EXTERN( SWS_PTR( const char), SWS_stristr( SWS_PTR( const char), SWS_PTR( const char)));

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* ndef SWS_STR_H */
/* End of file */
