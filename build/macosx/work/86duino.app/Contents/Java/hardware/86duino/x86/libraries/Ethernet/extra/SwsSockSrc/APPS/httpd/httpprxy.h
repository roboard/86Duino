/****************************************************************************
 *
 * File name   : httpprxy.h
 * Function    : web proxy server
 * Project     : SwsSock
 * Authors     : Lawrence Rust
 * Systems     : ANSI C
 * Version     : 1.0
 * Release Date:
 *
 ****************************************************************************
 * File History
 * ------------
 * 24-Oct-09, LVR, v1.00 - Created.
 *
 *
 * Comments:
 * --------
 * This file defines the proxy web server
 *
 ****************************************************************************/

#ifndef HTTPD_PROXY_H
#define HTTPD_PROXY_H 0x0100 /* Version no. BCD */


/* Types */


/*
 * Functions
 */
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

extern int HttpProxyInit( void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* ndef HTTPD_PROXY_H */
/* End of file */
