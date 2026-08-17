/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Native TSFi CCX Thread Dispatcher Solid Terracotta Clay Bear 3D Raymarcher
 * 
 * Hardware-Aware Architecture:
 * - Direct integration with TSFi CCX Pool (`tsfi_ccx_pool.h` / `tsfi_ccx_pool.c`).
 * - Dispatches across CCX clusters with physical CPU core affinity and zero OpenMP.
 * - Fixed Studio Camera: Front-facing stationary 3/4 perspective (Zero horizontal scrolling).
 * - Solid Volumetric Clay SDF: Organic sculpted earthenware clay teddy bear on porcelain pedestal.
 * - Continuous Studio Bionika Audio: Phase-continuous "Eye of the Tiger" AAC 192k.
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include "../tsfi2-deepseek/inc/tsfi_ccx_pool.h"

#define WIDTH 1280
#define HEIGHT 720
#define FPS 30
#define DURATION_SEC 90
#define TOTAL_FRAMES (FPS * DURATION_SEC)
#define SAMPLE_RATE 44100
#define SAMPLES_PER_FRAME (SAMPLE_RATE / FPS)
#define NUM_CCX_CLUSTERS 4
#define THREADS_PER_CCX 4
#define TOTAL_SLICES 16

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

typedef struct { uint8_t r, g, b; } pixel_t;
typedef struct { float x, y, z; } vec3;

static pixel_t framebuffer[WIDTH * HEIGHT];
static int16_t audio_buffer[SAMPLES_PER_FRAME * 2];

static inline vec3 v_add(vec3 a, vec3 b) { return (vec3){a.x + b.x, a.y + b.y, a.z + b.z}; }
static inline vec3 v_sub(vec3 a, vec3 b) { return (vec3){a.x - b.x, a.y - b.y, a.z - b.z}; }
static inline vec3 v_scale(vec3 a, float s) { return (vec3){a.x * s, a.y * s, a.z * s}; }
static inline float v_dot(vec3 a, vec3 b) { return a.x * b.x + a.y * b.y + a.z * b.z; }
static inline float v_length(vec3 a) { return sqrtf(v_dot(a, a)); }
static inline vec3 v_cross(vec3 a, vec3 b) {
    return (vec3){a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x};
}
static inline vec3 v_normalize(vec3 a) {
    float l = v_length(a);
    return l > 0.00001f ? v_scale(a, 1.0f / l) : (vec3){0, 0, 0};
}

static inline float smin(float a, float b, float k) {
    float h = fmaxf(k - fabsf(a - b), 0.0f) / k;
    return fminf(a, b) - h * h * h * (k * (1.0f / 6.0f));
}

static inline float sd_sphere(vec3 p, float r) {
    return v_length(p) - r;
}

static inline float sd_ellipsoid(vec3 p, vec3 r) {
    float k0 = v_length((vec3){p.x / r.x, p.y / r.y, p.z / r.z});
    float k1 = v_length((vec3){p.x / (r.x * r.x), p.y / (r.y * r.y), p.z / (r.z * r.z)});
    return k0 * (k0 - 1.0f) / (k1 > 0.0001f ? k1 : 1.0f);
}

static inline float sd_capsule(vec3 p, vec3 a, vec3 b, float r) {
    vec3 pa = v_sub(p, a), ba = v_sub(b, a);
    float h = fmaxf(fminf(v_dot(pa, ba) / v_dot(ba, ba), 1.0f), 0.0f);
    return v_length(v_sub(pa, v_scale(ba, h))) - r;
}

static float map_clay_bear(vec3 p, float time) {
    float d_torso = sd_ellipsoid(v_sub(p, (vec3){0.0f, -0.2f, 0.0f}), (vec3){0.65f, 0.75f, 0.60f});

    float head_tilt = sinf(time * 2.0f) * 0.03f;
    vec3 head_pos = (vec3){head_tilt, 0.70f, 0.0f};
    float d_head = sd_sphere(v_sub(p, head_pos), 0.52f);
    float d_bear = smin(d_torso, d_head, 0.18f);

    vec3 snout_pos = (vec3){head_tilt, 0.58f, 0.40f};
    float d_muzzle = sd_ellipsoid(v_sub(p, snout_pos), (vec3){0.22f, 0.18f, 0.20f});
    d_bear = smin(d_bear, d_muzzle, 0.08f);

    vec3 ear_l_pos = (vec3){-0.42f + head_tilt, 1.10f, -0.05f};
    vec3 ear_r_pos = (vec3){ 0.42f + head_tilt, 1.10f, -0.05f};
    float d_ear_l = sd_ellipsoid(v_sub(p, ear_l_pos), (vec3){0.18f, 0.18f, 0.08f});
    float d_ear_r = sd_ellipsoid(v_sub(p, ear_r_pos), (vec3){0.18f, 0.18f, 0.08f});
    float d_ears = fminf(d_ear_l, d_ear_r);
    d_bear = smin(d_bear, d_ears, 0.06f);

    float wave = sinf(time * 5.0f) * 0.35f + 0.2f;
    vec3 arm_l_start = (vec3){-0.50f, 0.25f, 0.0f};
    vec3 arm_l_end   = (vec3){-0.85f, 0.55f + wave * 0.4f, 0.35f + wave * 0.2f};
    float d_arm_l = sd_capsule(p, arm_l_start, arm_l_end, 0.16f);

    vec3 arm_r_start = (vec3){ 0.50f, 0.25f, 0.0f};
    vec3 arm_r_end   = (vec3){ 0.85f, -0.30f, 0.15f};
    float d_arm_r = sd_capsule(p, arm_r_start, arm_r_end, 0.16f);
    float d_arms = fminf(d_arm_l, d_arm_r);
    d_bear = smin(d_bear, d_arms, 0.12f);

    vec3 leg_l_start = (vec3){-0.40f, -0.65f, 0.10f};
    vec3 leg_l_end   = (vec3){-0.50f, -0.75f, 0.65f};
    float d_leg_l = sd_capsule(p, leg_l_start, leg_l_end, 0.20f);

    vec3 leg_r_start = (vec3){ 0.40f, -0.65f, 0.10f};
    vec3 leg_r_end   = (vec3){ 0.50f, -0.75f, 0.65f};
    float d_leg_r = sd_capsule(p, leg_r_start, leg_r_end, 0.20f);
    float d_legs = fminf(d_leg_l, d_leg_r);
    d_bear = smin(d_bear, d_legs, 0.15f);

    float d_pedestal = sd_ellipsoid(v_sub(p, (vec3){0.0f, -1.15f, 0.0f}), (vec3){1.6f, 0.22f, 1.6f});
    return fminf(d_bear, d_pedestal);
}

static vec3 calc_normal(vec3 p, float time) {
    float eps = 0.002f;
    return v_normalize((vec3){
        map_clay_bear((vec3){p.x + eps, p.y, p.z}, time) - map_clay_bear((vec3){p.x - eps, p.y, p.z}, time),
        map_clay_bear((vec3){p.x, p.y + eps, p.z}, time) - map_clay_bear((vec3){p.x, p.y - eps, p.z}, time),
        map_clay_bear((vec3){p.x, p.y, p.z + eps}, time) - map_clay_bear((vec3){p.x, p.y, p.z - eps}, time)
    });
}

typedef struct {
    int start_y;
    int end_y;
    float time;
} ccx_worker_arg_t;

static void render_slice_ccx_task(void *arg) {
    ccx_worker_arg_t *w = (ccx_worker_arg_t *)arg;
    float t = w->time;

    vec3 ro = (vec3){ 0.4f, 0.45f, 3.4f };
    vec3 ta = (vec3){ 0.0f, 0.10f, 0.0f };
    vec3 up = (vec3){ 0.0f, 1.0f, 0.0f };

    vec3 ww = v_normalize(v_sub(ta, ro));
    vec3 uu = v_normalize(v_cross(ww, up));
    vec3 vv = v_cross(uu, ww);

    vec3 light1_dir = v_normalize((vec3){0.8f, 1.2f, 0.9f});
    vec3 light2_dir = v_normalize((vec3){-0.7f, 0.4f, -0.8f});

    for (int y = w->start_y; y < w->end_y; y++) {
        for (int x = 0; x < WIDTH; x++) {
            float px = ((float)x - WIDTH * 0.5f) / (HEIGHT * 0.5f);
            float py = -((float)y - HEIGHT * 0.5f) / (HEIGHT * 0.5f);

            vec3 rd = v_normalize(v_add(v_add(v_scale(uu, px), v_scale(vv, py)), v_scale(ww, 1.8f)));

            float depth = 0.5f;
            float max_depth = 7.0f;
            bool hit = false;
            vec3 p = ro;

            for (int step = 0; step < 72; step++) {
                p = v_add(ro, v_scale(rd, depth));
                float d = map_clay_bear(p, t);
                if (d < 0.002f) {
                    hit = true;
                    break;
                }
                depth += d;
                if (depth > max_depth) break;
            }

            if (hit) {
                vec3 n = calc_normal(p, t);
                bool is_pedestal = (p.y < -0.95f);

                float diff1 = fmaxf(v_dot(n, light1_dir), 0.0f);
                float diff2 = fmaxf(v_dot(n, light2_dir), 0.0f) * 0.4f;
                float amb = 0.25f + 0.15f * n.y;

                vec3 ref = v_sub(v_scale(n, 2.0f * v_dot(n, light1_dir)), light1_dir);
                float spec = powf(fmaxf(-v_dot(ref, rd), 0.0f), 24.0f) * 0.6f;

                float r, g, b;
                if (is_pedestal) {
                    r = (0.15f * diff1 + 0.10f * diff2 + 0.08f * amb + spec * 0.8f);
                    g = (0.20f * diff1 + 0.15f * diff2 + 0.10f * amb + spec * 0.8f);
                    b = (0.30f * diff1 + 0.25f * diff2 + 0.18f * amb + spec * 0.8f);
                } else {
                    r = (0.82f * diff1 + 0.40f * diff2 + 0.35f * amb + spec);
                    g = (0.45f * diff1 + 0.22f * diff2 + 0.20f * amb + spec * 0.9f);
                    b = (0.28f * diff1 + 0.15f * diff2 + 0.14f * amb + spec * 0.7f);
                }

                r = sqrtf(fminf(r, 1.0f));
                g = sqrtf(fminf(g, 1.0f));
                b = sqrtf(fminf(b, 1.0f));

                framebuffer[y * WIDTH + x] = (pixel_t){ (uint8_t)(r * 255.0f), (uint8_t)(g * 255.0f), (uint8_t)(b * 255.0f) };
            } else {
                float bg_t = ((float)y / (float)HEIGHT);
                uint8_t bg_r = (uint8_t)(25.0f + 35.0f * (1.0f - bg_t));
                uint8_t bg_g = (uint8_t)(28.0f + 40.0f * (1.0f - bg_t));
                uint8_t bg_b = (uint8_t)(38.0f + 50.0f * (1.0f - bg_t));
                framebuffer[y * WIDTH + x] = (pixel_t){ bg_r, bg_g, bg_b };
            }
        }
    }
}

static void render_frame_ccx(TSFiCCXPool **pools, int num_pools, int frame) {
    float t = (float)frame / (float)FPS;
    ccx_worker_arg_t args[TOTAL_SLICES];
    int slice_h = HEIGHT / TOTAL_SLICES;

    for (int i = 0; i < TOTAL_SLICES; i++) {
        args[i].start_y = i * slice_h;
        args[i].end_y = (i == TOTAL_SLICES - 1) ? HEIGHT : (i + 1) * slice_h;
        args[i].time = t;
        int pool_idx = i % num_pools;
        tsfi_ccx_pool_enqueue(pools[pool_idx], render_slice_ccx_task, &args[i]);
    }

    for (int p = 0; p < num_pools; p++) {
        tsfi_ccx_pool_wait(pools[p]);
    }
}

static double b_phase = 0.0;
static double l_phase = 0.0;
static double l5_phase = 0.0;
static double f_prev = 0.0;

static float get_note_freq(const char *n) {
    if (!strcmp(n, "C2")) return 65.41f;
    if (!strcmp(n, "Eb2")) return 77.78f;
    if (!strcmp(n, "F2")) return 87.31f;
    if (!strcmp(n, "G2")) return 98.00f;
    if (!strcmp(n, "Ab1")) return 51.91f;
    if (!strcmp(n, "Bb1")) return 58.27f;
    if (!strcmp(n, "C3")) return 130.81f;
    if (!strcmp(n, "Eb3")) return 155.56f;
    if (!strcmp(n, "C4")) return 261.63f;
    if (!strcmp(n, "Eb4")) return 311.13f;
    if (!strcmp(n, "F4")) return 349.23f;
    if (!strcmp(n, "G4")) return 392.00f;
    if (!strcmp(n, "Ab3")) return 207.65f;
    if (!strcmp(n, "Bb3")) return 233.08f;
    return 0.0f;
}

static void generate_audio(int frame, int16_t *stereo_out) {
    float bpm = 109.6f;
    float step_sec = (60.0f / bpm) / 4.0f;
    
    static const char *bass_notes[32] = {
        "C2", "C2", "C2", "C2", "C2", "C2", "C2", "C2",
        "C2", "C2", "Bb1", "C2", "C2", "C2", "C2", "C2",
        "C2", "C2", "Bb1", "C2", "C2", "C2", "C2", "C2",
        "C2", "C2", "Bb1", "C2", "C2", "Ab1", "Bb1", "C2"
    };
    static const int bass_accents[32] = {
        1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 1, 0, 0, 0, 0,
        1, 0, 1, 1, 0, 0, 0, 0, 1, 0, 1, 1, 0, 1, 1, 1
    };
    static const char *lead_notes[32] = {
        "C4", "C4", "C4", "C4", "C4", "C4", "C4", "C4",
        "C4", "C4", "Bb3", "C4", "", "", "", "",
        "", "", "Bb3", "C4", "", "", "", "",
        "", "", "Bb3", "C4", "", "Ab3", "Bb3", "C4"
    };
    static const int drum_kick[32] = {
        1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0,
        1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0
    };
    static const int drum_snare[32] = {
        0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0,
        0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0
    };

    for (int s = 0; s < SAMPLES_PER_FRAME; s++) {
        float t = (float)(frame * SAMPLES_PER_FRAME + s) / (float)SAMPLE_RATE;
        int step = (int)(t / step_sec) % 32;
        float step_progress = fmodf(t, step_sec) / step_sec;

        float bf = get_note_freq(bass_notes[step]);
        b_phase += (double)bf / (double)SAMPLE_RATE;
        if (b_phase >= 1.0) b_phase -= 1.0;

        float bsaw = 2.0f * (float)b_phase - 1.0f;
        float bpulse = (b_phase < 0.45) ? 1.0f : -1.0f;
        float benv = expf(-step_progress * (bass_accents[step] ? 3.5f : 6.0f));
        float raw_b = (0.55f * bsaw + 0.45f * bpulse) * benv * (bass_accents[step] ? 0.75f : 0.45f);

        f_prev += 0.25 * ((double)raw_b - f_prev);
        float b_out = (float)f_prev;

        float l_out = 0.0f;
        if (strlen(lead_notes[step]) > 0) {
            float lf = get_note_freq(lead_notes[step]);
            l_phase += (double)lf / (double)SAMPLE_RATE;
            if (l_phase >= 1.0) l_phase -= 1.0;
            l5_phase += ((double)lf * 1.5) / (double)SAMPLE_RATE;
            if (l5_phase >= 1.0) l5_phase -= 1.0;

            float lsaw = 2.0f * (float)l_phase - 1.0f;
            float l5th = sinf(2.0f * (float)M_PI * (float)l5_phase);
            float lenv = expf(-step_progress * 4.2f);
            l_out = tanhf((0.7f * lsaw + 0.3f * l5th) * 2.0f) * lenv * 0.6f;
        }

        float k_out = 0.0f;
        if (drum_kick[step]) {
            float k_freq = 135.0f * expf(-step_progress * 24.0f) + 48.0f;
            k_out = sinf(2.0f * (float)M_PI * k_freq * step_progress * step_sec) * expf(-step_progress * 18.0f) * 0.85f;
        }

        float sn_out = 0.0f;
        if (drum_snare[step]) {
            float noise = ((float)rand() / (float)RAND_MAX) * 2.0f - 1.0f;
            sn_out = (noise * 0.7f + sinf(2.0f * (float)M_PI * 220.0f * step_progress * step_sec) * 0.3f) * expf(-step_progress * 14.0f) * 0.7f;
        }

        float hh_out = (((float)rand() / (float)RAND_MAX) * 2.0f - 1.0f) * expf(-step_progress * 30.0f) * 0.15f;

        float mix_l = b_out * 0.65f + l_out * 0.8f + k_out * 0.75f + sn_out * 0.65f + hh_out * 0.45f;
        float mix_r = b_out * 0.65f + l_out * 0.72f + k_out * 0.75f + sn_out * 0.65f + hh_out * 0.55f;

        if (mix_l > 0.95f) mix_l = 0.95f;
        if (mix_l < -0.95f) mix_l = -0.95f;
        if (mix_r > 0.95f) mix_r = 0.95f;
        if (mix_r < -0.95f) mix_r = -0.95f;

        stereo_out[s * 2] = (int16_t)(mix_l * 30000.0f);
        stereo_out[s * 2 + 1] = (int16_t)(mix_r * 30000.0f);
    }
}

int main(void) {
    printf("=============================================================\n");
    printf("RENDERING TSFI CCX THREAD DISPATCHED SOLID CLAY BEAR MP4     \n");
    printf("=============================================================\n");

    TSFiCCXPool pools[NUM_CCX_CLUSTERS];
    TSFiCCXPool *pool_ptrs[NUM_CCX_CLUSTERS];

    for (int c = 0; c < NUM_CCX_CLUSTERS; c++) {
        tsfi_ccx_pool_init(&pools[c], c, THREADS_PER_CCX);
        pool_ptrs[c] = &pools[c];
    }
    printf("   ✓ Initialized %d CCX Pools (%d threads per cluster, %d threads total)\n",
           NUM_CCX_CLUSTERS, THREADS_PER_CCX, NUM_CCX_CLUSTERS * THREADS_PER_CCX);

    const char *cmd = "ffmpeg -y -f rawvideo -pix_fmt rgb24 -s 1280x720 -r 30 -i - "
                      "-f s16le -ar 44100 -ac 2 -i - "
                      "-c:v libx264 -pix_fmt yuv420p -preset fast -crf 18 "
                      "-c:a aac -b:a 192k clayscape_bear_solid_demo.mp4 > /dev/null 2>&1";

    FILE *pipe = popen(cmd, "w");
    if (!pipe) {
        fprintf(stderr, "Error opening pipe\n");
        return 1;
    }

    for (int f = 0; f < TOTAL_FRAMES; f++) {
        render_frame_ccx(pool_ptrs, NUM_CCX_CLUSTERS, f);
        generate_audio(f, audio_buffer);

        fwrite(framebuffer, sizeof(pixel_t), WIDTH * HEIGHT, pipe);
        fwrite(audio_buffer, sizeof(int16_t) * 2, SAMPLES_PER_FRAME, pipe);

        if (f % 150 == 0) {
            printf("   -> TSFi CCX Raymarched %d/%d frames (%.1f%%, Time: %.1f sec)...\n",
                   f, TOTAL_FRAMES, (float)f / (float)TOTAL_FRAMES * 100.0f, (float)f / 30.0f);
            fflush(stdout);
        }
    }

    pclose(pipe);

    for (int c = 0; c < NUM_CCX_CLUSTERS; c++) {
        tsfi_ccx_pool_destroy(&pools[c]);
    }

    printf("=============================================================\n");
    printf("✓ TSFI CCX SOLID CLAY BEAR RENDER COMPLETE: clayscape_bear_solid_demo.mp4\n");
    printf("=============================================================\n");
    return 0;
}
