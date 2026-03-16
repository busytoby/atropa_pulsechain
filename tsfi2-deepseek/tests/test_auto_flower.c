#include <sys/mman.h>
#include <signal.h>
#include <time.h>
#include "tsfi_svdag.h"
#include "tsfi_opt_zmm.h"
#include "lau_memory.h"
#include "lau_thunk.h"
#include "tsfi_hilbert.h"
#include "tsfi_vision.h"
#include "tsfi_wave512.h"
#include "vulkan/vulkan_render.h" 
#include "vulkan/vulkan_logic.h"
#include "vulkan/vulkan_main.h"
#include "tsfi_logic.h"
#include "tsfi_k0rn_ops.h"
#include "tsfi_genetic.h"
#include "tsfi_io.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <unistd.h>
#include <linux/input.h>
#include <immintrin.h>
#include <stdalign.h>
#include "tsfi_resonance.h"
#include "tsfi_zhao.h"
#include <sys/sysinfo.h>

// Dispatcher task for Zhao scheduler
static void dispatcher_task(void *arg) {
    LauWireThread *wt = (LauWireThread *)arg;
    while (!wt->finished) {
        tsfi_wire_thread_dispatch(wt);
    }
}

#define LOG(...) fprintf(stderr, __VA_ARGS__)

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

typedef struct {
    int W, H;
    float* density_buffer;
    const TSFiFlowerPhenotype* p;
    const K0RnOp* ops;
    __m512 v_secret;
    int start_y, end_y;
} DensityThreadArgs;

static void* density_worker(void* data) {
    DensityThreadArgs* args = (DensityThreadArgs*)data;
    for (int y = args->start_y; y < args->end_y; y++) {
        float fy = ((float)y / args->H) * 2.0f - 1.0f;
        for (int x = 0; x <= args->W - 16; x += 16) {
            __m512 vx = _mm512_set_ps(((float)(x+15)/args->W)*2-1,((float)(x+14)/args->W)*2-1,((float)(x+13)/args->W)*2-1,((float)(x+12)/args->W)*2-1,((float)(x+11)/args->W)*2-1,((float)(x+10)/args->W)*2-1,((float)(x+9)/args->W)*2-1,((float)(x+8)/args->W)*2-1,((float)(x+7)/args->W)*2-1,((float)(x+6)/args->W)*2-1,((float)(x+5)/args->W)*2-1,((float)(x+4)/args->W)*2-1,((float)(x+3)/args->W)*2-1,((float)(x+2)/args->W)*2-1,((float)(x+1)/args->W)*2-1,((float)(x+0)/args->W)*2-1);
            __m512 vy = _mm512_set1_ps(fy); __m512 d = _mm512_set1_ps(1e5f);
            __m512 angle = tsfi_atan2_ps_local(vy, vx);
            
            __m512 perturb = _mm512_setzero_ps();
            if (args->p->trident_freq > 0) perturb = _mm512_add_ps(perturb, _mm512_mul_ps(tsfi_sin_ps_local(_mm512_mul_ps(angle, _mm512_set1_ps(args->p->trident_freq))), _mm512_set1_ps(0.015f)));
            if (args->p->curling_freq > 0) perturb = _mm512_add_ps(perturb, _mm512_mul_ps(tsfi_sin_ps_local(_mm512_mul_ps(angle, _mm512_set1_ps(args->p->curling_freq))), _mm512_set1_ps(0.005f)));
            perturb = _mm512_mul_ps(perturb, args->v_secret);

            for (int i = 0; i < 16; i++) {
                __m512 ox = _mm512_set1_ps(args->ops[i].x), oy = _mm512_set1_ps(args->ops[i].y), orad = _mm512_set1_ps(args->ops[i].p1);
                __m512 vdx = _mm512_sub_ps(vx, ox), vdy = _mm512_sub_ps(vy, oy);
                __m512 dist = _mm512_sqrt_ps(_mm512_add_ps(_mm512_mul_ps(vdx, vdx), _mm512_mul_ps(vdy, vdy)));
                if (args->ops[i].type == K0RN_OP_SPHERE) orad = _mm512_add_ps(orad, perturb);
                __m512 cur_d = _mm512_sub_ps(dist, orad);
                if (args->ops[i].type == K0RN_OP_SUB) d = _mm512_max_ps(d, _mm512_sub_ps(_mm512_setzero_ps(), cur_d));
                else {
                    __m512 h = _mm512_max_ps(_mm512_setzero_ps(), _mm512_sub_ps(_mm512_set1_ps(0.1f), _mm512_abs_ps(_mm512_sub_ps(d, cur_d))));
                    d = _mm512_sub_ps(_mm512_min_ps(d, cur_d), _mm512_mul_ps(h, _mm512_mul_ps(h, _mm512_set1_ps(2.5f))));
                }
            }
            _mm512_storeu_ps(&args->density_buffer[y * args->W + x], d);
        }
    }
    return NULL;
}

int main(int argc, char **argv) {
    signal(SIGPIPE, SIG_IGN);
    int seed = 42; 
    bool auto_test = false;
    char species_file[128] = "assets/flowers/xenostegia.flower";
    for(int i=1; i<argc; i++) {
        if(strcmp(argv[i], "--seed") == 0 && i+1 < argc) seed = atoi(argv[++i]);
        if(strcmp(argv[i], "--test") == 0) auto_test = true;
        if(strcmp(argv[i], "--species") == 0 && i+1 < argc) {
            snprintf(species_file, 127, "assets/flowers/%s.flower", argv[++i]);
        }
    }
    
    TSFiFlowerPhenotype p;
    if (!tsfi_phenotype_load(species_file, &p)) {
        LOG("[ERROR] Failed to load phenotype: %s\n", species_file);
        return 1;
    }

    if (strcmp(p.name, "Xenostegia") == 0) {
        setenv("TSFI_AB4H", "1", 1);
        LOG("[GARDENER] AB4H High-Precision Mode Enabled for %s\n", p.name);
    }

    LOG("[GARDENER] Target: %s | Seed: %d\n", p.name, seed);
    VulkanSystem *s = create_vulkan_system();
    if (!s) return 1;
    s->external_render_enabled = true;

    K0RnOp *best_ops = (K0RnOp*)lau_malloc(16 * sizeof(K0RnOp));
    K0RnOp *mutant_ops = (K0RnOp*)lau_malloc(16 * sizeof(K0RnOp));
    K0RnStream stream; memset(&stream, 0, sizeof(stream));
    stream.op_count = 16; stream.ops = mutant_ops;

    srand(seed);
    for(int i=0; i<16; i++) {
        mutant_ops[i].type = K0RN_OP_SPHERE;
        mutant_ops[i].x = ((float)rand()/RAND_MAX)*0.3f-0.15f;
        mutant_ops[i].y = ((float)rand()/RAND_MAX)*0.3f-0.15f;
        mutant_ops[i].p1 = 0.08f; mutant_ops[i].p2 = ((float)rand()/RAND_MAX)*0.5f;
    }
    mutant_ops[0].type = K0RN_OP_SUB; mutant_ops[0].x = 0; mutant_ops[0].y = 0; mutant_ops[0].p1 = 0.15f;
    memcpy(best_ops, mutant_ops, 16 * sizeof(K0RnOp));

    float best_score = -10000.0f;
    float temp = 1.0f; int gen = 0;
    alignas(64) TSFiResonanceAnalysis last_vis; memset(&last_vis, 0, sizeof(last_vis));
    uint32_t *baseline_pixels = NULL;
    int b_w = 0, b_h = 0;
    uint32_t *pixels = NULL;
    int W = 0, H = 0;

    while (s->running) {
        wl_display_dispatch_pending(s->display);
        if (s->last_key_scancode == KEY_N) { 
            seed = rand(); srand(seed); temp = 1.0f; best_score = -10000.0f;
            for(int i=1; i<16; i++) {
                mutant_ops[i].x = ((float)rand()/RAND_MAX)*0.3f-0.15f;
                mutant_ops[i].y = ((float)rand()/RAND_MAX)*0.3f-0.15f;
                mutant_ops[i].p2 = ((float)rand()/RAND_MAX)*0.5f;
            }
            mutant_ops[0].type = K0RN_OP_SUB; mutant_ops[0].x = 0; mutant_ops[0].y = 0; mutant_ops[0].p1 = 0.15f;
            memcpy(best_ops, mutant_ops, 16 * sizeof(K0RnOp));
            s->last_key_scancode = 0; 
            if (baseline_pixels) { lau_free(baseline_pixels); baseline_pixels = NULL; b_w = 0; b_h = 0; }
        }
        if (s->last_key_scancode == KEY_Y) { 
            LOG("[SAVE] Approved. Saving Gold Standard for %s...\n", p.name);
            if (pixels) {
                tsfi_vision_save_checkpoint(p.name, &last_vis, pixels, W, H);
                tsfi_k0rn_save(p.name, &stream);
            }
            break; 
        }

        if (!s->paint_buffer || !s->paint_buffer->data) { draw_frame(s); continue; }
        pixels = (uint32_t*)s->paint_buffer->data;
        W = s->paint_buffer->width; H = s->paint_buffer->height;

        tsfi_k0rn_evolve_vision(&stream, &last_vis, 2.0f * temp);
        mutant_ops[0].type = K0RN_OP_SUB; mutant_ops[0].x = 0; mutant_ops[0].y = 0;
        float secret = 1.0f + 0.02f * sinf((float)gen * 0.1f);
        __m512 v_secret = _mm512_set1_ps(secret);

        float* density_buffer = (float*)lau_malloc(W * H * sizeof(float));
        
        // Targeted Threading for Xenostegia
        if (strcmp(p.name, "Xenostegia") == 0) {
            int num_cpus = get_nprocs();
            if (num_cpus > 16) num_cpus = 16;
            tsfi_zhao_init(num_cpus);
            
            LauWireThread *threads[16];
            DensityThreadArgs targs[16];
            int rows_per_thread = H / num_cpus;

            for (int i = 0; i < num_cpus; i++) {
                targs[i].W = W; targs[i].H = H; targs[i].density_buffer = density_buffer;
                targs[i].p = &p; targs[i].ops = mutant_ops; targs[i].v_secret = v_secret;
                targs[i].start_y = i * rows_per_thread;
                targs[i].end_y = (i == num_cpus - 1) ? H : (i + 1) * rows_per_thread;
                
                threads[i] = tsfi_wire_thread_create(i, 1.0);
                threads[i]->start_routine = (void*(*)(void*))density_worker;
                threads[i]->arg = &targs[i];
                tsfi_zhao_submit(dispatcher_task, threads[i]);
            }

            tsfi_zhao_sync();

            for (int i = 0; i < num_cpus; i++) {
                LauSystemHeader *h = (LauSystemHeader *)((char *)threads[i] - offsetof(LauSystemHeader, payload));
                h->proxy = NULL;
                lau_free(threads[i]);
            }
            tsfi_zhao_shutdown();
        } else {
            for (int y = 0; y < H; y++) {
                float fy = ((float)y / H) * 2.0f - 1.0f;
                for (int x = 0; x <= W - 16; x += 16) {
                    __m512 vx = _mm512_set_ps(((float)(x+15)/W)*2-1,((float)(x+14)/W)*2-1,((float)(x+13)/W)*2-1,((float)(x+12)/W)*2-1,((float)(x+11)/W)*2-1,((float)(x+10)/W)*2-1,((float)(x+9)/W)*2-1,((float)(x+8)/W)*2-1,((float)(x+7)/W)*2-1,((float)(x+6)/W)*2-1,((float)(x+5)/W)*2-1,((float)(x+4)/W)*2-1,((float)(x+3)/W)*2-1,((float)(x+2)/W)*2-1,((float)(x+1)/W)*2-1,((float)(x+0)/W)*2-1);
                    __m512 vy = _mm512_set1_ps(fy); __m512 d = _mm512_set1_ps(1e5f);
                    __m512 angle = tsfi_atan2_ps_local(vy, vx);
                    
                    __m512 perturb = _mm512_setzero_ps();
                    if (p.trident_freq > 0) perturb = _mm512_add_ps(perturb, _mm512_mul_ps(tsfi_sin_ps_local(_mm512_mul_ps(angle, _mm512_set1_ps(p.trident_freq))), _mm512_set1_ps(0.015f)));
                    if (p.curling_freq > 0) perturb = _mm512_add_ps(perturb, _mm512_mul_ps(tsfi_sin_ps_local(_mm512_mul_ps(angle, _mm512_set1_ps(p.curling_freq))), _mm512_set1_ps(0.005f)));
                    perturb = _mm512_mul_ps(perturb, v_secret);

                    for (int i = 0; i < 16; i++) {
                        __m512 ox = _mm512_set1_ps(mutant_ops[i].x), oy = _mm512_set1_ps(mutant_ops[i].y), orad = _mm512_set1_ps(mutant_ops[i].p1);
                        __m512 vdx = _mm512_sub_ps(vx, ox), vdy = _mm512_sub_ps(vy, oy);
                        __m512 dist = _mm512_sqrt_ps(_mm512_add_ps(_mm512_mul_ps(vdx, vdx), _mm512_mul_ps(vdy, vdy)));
                        if (mutant_ops[i].type == K0RN_OP_SPHERE) orad = _mm512_add_ps(orad, perturb);
                        __m512 cur_d = _mm512_sub_ps(dist, orad);
                        if (mutant_ops[i].type == K0RN_OP_SUB) d = _mm512_max_ps(d, _mm512_sub_ps(_mm512_setzero_ps(), cur_d));
                        else {
                            __m512 h = _mm512_max_ps(_mm512_setzero_ps(), _mm512_sub_ps(_mm512_set1_ps(0.1f), _mm512_abs_ps(_mm512_sub_ps(d, cur_d))));
                            d = _mm512_sub_ps(_mm512_min_ps(d, cur_d), _mm512_mul_ps(h, _mm512_mul_ps(h, _mm512_set1_ps(2.5f))));
                        }
                    }
                    _mm512_storeu_ps(&density_buffer[y * W + x], d);
                }
            }
        }

        WaveStream density_stream = { .data = density_buffer, .count = W * H, .stride = 4 };
        TSFiFlowerRenderUniforms render_u = { .light_x = 0.5f, .light_y = 0.5f, .light_z = 1.0f, .sss_power = 0.5f, .fresnel_power = 0.3f, .secret_resonance = secret };
        memcpy(render_u.core_color, p.core_color, sizeof(float)*3);
        memcpy(render_u.body_color, p.body_color, sizeof(float)*3);
        memcpy(render_u.outer_color, p.outer_color, sizeof(float)*3);
        
        if (s->is_ab4h) {
            tsfi_flower_render_shading_ab4h_threaded(&density_stream, (uint16_t*)pixels, W, H, &render_u);
            tsfi_bloom_thunk_ab4h((uint16_t*)pixels, W, H, 0.85f + 0.05f * sinf((float)gen*0.05f), 0.4f);
        } else if (strcmp(p.name, "Xenostegia") == 0) {
            tsfi_flower_render_shading_threaded(&density_stream, pixels, W, H, &render_u);
            tsfi_bloom_thunk(pixels, W, H, 0.85f + 0.05f * sinf((float)gen*0.05f), 0.4f);
        } else {
            tsfi_flower_render_shading(&density_stream, pixels, W, H, &render_u);
            tsfi_bloom_thunk(pixels, W, H, 0.85f + 0.05f * sinf((float)gen*0.05f), 0.4f);
        }
        
        draw_frame(s);

        if (gen == 0 || W != b_w || H != b_h) {
            if (baseline_pixels) lau_free(baseline_pixels);
            size_t px_size = s->is_ab4h ? 8 : 4;
            baseline_pixels = (uint32_t*)lau_malloc(W * H * px_size);
            memcpy(baseline_pixels, pixels, W * H * px_size);
            b_w = W; b_h = H;
        }

        alignas(64) TSFiResonanceAnalysis vis;
        if (s->is_ab4h) {
            tsfi_vision_analyze_staging_ab4h((uint16_t*)pixels, W, H, &p, &vis);
        } else {
            tsfi_vision_analyze_staging(pixels, W, H, &p, &vis);
        }
        
        if (!tsfi_vision_verify_integrity(&vis)) {
            LOG("[CRITICAL] Visual Integrity Failure in Gen %d. Reverting to best.\n", gen);
            memcpy(mutant_ops, best_ops, 16 * sizeof(K0RnOp));
            lau_free(density_buffer); continue;
        }

        tsfi_vision_ray_query_thunk(mutant_ops, 16, &vis, W, H); 
        if (s->is_ab4h) {
            vis.baseline_similarity = tsfi_vision_calculate_similarity_ab4h((uint16_t*)pixels, (uint16_t*)baseline_pixels, W, H);
        } else {
            vis.baseline_similarity = tsfi_vision_calculate_similarity(pixels, baseline_pixels, W, H);
        }
        
        float score = (vis.progression_ratio * 100.0f) + (vis.complexity * 10.0f) + (vis.baseline_similarity * 20.0f);
        if (score > best_score || (temp > 0.1f && ((float)rand()/RAND_MAX) < expf((score - best_score) / temp))) {
            if (score > best_score) {
                best_score = score; memcpy(best_ops, mutant_ops, 16 * sizeof(K0RnOp));
                LOG("[%s] Gen %d | Progress: %.1f%% | TargetCorr: %.2f | Stable: %.2f\n", p.name, gen, vis.progression_ratio * 100.0f, vis.target_correlation, vis.baseline_similarity);
            }
        } else { memcpy(mutant_ops, best_ops, 16 * sizeof(K0RnOp)); }
        temp *= 0.9998f; if (temp < 0.05f) temp = 0.05f;
        last_vis = vis; gen++;
        lau_free(density_buffer);

        if (auto_test && gen >= 200) {
            LOG("[AUTO] Gen 200 reached. Saving Gold Standard for %s and exiting.\n", p.name);
            if (pixels) {
                tsfi_vision_save_checkpoint(p.name, &last_vis, pixels, W, H);
                tsfi_k0rn_save(p.name, &stream);
            }
            break;
        }
    }
    lau_free(best_ops); lau_free(mutant_ops);
    if (baseline_pixels) lau_free(baseline_pixels);
    destroy_vulkan_system(s);     extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}
