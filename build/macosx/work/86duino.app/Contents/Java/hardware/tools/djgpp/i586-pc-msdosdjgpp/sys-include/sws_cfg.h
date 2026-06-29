/****************************************************************************
 * Created by Lawrence Rust, Software Systems Consultants
 *
 * File name   : sws_cfg.h
 * Function    : Public API to access configuration data
 * Project     : SwsSock
 * Authors     : Lawrence Rust
 * Systems     : ANSI C
 * Version     : 1.0
 *
 ****************************************************************************
 * File History
 * ------------
 * 29-Jan-03, LVR, v1.00 - Created.
 * 14-Sep-03, LVR, v1.01 - Added.SWS_CFG_MAX_KEY
 * 17-Sep-04, LVR, v1.02 - SWS_CFG_MAX_LINE increased to 255 from 127
 * 18-Nov-04, LVR, v1.03 - Added SWS_CfgSetString
 * 30-Nov-04, LVR, v1.04 - Added SWS_CfgSetValue
 * 27-Jan-10, LVR, v1.05 - SWS_CfgSetName parses file, SWS_CfgEnum uses cache
 * 19-Jul-10, LVR, v1.06 - Add SWS_CfgGetName
 *
 * Comments:
 * --------
 * This file contains the external interface to the config module
 *
 ****************************************************************************/

#ifndef SWS_CFG_H
#define SWS_CFG_H 0x0106 /* Version no. BCD */

/* Dependencies */
#include <stddef.h>   /* size_t */
#include "sws_host.h" /* Basic types */


/* Constants */
#ifndef SWS_CFGFILE
#define SWS_CFGFILE "sws_sock.ini"    /* default configuration filename */
#endif

#define SWS_CFG_MAX_KEY 63            /* Max len of a section or value name (without nul) */
#define SWS_CFG_MAX_LINE 255          /* Max len of any line */


/*
 * Types
 */
/* SWS_CfgEnum callback */
typedef SWS_CB( int, SWS_FCfgCallback( /* Return !0 to terminate enumeration */
  SWS_PTR( void),                     /* IN: Context passed to SWS_CfgEnum */
  SWS_PTR( const char)                /* IN: Configuration string */
));


/*
 * Functions
 */
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Set ini file */
SWS_EXTERN( int, SWS_CfgSetName(      /* Returns !0 if config file found */
  SWS_PTR( const char),               /* IN: config filename or NULL for default */
  SWS_PTR( const char)                /* IN: argv[0] or NULL */
));
SWS_EXTERN( SWS_PTR( const char), SWS_CfgGetName( void));

/* Enumerate all configuration strings in a section */
SWS_EXTERN( int, SWS_CfgEnum(         /* Returns !0 if found */
  SWS_PTR( const char),               /* IN: Section name */
  SWS_FNPTR( SWS_FCfgCallback),       /* IN: Callback */
  SWS_PTR( void)                      /* IN: Callback context */
));

/* Get a configuration string */
SWS_EXTERN( int, SWS_CfgGetString(    /* Returns no. chars read */
  SWS_PTR( const char),               /* IN: Section name */
  SWS_PTR( const char),               /* IN: Value name */
  SWS_PTR( const char),               /* IN: Default value */
  SWS_PTR( char),                     /* OUT: String buffer */
  size_t                              /* IN: String buffer size */
));

/* Set a configuration string. Needs SWS_CFG_SET */
SWS_EXTERN( int, SWS_CfgSetString(    /* Returns !0 on success */
  SWS_PTR( const char),               /* IN: Section name */
  SWS_PTR( const char),               /* IN: Value name (maybe NULL) */
  SWS_PTR( const char)                /* IN: Value */
));

/* Get a configuration value */
SWS_EXTERN( ui32, SWS_CfgGetValue(    /* Returns the value */
  SWS_PTR( const char),               /* IN: Section name */
  SWS_PTR( const char),               /* IN: Value name */
  ui32                                /* IN: Default value */
));

/* Set a configuration value. Needs SWS_CFG_SET */
SWS_EXTERN( int, SWS_CfgSetValue(     /* Returns !0 on success */
  SWS_PTR( const char),               /* IN: Section name */
  SWS_PTR( const char),               /* IN: Value name */
  ui32                                /* IN: value */
));

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* ndef SWS_CFG_H */
/* End of file */
