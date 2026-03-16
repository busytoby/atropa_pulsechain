#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <assert.h>
#include <string.h>
#include "tsfi_wave_any.h"
#include "lau_memory.h"
#include "lau_thunk.h"
#include "tsfi_shader_dispatch.h"
#include "tsfi_spirv_kernels.h"
#include "tsfi_operator.h"
#include "vulkan_init.h"

typedef struct { float intensity; float decay; } ShaderUniforms;
typedef struct { float t; float secret; uint32_t count; uint32_t pad; } MorphUniforms;

extern void wave_shader_main(const WaveStream* stream, const ShaderUniforms* u);
extern void wave_morph_main(const WaveStream* stream, const MorphUniforms* u);

void cpu_shader_c(const WaveStream* s, const ShaderUniforms* u) {
    for (uint32_t i=0; i<s->count; i++) { float* p = (float*)WAVE_PTR(s, i); for(int k=0; k<4; k++) p[k] = p[k] * u->intensity - u->decay; }
}

void cpu_morph_c(const WaveStream* s, const MorphUniforms* u) {
    for (uint32_t i=0; i<s->count; i++) { float* p = (float*)WAVE_PTR(s, i); for(int k=0; k<4; k++) p[k] = p[k] * u->t + u->secret; }
}

static double get_time_s() {
    struct timespec ts; clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec * 1e-9;
}

void bench_operator_symmetry(VulkanContext* vk, const char* name, size_t atoms, void* cpu_shader_fn, void* cpu_thunk_fn, const uint32_t* spv, size_t spv_size, void* uniforms, size_t u_size) {
    printf("\n=== COMPREHENSIVE BIJECTIVE BENCHMARK: %s ===\n", name);
    size_t bytes = atoms * 16;
    void* shared_data = lau_malloc_gpu(bytes, LAU_GPU_REBAR);
    WaveStream stream = {0};
    stream.data = shared_data;
    stream.count = (uint32_t)atoms;
    stream.atom_size = 16;
    stream.stride = 16;
    
    // 1. CPU Shader (Reference C)
    memset(shared_data, 0, bytes);
    double t0 = get_time_s(); ((void(*)(const WaveStream*, void*))cpu_shader_fn)(&stream, uniforms); double t1 = get_time_s();
    printf("  [CPU/Shader] Time: %.6f s | Bandwidth: %.2f GB/s\n", t1-t0, (bytes/(t1-t0))/1e9);

    // 2. CPU Thunk (WAVE Assembly)
    memset(shared_data, 0, bytes);
    double t2 = get_time_s(); ((void(*)(const WaveStream*, void*))cpu_thunk_fn)(&stream, uniforms); double t3 = get_time_s();
    printf("  [CPU/Thunk]  Time: %.6f s | Bandwidth: %.2f GB/s\n", t3-t2, (bytes/(t3-t2))/1e9);

    if (vk && spv) {
        // 3. GPU Shader (Stateless)
        memset(shared_data, 0, bytes);
        double t4 = get_time_s(); tsfi_dispatch_shader(vk, spv, spv_size, shared_data, bytes, uniforms, u_size); double t5 = get_time_s();
        printf("  [GPU/Shader] Time: %.6f s | Bandwidth: %.2f GB/s (Stateless)\n", t5-t4, (bytes/(t5-t4))/1e9);

        // 4. GPU Thunk (Stateful)
        memset(shared_data, 0, bytes);
        TSFiOperator* op = tsfi_op_create(name, NULL, spv, spv_size);
        tsfi_shader_pipeline_create(vk, op, u_size); tsfi_shader_buffer_ensure(vk, op, bytes);
        double t6 = get_time_s(); tsfi_shader_pipeline_execute(vk, op, shared_data, bytes, uniforms, u_size); double t7 = get_time_s();
        printf("  [GPU/Thunk]  Time: %.6f s | Bandwidth: %.2f GB/s (Stateful/Baked)\n", t7-t6, (bytes/(t7-t6))/1e9);
        tsfi_op_destroy(op, vk);
    } else {
        printf("  [GPU/Symmetry] Skipped (No Vulkan/SPV)\n");
    }
    lau_free(shared_data);
}

int main() {
    VulkanContext* vk = init_vulkan();
    if (vk) lau_memory_init_gpu(vk);
    
    ShaderUniforms su = { 2.0f, 0.5f };
    bench_operator_symmetry(vk, "wave_shader", 1024*1024*4, (void*)cpu_shader_c, (void*)wave_shader_main, wave_shader_spv, wave_shader_spv_size, &su, sizeof(su));
    
    MorphUniforms mu = { 0.5f, 0.1f, 1024*1024*4, 0 };
    bench_operator_symmetry(vk, "wave_morph", 1024*1024*4, (void*)cpu_morph_c, (void*)wave_morph_main, wave_shader_spv, wave_shader_spv_size, &mu, sizeof(mu));
    
    if (vk) cleanup_vulkan(vk);
        extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}