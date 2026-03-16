#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "tsfi_wire_firmware.h"
#include "lau_memory.h"
#include "vulkan/vulkan_system.h"

extern VulkanSystem* create_vulkan_headless_system(void);

typedef struct {
    VkDevice device;
    VkSemaphore semaphore;
} ZhongSyncHandle;

int main(void) {
    printf("[INFO] Starting Zhong Firmware Test...\n");

    // 1. Initialize Vulkan context
    VulkanSystem *vs = create_vulkan_headless_system();
    if (!vs || !vs->vk || !vs->vk->device || !vs->vk->timelineSemaphore) {
        printf("[SKIP] Headless Vulkan not available or timeline semaphore not supported.\n");
        return 0; // Skip if no Vulkan support
    }

    // 2. Initialize Firmware
    LauWireFirmware *fw = tsfi_wire_firmware_get();
    assert(fw != NULL);

    // 3. Bind ReBAR and Timeline Semaphore to Firmware
    // Create a dummy ReBAR buffer (512 bytes for 1 wave)
    uint8_t *dummy_rebar = lau_malloc_wired(512);
    memset(dummy_rebar, 0, 512);

    ZhongSyncHandle *zh = lau_malloc_wired(sizeof(ZhongSyncHandle));
    zh->device = vs->vk->device;
    zh->semaphore = vs->vk->timelineSemaphore;

    fw->rtl.zhong_rebar_ptr = (uint64_t)(uintptr_t)dummy_rebar;
    fw->rtl.zhong_rebar_size = 512;
    fw->rtl.zhong_timeline_handle = (uint64_t)(uintptr_t)zh;

    // 4. Execute a Wave512 calculation (VADDPS)
    __m512i vec_a = _mm512_set1_epi32(10);
    __m512i vec_b = _mm512_set1_epi32(20);
    // Load into WRF slot 0 and 1
    for(int i=0; i<8; i++) {
        fw->rtl.wrf[0*8+i] = vec_a; // reg 0
        fw->rtl.wrf[1*8+i] = vec_b; // reg 1
    }

    if (fw->cell_wave_exec) {
        fw->cell_wave_exec(1, 2, 0, 1); // Op 1 (VADDPS): dest=2, src1=0, src2=1
    }
    
    // Now wrf[2*8+i] should be 30.
    int *res = (int*)&fw->rtl.wrf[2*8];
    assert(res[0] == 30);
    
    // Move result from reg 2 to reg 0 for REBAR store (since our mock ReBAR logic hardcodes wrf[0])
    for(int i=0; i<8; i++) fw->rtl.wrf[i] = fw->rtl.wrf[2*8+i];

    // 5. Store result to ReBAR via ZHONG_OP_REBAR_STORE
    fw->rtl.zhong_op = 0x21; // REBAR_STORE
    fw->rtl.zhong_strobe = true;
    tsfi_wire_firmware_step_peripheral(fw, NULL);
    assert(fw->rtl.zhong_done == true);
    
    fw->rtl.zhong_strobe = false;
    tsfi_wire_firmware_step_peripheral(fw, NULL);

    // Verify result from CPU-side ReBAR mapping (dummy_rebar)
    int *rebar_res = (int*)dummy_rebar;
    assert(rebar_res[0] == 30);
    printf("[PASS] Zhong ReBAR Store Verified.\n");

    // 6. Test REBAR Load
    // Clear wrf slot 0 to ensure we actually load it
    for(int i=0; i<8; i++) fw->rtl.wrf[i] = _mm512_setzero_si512();
    fw->rtl.zhong_op = 0x20; // REBAR_LOAD
    fw->rtl.zhong_strobe = true;
    tsfi_wire_firmware_step_peripheral(fw, NULL);
    assert(fw->rtl.zhong_done == true);
    fw->rtl.zhong_strobe = false;
    tsfi_wire_firmware_step_peripheral(fw, NULL);
    
    // Verify WRF[0] has loaded values from ReBAR (30)
    int *loaded_res = (int*)&fw->rtl.wrf[0];
    assert(loaded_res[0] == 30);
    assert(loaded_res[15] == 30);
    printf("[PASS] Zhong ReBAR Load Verified.\n");

    // 7. Signal completion via ZHONG_OP_SYNC_SIGNAL
    uint64_t current_val = 0;
    vkGetSemaphoreCounterValue(vs->vk->device, vs->vk->timelineSemaphore, &current_val);
    uint64_t target_val = current_val + 1;

    fw->rtl.zhong_op = 0x11; // SYNC_SIGNAL
    fw->rtl.zhong_timeline_sig_val = target_val;
    fw->rtl.zhong_strobe = true;
    tsfi_wire_firmware_step_peripheral(fw, NULL);
    assert(fw->rtl.zhong_done == true);
    
    fw->rtl.zhong_strobe = false;
    tsfi_wire_firmware_step_peripheral(fw, NULL);

    // Verify Vulkan timeline semaphore signaled
    uint64_t new_val = 0;
    vkGetSemaphoreCounterValue(vs->vk->device, vs->vk->timelineSemaphore, &new_val);
    assert(new_val >= target_val);
    printf("[PASS] Zhong Timeline Signal Verified.\n");

    // 8. Test ZHONG_OP_SYNC_WAIT (wait for the value we just signaled)
    fw->rtl.zhong_op = 0x10; // SYNC_WAIT
    fw->rtl.zhong_timeline_wait_val = new_val;
    fw->rtl.zhong_strobe = true;
    tsfi_wire_firmware_step_peripheral(fw, NULL);
    assert(fw->rtl.zhong_done == true);
    fw->rtl.zhong_strobe = false;
    tsfi_wire_firmware_step_peripheral(fw, NULL);
    
    printf("[PASS] Zhong Timeline Wait Verified.\n");

    // Clean up
    lau_free(zh);
    lau_free(dummy_rebar);
    
    printf("=== Zhong Standard Cells Verification Complete ===\n");
        extern void tsfi_wire_firmware_teardown(void);
    tsfi_wire_firmware_teardown();
    extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}
