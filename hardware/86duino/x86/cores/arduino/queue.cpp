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

#define __DMP_QUEUE_LIB

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "queue.h"
#include "io.h"

#if defined DMP_DOS_DJGPP
static volatile bool locked = false;
DMP_INLINE(void) lock_queue_functions(void);
#endif

DMPAPI(Queue *) CreateBufQueue(unsigned int size, unsigned int dsize)
{
	Queue *queue = (Queue *)ker_Malloc(sizeof(Queue));
	
	if ((queue->data = (void *)ker_Malloc(dsize * size)) == NULL)
	{
		ker_Mfree((void*)queue);
		return NULL;
	}
	
	io_DisableINT();
	{
		queue->head  = 0;
		queue->tail  = 0;
		queue->count = 0;
		queue->size  = size;
		queue->dsize = dsize;
		queue->total = dsize*size;
		queue->near_full_size = 3 * (size >> 2);
		queue->empty = true;
		queue->full  = false;
		queue->near_full = false;
	}
	io_RestoreINT();
	
	#if defined DMP_DOS_DJGPP
	if (locked == false) lock_queue_functions();
	#endif
	
	return queue;
}

DMPAPI(Queue *) CreateQueue(unsigned int size)
{
	return CreateBufQueue(size, 1);
}

DMPAPI(void) DestoryQueue(Queue *queue)
{
	if (queue == NULL) return;
	if (queue->data != NULL) ker_Mfree(queue->data);
	ker_Mfree((void*)queue);
}

DMPAPI(void) ClearQueue(Queue *queue)
{
	if (queue == NULL) return;
	io_DisableINT();
	{
		queue->head = queue->tail = queue->count = 0;
	}
	io_RestoreINT();
}
#if defined DMP_DOS_DJGPP
DPMI_END_OF_LOCKED_STATIC_FUNC(ClearQueue)
#endif

DMPAPI(bool) PushQueue(Queue *queue, unsigned char ch)
{
	bool success;
	
	io_DisableINT();
	{
		queue->temp = ch;
		success = PushBufQueue(queue, (void*)&queue->temp);
	}
	io_RestoreINT();
	
	return success;
}
#if defined DMP_DOS_DJGPP
DPMI_END_OF_LOCKED_STATIC_FUNC(PushQueue)
#endif

DMPAPI(bool) PushBufQueue(Queue *queue, void *buf)
{
	if (queue->count >= queue->size) return false;
	
	io_DisableINT();
	{
		memcpy((unsigned char *)queue->data + queue->tail, buf, queue->dsize);
		queue->tail += queue->dsize;
		if (queue->tail > (queue->total - queue->dsize)) queue->tail = 0;
		queue->count++;
		
		if (queue->count > 0) queue->empty = false;
		if (queue->count >= queue->near_full_size) queue->near_full = true;
		if (queue->count >= queue->size) queue->full = true;
	}
	io_RestoreINT();
	
	return true;
}
#if defined DMP_DOS_DJGPP
DPMI_END_OF_LOCKED_STATIC_FUNC(PushBufQueue)
#endif

DMPAPI(unsigned int) PopQueue(Queue *queue)
{
	unsigned char ch;
	
	io_DisableINT();
	{
		if (PopBufQueue(queue, (void *)(&queue->temp)) == false)
		{
			io_RestoreINT();
			return -1;
		}
		ch = queue->temp;
	}
	io_RestoreINT();
	
	return (unsigned int)ch;
}
#if defined DMP_DOS_DJGPP
DPMI_END_OF_LOCKED_STATIC_FUNC(PopQueue)
#endif

DMPAPI(bool) PopBufQueue(Queue *queue, void *buf)
{
	if (queue->count <= 0) return false;
	
	io_DisableINT();
	{
		memcpy(buf, (unsigned char *)queue->data + queue->head, queue->dsize);
		queue->head += queue->dsize;
		if (queue->head > (queue->total - queue->dsize)) queue->head = 0;
		queue->count--;
		
		if (queue->count < queue->size) queue->full = false;
		if (queue->count < queue->near_full_size) queue->near_full = false;
		if (queue->count <= 0) queue->empty = true;
	}
	io_RestoreINT();
	
	return true;
}
#if defined DMP_DOS_DJGPP
DPMI_END_OF_LOCKED_STATIC_FUNC(PopBufQueue)
#endif

DMPAPI(unsigned int) PeekQueue(Queue *queue)
{
	unsigned char ch;
	
	io_DisableINT();
	{
		if (PeekBufQueue(queue, (void *)(&queue->temp)) == false)
		{
			io_RestoreINT();
			return -1;
		}
		ch = queue->temp;
	}
	io_RestoreINT();
	
	return (unsigned int)ch;
}
#if defined DMP_DOS_DJGPP
DPMI_END_OF_LOCKED_STATIC_FUNC(PeekQueue)
#endif

DMPAPI(bool) PeekBufQueue(Queue *queue, void *buf)
{
	if (queue->count <= 0) return false;
	
	io_DisableINT();
	{
		memcpy(buf, (unsigned char *)queue->data + queue->head, queue->dsize);
	}
	io_RestoreINT();
	
	return true;
}
#if defined DMP_DOS_DJGPP
DPMI_END_OF_LOCKED_STATIC_FUNC(PeekBufQueue)
#endif

DMPAPI(bool) QueueEmpty(Queue *queue)
{
	return queue->empty;
}
#if defined DMP_DOS_DJGPP
DPMI_END_OF_LOCKED_STATIC_FUNC(QueueEmpty)
#endif

DMPAPI(bool) QueueFull(Queue *queue)
{
	return queue->full;
}
#if defined DMP_DOS_DJGPP
DPMI_END_OF_LOCKED_STATIC_FUNC(QueueFull)
#endif

DMPAPI(bool) QueueNearlyFull(Queue *queue)
{
	return queue->near_full;
}
#if defined DMP_DOS_DJGPP
DPMI_END_OF_LOCKED_STATIC_FUNC(QueueNearlyFull)
#endif

#if defined DMP_DOS_DJGPP
DMP_INLINE(void) lock_queue_functions(void)
{
	DPMI_LOCK_FUNC(ClearQueue);
	DPMI_LOCK_FUNC(PushQueue);
	DPMI_LOCK_FUNC(PushBufQueue);
	DPMI_LOCK_FUNC(PopQueue);
	DPMI_LOCK_FUNC(PopBufQueue);
	DPMI_LOCK_FUNC(PeekQueue);
	DPMI_LOCK_FUNC(PeekBufQueue);
	DPMI_LOCK_FUNC(QueueEmpty);
	DPMI_LOCK_FUNC(QueueFull);
	DPMI_LOCK_FUNC(QueueNearlyFull);
	locked = true;
}
#endif
