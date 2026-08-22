#define _POSIX_C_SOURCE 200809L
#include "../tsfi2-deepseek/src/auncient_stl_loader.h"
#include "../tsfi2-deepseek/src/auncient_usd_stl_bridge.h"
#include "../tsfi2-deepseek/src/auncient_ballet_animator.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define WIDTH 1280
#define HEIGHT 720
#define FRAMES 90

typedef struct {
    int sx[3], sy[3];
    float depth;
    uint8_t r, g, b;
} TransformedTriangle;

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

static void fill_triangle(uint8_t *canvas, int x0, int y0, int x1, int y1, int x2, int y2, uint8_t r, uint8_t g, uint8_t b) {
    if (y0 > y1) { int t = x0; x0 = x1; x1 = t; t = y0; y0 = y1; y1 = t; }
    if (y1 > y2) { int t = x1; x1 = x2; x2 = t; t = y1; y1 = y2; y2 = t; }
    if (y0 > y1) { int t = x0; x0 = x1; x1 = t; t = y0; y0 = y1; y1 = t; }
    if (y2 == y0) return;

    for (int y = y0; y <= y2; y++) {
        if (y < 0 || y >= HEIGHT) continue;
        float alpha = (float)(y - y0) / (y2 - y0);
        float beta = (y <= y1 && y1 != y0) ? (float)(y - y0) / (y1 - y0) : (float)(y - y1) / (y2 - y1);
        int xa = x0 + (int)(alpha * (x2 - x0));
        int xb = (y <= y1 && y1 != y0) ? x0 + (int)(beta * (x1 - x0)) : x1 + (int)(beta * (x2 - x1));
        if (xa > xb) { int t = xa; xa = xb; xb = t; }
        for (int x = xa; x <= xb; x++) {
            if (x >= 0 && x < WIDTH) {
                int idx = (y * WIDTH + x) * 3;
                canvas[idx + 0] = r;
                canvas[idx + 1] = g;
                canvas[idx + 2] = b;
            }
        }
    }
}

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT BALLET VIDEO RENDER PIPELINE\n");
    printf("=============================================================\n");

    // 1. Load component meshes
    AuncientStlMesh head_mesh = {0};
    AuncientStlMesh joint_mesh = {0};
    AuncientStlMesh torso_mesh = {0};
    
    if (!auncient_stl_load("tsfi2-deepseek/assets/toy_bear_head.stl", &head_mesh) ||
        !auncient_stl_load("tsfi2-deepseek/assets/toy_bear_joint.stl", &joint_mesh) ||
        !auncient_stl_load("tsfi2-deepseek/assets/toy_bear_torso.stl", &torso_mesh)) {
        printf("[RENDER ERROR] Failed to load STL component assets.\n");
        auncient_stl_free_mesh(&head_mesh);
        auncient_stl_free_mesh(&joint_mesh);
        auncient_stl_free_mesh(&torso_mesh);
        return 1;
    }

    // 2. Register ballet bear participant (Rule 16)
    AuncientBalletBear bear = {0};
    if (!auncient_ballet_register_bear("Ballet_Bear_01", "BLOCK_SEED_7777", &bear)) {
        auncient_stl_free_mesh(&head_mesh);
        auncient_stl_free_mesh(&joint_mesh);
        auncient_stl_free_mesh(&torso_mesh);
        return 1;
    }

    // Create bin and frames directory
    system("mkdir -p bin");
    system("mkdir -p tests/frames");

    // Open FFmpeg pipe
    FILE *ffmpeg_pipe = popen("ffmpeg -y -f rawvideo -pix_fmt rgb24 -s 1280x720 -r 30 -i - -an -vcodec libx264 -pix_fmt yuv420p bin/teddy_bear_ballet_demo.mp4", "w");
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
    printf("[RENDER] Compiling %d pose frames into bin/teddy_bear_ballet_demo.mp4...\n", FRAMES);
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

        // Rasterize meshes with wireframe lines representing full joint assembly (9 segments for recognizable teddy bear)
        AuncientStlMesh segment_meshes[9] = { torso_mesh, head_mesh, joint_mesh, joint_mesh, joint_mesh, joint_mesh, head_mesh, head_mesh, joint_mesh };
        const char *segment_names[9] = { "Torso", "Head", "LeftLeg", "RightLeg", "LeftArm", "RightArm", "LeftEar", "RightEar", "Snout" };

        // Set distinct joint rotations over time (contralateral walking & arabesque split leaps)
        float norm_t = fmodf(t, 1.0f);
        float leap_val = 150.0f * (1.0f - 4.0f * (norm_t - 0.5f) * (norm_t - 0.5f));
        if (leap_val < 0.0f) leap_val = 0.0f;

        float head_tilt = -0.2f * cosf(t * 5.0f);
        float l_leg_kick = 0.7f * sinf(t * 5.0f);
        float r_leg_kick = -0.7f * sinf(t * 5.0f);
        float l_arm_wave = -0.5f * sinf(t * 5.0f);
        float r_arm_wave = 0.5f * sinf(t * 5.0f);

        // Arabesque grand jete leap posing extension
        if (leap_val > 40.0f) {
            l_leg_kick = 0.9f;
            r_leg_kick = -1.1f;
            l_arm_wave = -0.7f;
            r_arm_wave = 0.7f;
            head_tilt = -0.4f;
        }

        int total_triangles = 0;
        for (int m = 0; m < 9; m++) {
            total_triangles += segment_meshes[m].facet_count;
        }

        TransformedTriangle *tri_list = (TransformedTriangle *)malloc(sizeof(TransformedTriangle) * total_triangles);
        int tri_idx = 0;
        float stretch = bear.verlet_charge_decay[0];

        for (int m = 0; m < 9; m++) {
            float joint_theta = 0.0f;
            if (m == 1 || m == 6 || m == 7 || m == 8) joint_theta = head_tilt;
            else if (m == 2) joint_theta = l_leg_kick;
            else if (m == 3) joint_theta = r_leg_kick;
            else if (m == 4) joint_theta = l_arm_wave;
            else if (m == 5) joint_theta = r_arm_wave;

            float cos_j = cosf(joint_theta);
            float sin_j = sinf(joint_theta);

            // Shaded colors representing stuffed teddy bear parts
            uint8_t r = 0x8B, g = 0x5A, b = 0x2B; // Stuffed warm brown
            if (m == 6 || m == 7) { r = 0xCD; g = 0x9B; b = 0x1D; } // Golden ears
            if (m == 8) { r = 0xEE; g = 0xC5; b = 0x91; } // Tan muzzle snout

            for (uint32_t i = 0; i < segment_meshes[m].facet_count; i++) {
                float sum_z = 0.0f;
                int sx[3], sy[3];
                for (int v = 0; v < 3; v++) {
                    float x = segment_meshes[m].facets[i].vertices[v][0];
                    float y = segment_meshes[m].facets[i].vertices[v][1];
                    float z = segment_meshes[m].facets[i].vertices[v][2];

                    float rx = x, ry = y, rz = z;

                    if (m == 0) {
                        rx = x * 1.3f;
                        ry = y * 1.2f;
                        rz = z * 1.3f;
                    } else if (m == 1) {
                        rx = (x * 0.8f) * cos_j - (z * 0.8f) * sin_j;
                        rz = (x * 0.8f) * sin_j + (z * 0.8f) * cos_j;
                        ry = y * 0.8f + 0.9f;
                    } else if (m == 2) {
                        // Left Leg: YZ pitch swing pivoting at hip (-0.3, -0.6)
                        float lx = x * 0.4f;
                        float ly = y * 0.8f;
                        float lz = z * 0.4f;
                        float local_y = ly - 0.4f;
                        rx = lx - 0.3f;
                        ry = local_y * cos_j - lz * sin_j - 0.6f;
                        rz = local_y * sin_j + lz * cos_j;
                    } else if (m == 3) {
                        // Right Leg: YZ pitch swing pivoting at hip (+0.3, -0.6)
                        float lx = x * 0.4f;
                        float ly = y * 0.8f;
                        float lz = z * 0.4f;
                        float local_y = ly - 0.4f;
                        rx = lx + 0.3f;
                        ry = local_y * cos_j - lz * sin_j - 0.6f;
                        rz = local_y * sin_j + lz * cos_j;
                    } else if (m == 4) {
                        // Left Arm: YZ pitch swing pivoting at shoulder (-0.35, +0.4)
                        float ax = x * 0.3f;
                        float ay = y * 0.7f;
                        float az = z * 0.3f;
                        float local_y = ay - 0.35f;
                        rx = ax - 0.35f;
                        ry = local_y * cos_j - az * sin_j + 0.4f;
                        rz = local_y * sin_j + az * cos_j;
                    } else if (m == 5) {
                        // Right Arm: YZ pitch swing pivoting at shoulder (+0.35, +0.4)
                        float ax = x * 0.3f;
                        float ay = y * 0.7f;
                        float az = z * 0.3f;
                        float local_y = ay - 0.35f;
                        rx = ax + 0.35f;
                        ry = local_y * cos_j - az * sin_j + 0.4f;
                        rz = local_y * sin_j + az * cos_j;
                    } else if (m == 6) {
                        float ex = x * 0.25f - 0.3f;
                        float ey = y * 0.25f + 0.3f;
                        float ez = z * 0.25f;
                        rx = ex * cos_j - ez * sin_j;
                        rz = ex * sin_j + ez * cos_j;
                        ry = ey + 0.9f;
                    } else if (m == 7) {
                        float ex = x * 0.25f + 0.3f;
                        float ey = y * 0.25f + 0.3f;
                        float ez = z * 0.25f;
                        rx = ex * cos_j - ez * sin_j;
                        rz = ex * sin_j + ez * cos_j;
                        ry = ey + 0.9f;
                    } else if (m == 8) {
                        float sx_pt = x * 0.25f;
                        float sy_pt = y * 0.2f;
                        float sz_pt = z * 0.25f + 0.3f;
                        rx = sx_pt * cos_j - sz_pt * sin_j;
                        rz = sx_pt * sin_j + sz_pt * cos_j;
                        ry = sy_pt + 0.9f;
                    }

                    if (rz < 0.0f) {
                        rz *= stretch;
                    }

                    sum_z += rz;

                    float sway = 200.0f * sinf(t * 1.5f);
                    float norm_t = fmodf(t, 1.0f);
                    float leap_y = 150.0f * (1.0f - 4.0f * (norm_t - 0.5f) * (norm_t - 0.5f));
                    if (leap_y < 0.0f) leap_y = 0.0f;

                    // Apply 3D Camera Yaw/Pitch (isometric view to render Z-axis kicks laterally)
                    float yaw = 0.5f;
                    float pitch = 0.2f;
                    float cx_val = rx * cosf(yaw) - rz * sinf(yaw);
                    float cz_val = rx * sinf(yaw) + rz * cosf(yaw);
                    float cy_val = ry * cosf(pitch) - cz_val * sinf(pitch);

                    sx[v] = (int)(cx_val * 250.0f) + WIDTH / 2 + (int)sway;
                    sy[v] = HEIGHT / 2 + 50 - (int)(cy_val * 250.0f) - (int)leap_y;
                }

                if (tri_list) {
                    tri_list[tri_idx].sx[0] = sx[0];
                    tri_list[tri_idx].sx[1] = sx[1];
                    tri_list[tri_idx].sx[2] = sx[2];
                    tri_list[tri_idx].sy[0] = sy[0];
                    tri_list[tri_idx].sy[1] = sy[1];
                    tri_list[tri_idx].sy[2] = sy[2];
                    tri_list[tri_idx].depth = sum_z / 3.0f;
                    tri_list[tri_idx].r = r;
                    tri_list[tri_idx].g = g;
                    tri_list[tri_idx].b = b;
                    tri_idx++;
                }
            }
        }

        // Painter's Depth Sorting (back-to-front rendering)
        if (tri_list) {
            for (int i = 0; i < total_triangles - 1; i++) {
                for (int j = 0; j < total_triangles - i - 1; j++) {
                    if (tri_list[j].depth > tri_list[j + 1].depth) {
                        TransformedTriangle tmp = tri_list[j];
                        tri_list[j] = tri_list[j + 1];
                        tri_list[j + 1] = tmp;
                    }
                }
            }

            // Fill solid polygons and outline boundaries
            for (int i = 0; i < total_triangles; i++) {
                fill_triangle(rgb_out, tri_list[i].sx[0], tri_list[i].sy[0],
                                      tri_list[i].sx[1], tri_list[i].sy[1],
                                      tri_list[i].sx[2], tri_list[i].sy[2],
                                      tri_list[i].r, tri_list[i].g, tri_list[i].b);
                draw_line(rgb_out, tri_list[i].sx[0], tri_list[i].sy[0], tri_list[i].sx[1], tri_list[i].sy[1], 0x30, 0x10, 0x05);
                draw_line(rgb_out, tri_list[i].sx[1], tri_list[i].sy[1], tri_list[i].sx[2], tri_list[i].sy[2], 0x30, 0x10, 0x05);
                draw_line(rgb_out, tri_list[i].sx[2], tri_list[i].sy[2], tri_list[i].sx[0], tri_list[i].sy[0], 0x30, 0x10, 0x05);
            }
            free(tri_list);
        }

        // Export current pose frame segments to USDA files for text telemetry verification
        AuncientStlMesh meshes_posed[9];
        for (int m = 0; m < 9; m++) {
            meshes_posed[m].facet_count = segment_meshes[m].facet_count;
            meshes_posed[m].facets = (AuncientStlFacet *)malloc(sizeof(AuncientStlFacet) * meshes_posed[m].facet_count);
            if (meshes_posed[m].facets) {
                float joint_theta = 0.0f;
                if (m == 1 || m == 6 || m == 7 || m == 8) joint_theta = head_tilt;
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
                            rx = x * 1.3f;
                            ry = y * 1.2f;
                            rz = z * 1.3f;
                        } else if (m == 1) {
                            rx = (x * 0.8f) * cos_j - (z * 0.8f) * sin_j;
                            rz = (x * 0.8f) * sin_j + (z * 0.8f) * cos_j;
                            ry = y * 0.8f + 0.9f;
                        } else if (m == 2) {
                            float lx = x * 0.4f;
                            float ly = y * 0.8f;
                            float lz = z * 0.4f;
                            float local_y = ly - 0.4f;
                            rx = lx - 0.3f;
                            ry = local_y * cos_j - lz * sin_j - 0.6f;
                            rz = local_y * sin_j + lz * cos_j;
                        } else if (m == 3) {
                            float lx = x * 0.4f;
                            float ly = y * 0.8f;
                            float lz = z * 0.4f;
                            float local_y = ly - 0.4f;
                            rx = lx + 0.3f;
                            ry = local_y * cos_j - lz * sin_j - 0.6f;
                            rz = local_y * sin_j + lz * cos_j;
                        } else if (m == 4) {
                            float ax = x * 0.3f;
                            float ay = y * 0.7f;
                            float az = z * 0.3f;
                            float local_y = ay - 0.35f;
                            rx = ax - 0.35f;
                            ry = local_y * cos_j - az * sin_j + 0.4f;
                            rz = local_y * sin_j + az * cos_j;
                        } else if (m == 5) {
                            float ax = x * 0.3f;
                            float ay = y * 0.7f;
                            float az = z * 0.3f;
                            float local_y = ay - 0.35f;
                            rx = ax + 0.35f;
                            ry = local_y * cos_j - az * sin_j + 0.4f;
                            rz = local_y * sin_j + az * cos_j;
                        } else if (m == 6) {
                            float ex = x * 0.25f - 0.3f;
                            float ey = y * 0.25f + 0.3f;
                            float ez = z * 0.25f;
                            rx = ex * cos_j - ez * sin_j;
                            rz = ex * sin_j + ez * cos_j;
                            ry = ey + 0.9f;
                        } else if (m == 7) {
                            float ex = x * 0.25f + 0.3f;
                            float ey = y * 0.25f + 0.3f;
                            float ez = z * 0.25f;
                            rx = ex * cos_j - ez * sin_j;
                            rz = ex * sin_j + ez * cos_j;
                            ry = ey + 0.9f;
                        } else if (m == 8) {
                            float sx_pt = x * 0.25f;
                            float sy_pt = y * 0.2f;
                            float sz_pt = z * 0.25f + 0.3f;
                            rx = sx_pt * cos_j - sz_pt * sin_j;
                            rz = sx_pt * sin_j + sz_pt * cos_j;
                            ry = sy_pt + 0.9f;
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
        auncient_bridge_multi_stl_to_usda(meshes_posed, segment_names, 9, frame_path);
        
        for (int m = 0; m < 9; m++) {
            if (meshes_posed[m].facets) free(meshes_posed[m].facets);
        }

        fwrite(rgb_out, 1, WIDTH * HEIGHT * 3, ffmpeg_pipe);
    }

    pclose(ffmpeg_pipe);
    free(rgb_out);

    auncient_stl_free_mesh(&head_mesh);
    auncient_stl_free_mesh(&joint_mesh);
    auncient_stl_free_mesh(&torso_mesh);

    printf("[SUCCESS] Ballet animation successfully rendered to bin/teddy_bear_ballet_demo.mp4\n");
    return 0;
}
