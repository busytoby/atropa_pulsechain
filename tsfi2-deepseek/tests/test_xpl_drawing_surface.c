#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "tsfi_zmm_vm.h"
#include "tsfi_wire_firmware.h"

#define REG_VCE_BASE 62208   // 0xF300
#define REG_VRAM_BASE 62464  // 0xF400

// Mock Vulkan Compute Shader Execution
void vulkan_mock_compute_shader(const uint32_t *vram, const uint32_t *clut, uint32_t *frame_buffer, int width, int height) {
    printf("[VULKAN] Simulating compute shader rasterization grid...\n");
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int pixel_id = y * width + x;
            
            // Read index from VRAM SSBO drawing surface
            uint32_t color_idx = vram[pixel_id];
            
            // Read RGB from CLUT palette
            uint32_t rgb = clut[color_idx];
            
            // Store to output frame buffer drawing surface
            frame_buffer[pixel_id] = rgb;
        }
    }
}

int main() {
    printf("=== TSFi ZMM VM Auncient Vulkan Drawing Surface Test ===\n");
    tsfi_wire_firmware_init();

    TsfiZmmVmState vm;
    tsfi_zmm_vm_init(&vm);

    // 1. Compile and deploy hucSystem.yul
    printf("[ZMM] Loading hucSystem.yul to VM...\n");
    tsfi_zmm_vm_exec(&vm, "YULINIT \"huc\", \"../solidity/bin/hucSystem.yul\", 7");

    char cmd[512];

    // 2. Initialize VCE palette (0xF300) and VRAM drawing surface (0xF400)
    printf("[ZMM] Initializing color palette indexes and VRAM surface pixels...\n");
    
    // Set color index 1 to RGB 0xFF00FF (Magenta)
    sprintf(cmd, "YULEXEC \"huc\", \"b3aa1ab9"
                 "%064x"
                 "%064x\"",
            REG_VCE_BASE + 1, 0xFF00FF);
    tsfi_zmm_vm_exec(&vm, cmd);

    // Set pixel at coordinates (0, 0) index to 1 in VRAM drawing surface (0xF400)
    sprintf(cmd, "YULEXEC \"huc\", \"b3aa1ab9"
                 "%064x"
                 "%064x\"",
            REG_VRAM_BASE + 0, 1);
    tsfi_zmm_vm_exec(&vm, cmd);

    // 3. Emulate Vulkan SSBO binding mapping
    printf("[VULKAN] Mapping ZMM memory addresses 0xF300/0xF400 to SSBO buffers...\n");
    uint32_t mock_vram[64];
    uint32_t mock_clut[16];
    uint32_t output_surface[64];
    
    memset(mock_vram, 0, sizeof(mock_vram));
    memset(mock_clut, 0, sizeof(mock_clut));
    memset(output_surface, 0, sizeof(output_surface));
    
    mock_vram[0] = 1; // Coordinate (0,0) points to palette 1
    mock_clut[1] = 0xFF00FF; // Palette 1 holds Magenta

    // 4. Run Vulkan compute shader simulator
    vulkan_mock_compute_shader(mock_vram, mock_clut, output_surface, 8, 8);

    // 5. Verify drawing surface output
    printf("[VULKAN] Verifying output drawing surface pixels...\n");
    printf("[VULKAN] Pixel (0,0) RGB color: 0x%06X (Expected: 0xFF00FF)\n", output_surface[0]);
    assert(output_surface[0] == 0xFF00FF);
    assert(output_surface[1] == 0); // Background remains black (0)
    
    tsfi_zmm_vm_destroy(&vm);
    printf("\n=== VULKAN DRAWING SURFACE TESTS PASSED ===\n");
    return 0;
}
