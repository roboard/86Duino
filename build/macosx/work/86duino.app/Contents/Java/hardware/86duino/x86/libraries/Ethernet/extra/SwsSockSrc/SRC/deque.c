/************************************************************************
 * SwsSock                                                              *
 *                                                                      *
 * Double ended queue management                                        *
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

/* Exports */
#include "deque.h"

/* Imports */
#include "debug.h"


/*
 * Add an item to the head of a deque
 */
void SWS_DequePrepend( SDeque* const pDeque, SDeque* const pItem)
  {
  ASSERT( NULL != pDeque );
  ASSERT( NULL != pItem );
  ASSERT( DEQUE_IS_EMPTY( pItem) );

  pItem->head = pDeque;
  pItem->tail = pDeque->tail;

  pDeque->tail->head = pItem;
  pDeque->tail = pItem;
  }


/*
 * Add an item to the tail of a deque
 */
void SWS_DequeAppend( SDeque* const pDeque, SDeque* const pItem)
  {
  ASSERT( NULL != pDeque );
  ASSERT( NULL != pItem );
  ASSERT( DEQUE_IS_EMPTY( pItem) );

  pItem->tail = pDeque;
  pItem->head = pDeque->head;

  pDeque->head->tail = pItem;
  pDeque->head = pItem;
  }


/*
 * Remove an item from a deque
 */
SDeque* SWS_DequeRemove( SDeque* const pItem)
  {
  ASSERT( NULL != pItem );
  ASSERT( !DEQUE_IS_EMPTY( pItem) );

  pItem->head->tail = pItem->tail;
  pItem->tail->head = pItem->head;

  DEQUE_INIT( pItem);

  return pItem;
  }

/* eof */
