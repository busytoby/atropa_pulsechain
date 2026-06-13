#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include "tsfi_wire_firmware.h"
#include "lau_memory.h"
#include "lau_vram.h"

// Custom C-implementation of the cv::Mat container in AB4H space
// Formatted exactly as CV_16FC4 (16-bit half-precision floats, 4 channels: R, G, B, A)
// No swizzling is performed (retains raw AB4H channel order natively)
typedef struct {
    int rows;          // Height (number of rows)
    int cols;          // Width (number of columns)
    int type;          // CV_16FC4 representation value
    int stride;        // Row stride in bytes
    uint16_t *data;    // Zero-copy pointer to raw AB4H pixel buffer data
} TsfiMat;

#define TSFI_CV_16FC4 0xFEE1D

// Struct to represent an AB4H pixel (4 half-precision floats)
typedef struct {
    uint16_t r, g, b, a;
} Ab4hPixel;

// Half-precision float helpers (Zero swizzling, maps float to 16-bit half precision)
static inline uint16_t double_to_half(double f) {
    union { float f; uint32_t u; } u = { (float)f };
    uint32_t sign = (u.u >> 16) & 0x8000;
    int32_t exponent = ((u.u >> 23) & 0xff) - 127;
    uint32_t mantissa = u.u & 0x7fffff;
    if (exponent <= -15) return sign;
    if (exponent >= 16) return sign | 0x7c00;
    return sign | ((exponent + 15) << 10) | (mantissa >> 13);
}

// Zero-copy Mat instantiation wrapping an external pixel buffer
static TsfiMat tsfi_mat_create_wrapper(int rows, int cols, int stride_bytes, void *external_data) {
    TsfiMat m;
    m.rows = rows;
    m.cols = cols;
    m.type = TSFI_CV_16FC4;
    m.stride = stride_bytes;
    m.data = (uint16_t *)external_data;
    return m;
}

// Direct Mat region of interest (ROI) sub-matrix view
static TsfiMat tsfi_mat_subrect(TsfiMat *parent, int offset_x, int offset_y, int sub_w, int sub_h) {
    TsfiMat m;
    m.rows = sub_h;
    m.cols = sub_w;
    m.type = parent->type;
    m.stride = parent->stride;
    
    // Calculate memory offset based on row byte stride
    char *base_ptr = (char *)parent->data;
    char *offset_ptr = base_ptr + (offset_y * parent->stride) + (offset_x * sizeof(Ab4hPixel));
    m.data = (uint16_t *)offset_ptr;
    return m;
}

// Draw a line inside Mat using pure AB4H float parameters (no swizzling)
static void tsfi_mat_draw_line(TsfiMat *m, int x1, int y1, int x2, int y2, Ab4hPixel color) {
    int dx = abs(x2 - x1), sx = x1 < x2 ? 1 : -1;
    int dy = -abs(y2 - y1), sy = y1 < y2 ? 1 : -1;
    int err = dx + dy, e2;
    
    while (1) {
        if (x1 >= 0 && x1 < m->cols && y1 >= 0 && y1 < m->rows) {
            // Write directly to row-stride memory
            char *row_ptr = (char *)m->data + (y1 * m->stride);
            Ab4hPixel *pixel = (Ab4hPixel *)row_ptr + x1;
            *pixel = color;
        }
        if (x1 == x2 && y1 == y2) break;
        e2 = 2 * err;
        if (e2 >= dy) { err += dy; x1 += sx; }
        if (e2 <= dx) { err += dx; y1 += sy; }
    }
}

// Draw a filled rectangle inside Mat
static void tsfi_mat_draw_fill_rect(TsfiMat *m, int x1, int y1, int w, int h, Ab4hPixel color) {
    for (int y = y1; y < y1 + h; y++) {
        if (y >= 0 && y < m->rows) {
            char *row_ptr = (char *)m->data + (y * m->stride);
            Ab4hPixel *pixels = (Ab4hPixel *)row_ptr;
            for (int x = x1; x < x1 + w; x++) {
                if (x >= 0 && x < m->cols) {
                    pixels[x] = color;
                }
            }
        }
    }
}

void test_mat_synthesizer_displays() {
    printf("[TEST] Initializing zero-copy TsfiMat wrapper in AB4H mode...\n");

    int width = 1280;
    int height = 720;
    int stride = width * sizeof(Ab4hPixel); // Stride aligned exactly with pixel structure sizes
    
    void *vram_buffer = calloc(1, height * stride);
    assert(vram_buffer != NULL);

    // 1. Wrap external buffer as a standard Mat structure
    TsfiMat main_canvas = tsfi_mat_create_wrapper(height, width, stride, vram_buffer);
    assert(main_canvas.rows == height);
    assert(main_canvas.cols == width);
    assert(main_canvas.type == TSFI_CV_16FC4);
    assert(main_canvas.stride == stride);

    // 2. Define colors in native AB4H space
    Ab4hPixel dark_background = {
        double_to_half(0.04), // Red
        double_to_half(0.04), // Green
        double_to_half(0.06), // Blue
        double_to_half(1.00)  // Alpha
    };

    Ab4hPixel bright_green = {
        double_to_half(0.00),
        double_to_half(0.95),
        double_to_half(0.00),
        double_to_half(1.00)
    };

    Ab4hPixel active_gold = {
        double_to_half(0.95),
        double_to_half(0.75),
        double_to_half(0.00),
        double_to_half(1.00)
    };

    // 3. Clear canvas using filled rectangle
    tsfi_mat_draw_fill_rect(&main_canvas, 0, 0, width, height, dark_background);

    // 4. Create Sub-Region view of the main display representing a separate terminal window
    int term_x = 100;
    int term_y = 50;
    int term_w = 1080;
    int term_h = 620;
    TsfiMat term_view = tsfi_mat_subrect(&main_canvas, term_x, term_y, term_w, term_h);
    assert(term_view.rows == term_h);
    assert(term_view.cols == term_w);
    assert(term_view.stride == stride); // Stride must match the main canvas stride

    // Draw active gold border inside the sub-region bounds (translates to parent memory)
    tsfi_mat_draw_line(&term_view, 0, 0, term_w - 1, 0, active_gold);
    tsfi_mat_draw_line(&term_view, 0, term_h - 1, term_w - 1, term_h - 1, active_gold);
    tsfi_mat_draw_line(&term_view, 0, 0, 0, term_h - 1, active_gold);
    tsfi_mat_draw_line(&term_view, term_w - 1, 0, term_w - 1, term_h - 1, active_gold);

    // 5. Draw green grid lines within the sub-region
    for (int y = 50; y < term_h; y += 50) {
        tsfi_mat_draw_line(&term_view, 10, y, term_w - 11, y, bright_green);
    }

    // 6. Verify layout translation to main canvas memory
    // Coordinates inside term_view at (0, 0) should be at (term_x, term_y) on the main canvas
    char *main_base = (char *)main_canvas.data;
    char *main_offset = main_base + (term_y * main_canvas.stride) + (term_x * sizeof(Ab4hPixel));
    Ab4hPixel *pixel_main = (Ab4hPixel *)main_offset;
    
    assert(pixel_main->r == active_gold.r);
    assert(pixel_main->g == active_gold.g);
    assert(pixel_main->b == active_gold.b);
    assert(pixel_main->a == active_gold.a);

    printf("  [PASS] Main canvas wrapped successfully as TsfiMat.\n");
    printf("  [PASS] Sub-region ROI extracted successfully with correct stride.\n");
    printf("  [PASS] Line drawing completed with double-to-half float conversions.\n");
    printf("  [PASS] Memory offset verification matched between parent and sub-region views.\n");

    free(vram_buffer);
}

int main() {
    printf("=== TSFi OpenCV Mat Simulation Test ===\n");
    test_mat_synthesizer_displays();
    printf("ALL TSFI MAT SIMULATION TESTS PASSED\n");
    
    extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}
