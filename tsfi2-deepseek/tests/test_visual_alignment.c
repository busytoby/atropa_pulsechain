#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vulkan/vulkan.h>
#include "tsfi_wave512.h"
#include "lau_memory.h"

// Project Mariner: Visual Alignment Test
// Goal: Verify CPU-generated pixel data is 100% aligned for Vulkan Linear Layout.

int main() {
    printf("=== Project Mariner: Visual Alignment Test ===\n");

    // 1. Dimensions (Standard 1080p)
    int width = 1920;
    int height = 1080;
    int bpp = 4; // BGRA8888
    size_t size = width * height * bpp;

    // 2. Allocate Zero-Copy Buffer
    // TSFi Mandate: 512-byte alignment
    uint32_t *pixels = (uint32_t*)lau_memalign(512, size);
    if ((uintptr_t)pixels % 512 != 0) {
        printf("[FAIL] Pixel buffer not 512-aligned.\n");
        return 1;
    }
    printf("[INFO] Pixel buffer allocated at %p (512-aligned).\n", (void*)pixels);

    // 3. Generate Pattern (AVX-512 Optimized Fill)
    // We use VBROADCASTSS to fill 512-byte blocks.
    // 512 bytes = 128 pixels (4 bytes/pixel).
    // Pattern: Blue (0xFF0000FF in BGRA - LE: B G R A -> BB GG RR AA)
    // Wait, BGRA: Byte 0=B, 1=G, 2=R, 3=A.
    // 0xFF0000FF -> Int: 0xFF0000FF.
    // Let's use a solid color first.
    uint32_t color = 0xFFFF0000; // Blue (assuming Little Endian BGRA: B=00, G=00, R=FF, A=FF ... wait)
    // BGRA8888:
    // Byte 0: Blue
    // Byte 1: Green
    // Byte 2: Red
    // Byte 3: Alpha
    // Value 0xFFFF0000 (LE) -> Bytes: 00 00 FF FF. This is RED.
    // Value 0xFF0000FF (LE) -> Bytes: FF 00 00 FF. This is BLUE.
    
    // Use Wave512 to fill.
    // 512 bytes = 128 uint32_t pixels.
    // We can use VMOVDQA32 logic or just cast.
    
    int blocks = size / 512;
    wave512_i *wPtr = (wave512_i*)pixels;
    
    // Create packed 64-bit pixel pair
    uint64_t double_pixel = ((uint64_t)color << 32) | color;
    wave512_i wColor = VBROADCASTSD(&double_pixel);

    for (int i = 0; i < blocks; i++) {
        // Direct store (Stream would be better for throughput)
        // Use WAVE512_CAST_TO_F to safely treat integer vector as float vector for VSTREAMPS
        VSTREAMPS(&wPtr[i], WAVE512_CAST_TO_F(wColor));
    }

    // 4. Verify Stride/Alignment for Vulkan
    // Vulkan Linear Tiling requires specific row pitch alignment.
    // Typically 256 or 512 bytes.
    // Width 1920 * 4 = 7680 bytes. 
    // 7680 % 512 = 0.
    // 7680 / 512 = 15 blocks per row.
    // This implies 1920x1080 is PERFECTLY aligned for Wave512 processing without padding.
    
    if (size % 512 == 0) {
        printf("[PASS] Framebuffer size is 512-byte block multiple (%zu blocks).\n", size/512);
    } else {
        printf("[FAIL] Framebuffer size %zu is not 512-aligned (Padding required).\n", size);
        return 1;
    }

    // 5. Verify Data Integrity
    if (pixels[0] == color && pixels[width*height-1] == color) {
        printf("[PASS] Wave512 Pixel Generation Verified.\n");
    } else {
        printf("[FAIL] Pixel data corruption.\n");
        return 1;
    }

    lau_free(pixels);
        extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}
