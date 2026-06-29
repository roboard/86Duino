/************************************************************************
 * SwsSock                                                              *
 *                                                                      *
 * Double ended queue operations                                        *
 *                                                                      *
 * Created by Lawrence Rust, Software Systems Consultants               *
 * lvr@softsystem.co.uk. Tel/Fax +33 5 49 72 79 63                      *
 *______________________________________________________________________*
 *                                                                      *
 * Revision History:                                                    *
 *                                                                      *
 * No.   Date     By   Reason                                           *
 *----------------------------------------------------------------------*
 * 100 23 Aug 02  lvr  Created                                          *
 * 101 14 Dec 02  lvr  Version for SwsSock                              *
 ************************************************************************/
#ifndef SWS_DEQUE_H
#define SWS_DEQUE_H 0x0101 /* Version no. BCD */

/* Double ended queue item */
typedef struct SDeque
  {
  struct SDeque* head;
  struct SDeque* tail;
  } SDeque;

#define DEQUE_INIT( p)     ((p)->tail = (p)->head = (p))
#define DEQUE_IS_EMPTY( p) ((p)->tail == (p))
#define DEQUE_FIRST( p)    ((p)->tail)
#define DEQUE_LAST( p)     ((p)->head)


/*
 * Functions
 */
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

extern SDeque* SWS_DequeRemove( SDeque* const);
extern void SWS_DequeAppend( SDeque* const pDeque, SDeque* const pItem);
extern void SWS_DequePrepend( SDeque* const pDeque, SDeque* const pItem);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* ndef SWS_DEQUE_H */
/* End of file */


