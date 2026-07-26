#ifndef AUNCIENT_VULKAN_MATERIALS_H
#define AUNCIENT_VULKAN_MATERIALS_H

#include <stdint.h>

// Auncient Vulkan Scope-Based Material Descriptor Layouts

// Descriptor Set 0: Binds once per frame (Global context)
typedef struct {
    float projection_matrix[16];
    float view_matrix[16];
    float camera_position[3];
    float global_time;
} GlobalUniformBlock;

// Descriptor Set 1: Binds once per material group (Material template constants)
typedef struct {
    uint32_t seed;
    float persistence;
    float lacunarity;
    uint32_t octaves;
    uint32_t color_preset;
    float material_blend_factor;
} MaterialUniformBlock;

// Descriptor Set 2: Binds once per draw call (Object instance variables)
typedef struct {
    float local_transform[16];
    float joint_swing_angle;
    uint32_t instance_id;
} InstanceUniformBlock;

#endif // AUNCIENT_VULKAN_MATERIALS_H
