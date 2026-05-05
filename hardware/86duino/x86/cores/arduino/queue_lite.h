#ifndef __WZ_QUEUE_H
#define __WZ_QUEUE_H

#include <stdlib.h>
#include <string.h>

typedef struct {
    
    unsigned int head;
    unsigned int tail;
    unsigned int size;
    unsigned char data[0];

} queue_t;

static inline queue_t *queue_create(unsigned int n)
{
    queue_t *q;
    unsigned int size = 2;

    if (n <= 0)
        return NULL;

    while (size <= n)
        size *= 2;
    
    q = (queue_t*)malloc(sizeof(queue_t) + size);
    if (q)
        memset(q, 0, sizeof(queue_t) + size);
    q->size = size;
    
    return q;
}

static inline void queue_destory(queue_t *q)
{
    free(q);
}

static inline unsigned int queue_size(queue_t *q)
{
    return (q->head <= q->tail) ? (q->tail - q->head) : (q->tail + q->size - q->head);  
}

static inline unsigned int queue_free(queue_t *q)
{
    return (q->head <= q->tail) ? (q->size + q->head - q->tail - 1) : (q->head - q->tail - 1);
}

static inline bool queue_empty(queue_t *q)
{
    return q->head == q->tail;
}

static inline bool queue_full(queue_t *q)
{
    return q->head == ((q->tail + 1) & (q->size - 1));
}

static inline void queue_push(queue_t *q, unsigned char data)
{
    q->data[q->tail] = data;
    q->tail = (q->tail + 1) & (q->size - 1);
}

static inline unsigned char queue_pop(queue_t *q)
{
    unsigned char val = q->data[q->head];
    q->head = (q->head + 1) & (q->size - 1);
    return val;
}

static inline void queue_push_seq(queue_t *q, unsigned char *buf, unsigned int len)
{
    unsigned int n1, n2;

    n1 = q->size - q->tail;
    if (n1 > len)
        n1 = len;
    n2 = len - n1;

    if (n1 > 0)
        memcpy(&q->data[q->tail], &buf[0], n1);
    if (n2 > 0)
        memcpy(&q->data[0], &buf[n1], n2);

    q->tail = (q->tail + len) & (q->size - 1);
}

static inline void queue_pop_seq(queue_t *q, unsigned char *buf, unsigned int len)
{
    unsigned int n1, n2;

    n1 = q->size - q->head;
    if (n1 > len)
        n1 = len;
    n2 = len - n1;

    if (n1 > 0)
        memcpy(&buf[0], &q->data[q->head], n1);
    if (n2 > 0)
        memcpy(&buf[n1], &q->data[0], n2);

    q->head = (q->head + len) & (q->size - 1);
}

static inline void queue_clear(queue_t *q)
{
    q->head = q->tail;
}

#endif
