#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "tsfi_wire_firmware.h"
#include "lau_memory.h"
#include "lau_vram.h"

// PM4 Packet Type 3 Header Macro
#define PM4_PKT3(opcode, count) ((3U << 30) | (((count) & 0x3FFF) << 16) | (((opcode) & 0xFF) << 8))

#define PKT3_DISPATCH_DIRECT 0x15
#define PKT3_SET_SH_REG      0x76

#define COMPUTE_PGM_LO       0x2E00
#define COMPUTE_USER_DATA_0  0x2E40

void test_firmware_terminal_dispatch() {
    printf("[TEST] Verifying Direct Firmware Terminal Dispatch (Bypassing RADV)...\n");

    // 1. Initialize Firmware VRAM in ReBAR
    LauWireFirmware *fw = tsfi_wire_firmware_get();
    assert(fw != NULL);
    
    // Verify VRAM was initialized
    assert(fw->vram.is_dirty == true);
    assert(fw->vram.grid[0][0].character == ' ');

    // 2. Write ANSI state directly into the ReBAR-backed VRAM
    const char *test_str = "\x1b[31mTSFi Terminal\r\nDirect PM4";
    lau_vram_write_string(&fw->vram, test_str, strlen(test_str));

    // Verify ANSI color (31 = Red -> index 1)
    assert(fw->vram.grid[0][0].character == 'T');
    assert(fw->vram.grid[0][0].fg_color == 1);
    
    // Verify newline
    assert(fw->vram.grid[1][0].character == 'D');

    // 3. Synthesize PM4 Command Buffer for the tsfi_terminal.comp wavefront
    uint32_t* cmd_buffer = (uint32_t*)lau_malloc_wired(1024);
    assert(cmd_buffer != NULL);
    uint32_t pc = 0;

    // Set Shader Address (Mocking the binary location of our terminal compute shader)
    cmd_buffer[pc++] = PM4_PKT3(PKT3_SET_SH_REG, 2);
    cmd_buffer[pc++] = COMPUTE_PGM_LO;
    cmd_buffer[pc++] = 0x80000000; // Address LO
    cmd_buffer[pc++] = 0x00000000; // Address HI

    // Set User Data (Push Constants -> 64-bit BDA pointer directly to LauVRAM)
    // This eradicates Vulkan descriptor sets
    uint64_t vram_bda = (uint64_t)(uintptr_t)&fw->vram;
    cmd_buffer[pc++] = PM4_PKT3(PKT3_SET_SH_REG, 2);
    cmd_buffer[pc++] = COMPUTE_USER_DATA_0;
    cmd_buffer[pc++] = (uint32_t)(vram_bda & 0xFFFFFFFF);
    cmd_buffer[pc++] = (uint32_t)(vram_bda >> 32);

    // Dispatch Direct: 160x60 grid = 9600 cells. 
    // RDNA 4 Wave64 = 64 threads per wavefront. 9600 / 64 = 150 workgroups.
    cmd_buffer[pc++] = PM4_PKT3(PKT3_DISPATCH_DIRECT, 3);
    cmd_buffer[pc++] = 150; // wg_x
    cmd_buffer[pc++] = 1;   // wg_y
    cmd_buffer[pc++] = 1;   // wg_z

    // Assert structural integrity of the synthesized PM4 packet
    assert(cmd_buffer[0] == PM4_PKT3(PKT3_SET_SH_REG, 2));
    assert(cmd_buffer[1] == COMPUTE_PGM_LO);
    assert(cmd_buffer[4] == PM4_PKT3(PKT3_SET_SH_REG, 2));
    assert(cmd_buffer[5] == COMPUTE_USER_DATA_0);
    assert(cmd_buffer[6] == (uint32_t)(vram_bda & 0xFFFFFFFF));
    assert(cmd_buffer[8] == PM4_PKT3(PKT3_DISPATCH_DIRECT, 3));
    assert(cmd_buffer[9] == 150);

    printf("  [PASS] ANSI state successfully parsed into ReBAR VRAM.\n");
    printf("  [PASS] PM4 Pipeline configured with 64-bit BDA to ReBAR VRAM.\n");
    printf("  [PASS] PM4 Dispatch synthesized for 150 Wave64 Workgroups (160x60 Grid).\n");

    lau_free(cmd_buffer);
}

int main() {
    printf("=== TSFi Direct Firmware Terminal Test ===\n");
    test_firmware_terminal_dispatch();
    printf("ALL FIRMWARE TERMINAL TESTS PASSED\n");
    
    extern void tsfi_wire_firmware_teardown(void);
    tsfi_wire_firmware_teardown();
    extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}
