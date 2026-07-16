#ifndef TSFI_PRIORITY_QUEUE_H
#define TSFI_PRIORITY_QUEUE_H

#include <stdint.h>

#define MAX_QUEUE_SIZE 128

// Queue Event Item
typedef struct {
    int priority; // Lower values represent higher priority
    int keycode;
    char payload[128];
} TSFiQueueItem;

// Priority Queue Context
typedef struct {
    TSFiQueueItem items[MAX_QUEUE_SIZE];
    int size;
} TSFiPriorityQueue;

// Initialize Priority Queue
void tsfi_priority_queue_init(TSFiPriorityQueue *pq);

// Push item into queue
int tsfi_priority_queue_push(TSFiPriorityQueue *pq, int priority, int keycode, const char *payload);

// Pop the highest priority item (lowest priority value)
int tsfi_priority_queue_pop(TSFiPriorityQueue *pq, TSFiQueueItem *item_out);

#endif // TSFI_PRIORITY_QUEUE_H
