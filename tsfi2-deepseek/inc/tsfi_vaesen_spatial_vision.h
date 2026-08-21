#ifndef TSFI_VAESEN_SPATIAL_VISION_H
#define TSFI_VAESEN_SPATIAL_VISION_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "tsfi_zmachine_vaesen_bridge.h"

#define TSFI_VISION_MAX_LINES 16

/* Spatial Vision Environmental Flags */
#define TSFI_ENV_HAS_RUNNING_WATER (1 << 0)
#define TSFI_ENV_HAS_DIRECT_SUN    (1 << 1)
#define TSFI_ENV_CONSECRATED_SOIL  (1 << 2)
#define TSFI_ENV_THRESHOLD_BLOCKED (1 << 3)

typedef struct {
    uint8_t num_lines;
    TsfiZMachineVectorLine lines[TSFI_VISION_MAX_LINES];
    uint32_t detected_env_flags;
    float    spatial_comfort_index; /* [0.0..1.0] */
} TsfiVaesenSpatialVisionField;

typedef struct {
    /* Static Immutable Folklore ROM */
    uint32_t forbidden_env_mask;     /* Flags that cause instant repulsion */
    uint32_t sanctuary_env_mask;     /* Flags that restore vitality */
    char     core_folklore_law[128];

    /* Dynamic Volatile Session RAM */
    float    active_spatial_x;
    float    active_spatial_y;
    uint8_t  reaction_state;         /* 0=Calm, 1=Guarding, 2=Repelled/Fleeing */
} TsfiVaesenPartitionedMemory;

/* 2D Vector Spatial Vision API */
int tsfi_vaesen_evaluate_spatial_vision(
    const TsfiVaesenDeviceSliceBin *slice,
    const TsfiZMachineVectorLine *lines,
    uint8_t num_lines,
    TsfiVaesenSpatialVisionField *out_field,
    TsfiVaesenPartitionedMemory *out_mem
);

#endif /* TSFI_VAESEN_SPATIAL_VISION_H */
