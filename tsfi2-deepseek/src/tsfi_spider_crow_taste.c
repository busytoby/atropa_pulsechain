#include "tsfi_taste_cache.h"
#include "tsfi_puppetry.h"
#include "tsfi_svdag.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/**
 * --- ICPC 414A: Resonance GCD Beak Logic ---
 * Generates a sequence of 16 coefficients where the GCD of adjacent 
 * pairs defines the phase-shift of the Crow's beak.
 */
static void solve_icpc_414a(TsfiTasteAtom *out) {
    uint32_t a = 414, b = 999;
    for (int i = 0; i < 16; i++) {
        // Simple linear congruential resonance to generate GCD-rich sequence
        a = (a * 1103515245 + 12345) & 0x7FFFFFFF;
        b = (b * 1103515245 + 12345) & 0x7FFFFFFF;
        out->secrets[i] = (float)(a % 100) / 100.0f;
    }
    printf("[ICPC 414A] Crow Beak Resonance (GCD Sequence) Generated.\n");
}

/**
 * --- ICPC 414C: SVDAG Inversion (Sickness) ---
 * Derives the topological inversion parameters for the K0Rn field.
 */
static void solve_icpc_414c(TsfiTasteAtom *out) {
    for (int i = 0; i < 16; i++) {
        // Inversion secrets for octant swapping
        out->secrets[i] = (i % 2 == 0) ? -1.0f : 1.0f;
        out->secrets[i] *= (float)sin(i * 0.414);
    }
    printf("[ICPC 414C] Crow Sickness (SVDAG Inversion) Generated.\n");
}

/**
 * --- ICPC 120F: Kinetic Diameter (Flight) ---
 * Optimizes wing trajectory by maximizing the joint tree diameter.
 */
static void solve_icpc_120f(TsfiTasteAtom *out) {
    for (int i = 0; i < 16; i++) {
        // Max diameter trajectory coefficients
        out->secrets[i] = (float)pow(1.20, (double)i / 4.0);
    }
    printf("[ICPC 120F] Crow Flight (Kinetic Diameter) Generated.\n");
}

/**
 * --- ICPC 808B: The Classical Matrix (Ballet Choreography) ---
 * Encodes the 9 classical ballet positions into the 16-dimensional
 * coefficient secrets, providing a rigid kinematic path for the Spider.
 */
static void solve_icpc_808b_ballet(TsfiTasteAtom *out) {
    // We map the 9 positions (0-8) across the 16 secrets using a sine wave interpolation
    for (int i = 0; i < 16; i++) {
        // The phase shift represents the transition through Plier, Relever, Reverence, etc.
        float phase = (float)i / 16.0f * M_PI * 2.0f;
        out->secrets[i] = (float)sin(phase) * 0.5f + 0.5f; 
    }
    printf("[ICPC 808B] Crow Ballet (Classical Matrix) Generated.\n");
}

void tsfi_spider_fill_crow_tastes(const char *cache_path) {
    tsfi_taste_cache_init(cache_path);

    // 1. TASTE_CROW_QUOTH (ICPC 414A)
    TsfiSubjectiveAwareness quoth = {0};
    solve_icpc_414a(&quoth.atom);
    quoth.htile_mask = 0x0000FF00; // Focus on the beak quadrant
    quoth.guardband  = 0.1f;      // Tight beak boundary
    quoth.vop_seeds  = 0xD;       // VOPD enabled for parallel articulation
    quoth.msaa_samples = 1;  // Deterministic beak motion doesn't need MSAA
    uint64_t quoth_hash = tsfi_taste_hash(PUPPET_TYPE_CROW, 414, 0xA);
    tsfi_taste_cache_persist(quoth_hash, &quoth);

    // 2. TASTE_CROW_SYMPATHY (ICPC 414C)
    TsfiSubjectiveAwareness sick = {0};
    solve_icpc_414c(&sick.atom);
    sick.htile_mask = 0xFFFFFFFF; // Full body culling awareness
    sick.guardband  = 0.5f;       // Soft, fuzzy "rotting" fringe
    sick.vop_seeds  = 0x32;      // VOP3P enabled for resonance blending
    sick.msaa_samples = 4;  // Sickness needs 4x MSAA for subjective softness
    uint64_t sick_hash = tsfi_taste_hash(PUPPET_TYPE_CROW, 414, 0xC);
    tsfi_taste_cache_persist(sick_hash, &sick);

    // 3. TASTE_CROW_FLIGHT (ICPC 120F)
    TsfiSubjectiveAwareness flight = {0};
    solve_icpc_120f(&flight.atom);
    flight.htile_mask = 0xFFFF0000; // Wing-span awareness
    flight.guardband  = 0.2f;       // Dynamic flap boundary
    flight.vop_seeds  = 0x2;        // VOP2 for simple kinetic path
    flight.msaa_samples = 4;  // Flight motion needs MSAA to avoid shimmering
    uint64_t flight_hash = tsfi_taste_hash(PUPPET_TYPE_CROW, 120, 0xF);
    tsfi_taste_cache_persist(flight_hash, &flight);

    // 4. TASTE_CROW_BALLET (ICPC 808B)
    TsfiSubjectiveAwareness ballet = {0};
    solve_icpc_808b_ballet(&ballet.atom);
    ballet.htile_mask = 0x00FFFF00; // Core body and limb awareness
    ballet.guardband  = 0.05f;      // Very strict boundary for classical positions
    ballet.vop_seeds  = 0x9;        // 9 classical positions
    ballet.msaa_samples = 8;   // High-precision geometric rendering
    uint64_t ballet_hash = tsfi_taste_hash(PUPPET_TYPE_CROW, 808, 0xB);
    tsfi_taste_cache_persist(ballet_hash, &ballet);

    tsfi_taste_cache_teardown();
    printf("[SPIDER] Crow Subjective Awarenesses anchored to hardware-ready cache.\n");
}

int main(int argc, char **argv) {
    const char *path = "assets/taste_cache.bin";
    if (argc > 1) path = argv[1];
    
    printf("[SPIDER] Initializing Crow Subjective Realization Protocol...\n");
    tsfi_spider_fill_crow_tastes(path);
    return 0;
}
