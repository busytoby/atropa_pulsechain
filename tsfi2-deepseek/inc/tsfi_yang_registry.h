#ifndef TSFI_YANG_REGISTRY_H
#define TSFI_YANG_REGISTRY_H

#include <stdint.h>

// The Absolute TSFi / Dysnomia Acoustic State Registry
// Mapped into /dev/shm/tsfi_acoustic_space for Zero-Copy LLM Participation
typedef struct {
    // 1. The Dysnomia Simulation State (PulseChain Economics)
    float pdai_manifold_pressure; // 0.0 (Calm) to 1.0 (Total Economic Collapse)
    
    // 2. The Yang Acoustic Manifestation
    float yang_heartbeat_hz;      // Maps to pressure (e.g. 40Hz normal, 80Hz panic)
    float yang_static_amplitude;  // Volume of Kr0wZ Sickness (0.0 to 1.0)
    
    // 3. The LLM Participant Vector (DeepSeek's Behavioral Override)
    int agent_participation_flag; // 1 = Agent wants to change the Bear's genome
    int target_genome_id;         // 0: Sitting, 1: Slumped, 2: Standing, 3: Fetal
    char agent_monologue[256];    // What the AI is "thinking" based on the sound
    
    // 4. The Alligator's Spatial Matrix (Form)
    float alligator_x;            // Geometric position X
    float alligator_y;            // Geometric position Y
    float alligator_form_ratio;   // Current IoU from the Vision Critic
    
    // 5. The Spider's Spectral Matrix (Content)
    float spider_x;               // Geometric position X
    float spider_y;               // Geometric position Y
    float spider_texture_sharpness; // Variance of Laplacian from Vision Critic
    
    // 6. The Moondream2 Genie (Observation)
    char genie_poetic_observation[256]; // The VLM's grading and text synthesis
} TsfiAcousticRegistry;

#endif // TSFI_YANG_REGISTRY_H
