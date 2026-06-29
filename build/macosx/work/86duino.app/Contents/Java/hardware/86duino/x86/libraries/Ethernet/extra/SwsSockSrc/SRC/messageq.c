/****************************************************************************
 *
 * File name   : messageq.c
 * Function    : Message queue API
 * Project     : SwsSock
 * Authors     : Lawrence Rust
 * Systems     : ANSI C.
 *
 ****************************************************************************
 *
 * Created by Lawrence Rust, Software Systems Consultants
 * lvr@softsystem.co.uk. Tel/Fax +33 5 49 72 79 63
 *
 * 11-Apr-01, LVR, Created.
 * 03-Oct-01, LVR, Fix TRACE warnings for ARM SDT 2.51 .
 *
 * Comments:
 * --------
 * This file defines the interface to message queues
 *
 ****************************************************************************
 */

/* Compilation options */
#include "config.h"

/* Exports */
#include "sws_msgq.h"

/* Imports */

#include "platform.h"
#include <stddef.h>
#include <stdlib.h>   /* calloc */
#include <string.h>

#include "sws_sock.h" /* For SWS_Exxx error codes */

#include "debug.h"


/*
 * Macros & Constants
 */


/*
 * Local data types
 */

/* Message queue instance */
typedef struct SWS_MsgQ
  {
  ui32 uMagic;
#define MSGQ_MAGIC SWS_MAKEFOURCC( 'm','s','g','q')

  size_t uLen;                        /* No. messages */
  size_t size;                        /* Size of each message */

  /* Read/write cursors */
  Atomic_t aWrite, aRead;

  /* The queue of messages */
  SWS_VARRAY( char, messages);        /* VARIABLE LENGTH */
  } SWS_MsgQ;


/*
 * Prototypes
 */


/*
 * Module data
 */


/*
 * Create a messageQ
 */
SWS_EXPORT( SWS_MsgqH, SWS_MsgqCreate( /* Returns NULL on error */
  size_t messages,
  size_t size                          /* IN: Message size */
)){
  SWS_MsgQ* pQ;

  TRACE( 9, ("SWS_MsgqCreate(%u,%u)\n", messages, size));

  /* Allocate a messageQ */
  ++messages;
  pQ = (SWS_MsgQ*) malloc( sizeof(*pQ) + messages * size );
  if ( NULL == pQ)
    {
    TRACE( 0, ("!!! SWS_MsgqCreate() out of memory\n"));
    return NULL;
    }

  pQ->uLen = messages;
  pQ->size = size;
  AtomicWrite( &pQ->aRead, 0);
  AtomicWrite( &pQ->aWrite , 0);
  pQ->uMagic = MSGQ_MAGIC;

  return pQ;
  }


/*
 * Destroy a messageQ
 */
SWS_EXPORT( void, SWS_MsgqDestroy(
  SWS_MsgqH pQ                        /* IN: MessageQ */
)){
  TRACE( 9, ("SWS_MsgqDestroy(%p)\n", pQ));
  ASSERT( NULL != pQ);

  if ( MSGQ_MAGIC != pQ->uMagic)
    {
    TRACE( 1, ("*** SWS_MsgqDestroy() invalid messageQ\n"));
    return;
    }

  pQ->uMagic = 0;
  free( pQ);
  }


/*
 * Add a message
 * !!! CAUTION IRQ TIME !!!
 * Safe for concurrent put/get access
 * Unsafe for concurrent put access to same queue
 */
SWS_EXPORT( int, SWS_MsgqPut(         /* Returns !0 on error */
  SWS_MsgqH pQ,                       /* IN: MessageQ */
  size_t size,                        /* IN: Message size (for checking) */
  SWS_PTR( const void) pMsg           /* IN: -> Message */
)){
  TRACE( 10, ("SWS_MsgqPut(%p,%u,%p)\n", pQ,size,pMsg));
  ASSERT( NULL != pQ);
  ASSERT( NULL != pMsg);
  ASSERT( MSGQ_MAGIC == pQ->uMagic);
  ASSERT( size == pQ->size);

  if ( MSGQ_MAGIC == pQ->uMagic)
    {
    size_t indx = (size_t) AtomicRead( &pQ->aWrite);
    void* const pSlot = pQ->messages + indx * size;

    /* Bump write indx */
    if ( ++indx >= pQ->uLen)
      indx = 0;

    /* Check for overflow */
    if ( indx != (size_t)AtomicRead( &pQ->aRead))
      {
      /* Store message */
      memcpy( pSlot, pMsg, size);

      /* Update write indx */
      AtomicWrite( &pQ->aWrite, (int)indx);

      return 0;
      }

    TRACE( 10, ("SWS_MsgqPut(%p) full\n", pQ));
    return SWS_ENOBUFS;
    }

  TRACE( 1, ("*** SWS_MsgqPut(%p) invalid messageQ\n", pQ));
  return SWS_EINVAL;
  }


/*
 * Get a message
 * Safe for concurrent put/get access
 * Unsafe for concurrent get access to same queue
 */
SWS_EXPORT( int, SWS_MsgqGet(         /* Returns !0 on error */
  SWS_MsgqH pQ,                       /* IN: MessageQ */
  size_t size,                        /* IN: Message size (for checking) */
  SWS_PTR( void) pMsg                 /* OUT: Message */
)){
  TRACE( 9, ("SWS_MsgqGet(%p,%u,%p)\n", pQ,size,pMsg));
  ASSERT( NULL != pQ);
  ASSERT( NULL != pMsg);
  ASSERT( MSGQ_MAGIC == pQ->uMagic);
  ASSERT( size == pQ->size);

  if ( MSGQ_MAGIC == pQ->uMagic)
    {
    size_t indx = (size_t)AtomicRead( &pQ->aRead);
    if ( indx != (size_t)AtomicRead( &pQ->aWrite))
      {
      /* Get next message */
      memcpy( pMsg, pQ->messages + indx * size, size);

      /* Bump read indx */
      if ( ++indx >= pQ->uLen)
        indx = 0;

      /* Update read indx */
      AtomicWrite( &pQ->aRead, (int)indx);

      return 0;
      }

    TRACE( 6, ("MsgqGet(%p) messageQ empty\n", pQ));
    return SWS_EWOULDBLOCK; /* No messages */
    }

  TRACE( 1, ("*** SWS_MsgqGet(%p) invalid messageQ\n", pQ));
  return SWS_EINVAL;
  }


/*
 * Get no. queued items
 */
SWS_EXPORT( size_t, SWS_MsgqCount(    /* Returns no. queued items */
  SWS_MsgqH pQ                        /* IN: MessageQ */
)){
  size_t read, write;

  TRACE( 9, ("SWS_MsgqCount(%p)\n", pQ));

  read = (size_t)AtomicRead( &pQ->aRead);
  write = (size_t)AtomicRead( &pQ->aWrite);

  if ( write >= read)
    return (size_t)(write - read);

  ASSERT( (size_t)read < pQ->uLen);
  return (size_t)(pQ->uLen - read + write);
  }

/* End of file */
