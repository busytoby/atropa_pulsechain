#include "tsfi_taste_cache.h"
#include "tsfi_puppetry.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

/**
 * --- TASTE_TEDDY_FUR ---
 * Exercises "Fuzzy" taste using MIN-resolve and ICPC variance math.
 */
static void solve_teddy_fur(TsfiSubjectiveAwareness *out) {
    for (int i = 0; i < 16; i++) {
        // High-variance matted texture secrets
        out->atom.secrets[i] = (float)(i % 4) * 0.414f;
    }
    out->htile_mask = 0x00FFFFFF; // Full body fur awareness
    out->guardband  = 0.8f;       // Large guardband for volumetrics
    out->vop_seeds  = TASTE_RESOLVE_MIN | 0x1; // Exercise MIN resolve
    out->msaa_samples = 8;        // High-quality fur needs 8x MSAA
    out->sdma_jump = 2;           // Jump to Slot 2 for fur complications
}

/**
 * --- TASTE_TEDDY_EYES ---
 * Exercises "Rigid" taste for extreme intensity.
 */
static void solve_teddy_eyes(TsfiSubjectiveAwareness *out) {
    for (int i = 0; i < 16; i++) {
        out->atom.secrets[i] = 1.0f; // Max intensity
    }
    out->htile_mask = 0x000000FF; // Pinpoint eye awareness
    out->guardband  = 0.05f;      // Rigid, sharp edges
    out->vop_seeds  = TASTE_RESOLVE_MAX | 0x3; // Exercise MAX resolve
    out->msaa_samples = 1;        // Sharp eyes don't need MSAA softness
}

void tsfi_spider_fill_teddy_tastes(const char *cache_path) {
    tsfi_taste_cache_init(cache_path);

    // 1. TASTE_TEDDY_FUR (Layer-0)
    TsfiSubjectiveAwareness fur = {0};
    solve_teddy_fur(&fur);
    uint64_t fur_hash = tsfi_taste_hash(PUPPET_TYPE_TEDDY, 0x999, 0xF);
    tsfi_taste_cache_persist(fur_hash, &fur);

    // 2. TASTE_TEDDY_EYES (Layer-0)
    TsfiSubjectiveAwareness eyes = {0};
    solve_teddy_eyes(&eyes);
    uint64_t eye_hash = tsfi_taste_hash(PUPPET_TYPE_TEDDY, 0x999, 0xE);
    tsfi_taste_cache_persist(eye_hash, &eyes);

    tsfi_taste_cache_teardown();
    printf("[SPIDER] Teddy Bear Subjective Awarenesses anchored to cache.\n");
}

int main(int argc, char **argv) {
    const char *path = "assets/taste_cache.bin";
    if (argc > 1) path = argv[1];
    
    printf("[SPIDER] Initializing Teddy Bear Subjective Realization Protocol...\n");
    tsfi_spider_fill_teddy_tastes(path);
    return 0;
}
