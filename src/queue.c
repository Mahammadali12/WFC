#include "queue.h"
#include <stdlib.h>
#include <stdbool.h>

bool queue_init(Queue *q, int capacity)
{
    if (capacity <= 0 || q == NULL) {
        return false;
    }

    q->data = (CellPos *)malloc(sizeof(CellPos) * (size_t)capacity);
    if (q->data == NULL) {
        return false;
    }

    q->capacity = capacity;
    q->front = 0;
    q->rear = 0;
    q->count = 0;

    return true;
}

void queue_free(Queue *q)
{
    if (q != NULL && q->data != NULL) {
        free(q->data);
        q->data = NULL;
    }

    q->capacity = 0;
    q->front = 0;
    q->rear = 0;
    q->count = 0;
}

bool queue_is_empty(const Queue *q)
{
    if (q == NULL) {
        return true;
    }
    return q->count == 0;
}

bool queue_is_full(const Queue *q)
{
    if (q == NULL) {
        return true;
    }
    return q->count == q->capacity;
}

bool queue_enqueue(Queue *q, CellPos pos)
{
    if (q == NULL || queue_is_full(q)) {
        return false;
    }

    q->data[q->rear] = pos;
    q->rear = (q->rear + 1) % q->capacity;
    q->count++;

    return true;
}

bool queue_dequeue(Queue *q, CellPos *out)
{
    if (q == NULL || queue_is_empty(q)) {
        return false;
    }

    if (out != NULL) {
        *out = q->data[q->front];
    }

    q->front = (q->front + 1) % q->capacity;
    q->count--;

    return true;
}
