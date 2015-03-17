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
////////////////////////////////////////////////////////////////////////////////
//    note that most of functions in this lib assume no paging issue when 
//    using them in ISR; so to use this lib in ISR in DJGPP, it is suggested 
//    to employ PMODE/DJ or HDPMI instead of CWSDPMI.
////////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "queue.h"
#include "io.h"

DMPAPI(Queue *) CreateBufQueue(unsigned long size, unsigned long dsize)
{
	Queue *queue = (Queue *)malloc(sizeof(Queue));
	if (queue == NULL)
		return NULL;
		
	queue->data = (void *)malloc(dsize*(size + 1));
	if (queue->data == NULL) {
		free((void *)queue);
		return NULL;
	}
	
	queue->head  = 0;
	queue->tail  = 0;
	queue->size  = size + 1;
	queue->dsize = dsize;
	
	return queue;
}

DMPAPI(Queue *) CreateQueue(unsigned long size)
{
	return CreateBufQueue(size, 1);
}

DMPAPI(void) DestoryQueue(Queue *queue)
{
	if (queue == NULL)
		return;
	if (queue->data != NULL)
		free((void *)queue->data);
	free((void *)queue);
}

DMPAPI(void) ClearQueue(Queue *queue)
{
	if (queue == NULL)
		return;
	
	io_DisableINT();
	queue->head = 0;
	queue->tail = 0;
	io_RestoreINT();
}

DMPAPI(bool) PushQueue(Queue *queue, unsigned char ch)
{
	return PushBufQueue(queue, (void *)&ch);
}

DMPAPI(bool) PushBufQueue(Queue *queue, void *buf)
{
	bool ret = false;
	
	io_DisableINT();
	if (QueueFull(queue) == false) {
		memcpy((unsigned char *)queue->data + queue->tail*queue->dsize, buf, queue->dsize);
		queue->tail = (queue->tail + 1) % queue->size;
		ret = true;
	}
	io_RestoreINT();
	
	return ret;
}

DMPAPI(unsigned int) PopQueue(Queue *queue)
{
	unsigned char ch;
	
	if (PopBufQueue(queue, (void *)&ch) == false)
		return 0xffff;
	
	return (unsigned int)ch;
}

DMPAPI(bool) PopBufQueue(Queue *queue, void *buf)
{
	bool ret = false;
	
	io_DisableINT();
	if (PeekBufQueue(queue, buf) == true) {
		queue->head = (queue->head + 1) % queue->size;
		ret = true;
	}
	io_RestoreINT();
	
	return ret;
}

DMPAPI(unsigned int) PeekQueue(Queue *queue)
{
	unsigned char ch;
	
	if (PeekBufQueue(queue, (void *)&ch) == false)
		return 0xffff;
	
	return (unsigned int)ch;
}

DMPAPI(bool) PeekBufQueue(Queue *queue, void *buf)
{
	bool ret = false;
	
	io_DisableINT();
	if (QueueEmpty(queue) == false) {
		memcpy(buf, (unsigned char *)queue->data + queue->head*queue->dsize, queue->dsize);
		ret = true;
	}
	io_RestoreINT();
	
	return ret;
}

DMPAPI(bool) QueueEmpty(Queue *queue)
{
	unsigned long head, tail;
	
	io_DisableINT();
	head = queue->head;
	tail = queue->tail;
	io_RestoreINT();
	
	return (head == tail) ? (true) : (false);
}

DMPAPI(bool) QueueFull(Queue *queue)
{
	unsigned long head, tail;
	
	io_DisableINT();
	head = queue->head;
	tail = queue->tail;
	io_RestoreINT();
	
	if (((tail + 1) % queue->size) != head)
		return false;

	return true;
}

DMPAPI(unsigned long) QueueSize(Queue *queue)
{
	unsigned long head, tail;
	
	io_DisableINT();
	head = queue->head;
	tail = queue->tail;
	io_RestoreINT();

	return (head <= tail) ? (tail - head) : (tail + queue->size - head);
}
