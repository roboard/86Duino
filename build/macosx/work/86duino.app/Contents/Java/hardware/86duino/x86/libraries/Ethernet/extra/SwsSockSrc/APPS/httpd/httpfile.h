/****************************************************************************
 *
 * File name   : httpfile.h
 * Function    : web server file ops
 * Project     : SwsSock
 * Authors     : Lawrence Rust
 * Systems     : ANSI C
 *
 ****************************************************************************
 * File History
 * ------------
 * 13-Jan-03, LVR, v1.00 - Created.
 * 11-Dec-09, LVR, v1.01 - Added HttpFileGetRoot
 * 22-Jan-10, LVR, v1.02 - Replace HttpFileGetRoot with HttpFilePath
 *
 *
 * Comments:
 * --------
 * This file defines file system support for the web server
 *
 ****************************************************************************/

#ifndef HTTPD_FILE_H
#define HTTPD_FILE_H 0x0102 /* Version no. BCD */

#include <stddef.h> /* size_t */


/* Types */

/*
 * Functions
 */
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

extern int HttpFileInit( void);
extern size_t HttpFilePath(
  char [],                            /* OUT: full path to file */
  const char*,                        /* IN: local path */
  size_t                              /* IN: output buffer size */
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* ndef HTTPD_FILE_H */
/* End of file */
