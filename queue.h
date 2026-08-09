#ifndef QUEUE_H
#define QUEUE_H

#include <stdbool.h>

typedef struct {
    int x;
    int y;
} CellPos;

typedef struct {
    CellPos *data;
    int capacity;
    int front;
    int rear;
    int count;
} Queue;

bool queue_init(Queue *q, int capacity);
void queue_free(Queue *q);
bool queue_enqueue(Queue *q, CellPos pos);
bool queue_dequeue(Queue *q, CellPos *out);
bool queue_is_empty(const Queue *q);
bool queue_is_full(const Queue *q);

#endif // QUEUE_H