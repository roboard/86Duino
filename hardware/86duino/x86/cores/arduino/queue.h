/*
  Copyright (c) 2013 DY Hung <Dyhung@dmp.com.tw>. All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

  (If you need a commercial license, please contact soc@dmp.com.tw 
   to get more information.)
*/

#ifndef __DMP_QUEUE_H
#define __DMP_QUEUE_H

#include "dmpcfg.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef volatile struct _queue
{
	unsigned int head;			 // The header of the queue
	unsigned int tail;			 // The tailer of the queue
	unsigned int count;			 // Count of the queue
	unsigned int size;			 // Maximum count of the queue
	unsigned int dsize;			 // Size of data element
	unsigned int total;			 // Totoal size of data element
	unsigned int near_full_size; // If 'count' is bigger than 'near_full_size' or equal, 
	                             // shows that queue is nearly full.
	bool empty;
	bool full;
	bool near_full;
	
	unsigned char temp;

	void *data;                  // The pointer of data queue.
	
} Queue;

DMPAPI(Queue *) CreateQueue(unsigned int size); // only for 'dsize' = 1 
DMPAPI(Queue *) CreateBufQueue(unsigned int size, unsigned int dszie);
DMPAPI(void) DestoryQueue(Queue *queue);

DMPAPI(void) ClearQueue(Queue *queue);

// only for 'dsize' = 1 
DMPAPI(bool) PushQueue(Queue *queue, unsigned char ch);
DMPAPI(unsigned int) PopQueue(Queue *queue);
DMPAPI(unsigned int) PeekQueue(Queue *queue);

DMPAPI(bool) PushBufQueue(Queue *queue, void *buf);
DMPAPI(bool) PopBufQueue(Queue *queue, void *buf);
DMPAPI(bool) PeekBufQueue(Queue *queue, void *buf);

DMPAPI(bool) QueueEmpty(Queue *queue);
DMPAPI(bool) QueueFull(Queue *queue);
DMPAPI(bool) QueueNearlyFull(Queue *queue);

#ifdef __cplusplus
}
#endif

#endif
