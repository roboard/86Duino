/*
  Copyright (c) 2019 Johnson Hung <Dyhung@dmp.com.tw>. All right reserved.

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

#include <stdlib.h>
#include <string.h>
#include "dmpcfg.h"
#include "io.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _queue
{
    volatile unsigned long head;    // The header of the queue
    volatile unsigned long tail;    // The tailer of the queue
    unsigned long size;             // Maximum count of the queue
    unsigned long dsize;            // Size of data element

    void *data;                     // The pointer of data queue.

} Queue;

DMP_INLINE(Queue *) CreateBufQueue(unsigned long size, unsigned long dsize)
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

DMP_INLINE(Queue *) CreateQueue(unsigned long size)
{
    return CreateBufQueue(size, 1);
}

DMP_INLINE(void) DestoryQueue(Queue *queue)
{
    if (queue == NULL)
        return;
    if (queue->data != NULL)
        free((void *)queue->data);
    free((void *)queue);
}

DMP_INLINE(void) ClearQueue(Queue *queue)
{
    if (queue == NULL)
        return;

    io_DisableINT();
    queue->head = 0;
    queue->tail = 0;
    io_RestoreINT();
}

DMP_INLINE(unsigned long) QueueSize(Queue *queue)
{
    unsigned long head, tail;

    io_DisableINT();
    head = queue->head;
    tail = queue->tail;
    io_RestoreINT();

    return (head <= tail) ? (tail - head) : (tail + queue->size - head);
}

DMP_INLINE(unsigned long) QueueFree(Queue *queue)
{
    unsigned long head, tail;

    io_DisableINT();
    head = queue->head;
    tail = queue->tail;
    io_RestoreINT();

    return (head <= tail) ? (queue->size - 1 - tail + head) : (head - tail - 1);
}

DMP_INLINE(bool) QueueEmpty(Queue *queue)
{
    return QueueSize(queue) <= 0;
}

DMP_INLINE(bool) QueueFull(Queue *queue)
{
    return QueueSize(queue) >= (queue->size - 1);
}

DMP_INLINE(unsigned int) PushQueueSeq(Queue *queue, void *buf, size_t len)
{
    unsigned long head, tail, free, tlen;
    
    io_DisableINT();
    head = queue->head;
    tail = queue->tail;
    free = (head <= tail) ? (queue->size - 1 - tail + head) : (head - tail - 1);
    if (len > free)
        len = free;
    tlen = queue->size - queue->tail;
    if (tlen > len)
        tlen = len;
    if (tlen > 0)
        memcpy(&((unsigned char*)queue->data)[queue->tail*queue->dsize], buf, tlen*queue->dsize);
    if (len - tlen > 0)
        memcpy(&((unsigned char*)queue->data)[0], &((unsigned char*)buf)[tlen*queue->dsize], (len - tlen)*queue->dsize);
    queue->tail = queue->tail + len;
    if (queue->tail >= queue->size)
        queue->tail -= queue->size;
    io_RestoreINT();
    
    return len;
}

DMP_INLINE(bool) PushQueue(Queue *queue, unsigned char ch)
{
    bool success = false;

    io_DisableINT();
    if (QueueFull(queue) == false) {
        ((unsigned char*)queue->data)[queue->tail] = ch;
        queue->tail++;
        if (queue->tail >= queue->size)
            queue->tail = 0;
        success = true;
    }
    io_RestoreINT();

    return success;
}

DMP_INLINE(bool) PushBufQueue(Queue *queue, void *buf)
{
	bool ret = false;
    unsigned long i;
	
	io_DisableINT();
	if (QueueFull(queue) == false) {
        for (i = 0; i < queue->dsize; i++) {
            ((unsigned char *)queue->data + queue->tail*queue->dsize)[i] = ((unsigned char *)buf)[i];
        }
        queue->tail += 1;
        if (queue->tail >= queue->size) queue->tail = 0;
		ret = true;
	}
	io_RestoreINT();
	
	return ret;
}

DMP_INLINE(unsigned int) PeekQueue(Queue *queue)
{
    unsigned int ch = 0xffff;

    io_DisableINT();
	if (QueueEmpty(queue) == false) {
        ch = ((unsigned char *)queue->data)[queue->head];
	}
    io_RestoreINT();

    return ch;
}

DMP_INLINE(bool) PeekBufQueue(Queue *queue, void *buf)
{
	bool ret = false;
    unsigned long i;
	
	io_DisableINT();
	if (QueueEmpty(queue) == false) {
        for (i = 0; i < queue->dsize; i++) {
            ((unsigned char *)buf)[i] = ((unsigned char *)queue->data + queue->head*queue->dsize)[i];
        }
		ret = true;
	}
	io_RestoreINT();
	
	return ret;
}


DMP_INLINE(unsigned int) PopQueueSeq(Queue *queue, void *buf, size_t len)
{
    unsigned long head, tail, size, tlen;
    
    io_DisableINT();
    head = queue->head;
    tail = queue->tail;
    size = (head <= tail) ? (tail - head) : (tail + queue->size - head);
    if (len > size)
        len = size;
    tlen = queue->size - queue->head;
    if (tlen > len)
        tlen = len;
    if (tlen > 0)
        memcpy(buf, &((unsigned char*)queue->data)[queue->head*queue->dsize], tlen*queue->dsize);
    if (len - tlen > 0)
        memcpy(&((unsigned char*)buf)[tlen*queue->dsize], &((unsigned char*)queue->data)[0], (len - tlen)*queue->dsize);
    queue->head = queue->head + len;
    if (queue->head >= queue->size)
        queue->head -= queue->size;
    io_RestoreINT();
    
    return len;
}

DMP_INLINE(unsigned int) PopQueue(Queue *queue)
{
    unsigned int ch = 0xffff;

    io_DisableINT();
	if (QueueEmpty(queue) == false) {
        ch = ((unsigned char *)queue->data)[queue->head];
        queue->head += 1;
        if (queue->head >= queue->size) queue->head = 0;
	}
    io_RestoreINT();

    return ch;
}

DMP_INLINE(bool) PopBufQueue(Queue *queue, void *buf)
{
	bool ret = false;
	
	io_DisableINT();
	if (PeekBufQueue(queue, buf) == true) {
        queue->head += 1;
        if (queue->head >= queue->size) queue->head = 0;
		ret = true;
	}
	io_RestoreINT();
	
	return ret;
}

#ifdef __cplusplus
}
#endif

#endif
