#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>

#define NUM_NODES 3

typedef struct {
    uint64_t counter;
    uint32_t writer_id;
} mwmr_timestamp_t;

typedef struct {
    uint32_t value;
    mwmr_timestamp_t ts;
} mwmr_register_t;

typedef struct {
    mwmr_register_t local_registers[NUM_NODES];
} mwmr_bus_t;

// -------------------------------------------------------------
// MWMR Timestamp Comparison (Tie-breaking tuple ordering)
// -------------------------------------------------------------
bool is_timestamp_greater(mwmr_timestamp_t ts1, mwmr_timestamp_t ts2) {
    if (ts1.counter > ts2.counter) {
        return true;
    }
    if (ts1.counter == ts2.counter && ts1.writer_id > ts2.writer_id) {
        return true;
    }
    return false;
}

// -------------------------------------------------------------
// MWMR ABD Write Protocol
// -------------------------------------------------------------
bool mwmr_write(mwmr_bus_t *bus, uint32_t val, uint32_t writer_id) {
    // Phase 1: Query majority to find maximum timestamp counter
    uint64_t max_counter = 0;
    for (int i = 0; i < NUM_NODES; i++) {
        if (bus->local_registers[i].ts.counter > max_counter) {
            max_counter = bus->local_registers[i].ts.counter;
        }
    }

    // Generate new timestamp with tie-breaker
    mwmr_timestamp_t new_ts = {
        .counter = max_counter + 1,
        .writer_id = writer_id
    };

    // Phase 2: Update local storage on all nodes
    int acks = 0;
    for (int i = 0; i < NUM_NODES; i++) {
        if (is_timestamp_greater(new_ts, bus->local_registers[i].ts)) {
            bus->local_registers[i].value = val;
            bus->local_registers[i].ts = new_ts;
            acks++;
        }
    }

    return (acks > NUM_NODES / 2);
}

// -------------------------------------------------------------
// MWMR ABD Read Protocol
// -------------------------------------------------------------
mwmr_register_t mwmr_read(mwmr_bus_t *bus) {
    mwmr_register_t highest = { .value = 0, .ts = { 0, 0 } };

    // Query majority to find highest timestamp tuple
    for (int i = 0; i < NUM_NODES; i++) {
        if (is_timestamp_greater(bus->local_registers[i].ts, highest.ts)) {
            highest = bus->local_registers[i];
        }
    }

    // Propagate Phase: Write back the highest value
    for (int i = 0; i < NUM_NODES; i++) {
        if (is_timestamp_greater(highest.ts, bus->local_registers[i].ts)) {
            bus->local_registers[i].value = highest.value;
            bus->local_registers[i].ts = highest.ts;
        }
    }

    return highest;
}

// -------------------------------------------------------------
// Unit Tests
// -------------------------------------------------------------
int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT MWMR-ABD CONSISTENT REGISTER SUITE\n");
    printf("=============================================================\n");
    fflush(stdout);

    mwmr_bus_t bus = {
        .local_registers = {
            { .value = 10, .ts = { 1, 0 } },
            { .value = 10, .ts = { 1, 0 } },
            { .value = 10, .ts = { 1, 0 } }
        }
    };

    // 1. Writer 1 proposals (Writer ID = 1)
    printf("[TEST] Writer 1 writing value 500...\n");
    fflush(stdout);
    bool ok = mwmr_write(&bus, 500, 1);
    assert(ok == true);

    mwmr_register_t current = mwmr_read(&bus);
    assert(current.value == 500);
    assert(current.ts.counter == 2);
    assert(current.ts.writer_id == 1);
    printf("   ✓ Writer 1 write committed successfully.\n");
    fflush(stdout);

    // 2. Simulating concurrent write tie-breaker resolution:
    // Writer 1 writes 600, Writer 2 writes 700. Both read counter = 2.
    // They will propose counter = 3.
    // Writer 1 timestamp tuple: (3, 1)
    // Writer 2 timestamp tuple: (3, 2) -> Should win tie-breaker.
    printf("[TEST] Simulating concurrent writes (Writer 1 vs. Writer 2)...\n");
    fflush(stdout);

    // Directly apply the two proposals to simulate reception order
    mwmr_timestamp_t ts_writer1 = { .counter = 3, .writer_id = 1 };
    mwmr_timestamp_t ts_writer2 = { .counter = 3, .writer_id = 2 };

    // Apply Writer 1 proposal (val = 600)
    for (int i = 0; i < NUM_NODES; i++) {
        if (is_timestamp_greater(ts_writer1, bus.local_registers[i].ts)) {
            bus.local_registers[i].value = 600;
            bus.local_registers[i].ts = ts_writer1;
        }
    }

    // Apply Writer 2 proposal (val = 700) - Wins tie-breaker due to higher ID
    for (int i = 0; i < NUM_NODES; i++) {
        if (is_timestamp_greater(ts_writer2, bus.local_registers[i].ts)) {
            bus.local_registers[i].value = 700;
            bus.local_registers[i].ts = ts_writer2;
        }
    }

    current = mwmr_read(&bus);
    assert(current.value == 700); // Writer 2 wins
    assert(current.ts.counter == 3);
    assert(current.ts.writer_id == 2);
    printf("   ✓ Tie-breaker resolved. Writer 2 (ID = 2) selected over Writer 1.\n");
    fflush(stdout);

    printf("=============================================================\n");
    printf("MWMR-ABD TESTS PASSED\n");
    printf("=============================================================\n");
    fflush(stdout);
    return 0;
}
