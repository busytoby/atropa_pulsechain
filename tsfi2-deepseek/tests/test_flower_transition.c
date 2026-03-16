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

// Internal renderer for evolution (doesn't need Vulkan)
void internal_render_dna(K0RnStream *s, const TSFiFlowerPhenotype *p, uint32_t *pixels, int W, int H) {
    float* density = (float*)lau_malloc(W * H * sizeof(float));
    float trident = p->trident_freq;
    float curling = p->curling_freq;

    for (int y = 0; y < H; y++) {
        float fy = ((float)y / H) * 2.0f - 1.0f;
        for (int x = 0; x <= W - 16; x += 16) {
            __m512 vx = _mm512_set_ps(((float)(x+15)/W)*2-1,((float)(x+14)/W)*2-1,((float)(x+13)/W)*2-1,((float)(x+12)/W)*2-1,((float)(x+11)/W)*2-1,((float)(x+10)/W)*2-1,((float)(x+9)/W)*2-1,((float)(x+8)/W)*2-1,((float)(x+7)/W)*2-1,((float)(x+6)/W)*2-1,((float)(x+5)/W)*2-1,((float)(x+4)/W)*2-1,((float)(x+3)/W)*2-1,((float)(x+2)/W)*2-1,((float)(x+1)/W)*2-1,((float)(x+0)/W)*2-1);
            __m512 vy = _mm512_set1_ps(fy); 
            __m512 d = _mm512_set1_ps(1e5f);
            __m512 angle = tsfi_atan2_ps_local(vy, vx);
            __m512 perturb = _mm512_setzero_ps();
            if (trident > 0) perturb = _mm512_add_ps(perturb, _mm512_mul_ps(tsfi_sin_ps_local(_mm512_mul_ps(angle, _mm512_set1_ps(trident))), _mm512_set1_ps(0.015f)));
            if (curling > 0) perturb = _mm512_add_ps(perturb, _mm512_mul_ps(tsfi_sin_ps_local(_mm512_mul_ps(angle, _mm512_set1_ps(curling))), _mm512_set1_ps(0.005f)));

            for (uint32_t i = 0; i < s->op_count; i++) {
                __m512 ox = _mm512_set1_ps(s->ops[i].x), oy = _mm512_set1_ps(s->ops[i].y), orad = _mm512_set1_ps(s->ops[i].p1);
                __m512 vdx = _mm512_sub_ps(vx, ox), vdy = _mm512_sub_ps(vy, oy);
                __m512 dist = _mm512_sqrt_ps(_mm512_add_ps(_mm512_mul_ps(vdx, vdx), _mm512_mul_ps(vdy, vdy)));
                if (s->ops[i].type == K0RN_OP_SPHERE) orad = _mm512_add_ps(orad, perturb);
                __m512 cur_d = _mm512_sub_ps(dist, orad);
                if (s->ops[i].type == K0RN_OP_SUB) d = _mm512_max_ps(d, _mm512_sub_ps(_mm512_setzero_ps(), cur_d));
                else {
                    __m512 h = _mm512_max_ps(_mm512_setzero_ps(), _mm512_sub_ps(_mm512_set1_ps(0.1f), _mm512_abs_ps(_mm512_sub_ps(d, cur_d))));
                    d = _mm512_sub_ps(_mm512_min_ps(d, cur_d), _mm512_mul_ps(h, _mm512_mul_ps(h, _mm512_set1_ps(2.5f))));
                }
            }
            _mm512_storeu_ps(&density[y * W + x], d);
        }
    }

    WaveStream density_stream = { .data = density, .count = W * H, .stride = 4 };
    TSFiFlowerRenderUniforms render_u = { .light_x = 0.5f, .light_y = 0.5f, .light_z = 1.0f, .sss_power = 0.5f, .fresnel_power = 0.3f, .secret_resonance = 1.0f };
    memcpy(render_u.core_color, p->core_color, 12); memcpy(render_u.body_color, p->body_color, 12); memcpy(render_u.outer_color, p->outer_color, 12);
    tsfi_flower_render_shading(&density_stream, pixels, W, H, &render_u);
    lau_free(density);
}

K0RnStream* evolve_species_dna(const TSFiFlowerPhenotype *p, int seed) {
    srand(seed);
    K0RnStream *s = (K0RnStream*)lau_malloc_wired(sizeof(K0RnStream));
    s->stream_capacity = 16; s->op_count = 16;
    s->ops = (K0RnOp*)lau_memalign(512, sizeof(K0RnOp) * 16);
    
    for(int i=0; i<16; i++) {
        s->ops[i].type = K0RN_OP_SPHERE;
        s->ops[i].x = ((float)rand()/RAND_MAX)*0.3f-0.15f;
        s->ops[i].y = ((float)rand()/RAND_MAX)*0.3f-0.15f;
        s->ops[i].p1 = 0.08f; s->ops[i].p2 = ((float)rand()/RAND_MAX)*0.5f;
    }
    s->ops[0].type = K0RN_OP_SUB; s->ops[0].x = 0; s->ops[0].y = 0; s->ops[0].p1 = 0.15f;

    uint32_t *scratch = (uint32_t*)lau_malloc(128 * 128 * sizeof(uint32_t));
    TSFiResonanceAnalysis vis;
    float best_score = -1e6f;
    K0RnOp *best_ops = (K0RnOp*)lau_malloc(16 * sizeof(K0RnOp));

    for (int gen = 0; gen < 100; gen++) {
        tsfi_k0rn_evolve_vision(s, &vis, 1.0f);
        s->ops[0].type = K0RN_OP_SUB; s->ops[0].x = 0; s->ops[0].y = 0;
        internal_render_dna(s, p, scratch, 128, 128);
        tsfi_vision_analyze_staging(scratch, 128, 128, p, &vis);
        float score = (vis.target_correlation * 100.0f) + (vis.coverage * 10.0f);
        if (score > best_score) { best_score = score; memcpy(best_ops, s->ops, 16 * sizeof(K0RnOp)); }
        else memcpy(s->ops, best_ops, 16 * sizeof(K0RnOp));
    }
    lau_free(scratch); lau_free(best_ops);
    return s;
}

int main() {
    printf("[FLOWER] Starting Dynamic Species Transition Demo...\n");
    VulkanSystem *s = create_vulkan_system();
    if (!s) return 1;
    s->external_render_enabled = true;

    TSFiFlowerPhenotype p_D, p_M, p_G;
    tsfi_phenotype_load("assets/flowers/distimake.flower", &p_D);
    tsfi_phenotype_load("assets/flowers/merremia.flower", &p_M);
    tsfi_phenotype_load("assets/flowers/generic.flower", &p_G);

    K0RnStream *stream_A = evolve_species_dna(&p_D, rand());
    K0RnStream *stream_B = evolve_species_dna(&p_M, rand());
    
    void *wired_obj = lau_malloc_wired(sizeof(uint32_t) + sizeof(TSFiResonanceAnalysis));
    uint32_t *magic = (uint32_t*)wired_obj; *magic = 0x41534956;
    TSFiResonanceAnalysis *vis_ptr = (TSFiResonanceAnalysis*)(magic + 1);

    int frame = 0; int cycle = 0; bool moving_to_B = true;
    int prev_class = -1;

    while (s->running && frame < 1200) {
        wl_display_dispatch_pending(s->display);
        if (!s->paint_buffer || !s->paint_buffer->data) { draw_frame(s); continue; }

        uint32_t *pixels = (uint32_t*)s->paint_buffer->data;
        int W = s->paint_buffer->width, H = s->paint_buffer->height;

        float speed = 0.02f;
        float progress = (float)frame * speed;
        float t = (sinf(progress - 1.57f) + 1.0f) * 0.5f;

        // Detect pole crossing to generate NEW destination
        if (moving_to_B && t > 0.995f) {
            printf("[GENESIS] Reached Merremia. Evolving new Distimake for return trip...\n");
            lau_free(stream_A->ops); lau_free(stream_A);
            stream_A = evolve_species_dna(&p_D, rand());
            moving_to_B = false; cycle++;
        } else if (!moving_to_B && t < 0.005f) {
            printf("[GENESIS] Reached Distimake. Evolving new Merremia for return trip...\n");
            lau_free(stream_B->ops); lau_free(stream_B);
            stream_B = evolve_species_dna(&p_M, rand());
            moving_to_B = true;
        }

        // Interpolate and Render
        K0RnStream *morph = tsfi_k0rn_interpolate(stream_A, stream_B, t);
        float trident = p_D.trident_freq + (p_M.trident_freq - p_D.trident_freq) * t;
        float curling = p_D.curling_freq + (p_M.curling_freq - p_D.curling_freq) * t;
        
        float* density_buffer = (float*)lau_malloc(W * H * sizeof(float));
        for (int y = 0; y < H; y++) {
            float fy = ((float)y / H) * 2.0f - 1.0f;
            for (int x = 0; x <= W - 16; x += 16) {
                __m512 vx = _mm512_set_ps(((float)(x+15)/W)*2-1,((float)(x+14)/W)*2-1,((float)(x+13)/W)*2-1,((float)(x+12)/W)*2-1,((float)(x+11)/W)*2-1,((float)(x+10)/W)*2-1,((float)(x+9)/W)*2-1,((float)(x+8)/W)*2-1,((float)(x+7)/W)*2-1,((float)(x+6)/W)*2-1,((float)(x+5)/W)*2-1,((float)(x+4)/W)*2-1,((float)(x+3)/W)*2-1,((float)(x+2)/W)*2-1,((float)(x+1)/W)*2-1,((float)(x+0)/W)*2-1);
                __m512 vy = _mm512_set1_ps(fy); __m512 d = _mm512_set1_ps(1e5f);
                __m512 angle = tsfi_atan2_ps_local(vy, vx);
                __m512 perturb = _mm512_setzero_ps();
                if (trident > 0) perturb = _mm512_add_ps(perturb, _mm512_mul_ps(tsfi_sin_ps_local(_mm512_mul_ps(angle, _mm512_set1_ps(trident))), _mm512_set1_ps(0.015f)));
                if (curling > 0) perturb = _mm512_add_ps(perturb, _mm512_mul_ps(tsfi_sin_ps_local(_mm512_mul_ps(angle, _mm512_set1_ps(curling))), _mm512_set1_ps(0.005f)));

                for (uint32_t i = 0; i < morph->op_count; i++) {
                    __m512 ox = _mm512_set1_ps(morph->ops[i].x), oy = _mm512_set1_ps(morph->ops[i].y), orad = _mm512_set1_ps(morph->ops[i].p1);
                    __m512 vdx = _mm512_sub_ps(vx, ox), vdy = _mm512_sub_ps(vy, oy);
                    __m512 dist = _mm512_sqrt_ps(_mm512_add_ps(_mm512_mul_ps(vdx, vdx), _mm512_mul_ps(vdy, vdy)));
                    if (morph->ops[i].type == K0RN_OP_SPHERE) orad = _mm512_add_ps(orad, perturb);
                    __m512 cur_d = _mm512_sub_ps(dist, orad);
                    if (morph->ops[i].type == K0RN_OP_SUB) d = _mm512_max_ps(d, _mm512_sub_ps(_mm512_setzero_ps(), cur_d));
                    else {
                        __m512 h = _mm512_max_ps(_mm512_setzero_ps(), _mm512_sub_ps(_mm512_set1_ps(0.1f), _mm512_abs_ps(_mm512_sub_ps(d, cur_d))));
                        d = _mm512_sub_ps(_mm512_min_ps(d, cur_d), _mm512_mul_ps(h, _mm512_mul_ps(h, _mm512_set1_ps(2.5f))));
                    }
                }
                _mm512_storeu_ps(&density_buffer[y * W + x], d);
            }
        }

        WaveStream density_stream = { .data = density_buffer, .count = W * H, .stride = 4 };
        TSFiFlowerRenderUniforms render_u = { .light_x = 0.5f, .light_y = 0.5f, .light_z = 1.0f, .sss_power = 0.5f, .fresnel_power = 0.3f, .secret_resonance = 1.0f };
        for(int i=0; i<3; i++) {
            render_u.core_color[i] = p_D.core_color[i] + (p_M.core_color[i] - p_D.core_color[i]) * t;
            render_u.body_color[i] = p_D.body_color[i] + (p_M.body_color[i] - p_D.body_color[i]) * t;
            render_u.outer_color[i] = p_D.outer_color[i] + (p_M.outer_color[i] - p_D.outer_color[i]) * t;
        }
        tsfi_flower_render_shading(&density_stream, pixels, W, H, &render_u);
        lau_free(density_buffer);

        tsfi_vision_analyze_staging(pixels, W, H, &p_G, vis_ptr);
        TSFiClassification classification = tsfi_vision_classify(vis_ptr);
        int current_class = (int)classification.class_id;
        if (current_class != prev_class) {
            printf("[TRANSITION] Frame %d: Identity shifted to %d (t=%.2f, Conf=%.2f)\n", frame, current_class, t, classification.confidence);
            prev_class = current_class;
        }
        draw_frame(s); tsfi_raw_usleep(16000); frame++;
        lau_free(morph->ops); lau_free(morph);
    }
    lau_free(wired_obj); destroy_vulkan_system(s);     extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}