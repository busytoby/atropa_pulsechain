#ifndef TSFI_TEXT_RENDER_H
#define TSFI_TEXT_RENDER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// A simple 5x7 bitmap font for rendering text directly in C/C++ without external font libraries
static uint8_t font5x7[128][7] = {{0}};
static bool font_initialized = false;

static void init_font() {
    if (font_initialized) return;
    
    uint8_t a[7] = {0x70, 0x88, 0x88, 0xF8, 0x88, 0x88, 0x88}; memcpy(font5x7['A'], a, 7);
    uint8_t b[7] = {0xF0, 0x88, 0x88, 0xF0, 0x88, 0x88, 0xF0}; memcpy(font5x7['B'], b, 7);
    uint8_t c[7] = {0x70, 0x88, 0x80, 0x80, 0x80, 0x88, 0x70}; memcpy(font5x7['C'], c, 7);
    uint8_t d[7] = {0xF0, 0x88, 0x88, 0x88, 0x88, 0x88, 0xF0}; memcpy(font5x7['D'], d, 7);
    uint8_t e[7] = {0xF8, 0x80, 0x80, 0xF0, 0x80, 0x80, 0xF8}; memcpy(font5x7['E'], e, 7);
    uint8_t g[7] = {0x78, 0x80, 0x80, 0xB8, 0x88, 0x88, 0x78}; memcpy(font5x7['G'], g, 7);
    uint8_t i[7] = {0xF8, 0x20, 0x20, 0x20, 0x20, 0x20, 0xF8}; memcpy(font5x7['I'], i, 7);
    uint8_t n[7] = {0x88, 0xC8, 0xA8, 0x98, 0x88, 0x88, 0x88}; memcpy(font5x7['N'], n, 7);
    uint8_t s[7] = {0x70, 0x88, 0x80, 0x70, 0x08, 0x88, 0x70}; memcpy(font5x7['S'], s, 7);
    uint8_t t[7] = {0xF8, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20}; memcpy(font5x7['T'], t, 7);
    uint8_t o[7] = {0x70, 0x88, 0x88, 0x88, 0x88, 0x88, 0x70}; memcpy(font5x7['O'], o, 7);
    uint8_t u[7] = {0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x70}; memcpy(font5x7['U'], u, 7);
    uint8_t h[7] = {0x88, 0x88, 0x88, 0xF8, 0x88, 0x88, 0x88}; memcpy(font5x7['H'], h, 7);
    uint8_t r[7] = {0x88, 0x88, 0x88, 0xF8, 0x88, 0x88, 0x88}; memcpy(font5x7['R'], r, 7);
    uint8_t k[7] = {0x88, 0x90, 0xA0, 0xC0, 0xA0, 0x90, 0x88}; memcpy(font5x7['K'], k, 7);
    uint8_t m[7] = {0x88, 0xD8, 0xA8, 0x88, 0x88, 0x88, 0x88}; memcpy(font5x7['M'], m, 7);
    uint8_t w[7] = {0x88, 0x88, 0x88, 0xA8, 0xA8, 0xD8, 0x88}; memcpy(font5x7['W'], w, 7);
    uint8_t p[7] = {0xF0, 0x88, 0x88, 0xF0, 0x80, 0x80, 0x80}; memcpy(font5x7['P'], p, 7);
    uint8_t y[7] = {0x88, 0x88, 0x88, 0x70, 0x20, 0x20, 0x20}; memcpy(font5x7['Y'], y, 7);
    uint8_t l[7] = {0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0xF8}; memcpy(font5x7['L'], l, 7);
    uint8_t v[7] = {0x88, 0x88, 0x88, 0x88, 0x88, 0x50, 0x20}; memcpy(font5x7['V'], v, 7);
    uint8_t colon[7] = {0x00, 0x30, 0x30, 0x00, 0x30, 0x30, 0x00}; memcpy(font5x7[':'], colon, 7);
    
    font_initialized = true;
}

static void draw_char(uint8_t* img, int W, int H, char c, int start_x, int start_y, uint8_t cr, uint8_t cg, uint8_t cb) {
    if (!font_initialized) init_font();
    if (c < 0 || c > 127) return;
    for (int row = 0; row < 7; row++) {
        uint8_t bits = font5x7[(int)c][row];
        for (int col = 0; col < 5; col++) {
            if (bits & (0x80 >> col)) {
                int px = start_x + col * 2; // Scale x2
                int py = start_y + row * 2;
                for(int dy=0; dy<2; dy++) {
                    for(int dx=0; dx<2; dx++) {
                        if (px+dx >= 0 && px+dx < W && py+dy >= 0 && py+dy < H) {
                            int idx = ((py+dy) * W + (px+dx)) * 3;
                            img[idx] = cr;
                            img[idx+1] = cg;
                            img[idx+2] = cb;
                        }
                    }
                }
            }
        }
    }
}

static void draw_matrix_rain(uint8_t* img, int W, int H, int f_idx, const char* thought) {
    int thought_len = strlen(thought);
    if (thought_len == 0) return;
    
    for (int column = 0; column < W; column += 14) {
        int offset = (f_idx * 5 + column * 13) % H;
        int char_idx_base = column % thought_len;
        
        for (int row = 0; row < H; row += 14) {
            int y_pos = (row + offset) % H;
            if (y_pos > H - 50) continue;
            
            int char_idx = (char_idx_base + (row / 14)) % thought_len;
            char c = thought[char_idx];
            
            // Fading green intensity
            float percent = 1.0f - ((float)y_pos / H);
            uint8_t intensity = (uint8_t)(255.0f * percent);
            
            draw_char(img, W, H, c, column, y_pos, 0, intensity, 0);
        }
    }
}

#endif
