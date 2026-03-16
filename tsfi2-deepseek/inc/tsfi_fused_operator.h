#ifndef TSFI_FUSED_OPERATOR_H
#define TSFI_FUSED_OPERATOR_H

#include "tsfi_operator.h"

typedef struct TSFiFusedOp {
    TSFiOperator** ops;
    size_t count;
    char* name;
    
    // Fused Resources
    VkCommandBuffer fused_cmd;
    void (*fused_cpu_entry)(void);
} TSFiFusedOp;

TSFiFusedOp* tsfi_fused_create(const char* name);
void tsfi_fused_add(TSFiFusedOp* fused, TSFiOperator* op);
void tsfi_fused_bake(TSFiFusedOp* fused, VulkanContext* vk, size_t buffer_size, void* push_constants, size_t pc_size);
void tsfi_fused_dispatch(TSFiFusedOp* fused, VulkanContext* vk, WaveStream* stream);
void tsfi_fused_destroy(TSFiFusedOp* fused, VulkanContext* vk);

#endif
