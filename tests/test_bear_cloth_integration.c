#include "../../../ds/apEx-public/apEx/Phoenix/dna_teddy_bear_sculptor_c.h"
#include "../tsfi2-deepseek/src/cloth_simulator.h"
#include <stdio.h>
#include <assert.h>
#include <math.h>

#include "../tsfi2-deepseek/src/auncient_cloth_material_bridge.h"
#include <string.h>

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT TEDDY BEAR AND CLOTH VERLET INTEGRATION TEST\n");
    printf("=============================================================\n");

    // 1. Initialize Bear DNA & Mesh
    uint8_t dna_bytes[8] = {0x80, 0x80, 0x80, 0x80, 0x01, 0x00, 0x00, 0x00};
    DNATeddyBearSculptDesc bear_desc;
    dna_teddy_bear_decode_dna(&bear_desc, dna_bytes);

    // Verify SSA lot registration for newly born teddy bear
    MaterialUniformBlock bear_mat;
    bear_mat.seed = 7; // Seed resolves to Area Lot 008 (Vermont)
    char ssn_out[16];
    char site_out[32];
    auncient_bridge_dna_to_ssa(&bear_mat, ssn_out, site_out, sizeof(site_out));
    assert(strcmp(ssn_out, "008-12-3456") == 0);
    assert(strcmp(site_out, "Vermont") == 0);
    printf("   ✓ Newly born teddy bear successfully allocated SSN %s in Area Lot (%s).\n", ssn_out, site_out);

    static DNATeddyBearVertex bear_vertices[5000];
    static int bear_indices[30000];
    int bear_v_count = 0;
    int bear_i_count = 0;

    // 2. Initialize Cloth
    cloth_init();
    printf("   ✓ Bear and Cloth data structures initialized.\n");

    // 3. Simulation Loop (30 Frames)
    for (int frame = 0; frame < 30; frame++) {
        // Update walk cycle animation timeline
        bear_desc.time = (float)frame * 0.05f;
        dna_teddy_bear_generate_mesh(bear_vertices, bear_indices, &bear_v_count, &bear_i_count, &bear_desc);

        // Find shoulder vertices (for example, first vertex of left arm and right arm)
        // Arms sit at indices around the mesh generation offsets
        float left_shoulder_x = bear_vertices[1000].x;
        float left_shoulder_y = bear_vertices[1000].y;
        float left_shoulder_z = bear_vertices[1000].z;

        float right_shoulder_x = bear_vertices[1200].x;
        float right_shoulder_y = bear_vertices[1200].y;
        float right_shoulder_z = bear_vertices[1200].z;

        // Attach cloth cape corners to bear's shoulder coordinates in Verlet space
        cloth_set_anchor(0, CLOTH_HEIGHT - 1, left_shoulder_x, left_shoulder_y, left_shoulder_z);
        cloth_set_anchor(CLOTH_WIDTH - 1, CLOTH_HEIGHT - 1, right_shoulder_x, right_shoulder_y, right_shoulder_z);

        // Run solver step
        cloth_update(0.005f, 0.0f, -0.002f);

        // Apply collisions with the bear's torso (center at 0,0,0 radius 1.0) and head (center at 0,1.8,0 radius 0.8)
        cloth_apply_sphere_collision(0.0f, 0.0f, 0.0f, 1.0f);
        cloth_apply_sphere_collision(0.0f, 1.8f, 0.0f, 0.8f);
    }
    printf("   ✓ 30 frames of animated shoulder attachment and sphere collision executed.\n");

    // Generate output mesh for verification
    static ClothVertex cloth_vertices[1000];
    static int cloth_indices[5000];
    int cloth_v_count = 0;
    int cloth_i_count = 0;
    cloth_generate_mesh(cloth_vertices, cloth_indices, &cloth_v_count, &cloth_i_count);

    assert(cloth_v_count > 0);
    printf("   ✓ Verlet integration drape coordinates generated successfully.\n");

    printf("=============================================================\n");
    printf("AUNCIENT INTEGRATION TEST COMPLETE\n");
    printf("=============================================================\n");
    return 0;
}
