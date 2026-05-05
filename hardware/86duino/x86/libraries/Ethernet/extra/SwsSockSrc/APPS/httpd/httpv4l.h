/****************************************************************************
 *
 * File name   : httpv4l.h
 * Function    : Video4Linux web cam
 * Project     : SwsSock
 * Authors     : Lawrence Rust
 * Systems     : ANSI C
 * Version     : 1.0
 * Release Date:
 *
 ****************************************************************************
 * File History
 * ------------
 * 19-May-09, LVR, v1.00 - Created.
 *
 *
 * Comments:
 * --------
 * This file defines the v4l web cam API
 *
 ****************************************************************************/

#ifndef HTTPD_V4L_H
#define HTTPD_V4L_H 0x0100 /* Version no. BCD */


/* Types */


/*
 * Functions
 */
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

extern int HttpV4lInit( void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* ndef HTTPD_V4L_H */
/* End of file */
