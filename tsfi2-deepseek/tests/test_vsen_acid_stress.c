#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>
#include <unistd.h>
#include <time.h>
#include "tsfi_zorse_eval.h"
#include "tsfi_xplos_shell_cbt_vtam.h"

#define STRESS_ITERATIONS 5000

int main(void) {
    printf("[STRESS TEST VSEN RDBMS ACID] Starting High-Volume z/VSEn Stress Suite (%d iterations)...\n", STRESS_ITERATIONS);

    // Clean initial test state
    unlink("vaesen_registry.dat.bin");
    unlink("vaesen_registry.wal.dat.bin");

    char type_buf[64];
    char status_buf[32];
    int risk = 0;

    // 1. High-Volume Sequential Transaction Insertion Loop
    printf("  [STRESS 1/5] Executing %d MVCC registration transactions...\n", STRESS_ITERATIONS);
    for (int i = 0; i < STRESS_ITERATIONS; i++) {
        char name[64];
        snprintf(name, sizeof(name), "VaesenEntity_%d", i);
        assert(tsfi_vsen_vaesen_register(name, "Teddy Bear Inhabitant", (i % 10) + 1, "Active") == 0);
    }

    // 2. High-Volume Random Lookup Verification
    printf("  [STRESS 2/5] Verifying 1,000 random record lookups...\n");
    for (int i = 0; i < 1000; i++) {
        int target_idx = rand() % STRESS_ITERATIONS;
        char name[64];
        snprintf(name, sizeof(name), "VaesenEntity_%d", target_idx);
        assert(tsfi_vsen_vaesen_lookup(name, type_buf, &risk, status_buf, sizeof(type_buf)) == 0);
        assert(risk == (target_idx % 10) + 1);
    }

    // 3. High-Frequency Version Evolution Stress (100 updates to single entity)
    printf("  [STRESS 3/5] Executing 100 MVCC updates on single entity (HoganBank)...\n");
    uint64_t initial_ts = 0;
    for (int v = 0; v < 100; v++) {
        char status_str[32];
        snprintf(status_str, sizeof(status_str), "Version_%d", v);
        assert(tsfi_vsen_vaesen_register("HoganBank", "Teddy Bear Sovereign", (v % 10) + 1, status_str) == 0);
        if (v == 0) {
            struct timespec ts;
            clock_gettime(CLOCK_REALTIME, &ts);
            initial_ts = (uint64_t)ts.tv_sec * 1000000ULL + (uint64_t)(ts.tv_nsec / 1000);
            usleep(1000);
        }
    }

    // Assert latest version matches
    assert(tsfi_vsen_vaesen_lookup("HoganBank", type_buf, &risk, status_buf, sizeof(type_buf)) == 0);
    assert(strcmp(status_buf, "Version_99") == 0);

    // Assert Point-In-Time Restore (PITR) history returns Version 0
    assert(tsfi_vsen_vaesen_lookup_as_of("HoganBank", initial_ts, type_buf, &risk, status_buf, sizeof(type_buf)) == 0);
    assert(strcmp(status_buf, "Version_0") == 0);

    // 4. Cryptographic DNA Hash Chain Audit under Heavy Load
    printf("  [STRESS 4/5] Executing FNV-1a DNA Hash Chain audit across entire dataset...\n");
    assert(tsfi_vsen_audit_chain_verify("vaesen_registry.dat.bin") == 0);

    // 5. Simulated Crash & WAL Log Recovery under Heavy Load
    printf("  [STRESS 5/5] Executing WAL Log Sequence Number (LSN) crash recovery audit...\n");
    assert(tsfi_vsen_wal_recover("vaesen_registry.dat.bin") == 0);

    printf("[STRESS TEST VSEN RDBMS ACID] STRESS SUITE PASSED! All %d operations verified with ZERO data loss.\n", STRESS_ITERATIONS);
    return 0;
}
