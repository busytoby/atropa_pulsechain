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
    double symmetry;         // Bilateral face/eye symmetry (1.0 symmetric, 0.8 crooked-face)
    double jaw_scale;        // Lower jaw width scaling for perceived strength
    double neck_thickness;   // Neck capsule thickness for formidability cues
    double focal_length;     // Camera lens focal length (35mm baby-face, 85mm flat/dominant)
    double camera_elevation; // Camera vertical translation offset (positive high, negative low)
    double torso_ratio;      // Shoulder-to-hip width ratio (1.5 broad/formidable, 0.7 squat)
    double fur_roughness;    // Specular roughness coefficient for hair shading
    double feature_vertical_offset; // Vertical feature placement (negative lower/baby-face)
    double behavioral_mismatch;     // Cognitive mismatch factor (0.0 aligned, 1.0 high discrepancy)
    double vocal_pitch;             // Vocal fundamental frequency in Hz (85Hz deep, 250Hz child-like)
    double expression_scale;        // Animation expression sensitivity multiplier (e.g. 1.5 amplified)
    double leadership_profile;       // Leadership divergence (0.0 supportive, 1.0 dominant)
    double maturity_index;           // Perceived age maturity (0.0 young, 1.0 mature/experienced)
    double persuasion_strategy;      // Persuasion strategy (0.0 sincerity/trust, 1.0 expertise/capability)
    double empathy_index;            // Perceived empathy and gentleness (0.0 low, 1.0 high/nurturing)
    double protection_index;         // Perceived physical vulnerability (0.0 independent, 1.0 vulnerable)
    double size_index;               // Perceived height/size (0.0 smaller/shorter, 1.0 larger/taller)
    double intellect_index;          // Perceived intellectual capability (0.0 lower, 1.0 higher)
    double resilience_index;         // Perceived physical resilience (0.0 lower, 1.0 higher)
    double agility_index;            // Perceived physical agility/speed (0.0 lower, 1.0 higher)
    double safety_index;             // Perceived safety (0.0 dangerous/threat, 1.0 safe/harmless)
    double purity_index;             // Perceived purity/cleanliness (0.0 contaminated, 1.0 pure/clean)
    double honesty_index;            // Perceived honesty/truthfulness (0.0 deceptive, 1.0 honest/sincere)
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
            geom->symmetry = 1.0;         // Perfect symmetry for perceived honesty
            geom->jaw_scale = 0.8;        // Slender jaw for non-threatening silhouette
            geom->neck_thickness = 0.6;   // Thin neck
            geom->focal_length = 35.0;    // Exaggerated perspective
            geom->camera_elevation = 1.2; // High angle looking down
            geom->torso_ratio = 0.7;      // Squat, stable center of gravity
            geom->fur_roughness = 0.9;    // High roughness for soft, diffuse fur
            geom->feature_vertical_offset = -0.3; // Lower placement for baby-face warmth
            geom->behavioral_mismatch = 0.0;     // Fully aligned friendly profile
            geom->vocal_pitch = 250.0;           // High, warm pitch
            geom->expression_scale = 1.5;        // Amplified expression
            geom->leadership_profile = 0.0;      // Supportive profile
            geom->maturity_index = 0.0;          // Young profile
            geom->persuasion_strategy = 0.0;     // Sincerity-based persuasion
            geom->empathy_index = 1.0;           // High empathy/nurturing
            geom->protection_index = 1.0;        // Vulnerable, requires protection
            geom->size_index = 0.0;              // Perceived smaller/shorter stature
            geom->intellect_index = 0.0;         // Perceived lower capability
            geom->resilience_index = 0.0;        // Perceived lower physical resilience
            geom->agility_index = 0.0;           // Perceived lower physical agility
            geom->safety_index = 1.0;            // Perceived safe/harmless
            geom->purity_index = 1.0;            // Perceived pure/clean
            geom->honesty_index = 1.0;           // Perceived honest
            break;
        case PERSONALITY_AGGRESSIVE:
            geom->head_fwhr = 0.7;        // Narrow head shape is rated most aggressive
            geom->eye_eccentricity = 0.5; // Almond eyes are rated most aggressive (squinting)
            geom->eye_scale = 1.0;        // Medium eyes
            geom->stiffness = 0.9;        // High stiffness for rigid, sudden movements
            geom->damping = 0.2;          // Low damping
            geom->taper = 0.4;            // Pointed taper for sharp silhouettes
            geom->curvature = 0.2;        // Flattened, angular surfaces
            geom->symmetry = 0.8;         // Crooked asymmetry
            geom->jaw_scale = 1.4;        // Wide jaw denoting high physical strength
            geom->neck_thickness = 1.3;   // Thick, formidable neck connection
            geom->focal_length = 85.0;    // Flat perspective for dominance
            geom->camera_elevation = -0.8; // Low angle looking up
            geom->torso_ratio = 1.5;      // Broad shoulders, dominant upper torso
            geom->fur_roughness = 0.3;    // Glossy, slick fur
            geom->feature_vertical_offset = 0.2; // Higher feature placement
            geom->behavioral_mismatch = 0.8;     // High mismatch to indicate threat
            geom->vocal_pitch = 85.0;            // Deep, formidable pitch
            geom->expression_scale = 0.5;        // Deadpan expression
            geom->leadership_profile = 1.0;      // Dominant profile
            geom->maturity_index = 1.0;          // Mature profile
            geom->persuasion_strategy = 1.0;     // Expertise-based persuasion
            geom->empathy_index = 0.0;           // Low empathy
            geom->protection_index = 0.0;        // Independent
            geom->size_index = 1.0;              // Perceived larger/taller stature
            geom->intellect_index = 1.0;         // Perceived higher capability
            geom->resilience_index = 1.0;        // Perceived higher physical resilience
            geom->agility_index = 1.0;           // Perceived higher physical agility
            geom->safety_index = 0.0;            // Perceived dangerous
            geom->purity_index = 0.0;            // Perceived contaminated
            geom->honesty_index = 0.0;           // Perceived deceptive
            break;
        case PERSONALITY_EERIE:
            geom->head_fwhr = 0.7;        // Narrow head shape is rated most eerie
            geom->eye_eccentricity = 0.5; // Almond eyes are rated most eerie
            geom->eye_scale = 1.8;        // Large eyes are rated most eerie
            geom->stiffness = 0.8;        // Rigid mesh constraints
            geom->damping = 0.1;          // Erratic motion decay
            geom->taper = 0.3;            // Tapered joints
            geom->curvature = 0.3;        // Sharp boundary edges
            geom->symmetry = 0.8;         // Crooked asymmetry
            geom->jaw_scale = 1.3;        // Heavy jaw
            geom->neck_thickness = 1.2;   // Thick neck
            geom->focal_length = 85.0;    // Compressed perspective
            geom->camera_elevation = -0.8; // Low angle looking up
            geom->torso_ratio = 1.4;      // Formidable upper body proportions
            geom->fur_roughness = 0.3;    // Slick, high-specular sheen
            geom->feature_vertical_offset = 0.2; // Higher feature placement
            geom->behavioral_mismatch = 0.8;     // High mismatch
            geom->vocal_pitch = 85.0;            // Deep pitch
            geom->expression_scale = 0.5;        // Deadpan expression
            geom->leadership_profile = 1.0;      // Dominant profile
            geom->maturity_index = 1.0;          // Mature profile
            geom->persuasion_strategy = 1.0;     // Expertise-based persuasion
            geom->empathy_index = 0.0;           // Low empathy
            geom->protection_index = 0.0;        // Independent
            geom->size_index = 1.0;              // Perceived larger/taller stature
            geom->intellect_index = 1.0;         // Perceived higher capability
            geom->resilience_index = 1.0;        // Perceived higher physical resilience
            geom->agility_index = 1.0;           // Perceived higher physical agility
            geom->safety_index = 0.0;            // Perceived dangerous
            geom->purity_index = 0.0;            // Perceived contaminated
            geom->honesty_index = 0.0;           // Perceived deceptive
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
