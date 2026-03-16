#include "tsfi_opt_zmm.h"
#include "lau_thunk.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "tsfi_resonance.h"
#include <immintrin.h>
#include <stdatomic.h>

static double get_time_sec() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec * 1e-9;
}

static void print_header(const char *name) {
    printf("\n================================================================\n");
    printf("[BENCH] %s\n", name);
    printf("================================================================\n");
}

static void print_result(const char *metric, double val, const char *unit) {
    printf("  %-25s : %10.2f %s\n", metric, val, unit);
    if (strstr(unit, "KWave/s")) {
        double waves_per_sec = val * 1000.0;
        double cycles = 2500000000.0 / waves_per_sec;
        printf("    [RDNA4 Est.] Cycles/Wave : %10.2f cycles\n", cycles);
    }
}

// --- Kernels ---

void kernel_scalar_fma(void *ctx, ZmmSynapse *syn) {
    (void)syn; __m512 *r = (__m512*)ctx;
    for(int i=0; i<100; i++) r[0] = _mm512_fmadd_ps(r[0], r[1], r[2]);
}

void kernel_wide_wave(void *ctx, ZmmSynapse *syn) {
    (void)syn; __m512 *r = (__m512*)ctx;
    for(int i=0; i<100; i++) {
        r[0] = _mm512_fmadd_ps(r[4],  r[5],  r[0]);
        r[1] = _mm512_fmadd_ps(r[6],  r[7],  r[1]);
        r[2] = _mm512_fmadd_ps(r[8],  r[9],  r[2]);
        r[3] = _mm512_fmadd_ps(r[10], r[11], r[3]);
    }
}

void kernel_masked_div(void *ctx, ZmmSynapse *syn) {
    (void)syn; __m512 *r = (__m512*)ctx;
    __m512 thresh = _mm512_set1_ps(10.0f);
    for(int i=0; i<100; i++) {
        __mmask16 k = _mm512_cmp_ps_mask(r[0], thresh, _CMP_GT_OQ);
        r[1] = _mm512_mask_add_ps(r[1], k, r[1], r[2]);
        r[1] = _mm512_mask_sub_ps(r[1], ~k, r[1], r[2]);
    }
}

void kernel_branchy_div(void *ctx, ZmmSynapse *syn) {
    (void)syn; __m512 *r = (__m512*)ctx;
    float vals[16] __attribute__((aligned(512)));
    float a[16]    __attribute__((aligned(512)));
    float b[16]    __attribute__((aligned(512)));
    _mm512_storeu_ps(vals, r[0]);
    _mm512_storeu_ps(a, r[1]);
    _mm512_storeu_ps(b, r[2]);
    for(int i=0; i<100; i++) {
        for(int j=0; j<16; j++) {
            if(vals[j] > 10.0f) a[j] += b[j];
            else a[j] -= b[j];
        }
    }
    r[1] = _mm512_loadu_ps(a);
}

void kernel_wmma_simulate(void *ctx, ZmmSynapse *syn) {
    (void)syn; __m512 *r = (__m512*)ctx;
    for(int i=0; i<10; i++) {
        for(int j=0; j<15; j++) {
            r[17+j] = _mm512_fmadd_ps(r[j], r[16], r[17+j]);
        }
    }
}

void kernel_lds_shuffle(void *ctx, ZmmSynapse *syn) {
    (void)syn; __m512 *r = (__m512*)ctx;
    __m512i indices = _mm512_set_epi32(0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15);
    for(int i=0; i<100; i++) {
        r[0] = _mm512_permutexvar_ps(indices, r[0]);
        r[1] = _mm512_permutexvar_ps(indices, r[1]);
        r[2] = _mm512_permutexvar_ps(indices, r[2]);
        r[3] = _mm512_permutexvar_ps(indices, r[3]);
        r[4] = _mm512_permutexvar_ps(indices, r[4]);
        r[5] = _mm512_permutexvar_ps(indices, r[5]);
        r[6] = _mm512_permutexvar_ps(indices, r[6]);
        r[7] = _mm512_permutexvar_ps(indices, r[7]);
    }
}

void kernel_wave512_compute(void *ctx, ZmmSynapse *syn) {
    (void)syn; __m512 *r = (__m512*)ctx;
    for(int i=0; i<100; i++) {
        #pragma GCC unroll 32
        for(int j=0; j<32; j++) r[j] = _mm512_fmadd_ps(r[j], r[j], r[j]);
    }
}

void kernel_vram_bw(void *ctx, ZmmSynapse *syn) {
    (void)syn; __m512 *r = (__m512*)ctx;
    for(int i=0; i<16; i++) r[i] = _mm512_add_ps(r[i], r[i+16]);
}

void kernel_lds_stride(void *ctx, ZmmSynapse *syn) {
    (void)syn; __m512 *r = (__m512*)ctx;
    for(int i=0; i<100; i++) {
        r[0] = _mm512_add_ps(r[0], r[1]);
        r[0] = _mm512_add_ps(r[0], r[2]);
    }
}

static _Atomic int lds_atomic_counter = 0;
void kernel_lds_atomic(void *ctx, ZmmSynapse *syn) {
    (void)ctx; (void)syn;
    for(int i=0; i<100; i++) atomic_fetch_add(&lds_atomic_counter, 1);
}

// Bijective Helmholtz Kernel
// Maps Boltzmann Operators (SGPRs ZMM16-31) to Feynman Points (VGPRs ZMM0-15)
// Simulates SVDAG evolution where Structure (Entropy) dictates Substance (Energy).
void kernel_helmholtz_svdag(void *ctx, ZmmSynapse *syn) {
    (void)syn; __m512 *r = (__m512*)ctx;
    
    // ZMM16-31: Boltzmann Operators (Structure/Entropy)
    // ZMM0-15:  Feynman Points (Substance/Energy)
    
    // Process 16 "Nodes" in parallel (Wave512 split)
    for(int i=0; i<100; i++) {
        #pragma GCC unroll 16
        for(int j=0; j<16; j++) {
            // 1. Boltzmann Step: Evaluate Entropy (SGPR)
            // Generate topology mask based on structural threshold
            __mmask16 structure_mask = _mm512_cmp_ps_mask(r[16+j], _mm512_set1_ps(0.5f), _CMP_GT_OQ);
            
            // 2. Feynman Step: Evolve Energy (VGPR)
            // Apply energy transfer only where structure exists (Masked)
            // Energy += Entropy * Resonance
            r[j] = _mm512_mask_fmadd_ps(r[j], structure_mask, r[16+j], _mm512_set1_ps(1.618f));
            
            // 3. Feedback: Update Structure based on new Energy density (Bijective Loop)
            r[16+j] = _mm512_fnmadd_ps(r[j], _mm512_set1_ps(0.01f), r[16+j]);
        }
    }
}

void kernel_resident_switch(void *ctx, ZmmSynapse *syn) {
    (void)syn; __m512 *r = (__m512*)ctx;
    __m512 *active_wave;
    float signal = _mm_cvtss_f32(_mm512_castps512_ps128(r[31]));
    if (signal > 0.0f) active_wave = &r[16];
    else active_wave = &r[0];
    active_wave[0] = _mm512_add_ps(active_wave[0], active_wave[1]);
}

void kernel_svdag_traverse(void *ctx, ZmmSynapse *syn) {
    (void)syn; __m512 *r = (__m512*)ctx;
    for(int depth=0; depth<8; depth++) {
        __m512 children = _mm512_add_ps(r[0], _mm512_set1_ps(1.0f));
        r[0] = _mm512_mask_blend_ps(0xAAAA, r[0], children);
        r[16+depth] = r[0]; 
    }
    for(int depth=7; depth>=0; depth--) r[0] = r[16+depth];
}

static _Atomic int global_work_cursor = 0;
#define WORK_BATCH_SIZE 100000
void kernel_persistent_scheduler(void *ctx, ZmmSynapse *syn) {
    (void)syn; __m512 *r = (__m512*)ctx;
    int batch_idx = atomic_fetch_add(&global_work_cursor, 1); (void)batch_idx;
    for(int i=0; i<100; i++) r[0] = _mm512_fmadd_ps(r[0], r[1], r[2]);
}

static void (*spill_z0)(void);
static void (*fill_z0)(void);
static float global_spill[16 * 16] __attribute__((aligned(512)));
void kernel_spill_wrapper(void *ctx, ZmmSynapse *syn) {
    (void)syn; __m512 *r = (__m512*)ctx;
    spill_z0(); r[0] = _mm512_add_ps(r[1], r[2]); fill_z0();
}

static void* dispatch_wrapper(void *arg) {
    tsfi_dispatch_zmm_dynamic((TsfiZmmManifest*)arg);
    return NULL;
}

// --- Shared Resources ---
static float global_vram[256 * 32 * 16] __attribute__((aligned(512)));
static float global_lds[1024 * 1024] __attribute__((aligned(512)));

// --- Scenarios ---

void kernel_baseline_fast(void *ctx, ZmmSynapse *syn) {
    (void)syn; __m512 *r = (__m512*)ctx;
    // 16 independent FMAs (Max ILP)
    // Minimizes kernel latency to reveal dispatcher overhead.
    #pragma GCC unroll 16
    for(int i=0; i<16; i++) r[i] = _mm512_fmadd_ps(r[i], r[i], r[i]);
}

void run_baseline_occupancy() {
    print_header("Baseline Occupancy (1 Wave / WGP)");
    TsfiZmmManifest m; memset(&m, 0, sizeof(m));
    m.micro_kernel = kernel_baseline_fast;
    m.active_mask = 0xFFFFFFFF; // Full Wave Saturation
    m.cache_policy = 2; // L1 Pin
    m.contiguous_rf = global_vram;
    
    double t0 = get_time_sec();
    for(int i=0; i<100000; i++) tsfi_dispatch_zmm_dynamic(&m);
    double dt = get_time_sec() - t0;
    
    double waves_per_sec = 100000.0 / dt;
    double cycles = 2500000000.0 / waves_per_sec;
    print_result("Throughput", waves_per_sec / 1000.0, "KWave/s");
    printf("    [RDNA4 Est.] Cycles/Wave : %10.2f cycles\n", cycles);
    
    if (cycles > 100.0) {
        printf("    [FAIL] Policy Violation: Baseline Occupancy > 100 cycles.\n");
    } else {
        printf("    [PASS] Policy Compliance: High-Occupancy Dispatch Verified.\n");
    }
}

void run_persistent_ilp() {
    print_header("Persistent ILP (GPU Occupancy Mode)");
    TsfiZmmManifest m; memset(&m, 0, sizeof(m));
    m.micro_kernel = kernel_wide_wave;
    m.persistent_cycles = 1000; m.active_mask = 0xFFF; m.cache_policy = 2;
    for(int i=0; i<12; i++) m.slots[i].data_ptr = &global_vram[i*16];
    double t0 = get_time_sec();
    for(int i=0; i<1000; i++) tsfi_dispatch_zmm_dynamic(&m);
    double dt = get_time_sec() - t0;
    print_result("Throughput", 1000.0 / dt, "KWave/s");
}

void run_bijective_helmholtz_svdag() {
    print_header("Bijective Helmholtz SVDAG (SGPR-VGPR Wiring)");
    TsfiZmmManifest m; memset(&m, 0, sizeof(m));
    m.micro_kernel = kernel_helmholtz_svdag;
    m.persistent_cycles = 1000;
    m.active_mask = 0xFFFFFFFF; // All 32 ZMMs (16 SGPR + 16 VGPR)
    m.cache_policy = 2; // Pin L1
    
    // Map VRAM: 0-15 Data (Feynman), 16-31 Control (Boltzmann)
    for(int i=0; i<32; i++) m.slots[i].data_ptr = &global_vram[i*16];

    double t0 = get_time_sec();
    for(int i=0; i<500; i++) tsfi_dispatch_zmm_dynamic(&m);
    double dt = get_time_sec() - t0;
    
    // Ops: 500 * 1000 * 100 * 16 lanes * (1 compare + 1 mask_fma + 1 fma)
    // Roughly 3 FLOPS/lane/iter.
    print_result("SVDAG Throughput", 500.0 * 1000.0 / dt / 1000.0, "KWave/s");
    double gflops = (500.0 * 1000.0 * 100.0 * 16.0 * 16.0 * 3.0) / dt / 1e9;
    print_result("Bijective Compute Rate", gflops, "GFLOPS");
}

void run_wmma_saturation() {
    print_header("WMMA Matrix-Core Saturation");
    TsfiZmmManifest m; memset(&m, 0, sizeof(m));
    m.micro_kernel = kernel_wmma_simulate;
    m.persistent_cycles = 1000; m.active_mask = 0xFFFFFFFF; m.cache_policy = 2;
    for(int i=0; i<32; i++) m.slots[i].data_ptr = &global_vram[i*16];
    double t0 = get_time_sec();
    for(int i=0; i<500; i++) tsfi_dispatch_zmm_dynamic(&m);
    double dt = get_time_sec() - t0;
    print_result("Matrix Op Rate", (500.0*1000.0*10.0*15.0*16.0*2.0) / dt / 1e9, "GFLOPS");
}

void run_wave512_saturation() {
    print_header("Wave512 Massive SIMD");
    TsfiZmmManifest m; memset(&m, 0, sizeof(m));
    m.micro_kernel = kernel_wave512_compute;
    m.persistent_cycles = 1000; m.active_mask = 0xFFFFFFFF; m.cache_policy = 2;
    for(int i=0; i<32; i++) m.slots[i].data_ptr = &global_vram[i*16];
    double t0 = get_time_sec();
    for(int i=0; i<500; i++) tsfi_dispatch_zmm_dynamic(&m);
    double dt = get_time_sec() - t0;
    print_result("Compute Rate", (500.0*1000.0*100.0*32.0*16.0*2.0) / dt / 1e9, "GFLOPS");
}

void run_svdag_traversal() {
    print_header("Recursive SVDAG Traversal");
    TsfiZmmManifest m; memset(&m, 0, sizeof(m));
    m.micro_kernel = kernel_svdag_traverse;
    m.persistent_cycles = 1000; m.active_mask = 0xFFFFFFFF; m.cache_policy = 2;
    for(int i=0; i<32; i++) m.slots[i].data_ptr = &global_vram[i*16];
    double t0 = get_time_sec();
    for(int i=0; i<500; i++) tsfi_dispatch_zmm_dynamic(&m);
    double dt = get_time_sec() - t0;
    print_result("Traversal Throughput", 500.0 / dt, "KWave/s");
}

void run_lds_stride_impact() {
    print_header("LDS Stride Access Impact");
    TsfiZmmManifest m; memset(&m, 0, sizeof(m));
    m.micro_kernel = kernel_lds_stride;
    m.persistent_cycles = 1000; m.active_mask = 0xFFFF; m.cache_policy = 2;
    int strides[] = {1, 4, 16};
    for(int s=0; s<3; s++) {
        int stride = strides[s];
        for(int i=0; i<16; i++) m.slots[i].data_ptr = &global_lds[i * stride * 16]; 
        double t0 = get_time_sec();
        for(int i=0; i<500; i++) tsfi_dispatch_zmm_dynamic(&m);
        double dt = get_time_sec() - t0;
        char label[32]; sprintf(label, "Stride %d lines", stride);
        print_result(label, 500.0 / dt, "KWave/s");
    }
}

void run_vram_bandwidth() {
    print_header("VRAM Bandwidth Saturation");
    TsfiZmmManifest m; memset(&m, 0, sizeof(m));
    m.micro_kernel = kernel_vram_bw;
    m.active_mask = 0xFFFFFFFF; m.cache_policy = 1;
    for(int i=0; i<32; i++) m.slots[i].data_ptr = &global_vram[i*16];
    double t0 = get_time_sec();
    for(int i=0; i<50000; i++) tsfi_dispatch_zmm_dynamic(&m);
    double dt = get_time_sec() - t0;
    print_result("Effective Bandwidth", (50000.0 * 64 * 64) / dt / 1e9, "GB/s");
}

void run_lds_atomic_stress() {
    print_header("LDS Atomic Contention Stress");
    LauWireThread *threads[4]; TsfiZmmManifest manifests[4];
    atomic_store(&lds_atomic_counter, 0);
    for(int w=0; w<4; w++) {
        memset(&manifests[w], 0, sizeof(TsfiZmmManifest));
        manifests[w].micro_kernel = kernel_lds_atomic;
        manifests[w].persistent_cycles = 100; manifests[w].active_mask = 0x0; 
        manifests[w].cache_policy = 2;
        
        threads[w] = tsfi_wire_thread_create(w, 1.0);
        threads[w]->start_routine = dispatch_wrapper;
        threads[w]->arg = &manifests[w];
    }

    bool all_done = false;
    while (!all_done) {
        all_done = true;
        for (int i = 0; i < 4; i++) {
            if (!threads[i]->finished) {
                tsfi_wire_thread_dispatch(threads[i]);
                all_done = false;
            }
        }
    }

    for (int i = 0; i < 4; i++) {
        LauSystemHeader *h = (LauSystemHeader *)((char *)threads[i] - offsetof(LauSystemHeader, payload));
        h->proxy = NULL;
        lau_free(threads[i]);
    }
}

void run_register_spilling() {
    print_header("Software-Managed Register Spilling");
    ThunkProxy *p = ThunkProxy_create();
    spill_z0 = (void(*)(void))ThunkProxy_emit_zmm_spill(p, global_spill, 0, 0);
    fill_z0 = (void(*)(void))ThunkProxy_emit_zmm_fill_slot(p, global_spill, 0, 0);
    TsfiZmmManifest m; memset(&m, 0, sizeof(m));
    m.micro_kernel = kernel_spill_wrapper;
    m.persistent_cycles = 1000; m.active_mask = 0x7; m.cache_policy = 1;
    for(int i=0; i<3; i++) m.slots[i].data_ptr = &global_vram[i*16];
    double t0 = get_time_sec();
    for(int i=0; i<1000; i++) tsfi_dispatch_zmm_dynamic(&m);
    double dt = get_time_sec() - t0;
    print_result("Throughput", 1000.0 / dt, "KWave/s");
    ThunkProxy_destroy(p);
}

void run_wavefront_scheduler_latency() {
    print_header("Wavefront Scheduler Latency (Zero-Copy)");
    TsfiZmmManifest m; memset(&m, 0, sizeof(m));
    m.micro_kernel = kernel_resident_switch;
    m.persistent_cycles = 1000000; m.active_mask = 0xFFFFFFFF; 
    for(int i=0; i<32; i++) m.slots[i].data_ptr = &global_vram[i*16];
    double t0 = get_time_sec();
    tsfi_dispatch_zmm_dynamic(&m); 
    double dt = get_time_sec() - t0;
    double latency_ns = dt * 1e9 / 1000000.0;
    print_result("Switch Latency", latency_ns, "ns/switch");
}

int main() {
    printf("TSFi2 Comprehensive GPU Assembly Benchmark Suite v4.1\n");
    run_baseline_occupancy();
    run_persistent_ilp();
    run_bijective_helmholtz_svdag();
    run_wmma_saturation();
    run_wave512_saturation();
    run_svdag_traversal();
    run_lds_stride_impact();
    run_vram_bandwidth();
    run_lds_atomic_stress();
    run_register_spilling();
    run_wavefront_scheduler_latency();
    printf("\n[SUITE] All benchmarks completed.\n");
        extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}
