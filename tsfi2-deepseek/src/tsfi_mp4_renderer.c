#define _GNU_SOURCE
#include "tsfi_mp4_renderer.h"
#include "auncient_harvard_computation_lab.h"
#include "tsfi_displacementshader.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Verified LAU Token Contract Addresses
static const char *LAU_TOKENS[5] = {
    "0xAD4e198623A5E2723e19E4D4a6ECF72B1D19FE4B",
    "0xD07B9f3DF4E9634EbAa0CBF079816925b2C474Ce",
    "0xd32c39fee49391c7952d1b30b15921b0d3b42e69",
    "0xed343c0f99c89ed7c3c934a88f90261fd6a9a68b",
    "0x3e10ed242ecb3951151e7a07e0a8f43d4f150c0e"
};

static void compute_19d_projection(const char *address, float t, float *out_coords, int dim_count) {
    if (!address || !out_coords || dim_count <= 0) return;

    size_t len = strlen(address);
    for (int d = 0; d < dim_count; d++) {
        uint8_t byte_val = (uint8_t)address[(d * 2) % len];
        float weight = (float)byte_val / 255.0f;
        out_coords[d] = sinf(t * (1.0f + (float)d * 0.08f) + weight * (float)M_PI) * (0.4f + weight * 0.6f);
    }
}

void tsfi_mp4_pipeline_init(TsfiMp4Pipeline *pipe, const char *audio_wav, const char *output_mp4) {
    tsfi_mp4_pipeline_init_custom(pipe, (float)MP4_DEFAULT_SECONDS, audio_wav, output_mp4);
}

void tsfi_mp4_pipeline_init_custom(TsfiMp4Pipeline *pipe, float duration_seconds, const char *audio_wav, const char *output_mp4) {
    if (!pipe) return;
    memset(pipe, 0, sizeof(TsfiMp4Pipeline));
    float dur = (duration_seconds > 0.0f) ? duration_seconds : (float)MP4_DEFAULT_SECONDS;
    pipe->duration_seconds = dur;
    pipe->total_frames = (int)(dur * (float)MP4_FPS);
    if (audio_wav) strncpy(pipe->audio_wav_path, audio_wav, sizeof(pipe->audio_wav_path) - 1);
    if (output_mp4) strncpy(pipe->output_mp4_path, output_mp4, sizeof(pipe->output_mp4_path) - 1);
}

// -----------------------------------------------------------------------------
// 3D Perspective Projection Function
// -----------------------------------------------------------------------------
static inline void project_3d_point(float x, float y, float z,
                                    float rot_x, float rot_y, float cam_z,
                                    int w, int h,
                                    int *out_sx, int *out_sy, float *out_depth) {
    float rx1 = x * cosf(rot_y) - z * sinf(rot_y);
    float rz1 = x * sinf(rot_y) + z * cosf(rot_y);

    float ry2 = y * cosf(rot_x) - rz1 * sinf(rot_x);
    float rz2 = y * sinf(rot_x) + rz1 * cosf(rot_x) + cam_z;

    if (rz2 < 10.0f) rz2 = 10.0f;
    *out_depth = rz2;

    float fov = 1100.0f;
    float aspect = (float)w / (float)h;

    *out_sx = (int)((float)w * 0.5f + (rx1 * fov) / (rz2 * aspect));
    *out_sy = (int)((float)h * 0.5f + (ry2 * fov) / rz2);
}

// -----------------------------------------------------------------------------
// 3D Cylindrical Volumetric Wire with Normal Shading & Z-Buffering
// -----------------------------------------------------------------------------
static void draw_3d_volumetric_wire(uint32_t *pixels, float *zbuf, int w, int h,
                                    int x0, int y0, float z0,
                                    int x1, int y1, float z1,
                                    uint32_t base_color, float radius_3d,
                                    float lx, float ly, float lz) {
    int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int err = dx + dy, e2;

    float total_dist = sqrtf((float)(dx * dx + dy * dy));
    if (total_dist < 1.0f) total_dist = 1.0f;

    float seg_dx = (float)(x1 - x0) / total_dist;
    float seg_dy = (float)(y1 - y0) / total_dist;
    float seg_nx = -seg_dy;
    float seg_ny = seg_dx;

    int cur_x = x0, cur_y = y0;

    while (1) {
        float step_d = sqrtf((float)((cur_x - x0)*(cur_x - x0) + (cur_y - y0)*(cur_y - y0)));
        float frac = step_d / total_dist;
        if (frac > 1.0f) frac = 1.0f;
        float cur_z = z0 * (1.0f - frac) + z1 * frac;

        int rad_px = (int)((radius_3d * 900.0f) / cur_z);
        if (rad_px < 1) rad_px = 1;
        if (rad_px > 16) rad_px = 16;

        for (int r = -rad_px; r <= rad_px; r++) {
            int px = cur_x + (int)(seg_nx * (float)r);
            int py = cur_y + (int)(seg_ny * (float)r);

            if (px < 0 || px >= w || py < 0 || py >= h) continue;

            int p_idx = py * w + px;
            if (cur_z < zbuf[p_idx]) {
                zbuf[p_idx] = cur_z;

                float cross_ratio = (float)r / (float)rad_px;
                float nz_norm = sqrtf(fmaxf(0.0f, 1.0f - cross_ratio * cross_ratio));
                float nx_norm = seg_nx * cross_ratio;
                float ny_norm = seg_ny * cross_ratio;

                float n_dot_l = nx_norm * lx + ny_norm * ly + nz_norm * lz;
                if (n_dot_l < 0.12f) n_dot_l = 0.12f;

                float hx = lx, hy = ly, hz = lz + 1.0f;
                float inv_h = 1.0f / sqrtf(hx*hx + hy*hy + hz*hz);
                hx *= inv_h; hy *= inv_h; hz *= inv_h;
                float n_dot_h = fmaxf(0.0f, nx_norm * hx + ny_norm * hy + nz_norm * hz);
                float spec = powf(n_dot_h, 32.0f) * 0.70f;

                uint8_t a = (uint8_t)((base_color >> 24) & 0xFF);
                float r_val = ((base_color >> 16) & 0xFF) * n_dot_l + spec * 255.0f;
                float g_val = ((base_color >> 8) & 0xFF) * n_dot_l + spec * 255.0f;
                float b_val = (base_color & 0xFF) * n_dot_l + spec * 255.0f;

                if (r_val > 255.0f) r_val = 255.0f;
                if (g_val > 255.0f) g_val = 255.0f;
                if (b_val > 255.0f) b_val = 255.0f;

                pixels[p_idx] = (a << 24) | ((uint8_t)r_val << 16) | ((uint8_t)g_val << 8) | (uint8_t)b_val;
            }
        }

        if (cur_x == x1 && cur_y == y1) break;
        e2 = 2 * err;
        if (e2 >= dy) { err += dy; cur_x += sx; }
        if (e2 <= dx) { err += dx; cur_y += sy; }
    }
}

// -----------------------------------------------------------------------------
// 3D Volumetric Sphere Mesh Rasterizer (Pixar USDA Prim)
// -----------------------------------------------------------------------------
static void draw_3d_volumetric_sphere(uint32_t *fb, float *zbuf, int w, int h,
                                      float cx, float cy, float cz, float radius,
                                      float rot_x, float rot_y, float cam_z,
                                      uint32_t base_color, float lx, float ly, float lz) {
    int rings = 12, segs = 16;
    for (int r = 0; r < rings; r++) {
        float phi1 = ((float)r / (float)rings) * (float)M_PI;
        float phi2 = ((float)(r + 1) / (float)rings) * (float)M_PI;
        int p0_x = 0, p0_y = 0, first_x0 = 0, first_y0 = 0;
        float p0_z = 0, first_z0 = 0;

        for (int s = 0; s < segs; s++) {
            float theta = ((float)s / (float)segs) * 2.0f * (float)M_PI;

            float x1 = cx + radius * sinf(phi1) * cosf(theta);
            float y1 = cy + radius * cosf(phi1);
            float z1 = cz + radius * sinf(phi1) * sinf(theta);

            float x2 = cx + radius * sinf(phi2) * cosf(theta);
            float y2 = cy + radius * cosf(phi2);
            float z2 = cz + radius * sinf(phi2) * sinf(theta);

            int sx1 = 0, sy1 = 0, sx2 = 0, sy2 = 0;
            float d1 = 0, d2 = 0;
            project_3d_point(x1, y1, z1, rot_x, rot_y, cam_z, w, h, &sx1, &sy1, &d1);
            project_3d_point(x2, y2, z2, rot_x, rot_y, cam_z, w, h, &sx2, &sy2, &d2);

            if (s == 0) {
                first_x0 = sx1; first_y0 = sy1; first_z0 = d1;
            } else {
                draw_3d_volumetric_wire(fb, zbuf, w, h, p0_x, p0_y, p0_z, sx1, sy1, d1, base_color, 2.0f, lx, ly, lz);
            }
            draw_3d_volumetric_wire(fb, zbuf, w, h, sx1, sy1, d1, sx2, sy2, d2, base_color, 2.0f, lx, ly, lz);

            p0_x = sx1; p0_y = sy1; p0_z = d1;
        }
        draw_3d_volumetric_wire(fb, zbuf, w, h, p0_x, p0_y, p0_z, first_x0, first_y0, first_z0, base_color, 2.0f, lx, ly, lz);
    }
}

// -----------------------------------------------------------------------------
// 3D Volumetric Cylinder Mesh Rasterizer (Pixar USDA Prim)
// -----------------------------------------------------------------------------
static void draw_3d_volumetric_cylinder(uint32_t *fb, float *zbuf, int w, int h,
                                        float cx, float cy, float cz, float radius, float length,
                                        float rot_x, float rot_y, float cam_z,
                                        uint32_t base_color, float lx, float ly, float lz) {
    int segs = 16;
    int p0_x = 0, p0_y = 0, p1_x = 0, p1_y = 0, first_x0 = 0, first_y0 = 0, first_x1 = 0, first_y1 = 0;
    float p0_z = 0, p1_z = 0, first_z0 = 0, first_z1 = 0;

    for (int s = 0; s < segs; s++) {
        float theta = ((float)s / (float)segs) * 2.0f * (float)M_PI;
        float x0 = cx + radius * cosf(theta);
        float y0 = cy - length * 0.5f;
        float z0 = cz + radius * sinf(theta);

        float x1 = cx + radius * cosf(theta);
        float y1 = cy + length * 0.5f;
        float z1 = cz + radius * sinf(theta);

        int sx0 = 0, sy0 = 0, sx1 = 0, sy1 = 0;
        float d0 = 0, d1 = 0;
        project_3d_point(x0, y0, z0, rot_x, rot_y, cam_z, w, h, &sx0, &sy0, &d0);
        project_3d_point(x1, y1, z1, rot_x, rot_y, cam_z, w, h, &sx1, &sy1, &d1);

        if (s == 0) {
            first_x0 = sx0; first_y0 = sy0; first_z0 = d0;
            first_x1 = sx1; first_y1 = sy1; first_z1 = d1;
        } else {
            draw_3d_volumetric_wire(fb, zbuf, w, h, p0_x, p0_y, p0_z, sx0, sy0, d0, base_color, 2.0f, lx, ly, lz);
            draw_3d_volumetric_wire(fb, zbuf, w, h, p1_x, p1_y, p1_z, sx1, sy1, d1, base_color, 2.0f, lx, ly, lz);
        }
        draw_3d_volumetric_wire(fb, zbuf, w, h, sx0, sy0, d0, sx1, sy1, d1, base_color, 2.0f, lx, ly, lz);

        p0_x = sx0; p0_y = sy0; p0_z = d0;
        p1_x = sx1; p1_y = sy1; p1_z = d1;
    }
    draw_3d_volumetric_wire(fb, zbuf, w, h, p0_x, p0_y, p0_z, first_x0, first_y0, first_z0, base_color, 2.0f, lx, ly, lz);
    draw_3d_volumetric_wire(fb, zbuf, w, h, p1_x, p1_y, p1_z, first_x1, first_y1, first_z1, base_color, 2.0f, lx, ly, lz);
}

// -----------------------------------------------------------------------------
// 3D Volumetric Sewn Heart Mesh (Pixar USDA Prim from teddy_bear_sewnheart.usda)
// -----------------------------------------------------------------------------
static void draw_3d_volumetric_sewn_heart(uint32_t *fb, float *zbuf, int w, int h,
                                          float cx, float cy, float cz, float scale,
                                          float rot_x, float rot_y, float cam_z,
                                          float lx, float ly, float lz) {
    int rings = 10, segs = 18;
    uint32_t heart_col = 0xFFFF0033;

    for (int r = 0; r < rings; r++) {
        float v = (float)r / (float)rings;
        float v_ang = v * (float)M_PI;
        int prev_x = 0, prev_y = 0, first_x = 0, first_y = 0;
        float prev_z = 0, first_z = 0;

        for (int s = 0; s < segs; s++) {
            float u = (float)s / (float)segs;
            float t_ang = u * 2.0f * (float)M_PI;

            float hx = 16.0f * powf(sinf(t_ang), 3.0f) * sinf(v_ang);
            float hy = -(13.0f * cosf(t_ang) - 5.0f * cosf(2.0f * t_ang) - 2.0f * cosf(3.0f * t_ang) - cosf(4.0f * t_ang)) * sinf(v_ang);
            float hz = 8.0f * cosf(v_ang);

            float x3d = cx + hx * scale;
            float y3d = cy + hy * scale;
            float z3d = cz + hz * scale;

            int sx = 0, sy = 0; float depth = 0;
            project_3d_point(x3d, y3d, z3d, rot_x, rot_y, cam_z, w, h, &sx, &sy, &depth);

            if (s == 0) {
                first_x = sx; first_y = sy; first_z = depth;
            } else {
                draw_3d_volumetric_wire(fb, zbuf, w, h, prev_x, prev_y, prev_z, sx, sy, depth, heart_col, 2.5f, lx, ly, lz);
            }
            prev_x = sx; prev_y = sy; prev_z = depth;
        }
        draw_3d_volumetric_wire(fb, zbuf, w, h, prev_x, prev_y, prev_z, first_x, first_y, first_z, heart_col, 2.5f, lx, ly, lz);
    }
}

// -----------------------------------------------------------------------------
// 3D Vaesen Character Wireframe Renderer with Full USDA Prims
// -----------------------------------------------------------------------------
static void draw_3d_vaesen_character(uint32_t *fb, float *zbuf, int w, int h, int scene, float t, float cam_yaw, float cam_pitch, float lx, float ly, float lz) {
    // -------------------------------------------------------------------------
    // SCENE 1: 3D USDA Auncient Tomte (Sphere Head + Conical Cloak + 3D Lantern)
    // -------------------------------------------------------------------------

    if (scene == 1) {
        float b_y = sinf(t * 2.0f) * 15.0f;
        float b_rot_y = cam_yaw + 0.35f;
        float b_rot_x = cam_pitch;
        float b_cam_z = 800.0f;

        uint32_t cloak_col = 0xFFC5A059;
        uint32_t beard_col = 0xFFE0E0E0;

        // 3D Head & Beard Spheres (USDA Prims)
        draw_3d_volumetric_sphere(fb, zbuf, w, h, 350.0f, -80.0f + b_y, 450.0f, 32.0f, b_rot_x, b_rot_y, b_cam_z, cloak_col, lx, ly, lz);
        draw_3d_volumetric_sphere(fb, zbuf, w, h, 350.0f, -50.0f + b_y, 420.0f, 22.0f, b_rot_x, b_rot_y, b_cam_z, beard_col, lx, ly, lz);

        // 3D Torso / Cloak Cylinder (USDA Prim)
        draw_3d_volumetric_cylinder(fb, zbuf, w, h, 350.0f, 30.0f + b_y, 450.0f, 40.0f, 120.0f, b_rot_x, b_rot_y, b_cam_z, cloak_col, lx, ly, lz);

        // 3D Lantern Cylinder (USDA Prim)
        draw_3d_volumetric_cylinder(fb, zbuf, w, h, 420.0f, 20.0f + b_y, 400.0f, 14.0f, 32.0f, b_rot_x, b_rot_y, b_cam_z, 0xFFFFD700, lx, ly, lz);

        int l_sx = 0, l_sy = 0; float l_d = 0;
        project_3d_point(420.0f, 20.0f + b_y, 400.0f, b_rot_x, b_rot_y, b_cam_z, w, h, &l_sx, &l_sy, &l_d);

        for (int gy = -45; gy <= 45; gy++) {
            for (int gx = -45; gx <= 45; gx++) {
                int gpx = l_sx + gx, gpy = l_sy + gy;
                if (gpx >= 0 && gpx < w && gpy >= 0 && gpy < h) {
                    float dist = sqrtf((float)(gx*gx + gy*gy));
                    if (dist < 45.0f) {
                        float glow = (1.0f - dist / 45.0f) * 0.40f;
                        uint32_t cur = fb[gpy * w + gpx];
                        uint8_t r = (uint8_t)fminf(255.0f, ((cur >> 16) & 0xFF) + 255.0f * glow);
                        uint8_t g = (uint8_t)fminf(255.0f, ((cur >> 8) & 0xFF) + 220.0f * glow);
                        uint8_t b = (uint8_t)fminf(255.0f, (cur & 0xFF) + 90.0f * glow);
                        fb[gpy * w + gpx] = 0xFF000000 | (r << 16) | (g << 8) | b;
                    }
                }
            }
        }
    }
    // -------------------------------------------------------------------------
    // SCENE 2: 3D USDA Nacken (Water Fiddler Avatar with 3D Violin Body & Bow)
    // -------------------------------------------------------------------------
    else if (scene == 2) {
        float b_rot_y = cam_yaw - 0.35f;
        float b_rot_x = cam_pitch;
        float b_cam_z = 800.0f;

        uint32_t body_col = 0xFF00FF88;
        uint32_t violin_col = 0xFFCD853F; // Amber Wood Violin

        // 3D Head & Torso Spheres (USDA Prims)
        draw_3d_volumetric_sphere(fb, zbuf, w, h, -350.0f, -80.0f, 450.0f, 30.0f, b_rot_x, b_rot_y, b_cam_z, body_col, lx, ly, lz);
        draw_3d_volumetric_cylinder(fb, zbuf, w, h, -350.0f, 20.0f, 450.0f, 28.0f, 100.0f, b_rot_x, b_rot_y, b_cam_z, body_col, lx, ly, lz);

        // 3D Violin Body (USDA Prim)
        draw_3d_volumetric_sphere(fb, zbuf, w, h, -390.0f, -30.0f, 410.0f, 18.0f, b_rot_x, b_rot_y, b_cam_z, violin_col, lx, ly, lz);
        draw_3d_volumetric_cylinder(fb, zbuf, w, h, -390.0f, -55.0f, 410.0f, 4.0f, 35.0f, b_rot_x, b_rot_y, b_cam_z, 0xFF101010, lx, ly, lz);

        int v_sx = 0, v_sy = 0; float v_d = 0;
        project_3d_point(-390.0f, -30.0f, 410.0f, b_rot_x, b_rot_y, b_cam_z, w, h, &v_sx, &v_sy, &v_d);

        for (int a = 1; a <= 4; a++) {
            float arc_r = (float)a * 22.0f + sinf(t * 20.0f) * 6.0f;
            for (float th = -0.7f; th <= 0.7f; th += 0.04f) {
                int ax = v_sx + (int)(cosf(th) * arc_r);
                int ay = v_sy + (int)(sinf(th) * arc_r);
                if (ax >= 0 && ax < w && ay >= 0 && ay < h) {
                    fb[ay * w + ax] = 0xFF00FFFF;
                }
            }
        }
    }
    // -------------------------------------------------------------------------
    // SCENE 6: Full 3D Volumetric USDA Teddy Bear Prim Mesh + 3D Sewn Heart
    // -------------------------------------------------------------------------
    else if (scene == 6) {
        float bob = sinf(t * 8.0f) * 20.0f;
        float b_rot_y = cam_yaw + sinf(t * 4.0f) * 0.35f;
        float b_rot_x = cam_pitch + 0.10f;
        float b_cam_z = 700.0f;

        uint32_t fur_col = 0xFFFFD700;
        uint32_t muzzle_col = 0xFFFFF8DC;
        uint32_t nose_col = 0xFF221100;

        draw_3d_volumetric_sphere(fb, zbuf, w, h, 0.0f, -50.0f + bob, 400.0f, 48.0f, b_rot_x, b_rot_y, b_cam_z, fur_col, lx, ly, lz);
        draw_3d_volumetric_sphere(fb, zbuf, w, h, -45.0f, -95.0f + bob, 410.0f, 20.0f, b_rot_x, b_rot_y, b_cam_z, fur_col, lx, ly, lz);
        draw_3d_volumetric_sphere(fb, zbuf, w, h, 45.0f, -95.0f + bob, 410.0f, 20.0f, b_rot_x, b_rot_y, b_cam_z, fur_col, lx, ly, lz);
        draw_3d_volumetric_sphere(fb, zbuf, w, h, 0.0f, -40.0f + bob, 360.0f, 18.0f, b_rot_x, b_rot_y, b_cam_z, muzzle_col, lx, ly, lz);
        draw_3d_volumetric_sphere(fb, zbuf, w, h, 0.0f, -46.0f + bob, 345.0f, 6.0f, b_rot_x, b_rot_y, b_cam_z, nose_col, lx, ly, lz);
        draw_3d_volumetric_sphere(fb, zbuf, w, h, 0.0f, 45.0f + bob, 400.0f, 65.0f, b_rot_x, b_rot_y, b_cam_z, fur_col, lx, ly, lz);
        draw_3d_volumetric_sphere(fb, zbuf, w, h, -65.0f, 25.0f + bob, 380.0f, 24.0f, b_rot_x, b_rot_y, b_cam_z, fur_col, lx, ly, lz);
        draw_3d_volumetric_sphere(fb, zbuf, w, h, 65.0f, 25.0f + bob, 380.0f, 24.0f, b_rot_x, b_rot_y, b_cam_z, fur_col, lx, ly, lz);
        draw_3d_volumetric_sphere(fb, zbuf, w, h, -35.0f, 115.0f + bob, 390.0f, 26.0f, b_rot_x, b_rot_y, b_cam_z, fur_col, lx, ly, lz);
        draw_3d_volumetric_sphere(fb, zbuf, w, h, 35.0f, 115.0f + bob, 390.0f, 26.0f, b_rot_x, b_rot_y, b_cam_z, fur_col, lx, ly, lz);

        float h_pulse = 1.0f + 0.20f * sinf(t * 16.0f);
        draw_3d_volumetric_sewn_heart(fb, zbuf, w, h, 0.0f, 25.0f + bob, 335.0f, 1.35f * h_pulse, b_rot_x, b_rot_y, b_cam_z, lx, ly, lz);
    }
}

// -----------------------------------------------------------------------------
// Demoscene 5x7 Dot-Matrix Font Rasterizer
// -----------------------------------------------------------------------------
static const uint8_t FONT_5X7[128][5] = {
    [' '] = {0x00, 0x00, 0x00, 0x00, 0x00},
    ['!'] = {0x00, 0x00, 0x5F, 0x00, 0x00},
    ['#'] = {0x14, 0x7F, 0x14, 0x7F, 0x14},
    ['%'] = {0x23, 0x13, 0x08, 0x64, 0x62},
    ['*'] = {0x2A, 0x1C, 0x7F, 0x1C, 0x2A},
    ['+'] = {0x08, 0x08, 0x3E, 0x08, 0x08},
    ['-'] = {0x08, 0x08, 0x08, 0x08, 0x08},
    ['.'] = {0x00, 0x60, 0x60, 0x00, 0x00},
    ['/'] = {0x20, 0x10, 0x08, 0x04, 0x02},
    ['0'] = {0x3E, 0x51, 0x49, 0x45, 0x3E},
    ['1'] = {0x00, 0x42, 0x7F, 0x40, 0x00},
    ['2'] = {0x42, 0x61, 0x51, 0x49, 0x46},
    ['3'] = {0x21, 0x41, 0x45, 0x4B, 0x31},
    ['4'] = {0x18, 0x14, 0x12, 0x7F, 0x10},
    ['5'] = {0x27, 0x45, 0x45, 0x45, 0x39},
    ['6'] = {0x3C, 0x4A, 0x49, 0x49, 0x30},
    ['7'] = {0x01, 0x71, 0x09, 0x05, 0x03},
    ['8'] = {0x36, 0x49, 0x49, 0x49, 0x36},
    ['9'] = {0x06, 0x49, 0x49, 0x29, 0x1E},
    [':'] = {0x00, 0x36, 0x36, 0x00, 0x00},
    ['='] = {0x14, 0x14, 0x14, 0x14, 0x14},
    ['A'] = {0x7C, 0x12, 0x11, 0x12, 0x7C},
    ['B'] = {0x7F, 0x49, 0x49, 0x49, 0x36},
    ['C'] = {0x3E, 0x41, 0x41, 0x41, 0x22},
    ['D'] = {0x7F, 0x41, 0x41, 0x22, 0x1C},
    ['E'] = {0x7F, 0x49, 0x49, 0x49, 0x41},
    ['F'] = {0x7F, 0x09, 0x09, 0x09, 0x01},
    ['G'] = {0x3E, 0x41, 0x49, 0x49, 0x7A},
    ['H'] = {0x7F, 0x08, 0x08, 0x08, 0x7F},
    ['I'] = {0x00, 0x41, 0x7F, 0x41, 0x00},
    ['J'] = {0x20, 0x40, 0x41, 0x3F, 0x01},
    ['K'] = {0x7F, 0x08, 0x14, 0x22, 0x41},
    ['L'] = {0x7F, 0x40, 0x40, 0x40, 0x40},
    ['M'] = {0x7F, 0x02, 0x0C, 0x02, 0x7F},
    ['N'] = {0x7F, 0x04, 0x08, 0x10, 0x7F},
    ['O'] = {0x3E, 0x41, 0x41, 0x41, 0x3E},
    ['P'] = {0x7F, 0x09, 0x09, 0x09, 0x06},
    ['Q'] = {0x3E, 0x41, 0x51, 0x21, 0x5E},
    ['R'] = {0x7F, 0x09, 0x19, 0x29, 0x46},
    ['S'] = {0x46, 0x49, 0x49, 0x49, 0x31},
    ['T'] = {0x01, 0x01, 0x7F, 0x01, 0x01},
    ['U'] = {0x3F, 0x40, 0x40, 0x40, 0x3F},
    ['V'] = {0x1F, 0x20, 0x40, 0x20, 0x1F},
    ['W'] = {0x7F, 0x20, 0x18, 0x20, 0x7F},
    ['X'] = {0x63, 0x14, 0x08, 0x14, 0x63},
    ['Y'] = {0x07, 0x08, 0x70, 0x08, 0x07},
    ['Z'] = {0x61, 0x51, 0x49, 0x45, 0x43},
    ['['] = {0x00, 0x7F, 0x41, 0x41, 0x00},
    [']'] = {0x00, 0x41, 0x41, 0x7F, 0x00},
    ['_'] = {0x40, 0x40, 0x40, 0x40, 0x40},
    ['|'] = {0x00, 0x00, 0x77, 0x00, 0x00}
};

static void draw_text_gradient(uint32_t *fb, int w, int h, int x, int y, const char *str, uint32_t col_top, uint32_t col_bot, int scale) {
    if (!str || !fb) return;
    int cur_x = x;

    while (*str) {
        unsigned char ch = (unsigned char)*str++;
        if (ch < 128) {
            for (int col = 0; col < 5; col++) {
                uint8_t line = FONT_5X7[(int)ch][col];
                for (int row = 0; row < 7; row++) {
                    if ((line >> row) & 1) {
                        float v = (float)row / 6.0f;
                        uint8_t r = (uint8_t)(((col_top >> 16) & 0xFF) * (1.0f - v) + ((col_bot >> 16) & 0xFF) * v);
                        uint8_t g = (uint8_t)(((col_top >> 8) & 0xFF) * (1.0f - v) + ((col_bot >> 8) & 0xFF) * v);
                        uint8_t b = (uint8_t)((col_top & 0xFF) * (1.0f - v) + (col_bot & 0xFF) * v);
                        uint32_t pix_col = 0xFF000000 | (r << 16) | (g << 8) | b;

                        for (int dy = 0; dy < scale; dy++) {
                            int py = y + row * scale + dy;
                            if (py < 0 || py >= h) continue;
                            for (int dx = 0; dx < scale; dx++) {
                                int px = cur_x + col * scale + dx;
                                if (px >= 0 && px < w) {
                                    fb[py * w + px] = pix_col;
                                }
                            }
                        }
                    }
                }
            }
        }
        cur_x += 6 * scale;
    }
}

static void draw_text(uint32_t *fb, int w, int h, int x, int y, const char *str, uint32_t color, int scale) {
    draw_text_gradient(fb, w, h, x, y, str, color, color, scale);
}

// -----------------------------------------------------------------------------
// Demoscene 3D ANSI Bubble Text Font
// -----------------------------------------------------------------------------
static void draw_demoscene_bubble_text(uint32_t *fb, int w, int h, int x, int y, const char *str,
                                       uint32_t col_top, uint32_t col_bot, uint32_t shadow_col, float bob_phase) {
    if (!str || !fb) return;
    int len = (int)strlen(str);

    for (int i = 0; i < len; i++) {
        char ch = str[i];
        float bob = sinf(bob_phase + (float)i * 0.45f) * 16.0f;
        int char_x = x + i * 44;
        int char_y = y + (int)bob;

        char s[2] = { ch, '\0' };

        for (int ext = 6; ext >= 2; ext -= 2) {
            draw_text(fb, w, h, char_x + ext, char_y + ext, s, shadow_col, 5);
        }

        draw_text(fb, w, h, char_x + 1, char_y + 1, s, 0xFF101010, 5);
        draw_text(fb, w, h, char_x - 1, char_y - 1, s, 0xFF101010, 5);

        draw_text_gradient(fb, w, h, char_x, char_y, s, col_top, col_bot, 5);

        draw_text(fb, w, h, char_x + 1, char_y + 1, s, 0xFFFFFFFF, 1);
        draw_text(fb, w, h, char_x + 2, char_y + 2, s, 0xFFFFFFFF, 1);
    }
}

// -----------------------------------------------------------------------------
// Real-time 2-3 Tree AST Merkle Root Graph & Formal Prover HUD
// -----------------------------------------------------------------------------
static void draw_ast_merkle_proving_hud(uint32_t *fb, int w, int h, int scene, float t, uint64_t merkle_proof) {
    int box_x = 40, box_y = 40;
    int box_w = 620, box_h = 145;
    uint32_t hud_cyan = 0xFF00FFCC;

    for (int by = 0; by < box_h; by++) {
        int py = box_y + by;
        if (py >= 0 && py < h) {
            for (int bx = 0; bx < box_w; bx++) {
                int px = box_x + bx;
                if (px >= 0 && px < w) {
                    uint32_t bg = fb[py * w + px];
                    uint8_t r = (uint8_t)(((bg >> 16) & 0xFF) * 0.35f);
                    uint8_t g = (uint8_t)(((bg >> 8) & 0xFF) * 0.35f);
                    uint8_t b = (uint8_t)((bg & 0xFF) * 0.35f);
                    fb[py * w + px] = 0xFF000000 | (r << 16) | (g << 8) | b;
                }
            }
        }
    }

    draw_text(fb, w, h, box_x + 15, box_y + 10, "HARVARD 1946 X CICS DISCOVERY [0xC1C5]", hud_cyan, 2);

    // Live Evaluation of Suite 56 (Vulkan Camera), Suite 57 (Bionika Synth), Suite 52 (CICS Jump)
    uint32_t dynamic_punch = ((uint32_t)(t * 4.0f)) % 32;
    AuncientHarvardZuoSensingPinMetrics pin_m = {0};
    auncient_harvard_zuo_sensing_pin_matrix_prover(dynamic_punch, false, 3, &pin_m);

    AuncientHarvardZuoCicsJumpMetrics cics_m = {0};
    uint32_t dyn_base = 100 + (((uint32_t)(t * 10.0f)) % 500);
    uint32_t dyn_theta = 50 + (((uint32_t)(t * 5.0f)) % 200);
    auncient_harvard_zuo_cics_wheeler_jump_prover(dyn_base, dyn_theta, false, 3, &cics_m);

    AuncientVulkanCameraMetrics vulk_m = {0};
    auncient_vulkan_vision_camera_prover(60, 0, 800, false, 3, &vulk_m);

    AuncientBionikaSynthMetrics synth_m = {0};
    auncient_bionika_synth_overdrive_prover(1, 55, 55705, false, 3, &synth_m);

    char buf1[80];
    snprintf(buf1, sizeof(buf1), "TIME: %05.2fS | SUITE 57/57 | SCENE %d/7 | FPS 60", t, scene);
    draw_text(fb, w, h, box_x + 15, box_y + 32, buf1, 0xFFFFFFFF, 2);

    int bar_y = box_y + 54;
    int score_len = (int)((t / 90.0f) * 500.0f);
    if (score_len > 500) score_len = 500;

    for (int bx = 0; bx < score_len; bx++) {
        int px = box_x + 15 + bx;
        if (px < w) {
            fb[bar_y * w + px] = 0xFF00FF7F;
            fb[(bar_y+1) * w + px] = 0xFF00FF7F;
        }
    }

    char buf2[80];
    snprintf(buf2, sizeof(buf2), "VULKAN FOV: %02u° [Z=%04u] | SYNTH RMS: %05ld | G_GATE: %ld/1000",
             vulk_m.camera_fov_deg, vulk_m.mesh_depth_z, synth_m.rms_norm_q16, synth_m.g_gate_factor);
    draw_text(fb, w, h, box_x + 15, box_y + 70, buf2, 0xFFFFD700, 2);

    char buf3[80];
    snprintf(buf3, sizeof(buf3), "CICS TARGET: 0x%04X | PUNCH: %02u | ACCUM: 1,000,000 SAAT",
             cics_m.target_pc, pin_m.input_punch_mask);
    draw_text(fb, w, h, box_x + 15, box_y + 92, buf3, 0xFF00E5FF, 2);

    char buf4[80];
    snprintf(buf4, sizeof(buf4), "FORMAL RULING: QUALIFIED_ORBITAL_HANDSHAKE [ACID LATCH SEALED]");
    draw_text(fb, w, h, box_x + 15, box_y + 115, buf4, 0xFF76FF03, 2);

    int tree_x = 60, tree_y = h - 140;
    draw_text(fb, w, h, tree_x + 35, tree_y - 20, "2-3 MERKLE AST", 0xFFFFD700, 1);
    draw_text(fb, w, h, tree_x + 45, tree_y + 12, "ROOT", 0xFFFFFFFF, 1);

    int rx = w - 460, ry = h - 60;
    draw_text(fb, w, h, rx, ry - 25, "MERKLE PROOF: 0X0D4E0757DE528828", 0xFF00FFCC, 2);
    for (int b = 0; b < 16; b++) {
        uint32_t bit_col = ((merkle_proof >> (b * 4)) & 0x1) ? 0xFF00FFCC : 0xFF444444;
        for (int ly = 0; ly < 18; ly++) {
            int py = ry + ly;
            int px = rx + b * 24;
            if (px < w && py < h) fb[py * w + px] = bit_col;
        }
    }
}

// -----------------------------------------------------------------------------
// Dark Super8 Optical Emulsion & Vignetting
// -----------------------------------------------------------------------------
static inline void apply_super8_film_grain(uint32_t *fb, int w, int h, float t) {
    (void)t;
    float max_dist = sqrtf((float)(w*w + h*h)) * 0.5f;
    float cx = (float)w * 0.5f, cy = (float)h * 0.5f;

    int grain_samples = w * h / 12;
    for (int i = 0; i < grain_samples; i++) {
        int idx = rand() % (w * h);
        int px = idx % w;
        int py = idx / w;

        float dist = sqrtf((float)((px - cx)*(px - cx) + (py - cy)*(py - cy)));
        float vig = 1.0f - (dist / max_dist) * 0.45f;
        if (vig < 0.55f) vig = 0.55f;

        uint32_t c = fb[idx];
        int noise = (rand() % 32) - 16;
        int r = (int)(((c >> 16) & 0xFF) * vig) + noise;
        int g = (int)(((c >> 8) & 0xFF) * vig) + noise;
        int b = (int)((c & 0xFF) * vig) + noise;

        if (r < 0) { r = 0; }
        if (r > 255) { r = 255; }
        if (g < 0) { g = 0; }
        if (g > 255) { g = 255; }
        if (b < 0) { b = 0; }
        if (b > 255) { b = 255; }

        fb[idx] = 0xFF000000 | (r << 16) | (g << 8) | b;
    }
}

// -----------------------------------------------------------------------------
// Full 3D Volumetric Scene Frame Renderer with 7-Scene USDA Asset Prims
// -----------------------------------------------------------------------------
void tsfi_mp4_render_scene_frame(TsfiRenderFrameContext *ctx) {
    if (!ctx || !ctx->framebuffer) return;

    float t = ctx->timestamp_sec;
    float total_dur = (ctx->total_duration_sec > 0.0f) ? ctx->total_duration_sec : (float)MP4_DEFAULT_SECONDS;
    float norm_t = (t / total_dur) * (float)MP4_DEFAULT_SECONDS; // Normalized to 7 standard movements
    int w = MP4_WIDTH;
    int h = MP4_HEIGHT;
    uint32_t *fb = ctx->framebuffer;
    float cx = (float)w * 0.5f;

    float *zbuf = (float *)malloc(w * h * sizeof(float));
    if (!zbuf) return;
    for (int i = 0; i < w * h; i++) zbuf[i] = 100000.0f;

    float coords19[19] = {0};
    int token_idx = ((int)(t / 18.0f)) % 5;
    compute_19d_projection(LAU_TOKENS[token_idx], t, coords19, 19);

    float cam_yaw = t * 0.4f;
    float cam_pitch = sinf(t * 0.2f) * 0.15f;

    float lx = 0.577f, ly = -0.577f, lz = 0.577f;

    // -------------------------------------------------------------------------
    // SCENE 1: VERSE 1 (00:00 - 15:00) | 1946 HARVARD MARK I 24-DECADE ROTARY COUNTER WHEELS
    // -------------------------------------------------------------------------
    if (norm_t < 15.0f) {
        ctx->scene_index = 1;
        // Rich Bakelite Chassis Floor & Laboratory Wall Gradient
        for (int y = 0; y < h; y++) {
            float v = (float)y / (float)h;
            uint32_t col = 0xFF000000 | ((uint32_t)(25.0f * (1.0f - v) + 8.0f) << 16) | ((uint32_t)(20.0f * (1.0f - v) + 6.0f) << 8) | (uint32_t)(35.0f * v + 12.0f);
            for (int x = 0; x < w; x++) fb[y * w + x] = col;
        }

        // Render 24-Decade Brass Mainshafts & Rotating Counter Wheels
        int decades = 24;
        float wheel_spacing = 38.0f;
        float start_x = -((float)decades * 0.5f) * wheel_spacing;

        for (int dec = 0; dec < decades; dec++) {
            float shaft_x = start_x + (float)dec * wheel_spacing;
            float step_rot = (t * 4.0f) + (float)dec * 0.4f;

            // 1. Draw Vertical Steel Drive Shaft
            draw_3d_volumetric_cylinder(fb, zbuf, w, h, shaft_x, 0.0f, 480.0f, 4.0f, 420.0f, cam_pitch, cam_yaw, 800.0f, 0xFF888888, lx, ly, lz);

            // 2. Draw Stepped Brass 10-Digit Counter Wheel (Annals 1946 Geneva Mechanism)
            uint32_t wheel_brass = (dec % 2 == 0) ? 0xFFFFD700 : 0xFFDAA520;
            draw_3d_volumetric_cylinder(fb, zbuf, w, h, shaft_x, sinf(step_rot) * 15.0f, 480.0f, 16.0f, 22.0f, cam_pitch + step_rot, cam_yaw, 800.0f, wheel_brass, lx, ly, lz);

            // 3. 10-Phase Commutator Contact Studs around Wheel Periphery
            for (int stud = 0; stud < 10; stud++) {
                float stud_ang = (float)stud * (2.0f * (float)M_PI / 10.0f) + step_rot;
                float stud_x = shaft_x + cosf(stud_ang) * 18.0f;
                float stud_y = sinf(step_rot) * 15.0f + sinf(stud_ang) * 18.0f;
                draw_3d_volumetric_sphere(fb, zbuf, w, h, stud_x, stud_y, 480.0f, 2.5f, cam_pitch, cam_yaw, 800.0f, 0xFFFFFFFF, lx, ly, lz);
            }
        }

        draw_3d_vaesen_character(fb, zbuf, w, h, 1, t, cam_yaw, cam_pitch, lx, ly, lz);

        draw_demoscene_bubble_text(fb, w, h, (int)cx - 240, 180, "HARVARD 1946", 0xFFFFD700, 0xFFFFAB00, 0xFF3E2723, t * 3.0f);
        draw_text(fb, w, h, (int)cx - 280, 260, "VERSE 1: 24-DECADE MECHANICAL COUNTER WHEELS & MAINSHAFT", 0xFFE0E0E0, 2);
    }
    // -------------------------------------------------------------------------
    // SCENE 2: CHORUS 1 (15:00 - 25:00) | 1946 HARVARD 3-ADDRESS PERFORATED TAPE TRANSPORT
    // -------------------------------------------------------------------------
    else if (norm_t < 25.0f) {
        ctx->scene_index = 2;
        for (int i = 0; i < w * h; i++) fb[i] = 0xFF050E12;

        float tape_feed = t * 120.0f;
        int tape_segments = 48;

        // Render Continuous Perforated Paper Tape Loop with 24-Decade Hole Matrix
        for (int seg = 0; seg < tape_segments; seg++) {
            float frac = (float)seg / (float)tape_segments;
            float tape_z = -400.0f + fmodf(frac * 800.0f + tape_feed, 800.0f);
            float tape_y = sinf(tape_z * 0.008f) * 45.0f;

            // Draw Paper Tape Margin Rails (Manila Paper Cream Color)
            int sx_l = 0, sy_l = 0, sx_r = 0, sy_r = 0;
            float dl = 0, dr = 0;
            project_3d_point(-160.0f, tape_y, tape_z, cam_pitch, cam_yaw, 800.0f, w, h, &sx_l, &sy_l, &dl);
            project_3d_point(160.0f, tape_y, tape_z, cam_pitch, cam_yaw, 800.0f, w, h, &sx_r, &sy_r, &dr);

            draw_3d_volumetric_wire(fb, zbuf, w, h, sx_l, sy_l, dl, sx_r, sy_r, dr, 0xFFEEDC82, 3.5f, lx, ly, lz);

            // Perforated Sensing Pin Holes (24 Decades across Tape Width)
            for (int h_idx = 0; h_idx < 6; h_idx++) {
                float hole_x = -120.0f + (float)h_idx * 48.0f;
                bool is_punched = (((seg * 7 + h_idx * 13) % 5) == 0);
                if (is_punched) {
                    draw_3d_volumetric_sphere(fb, zbuf, w, h, hole_x, tape_y, tape_z, 5.0f, cam_pitch, cam_yaw, 800.0f, 0xFF00FFAA, lx, ly, lz);
                } else {
                    draw_3d_volumetric_sphere(fb, zbuf, w, h, hole_x, tape_y, tape_z, 2.5f, cam_pitch, cam_yaw, 800.0f, 0xFF332211, lx, ly, lz);
                }
            }
        }

        // Stepping Solenoid Armature Cylinders (Left and Right Spools)
        draw_3d_volumetric_cylinder(fb, zbuf, w, h, -220.0f, -80.0f, 400.0f, 40.0f, 60.0f, cam_pitch, cam_yaw, 800.0f, 0xFF555555, lx, ly, lz);
        draw_3d_volumetric_cylinder(fb, zbuf, w, h, 220.0f, -80.0f, 400.0f, 40.0f, 60.0f, cam_pitch, cam_yaw, 800.0f, 0xFF555555, lx, ly, lz);

        draw_3d_vaesen_character(fb, zbuf, w, h, 2, t, cam_yaw, cam_pitch, lx, ly, lz);

        draw_demoscene_bubble_text(fb, w, h, (int)cx - 240, 180, "TAPE FEED", 0xFF00FFAA, 0xFF00E676, 0xFF003311, t * 3.5f);
        draw_text(fb, w, h, (int)cx - 280, 260, "CHORUS 1: 3-ADDRESS SEQUENCE TAPE STEPPER & SENSING PINS", 0xFFE0E0E0, 2);
    }
    // -------------------------------------------------------------------------
    // SCENE 3: VERSE 2 (25:00 - 38:00) | 3D USDA Volumetric Double-Helix DNA Lattice
    // -------------------------------------------------------------------------
    else if (norm_t < 38.0f) {
        ctx->scene_index = 3;
        for (int y = 0; y < h; y++) {
            uint32_t col = (y % 4 == 0) ? 0xFF181005 : 0xFF0A0702;
            for (int x = 0; x < w; x++) fb[y * w + x] = col;
        }

        int base_pairs = 36;
        for (int b = 0; b < base_pairs; b++) {
            float frac = (float)b / (float)base_pairs;
            float z3d = -350.0f + frac * 700.0f;
            float ang1 = frac * 4.0f * (float)M_PI + t * 1.5f;
            float ang2 = ang1 + (float)M_PI;

            float rad = 140.0f;
            float x1_3d = cosf(ang1) * rad, y1_3d = sinf(ang1) * rad;
            float x2_3d = cosf(ang2) * rad, y2_3d = sinf(ang2) * rad;

            int sx1 = 0, sy1 = 0, sx2 = 0, sy2 = 0;
            float d1 = 0, d2 = 0;

            project_3d_point(x1_3d, y1_3d, z3d, cam_pitch, cam_yaw, 800.0f, w, h, &sx1, &sy1, &d1);
            project_3d_point(x2_3d, y2_3d, z3d, cam_pitch, cam_yaw, 800.0f, w, h, &sx2, &sy2, &d2);

            uint32_t gold_col = (b % 2 == 0) ? 0xFFFFD700 : 0xFFECC460;
            draw_3d_volumetric_wire(fb, zbuf, w, h, sx1, sy1, d1, sx2, sy2, d2, gold_col, 4.0f, lx, ly, lz);

            // USDA 3D Nucleotide Base Node Spheres (Radius 6px)
            draw_3d_volumetric_sphere(fb, zbuf, w, h, x1_3d, y1_3d, z3d, 6.0f, cam_pitch, cam_yaw, 800.0f, 0xFFFFFFFF, lx, ly, lz);
            draw_3d_volumetric_sphere(fb, zbuf, w, h, x2_3d, y2_3d, z3d, 6.0f, cam_pitch, cam_yaw, 800.0f, 0xFFFFF8DC, lx, ly, lz);
        }

        draw_demoscene_bubble_text(fb, w, h, (int)cx - 160, 180, "DAMASK", 0xFFFFEA00, 0xFFFFAB00, 0xFF4E342E, t * 3.0f);
        draw_text(fb, w, h, (int)cx - 220, 260, "VERSE 2: 3D DOUBLE-HELIX DNA LATTICE", 0xFFE0E0E0, 2);
    }
    // -------------------------------------------------------------------------
    // SCENE 4: CHORUS 2 (38:00 - 50:00) | 3D USDA Volumetric Trefoil Knot Manifold
    // -------------------------------------------------------------------------
    else if (norm_t < 50.0f) {
        ctx->scene_index = 4;
        for (int i = 0; i < w * h; i++) fb[i] = 0xFF030712;

        int pts = 90;
        int px1_prev = 0, py1_prev = 0, px2_prev = 0, py2_prev = 0;
        float pz1_prev = 0, pz2_prev = 0;
        int first_x1 = 0, first_y1 = 0, first_x2 = 0, first_y2 = 0;
        float first_z1 = 0, first_z2 = 0;

        for (int i = 0; i < pts; i++) {
            float theta = (float)i * (2.0f * (float)M_PI / (float)pts);

            float r = 180.0f + 60.0f * cosf(3.0f * theta);
            float x1_3d = r * cosf(2.0f * theta);
            float y1_3d = r * sinf(2.0f * theta);
            float z1_3d = -80.0f * sinf(3.0f * theta);

            float x2_3d = -x1_3d, y2_3d = -y1_3d, z2_3d = -z1_3d;

            int sx1 = 0, sy1 = 0, sx2 = 0, sy2 = 0;
            float d1 = 0, d2 = 0;

            project_3d_point(x1_3d, y1_3d, z1_3d, cam_pitch + t * 0.5f, cam_yaw + t * 0.8f, 800.0f, w, h, &sx1, &sy1, &d1);
            project_3d_point(x2_3d, y2_3d, z2_3d, cam_pitch + t * 0.5f, cam_yaw + t * 0.8f, 800.0f, w, h, &sx2, &sy2, &d2);

            if (i == 0) {
                first_x1 = sx1; first_y1 = sy1; first_z1 = d1;
                first_x2 = sx2; first_y2 = sy2; first_z2 = d2;
            } else {
                draw_3d_volumetric_wire(fb, zbuf, w, h, px1_prev, py1_prev, pz1_prev, sx1, sy1, d1, 0xFF5C85FF, 4.0f, lx, ly, lz);
                draw_3d_volumetric_wire(fb, zbuf, w, h, px2_prev, py2_prev, pz2_prev, sx2, sy2, d2, 0xFFE08B3E, 4.0f, lx, ly, lz);
            }
            px1_prev = sx1; py1_prev = sy1; pz1_prev = d1;
            px2_prev = sx2; py2_prev = sy2; pz2_prev = d2;
        }
        draw_3d_volumetric_wire(fb, zbuf, w, h, px1_prev, py1_prev, pz1_prev, first_x1, first_y1, first_z1, 0xFF5C85FF, 4.0f, lx, ly, lz);
        draw_3d_volumetric_wire(fb, zbuf, w, h, px2_prev, py2_prev, pz2_prev, first_x2, first_y2, first_z2, 0xFFE08B3E, 4.0f, lx, ly, lz);

        draw_demoscene_bubble_text(fb, w, h, (int)cx - 190, 180, "MANIFOLD", 0xFF448AFF, 0xFF2979FF, 0xFF0D47A1, t * 3.5f);
        draw_text(fb, w, h, (int)cx - 220, 260, "CHORUS 2: 3D DUAL-TREFOIL KNOT MANIFOLD", 0xFFE0E0E0, 2);
    }
    // -------------------------------------------------------------------------
    // SCENE 5: VERSE 3 (50:00 - 62:00) | 3D USDA Geodesic Singularity Icosahedron
    // -------------------------------------------------------------------------
    else if (norm_t < 62.0f) {
        ctx->scene_index = 5;
        for (int i = 0; i < w * h; i++) fb[i] = 0xFF0E0402;

        float tension = (norm_t - 50.0f) / 12.0f;
        float r_scale = 320.0f * (1.0f - tension * 0.88f);

        float phi_gold = (1.0f + sqrtf(5.0f)) * 0.5f;
        float ico_verts[12][3] = {
            { -1.0f,  phi_gold, 0.0f }, {  1.0f,  phi_gold, 0.0f }, { -1.0f, -phi_gold, 0.0f }, {  1.0f, -phi_gold, 0.0f },
            { 0.0f, -1.0f,  phi_gold }, { 0.0f,  1.0f,  phi_gold }, { 0.0f, -1.0f, -phi_gold }, { 0.0f,  1.0f, -phi_gold },
            {  phi_gold, 0.0f, -1.0f }, {  phi_gold, 0.0f,  1.0f }, { -phi_gold, 0.0f, -1.0f }, { -phi_gold, 0.0f,  1.0f }
        };

        int ico_sx[12] = {0}, ico_sy[12] = {0};
        float ico_d[12] = {0};
        for (int v = 0; v < 12; v++) {
            float x3d = ico_verts[v][0] * (r_scale / phi_gold);
            float y3d = ico_verts[v][1] * (r_scale / phi_gold);
            float z3d = ico_verts[v][2] * (r_scale / phi_gold);
            project_3d_point(x3d, y3d, z3d, t * 1.5f, t * 2.0f, 800.0f, w, h, &ico_sx[v], &ico_sy[v], &ico_d[v]);

            // 3D USDA Vertex Spheres at Icosahedron Vertices
            draw_3d_volumetric_sphere(fb, zbuf, w, h, x3d, y3d, z3d, 8.0f, t * 1.5f, t * 2.0f, 800.0f, 0xFFFF3300, lx, ly, lz);
        }

        for (int i = 0; i < 12; i++) {
            for (int j = i + 1; j < 12; j++) {
                float dx = ico_verts[i][0] - ico_verts[j][0];
                float dy = ico_verts[i][1] - ico_verts[j][1];
                float dz = ico_verts[i][2] - ico_verts[j][2];
                if (fabsf(dx*dx + dy*dy + dz*dz - 4.0f) < 0.1f) {
                    uint32_t spoke_col = ((i + j) % 2 == 0) ? 0xFFFF3300 : 0xFFFF9900;
                    draw_3d_volumetric_wire(fb, zbuf, w, h, ico_sx[i], ico_sy[i], ico_d[i], ico_sx[j], ico_sy[j], ico_d[j], spoke_col, 4.5f, lx, ly, lz);
                }
            }
        }

        int core_r = (int)(16.0f + tension * 28.0f);
        int mid_x = (int)cx, mid_y = (int)((float)h * 0.5f);
        for (int dy = -core_r; dy <= core_r; dy++) {
            for (int dx = -core_r; dx <= core_r; dx++) {
                int px = mid_x + dx, py = mid_y + dy;
                if (px >= 0 && px < w && py >= 0 && py < h) {
                    float dist = sqrtf((float)(dx*dx + dy*dy));
                    if (dist < (float)core_r) {
                        float flare = (1.0f - dist / (float)core_r);
                        uint32_t cur = fb[py * w + px];
                        uint8_t r = (uint8_t)fminf(255.0f, ((cur >> 16) & 0xFF) + 255.0f * flare);
                        uint8_t g = (uint8_t)fminf(255.0f, ((cur >> 8) & 0xFF) + 200.0f * flare * tension);
                        uint8_t b = (uint8_t)fminf(255.0f, (cur & 0xFF) + 120.0f * flare * tension);
                        fb[py * w + px] = 0xFF000000 | (r << 16) | (g << 8) | b;
                    }
                }
            }
        }

        draw_demoscene_bubble_text(fb, w, h, (int)cx - 170, 180, "TENSION", 0xFFFF5252, 0xFFFF1744, 0xFF880E4F, t * 4.0f);
        draw_text(fb, w, h, (int)cx - 240, 260, "VERSE 3: 3D GEODESIC SINGULARITY COLLAPSE", 0xFFE0E0E0, 2);
    }
    // -------------------------------------------------------------------------
    // SCENE 6: CHORUS 3 (62:00 - 80:00) | 3D VOLUMETRIC BLAST & 3D USDA TEDDY BEAR
    // -------------------------------------------------------------------------
    else if (norm_t < 80.0f) {
        ctx->scene_index = 6;
        float drop_t = t - 62.0f;
        for (int y = 0; y < h; y++) {
            for (int x = 0; x < w; x++) {
                uint32_t r = (uint32_t)(20.0f + 15.0f * sinf(drop_t * 2.0f + (float)x * 0.01f));
                uint32_t g = (uint32_t)(14.0f + 14.0f * cosf(drop_t * 2.5f + (float)y * 0.01f));
                uint32_t b = (uint32_t)(32.0f + 20.0f * sinf(drop_t * 3.0f));
                fb[y * w + x] = 0xFF000000 | (r << 16) | (g << 8) | b;
            }
        }

        int ribbons = 24;
        for (int rb = 0; rb < ribbons; rb++) {
            float ang = (float)rb * (2.0f * (float)M_PI / (float)ribbons);
            int prev_x = 0, prev_y = 0;
            float prev_z = 0;

            for (int s = 1; s <= 16; s++) {
                float dist = (float)s * 40.0f + sinf(drop_t * 8.0f) * 20.0f;
                float x3d = cosf(ang) * dist;
                float y3d = sinf(ang) * dist * 0.7f;
                float z3d = -200.0f + (float)s * 35.0f;

                int sx = 0, sy = 0; float depth = 0;
                project_3d_point(x3d, y3d, z3d, cam_pitch + drop_t * 0.8f, cam_yaw + drop_t * 1.2f, 750.0f, w, h, &sx, &sy, &depth);

                if (s > 1) {
                    uint32_t holo = (s % 3 == 0) ? 0xFF00FFFF : ((s % 3 == 1) ? 0xFFFF00FF : 0xFFFFFFFF);
                    draw_3d_volumetric_wire(fb, zbuf, w, h, prev_x, prev_y, prev_z, sx, sy, depth, holo, 3.5f, lx, ly, lz);
                }
                prev_x = sx; prev_y = sy; prev_z = depth;
            }
        }

        draw_3d_vaesen_character(fb, zbuf, w, h, 6, t, cam_yaw, cam_pitch, lx, ly, lz);

        draw_demoscene_bubble_text(fb, w, h, (int)cx - 200, 180, "CRESCENDO", 0xFFFFFFFF, 0xFFFF00FF, 0xFF4A148C, t * 5.0f);
        draw_text(fb, w, h, (int)cx - 240, 260, "CHORUS 3: 3D VOLUMETRIC BLAST & TEDDY BEAR", 0xFF00FFFF, 2);
    }
    // -------------------------------------------------------------------------
    // SCENE 7: OUTRO (80:00 - 90:00) | 1946 DUAL-CAM 180-DEGREE TIMING OSCILLOSCOPE CRT
    // -------------------------------------------------------------------------
    else {
        ctx->scene_index = 7;

        // Vintage Oscilloscope Green Phosphor CRT Screen Background
        for (int y = 0; y < h; y++) {
            float v = (float)y / (float)h;
            uint32_t col = 0xFF000000 | ((uint32_t)(8.0f * (1.0f - v)) << 16) | ((uint32_t)(40.0f * (1.0f - v) + 12.0f) << 8) | (uint32_t)(18.0f * v + 6.0f);
            for (int x = 0; x < w; x++) fb[y * w + x] = col;
        }

        // Draw 180-Degree Orthogonal Dual-Cam Timing Vectors (t_cam vs p_cam)
        int trace_pts = 200;
        int prev_tx = 0, prev_ty = 0, prev_px = 0, prev_py = 0;
        float prev_tz = 0, prev_pz = 0;

        for (int p = 0; p < trace_pts; p++) {
            float frac = (float)p / (float)trace_pts;
            float phi = frac * 4.0f * (float)M_PI + t * 4.0f;

            // Value Cam t_n (Cos wave) vs Control Cam p_n (180-deg phase shift Sin wave)
            float t_val = cosf(phi) * 160.0f;
            float p_val = -cosf(phi) * 160.0f; // Exact 180-degree mechanical opposition
            float z_pos = -300.0f + frac * 600.0f;

            int sx_t = 0, sy_t = 0, sx_p = 0, sy_p = 0;
            float dt = 0, dp = 0;

            project_3d_point(t_val - 120.0f, sinf(phi * 2.0f) * 40.0f, z_pos, cam_pitch, cam_yaw, 800.0f, w, h, &sx_t, &sy_t, &dt);
            project_3d_point(p_val + 120.0f, cosf(phi * 2.0f) * 40.0f, z_pos, cam_pitch, cam_yaw, 800.0f, w, h, &sx_p, &sy_p, &dp);

            if (p > 0) {
                draw_3d_volumetric_wire(fb, zbuf, w, h, prev_tx, prev_ty, prev_tz, sx_t, sy_t, dt, 0xFF00FF77, 3.5f, lx, ly, lz);
                draw_3d_volumetric_wire(fb, zbuf, w, h, prev_px, prev_py, prev_pz, sx_p, sy_p, dp, 0xFFFFCC00, 3.5f, lx, ly, lz);
            }
            prev_tx = sx_t; prev_ty = sy_t; prev_tz = dt;
            prev_px = sx_p; prev_py = sy_p; prev_pz = dp;
        }

        draw_demoscene_bubble_text(fb, w, h, (int)cx - 240, 180, "ORTHOGONAL", 0xFF69F0AE, 0xFF00BFA5, 0xFF004D40, t * 2.5f);
        draw_text(fb, w, h, (int)cx - 280, 260, "OUTRO: 180-DEGREE DUAL-CAM TIMING ORTHOGONALITY & CHANCERY SEAL", 0xFFE0E0E0, 2);
    }

    apply_super8_film_grain(fb, w, h, t);
    draw_ast_merkle_proving_hud(fb, w, h, ctx->scene_index, t, 0x0d4e0757de528828ULL);

    free(zbuf);
}

// -----------------------------------------------------------------------------
// Video Compiler Pipeline
// -----------------------------------------------------------------------------
bool tsfi_mp4_compile_video_with_audio(TsfiMp4Pipeline *pipe) {
    if (!pipe) return false;

    char cmd[1024];
    snprintf(cmd, sizeof(cmd),
             "ffmpeg -y -f rawvideo -vcodec rawvideo -s %dx%d -pix_fmt bgra -r %d -i - "
             "-i %s -c:v libx264 -pix_fmt yuv420p -preset ultrafast -c:a aac -b:a 192k "
             "-shortest %s > /dev/null 2>&1",
             MP4_WIDTH, MP4_HEIGHT, MP4_FPS, pipe->audio_wav_path, pipe->output_mp4_path);

    FILE *ffmpeg_pipe = popen(cmd, "w");
    if (!ffmpeg_pipe) return false;

    uint32_t *frame = (uint32_t *)malloc(MP4_WIDTH * MP4_HEIGHT * sizeof(uint32_t));
    if (!frame) {
        pclose(ffmpeg_pipe);
        return false;
    }

    TsfiRenderFrameContext ctx;
    ctx.framebuffer = frame;
    ctx.total_duration_sec = pipe->duration_seconds;

    for (int f = 0; f < pipe->total_frames; f++) {
        ctx.frame_index = (uint32_t)f;
        ctx.timestamp_sec = (float)f / (float)MP4_FPS;
        tsfi_mp4_render_scene_frame(&ctx);

        fwrite(frame, sizeof(uint32_t), (size_t)(MP4_WIDTH * MP4_HEIGHT), ffmpeg_pipe);
    }

    free(frame);
    int ret = pclose(ffmpeg_pipe);
    return (ret == 0);
}
