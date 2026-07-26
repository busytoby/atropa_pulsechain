#include "../src/auncient_cloth_material_bridge.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT CLOTH TO MATERIAL BRIDGE TEST\n");
    printf("=============================================================\n");

    // 1. Initialize Mock Cloth Mesh
    ClothVertex vertices[5];
    for (int i = 0; i < 5; i++) {
        vertices[i].x = 0.0f;
        vertices[i].y = 0.0f;
        vertices[i].z = 0.0f;
        vertices[i].nx = 0.0f;
        vertices[i].ny = 1.0f;
        vertices[i].nz = 0.0f;
        vertices[i].u = (float)i / 4.0f;
        vertices[i].v = (float)i / 4.0f;
        vertices[i].color = 0xFFFFFFFF; // White default
    }

    // 2. Initialize Material Block
    MaterialUniformBlock mat_block;
    mat_block.seed = 1000;
    mat_block.persistence = 0.5f;
    mat_block.lacunarity = 2.0f;
    mat_block.octaves = 4;
    mat_block.color_preset = 1; // Ocean preset
    mat_block.material_blend_factor = 0.5f;

    // Apply color bridge
    auncient_bridge_material_to_cloth_color(&mat_block, vertices, 5);
    // Verifying blue channel is colored under Ocean preset (color has blue components in upper bits)
    assert((vertices[4].color & 0x00FF0000) != 0);
    printf("   ✓ Ocean material color bridge verified.\n");

    // Apply Fire preset
    mat_block.color_preset = 2;
    auncient_bridge_material_to_cloth_color(&mat_block, vertices, 5);
    // Verifying red channel is colored under Fire preset (lower bits)
    assert((vertices[4].color & 0x000000FF) != 0);
    printf("   ✓ Fire material color bridge verified.\n");

    // 3. Test Physics-to-Instance bridge
    InstanceUniformBlock inst_block;
    auncient_bridge_cloth_to_instance(0.01f, 0.0f, -0.015f, &inst_block);
    assert(inst_block.joint_swing_angle > 0.0f);
    printf("   ✓ Physics wind magnitude feedback loop verified.\n");

    // 4. Test Material-Modulated dynamic physics updates
    mat_block.wind_turbulence_frequency = 4.0f;
    mat_block.fabric_friction_coefficient = 0.8f;
    cloth_init();
    auncient_bridge_update_cloth_physics(&mat_block, 1.5f, 0.01f, 0.0f, -0.015f);
    printf("   ✓ Material-driven simulation step executed.\n");

    // 5. Test DNA-to-SSA lot allocation bridge
    char ssn_buf[16];
    char site_buf[32];
    mat_block.seed = 1; // Maps to Area Lot 002 -> New Hampshire
    auncient_bridge_dna_to_ssa(&mat_block, ssn_buf, site_buf, sizeof(site_buf));
    assert(strcmp(ssn_buf, "002-12-3456") == 0);
    assert(strcmp(site_buf, "New Hampshire") == 0);

    mat_block.seed = 7; // Maps to Area Lot 008 -> Vermont
    auncient_bridge_dna_to_ssa(&mat_block, ssn_buf, site_buf, sizeof(site_buf));
    assert(strcmp(ssn_buf, "008-12-3456") == 0);
    assert(strcmp(site_buf, "Vermont") == 0);
    printf("   ✓ DNA-to-SSA Area Lot matching verified.\n");

    printf("=============================================================\n");
    printf("AUNCIENT CLOTH TO MATERIAL BRIDGE TEST COMPLETE\n");
    printf("=============================================================\n");
    return 0;
}
