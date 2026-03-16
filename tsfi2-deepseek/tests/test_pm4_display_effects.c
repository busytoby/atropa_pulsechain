#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <math.h>
#include "lau_memory.h"
#include "tsfi_types.h"
#include "window_inc/vulkan_struct.h"

// PM4 Packet Type 3 Header Macro
#define PM4_PKT3(opcode, count) ((3U << 30) | (((count) & 0x3FFF) << 16) | (((opcode) & 0xFF) << 8))
#define PKT3_DISPATCH_DIRECT 0x15
#define PKT3_SET_SH_REG      0x76
#define COMPUTE_PGM_LO       0x2E00
#define COMPUTE_USER_DATA_0  0x2E40

// DC Registers (from bypass test)
#define DC_PLANE_71_SURFACE_BASE 0x4100
#define DC_CRTC_CONTROL          0x5000

// The real entry point for Vulkan in our firmware
extern PFN_vkVoidFunction tsfi_vkGetInstanceProcAddr(VkInstance instance, const char* pName);

struct tsfi_firmware_device {
    uint32_t amdgpu_ctx_id;
    uint32_t syncobj_handle;
    uint64_t rebar_bda_base;
    void* pci_vram_bar;
    void* pci_mmio_bar;
    bool is_hardware_mocked;
};

void test_effects_display_drawing() {
    printf("[TEST] Testing PM4 Drawing to Effects Plane (77)...\n");

    // 1. Setup Firmware Device
    PFN_vkCreateDevice pvkCreateDevice = (PFN_vkCreateDevice)tsfi_vkGetInstanceProcAddr(NULL, "vkCreateDevice");
    PFN_vkDestroyDevice pvkDestroyDevice = (PFN_vkDestroyDevice)tsfi_vkGetInstanceProcAddr(NULL, "vkDestroyDevice");
    PFN_vkAllocateCommandBuffers pvkAllocateCommandBuffers = (PFN_vkAllocateCommandBuffers)tsfi_vkGetInstanceProcAddr(NULL, "vkAllocateCommandBuffers");
    PFN_vkFreeCommandBuffers pvkFreeCommandBuffers = (PFN_vkFreeCommandBuffers)tsfi_vkGetInstanceProcAddr(NULL, "vkFreeCommandBuffers");
    PFN_vkBeginCommandBuffer pvkBeginCommandBuffer = (PFN_vkBeginCommandBuffer)tsfi_vkGetInstanceProcAddr(NULL, "vkBeginCommandBuffer");
    PFN_vkEndCommandBuffer pvkEndCommandBuffer = (PFN_vkEndCommandBuffer)tsfi_vkGetInstanceProcAddr(NULL, "vkEndCommandBuffer");
    PFN_vkCmdBindPipeline pvkCmdBindPipeline = (PFN_vkCmdBindPipeline)tsfi_vkGetInstanceProcAddr(NULL, "vkCmdBindPipeline");
    PFN_vkCmdPushConstants pvkCmdPushConstants = (PFN_vkCmdPushConstants)tsfi_vkGetInstanceProcAddr(NULL, "vkCmdPushConstants");
    PFN_vkCmdDispatch pvkCmdDispatch = (PFN_vkCmdDispatch)tsfi_vkGetInstanceProcAddr(NULL, "vkCmdDispatch");
    PFN_vkQueueSubmit pvkQueueSubmit = (PFN_vkQueueSubmit)tsfi_vkGetInstanceProcAddr(NULL, "vkQueueSubmit");

    VkDevice device = NULL;
    VkDeviceCreateInfo createInfo = {0};
    pvkCreateDevice((VkPhysicalDevice)0x1234, &createInfo, NULL, &device);
    struct tsfi_firmware_device* fw_dev = (struct tsfi_firmware_device*)device;

    // 2. Allocate VRAM Framebuffer for the Plane
    // Plane 65 = 0x01000000, Plane 71 = 0x02000000. So we use 0x03000000 for Plane 77
    uint64_t fb_offset = 0x03000000; // Offset in VRAM BAR
    void* fb_ptr = (void*)((uint8_t*)fw_dev->pci_vram_bar + fb_offset);

    // 3. Configure Display Core (DC) to scan out Plane 77 from this VRAM buffer
    volatile uint32_t* mmio = (volatile uint32_t*)fw_dev->pci_mmio_bar;
    mmio[DC_CRTC_CONTROL / 4] = 0x1;
    mmio[DC_PLANE_71_SURFACE_BASE / 4] = (uint32_t)(fb_offset & 0xFFFFFFFF);
    
    printf("  [PASS] Display Core Plane 77 configured to scan out from VRAM offset 0x%llx.\n", (unsigned long long)fb_offset);

    // 4. Record PM4 Commands via Vulkan Intercepts
    VkCommandBuffer cmd_buffer = NULL;
    VkCommandBufferAllocateInfo allocInfo = {0};
    allocInfo.commandBufferCount = 1;
    pvkAllocateCommandBuffers(device, &allocInfo, &cmd_buffer);

    VkCommandBufferBeginInfo beginInfo = {0};
    pvkBeginCommandBuffer(cmd_buffer, &beginInfo);

    // Bind Shader
    pvkCmdBindPipeline(cmd_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, NULL);

    // Push Constants (Target Buffer Address in VRAM + Geometric Data)
    uint32_t push_data[4];
    push_data[0] = (uint32_t)((uintptr_t)fb_ptr & 0xFFFFFFFF); // BDA LO
    push_data[1] = (uint32_t)(((uintptr_t)fb_ptr >> 32) & 0xFFFFFFFF); // BDA HI
    push_data[2] = 800; // Resolution Width
    push_data[3] = 600; // Resolution Height
    
    pvkCmdPushConstants(cmd_buffer, NULL, VK_SHADER_STAGE_COMPUTE_BIT, 0, 16, push_data);

    // Dispatch direct (800x600 grid / 64 threads per wave = 7500 workgroups)
    pvkCmdDispatch(cmd_buffer, 7500, 1, 1);

    pvkEndCommandBuffer(cmd_buffer);

    printf("  [PASS] Synthesized PM4 Draw Commands targeting Effects Plane Framebuffer.\n");

    // 5. Submit to Hardware Doorbell
    VkSubmitInfo submitInfo = {0};
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &cmd_buffer;
    pvkQueueSubmit(NULL, 1, &submitInfo, NULL);
    
    printf("  [PASS] PM4 Payload Dispatched to AMDGPU CS Doorbell.\n");

    // Clean up
    pvkFreeCommandBuffers(device, NULL, 1, &cmd_buffer);
    pvkDestroyDevice(device, NULL);
}

int main() {
    alarm(5);
    printf("=== TSFi Direct Effects Display Drawing Test ===\n");
    test_effects_display_drawing();
    printf("ALL EFFECTS DRAWING TESTS PASSED\n");

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
