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
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <unistd.h>
#include <linux/input.h>
#include <immintrin.h>

#define LOG(...) fprintf(stderr, __VA_ARGS__)

static inline double get_ms() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec * 1000.0 + (double)ts.tv_nsec / 1000000.0;
}

static inline __m512 kernel_ssaa(__m512 rx, __m512 ry, const __m512 *pax, const __m512 *pay, const __m512 *pbx, const __m512 *pby, const __m512 *rad, int count, float softness) {
    __m512 d = _mm512_set1_ps(1e5f);
    __m512 zero = _mm512_setzero_ps(); __m512 one = _mm512_set1_ps(1.0f);
    __m512 half = _mm512_set1_ps(0.5f); __m512 k_factor = _mm512_set1_ps(0.12f);
    __m512 eps = _mm512_set1_ps(1e-6f);
    for (int i = 0; i < count; i++) {
        __m512 ba_x = _mm512_sub_ps(pbx[i], pax[i]); __m512 ba_y = _mm512_sub_ps(pby[i], pay[i]);
        __m512 pa_dx = _mm512_sub_ps(rx, pax[i]); __m512 pa_dy = _mm512_sub_ps(ry, pay[i]);
        __m512 h = _mm512_div_ps(_mm512_add_ps(_mm512_mul_ps(pa_dx, ba_x), _mm512_mul_ps(pa_dy, ba_y)), _mm512_max_ps(_mm512_add_ps(_mm512_mul_ps(ba_x, ba_x), _mm512_mul_ps(ba_y, ba_y)), eps));
        h = _mm512_min_ps(one, _mm512_max_ps(zero, h));
        __m512 dx = _mm512_sub_ps(pa_dx, _mm512_mul_ps(ba_x, h)); __m512 dy = _mm512_sub_ps(pa_dy, _mm512_mul_ps(ba_y, h));
        __m512 dist = _mm512_sqrt_ps(_mm512_add_ps(_mm512_mul_ps(dx, dx), _mm512_mul_ps(dy, dy)));
        __m512 cur_d = _mm512_sub_ps(dist, rad[i]);
        __m512 h_mix = _mm512_add_ps(half, _mm512_mul_ps(half, _mm512_div_ps(_mm512_sub_ps(d, cur_d), k_factor)));
        h_mix = _mm512_min_ps(one, _mm512_max_ps(zero, h_mix));
        d = _mm512_sub_ps(_mm512_add_ps(_mm512_mul_ps(d, _mm512_sub_ps(one, h_mix)), _mm512_mul_ps(cur_d, h_mix)), _mm512_mul_ps(k_factor, _mm512_mul_ps(h_mix, _mm512_sub_ps(one, h_mix))));
    }
    return _mm512_max_ps(zero, _mm512_min_ps(one, _mm512_sub_ps(one, _mm512_mul_ps(d, _mm512_set1_ps(softness)))));
}

int main(int argc, char **argv) {
    signal(SIGPIPE, SIG_IGN);
    int seed = 42; int max_frames = 2000000000;
    for(int i=1; i<argc; i++) {
        if(strcmp(argv[i], "--test") == 0) max_frames = 20; 
        else if(strcmp(argv[i], "--seed") == 0 && i+1 < argc) seed = atoi(argv[++i]);
    }
    
    LOG("[VISION] SSAA High-Fidelity Run | Seed: %d | Y=Approve, N=Mutate\n", seed);
    VulkanSystem *s = create_vulkan_system();
    if (!s) return 1;
    s->external_render_enabled = true;
    
    K0RnStream *base_genome = tsfi_k0rn_compile_teddy();
    K0RnStream *mutant = (K0RnStream*)lau_malloc_wired(sizeof(K0RnStream));
    mutant->stream_capacity = 32; mutant->op_count = base_genome->op_count;
    mutant->ops = (K0RnOp*)lau_memalign(512, sizeof(K0RnOp) * 32);
    memcpy(mutant->ops, base_genome->ops, sizeof(K0RnOp) * base_genome->op_count);

    srand(seed);
    int frames = 0; __m512 op_pax[16], op_pay[16], op_pbx[16], op_pby[16], op_rad[16];
    TSFiResonanceAnalysis last_vis; memset(&last_vis, 0, sizeof(last_vis));
    double last_frame_time = get_ms();
    float current_softness = 35.0f;

    while (s->running && frames < max_frames) {
        wl_display_roundtrip(s->display);
        if (s->last_key_scancode == KEY_Y) { LOG("[VERDICT] Approved Seed: %d\n", seed); break; }
        if (s->last_key_scancode == KEY_N) { seed = rand(); srand(seed); memcpy(mutant->ops, base_genome->ops, sizeof(K0RnOp) * base_genome->op_count); s->last_key_scancode = 0; }

        double now = get_ms(); double dt = now - last_frame_time; last_frame_time = now;
        uint32_t *pixels = (uint32_t*)s->paint_buffer->data;
        int W = s->paint_buffer->width; int H = s->paint_buffer->height;

        tsfi_k0rn_evolve_vision(mutant, &last_vis, 1.0f);
        
        if (last_vis.complexity > 0.5f) current_softness -= 0.5f;
        else if (last_vis.complexity < 0.2f) current_softness += 0.5f;
        if (current_softness < 5.0f) current_softness = 5.0f;
        if (current_softness > 60.0f) current_softness = 60.0f;

        int op_count = mutant->op_count; if (op_count > 16) op_count = 16;
        for (int i=0; i<op_count; i++) {
            op_pax[i] = _mm512_set1_ps(mutant->ops[i].x); op_pay[i] = _mm512_set1_ps(mutant->ops[i].y);
            op_pbx[i] = _mm512_set1_ps(mutant->ops[i].z); op_pby[i] = _mm512_set1_ps(mutant->ops[i].w);
            op_rad[i] = _mm512_set1_ps(mutant->ops[i].p1);
        }

        __m512 dx_ss = _mm512_set_ps(0.25f/W, -0.25f/W, 0.25f/W, -0.25f/W, 0.25f/W, -0.25f/W, 0.25f/W, -0.25f/W, 0.25f/W, -0.25f/W, 0.25f/W, -0.25f/W, 0.25f/W, -0.25f/W, 0.25f/W, -0.25f/W);
        __m512 dy_ss = _mm512_set_ps(0.25f/H, 0.25f/H, -0.25f/H, -0.25f/H, 0.25f/H, 0.25f/H, -0.25f/H, -0.25f/H, 0.25f/H, 0.25f/H, -0.25f/H, -0.25f/H, 0.25f/H, 0.25f/H, -0.25f/H, -0.25f/H);

        for (int y = 0; y < H; y++) {
            float fy = ((float)y / H) * 2.0f - 1.0f;
            for (int x = 0; x < W; x += 4) {
                __m512 vx_base = _mm512_set_ps(
                    ((float)(x+3)/W)*2-1, ((float)(x+3)/W)*2-1, ((float)(x+3)/W)*2-1, ((float)(x+3)/W)*2-1,
                    ((float)(x+2)/W)*2-1, ((float)(x+2)/W)*2-1, ((float)(x+2)/W)*2-1, ((float)(x+2)/W)*2-1,
                    ((float)(x+1)/W)*2-1, ((float)(x+1)/W)*2-1, ((float)(x+1)/W)*2-1, ((float)(x+1)/W)*2-1,
                    ((float)(x+0)/W)*2-1, ((float)(x+0)/W)*2-1, ((float)(x+0)/W)*2-1, ((float)(x+0)/W)*2-1
                );
                __m512 vy_vec = _mm512_set1_ps(fy);
                __m512 v_res = kernel_ssaa(_mm512_add_ps(vx_base, dx_ss), _mm512_add_ps(vy_vec, dy_ss), op_pax, op_pay, op_pbx, op_pby, op_rad, op_count, current_softness);
                
                float results[16]; _mm512_storeu_ps(results, v_res);
                for(int p=0; p<4; p++) {
                    float avg = (results[p*4+0] + results[p*4+1] + results[p*4+2] + results[p*4+3]) * 0.25f;
                    uint8_t vr = (uint8_t)(avg * 0.5f * 255.0f);
                    uint8_t vg = (uint8_t)(avg * 0.7f * 255.0f);
                    uint8_t vb = (uint8_t)(avg * 1.0f * 255.0f);
                    pixels[y * W + x + p] = (0xFFu << 24) | (vr << 16) | (vg << 8) | vb;
                }
            }
        }
        draw_frame(s);

        static TSFiFlowerPhenotype dummy_p; // For generic vision testing
        TSFiResonanceAnalysis vis;
        tsfi_vision_analyze_temporal(pixels, W, H, &last_vis, (float)dt, &dummy_p, &vis);
        last_vis = vis;
        if (frames % 20 == 0) LOG("[VISION] Content FPS: %.1f | Edge Softness: %.1f | Complexity: %.3f\n", vis.fps, current_softness, vis.complexity);
        frames++;
    }
    destroy_vulkan_system(s);     extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    
    extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}