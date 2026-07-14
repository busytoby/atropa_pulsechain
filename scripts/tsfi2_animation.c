#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "tsfi2_animation.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Canvas Memory Management
tsfi2_canvas_t *tsfi2_canvas_create(uint32_t width, uint32_t height) {
    tsfi2_canvas_t *canvas = malloc(sizeof(tsfi2_canvas_t));
    if (!canvas) return NULL;
    canvas->width = width;
    canvas->height = height;
    canvas->data = malloc(width * height * 3);
    if (!canvas->data) {
        free(canvas);
        return NULL;
    }
    return canvas;
}

void tsfi2_canvas_clear(tsfi2_canvas_t *canvas, uint8_t r, uint8_t g, uint8_t b) {
    if (!canvas || !canvas->data) return;
    for (uint32_t i = 0; i < canvas->width * canvas->height; i++) {
        canvas->data[i * 3 + 0] = r;
        canvas->data[i * 3 + 1] = g;
        canvas->data[i * 3 + 2] = b;
    }
}

void tsfi2_canvas_free(tsfi2_canvas_t *canvas) {
    if (canvas) {
        if (canvas->data) free(canvas->data);
        free(canvas);
    }
}

// Drawing Rasterization Functions
void tsfi2_draw_pixel(tsfi2_canvas_t *canvas, int x, int y, uint8_t r, uint8_t g, uint8_t b, float alpha) {
    if (!canvas || x < 0 || x >= (int)canvas->width || y < 0 || y >= (int)canvas->height) return;
    uint32_t idx = (y * canvas->width + x) * 3;
    
    // Alpha blending with background
    canvas->data[idx + 0] = (uint8_t)(canvas->data[idx + 0] * (1.0f - alpha) + r * alpha);
    canvas->data[idx + 1] = (uint8_t)(canvas->data[idx + 1] * (1.0f - alpha) + g * alpha);
    canvas->data[idx + 2] = (uint8_t)(canvas->data[idx + 2] * (1.0f - alpha) + b * alpha);
}

// Xiaolin Wu's Anti-Aliased Line Drawing
static inline float fpart(float x) { return x - floorf(x); }
static inline float rfpart(float x) { return 1.0f - fpart(x); }

void tsfi2_draw_line_aa(tsfi2_canvas_t *canvas, float x0, float y0, float x1, float y1, uint8_t r, uint8_t g, uint8_t b, float alpha) {
    int steep = fabsf(y1 - y0) > fabsf(x1 - x0);
    if (steep) {
        float temp = x0; x0 = y0; y0 = temp;
        temp = x1; x1 = y1; y1 = temp;
    }
    if (x0 > x1) {
        float temp = x0; x0 = x1; x1 = temp;
        temp = y0; y0 = y1; y1 = temp;
    }
    
    float dx = x1 - x0;
    float dy = y1 - y0;
    
    if (dx == 0.0f) {
        for (int y = (int)y0; y <= (int)y1; y++) {
            if (steep) tsfi2_draw_pixel(canvas, y, (int)x0, r, g, b, alpha);
            else tsfi2_draw_pixel(canvas, (int)x0, y, r, g, b, alpha);
        }
        return;
    }
    
    float gradient = dy / dx;
    
    // First endpoint
    int xend = (int)(x0 + 0.5f);
    float yend = y0 + gradient * (xend - x0);
    float xgap = rfpart(x0 + 0.5f);
    int xpxl1 = xend;
    int ypxl1 = (int)yend;
    
    if (steep) {
        tsfi2_draw_pixel(canvas, ypxl1, xpxl1, r, g, b, rfpart(yend) * xgap * alpha);
        tsfi2_draw_pixel(canvas, ypxl1 + 1, xpxl1, r, g, b, fpart(yend) * xgap * alpha);
    } else {
        tsfi2_draw_pixel(canvas, xpxl1, ypxl1, r, g, b, rfpart(yend) * xgap * alpha);
        tsfi2_draw_pixel(canvas, xpxl1, ypxl1 + 1, r, g, b, fpart(yend) * xgap * alpha);
    }
    float intery = yend + gradient;
    
    // Second endpoint
    xend = (int)(x1 + 0.5f);
    yend = y1 + gradient * (xend - x1);
    xgap = fpart(x1 + 0.5f);
    int xpxl2 = xend;
    int ypxl2 = (int)yend;
    
    if (steep) {
        tsfi2_draw_pixel(canvas, ypxl2, xpxl2, r, g, b, rfpart(yend) * xgap * alpha);
        tsfi2_draw_pixel(canvas, ypxl2 + 1, xpxl2, r, g, b, fpart(yend) * xgap * alpha);
    } else {
        tsfi2_draw_pixel(canvas, xpxl2, ypxl2, r, g, b, rfpart(yend) * xgap * alpha);
        tsfi2_draw_pixel(canvas, xpxl2, ypxl2 + 1, r, g, b, fpart(yend) * xgap * alpha);
    }
    
    // Main loop
    if (steep) {
        for (int x = xpxl1 + 1; x < xpxl2; x++) {
            tsfi2_draw_pixel(canvas, (int)intery, x, r, g, b, rfpart(intery) * alpha);
            tsfi2_draw_pixel(canvas, (int)intery + 1, x, r, g, b, fpart(intery) * alpha);
            intery += gradient;
        }
    } else {
        for (int x = xpxl1 + 1; x < xpxl2; x++) {
            tsfi2_draw_pixel(canvas, x, (int)intery, r, g, b, rfpart(intery) * alpha);
            tsfi2_draw_pixel(canvas, x, (int)intery + 1, r, g, b, fpart(intery) * alpha);
            intery += gradient;
        }
    }
}

void tsfi2_draw_line_glow(tsfi2_canvas_t *canvas, float x0, float y0, float x1, float y1, uint8_t r, uint8_t g, uint8_t b) {
    // Halo bloom pass
    tsfi2_draw_line_aa(canvas, x0 - 1.0f, y0, x1 - 1.0f, y1, r, g, b, 0.2f);
    tsfi2_draw_line_aa(canvas, x0 + 1.0f, y0, x1 + 1.0f, y1, r, g, b, 0.2f);
    tsfi2_draw_line_aa(canvas, x0, y0 - 1.0f, x1, y1 - 1.0f, r, g, b, 0.2f);
    tsfi2_draw_line_aa(canvas, x0, y0 + 1.0f, x1, y1 + 1.0f, r, g, b, 0.2f);
    // Core pass
    tsfi2_draw_line_aa(canvas, x0, y0, x1, y1, r, g, b, 1.0f);
}

void tsfi2_draw_circle(tsfi2_canvas_t *canvas, int cx, int cy, int radius, uint8_t r, uint8_t g, uint8_t b, int fill) {
    if (fill) {
        for (int dy = -radius; dy <= radius; dy++) {
            for (int dx = -radius; dx <= radius; dx++) {
                if (dx * dx + dy * dy <= radius * radius) {
                    tsfi2_draw_pixel(canvas, cx + dx, cy + dy, r, g, b, 1.0f);
                }
            }
        }
    } else {
        int x = radius;
        int y = 0;
        int err = 0;
        while (x >= y) {
            tsfi2_draw_pixel(canvas, cx + x, cy + y, r, g, b, 1.0f);
            tsfi2_draw_pixel(canvas, cx + y, cy + x, r, g, b, 1.0f);
            tsfi2_draw_pixel(canvas, cx - y, cy + x, r, g, b, 1.0f);
            tsfi2_draw_pixel(canvas, cx - x, cy + y, r, g, b, 1.0f);
            tsfi2_draw_pixel(canvas, cx - x, cy - y, r, g, b, 1.0f);
            tsfi2_draw_pixel(canvas, cx - y, cy - x, r, g, b, 1.0f);
            tsfi2_draw_pixel(canvas, cx + y, cy - x, r, g, b, 1.0f);
            tsfi2_draw_pixel(canvas, cx + x, cy - y, r, g, b, 1.0f);
            
            y++;
            if (err <= 0) {
                err += 2 * y + 1;
            } else {
                x--;
                err += 2 * (y - x) + 1;
            }
        }
    }
}

// Easing Functions
float tsfi2_ease_linear(float t) { return t; }
float tsfi2_ease_slow_in_out(float t) {
    return 0.5f * (1.0f - cosf(t * (float)M_PI));
}
float tsfi2_ease_elastic(float t) {
    if (t == 0.0f || t == 1.0f) return t;
    float p = 0.3f;
    return powf(2.0f, -10.0f * t) * sinf((t - p / 4.0f) * (2.0f * (float)M_PI) / p) + 1.0f;
}

// 3D/4D Projection
void tsfi2_rotate_4d(tsfi2_vec4_t *v, float theta, float phi, tsfi2_vec4_t *out) {
    // Rotate in X-W plane
    float x_rot = v->x * cosf(theta) - v->w * sinf(theta);
    float w_rot = v->x * sinf(theta) + v->w * cosf(theta);
    // Rotate in Y-Z plane
    float y_rot = v->y * cosf(phi) - v->z * sinf(phi);
    float z_rot = v->y * sinf(phi) + v->z * cosf(phi);
    
    out->x = x_rot;
    out->y = y_rot;
    out->z = z_rot;
    out->w = w_rot;
}

void tsfi2_project_4d_to_3d(const tsfi2_vec4_t *v, tsfi2_vec3_t *out) {
    float factor = 1.0f / (2.5f - v->w * 0.4f);
    out->x = v->x * factor;
    out->y = v->y * factor;
    out->z = v->z * factor;
}

void tsfi2_project_3d_to_2d(const tsfi2_vec3_t *v, float angle, float scale, uint32_t width, uint32_t height, int *sx, int *sy) {
    // Yaw rotation
    float x_rot = v->x * cosf(angle) - v->z * sinf(angle);
    *sx = (int)(width / 2.0f + x_rot * scale);
    *sy = (int)(height / 2.0f + v->y * scale);
}

static const uint16_t C_FONT[128] = {
    ['A'] = 0x2bdd, ['B'] = 0x6bad, ['C'] = 0x7927, ['D'] = 0x6dad, ['E'] = 0x79e7,
    ['F'] = 0x79a4, ['G'] = 0x796f, ['H'] = 0x5fdd, ['I'] = 0x7497, ['J'] = 0x126a,
    ['K'] = 0x5edd, ['L'] = 0x4927, ['M'] = 0x5f6d, ['N'] = 0x5f6d, ['O'] = 0x7b6f,
    ['P'] = 0x7be4, ['Q'] = 0x7b79, ['R'] = 0x7bfd, ['S'] = 0x79cf, ['T'] = 0x7492,
    ['U'] = 0x5b6f, ['V'] = 0x5b52, ['W'] = 0x5b7d, ['X'] = 0x5aad, ['Y'] = 0x5a92,
    ['Z'] = 0x72a7, ['0'] = 0x7b6f, ['1'] = 0x2492, ['2'] = 0x73cf, ['3'] = 0x713f,
    ['4'] = 0x5f21, ['5'] = 0x7cf7, ['6'] = 0x7ddf, ['7'] = 0x7148, ['8'] = 0x7ddf,
    ['9'] = 0x7d9f, [' '] = 0x0000, [':'] = 0x00a0, ['+'] = 0x02e8, ['-'] = 0x00e0,
    ['>'] = 0x4454, ['='] = 0x0e38, ['('] = 0x2922, [')'] = 0x2222
};

void tsfi2_draw_char(tsfi2_canvas_t *canvas, char c, int px, int py, int scale, uint8_t r, uint8_t g, uint8_t b) {
    if (c >= 'a' && c <= 'z') c -= 32;
    if (c < 0 || c >= 128) return;
    uint16_t val = C_FONT[(int)c];
    for (int row = 0; row < 5; row++) {
        for (int col = 0; col < 3; col++) {
            if ((val >> (14 - (row * 3 + col))) & 1) {
                for (int dy = 0; dy < scale; dy++) {
                    for (int dx = 0; dx < scale; dx++) {
                        tsfi2_draw_pixel(canvas, px + col * scale + dx, py + row * scale + dy, r, g, b, 1.0f);
                    }
                }
            }
        }
    }
}

void tsfi2_draw_string(tsfi2_canvas_t *canvas, const char *text, int x, int y, int scale, uint8_t r, uint8_t g, uint8_t b) {
    int i = 0;
    while (text[i] != '\0') {
        tsfi2_draw_char(canvas, text[i], x + i * 5 * scale, y, scale, r, g, b);
        i++;
    }
}

