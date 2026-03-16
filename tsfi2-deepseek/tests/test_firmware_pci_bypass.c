#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <stdbool.h>
#include <string.h>
#include <sys/mman.h>
#include "lau_memory.h"
#include "window_inc/vulkan_struct.h"

// The real entry point for Vulkan in our firmware
extern PFN_vkVoidFunction tsfi_vkGetInstanceProcAddr(VkInstance instance, const char* pName);

// Firmware Context replacing RADV
struct tsfi_firmware_device {
    uint32_t amdgpu_ctx_id;
    uint32_t syncobj_handle;
    uint64_t rebar_bda_base;
    
    // Direct PCI Hardware Access
    void* pci_vram_bar; // resource0
    void* pci_mmio_bar; // resource2
    bool is_hardware_mocked;
};

// Mock Display Core (DC) Register Offsets (in MMIO Space)
#define DC_PLANE_65_SURFACE_BASE 0x4000
#define DC_PLANE_71_SURFACE_BASE 0x4100
#define DC_PLANE_77_SURFACE_BASE 0x4200

#define DC_CRTC_CONTROL          0x5000

void test_firmware_pci_bypass_and_dc_setup() {
    printf("[TEST] Verifying Firmware PCI BAR Mapping & Display Core Setup...\n");
    
    // 1. Get the proc addrs
    PFN_vkCreateDevice pvkCreateDevice = (PFN_vkCreateDevice)tsfi_vkGetInstanceProcAddr(NULL, "vkCreateDevice");
    PFN_vkDestroyDevice pvkDestroyDevice = (PFN_vkDestroyDevice)tsfi_vkGetInstanceProcAddr(NULL, "vkDestroyDevice");
    
    assert(pvkCreateDevice != NULL);
    assert(pvkDestroyDevice != NULL);

    // 2. Call CreateDevice
    VkDevice device = NULL;
    VkDeviceCreateInfo createInfo = {0};
    VkResult result = pvkCreateDevice((VkPhysicalDevice)0x1234, &createInfo, NULL, &device);
    
    assert(result == VK_SUCCESS);
    assert(device != NULL);
    
    // 3. Verify the PCI BARs are mapped
    struct tsfi_firmware_device* fw_dev = (struct tsfi_firmware_device*)device;
    assert(fw_dev->pci_vram_bar != NULL);
    assert(fw_dev->pci_mmio_bar != NULL);
    
    if (fw_dev->is_hardware_mocked) {
        printf("  [INFO] Running in unprivileged mode: Using Mock PCI BARs.\n");
    } else {
        printf("  [INFO] Running in privileged mode: Real PCI BARs Mapped!\n");
    }
    
    printf("  [PASS] PCI VRAM & MMIO BARs successfully mapped into firmware context.\n");

    // 4. Simulate programming the Display Core (DC) directly via MMIO
    volatile uint32_t* mmio = (volatile uint32_t*)fw_dev->pci_mmio_bar;
    
    printf("[TEST] Configuring Display Controller (DC) for TSFi Layers...\n");
    
    // Turn on the physical display clock (Simulated)
    mmio[DC_CRTC_CONTROL / 4] = 0x1;
    
    // Point the planes to VRAM addresses (Simulating framebuffers located in the VRAM BAR)
    uint64_t bg_fb_offset = 0x01000000;  // Plane 65: Background
    uint64_t fg_fb_offset = 0x02000000;  // Plane 71: Foreground
    uint64_t fx_fb_offset = 0x03000000;  // Plane 77: Effects
    
    mmio[DC_PLANE_65_SURFACE_BASE / 4] = (uint32_t)(bg_fb_offset & 0xFFFFFFFF);
    mmio[DC_PLANE_71_SURFACE_BASE / 4] = (uint32_t)(fg_fb_offset & 0xFFFFFFFF);
    mmio[DC_PLANE_77_SURFACE_BASE / 4] = (uint32_t)(fx_fb_offset & 0xFFFFFFFF);
    
    // Verify the registers were "written"
    assert(mmio[DC_CRTC_CONTROL / 4] == 0x1);
    assert(mmio[DC_PLANE_65_SURFACE_BASE / 4] == 0x01000000);
    assert(mmio[DC_PLANE_71_SURFACE_BASE / 4] == 0x02000000);
    assert(mmio[DC_PLANE_77_SURFACE_BASE / 4] == 0x03000000);

    printf("  [PASS] Display Core Plane 65 (Background) configured.\n");
    printf("  [PASS] Display Core Plane 71 (Foreground) configured.\n");
    printf("  [PASS] Display Core Plane 77 (Effects) configured.\n");

    // 5. Destroy it
    pvkDestroyDevice(device, NULL);
    printf("  [PASS] vkDestroyDevice successfully unmapped and freed context.\n");
}

int main() {
    alarm(5);
    printf("=== TSFi Firmware PCI Hardware Bypass Verification ===\n");
    test_firmware_pci_bypass_and_dc_setup();
    printf("ALL PCI BYPASS TESTS PASSED\n");

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