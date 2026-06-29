/************************************************************************
 * SwsSock                                                              *
 *                                                                      *
 * Thread management                                                    *
 *                                                                      *
 * Created by Lawrence Rust, Software Systems Consultants               *
 *______________________________________________________________________*
 *                                                                      *
 * Revision History:                                                    *
 *                                                                      *
 * No.   Date      By   Reason                                          *
 *----------------------------------------------------------------------*
 * 100  02 Jul 02  lvr  Created (MTCLIB)                                *
 * 101  14 Dec 02  lvr  Version for SwsSock                             *
 ************************************************************************/
#ifndef SWS_THRD_H
#define SWS_THRD_H 0x101 /* Version no. BCD */

/* Dependencies */
#include "sws_host.h"


/*
 * Constants
 */


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*
 * Types
 */
typedef SWS_PTR( struct SWS_Thread) SWS_ThreadHandle;

/* Thread 'main' function */
typedef SWS_CB( i32, SWS_ThreadFunction(
  SWS_PTR( void)                        /* IN: From SWS_ThreadCreate */
));


/*
 * Functions
 */

/* Create a thread */
SWS_EXPORT( SWS_ThreadHandle, SWS_ThreadCreate( /* OUT: Thread handle, 0= error */
  SWS_FNPTR( SWS_ThreadFunction),       /* IN: -> thread function */
  SWS_PTR( void),                       /* IN: Client context for SWS_ThreadFunction */
  SWS_size_t                            /* IN: Stack size in bytes, 0= default */
));

/* Destroy a thread.
 * NB it is an error to destroy the thread that called SWS_SockStartup
 */
SWS_EXPORT( int, SWS_ThreadDestroy(     /* OUT: 0= no error */
  SWS_ThreadHandle                      /* IN: Thread, NULL= current thread */
));

/* Exit a thread.
 * NB the thread that called SWS_SockStartup will call exit().
 */
SWS_EXPORT( void, SWS_ThreadExit(
  i32                                   /* IN: Exit status */
));

/* Get the current thread handle */
SWS_EXPORT( SWS_ThreadHandle, SWS_ThreadCurrent(
  void
));

/* Suspend and resume a thread */
SWS_EXPORT( int, SWS_ThreadStop(        /* OUT: 0= no error */
  SWS_ThreadHandle                      /* IN: Thread */
));
SWS_EXPORT( int, SWS_ThreadStart(       /* OUT: 0= no error */
  SWS_ThreadHandle                      /* IN: Thread */
));

/* Yield the current thread */
SWS_EXPORT( void, SWS_ThreadYield( void));

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* ndef SWS_THRD_H */
/* eof */
