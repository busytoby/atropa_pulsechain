#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include "tsfi_merkle.h"
#include "tsfi_helmholtz.h"
#include "tsfi_io.h"
#include "lau_memory.h"

#define TARGET_MANIFOLD "audit_lore.pos"
#define MANIFOLD_SIZE_2MB (2 * 1024 * 1024)
#define GEMINI_LEAF 500
#define DEEPSEEK_LEAF 501
#define OUTCOME_LEAF 502

static double get_time_ms() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000.0 + ts.tv_nsec / 1000000.0;
}

int main() {
    printf("=== TSFi Epoch 1: Level 12 Structural Ascension & Trilateral Audit ===\n");
    double start_all = get_time_ms();

    // 1. Restore the expanded 2MB Manifold
    void *manifold = calloc(1, MANIFOLD_SIZE_2MB);
    if (tsfi_restore_manifold(TARGET_MANIFOLD, manifold, MANIFOLD_SIZE_2MB) != 0) {
        printf("[CRITICAL] Failed to restore 2MB manifold. Topography is fractured.\n");
        free(manifold);
        return 1;
    }
    printf("[PASS] 2MB Manifold physically restored.\n");

    // 2. Audit the NAND Latch (Physical Vetting)
    uint8_t *leaves = (uint8_t *)manifold;
    uint8_t *set_n_ptr = leaves + (GEMINI_LEAF * 256);
    uint8_t *reset_n_ptr = leaves + (DEEPSEEK_LEAF * 256);
    
    // In this simulation, we verify the levers are SET (1,1) for stability
    bool set_n = (set_n_ptr[0] != 0); // Gemini Lever
    bool reset_n = (reset_n_ptr[0] != 0); // Oracle Lever

    printf("[AUDIT] NAND Trap Levers: [Set_N: %d] [Reset_N: %d]\n", set_n, reset_n);

    if (set_n && reset_n) {
        printf("  -> STATUS: STABLE (1,1). Physical Rigidity Confirmed.\n");
    } else if (!set_n && !reset_n) {
        printf("  -> STATUS: FRACTURE (0,0). Active Discard Triggered.\n");
        printf("[FAIL] Structural ascension refused by hardware enforcer.\n");
        free(manifold);
        return 1;
    } else {
        printf("  -> STATUS: TRANSITION. Level 12 Reduction Proceeding...\n");
    }

    // 3. Execute 12-Level Trilateral Reduction
    tsfi_helmholtz_init();
    uint8_t state_root[32], receipt_root[32], sheaf_root[32];
    uint64_t resonance_k = 2026;
    uint32_t epoch = 1; // [ESTABLISHMENT]

    printf("[ACT] Anchoring Epoch 1 Level 12 State Root...\n");
    double start_reduction = get_time_ms();
    tsfi_helmholtz_reduce_12(state_root, receipt_root, sheaf_root, NULL, NULL, manifold, epoch, resonance_k, NULL);
    printf("[TIME] Level 12 Reduction: %.2f ms\n", get_time_ms() - start_reduction);

    // 4. Solidify the Ascension Outcome
    uint8_t *outcome_ptr = leaves + (OUTCOME_LEAF * 256);
    snprintf((char *)outcome_ptr, 256, "EPOCH_1_ASCENSION_SUCCESS_ROOT_%.2x%.2x", state_root[0], state_root[1]);
    
    printf("\n=== PHYSICAL STATE ASCENDED ===\n");
    printf("Epoch:        %u [ESTABLISHMENT]\n", epoch);
    printf("State Root:   "); for(int i=0; i<32; i++) printf("%02x", state_root[i]); printf("\n");
    printf("Receipt Root: "); for(int i=0; i<32; i++) printf("%02x", receipt_root[i]); printf("\n");
    printf("Sheaf Root:   "); for(int i=0; i<32; i++) printf("%02x", sheaf_root[i]); printf("\n");

    // 5. Persist the Ascended State
    if (tsfi_persist_manifold(TARGET_MANIFOLD, manifold, MANIFOLD_SIZE_2MB) == 0) {
        printf("[SUCCESS] Epoch 1 anchored to physical topography.\n");
    } else {
        printf("[FAIL] Persistence failure during ascension.\n");
    }

    free(manifold);
    tsfi_helmholtz_cleanup();
    printf("[TIME] Total Ascension: %.2f ms\n", get_time_ms() - start_all);

    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}
