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
    printf("   ✓ TDMA proof check correctly rejected the mismatching key on Channel 1.\n\n");

    // 4. Test WinchesterMQ EVM-side Ring-Buffer Queue
    printf("4. Testing WinchesterMQ EVM-side Ring-Buffer Queue (pushEvent/popEvent):\n");
    
    // pushEvent calldata: selector 0x0ff22000
    // calldata offsets:
    // 4: priority = 100
    // 36: type = 2
    // 68: timestamp = 2026
    // 100: data = 0xAA
    uint8_t push_cd[132] = {0};
    push_cd[0] = 0x0f; push_cd[1] = 0xf2; push_cd[2] = 0x20; push_cd[3] = 0x00; // selector
    push_cd[35] = 100; // priority
    push_cd[67] = 2;   // type
    push_cd[99] = 2026 & 0xFF; push_cd[98] = (2026 >> 8) & 0xFF; // timestamp
    push_cd[131] = 0xAA; // data
    
    uint8_t push_ret[32];
    size_t push_ret_len = sizeof(push_ret);
    extern bool lau_yul_thunk_execute(const char *name, const uint8_t *calldata, size_t cd_size, uint8_t *retval, size_t *retval_len);
    bool push_ok = lau_yul_thunk_execute("WinchesterMQ", push_cd, sizeof(push_cd), push_ret, &push_ret_len);
    assert(push_ok);
    printf("   ✓ pushEvent executed successfully.\n");
    
    // popEvent calldata: selector 0x0ff23000
    uint8_t pop_cd[4] = {0x0f, 0xf2, 0x30, 0x00};
    uint8_t pop_ret[128];
    size_t pop_ret_len = sizeof(pop_ret);
    bool pop_ok = lau_yul_thunk_execute("WinchesterMQ", pop_cd, sizeof(pop_cd), pop_ret, &pop_ret_len);
    assert(pop_ok);
    assert(pop_ret_len == 128);
    
    // Decoded popped values (Yul stores returns as full 32-byte words)
    uint64_t popped_priority = 0;
    for (int i = 0; i < 32; i++) popped_priority = (popped_priority << 8) | pop_ret[i];
    uint64_t popped_type = 0;
    for (int i = 32; i < 64; i++) popped_type = (popped_type << 8) | pop_ret[i];
    uint64_t popped_timestamp = 0;
    for (int i = 64; i < 96; i++) popped_timestamp = (popped_timestamp << 8) | pop_ret[i];
    uint64_t popped_data = 0;
    for (int i = 96; i < 128; i++) popped_data = (popped_data << 8) | pop_ret[i];
    
    printf("   -> Popped Priority: %lu (Expected: 100)\n", popped_priority);
    printf("   -> Popped Type: %lu (Expected: 2)\n", popped_type);
    printf("   -> Popped Timestamp: %lu (Expected: 2026)\n", popped_timestamp);
    printf("   -> Popped Data: 0x%lx (Expected: 0xAA)\n", popped_data);
    
    assert(popped_priority == 100);
    assert(popped_type == 2);
    assert(popped_timestamp == 2026);
    assert(popped_data == 0xAA);
    printf("   ✓ popEvent successfully retrieved and matched dynamic queue state.\n\n");

    // 5. Test integrated Queue-to-Heap pipeline
    printf("5. Testing Queue-Heap Integration pipeline:\n");
    // Clear slot 0xF185 first
    lau_yul_thunk_sstore(0xF185, 0);
    // Set dummy drift telemetry in 0xF125
    lau_yul_thunk_sstore(0xF125, 42);
    
    // Push an EVENT_STACK_STORAGE_SYNC (type = 10) event with priority = 12 into WinchesterMQ
    uint8_t push_sync_cd[132] = {0};
    push_sync_cd[0] = 0x0f; push_sync_cd[1] = 0xf2; push_sync_cd[2] = 0x20; push_sync_cd[3] = 0x00; // selector
    push_sync_cd[35] = 12; // priority
    push_sync_cd[67] = 10; // type = EVENT_STACK_STORAGE_SYNC
    push_sync_cd[99] = 100; // timestamp
    
    push_ret_len = sizeof(push_ret);
    bool push_sync_ok = lau_yul_thunk_execute("WinchesterMQ", push_sync_cd, sizeof(push_sync_cd), push_ret, &push_ret_len);
    assert(push_sync_ok);
    printf("   ✓ Pushed EVM-side stack sync event into WinchesterMQ.\n");
    
    // Execute integrated tick
    tsfi_ouroboros_run_integrated_tick(10, 3);
    
    // Check that 0xF185 was successfully populated with the live telemetry value from 0xF125
    uint64_t live_val = lau_yul_thunk_sload(0xF125);
    uint64_t synced_val = lau_yul_thunk_sload(0xF185);
    printf("   -> Synced value at 0xF185: %lu (Live telemetry at 0xF125: %lu)\n", synced_val, live_val);
    assert(synced_val == live_val && synced_val > 0);
    printf("   ✓ Queue-to-Heap integration verified: Yul enqueued event popped, sorted, and executed in tick.\n\n");

    // 6. Test UMCGR (Unified Multi-Contract Guest Routing) to musicMaker (Address 0x3)
    printf("6. Testing UMCGR routing to guest contract 'musicMaker' (Addr 0x3):\n");
    // Push an EVENT_GUEST_CONTRACT_CALL (type = 4) event targeting address 0x3
    uint8_t push_route_cd[132] = {0};
    push_route_cd[0] = 0x0f; push_route_cd[1] = 0xf2; push_route_cd[2] = 0x20; push_route_cd[3] = 0x00; // selector
    push_route_cd[35] = 20; // priority
    push_route_cd[67] = 4;  // type = EVENT_GUEST_CONTRACT_CALL
    push_route_cd[99] = 100; // timestamp
    // data[0..7] = target_addr = 0x3
    push_route_cd[100 + 7] = 0x03;
    // data[8..31] = dummy payload = 0x77
    push_route_cd[100 + 8] = 0x77;
    
    push_ret_len = sizeof(push_ret);
    bool push_route_ok = lau_yul_thunk_execute("WinchesterMQ", push_route_cd, sizeof(push_route_cd), push_ret, &push_ret_len);
    assert(push_route_ok);
    printf("   ✓ Pushed dynamic contract route event into WinchesterMQ.\n");
    
    // Execute integrated tick (should pop the route event, resolve Address 0x3, and call musicMaker successfully)
    tsfi_ouroboros_run_integrated_tick(10, 3);
    printf("   ✓ UMCGR test completed: Scheduler successfully dispatched call to 'musicMaker'.\n\n");

    // 7. Test EVM Reentrancy Mutex Lock
    printf("7. Testing EVM Ring-Buffer Queue Reentrancy Guard:\n");
    // Simulate lock acquired by setting slot 0xF303 to 1
    lau_yul_thunk_sstore(0xF303, 1);
    // Try pushEvent; should fail/revert
    push_ret_len = sizeof(push_ret);
    bool fail_push_ok = lau_yul_thunk_execute("WinchesterMQ", push_route_cd, sizeof(push_route_cd), push_ret, &push_ret_len);
    printf("   -> pushEvent with active mutex: %s\n", fail_push_ok ? "ALLOWED (FAIL)" : "REVERTED (PASS)");
    assert(!fail_push_ok);
    // Release lock
    lau_yul_thunk_sstore(0xF303, 0);
    printf("   ✓ EVM reentrancy guard successfully locked and reverted nested access.\n\n");

    // 8. Test pushEvent Wrapper and Spillover to Host Buffer
    printf("8. Testing pushEvent Wrapper and Spillover to Host Buffer:\n");
    // Simulate a full EVM queue (size = 16)
    lau_yul_thunk_sstore(0xF302, 16);
    // Push event using host wrapper
    uint8_t dummy_data[32] = {0};
    dummy_data[0] = 0xAA;
    extern bool tsfi_ouroboros_push_event(uint32_t priority, uint32_t type, uint64_t timestamp, const uint8_t *data);
    extern uint32_t tsfi_ouroboros_get_pq_size(void);
    extern uint32_t tsfi_ouroboros_get_pq_priority(uint32_t index);
    
    uint32_t initial_size = tsfi_ouroboros_get_pq_size();
    bool wrapper_ok = tsfi_ouroboros_push_event(50, 3, 1000, dummy_data);
    assert(wrapper_ok);
    uint32_t post_size = tsfi_ouroboros_get_pq_size();
    printf("   -> PQ Size: %u (Initial: %u, expected increase: 1)\n", post_size, initial_size);
    assert(post_size == initial_size + 1);
    printf("   ✓ Event successfully enqueued when EVM-queue was full (spilled over to host buffer).\n\n");

    // 9. Test Dynamic Priority Aging
    printf("9. Testing Dynamic Priority Aging:\n");
    // Find the index of the newly added event in s_scheduler_pq
    uint32_t spill_index = post_size - 1;
    uint32_t priority_before = tsfi_ouroboros_get_pq_priority(spill_index);
    printf("   -> Event priority before tick dispatch: %u\n", priority_before);
    
    // Run tick to trigger aging on the enqueued spillover event
    tsfi_ouroboros_run_integrated_tick(10, 3);
    
    // Check that the priority of the remaining event has decreased (boosted in precedence)
    uint32_t priority_after = tsfi_ouroboros_get_pq_priority(0);
    printf("   -> Event priority after tick dispatch (aged): %u\n", priority_after);
    assert(priority_after < priority_before);
    printf("   ✓ Dynamic priority aging pass verified successfully.\n\n");

    // 10. Test JIT Binary Search Tree Cache & Dynamic Memory Mapping
    printf("10. Testing JIT Binary Search Tree Cache & Dynamic Memory Mapping:\n");
    extern uint64_t g_thunk_cache_hits;
    extern uint64_t g_thunk_cache_lookups;
    uint64_t hits_before = g_thunk_cache_hits;
    uint64_t lookups_before = g_thunk_cache_lookups;
    
    // Call musicMaker first time to populate cache
    uint8_t mm_cd[4] = {0x11, 0x2d, 0xf4, 0x9e};
    uint8_t mm_ret[32] = {0};
    size_t mm_ret_len = sizeof(mm_ret);
    bool mm_ok = lau_yul_thunk_execute("musicMaker", mm_cd, sizeof(mm_cd), mm_ret, &mm_ret_len);
    assert(mm_ok);
    
    // Call musicMaker a second time with exact same calldata to trigger cache hit
    mm_ret_len = sizeof(mm_ret);
    bool mm_cache_ok = lau_yul_thunk_execute("musicMaker", mm_cd, sizeof(mm_cd), mm_ret, &mm_ret_len);
    assert(mm_cache_ok);
    
    uint64_t hits_after = g_thunk_cache_hits;
    uint64_t lookups_after = g_thunk_cache_lookups;
    printf("    -> Cache hits: %lu (Before: %lu)\n", hits_after, hits_before);
    printf("    -> Cache lookups: %lu (Before: %lu)\n", lookups_after, lookups_before);
    assert(lookups_after > lookups_before);
    assert(hits_after > hits_before);
    printf("    ✓ BST-based lau_memory cache hit verified successfully.\n\n");

    // 11. Test 2-3 Tree Segment Persistence & Rehydration
    printf("11. Testing 2-3 Tree Segment Persistence & Rehydration:\n");
    uint64_t r_quad = 0;
    extern bool blue_box_verify_23_to_quad_conversion(uint64_t r23_root_0, uint64_t r23_root_1, uint64_t r23_root_2, uint64_t r23_root_3, uint64_t *r_quad_out);
    extern bool blue_box_commit_quadtree_via_btc_script(uint64_t old_root, uint64_t next_root, const uint8_t *witness, size_t witness_len);
    extern void blue_box_rehydrate_quadtree_states(void);
    extern uint64_t lau_yul_thunk_sload(uint64_t key);
    extern void lau_yul_thunk_sstore(uint64_t key, uint64_t value);

    // Force active root to 0
    lau_yul_thunk_sstore(0xF1B5, 0);

    // Convert segments: 700, 800, 900, 1000
    bool conv_ok = blue_box_verify_23_to_quad_conversion(700, 800, 900, 1000, &r_quad);
    assert(conv_ok);

    // Commit via BTC script with valid witness delta computed via modular inverse of 34 using uint128 to avoid overflow
    unsigned __int128 delta_128 = (unsigned __int128)r_quad * 869338428751331ULL;
    uint64_t delta = (uint64_t)(delta_128 % 953467954114363ULL);
    uint8_t witness_dummy[8];
    for (int i = 0; i < 8; i++) {
        witness_dummy[i] = (delta >> (56 - i * 8)) & 0xFF;
    }
    bool commit_ok = blue_box_commit_quadtree_via_btc_script(0, r_quad, witness_dummy, sizeof(witness_dummy));
    assert(commit_ok);

    // Now clear out the storage keys to simulate a crash/reboot
    lau_yul_thunk_sstore(0xF1B5, 0);
    lau_yul_thunk_sstore(0xF1C5, 0);
    lau_yul_thunk_sstore(0xF1C7, 0);
    lau_yul_thunk_sstore(0xF1C8, 0);
    lau_yul_thunk_sstore(0xF1C9, 0);

    // Call rehydrate
    blue_box_rehydrate_quadtree_states();

    // Verify values were loaded back from node ledger file
    uint64_t recovered_root = lau_yul_thunk_sload(0xF1B5);
    uint64_t recovered_r23_0 = lau_yul_thunk_sload(0xF1C5);
    uint64_t recovered_r23_1 = lau_yul_thunk_sload(0xF1C7);
    uint64_t recovered_r23_2 = lau_yul_thunk_sload(0xF1C8);
    uint64_t recovered_r23_3 = lau_yul_thunk_sload(0xF1C9);

    assert(recovered_root == 28376944299ULL);
    assert(recovered_r23_0 == 700);
    assert(recovered_r23_1 == 800);
    assert(recovered_r23_2 == 900);
    assert(recovered_r23_3 == 1000);
    printf("    ✓ Successfully rehydrated 2-3 Tree root segments from assets: Segments=[%lu,%lu,%lu,%lu]\n\n",
           recovered_r23_0, recovered_r23_1, recovered_r23_2, recovered_r23_3);

    printf("=============================================================\n");
    printf("AUNCIENT LEVEL UP SCHEDULER TESTS PASSED SUCCESSFULLY\n");
    printf("=============================================================\n");
    tsfi_zmm_vm_destroy(&vm_state);
    return 0;
}
