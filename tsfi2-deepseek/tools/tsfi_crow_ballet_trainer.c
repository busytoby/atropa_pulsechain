#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <math.h>
#include <sys/stat.h>
#include <immintrin.h>
#include <stdalign.h>
#include "tsfi_controlnet_shm.h"
#include "stable-diffusion.h"
#include "lau_memory.h"

#define W 512
#define H 512
#define STEPS 20
#define CFG 7.5f

typedef struct { float x, y; } Joint;

typedef struct {
    Joint head_top, neck_pos, body_mid, beak_base, beak_tip;
    Joint l_shoulder, r_shoulder, l_wing_tip, r_wing_tip;
    Joint l_hip, r_hip, l_foot, r_foot;
    Joint tail_base, tail_tip;
} CrowSkeleton;

static void get_posture_joints(const char *posture, float t, CrowSkeleton *out) {
    float cx = W / 2.0f;
    float cy = H * 0.6f;
    float s = 3.0f;
    out->neck_pos = (Joint){cx, cy - 20*s};
    out->head_top = (Joint){cx, cy - 35*s};
    out->body_mid = (Joint){cx, cy + 10*s};
    out->beak_base = (Joint){cx + 10*s, cy - 25*s};
    out->beak_tip = (Joint){cx + 25*s, cy - 25*s + sinf(t*2.0f)*5*s};
    out->l_shoulder = (Joint){cx - 10*s, cy - 5*s};
    out->r_shoulder = (Joint){cx + 10*s, cy - 5*s};
    out->l_wing_tip = (Joint){cx - 70*s, cy - 5*s};
    out->r_wing_tip = (Joint){cx + 70*s, cy - 5*s};
    out->l_hip = (Joint){cx - 8*s, cy + 25*s};
    out->r_hip = (Joint){cx + 8*s, cy + 25*s};
    out->l_foot = (Joint){cx - 12*s, cy + 60*s};
    out->r_foot = (Joint){cx + 12*s, cy + 60*s};
    out->tail_base = (Joint){cx - 15*s, cy + 30*s};
    out->tail_tip = (Joint){cx - 40*s, cy + 45*s};
    if (strcmp(posture, "First Position") == 0) {
        out->l_foot = (Joint){cx - 5*s, cy + 60*s}; out->r_foot = (Joint){cx + 5*s, cy + 60*s};
        out->l_wing_tip = (Joint){cx - 15*s, cy + 20*s}; out->r_wing_tip = (Joint){cx + 15*s, cy + 20*s};
    } else if (strcmp(posture, "Second Position") == 0) {
        out->l_foot = (Joint){cx - 40*s, cy + 60*s}; out->r_foot = (Joint){cx + 40*s, cy + 60*s};
        out->l_wing_tip = (Joint){cx - 80*s, cy - 5*s}; out->r_wing_tip = (Joint){cx + 80*s, cy - 5*s};
    } else if (strcmp(posture, "Third Position") == 0) {
        out->l_foot = (Joint){cx - 5*s, cy + 55*s}; out->r_foot = (Joint){cx + 5*s, cy + 65*s};
        out->l_wing_tip = (Joint){cx - 15*s, cy + 20*s}; out->r_wing_tip = (Joint){cx + 80*s, cy - 5*s};
    } else if (strcmp(posture, "Fourth Position") == 0) {
        out->l_foot = (Joint){cx - 10*s, cy + 45*s}; out->r_foot = (Joint){cx + 10*s, cy + 75*s};
        out->l_wing_tip = (Joint){cx - 15*s, cy + 20*s}; out->r_wing_tip = (Joint){cx + 20*s, cy - 80*s};
    } else if (strcmp(posture, "Fifth Position") == 0) {
        out->l_foot = (Joint){cx + 5*s, cy + 60*s}; out->r_foot = (Joint){cx - 5*s, cy + 60*s};
        out->l_wing_tip = (Joint){cx - 20*s, cy - 80*s}; out->r_wing_tip = (Joint){cx + 20*s, cy - 80*s};
    } else if (strcmp(posture, "Plier") == 0) {
        float bcy = cy + 20*s;
        out->neck_pos = (Joint){cx, bcy - 20*s}; out->head_top = (Joint){cx, bcy - 35*s}; out->body_mid = (Joint){cx, bcy + 10*s};
        out->l_foot = (Joint){cx - 30*s, bcy + 40*s}; out->r_foot = (Joint){cx + 30*s, bcy + 40*s};
        out->l_wing_tip = (Joint){cx - 10*s, bcy + 5*s}; out->r_wing_tip = (Joint){cx + 10*s, bcy + 5*s};
    } else if (strcmp(posture, "Etendre") == 0) {
        float bcy = cy - 20*s;
        out->neck_pos = (Joint){cx, bcy - 20*s}; out->head_top = (Joint){cx, bcy - 45*s};
        out->l_wing_tip = (Joint){cx - 90*s, bcy - 40*s}; out->r_wing_tip = (Joint){cx + 90*s, bcy - 40*s};
    } else if (strcmp(posture, "Relever") == 0) {
        out->l_foot = (Joint){cx - 12*s, cy + 75*s}; out->r_foot = (Joint){cx + 12*s, cy + 75*s};
        out->l_wing_tip = (Joint){cx - 60*s, cy + 0*s}; out->r_wing_tip = (Joint){cx + 60*s, cy + 0*s};
    } else if (strcmp(posture, "Reverence") == 0) {
        out->head_top = (Joint){cx + 20*s, cy + 10*s}; out->neck_pos = (Joint){cx + 10*s, cy + 0*s};
        out->beak_base = (Joint){cx + 30*s, cy + 20*s}; out->beak_tip = (Joint){cx + 45*s, cy + 30*s};
        out->l_wing_tip = (Joint){cx - 50*s, cy - 30*s}; out->r_wing_tip = (Joint){cx + 10*s, cy + 40*s};
    }
}

static void draw_sphere_rgb_zmm(uint8_t *data, Joint c, float radius, uint8_t r, uint8_t g, uint8_t b) {
    int rx = (int)radius;
    float r2 = radius * radius;
    __m512 vr2 = _mm512_set1_ps(r2);
    __m512 vcx = _mm512_set1_ps(c.x);
    for (int y = -rx; y <= rx; y++) {
        int ny = (int)c.y + y;
        if (ny < 0 || ny >= H) continue;
        float dy = (float)y;
        __m512 vdy2 = _mm512_set1_ps(dy * dy);
        for (int x = -rx; x <= rx; x += 16) {
            __m512 vx = _mm512_add_ps(_mm512_set1_ps(c.x + x), _mm512_set_ps(15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0));
            __m512 vdx = _mm512_sub_ps(vx, vcx);
            __m512 vdist2 = _mm512_fmadd_ps(vdx, vdx, vdy2);
            __mmask16 mask = _mm512_cmp_ps_mask(vdist2, vr2, _CMP_LE_OQ);
            if (mask == 0) continue;
            for(int k=0; k<16; k++) {
                if (mask & (1 << k)) {
                    int nx = (int)c.x + x + k;
                    if (nx >= 0 && nx < W) {
                        int idx = (ny * W + nx) * 3;
                        data[idx] = r; data[idx+1] = g; data[idx+2] = b;
                    }
                }
            }
        }
    }
}

static void draw_sphere_depth_zmm(uint8_t *data, Joint c, float radius, uint8_t z_val) {
    int rx = (int)radius;
    float r2 = radius * radius;
    __m512 vr2 = _mm512_set1_ps(r2);
    __m512 vcx = _mm512_set1_ps(c.x);
    __m512 vz_val = _mm512_set1_ps(z_val);
    __m512 v255 = _mm512_set1_ps(255.0f);
    for (int y = -rx; y <= rx; y++) {
        int ny = (int)c.y + y;
        if (ny < 0 || ny >= H) continue;
        float dy = (float)y;
        __m512 vdy2 = _mm512_set1_ps(dy * dy);
        for (int x = -rx; x <= rx; x += 16) {
            __m512 vx = _mm512_add_ps(_mm512_set1_ps(c.x + x), _mm512_set_ps(15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0));
            __m512 vdx = _mm512_sub_ps(vx, vcx);
            __m512 vdist2 = _mm512_fmadd_ps(vdx, vdx, vdy2);
            __mmask16 mask = _mm512_cmp_ps_mask(vdist2, vr2, _CMP_LE_OQ);
            if (mask == 0) continue;
            __m512 vdz = _mm512_maskz_sqrt_ps(mask, _mm512_sub_ps(vr2, vdist2));
            __m512 vval = _mm512_min_ps(_mm512_add_ps(vz_val, vdz), v255);
            alignas(64) float results[16];
            _mm512_storeu_ps(results, vval);
            for(int k=0; k<16; k++) {
                if (mask & (1 << k)) {
                    int nx = (int)c.x + x + k;
                    if (nx >= 0 && nx < W) {
                        int idx = (ny * W + nx) * 3;
                        uint8_t val = (uint8_t)results[k];
                        if (val > data[idx]) {
                            data[idx] = val; data[idx+1] = val; data[idx+2] = val;
                        }
                    }
                }
            }
        }
    }
}

static void draw_line_zmm(uint8_t *data, Joint p1, Joint p2, uint8_t r, uint8_t g, uint8_t b, int thick) {
    float dx = p2.x - p1.x; float dy = p2.y - p1.y;
    float len = sqrtf(dx*dx + dy*dy);
    if (len < 1.0f) return;
    dx /= len; dy /= len;
    for (float i = 0; i <= len; i += 0.5f) {
        Joint c = { p1.x + dx * i, p1.y + dy * i };
        draw_sphere_rgb_zmm(data, c, (float)thick, r, g, b);
    }
}

static void draw_skeleton(uint8_t *data, const CrowSkeleton *s) {
    memset(data, 0, W * H * 3);
    draw_line_zmm(data, s->head_top, s->neck_pos, 255, 0, 0, 3);
    draw_line_zmm(data, s->head_top, s->beak_base, 255, 0, 0, 2);
    draw_line_zmm(data, s->beak_base, s->beak_tip, 255, 0, 255, 4);
    draw_line_zmm(data, s->neck_pos, s->body_mid, 255, 85, 0, 4);
    draw_line_zmm(data, s->body_mid, s->tail_base, 255, 170, 0, 3);
    draw_line_zmm(data, s->tail_base, s->tail_tip, 255, 255, 0, 3);
    draw_line_zmm(data, s->neck_pos, s->l_shoulder, 170, 255, 0, 2);
    draw_line_zmm(data, s->neck_pos, s->r_shoulder, 85, 255, 0, 2);
    draw_line_zmm(data, s->l_shoulder, s->l_wing_tip, 0, 255, 0, 3);
    draw_line_zmm(data, s->r_shoulder, s->r_wing_tip, 0, 255, 85, 3);
    draw_line_zmm(data, s->body_mid, s->l_hip, 0, 255, 170, 2);
    draw_line_zmm(data, s->body_mid, s->r_hip, 0, 255, 255, 2);
    draw_line_zmm(data, s->l_hip, s->l_foot, 0, 170, 255, 2);
    draw_line_zmm(data, s->r_hip, s->r_foot, 0, 85, 255, 2);
}

static void draw_depth(uint8_t *data, const CrowSkeleton *s) {
    memset(data, 0, W * H * 3);
    float sc = 3.0f;
    draw_sphere_depth_zmm(data, s->head_top, 18*sc, 180);
    draw_sphere_depth_zmm(data, s->neck_pos, 15*sc, 160);
    draw_sphere_depth_zmm(data, s->body_mid, 30*sc, 140);
    draw_sphere_depth_zmm(data, s->tail_base, 20*sc, 130);
}

int main() {
    printf("=== TSFi C-Native Crow Ballet Training Studio (Turbo-Persistent Optimized) ===\n");
    sd_ctx_params_t params;
    sd_ctx_params_init(&params);
    params.model_path = "assets/models/sd_turbo.safetensors";
    params.vae_path = "assets/models/taesd.safetensors";
    params.control_net_path = "assets/models/control_depth.safetensors";
    params.control_net_path_2 = "assets/models/control_openpose.safetensors";
    params.n_threads = 16;
    params.wtype = SD_TYPE_F32;
    params.rng_type = CUDA_RNG;
    params.sampler_rng_type = CUDA_RNG;
    params.enable_mmap = true;
    params.free_params_immediately = true;

    const char *postures[] = {"First Position", "Second Position", "Third Position", "Fourth Position", "Fifth Position", "Plier", "Etendre", "Relever", "Reverence"};
    const char *tastes[][2] = {
        {"QUOTH", "beak open in a cawing expression, throat feathers ruffled, expressive face, synthetic plumage texture detail"},
        {"SYMPATHY", "slightly tilted head, soft melting synthetic feathers, empathetic synthetic eyes, gentle expression, matted faux fur depth"},
        {"FLIGHT", "dynamic wing extension, individual synthetic flight feathers catching light, overlapping plush feather scales, kinetic energy, powerful silhouette"}
    };

    uint8_t *pose_buf = (uint8_t*)malloc(W * H * 3);
    uint8_t *depth_buf = (uint8_t*)malloc(W * H * 3);
    char out_dir[] = "assets/crow_ballet_c_training";
    mkdir(out_dir, 0777);

    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    for (int p = 0; p < 9; p++) {
        CrowSkeleton skel;
        get_posture_joints(postures[p], 0.0f, &skel);
        draw_skeleton(pose_buf, &skel);
        draw_depth(depth_buf, &skel);
        for (int t = 0; t < 3; t++) {
            printf("[TRAIN] %s | %s\n", postures[p], tastes[t][0]);
            sd_ctx_t* ctx = new_sd_ctx(&params);
            if (!ctx) continue;
            sd_img_gen_params_t gen;
            sd_img_gen_params_init(&gen);
            char prompt[1024];
            snprintf(prompt, 1024, "extreme macro photography of a stuffed animal crow plush, hyper-detailed individual synthetic feathers, matted black faux-plumage textures, visible stitching, soft plush under-down, studio lighting catching iridescent sheen, neutral background, 8k resolution masterpiece, %s", tastes[t][1]);
            gen.prompt = prompt;
            gen.width = W; gen.height = H;
            gen.sample_params.sample_steps = STEPS;
            gen.sample_params.guidance.txt_cfg = CFG;
            gen.sample_params.sample_method = EULER_A_SAMPLE_METHOD;
            gen.sample_params.scheduler = DISCRETE_SCHEDULER;
            gen.vae_tiling_params.enabled = true;
            gen.vae_tiling_params.tile_size_x = 256;
            gen.vae_tiling_params.tile_size_y = 256;
            gen.control_image.width = W; gen.control_image.height = H;
            gen.control_image.channel = 3; gen.control_image.data = depth_buf;
            gen.control_strength = 0.9f;
            gen.control_image_2.width = W; gen.control_image_2.height = H;
            gen.control_image_2.channel = 3; gen.control_image_2.data = pose_buf;
            gen.control_strength_2 = 0.8f;
            sd_image_t *res = generate_image(ctx, &gen);
            if (res && res->data) {
                char path[256];
                snprintf(path, 256, "%s/%d_%d.raw", out_dir, p, t);
                FILE *f = fopen(path, "wb");
                fwrite(res->data, 1, W * H * 3, f);
                fclose(f);
                free(res->data);
                free(res);
            }
            free_sd_ctx(ctx);
        }
    }
    clock_gettime(CLOCK_MONOTONIC, &end);
    double elapsed = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
    printf("\n[COMPLETE] 27 High-Quality frames rendered in %.2f seconds (Avg: %.2fs/frame)\n", elapsed, elapsed/27.0);
    free(pose_buf); free(depth_buf);
    return 0;
}
