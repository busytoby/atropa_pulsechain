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
#include "tsfi_c_math.h"
#include <immintrin.h>
#include <stdalign.h>
#include "tsfi_raw.h"

#define sinf tsfi_sinf

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

K0RnStream* evolve_high_qual(const TSFiFlowerPhenotype *p, int seed) {
    (void)p;
    srand(seed);
    K0RnStream *s = (K0RnStream*)lau_malloc_wired(sizeof(K0RnStream));
    s->stream_capacity = 16; s->op_count = 16;
    s->ops = (K0RnOp*)lau_memalign(512, sizeof(K0RnOp) * 16);
    for(int i=0; i<16; i++) {
        s->ops[i].type = K0RN_OP_SPHERE;
        s->ops[i].x = ((float)rand()/RAND_MAX)*0.3f-0.15f;
        s->ops[i].y = ((float)rand()/RAND_MAX)*0.3f-0.15f;
        s->ops[i].p1 = 0.08f; s->ops[i].p2 = 0;
    }
    s->ops[0].type = K0RN_OP_SUB; s->ops[0].x = 0; s->ops[0].y = 0; s->ops[0].p1 = 0.15f;
    TSFiResonanceAnalysis vis; memset(&vis, 0, sizeof(vis));
    for (int gen = 0; gen < 200; gen++) {
        tsfi_k0rn_evolve_vision(s, &vis, 1.0f);
        s->ops[0].type = K0RN_OP_SUB; s->ops[0].x = 0; s->ops[0].y = 0;
    }
    return s;
}

int main() {
    printf("[FLOWER-A] Starting High-Fidelity Hyalocystis to 'A' Transition Demo...\n");
    VulkanSystem *s = create_vulkan_system(); if (!s) return 1; s->external_render_enabled = true;
    TSFiFlowerPhenotype p_H, p_G;
    tsfi_phenotype_load("assets/flowers/hyalocystis.flower", &p_H);
    tsfi_phenotype_load("assets/flowers/generic.flower", &p_G);
    K0RnStream *stream_H = evolve_high_qual(&p_H, rand());
    K0RnStream *stream_A = tsfi_k0rn_load("0x41");
    if (!stream_A) { fprintf(stderr, "Error: A DNA not found\n"); return 1; }
    
    // Normalize 'A' pole: explicitly ensure all 5 segments are present
    if (stream_A->op_count < 5) {
        printf("[FIX] Re-bootstrapping 'A' DNA within demo...\n");
        lau_free(stream_A->ops); lau_free(stream_A);
        stream_A = tsfi_k0rn_compile_char(0x41);
    }

    float glyph_scale = 0.35f;
    for (uint32_t i=0; i<stream_A->op_count; i++) {
        stream_A->ops[i].x *= glyph_scale; stream_A->ops[i].y *= glyph_scale;
        stream_A->ops[i].z *= glyph_scale; stream_A->ops[i].w *= glyph_scale;
        stream_A->ops[i].p1 *= glyph_scale;
    }

    void *wired_obj = lau_malloc_wired(sizeof(uint32_t) + sizeof(TSFiResonanceAnalysis));
    uint32_t *magic = (uint32_t*)wired_obj; *magic = 0x41534956;
    int frame = 0; bool moving_to_A = true;
    while (s->running && frame < 2400) {
        wl_display_dispatch_pending(s->display);
        if (!s->paint_buffer || !s->paint_buffer->data) { draw_frame(s); continue; }
        uint32_t *pixels = (uint32_t*)s->paint_buffer->data;
        int W = s->paint_buffer->width, H = s->paint_buffer->height;
        memset(pixels, 0, W * H * sizeof(uint32_t));

        float progress = (float)frame * 0.015f;
        float t = (sinf(progress - 1.57f) + 1.0f) * 0.5f;
        if (t > 0.995f && !moving_to_A) {
            printf("[GENESIS] Refreshing Hyalocystis DNA...\n");
            lau_free(stream_H->ops); lau_free(stream_H);
            stream_H = evolve_high_qual(&p_H, rand());
            moving_to_A = true;
        } else if (t < 0.005f) { moving_to_A = false; }

        float* density_buffer = (float*)lau_malloc(W * H * sizeof(float));
        __m512 v_step = _mm512_set1_ps(2.0f / (float)W);
        __m512 v_ramp = _mm512_set_ps(15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0);
        for (int y = 0; y < H; y++) {
            float fy = ((float)y / H) * 2.0f - 1.0f;
            for (int x = 0; x <= W - 16; x += 16) {
                __m512 v_base = _mm512_set1_ps((float)x * (2.0f / (float)W) - 1.0f);
                __m512 vx = _mm512_fmadd_ps(v_ramp, v_step, v_base), vy = _mm512_set1_ps(fy);
                __m512 angle = tsfi_atan2_ps_local(vy, vx);
                
                // Hyalocystis (Pole 0): Organic smin, curling perturbation
                __m512 v_h_freq = _mm512_set1_ps(p_H.curling_freq > 0 ? p_H.curling_freq : 12.0f);
                __m512 perturb = _mm512_mul_ps(tsfi_sin_ps_local(_mm512_mul_ps(angle, v_h_freq)), _mm512_set1_ps(0.015f));
                // Scale coordinates for flower detail (0.5x coordinate = 2x size)
                __m512 dH = render_stream_density(stream_H, _mm512_mul_ps(vx, _mm512_set1_ps(0.5f)), _mm512_mul_ps(vy, _mm512_set1_ps(0.5f)), perturb, 0.15f);
                
                // 'A' Pole (Pole 1): Sharp k, no perturbation
                __m512 dA = render_stream_density(stream_A, vx, vy, _mm512_setzero_ps(), 0.01f);
                
                // Final Blended density
                __m512 d = _mm512_add_ps(_mm512_mul_ps(dH, _mm512_set1_ps(1.0f - t)), _mm512_mul_ps(dA, _mm512_set1_ps(t)));
                _mm512_storeu_ps(&density_buffer[y * W + x], d);
            }
        }
        WaveStream density_stream = { .data = density_buffer, .count = W * H, .stride = 4 };
        TSFiFlowerRenderUniforms render_u = { .light_x = 0.5f, .light_y = 0.5f, .light_z = 1.0f, .sss_power = 0.5f, .fresnel_power = 0.3f, .secret_resonance = 1.0f };
        float white[3] = {1,1,1};
        for(int i=0; i<3; i++) {
            render_u.core_color[i] = p_H.core_color[i] * (1.0f - t) + white[i] * t;
            render_u.body_color[i] = p_H.body_color[i] * (1.0f - t) + white[i] * t;
            render_u.outer_color[i] = p_H.outer_color[i] * (1.0f - t) + white[i] * t;
        }
        tsfi_flower_render_shading(&density_stream, pixels, W, H, &render_u); lau_free(density_buffer);
        tsfi_bloom_thunk(pixels, W, H, 0.85f, 0.45f);
        draw_frame(s); tsfi_raw_usleep(16000); frame++;
    }
    lau_free(wired_obj); destroy_vulkan_system(s);     extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}
