#include <stdint.h>
#include <math.h>
#include <immintrin.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include "tsfi_svdag.h"
#include "tsfi_vae_firmware.h"
#include "tsfi_pbr.h"
#include "tsfi_hair.h"
#include "tsfi_c_math.h"

// Helmholtz Voxel-Path Tracer (VLM-Enhanced + Ultra PBR + Hair + Sovereign Secrets + Depth)
// This thunk traces rays through the SVDAG geometry for physical photorealism.

static inline Vector3 v_sub(Vector3 a, Vector3 b) { return (Vector3){a.x - b.x, a.y - b.y, a.z - b.z}; }
static inline float v_dot(Vector3 a, Vector3 b) { return a.x * b.x + a.y * b.y + a.z * b.z; }
static inline Vector3 v_cross(Vector3 a, Vector3 b) { return (Vector3){a.y*b.z - a.z*b.y, a.z*b.x - a.x*b.z, a.x*b.y - a.y*b.x}; }
static inline Vector3 v_normalize(Vector3 v) {
    float mag = sqrtf(v_dot(v, v)) + 1e-6f;
    return (Vector3){v.x / mag, v.y / mag, v.z / mag};
}

void tsfi_svdag_path_trace(uint32_t *pixels, float *depth_buffer, const TSFiHelmholtzSVDAG *dag_flower, const TSFiHelmholtzSVDAG *dag_bear, int w, int h, float t, float melanin, float roughness, float iridescence) {
    if (!pixels || !dag_flower || !dag_bear) return;

    // Spinning Genie Camera: Orbits and bobs to view the 3D manifold from all angles
    float angle = t * (float)TSFI_TAU * 2.0f; // Spin twice during the lifecycle
    float elevation = 0.5f + 1.5f * sinf(t * (float)TSFI_SECRET_CORE); // Bob up and down
    Vector3 cam_pos = {2.5f * sinf(angle), elevation, -2.5f * cosf(angle)};
    
    Vector3 target = {0.0f, 0.0f, 0.0f};
    Vector3 forward = v_normalize(v_sub(target, cam_pos));
    Vector3 world_up = {0.0f, 1.0f, 0.0f};
    Vector3 right = v_normalize(v_cross(forward, world_up));
    Vector3 up = v_cross(right, forward);

    Vector3 light_pos = {1.5f, 2.0f, -1.0f};
    
    // MSAA 4x Jitter Pattern (Rotated Grid)
    const float jx[4] = {0.375f, 0.875f, 0.125f, 0.625f};
    const float jy[4] = {0.125f, 0.375f, 0.625f, 0.875f};

    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            int idx = y * w + x;

            // --- Software HiZ Early Rejection ---
            // If the depth_buffer indicates this tile is already 'closer' than the 
            // minimum bounding volume of our realizations, we skip the wave.
            if (depth_buffer && depth_buffer[idx] < 0.5f) {
                // Background pixels or occluded pixels skipped
                continue;
            }

            float total_r = 0, total_g = 0, total_b = 0;
            float samples_depth[4] = {10.0f, 10.0f, 10.0f, 10.0f};

            for (int s = 0; s < 4; s++) {
                float fx = ((float)x + jx[s]) / w * 2.0f - 1.0f;
                float fy = ((float)y + jy[s]) / h * 2.0f - 1.0f;

                Vector3 ray_dir = v_normalize((Vector3){
                    forward.x * 1.5f + right.x * fx - up.x * fy,
                    forward.y * 1.5f + right.y * fx - up.y * fy,
                    forward.z * 1.5f + right.z * fx - up.z * fy
                });
                
                Vector3 current_pos = cam_pos;
                float accum_r = 0.01f, accum_g = 0.01f, accum_b = 0.02f;
                float transmit = 1.0f;
                float first_hit_dist = 10.0f;
                bool hit = false;

                for (int step = 0; step < 128; step++) {
                    float current_dist = step * 0.03f;
                    current_pos.x = cam_pos.x + ray_dir.x * current_dist;
                    current_pos.y = cam_pos.y + ray_dir.y * current_dist;
                    current_pos.z = cam_pos.z + ray_dir.z * current_dist;

                    int vx = (int)((current_pos.x + 1.0f) * 64.0f);
                    int vy = (int)((current_pos.y + 1.0f) * 64.0f);
                    int vz = (int)((current_pos.z + 1.0f) * 64.0f);

                    if (vx < 0 || vx >= 128 || vy < 0 || vy >= 128 || vz < 0 || vz >= 128) continue;

                    // 1. FLOWER VOXELS
                    float df = tsfi_svdag_trace_point(dag_flower, vx, vy, vz);
                    if (df > 0.1f) {
                        if (!hit) { first_hit_dist = current_dist; hit = true; }
                        float venation = 0.9f + 0.2f * sinf(current_pos.x * 50.0f + current_pos.y * 50.0f + (float)TSFI_SECRET_CORE);
                        Vector3 L = v_normalize(v_sub(light_pos, current_pos));
                        Vector3 V = v_normalize(v_sub(cam_pos, current_pos));
                        Vector3 N = {0, 1, 0}; Vector3 T = {1, 0, 0};
                        Color3 albedo = {1.0f * venation, 1.0f * venation, 0.95f};
                        Color3 pbr = tsfi_sovereign_brdf(L, V, N, T, albedo, roughness * 0.8f, 0.0f, 0.0f, 0.0f, 0.4f, iridescence);
                        float opacity = fminf(1.0f, df * 0.8f);
                        accum_r += pbr.r * transmit * opacity;
                        accum_g += pbr.g * transmit * opacity;
                        accum_b += pbr.b * transmit * opacity;
                        transmit *= (1.0f - opacity);
                        if (transmit < 0.01f) break;
                    }

                    // 2. BEAR VOXELS
                    float db = tsfi_svdag_trace_point(dag_bear, vx, vy, vz);
                    if (db > 0.1f) {
                        if (!hit) { first_hit_dist = current_dist; hit = true; }
                        float jitter = sinf(current_pos.x * 100.0f + current_pos.y * 100.0f + t * 10.0f);
                        Vector3 L = v_normalize(v_sub(light_pos, current_pos));
                        Vector3 V = v_normalize(v_sub(cam_pos, current_pos));
                        Vector3 N = {0, 1, 0}; Vector3 T = {0, 1, 0};
                        Color3 albedo = {0.4f + 0.1f * jitter, 0.25f, 0.15f};
                        Color3 pbr = tsfi_hair_brdf(L, V, N, T, roughness, albedo);
                        pbr.r *= (1.0f - melanin * 0.5f); pbr.g *= (1.0f - melanin * 0.7f); pbr.b *= (1.0f - melanin * 0.9f);
                        float opacity = fminf(1.0f, db * 0.9f);
                        accum_r += pbr.r * transmit * opacity;
                        accum_g += pbr.g * transmit * opacity;
                        accum_b += pbr.b * transmit * opacity;
                        transmit *= (1.0f - opacity);
                        if (transmit < 0.01f) break;
                    }
                }
                total_r += accum_r; total_g += accum_g; total_b += accum_b;
                samples_depth[s] = first_hit_dist;
            }

            // --- Subjective MSAA Depth Resolve ---
            float final_depth = (samples_depth[0] + samples_depth[1] + samples_depth[2] + samples_depth[3]) * 0.25f;
            
            // Note: In a full implementation, we would query the active taste's vop_seeds here.
            // For now, we default to AVERAGE, but the infrastructure is ready for MIN/MAX exercise.
            if (depth_buffer) depth_buffer[idx] = final_depth;

            uint8_t final_r = (uint8_t)(fminf(1.0f, powf(total_r * 0.25f, 0.4545f)) * 255);
            uint8_t final_g = (uint8_t)(fminf(1.0f, powf(total_g * 0.25f, 0.4545f)) * 255);
            uint8_t final_b = (uint8_t)(fminf(1.0f, powf(total_b * 0.25f, 0.4545f)) * 255);
            pixels[idx] = (0xFF << 24) | (final_r << 16) | (final_g << 8) | final_b;
        }
    }
}
