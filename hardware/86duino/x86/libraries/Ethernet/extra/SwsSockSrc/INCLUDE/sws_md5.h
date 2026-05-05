/****************************************************************************
 *
 * File name   : md5.h
 * Function    : Public API to the MD5 module
 * Project     : SwsSock
 * Authors     : Lawrence Rust
 * Systems     : ANSI C
 *
 ****************************************************************************
 * File History
 * ------------
 * 09-Aug-05, LVR, v1.00 - Created.
 *
 * Comments:
 * --------
 * This file contains the external interface to the MD5 module
 *
 ****************************************************************************/

#ifndef SWS_MD5_H
#define SWS_MD5_H 0x0101 /* Version no. BCD */

/* Dependencies */
#include <stddef.h>   /* size_t */
#include "sws_host.h" /* Basic types */


/*
 * Types
 */

/* MD5 context. */
typedef struct SWS_MD5
  {
  ui32 state[4];                /* state (ABCD) */
  ui32 count[2];                /* no. bits, modulo 2^64 (lsb first) */
  unsigned char buffer[64];     /* input buffer */
  } SWS_MD5;

/* MD5 digest */
typedef struct SWS_MD5Digest
  {
  unsigned char digest[16];
  } SWS_MD5Digest;


/*
 * Functions
 */
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Init an MD5 context */
SWS_EXTERN( void, SWS_MD5Init(
  SWS_PTR( SWS_MD5)                   /* OUT: context */
));

/* Add a buffer */
SWS_EXTERN( void, SWS_MD5Update(
  SWS_PTR( SWS_MD5),                  /* I/O: context */
  SWS_PTR( const void),               /* IN: -> buffer */
  SWS_size_t                          /* IN: buffer size */
));

/* Complete the MD5 digest */
SWS_EXTERN( void, SWS_MD5Final(
  SWS_PTR( SWS_MD5),                  /* I/O: context */
  SWS_PTR( SWS_MD5Digest)             /* OUT, digest */
));

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* ndef SWS_MD5_H */
/* End of file */
