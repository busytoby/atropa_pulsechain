#include "auncient_cloth_material_bridge.h"
#include <math.h>

void auncient_bridge_material_to_cloth_color(const MaterialUniformBlock *mat_block, ClothVertex *vertices, int count) {
    if (!mat_block || !vertices) return;

    for (int i = 0; i < count; i++) {
        // Dynamic procedural color grading overrides based on material presets
        if (mat_block->color_preset == 1) {
            // Ocean preset: blue-cyan gradient mapping based on V texture coordinates
            uint8_t b = (uint8_t)(200.0f * vertices[i].v + 55.0f);
            uint8_t g = (uint8_t)(100.0f * vertices[i].v + 20.0f);
            vertices[i].color = 0xFF000000 | (b << 16) | (g << 8) | 0x11;
        } 
        else if (mat_block->color_preset == 2) {
            // Fire preset: orange-red gradient mapping based on V texture coordinates
            uint8_t r = (uint8_t)(200.0f * vertices[i].v + 55.0f);
            uint8_t g = (uint8_t)(120.0f * vertices[i].v + 10.0f);
            vertices[i].color = 0xFF000000 | 0x11 | (g << 8) | r;
        } 
        else if (mat_block->color_preset == 3) {
            // Obsidian preset: dark obsidian purple mapping
            uint8_t r = (uint8_t)(40.0f * vertices[i].v + 10.0f);
            uint8_t b = (uint8_t)(60.0f * vertices[i].v + 20.0f);
            vertices[i].color = 0xFF000000 | (b << 16) | r;
        }
    }
}

void auncient_bridge_cloth_to_instance(float wind_x, float wind_y, float wind_z, InstanceUniformBlock *inst_block) {
    if (!inst_block) return;

    // Calculate wind magnitude to modulate instance swing rotations
    float wind_mag = sqrtf(wind_x*wind_x + wind_y*wind_y + wind_z*wind_z);
    
    // Scale swing parameter feedback (15.0 radian scalar)
    inst_block->joint_swing_angle = wind_mag * 15.0f;
}
