#include "tsfi_operator.h"
#include "tsfi_shader_dispatch.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static double get_time_s() {
    struct timespec ts; clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec * 1e-9;
}

TSFiOperator* tsfi_op_create(const char* name, void* cpu_fn, const uint32_t* spv, size_t spv_size) {
    TSFiOperator* op = (TSFiOperator*)lau_malloc(sizeof(TSFiOperator));
    memset(op, 0, sizeof(TSFiOperator));
    op->name = name;
    op->spv_code = spv;
    op->spv_size = spv_size;
    op->offload_threshold = 64 * 1024 * 1024;
    if (cpu_fn) {
        op->cpu_thunk = ThunkProxy_create();
        op->cpu_entry = (void(*)(void))cpu_fn;
    }
    return op;
}

void tsfi_op_dispatch(struct TSFiOperator* op, VulkanContext* vk, WaveStream* stream, void* uniforms, size_t u_size) {
    size_t data_size = stream->count * stream->stride;
        int pressure = (stream->flags & WAVE_FLAG_BACKPRESSURE);
        int verify = (stream->flags & WAVE_FLAG_VERIFY);
        
        // Safety: GPU shaders currently expect float alignment (stride % 4 == 0)
        // If stride is packed (e.g. 15), fallback to CPU until specialized unpacking shader exists.
        bool packed_unsafe = (stream->stride % 4 != 0);
        
        bool use_gpu = (vk && op->spv_code && data_size >= op->offload_threshold && !pressure && !packed_unsafe);
        
        if (verify) {        if (data_size > op->verify_capacity) {
            if (op->verify_backup) lau_free(op->verify_backup);
            if (op->verify_result) lau_free(op->verify_result);
            op->verify_backup = lau_malloc(data_size + 4096);
            op->verify_result = lau_malloc(data_size + 4096);
            op->verify_capacity = data_size;
        }
        memcpy(op->verify_backup, stream->data, data_size);
        if (op->cpu_entry) {
            void (*fn)(const WaveStream*, void*) = (void (*)(const WaveStream*, void*))op->cpu_entry;
            fn(stream, uniforms);
            memcpy(op->verify_result, stream->data, data_size);
        }
        memcpy(stream->data, op->verify_backup, data_size);
        if (vk && op->spv_code) {
            if (!op->gpu_pipeline) tsfi_shader_pipeline_create(vk, op, u_size);
            tsfi_shader_pipeline_execute(vk, op, stream->data, data_size, uniforms, u_size);
        }
    } else if (use_gpu) {
        if (!op->gpu_pipeline) tsfi_shader_pipeline_create(vk, op, u_size);
        double t0 = get_time_s();
        tsfi_shader_pipeline_execute(vk, op, stream->data, data_size, uniforms, u_size);
        double t1 = get_time_s();
        op->last_gpu_bw = data_size / (t1 - t0) / 1e9;
    } else if (op->cpu_entry) {
        double t0 = get_time_s();
        void (*fn)(const WaveStream*, void*) = (void (*)(const WaveStream*, void*))op->cpu_entry;
        fn(stream, uniforms);
        double t1 = get_time_s();
        op->last_cpu_bw = data_size / (t1 - t0) / 1e9;
    }
    op->execution_count++;
}

void tsfi_op_destroy(struct TSFiOperator* op, VulkanContext* vk) {
    if (!op) return;
    if (vk) {
        if (op->gpu_pipeline) vk->vkDestroyPipeline(vk->device, op->gpu_pipeline, NULL);
        if (op->gpu_layout) vk->vkDestroyPipelineLayout(vk->device, op->gpu_layout, NULL);
        if (op->gpu_descriptor_layout) vk->vkDestroyDescriptorSetLayout(vk->device, op->gpu_descriptor_layout, NULL);
        if (op->gpu_pool) vk->vkDestroyDescriptorPool(vk->device, op->gpu_pool, NULL);
        if (op->gpu_module) vk->vkDestroyShaderModule(vk->device, op->gpu_module, NULL);
        if (op->gpu_cmd) vk->vkFreeCommandBuffers(vk->device, vk->command_pool, 1, &op->gpu_cmd);
        if (op->gpu_buffer) vk->vkDestroyBuffer(vk->device, op->gpu_buffer, NULL);
        if (op->gpu_memory) vk->vkFreeMemory(vk->device, op->gpu_memory, NULL);
    }
    if (op->cpu_thunk) ThunkProxy_destroy(op->cpu_thunk);
    if (op->verify_backup) lau_free(op->verify_backup);
    if (op->verify_result) lau_free(op->verify_result);
    lau_free(op);
}
