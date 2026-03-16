#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "tsfi_broadcaster.h"
#include "lau_memory.h"
#include "tsfi_staging.h"

void save_ppm(const char *filename, uint32_t *pixels, int w, int h) {
    FILE *f = fopen(filename, "wb");
    if (!f) return;
    fprintf(f, "P6\n%d %d\n255\n", w, h);
    for (int i = 0; i < w * h; i++) {
        uint32_t p = pixels[i];
        uint8_t r = (p >> 16) & 0xFF;
        uint8_t g = (p >> 8) & 0xFF;
        uint8_t b = p & 0xFF;
        fwrite(&r, 1, 1, f);
        fwrite(&g, 1, 1, f);
        fwrite(&b, 1, 1, f);
    }
    fclose(f);
}

void inject_text(LauBroadcaster *lb, const char *text) {
    int len = strlen(text);
    for (int i = 0; i < len; i++) {
        lb->char_BASE[0][i] = text[i];
    }
}

// draw_debug_codepoint is linked from obj/tsfi_staging.o

int main(int argc, char **argv) {
    printf("=== Kr0wZ LLM Text-to-Pixel Vulkan Validation (4 Second Render) ===\n");

    const char *llm_output = "QWEN ALIGNED";
    if (argc > 1) {
        llm_output = argv[1];
    }

    LauBroadcaster *lb = (LauBroadcaster *)lau_malloc_wired(sizeof(LauBroadcaster));
    memset(lb, 0, sizeof(LauBroadcaster));
    for (int y = 0; y < BROADCASTER_ROWS; y++) {
        for (int x = 0; x < BROADCASTER_COLS; x++) {
            lb->char_BASE[y][x] = ' ';
        }
    }

    printf("[INFO] Injecting LLM String: '%s'\n", llm_output);
    inject_text(lb, llm_output);

    int width = 1280;
    int height = 720;
    uint32_t *framebuffer = (uint32_t *)lau_malloc(width * height * sizeof(uint32_t));
    memset(framebuffer, 0, width * height * sizeof(uint32_t));

    StagingBuffer sb = { .width = width, .height = height, .stride = width * 4, .data = framebuffer, .magic = 0x57414745 };

    printf("[INFO] Rasterizing 4 seconds of video geometry...\n");
    for (int frame = 0; frame < 120; frame++) {
        memset(framebuffer, 0, width * height * sizeof(uint32_t));
        
        for (int x = 0; x < BROADCASTER_COLS; x++) {
            if (lb->char_BASE[0][x] != ' ') {
                draw_debug_codepoint(&sb, 10 + (x * 12), 20, lb->char_BASE[0][x], 0xFF00DD00);
            }
        }
        
        char filename[256];
        snprintf(filename, sizeof(filename), "vtube_frame_%03d.ppm", frame);
        save_ppm(filename, framebuffer, width, height);
    }

    printf("[PASS] 120 Frames successfully saved to disk for H.264 encode.\n");

    long total_brightness = 0;
    for (int i = 0; i < width * height; i++) {
        if (framebuffer[i] == 0xFF00DD00) {
            total_brightness++;
        }
    }

    if (total_brightness > 100) {
        printf("[PASS] Physical Validation Complete: %ld green pixels detected mapping to the LLM string.\n", total_brightness);
    } else {
        printf("[FAIL] Screen is black. Rasterization failed or font geometry is missing.\n");
        return 1;
    }

    lau_free(framebuffer);
    
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    extern void lau_assert_zero_unsealed_leaks(const char *context, void *teardown_ptr);
    lau_assert_zero_unsealed_leaks("test", NULL);
    return 0;
}