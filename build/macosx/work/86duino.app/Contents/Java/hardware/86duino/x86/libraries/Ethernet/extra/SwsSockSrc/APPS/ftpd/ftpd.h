/****************************************************************************
 *
 * File name   : ftpd.h
 * Function    : ftp server.
 * Project     : SwsSock
 * Authors     : Lawrence Rust
 * Systems     : ANSI C
 * Version     : 1.0
 * Release Date:
 *
 ****************************************************************************
 * File History
 * ------------
 * 12-Nov-02, LVR, v1.00 - Created.
 * 15-Jul-09, LVR, v1.01 - Added FTPD_PORT
 * 07-Aug-09, LVR, v1.02 - New version
 * 15-Sep-09, LVR, v1.03 - New version
 * 23-Dec-09, LVR, v1.04
 * 22-Jan-10, LVR, v1.05
 * 30-Aug-10, LVR, v1.06
 * 30-Jan-11, LVR, v1.07
 *
 *
 * Comments:
 * --------
 * This file defines the interface to the ftp server
 *
 ****************************************************************************/

#ifndef FTPD_H
#define FTPD_H 0x0107 /* Version no. BCD */

#include <stdio.h>

#define FTPD_VER_MAJOR 1
#define FTPD_VER_MINOR 0.7

#define FTPD_PORT 21 /* The default server port */


extern int Ftpd( const void*, int, FILE*);

#endif /* ndef FTPD_H */
/* End of file */

