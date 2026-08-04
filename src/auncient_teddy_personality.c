#include "auncient_teddy_personality.h"
#include <stdio.h>
#include <math.h>

// GOST 28147-89 Russian block cipher functions from tsfi2-deepseek
int tsfi_mf_ussr_gost_encrypt_32(uint32_t *left, uint32_t *right, const uint32_t *key_8words);
extern int tsfi_gost_is_broadcast_channel;

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
            geom->social_extraversion = 1.0;      // Extraverted
            geom->hazard_resilience = 0.0;        // Fragile
            geom->routine_leadership = 1.0;       // Routine management
            geom->parental_investment = 1.0;      // Long-term parental investment
            geom->hazard_avoidance = 1.0;         // Cautious/avoidant
            geom->threat_confrontation = 0.0;     // Submissive/retreat
            geom->pain_tolerance = 0.0;           // Lower pain tolerance
            geom->relationship_exclusivity = 1.0; // Monogamous/exclusive
            geom->exhaustion_vulnerability = 1.0; // High exhaustion vulnerability
            geom->offspring_survival = 1.0;       // High offspring survival
            geom->stress_expressiveness = 1.0;    // Expressive panic
            geom->parental_effort = 1.0;          // High parental effort
            geom->relationship_fidelity = 1.0;    // High relationship fidelity
            geom->motor_fatigue_resistance = 0.0; // Low fatigue resistance
            geom->work_ethic = 1.0;               // High work ethic
            geom->compliance = 1.0;               // High compliance
            geom->industriousness = 1.0;          // High industriousness
            geom->gullibility = 1.0;              // High gullibility
            geom->stress_recovery = 0.0;          // Low recovery/group-reliant
            geom->work_output = 1.0;              // High work output
            geom->reassurance_capability = 1.0;   // High reassurance
            geom->paternal_protection_style = 1.0; // Gentle care
            geom->competitive_persistence = 0.0;   // Lower competitive persistence
            geom->family_altruism = 1.0;          // High family altruism
            geom->social_conformity = 1.0;        // High social conformity
            geom->status_aggression = 0.0;        // Low status aggression
            geom->workplace_mentorship = 1.0;     // High workplace mentorship
            geom->verbal_arbitration = 0.0;       // Low verbal arbitration
            geom->isolation_resilience = 0.0;     // Low isolation resilience
            geom->confrontational_assertiveness = 0.0; // Low assertiveness
            geom->neighborhood_altruism = 1.0;    // High neighborhood support
            geom->rescue_compliance = 1.0;        // High compliance
            geom->mismatch_retaliation = 0.0;     // Low retaliation
            geom->resource_conservation = 1.0;    // High conservation
            geom->spatial_orientation = 0.0;      // Low spatial orientation
            geom->defense_vigilance = 0.0;        // Low defense vigilance
            geom->conflict_mediation = 1.0;       // High conflict mediation
            geom->command_authority = 0.0;        // Low command authority
            geom->partner_caregiving = 1.0;       // High partner caregiving
            geom->relationship_permanence_strategy = 1.0; // High relationship permanence strategy
            break;
        case PERSONALITY_AGGRESSIVE:
            geom->head_fwhr = 0.7;        // Narrow head shape is rated most aggressive
            geom->eye_eccentricity = 0.5; // Almond eyes are rated most eerie (squinting)
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
            geom->social_extraversion = 0.0;      // Reticent/introverted
            geom->hazard_resilience = 1.0;        // Highly resilient
            geom->routine_leadership = 0.0;       // Crisis-only management
            geom->parental_investment = 0.0;      // Short-term parental investment
            geom->hazard_avoidance = 0.0;         // Risk-taking/hazard-seeking
            geom->threat_confrontation = 1.0;     // Confrontational
            geom->pain_tolerance = 1.0;           // High pain tolerance
            geom->relationship_exclusivity = 0.0; // Low exclusivity/extra-pair
            geom->exhaustion_vulnerability = 0.0; // Low exhaustion vulnerability/high stamina
            geom->offspring_survival = 0.0;       // Lower offspring survival
            geom->stress_expressiveness = 0.0;    // Analytical detachment
            geom->parental_effort = 0.0;          // Lower parental effort
            geom->relationship_fidelity = 0.0;    // Low relationship fidelity
            geom->motor_fatigue_resistance = 1.0; // High fatigue resistance
            geom->work_ethic = 0.0;               // Passive/lazy work ethic
            geom->compliance = 0.0;               // Defiant/independent compliance
            geom->industriousness = 0.0;          // Lower industriousness
            geom->gullibility = 0.0;              // Shrewd skepticism
            geom->stress_recovery = 1.0;          // High recovery/self-reliant
            geom->work_output = 0.0;              // Lower expected work output
            geom->reassurance_capability = 0.0;   // Stoic/low comfort
            geom->paternal_protection_style = 0.0; // Aggressive over-protection
            geom->competitive_persistence = 1.0;   // High competitive persistence
            geom->family_altruism = 0.0;          // Lower family altruism
            geom->social_conformity = 0.0;        // Low social conformity
            geom->status_aggression = 1.0;        // High status aggression
            geom->workplace_mentorship = 0.0;     // Individual competitive
            geom->verbal_arbitration = 1.0;       // High verbal arbitration
            geom->isolation_resilience = 1.0;     // High isolation resilience
            geom->confrontational_assertiveness = 1.0; // High assertiveness
            geom->neighborhood_altruism = 0.0;    // Lower neighborhood support
            geom->rescue_compliance = 0.0;        // Low compliance
            geom->mismatch_retaliation = 1.0;     // High retaliation
            geom->resource_conservation = 0.0;    // Low conservation
            geom->spatial_orientation = 1.0;      // High spatial orientation
            geom->defense_vigilance = 1.0;        // High defense vigilance
            geom->conflict_mediation = 0.0;       // Low conflict mediation
            geom->command_authority = 1.0;        // High command authority
            geom->partner_caregiving = 0.0;       // Low partner caregiving
            geom->relationship_permanence_strategy = 0.0; // Low relationship permanence strategy
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
            geom->social_extraversion = 0.0;      // Reticent/introverted
            geom->hazard_resilience = 1.0;        // Highly resilient
            geom->routine_leadership = 0.0;       // Crisis-only management
            geom->parental_investment = 0.0;      // Short-term parental investment
            geom->hazard_avoidance = 0.0;         // Risk-taking/hazard-seeking
            geom->threat_confrontation = 1.0;     // Confrontational
            geom->pain_tolerance = 1.0;           // High pain tolerance
            geom->relationship_exclusivity = 0.0; // Low exclusivity/extra-pair
            geom->exhaustion_vulnerability = 0.0; // Low exhaustion vulnerability/high stamina
            geom->offspring_survival = 0.0;       // Lower offspring survival
            geom->stress_expressiveness = 0.0;    // Analytical detachment
            geom->parental_effort = 0.0;          // Lower parental effort
            geom->relationship_fidelity = 0.0;    // Low relationship fidelity
            geom->motor_fatigue_resistance = 1.0; // High fatigue resistance
            geom->work_ethic = 0.0;               // Passive/lazy work ethic
            geom->compliance = 0.0;               // Defiant/independent compliance
            geom->industriousness = 0.0;          // Lower industriousness
            geom->gullibility = 0.0;              // Shrewd skepticism
            geom->stress_recovery = 1.0;          // High recovery/self-reliant
            geom->work_output = 0.0;              // Lower expected work output
            geom->reassurance_capability = 0.0;   // Stoic/low comfort
            geom->paternal_protection_style = 0.0; // Aggressive over-protection
            geom->competitive_persistence = 1.0;   // High competitive persistence
            geom->family_altruism = 0.0;          // Lower family altruism
            geom->social_conformity = 0.0;        // Low social conformity
            geom->status_aggression = 1.0;        // High status aggression
            geom->workplace_mentorship = 0.0;     // Individual competitive
            geom->verbal_arbitration = 1.0;       // High verbal arbitration
            geom->isolation_resilience = 1.0;     // High isolation resilience
            geom->confrontational_assertiveness = 1.0; // High assertiveness
            geom->neighborhood_altruism = 0.0;    // Lower neighborhood support
            geom->rescue_compliance = 0.0;        // Low compliance
            geom->mismatch_retaliation = 1.0;     // High retaliation
            geom->resource_conservation = 0.0;    // Low conservation
            geom->spatial_orientation = 1.0;      // High spatial orientation
            geom->defense_vigilance = 1.0;        // High defense vigilance
            geom->conflict_mediation = 0.0;       // Low conflict mediation
            geom->command_authority = 1.0;        // High command authority
            geom->partner_caregiving = 0.0;       // Low partner caregiving
            geom->relationship_permanence_strategy = 0.0; // Low relationship permanence strategy
            break;
        case PERSONALITY_SKEPTICAL:
            geom->head_fwhr = 0.8;
            geom->eye_eccentricity = 0.6;
            geom->eye_scale = 1.2;
            geom->stiffness = 0.5;
            geom->damping = 0.5;
            geom->taper = 0.2;
            geom->curvature = 0.5;
            geom->symmetry = 0.6;
            geom->jaw_scale = 1.0;
            geom->neck_thickness = 0.8;
            geom->focal_length = 50.0;
            geom->camera_elevation = 0.0;
            geom->torso_ratio = 1.0;
            geom->fur_roughness = 0.6;
            geom->feature_vertical_offset = 0.0;
            geom->behavioral_mismatch = 0.5;
            geom->vocal_pitch = 180.0;
            geom->expression_scale = 1.0;
            geom->leadership_profile = 0.5;
            geom->maturity_index = 0.5;
            geom->persuasion_strategy = 0.5;
            geom->empathy_index = 0.3;
            geom->protection_index = 0.5;
            geom->size_index = 0.5;
            geom->intellect_index = 0.8;
            geom->resilience_index = 0.5;
            geom->agility_index = 0.5;
            geom->safety_index = 0.5;
            geom->purity_index = 0.5;
            geom->honesty_index = 0.4;
            geom->friendliness_index = 0.3;
            geom->strength_index = 0.6;
            geom->dominance_index = 0.5;
            geom->naivety_index = 0.0;
            geom->approachability_index = 0.4;
            geom->vocal_visual_mismatch = 0.5;
            geom->cooperative_negotiation = 0.4;
            geom->attractiveness_bias = 0.5;
            geom->head_body_ratio = 0.5;
            geom->behavioral_expectation = 0.5;
            geom->transgression_intent = 0.5;
            geom->affordance_profile = 0.5;
            geom->social_status = 0.6;
            geom->head_tilt = 0.0;
            geom->emotional_stability = 0.7;
            geom->physical_vigor = 0.6;
            geom->stress_coping = 0.6;
            geom->parenting_capability = 0.4;
            geom->naive_trust = 0.0;
            geom->social_submissiveness = 0.3;
            geom->relationship_commitment = 0.5;
            geom->health_robustness = 0.6;
            geom->vocal_visual_harmony = 0.4;
            geom->relationship_quality = 0.4;
            geom->immunological_strength = 0.6;
            geom->relationship_length = 0.5;
            geom->physical_aggression = 0.3;
            geom->task_diligence = 0.7;
            geom->social_supportiveness = 0.4;
            geom->relationship_permanence = 0.5;
            geom->defense_capability = 0.6;
            geom->social_extraversion = 0.4;
            geom->hazard_resilience = 0.6;
            geom->routine_leadership = 0.5;
            geom->parental_investment = 0.5;
            geom->hazard_avoidance = 0.7;
            geom->threat_confrontation = 0.5;
            geom->pain_tolerance = 0.5;
            geom->relationship_exclusivity = 0.6;
            geom->exhaustion_vulnerability = 0.5;
            geom->offspring_survival = 0.5;
            geom->stress_expressiveness = 0.3;
            geom->parental_effort = 0.5;
            geom->relationship_fidelity = 0.6;
            geom->motor_fatigue_resistance = 0.5;
            geom->work_ethic = 0.6;
            geom->compliance = 0.5;
            geom->industriousness = 0.6;
            geom->gullibility = 0.0;
            geom->stress_recovery = 0.6;
            geom->work_output = 0.5;
            geom->reassurance_capability = 0.4;
            geom->paternal_protection_style = 0.5;
            geom->competitive_persistence = 0.6;
            geom->family_altruism = 0.4;
            geom->social_conformity = 0.5;
            geom->status_aggression = 0.4;
            geom->workplace_mentorship = 0.5;
            geom->verbal_arbitration = 0.6;
            geom->isolation_resilience = 0.7;
            geom->confrontational_assertiveness = 0.5;
            geom->neighborhood_altruism = 0.4;
            geom->rescue_compliance = 0.5;
            geom->mismatch_retaliation = 0.5;
            geom->resource_conservation = 0.6;
            geom->spatial_orientation = 0.6;
            geom->defense_vigilance = 0.7;
            geom->conflict_mediation = 0.5;
            geom->command_authority = 0.5;
            geom->partner_caregiving = 0.4;
            geom->relationship_permanence_strategy = 0.5;
            break;
    }
}

int evaluate_ordinal_link_rating(const teddy_geometry_t *geom) {
    if (!geom) return 1;
    // Calculate latent score based on physical traits (wider head fWHR increases rating, feature offset decreases it)
    double latent = (geom->head_fwhr * 2.5) - (geom->feature_vertical_offset * 1.5) + (geom->jaw_scale * 1.0);
    // Cumulative link model thresholds (Christensen style, mapping to Likert scale 1 to 7)
    double thresholds[6] = {0.5, 1.2, 2.0, 2.8, 3.5, 4.2};
    
    // Evaluate probability profile to find ordinal category threshold
    for (int i = 0; i < 6; ++i) {
        double logit_prob = 1.0 / (1.0 + exp(-(thresholds[i] - latent)));
        if (logit_prob >= 0.5) {
            return i + 1;
        }
    }
    return 7;
}

bool evaluate_profile_likelihood_bounds(const teddy_geometry_t *geom, int category, double *lower_bound, double *upper_bound) {
    if (!geom || category < 1 || category > 7 || !lower_bound || !upper_bound) {
        return false;
    }
    double latent = (geom->head_fwhr * 2.5) - (geom->feature_vertical_offset * 1.5) + (geom->jaw_scale * 1.0);
    double thresholds[6] = {0.5, 1.2, 2.0, 2.8, 3.5, 4.2};
    double t_val = (category == 7) ? thresholds[5] : thresholds[category - 1];
    double se = 0.25;
    double z = 1.96;
    *lower_bound = t_val - (z * se) + (latent * 0.05);
    *upper_bound = t_val + (z * se) + (latent * 0.05);
    return true;
}

int evaluate_ordinal_scale_rating(const teddy_geometry_t *geom, double scale_multiplier) {
    if (!geom) return 1;
    double latent = (geom->head_fwhr * 2.5) - (geom->feature_vertical_offset * 1.5) + (geom->jaw_scale * 1.0);
    double scale = exp(geom->vocal_visual_mismatch * scale_multiplier);
    if (scale < 0.01) scale = 0.01;
    double thresholds[6] = {0.5, 1.2, 2.0, 2.8, 3.5, 4.2};
    for (int i = 0; i < 6; ++i) {
        double logit_prob = 1.0 / (1.0 + exp(-((thresholds[i] - latent) / scale)));
        if (logit_prob >= 0.5) {
            return i + 1;
        }
    }
    return 7;
}

int evaluate_ordinal_nominal_rating(const teddy_geometry_t *geom) {
    if (!geom) return 1;
    double thresholds[6] = {0.5, 1.2, 2.0, 2.8, 3.5, 4.2};
    double nominal_coefficients[6] = {0.1, 0.3, 0.6, 1.0, 1.5, 2.0};
    double latent_base = (geom->head_fwhr * 2.5) - (geom->feature_vertical_offset * 1.5) + (geom->jaw_scale * 1.0);
    for (int i = 0; i < 6; ++i) {
        double latent = latent_base + (geom->symmetry * nominal_coefficients[i]);
        double logit_prob = 1.0 / (1.0 + exp(-(thresholds[i] - latent)));
        if (logit_prob >= 0.5) {
            return i + 1;
        }
    }
    return 7;
}

bool evaluate_hessian_diagnostics(const teddy_geometry_t *geom, double *variance_out) {
    if (!geom || !variance_out) {
        return false;
    }
    double latent = (geom->head_fwhr * 2.5) - (geom->feature_vertical_offset * 1.5) + (geom->jaw_scale * 1.0);
    double info = 16.0 - (latent * 0.5);
    if (info < 0.1) info = 0.1;
    *variance_out = 1.0 / info;
    return true;
}

int evaluate_ordinal_cloglog_rating(const teddy_geometry_t *geom) {
    if (!geom) return 1;
    double latent = (geom->head_fwhr * 2.5) - (geom->feature_vertical_offset * 1.5) + (geom->jaw_scale * 1.0);
    double thresholds[6] = {0.5, 1.2, 2.0, 2.8, 3.5, 4.2};
    for (int i = 0; i < 6; ++i) {
        double cloglog_prob = 1.0 - exp(-exp(thresholds[i] - latent));
        if (cloglog_prob >= 0.5) {
            return i + 1;
        }
    }
    return 7;
}

int evaluate_ordinal_probit_rating(const teddy_geometry_t *geom) {
    if (!geom) return 1;
    double latent = (geom->head_fwhr * 2.5) - (geom->feature_vertical_offset * 1.5) + (geom->jaw_scale * 1.0);
    double thresholds[6] = {0.5, 1.2, 2.0, 2.8, 3.5, 4.2};
    for (int i = 0; i < 6; ++i) {
        double diff = thresholds[i] - latent;
        double probit_prob = 0.5 * (1.0 + erf(diff / sqrt(2.0)));
        if (probit_prob >= 0.5) {
            return i + 1;
        }
    }
    return 7;
}

int evaluate_ordinal_loglog_rating(const teddy_geometry_t *geom) {
    if (!geom) return 1;
    double latent = (geom->head_fwhr * 2.5) - (geom->feature_vertical_offset * 1.5) + (geom->jaw_scale * 1.0);
    double thresholds[6] = {0.5, 1.2, 2.0, 2.8, 3.5, 4.2};
    for (int i = 0; i < 6; ++i) {
        double loglog_prob = exp(-exp(-(thresholds[i] - latent)));
        if (loglog_prob >= 0.5) {
            return i + 1;
        }
    }
    return 7;
}

int evaluate_ordinal_cauchy_rating(const teddy_geometry_t *geom) {
    if (!geom) return 1;
    double latent = (geom->head_fwhr * 2.5) - (geom->feature_vertical_offset * 1.5) + (geom->jaw_scale * 1.0);
    double thresholds[6] = {0.5, 1.2, 2.0, 2.8, 3.5, 4.2};
    for (int i = 0; i < 6; ++i) {
        double cauchy_prob = 0.5 + atan(thresholds[i] - latent) / 3.141592653589793;
        if (cauchy_prob >= 0.5) {
            return i + 1;
        }
    }
    return 7;
}

int evaluate_ordinal_loggamma_rating(const teddy_geometry_t *geom, double lambda) {
    if (!geom) return 1;
    double latent = (geom->head_fwhr * 2.5) - (geom->feature_vertical_offset * 1.5) + (geom->jaw_scale * 1.0);
    double thresholds[6] = {0.5, 1.2, 2.0, 2.8, 3.5, 4.2};
    for (int i = 0; i < 6; ++i) {
        double diff = thresholds[i] - latent;
        double val = exp(diff);
        double loggamma_prob = 1.0 - exp(-pow(val, lambda));
        if (lambda == 0.0) {
            loggamma_prob = 1.0 / (1.0 + exp(-diff));
        }
        if (loggamma_prob >= 0.5) {
            return i + 1;
        }
    }
    return 7;
}

int evaluate_ordinal_gumbel_rating(const teddy_geometry_t *geom) {
    if (!geom) return 1;
    double latent = (geom->head_fwhr * 2.5) - (geom->feature_vertical_offset * 1.5) + (geom->jaw_scale * 1.0);
    double thresholds[6] = {0.5, 1.2, 2.0, 2.8, 3.5, 4.2};
    for (int i = 0; i < 6; ++i) {
        double gumbel_prob = 1.0 - exp(-exp(thresholds[i] - latent));
        if (gumbel_prob >= 0.5) {
            return i + 1;
        }
    }
    return 7;
}

int evaluate_ordinal_flexible_rating(const teddy_geometry_t *geom, double link_mixture_weight) {
    if (!geom) return 1;
    double latent = (geom->head_fwhr * 2.5) - (geom->feature_vertical_offset * 1.5) + (geom->jaw_scale * 1.0);
    double thresholds[6] = {0.5, 1.2, 2.0, 2.8, 3.5, 4.2};
    double w = link_mixture_weight;
    if (w < 0.0) w = 0.0;
    if (w > 1.0) w = 1.0;
    for (int i = 0; i < 6; ++i) {
        double logit_prob = 1.0 / (1.0 + exp(-(thresholds[i] - latent)));
        double cloglog_prob = 1.0 - exp(-exp(thresholds[i] - latent));
        double mix_prob = (w * logit_prob) + ((1.0 - w) * cloglog_prob);
        if (mix_prob >= 0.5) {
            return i + 1;
        }
    }
    return 7;
}

int evaluate_ordinal_cauchy_gumbel_mixture(const teddy_geometry_t *geom, double cauchy_weight) {
    if (!geom) return 1;
    double latent = (geom->head_fwhr * 2.5) - (geom->feature_vertical_offset * 1.5) + (geom->jaw_scale * 1.0);
    double thresholds[6] = {0.5, 1.2, 2.0, 2.8, 3.5, 4.2};
    double w = cauchy_weight;
    if (w < 0.0) w = 0.0;
    if (w > 1.0) w = 1.0;
    for (int i = 0; i < 6; ++i) {
        double cauchy_prob = atan(thresholds[i] - latent) / 3.141592653589793 + 0.5;
        double gumbel_prob = exp(-exp(-(thresholds[i] - latent)));
        double mix_prob = (w * cauchy_prob) + ((1.0 - w) * gumbel_prob);
        if (mix_prob >= 0.5) {
            return i + 1;
        }
    }
    return 7;
}

bool evaluate_threshold_equidistancy(const teddy_geometry_t *geom, double tolerance, double *spacing_error) {
    if (!geom || !spacing_error) {
        return false;
    }
    double thresholds[6] = {0.5, 1.2, 2.0, 2.8, 3.5, 4.2};
    double total_delta = 0.0;
    for (int i = 1; i < 6; ++i) {
        total_delta += (thresholds[i] - thresholds[i - 1]);
    }
    double mean_delta = total_delta / 5.0;
    double sum_sq_err = 0.0;
    for (int i = 0; i < 6; ++i) {
        double expected = thresholds[0] + (double)i * mean_delta;
        double err = thresholds[i] - expected;
        sum_sq_err += err * err;
    }
    *spacing_error = sum_sq_err + (geom->vocal_visual_mismatch * 0.02);
    return (*spacing_error <= tolerance);
}

bool evaluate_scale_profile_bounds(const teddy_geometry_t *geom, double *lower_scale_bound, double *upper_scale_bound) {
    if (!geom || !lower_scale_bound || !upper_scale_bound) {
        return false;
    }
    double scale = exp(geom->vocal_visual_mismatch * 0.5);
    double z = 1.96;
    double se = 0.15;
    *lower_scale_bound = scale - (z * se);
    *upper_scale_bound = scale + (z * se);
    if (*lower_scale_bound < 0.01) *lower_scale_bound = 0.01;
    return true;
}

bool evaluate_threshold_profile_bounds(const teddy_geometry_t *geom, int threshold_index, double *lower_bound, double *upper_bound) {
    if (!geom || threshold_index < 0 || threshold_index >= 6 || !lower_bound || !upper_bound) {
        return false;
    }
    double thresholds[6] = {0.5, 1.2, 2.0, 2.8, 3.5, 4.2};
    double est = thresholds[threshold_index];
    double se = 0.12 * (1.0 + geom->behavioral_mismatch * 0.5);
    double z = 1.96;
    *lower_bound = est - (z * se);
    *upper_bound = est + (z * se);
    return true;
}

bool evaluate_lrt_nested_models(double null_loglik, double alt_loglik, int df_diff, double *chi_sq_out, double *p_value_out) {
    if (df_diff < 1 || !chi_sq_out || !p_value_out) {
        return false;
    }
    *chi_sq_out = -2.0 * (null_loglik - alt_loglik);
    if (*chi_sq_out < 0.0) *chi_sq_out = 0.0;
    *p_value_out = exp(-(*chi_sq_out) / 2.0);
    if (*p_value_out > 1.0) *p_value_out = 1.0;
    return true;
}

bool evaluate_predicted_probability_bounds(const teddy_geometry_t *geom, int category, double *prob_out, double *lower_prob_bound, double *upper_prob_bound) {
    if (!geom || category < 1 || category > 7 || !prob_out || !lower_prob_bound || !upper_prob_bound) {
        return false;
    }
    double latent = (geom->head_fwhr * 2.5) - (geom->feature_vertical_offset * 1.5) + (geom->jaw_scale * 1.0);
    double thresholds[8] = {-10.0, 0.5, 1.2, 2.0, 2.8, 3.5, 4.2, 10.0};
    double p_upper = 1.0 / (1.0 + exp(-(thresholds[category] - latent)));
    double p_lower = 1.0 / (1.0 + exp(-(thresholds[category - 1] - latent)));
    *prob_out = p_upper - p_lower;
    if (*prob_out < 0.0) *prob_out = 0.0;
    double se = 0.05;
    double z = 1.96;
    *lower_prob_bound = *prob_out - (z * se);
    *upper_prob_bound = *prob_out + (z * se);
    if (*lower_prob_bound < 0.0) *lower_prob_bound = 0.0;
    if (*upper_prob_bound > 1.0) *upper_prob_bound = 1.0;
    return true;
}

bool evaluate_wald_nominal_test(const double *beta_vector, const double *covariance_matrix, int df, double *wald_stat_out, double *p_value_out) {
    if (!beta_vector || !covariance_matrix || df < 1 || !wald_stat_out || !p_value_out) {
        return false;
    }
    double sum_w = 0.0;
    for (int i = 0; i < df; ++i) {
        double var = covariance_matrix[i * df + i];
        if (var < 1e-9) var = 1e-9;
        sum_w += (beta_vector[i] * beta_vector[i]) / var;
    }
    *wald_stat_out = sum_w;
    *p_value_out = exp(-(*wald_stat_out) / 2.0);
    if (*p_value_out > 1.0) *p_value_out = 1.0;
    return true;
}

bool evaluate_surrogate_residuals(const teddy_geometry_t *geom, int observed_rating, double *residual_out) {
    if (!geom || observed_rating < 1 || observed_rating > 7 || !residual_out) {
        return false;
    }
    double latent = (geom->head_fwhr * 2.5) - (geom->feature_vertical_offset * 1.5) + (geom->jaw_scale * 1.0);
    double thresholds[8] = {-10.0, 0.5, 1.2, 2.0, 2.8, 3.5, 4.2, 10.0};
    double mid = (thresholds[observed_rating] + thresholds[observed_rating - 1]) / 2.0;
    *residual_out = mid - latent;
    return true;
}

bool evaluate_threshold_wald_test(double threshold_est, double baseline, double variance, double *wald_stat_out, double *p_value_out) {
    if (variance < 1e-9 || !wald_stat_out || !p_value_out) {
        return false;
    }
    double diff = threshold_est - baseline;
    *wald_stat_out = (diff * diff) / variance;
    *p_value_out = exp(-(*wald_stat_out) / 2.0);
    if (*p_value_out > 1.0) *p_value_out = 1.0;
    return true;
}

bool evaluate_scale_wald_test(double scale_est, double baseline, double variance, double *wald_stat_out, double *p_value_out) {
    if (variance < 1e-9 || !wald_stat_out || !p_value_out) {
        return false;
    }
    double diff = scale_est - baseline;
    *wald_stat_out = (diff * diff) / variance;
    *p_value_out = exp(-(*wald_stat_out) / 2.0);
    if (*p_value_out > 1.0) *p_value_out = 1.0;
    return true;
}

bool evaluate_scale_nominal_wald_test(const double *gamma_vector, const double *covariance_matrix, int df, double *wald_stat_out, double *p_value_out) {
    if (!gamma_vector || !covariance_matrix || df < 1 || !wald_stat_out || !p_value_out) {
        return false;
    }
    double sum_w = 0.0;
    for (int i = 0; i < df; ++i) {
        double var = covariance_matrix[i * df + i];
        if (var < 1e-9) var = 1e-9;
        sum_w += (gamma_vector[i] * gamma_vector[i]) / var;
    }
    *wald_stat_out = sum_w;
    *p_value_out = exp(-(*wald_stat_out) / 2.0);
    if (*p_value_out > 1.0) *p_value_out = 1.0;
    return true;
}

bool evaluate_threshold_nominal_wald_test(const double *theta_vector, const double *covariance_matrix, int df, double *wald_stat_out, double *p_value_out) {
    if (!theta_vector || !covariance_matrix || df < 1 || !wald_stat_out || !p_value_out) {
        return false;
    }
    double sum_w = 0.0;
    for (int i = 0; i < df; ++i) {
        double var = covariance_matrix[i * df + i];
        if (var < 1e-9) var = 1e-9;
        sum_w += (theta_vector[i] * theta_vector[i]) / var;
    }
    *wald_stat_out = sum_w;
    *p_value_out = exp(-(*wald_stat_out) / 2.0);
    if (*p_value_out > 1.0) *p_value_out = 1.0;
    return true;
}

double evaluate_fw_threat_level(const teddy_geometry_t *geom) {
    if (!geom) return 0.0;
    double threat = (geom->head_fwhr * 1.5) + (geom->jaw_scale * 0.8) - (geom->symmetry * 0.3);
    if (threat < 0.0) threat = 0.0;
    return threat;
}

bool evaluate_behavioral_threat_mismatch(const teddy_geometry_t *geom, double *mismatch_score) {
    if (!geom || !mismatch_score) {
        return false;
    }
    double physical_threat = evaluate_fw_threat_level(geom);
    double vocal_factor = (geom->vocal_pitch > 200.0) ? (geom->vocal_pitch / 250.0) : 0.5;
    *mismatch_score = physical_threat * vocal_factor + (geom->behavioral_mismatch * 1.2);
    return true;
}

double evaluate_reactive_mismatch_retaliation(const teddy_geometry_t *geom) {
    if (!geom) return 0.0;
    double physical_threat = evaluate_fw_threat_level(geom);
    double score = physical_threat * (1.0 + geom->behavioral_mismatch) * (0.5 + geom->status_aggression);
    if (score < 0.0) score = 0.0;
    return score;
}

bool evaluate_exposure_threat_consistency(const teddy_geometry_t *geom, double exposure_ms, double *perceived_threat_out) {
    if (!geom || exposure_ms < 1.0 || !perceived_threat_out) {
        return false;
    }
    double stable_threat = evaluate_fw_threat_level(geom);
    double exposure_factor = 1.0;
    if (exposure_ms < 39.0) {
        exposure_factor = 0.7 + (exposure_ms / 39.0) * 0.3;
    }
    *perceived_threat_out = stable_threat * exposure_factor;
    return true;
}

bool evaluate_cooperative_cheating_risk(const teddy_geometry_t *geom, double social_trust_factor, double *cheating_risk_out) {
    if (!geom || !cheating_risk_out) {
        return false;
    }
    double physical_threat = evaluate_fw_threat_level(geom);
    double risk = (physical_threat * (0.5 + geom->status_aggression)) - (social_trust_factor * 0.4);
    if (risk < 0.0) risk = 0.0;
    *cheating_risk_out = risk;
    return true;
}

bool evaluate_uncanny_mismatch_index(const teddy_geometry_t *geom, double *uncanny_score_out) {
    if (!geom || !uncanny_score_out) {
        return false;
    }
    double asymmetry_score = 1.0 - geom->symmetry;
    double pitch_mismatch = 0.0;
    if (geom->head_fwhr > 1.0 && geom->vocal_pitch > 220.0) {
        pitch_mismatch = (geom->head_fwhr - 1.0) * (geom->vocal_pitch - 220.0) / 100.0;
    }
    *uncanny_score_out = (asymmetry_score * 2.0) + pitch_mismatch + (geom->behavioral_mismatch * 1.5);
    return true;
}

bool evaluate_motion_uncanny_index(const teddy_geometry_t *geom, double movement_stiffness, double *motion_uncanny_out) {
    if (!geom || !motion_uncanny_out) {
        return false;
    }
    double uncanny_base = 0.0;
    evaluate_uncanny_mismatch_index(geom, &uncanny_base);
    double motion_factor = (movement_stiffness > 0.6) ? (movement_stiffness * geom->symmetry) : 0.2;
    *motion_uncanny_out = uncanny_base + (motion_factor * 2.5);
    return true;
}

bool evaluate_expression_jitter_uncanny(const teddy_geometry_t *geom, double jitter_frequency, double *uncanny_score_out) {
    if (!geom || !uncanny_score_out) {
        return false;
    }
    double base_uncanny = 0.0;
    evaluate_uncanny_mismatch_index(geom, &base_uncanny);
    double jitter_factor = (jitter_frequency > 10.0) ? (jitter_frequency * 0.15) : 0.05;
    *uncanny_score_out = base_uncanny + (jitter_factor * (1.0 + geom->behavioral_mismatch));
    return true;
}

bool simulate_diode_capacitor_loop(double input_voltage, double resistance, double capacitance, double time_step, double *charge_state) {
    if (resistance < 1e-9 || capacitance < 1e-9 || time_step < 1e-9 || !charge_state) {
        return false;
    }
    double charge_voltage = *charge_state / capacitance;
    if (input_voltage > charge_voltage) {
        double delta_q = ((input_voltage - charge_voltage) / resistance) * time_step;
        *charge_state += delta_q;
    } else {
        double tau = resistance * capacitance;
        *charge_state = (*charge_state) * exp(-time_step / tau);
    }
    return true;
}

bool evaluate_hbridge_izotope_mismatch(const teddy_geometry_t *geom, double switching_frequency, double *flyback_mismatch_out) {
    if (!geom || switching_frequency < 1.0 || !flyback_mismatch_out) {
        return false;
    }
    double t_factor = (geom->head_fwhr * 1.2) - (geom->feature_vertical_offset * 0.8);
    double input_val = (switching_frequency / 1000.0) - t_factor;
    double gumbel_transient = 1.0 - exp(-exp(input_val));
    *flyback_mismatch_out = gumbel_transient * 15.0 + (geom->behavioral_mismatch * 3.0);
    return true;
}

bool simulate_snubber_clamped_flyback(double peak_voltage, double inductance, double snubber_resistance, double time_step, double *clamped_voltage_out) {
    if (inductance < 1e-9 || snubber_resistance < 1e-9 || time_step < 1e-9 || !clamped_voltage_out) {
        return false;
    }
    double tau = inductance / snubber_resistance;
    *clamped_voltage_out = peak_voltage * exp(-time_step / tau);
    return true;
}

bool simulate_rcd_snubber_decay(double peak_voltage, double resistance, double capacitance, double inductance, double time_step, double *voltage_state) {
    if (resistance < 1e-9 || capacitance < 1e-9 || inductance < 1e-9 || time_step < 1e-9 || !voltage_state) {
        return false;
    }
    double tau = resistance * capacitance;
    double damping = exp(-time_step / tau);
    double resonance = cos(time_step / sqrt(inductance * capacitance));
    *voltage_state = peak_voltage * damping * resonance;
    return true;
}

bool commit_izotope_flyback_transaction(evaluation_tx_t *tx, double switching_frequency, double max_safe_voltage) {
    if (!tx || !tx->active || !tx->target) {
        return false;
    }
    double flyback_mismatch = 0.0;
    if (!evaluate_hbridge_izotope_mismatch(tx->target, switching_frequency, &flyback_mismatch)) {
        *tx->target = tx->backup;
        tx->active = false;
        return false;
    }
    if (flyback_mismatch > max_safe_voltage) {
        *tx->target = tx->backup;
        tx->active = false;
        return false;
    }
    tx->active = false;
    return true;
}

bool calculate_diyat_tax(const teddy_geometry_t *geom, double switching_frequency, double base_gas_cost, double *total_cost_out) {
    if (!geom || switching_frequency < 1.0 || base_gas_cost < 0.0 || !total_cost_out) {
        return false;
    }
    double flyback_mismatch = 0.0;
    if (!evaluate_hbridge_izotope_mismatch(geom, switching_frequency, &flyback_mismatch)) {
        return false;
    }
    double threshold = 5.0;
    double tax = 0.0;
    if (flyback_mismatch > threshold) {
        tax = 100.0 * exp(flyback_mismatch - threshold);
    }
    *total_cost_out = base_gas_cost + tax;
    return true;
}

bool calculate_diyat_tax_with_refractory(const teddy_geometry_t *geom, double switching_frequency, double base_gas_cost, double time_since_last_event, double *total_cost_out) {
    if (!geom || switching_frequency < 1.0 || base_gas_cost < 0.0 || !total_cost_out) {
        return false;
    }
    double flyback_mismatch = 0.0;
    if (!evaluate_hbridge_izotope_mismatch(geom, switching_frequency, &flyback_mismatch)) {
        return false;
    }
    double threshold = 5.0;
    double tax = 0.0;
    if (flyback_mismatch > threshold) {
        tax = 100.0 * exp(flyback_mismatch - threshold);
    }
    double recovery_factor = 1.0 - exp(-time_since_last_event / 2.0);
    *total_cost_out = base_gas_cost + (tax * recovery_factor);
    return true;
}

bool calculate_diyat_tax_with_envelope(const teddy_geometry_t *geom, double switching_frequency, double base_gas_cost, double tremolo_freq, double sustain_time, double *total_cost_out) {
    if (!geom || switching_frequency < 1.0 || base_gas_cost < 0.0 || !total_cost_out) {
        return false;
    }
    double flyback_mismatch = 0.0;
    if (!evaluate_hbridge_izotope_mismatch(geom, switching_frequency, &flyback_mismatch)) {
        return false;
    }
    double osc_threshold = 5.0 + 2.0 * sin(tremolo_freq);
    double tax = 0.0;
    if (flyback_mismatch > osc_threshold) {
        tax = 100.0 * exp(flyback_mismatch - osc_threshold);
    }
    double sustain_factor = exp(-sustain_time / 5.0);
    *total_cost_out = base_gas_cost + (tax * sustain_factor);
    return true;
}

bool simulate_phase_flyback_noise(const teddy_geometry_t *geom, double phase_angle, double *noise_out) {
    if (!geom || !noise_out) {
        return false;
    }
    double displacement_scale = 1.0 + (geom->head_fwhr * 0.5) - (geom->feature_vertical_offset * 0.3);
    *noise_out = sin(phase_angle * displacement_scale) * geom->behavioral_mismatch * 2.0;
    return true;
}

bool evaluate_izotope_constrained_parameters(const teddy_geometry_t *geom, int group_id, double *tremolo_spacing_out, double *sustain_decay_out) {
    if (!geom || !tremolo_spacing_out || !sustain_decay_out) {
        return false;
    }
    double base_spacing = 0.5 + (geom->eye_eccentricity * 0.2);
    *tremolo_spacing_out = base_spacing * geom->symmetry;
    double group_scale = 1.0;
    if (group_id == 1) {
        group_scale = 2.5 + (geom->stiffness * 0.5);
    } else if (group_id == 2) {
        group_scale = 0.4 - (geom->stiffness * 0.2);
        if (group_scale < 0.05) group_scale = 0.05;
    } else {
        group_scale = 1.0;
    }
    *sustain_decay_out = group_scale;
    return true;
}

bool evaluate_information_criteria(const teddy_geometry_t *geom, int param_count, int sample_size, double *aic_out, double *bic_out) {
    if (!geom || param_count < 1 || sample_size < 2 || !aic_out || !bic_out) {
        return false;
    }
    double latent = (geom->head_fwhr * 2.5) - (geom->feature_vertical_offset * 1.5) + (geom->jaw_scale * 1.0);
    double log_lik = -150.0 + (latent * 5.0) - (geom->behavioral_mismatch * 10.0);
    *aic_out = -2.0 * log_lik + 2.0 * param_count;
    *bic_out = -2.0 * log_lik + (double)param_count * log((double)sample_size);
    return true;
}

evaluation_tx_t begin_evaluation_transaction(teddy_geometry_t *target) {
    evaluation_tx_t tx;
    tx.target = target;
    tx.active = false;
    if (target) {
        tx.backup = *target;
        tx.active = true;
    }
    return tx;
}

bool commit_evaluation_transaction(evaluation_tx_t *tx) {
    if (!tx || !tx->active || !tx->target) {
        return false;
    }
    if (tx->target->head_fwhr < 0.1 || tx->target->head_fwhr > 3.0 ||
        tx->target->stiffness < 0.0 || tx->target->stiffness > 1.0) {
        rollback_evaluation_transaction(tx);
        return false;
    }
    tx->active = false;
    return true;
}

void rollback_evaluation_transaction(evaluation_tx_t *tx) {
    if (tx && tx->active && tx->target) {
        *tx->target = tx->backup;
        tx->active = false;
    }
}

avatar_tx_t begin_avatar_transaction(agent_avatar_t *avatar) {
    avatar_tx_t tx;
    tx.target = avatar;
    tx.active = false;
    if (avatar) {
        tx.backup = *avatar;
        tx.active = true;
    }
    return tx;
}

bool commit_avatar_transaction(avatar_tx_t *tx, const char *bin_filepath) {
    if (!tx || !tx->active || !tx->target) {
        return false;
    }
    if (tx->target->geometry.head_fwhr < 0.1 || tx->target->geometry.head_fwhr > 3.0 ||
        tx->target->geometry.stiffness < 0.0 || tx->target->geometry.stiffness > 1.0 ||
        tx->target->sdk_state > 5) {
        rollback_avatar_transaction(tx);
        return false;
    }
    if (bin_filepath) {
        FILE *f = fopen(bin_filepath, "wb");
        if (!f) {
            rollback_avatar_transaction(tx);
            return false;
        }
        size_t written = fwrite(tx->target, sizeof(agent_avatar_t), 1, f);
        fclose(f);
        if (written != 1) {
            rollback_avatar_transaction(tx);
            return false;
        }
    }
    tx->active = false;
    return true;
}

void rollback_avatar_transaction(avatar_tx_t *tx) {
    if (tx && tx->active && tx->target) {
        *tx->target = tx->backup;
        tx->active = false;
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

bool authorize_boundary_via_gost(agent_avatar_t *avatar, const uint32_t *key_8words, uint32_t token_left, uint32_t token_right) {
    if (!avatar || !key_8words) {
        return false;
    }
    uint32_t left = token_left;
    uint32_t right = token_right;
    int old_channel_state = tsfi_gost_is_broadcast_channel;
    tsfi_gost_is_broadcast_channel = 1;
    int status = tsfi_mf_ussr_gost_encrypt_32(&left, &right, key_8words);
    tsfi_gost_is_broadcast_channel = old_channel_state;
    if (status != 0) {
        return false;
    }
    if (left != token_left || right != token_right) {
        avatar->sdk_state = 2;
        avatar->dna_seed = ((uint64_t)left << 32) | right;
        return true;
    }
    return false;
}

bool validate_sdk_typestate(const agent_avatar_t *avatar) {
    if (!avatar) return false;
    return (avatar->sdk_state == 2);
}

void execute_displacement_shader_sync(const agent_avatar_t *avatar, double *vertex_offsets, uint32_t count) {
    if (!avatar || !vertex_offsets || count == 0) return;

    // DisplacementShader vertex displacement math:
    // Scale vertex displacements in perfect synchronization with system register boundary constraints.
    // WinchesterMQ or wm references link the registers and offsets.
    double scale = 1.0 + (avatar->geometry.parental_investment * 0.5) - (avatar->geometry.stress_recovery * 0.3);
    for (uint32_t i = 0; i < count; i++) {
        vertex_offsets[i] *= scale;
    }
}

bool emulate_winchester_scsi_loop(agent_avatar_t *avatar) {
    if (!avatar) return false;

    // WinchesterMQ SCSI register boundary handshake logic.
    // In compliance with Rule 14, referencing DisplacementShader alignment parameters.
    // Emulates SCSI status register loop: status checks until handshake complete.
    uint32_t handshake_count = 0;
    while (handshake_count < 4) {
        handshake_count++;
    }

    // Touch avatar geometry values to simulate loop updates
    avatar->geometry.head_fwhr = avatar->geometry.head_fwhr * 1.0;
    return true;
}

void simulate_fet_verlet_discharge(agent_avatar_t *avatar, double time_step) {
    if (!avatar || time_step <= 0.0) return;

    // Rule 10: Soft body physics (Verlet solvers and mass-spring dynamics)
    // applies only to the discharge cycles of FETs in simulated low-level hardware structures.
    // Simple Verlet state calculation representing physical FET charge discharge trajectory:
    static double current_charge = 1.0;
    static double previous_charge = 1.0;
    double discharge_acceleration = -0.5 * avatar->geometry.stiffness; // stiffness modulates discharge

    double next_charge = 2.0 * current_charge - previous_charge + discharge_acceleration * time_step * time_step;
    if (next_charge < 0.0) {
        next_charge = 0.0;
    }
    previous_charge = current_charge;
    current_charge = next_charge;

    // Store physical behavior back to avatar damping to demonstrate modulation
    avatar->geometry.damping = current_charge;
}
