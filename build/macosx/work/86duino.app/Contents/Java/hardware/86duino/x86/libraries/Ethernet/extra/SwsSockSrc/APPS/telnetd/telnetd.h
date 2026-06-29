/****************************************************************************
 *
 * File name   : telnetd.h
 * Function    : telnet server interface.
 * Project     : SwsSock
 * Authors     : Lawrence Rust
 * Systems     : ANSI C
 * Version     : 1.0
 * Release Date:
 *
 ****************************************************************************
 * File History
 * ------------
 * 04-Jun-01, LVR, v1.00 - Created.
 * 27-Nov-02, LVR, v1.01 - Added logfile to Telnetd.
 * 20-Feb-03, LVR, v2.00
 * 15-Jul-09, LVR, v2.01 - Added TELNETD_PORT
 * 07-Aug-09, LVR, v2.02
 * 23-Dec-09, LVR, v2.03
 * 29-Jan-10, LVR, v2.04
 *
 *
 * Comments:
 * --------
 * This file defines the interface to the telnet server
 *
 ****************************************************************************/

#ifndef TELNETD_H
#define TELNETD_H 0x0204 /* Version no. BCD */

#include <stddef.h>
#include <stdio.h>

#define TELNETD_VER_MAJOR 2
#define TELNETD_VER_MINOR 0.4

#define TELNETD_PORT 23 /* The default server port */

/* Forward declarations */
typedef struct STelnetConnection STelnetConnection;
typedef struct STelnetCommand STelnetCommand;

/* Telnet command function */
typedef int FTelnetCommand( STelnetConnection*, const char*);

/* Telnet command descriptor */
struct STelnetCommand
  {
  const char* pszName;
  FTelnetCommand* pfnExecute;
  const char* pszHelp;
  };

extern int Telnetd( const void*, int, FILE*, size_t, const STelnetCommand []);

extern int TelnetWrite( STelnetConnection*, const void*, size_t);
extern int TelnetRead( STelnetConnection*, void*, size_t);
extern int TelnetReadLine( STelnetConnection*);

#endif /* ndef TELNETD_H */
/* End of file */

