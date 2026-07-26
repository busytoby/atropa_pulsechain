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

void auncient_bridge_update_cloth_physics(const MaterialUniformBlock *mat_block, float global_time, float base_wind_x, float base_wind_y, float base_wind_z) {
    if (!mat_block) return;

    // Dynamic wind turbulence modulation using material frequency
    float freq = mat_block->wind_turbulence_frequency;
    float turb = sinf(global_time * freq) * 0.005f;

    float active_wind_x = base_wind_x + turb;
    float active_wind_y = base_wind_y + sinf(global_time * freq * 1.5f) * 0.002f;
    float active_wind_z = base_wind_z + cosf(global_time * freq * 0.8f) * 0.003f;

    // Scale active wind by the inverse of the fabric friction coefficient to simulate resistance drag
    float drag_scale = 1.0f / (mat_block->fabric_friction_coefficient + 0.1f);
    active_wind_x *= drag_scale;
    active_wind_y *= drag_scale;
    active_wind_z *= drag_scale;

    // Invoke soft body Verlet physics update step
    cloth_update(active_wind_x, active_wind_y, active_wind_z);
}
