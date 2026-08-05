#include "auncient_teddy_personality.h"
#include <stdio.h>
#include <math.h>

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

bool evaluate_rapid_threat_limit(const teddy_geometry_t *geom, double exposure_ms, double *detected_threat_out) {
    if (!geom || exposure_ms < 0.0 || !detected_threat_out) {
        return false;
    }
    double base_threat = evaluate_fw_threat_level(geom);
    double scale = 1.0;
    if (exposure_ms < 100.0) {
        scale = exposure_ms / 100.0;
    }
    *detected_threat_out = base_threat * scale;
    return true;
}

bool evaluate_reactive_retaliation_profile(const teddy_geometry_t *geom, double provocation_intensity, double *retaliation_out) {
    if (!geom || provocation_intensity < 0.0 || !retaliation_out) {
        return false;
    }
    double formidability = geom->jaw_scale * 1.5;
    *retaliation_out = provocation_intensity * formidability * (1.0 + geom->status_aggression);
    return true;
}

bool evaluate_provocation_aggression_threshold(const teddy_geometry_t *geom, double base_provocation, double *aggression_threshold_out) {
    if (!geom || base_provocation < 0.0 || !aggression_threshold_out) {
        return false;
    }
    double sensitivity = (geom->head_fwhr * 1.5) + (geom->status_aggression * 2.0);
    double threshold = base_provocation / (1.0 + sensitivity);
    if (threshold < 0.0) threshold = 0.0;
    *aggression_threshold_out = threshold;
    return true;
}

bool evaluate_exposure_adjusted_threat_threshold(const teddy_geometry_t *geom, double exposure_ms, double *adjusted_threat_threshold_out) {
    if (!geom || exposure_ms < 0.0 || !adjusted_threat_threshold_out) {
        return false;
    }
    double base_threshold = evaluate_fw_threat_level(geom);
    double duration_scale = (exposure_ms > 300.0) ? 1.0 : (exposure_ms / 300.0);
    *adjusted_threat_threshold_out = base_threshold * duration_scale;
    return true;
}

bool evaluate_status_dominance_provocation(const teddy_geometry_t *geom, double provocation_scale, double *dominance_threat_out) {
    if (!geom || provocation_scale < 0.0 || !dominance_threat_out) {
        return false;
    }
    double formidability = evaluate_fw_threat_level(geom);
    *dominance_threat_out = formidability * provocation_scale * (1.0 + geom->status_aggression);
    return true;
}

bool evaluate_threat_replication_consistency(const teddy_geometry_t *geom, const double *threat_observations, int count, double *consistency_out) {
    if (!geom || !threat_observations || count < 1 || !consistency_out) {
        return false;
    }
    double expected_threat = evaluate_fw_threat_level(geom);
    double error_sum = 0.0;
    for (int i = 0; i < count; ++i) {
        double diff = threat_observations[i] - expected_threat;
        error_sum += diff * diff;
    }
    double variance = error_sum / count;
    *consistency_out = 1.0 / (1.0 + variance);
    return true;
}

bool evaluate_reactive_retaliation_boundary(const teddy_geometry_t *geom, double provocation_scale, double *retaliation_boundary_out) {
    if (!geom || provocation_scale < 0.0 || !retaliation_boundary_out) {
        return false;
    }
    double base_retaliation = 0.0;
    evaluate_reactive_retaliation_profile(geom, provocation_scale, &base_retaliation);
    *retaliation_boundary_out = base_retaliation * (1.0 + (geom->head_fwhr * 0.5));
    return true;
}

bool evaluate_retaliation_aggression_scaling(const teddy_geometry_t *geom, double baseline_aggression, double *scaled_aggression_out) {
    if (!geom || baseline_aggression < 0.0 || !scaled_aggression_out) {
        return false;
    }
    double threat_level = evaluate_fw_threat_level(geom);
    *scaled_aggression_out = baseline_aggression * (1.0 + (threat_level * 0.4));
    return true;
}

bool evaluate_retaliation_threshold_decay(const teddy_geometry_t *geom, double exposure_ms, double *decayed_threshold_out) {
    if (!geom || exposure_ms < 0.0 || !decayed_threshold_out) {
        return false;
    }
    double base_threshold = 2.0 / (1.0 + (geom->head_fwhr * 0.8));
    double decay_factor = exp(-exposure_ms * 0.005);
    *decayed_threshold_out = base_threshold * decay_factor;
    return true;
}

bool evaluate_provocation_exposure_decay(const teddy_geometry_t *geom, double provocation_scale, double exposure_ms, double *decayed_threat_out) {
    if (!geom || provocation_scale < 0.0 || exposure_ms < 0.0 || !decayed_threat_out) {
        return false;
    }
    double base_threat = 0.0;
    evaluate_status_dominance_provocation(geom, provocation_scale, &base_threat);
    double decay_factor = exp(-exposure_ms * 0.003);
    *decayed_threat_out = base_threat * decay_factor;
    return true;
}

bool evaluate_retaliation_aggression_ceiling(const teddy_geometry_t *geom, double provocation_scale, double *aggression_ceiling_out) {
    if (!geom || provocation_scale < 0.0 || !aggression_ceiling_out) {
        return false;
    }
    double threat_level = evaluate_fw_threat_level(geom);
    *aggression_ceiling_out = (threat_level * 2.0) + (provocation_scale * 1.5 * geom->status_aggression);
    return true;
}

bool evaluate_status_dominance_multiplier(const teddy_geometry_t *geom, double provocation_scale, double *multiplier_out) {
    if (!geom || provocation_scale < 0.0 || !multiplier_out) {
        return false;
    }
    double base_multiplier = 1.0 + (geom->status_aggression * 0.5);
    *multiplier_out = base_multiplier * (1.0 + (provocation_scale * geom->head_fwhr * 0.4));
    return true;
}

bool evaluate_cheating_threat_index(const teddy_geometry_t *geom, double provocation_scale, double *cheating_threat_out) {
    if (!geom || provocation_scale < 0.0 || !cheating_threat_out) {
        return false;
    }
    double base_cheat = 0.0;
    if (!evaluate_cooperative_cheating_risk(geom, 0.5, &base_cheat)) {
        return false;
    }
    *cheating_threat_out = base_cheat + (provocation_scale * 0.35 * geom->head_fwhr);
    return true;
}

bool evaluate_provocation_replication_consistency(const teddy_geometry_t *geom, double replication_scale, double *consistency_out) {
    if (!geom || replication_scale < 0.0 || !consistency_out) {
        return false;
    }
    double threat = evaluate_fw_threat_level(geom);
    *consistency_out = threat * (1.0 + (replication_scale * geom->status_aggression * 0.15));
    return true;
}

bool evaluate_provocation_rapid_threat_limit(const teddy_geometry_t *geom, double provocation_scale, double *limit_out) {
    if (!geom || provocation_scale < 0.0 || !limit_out) {
        return false;
    }
    double threat = evaluate_fw_threat_level(geom);
    *limit_out = threat * (1.5 + (provocation_scale * geom->status_aggression * 0.25));
    return true;
}

bool evaluate_provocation_retaliation_boundary(const teddy_geometry_t *geom, double exposure_ms, double *boundary_out) {
    if (!geom || exposure_ms < 0.0 || !boundary_out) {
        return false;
    }
    double base_boundary = 0.0;
    if (!evaluate_reactive_retaliation_boundary(geom, exposure_ms, &base_boundary)) {
        return false;
    }
    *boundary_out = base_boundary * (1.1 + geom->status_aggression * 0.1);
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

bool evaluate_exposure_decay_uncanny(const teddy_geometry_t *geom, double exposure_duration_sec, double *decayed_uncanny_out) {
    if (!geom || exposure_duration_sec < 0.0 || !decayed_uncanny_out) {
        return false;
    }
    double base_uncanny = 0.0;
    evaluate_uncanny_mismatch_index(geom, &base_uncanny);
    double tau = 10.0;
    *decayed_uncanny_out = base_uncanny * exp(-exposure_duration_sec / tau);
    return true;
}

bool evaluate_freeze_habituation_decay(const teddy_geometry_t *geom, double exposure_duration_sec, double *decayed_uncanny_out) {
    if (!geom || exposure_duration_sec < 0.0 || !decayed_uncanny_out) {
        return false;
    }
    double base_uncanny = 0.0;
    evaluate_uncanny_mismatch_index(geom, &base_uncanny);
    double tau = 15.0;
    *decayed_uncanny_out = base_uncanny * exp(-exposure_duration_sec / tau);
    return true;
}

bool evaluate_velocity_jitter_uncanny(const teddy_geometry_t *geom, double velocity_variance, double *uncanny_score_out) {
    if (!geom || velocity_variance < 0.0 || !uncanny_score_out) {
        return false;
    }
    double base_uncanny = 0.0;
    evaluate_uncanny_mismatch_index(geom, &base_uncanny);
    *uncanny_score_out = base_uncanny + (velocity_variance * 3.5 * geom->behavioral_mismatch);
    return true;
}

bool evaluate_expression_freeze_uncanny(const teddy_geometry_t *geom, double freeze_duration_sec, double *uncanny_score_out) {
    if (!geom || freeze_duration_sec < 0.0 || !uncanny_score_out) {
        return false;
    }
    double base_uncanny = 0.0;
    evaluate_uncanny_mismatch_index(geom, &base_uncanny);
    double freeze_factor = (freeze_duration_sec > 1.5) ? (freeze_duration_sec * 2.0) : 0.2;
    *uncanny_score_out = base_uncanny + (freeze_factor * (1.0 + geom->behavioral_mismatch));
    return true;
}

bool evaluate_expression_freeze_frequency(const teddy_geometry_t *geom, double freeze_frequency_hz, double *uncanny_score_out) {
    if (!geom || freeze_frequency_hz < 0.0 || !uncanny_score_out) {
        return false;
    }
    double base_uncanny = 0.0;
    evaluate_uncanny_mismatch_index(geom, &base_uncanny);
    *uncanny_score_out = base_uncanny + (freeze_frequency_hz * 1.5 * (1.0 + geom->behavioral_mismatch));
    return true;
}

bool evaluate_expression_asymmetry_uncanny(const teddy_geometry_t *geom, double asymmetry_deviation, double *uncanny_score_out) {
    if (!geom || asymmetry_deviation < 0.0 || !uncanny_score_out) {
        return false;
    }
    double base_uncanny = 0.0;
    evaluate_uncanny_mismatch_index(geom, &base_uncanny);
    *uncanny_score_out = base_uncanny + (asymmetry_deviation * 4.5 * (1.0 - geom->symmetry));
    return true;
}

bool evaluate_expression_asymmetry_duration(const teddy_geometry_t *geom, double asymmetry_duration_ms, double *uncanny_score_out) {
    if (!geom || asymmetry_duration_ms < 0.0 || !uncanny_score_out) {
        return false;
    }
    double base_uncanny = 0.0;
    evaluate_uncanny_mismatch_index(geom, &base_uncanny);
    double duration_factor = (asymmetry_duration_ms > 200.0) ? (asymmetry_duration_ms * 0.02) : 0.01;
    *uncanny_score_out = base_uncanny + (duration_factor * (1.0 - geom->symmetry));
    return true;
}

bool evaluate_expression_sync_uncanny(const teddy_geometry_t *geom, double sync_delay_ms, double *uncanny_score_out) {
    if (!geom || sync_delay_ms < 0.0 || !uncanny_score_out) {
        return false;
    }
    double base_uncanny = 0.0;
    evaluate_uncanny_mismatch_index(geom, &base_uncanny);
    double sync_factor = (sync_delay_ms > 50.0) ? (sync_delay_ms * 0.08) : 0.05;
    *uncanny_score_out = base_uncanny + (sync_factor * (1.0 + geom->behavioral_mismatch));
    return true;
}

bool evaluate_sync_habituation_decay(const teddy_geometry_t *geom, double exposure_duration_sec, double *decayed_uncanny_out) {
    if (!geom || exposure_duration_sec < 0.0 || !decayed_uncanny_out) {
        return false;
    }
    double base_uncanny = 0.0;
    evaluate_uncanny_mismatch_index(geom, &base_uncanny);
    double tau = 12.0;
    *decayed_uncanny_out = base_uncanny * exp(-exposure_duration_sec / tau);
    return true;
}

bool evaluate_acceleration_jitter_uncanny(const teddy_geometry_t *geom, double acceleration_variance, double *uncanny_score_out) {
    if (!geom || acceleration_variance < 0.0 || !uncanny_score_out) {
        return false;
    }
    double base_uncanny = 0.0;
    evaluate_uncanny_mismatch_index(geom, &base_uncanny);
    *uncanny_score_out = base_uncanny + (acceleration_variance * 5.0 * geom->behavioral_mismatch);
    return true;
}

bool evaluate_acceleration_habituation_decay(const teddy_geometry_t *geom, double exposure_duration_sec, double *decayed_uncanny_out) {
    if (!geom || exposure_duration_sec < 0.0 || !decayed_uncanny_out) {
        return false;
    }
    double base_uncanny = 0.0;
    evaluate_uncanny_mismatch_index(geom, &base_uncanny);
    double tau = 14.0;
    *decayed_uncanny_out = base_uncanny * exp(-exposure_duration_sec / tau);
    return true;
}

bool evaluate_animation_acceleration_jitter(const teddy_geometry_t *geom, double acceleration_jitter, double *uncanny_score_out) {
    if (!geom || acceleration_jitter < 0.0 || !uncanny_score_out) {
        return false;
    }
    double base_uncanny = 0.0;
    evaluate_uncanny_mismatch_index(geom, &base_uncanny);
    *uncanny_score_out = base_uncanny + (acceleration_jitter * 6.0 * geom->behavioral_mismatch);
    return true;
}

bool evaluate_amplitude_jitter_uncanny(const teddy_geometry_t *geom, double amplitude_variance, double *uncanny_score_out) {
    if (!geom || amplitude_variance < 0.0 || !uncanny_score_out) {
        return false;
    }
    double base_uncanny = 0.0;
    evaluate_uncanny_mismatch_index(geom, &base_uncanny);
    *uncanny_score_out = base_uncanny + (amplitude_variance * 4.0 * geom->behavioral_mismatch);
    return true;
}

bool evaluate_expression_amplitude_jitter(const teddy_geometry_t *geom, double amplitude_variance, double *uncanny_score_out) {
    return evaluate_amplitude_jitter_uncanny(geom, amplitude_variance, uncanny_score_out);
}

bool evaluate_amplitude_habituation_decay(const teddy_geometry_t *geom, double exposure_duration_sec, double *decayed_uncanny_out) {
    if (!geom || exposure_duration_sec < 0.0 || !decayed_uncanny_out) {
        return false;
    }
    double base_uncanny = 0.0;
    evaluate_uncanny_mismatch_index(geom, &base_uncanny);
    double tau = 16.0;
    *decayed_uncanny_out = base_uncanny * exp(-exposure_duration_sec / tau);
    return true;
}

bool evaluate_vocal_visual_sync_uncanny(const teddy_geometry_t *geom, double audio_lag_ms, double *uncanny_score_out) {
    if (!geom || audio_lag_ms < 0.0 || !uncanny_score_out) {
        return false;
    }
    double base_uncanny = 0.0;
    evaluate_uncanny_mismatch_index(geom, &base_uncanny);
    double lag_factor = (audio_lag_ms > 80.0) ? (audio_lag_ms * 0.05) : 0.05;
    *uncanny_score_out = base_uncanny + (lag_factor * (1.0 + geom->behavioral_mismatch));
    return true;
}

bool evaluate_vocal_visual_acceleration_sync(const teddy_geometry_t *geom, double acceleration_delay_ms, double *uncanny_score_out) {
    if (!geom || acceleration_delay_ms < 0.0 || !uncanny_score_out) {
        return false;
    }
    double base_uncanny = 0.0;
    evaluate_uncanny_mismatch_index(geom, &base_uncanny);
    double delay_factor = (acceleration_delay_ms > 40.0) ? (acceleration_delay_ms * 0.1) : 0.05;
    *uncanny_score_out = base_uncanny + (delay_factor * (1.0 + geom->behavioral_mismatch));
    return true;
}

bool evaluate_pitch_velocity_mismatch(const teddy_geometry_t *geom, double pitch_velocity_mismatch, double *uncanny_score_out) {
    if (!geom || pitch_velocity_mismatch < 0.0 || !uncanny_score_out) {
        return false;
    }
    double base_uncanny = 0.0;
    evaluate_uncanny_mismatch_index(geom, &base_uncanny);
    *uncanny_score_out = base_uncanny + (pitch_velocity_mismatch * 3.5 * geom->behavioral_mismatch);
    return true;
}

bool evaluate_vocal_visual_pitch_mismatch(const teddy_geometry_t *geom, double pitch_velocity_mismatch, double *uncanny_score_out) {
    return evaluate_pitch_velocity_mismatch(geom, pitch_velocity_mismatch, uncanny_score_out);
}

bool evaluate_pitch_freeze_uncanny(const teddy_geometry_t *geom, double pitch_velocity_mismatch, double freeze_duration_sec, double *uncanny_score_out) {
    if (!geom || pitch_velocity_mismatch < 0.0 || freeze_duration_sec < 0.0 || !uncanny_score_out) {
        return false;
    }
    double base_mismatch = 0.0;
    if (!evaluate_vocal_visual_pitch_mismatch(geom, pitch_velocity_mismatch, &base_mismatch)) {
        return false;
    }
    double freeze_factor = (freeze_duration_sec > 1.5) ? (freeze_duration_sec * 0.4) : 0.1;
    *uncanny_score_out = base_mismatch + (freeze_factor * (1.0 + geom->behavioral_mismatch));
    return true;
}

bool evaluate_pitch_jitter_uncanny(const teddy_geometry_t *geom, double pitch_velocity_mismatch, double jitter_deviation, double *uncanny_score_out) {
    if (!geom || pitch_velocity_mismatch < 0.0 || jitter_deviation < 0.0 || !uncanny_score_out) {
        return false;
    }
    double base_mismatch = 0.0;
    if (!evaluate_vocal_visual_pitch_mismatch(geom, pitch_velocity_mismatch, &base_mismatch)) {
        return false;
    }
    double jitter_factor = (jitter_deviation > 0.05) ? (jitter_deviation * 2.5) : 0.05;
    *uncanny_score_out = base_mismatch + (jitter_factor * (1.0 + geom->behavioral_mismatch));
    return true;
}

bool evaluate_motion_acceleration_mismatch(const teddy_geometry_t *geom, double acceleration_mismatch, double *uncanny_score_out) {
    if (!geom || acceleration_mismatch < 0.0 || !uncanny_score_out) {
        return false;
    }
    *uncanny_score_out = acceleration_mismatch * (1.2 + geom->behavioral_mismatch * 0.4);
    return true;
}

bool evaluate_motion_freeze_uncanny(const teddy_geometry_t *geom, double motion_mismatch, double freeze_duration_sec, double *uncanny_score_out) {
    if (!geom || motion_mismatch < 0.0 || freeze_duration_sec < 0.0 || !uncanny_score_out) {
        return false;
    }
    double base_mismatch = 0.0;
    if (!evaluate_motion_uncanny_index(geom, motion_mismatch, &base_mismatch)) {
        return false;
    }
    double freeze_factor = (freeze_duration_sec > 1.5) ? (freeze_duration_sec * 0.45) : 0.12;
    *uncanny_score_out = base_mismatch + (freeze_factor * (1.0 + geom->behavioral_mismatch));
    return true;
}

bool evaluate_pitch_mismatch_habituation_decay(const teddy_geometry_t *geom, double exposure_duration_sec, double *decayed_uncanny_out) {
    if (!geom || exposure_duration_sec < 0.0 || !decayed_uncanny_out) {
        return false;
    }
    double base_uncanny = 0.0;
    evaluate_uncanny_mismatch_index(geom, &base_uncanny);
    double tau = 18.0;
    *decayed_uncanny_out = base_uncanny * exp(-exposure_duration_sec / tau);
    return true;
}

bool evaluate_spatial_interaction_distance(const teddy_geometry_t *geom, double physical_distance_meters, double *interaction_uncanny_out) {
    if (!geom || physical_distance_meters < 0.0 || !interaction_uncanny_out) {
        return false;
    }
    double proximity_factor = 0.0;
    if (physical_distance_meters < 1.0) {
        proximity_factor = (1.0 - physical_distance_meters) * 4.0;
    }
    *interaction_uncanny_out = proximity_factor * (1.2 + geom->eye_scale * 0.5);
    return true;
}

bool evaluate_dynamic_auditory_offset(const teddy_geometry_t *geom, double ambient_noise_db, double vocal_gain_db, double *auditory_offset_out) {
    if (!geom || ambient_noise_db < 0.0 || vocal_gain_db < 0.0 || !auditory_offset_out) {
        return false;
    }
    double diff = fabs(vocal_gain_db - ambient_noise_db);
    *auditory_offset_out = diff * (0.8 + geom->head_fwhr * 0.3);
    return true;
}

bool evaluate_keating_babyfacedness_index(const teddy_geometry_t *geom, double *babyfacedness_out) {
    if (!geom || !babyfacedness_out) {
        return false;
    }
    *babyfacedness_out = (geom->eye_scale * 0.4) + (-geom->feature_vertical_offset * 0.4) + ((1.0 / (1.0 + geom->jaw_scale)) * 0.2);
    return true;
}

bool evaluate_keating_dominance_cue_index(const teddy_geometry_t *geom, double *dominance_out) {
    if (!geom || !dominance_out) {
        return false;
    }
    *dominance_out = (geom->jaw_scale * 0.4) + (geom->feature_vertical_offset * 0.4) + ((1.0 / (1.0 + geom->eye_scale)) * 0.2);
    return true;
}

bool evaluate_hyde_gaze_direction_index(const teddy_geometry_t *geom, double target_angle, double *gaze_alignment_out) {
    if (!geom || !gaze_alignment_out) {
        return false;
    }
    *gaze_alignment_out = fabs(sin(target_angle)) * (1.0 + (1.0 - geom->symmetry) * 2.0);
    return true;
}

bool evaluate_hyde_vocal_tempo_variance(const teddy_geometry_t *geom, double vocal_tempo_bpm, double *vocal_uncanny_out) {
    if (!geom || vocal_tempo_bpm < 0.0 || !vocal_uncanny_out) {
        return false;
    }
    *vocal_uncanny_out = fabs(vocal_tempo_bpm - 120.0) * (0.05 + geom->behavioral_mismatch * 0.2);
    return true;
}

bool evaluate_geniole_provocation_aggression_limit(const teddy_geometry_t *geom, double provocation_scale, double *aggression_limit_out) {
    if (!geom || provocation_scale < 0.0 || !aggression_limit_out) {
        return false;
    }
    *aggression_limit_out = provocation_scale * (0.5 + geom->head_fwhr * 1.5 + geom->leadership_profile * 0.5);
    return true;
}

bool evaluate_geniole_testosterone_modulator(const teddy_geometry_t *geom, double baseline_testosterone, double *modulator_out) {
    if (!geom || baseline_testosterone < 0.0 || !modulator_out) {
        return false;
    }
    *modulator_out = baseline_testosterone * (0.8 + geom->head_fwhr * 0.4 + geom->jaw_scale * 0.2);
    return true;
}

bool evaluate_keating_brow_dominance(const teddy_geometry_t *geom, double brow_height, double *brow_dominance_out) {
    if (!geom || brow_height < 0.0 || !brow_dominance_out) {
        return false;
    }
    *brow_dominance_out = (1.0 / (1.0 + brow_height)) * (1.2 + geom->leadership_profile * 0.4);
    return true;
}

bool evaluate_keating_lip_trustworthiness(const teddy_geometry_t *geom, double lip_thickness, double *lip_trustworthiness_out) {
    if (!geom || lip_thickness < 0.0 || !lip_trustworthiness_out) {
        return false;
    }
    *lip_trustworthiness_out = lip_thickness * (0.8 + geom->empathy_index * 0.5);
    return true;
}

bool evaluate_hyde_conversational_latency(const teddy_geometry_t *geom, double lag_seconds, double *latency_uncanny_out) {
    if (!geom || lag_seconds < 0.0 || !latency_uncanny_out) {
        return false;
    }
    *latency_uncanny_out = lag_seconds * (0.6 + geom->behavioral_mismatch * 0.5);
    return true;
}

bool evaluate_hyde_vocal_size_mismatch(const teddy_geometry_t *geom, double voice_pitch_hz, double *size_mismatch_out) {
    if (!geom || voice_pitch_hz < 0.0 || !size_mismatch_out) {
        return false;
    }
    *size_mismatch_out = fabs(voice_pitch_hz - (300.0 - geom->torso_ratio * 100.0)) * 0.05;
    return true;
}

bool evaluate_keating_brow_eye_ratio(const teddy_geometry_t *geom, double brow_eye_distance, double *ratio_dominance_out) {
    if (!geom || brow_eye_distance < 0.0 || !ratio_dominance_out) {
        return false;
    }
    *ratio_dominance_out = (1.0 / (1.0 + brow_eye_distance)) * (1.1 + geom->leadership_profile * 0.5);
    return true;
}

bool evaluate_geniole_fwhr_dilation_map(const teddy_geometry_t *geom, double base_dilation, double *mapped_dilation_out) {
    if (!geom || base_dilation < 0.0 || !mapped_dilation_out) {
        return false;
    }
    *mapped_dilation_out = base_dilation * (1.0 + geom->head_fwhr * 0.6);
    return true;
}

bool evaluate_keating_head_roundness_index(const teddy_geometry_t *geom, double *roundness_out) {
    if (!geom || !roundness_out) {
        return false;
    }
    *roundness_out = geom->head_fwhr * (1.1 + geom->empathy_index * 0.4);
    return true;
}

bool evaluate_keating_symmetry_trust(const teddy_geometry_t *geom, double *trust_out) {
    if (!geom || !trust_out) {
        return false;
    }
    *trust_out = geom->symmetry * (1.0 + geom->honesty_index * 0.5);
    return true;
}

bool evaluate_hyde_vocal_amplitude_mismatch(const teddy_geometry_t *geom, double voice_amplitude_db, double *amplitude_mismatch_out) {
    if (!geom || voice_amplitude_db < 0.0 || !amplitude_mismatch_out) {
        return false;
    }
    *amplitude_mismatch_out = fabs(voice_amplitude_db - 60.0) * (0.4 + geom->behavioral_mismatch * 0.4);
    return true;
}

bool evaluate_hyde_mouth_speed_synchrony(const teddy_geometry_t *geom, double mouth_speed, double pitch_acceleration, double *sync_mismatch_out) {
    if (!geom || mouth_speed < 0.0 || !sync_mismatch_out) {
        return false;
    }
    *sync_mismatch_out = fabs(mouth_speed - fabs(pitch_acceleration)) * (0.8 + geom->stiffness * 0.5);
    return true;
}

bool evaluate_keating_brow_chin_proportion(const teddy_geometry_t *geom, double brow_chin_distance, double *proportion_dominance_out) {
    if (!geom || brow_chin_distance < 0.0 || !proportion_dominance_out) {
        return false;
    }
    *proportion_dominance_out = (1.0 / (1.0 + brow_chin_distance)) * (1.3 + geom->leadership_profile * 0.3);
    return true;
}

bool evaluate_geniole_fwhr_jitter_mod(const teddy_geometry_t *geom, double base_jitter, double *mapped_jitter_out) {
    if (!geom || base_jitter < 0.0 || !mapped_jitter_out) {
        return false;
    }
    *mapped_jitter_out = base_jitter * (0.9 + geom->head_fwhr * 0.5);
    return true;
}

bool evaluate_keating_gaze_dominance(const teddy_geometry_t *geom, double gaze_duration_sec, double aversion_angle, double *gaze_dominance_out) {
    if (!geom || gaze_duration_sec < 0.0 || aversion_angle < 0.0 || !gaze_dominance_out) {
        return false;
    }
    *gaze_dominance_out = (gaze_duration_sec / (1.0 + aversion_angle)) * (1.1 + geom->leadership_profile * 0.4);
    return true;
}

bool evaluate_keating_brow_gesture(const teddy_geometry_t *geom, double brow_raise_amplitude, double *brow_submissiveness_out) {
    if (!geom || brow_raise_amplitude < 0.0 || !brow_submissiveness_out) {
        return false;
    }
    *brow_submissiveness_out = brow_raise_amplitude * (0.9 + geom->empathy_index * 0.5);
    return true;
}

bool evaluate_hyde_turn_interruption(const teddy_geometry_t *geom, double overlap_duration_sec, double *interruption_uncanny_out) {
    if (!geom || overlap_duration_sec < 0.0 || !interruption_uncanny_out) {
        return false;
    }
    *interruption_uncanny_out = overlap_duration_sec * (0.5 + geom->behavioral_mismatch * 0.4);
    return true;
}

bool evaluate_hyde_vocal_tremor_index(const teddy_geometry_t *geom, double pitch_variance, double *tremor_uncanny_out) {
    if (!geom || pitch_variance < 0.0 || !tremor_uncanny_out) {
        return false;
    }
    *tremor_uncanny_out = pitch_variance * (0.8 + (1.0 - geom->resilience_index) * 0.5);
    return true;
}

bool evaluate_keating_torso_head_ratio(const teddy_geometry_t *geom, double torso_span, double *ratio_dominance_out) {
    if (!geom || torso_span < 0.0 || !ratio_dominance_out) {
        return false;
    }
    *ratio_dominance_out = torso_span * (0.8 + geom->torso_ratio * 0.4 + geom->leadership_profile * 0.3);
    return true;
}

bool evaluate_geniole_fwhr_boundary_map(const teddy_geometry_t *geom, double threshold_scale, double *mapped_boundary_out) {
    if (!geom || threshold_scale < 0.0 || !mapped_boundary_out) {
        return false;
    }
    *mapped_boundary_out = threshold_scale * (1.1 + geom->head_fwhr * 0.5);
    return true;
}

