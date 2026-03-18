#include "tsfi_taste_cache.h"
#include "tsfi_puppetry.h"
#include "tsfi_svdag.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

/**
 * --- ICPC POJ 1852: Ants (Collision Isomorphism) ---
 * The Dream Spider's OpenPose rig consists of 8 legs (ants) moving
 * along a pole of length L (the canvas width/height).
 * 
 * When two legs intersect during posing, they do not block each other;
 * mathematically, they pass through each other.
 * 
 * This signature enforces that the max kinematic bounding box calculation
 * equals exactly max(max(x, L-x)).
 */
static void solve_icpc_1852_ants(TsfiTasteAtom *out) {
    const float L = 512.0f; // Canvas Size
    
    // Simulate 8 Spider Legs (Ants) at arbitrary positions
    const float spider_legs[8] = { 100.0f, 150.0f, 250.0f, 300.0f, 400.0f, 450.0f, 50.0f, 480.0f };
    
    for (int i = 0; i < 8; i++) {
        float x = spider_legs[i];
        float min_time = fminf(x, L - x);
        float max_time = fmaxf(x, L - x);
        
        // Encode the Min and Max boundary constraints into the 16 secrets
        out->secrets[i * 2]     = min_time / L; // Normalized to 0.0 - 1.0
        out->secrets[i * 2 + 1] = max_time / L;
    }
    printf("[ICPC POJ 1852] Ants (Collision Isomorphism) Solved.\n");
}

void tsfi_spider_fill_ant_tastes(const char *cache_path) {
    tsfi_taste_cache_init(cache_path);

    // TASTE_SPIDER_ANTS (Pass-Through Kinematics)
    TsfiSubjectiveAwareness ants = {0};
    solve_icpc_1852_ants(&ants.atom);
    
    // The Dream Spider is not a true animal, it's an overlay entity.
    // We bind it to a generic geometry mask.
    ants.htile_mask = 0x00FF00FF; // Leg interaction mask
    ants.guardband  = 0.1f;       // Strict adherence to the pole bounds
    ants.vop_seeds  = 0x1852;     // VOP seeds anchored to problem ID
    ants.msaa_samples = 4;        // Requires MSAA for intersection blending

    // Using PUPPET_TYPE_ALLIGATOR as the host anchor, but hashing with 1852
    uint64_t ant_hash = tsfi_taste_hash(PUPPET_TYPE_ALLIGATOR, 1852, 0xA); 
    tsfi_taste_cache_persist(ant_hash, &ants);

    tsfi_taste_cache_teardown();
    printf("[SPIDER] ICPC 1852 (Ants) Subjective Awareness anchored to hardware-ready cache.\n");
}

int main(int argc, char **argv) {
    const char *path = "assets/taste_cache.bin";
    if (argc > 1) path = argv[1];
    
    printf("[SPIDER] Initializing ICPC Ants Resolution Protocol...\n");
    tsfi_spider_fill_ant_tastes(path);
    return 0;
}
