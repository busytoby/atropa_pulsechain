#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>

#define NUM_NODES 3

typedef struct {
    uint32_t value;
    uint64_t timestamp;
} register_state_t;

typedef struct {
    register_state_t local_registers[NUM_NODES];
} coaxial_bus_t;

// -------------------------------------------------------------
// ABD Sequentially Consistent Register Protocol (Attiya-Welch)
// -------------------------------------------------------------
bool abd_write(coaxial_bus_t *bus, uint32_t val, uint64_t ts) {
    int acks = 0;

    // Phase 1: Propose and write to local nodes
    for (int i = 0; i < NUM_NODES; i++) {
        // Only accept writes with strictly higher timestamps
        if (ts > bus->local_registers[i].timestamp) {
            bus->local_registers[i].value = val;
            bus->local_registers[i].timestamp = ts;
            acks++;
        }
    }

    // Phase 2: Verify majority consensus acknowledgment (at least 2/3 nodes)
    return (acks > NUM_NODES / 2);
}

register_state_t abd_read(const coaxial_bus_t *bus) {
    register_state_t highest_state = { .value = 0, .timestamp = 0 };

    // Query all nodes and select the state with the highest timestamp
    for (int i = 0; i < NUM_NODES; i++) {
        if (bus->local_registers[i].timestamp > highest_state.timestamp) {
            highest_state = bus->local_registers[i];
        }
    }

    return highest_state;
}

// -------------------------------------------------------------
// Unit Tests
// -------------------------------------------------------------
int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT ABD CONSISTENT REGISTER SUITE\n");
    printf("=============================================================\n");
    fflush(stdout);

    coaxial_bus_t bus = {
        .local_registers = {
            { .value = 100, .timestamp = 10 },
            { .value = 100, .timestamp = 10 },
            { .value = 100, .timestamp = 10 }
        }
    };

    // 1. Compliant Write: High timestamp proposal -> Should commit and update
    printf("[TEST] Proposing valid ABD write (val = 200, ts = 12)...\n");
    fflush(stdout);
    bool ok = abd_write(&bus, 200, 12);
    assert(ok == true);

    register_state_t read_state = abd_read(&bus);
    assert(read_state.value == 200);
    assert(read_state.timestamp == 12);
    printf("   ✓ Write approved by majority. Read returned consistent value 200.\n");
    fflush(stdout);

    // 2. Stale Write: Stale timestamp proposal -> Should fail majority ACK
    printf("[TEST] Proposing stale ABD write (val = 300, ts = 11)...\n");
    fflush(stdout);
    ok = abd_write(&bus, 300, 11);
    assert(ok == false); // Should not achieve majority updates

    read_state = abd_read(&bus);
    assert(read_state.value == 200); // Value remains unchanged
    printf("   ✓ Stale write rejected successfully. State preserved.\n");
    fflush(stdout);

    printf("=============================================================\n");
    printf("ABD CONSISTENT REGISTER TESTS PASSED\n");
    printf("=============================================================\n");
    fflush(stdout);
    return 0;
}
