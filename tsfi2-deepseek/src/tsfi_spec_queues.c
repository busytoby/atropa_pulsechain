#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tsfi_spec_queues.h"

// 1. Lock-Free Priority Ring Buffer
void tsfi_ring_init(TSFiLockFreeRing *ring) {
    if (!ring) return;
    ring->write_idx = 0;
    ring->read_idx = 0;
    memset(ring->items, 0, sizeof(ring->items));
}

int tsfi_ring_push(TSFiLockFreeRing *ring, int priority, int keycode) {
    if (!ring) return -1;
    int current_w = ring->write_idx;
    int next_w = (current_w + 1) % SPEC_QUEUE_MAX;
    if (next_w == ring->read_idx) {
        return -1; // Ring buffer full
    }
    
    // Write item
    ring->items[current_w].priority = priority;
    ring->items[current_w].keycode = keycode;
    
    // Atomic write index release
    __sync_synchronize();
    ring->write_idx = next_w;
    return 0;
}

int tsfi_ring_pop(TSFiLockFreeRing *ring, TSFiRingItem *item_out) {
    if (!ring || !item_out) return -1;
    int current_r = ring->read_idx;
    if (current_r == ring->write_idx) {
        return -1; // Ring buffer empty
    }
    
    // Copy item
    *item_out = ring->items[current_r];
    
    // Atomic read index release
    __sync_synchronize();
    ring->read_idx = (current_r + 1) % SPEC_QUEUE_MAX;
    return 0;
}

// Helper to bubble up intrusive node
static void intrusive_bubble_up(TSFiIntrusiveHeap *heap, int i) {
    while (i > 0) {
        int parent = (i - 1) / 2;
        if (heap->nodes[i]->priority >= heap->nodes[parent]->priority) {
            break;
        }
        // Swap nodes
        TSFiIntrusiveNode *temp = heap->nodes[i];
        heap->nodes[i] = heap->nodes[parent];
        heap->nodes[parent] = temp;
        
        // Update heap indices
        heap->nodes[i]->heap_index = i;
        heap->nodes[parent]->heap_index = parent;
        
        i = parent;
    }
}

// Helper to bubble down intrusive node
static void intrusive_bubble_down(TSFiIntrusiveHeap *heap, int i) {
    while (2 * i + 1 < heap->size) {
        int left = 2 * i + 1;
        int right = left + 1;
        int smallest = left;
        if (right < heap->size && heap->nodes[right]->priority < heap->nodes[left]->priority) {
            smallest = right;
        }
        if (heap->nodes[i]->priority <= heap->nodes[smallest]->priority) {
            break;
        }
        // Swap nodes
        TSFiIntrusiveNode *temp = heap->nodes[i];
        heap->nodes[i] = heap->nodes[smallest];
        heap->nodes[smallest] = temp;
        
        // Update heap indices
        heap->nodes[i]->heap_index = i;
        heap->nodes[smallest]->heap_index = smallest;
        
        i = smallest;
    }
}

// 2. Intrusive Heap
void tsfi_intrusive_heap_init(TSFiIntrusiveHeap *heap) {
    if (!heap) return;
    heap->size = 0;
    memset(heap->nodes, 0, sizeof(heap->nodes));
}

int tsfi_intrusive_heap_push(TSFiIntrusiveHeap *heap, TSFiIntrusiveNode *node) {
    if (!heap || !node || heap->size >= SPEC_QUEUE_MAX) return -1;
    
    int i = heap->size++;
    heap->nodes[i] = node;
    node->heap_index = i;
    
    intrusive_bubble_up(heap, i);
    return 0;
}

void tsfi_intrusive_heap_update(TSFiIntrusiveHeap *heap, TSFiIntrusiveNode *node, int new_priority) {
    if (!heap || !node) return;
    int idx = node->heap_index;
    if (idx < 0 || idx >= heap->size || heap->nodes[idx] != node) return;
    
    int old_priority = node->priority;
    node->priority = new_priority;
    
    if (new_priority < old_priority) {
        intrusive_bubble_up(heap, idx);
    } else {
        intrusive_bubble_down(heap, idx);
    }
}

TSFiIntrusiveNode* tsfi_intrusive_heap_pop(TSFiIntrusiveHeap *heap) {
    if (!heap || heap->size <= 0) return NULL;
    
    TSFiIntrusiveNode *root = heap->nodes[0];
    root->heap_index = -1;
    
    heap->size--;
    if (heap->size > 0) {
        heap->nodes[0] = heap->nodes[heap->size];
        heap->nodes[0]->heap_index = 0;
        intrusive_bubble_down(heap, 0);
    }
    
    return root;
}

// 3. Monotonic Buckets
void tsfi_monotonic_buckets_init(TSFiMonotonicBuckets *mb) {
    if (!mb) return;
    mb->min_bucket_idx = 0;
    memset(mb->buckets, 0, sizeof(mb->buckets));
}

int tsfi_monotonic_buckets_push(TSFiMonotonicBuckets *mb, int priority, int keycode) {
    if (!mb) return -1;
    int bucket_idx = priority % BUCKET_COUNT;
    
    TSFiBucketItem *item = calloc(1, sizeof(TSFiBucketItem));
    if (!item) return -1;
    item->keycode = keycode;
    
    // Append to bucket link
    item->next = mb->buckets[bucket_idx];
    mb->buckets[bucket_idx] = item;
    return 0;
}

int tsfi_monotonic_buckets_pop(TSFiMonotonicBuckets *mb, int *keycode_out) {
    if (!mb || !keycode_out) return -1;
    
    // Scan buckets starting from min_bucket_idx
    for (int i = 0; i < BUCKET_COUNT; i++) {
        int idx = (mb->min_bucket_idx + i) % BUCKET_COUNT;
        if (mb->buckets[idx]) {
            mb->min_bucket_idx = idx;
            TSFiBucketItem *item = mb->buckets[idx];
            mb->buckets[idx] = item->next;
            
            *keycode_out = item->keycode;
            free(item);
            return 0;
        }
    }
    return -1; // All buckets empty
}
