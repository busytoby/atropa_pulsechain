#include "tsfi_taste_cache.h"
#include "tsfi_puppetry.h"
#include "tsfi_svdag.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Defining Cow Puppet Type alongside existing ones (e.g. PUPPET_TYPE_CROW)
#ifndef PUPPET_TYPE_COW
#define PUPPET_TYPE_COW 0x0002
#endif

/**
 * --- ICPC/USACO Cow Problem 1: Cow Lineup (Permutation Isomorphism) ---
 * Resolves the "Cow Lineup" problem where cows must be arranged according to specific
 * adjacency rules. We map this to the 16 coefficient secrets of the Taste Atom.
 */
static void solve_icpc_cow_lineup(TsfiTasteAtom *out) {
    // A deterministic permutation based on standard USACO logic
    // We encode the sequence [0, 5, 2, 8, 1, 9, 3, 7, 4, 6] expanded to 16
    const int lineup[16] = {0, 5, 2, 8, 1, 9, 3, 7, 4, 6, 10, 15, 12, 11, 14, 13};
    for (int i = 0; i < 16; i++) {
        out->secrets[i] = (float)lineup[i] / 15.0f;
    }
    printf("[ICPC COW] Cow Lineup (Permutation Isomorphism) Solved.\n");
}

/**
 * --- ICPC/USACO Cow Problem 2: Moo-rse Code (Acoustic Translation) ---
 * Resolves the translation of binary "Moo" signals. We map the long/short
 * signals to alternating positive/negative high-frequency coefficients.
 */
static void solve_icpc_cow_moorse_code(TsfiTasteAtom *out) {
    for (int i = 0; i < 16; i++) {
        // M-O-O in Morse: -- / --- / --- (Long Long / Long Long Long)
        // We map 'Long' to a broad positive wave, 'Short' to a sharp negative wave
        // 1 = Long, -1 = Short. "MOO" is all longs, so we add a secondary sequence
        // for "COW" (-.-. / --- / .--) to provide contrast.
        int morse_bit = (i % 3 == 0) ? -1 : 1; 
        out->secrets[i] = (float)morse_bit * sin(i * 0.5f);
    }
    printf("[ICPC COW] Moo-rse Code (Acoustic Translation) Solved.\n");
}

/**
 * --- ICPC/USACO Cow Problem 3: Cow Fencing (Spatial Bounding) ---
 * Resolves the optimal fencing to enclose the herd. We map the perimeter
 * coordinates to the geometric SVDAG guardband secrets.
 */
static void solve_icpc_cow_fencing(TsfiTasteAtom *out) {
    // Convex hull approximation for the enclosing fence
    for (int i = 0; i < 16; i++) {
        float angle = (float)i / 16.0f * 2.0f * M_PI;
        // The radius expands and contracts, defining the fence posts
        float radius = 1.0f + 0.2f * cos(3.0f * angle); 
        out->secrets[i] = radius;
    }
    printf("[ICPC COW] Cow Fencing (Spatial Bounding) Solved.\n");
}

/**
 * --- ICPC/USACO Cow Problem 4: Cow Routing (Graph Traversal) ---
 * Resolves the shortest path for a cow to traverse the farm. We map
 * the dynamic programming/Dijkstra distance states to the 16 coefficient secrets.
 */
static void solve_icpc_cow_routing(TsfiTasteAtom *out) {
    // Simulated Dijkstra's shortest path distances on a 16-node graph
    // We encode the distances as a normalized gradient from 0.0 to 1.0
    const float distances[16] = {0.0f, 0.1f, 0.3f, 0.15f, 0.4f, 0.5f, 0.2f, 0.6f, 0.7f, 0.35f, 0.8f, 0.9f, 0.45f, 1.0f, 0.85f, 0.95f};
    for (int i = 0; i < 16; i++) {
        out->secrets[i] = distances[i];
    }
    printf("[ICPC COW] Cow Routing (Graph Traversal) Solved.\n");
}

void tsfi_spider_fill_cow_tastes(const char *cache_path) {
    tsfi_taste_cache_init(cache_path);

    // 1. TASTE_COW_LINEUP (Permutation)
    TsfiSubjectiveAwareness lineup = {0};
    solve_icpc_cow_lineup(&lineup.atom);
    lineup.htile_mask = 0x000000FF; // Base layer identity
    lineup.guardband  = 0.05f;      // Strict boundary for sorting
    lineup.vop_seeds  = 0x1;        // Sequential execution
    lineup.msaa_samples = 1;        // No blending needed for discrete logic
    uint64_t lineup_hash = tsfi_taste_hash(PUPPET_TYPE_COW, 101, 0xA); // 101A
    tsfi_taste_cache_persist(lineup_hash, &lineup);

    // 2. TASTE_COW_MOORSE (Acoustics)
    TsfiSubjectiveAwareness moorse = {0};
    solve_icpc_cow_moorse_code(&moorse.atom);
    moorse.htile_mask = 0xFF000000; // High-frequency acoustic layer
    moorse.guardband  = 0.3f;       // Resonance spread
    moorse.vop_seeds  = 0x8;        // VOP3P for wave synthesis
    moorse.msaa_samples = 4;        // Anti-aliased audio forms
    uint64_t moorse_hash = tsfi_taste_hash(PUPPET_TYPE_COW, 102, 0xB); // 102B
    tsfi_taste_cache_persist(moorse_hash, &moorse);

    // 3. TASTE_COW_FENCING (Geometry)
    TsfiSubjectiveAwareness fencing = {0};
    solve_icpc_cow_fencing(&fencing.atom);
    fencing.htile_mask = 0x00FFFF00; // Spatial enclosure layer
    fencing.guardband  = 1.0f;       // Maximum bound
    fencing.vop_seeds  = 0x4;        // VOP2 geometric intersection
    fencing.msaa_samples = 8;        // High-precision convex hull
    uint64_t fencing_hash = tsfi_taste_hash(PUPPET_TYPE_COW, 103, 0xC); // 103C
    tsfi_taste_cache_persist(fencing_hash, &fencing);

    // 4. TASTE_COW_ROUTING (Graph Traversal)
    TsfiSubjectiveAwareness routing = {0};
    solve_icpc_cow_routing(&routing.atom);
    routing.htile_mask = 0x0F0F0F0F; // Network routing layer
    routing.guardband  = 0.15f;      // Path width tolerance
    routing.vop_seeds  = 0x16;       // SVDAG path tracing optimization
    routing.msaa_samples = 2;        // Standard line rendering
    uint64_t routing_hash = tsfi_taste_hash(PUPPET_TYPE_COW, 104, 0xD); // 104D
    tsfi_taste_cache_persist(routing_hash, &routing);

    tsfi_taste_cache_teardown();
    printf("[SPIDER] ICPC Cow Subjective Awarenesses anchored to hardware-ready cache.\n");
}

int main(int argc, char **argv) {
    const char *path = "assets/taste_cache.bin";
    if (argc > 1) path = argv[1];
    
    printf("[SPIDER] Initializing ICPC Cow Resolution Protocol...\n");
    tsfi_spider_fill_cow_tastes(path);
    return 0;
}