#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <stdlib.h>

#define MAX_HEAP_SIZE 64

// System Event Types
typedef enum {
    EVENT_PLL_DRIFT = 1,
    EVENT_PMG_COLLISION = 2,
    EVENT_STACK_STORAGE_SYNC = 3
} EventType;

// Event Payload structure coordinating PLL, PMG, and Stack Storage
typedef struct {
    uint32_t priority;      // Lower priority value = higher precedence (Min-Heap)
    EventType type;         // PLL, PMG, or Storage
    uint64_t timestamp;     // PLL calibrated clock tick
    uint8_t data[32];       // Associated data (e.g. collision mask or keycode)
} CoordinatedEvent;

// Min-Heap (Priority Queue) structure
typedef struct {
    CoordinatedEvent data[MAX_HEAP_SIZE];
    uint32_t size;
} PriorityQueue;

// Initialize queue
void pq_init(PriorityQueue *pq) {
    pq->size = 0;
}

// Helper to swap events
void pq_swap(CoordinatedEvent *a, CoordinatedEvent *b) {
    CoordinatedEvent temp = *a;
    *a = *b;
    *b = temp;
}

// Push event into Min-Heap
bool pq_push(PriorityQueue *pq, CoordinatedEvent event) {
    if (pq->size >= MAX_HEAP_SIZE) return false;
    
    // Insert at the bottom leaf
    uint32_t i = pq->size;
    pq->data[i] = event;
    pq->size++;
    
    // Heapify up
    while (i > 0) {
        uint32_t parent = (i - 1) / 2;
        if (pq->data[i].priority < pq->data[parent].priority) {
            pq_swap(&pq->data[i], &pq->data[parent]);
            i = parent;
        } else {
            break;
        }
    }
    return true;
}

// Pop highest priority event from Min-Heap
bool pq_pop(PriorityQueue *pq, CoordinatedEvent *out_event) {
    if (pq->size == 0) return false;
    
    *out_event = pq->data[0];
    pq->size--;
    
    // Move last element to root
    pq->data[0] = pq->data[pq->size];
    
    // Heapify down
    uint32_t i = 0;
    while (2 * i + 1 < pq->size) {
        uint32_t left = 2 * i + 1;
        uint32_t right = 2 * i + 2;
        uint32_t smallest = left;
        
        if (right < pq->size && pq->data[right].priority < pq->data[left].priority) {
            smallest = right;
        }
        
        if (pq->data[smallest].priority < pq->data[i].priority) {
            pq_swap(&pq->data[i], &pq->data[smallest]);
            i = smallest;
        } else {
            break;
        }
    }
    return true;
}

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT ZMM VM: LEVEL UP COORDINATED SCHEDULER TESTS\n");
    printf("=============================================================\n");

    PriorityQueue pq;
    pq_init(&pq);

    // Setup coordinates/masks simulating PLL and PMG telemetry data
    uint8_t pmg_collision_data[32] = {0};
    pmg_collision_data[0] = 0x1F; // Atari collision register mask

    uint8_t pll_drift_data[32] = {0};
    pll_drift_data[0] = 0x90; // Drift deviation metric

    uint8_t storage_data[32] = {0};
    strcpy((char*)storage_data, "OUROBOROS_STACK_PUSH");

    // 1. Push events with differing priorities representing game clock alignments
    printf("1. Enqueuing system events into Priority Queue...\n");
    
    // PMG Collision: Immediate high priority (value 1)
    CoordinatedEvent ev_pmg = {
        .priority = 1,
        .type = EVENT_PMG_COLLISION,
        .timestamp = 100000,
    };
    memcpy(ev_pmg.data, pmg_collision_data, 32);
    assert(pq_push(&pq, ev_pmg));
    
    // Storage sync: Medium priority (value 10)
    CoordinatedEvent ev_storage = {
        .priority = 10,
        .type = EVENT_STACK_STORAGE_SYNC,
        .timestamp = 100010,
    };
    memcpy(ev_storage.data, storage_data, 32);
    assert(pq_push(&pq, ev_storage));

    // PLL Drift adjustment: Lower priority (value 5)
    CoordinatedEvent ev_pll = {
        .priority = 5,
        .type = EVENT_PLL_DRIFT,
        .timestamp = 100005,
    };
    memcpy(ev_pll.data, pll_drift_data, 32);
    assert(pq_push(&pq, ev_pll));
    
    printf("   ✓ Events enqueued successfully.\n\n");

    // 2. Pop events and verify prioritization ordering (Min-Heap: 1 -> 5 -> 10)
    printf("2. Processing event queue in priority sequence:\n");
    CoordinatedEvent popped;
    
    // Expected 1: EVENT_PMG_COLLISION
    assert(pq_pop(&pq, &popped));
    printf("   -> Pop 1: Priority %u | Type %d | Timestamp %lu | Mask: 0x%02X\n", 
           popped.priority, popped.type, popped.timestamp, popped.data[0]);
    assert(popped.type == EVENT_PMG_COLLISION);
    assert(popped.priority == 1);
    
    // Expected 2: EVENT_PLL_DRIFT
    assert(pq_pop(&pq, &popped));
    printf("   -> Pop 2: Priority %u | Type %d | Timestamp %lu | Drift: 0x%02X\n", 
           popped.priority, popped.type, popped.timestamp, popped.data[0]);
    assert(popped.type == EVENT_PLL_DRIFT);
    assert(popped.priority == 5);
    
    // Expected 3: EVENT_STACK_STORAGE_SYNC
    assert(pq_pop(&pq, &popped));
    printf("   -> Pop 3: Priority %u | Type %d | Timestamp %lu | Sync Command: '%s'\n", 
           popped.priority, popped.type, popped.timestamp, (char*)popped.data);
    assert(popped.type == EVENT_STACK_STORAGE_SYNC);
    assert(popped.priority == 10);

    printf("\n   ✓ Queue popping follows Min-Heap bounds correctly!\n");
    printf("=============================================================\n");
    printf("AUNCIENT LEVEL UP SCHEDULER TESTS PASSED SUCCESSFULLY\n");
    printf("=============================================================\n");
    return 0;
}
