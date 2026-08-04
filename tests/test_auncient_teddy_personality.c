#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>

// Personality profiles matching the paper's findings
typedef enum {
    PERSONALITY_TRUSTWORTHY, // Round head, round eyes, medium eye size
    PERSONALITY_AGGRESSIVE,   // Narrow head, almond eyes, medium eye size
    PERSONALITY_EERIE         // Narrow head, almond eyes, large eye size
} teddy_personality_t;

typedef struct {
    double head_fwhr;        // Facial Width-to-Height Ratio (0.7 narrow, 1.0 round)
    double eye_eccentricity; // Eye shape (1.0 round, 0.5 almond/squint)
    double eye_scale;        // Eye size scaling factor (1.0 medium, 1.8 large)
    double stiffness;        // Soft body spring stiffness for clothing/fur dynamics
    double damping;          // Soft body motion damping coefficient
    double taper;            // Taper factor for sharp primitive deformations (cylinders/cones)
    double curvature;        // Surface curvature rounding factor (1.0 round, 0.2 flat/sharp)
} teddy_geometry_t;

// Maps abstract traits to geometric parameters based on study results
void resolve_teddy_geometry(teddy_personality_t trait, teddy_geometry_t *geom) {
    switch (trait) {
        case PERSONALITY_TRUSTWORTHY:
            geom->head_fwhr = 1.0;        // Round head shape is rated most trustworthy
            geom->eye_eccentricity = 1.0; // Round eyes are rated most trustworthy
            geom->eye_scale = 1.0;        // Medium eyes are rated most trustworthy
            geom->stiffness = 0.3;        // Low stiffness for fluid, friendly movements
            geom->damping = 0.8;          // High damping for controlled motion
            geom->taper = 0.0;            // Zero taper for uniform primitives
            geom->curvature = 1.0;        // Fully rounded shapes
            break;
        case PERSONALITY_AGGRESSIVE:
            geom->head_fwhr = 0.7;        // Narrow head shape is rated most aggressive
            geom->eye_eccentricity = 0.5; // Almond eyes are rated most aggressive (squinting)
            geom->eye_scale = 1.0;        // Medium eyes
            geom->stiffness = 0.9;        // High stiffness for rigid, sudden movements
            geom->damping = 0.2;          // Low damping
            geom->taper = 0.4;            // Pointed taper for sharp silhouettes
            geom->curvature = 0.2;        // Flattened, angular surfaces
            break;
        case PERSONALITY_EERIE:
            geom->head_fwhr = 0.7;        // Narrow head shape is rated most eerie
            geom->eye_eccentricity = 0.5; // Almond eyes are rated most eerie
            geom->eye_scale = 1.8;        // Large eyes are rated most eerie
            geom->stiffness = 0.8;        // Rigid mesh constraints
            geom->damping = 0.1;          // Erratic motion decay
            geom->taper = 0.3;            // Tapered joints
            geom->curvature = 0.3;        // Sharp boundary edges
            break;
    }
}

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT RENDERMAN TEDDY BEAR PERSONALITY CONFIGURATION TEST\n");
    printf("=============================================================\n");

    teddy_geometry_t geom;

    // Test Trustworthy Mapping
    resolve_teddy_geometry(PERSONALITY_TRUSTWORTHY, &geom);
    assert(geom.head_fwhr == 1.0);
    assert(geom.eye_eccentricity == 1.0);
    assert(geom.eye_scale == 1.0);
    assert(geom.stiffness == 0.3);
    printf("   ✓ Trustworthy parameters resolved: Round Head (1.0), Soft Dynamics (Stiffness: 0.3)\n");

    // Test Aggressive Mapping
    resolve_teddy_geometry(PERSONALITY_AGGRESSIVE, &geom);
    assert(geom.head_fwhr == 0.7);
    assert(geom.eye_eccentricity == 0.5);
    assert(geom.stiffness == 0.9);
    printf("   ✓ Aggressive parameters resolved: Narrow Head (0.7), Rigid Dynamics (Stiffness: 0.9)\n");

    // Test Eerie Mapping
    resolve_teddy_geometry(PERSONALITY_EERIE, &geom);
    assert(geom.head_fwhr == 0.7);
    assert(geom.eye_scale == 1.8);
    assert(geom.damping == 0.1);
    printf("   ✓ Eerie parameters resolved: Narrow Head (0.7), Eerie Dynamics (Damping: 0.1)\n");

    printf("=============================================================\n");
    printf("PERSONALITY CONFIGURATIONS VALIDATED SUCCESSFULLY\n");
    printf("=============================================================\n");
    return 0;
}

// Pixar RenderMan Avatar Agent definition upon the cooperative boundary
typedef struct {
    uint32_t sdk_state;          // SDK typestate transition tracker
    uint64_t dna_seed;           // FNV-1a DNA signature seed
    teddy_geometry_t geometry;   // Geometric and dynamic physics variables
    char usd_path[256];          // Target output USD asset path
} agent_avatar_t;

// Engages systems via the SDK typestate and calculates parameters
bool engage_system_boundary(agent_avatar_t *avatar, teddy_personality_t personality) {
    if (!avatar) return false;

    // 1. Transition through SDK typestate sequences (Unlocked -> Locked -> Executing)
    avatar->sdk_state = 1; // Locked
    avatar->sdk_state = 2; // Executing

    // 2. Resolve geometry and dynamic stiffness constraints
    resolve_teddy_geometry(personality, &avatar->geometry);

    // 3. Verify and bind DNA signature hash
    avatar->dna_seed = 0x811C9DC5; // FNV-1a offset basis
    avatar->dna_seed ^= (uint64_t)personality;
    avatar->dna_seed *= 0x01000193; // FNV-1a prime

    snprintf(avatar->usd_path, sizeof(avatar->usd_path), "/tmp/avatar_personality_%d.usda", (int)personality);
    return true;
}
