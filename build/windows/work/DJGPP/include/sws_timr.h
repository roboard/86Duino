/****************************************************************************
 *
 * File name   : sws_timr
 * Function    : Timer module public interface
 * Project     : SwsSock
 * Authors     : Lawrence Rust
 * Systems     : ANSI C
 * Version     : 1.0
 * Release Date:
 *
 ****************************************************************************
 * File History
 * ------------
 * 17-Apr-01, LVR, v1.00 - Created.
 *
 *
 * Comments:
 * --------
 * This file contains the public interface to the timer module
 *
 ****************************************************************************/

#ifndef SWS_TIMR_H
#define SWS_TIMR_H 0x0100 /* Version no. BCD */

/* Dependencies */
#include "sws_host.h" /* Basic types */


/*
 * Types
 */

/* Timer handle */
typedef SWS_PTR( struct SWS_Timer) SWS_TimerH;


/*
 * Prototypes
 */
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Timer expiry callback */
typedef SWS_CB( void, SWS_FTimer(
  SWS_TimerH,                         /* IN: Timer that expired */
  SWS_PTR( void)                      /* IN: User context */
));

#ifdef __cplusplus
}
#endif /* __cplusplus */


/*
 * Functions
 */
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Allocate a timer */
SWS_EXPORT( SWS_TimerH, SWS_TimerAlloc( /* Timer handle or NULL if none */
  SWS_FNPTR( SWS_FTimer),             /* IN: Optional callback on expiry */
  SWS_PTR( void)                      /* IN: Callback context */
));

/* Release a timer */
SWS_EXPORT( void, SWS_TimerRelease(
  SWS_TimerH                          /* IN: Timer to free */
));

/* Start a timer */
SWS_EXPORT( int, SWS_TimerStart(      /* Returns !0 on error */
  SWS_TimerH,                         /* IN: Timer */
  ui32                                /* IN: Ticks until expiry */
));

/* Stop a timer */
SWS_EXPORT( ui32, SWS_TimerStop(      /* Returns ticks remaining */
  SWS_TimerH                          /* IN: Timer */
));

/* Get ticks remaining */
SWS_EXPORT( ui32, SWS_TimerRemaining( /* Ticks remaining, 0= expired */
  SWS_TimerH                          /* IN: Timer */
));


/* Initialise the timer module */
extern int SWS_TimerInit( void);      /* Returns !0 on error */

/* Finalise the module */
extern void SWS_TimerCleanup( void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* ndef SWS_TIMR_H */
/* End of file */
