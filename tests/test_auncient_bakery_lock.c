#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>

#define NUM_NODES 3

// Shared STANAG / DECnet device lock state
typedef struct {
    bool entering[NUM_NODES];
    uint32_t number[NUM_NODES];
    bool device_busy; // Asserts mutual exclusion
} bakery_lock_t;

// -------------------------------------------------------------
// Lamport's Bakery Algorithm for Shared STANAG/DECnet Access
// -------------------------------------------------------------
void lock_bakery(bakery_lock_t *lock, int node_id) {
    lock->entering[node_id] = true;

    // Find maximum ticket number currently assigned
    uint32_t max_ticket = 0;
    for (int i = 0; i < NUM_NODES; i++) {
        if (lock->number[i] > max_ticket) {
            max_ticket = lock->number[i];
        }
    }
    lock->number[node_id] = max_ticket + 1;
    lock->entering[node_id] = false;

    // Wait for turn
    for (int j = 0; j < NUM_NODES; j++) {
        if (j == node_id) continue;

        // Wait if node j is currently choosing its ticket
        while (lock->entering[j]) {
            // Busy wait (simulated hardware loop)
        }

        // Wait if node j has a non-zero ticket with higher priority (lower ticket number or lower node ID)
        while (lock->number[j] != 0 &&
               (lock->number[j] < lock->number[node_id] ||
                (lock->number[j] == lock->number[node_id] && j < node_id))) {
            // Busy wait (simulated hardware loop)
        }
    }

    // Critical section entered: access shared STANAG / DECnet device
    assert(lock->device_busy == false); // Ensure mutual exclusion
    lock->device_busy = true;
}

void unlock_bakery(bakery_lock_t *lock, int node_id) {
    lock->device_busy = false;
    lock->number[node_id] = 0; // Release ticket
}

// -------------------------------------------------------------
// Unit Tests
// -------------------------------------------------------------
int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT BAKERY DISTRIBUTED MUTUAL EXCLUSION SUITE\n");
    printf("=============================================================\n");
    fflush(stdout);

    bakery_lock_t lock = {
        .entering = { false, false, false },
        .number = { 0, 0, 0 },
        .device_busy = false
    };

    // 1. Node 0 requests and acquires the lock
    printf("[TEST] Node 0 acquiring shared STANAG device...\n");
    fflush(stdout);
    lock_bakery(&lock, 0);
    assert(lock.device_busy == true);
    assert(lock.number[0] == 1);
    printf("   ✓ Node 0 successfully locked device.\n");
    fflush(stdout);

    // 2. Node 1 requests ticket but must wait (simulated concurrent request)
    printf("[TEST] Node 1 requesting ticket concurrently...\n");
    fflush(stdout);
    lock.entering[1] = true;
    lock.number[1] = 2; // Node 1 gets ticket 2
    lock.entering[1] = false;
    assert(lock.device_busy == true); // Node 0 still has the device
    printf("   ✓ Node 1 ticket queued (ticket = 2).\n");
    fflush(stdout);

    // 3. Node 0 releases the device, Node 1 acquires the lock
    printf("[TEST] Node 0 releasing STANAG device. Node 1 acquiring...\n");
    fflush(stdout);
    unlock_bakery(&lock, 0);
    
    // Now evaluate Node 1 lock sequence (waiting loop completes)
    lock_bakery(&lock, 1);
    assert(lock.device_busy == true);
    assert(lock.number[0] == 0); // Node 0 ticket cleared
    printf("   ✓ Node 1 successfully locked device after Node 0 release.\n");
    fflush(stdout);

    printf("[TEST] Node 1 releasing STANAG device...\n");
    fflush(stdout);
    unlock_bakery(&lock, 1);
    assert(lock.device_busy == false);
    printf("   ✓ Device released successfully.\n");
    fflush(stdout);

    printf("=============================================================\n");
    printf("BAKERY DISTRIBUTED LOCK TESTS PASSED\n");
    printf("=============================================================\n");
    fflush(stdout);
    return 0;
}
