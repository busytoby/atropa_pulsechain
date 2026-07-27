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
#define FRAMES 90

static void draw_line(uint8_t *canvas, int x0, int y0, int x1, int y1, uint8_t r, uint8_t g, uint8_t b) {
    int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int err = dx + dy, e2;

    while (1) {
        if (x0 >= 0 && x0 < WIDTH && y0 >= 0 && y0 < HEIGHT) {
            int idx = (y0 * WIDTH + x0) * 3;
            canvas[idx + 0] = r;
            canvas[idx + 1] = g;
            canvas[idx + 2] = b;
        }
        if (x0 == x1 && y0 == y1) break;
        e2 = 2 * err;
        if (e2 >= dy) { err += dy; x0 += sx; }
        if (e2 <= dx) { err += dx; y0 += sy; }
    }
}

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
        memset(rgb_out, 0x05, WIDTH * HEIGHT * 3);

        // Draw oscilloscope background grids
        for (int y = 0; y < HEIGHT; y += 40) {
            draw_line(rgb_out, 0, y, WIDTH - 1, y, 0x11, 0x22, 0x11);
        }
        for (int x = 0; x < WIDTH; x += 40) {
            draw_line(rgb_out, x, 0, x, HEIGHT - 1, 0x11, 0x22, 0x11);
        }

        // Draw primary diagnostic axis split lines
        draw_line(rgb_out, WIDTH / 2, 0, WIDTH / 2, HEIGHT - 1, 0x88, 0x66, 0x33);
        draw_line(rgb_out, 0, HEIGHT / 2, WIDTH - 1, HEIGHT / 2, 0x88, 0x66, 0x33);

        // Rasterize meshes with wireframe lines
        float theta = bear.joint_angle_hip;
        float cos_t = cosf(theta);
        float sin_t = sinf(theta);
        float stretch = bear.verlet_charge_decay[0];

        AuncientStlMesh meshes[2] = { head_mesh, joint_mesh };
        for (int m = 0; m < 2; m++) {
            for (uint32_t i = 0; i < meshes[m].facet_count; i++) {
                int sx[3], sy[3];
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

                    // Map to screen coordinates (centered at screen center with sway offset)
                    float sway = 200.0f * sinf(t * 1.5f);
                    sx[v] = (int)(rx * 300.0f) + WIDTH / 2 + (int)sway;
                    sy[v] = (int)(y * 300.0f) + HEIGHT / 2;
                }

                // Draw wireframe links representing Verlet FET network (Rule 10)
                draw_line(rgb_out, sx[0], sy[0], sx[1], sy[1], 0x39, 0xFF, 0x14);
                draw_line(rgb_out, sx[1], sy[1], sx[2], sy[2], 0x39, 0xFF, 0x14);
                draw_line(rgb_out, sx[2], sy[2], sx[0], sy[0], 0x39, 0xFF, 0x14);
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
