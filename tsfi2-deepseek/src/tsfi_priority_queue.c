#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tsfi_priority_queue.h"

void tsfi_priority_queue_init(TSFiPriorityQueue *pq) {
    if (!pq) return;
    pq->size = 0;
    memset(pq->items, 0, sizeof(pq->items));
}

int tsfi_priority_queue_push(TSFiPriorityQueue *pq, int priority, int keycode, const char *payload) {
    if (!pq || pq->size >= MAX_QUEUE_SIZE) return -1;

    // Insert at the end
    int i = pq->size++;
    pq->items[i].priority = priority;
    pq->items[i].keycode = keycode;
    if (payload) {
        strncpy(pq->items[i].payload, payload, 127);
    } else {
        pq->items[i].payload[0] = '\0';
    }

    // Bubble Up
    while (i > 0) {
        int parent = (i - 1) / 2;
        if (pq->items[i].priority >= pq->items[parent].priority) {
            break;
        }
        // Swap
        TSFiQueueItem temp = pq->items[i];
        pq->items[i] = pq->items[parent];
        pq->items[parent] = temp;
        i = parent;
    }

    return 0;
}

int tsfi_priority_queue_pop(TSFiPriorityQueue *pq, TSFiQueueItem *item_out) {
    if (!pq || pq->size <= 0 || !item_out) return -1;

    // Retrieve root item
    *item_out = pq->items[0];

    // Move last item to root
    pq->size--;
    if (pq->size > 0) {
        pq->items[0] = pq->items[pq->size];
        
        // Bubble Down
        int i = 0;
        while (2 * i + 1 < pq->size) {
            int left = 2 * i + 1;
            int right = left + 1;
            int smallest = left;

            if (right < pq->size && pq->items[right].priority < pq->items[left].priority) {
                smallest = right;
            }
            if (pq->items[i].priority <= pq->items[smallest].priority) {
                break;
            }
            // Swap
            TSFiQueueItem temp = pq->items[i];
            pq->items[i] = pq->items[smallest];
            pq->items[smallest] = temp;
            i = smallest;
        }
    }

    return 0;
}
