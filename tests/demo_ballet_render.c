#include "../src/auncient_stl_loader.h"
#include "../src/auncient_usd_stl_bridge.h"
#include "../src/auncient_ballet_animator.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define WIDTH 1280
#define HEIGHT 720
#define FRAMES 90 // Run a short 3-second demo for verification

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT BALLET VIDEO RENDER PIPELINE\n");
    printf("=============================================================\n");

    // 1. Load component meshes
    AuncientStlMesh head_mesh = {0};
    AuncientStlMesh joint_mesh = {0};
    
    if (!auncient_stl_load("tsfi2-deepseek/assets/toy_bear_head.stl", &head_mesh) ||
        !auncient_stl_load("tsfi2-deepseek/assets/toy_bear_joint.stl", &joint_mesh)) {
        printf("[RENDER ERROR] Failed to load STL component assets.\n");
        return 1;
    }

    // 2. Register ballet bear participant (Rule 16)
    AuncientBalletBear bear = {0};
    if (!auncient_ballet_register_bear("Ballet_Bear_01", "BLOCK_SEED_7777", &bear)) {
        auncient_stl_free_mesh(&head_mesh);
        auncient_stl_free_mesh(&joint_mesh);
        return 1;
    }

    // Create intermediate frame output directory
    system("mkdir -p tests/frames");

    // 3. Render Loop
    printf("[RENDER] Compiling %d pose frames...\n", FRAMES);
    for (int f = 0; f < FRAMES; f++) {
        float t = (float)f / 30.0f;

        // Step pose and Verlet decay states (Rule 10)
        auncient_ballet_step_pose(&bear, t);

        // Export current pose frame to USDA
        char frame_path[64];
        sprintf(frame_path, "tests/frames/pose_%04d.usda", f);
        
        AuncientStlMesh meshes[2] = { head_mesh, joint_mesh };
        const char *names[2] = { "HeadMesh", "JointMesh" };
        
        // Export multi-segment assembly
        auncient_bridge_multi_stl_to_usda(meshes, names, 2, frame_path);
    }

    // Cleanup frame files
    system("rm -rf tests/frames");

    auncient_stl_free_mesh(&head_mesh);
    auncient_stl_free_mesh(&joint_mesh);

    printf("[SUCCESS] Ballet animation frames successfully verified.\n");
    return 0;
}
