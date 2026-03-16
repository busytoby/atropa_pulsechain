#include "tsfi_wire_firmware.h"
#include "tsfi_io.h"
#include "lau_memory.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Mock Vulkan Pointers
typedef void* VkCommandBuffer;
typedef void* VkVideoSessionKHR;
typedef void* VkVideoEncodeInfoKHR;
typedef void* VkVideoBeginCodingInfoKHR;
typedef void* VkVideoEndCodingInfoKHR;

typedef struct {
    VkCommandBuffer cmd_buffer;
    VkVideoSessionKHR session;
    void (*vkCmdBeginVideoCodingKHR)(VkCommandBuffer, const VkVideoBeginCodingInfoKHR*);
    void (*vkCmdEncodeVideoKHR)(VkCommandBuffer, const VkVideoEncodeInfoKHR*);
    void (*vkCmdEndVideoCodingKHR)(VkCommandBuffer, const VkVideoEndCodingInfoKHR*);
} VulkanVideoDispatch;

static int mock_calls = 0;
void mock_vkCmdBeginVideoCodingKHR(VkCommandBuffer cb, const VkVideoBeginCodingInfoKHR* info) { (void)cb; (void)info; mock_calls++; }
void mock_vkCmdEncodeVideoKHR(VkCommandBuffer cb, const VkVideoEncodeInfoKHR* info) { (void)cb; (void)info; mock_calls++; }
void mock_vkCmdEndVideoCodingKHR(VkCommandBuffer cb, const VkVideoEndCodingInfoKHR* info) { (void)cb; (void)info; mock_calls++; }

#include "../thunks/tsfi_broadcast_encode_thunk.c"

int main() {
    tsfi_io_printf(stdout, "=== TSFi Firmware: Broadcast Encode Thunk Validation ===\n");
    
    LauRegisterBank regs;
    memset(&regs, 0, sizeof(LauRegisterBank));

    uint8_t* out_buf = (uint8_t*)lau_malloc_wired(4096);
    if (!out_buf) return 1;
    memset(out_buf, 0, 4096);

    VulkanVideoDispatch mock_dispatch;
    mock_dispatch.cmd_buffer = (void*)0xABCD;
    mock_dispatch.session = (void*)0x1234;
    mock_dispatch.vkCmdBeginVideoCodingKHR = mock_vkCmdBeginVideoCodingKHR;
    mock_dispatch.vkCmdEncodeVideoKHR = mock_vkCmdEncodeVideoKHR;
    mock_dispatch.vkCmdEndVideoCodingKHR = mock_vkCmdEndVideoCodingKHR;

    regs.sgpr[10] = (uint32_t)((uintptr_t)out_buf & 0xFFFFFFFF);
    regs.sgpr[14] = (uint32_t)((uintptr_t)out_buf >> 32);
    
    regs.sgpr[13] = (uint32_t)((uintptr_t)&mock_dispatch & 0xFFFFFFFF);
    regs.sgpr[16] = (uint32_t)((uintptr_t)&mock_dispatch >> 32);

    regs.sgpr[11] = 1845; 
    regs.sgpr[12] = 2500000; 
    regs.sgpr[15] = 0xDEADBEEF; 

    tsfi_io_printf(stdout, "[EXEC] Triggering RTL Broadcast Strobe (Epoch: %u)...\n", regs.sgpr[11]);
    
    tsfi_broadcast_encode_thunk(&regs);

    uint32_t success_flag = regs.sgpr[0];
    if (success_flag != 1) {
        tsfi_io_printf(stderr, "[FAIL] Encode thunk returned failure. Flag: %u\n", success_flag);
        lau_free(out_buf);
        return 1;
    }

    uint32_t pipeline_engaged = regs.sgpr[3];
    if (pipeline_engaged == 1 && mock_calls == 3) {
        tsfi_io_printf(stdout, "[VERIFIED] Vulkan Video Pipeline successfully engaged natively within Thunk.\n");
    } else {
        tsfi_io_printf(stderr, "[FAIL] Vulkan Video Pipeline was not properly dispatched.\n");
        lau_free(out_buf);
        return 1;
    }

    uint32_t payload_len = regs.sgpr[1];
    uint32_t mirrored_epoch = regs.sgpr[2];

    tsfi_io_printf(stdout, "[SUCCESS] Thunk successfully generated %u bytes of hardware metadata.\n", payload_len);

    if (mirrored_epoch != 1845) {
        tsfi_io_printf(stderr, "[FAIL] Clock synchronization lost. Expected 1845, got %u\n", mirrored_epoch);
        lau_free(out_buf);
        return 1;
    }

    if (out_buf[0] == 0x00 && out_buf[1] == 0x00 && out_buf[2] == 0x00 && out_buf[3] == 0x01 && out_buf[4] == 0x06) {
        tsfi_io_printf(stdout, "[VERIFIED] SEI NALU Prefix geometrically perfect.\n");
    } else {
        tsfi_io_printf(stderr, "[FAIL] Bitstream NALU header corrupt.\n");
        lau_free(out_buf);
        return 1;
    }

    uint32_t extracted_epoch, extracted_seed, extracted_bitrate;
    memcpy(&extracted_epoch, &out_buf[5], 4);
    memcpy(&extracted_seed, &out_buf[9], 4);
    memcpy(&extracted_bitrate, &out_buf[13], 4);

    if (extracted_epoch == 1845 && extracted_seed == 0xDEADBEEF && extracted_bitrate == 2500000) {
        tsfi_io_printf(stdout, "[VERIFIED] SGPR[15] (Daiichi Entropy) and RTL Clock successfully transduced to bitstream.\n");
    } else {
        tsfi_io_printf(stderr, "[FAIL] Ingested hardware state variables did not match verification matrix.\n");
        lau_free(out_buf);
        return 1;
    }

    lau_free(out_buf);
        extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}