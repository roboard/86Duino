/************************************************************************
 * SwsSock                                                              *
 *                                                                      *
 * This file contains definitions of the private cooperative thread API *
 *                                                                      *
 * Created by Lawrence Rust, Software Systems Consultants               *
 * lvr@softsystem.co.uk. Tel/Fax +33 5 49 72 79 63                      *
 *______________________________________________________________________*
 *                                                                      *
 * Revision History:                                                    *
 *                                                                      *
 * No.   Date     By   Reason                                           *
 *----------------------------------------------------------------------*
 * 01  23 Aug 02  lvr  Created                                          *
 * 02  14 Dec 02  lvr  Version for SwsSock                              *
 ************************************************************************/
#ifndef SWS_THREAD_H
#define SWS_THREAD_H 0x0101 /* Version no. BCD */

/* Dependencies */
#include "sws_host.h"
#include "sws_thrd.h"
#include "deque.h"


/* Thread instance data */
typedef struct SWS_Thread
  {
  SDeque queue;         /* Doubly linked list */
  unsigned long magic;

  SWS_FNPTR( SWS_ThreadFunction) pfn;
  SWS_PTR( void) pvContext;
  } SWS_Thread;


/*
 * Functions
 */
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

extern int SWS_ThreadInit( void);
extern void SWS_ThreadCleanup( void);

/* Platform implementation */
extern SWS_Thread* PlatformThreadAlloc( size_t);
extern void PlatformThreadFree( SWS_Thread*);
extern void PlatformThreadSwitch( SWS_Thread*, SWS_Thread*);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* ndef SWS_THREAD_H */
/* End of file */
