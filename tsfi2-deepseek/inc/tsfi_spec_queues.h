#ifndef TSFI_SPEC_QUEUES_H
#define TSFI_SPEC_QUEUES_H

#include <stdint.h>
#include <stdbool.h>

#define SPEC_QUEUE_MAX 64
#define BUCKET_COUNT 16

// 1. Lock-Free Priority Ring Buffer Struct
typedef struct {
    int priority;
    int keycode;
} TSFiRingItem;

typedef struct {
    TSFiRingItem items[SPEC_QUEUE_MAX];
    volatile int write_idx;
    volatile int read_idx;
} TSFiLockFreeRing;

// 2. Intrusive Heap Node Struct
typedef struct TSFiIntrusiveNode {
    int priority;
    int keycode;
    int heap_index; // Tracks current index inside the heap array
} TSFiIntrusiveNode;

typedef struct {
    TSFiIntrusiveNode* nodes[SPEC_QUEUE_MAX];
    int size;
} TSFiIntrusiveHeap;

// 3. Monotonic Bucket Struct
typedef struct TSFiBucketItem {
    int keycode;
    struct TSFiBucketItem *next;
} TSFiBucketItem;

typedef struct {
    TSFiBucketItem *buckets[BUCKET_COUNT];
    int min_bucket_idx;
} TSFiMonotonicBuckets;


// --- API Declarations ---

// Lock-Free Ring Operations
void tsfi_ring_init(TSFiLockFreeRing *ring);
int tsfi_ring_push(TSFiLockFreeRing *ring, int priority, int keycode);
int tsfi_ring_pop(TSFiLockFreeRing *ring, TSFiRingItem *item_out);

// Intrusive Heap Operations
void tsfi_intrusive_heap_init(TSFiIntrusiveHeap *heap);
int tsfi_intrusive_heap_push(TSFiIntrusiveHeap *heap, TSFiIntrusiveNode *node);
void tsfi_intrusive_heap_update(TSFiIntrusiveHeap *heap, TSFiIntrusiveNode *node, int new_priority);
TSFiIntrusiveNode* tsfi_intrusive_heap_pop(TSFiIntrusiveHeap *heap);

// Monotonic Buckets Operations
void tsfi_monotonic_buckets_init(TSFiMonotonicBuckets *mb);
int tsfi_monotonic_buckets_push(TSFiMonotonicBuckets *mb, int priority, int keycode);
int tsfi_monotonic_buckets_pop(TSFiMonotonicBuckets *mb, int *keycode_out);

#endif // TSFI_SPEC_QUEUES_H
