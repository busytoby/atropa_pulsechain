#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <assert.h>

// 64KB Virtual memory representing the RAU
static uint8_t virtual_rau_mem[65536];

// Video memory boundaries
#define VRAM_START 0x0400
#define VRAM_END   0x0800 // 1024 bytes (40x25 character matrix)
#define WIDTH      40
#define HEIGHT     12

// 1. RED ANALYZER Gating: Static image frame constraint validation
static bool red_analyzer_validate_frame_dim(uint32_t width, uint32_t height) {
    if (width > 80 || height > 25) {
        return false; // Rejected by RED rail (resolution exceeds terminal limits)
    }
    return true;
}

// 2. BLACK XPLSM Gating: Dynamic VRAM boundary checks
static bool black_xplsm_write_vram(uint32_t address, uint8_t val, uint32_t *interrupt_out) {
    if (address < VRAM_START || address >= VRAM_END) {
        if (interrupt_out) *interrupt_out = 25; // Trap 25: Video Memory Range Overflow
        return false; // BLACK rail block
    }
    virtual_rau_mem[address] = val;
    return true;
}

// Simulate running the XPL tiger sunset code
static void run_sunset_rasterizer(uint32_t *interrupt_out) {
    // Clear video memory
    for (uint32_t addr = VRAM_START; addr < VRAM_END; addr++) {
        black_xplsm_write_vram(addr, ' ', interrupt_out);
    }

    // Draw sunset horizon (Y=10)
    for (uint32_t x = 0; x < WIDTH; x++) {
        uint32_t offset = VRAM_START + 10 * WIDTH + x;
        black_xplsm_write_vram(offset, '=', interrupt_out);
    }

    // Draw sun (X=20, Y=5)
    black_xplsm_write_vram(VRAM_START + 5 * WIDTH + 20, 'O', interrupt_out);
    black_xplsm_write_vram(VRAM_START + 5 * WIDTH + 19, '(', interrupt_out);
    black_xplsm_write_vram(VRAM_START + 5 * WIDTH + 21, ')', interrupt_out);

    // Draw tiger silhouette (X=15, Y=9)
    black_xplsm_write_vram(VRAM_START + 9 * WIDTH + 15, '^', interrupt_out);
    black_xplsm_write_vram(VRAM_START + 9 * WIDTH + 16, '^', interrupt_out);
    black_xplsm_write_vram(VRAM_START + 9 * WIDTH + 17, 'T', interrupt_out);
}

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT XPLOS TIGER SUNSET SOFTWARE RASTERIZER SUITE\n");
    printf("=============================================================\n");

    // Test 1: RED rail static dimension validation
    printf("[TEST] Checking frame dimensions on RED rail...\n");
    assert(red_analyzer_validate_frame_dim(WIDTH, HEIGHT) == true);
    assert(red_analyzer_validate_frame_dim(100, 50) == false); // Too large
    printf("   ✓ RED rail successfully checked frame dimension parameters.\n");

    // Test 2: Execution and software rasterization output
    printf("[TEST] Running Tiger Sunset software rasterizer...\n");
    uint32_t trap_code = 0;
    run_sunset_rasterizer(&trap_code);
    assert(trap_code == 0);

    // Print rasterized terminal output
    printf("\n--- TIGER SUNSET SOFTWARE RENDER WINDOW ---\n");
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            printf("%c", virtual_rau_mem[VRAM_START + y * WIDTH + x]);
        }
        printf("\n");
    }
    printf("-------------------------------------------\n\n");
    printf("   ✓ Software rasterizer successfully drew shapes to target VRAM pages.\n");

    // Test 3: BLACK rail VRAM boundary traps
    printf("[TEST] Verifying VRAM boundary protection on BLACK rail...\n");
    trap_code = 0;
    // Attempt write outside video memory segment
    assert(black_xplsm_write_vram(0x0900, 'X', &trap_code) == false);
    assert(trap_code == 25); // Trap 25: Memory overflow
    printf("   ✓ BLACK rail successfully trapped VRAM out of bounds access (Trap 25).\n");

    printf("=============================================================\n");
    printf("ALL TIGER SUNSET GRAPHICS TESTS PASSED CLEANLY\n");
    printf("=============================================================\n");
    return 0;
}
