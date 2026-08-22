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
#include "tsfi_montecarlo.h"
#include "auncient_teddy_bear_personality.h"
#include "tsfi_personality_models_adv.h"
#include "tsfi_zorse_eval.h"
#include "tsfi_lnr_solvers.h"


// Helmholtz Voxel-Path Tracer (VLM-Enhanced + Ultra PBR + Hair + Sovereign Secrets + Depth)
// This thunk traces rays through the SVDAG geometry for physical photorealism.

static inline Vector3 v_sub(Vector3 a, Vector3 b) { return (Vector3){a.x - b.x, a.y - b.y, a.z - b.z}; }
static inline float v_dot(Vector3 a, Vector3 b) { return a.x * b.x + a.y * b.y + a.z * b.z; }
static inline Vector3 v_cross(Vector3 a, Vector3 b) { return (Vector3){a.y*b.z - a.z*b.y, a.z*b.x - a.x*b.z, a.x*b.y - a.y*b.x}; }
static inline Vector3 v_normalize(Vector3 v) {
    float mag = sqrtf(v_dot(v, v)) + 1e-6f;
    return (Vector3){v.x / mag, v.y / mag, v.z / mag};
}

static float calculate_personality_guidance_weight(const teddy_bear_geometry_t *geom, float base_emot, float db, float jitter, float elevation, float t, int fear_level) {
    char type_buf[64];
    char status_buf[64];
    int risk = 0;

    // Modulate base emotional sensitivity based on the main character's (HoganBank) registered risk level
    if (tsfi_vsen_vaesen_lookup("HoganBank", type_buf, &risk, status_buf, sizeof(type_buf)) == 0) {
        base_emot *= (float)(1.0 + 0.1 * (double)risk);
    }

    // Scale threat models up and safety models down if regional fear telemetry is active
    double threat_multiplier = 1.0 + 0.15 * (double)fear_level;
    double safety_multiplier = 1.0 / (1.0 + 0.15 * (double)fear_level);

    double babyfacedness = 1.0;
    evaluate_keating_babyfacedness_index(geom, &babyfacedness);
    if (tsfi_vsen_vaesen_lookup("Tomte", type_buf, &risk, status_buf, sizeof(type_buf)) == 0) {
        babyfacedness *= (1.0 + 0.1 * (double)risk) * safety_multiplier;
    }

    double playfulness = 1.0;
    evaluate_scarpi_hedonic_playfulness(geom, 0.5, &playfulness);
    if (tsfi_vsen_vaesen_lookup("Nacken", type_buf, &risk, status_buf, sizeof(type_buf)) == 0) {
        playfulness *= (1.0 + 0.1 * (double)risk) * safety_multiplier;
    }

    double alignment = 1.0;
    evaluate_castle_diplomatic_alignment(geom, 0.5, 0.5, &alignment);
    if (tsfi_vsen_vaesen_lookup("Vaettir", type_buf, &risk, status_buf, sizeof(type_buf)) == 0) {
        alignment *= (1.0 + 0.1 * (double)risk);
    }

    double agreeableness = 1.0;
    evaluate_kramer_king_ward_perceived_agreeableness_consensus(geom, 0.5, 0.5, &agreeableness);
    if (tsfi_vsen_vaesen_lookup("Mara", type_buf, &risk, status_buf, sizeof(type_buf)) == 0) {
        agreeableness *= (1.0 + 0.1 * (double)risk) * threat_multiplier;
    }

    // Wang model character warmth modulated dynamically by time and camera tilt (elevation)
    double warmth = 1.0;
    double gaze_shift_freq = 1.0 + 0.5 * sin((double)t);
    double head_tilt_val = (double)elevation;
    evaluate_wang_geigel_character_warmth(geom, gaze_shift_freq, head_tilt_val, &warmth);
    if (tsfi_vsen_vaesen_lookup("Huldra", type_buf, &risk, status_buf, sizeof(type_buf)) == 0) {
        warmth *= (1.0 + 0.1 * (double)risk) * safety_multiplier;
    }

    // Masuda model perceived naturalness modulated dynamically by noise jitter and bear SVDAG density (db)
    double naturalness = 1.0;
    double sync_delay_ms = 100.0 * (1.0 + (double)jitter);
    double smile_intensity = (double)db;
    evaluate_masuda_perceived_naturalness(geom, sync_delay_ms, smile_intensity, &naturalness);
    if (tsfi_vsen_vaesen_lookup("Myling", type_buf, &risk, status_buf, sizeof(type_buf)) == 0) {
        naturalness *= (1.0 + 0.1 * (double)risk) * threat_multiplier;
    }

    double fwhr_dom = 1.0;
    evaluate_kramer_ward_fwhr_dominance(geom, 1.85, &fwhr_dom);
    if (tsfi_vsen_vaesen_lookup("Varulv", type_buf, &risk, status_buf, sizeof(type_buf)) == 0) {
        fwhr_dom *= (1.0 + 0.1 * (double)risk) * threat_multiplier;
    }

    float combined_weight = (float)(babyfacedness * playfulness * alignment * agreeableness * warmth * naturalness * fwhr_dom);
    return base_emot * combined_weight;
}

void tsfi_svdag_path_trace(uint32_t *pixels, float *depth_buffer, const TSFiHelmholtzSVDAG *dag_flower, const TSFiHelmholtzSVDAG *dag_bear, int w, int h, float t, float melanin, float roughness, float iridescence) {
    if (!pixels || !dag_flower || !dag_bear) return;

    // Register each of our 7 Vaesen models into the system registry
    tsfi_vsen_vaesen_register("Tomte", "Nisse", 3, "Friendly");
    tsfi_vsen_vaesen_register("Nacken", "Neck", 9, "Active");
    tsfi_vsen_vaesen_register("Vaettir", "Land Spirit", 4, "Neutral");
    tsfi_vsen_vaesen_register("Mara", "Nightmare", 6, "Active");
    tsfi_vsen_vaesen_register("Huldra", "Skogsra", 5, "Alluring");
    tsfi_vsen_vaesen_register("Myling", "Ghost", 7, "Restless");
    tsfi_vsen_vaesen_register("Varulv", "Werewolf", 8, "Aggressive");
    tsfi_vsen_vaesen_register("HoganBank", "Teddy Bear", 5, "Active");

    int fear_level = 0;
    tsfi_vsen_vaesen_get_aggregate_fear("Upsala", &fear_level);

    teddy_bear_geometry_t geom;
    resolve_teddy_bear_geometry(PERSONALITY_TRUSTWORTHY, &geom);

    TSFiMCAuxFeatures *aux_features = malloc(sizeof(TSFiMCAuxFeatures) * w * h);

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
            float total_emot = 0.0f;
            float total_normal_x = 0.0f, total_normal_y = 0.0f, total_normal_z = 0.0f;
            float total_albedo_r = 0.0f, total_albedo_g = 0.0f, total_albedo_b = 0.0f;
            float samples_depth[4] = {10.0f, 10.0f, 10.0f, 10.0f};
            float sample_luminance[2] = {0.0f, 0.0f};

            // Sample the first 2 rays for variance estimation
            for (int s = 0; s < 2; s++) {
                float fx = ((float)x + jx[s]) / w * 2.0f - 1.0f;
                float fy = ((float)y + jy[s]) / h * 2.0f - 1.0f;

                Vector3 ray_dir = v_normalize((Vector3){
                    forward.x * 1.5f + right.x * fx - up.x * fy,
                    forward.y * 1.5f + right.y * fx - up.y * fy,
                    forward.z * 1.5f + right.z * fx - up.z * fy
                });
                
                Vector3 current_pos = cam_pos;
                float accum_r = 0.01f, accum_g = 0.01f, accum_b = 0.02f;
                float sample_emot = 0.0f;
                Vector3 sample_normal = {0.0f, 1.0f, 0.0f};
                Color3 sample_albedo = {0.4f, 0.25f, 0.15f};
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
                        float venation = 0.9f + 0.2f * sinf(current_pos.x * 50.0f + current_pos.y * 50.0f + (float)TSFI_SECRET_CORE);
                        Vector3 L = v_normalize(v_sub(light_pos, current_pos));
                        Vector3 V = v_normalize(v_sub(cam_pos, current_pos));
                        Vector3 N = {0, 1, 0}; Vector3 T = {1, 0, 0};
                        Color3 albedo = {1.0f * venation, 1.0f * venation, 0.95f};
                        if (!hit) {
                            first_hit_dist = current_dist;
                            hit = true;
                            sample_normal = N;
                            sample_albedo = albedo;
                        }
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
                        float jitter = sinf(current_pos.x * 100.0f + current_pos.y * 100.0f + t * 10.0f);
                        Vector3 L = v_normalize(v_sub(light_pos, current_pos));
                        Vector3 V = v_normalize(v_sub(cam_pos, current_pos));
                        Vector3 N = {0, 1, 0}; Vector3 T = {0, 1, 0};
                        Color3 albedo = {0.4f + 0.1f * jitter, 0.25f, 0.15f};
                        if (!hit) {
                            first_hit_dist = current_dist;
                            hit = true;
                            sample_normal = N;
                            sample_albedo = albedo;
                        }
                        Color3 pbr = tsfi_hair_brdf(L, V, N, T, roughness, albedo);
                        pbr.r *= (1.0f - melanin * 0.5f); pbr.g *= (1.0f - melanin * 0.7f); pbr.b *= (1.0f - melanin * 0.9f);
                        float opacity = fminf(1.0f, db * 0.9f);
                        accum_r += pbr.r * transmit * opacity;
                        accum_g += pbr.g * transmit * opacity;
                        accum_b += pbr.b * transmit * opacity;
                        transmit *= (1.0f - opacity);
                        float dist_to_face = sqrtf(current_pos.x * current_pos.x + current_pos.y * current_pos.y);
                        float shading_factor = fabsf(N.y);
                        float excitation = 0.5f + 0.5f * jitter;
                        float base_emot = expf(-dist_to_face * dist_to_face) * shading_factor * excitation;
                        sample_emot = calculate_personality_guidance_weight(&geom, base_emot, db, jitter, elevation, t, fear_level);
                        if (transmit < 0.01f) break;
                    }
                }
                total_r += accum_r; total_g += accum_g; total_b += accum_b;
                total_emot += sample_emot;
                total_normal_x += sample_normal.x;
                total_normal_y += sample_normal.y;
                total_normal_z += sample_normal.z;
                total_albedo_r += sample_albedo.r;
                total_albedo_g += sample_albedo.g;
                total_albedo_b += sample_albedo.b;
                samples_depth[s] = first_hit_dist;
                sample_luminance[s] = accum_r * 0.2126f + accum_g * 0.7152f + accum_b * 0.0722f;
            }

            // Estimate a posteriori relative standard error; if above threshold (e.g. 0.05), sample 2 more rays
            double rel_error = 0.0;
            bool need_more = tsfi_montecarlo_aposteriori_error_estimate(sample_luminance, 2, 0.05, &rel_error);

            if (need_more) {
                for (int s = 2; s < 4; s++) {
                    float fx = ((float)x + jx[s]) / w * 2.0f - 1.0f;
                    float fy = ((float)y + jy[s]) / h * 2.0f - 1.0f;

                    Vector3 ray_dir = v_normalize((Vector3){
                        forward.x * 1.5f + right.x * fx - up.x * fy,
                        forward.y * 1.5f + right.y * fx - up.y * fy,
                        forward.z * 1.5f + right.z * fx - up.z * fy
                    });
                    
                    Vector3 current_pos = cam_pos;
                    float accum_r = 0.01f, accum_g = 0.01f, accum_b = 0.02f;
                    float sample_emot = 0.0f;
                    Vector3 sample_normal = {0.0f, 1.0f, 0.0f};
                    Color3 sample_albedo = {0.4f, 0.25f, 0.15f};
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
                            float venation = 0.9f + 0.2f * sinf(current_pos.x * 50.0f + current_pos.y * 50.0f + (float)TSFI_SECRET_CORE);
                            Vector3 L = v_normalize(v_sub(light_pos, current_pos));
                            Vector3 V = v_normalize(v_sub(cam_pos, current_pos));
                            Vector3 N = {0, 1, 0}; Vector3 T = {1, 0, 0};
                            Color3 albedo = {1.0f * venation, 1.0f * venation, 0.95f};
                            if (!hit) {
                                first_hit_dist = current_dist;
                                hit = true;
                                sample_normal = N;
                                sample_albedo = albedo;
                            }
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
                            float jitter = sinf(current_pos.x * 100.0f + current_pos.y * 100.0f + t * 10.0f);
                            Vector3 L = v_normalize(v_sub(light_pos, current_pos));
                            Vector3 V = v_normalize(v_sub(cam_pos, current_pos));
                            Vector3 N = {0, 1, 0}; Vector3 T = {0, 1, 0};
                            Color3 albedo = {0.4f + 0.1f * jitter, 0.25f, 0.15f};
                            if (!hit) {
                                first_hit_dist = current_dist;
                                hit = true;
                                sample_normal = N;
                                sample_albedo = albedo;
                            }
                            Color3 pbr = tsfi_hair_brdf(L, V, N, T, roughness, albedo);
                            pbr.r *= (1.0f - melanin * 0.5f); pbr.g *= (1.0f - melanin * 0.7f); pbr.b *= (1.0f - melanin * 0.9f);
                            float opacity = fminf(1.0f, db * 0.9f);
                            accum_r += pbr.r * transmit * opacity;
                            accum_g += pbr.g * transmit * opacity;
                            accum_b += pbr.b * transmit * opacity;
                            transmit *= (1.0f - opacity);
                            float dist_to_face = sqrtf(current_pos.x * current_pos.x + current_pos.y * current_pos.y);
                            float shading_factor = fabsf(N.y);
                            float excitation = 0.5f + 0.5f * jitter;
                            float base_emot = expf(-dist_to_face * dist_to_face) * shading_factor * excitation;
                            sample_emot = calculate_personality_guidance_weight(&geom, base_emot, db, jitter, elevation, t, fear_level);
                            if (transmit < 0.01f) break;
                        }
                    }
                    total_r += accum_r; total_g += accum_g; total_b += accum_b;
                    total_emot += sample_emot;
                    total_normal_x += sample_normal.x;
                    total_normal_y += sample_normal.y;
                    total_normal_z += sample_normal.z;
                    total_albedo_r += sample_albedo.r;
                    total_albedo_g += sample_albedo.g;
                    total_albedo_b += sample_albedo.b;
                    samples_depth[s] = first_hit_dist;
                }
            } else {
                // Reuse first two samples to scale up to four
                total_r *= 2.0f;
                total_g *= 2.0f;
                total_b *= 2.0f;
                total_emot *= 2.0f;
                total_normal_x *= 2.0f;
                total_normal_y *= 2.0f;
                total_normal_z *= 2.0f;
                total_albedo_r *= 2.0f;
                total_albedo_g *= 2.0f;
                total_albedo_b *= 2.0f;
                samples_depth[2] = samples_depth[0];
                samples_depth[3] = samples_depth[1];
            }

            // --- Subjective MSAA Depth Resolve ---
            float final_depth = (samples_depth[0] + samples_depth[1] + samples_depth[2] + samples_depth[3]) * 0.25f;
            
            if (depth_buffer) depth_buffer[idx] = final_depth;

            if (aux_features) {
                aux_features[idx].depth = final_depth;
                float norm_mag = sqrtf(total_normal_x*total_normal_x + total_normal_y*total_normal_y + total_normal_z*total_normal_z) + 1e-6f;
                aux_features[idx].normal.x = total_normal_x / norm_mag;
                aux_features[idx].normal.y = total_normal_y / norm_mag;
                aux_features[idx].normal.z = total_normal_z / norm_mag;
                aux_features[idx].albedo.x = total_albedo_r * 0.25f;
                aux_features[idx].albedo.y = total_albedo_g * 0.25f;
                aux_features[idx].albedo.z = total_albedo_b * 0.25f;
                aux_features[idx].emotional_weight = total_emot * 0.25f;
            }

            uint8_t final_r = (uint8_t)(fminf(1.0f, powf(total_r * 0.25f, 0.4545f)) * 255);
            uint8_t final_g = (uint8_t)(fminf(1.0f, powf(total_g * 0.25f, 0.4545f)) * 255);
            uint8_t final_b = (uint8_t)(fminf(1.0f, powf(total_b * 0.25f, 0.4545f)) * 255);
            pixels[idx] = (0xFF << 24) | (final_r << 16) | (final_g << 8) | final_b;
        }
    }

    float *guidance_map = malloc(sizeof(float) * w * h);
    if (guidance_map) {
        for (int i = 0; i < w * h; i++) {
            guidance_map[i] = aux_features ? aux_features[i].emotional_weight : 0.0f;
        }
    }

    if (aux_features) {
        uint32_t *denoised_pixels = malloc(sizeof(uint32_t) * w * h);
        if (denoised_pixels) {
            if (tsfi_montecarlo_cross_bilateral_filter(pixels, aux_features, denoised_pixels, w, h, 1.5f, 0.2f)) {
                memcpy(pixels, denoised_pixels, sizeof(uint32_t) * w * h);
            }
            free(denoised_pixels);
        }
    }

    if (aux_features && guidance_map) {
        float *chan_in_lnr = malloc(sizeof(float) * w * h);
        float *chan_out_lnr = malloc(sizeof(float) * w * h);
        if (chan_in_lnr && chan_out_lnr) {
            // Red Channel
            for (int i = 0; i < w * h; i++) {
                chan_in_lnr[i] = (float)((pixels[i] >> 16) & 0xFF) / 255.0f;
            }
            if (tsfi_montecarlo_regression_denoise_lnr(chan_in_lnr, aux_features, guidance_map, chan_out_lnr, w, h, 2, 1.5f, 0.5f)) {
                for (int i = 0; i < w * h; i++) {
                    uint32_t val = (uint32_t)(fmaxf(0.0f, fminf(1.0f, chan_out_lnr[i])) * 255.0f);
                    pixels[i] = (pixels[i] & 0xFF00FFFF) | (val << 16);
                }
            }
            // Green Channel
            for (int i = 0; i < w * h; i++) {
                chan_in_lnr[i] = (float)((pixels[i] >> 8) & 0xFF) / 255.0f;
            }
            if (tsfi_montecarlo_regression_denoise_lnr(chan_in_lnr, aux_features, guidance_map, chan_out_lnr, w, h, 2, 1.5f, 0.5f)) {
                for (int i = 0; i < w * h; i++) {
                    uint32_t val = (uint32_t)(fmaxf(0.0f, fminf(1.0f, chan_out_lnr[i])) * 255.0f);
                    pixels[i] = (pixels[i] & 0xFFFF00FF) | (val << 8);
                }
            }
            // Blue Channel
            for (int i = 0; i < w * h; i++) {
                chan_in_lnr[i] = (float)(pixels[i] & 0xFF) / 255.0f;
            }
            if (tsfi_montecarlo_regression_denoise_lnr(chan_in_lnr, aux_features, guidance_map, chan_out_lnr, w, h, 2, 1.5f, 0.5f)) {
                for (int i = 0; i < w * h; i++) {
                    uint32_t val = (uint32_t)(fmaxf(0.0f, fminf(1.0f, chan_out_lnr[i])) * 255.0f);
                    pixels[i] = (pixels[i] & 0xFFFFFF00) | val;
                }
            }
        }
        if (chan_in_lnr) free(chan_in_lnr);
        if (chan_out_lnr) free(chan_out_lnr);
    }

    // Apply Non-Local Means (NLM) with guided path selection to reconstruct complex high-frequency patterns
    float *chan_in = malloc(sizeof(float) * w * h);
    float *chan_out = malloc(sizeof(float) * w * h);
    if (chan_in && chan_out && guidance_map) {
        // Red Channel
        for (int i = 0; i < w * h; i++) {
            chan_in[i] = (float)((pixels[i] >> 16) & 0xFF) / 255.0f;
        }
        if (tsfi_montecarlo_guided_path_non_local_means(chan_in, guidance_map, chan_out, w, h, 0.15f, 1, 2, 0.8f)) {
            for (int i = 0; i < w * h; i++) {
                uint32_t val = (uint32_t)(fmaxf(0.0f, fminf(1.0f, chan_out[i])) * 255.0f);
                pixels[i] = (pixels[i] & 0xFF00FFFF) | (val << 16);
            }
        }
        // Green Channel
        for (int i = 0; i < w * h; i++) {
            chan_in[i] = (float)((pixels[i] >> 8) & 0xFF) / 255.0f;
        }
        if (tsfi_montecarlo_guided_path_non_local_means(chan_in, guidance_map, chan_out, w, h, 0.15f, 1, 2, 0.8f)) {
            for (int i = 0; i < w * h; i++) {
                uint32_t val = (uint32_t)(fmaxf(0.0f, fminf(1.0f, chan_out[i])) * 255.0f);
                pixels[i] = (pixels[i] & 0xFFFF00FF) | (val << 8);
            }
        }
        // Blue Channel
        for (int i = 0; i < w * h; i++) {
            chan_in[i] = (float)(pixels[i] & 0xFF) / 255.0f;
        }
        if (tsfi_montecarlo_guided_path_non_local_means(chan_in, guidance_map, chan_out, w, h, 0.15f, 1, 2, 0.8f)) {
            for (int i = 0; i < w * h; i++) {
                uint32_t val = (uint32_t)(fmaxf(0.0f, fminf(1.0f, chan_out[i])) * 255.0f);
                pixels[i] = (pixels[i] & 0xFFFFFF00) | val;
            }
        }
    }
    if (chan_in) free(chan_in);
    if (chan_out) free(chan_out);
    if (guidance_map) free(guidance_map);
    if (aux_features) free(aux_features);
}

bool tsfi_montecarlo_render_maniac_mansion(
    uint32_t room_id,
    uint32_t *pixels_out,
    float *depth_out,
    int width,
    int height,
    float time
) {
    if (!pixels_out || width <= 0 || height <= 0) {
        return false;
    }

    TSFiHelmholtzSVDAG dag_mansion;
    TSFiHelmholtzSVDAG dag_inhabitants;
    memset(&dag_mansion, 0, sizeof(TSFiHelmholtzSVDAG));
    memset(&dag_inhabitants, 0, sizeof(TSFiHelmholtzSVDAG));

    // Allocate temporary intensity streams for voxel mapping
    float *intensity_mansion = calloc(128 * 128 * 128, sizeof(float));
    float *intensity_inhabitants = calloc(128 * 128 * 128, sizeof(float));
    if (!intensity_mansion || !intensity_inhabitants) {
        free(intensity_mansion);
        free(intensity_inhabitants);
        return false;
    }

    // Generate room floor structure at y = 0
    for (int z = 0; z < 128; z++) {
        for (int x = 0; x < 128; x++) {
            intensity_mansion[0 * 16384 + z * 128 + x] = 1.0f;
        }
    }

    // Place character box representing Z-Machine inhabitant in the center
    for (int y = 2; y < 12; y++) {
        for (int z = 60; z < 68; z++) {
            for (int x = 60; x < 68; x++) {
                intensity_inhabitants[y * 16384 + z * 128 + x] = 1.0f;
            }
        }
    }

    dag_mansion.intensity_stream = intensity_mansion;
    dag_inhabitants.intensity_stream = intensity_inhabitants;

    // Dynamically adjust Vaesen registry fear levels based on the active Z-machine room ID
    if (room_id == 13) {
        tsfi_vsen_vaesen_record_sight("Mara", "Upsala", 8);
    } else {
        tsfi_vsen_vaesen_record_sight("Tomte", "Upsala", 1);
    }

    tsfi_svdag_path_trace(
        pixels_out,
        depth_out,
        &dag_mansion,
        &dag_inhabitants,
        width,
        height,
        time,
        0.5f,
        0.3f,
        0.2f
    );

    free(intensity_mansion);
    free(intensity_inhabitants);
    return true;
}
