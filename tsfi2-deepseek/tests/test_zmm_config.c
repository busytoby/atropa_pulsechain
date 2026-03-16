#include "tsfi_opt_zmm.h"
#include "lau_thunk.h"
#include <stdio.h>
#include <string.h>
#include <immintrin.h>

// Micro-Kernel: Just multiply density by scale
static void kernel_low(void *ctx, ZmmSynapse *syn) {
    __m512 *regs = (__m512*)ctx;
    regs[0] = _mm512_mul_ps(regs[0], regs[1]); 
    (void)syn;
}

// Density Thunk (Simulated JIT code)
static void kernel_density_analyzer(void *ctx, ZmmSynapse *syn) {
    __m512 *regs = (__m512*)ctx;
    float mass = _mm_cvtss_f32(_mm512_castps512_ps128(regs[0]));
    
    syn->mass_density = mass;
    if (mass > 10.0f) {
        syn->request_kernel_swap = 1;
    }
}

static void kernel_high(void *ctx, ZmmSynapse *syn) {
    __m512 *regs = (__m512*)ctx;
    regs[0] = _mm512_add_ps(regs[0], regs[1]);
    (void)syn;
}

int main() {
    printf("[TEST] Thunked Initialization & Density Analysis...\n");

    ThunkProxy *p = ThunkProxy_create();
    if (!p) return 1;

    TsfiZmmManifest m;
    memset(&m, 0, sizeof(m));
    
    m.micro_kernel = kernel_low;
    m.density_kernel = kernel_density_analyzer;
    m.kernel_high_density = kernel_high;

    // Data Setup (64-byte aligned)
    float density[16] __attribute__((aligned(512)));
    float scale[16]   __attribute__((aligned(512)));

    // --- NEW: Addressing initialization specifically with thunking ---
    // Instead of:
    // for(int i=0; i<16; i++) { density[i] = 10.0f; scale[i] = 2.0f; }
    
    void (*fill_density)(void) = (void(*)(void))ThunkProxy_emit_zmm_fill(p, density, 10.0f);
    void (*fill_scale)(void)   = (void(*)(void))ThunkProxy_emit_zmm_fill(p, scale, 2.0f);

    fill_density();
    fill_scale();
    // ------------------------------------------------------------------

    m.slots[0].type = ZMM_TYPE_MASS;
    m.slots[0].data_ptr = density;
    m.slots[1].type = ZMM_TYPE_VGPR;
    m.slots[1].data_ptr = scale;

    // Dispatch Frame 1
    tsfi_dispatch_zmm_dynamic(&m);
    
    if (m.micro_kernel == kernel_high) {
        printf("[PASS] Thunked Init + Density successfully triggered swap.\n");
        printf("[INFO] Final Value: %.1f\n", density[0]);
    } else {
        printf("[FAIL] Initialization or trigger failed.\n");
        ThunkProxy_destroy(p);
        return 1;
    }

    ThunkProxy_destroy(p);
        extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}