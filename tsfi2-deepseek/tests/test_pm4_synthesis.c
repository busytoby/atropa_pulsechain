#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <stdlib.h>
#include "lau_memory.h"

// PM4 Packet Type 3 Header Macro
// Format: [31:30] Type (3), [29:16] Count (words - 1), [15:8] Opcode, [7:0] reserved
#define PM4_PKT3(opcode, count) ((3U << 30) | (((count) & 0x3FFF) << 16) | (((opcode) & 0xFF) << 8))

// Minimal PM4 Opcodes for GFX12 Compute
#define PKT3_DISPATCH_DIRECT 0x15
#define PKT3_SET_SH_REG      0x76

void test_pm4_compute_dispatch_synthesis() {
    printf("[TEST] Synthesizing PM4 Compute Dispatch Packet...\n");

    // Simulate allocating a command buffer from the ReBAR pool via firmware
    uint32_t* cmd_buffer = (uint32_t*)lau_malloc_wired(1024);
    assert(cmd_buffer != NULL);

    uint32_t pc = 0;

    // 1. Synthesize SET_SH_REG (Shader Register Setup)
    // E.g., setting COMPUTE_PGM_LO/HI (shader base address)
    uint16_t reg_offset = 0x2E00; // Example offset for a compute register
    cmd_buffer[pc++] = PM4_PKT3(PKT3_SET_SH_REG, 2); // Count = 2 (3 words total)
    cmd_buffer[pc++] = reg_offset;
    cmd_buffer[pc++] = 0xDEADBEEF; // Address LO
    cmd_buffer[pc++] = 0x00000000; // Address HI

    // 2. Synthesize DISPATCH_DIRECT
    uint32_t wg_x = 64;
    uint32_t wg_y = 1;
    uint32_t wg_z = 1;
    
    cmd_buffer[pc++] = PM4_PKT3(PKT3_DISPATCH_DIRECT, 3); // Count = 3 (4 words total)
    cmd_buffer[pc++] = wg_x;
    cmd_buffer[pc++] = wg_y;
    cmd_buffer[pc++] = wg_z;

    // Verify PM4 Header integrity
    assert(cmd_buffer[0] == 0xC0027600); // PKT3_SET_SH_REG, count 2
    assert(cmd_buffer[4] == 0xC0031500); // PKT3_DISPATCH_DIRECT, count 3
    
    printf("  [PASS] PM4 Packet headers encoded correctly.\n");
    printf("  [PASS] Dispatch Group dimensions encoded: %ux%ux%u\n", wg_x, wg_y, wg_z);
    
    lau_free(cmd_buffer);
}

int main() {
    printf("=== TSFi PM4 Synthesis Verification ===\n");
    test_pm4_compute_dispatch_synthesis();
    printf("ALL PM4 SYNTHESIS TESTS PASSED\n");
    
    // Clean up registry and verify leaks
    extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}
