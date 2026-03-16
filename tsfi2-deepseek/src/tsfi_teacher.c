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

#include "lau_memory.h"
#include "tsfi_ottype.h"
#include "vulkan/vulkan_render.h" 
#include "vulkan/vulkan_logic.h"
#include "vulkan/vulkan_main.h"
#include "tsfi_logic.h"
#include "tsfi_vision.h"
#include "tsfi_k0rn_ops.h"
#include "tsfi_io.h"

#include "stb_truetype.h"

typedef struct {
    char path[256];
    uint8_t *data;
    size_t size;
    stbtt_fontinfo stb;
    tsfi_ottype_t ot;
} font_ref_t;

#define MAX_FONTS 16
static font_ref_t g_fonts[MAX_FONTS];
static int g_font_count = 0;

static bool load_font(const char *path) {
    if (g_font_count >= MAX_FONTS) return false;
    int f_fd = open(path, O_RDONLY);
    if (f_fd < 0) return false;
    struct stat st; fstat(f_fd, &st);
    uint8_t *data = mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, f_fd, 0);
    close(f_fd);
    if (data == MAP_FAILED) return false;

    font_ref_t *f = &g_fonts[g_font_count];
    strncpy(f->path, path, 255);
    f->data = data;
    f->size = st.st_size;
    if (!stbtt_InitFont(&f->stb, data, 0)) { munmap(data, st.st_size); return false; }
    if (!tsfi_ottype_init(&f->ot, data, st.st_size, 0)) { lau_memset(data, 0, st.st_size); munmap(data, st.st_size); return false; }
    
    g_font_count++;
    return true;
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
    int steps = 8; 
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

int main(int argc, char **argv) {
    if (argc < 4) {
        tsfi_io_printf(stderr, "Usage: %s <start_cp> <end_cp> <font1> [font2...]\n", argv[0]);
        return 1;
    }

    uint32_t start_cp = (uint32_t)strtoul(argv[1], NULL, 0);
    uint32_t end_cp = (uint32_t)strtoul(argv[2], NULL, 0);

    for (int i = 3; i < argc; i++) {
        if (load_font(argv[i])) {
            tsfi_io_printf(stdout, "[TEACHER] Loaded font: %s\n", argv[i]);
        }
    }

    if (g_font_count == 0) {
        tsfi_io_printf(stderr, "[TEACHER] Error: No valid fonts loaded.\n");
        return 1;
    }

    tsfi_io_printf(stdout, "[TEACHER] Processing range 0x%X - 0x%X\n", start_cp, end_cp);

    uint32_t *stb_comp = (uint32_t*)lau_malloc(128 * 128 * 4);
    uint32_t *tsfi_comp = (uint32_t*)lau_malloc(128 * 128 * 4);

    int success_count = 0;
    int composite_skip = 0;
    int missing_skip = 0;
    int low_quality_skip = 0;

    for (uint32_t cp = start_cp; cp <= end_cp; cp++) {
        font_ref_t *best_f = NULL;
        uint32_t g_idx = 0;

        for (int i = 0; i < g_font_count; i++) {
            g_idx = tsfi_ottype_get_glyph_index(&g_fonts[i].ot, cp, 0);
            if (g_idx != 0) {
                best_f = &g_fonts[i];
                break;
            }
        }

        if (!best_f) {
            missing_skip++;
            continue;
        }

        tsfi_glyph_geom_t geom = {0};
        if (!tsfi_ottype_get_glyph_geom(&best_f->ot, g_idx, &geom)) {
            composite_skip++;
            continue;
        }

        // 1. Generate STB Reference
        memset(stb_comp, 0, 128 * 128 * 4);
        int x0_stb, y0_stb, x1_stb, y1_stb;
        stbtt_GetGlyphBox(&best_f->stb, (int)g_idx, &x0_stb, &y0_stb, &x1_stb, &y1_stb);
        float scale = stbtt_ScaleForPixelHeight(&best_f->stb, 100);
        int w_stb, h_stb, x_off_stb, y_off_stb;
        uint8_t *bitmap_stb = stbtt_GetGlyphBitmap(&best_f->stb, scale, scale, (int)g_idx, &w_stb, &h_stb, &x_off_stb, &y_off_stb);
        if (bitmap_stb) {
            for (int y = 0; y < h_stb && y < 120; y++) {
                for (int x = 0; x < w_stb && x < 120; x++) {
                    uint8_t alpha = bitmap_stb[y * w_stb + x];
                    stb_comp[(10 + y) * 128 + (10 + x)] = (0xFFu << 24) | (alpha << 16) | (alpha << 8) | alpha;
                }
            }
            stbtt_FreeBitmap(bitmap_stb, NULL);
        }

        // 2. Generate TSFi Outline Reference
        memset(tsfi_comp, 0, 128 * 128 * 4);
        int start_pt = 0;
        for (int c = 0; c < (int)geom.num_contours; c++) {
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

        // 3. Compare and Save
        float edge_sim = tsfi_vision_calculate_edge_similarity(stb_comp, tsfi_comp, 128, 128);
        if (edge_sim > 0.45f) {
            K0RnStream *dna = tsfi_ottype_compile_dna(&geom);
            if (dna) {
                char dna_name[32]; snprintf(dna_name, 31, "0x%X", cp);
                tsfi_k0rn_save(dna_name, dna);
                lau_free(dna->ops);
                lau_free(dna);
                success_count++;
            }
        } else {
            low_quality_skip++;
        }

        tsfi_ottype_free_geom(&geom);
        if (cp % 100 == 0) {
            tsfi_io_printf(stdout, "[TEACHER] Progress: 0x%X (Success: %d, Composite: %d, Missing: %d, LowQual: %d)\n", 
                   cp, success_count, composite_skip, missing_skip, low_quality_skip);
        }
    }

    tsfi_io_printf(stdout, "[TEACHER] Batch Complete.\n");
    tsfi_io_printf(stdout, "  Total Success: %d\n", success_count);
    tsfi_io_printf(stdout, "  Composites skipped: %d\n", composite_skip);
    tsfi_io_printf(stdout, "  Missing from fonts: %d\n", missing_skip);
    tsfi_io_printf(stdout, "  Low Quality: %d\n", low_quality_skip);

    lau_free(stb_comp);
    lau_free(tsfi_comp);
    for (int i = 0; i < g_font_count; i++) {
        munmap(g_fonts[i].data, g_fonts[i].size);
    }

    return 0;
}
