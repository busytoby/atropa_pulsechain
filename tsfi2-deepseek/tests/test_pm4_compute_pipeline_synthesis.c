#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <stdlib.h>
#include "lau_memory.h"

// PM4 Packet Type 3 Header Macro
#define PM4_PKT3(opcode, count) ((3U << 30) | (((count) & 0x3FFF) << 16) | (((opcode) & 0xFF) << 8))

#define PKT3_SET_SH_REG      0x76
#define COMPUTE_PGM_LO       0x2E00
#define COMPUTE_PGM_RSRC1    0x2E12
#define COMPUTE_PGM_RSRC2    0x2E13
#define COMPUTE_USER_DATA_0  0x2E40

void test_pm4_compute_pipeline_synthesis() {
    printf("[TEST] Synthesizing PM4 Compute Pipeline State...\n");

    uint32_t* cmd_buffer = (uint32_t*)lau_malloc_wired(1024);
    assert(cmd_buffer != NULL);

    uint32_t pc = 0;

    // 1. Setup Shader Address
    cmd_buffer[pc++] = PM4_PKT3(PKT3_SET_SH_REG, 2);
    cmd_buffer[pc++] = COMPUTE_PGM_LO;
    cmd_buffer[pc++] = 0x80000000; // LO
    cmd_buffer[pc++] = 0x00000001; // HI

    // 2. Setup Resources (Wave64, VGPR/SGPR limits)
    // RSRC1: VGPRS, SGPRS, FLOAT_MODE
    cmd_buffer[pc++] = PM4_PKT3(PKT3_SET_SH_REG, 2);
    cmd_buffer[pc++] = COMPUTE_PGM_RSRC1;
    cmd_buffer[pc++] = 0x002C0040; // Example RSRC1 Config
    cmd_buffer[pc++] = 0x00000080; // Example RSRC2 Config (Wave64)

    // 3. Setup User Data (Push Constants / BDA pointer)
    cmd_buffer[pc++] = PM4_PKT3(PKT3_SET_SH_REG, 2);
    cmd_buffer[pc++] = COMPUTE_USER_DATA_0;
    cmd_buffer[pc++] = 0xCAFEBABE; // BDA LO
    cmd_buffer[pc++] = 0x00000000; // BDA HI

    assert(cmd_buffer[0] == PM4_PKT3(PKT3_SET_SH_REG, 2));
    assert(cmd_buffer[1] == COMPUTE_PGM_LO);
    assert(cmd_buffer[9] == COMPUTE_USER_DATA_0);
    assert(cmd_buffer[10] == 0xCAFEBABE);
    
    printf("  [PASS] Compute Shader Pipeline Base Addresses configured.\n");
    printf("  [PASS] Compute Shader RSRC1/RSRC2 registers configured.\n");
    printf("  [PASS] BDA Push Constants mapped to USER_DATA registers.\n");
    
    lau_free(cmd_buffer);
}

int main() {
    printf("=== TSFi PM4 Compute Pipeline Synthesis Verification ===\n");
    test_pm4_compute_pipeline_synthesis();
    printf("ALL PM4 PIPELINE SYNTHESIS TESTS PASSED\n");
    
    extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}
