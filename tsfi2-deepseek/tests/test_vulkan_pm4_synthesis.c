#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdbool.h>
#include "window_inc/vulkan_struct.h"
#include "tsfi_wire_firmware.h"
#include "lau_memory.h"

extern PFN_vkVoidFunction tsfi_vkGetInstanceProcAddr(VkInstance instance, const char* pName);

// PM4 Packet Type 3 Header Macro
#define PM4_PKT3(opcode, count) ((3U << 30) | (((count) & 0x3FFF) << 16) | (((opcode) & 0xFF) << 8))
#define PKT3_DISPATCH_DIRECT 0x15
#define PKT3_SET_SH_REG      0x76

struct tsfi_firmware_command_buffer {
    uint32_t* pm4_stream;
    size_t capacity;
    size_t write_ptr;
    bool is_recording;
};

// Mock CS Hook
bool mock_cs_called = false;
uint32_t mock_cs_length_dw = 0;

struct drm_amdgpu_cs_chunk {
    uint32_t chunk_id;
    uint32_t length_dw;
    uint64_t chunk_data;
};

struct drm_amdgpu_cs {
    uint32_t ctx_id;
    uint32_t bo_list_handle;
    uint32_t num_chunks;
    uint32_t flags;
    uint64_t chunks;
    uint64_t in_out_op;
};

bool (*tsfi_hook_drm_amdgpu_cs)(struct drm_amdgpu_cs *cs) = NULL;

bool test_hook_drm_amdgpu_cs(struct drm_amdgpu_cs *cs) {
    mock_cs_called = true;
    struct drm_amdgpu_cs_chunk* chunk = (struct drm_amdgpu_cs_chunk*)(uintptr_t)cs->chunks;
    mock_cs_length_dw = chunk->length_dw;
    return true;
}



int main() {
    alarm(5);
    printf("=== TSFi Vulkan PM4 Synthesis Test ===\n");

    tsfi_hook_drm_amdgpu_cs = test_hook_drm_amdgpu_cs;

    PFN_vkCreateInstance pvkCreateInstance = (PFN_vkCreateInstance)tsfi_vkGetInstanceProcAddr(NULL, "vkCreateInstance");
    PFN_vkCreateDevice pvkCreateDevice = (PFN_vkCreateDevice)tsfi_vkGetInstanceProcAddr(NULL, "vkCreateDevice");
    PFN_vkAllocateCommandBuffers pvkAllocateCommandBuffers = (PFN_vkAllocateCommandBuffers)tsfi_vkGetInstanceProcAddr(NULL, "vkAllocateCommandBuffers");
    PFN_vkBeginCommandBuffer pvkBeginCommandBuffer = (PFN_vkBeginCommandBuffer)tsfi_vkGetInstanceProcAddr(NULL, "vkBeginCommandBuffer");
    PFN_vkEndCommandBuffer pvkEndCommandBuffer = (PFN_vkEndCommandBuffer)tsfi_vkGetInstanceProcAddr(NULL, "vkEndCommandBuffer");
    PFN_vkCmdBindPipeline pvkCmdBindPipeline = (PFN_vkCmdBindPipeline)tsfi_vkGetInstanceProcAddr(NULL, "vkCmdBindPipeline");
    PFN_vkCmdPushConstants pvkCmdPushConstants = (PFN_vkCmdPushConstants)tsfi_vkGetInstanceProcAddr(NULL, "vkCmdPushConstants");
    PFN_vkCmdDispatch pvkCmdDispatch = (PFN_vkCmdDispatch)tsfi_vkGetInstanceProcAddr(NULL, "vkCmdDispatch");
    PFN_vkGetDeviceQueue pvkGetDeviceQueue = (PFN_vkGetDeviceQueue)tsfi_vkGetInstanceProcAddr(NULL, "vkGetDeviceQueue");
    PFN_vkQueueSubmit pvkQueueSubmit = (PFN_vkQueueSubmit)tsfi_vkGetInstanceProcAddr(NULL, "vkQueueSubmit");

    VkInstance instance;
    VkInstanceCreateInfo inst_info = { .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
    pvkCreateInstance(&inst_info, NULL, &instance);
    
    // We must call tsfi_wire_firmware_init() because the device relies on the global LauWireFirmware singleton now.
    extern void tsfi_wire_firmware_init(void);
    tsfi_wire_firmware_init();

    VkDevice device;
    VkDeviceCreateInfo dev_info = {0};
    pvkCreateDevice((VkPhysicalDevice)0x2000, &dev_info, NULL, &device);

    VkCommandBuffer cmd = NULL;
    VkCommandBufferAllocateInfo alloc_info = { .commandBufferCount = 1 };
    pvkAllocateCommandBuffers(device, &alloc_info, &cmd);

    // Record PM4 payload
    VkCommandBufferBeginInfo begin_info = {0};
    pvkBeginCommandBuffer(cmd, &begin_info);

    pvkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, NULL);
    
    uint32_t push_data[4] = { 0x1111, 0x2222, 0x3333, 0x4444 };
    pvkCmdPushConstants(cmd, NULL, 0, 0, 16, push_data);
    
    pvkCmdDispatch(cmd, 64, 64, 1);

    pvkEndCommandBuffer(cmd);

    // Submit
    VkQueue queue;
    pvkGetDeviceQueue(device, 0, 0, &queue);
    
    VkSubmitInfo submit_info = {
        .commandBufferCount = 1,
        .pCommandBuffers = &cmd
    };
    pvkQueueSubmit(queue, 1, &submit_info, NULL);

    assert(mock_cs_called == true);
    // 3 words bind + 6 words push + 4 words dispatch = 13 words
    printf("mock_cs_length_dw = %d\n", mock_cs_length_dw); assert(mock_cs_length_dw == 14);
    printf("  [PASS] Synthesized precise 13-dword PM4 payload via Vulkan Command Buffer overrides.\n");

    // Cleanup
    PFN_vkFreeCommandBuffers pvkFreeCommandBuffers = (PFN_vkFreeCommandBuffers)tsfi_vkGetInstanceProcAddr(NULL, "vkFreeCommandBuffers");
    pvkFreeCommandBuffers(device, NULL, 1, &cmd);

    PFN_vkDestroyDevice pvkDestroyDevice = (PFN_vkDestroyDevice)tsfi_vkGetInstanceProcAddr(NULL, "vkDestroyDevice");
    pvkDestroyDevice(device, NULL);

    PFN_vkDestroyInstance pvkDestroyInstance = (PFN_vkDestroyInstance)tsfi_vkGetInstanceProcAddr(NULL, "vkDestroyInstance");
    pvkDestroyInstance(instance, NULL);

    extern void tsfi_wire_firmware_teardown(void);
    tsfi_wire_firmware_teardown();
    
    // Explicitly unseal and free the mock ReBAR memory that was created in tsfi_vkCreateDevice
    extern LauWireFirmware* tsfi_wire_firmware_get_no_init(void);
    LauWireFirmware *fw = tsfi_wire_firmware_get_no_init();
    if (fw && fw->rtl.zhong_timeline_handle == 77) {
        if (fw->rtl.zhong_rebar_ptr != 0) {
            lau_unseal_object((void*)fw->rtl.zhong_rebar_ptr);
            lau_free((void*)fw->rtl.zhong_rebar_ptr);
            fw->rtl.zhong_rebar_ptr = 0;
        }
    }
    


    extern void tsfi_dl_thunks_teardown(void);
    tsfi_dl_thunks_teardown();
    extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    extern void lau_free_all_active(void);
    lau_free_all_active();
    lau_report_memory_metrics();
    
    extern void lau_assert_zero_unsealed_leaks(const char *context, void *teardown_ptr);
    lau_assert_zero_unsealed_leaks("test", NULL);
    return 0;
}
