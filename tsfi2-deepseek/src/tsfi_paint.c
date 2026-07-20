#define _GNU_SOURCE
#include "tsfi_paint.h"
#include "stb_truetype.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

// Minimal 8x8 monochrome bitmap font definition (covering standard ASCII 32 - 126)
static const uint8_t font_8x8[96][8] = {
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // space
    {0x18, 0x3c, 0x3c, 0x18, 0x18, 0x00, 0x18, 0x00}, // !
    {0x6c, 0x6c, 0x6c, 0x00, 0x00, 0x00, 0x00, 0x00}, // "
    {0x36, 0x36, 0x7f, 0x36, 0x7f, 0x36, 0x36, 0x00}, // #
    {0x18, 0x3e, 0x60, 0x3c, 0x06, 0x7c, 0x18, 0x00}, // $
    {0x00, 0x66, 0x6c, 0x18, 0x30, 0x66, 0x46, 0x00}, // %
    {0x3c, 0x66, 0x3c, 0x38, 0x67, 0x66, 0x3f, 0x00}, // &
    {0x06, 0x0c, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00}, // '
    {0x0c, 0x18, 0x30, 0x30, 0x30, 0x18, 0x0c, 0x00}, // (
    {0x30, 0x18, 0x0c, 0x0c, 0x0c, 0x18, 0x30, 0x00}, // )
    {0x00, 0x66, 0x3c, 0xff, 0x3c, 0x66, 0x00, 0x00}, // *
    {0x00, 0x18, 0x18, 0x7e, 0x18, 0x18, 0x00, 0x00}, // +
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 0x30}, // ,
    {0x00, 0x00, 0x00, 0x7e, 0x00, 0x00, 0x00, 0x00}, // -
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 0x00}, // .
    {0x03, 0x06, 0x0c, 0x18, 0x30, 0x60, 0xc0, 0x00}, // /
    {0x3e, 0x63, 0x67, 0x6f, 0x7b, 0x63, 0x3e, 0x00}, // 0
    {0x0c, 0x1e, 0x0c, 0x0c, 0x0c, 0x0c, 0x3f, 0x00}, // 1
    {0x3e, 0x63, 0x06, 0x1c, 0x30, 0x63, 0x7f, 0x00}, // 2
    {0x7f, 0x06, 0x0c, 0x1e, 0x03, 0x63, 0x3e, 0x00}, // 3
    {0x06, 0x0e, 0x1e, 0x36, 0x7f, 0x06, 0x06, 0x00}, // 4
    {0x7f, 0x60, 0x7e, 0x03, 0x03, 0x63, 0x3e, 0x00}, // 5
    {0x1c, 0x30, 0x60, 0x7c, 0x63, 0x63, 0x3e, 0x00}, // 6
    {0x7f, 0x03, 0x06, 0x0c, 0x18, 0x18, 0x18, 0x00}, // 7
    {0x3e, 0x63, 0x63, 0x3e, 0x63, 0x63, 0x3e, 0x00}, // 8
    {0x3e, 0x63, 0x63, 0x3e, 0x03, 0x06, 0x3c, 0x00}, // 9
    {0x00, 0x18, 0x18, 0x00, 0x00, 0x18, 0x18, 0x00}, // :
    {0x00, 0x18, 0x18, 0x00, 0x00, 0x18, 0x18, 0x30}, // ;
    {0x0c, 0x18, 0x30, 0x60, 0x30, 0x18, 0x0c, 0x00}, // <
    {0x00, 0x00, 0x7e, 0x00, 0x7e, 0x00, 0x00, 0x00}, // =
    {0x30, 0x18, 0x0c, 0x06, 0x0c, 0x18, 0x30, 0x00}, // >
    {0x3e, 0x63, 0x03, 0x06, 0x0c, 0x00, 0x0c, 0x00}, // ?
    {0x3e, 0x63, 0x6f, 0x6b, 0x6f, 0x60, 0x3e, 0x00}, // @
    {0x18, 0x3c, 0x66, 0x66, 0x7e, 0x66, 0x66, 0x00}, // A
    {0x7e, 0x33, 0x33, 0x3e, 0x33, 0x33, 0x7e, 0x00}, // B
    {0x1e, 0x33, 0x60, 0x60, 0x60, 0x33, 0x1e, 0x00}, // C
    {0x7c, 0x22, 0x22, 0x22, 0x22, 0x22, 0x7c, 0x00}, // D
    {0x7f, 0x60, 0x60, 0x7c, 0x60, 0x60, 0x7f, 0x00}, // E
    {0x7f, 0x60, 0x60, 0x7c, 0x60, 0x60, 0x60, 0x00}, // F
    {0x3e, 0x63, 0x60, 0x6f, 0x63, 0x63, 0x3e, 0x00}, // G
    {0x66, 0x66, 0x66, 0x7e, 0x66, 0x66, 0x66, 0x00}, // H
    {0x3c, 0x18, 0x18, 0x18, 0x18, 0x18, 0x3c, 0x00}, // I
    {0x0f, 0x03, 0x03, 0x03, 0x03, 0x63, 0x3e, 0x00}, // J
    {0x66, 0x6c, 0x78, 0x70, 0x78, 0x6c, 0x66, 0x00}, // K
    {0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x7f, 0x00}, // L
    {0x63, 0x77, 0x7f, 0x6b, 0x63, 0x63, 0x63, 0x00}, // M
    {0x66, 0x76, 0x7e, 0x7e, 0x6e, 0x66, 0x66, 0x00}, // N
    {0x3e, 0x63, 0x63, 0x63, 0x63, 0x63, 0x3e, 0x00}, // O
    {0x7e, 0x63, 0x63, 0x7e, 0x60, 0x60, 0x60, 0x00}, // P
    {0x3e, 0x63, 0x63, 0x63, 0x6b, 0x67, 0x3e, 0x03}, // Q
    {0x7e, 0x63, 0x63, 0x7e, 0x78, 0x6c, 0x66, 0x00}, // R
    {0x3e, 0x63, 0x38, 0x0e, 0x07, 0x63, 0x3e, 0x00}, // S
    {0x7f, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x00}, // T
    {0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x3e, 0x00}, // U
    {0x66, 0x66, 0x66, 0x66, 0x66, 0x3c, 0x18, 0x00}, // V
    {0x63, 0x63, 0x63, 0x6b, 0x7f, 0x77, 0x63, 0x00}, // W
    {0x66, 0x66, 0x3c, 0x18, 0x3c, 0x66, 0x66, 0x00}, // X
    {0x66, 0x66, 0x66, 0x3c, 0x18, 0x18, 0x18, 0x00}, // Y
    {0x7f, 0x03, 0x06, 0x0c, 0x18, 0x30, 0x7f, 0x00}, // Z
    {0x3c, 0x30, 0x30, 0x30, 0x30, 0x30, 0x3c, 0x00}, // [
    {0xc0, 0x60, 0x30, 0x18, 0x0c, 0x06, 0x03, 0x00}, // backslash
    {0x3c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x3c, 0x00}, // ]
    {0x18, 0x3c, 0x66, 0x00, 0x00, 0x00, 0x00, 0x00}, // ^
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff}, // _
    {0x18, 0x0c, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00}, // `
    {0x00, 0x00, 0x3c, 0x03, 0x3f, 0x63, 0x3f, 0x00}, // a
    {0x60, 0x60, 0x7c, 0x66, 0x66, 0x66, 0x7c, 0x00}, // b
    {0x00, 0x00, 0x3e, 0x60, 0x60, 0x63, 0x3e, 0x00}, // c
    {0x03, 0x03, 0x3f, 0x63, 0x63, 0x63, 0x3f, 0x00}, // d
    {0x00, 0x00, 0x3e, 0x63, 0x7f, 0x60, 0x3e, 0x00}, // e
    {0x1c, 0x36, 0x30, 0x78, 0x30, 0x30, 0x30, 0x00}, // f
    {0x00, 0x00, 0x3f, 0x63, 0x63, 0x3f, 0x03, 0x3e}, // g
    {0x60, 0x60, 0x7c, 0x66, 0x66, 0x66, 0x66, 0x00}, // h
    {0x18, 0x00, 0x38, 0x18, 0x18, 0x18, 0x3c, 0x00}, // i
    {0x06, 0x00, 0x0e, 0x06, 0x06, 0x66, 0x3c, 0x00}, // j
    {0x60, 0x60, 0x66, 0x6c, 0x78, 0x6c, 0x66, 0x00}, // k
    {0x38, 0x18, 0x18, 0x18, 0x18, 0x18, 0x3c, 0x00}, // l
    {0x00, 0x00, 0x76, 0x7f, 0x6b, 0x63, 0x63, 0x00}, // m
    {0x00, 0x00, 0x7c, 0x66, 0x66, 0x66, 0x66, 0x00}, // n
    {0x00, 0x00, 0x3e, 0x63, 0x63, 0x63, 0x3e, 0x00}, // o
    {0x00, 0x00, 0x7c, 0x66, 0x66, 0x7c, 0x60, 0x60}, // p
    {0x00, 0x00, 0x3f, 0x63, 0x63, 0x3f, 0x03, 0x03}, // q
    {0x00, 0x00, 0x5e, 0x30, 0x30, 0x30, 0x30, 0x00}, // r
    {0x00, 0x00, 0x3e, 0x60, 0x3e, 0x03, 0x3e, 0x00}, // s
    {0x30, 0x30, 0x7c, 0x30, 0x30, 0x30, 0x1e, 0x00}, // t
    {0x00, 0x00, 0x66, 0x66, 0x66, 0x66, 0x3f, 0x00}, // u
    {0x00, 0x00, 0x66, 0x66, 0x66, 0x3c, 0x18, 0x00}, // v
    {0x00, 0x00, 0x63, 0x63, 0x6b, 0x7f, 0x36, 0x00}, // w
    {0x00, 0x00, 0x66, 0x3c, 0x18, 0x3c, 0x66, 0x00}, // x
    {0x00, 0x00, 0x66, 0x66, 0x66, 0x3f, 0x03, 0x3e}, // y
    {0x00, 0x00, 0x7f, 0x06, 0x0c, 0x18, 0x7f, 0x00}, // z
    {0x0c, 0x18, 0x18, 0x30, 0x18, 0x18, 0x0c, 0x00}, // {
    {0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x00}, // |
    {0x30, 0x18, 0x18, 0x0c, 0x18, 0x18, 0x30, 0x00}, // }
    {0x76, 0xdc, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}  // ~
};

// Internal parser helpers
static const char* get_style_val_paint(const char *style, const char *prop_name, char *buf, size_t buf_sz) {
    if (!style) return NULL;
    const char *p = style;
    size_t prop_len = strlen(prop_name);
    while (*p) {
        p = strstr(p, prop_name);
        if (!p) break;
        if (p == style || *(p - 1) == ';' || isspace((unsigned char)*(p - 1))) {
            const char *colon = strchr(p, ':');
            if (colon) {
                const char *val_start = colon + 1;
                while (*val_start && isspace((unsigned char)*val_start)) val_start++;
                const char *semi = strchr(val_start, ';');
                size_t val_len = semi ? (size_t)(semi - val_start) : strlen(val_start);
                while (val_len > 0 && isspace((unsigned char)val_start[val_len - 1])) val_len--;
                if (val_len < buf_sz) {
                    memcpy(buf, val_start, val_len);
                    buf[val_len] = '\0';
                    return buf;
                }
            }
        }
        p += prop_len;
    }
    return NULL;
}

#include <math.h>

static uint32_t blend_pixels(uint32_t src, uint32_t dst, float intensity) {
    if (intensity <= 0.001f) return dst;
    if (intensity >= 0.999f) return src;
    
    uint8_t sa = (src >> 24) & 0xFF;
    uint8_t sr = (src >> 16) & 0xFF;
    uint8_t sg = (src >> 8) & 0xFF;
    uint8_t sb = src & 0xFF;
    
    uint8_t da = (dst >> 24) & 0xFF;
    uint8_t dr = (dst >> 16) & 0xFF;
    uint8_t dg = (dst >> 8) & 0xFF;
    uint8_t db = dst & 0xFF;
    
    float factor = intensity * (sa / 255.0f);
    uint8_t r = (uint8_t)(sr * factor + dr * (1.0f - factor));
    uint8_t g = (uint8_t)(sg * factor + dg * (1.0f - factor));
    uint8_t b = (uint8_t)(sb * factor + db * (1.0f - factor));
    uint8_t a = (uint8_t)(sa * intensity + da * (1.0f - intensity));
    
    return ((uint32_t)a << 24) | ((uint32_t)r << 16) | ((uint32_t)g << 8) | b;
}

// Parses hex or CSS colors into a 32-bit ARGB pixel format
static uint32_t parse_color(const char *color_str, uint32_t fallback) {
    if (!color_str) return fallback;
    while (*color_str && isspace((unsigned char)*color_str)) color_str++;

    if (color_str[0] == '#') {
        color_str++;
        unsigned int val = 0;
        if (strlen(color_str) == 6) {
            sscanf(color_str, "%x", &val);
            return 0xFF000000 | val;
        } else if (strlen(color_str) == 3) {
            unsigned int r, g, b;
            sscanf(color_str, "%1x%1x%1x", &r, &g, &b);
            r = (r << 4) | r;
            g = (g << 4) | g;
            b = (b << 4) | b;
            return 0xFF000000 | (r << 16) | (g << 8) | b;
        }
    }

    if (strncasecmp(color_str, "rgb", 3) == 0) {
        const char *p = strchr(color_str, '(');
        if (p) {
            p++;
            int r = 0, g = 0, b = 0;
            float a = 1.0f;
            if (sscanf(p, "%d , %d , %d , %f", &r, &g, &b, &a) == 4) {
                uint32_t alpha = (uint32_t)(a * 255.0f) & 0xFF;
                return (alpha << 24) | ((r & 0xFF) << 16) | ((g & 0xFF) << 8) | (b & 0xFF);
            } else if (sscanf(p, "%d , %d , %d", &r, &g, &b) == 3) {
                return 0xFF000000 | ((r & 0xFF) << 16) | ((g & 0xFF) << 8) | (b & 0xFF);
            }
        }
    }

    // Color string names
    if (strcasecmp(color_str, "red") == 0) return 0xFFFF0000;
    if (strcasecmp(color_str, "blue") == 0) return 0xFF0000FF;
    if (strcasecmp(color_str, "green") == 0) return 0xFF00FF00;
    if (strcasecmp(color_str, "purple") == 0) return 0xFF800080;
    if (strcasecmp(color_str, "white") == 0) return 0xFFFFFFFF;
    if (strcasecmp(color_str, "black") == 0) return 0xFF000000;
    if (strcasecmp(color_str, "gray") == 0 || strcasecmp(color_str, "grey") == 0) return 0xFF808080;
    if (strcasecmp(color_str, "darkgrey") == 0 || strcasecmp(color_str, "darkgray") == 0) return 0xFF5A5A5A;
    if (strcasecmp(color_str, "lightgrey") == 0 || strcasecmp(color_str, "lightgray") == 0) return 0xFFD3D3D3;
    if (strcasecmp(color_str, "silver") == 0) return 0xFFC0C0C0;
    if (strcasecmp(color_str, "navy") == 0) return 0xFF000080;
    if (strcasecmp(color_str, "teal") == 0) return 0xFF008080;
    if (strcasecmp(color_str, "olive") == 0) return 0xFF808000;
    if (strcasecmp(color_str, "maroon") == 0) return 0xFF800000;
    if (strcasecmp(color_str, "orange") == 0) return 0xFFFFA500;
    if (strcasecmp(color_str, "yellow") == 0) return 0xFFFFD700;
    if (strcasecmp(color_str, "transparent") == 0) return 0x00000000;

    return fallback;
}

// Visual drawing primitives
static void draw_rect(uint32_t *pixels, int w, int h, int rx, int ry, int rw, int rh, uint32_t color) {
    for (int y = ry; y < ry + rh; y++) {
        if (y < 0 || y >= h) continue;
        uint32_t *row = pixels + y * w;
        for (int x = rx; x < rx + rw; x++) {
            if (x < 0 || x >= w) continue;
            row[x] = color;
        }
    }
}

static void draw_border(uint32_t *pixels, int w, int h, int rx, int ry, int rw, int rh, int border_w, uint32_t color) {
    if (border_w <= 0) return;
    // Top border
    draw_rect(pixels, w, h, rx, ry, rw, border_w, color);
    // Bottom border
    draw_rect(pixels, w, h, rx, ry + rh - border_w, rw, border_w, color);
    // Left border
    draw_rect(pixels, w, h, rx, ry, border_w, rh, color);
    // Right border
    draw_rect(pixels, w, h, rx + rw - border_w, ry, border_w, rh, color);
}

static stbtt_fontinfo g_font_info;
static unsigned char *g_font_buffer = NULL;
static bool g_font_initialized = false;

static void init_vector_font() {
    if (g_font_initialized) return;
    const char *font_paths[] = {
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        "/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf",
        "/usr/share/fonts/truetype/freefont/FreeSans.ttf"
    };
    for (int i = 0; i < 3; i++) {
        FILE *f = fopen(font_paths[i], "rb");
        if (f) {
            fseek(f, 0, SEEK_END);
            long sz = ftell(f);
            fseek(f, 0, SEEK_SET);
            g_font_buffer = (unsigned char *)malloc(sz);
            size_t read_bytes = fread(g_font_buffer, 1, sz, f);
            (void)read_bytes;
            fclose(f);
            if (stbtt_InitFont(&g_font_info, g_font_buffer, 0)) {
                g_font_initialized = true;
                printf("[Auncient Font] Loaded vector font: %s\n", font_paths[i]);
                return;
            }
            free(g_font_buffer);
            g_font_buffer = NULL;
        }
    }
    printf("[Auncient Font] Warning: No vector font loaded, falling back to 8x8 bitmap font.\n");
}

static void draw_char(uint32_t *pixels, int w, int h, int rx, int ry, unsigned char c, uint32_t color, int scale) {
    init_vector_font();
    if (!g_font_initialized) {
        if (c < 32 || c > 127) return;
        int idx = c - 32;
        int char_w = 8 * scale;
        int char_h = 8 * scale;
        for (int dy = 0; dy < char_h; dy++) {
            int y = ry + dy;
            if (y < 0 || y >= h) continue;
            uint32_t *row_pixels = pixels + y * w;
            float v = ((float)dy + 0.5f) / (float)scale - 0.5f;
            int v0 = (int)floorf(v);
            int v1 = v0 + 1;
            float wv = v - (float)v0;
            if (v0 < 0) { v0 = 0; v1 = 0; wv = 0.0f; }
            if (v1 > 7) { v1 = 7; v0 = 7; wv = 0.0f; }
            uint8_t row0_bits = font_8x8[idx][v0];
            uint8_t row1_bits = font_8x8[idx][v1];
            for (int dx = 0; dx < char_w; dx++) {
                int x = rx + dx;
                if (x < 0 || x >= w) continue;
                float u = ((float)dx + 0.5f) / (float)scale - 0.5f;
                int u0 = (int)floorf(u);
                int u1 = u0 + 1;
                float wu = u - (float)u0;
                if (u0 < 0) { u0 = 0; u1 = 0; wu = 0.0f; }
                if (u1 > 7) { u1 = 7; u0 = 7; wu = 0.0f; }
                float val00 = (float)((row0_bits >> (7 - u0)) & 1);
                float val10 = (float)((row0_bits >> (7 - u1)) & 1);
                float val01 = (float)((row1_bits >> (7 - u0)) & 1);
                float val11 = (float)((row1_bits >> (7 - u1)) & 1);
                float intensity = val00 * (1.0f - wu) * (1.0f - wv) +
                                  val10 * wu * (1.0f - wv) +
                                  val01 * (1.0f - wu) * wv +
                                  val11 * wu * wv;
                if (intensity > 0.001f) {
                    row_pixels[x] = blend_pixels(color, row_pixels[x], intensity);
                }
            }
        }
        return;
    }

    float font_size = 12.0f * scale;
    float scale_factor = stbtt_ScaleForPixelHeight(&g_font_info, font_size);
    int char_w, char_h, xoff, yoff;
    unsigned char *bitmap = stbtt_GetCodepointBitmap(&g_font_info, scale_factor, scale_factor, c, &char_w, &char_h, &xoff, &yoff);
    if (!bitmap) return;

    int start_y = ry + yoff + (int)(font_size * 0.8f);

    for (int dy = 0; dy < char_h; dy++) {
        int y = start_y + dy;
        if (y < 0 || y >= h) continue;
        uint32_t *row_pixels = pixels + y * w;
        for (int dx = 0; dx < char_w; dx++) {
            int x = rx + xoff + dx;
            if (x < 0 || x >= w) continue;
            float intensity = (float)bitmap[dy * char_w + dx] / 255.0f;
            if (intensity > 0.001f) {
                row_pixels[x] = blend_pixels(color, row_pixels[x], intensity);
            }
        }
    }
    stbtt_FreeBitmap(bitmap, NULL);
}

static void draw_text(uint32_t *pixels, int w, int h, int rx, int ry, const char *text, uint32_t color, int scale) {
    int cur_x = rx;
    while (*text) {
        draw_char(pixels, w, h, cur_x, ry, *text, color, scale);
        init_vector_font();
        if (g_font_initialized) {
            float font_size = 12.0f * scale;
            float scale_factor = stbtt_ScaleForPixelHeight(&g_font_info, font_size);
            int advance, lsb;
            stbtt_GetCodepointHMetrics(&g_font_info, *text, &advance, &lsb);
            cur_x += (int)(advance * scale_factor);
        } else {
            cur_x += 8 * scale;
        }
        text++;
    }
}

const uint8_t *g_active_video_frame = NULL;
int g_active_video_w = 0;
int g_active_video_h = 0;

static void draw_rect_rounded(uint32_t *pixels, int w, int h, int rx, int ry, int rw, int rh, int radius, uint32_t color) {
    if (radius <= 0) {
        draw_rect(pixels, w, h, rx, ry, rw, rh, color);
        return;
    }
    for (int y = ry; y < ry + rh; y++) {
        if (y < 0 || y >= h) continue;
        uint32_t *row = pixels + y * w;
        int dy = 0;
        if (y < ry + radius) dy = (ry + radius) - y;
        else if (y >= ry + rh - radius) dy = y - (ry + rh - radius - 1);

        for (int x = rx; x < rx + rw; x++) {
            if (x < 0 || x >= w) continue;
            int dx = 0;
            if (x < rx + radius) dx = (rx + radius) - x;
            else if (x >= rx + rw - radius) dx = x - (rx + rw - radius - 1);

            if (dx > 0 && dy > 0) {
                if (dx * dx + dy * dy <= radius * radius) {
                    row[x] = color;
                }
            } else {
                row[x] = color;
            }
        }
    }
}

static void draw_video_frame(uint32_t *pixels, int w, int h, int rx, int ry, int rw, int rh, const uint8_t *video_rgba, int video_w, int video_h) {
    for (int y = 0; y < rh; y++) {
        int dy = ry + y;
        if (dy < 0 || dy >= h) continue;
        uint32_t *row = pixels + dy * w;
        float sy = (float)y * (float)video_h / (float)rh;
        int vy = (int)sy;
        if (vy >= video_h) vy = video_h - 1;
        const uint8_t *src_row = video_rgba + vy * video_w * 4;

        for (int x = 0; x < rw; x++) {
            int dx = rx + x;
            if (dx < 0 || dx >= w) continue;
            float sx = (float)x * (float)video_w / (float)rw;
            int vx_idx = (int)sx;
            if (vx_idx >= video_w) vx_idx = video_w - 1;
            const uint8_t *pixel = src_row + vx_idx * 4;
            row[dx] = (0xFF000000) | (pixel[0] << 16) | (pixel[1] << 8) | pixel[2];
        }
    }
}

void tsfi_paint_tree(const TsfiLayoutBox *box, uint32_t *pixels, int width, int height) {
    if (!box) return;

    const char *style = box->node ? tsfi_node_get_attribute(box->node, "style") : NULL;
    char buf[128];

    // Paint backgrounds for block boxes if styling is present
    if (box->type == TSFI_BOX_BLOCK || box->type == TSFI_BOX_FLEX || box->type == TSFI_BOX_INLINE) {
        uint32_t bg_color = 0;
        bool has_bg = false;
        if (get_style_val_paint(style, "background-color", buf, sizeof(buf))) {
            bg_color = parse_color(buf, 0);
            has_bg = true;
        } else if (get_style_val_paint(style, "background", buf, sizeof(buf))) {
            bg_color = parse_color(buf, 0);
            has_bg = true;
        }

        int radius = 0;
        if (get_style_val_paint(style, "border-radius", buf, sizeof(buf))) {
            radius = atoi(buf);
        }

        if (has_bg) {
            draw_rect_rounded(pixels, width, height, (int)box->x, (int)box->y, (int)box->w, (int)box->h, radius, bg_color);
        }

        // Draw borders if applicable
        float max_border = box->border_left;
        if (box->border_right > max_border) max_border = box->border_right;
        if (box->border_top > max_border) max_border = box->border_top;
        if (box->border_bottom > max_border) max_border = box->border_bottom;

        if (max_border > 0.0f) {
            uint32_t border_color = 0xFFFFFFFF; // default white
            if (get_style_val_paint(style, "border-color", buf, sizeof(buf))) {
                border_color = parse_color(buf, border_color);
            }
            draw_border(pixels, width, height, (int)box->x, (int)box->y, (int)box->w, (int)box->h, (int)max_border, border_color);
        }
    }

    if (box->type == TSFI_BOX_VIDEO) {
        if (g_active_video_frame) {
            draw_video_frame(pixels, width, height, (int)box->x, (int)box->y, (int)box->w, (int)box->h, g_active_video_frame, g_active_video_w, g_active_video_h);
        } else {
            // Draw black backdrop
            draw_rect(pixels, width, height, (int)box->x, (int)box->y, (int)box->w, (int)box->h, 0xFF1A1A1A); // Dark grey
            draw_border(pixels, width, height, (int)box->x, (int)box->y, (int)box->w, (int)box->h, 2, 0xFF00FF00); // Green border
            
            const char *src = box->node ? tsfi_node_get_attribute(box->node, "src") : "NULL";
            char video_overlay[256];
            snprintf(video_overlay, sizeof(video_overlay), "[VIDEO: %s]", src);
            draw_text(pixels, width, height, (int)box->x + 10, (int)box->y + 10, video_overlay, 0xFF00FF00, 1); // Green text
        }
    }

    // Paint text if it's a text node
    if (box->node && box->node->type == TSFI_NODE_TEXT) {
        uint32_t text_color = 0xFFFFFFFF; // default white text
        if (get_style_val_paint(style, "color", buf, sizeof(buf))) {
            text_color = parse_color(buf, text_color);
        } else {
            // Inherit parent text color if not explicitly defined
            const TsfiLayoutBox *p = box->parent;
            while (p) {
                const char *p_style = p->node ? tsfi_node_get_attribute(p->node, "style") : NULL;
                if (p_style && get_style_val_paint(p_style, "color", buf, sizeof(buf))) {
                    text_color = parse_color(buf, text_color);
                    break;
                }
                p = p->parent;
            }
        }
        int text_scale = 2;
        if (get_style_val_paint(style, "font-size", buf, sizeof(buf))) {
            int fs = atoi(buf);
            if (fs >= 24) text_scale = 3;
            else if (fs >= 16) text_scale = 2;
            else text_scale = 1;
        }

        // Align center support
        int text_x = (int)box->x;
        const TsfiLayoutBox *p = box->parent;
        bool align_center = false;
        while (p) {
            const char *p_style = p->node ? tsfi_node_get_attribute(p->node, "style") : NULL;
            if (p_style && get_style_val_paint(p_style, "text-align", buf, sizeof(buf))) {
                if (strcmp(buf, "center") == 0) {
                    align_center = true;
                }
                break;
            }
            p = p->parent;
        }
        if (align_center) {
            int text_w = strlen(box->node->text_content) * 8 * text_scale;
            if (box->parent && box->parent->w > text_w) {
                text_x = (int)box->parent->x + ((int)box->parent->w - text_w) / 2;
            }
        }

        draw_text(pixels, width, height, text_x, (int)box->y, box->node->text_content, text_color, text_scale);
    }

    // Paint children recursively
    TsfiLayoutBox *child = box->first_child;
    while (child) {
        tsfi_paint_tree(child, pixels, width, height);
        child = child->next_sibling;
    }
}

int tsfi_quantel_paintbox_airbrush(uint32_t *pixels, int w, int h, int cx, int cy, int radius, float pressure, uint32_t color) {
    if (!pixels || w <= 0 || h <= 0 || radius <= 0) return -1;
    
    uint8_t r_src = (color >> 16) & 0xFF;
    uint8_t g_src = (color >> 8) & 0xFF;
    uint8_t b_src = color & 0xFF;
    float sigma = radius / 3.0f;
    float denom = 2.0f * sigma * sigma;

    for (int y = cy - radius; y <= cy + radius; y++) {
        if (y < 0 || y >= h) continue;
        uint32_t *row = pixels + y * w;
        int dy = y - cy;
        for (int x = cx - radius; x <= cx + radius; x++) {
            if (x < 0 || x >= w) continue;
            int dx = x - cx;
            float dist_sq = (float)(dx * dx + dy * dy);
            if (dist_sq <= radius * radius) {
                float intensity = expf(-dist_sq / denom) * pressure;
                if (intensity > 1.0f) intensity = 1.0f;
                if (intensity < 0.0f) intensity = 0.0f;
                
                uint32_t dest = row[x];
                uint8_t r_dst = (dest >> 16) & 0xFF;
                uint8_t g_dst = (dest >> 8) & 0xFF;
                uint8_t b_dst = dest & 0xFF;

                uint8_t r_res = (uint8_t)(r_src * intensity + r_dst * (1.0f - intensity));
                uint8_t g_res = (uint8_t)(g_src * intensity + g_dst * (1.0f - intensity));
                uint8_t b_res = (uint8_t)(b_src * intensity + b_dst * (1.0f - intensity));

                row[x] = (0xFF000000) | (r_res << 16) | (g_res << 8) | b_res;
            }
        }
    }
    return 0;
}

int tsfi_quantel_paintbox_apply_stencil(uint32_t *pixels, int w, int h, const uint8_t *stencil_mask, uint32_t color) {
    if (!pixels || !stencil_mask || w <= 0 || h <= 0) return -1;
    
    uint8_t r_src = (color >> 16) & 0xFF;
    uint8_t g_src = (color >> 8) & 0xFF;
    uint8_t b_src = color & 0xFF;

    for (int i = 0; i < w * h; i++) {
        float alpha = stencil_mask[i] / 255.0f;
        if (alpha > 0.0f) {
            uint32_t dest = pixels[i];
            uint8_t r_dst = (dest >> 16) & 0xFF;
            uint8_t g_dst = (dest >> 8) & 0xFF;
            uint8_t b_dst = dest & 0xFF;

            uint8_t r_res = (uint8_t)(r_src * alpha + r_dst * (1.0f - alpha));
            uint8_t g_res = (uint8_t)(g_src * alpha + g_dst * (1.0f - alpha));
            uint8_t b_res = (uint8_t)(b_src * alpha + b_dst * (1.0f - alpha));

            pixels[i] = (0xFF000000) | (r_res << 16) | (g_res << 8) | b_res;
        }
    }
    return 0;
}

int tsfi_quantel_mirage_page_curl(const uint32_t *src_pixels, int src_w, int src_h, uint32_t *dst_pixels, int dst_w, int dst_h, float curl_radius, float roll_percent) {
    if (!src_pixels || !dst_pixels || src_w <= 0 || src_h <= 0 || dst_w <= 0 || dst_h <= 0) return -1;

    // Fill dest with transparent/black
    memset(dst_pixels, 0, dst_w * dst_h * sizeof(uint32_t));
    float roll_line = roll_percent * src_w;

    for (int y = 0; y < src_h; y++) {
        float v = (float)y / src_h;
        int dy = (int)(v * dst_h);
        if (dy < 0 || dy >= dst_h) continue;

        for (int x = 0; x < src_w; x++) {
            float u = (float)x / src_w;
            float px = (float)x;

            if (px < roll_line) {
                // Page curl warp along cylinders
                float dist_to_roll = roll_line - px;
                float theta = dist_to_roll / curl_radius;
                if (theta <= M_PI) {
                    // Warp position
                    float warp_x = roll_line - curl_radius * sinf(theta);
                    int dx = (int)(warp_x * dst_w / src_w);
                    if (dx >= 0 && dx < dst_w) {
                        dst_pixels[dy * dst_w + dx] = src_pixels[y * src_w + x];
                    }
                }
            } else {
                // Uncurled flat region
                int dx = (int)(u * dst_w);
                if (dx >= 0 && dx < dst_w) {
                    dst_pixels[dy * dst_w + dx] = src_pixels[y * src_w + x];
                }
            }
        }
    }
    return 0;
}

int tsfi_quantel_harry_chroma_key(const uint32_t *src_pixels, int w, int h, uint8_t *out_mask, uint32_t key_color, float tolerance) {
    if (!src_pixels || !out_mask || w <= 0 || h <= 0) return -1;

    uint8_t kr = (key_color >> 16) & 0xFF;
    uint8_t kg = (key_color >> 8) & 0xFF;
    uint8_t kb = key_color & 0xFF;

    for (int i = 0; i < w * h; i++) {
        uint32_t pix = src_pixels[i];
        uint8_t r = (pix >> 16) & 0xFF;
        uint8_t g = (pix >> 8) & 0xFF;
        uint8_t b = pix & 0xFF;

        float dr = r - kr;
        float dg = g - kg;
        float db = b - kb;
        float dist = sqrtf(dr * dr + dg * dg + db * db);

        if (dist < tolerance) {
            out_mask[i] = 0; // Keyed out (transparent)
        } else {
            out_mask[i] = 255; // Solid foreground
        }
    }
    return 0;
}

int tsfi_quantel_harry_blend(const uint32_t *fg_pixels, const uint32_t *bg_pixels, uint32_t *dst_pixels, int w, int h, const uint8_t *mask, const char *blend_mode) {
    if (!fg_pixels || !bg_pixels || !dst_pixels || w <= 0 || h <= 0) return -1;

    for (int i = 0; i < w * h; i++) {
        float alpha = mask ? (mask[i] / 255.0f) : 1.0f;
        uint32_t fg = fg_pixels[i];
        uint32_t bg = bg_pixels[i];

        uint8_t r_f = (fg >> 16) & 0xFF;
        uint8_t g_f = (fg >> 8) & 0xFF;
        uint8_t b_f = fg & 0xFF;

        uint8_t r_b = (bg >> 16) & 0xFF;
        uint8_t g_b = (bg >> 8) & 0xFF;
        uint8_t b_b = bg & 0xFF;

        uint8_t r_blended = r_f;
        uint8_t g_blended = g_f;
        uint8_t b_blended = b_f;

        if (strcmp(blend_mode, "multiply") == 0) {
            r_blended = (uint8_t)((r_f * r_b) / 255);
            g_blended = (uint8_t)((g_f * g_b) / 255);
            b_blended = (uint8_t)((b_f * b_b) / 255);
        } else if (strcmp(blend_mode, "screen") == 0) {
            r_blended = (uint8_t)(255 - ((255 - r_f) * (255 - r_b)) / 255);
            g_blended = (uint8_t)(255 - ((255 - g_f) * (255 - g_b)) / 255);
            b_blended = (uint8_t)(255 - ((255 - b_f) * (255 - b_b)) / 255);
        }

        // Apply alpha matte blend
        uint8_t r_res = (uint8_t)(r_blended * alpha + r_b * (1.0f - alpha));
        uint8_t g_res = (uint8_t)(g_blended * alpha + g_b * (1.0f - alpha));
        uint8_t b_res = (uint8_t)(b_blended * alpha + b_b * (1.0f - alpha));

        dst_pixels[i] = (0xFF000000) | (r_res << 16) | (g_res << 8) | b_res;
    }
    return 0;
}

int tsfi_quantel_paintbox_custom_brush(uint32_t *pixels, int w, int h, int cx, int cy, const uint8_t *brush_tex, int brush_w, int brush_h, float opacity, uint32_t color) {
    if (!pixels || !brush_tex || w <= 0 || h <= 0 || brush_w <= 0 || brush_h <= 0) return -1;

    uint8_t r_src = (color >> 16) & 0xFF;
    uint8_t g_src = (color >> 8) & 0xFF;
    uint8_t b_src = color & 0xFF;

    int half_bw = brush_w / 2;
    int half_bh = brush_h / 2;

    for (int by = 0; by < brush_h; by++) {
        int canvas_y = cy - half_bh + by;
        if (canvas_y < 0 || canvas_y >= h) continue;
        uint32_t *canvas_row = pixels + canvas_y * w;
        const uint8_t *brush_row = brush_tex + by * brush_w;

        for (int bx = 0; bx < brush_w; bx++) {
            int canvas_x = cx - half_bw + bx;
            if (canvas_x < 0 || canvas_x >= w) continue;

            float intensity = (brush_row[bx] / 255.0f) * opacity;
            if (intensity > 0.0f) {
                if (intensity > 1.0f) intensity = 1.0f;
                uint32_t dest = canvas_row[canvas_x];
                uint8_t r_dst = (dest >> 16) & 0xFF;
                uint8_t g_dst = (dest >> 8) & 0xFF;
                uint8_t b_dst = dest & 0xFF;

                uint8_t r_res = (uint8_t)(r_src * intensity + r_dst * (1.0f - intensity));
                uint8_t g_res = (uint8_t)(g_src * intensity + g_dst * (1.0f - intensity));
                uint8_t b_res = (uint8_t)(b_src * intensity + b_dst * (1.0f - intensity));

                canvas_row[canvas_x] = (0xFF000000) | (r_res << 16) | (g_res << 8) | b_res;
            }
        }
    }
    return 0;
}

int tsfi_quantel_mirage_sphere_wrap(const uint32_t *src_pixels, int src_w, int src_h, uint32_t *dst_pixels, int dst_w, int dst_h, float sphere_radius) {
    if (!src_pixels || !dst_pixels || src_w <= 0 || src_h <= 0 || dst_w <= 0 || dst_h <= 0 || sphere_radius <= 0.0f) return -1;

    memset(dst_pixels, 0, dst_w * dst_h * sizeof(uint32_t));
    float cx = dst_w / 2.0f;
    float cy = dst_h / 2.0f;

    for (int y = 0; y < dst_h; y++) {
        float dy = y - cy;
        for (int x = 0; x < dst_w; x++) {
            float dx = x - cx;
            float r = sqrtf(dx * dx + dy * dy);
            if (r < sphere_radius) {
                // Orthographic sphere mapping projection
                float z = sqrtf(sphere_radius * sphere_radius - r * r);
                float lon = atan2f(dx, z);
                float lat = asinf(dy / sphere_radius);

                float u = (lon + M_PI) / (2.0f * M_PI);
                float v = (lat + M_PI_2) / M_PI;

                int sx = (int)(u * src_w);
                int sy = (int)(v * src_h);

                if (sx >= 0 && sx < src_w && sy >= 0 && sy < src_h) {
                    dst_pixels[y * dst_w + x] = src_pixels[sy * src_w + sx];
                }
            }
        }
    }
    return 0;
}

int tsfi_quantel_mirage_flag_ripple(const uint32_t *src_pixels, int src_w, int src_h, uint32_t *dst_pixels, int dst_w, int dst_h, float amplitude, float frequency, float phase) {
    if (!src_pixels || !dst_pixels || src_w <= 0 || src_h <= 0 || dst_w <= 0 || dst_h <= 0) return -1;

    memset(dst_pixels, 0, dst_w * dst_h * sizeof(uint32_t));

    for (int y = 0; y < dst_h; y++) {
        float v = (float)y / dst_h;
        int sy = (int)(v * src_h);
        if (sy < 0 || sy >= src_h) continue;

        for (int x = 0; x < dst_w; x++) {
            float u = (float)x / dst_w;
            
            // Apply ripple displacement to src_x sampling index
            float ripple = amplitude * sinf(2.0f * M_PI * frequency * v + phase);
            int sx = (int)((u * src_w) + ripple);

            if (sx >= 0 && sx < src_w) {
                dst_pixels[y * dst_w + x] = src_pixels[sy * src_w + sx];
            }
        }
    }
    return 0;
}

int tsfi_quantel_harry_wipe(const uint32_t *src_a, const uint32_t *src_b, uint32_t *dst, int w, int h, float progress, const char *wipe_type) {
    if (!src_a || !src_b || !dst || w <= 0 || h <= 0) return -1;

    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            int idx = y * w + x;
            bool show_b = false;

            if (strcmp(wipe_type, "radial") == 0) {
                float dx = x - (w / 2.0f);
                float dy = y - (h / 2.0f);
                float angle = atan2f(dy, dx) + M_PI; // [0, 2*PI]
                float normalized_angle = angle / (2.0f * M_PI);
                show_b = (normalized_angle < progress);
            } else {
                // Default: horizontal linear wipe
                float px = (float)x / w;
                show_b = (px < progress);
            }

            dst[idx] = show_b ? src_b[idx] : src_a[idx];
        }
    }
    return 0;
}

int tsfi_quantel_harry_luma_key(const uint32_t *src_pixels, int w, int h, uint8_t *out_mask, uint8_t low_threshold, uint8_t high_threshold) {
    if (!src_pixels || !out_mask || w <= 0 || h <= 0) return -1;

    for (int i = 0; i < w * h; i++) {
        uint32_t pix = src_pixels[i];
        uint8_t r = (pix >> 16) & 0xFF;
        uint8_t g = (pix >> 8) & 0xFF;
        uint8_t b = pix & 0xFF;

        if (luma >= low_threshold && luma <= high_threshold) {
            out_mask[i] = 255; // Keep foreground details
        } else {
            out_mask[i] = 0; // Keyed out
        }
    }
    return 0;
}

int tsfi_quantel_harry_filter(const uint32_t *src, uint32_t *dst, int w, int h, const float kernel[9]) {
    if (!src || !dst || w <= 0 || h <= 0) return -1;

    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            float sum_r = 0.0f, sum_g = 0.0f, sum_b = 0.0f;
            for (int ky = -1; ky <= 1; ky++) {
                int py = y + ky;
                if (py < 0) py = 0;
                if (py >= h) py = h - 1;
                const uint32_t *row = src + py * w;

                for (int kx = -1; kx <= 1; kx++) {
                    int px = x + kx;
                    if (px < 0) px = 0;
                    if (px >= w) px = w - 1;

                    uint32_t pix = row[px];
                    float k_val = kernel[(ky + 1) * 3 + (kx + 1)];
                    sum_r += ((pix >> 16) & 0xFF) * k_val;
                    sum_g += ((pix >> 8) & 0xFF) * k_val;
                    sum_b += (pix & 0xFF) * k_val;
                }
            }

            int r = (int)sum_r;
            int g = (int)sum_g;
            int b = (int)sum_b;
            if (r < 0) r = 0; if (r > 255) r = 255;
            if (g < 0) g = 0; if (g > 255) g = 255;
            if (b < 0) b = 0; if (b > 255) b = 255;

            dst[y * w + x] = (0xFF000000) | (r << 16) | (g << 8) | b;
        }
    }
    return 0;
}

static void tsfi_rgb_to_hsl(uint8_t r, uint8_t g, uint8_t b, float *h, float *s, float *l) {
    float fr = r / 255.0f;
    float fg = g / 255.0f;
    float fb = b / 255.0f;
    float max_c = fr > fg ? (fr > fb ? fr : fb) : (fg > fb ? fg : fb);
    float min_c = fr < fg ? (fr < fb ? fr : fb) : (fg < fb ? fg : fb);
    *l = (max_c + min_c) / 2.0f;
    if (max_c == min_c) {
        *h = 0.0f;
        *s = 0.0f;
    } else {
        float d = max_c - min_c;
        *s = (*l > 0.5f) ? d / (2.0f - max_c - min_c) : d / (max_c + min_c);
        if (max_c == fr) {
            *h = (fg - fb) / d + (fg < fb ? 6.0f : 0.0f);
        } else if (max_c == fg) {
            *h = (fb - fr) / d + 2.0f;
        } else {
            *h = (fr - fg) / d + 4.0f;
        }
        *h /= 6.0f;
    }
}

static float tsfi_hue_to_rgb(float p, float q, float t) {
    if (t < 0.0f) t += 1.0f;
    if (t > 1.0f) t -= 1.0f;
    if (t < 1.0f/6.0f) return p + (q - p) * 6.0f * t;
    if (t < 1.0f/2.0f) return q;
    if (t < 2.0f/3.0f) return p + (q - p) * (2.0f/3.0f - t) * 6.0f;
    return p;
}

static void tsfi_hsl_to_rgb(float h, float s, float l, uint8_t *r, uint8_t *g, uint8_t *b) {
    if (s == 0.0f) {
        *r = *g = *b = (uint8_t)(l * 255.0f);
    } else {
        float q = l < 0.5f ? l * (1.0f + s) : l + s - l * s;
        float p = 2.0f * l - q;
        *r = (uint8_t)(tsfi_hue_to_rgb(p, q, h + 1.0f/3.0f) * 255.0f);
        *g = (uint8_t)(tsfi_hue_to_rgb(p, q, h) * 255.0f);
        *b = (uint8_t)(tsfi_hue_to_rgb(p, q, h - 1.0f/3.0f) * 255.0f);
    }
}

int tsfi_quantel_harry_color_adjust(const uint32_t *src, uint32_t *dst, int w, int h, float hue_shift, float sat_scale, uint32_t tint_color, float tint_amount) {
    if (!src || !dst || w <= 0 || h <= 0) return -1;

    uint8_t tr = (tint_color >> 16) & 0xFF;
    uint8_t tg = (tint_color >> 8) & 0xFF;
    uint8_t tb = tint_color & 0xFF;

    for (int i = 0; i < w * h; i++) {
        uint32_t pix = src[i];
        uint8_t r = (pix >> 16) & 0xFF;
        uint8_t g = (pix >> 8) & 0xFF;
        uint8_t b = pix & 0xFF;

        float hue, sat, luma;
        tsfi_rgb_to_hsl(r, g, b, &hue, &sat, &luma);

        // Adjust parameters
        hue += hue_shift;
        if (hue > 1.0f) hue -= 1.0f;
        if (hue < 0.0f) hue += 1.0f;
        sat *= sat_scale;
        if (sat > 1.0f) sat = 1.0f;
        if (sat < 0.0f) sat = 0.0f;

        uint8_t r_adj, g_adj, b_adj;
        tsfi_hsl_to_rgb(hue, sat, luma, &r_adj, &g_adj, &b_adj);

        // Apply tint
        uint8_t r_res = (uint8_t)(r_adj * (1.0f - tint_amount) + tr * tint_amount);
        uint8_t g_res = (uint8_t)(g_adj * (1.0f - tint_amount) + tg * tint_amount);
        uint8_t b_res = (uint8_t)(b_adj * (1.0f - tint_amount) + tb * tint_amount);

        dst[i] = (0xFF000000) | (r_res << 16) | (g_res << 8) | b_res;
    }
    return 0;
}

int tsfi_quantel_mirage_page_peel_transition(const uint32_t *src_a, const uint32_t *src_b, uint32_t *dst, int w, int h, float progress, float peel_radius) {
    if (!src_a || !src_b || !dst || w <= 0 || h <= 0) return -1;

    float roll_line = progress * w;

    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            int idx = y * w + x;
            float px = (float)x;

            if (px < roll_line) {
                // Curved/peeling page showing source A
                float dist_to_roll = roll_line - px;
                float theta = dist_to_roll / peel_radius;
                if (theta <= M_PI) {
                    float warp_x = roll_line - peel_radius * sinf(theta);
                    int sx = (int)warp_x;
                    if (sx >= 0 && sx < w) {
                        dst[idx] = src_a[y * w + sx];
                    } else {
                        dst[idx] = src_b[idx];
                    }
                } else {
                    // Underneath revealed layer B
                    dst[idx] = src_b[idx];
                }
            } else {
                // Unpeeled flat layer A
                dst[idx] = src_a[idx];
            }
        }
    }
    return 0;
}

int tsfi_quantel_paintbox_velocity_brush(uint32_t *pixels, int w, int h, int prev_x, int prev_y, int curr_x, int curr_y, float max_radius, float pressure, uint32_t color) {
    if (!pixels || w <= 0 || h <= 0) return -1;

    float dx = curr_x - prev_x;
    float dy = curr_y - prev_y;
    float dist = sqrtf(dx * dx + dy * dy);

    // Brush radius decreases with stroke speed (velocity dynamics)
    float speed_factor = dist > 1.0f ? (1.0f / (1.0f + 0.05f * dist)) : 1.0f;
    float active_radius = max_radius * speed_factor;
    if (active_radius < 1.0f) active_radius = 1.0f;

    // Interpolate stamps along the line segment
    int steps = (int)(dist + 1.0f);
    for (int i = 0; i <= steps; i++) {
        float t = (float)i / steps;
        int cx = (int)(prev_x + dx * t);
        int cy = (int)(prev_y + dy * t);
        tsfi_quantel_paintbox_airbrush(pixels, w, h, cx, cy, (int)active_radius, pressure, color);
    }
    return 0;
}

int tsfi_quantel_paintbox_wet_paint(uint32_t *pixels, int w, int h, int cx, int cy, int radius, float smudge_rate, uint32_t *brush_color_in_out) {
    if (!pixels || !brush_color_in_out || w <= 0 || h <= 0 || radius <= 0) return -1;

    double sum_r = 0, sum_g = 0, sum_b = 0;
    int count = 0;

    for (int y = cy - radius; y <= cy + radius; y++) {
        if (y < 0 || y >= h) continue;
        uint32_t *row = pixels + y * w;
        int dy = y - cy;
        for (int x = cx - radius; x <= cx + radius; x++) {
            if (x < 0 || x >= w) continue;
            int dx = x - cx;
            if (dx * dx + dy * dy <= radius * radius) {
                uint32_t pix = row[x];
                sum_r += (pix >> 16) & 0xFF;
                sum_g += (pix >> 8) & 0xFF;
                sum_b += pix & 0xFF;
                count++;
            }
        }
    }

    if (count > 0) {
        uint8_t avg_r = (uint8_t)(sum_r / count);
        uint8_t avg_g = (uint8_t)(sum_g / count);
        uint8_t avg_b = (uint8_t)(sum_b / count);

        uint32_t br_col = *brush_color_in_out;
        uint8_t br_r = (br_col >> 16) & 0xFF;
        uint8_t br_g = (br_col >> 8) & 0xFF;
        uint8_t br_b = br_col & 0xFF;

        uint8_t res_r = (uint8_t)(br_r * (1.0f - smudge_rate) + avg_r * smudge_rate);
        uint8_t res_g = (uint8_t)(br_g * (1.0f - smudge_rate) + avg_g * smudge_rate);
        uint8_t res_b = (uint8_t)(br_b * (1.0f - smudge_rate) + avg_b * smudge_rate);

        *brush_color_in_out = (0xFF000000) | (res_r << 16) | (res_g << 8) | res_b;
    }

    // Paint with smudge blended color
    return tsfi_quantel_paintbox_airbrush(pixels, w, h, cx, cy, radius, 1.0f, *brush_color_in_out);
}

int tsfi_quantel_mirage_perspective_rotate(const uint32_t *src, int src_w, int src_h, uint32_t *dst, int dst_w, int dst_h, float rot_x, float rot_y, float rot_z, float fov) {
    if (!src || !dst || src_w <= 0 || src_h <= 0 || dst_w <= 0 || dst_h <= 0) return -1;

    memset(dst, 0, dst_w * dst_h * sizeof(uint32_t));

    float cos_x = cosf(rot_x); float sin_x = sinf(rot_x);
    float cos_y = cosf(rot_y); float sin_y = sinf(rot_y);
    float cos_z = cosf(rot_z); float sin_z = sinf(rot_z);

    float cx_s = src_w / 2.0f;
    float cy_s = src_h / 2.0f;
    float cx_d = dst_w / 2.0f;
    float cy_d = dst_h / 2.0f;

    for (int y = 0; y < src_h; y++) {
        float sy = y - cy_s;
        for (int x = 0; x < src_w; x++) {
            float sx = x - cx_s;

            // 1. Z-axis rotation
            float x1 = sx * cos_z - sy * sin_z;
            float y1 = sx * sin_z + sy * cos_z;
            float z1 = 0;

            // 2. Y-axis rotation
            float x2 = x1 * cos_y + z1 * sin_y;
            float y2 = y1;
            float z2 = -x1 * sin_y + z1 * cos_y;

            // 3. X-axis rotation
            float x3 = x2;
            float y3 = y2 * cos_x - z2 * sin_x;
            float z3 = y2 * sin_x + z2 * cos_x;

            // Perspective scale
            float z_offset = z3 + fov;
            if (z_offset > 1.0f) {
                float scale = fov / z_offset;
                int dx = (int)(x3 * scale + cx_d);
                int dy = (int)(y3 * scale + cy_d);

                if (dx >= 0 && dx < dst_w && dy >= 0 && dy < dst_h) {
                    dst[dy * dst_w + dx] = src[y * src_w + x];
                }
            }
        }
    }
    return 0;
}

int tsfi_quantel_harry_bezier_animate(const uint32_t *fg, int fg_w, int fg_h, uint32_t *bg, int w, int h, float t, float p0_x, float p0_y, float p1_x, float p1_y, float p2_x, float p2_y) {
    if (!fg || !bg || w <= 0 || h <= 0 || fg_w <= 0 || fg_h <= 0) return -1;

    // Evaluate quadratic bezier position
    float omt = 1.0f - t;
    float bx = omt * omt * p0_x + 2.0f * omt * t * p1_x + t * t * p2_x;
    float by = omt * omt * p0_y + 2.0f * omt * t * p1_y + t * t * p2_y;

    int ox = (int)(bx - fg_w / 2.0f);
    int oy = (int)(by - fg_h / 2.0f);

    for (int y = 0; y < fg_h; y++) {
        int cy = oy + y;
        if (cy < 0 || cy >= h) continue;
        uint32_t *bg_row = bg + cy * w;
        const uint32_t *fg_row = fg + y * fg_w;

        for (int x = 0; x < fg_w; x++) {
            int cx = ox + x;
            if (cx < 0 || cx >= w) continue;

            uint32_t fg_pixel = fg_row[x];
            uint8_t alpha = (fg_pixel >> 24) & 0xFF;
            if (alpha > 0) {
                float a = alpha / 255.0f;
                uint8_t rf = (fg_pixel >> 16) & 0xFF;
                uint8_t gf = (fg_pixel >> 8) & 0xFF;
                uint8_t bf = fg_pixel & 0xFF;

                uint32_t bg_pixel = bg_row[cx];
                uint8_t rb = (bg_pixel >> 16) & 0xFF;
                uint8_t gb = (bg_pixel >> 8) & 0xFF;
                uint8_t bb = bg_pixel & 0xFF;

                uint8_t r_res = (uint8_t)(rf * a + rb * (1.0f - a));
                uint8_t g_res = (uint8_t)(gf * a + gb * (1.0f - a));
                uint8_t b_res = (uint8_t)(bf * a + bb * (1.0f - a));

                bg_row[cx] = (0xFF000000) | (r_res << 16) | (g_res << 8) | b_res;
            }
        }
    }
    return 0;
}

int tsfi_quantel_paintbox_tablet_brush(uint32_t *pixels, int w, int h, int cx, int cy, int max_radius, float pressure, float aspect_ratio, float angle, uint32_t color) {
    if (!pixels || w <= 0 || h <= 0 || max_radius <= 0) return -1;

    uint8_t r_src = (color >> 16) & 0xFF;
    uint8_t g_src = (color >> 8) & 0xFF;
    uint8_t b_src = color & 0xFF;

    float active_r = max_radius * pressure;
    if (active_r < 1.0f) active_r = 1.0f;
    float cos_a = cosf(angle);
    float sin_a = sinf(angle);

    int bounds = (int)(active_r + 1.0f);
    for (int y = cy - bounds; y <= cy + bounds; y++) {
        if (y < 0 || y >= h) continue;
        uint32_t *row = pixels + y * w;
        int dy = y - cy;

        for (int x = cx - bounds; x <= cx + bounds; x++) {
            if (x < 0 || x >= w) continue;
            int dx = x - cx;

            // Calligraphy rotational projection
            float rx = dx * cos_a + dy * sin_a;
            float ry = -dx * sin_a + dy * cos_a;

            float term_x = rx / active_r;
            float term_y = ry / (active_r * aspect_ratio);

            float ellipse = term_x * term_x + term_y * term_y;
            if (ellipse <= 1.0f) {
                // Modulate intensity based on pressure flow rate
                float intensity = (1.0f - ellipse) * pressure;
                if (intensity > 1.0f) intensity = 1.0f;
                if (intensity < 0.0f) intensity = 0.0f;

                uint32_t dest = row[x];
                uint8_t r_dst = (dest >> 16) & 0xFF;
                uint8_t g_dst = (dest >> 8) & 0xFF;
                uint8_t b_dst = dest & 0xFF;

                uint8_t r_res = (uint8_t)(r_src * intensity + r_dst * (1.0f - intensity));
                uint8_t g_res = (uint8_t)(g_src * intensity + g_dst * (1.0f - intensity));
                uint8_t b_res = (uint8_t)(b_src * intensity + b_dst * (1.0f - intensity));

                row[x] = (0xFF000000) | (r_res << 16) | (g_res << 8) | b_res;
            }
        }
    }
    return 0;
}

int tsfi_quantel_paintbox_stylus_shear(uint32_t *pixels, int w, int h, int cx, int cy, int radius, float tilt_x, float tilt_y, float rotation, uint32_t color) {
    if (!pixels || w <= 0 || h <= 0 || radius <= 0) return -1;

    uint8_t r_src = (color >> 16) & 0xFF;
    uint8_t g_src = (color >> 8) & 0xFF;
    uint8_t b_src = color & 0xFF;

    float cos_r = cosf(rotation);
    float sin_r = sinf(rotation);

    int bounds = radius * 2;
    for (int y = cy - bounds; y <= cy + bounds; y++) {
        if (y < 0 || y >= h) continue;
        uint32_t *row = pixels + y * w;
        int dy = y - cy;

        for (int x = cx - bounds; x <= cx + bounds; x++) {
            if (x < 0 || x >= w) continue;
            int dx = x - cx;

            // Apply tilt shearing
            float sx = dx + tilt_x * dy;
            float sy = dy + tilt_y * dx;

            // Apply stylus barrel rotation
            float rx = sx * cos_r + sy * sin_r;
            float ry = -sx * sin_r + sy * cos_r;

            float dist_sq = rx * rx + ry * ry;
            if (dist_sq <= radius * radius) {
                float intensity = 1.0f - sqrtf(dist_sq) / radius;
                uint32_t dest = row[x];
                uint8_t r_dst = (dest >> 16) & 0xFF;
                uint8_t g_dst = (dest >> 8) & 0xFF;
                uint8_t b_dst = dest & 0xFF;

                uint8_t r_res = (uint8_t)(r_src * intensity + r_dst * (1.0f - intensity));
                uint8_t g_res = (uint8_t)(g_src * intensity + g_dst * (1.0f - intensity));
                uint8_t b_res = (uint8_t)(b_src * intensity + b_dst * (1.0f - intensity));

                row[x] = (0xFF000000) | (r_res << 16) | (g_res << 8) | b_res;
            }
        }
    }
    return 0;
}

int tsfi_quantel_paintbox_filter_jitter(int raw_x, int raw_y, int prev_x, int prev_y, float alpha, int *smooth_x, int *smooth_y) {
    if (!smooth_x || !smooth_y) return -1;
    *smooth_x = (int)(alpha * raw_x + (1.0f - alpha) * prev_x);
    *smooth_y = (int)(alpha * raw_y + (1.0f - alpha) * prev_y);
    return 0;
}





