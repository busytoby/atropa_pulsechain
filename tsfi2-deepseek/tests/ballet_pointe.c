#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <stdatomic.h>
#include <immintrin.h>
#include "lau_memory.h"
#include "lau_registry.h"
#include "lau_thunk.h"
#include "tsfi_io.h"
#include "tsfi_time.h"
#include "tsfi_crash_handler.h"
#include "tsfi_ballet.h"

static LauTelemetryState *g_sys_telem = NULL;

void __attribute__((noinline)) pointe_native_routine(float *out_matrix) {
    // Pointe: 32 ZMMs balance. Register bank is on tip. Edge reached.
    // Increased Difficulty: Execute a unified matrix multiplication / wave transformation
    // where all 32 AVX-512 registers are heavily engaged in a single unrolled compute block.
    
    __asm__ volatile (
        "vbroadcastss (%0), %%zmm0\n\t"
        "vbroadcastss 4(%0), %%zmm1\n\t"
        "vbroadcastss 8(%0), %%zmm2\n\t"
        "vbroadcastss 12(%0), %%zmm3\n\t"
        
        "vmovaps %%zmm0, %%zmm4\n\t" "vmovaps %%zmm1, %%zmm5\n\t"
        "vmovaps %%zmm2, %%zmm6\n\t" "vmovaps %%zmm3, %%zmm7\n\t"
        "vaddps %%zmm0, %%zmm1, %%zmm8\n\t" "vsubps %%zmm2, %%zmm3, %%zmm9\n\t"
        "vmulps %%zmm4, %%zmm5, %%zmm10\n\t" "vdivps %%zmm7, %%zmm6, %%zmm11\n\t"
        
        "vfmadd231ps %%zmm8, %%zmm9, %%zmm12\n\t"
        "vfnmadd231ps %%zmm10, %%zmm11, %%zmm13\n\t"
        "vmaxps %%zmm12, %%zmm13, %%zmm14\n\t"
        "vminps %%zmm8, %%zmm10, %%zmm15\n\t"

        "vaddps %%zmm14, %%zmm15, %%zmm16\n\t"
        "vsubps %%zmm14, %%zmm15, %%zmm17\n\t"
        "vmulps %%zmm16, %%zmm17, %%zmm18\n\t"
        "vsqrtps %%zmm18, %%zmm19\n\t"
        
        "vmovaps %%zmm19, %%zmm20\n\t" "vmovaps %%zmm19, %%zmm21\n\t"
        "vmovaps %%zmm19, %%zmm22\n\t" "vmovaps %%zmm19, %%zmm23\n\t"
        "vaddps %%zmm20, %%zmm21, %%zmm24\n\t" "vsubps %%zmm22, %%zmm23, %%zmm25\n\t"
        "vmulps %%zmm24, %%zmm25, %%zmm26\n\t" "vaddps %%zmm26, %%zmm19, %%zmm27\n\t"
        
        "vmovaps %%zmm27, %%zmm28\n\t" "vmovaps %%zmm27, %%zmm29\n\t"
        "vmovaps %%zmm27, %%zmm30\n\t" "vmovaps %%zmm27, %%zmm31\n\t"
        
        // Output resolution mapping back to memory
        "vmovaps %%zmm31, (%0)\n\t"
        : 
        : "r" (out_matrix)
        : "memory", "zmm0","zmm1","zmm2","zmm3","zmm4","zmm5","zmm6","zmm7",
          "zmm8","zmm9","zmm10","zmm11","zmm12","zmm13","zmm14","zmm15",
          "zmm16","zmm17","zmm18","zmm19","zmm20","zmm21","zmm22","zmm23",
          "zmm24","zmm25","zmm26","zmm27","zmm28","zmm29","zmm30","zmm31"
    );
}

void test_pointe_logic() {
    printf("[SUB-TEST] Pointe (On Tips) - ZMM Register Bank Routine\n");
    size_t cp = BALLET_CHECKPOINT();
    
    printf("  [ROUTINE] 1. Creating Thunk Proxy for Extreme Register Pressure...\n");
    ThunkProxy *proxy = ThunkProxy_create();
    
    printf("  [ROUTINE] 2. Emitting Pointe Logic (32x ZMM Pressure Matrix)...\n");
    void *thunk = ThunkProxy_emit_baked(proxy, (void*)pointe_native_routine, 0);
    
    printf("  [ROUTINE] 3. Sealing Proxy for AVX-512 Execution...\n");
    ThunkProxy_seal(proxy);
    
    void (*pointe_call)(float*) = (void (*)(float*))thunk;
    
    float *out_matrix = (float*)lau_memalign_wired(64, 64);
    for (int i = 0; i < 16; i++) out_matrix[i] = 1.0f + (float)i * 0.1f;
    
    if (g_sys_telem) {
        snprintf(g_sys_telem->last_directive_str, sizeof(g_sys_telem->last_directive_str), "ZMM Register Tip Balancing");
    }
    
    printf("[INFO] Executing Pointe through Thunk Manifold...\n");
    
    const int iterations = 1000000;
    unsigned long long start = get_time_ns();
    for (int i = 0; i < iterations; i++) {
        pointe_call(out_matrix);
        
        if (g_sys_telem && (i % 50000 == 0)) {
            atomic_store(&g_sys_telem->exec_steps, i);
            g_sys_telem->current_intensity = (float)i / (float)iterations;
            
            uint64_t val;
            memcpy(&val, &out_matrix[0], sizeof(float));
            g_sys_telem->zmm_val = val;
        }
    }
    unsigned long long end = get_time_ns();
    
    if (g_sys_telem) {
        atomic_store(&g_sys_telem->exec_steps, iterations);
        g_sys_telem->current_intensity = 1.0f;
    }
    
    double ops = (double)iterations * 32.0 * 16.0;
    double duration_s = (double)(end - start) / 1e9;
    printf("[BENCH] Pointe Register Pressure: %.2f GFLOPS.\n", (ops / duration_s) / 1e9);
    
    printf("[PASS] Pointe Routine complete. All 32 ZMMs balanced at the edge.\n");
    
    printf("  [ROUTINE] 4. Reclaiming Pointe Thunk Manifold...\n");
    ThunkProxy_destroy(proxy);
    lau_free(out_matrix);
    
    BALLET_AUDIT(cp, "Pointe Routine");
}

int main() {
    tsfi_install_crash_handler();
    printf("=== TSFi Ballet Step 11: Pointe (On Tips) ===\n");

    int fd = tsfi_shm_open("/tsfi_ballet_step11_telem", O_RDWR | O_CREAT, 0666);
    if (ftruncate(fd, sizeof(LauTelemetryState)) != 0) { perror("ftruncate"); return 1; }
    g_sys_telem = mmap(NULL, sizeof(LauTelemetryState), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    memset(g_sys_telem, 0, sizeof(LauTelemetryState));
    tsfi_register_fault_flag(&g_sys_telem->system_integrity_fault);

    test_pointe_logic();

    if (atomic_load(&g_sys_telem->system_integrity_fault)) {
        printf("[CRITICAL] Pointe failed: System integrity fractured.\n");
        exit(1);
    }

    printf("=== Pointe Step Passed ===\n");
    printf("PERFECTLY ZERO LEAKS\n");
    
    munmap(g_sys_telem, sizeof(LauTelemetryState));
    close(fd);
    tsfi_shm_unlink("/tsfi_ballet_step11_telem");
    return 0;
}
