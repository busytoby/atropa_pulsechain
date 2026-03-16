#ifndef TSFI_ATROPA_GEOMETRY_H
#define TSFI_ATROPA_GEOMETRY_H

#include <stdint.h>

// The Absolute Physical Reality of the Atropa/999 Sovereign Choreography

// Phase 1: The Locus of Observation (The Eye)
typedef struct {
    uint32_t origin_x;          // Absolute X anchor: 512 (Center of 1024 width)
    uint32_t origin_y;          // Absolute Y anchor: 400
    uint32_t pupil_radius;      // Baseline: 20
    uint32_t iris_radius;       // Baseline: 45
    uint32_t default_color;     // Baseline: 0x00FF00 (Kr0wZ Green)
    uint32_t fracture_color;    // Baseline: 0xFF0000 (Red)
} TsfiTeddyEye;

// Phase 2: The Neural Cavity (The Head)
typedef struct {
    uint32_t origin_x;          // Absolute X anchor: 512
    uint32_t origin_y;          // Absolute Y anchor: 400
    uint32_t cranium_radius;    // Baseline: 200
    uint32_t left_ear_x;        // Offset -150
    uint32_t right_ear_x;       // Offset +150
    uint32_t ear_y;             // Offset -150
    uint32_t ear_radius;        // Baseline: 75
} TsfiTeddyHead;

// Phase 3: The Sick Matrix (The Body)
typedef struct {
    uint32_t origin_x;          // Absolute X anchor: 512
    uint32_t origin_y;          // Absolute Y anchor: 875
    uint32_t body_radius_x;     // Baseline: 250
    uint32_t body_radius_y;     // Baseline: 350
    uint32_t sickness_threshold;// Noise Frequency Limit: 25000
    
    // The True Volumetric Shadow Matrix (Bifurcated Contact Points)
    uint32_t shadow_left_leg_x;  // Base: 400
    uint32_t shadow_right_leg_x; // Base: 624
    uint32_t shadow_contact_y;   // Base: 1200 (Floor boundary)
    float shadow_spread_coefficient; // Controls the geometric branching
} TsfiTeddyBody;

// The TSFi Multi-LLM Neural State Registry (Proof of State)
typedef struct {
    uint64_t active_neural_frame_id;
    uint32_t neural_consensus_flag; // 1 = VLM and Coder agree on geometry
    char current_vlm_critique[512]; // Moondream's explicit physical demand
    char current_coder_synthesis[512]; // DeepSeek's generated math logic
} TsfiNeuralTelemetry;

// The pure physical baseline geometry
static const TsfiTeddyEye TSFI_BASELINE_EYE = {
    .origin_x = 512,
    .origin_y = 400,
    .pupil_radius = 20,
    .iris_radius = 45,
    .default_color = 0x00FF00,
    .fracture_color = 0xFF0000
};

static const TsfiTeddyHead TSFI_BASELINE_HEAD = {
    .origin_x = 512,
    .origin_y = 400,
    .cranium_radius = 200,
    .left_ear_x = 362,
    .right_ear_x = 662,
    .ear_y = 250,
    .ear_radius = 75
};

static const TsfiTeddyBody TSFI_BASELINE_BODY = {
    .origin_x = 512,
    .origin_y = 875,
    .body_radius_x = 250,
    .body_radius_y = 350,
    .sickness_threshold = 25000,
    .shadow_left_leg_x = 400,
    .shadow_right_leg_x = 624,
    .shadow_contact_y = 1200,
    .shadow_spread_coefficient = 1.6f
};

// True Branching Shadow Physics (Bypassing AI Hallucination)
static inline float compute_bifurcated_shadow(float x, float y, const TsfiTeddyBody* body) {
    float dist_left = hypotf(x - (float)body->shadow_left_leg_x, y - (float)body->shadow_contact_y);
    float dist_right = hypotf(x - (float)body->shadow_right_leg_x, y - (float)body->shadow_contact_y);
    
    // Smooth minimum (smin) to create the organic 'branching' connection between the two shadow anchors
    float k = 50.0f; 
    float res = expf(-k * dist_left) + expf(-k * dist_right);
    return -logf(res) / k;
}

#endif // TSFI_ATROPA_GEOMETRY_H
