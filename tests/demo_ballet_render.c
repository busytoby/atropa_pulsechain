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

    // Create bin and frames directory
    system("mkdir -p bin");
    system("mkdir -p tests/frames");

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
        float stretch = bear.verlet_charge_decay[0];

        // Draw perspective ballet stage floor grid
        for (int d = -4; d <= 4; d++) {
            int bx = WIDTH / 2 + d * 60;
            int by = HEIGHT / 2 + 50;
            int fx = WIDTH / 2 + d * 220;
            int fy = HEIGHT - 40;
            draw_line(rgb_out, bx, by, fx, fy, 0x22, 0x22, 0x55);
        }
        draw_line(rgb_out, WIDTH / 2 - 240, HEIGHT / 2 + 50, WIDTH / 2 + 240, HEIGHT / 2 + 50, 0x22, 0x22, 0x55);
        draw_line(rgb_out, WIDTH / 2 - 880, HEIGHT - 40, WIDTH / 2 + 880, HEIGHT - 40, 0x22, 0x22, 0x55);

        // Rasterize meshes with wireframe lines representing full joint assembly
        AuncientStlMesh segment_meshes[6] = { joint_mesh, head_mesh, joint_mesh, joint_mesh, joint_mesh, joint_mesh };
        const char *segment_names[6] = { "Torso", "Head", "LeftLeg", "RightLeg", "LeftArm", "RightArm" };

        // Set distinct joint rotations over time
        float head_tilt = -0.3f * cosf(t * 3.0f);
        float l_leg_kick = 1.0f * sinf(t * 3.0f);
        float r_leg_kick = -0.8f * sinf(t * 2.5f);
        float l_arm_wave = 0.8f * cosf(t * 3.5f);
        float r_arm_wave = -0.8f * cosf(t * 3.5f);

        for (int m = 0; m < 6; m++) {
            float joint_theta = 0.0f;
            if (m == 1) joint_theta = head_tilt;
            else if (m == 2) joint_theta = l_leg_kick;
            else if (m == 3) joint_theta = r_leg_kick;
            else if (m == 4) joint_theta = l_arm_wave;
            else if (m == 5) joint_theta = r_arm_wave;

            float cos_j = cosf(joint_theta);
            float sin_j = sinf(joint_theta);

            for (uint32_t i = 0; i < segment_meshes[m].facet_count; i++) {
                int sx[3], sy[3];
                for (int v = 0; v < 3; v++) {
                    float x = segment_meshes[m].facets[i].vertices[v][0];
                    float y = segment_meshes[m].facets[i].vertices[v][1];
                    float z = segment_meshes[m].facets[i].vertices[v][2];

                    float rx = x, ry = y, rz = z;

                    if (m == 0) {
                        // Torso Segment: root core coordinate bounds
                        ry = y * 1.2f;
                    } else if (m == 1) {
                        // Head Segment: neck joint sway, offset upwards
                        rx = x * cos_j - z * sin_j;
                        rz = x * sin_j + z * cos_j;
                        ry = y + 0.8f;
                    } else if (m == 2) {
                        // Left Leg: left hip joint kick, offset left/downwards
                        rx = (x - 0.3f) * cos_j - z * sin_j - 0.3f;
                        rz = (x - 0.3f) * sin_j + z * cos_j;
                        ry = y - 0.8f;
                    } else if (m == 3) {
                        // Right Leg: right hip joint kick, offset right/downwards
                        rx = (x + 0.3f) * cos_j - z * sin_j + 0.3f;
                        rz = (x + 0.3f) * sin_j + z * cos_j;
                        ry = y - 0.8f;
                    } else if (m == 4) {
                        // Left Arm: left shoulder joint sway, offset left/upwards
                        rx = (x - 0.4f) * cos_j - z * sin_j - 0.5f;
                        rz = (x - 0.4f) * sin_j + z * cos_j;
                        ry = y + 0.3f;
                    } else if (m == 5) {
                        // Right Arm: right shoulder joint sway, offset right/upwards
                        rx = (x + 0.4f) * cos_j - z * sin_j + 0.5f;
                        rz = (x + 0.4f) * sin_j + z * cos_j;
                        ry = y + 0.3f;
                    }

                    // Verlet stretch (Rule 10)
                    if (rz < 0.0f) {
                        rz *= stretch;
                    }

                    // Map to screen coordinates (centered at screen center with sway and leap offsets)
                    float sway = 200.0f * sinf(t * 1.5f);
                    float norm_t = fmodf(t, 1.0f);
                    float leap_y = 150.0f * (1.0f - 4.0f * (norm_t - 0.5f) * (norm_t - 0.5f));
                    if (leap_y < 0.0f) leap_y = 0.0f;

                    sx[v] = (int)(rx * 250.0f) + WIDTH / 2 + (int)sway;
                    sy[v] = (int)(ry * 250.0f) + HEIGHT / 2 - 100 - (int)leap_y;
                }

                // Draw wireframe links representing Verlet FET network (Rule 10)
                draw_line(rgb_out, sx[0], sy[0], sx[1], sy[1], 0x39, 0xFF, 0x14);
                draw_line(rgb_out, sx[1], sy[1], sx[2], sy[2], 0x39, 0xFF, 0x14);
                draw_line(rgb_out, sx[2], sy[2], sx[0], sy[0], 0x39, 0xFF, 0x14);
            }
        }

        // Export current pose frame segments to USDA files for text telemetry verification
        AuncientStlMesh meshes_posed[6];
        for (int m = 0; m < 6; m++) {
            meshes_posed[m].facet_count = segment_meshes[m].facet_count;
            meshes_posed[m].facets = (AuncientStlFacet *)malloc(sizeof(AuncientStlFacet) * meshes_posed[m].facet_count);
            if (meshes_posed[m].facets) {
                float joint_theta = 0.0f;
                if (m == 1) joint_theta = head_tilt;
                else if (m == 2) joint_theta = l_leg_kick;
                else if (m == 3) joint_theta = r_leg_kick;
                else if (m == 4) joint_theta = l_arm_wave;
                else if (m == 5) joint_theta = r_arm_wave;

                float cos_j = cosf(joint_theta);
                float sin_j = sinf(joint_theta);

                for (uint32_t i = 0; i < segment_meshes[m].facet_count; i++) {
                    for (int v = 0; v < 3; v++) {
                        float x = segment_meshes[m].facets[i].vertices[v][0];
                        float y = segment_meshes[m].facets[i].vertices[v][1];
                        float z = segment_meshes[m].facets[i].vertices[v][2];

                        float rx = x, ry = y, rz = z;

                        if (m == 0) {
                            ry = y * 1.2f;
                        } else if (m == 1) {
                            rx = x * cos_j - z * sin_j;
                            rz = x * sin_j + z * cos_j;
                            ry = y + 0.8f;
                        } else if (m == 2) {
                            rx = (x - 0.3f) * cos_j - z * sin_j - 0.3f;
                            rz = (x - 0.3f) * sin_j + z * cos_j;
                            ry = y - 0.8f;
                        } else if (m == 3) {
                            rx = (x + 0.3f) * cos_j - z * sin_j + 0.3f;
                            rz = (x + 0.3f) * sin_j + z * cos_j;
                            ry = y - 0.8f;
                        } else if (m == 4) {
                            rx = (x - 0.4f) * cos_j - z * sin_j - 0.5f;
                            rz = (x - 0.4f) * sin_j + z * cos_j;
                            ry = y + 0.3f;
                        } else if (m == 5) {
                            rx = (x + 0.4f) * cos_j - z * sin_j + 0.5f;
                            rz = (x + 0.4f) * sin_j + z * cos_j;
                            ry = y + 0.3f;
                        }

                        if (rz < 0.0f) {
                            rz *= stretch;
                        }

                        meshes_posed[m].facets[i].vertices[v][0] = rx;
                        meshes_posed[m].facets[i].vertices[v][1] = ry;
                        meshes_posed[m].facets[i].vertices[v][2] = rz;
                        meshes_posed[m].facets[i].normal[0] = segment_meshes[m].facets[i].normal[0];
                        meshes_posed[m].facets[i].normal[1] = segment_meshes[m].facets[i].normal[1];
                        meshes_posed[m].facets[i].normal[2] = segment_meshes[m].facets[i].normal[2];
                    }
                }
            }
        }

        char frame_path[64];
        sprintf(frame_path, "tests/frames/pose_%04d.usda", f);
        auncient_bridge_multi_stl_to_usda(meshes_posed, segment_names, 6, frame_path);
        
        for (int m = 0; m < 6; m++) {
            if (meshes_posed[m].facets) free(meshes_posed[m].facets);
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
