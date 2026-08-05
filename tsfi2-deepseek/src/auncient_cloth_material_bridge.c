#include "auncient_cloth_material_bridge.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

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

extern int tsfi_mf_ssa_resolve_issuance_site(const char *ssn, char *site_name_out, int max_len);

void auncient_bridge_dna_to_ssa(const MaterialUniformBlock *mat_block, char *ssn_out, char *site_out, int max_len) {
    if (!mat_block || !ssn_out || !site_out || max_len < 16) return;

    // Deterministic Area Lot from seed: value from 1 to 9 (e.g. area = (seed % 9) + 1)
    int area = (int)(mat_block->seed % 9) + 1;

    // Format the simulated SSN: AAA-12-3456
    snprintf(ssn_out, 12, "%03d-12-3456", area);

    // Resolve historical creation location via the SSA site resolver
    tsfi_mf_ssa_resolve_issuance_site(ssn_out, site_out, max_len);
}

void auncient_bridge_usd_to_ssa(const TSFiUsdShade *usd_shade, char *ssn_out, char *site_out, int max_len) {
    if (!usd_shade || !ssn_out || !site_out || max_len < 16) return;

    // Generate a deterministic seed from the material_id string using FNV-1a
    uint32_t seed = 0x811C9DC5;
    size_t id_len = strlen(usd_shade->material_id);
    for (size_t i = 0; i < id_len; i++) {
        seed = (seed ^ usd_shade->material_id[i]) * 0x01000193;
    }

    // Deterministic Area Lot from parsed seed
    int area = (int)(seed % 9) + 1;

    // Format simulated SSN: AAA-12-3456
    snprintf(ssn_out, 12, "%03d-12-3456", area);

    // Resolve historical creation location via the SSA site resolver
    tsfi_mf_ssa_resolve_issuance_site(ssn_out, site_out, max_len);
}

void auncient_bridge_entity_to_ssa(const char *entity_name, char *ssn_out, char *site_out, int max_len) {
    if (!entity_name || !ssn_out || !site_out || max_len < 16) return;

    // Generate a deterministic seed from the entity_name string using FNV-1a
    uint32_t seed = 0x811C9DC5;
    size_t name_len = strlen(entity_name);
    for (size_t i = 0; i < name_len; i++) {
        seed = (seed ^ entity_name[i]) * 0x01000193;
    }

    // Deterministic Area Lot from parsed seed
    int area = (int)(seed % 9) + 1;

    // Format simulated SSN: AAA-12-3456
    snprintf(ssn_out, 12, "%03d-12-3456", area);

    // Resolve historical creation location via the SSA site resolver
    tsfi_mf_ssa_resolve_issuance_site(ssn_out, site_out, max_len);
}
