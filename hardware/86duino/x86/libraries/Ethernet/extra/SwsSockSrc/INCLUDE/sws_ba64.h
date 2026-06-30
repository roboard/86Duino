/****************************************************************************
 *
 * File name   : sws_ba64.h
 * Function    : Public API to the base64 module
 * Project     : SwsSock
 * Authors     : Lawrence Rust
 * Systems     : ANSI C
 *
 ****************************************************************************
 * File History
 * ------------
 * 11-Mar-10, LVR, v1.00 - Created.
 *
 * Comments:
 * --------
 * This file contains the external interface to the base54 module
 *
 ****************************************************************************/

#ifndef SWS_BASE64_H
#define SWS_BASE64_H 0x0100 /* Version no. BCD */

/* Dependencies */
#include <stddef.h>   /* size_t */


/*
 * Types
 */

/*
 * Functions
 */
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/** Decode a base64 string */
extern size_t SWS_Base64Decode(       /* Returns size of decoded data, 0 if invalid input */
  unsigned char [],                   /* OUT: buffer for decoded data - may overlap input */
  size_t,                             /* IN: length of output buffer */
  const char*                         /* IN: string to decode */
);

/** Encode a base64 string */
extern void SWS_Base64Encode(
  char*,                              /* OUT: buffer for encoded data */
  size_t,                             /* IN: length of output buffer */
  const unsigned char [],             /* IN: input buffer */
  size_t                              /* IN: length of input */
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* ndef SWS_BASE64_H */
/* End of file */
