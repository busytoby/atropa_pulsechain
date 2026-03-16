#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <time.h>
#include <math.h>
#include <signal.h>
#include "tsfi_raw.h"

#include "lau_memory.h"
#include "tsfi_ottype.h"
#include "vulkan/vulkan_render.h" 
#include "vulkan/vulkan_logic.h"
#include "vulkan/vulkan_main.h"
#include "tsfi_logic.h"
#include "tsfi_vision.h"
#include "tsfi_k0rn_ops.h"
#include "tsfi_io.h"
#include "tsfi_raw.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"
#include "tsfi_raw.h"

static inline double get_ms() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec * 1000.0 + (double)ts.tv_nsec / 1000000.0;
}

static void draw_line(uint32_t *px, int W, int H, float x0, float y0, float x1, float y1, uint32_t color) {
    int ix0 = (int)roundf(x0), iy0 = (int)roundf(y0), ix1 = (int)roundf(x1), iy1 = (int)roundf(y1);
    int dx = abs(ix1 - ix0), sx = x0 < x1 ? 1 : -1;
    int dy = -abs(iy1 - iy0), sy = y0 < y1 ? 1 : -1;
    int err = dx + dy, e2;
    while (1) {
        if (ix0 >= 0 && ix0 < W && iy0 >= 0 && iy0 < H) px[iy0 * W + ix0] = color;
        if (ix0 == ix1 && iy0 == iy1) break;
        e2 = 2 * err;
        if (e2 >= dy) { err += dy; ix0 += sx; }
        if (e2 <= dx) { err += dx; iy0 += sy; }
    }
}

static void draw_quad_bezier(uint32_t *px, int W, int H, float x0, float y0, float x1, float y1, float x2, float y2, uint32_t color) {
    int steps = 16; 
    float lx = x0, ly = y0;
    for (int i = 1; i <= steps; i++) {
        float t = (float)i / (float)steps;
        float omt = 1.0f - t;
        float nx = omt * omt * x0 + 2.0f * omt * t * x1 + t * t * x2;
        float ny = omt * omt * y0 + 2.0f * omt * t * y1 + t * t * y2;
        draw_line(px, W, H, lx, ly, nx, ny, color);
        lx = nx; ly = ny;
    }
}

// Helper for sorting X intersections
static int compare_floats(const void *a, const void *b) {
    float fa = *(const float *)a, fb = *(const float *)b;
    return (fa > fb) - (fa < fb);
}

int main(int argc, char **argv) {
    signal(SIGALRM, SIG_DFL);
    alarm(15);

    if (argc < 2) {
        fprintf(stderr, "Usage: %s <font_path>\n", argv[0]);
        return 1;
    }
    const char *font_path = argv[1];
    int f_fd = open(font_path, O_RDONLY);
    if (f_fd < 0) { perror("open font"); return 1; }
    struct stat st; fstat(f_fd, &st);
    uint8_t *font_data = mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, f_fd, 0);
    if (font_data == MAP_FAILED) { perror("mmap font"); return 1; }

    uint32_t target_cp = 0x0da4; // 'ඤ'

    stbtt_fontinfo stb_font;
    if (!stbtt_InitFont(&stb_font, font_data, 0)) return 1;
    int g_idx = stbtt_FindGlyphIndex(&stb_font, target_cp);
    int x0_stb, y0_stb, x1_stb, y1_stb;
    stbtt_GetGlyphBox(&stb_font, g_idx, &x0_stb, &y0_stb, &x1_stb, &y1_stb);
    float scale = stbtt_ScaleForPixelHeight(&stb_font, 100);
    int w_stb, h_stb, x_off_stb, y_off_stb;
    uint8_t *bitmap_stb = stbtt_GetGlyphBitmap(&stb_font, scale, scale, g_idx, &w_stb, &h_stb, &x_off_stb, &y_off_stb);

    uint32_t *stb_comp = lau_malloc(128 * 128 * 4);
    memset(stb_comp, 0, 128 * 128 * 4);
    if (bitmap_stb) {
        for (int y = 0; y < h_stb; y++) {
            if (y >= 120) break;
            for (int x = 0; x < w_stb; x++) {
                if (x >= 120) break;
                uint8_t alpha = bitmap_stb[y * w_stb + x];
                stb_comp[(10 + y) * 128 + (10 + x)] = (0xFFu << 24) | (alpha << 16) | (alpha << 8) | alpha;
            }
        }
    }

    tsfi_ottype_t ot;
    if (!tsfi_ottype_init(&ot, font_data, st.st_size, 0)) return 1;
    uint32_t g_idx_ot = tsfi_ottype_get_glyph_index(&ot, target_cp, 0);
    tsfi_glyph_geom_t geom = {0};
    bool has_geom = tsfi_ottype_get_glyph_geom(&ot, g_idx_ot, &geom);

    uint32_t *tsfi_comp = lau_malloc(128 * 128 * 4);
    memset(tsfi_comp, 0, 128 * 128 * 4);
    if (has_geom) {
        int start_pt = 0;
        for (int c = 0; c < geom.num_contours; c++) {
            int end_pt = geom.contour_ends[c];
            int count = end_pt - start_pt + 1;
            int first_on = -1;
            for (int i = 0; i < count; i++) if (geom.points[start_pt + i].type == 1) { first_on = i; break; }
            float cur_x, cur_y;
            int secret = 0;
            if (first_on == -1) {
                cur_x = (geom.points[end_pt].x + geom.points[start_pt].x) * 0.5f;
                cur_y = (geom.points[end_pt].y + geom.points[start_pt].y) * 0.5f;
            } else {
                cur_x = geom.points[start_pt + first_on].x;
                cur_y = geom.points[start_pt + first_on].y;
                secret = first_on + 1;
            }
            for (int k = 0; k < count; k++) {
                int idx = start_pt + ((secret + k) % count);
                tsfi_point_t p = geom.points[idx];
                float px0 = (cur_x - x0_stb) * scale + 10, py0 = (y1_stb - cur_y) * scale + 10;
                if (p.type == 1) {
                    float px1 = (p.x - x0_stb) * scale + 10, py1 = (y1_stb - p.y) * scale + 10;
                    draw_line(tsfi_comp, 128, 128, px0, py0, px1, py1, 0xFFFFFFFF);
                    cur_x = p.x; cur_y = p.y;
                } else {
                    int next_idx = start_pt + ((secret + k + 1) % count);
                    tsfi_point_t p_next = geom.points[next_idx];
                    float x_end = p_next.x, y_end = p_next.y;
                    if (p_next.type == 2) { x_end = (p.x + p_next.x) * 0.5f; y_end = (p.y + p_next.y) * 0.5f; }
                    else { k++; }
                    float px_ctrl = (p.x - x0_stb) * scale + 10, py_ctrl = (y1_stb - p.y) * scale + 10;
                    float px_end = (x_end - x0_stb) * scale + 10, py_end = (y1_stb - y_end) * scale + 10;
                    draw_quad_bezier(tsfi_comp, 128, 128, px0, py0, px_ctrl, py_ctrl, px_end, py_end, 0xFFFFFFFF);
                    cur_x = x_end; cur_y = y_end;
                }
            }
            start_pt = end_pt + 1;
        }
    }

    K0RnStream *dna = tsfi_ottype_compile_dna(&geom);
    uint32_t *dna_comp = lau_malloc(128 * 128 * 4);
    memset(dna_comp, 0, 128 * 128 * 4);
    if (dna) {
        printf("[TSFI] Manifesting DNA High-Quality SSAA Fill (%u ops)...\n", dna->op_count); fflush(stdout);
        char dna_name[32]; snprintf(dna_name, 31, "0x%X", target_cp);
        tsfi_k0rn_save(dna_name, dna);
        
        // 4x4 SSAA Scanline Accumulator
        for (int y = 0; y < 128; y++) {
            for (int x = 0; x < 128; x++) {
                int total_subpixels = 0;
                for (int sy = 0; sy < 4; sy++) {
                    float fy = (1.0f - ((float)y + (float)sy * 0.25f + 0.125f) / 64.0f);
                    float intersections[1024];
                    int int_count = 0;
                    for (uint32_t i = 0; i < dna->op_count; i++) {
                        K0RnOp op = dna->ops[i];
                        float dy = op.w - op.y;
                        if (fabsf(dy) < 1e-6f) continue;
                        if (((op.y > fy) != (op.w > fy))) {
                            float intersect_x = (op.z - op.x) * (fy - op.y) / dy + op.x;
                            intersections[int_count++] = (intersect_x + 1.0f) * 64.0f;
                        }
                    }
                    qsort(intersections, int_count, sizeof(float), compare_floats);
                    for (int sx = 0; sx < 4; sx++) {
                        float fx = (float)x + (float)sx * 0.25f + 0.125f;
                        int inside = 0;
                        for (int m = 0; m < int_count; m++) {
                            if (fx < intersections[m]) { inside = (int_count - m) % 2; break; }
                        }
                        if (inside) total_subpixels++;
                    }
                }
                if (total_subpixels > 0) {
                    uint8_t alpha = (uint8_t)((float)total_subpixels / 16.0f * 255.0f);
                    dna_comp[y * 128 + x] = (0xFFu << 24) | (alpha << 16) | (alpha << 8) | alpha; // White with AA
                    // Apply Cyan tint
                    uint32_t c = dna_comp[y * 128 + x];
                    dna_comp[y * 128 + x] = (c & 0xFF000000) | ((c & 0x00FFFFFF) & 0x00FFFF);
                }
            }
            if (y % 16 == 0) { printf("."); fflush(stdout); }
        }
        printf(" Done.\n"); fflush(stdout);
    }

    float edge_sim = tsfi_vision_calculate_edge_similarity(stb_comp, tsfi_comp, 128, 128);
    printf("[TSFI_VISION] Edge Similarity (0x%X): %.4f\n", target_cp, edge_sim);

    VulkanSystem *s = create_vulkan_system();
    if (!s) return 1;
    s->external_render_enabled = true; 
    char title[128]; snprintf(title, 128, "TSFi Font [0x%X] - DNA High-Quality", target_cp);
    xdg_toplevel_set_title(s->xdg_toplevel, title);

    double start_time = get_ms();
    while (s->running) {
        if (get_ms() - start_time >= 5000.0) break;
        wl_display_dispatch_pending(s->display);
        uint32_t *px = (uint32_t*)s->paint_buffer->data;
        int W = s->paint_buffer->width, H = s->paint_buffer->height;
        for (int y = 0; y < H; y++) for (int x = 0; x < W; x++) {
            if ((x / 64 + y / 64) % 2 == 0) px[y * W + x] = 0xFF333333; else px[y * W + x] = 0xFF222222;
        }
        for (int y = 0; y < 128; y++) for (int x = 0; x < 128; x++) {
            px[(100 + y) * W + (100 + x)] = stb_comp[y * 128 + x];
            px[(100 + y) * W + (500 + x)] = tsfi_comp[y * 128 + x];
            px[(300 + y) * W + (300 + x)] = dna_comp[y * 128 + x];
        }
        draw_frame(s); tsfi_raw_usleep(16000); 
    }
    tsfi_ottype_free_geom(&geom); lau_free(stb_comp); lau_free(tsfi_comp); lau_free(dna_comp);
    if (dna) { lau_free(dna->ops); lau_free(dna); }
    destroy_vulkan_system(s);     extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}
