#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include <time.h>
#include <jpeglib.h>
#include <setjmp.h>
#include "tsfi_zmm_vm.h"
#include "tsfi_wire_firmware.h"
#include "lau_yul_thunk.h"

// Terminology Constraint: Never use the spelling "ancient". Always spell it as "Auncient".

#define VIEWPORT_WIDTH 320
#define VIEWPORT_HEIGHT 300

typedef struct {
    bool is_initialized;
    uint16_t viewport_w;
    uint16_t viewport_h;
    uint32_t *framebuffer; // Shared ZMM memory frame buffer
    uint8_t *jpeg_data;
    size_t jpeg_size;
} NodeEmbedContext;

struct my_error_mgr {
    struct jpeg_error_mgr pub;
    jmp_buf setjmp_buffer;
};

static void my_error_exit(j_common_ptr cinfo) {
    struct my_error_mgr *myerr = (struct my_error_mgr *)cinfo->err;
    (*cinfo->err->output_message)(cinfo);
    longjmp(myerr->setjmp_buffer, 1);
}

// Decodes a YouTube video frame directly to ZMM framebuffer
bool decode_jpeg_to_fb(const uint8_t *jpeg_buf, size_t jpeg_sz, uint32_t *scanout_px, int width, int height) {
    struct jpeg_decompress_struct cinfo;
    struct my_error_mgr jerr;
    cinfo.err = jpeg_std_error(&jerr.pub);
    jerr.pub.error_exit = my_error_exit;
    if (setjmp(jerr.setjmp_buffer)) {
        jpeg_destroy_decompress(&cinfo);
        return false;
    }
    jpeg_create_decompress(&cinfo);
    jpeg_mem_src(&cinfo, jpeg_buf, jpeg_sz);
    if (jpeg_read_header(&cinfo, TRUE) != JPEG_HEADER_OK) {
        jpeg_destroy_decompress(&cinfo);
        return false;
    }
    cinfo.out_color_space = JCS_RGB;
    if (!jpeg_start_decompress(&cinfo)) {
        jpeg_destroy_decompress(&cinfo);
        return false;
    }
    int row_stride = cinfo.output_width * cinfo.output_components;
    JSAMPARRAY buffer = (*cinfo.mem->alloc_sarray)((j_common_ptr)&cinfo, JPOOL_IMAGE, row_stride, 1);
    while (cinfo.output_scanline < cinfo.output_height) {
        int y = cinfo.output_scanline;
        jpeg_read_scanlines(&cinfo, buffer, 1);
        if (y >= height) continue;
        uint8_t *src = buffer[0];
        uint32_t *dst = scanout_px + y * width;
        for (int x = 0; x < width && x < (int)cinfo.output_width; x++) {
            uint8_t r = src[x * 3];
            uint8_t g = src[x * 3 + 1];
            uint8_t b = src[x * 3 + 2];
            dst[x] = (0xFF000000) | (r << 16) | (g << 8) | b;
        }
    }
    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);
    return true;
}

// Interop C-Embedding interface for Node.js
void node_embed_init(NodeEmbedContext *ctx, uint16_t w, uint16_t h) {
    ctx->viewport_w = w;
    ctx->viewport_h = h;
    ctx->framebuffer = calloc(w * h, sizeof(uint32_t));
    
    // Load YouTube video frame screenshot from the frontend assets
    FILE *f = fopen("../frontend/latest_frame.jpg", "rb");
    if (!f) {
        // Fallback to local build path if run from a different directory
        f = fopen("frontend/latest_frame.jpg", "rb");
    }
    
    if (f) {
        fseek(f, 0, SEEK_END);
        ctx->jpeg_size = ftell(f);
        fseek(f, 0, SEEK_SET);
        ctx->jpeg_data = malloc(ctx->jpeg_size);
        size_t read_bytes = fread(ctx->jpeg_data, 1, ctx->jpeg_size, f);
        (void)read_bytes;
        fclose(f);
        printf("[NODE EMBED] Successfully loaded YouTube frame (%zu bytes).\n", ctx->jpeg_size);
    } else {
        printf("[NODE EMBED] WARNING: could not load latest_frame.jpg. Benchmarking with synthetic background.\n");
        ctx->jpeg_data = NULL;
        ctx->jpeg_size = 0;
    }
    ctx->is_initialized = true;
}

void node_embed_shutdown(NodeEmbedContext *ctx) {
    if (ctx->framebuffer) {
        free(ctx->framebuffer);
        ctx->framebuffer = NULL;
    }
    if (ctx->jpeg_data) {
        free(ctx->jpeg_data);
        ctx->jpeg_data = NULL;
    }
    ctx->is_initialized = false;
}

// Convert numbers to 32-byte big-endian hex words for Yul calldata
void fill_hex_uint256(char *dest, uint64_t val) {
    memset(dest, '0', 64);
    char tmp[32];
    int len = snprintf(tmp, sizeof(tmp), "%lx", val);
    memcpy(dest + 64 - len, tmp, len);
    dest[64] = '\0';
}

// Process logs and render graphics directly to the ZMM memory framebuffer
void process_logs(NodeEmbedContext *ctx) {
    int ui_logs = lau_yul_thunk_get_log_count();
    for (int k = 0; k < ui_logs; k++) {
        uint64_t log_addr = 0;
        int num_topics = 0;
        u256_t topics[4] = {0};
        uint8_t log_data[2048] = {0};
        size_t log_data_size = sizeof(log_data);
        if (lau_yul_thunk_get_log(k, &log_addr, &num_topics, topics, log_data, &log_data_size)) {
            // Check topic hash matching the drawRect signature (0x9e7a02c89e7a02c8...)
            if (num_topics > 0 && topics[0].d[3] == 0x9e7a02c89e7a02c8ULL && topics[0].d[2] == 0x9e7a02c89e7a02c8ULL) {
                uint16_t rx = (log_data[30] << 8) | log_data[31];
                uint16_t ry = (log_data[62] << 8) | log_data[63];
                uint16_t rw = (log_data[94] << 8) | log_data[95];
                uint16_t rh = (log_data[126] << 8) | log_data[127];
                uint32_t rcol = (log_data[156] << 24) | (log_data[157] << 16) | (log_data[158] << 8) | log_data[159];
                if ((rcol & 0xFF000000) == 0) rcol |= 0xFF000000;

                // Render directly to the ZMM memory framebuffer
                for (int y = ry; y < ry + rh && y < ctx->viewport_h; y++) {
                    for (int x = rx; x < rx + rw && x < ctx->viewport_w; x++) {
                        ctx->framebuffer[y * ctx->viewport_w + x] = rcol;
                    }
                }
            }
        }
    }
}

// Tick execution: updates input state, triggers buttons on Yul VM, parses logs and renders to memory
void node_embed_render_tick(NodeEmbedContext *ctx, TsfiZmmVmState *vm, uint16_t mx, uint16_t my, uint8_t click_state) {
    // 1. Format input state update calldata
    // Selector: 0x8b5c90d2 -> updateInputState(uint16 x, uint16 y, uint8 buttonState)
    char cmd[1024];
    char hex_mx[65], hex_my[65], hex_click[65];
    fill_hex_uint256(hex_mx, mx);
    fill_hex_uint256(hex_my, my);
    fill_hex_uint256(hex_click, click_state);

    snprintf(cmd, sizeof(cmd), "YULEXEC \"MicroUI\", \"8b5c90d2%s%s%s\"", hex_mx, hex_my, hex_click);
    tsfi_zmm_vm_exec(vm, cmd);

    // 2. Decode the YouTube video frame screenshot as the display background
    if (ctx->jpeg_data && ctx->jpeg_size > 0) {
        decode_jpeg_to_fb(ctx->jpeg_data, ctx->jpeg_size, ctx->framebuffer, ctx->viewport_w, ctx->viewport_h);
    } else {
        memset(ctx->framebuffer, 0x11, ctx->viewport_w * ctx->viewport_h * sizeof(uint32_t));
    }

    // 3. Format button triggers (Clear: 10, 260, 80, 25; Invert: 100, 260, 80, 25)
    // Selector: 0xb5ba0c68 -> button(uint16 x, uint16 y, uint16 w, uint16 h)
    char hex_x[65], hex_y[65], hex_w[65], hex_h[65];
    
    // Clear Button
    fill_hex_uint256(hex_x, 10);
    fill_hex_uint256(hex_y, 260);
    fill_hex_uint256(hex_w, 80);
    fill_hex_uint256(hex_h, 25);
    snprintf(cmd, sizeof(cmd), "YULEXEC \"MicroUI\", \"b5ba0c68%s%s%s%s\"", hex_x, hex_y, hex_w, hex_h);
    tsfi_zmm_vm_exec(vm, cmd);
    process_logs(ctx);

    // Invert Button
    fill_hex_uint256(hex_x, 100);
    fill_hex_uint256(hex_y, 260);
    fill_hex_uint256(hex_w, 80);
    fill_hex_uint256(hex_h, 25);
    snprintf(cmd, sizeof(cmd), "YULEXEC \"MicroUI\", \"b5ba0c68%s%s%s%s\"", hex_x, hex_y, hex_w, hex_h);
    tsfi_zmm_vm_exec(vm, cmd);
    process_logs(ctx);
}

// -----------------------------------------------------------------------------
// UNIT TESTS & BENCHMARKS
// -----------------------------------------------------------------------------

void run_unit_tests(NodeEmbedContext *ctx, TsfiZmmVmState *vm) {
    printf("[UNIT TEST] Running state validation tests...\n");

    // Test 1: Button color under Idle State (no mouse hover)
    // Clear button at (10, 260, 80, 25)
    // Send mouse far away (0, 0)
    printf("  -> Testing Button Idle Color...\n");
    node_embed_render_tick(ctx, vm, 0, 0, 0);

    // Read pixel inside Clear Button bounds (say X=20, Y=270)
    uint32_t pixel_idle = ctx->framebuffer[270 * ctx->viewport_w + 20];
    // Expected default grey 0x444444 (represented as 0xFF444444 with alpha mask)
    printf("     Idle Pixel Color: 0x%08X\n", pixel_idle);
    assert(pixel_idle == 0xFF444444);
    printf("     PASS: Idle state color is correct.\n");

    // Test 2: Button color under Hover State (mouse over button, not clicked)
    // Clear button at (10, 260, 80, 25) -> hover at X=20, Y=270
    printf("  -> Testing Button Hover Color...\n");
    node_embed_render_tick(ctx, vm, 20, 270, 0);
    uint32_t pixel_hover = ctx->framebuffer[270 * ctx->viewport_w + 20];
    // Expected hover grey 0x666666 (represented as 0xFF666666 with alpha mask)
    printf("     Hover Pixel Color: 0x%08X\n", pixel_hover);
    assert(pixel_hover == 0xFF666666);
    printf("     PASS: Hover state color is correct.\n");

    // Test 3: Button color under Clicked State (mouse over button and click state active)
    printf("  -> Testing Button Clicked Color...\n");
    node_embed_render_tick(ctx, vm, 20, 270, 1);
    uint32_t pixel_clicked = ctx->framebuffer[270 * ctx->viewport_w + 20];
    // Expected active purple 0xa855f7 (represented as 0xFFa855f7 with alpha mask)
    printf("     Clicked Pixel Color: 0x%08X\n", pixel_clicked);
    assert(pixel_clicked == 0xFFa855f7);
    printf("     PASS: Clicked state color is correct.\n");

    printf("[SUCCESS] All state validation unit tests passed!\n\n");
}

void run_performance_benchmarks(NodeEmbedContext *ctx, TsfiZmmVmState *vm) {
    const int iterations = 100;
    printf("[BENCHMARK] Rendering %d YouTube frames (JPEG decoding + MicroUI logs) directly to ZMM memory...\n", iterations);

    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    for (int i = 0; i < iterations; i++) {
        // Alternate mouse coordinates to trigger redrawing and state switching
        uint16_t mx = (i % 2) ? 20 : 0;
        uint16_t my = (i % 2) ? 270 : 0;
        uint8_t btn = (i % 4) == 0 ? 1 : 0;

        node_embed_render_tick(ctx, vm, mx, my, btn);
    }

    clock_gettime(CLOCK_MONOTONIC, &end);

    double elapsed = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
    double fps = iterations / elapsed;
    double frame_size_mb = (VIEWPORT_WIDTH * VIEWPORT_HEIGHT * sizeof(uint32_t)) / (1024.0 * 1024.0);
    double throughput = fps * frame_size_mb;

    printf("[SUCCESS] Auncient ZMM VM rendering performance with YouTube video frames:\n");
    printf("  -> Total Frames: %d\n", iterations);
    printf("  -> Time Elapsed: %.4f seconds\n", elapsed);
    printf("  -> Performance:  %.2f FPS\n", fps);
    printf("  -> Throughput:   %.2f MB/s\n\n", throughput);
}

int main() {
    printf("=========================================================\n");
    printf("Auncient ZMM VM Embedded Node.js Browser Controller Unit Tests\n");
    printf("=========================================================\n");

    // Initialize simulation dependencies
    tsfi_wire_firmware_init();

    TsfiZmmVmState vm;
    tsfi_zmm_vm_init(&vm);

    // Initialize Yul MicroUI in VM
    printf("[VM INIT] Initializing MicroUI Yul Engine on ZMM VM...\n");
    tsfi_zmm_vm_exec(&vm, "YULINIT \"MicroUI\", \"../solidity/bin/microui.yul\", 256");

    NodeEmbedContext ctx;
    node_embed_init(&ctx, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);

    // Execute tests
    run_unit_tests(&ctx, &vm);
    run_performance_benchmarks(&ctx, &vm);

    // Cleanup
    node_embed_shutdown(&ctx);
    tsfi_zmm_vm_destroy(&vm);

    printf("=========================================================\n");
    printf("ZMM VM MICROUI BENCHMARKS & UNIT TESTS COMPLETED SUCCESS\n");
    printf("=========================================================\n");

    return 0;
}
