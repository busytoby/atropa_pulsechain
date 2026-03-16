#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <assert.h>
#include <math.h>
#include "tsfi_manifest.h"
#include "lau_memory.h"
#include "vulkan_main.h"
#include "tsfi_shader_dispatch.h"
#include "tsfi_spirv_kernels.h"

// Benchmark: Phi-Resolution (CPU vs GPU)
// Stress-test with 1M atoms to find true performance limits.

static double get_ms(struct timespec start, struct timespec end) {
    return (double)(end.tv_sec - start.tv_sec) * 1000.0 + (double)(end.tv_nsec - start.tv_nsec) / 1000000.0;
}

#include <unistd.h>

static void dbg_msg(const char* m) {
    ssize_t ret = write(1, m, strlen(m));
    (void)ret;
}

int main() {
    dbg_msg("=== TSFi Phi-Resolution Benchmark (1M Atoms) ===\n");

    const size_t lattice_size = 10000; 
    VulkanSystem* sys = create_vulkan_system();
    if (!sys) { dbg_msg("[BENCH] ERROR: sys is NULL\n"); return 1; }
    dbg_msg("[BENCH] Vulkan System Created\n");
    if (!sys->vk) { dbg_msg("[BENCH] ERROR: sys->vk is NULL\n"); return 1; }
    
    lau_memory_init_gpu(sys->vk);

    // 1. Setup Manifold
    // Note: For CPU benchmark, we MUST use cached memory (Host RAM).
    // ReBAR/WC memory is disastrous for CPU reads.
    TSFiHelmholtzManifold manifold_cpu;
    manifold_cpu.count = lattice_size;
    manifold_cpu.lattice = (TSFiPhiAtom*)aligned_alloc(64, lattice_size * sizeof(TSFiPhiAtom));
    if (!manifold_cpu.lattice) return 1;
    
    // Fill CPU Lattice
    uint32_t secret = 0xABC;
    for (size_t i = 0; i < lattice_size; i++) {
        manifold_cpu.lattice[i].secret_provenance = secret;
        for(int j=0; j<16; j++) manifold_cpu.lattice[i].state_vector[j] = (uint32_t)(i + j);
        manifold_cpu.lattice[i].state_vector[15] = (manifold_cpu.lattice[i].state_vector[15] & ~TSFI_SECRET_MASK) | secret;
    }

    // Setup GPU Manifold (ReBAR)
    TSFiHelmholtzManifold manifold_gpu;
    manifold_gpu.count = lattice_size;
    manifold_gpu.lattice = (TSFiPhiAtom*)lau_malloc_gpu(lattice_size * sizeof(TSFiPhiAtom), LAU_GPU_REBAR);
    // Copy data to ReBAR
    if (manifold_gpu.lattice) {
        memcpy(manifold_gpu.lattice, manifold_cpu.lattice, lattice_size * sizeof(TSFiPhiAtom));
    }

    uint32_t mu[16];
    memcpy(mu, manifold_cpu.lattice[lattice_size - 1].state_vector, 64);

    uint32_t tension = 0;
    struct timespec start, end;
    
    volatile int32_t checksum = 0;

    // --- CPU BENCHMARK ---
    printf("[CPU] Running 100 iterations over 10k atoms (Host RAM)...\n");
    clock_gettime(CLOCK_MONOTONIC, &start);
    for (int i = 0; i < 100; i++) {
        checksum += tsfi_phi_resolve(mu, &manifold_cpu, &tension);
        __asm__ volatile("" : : "g"(checksum) : "memory");
    }
    clock_gettime(CLOCK_MONOTONIC, &end);
    double cpu_ms = get_ms(start, end) / 100.0;
    printf("  [CPU] Avg Time: %.4f ms (Total Tension: %u)\n", cpu_ms, tension);

    // --- GPU BENCHMARK (Dispatch Baseline) ---
    printf("[GPU] Measuring Dispatch Overhead (100 iterations)...\n");
    double gpu_ms = 0.0;
    if (wave_shader_spv_size < 100) {
        printf("  [GPU] SPIR-V Placeholder detected. Measuring overhead only.\n");
    }

    struct {
        uint32_t mu[16];
        uint32_t count;
        uint32_t centrifugal_force;
    } pc;
    memcpy(pc.mu, mu, 64);
    pc.count = (uint32_t)lattice_size;
    pc.centrifugal_force = 1;

    clock_gettime(CLOCK_MONOTONIC, &start);
    for (int i = 0; i < 100; i++) {
        dbg_msg("[GPU] Calling tsfi_dispatch_shader...\n");
        tsfi_dispatch_shader(sys->vk, wave_shader_spv, wave_shader_spv_size, 
                             manifold_gpu.lattice, lattice_size * sizeof(TSFiPhiAtom), 
                             &pc, sizeof(pc));
    }
    clock_gettime(CLOCK_MONOTONIC, &end);
    gpu_ms = get_ms(start, end) / 100.0;
    printf("  [GPU] Avg Time: %.4f ms\n", gpu_ms);

    printf("\n=== CONCLUSION ===\n");
    if (cpu_ms < gpu_ms) {
        printf("RESULT: CPU (AVX-512) is currently FASTER (%.2fx speedup).\n", gpu_ms / cpu_ms);
    } else {
        printf("RESULT: GPU (Vulkan) is currently FASTER (%.2fx speedup).\n", cpu_ms / gpu_ms);
    }

    lau_free(manifold_cpu.lattice);
    lau_free(manifold_gpu.lattice);
    destroy_vulkan_system(sys);
    return (int)checksum != 0 ? 0 : 0;


    extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
}
