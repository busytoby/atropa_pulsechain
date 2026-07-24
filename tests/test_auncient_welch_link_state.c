#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>

typedef struct {
    uint64_t sequence_number;
    uint32_t age;
    uint32_t source_node;
    uint32_t link_metric;
} link_state_update_t;

typedef struct {
    uint64_t last_sequence;
    uint32_t link_metric;
    bool sync_requested;
} routing_entry_t;

// -------------------------------------------------------------
// Welch Link-State Update Validation (Semi-Synchronous Bounds)
// -------------------------------------------------------------
bool validate_welch_link_state(routing_entry_t *entry, const link_state_update_t *update) {
    entry->sync_requested = false;

    // Detect erratic sequence jumps (indicating Byzantine failures or link malfunctions)
    if (update->sequence_number > entry->last_sequence + 1000) {
        entry->sync_requested = true; // Request full topology synchronization
        return false;
    }

    // Monotonically increasing sequence check
    if (update->sequence_number > entry->last_sequence) {
        entry->last_sequence = update->sequence_number;
        entry->link_metric = update->link_metric;
        return true; // Valid link state update accepted
    }

    return false; // Stale update discarded
}

// -------------------------------------------------------------
// Unit Tests
// -------------------------------------------------------------
int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT WELCH LINK-STATE ROUTING SUITE\n");
    printf("=============================================================\n");
    fflush(stdout);

    routing_entry_t entry = {
        .last_sequence = 100,
        .link_metric = 10,
        .sync_requested = false
    };

    // 1. Compliant Update: Monotonically increasing sequence number
    link_state_update_t update_good = {
        .sequence_number = 101,
        .age = 1,
        .source_node = 2,
        .link_metric = 12
    };

    printf("[TEST] Validating compliant link-state update...\n");
    fflush(stdout);
    bool ok = validate_welch_link_state(&entry, &update_good);
    assert(ok == true);
    assert(entry.last_sequence == 101);
    assert(entry.link_metric == 12);
    assert(entry.sync_requested == false);
    printf("   ✓ Compliant update accepted. Metric updated to 12.\n");
    fflush(stdout);

    // 2. Stale Update: Sequence number equal to or less than stored sequence
    link_state_update_t update_stale = {
        .sequence_number = 99,
        .age = 2,
        .source_node = 2,
        .link_metric = 15
    };

    printf("[TEST] Validating stale link-state update...\n");
    fflush(stdout);
    ok = validate_welch_link_state(&entry, &update_stale);
    assert(ok == false);
    assert(entry.last_sequence == 101); // Remains unchanged
    printf("   ✓ Stale update discarded successfully.\n");
    fflush(stdout);

    // 3. Byzantine/Erratic Update: Sequence number jump exceeds safety limits
    link_state_update_t update_erratic = {
        .sequence_number = 2000, // Erratic jump (101 -> 2000)
        .age = 1,
        .source_node = 2,
        .link_metric = 5
    };

    printf("[TEST] Validating erratic Byzantine link-state update...\n");
    fflush(stdout);
    ok = validate_welch_link_state(&entry, &update_erratic);
    assert(ok == false);
    assert(entry.sync_requested == true); // Trigger full recovery sync
    printf("   ✓ Erratic jump trapped. Synchronization requested successfully.\n");
    fflush(stdout);

    printf("=============================================================\n");
    printf("WELCH LINK-STATE TESTS PASSED\n");
    printf("=============================================================\n");
    fflush(stdout);
    return 0;
}
