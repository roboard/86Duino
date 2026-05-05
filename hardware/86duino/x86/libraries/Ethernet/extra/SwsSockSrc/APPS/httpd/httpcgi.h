/****************************************************************************
 *
 * File name   : httpcgi.h
 * Function    : web server CGI
 * Project     : SwsSock
 * Authors     : Lawrence Rust
 * Systems     : ANSI C
 * Version     : 1.0
 * Release Date:
 *
 ****************************************************************************
 * File History
 * ------------
 * 07-Dec-09, LVR, v1.00 - Created.
 *
 *
 * Comments:
 * --------
 * This file defines CGI support for the web server
 *
 ****************************************************************************/

#ifndef HTTPD_CGI_H
#define HTTPD_CGI_H 0x0100 /* Version no. BCD */

#include "httpd.h"


/* Types */

/*
 * Functions
 */
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

extern int HttpCgiInit( void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* ndef HTTPD_CGI_H */
/* End of file */
