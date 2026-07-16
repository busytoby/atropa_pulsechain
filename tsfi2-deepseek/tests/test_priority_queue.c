#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "tsfi_priority_queue.h"

int main(void) {
    printf("[Priority Queue] Starting binary heap priority queue tests...\n");
    fflush(stdout);

    TSFiPriorityQueue pq;
    tsfi_priority_queue_init(&pq);

    // 1. Push items out-of-order
    int res = tsfi_priority_queue_push(&pq, 10, 32, "LOW_PRIO");
    assert(res == 0);
    res = tsfi_priority_queue_push(&pq, 1, 30, "HIGH_PRIO");
    assert(res == 0);
    res = tsfi_priority_queue_push(&pq, 5, 31, "MED_PRIO");
    assert(res == 0);

    assert(pq.size == 3);

    // 2. Pop and assert min-heap order (lowest priority value is highest priority)
    TSFiQueueItem item;
    
    res = tsfi_priority_queue_pop(&pq, &item);
    assert(res == 0);
    printf("  [PQ Pop 1] Keycode: %d, Priority: %d (Expected Keycode: 30, Priority: 1)\n", item.keycode, item.priority);
    fflush(stdout);
    assert(item.keycode == 30 && item.priority == 1);

    res = tsfi_priority_queue_pop(&pq, &item);
    assert(res == 0);
    printf("  [PQ Pop 2] Keycode: %d, Priority: %d (Expected Keycode: 31, Priority: 5)\n", item.keycode, item.priority);
    fflush(stdout);
    assert(item.keycode == 31 && item.priority == 5);

    res = tsfi_priority_queue_pop(&pq, &item);
    assert(res == 0);
    printf("  [PQ Pop 3] Keycode: %d, Priority: %d (Expected Keycode: 32, Priority: 10)\n", item.keycode, item.priority);
    fflush(stdout);
    assert(item.keycode == 32 && item.priority == 10);

    assert(pq.size == 0);

    printf("[PASS] Priority queue verified successfully!\n");
    fflush(stdout);
    return 0;
}
