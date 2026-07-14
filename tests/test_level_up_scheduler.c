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

#include "tsfi_zmm_vm.h"
#include "lau_yul_thunk.h"

extern void blue_box_init_block(uint64_t block_num, const uint8_t *state_root_hash);
extern void tsfi_ouroboros_run_integrated_tick(uint32_t delta_time_ms, uint64_t base);
extern uint64_t lau_yul_thunk_sload(uint64_t key);
extern void lau_yul_thunk_sstore(uint64_t key, uint64_t val);

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT ZMM VM: LEVEL UP COORDINATED SCHEDULER TESTS\n");
    printf("=============================================================\n");

    // Initialize Yul compiler and VM state context
    static TsfiZmmVmState vm_state;
    tsfi_zmm_vm_init(&vm_state);
    blue_box_init_block(1, NULL);
    g_yul_evm_context.self_address = 0x200;

    // 1. Configure PMG positions and collision registers
    printf("1. Configuring PMG coordinates and collision data:\n");
    
    // Player position (10, 20)
    lau_yul_thunk_sstore(0xF200, 10);
    lau_yul_thunk_sstore(0xF201, 20);
    
    // Missile position (12, 22) => Distance = |10-12| + |20-22| = 4
    lau_yul_thunk_sstore(0xF202, 12);
    lau_yul_thunk_sstore(0xF203, 22);
    
    // Base frequency is 3, expected key = 3^4 = 81
    lau_yul_thunk_sstore(0xF205, 81);
    
    // Set up Channel 1 coordinates: Player(15, 25), Missile(18, 28) => Distance = |15-18| + |25-28| = 6
    // Expected key = 3^6 = 729
    lau_yul_thunk_sstore(0xF220, 15);
    lau_yul_thunk_sstore(0xF221, 25);
    lau_yul_thunk_sstore(0xF222, 18);
    lau_yul_thunk_sstore(0xF223, 28);
    lau_yul_thunk_sstore(0xF225, 729);

    // Trigger PMG collision register mask (with bit 0 and bit 1 active)
    lau_yul_thunk_sstore(0xF210, 0x1F);

    printf("   ✓ PMG state loaded:\n");
    printf("     - Ch 0: Player(10,20), Missile(12,22), Expected Key: 81\n");
    printf("     - Ch 1: Player(15,25), Missile(18,28), Expected Key: 729\n\n");

    // 2. Execute integrated tick with VALID TDMA proof keys
    printf("2. Running integrated scheduler tick with VALID TDMA proof keys...\n");
    tsfi_ouroboros_run_integrated_tick(10, 3);
    
    uint64_t authorized_ch0 = lau_yul_thunk_sload(0xF208);
    uint64_t authorized_ch1 = lau_yul_thunk_sload(0xF228);
    printf("   -> Ch 0 TDMA Slot Lock status: %lu\n", authorized_ch0);
    printf("   -> Ch 1 TDMA Slot Lock status: %lu\n", authorized_ch1);
    assert(authorized_ch0 == 1);
    assert(authorized_ch1 == 1);
    printf("   ✓ TDMA proof check correctly authorized both channels' locks.\n\n");

    // 3. Execute integrated tick with INVALID keys
    printf("3. Running integrated scheduler tick with INVALID TDMA proof keys...\n");
    
    // Set incorrect expected key for Channel 1
    lau_yul_thunk_sstore(0xF225, 999);
    tsfi_ouroboros_run_integrated_tick(10, 3);
    
    authorized_ch0 = lau_yul_thunk_sload(0xF208);
    authorized_ch1 = lau_yul_thunk_sload(0xF228);
    printf("   -> Ch 0 TDMA Slot Lock status: %lu\n", authorized_ch0);
    printf("   -> Ch 1 TDMA Slot Lock status: %lu\n", authorized_ch1);
    assert(authorized_ch0 == 1);
    assert(authorized_ch1 == 0);
    printf("   ✓ TDMA proof check correctly rejected the mismatching key on Channel 1.\n");

    printf("=============================================================\n");
    printf("AUNCIENT LEVEL UP SCHEDULER TESTS PASSED SUCCESSFULLY\n");
    printf("=============================================================\n");
    tsfi_zmm_vm_destroy(&vm_state);
    return 0;
}
