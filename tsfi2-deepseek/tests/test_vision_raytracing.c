#include "tsfi_font_vectors.h"
#include "tsfi_vision.h"
#include "lau_memory.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

// Simulating GFX12 ReBAR Upload (Linear CPU -> GPU write)
// In a real scenario, this writes to a mapped VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT buffer.
void simulate_gfx12_rebar_upload(uint8_t *screen, int sw, int sh, uint8_t *glyph, int gx, int gy, int gw, int gh) {
    // Simulating linear tiling upload to a specific region (Texture Update)
    for (int y = 0; y < gh; y++) {
        for (int x = 0; x < gw; x++) {
            uint8_t pixel = glyph[y * gw + x];
            if (pixel > 0) {
                int screen_x = gx + x;
                int screen_y = gy + y;
                if (screen_x < sw && screen_y < sh) {
                    // Simple overwrite (Source Copy) to ensure exact match for reference check
                    screen[screen_y * sw + screen_x] = pixel;
                }
            }
        }
    }
}

int main() {
    printf("[TEST] Vision Raytracing Integration (Wave512 & GFX12 Logic)...\n");

    // 1. Initialize Font System
    TSFiFontSystem *fs = tsfi_font_init();
    tsfi_font_generate_default(fs); // Uses new centered coordinates

    // 2. Rasterize 'A' (Wave512 Production)
    printf("[TEST] Producing Glyph 'A'...\n");
    uint32_t gid_A = fs->unicode_map['A'];
    uint8_t glyph_A[64 * 64];
    tsfi_font_rasterize_glyph_avx512(glyph_A, &fs->registry[gid_A], fs->segments);

    // 3. Rasterize 'B' (Reference for mismatch)
    printf("[TEST] Producing Glyph 'B' (Reference)...\n");
    // Note: 'B' isn't explicitly defined in generate_default, it falls to "Box".
    // 'A' is defined. So they should differ.
    // 'A' = /-\ , 'B' (default) = Box [].
    uint32_t gid_B = fs->unicode_map['B'];
    uint8_t glyph_B[64 * 64];
    tsfi_font_rasterize_glyph_avx512(glyph_B, &fs->registry[gid_B], fs->segments);

    // 4. Render 'A' to "Screen"
    int width = 256;
    int height = 256;
    uint8_t *screen_buffer = (uint8_t*)lau_malloc(width * height);
    memset(screen_buffer, 0, width * height); // Clear Black

    printf("[TEST] Uploading to Screen via ReBAR Simulation...\n");
    simulate_gfx12_rebar_upload(screen_buffer, width, height, glyph_A, 100, 100, 64, 64);

    // 5. Vision: Basic Visibility (Probe Heart)
    // Rendered at (100, 100). Size 64x64.
    // Center is (132, 132).
    // 'A' Crossbar is at relative Y=31 (approx screen Y=131).
    // Let's probe (132, 131).
    // For this specific test, let's manually probe the screen_buffer since we know it's A8.
    uint8_t probe_val = screen_buffer[131 * width + 132];
    printf("[TEST] Probe Heart (132, 131): %d\n", probe_val);
    if (probe_val == 0) {
        printf("[FAIL] Probe missed. Coordinate system or rasterizer still off.\n");
        // Debug dump small area
        printf("Debug Dump (130-134, 129-133):\n");
        for(int dy=129; dy<134; dy++) {
            for(int dx=130; dx<134; dx++) {
                printf("%02X ", screen_buffer[dy*width+dx]);
            }
            printf("\n");
        }
    }
    assert(probe_val > 0);

    // 6. Vision: Identification (OCR)
    printf("[TEST] Identification / Pattern Matching...\n");
    
    // Extract ROI from screen
    uint8_t roi[64 * 64];
    for(int y=0; y<64; y++) {
        for(int x=0; x<64; x++) {
            roi[y*64 + x] = screen_buffer[(100+y)*width + (100+x)];
        }
    }

    // Compare ROI vs Reference A
    bool is_A = tsfi_vision_compare_raw(roi, glyph_A, 64*64);
    printf("[TEST] Is ROI == Reference A? %s\n", is_A ? "YES" : "NO");
    assert(is_A == true);

    // Compare ROI vs Reference B
    bool is_B = tsfi_vision_compare_raw(roi, glyph_B, 64*64);
    printf("[TEST] Is ROI == Reference B? %s\n", is_B ? "YES" : "NO");
    assert(is_B == false);

    // 7. Vision: Symmetry Analysis
    printf("[TEST] Symmetry Analysis (Vision)...\n");
    // Define 'O' (Symmetric Box/Diamond)
    int idx_O = 0;
    TSFiPackedSegment segs_O[4];
    // Diamond shape: Top(0,12k), Right(6k,0), Bottom(0,-12k), Left(-6k,0)
    add_line(segs_O, &idx_O, 0, 12000, 6000, 0);
    add_line(segs_O, &idx_O, 6000, 0, 0, -12000);
    add_line(segs_O, &idx_O, 0, -12000, -6000, 0);
    add_line(segs_O, &idx_O, -6000, 0, 0, 12000);
    tsfi_font_set_glyph(fs, 'O', segs_O, 4, 12000, -6000, -12000, 6000, 12000);

    // Rasterize 'O'
    uint32_t gid_O = fs->unicode_map['O'];
    uint8_t glyph_O[64*64];
    tsfi_font_rasterize_glyph_avx512(glyph_O, &fs->registry[gid_O], fs->segments);

    // Analyze X-Symmetry (Left vs Right)
    int diff_O = 0;
    for(int y=0; y<64; y++) {
        for(int x=0; x<32; x++) {
            uint8_t left = glyph_O[y*64 + x];
            uint8_t right = glyph_O[y*64 + (63-x)];
            diff_O += abs(left - right);
        }
    }
    printf("[TEST] 'O' Symmetry Score (Diff): %d\n", diff_O);
    // Perfect symmetry might be slightly off due to rasterization secret or odd/even width
    // But it should be very low.
    assert(diff_O < 100);

    // Analyze 'L' (Asymmetric)
    // L: Left Line, Bottom Line.
    int idx_L = 0;
    TSFiPackedSegment segs_L[2];
    add_line(segs_L, &idx_L, -6000, 12000, -6000, -12000); // Vertical
    add_line(segs_L, &idx_L, -6000, -12000, 6000, -12000); // Horizontal
    tsfi_font_set_glyph(fs, 'L', segs_L, 2, 12000, -6000, -12000, 6000, 12000);
    
    uint32_t gid_L = fs->unicode_map['L'];
    uint8_t glyph_L[64*64];
    tsfi_font_rasterize_glyph_avx512(glyph_L, &fs->registry[gid_L], fs->segments);

    int diff_L = 0;
    for(int y=0; y<64; y++) {
        for(int x=0; x<32; x++) {
            uint8_t left = glyph_L[y*64 + x];
            uint8_t right = glyph_L[y*64 + (63-x)];
            diff_L += abs(left - right);
        }
    }
    printf("[TEST] 'L' Symmetry Score (Diff): %d\n", diff_L);
    assert(diff_L > 1000); // Must be high

    lau_free(screen_buffer);
    tsfi_font_destroy(fs);
    printf("[TEST] Passed.\n");
        extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}
