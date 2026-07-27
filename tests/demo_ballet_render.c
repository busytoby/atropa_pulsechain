#define _POSIX_C_SOURCE 200809L
#include "../src/auncient_stl_loader.h"
#include "../src/auncient_usd_stl_bridge.h"
#include "../src/auncient_ballet_animator.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

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

    // Create bin directory for output
    system("mkdir -p bin");

    // Open FFmpeg pipe
    FILE *ffmpeg_pipe = popen("ffmpeg -y -f rawvideo -pix_fmt rgb24 -s 1280x720 -r 30 -i - -an -vcodec libx264 -pix_fmt yuv420p bin/teddy_ballet_demo.mp4", "w");
    if (!ffmpeg_pipe) {
        printf("[RENDER ERROR] Failed to open FFmpeg pipeline.\n");
        auncient_stl_free_mesh(&head_mesh);
        auncient_stl_free_mesh(&joint_mesh);
        return 1;
    }

    uint8_t *rgb_out = (uint8_t *)malloc(WIDTH * HEIGHT * 3);
    if (!rgb_out) {
        pclose(ffmpeg_pipe);
        auncient_stl_free_mesh(&head_mesh);
        auncient_stl_free_mesh(&joint_mesh);
        return 1;
    }

    // 3. Render Loop
    printf("[RENDER] Compiling %d pose frames into bin/teddy_ballet_demo.mp4...\n", FRAMES);
    for (int f = 0; f < FRAMES; f++) {
        float t = (float)f / 30.0f;

        // Step pose and Verlet decay states (Rule 10)
        auncient_ballet_step_pose(&bear, t);

        // Clear canvas to dark blue
        memset(rgb_out, 0x10, WIDTH * HEIGHT * 3);

        // Rasterize meshes
        float theta = bear.joint_angle_hip;
        float cos_t = cosf(theta);
        float sin_t = sinf(theta);
        float stretch = bear.verlet_charge_decay[0];

        AuncientStlMesh meshes[2] = { head_mesh, joint_mesh };
        for (int m = 0; m < 2; m++) {
            for (uint32_t i = 0; i < meshes[m].facet_count; i++) {
                for (int v = 0; v < 3; v++) {
                    float x = meshes[m].facets[i].vertices[v][0];
                    float y = meshes[m].facets[i].vertices[v][1];
                    float z = meshes[m].facets[i].vertices[v][2];

                    // 1. Joint Rotation (XZ plane transformation)
                    float rx = x * cos_t - z * sin_t;
                    float rz = x * sin_t + z * cos_t;

                    // 2. Verlet stretch (Rule 10)
                    if (rz < 0.0f) {
                        rz *= stretch;
                    }

                    // Map 3D to 2D screen coordinates (centered at screen center)
                    int sx = (int)(rx * 300.0f) + WIDTH / 2;
                    int sy = (int)(y * 300.0f) + HEIGHT / 2;

                    if (sx >= 0 && sx < WIDTH && sy >= 0 && sy < HEIGHT) {
                        int idx = (sy * WIDTH + sx) * 3;
                        // Bright green points
                        rgb_out[idx + 0] = 0x39;
                        rgb_out[idx + 1] = 0xFF;
                        rgb_out[idx + 2] = 0x14;
                    }
                }
            }
        }

        fwrite(rgb_out, 1, WIDTH * HEIGHT * 3, ffmpeg_pipe);
    }

    pclose(ffmpeg_pipe);
    free(rgb_out);

    auncient_stl_free_mesh(&head_mesh);
    auncient_stl_free_mesh(&joint_mesh);

    printf("[SUCCESS] Ballet animation successfully rendered to bin/teddy_ballet_demo.mp4\n");
    return 0;
}
