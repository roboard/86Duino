/****************************************************************************
 *
 * File name   : sws_msgq.h
 * Function    : Message queue public interface
 * Project     : SwsSock
 * Authors     : Lawrence Rust
 * Systems     : ANSI C
 * Version     : 1.0
 * Release Date:
 *
 ****************************************************************************
 * File History
 * ------------
 * 11-Apr-01, LVR, v1.00 - Created.
 *
 *
 * Comments:
 * --------
 * This file defines the public interface to message queues..
 *
 ****************************************************************************/

#ifndef SWS_MSGQ_H
#define SWS_MSGQ_H 0x0100 /* Version no. BCD */

/* Dependencies */
#include <stddef.h>
#include "sws_host.h" /* Basic types */


/* Forward declarations */
typedef SWS_PTR( struct SWS_MsgQ) SWS_MsgqH;


/*
 * Functions
 */
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Create a message queue */
SWS_EXPORT( SWS_MsgqH, SWS_MsgqCreate( /* Returns NULL on error */
  size_t,                             /* IN: Max queue length */
  size_t                              /* IN: Message size */
));

/* Destroy a message queue */
SWS_EXPORT( void, SWS_MsgqDestroy(
  SWS_MsgqH                           /* IN: Message queue */
));

/* Put a message */
SWS_EXPORT( int, SWS_MsgqPut(         /* Returns !0 on error */
  SWS_MsgqH,                          /* IN: Message queue */
  size_t,                             /* IN: Message size (for checking) */
  SWS_PTR( const void)                /* IN: Message */
));

/* Get a message */
SWS_EXPORT( int, SWS_MsgqGet(         /* Returns !0 if none */
  SWS_MsgqH,                          /* IN: Message queue */
  size_t,                             /* IN: Message size (for checking) */
  SWS_PTR( void)                      /* OUT: Message */
));

SWS_EXPORT( size_t, SWS_MsgqCount(    /* Returns no. queued items */
  SWS_MsgqH                           /* IN: Message queue */
));

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* ndef SWS_MSGQ_H */
/* End of file */
