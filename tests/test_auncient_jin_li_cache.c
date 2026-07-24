#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>

typedef enum {
    CUTOFF_STATE,
    CONDUC_STATE
} gate_state_t;

// Simulated Cache Write Transaction
typedef struct {
    uint32_t start_addr;
    uint32_t end_addr;
} cache_transaction_t;

// Cache line allocation monitor
typedef struct {
    gate_state_t write_gate;
    cache_transaction_t active_writes[2];
} jin_li_cache_t;

// -------------------------------------------------------------
// Jin Li Cache Collision Gating
// -------------------------------------------------------------
bool verify_jin_li_cache_concurrency(jin_li_cache_t *cache) {
    cache->write_gate = CUTOFF_STATE;

    // Jin Li check: Ensure the two active writes do not overlap (Horning layout on cache lines)
    const cache_transaction_t *t1 = &cache->active_writes[0];
    const cache_transaction_t *t2 = &cache->active_writes[1];

    // Check overlap: (StartA <= EndB) and (StartB <= EndA)
    bool overlap = (t1->start_addr <= t2->end_addr) && (t2->start_addr <= t1->end_addr);
    if (overlap) {
        return false; // Collision detected. Cache write path cutoff.
    }

    cache->write_gate = CONDUC_STATE;
    return true; // Path safe to conduct
}

// -------------------------------------------------------------
// Unit Tests
// -------------------------------------------------------------
int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT JIN LI CONCURRENT CACHE VALIDATION SUITE\n");
    printf("=============================================================\n");
    fflush(stdout);

    // 1. Compliant Case: Disjoint cache line transactions -> Should pass
    jin_li_cache_t cache_ok = {
        .write_gate = CUTOFF_STATE,
        .active_writes = {
            { .start_addr = 0x1000, .end_addr = 0x103F }, // Cache Line 1
            { .start_addr = 0x1040, .end_addr = 0x107F }  // Cache Line 2
        }
    };

    printf("[TEST] Checking disjoint cache writes...\n");
    fflush(stdout);
    bool ok = verify_jin_li_cache_concurrency(&cache_ok);
    assert(ok == true);
    assert(cache_ok.write_gate == CONDUC_STATE);
    printf("   ✓ Cache path open. Concurrent writes approved.\n");
    fflush(stdout);

    // 2. Colliding Case: Overlapping address transaction -> Should fail
    jin_li_cache_t cache_bad = {
        .write_gate = CUTOFF_STATE,
        .active_writes = {
            { .start_addr = 0x1000, .end_addr = 0x104F }, // Overlaps
            { .start_addr = 0x1040, .end_addr = 0x107F }
        }
    };

    printf("[TEST] Checking overlapping cache writes...\n");
    fflush(stdout);
    ok = verify_jin_li_cache_concurrency(&cache_bad);
    assert(ok == false);
    assert(cache_bad.write_gate == CUTOFF_STATE);
    printf("   ✓ Cache write collision trapped successfully.\n");
    fflush(stdout);

    printf("=============================================================\n");
    printf("JIN LI CACHE TESTS PASSED\n");
    printf("=============================================================\n");
    fflush(stdout);
    return 0;
}
