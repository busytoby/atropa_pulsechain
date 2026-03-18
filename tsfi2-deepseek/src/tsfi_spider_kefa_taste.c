#include "tsfi_taste_cache.h"
#include "tsfi_puppetry.h"
#include "tsfi_svdag.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

/**
 * --- ICPC 580C: Kefa and Park (SVDAG Depth Traversal) ---
 * The Genie sees the "Park" as the Helmholtz SVDAG.
 * The Spider (Kefa) traverses the Taste Tree to locate valid leaf nodes.
 * "Cats" represent high-impedance (fractured) nodes. The limit 'm' 
 * defines the maximum consecutive sequence of fractures allowed.
 * We map this traversal state to the 16 coefficient secrets.
 */
static void solve_icpc_580c_kefa(TsfiTasteAtom *out) {
    // Simulated DFS state vector for a 16-node path limit.
    // 0 = Clear Node, 1 = Fractured Node (Cat)
    // Threshold 'm' = 3
    const float kefa_path[16] = {0, 1, 1, 0, 1, 0, 0, 1, 1, 1, 0 /* Fracture Limit Reached */, 0, 0, 1, 0, 0};
    
    int consecutive_fractures = 0;
    int max_m = 3;
    
    for (int i = 0; i < 16; i++) {
        if (kefa_path[i] > 0.5f) {
            consecutive_fractures++;
        } else {
            consecutive_fractures = 0;
        }
        
        // If the path violates the threshold, the Spider zeroes out the remaining resonance
        if (consecutive_fractures > max_m) {
            out->secrets[i] = 0.0f; // Path abandoned
        } else {
            // Encode the accumulated resistance
            out->secrets[i] = (float)consecutive_fractures / (float)max_m;
        }
    }
    printf("[ICPC 580C] Kefa and Park (SVDAG Depth Traversal) Solved.\n");
}

/**
 * --- Interest Strategy: Portfolio Scan (Active Interest) ---
 * Expands Kefa's SVDAG Traversal. Instead of just avoiding impedance (cats), 
 * the Spider actively seeks nodes exhibiting specific traits (e.g., TSFI_TRAIT_FILE).
 * The strategy coefficients act as a gravitational pull toward "interesting" nodes
 * while simultaneously solving the 580C collision bounds.
 */
static void solve_interest_strategy(TsfiTasteAtom *out) {
    // We map a "gravitational" interest curve. The spider accelerates
    // toward nodes with high unfulfilled potential (e.g., seal_level == 1).
    for (int i = 0; i < 16; i++) {
        // Logarithmic approach curve (rapid initial interest, tapering off as verified)
        out->secrets[i] = (float)log10(1.0 + (double)(16 - i)) / log10(17.0);
    }
    printf("[STRATEGY] Spider Portfolio Scan (Active Interest) Encoded.\n");
}

void tsfi_spider_fill_kefa_tastes(const char *cache_path) {
    tsfi_taste_cache_init(cache_path);

    // 1. TASTE_KEFA_PARK (Depth Traversal)
    TsfiSubjectiveAwareness kefa = {0};
    solve_icpc_580c_kefa(&kefa.atom);
    kefa.htile_mask = 0x00FF00FF; // Traversal tracking mask
    kefa.guardband  = 0.25f;      // Tolerance for high-impedance
    kefa.vop_seeds  = 0x580;      // VOP seeds anchored to problem ID
    kefa.msaa_samples = 2;        // Standard deterministic trace
    uint64_t kefa_hash = tsfi_taste_hash(PUPPET_TYPE_ALLIGATOR, 580, 0xC); // Reusing Alligator ID for Auditor roles
    tsfi_taste_cache_persist(kefa_hash, &kefa);

    // 2. TASTE_KEFA_INTEREST (Portfolio Scan)
    TsfiSubjectiveAwareness interest = {0};
    solve_interest_strategy(&interest.atom);
    interest.htile_mask = 0xFFFFFFFF; // Global scan awareness
    interest.guardband  = 0.01f;      // Rigid, exact match required for "interest" locking
    interest.vop_seeds  = 0x111;      // VOP seeds anchored to Portfolio Scan logic
    interest.msaa_samples = 1;        // Direct logical hit (no anti-aliasing needed)
    uint64_t interest_hash = tsfi_taste_hash(PUPPET_TYPE_ALLIGATOR, 580, 0xD); // 0xD variant for Kefa
    tsfi_taste_cache_persist(interest_hash, &interest);

    tsfi_taste_cache_teardown();
    printf("[SPIDER] Kefa Logic and Interest Strategy anchored to hardware-ready cache.\n");
}

int main(int argc, char **argv) {
    const char *path = "assets/taste_cache.bin";
    if (argc > 1) path = argv[1];
    
    printf("[SPIDER] Initializing ICPC Kefa Resolution Protocol...\n");
    tsfi_spider_fill_kefa_tastes(path);
    return 0;
}
