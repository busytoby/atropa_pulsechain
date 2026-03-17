#include "tsfi_taste_cache.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

/**
 * --- TASTE_FUR_MATTED_WET ---
 * Slot 2, Entry 0: High-frequency clumped texture.
 */
static void solve_fur_matted(TsfiSubjectiveAwareness *out) {
    for (int i = 0; i < 16; i++) {
        out->atom.secrets[i] = (float)pow(0.414, (double)i / 8.0);
    }
    out->htile_mask = 0x0F0F0F0F; // Clumped awareness
    out->guardband  = 1.2f;       // Extended fringe for clumping
    out->vop_seeds  = TASTE_RESOLVE_MIN | 0x5;
    out->msaa_samples = 8;
}

/**
 * --- TASTE_FUR_WIND_RIPPLE ---
 * Slot 2, Entry 1: Kinetic strand motion.
 */
static void solve_fur_ripple(TsfiSubjectiveAwareness *out) {
    for (int i = 0; i < 16; i++) {
        out->atom.secrets[i] = (float)sin(i * 0.999);
    }
    out->htile_mask = 0xF0F0F0F0; // Ripple awareness
    out->guardband  = 0.5f;
    out->vop_seeds  = TASTE_RESOLVE_MIN | 0x9;
    out->msaa_samples = 4;
}

void tsfi_spider_fill_fur_complications(const char *cache_path) {
    tsfi_taste_cache_init(cache_path);

    // MIGRATION: Jump to Slot 2
    if (!tsfi_taste_cache_migrate(2)) {
        fprintf(stderr, "[FRACTURE] SDMA Migration to Slot 2 Failed.\n");
        return;
    }

    // 1. TASTE_FUR_MATTED_WET
    TsfiSubjectiveAwareness matted = {0};
    solve_fur_matted(&matted);
    uint64_t matted_hash = 0xF001; // Specific Complication Hash
    tsfi_taste_cache_persist(matted_hash, &matted);

    // 2. TASTE_FUR_WIND_RIPPLE
    TsfiSubjectiveAwareness ripple = {0};
    solve_fur_ripple(&ripple);
    uint64_t ripple_hash = 0xF002;
    tsfi_taste_cache_persist(ripple_hash, &ripple);

    tsfi_taste_cache_teardown();
    printf("[SPIDER] Fur Complications anchored to Slot 2 Awareness Registry.\n");
}

int main(int argc, char **argv) {
    const char *path = "assets/taste_cache.bin";
    if (argc > 1) path = argv[1];
    
    printf("[SPIDER] Initializing Slot 2 SDMA Complication Protocol...\n");
    tsfi_spider_fill_fur_complications(path);
    return 0;
}
