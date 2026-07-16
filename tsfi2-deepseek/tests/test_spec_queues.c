#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "tsfi_spec_queues.h"

int main(void) {
    printf("[Specialized Queues] Starting lock-free, intrusive, and monotonic bucket queue tests...\n");
    fflush(stdout);

    // 1. Lock-Free Priority Ring Buffer
    TSFiLockFreeRing ring;
    tsfi_ring_init(&ring);
    
    int res = tsfi_ring_push(&ring, 10, 30);
    assert(res == 0);
    res = tsfi_ring_push(&ring, 5, 31);
    assert(res == 0);
    
    TSFiRingItem ring_item;
    res = tsfi_ring_pop(&ring, &ring_item);
    assert(res == 0 && ring_item.keycode == 30);
    printf("  [Lock-Free Ring] Popped item keycode: %d (Expected 30)\n", ring_item.keycode);
    fflush(stdout);

    // 2. Intrusive Heap Node
    TSFiIntrusiveHeap heap;
    tsfi_intrusive_heap_init(&heap);
    
    TSFiIntrusiveNode n1 = { .priority = 10, .keycode = 30, .heap_index = -1 };
    TSFiIntrusiveNode n2 = { .priority = 5, .keycode = 31, .heap_index = -1 };
    
    tsfi_intrusive_heap_push(&heap, &n1);
    tsfi_intrusive_heap_push(&heap, &n2);
    
    // Update n1's priority to 2 (should bubble to root)
    tsfi_intrusive_heap_update(&heap, &n1, 2);
    
    TSFiIntrusiveNode *popped_node = tsfi_intrusive_heap_pop(&heap);
    printf("  [Intrusive Heap] Popped keycode: %d, heap_index: %d (Expected Keycode: 30, Heap Index: -1)\n", 
           popped_node->keycode, popped_node->heap_index);
    fflush(stdout);
    assert(popped_node->keycode == 30);

    // 3. Monotonic Buckets
    TSFiMonotonicBuckets mb;
    tsfi_monotonic_buckets_init(&mb);
    
    tsfi_monotonic_buckets_push(&mb, 2, 40);
    tsfi_monotonic_buckets_push(&mb, 3, 41);
    
    int popped_keycode;
    res = tsfi_monotonic_buckets_pop(&mb, &popped_keycode);
    assert(res == 0 && popped_keycode == 40);
    printf("  [Monotonic Buckets] Popped keycode: %d (Expected 40)\n", popped_keycode);
    fflush(stdout);
    
    // Cleanup remaining allocated bucket item
    res = tsfi_monotonic_buckets_pop(&mb, &popped_keycode);
    assert(res == 0 && popped_keycode == 41);

    printf("[PASS] Lock-free, intrusive, and monotonic bucket priority queues verified successfully!\n");
    fflush(stdout);
    return 0;
}
