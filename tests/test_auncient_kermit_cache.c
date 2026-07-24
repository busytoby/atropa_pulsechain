#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>

// Aho-Corasick style simple state cache for Kermit Policy signatures
typedef struct {
    char signature[4]; // e.g. "110", "100"
    bool is_approved;
    bool is_valid;
} kermit_cache_entry_t;

#define CACHE_SIZE 4
static kermit_cache_entry_t policy_cache[CACHE_SIZE];

// Initialize lookup cache
void init_kermit_policy_cache(void) {
    strcpy(policy_cache[0].signature, "110");
    policy_cache[0].is_approved = true;
    policy_cache[0].is_valid = true;

    strcpy(policy_cache[1].signature, "111");
    policy_cache[1].is_approved = true;
    policy_cache[1].is_valid = true;

    strcpy(policy_cache[2].signature, "100");
    policy_cache[2].is_approved = false;
    policy_cache[2].is_valid = true;

    strcpy(policy_cache[3].signature, "000");
    policy_cache[3].is_approved = false;
    policy_cache[3].is_valid = true;
}

// -------------------------------------------------------------
// Rule 11: Aho-Corasick Cache Lookup (Hot-Path, No Console Prints)
// -------------------------------------------------------------
bool kermit_policy_cache_lookup(const char *signature, bool *is_approved_out) {
    // CRITICAL: No console output (printf) inside lookup pathway to preserve latency
    for (int i = 0; i < CACHE_SIZE; i++) {
        if (policy_cache[i].is_valid && strcmp(policy_cache[i].signature, signature) == 0) {
            *is_approved_out = policy_cache[i].is_approved;
            return true; // Cache hit
        }
    }
    return false; // Cache miss
}

// -------------------------------------------------------------
// Unit Tests
// -------------------------------------------------------------
int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT KERMIT POLICY AHO-CORASICK CACHE SUITE\n");
    printf("=============================================================\n");
    fflush(stdout);

    init_kermit_policy_cache();

    // Measure lookup latency
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    bool approved = false;
    bool hit = kermit_policy_cache_lookup("110", &approved);

    clock_gettime(CLOCK_MONOTONIC, &end);

    // Calculate elapsed time in nanoseconds
    uint64_t start_ns = (uint64_t)start.tv_sec * 1000000000ULL + (uint64_t)start.tv_nsec;
    uint64_t end_ns = (uint64_t)end.tv_sec * 1000000000ULL + (uint64_t)end.tv_nsec;
    uint64_t diff_ns = end_ns - start_ns;

    assert(hit == true);
    assert(approved == true);
    printf("[TEST] Cache lookup hit verified. Resolved: %s\n", approved ? "APPROVED" : "REJECTED");
    fflush(stdout);

    // Rule 11 Latency Guard check (< 1000 ns)
    printf("[TEST] Checking lookup latency: %lu ns...\n", (unsigned long)diff_ns);
    fflush(stdout);
    assert(diff_ns < 1000);
    printf("   ✓ Latency Guard Gate PASSED (< 1000 ns).\n");
    fflush(stdout);

    printf("=============================================================\n");
    printf("KERMIT CACHE TESTS PASSED\n");
    printf("=============================================================\n");
    fflush(stdout);
    return 0;
}
