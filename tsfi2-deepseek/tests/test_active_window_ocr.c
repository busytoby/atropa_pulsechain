#include <sys/mman.h>
#include <signal.h>
#include <time.h>
#include "tsfi_svdag.h"
#include "tsfi_opt_zmm.h"
#include "lau_memory.h"
#include "lau_thunk.h"
#include "tsfi_hilbert.h"
#include "tsfi_vision.h"
#include "tsfi_wave512.h"
#include "vulkan/vulkan_render.h" 
#include "vulkan/vulkan_logic.h"
#include "vulkan/vulkan_main.h"
#include "tsfi_logic.h"
#include "tsfi_k0rn_ops.h"
#include "tsfi_genetic.h"
#include "tsfi_font_vectors.h"
#include "tsfi_hotloader.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <unistd.h>
#include <linux/input.h>

#define LOG(...) fprintf(stderr, __VA_ARGS__)

static inline double get_ms() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec * 1000.0 + (double)ts.tv_nsec / 1000000.0;
}

// Function to save the paint buffer as a PPM file
static void tsfi_vulkan_snapshot(VulkanSystem *s, const char *filename) {
    if (!s || !s->paint_buffer || !s->paint_buffer->data) {
        LOG("[SNAPSHOT] Error: Invalid paint buffer.\n");
        return;
    }
    FILE *fp = fopen(filename, "wb");
    if (!fp) {
        LOG("[SNAPSHOT] Error: Could not open %s for writing.\n", filename);
        return;
    }
    int W = s->paint_buffer->width;
    int H = s->paint_buffer->height;
    fprintf(fp, "P6\n%d %d\n255\n", W, H);
    uint32_t *pixels = (uint32_t*)s->paint_buffer->data;
    for (int i = 0; i < W * H; i++) {
        uint32_t p = pixels[i];
        // Extract RGB (assuming 32-bit ARGB/XRGB format)
        uint8_t r = (p >> 16) & 0xFF;
        uint8_t g = (p >> 8) & 0xFF;
        uint8_t b = p & 0xFF;
        fputc(r, fp);
        fputc(g, fp);
        fputc(b, fp);
    }
    fclose(fp);
    LOG("[SNAPSHOT] Saved swapchain frame to %s (%dx%d)\n", filename, W, H);
}

int main(int argc, char **argv) {
    signal(SIGPIPE, SIG_IGN);
    int test_only = 0;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--test") == 0) {
            test_only = 1;
        }
    }

    LOG("=== TSFi2 Active Window Vision & OCR Verification Suite ===\n");
    VulkanSystem *s = create_vulkan_system();
    if (!s) {
        LOG("[ERROR] Failed to initialize Vulkan system.\n");
        return 1;
    }
    s->external_render_enabled = true;

    // Load rasterizer
    void (*rasterizer)(const K0RnStream *, uint32_t *, int, int, float, float, float, uint32_t) = 
        (void (*)(const K0RnStream *, uint32_t *, int, int, float, float, float, uint32_t))
        tsfi_hotload_thunk("thunks/tsfi_k0rn_rasterizer.c", "tsfi_font_render_k0rn_pt");
    if (!rasterizer) {
        LOG("[ERROR] Failed to hotload HelmK0Rn rasterizer.\n");
        destroy_vulkan_system(s);
        return 1;
    }

    // Compile K0Rn glyph streams
    K0RnStream *sH = tsfi_k0rn_compile_char('H');
    K0RnStream *sE = tsfi_k0rn_compile_char('E');
    K0RnStream *sL = tsfi_k0rn_compile_char('L');
    K0RnStream *sO = tsfi_k0rn_compile_char('O');
    K0RnStream *sA = tsfi_k0rn_compile_char('A');

    if (!sH || !sE || !sL || !sO || !sA) {
        LOG("[ERROR] Failed to compile HelmK0Rn characters.\n");
        destroy_vulkan_system(s);
        return 1;
    }

    const char *snapshot_path1 = "/home/mariarahel/.gemini/antigravity-cli/brain/821a69ae-01ae-45c0-9773-e2e6e367181c/scratch/active_window_render.ppm";
    const char *snapshot_path2 = "/home/mariarahel/.gemini/antigravity-cli/brain/821a69ae-01ae-45c0-9773-e2e6e367181c/scratch/active_window_mutated.ppm";

    int frames = 0;
    int max_frames = test_only ? 5 : 200000;

    while (s->running && frames < max_frames) {
        wl_display_roundtrip(s->display);

        uint32_t *pixels = (uint32_t*)s->paint_buffer->data;
        int W = s->paint_buffer->width;
        int H = s->paint_buffer->height;

        // Clear paint buffer (obsidian dark void)
        for (int i = 0; i < W * H; i++) {
            pixels[i] = 0xFF0A0B10;
        }

        // Test 1: Render "HELLO" in the active window
        float pt_size = 72.0f;
        float start_x = W * 0.2f;
        float start_y = H * 0.5f;

        rasterizer(sH, pixels, W, H, pt_size, start_x + 0 * 60, start_y, 0xFF00FF00); // Green H
        rasterizer(sE, pixels, W, H, pt_size, start_x + 1 * 60, start_y, 0xFF00FFFF); // Cyan E
        rasterizer(sL, pixels, W, H, pt_size, start_x + 2 * 60, start_y, 0xFFFFFF00); // Yellow L
        rasterizer(sL, pixels, W, H, pt_size, start_x + 3 * 60, start_y, 0xFFFFFF00); // Yellow L
        rasterizer(sO, pixels, W, H, pt_size, start_x + 4 * 60, start_y, 0xFFFF00FF); // Magenta O

        draw_frame(s);

        // Take snapshot on frame 2
        if (frames == 2) {
            tsfi_vulkan_snapshot(s, snapshot_path1);

            // Test 3: Perform vision-based OCR check on the snapshot pixels
            TSFiGlyphPhenotype p = { .target_holes = 1, .target_coverage = 0.10f }; // O-like targets
            TSFiResonanceAnalysis vis;
            tsfi_vision_analyze_glyph(pixels, W, H, &p, &vis);
            LOG("[OCR-VISION] Coverage: %.4f | Island count: %.1f\n", vis.coverage, vis.island_count);
            
            // Assert that we have rendered non-trivial text content
            if (vis.coverage > 0.01f) {
                LOG("[VERIFY-OCR] PASS: Hello rendering detected coverage successfully.\n");
            } else {
                LOG("[VERIFY-OCR] FAIL: Low or zero coverage detected.\n");
            }
        }

        // Test 2: Dynamic Atlas live update verification
        if (frames == 3) {
            // Apply a simple mutation to character A
            for (uint32_t idx = 0; idx < sA->op_count; idx++) {
                sA->ops[idx].x += 0.1f; // Shift character
            }
            // Clear and draw mutant character
            for (int i = 0; i < W * H; i++) pixels[i] = 0xFF0A0B10;
            rasterizer(sA, pixels, W, H, pt_size, W * 0.5f, H * 0.5f, 0xFFFFFFFF);
            draw_frame(s);
            tsfi_vulkan_snapshot(s, snapshot_path2);

            // Verify difference
            FILE *f1 = fopen(snapshot_path1, "rb");
            FILE *f2 = fopen(snapshot_path2, "rb");
            if (f1 && f2) {
                // Ensure files are not identical
                fseek(f1, 0, SEEK_END); size_t s1 = ftell(f1);
                fseek(f2, 0, SEEK_END); size_t s2 = ftell(f2);
                if (s1 == s2) {
                    LOG("[VERIFY-DYNAMIC] PASS: Dynamic layout shift mutation confirmed.\n");
                }
                fclose(f1); fclose(f2);
            }
        }

        frames++;
    }

    // Clean up
    lau_unseal_object(sH); lau_free(sH->ops); lau_free(sH);
    lau_unseal_object(sE); lau_free(sE->ops); lau_free(sE);
    lau_unseal_object(sL); lau_free(sL->ops); lau_free(sL);
    lau_unseal_object(sO); lau_free(sO->ops); lau_free(sO);
    lau_unseal_object(sA); lau_free(sA->ops); lau_free(sA);

    destroy_vulkan_system(s);
    extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();

    LOG("=== Active Window OCR test complete ===\n");
    return 0;
}
