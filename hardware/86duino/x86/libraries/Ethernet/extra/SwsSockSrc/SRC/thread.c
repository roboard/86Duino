/************************************************************************
 * SwsSock                                                              *
 *                                                                      *
 * Platform independent cooperative thread management                   *
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
/* Compilation options */
#include "config.h"

/* Exports */
#include "sws_thrd.h"

#if OPT_THREADS == OPT_THREADS_COOP
#include "thread.h"

/* Imports */
#include <stddef.h>
#include <string.h>

#include "platform.h"
#include "sws_sock.h"
#include "deque.h"
#include "debug.h"


/*
 * Constants & macros
 */
#define THREAD_MAGIC SWS_MAKEFOURCC( 't','h','r','d')
#define ThreadIsValid( h) (NULL != (h) && THREAD_MAGIC == (h)->magic)
#define THREAD( p) SWS_CONTAINER( p, SWS_Thread, queue)


/*
 * Local types
 */


/*
 * Prototypes
 */
static void ThreadDispatch( void);
static void ThreadDestroy( SWS_Thread*);


/*
 * Module data
 */
static SWS_Thread* s_pMainThread;     /* The main thread */
static SWS_Thread* s_pCurrentThread;  /* The current thread (if any) */
static SDeque s_runnable;             /* Queue of runnable threads */


/*
 * Initialise this module
 */
int SWS_ThreadInit( void) /* Returns !0 on error */
  {
  TRACE( 9, ("SWS_ThreadInit()\n"));

  /* Init statics */
  DEQUE_INIT( &s_runnable);

  /* Create the 'main' thread */
  s_pMainThread = PlatformThreadAlloc( 1);
  if ( NULL == s_pMainThread)
    return 1;

  memset( s_pMainThread, 0, sizeof( *s_pMainThread) );
  DEQUE_INIT( &s_pMainThread->queue);

  s_pMainThread->pfn = 0;
  s_pMainThread->magic = THREAD_MAGIC;
  ASSERT( ThreadIsValid( s_pMainThread));
  TRACE( 2, ("SWS_ThreadInit() main= %p\n", s_pMainThread));

  s_pCurrentThread = s_pMainThread;
  return 0;
  }


/*
 * Finalise this module
 */
void SWS_ThreadCleanup( void)
  {
  SDeque* pItem;

  TRACE( 9, ("SWS_ThreadCleanup()\n"));

  /* Destroy any pending threads */
  while ( &s_runnable != (pItem = DEQUE_FIRST( &s_runnable)) )
    {
    SWS_Thread* const pThread = THREAD( SWS_DequeRemove( pItem));
    TRACE( 1, ("*** SWS_ThreadCleanup destroying thread %p\n", pThread));
    ThreadDestroy( pThread);
    }

  /* Destroy the current thread */
  if ( NULL != s_pCurrentThread)
    ThreadDestroy( s_pCurrentThread);

  /* Dispose of the main thread */
  if ( NULL != s_pMainThread)
    ThreadDestroy( s_pMainThread);
  }


/****************************************************************************
 * Exported functions
 ****************************************************************************/

/*
 * Allocate a thread instance
 */
SWS_EXPORT( SWS_ThreadHandle, SWS_ThreadCreate( /* OUT: Thread handle, NULL= error */
  SWS_FNPTR( SWS_ThreadFunction) pfn,   /* IN: -> thread function */
  SWS_PTR( void) pvContext,             /* IN: Client context for SWS_ThreadFunction */
  SWS_size_t size                       /* IN: Stack size in bytes, 0= default */
)){
  SWS_Thread* pThread;

  TRACE( 9, ("SWS_ThreadCreate(%p,%p,%u)\n", pfn, pvContext, size));
  ASSERT( pfn);

  /* Ensure main thread is created */
  if ( NULL == s_pMainThread
    && SWS_ThreadInit()
  ) {
    return NULL;
    }

  pThread = PlatformThreadAlloc( size);
  if ( NULL == pThread)
    return NULL;

  memset( pThread, 0, sizeof( SWS_Thread) );

  /* Init thread data */
  DEQUE_INIT( &pThread->queue);
  pThread->pfn = pfn;
  pThread->pvContext = pvContext;

  /* Set thread signature */
  pThread->magic = THREAD_MAGIC;
  ASSERT( ThreadIsValid( pThread) );
  TRACE( 2, ("SWS_ThreadCreate(%p,%p,%u) => %p\n", pfn,pvContext,size,pThread));

  return pThread;
  }


/*
 * Destroy a thread - with prejudice!
 */
SWS_EXPORT( int, SWS_ThreadDestroy( SWS_ThreadHandle hThread))
  {
  TRACE( 8, ("SWS_ThreadDestroy(%p)\n", hThread));
  ASSERT( !PlatformIsCritical());

  if ( NULL == hThread)
    hThread = s_pCurrentThread;
  else if ( !ThreadIsValid( hThread))
    return SWS_EINVAL;

  /* Destroying the main thread disables all threads */
  if ( hThread == s_pMainThread)
    {
    SWS_ThreadCleanup();
    return 0;
    }

  ThreadDestroy( hThread);

  /* Dispatch next runnable thread if current thread destroyed */
  if ( NULL == s_pCurrentThread)
    ThreadDispatch();

  return 0;
  }

void ThreadDestroy( SWS_Thread* pThread)
  {
  TRACE( 8, ("ThreadDestroy(%p)\n", pThread));

  /* Unlink from any suspense queue */
  if ( !DEQUE_IS_EMPTY( &pThread->queue))
    SWS_DequeRemove( &pThread->queue);

  if ( pThread == s_pMainThread)
    s_pMainThread = NULL;

  if ( pThread == s_pCurrentThread)
    s_pCurrentThread = NULL;

  pThread->magic = 0;
  PlatformThreadFree( pThread);
  }


/*
 * Exit a thread
 */
SWS_EXPORT( void, SWS_ThreadExit(
  i32 iExit                         /* IN: Exit status */
)){
  SWS_Thread* const pThread = s_pCurrentThread;

  TRACE( 2, ("SWS_ThreadExit %p (%ld)\n",pThread,iExit));

  ASSERT( ThreadIsValid( pThread));
  ASSERT( !PlatformIsCritical());

  /* Remove from runnable queue */
  if ( !DEQUE_IS_EMPTY( &pThread->queue) )
    SWS_DequeRemove( &pThread->queue);

  if ( s_pMainThread == pThread)
    {
    SWS_ThreadCleanup();
    exit( (int)iExit);
    }

  /* Dispatch highest priority runnable thread */
  s_pCurrentThread = NULL;
  ThreadDispatch();

  /* Should never get here */
  ASSERT( !"Zombie thread");
  }


/*
 * Resume a thread
 */
SWS_EXPORT( int, SWS_ThreadStart( SWS_ThreadHandle hThread))
  {
  TRACE( 8, ("SWS_ThreadStart(%p)\n", hThread));
  ASSERT( ThreadIsValid( hThread) );

  if ( DEQUE_IS_EMPTY( &hThread->queue))
    SWS_DequeAppend( &s_runnable, &hThread->queue);

  return 0;
  }


/*
 * Suspend a thread
 */
SWS_EXPORT( int, SWS_ThreadStop( SWS_ThreadHandle hThread))
  {
  TRACE( 8, ("SWS_ThreadStop(%p)\n", hThread));
  ASSERT( ThreadIsValid( hThread) );

  if ( hThread == SWS_ThreadCurrent() )
    {
    /* Current thread stopped so dispatch highest priority runnable thread */
    ASSERT( !PlatformIsCritical());
    ThreadDispatch();
    }
  else if ( !DEQUE_IS_EMPTY( &hThread->queue))
    {
    SWS_DequeRemove( &hThread->queue);
    }

  return 0;
  }


/*
 * Get handle for current thread
 */
SWS_EXPORT( SWS_ThreadHandle, SWS_ThreadCurrent( void))
  {
  TRACE( 9, ("SWS_ThreadCurrent()\n"));
  return s_pCurrentThread;
  }


/*
 * Switch to the next runnable thread
 */
SWS_EXPORT( void, SWS_ThreadYield( void))
  {
  SWS_Thread* pThread;

  TRACE( 8, ("SWS_ThreadDispatch()\n"));

  pThread = s_pCurrentThread;
  if ( NULL != pThread)
    {
    ASSERT( ThreadIsValid( pThread) );

    if ( !PlatformIsCritical() )
      {
      /* Any other runnable thread? */
      if ( !DEQUE_IS_EMPTY( &s_runnable) )
        {
        /* Add the current thread to the runnable queue */
        if ( DEQUE_IS_EMPTY( &pThread->queue) )
          SWS_DequeAppend( &s_runnable, &pThread->queue);

        ThreadDispatch();
        }
      }
    }
  }


/****************************************************************************
 * Private functions
 ****************************************************************************/

/*
 * Switch to the next runnable thread
 */
void ThreadDispatch( void)
  {
  SDeque* pItem;
  SWS_Thread* pNextThread, *pThread;

  TRACE( 9, ("ThreadDispatch()\n"));
  ASSERT( !PlatformIsCritical());

  pThread = s_pCurrentThread;
  s_pCurrentThread = NULL;
  TRACE( 4, ("ThreadDispatch() Suspend %p\n",pThread));

  /* Get highest priorty runnable thread */
  while ( &s_runnable == (pItem = DEQUE_FIRST( &s_runnable)) )
    {
    /* Idle until any thread ready to run */
    PlatformIdle();
    }
  pNextThread = THREAD( SWS_DequeRemove( pItem));
  TRACE( 4, ("ThreadDispatch() Resume %p\n", pNextThread));

  ASSERT( ThreadIsValid( pNextThread) );
  ASSERT( DEQUE_IS_EMPTY( &pNextThread->queue));
  s_pCurrentThread = pNextThread;

  if ( pThread != pNextThread )
    {
    PlatformThreadSwitch( pThread, pNextThread);
    }
  }

#else
char SWS_Thread;
#endif /* OPT_THREADS == OPT_THREADS_COOP */
/* eof */
