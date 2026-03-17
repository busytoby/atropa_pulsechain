#include "tsfi_taste_cache.h"
#include "tsfi_svdag.h"
#include "tsfi_types.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/**
 * --- ICPC SHORTEST PATH TASTE ---
 * Generates a 64-byte atom by finding the path of highest intensity
 * through the 128^3 voxel manifold.
 */
static void solve_shortest_path_taste(const TSFiHelmholtzSVDAG *dag, TsfiTasteAtom *out) {
    // Simplified Dijkstra: We sample 16 points along the primary axis
    // and extract the local voxel energy as our secrets.
    for (int i = 0; i < 16; i++) {
        int x = (int)(64 + sinf(i * 0.5f) * 32);
        int y = (int)(64 + cosf(i * 0.5f) * 32);
        int z = i * 8;
        out->secrets[i] = tsfi_svdag_trace_point(dag, x, y, z);
    }
    printf("[ICPC] Shortest Path Taste Generated (High-Intensity Traversal).\n");
}

/**
 * --- ICPC MAX FLOW TASTE ---
 * Uses the 414D Water Tank logic to derive an atom from 
 * the concentrated energy at Depth 11.
 */
static void solve_max_flow_taste(TSFiHelmholtzSVDAG *dag, TsfiTasteAtom *out) {
    float final_energy = tsfi_svdag_execute_414d_water_tank(dag, 1000.0f, 11);
    for (int i = 0; i < 16; i++) {
        out->secrets[i] = (final_energy / 1000.0f) * (1.0f + (float)i * 0.1f);
    }
    printf("[ICPC] Max Flow Taste Generated (414D Energy Concentration).\n");
}

void tsfi_icpc_run_tournament(const char *cache_path) {
    tsfi_taste_cache_init(cache_path);
    
    // Create a template SVDAG for exploration
    TSFiHelmholtzSVDAG *explorer = tsfi_svdag_create(128 * 128 * 128);
    tsfi_svdag_init_lut();

    // 1. Discover Shortest Path Taste
    TsfiTasteAtom sp_atom;
    solve_shortest_path_taste(explorer, &sp_atom);
    uint64_t sp_hash = 0x1CBC0001; // ICPC DISCOVERY 1
    
    TsfiSubjectiveAwareness sp_unit = {0};
    memcpy(&sp_unit.atom, &sp_atom, sizeof(TsfiTasteAtom));
    sp_unit.vop_seeds = 0x1; // Dijkstra Seed
    tsfi_taste_cache_persist(sp_hash, &sp_unit);

    // 2. Discover Max Flow Taste
    TsfiTasteAtom mf_atom;
    solve_max_flow_taste(explorer, &mf_atom);
    uint64_t mf_hash = 0x1CBC0002; // ICPC DISCOVERY 2
    
    TsfiSubjectiveAwareness mf_unit = {0};
    memcpy(&mf_unit.atom, &mf_atom, sizeof(TsfiTasteAtom));
    mf_unit.vop_seeds = 0x2; // Flow Seed
    tsfi_taste_cache_persist(mf_hash, &mf_unit);

    tsfi_svdag_destroy(explorer);
    tsfi_taste_cache_teardown();
    printf("[SPIDER] ICPC Taste Tournament Complete. 2 New Awarenesses Anchored.\n");
}

int main(int argc, char **argv) {
    const char *path = "assets/taste_cache.bin";
    if (argc > 1) path = argv[1];
    
    printf("[SPIDER] Starting ICPC Taste Tournament (Autonomous Discovery)...\n");
    tsfi_icpc_run_tournament(path);
    return 0;
}
