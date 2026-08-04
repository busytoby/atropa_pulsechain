#include "auncient_teddy_personality.h"
#include <stdio.h>

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
            geom->friendliness_index = 1.0;      // Perceived friendly
            geom->strength_index = 0.0;          // Perceived weaker
            geom->dominance_index = 0.0;         // Perceived submissive
            geom->naivety_index = 1.0;           // Perceived naive
            geom->approachability_index = 1.0;    // Perceived approachable
            geom->vocal_visual_mismatch = 0.0;   // Congruent
            geom->cooperative_negotiation = 1.0; // Cooperative
            geom->attractiveness_bias = 0.0;     // Babyface warmth
            geom->head_body_ratio = 1.0;         // Babyface silhouette
            geom->behavioral_expectation = 1.0;  // Child-like playful
            geom->transgression_intent = 0.0;    // Accidental
            geom->affordance_profile = 0.0;      // Nurturing/protective affordance
            geom->social_status = 0.0;           // Lower perceived status
            geom->head_tilt = 1.0;               // Upward tilt
            geom->emotional_stability = 1.0;     // Stable
            geom->physical_vigor = 0.0;          // Fragile
            geom->stress_coping = 0.0;           // Dependency profile
            geom->parenting_capability = 1.0;    // High parenting warmth
            geom->naive_trust = 1.0;             // Naive trust
            geom->social_submissiveness = 1.0;   // Submissive
            geom->relationship_commitment = 1.0; // Committed
            geom->health_robustness = 0.0;        // Fragile health
            geom->vocal_visual_harmony = 1.0;     // High friendliness/harmony
            geom->relationship_quality = 1.0;     // High relationship quality
            geom->immunological_strength = 0.0;   // Lower resistance
            geom->relationship_length = 1.0;      // Long-term preference
            geom->physical_aggression = 0.0;      // Cooperative non-violent
            geom->task_diligence = 0.0;           // Lower/emotional profile
            geom->social_supportiveness = 1.0;    // High support/altruism
            geom->relationship_permanence = 1.0;  // High stability/permanence
            geom->defense_capability = 0.0;       // Lower defense capability
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
            geom->friendliness_index = 0.0;      // Perceived hostile
            geom->strength_index = 1.0;          // Perceived stronger
            geom->dominance_index = 1.0;         // Perceived dominant
            geom->naivety_index = 0.0;           // Perceived shrewd
            geom->approachability_index = 0.0;    // Perceived unapproachable
            geom->vocal_visual_mismatch = 0.5;   // Moderate mismatch
            geom->cooperative_negotiation = 0.0; // Competitive
            geom->attractiveness_bias = 1.0;     // Mature competence
            geom->head_body_ratio = 0.0;         // Mature silhouette
            geom->behavioral_expectation = 0.0;  // Adult-like analytical
            geom->transgression_intent = 1.0;    // Intentional
            geom->affordance_profile = 1.0;      // Threat avoidance affordance
            geom->social_status = 1.0;           // Higher perceived status
            geom->head_tilt = -1.0;              // Downward tilt
            geom->emotional_stability = 0.0;     // Unstable
            geom->physical_vigor = 1.0;          // Robust vigor
            geom->stress_coping = 1.0;           // Self-reliance
            geom->parenting_capability = 0.0;    // Low parenting warmth
            geom->naive_trust = 0.0;             // Suspicious caution
            geom->social_submissiveness = 0.0;   // Assertive/dominant
            geom->relationship_commitment = 0.0; // Uncommitted
            geom->health_robustness = 1.0;        // Robust health
            geom->vocal_visual_harmony = 0.0;     // Low friendliness bias
            geom->relationship_quality = 0.0;     // Low relationship quality
            geom->immunological_strength = 1.0;   // Robust resistance
            geom->relationship_length = 0.0;      // Short-term preference
            geom->physical_aggression = 1.0;      // Aggressive
            geom->task_diligence = 1.0;           // Diligent/systematic
            geom->social_supportiveness = 0.0;    // Low support/altruism
            geom->relationship_permanence = 0.0;  // Opportunistic dissolution
            geom->defense_capability = 1.0;       // High defense capability
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
            geom->friendliness_index = 0.0;      // Perceived hostile
            geom->strength_index = 1.0;          // Perceived stronger
            geom->dominance_index = 1.0;         // Perceived dominant
            geom->naivety_index = 0.0;           // Perceived shrewd
            geom->approachability_index = 0.0;    // Perceived unapproachable
            geom->vocal_visual_mismatch = 1.0;   // Highly incongruent (eerie)
            geom->cooperative_negotiation = 0.0; // Competitive
            geom->attractiveness_bias = 1.0;     // Mature competence
            geom->head_body_ratio = 0.0;         // Mature silhouette
            geom->behavioral_expectation = 0.0;  // Adult-like analytical
            geom->transgression_intent = 1.0;    // Intentional
            geom->affordance_profile = 1.0;      // Threat avoidance affordance
            geom->social_status = 1.0;           // Higher perceived status
            geom->head_tilt = -1.0;              // Downward tilt
            geom->emotional_stability = 0.0;     // Unstable
            geom->physical_vigor = 1.0;          // Robust vigor
            geom->stress_coping = 1.0;           // Self-reliance
            geom->parenting_capability = 0.0;    // Low parenting warmth
            geom->naive_trust = 0.0;             // Suspicious caution
            geom->social_submissiveness = 0.0;   // Assertive/dominant
            geom->relationship_commitment = 0.0; // Uncommitted
            geom->health_robustness = 1.0;        // Robust health
            geom->vocal_visual_harmony = 0.0;     // Low friendliness bias
            geom->relationship_quality = 0.0;     // Low relationship quality
            geom->immunological_strength = 1.0;   // Robust resistance
            geom->relationship_length = 0.0;      // Short-term preference
            geom->physical_aggression = 1.0;      // Aggressive
            geom->task_diligence = 1.0;           // Diligent/systematic
            geom->social_supportiveness = 0.0;    // Low support/altruism
            geom->relationship_permanence = 0.0;  // Opportunistic dissolution
            geom->defense_capability = 1.0;       // High defense capability
            break;
    }
}

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
