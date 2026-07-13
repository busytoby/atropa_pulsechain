#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>

#define NUM_SLOTS 4
#define TEST_PACKETS 40000

typedef struct {
    uint32_t peer_id;
    uint32_t slot_id;
    uint32_t successful_transmissions;
    uint32_t blocked_transmissions;
} TdmaPeer;

// Evaluates transmission request against the active TDMA slot schedule
void process_tdma_slot(TdmaPeer *peer, uint32_t active_slot_id, uint32_t *collision_count) {
    if (peer->slot_id == active_slot_id) {
        peer->successful_transmissions++;
    } else {
        // Reject transmission if node attempts to write outside its allocated slot
        peer->blocked_transmissions++;
        (*collision_count)++;
    }
}

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT COAXIAL SHARED-CHANNEL TDMA SCHEDULER\n");
    printf("=============================================================\n");

    // Initialize 4 peers, each assigned to a dedicated slot in the TDMA frame
    TdmaPeer peers[NUM_SLOTS] = {
        {.peer_id = 200, .slot_id = 0, .successful_transmissions = 0, .blocked_transmissions = 0},
        {.peer_id = 201, .slot_id = 1, .successful_transmissions = 0, .blocked_transmissions = 0},
        {.peer_id = 202, .slot_id = 2, .successful_transmissions = 0, .blocked_transmissions = 0},
        {.peer_id = 203, .slot_id = 3, .successful_transmissions = 0, .blocked_transmissions = 0}
    };

    uint32_t collision_count = 0;
    uint32_t active_slot = 0;

    // 1. Simulate synchronized transmission flow (each peer transmits only in its slot)
    printf("1. Simulating synchronized TDMA transmissions...\n");
    for (int i = 0; i < TEST_PACKETS; i++) {
        active_slot = i % NUM_SLOTS;
        // The peer owning the active slot transmits
        process_tdma_slot(&peers[active_slot], active_slot, &collision_count);
    }
    assert(collision_count == 0);
    printf("   ✓ Synchronized transmission: Collisions = %u\n\n", collision_count);

    // 2. Simulate out-of-sync transmission request (violating schedule)
    printf("2. Simulating out-of-sync transmission attempt (collision avoidance)...\n");
    active_slot = 0; // Peer 200's slot
    // Peer 202 attempts to transmit during Peer 200's slot
    process_tdma_slot(&peers[2], active_slot, &collision_count);
    
    assert(peers[2].blocked_transmissions == 1);
    assert(collision_count == 1);
    printf("   ✓ Blocked out-of-slot write. Collision register incremented: %u\n\n", collision_count);

    // Print summary stats
    for (int i = 0; i < NUM_SLOTS; i++) {
        printf("   Peer [%u]: Slot %u | Success Transmissions = %u, Blocked = %u\n", 
               peers[i].peer_id, peers[i].slot_id, peers[i].successful_transmissions, peers[i].blocked_transmissions);
    }

    printf("=============================================================\n");
    printf("TDMA SHARED-CHANNEL SCHEDULER TESTS PASSED\n");
    printf("=============================================================\n");

    return 0;
}
