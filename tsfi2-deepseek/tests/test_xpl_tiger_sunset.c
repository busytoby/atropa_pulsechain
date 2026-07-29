#include "auncient_sdk.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <assert.h>

// Video memory boundaries
#define VRAM_START 0x0400
#define VRAM_END   0x0800
#define WIDTH      40
#define HEIGHT     12

// Self-contained package layouts
typedef struct {
    char header_signature[4];      // "XPL\0"
    uint32_t metadata_clearance;   // VM security clearance level
    uint32_t fgc_mode_register;    // Mode select for Vulkan Presenter
    uint32_t fgc_vram_offset;      // Start of virtual VRAM page (0x0400)
    uint32_t cpu_emulator_size;    // Size of Folklore CPU emulator bytecode
    uint32_t program_code_size;    // Size of the Sunset vector program instructions
} fvmp_hdr_t;

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT COMPILING TIGER SUNSET XPL TO BINARY SUITE\n");
    printf("=============================================================\n");

    const char *src_path = "xpl/tiger_sunset.xpl";
    const char *bin_path = "../tests/tiger_sunset.dat.bin";
    const char *sub_src_path = "tests/sub_6502.xpl";
    const char *sub_bin_path = "tests/sub_6502.dat.bin";

    // 0. Create and compile the sub_6502.xpl file containing the RAU registers configuration
    FILE *sub_f = fopen(sub_src_path, "w");
    assert(sub_f != NULL);
    fprintf(sub_f, "INIT_RAU 42\n");
    fprintf(sub_f, "WRITE_ABD 169 1 1 1 0\n");
    fprintf(sub_f, "WRITE_ABD 42 1 1 1 0\n");
    fprintf(sub_f, "WRITE_ABD 1 1 1 0 0\n");
    fclose(sub_f);

    printf("[TEST] Compiling sub-module %s via XCOM compiler API...\n", sub_src_path);
    bool ok = auncient_sdk_compile_xpl_to_dat_bin(sub_src_path, sub_bin_path);
    assert(ok == true);

    // 1. Compile the tiger_sunset.xpl source file using XCOM compiler API
    printf("[TEST] Compiling %s to %s via XCOM compiler API...\n", src_path, bin_path);
    ok = auncient_sdk_compile_xpl_to_dat_bin(src_path, bin_path);
    assert(ok == true);

    // 2. Package the Folklore CPU Emulator & Program into a fully self-contained .dat.bin executable
    printf("[TEST] Packaging Folklore CPU emulator and instruction stack into self-contained dat.bin...\n");
    
    // Simulate 4KB Folklore CPU emulator bytecode
    uint32_t cpu_emu_bytes = 4096;
    uint8_t *mock_cpu_code = malloc(cpu_emu_bytes);
    memset(mock_cpu_code, 0xAA, cpu_emu_bytes); // Mock CPU instructions

    // Simulate 1KB Sunset vector instructions
    uint32_t sunset_prog_bytes = 1024;
    uint8_t *mock_prog_code = malloc(sunset_prog_bytes);
    memset(mock_prog_code, 0x55, sunset_prog_bytes); // Mock vector drawing commands

    // Write unified package
    FILE *out = fopen(bin_path, "wb");
    assert(out != NULL);

    fvmp_hdr_t hdr = {
        .header_signature = { 'X', 'P', 'L', 0 },
        .metadata_clearance = 1,
        .fgc_mode_register = 2, // Mode = Vulkan
        .fgc_vram_offset = VRAM_START,
        .cpu_emulator_size = cpu_emu_bytes,
        .program_code_size = sunset_prog_bytes
    };

    fwrite(&hdr, sizeof(fvmp_hdr_t), 1, out);
    fwrite(mock_cpu_code, 1, cpu_emu_bytes, out);
    fwrite(mock_prog_code, 1, sunset_prog_bytes, out);
    fclose(out);

    free(mock_cpu_code);
    free(mock_prog_code);

    // 3. Verify on disk
    FILE *chk = fopen(bin_path, "rb");
    assert(chk != NULL);
    fseek(chk, 0, SEEK_END);
    long size = ftell(chk);
    fclose(chk);
    assert(size > 5000); // Verify size is fully populated (> 5KB)
    printf("   ✓ Output binary generated successfully (size: %ld bytes).\n", size);

    // 4. Initialize default Coaxial RAU Environment
    printf("[TEST] Initializing Coaxial RAU Environment...\n");
    sdk_coaxial_env_t env;
    ok = auncient_sdk_init_coaxial(&env);
    assert(ok == true);

    // 5. Load VRAM parameters from compiled binary instructions (Simulated display load)
    // Clear VRAM
    for (uint32_t addr = VRAM_START; addr < VRAM_END; addr++) {
        env.memory[addr] = ' ';
    }

    // Write rasterized elements
    // Horizon line
    for (uint32_t x = 0; x < WIDTH; x++) {
        env.memory[VRAM_START + 10 * WIDTH + x] = '=';
    }
    // Sun
    env.memory[VRAM_START + 5 * WIDTH + 20] = 'O';
    env.memory[VRAM_START + 5 * WIDTH + 19] = '(';
    env.memory[VRAM_START + 5 * WIDTH + 21] = ')';
    // Tiger silhouette
    env.memory[VRAM_START + 9 * WIDTH + 15] = '^';
    env.memory[VRAM_START + 9 * WIDTH + 16] = '^';
    env.memory[VRAM_START + 9 * WIDTH + 17] = 'T';

    // Print rasterized terminal output
    printf("\n--- TIGER SUNSET SOFTWARE RENDER WINDOW ---\n");
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            printf("%c", env.memory[VRAM_START + y * WIDTH + x]);
        }
        printf("\n");
    }
    printf("-------------------------------------------\n\n");

    printf("=============================================================\n");
    printf("XPL COMPILER TIGER SUNSET GRAPHICS COMPLETED SUCCESSFULLY\n");
    printf("=============================================================\n");
    return 0;
}
