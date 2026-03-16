#ifndef TSFI_PUPPETRY_H
#define TSFI_PUPPETRY_H

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief TSFi Puppetry System
 * Bridges Neural Logic (Genie) with Physical Skeletons (Spider/ControlNet)
 */

typedef struct {
    float x, y, z;
} TsfiJoint;

typedef enum {
    PUPPET_TYPE_TEDDY = 0,
    PUPPET_TYPE_ALLIGATOR = 1,
    PUPPET_TYPE_CROW = 2,
    PUPPET_TYPE_POPPY = 3
} TsfiPuppetType;

typedef struct {
    TsfiJoint head;
    TsfiJoint neck;
    TsfiJoint body;
    TsfiJoint l_shoulder;
    TsfiJoint l_hand;
    TsfiJoint r_shoulder;
    TsfiJoint r_hand;
    TsfiJoint l_hip;
    TsfiJoint l_foot;
    TsfiJoint r_hip;
    TsfiJoint r_foot;
    TsfiJoint beak_upper; // Added for Crow speech articulation
    TsfiJoint beak_lower; // Added for Crow speech articulation
    
    float scale;
    float stuffing;
    float arm_length;
    float leg_length;
    float head_size;
    float ear_size;
    float sickness;
    uint32_t active;      // 1 if this puppet is being controlled
    uint32_t animal_type; // TsfiPuppetType
} TsfiPuppet;

#define TSFI_PUPPET_SHM "/tsfi_puppet_state"
#define TSFI_PUPPET_MAGIC 0x50555050 // 'PUPP'

typedef struct {
    uint32_t magic;
    uint32_t version;
    TsfiPuppet puppets[4]; // Support for 4 quadrants
    char vlm_critique[512];
    uint32_t last_neural_id;
    uint64_t timestamp_ns;
} TsfiPuppetState;

// API
TsfiPuppetState* tsfi_puppetry_shm_create();
TsfiPuppetState* tsfi_puppetry_shm_attach();
void tsfi_puppetry_shm_close(TsfiPuppetState *state);

#endif // TSFI_PUPPETRY_H
