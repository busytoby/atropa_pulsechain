#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>

#define NUM_PEERS 4
#define TOTAL_SLOTS 8

typedef struct {
    uint32_t peer_id;
    bool is_active;
    uint32_t queue_size;
    uint32_t slots_owned;
    uint32_t slots_borrowed;
    uint32_t packets_transmitted;
} TelecomNode;

// Advanced TDMA Controller
typedef struct {
    uint32_t active_slot;
    uint32_t guard_band_us;  // Adaptive Guard Band
    uint32_t noise_level_db;
    uint32_t sliding_window_sz;
} TdmaController;

// Decides transmission and executes slot borrowing and guard band scaling
void process_telecom_transmission(TelecomNode *peers, size_t peer_count, TdmaController *ctrl) {
    uint32_t primary_owner_idx = ctrl->active_slot % peer_count;
    TelecomNode *owner = &peers[primary_owner_idx];

    // 1. Adaptive Guard Band Scaling: Adjust guard band based on noise levels
    if (ctrl->noise_level_db < 10) {
        ctrl->guard_band_us = 2;  // Shrink guard bands in low-noise channels
    } else {
        ctrl->guard_band_us = 10; // Expand guard bands to prevent signal leakage
    }

    // 2. Dynamic Slot Borrowing: If owner is idle, active nodes with large queues can borrow slot
    if (!owner->is_active || owner->queue_size == 0) {
        for (size_t i = 0; i < peer_count; i++) {
            if (peers[i].is_active && peers[i].queue_size > 0) {
                // Borrow the slot
                uint32_t tx_packets = (peers[i].queue_size > ctrl->sliding_window_sz) ? ctrl->sliding_window_sz : peers[i].queue_size;
                peers[i].queue_size -= tx_packets;
                peers[i].packets_transmitted += tx_packets;
                peers[i].slots_borrowed++;
                printf("   [Slot Borrowed] Peer %u borrowed Slot %u from Idle Peer %u (Sent %u packets)\n", 
                       peers[i].peer_id, ctrl->active_slot, owner->peer_id, tx_packets);
                return;
            }
        }
    } else {
        // Owner transmits natively using sliding window schedule
        uint32_t tx_packets = (owner->queue_size > ctrl->sliding_window_sz) ? ctrl->sliding_window_sz : owner->queue_size;
        owner->queue_size -= tx_packets;
        owner->packets_transmitted += tx_packets;
        printf("   [Slot Native] Peer %u transmitted %u packets at Slot %u\n", 
               owner->peer_id, tx_packets, ctrl->active_slot);
    }
}

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT ADVANCED TELECOM TDMA SCHEDULER\n");
    printf("=============================================================\n");

    // Initialize 4 peers: Node 300 (active with large queue), others are idle
    TelecomNode peers[NUM_PEERS] = {
        {.peer_id = 300, .is_active = true, .queue_size = 25, .slots_owned = 2, .slots_borrowed = 0, .packets_transmitted = 0},
        {.peer_id = 301, .is_active = false, .queue_size = 0, .slots_owned = 2, .slots_borrowed = 0, .packets_transmitted = 0},
        {.peer_id = 302, .is_active = false, .queue_size = 0, .slots_owned = 2, .slots_borrowed = 0, .packets_transmitted = 0},
        {.peer_id = 303, .is_active = false, .queue_size = 0, .slots_owned = 2, .slots_borrowed = 0, .packets_transmitted = 0}
    };

    TdmaController ctrl = {
        .active_slot = 0,
        .guard_band_us = 5,
        .noise_level_db = 5,       // Low noise
        .sliding_window_sz = 4     // Sliding window size
    };

    // 1. Run simulation loop across 8 slots
    printf("1. Running teleconference schedule loop...\n");
    for (uint32_t slot = 0; slot < TOTAL_SLOTS; slot++) {
        ctrl.active_slot = slot;
        process_telecom_transmission(peers, NUM_PEERS, &ctrl);
    }
    printf("\n");

    // 2. Validate Slot Borrowing
    // Node 300 should borrow slots from the idle nodes (301, 302, 303)
    assert(peers[0].slots_borrowed > 0);
    // Dynamic Guard Band should scale down to 2 us in low noise
    assert(ctrl.guard_band_us == 2);
    printf("   ✓ Slot borrowing verified (Slots borrowed = %u)\n", peers[0].slots_borrowed);
    printf("   ✓ Adaptive Guard Band scaling verified (Guard band = %u us)\n\n", ctrl.guard_band_us);

    // 3. Test Adaptive Guard Band Expansion on high noise
    printf("3. Simulating noise spike to test guard band expansion...\n");
    ctrl.noise_level_db = 15; // Spike noise
    process_telecom_transmission(peers, NUM_PEERS, &ctrl);
    assert(ctrl.guard_band_us == 10);
    printf("   ✓ Guard band expanded successfully to %u us\n\n", ctrl.guard_band_us);

    printf("=============================================================\n");
    printf("ADVANCED TELECOM TDMA TESTS PASSED SUCCESSFULLY\n");
    printf("=============================================================\n");
    return 0;
}
