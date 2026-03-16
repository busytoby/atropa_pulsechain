#include "lau_memory.h"
#include "tsfi_k0rn_ops.h"
#include "tsfi_genetic.h"
#include "tsfi_io.h"
#include "tsfi_vision.h"
#include "vulkan/vulkan_render.h"
#include "vulkan/vulkan_main.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <time.h>
#include <immintrin.h>
#include <stdalign.h>
#include "tsfi_raw.h"

static inline __m512 tsfi_sin_ps_local(__m512 x) {
    __m512 v_2pi = _mm512_set1_ps(6.2831853f);
    __m512 v_inv2pi = _mm512_set1_ps(0.1591549f);
    __m512 q = _mm512_roundscale_ps(_mm512_mul_ps(x, v_inv2pi), _MM_FROUND_TO_NEAREST_INT | _MM_FROUND_NO_EXC);
    x = _mm512_fnmadd_ps(q, v_2pi, x);
    __m512 x2 = _mm512_mul_ps(x, x);
    return _mm512_sub_ps(x, _mm512_mul_ps(_mm512_mul_ps(x, x2), _mm512_set1_ps(0.166666f)));
}

static inline __m512 tsfi_atan2_ps_local(__m512 y, __m512 x) {
    __m512 abs_y = _mm512_abs_ps(y), abs_x = _mm512_abs_ps(x);
    __m512 a = _mm512_div_ps(_mm512_min_ps(abs_x, abs_y), _mm512_max_ps(abs_x, abs_y));
    __m512 s = _mm512_mul_ps(a, a);
    __m512 r = _mm512_fmadd_ps(_mm512_set1_ps(-0.330299f), s, _mm512_set1_ps(0.999866f));
    return _mm512_mul_ps(r, a); 
}

static inline __m512 render_stream_density(const K0RnStream *s, __m512 vx, __m512 vy, __m512 perturb, float k_val) {
    __m512 d = _mm512_set1_ps(1e5f);
    __m512 v_one = _mm512_set1_ps(1.0f), v_zero = _mm512_setzero_ps();
    __m512 v_k = _mm512_set1_ps(k_val);
    for (uint32_t i = 0; i < s->op_count; i++) {
        K0RnOp *op = &s->ops[i];
        __m512 pax = _mm512_set1_ps(op->x), pay = _mm512_set1_ps(op->y);
        __m512 pbx = _mm512_set1_ps(op->z), pby = _mm512_set1_ps(op->w);
        __m512 rad = _mm512_set1_ps(op->p1);
        __m512 cur_d;
        if (op->type == K0RN_OP_CAPSULE) {
            __m512 ba_x = _mm512_sub_ps(pbx, pax), ba_y = _mm512_sub_ps(pby, pay);
            __m512 pa_dx = _mm512_sub_ps(vx, pax), pa_dy = _mm512_sub_ps(vy, pay);
            __m512 h = _mm512_div_ps(_mm512_add_ps(_mm512_mul_ps(pa_dx, ba_x), _mm512_mul_ps(pa_dy, ba_y)), 
                                     _mm512_max_ps(_mm512_add_ps(_mm512_mul_ps(ba_x, ba_x), _mm512_mul_ps(ba_y, ba_y)), _mm512_set1_ps(1e-6f)));
            h = _mm512_min_ps(v_one, _mm512_max_ps(v_zero, h));
            __m512 dx = _mm512_sub_ps(pa_dx, _mm512_mul_ps(ba_x, h)), dy = _mm512_sub_ps(pa_dy, _mm512_mul_ps(ba_y, h));
            cur_d = _mm512_sub_ps(_mm512_sqrt_ps(_mm512_add_ps(_mm512_mul_ps(dx, dx), _mm512_mul_ps(dy, dy))), rad);
        } else {
            __m512 vdx = _mm512_sub_ps(vx, pax), vdy = _mm512_sub_ps(vy, pay);
            __m512 dist = _mm512_sqrt_ps(_mm512_add_ps(_mm512_mul_ps(vdx, vdx), _mm512_mul_ps(vdy, vdy)));
            if (op->type == K0RN_OP_SPHERE) rad = _mm512_add_ps(rad, perturb);
            cur_d = _mm512_sub_ps(dist, rad);
        }
        if (op->type == K0RN_OP_SUB) d = _mm512_max_ps(d, _mm512_sub_ps(v_zero, cur_d));
        else {
            __m512 soft_h = _mm512_max_ps(v_zero, _mm512_sub_ps(v_k, _mm512_abs_ps(_mm512_sub_ps(d, cur_d))));
            d = _mm512_sub_ps(_mm512_min_ps(d, cur_d), _mm512_mul_ps(soft_h, _mm512_mul_ps(soft_h, _mm512_div_ps(_mm512_set1_ps(0.25f), _mm512_max_ps(v_k, _mm512_set1_ps(1e-6f))))));
        }
    }
    return d;
}

K0RnStream* evolve_pole_dna(const char *species, int seed) {
    TSFiFlowerPhenotype p;
    if (species && !tsfi_phenotype_load(species, &p)) return NULL;
    srand(seed);
    K0RnStream *s = (K0RnStream*)lau_malloc_wired(sizeof(K0RnStream));
    s->stream_capacity = 32; s->op_count = 16;
    s->ops = (K0RnOp*)lau_memalign(512, sizeof(K0RnOp) * s->stream_capacity);
    for(int i=0; i<16; i++) {
        s->ops[i].type = K0RN_OP_SPHERE;
        s->ops[i].x = ((float)rand()/RAND_MAX)*0.3f-0.15f;
        s->ops[i].y = ((float)rand()/RAND_MAX)*0.3f-0.15f;
        s->ops[i].p1 = 0.08f; s->ops[i].p2 = 0;
    }
    s->ops[0].type = K0RN_OP_SUB; s->ops[0].x = 0; s->ops[0].y = 0; s->ops[0].p1 = 0.15f;
    TSFiResonanceAnalysis vis; memset(&vis, 0, sizeof(vis));
    for (int gen = 0; gen < 100; gen++) { tsfi_k0rn_evolve_vision(s, &vis, 1.0f); s->ops[0].type = K0RN_OP_SUB; }
    return s;
}

typedef struct {
    K0RnStream *dna;
    float core[3], body[3], outer[3];
    float trident, curling;
    float k_val;
    float coord_scale;
    bool is_flower;
    char species_file[128];
} PoleState;

const char* g_flower_files[] = {
    "assets/flowers/convolvulus.flower", "assets/flowers/distimake.flower",
    "assets/flowers/hewittia.flower", "assets/flowers/hyalocystis.flower",
    "assets/flowers/ipomoea.flower", "assets/flowers/merremia.flower",
    "assets/flowers/operculina.flower", "assets/flowers/remirema.flower",
    "assets/flowers/xenostegia.flower"
};

void load_pole(int pole_idx, PoleState *out) {
    memset(out, 0, sizeof(PoleState));
    if (pole_idx == 0) {
        out->dna = tsfi_k0rn_compile_char(0x41); out->is_flower = false; out->k_val = 0.01f; out->coord_scale = 1.0f;
        float w[3]={1,1,1}; memcpy(out->core, w, 12); memcpy(out->body, w, 12); memcpy(out->outer, w, 12);
    } else if (pole_idx == 1) {
        out->dna = tsfi_k0rn_compile_char(0x42); out->is_flower = false; out->k_val = 0.01f; out->coord_scale = 1.0f;
        float w[3]={1,1,1}; memcpy(out->core, w, 12); memcpy(out->body, w, 12); memcpy(out->outer, w, 12);
    } else if (pole_idx == 11) {
        out->dna = tsfi_k0rn_compile_teddy(); out->is_flower = false; out->k_val = 0.1f; out->coord_scale = 1.0f;
        float br_c[3]={0.3f,0.2f,0.1f}, br_b[3]={0.5f,0.35f,0.2f}, br_o[3]={0.6f,0.45f,0.3f};
        memcpy(out->core, br_c, 12); memcpy(out->body, br_b, 12); memcpy(out->outer, br_o, 12);
    } else {
        int f_idx = pole_idx - 2;
        strcpy(out->species_file, g_flower_files[f_idx]);
        TSFiFlowerPhenotype p; tsfi_phenotype_load(out->species_file, &p);
        out->dna = evolve_pole_dna(out->species_file, rand()); out->is_flower = true; out->k_val = 0.15f; out->coord_scale = 0.8f;
        memcpy(out->core, p.core_color, 12); memcpy(out->body, p.body_color, 12); memcpy(out->outer, p.outer_color, 12);
        out->trident = p.trident_freq; out->curling = p.curling_freq;
    }
    if (!out->is_flower && pole_idx < 11) {
        float s = 0.35f;
        for(uint32_t i=0; i<out->dna->op_count; i++) {
            out->dna->ops[i].x *= s; out->dna->ops[i].y *= s; out->dna->ops[i].z *= s; out->dna->ops[i].w *= s; out->dna->ops[i].p1 *= s;
        }
    }
}

int main() {
    printf("[RANDOM-12] Starting 1-Minute Dynamic randomized Transition Demo (Pure)...\n");
    VulkanSystem *s = create_vulkan_system(); if (!s) return 1; s->external_render_enabled = true;
    srand(time(NULL));
    PoleState poles[12]; for(int i=0; i<12; i++) load_pole(i, &poles[i]);
    int curr_idx = rand() % 12; int next_idx = (curr_idx + 1 + (rand() % 11)) % 12;
    time_t start_time = time(NULL); float t = 0.0f;
    while (s->running && (time(NULL) - start_time) < 60) {
        wl_display_dispatch_pending(s->display);
        if (!s->paint_buffer || !s->paint_buffer->data) { draw_frame(s); continue; }
        uint32_t *pixels = (uint32_t*)s->paint_buffer->data;
        int W = s->paint_buffer->width, H = s->paint_buffer->height;
        memset(pixels, 0, W * H * sizeof(uint32_t));
        t += 0.008f;
        if (t >= 1.0f) {
            if (poles[curr_idx].is_flower) {
                K0RnStream *old = poles[curr_idx].dna;
                poles[curr_idx].dna = evolve_pole_dna(poles[curr_idx].species_file, rand());
                lau_free(old->ops); lau_free(old);
            }
            curr_idx = next_idx; next_idx = (curr_idx + 1 + (rand() % 11)) % 12; t = 0.0f;
        }
        float ts = t * t * (3.0f - 2.0f * t);
        float core[3], body[3], outer[3];
        for(int i=0; i<3; i++) {
            core[i] = poles[curr_idx].core[i] * (1.0f - ts) + poles[next_idx].core[i] * ts;
            body[i] = poles[curr_idx].body[i] * (1.0f - ts) + poles[next_idx].body[i] * ts;
            outer[i] = poles[curr_idx].outer[i] * (1.0f - ts) + poles[next_idx].outer[i] * ts;
        }
        float* density_buffer = (float*)lau_malloc(W * H * sizeof(float));
        __m512 v_step = _mm512_set1_ps(2.0f / (float)W), v_ramp = _mm512_set_ps(15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0);
        for (int y = 0; y < H; y++) {
            float fy = ((float)y / H) * 2.0f - 1.0f;
            for (int x = 0; x <= W - 16; x += 16) {
                __m512 v_base = _mm512_set1_ps((float)x * (2.0f / (float)W) - 1.0f);
                __m512 vx = _mm512_fmadd_ps(v_ramp, v_step, v_base), vy = _mm512_set1_ps(fy);
                __m512 angle = tsfi_atan2_ps_local(vy, vx);
                __m512 p1 = _mm512_mul_ps(tsfi_sin_ps_local(_mm512_mul_ps(angle, _mm512_set1_ps(poles[curr_idx].trident + poles[curr_idx].curling))), _mm512_set1_ps(0.015f));
                __m512 p2 = _mm512_mul_ps(tsfi_sin_ps_local(_mm512_mul_ps(angle, _mm512_set1_ps(poles[next_idx].trident + poles[next_idx].curling))), _mm512_set1_ps(0.015f));
                __m512 d1 = render_stream_density(poles[curr_idx].dna, _mm512_mul_ps(vx, _mm512_set1_ps(poles[curr_idx].coord_scale)), _mm512_mul_ps(vy, _mm512_set1_ps(poles[curr_idx].coord_scale)), p1, poles[curr_idx].k_val);
                __m512 d2 = render_stream_density(poles[next_idx].dna, _mm512_mul_ps(vx, _mm512_set1_ps(poles[next_idx].coord_scale)), _mm512_mul_ps(vy, _mm512_set1_ps(poles[next_idx].coord_scale)), p2, poles[next_idx].k_val);
                __m512 d = _mm512_add_ps(_mm512_mul_ps(d1, _mm512_set1_ps(1.0f - ts)), _mm512_mul_ps(d2, _mm512_set1_ps(ts)));
                _mm512_storeu_ps(&density_buffer[y * W + x], d);
            }
        }
        WaveStream density_stream = { .data = density_buffer, .count = W * H, .stride = 4 };
        TSFiFlowerRenderUniforms render_u = { .light_x = 0.5f, .light_y = 0.5f, .light_z = 1.0f, .sss_power = 0.5f, .fresnel_power = 0.3f, .secret_resonance = 1.0f };
        memcpy(render_u.core_color, core, 12); memcpy(render_u.body_color, body, 12); memcpy(render_u.outer_color, outer, 12);
        tsfi_flower_render_shading(&density_stream, pixels, W, H, &render_u); lau_free(density_buffer);
        tsfi_bloom_thunk(pixels, W, H, 0.85f, 0.5f);
        draw_frame(s); tsfi_raw_usleep(16000);
    }
    for(int i=0; i<12; i++) { if(poles[i].dna) { lau_free(poles[i].dna->ops); lau_free(poles[i].dna); } }
    destroy_vulkan_system(s);     extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}
