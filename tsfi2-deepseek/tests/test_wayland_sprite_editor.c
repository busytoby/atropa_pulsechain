#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <signal.h>
#include <pthread.h>
#include "vulkan/vulkan_main.h"
#include "vulkan/vulkan_render.h"
#include "tsfi_raw.h"
#include "tsfi_staging.h"
#include "tsfi_font_vectors.h"
#include "tsfi_font_ai.h"

void tsfi_input_set_key_hook(void *hook_func);

#define GRID_SIZE 16

// AB4H Pixel Layout
typedef struct {
    uint16_t r, g, b, a;
} AB4HPixel;

static inline uint16_t float_to_half(float f) {
    union { float f; uint32_t u; } u = { f };
    uint32_t sign = (u.u >> 16) & 0x8000;
    int32_t exponent = ((u.u >> 23) & 0xff) - 127;
    uint32_t mantissa = u.u & 0x7fffff;
    if (exponent <= -15) return sign;
    if (exponent >= 16) return sign | 0x7c00;
    return sign | ((exponent + 15) << 10) | (mantissa >> 13);
}

static inline float half_to_float(uint16_t h) {
    uint32_t sign = (h & 0x8000) << 16;
    uint32_t exponent = (h & 0x7c00) >> 10;
    uint32_t mantissa = (h & 0x03ff) << 13;
    if (exponent == 0) {
        if (mantissa == 0) return 0.0f;
        while (!(mantissa & 0x00800000)) { mantissa <<= 1; exponent--; }
        exponent++; exponent += 127 - 15;
    } else if (exponent == 31) exponent = 255;
    else exponent += 127 - 15;
    union { uint32_t u; float f; } u = { sign | (exponent << 23) | mantissa };
    return u.f;
}

static inline AB4HPixel make_ab4h_pixel(float r, float g, float b, float a) {
    AB4HPixel p;
    p.r = float_to_half(r);
    p.g = float_to_half(g);
    p.b = float_to_half(b);
    p.a = float_to_half(a);
    return p;
}

void draw_rect_ab4h(AB4HPixel *pixels, int w, int h, int x, int y, int rw, int rh, AB4HPixel color) {
    float tr = half_to_float(color.r), tg = half_to_float(color.g), tb = half_to_float(color.b), ta = half_to_float(color.a);
    for (int cy = y; cy < y + rh; cy++) {
        for (int cx = x; cx < x + rw; cx++) {
            if (cx >= 0 && cx < w && cy >= 0 && cy < h) {
                if (ta >= 1.0f) {
                    pixels[cy * w + cx] = color;
                } else {
                    AB4HPixel curr = pixels[cy * w + cx];
                    float cr = half_to_float(curr.r), cg = half_to_float(curr.g), cb = half_to_float(curr.b), ca = half_to_float(curr.a);
                    float nr = cr * (1.0f - ta) + tr * ta;
                    float ng = cg * (1.0f - ta) + tg * ta;
                    float nb = cb * (1.0f - ta) + tb * ta;
                    float na = ca * (1.0f - ta) + ta;
                    pixels[cy * w + cx] = make_ab4h_pixel(nr, ng, nb, na);
                }
            }
        }
    }
}

void draw_line_aa(AB4HPixel *pixels, int w, int h, float x0, float y0, float x1, float y1, AB4HPixel color, float thickness) {
    int min_x = fmaxf(0.0f, fminf(x0, x1) - thickness - 1.0f);
    int max_x = fminf(w - 1, fmaxf(x0, x1) + thickness + 1.0f);
    int min_y = fmaxf(0.0f, fminf(y0, y1) - thickness - 1.0f);
    int max_y = fminf(h - 1, fmaxf(y0, y1) + thickness + 1.0f);
    float dx = x1 - x0, dy = y1 - y0;
    float len2 = dx * dx + dy * dy;
    float tr = half_to_float(color.r), tg = half_to_float(color.g), tb = half_to_float(color.b), ta = half_to_float(color.a);
    for (int y = min_y; y <= max_y; y++) {
        for (int x = min_x; x <= max_x; x++) {
            float px = (float)x + 0.5f, py = (float)y + 0.5f;
            float t = 0.0f;
            if (len2 > 0.0f) {
                t = ((px - x0) * dx + (py - y0) * dy) / len2;
                t = fmaxf(0.0f, fminf(1.0f, t));
            }
            float proj_x = x0 + t * dx, proj_y = y0 + t * dy;
            float dist = sqrtf((px - proj_x)*(px - proj_x) + (py - proj_y)*(py - proj_y));
            if (dist <= thickness) {
                AB4HPixel curr = pixels[y * w + x];
                float cr = half_to_float(curr.r), cg = half_to_float(curr.g), cb = half_to_float(curr.b), ca = half_to_float(curr.a);
                float nr = cr * (1.0f - ta) + tr * ta;
                float ng = cg * (1.0f - ta) + tg * ta;
                float nb = cb * (1.0f - ta) + tb * ta;
                float na = ca * (1.0f - ta) + ta;
                pixels[y * w + x] = make_ab4h_pixel(nr, ng, nb, na);
            } else if (dist < thickness + 1.0f) {
                float alpha = ta * (1.0f - (dist - thickness));
                AB4HPixel curr = pixels[y * w + x];
                float cr = half_to_float(curr.r), cg = half_to_float(curr.g), cb = half_to_float(curr.b), ca = half_to_float(curr.a);
                float nr = cr * (1.0f - alpha) + tr * alpha;
                float ng = cg * (1.0f - alpha) + tg * alpha;
                float nb = cb * (1.0f - alpha) + tb * alpha;
                float na = ca * (1.0f - alpha) + alpha;
                pixels[y * w + x] = make_ab4h_pixel(nr, ng, nb, na);
            }
        }
    }
}

void draw_char_ab4h(AB4HPixel *pixels, int w, int h, char c, int x, int y, AB4HPixel color) {
    uint32_t temp_buf[32 * 32] = {0};
    StagingBuffer temp_sb = {
        .magic = TSFI_STAGING_MAGIC,
        .data = temp_buf,
        .width = 32,
        .height = 32,
        .stride = 32 * 4,
        .size = 32 * 32 * 4
    };
    draw_debug_codepoint(&temp_sb, 0, 0, (uint32_t)c, 0xFFFFFFFF);
    float tr = half_to_float(color.r), tg = half_to_float(color.g), tb = half_to_float(color.b), ta = half_to_float(color.a);
    for (int j = 0; j < 32; j++) {
        int py = y - 4 + j;
        if (py < 0 || py >= h) continue;
        for (int i = 0; i < 32; i++) {
            int px = x + i;
            if (px < 0 || px >= w) continue;
            uint32_t p = temp_buf[j * 32 + i];
            float alpha = ((p >> 24) & 0xFF) / 255.0f * ta;
            if (alpha > 0.01f) {
                AB4HPixel curr = pixels[py * w + px];
                float cr = half_to_float(curr.r), cg = half_to_float(curr.g), cb = half_to_float(curr.b), ca = half_to_float(curr.a);
                float nr = cr * (1.0f - alpha) + tr * alpha;
                float ng = cg * (1.0f - alpha) + tg * alpha;
                float nb = cb * (1.0f - alpha) + tb * alpha;
                float na = ca * (1.0f - alpha) + alpha;
                pixels[py * w + px] = make_ab4h_pixel(nr, ng, nb, na);
            }
        }
    }
}

void draw_string_ab4h(AB4HPixel *pixels, int w, int h, const char *str, int x, int y, AB4HPixel color) {
    AB4HPixel default_color = color;
    AB4HPixel current_color = color;
    while (*str) {
        if (*str == '^' && *(str + 1) != '\0') {
            char code = *(str + 1);
            if (code == 'B' || code == 'b') {
                float r = half_to_float(default_color.r) * 1.5f;
                float g = half_to_float(default_color.g) * 1.5f;
                float b = half_to_float(default_color.b) * 1.5f;
                float a = half_to_float(default_color.a);
                current_color = make_ab4h_pixel(r, g, b, a);
                str += 2;
                continue;
            } else if (code == 'I' || code == 'i') {
                current_color = make_ab4h_pixel(1.0f, 0.05f, 0.6f, 1.0f); // neon_pink
                str += 2;
                continue;
            } else if (code == 'U' || code == 'u') {
                current_color = make_ab4h_pixel(0.0f, 0.9f, 1.0f, 1.0f); // neon_cyan
                str += 2;
                continue;
            } else if (code == 'Y' || code == 'y') {
                current_color = make_ab4h_pixel(0.9f, 0.9f, 0.0f, 1.0f); // neon_yellow
                str += 2;
                continue;
            } else if (code == 'G' || code == 'g') {
                current_color = make_ab4h_pixel(0.0f, 1.0f, 0.3f, 1.0f); // neon_green
                str += 2;
                continue;
            } else if (code == 'C' || code == 'c') {
                current_color = default_color;
                str += 2;
                continue;
            }
        }
        draw_char_ab4h(pixels, w, h, *str, x, y, current_color);
        x += 16;
        str++;
    }
}

// Global UI State
static volatile int exit_requested = 0;
static int active_palette_idx = 1; // 1: Cyan, 2: Pink, 3: Yellow, 4: Green, 0: Erase/Transparent
static uint8_t sprite_canvas[GRID_SIZE][GRID_SIZE] = {0};
static char info_message[256] = "^B^USPRITE TUNER^C ^IReady to Draw!^C";

static const AB4HPixel colors[] = {
    {0, 0, 0, 0},                                      // 0: Transparent
    {0x13c0, 0x1f90, 0x1fc0, 0x3c00},                  // 1: Cyan (neon_cyan)
    {0x1fc0, 0x0ab0, 0x18cc, 0x3c00},                  // 2: Pink (neon_pink)
    {0x1b9a, 0x1b9a, 0, 0x3c00},                       // 3: Yellow (neon_yellow)
    {0, 0x1fc0, 0x0d9a, 0x3c00}                        // 4: Green (neon_green)
};

static void editor_key_hook(void *data, uint32_t serial, uint32_t time, uint32_t key, uint32_t state) {
    (void)data; (void)serial; (void)time;
    bool pressed = (state != 0);
    if (!pressed) return;

    if (key == 1) { // ESC
        exit_requested = 1;
    } else if (key == 2) { // '1' Key
        active_palette_idx = 1;
        strcpy(info_message, "[TOOL] Color: ^B^UCYAN^C Selected.");
    } else if (key == 3) { // '2' Key
        active_palette_idx = 2;
        strcpy(info_message, "[TOOL] Color: ^B^UPINK^C Selected.");
    } else if (key == 4) { // '3' Key
        active_palette_idx = 3;
        strcpy(info_message, "[TOOL] Color: ^B^UYELLOW^C Selected.");
    } else if (key == 5) { // '4' Key
        active_palette_idx = 4;
        strcpy(info_message, "[TOOL] Color: ^B^UGREEN^C Selected.");
    } else if (key == 11) { // '0' Key
        active_palette_idx = 0;
        strcpy(info_message, "[TOOL] ^B^UERASER^C Mode Selected.");
    } else if (key == 46) { // 'C' Key -> Clear
        memset(sprite_canvas, 0, sizeof(sprite_canvas));
        strcpy(info_message, "[ACTION] Canvas ^B^UCLEARED^C.");
    } else if (key == 18) { // 'E' Key -> Export
        printf("\n--- EXPORTED SPRITE ARRAYS (16x16) ---\n");
        printf("/* Assembly Byte representation (Row-by-Row 16-bit word binary) */\n");
        printf("Shield columns array:\n");
        for (int r = 0; r < GRID_SIZE; r++) {
            uint16_t row_val = 0;
            for (int c = 0; c < GRID_SIZE; c++) {
                if (sprite_canvas[r][c] != 0) {
                    row_val |= (1 << (15 - c));
                }
            }
            printf("  Row %02d: 0x%04x  (%%", r, row_val);
            for (int bit = 15; bit >= 0; bit--) {
                printf("%d", (row_val >> bit) & 1);
            }
            printf(")\n");
        }
        
        FILE *out = fopen("sprite_output.txt", "w");
        if (out) {
            fprintf(out, "/* Unified 16-Bit AUB Sprite Data */\n");
            for (int r = 0; r < GRID_SIZE; r++) {
                uint16_t row_val = 0;
                for (int c = 0; c < GRID_SIZE; c++) {
                    if (sprite_canvas[r][c] != 0) row_val |= (1 << (15 - c));
                }
                fprintf(out, "0x%04x,\n", row_val);
            }
            fclose(out);
            strcpy(info_message, "[ACTION] Exported bytes to stdout and ^B^Usprite_output.txt^C");
        }
    } else if (key == 38) { // 'L' Key -> Load Preset
        // Simple default Yar pattern
        memset(sprite_canvas, 0, sizeof(sprite_canvas));
        for (int i = 0; i < 8; i++) {
            sprite_canvas[i][2 + i] = 1;
            sprite_canvas[i][13 - i] = 1;
            sprite_canvas[8 + i][9 - i] = 2;
            sprite_canvas[8 + i][6 + i] = 2;
        }
        strcpy(info_message, "[PRESET] Preloaded ^B^UYar Wingflyer^C Template.");
    } else if (key == 37) { // 'K' Key -> Cycle Karateka Stance Presets
        static int karateka_preset_idx = 0;
        memset(sprite_canvas, 0, sizeof(sprite_canvas));
        if (karateka_preset_idx == 0) {
            // Running Stance (Stance 0)
            for (int r = 1; r <= 3; r++)
                for (int c = 6; c <= 8; c++) sprite_canvas[r][c] = 2; // Pink skin/hair
            for (int r = 4; r <= 9; r++)
                for (int c = 5; c <= 9; c++) sprite_canvas[r][c] = 1; // Cyan Gi
            sprite_canvas[10][4] = 3; sprite_canvas[11][3] = 3; sprite_canvas[12][2] = 3;
            sprite_canvas[10][8] = 3; sprite_canvas[11][9] = 3; sprite_canvas[12][10] = 3;
            strcpy(info_message, "[KARATEKA] Loaded ^B^URunning Stance^C (Stance 0) Template.");
        } else if (karateka_preset_idx == 1) {
            // Fighting/Neutral Stance (Stance 1)
            for (int r = 2; r <= 4; r++)
                for (int c = 6; c <= 8; c++) sprite_canvas[r][c] = 2;
            for (int r = 5; r <= 10; r++)
                for (int c = 5; c <= 9; c++) sprite_canvas[r][c] = 1;
            sprite_canvas[5][10] = 2; sprite_canvas[5][11] = 2;
            sprite_canvas[6][4] = 2;
            for (int r = 11; r <= 13; r++) {
                sprite_canvas[r][4] = 3;
                sprite_canvas[r][10] = 3;
            }
            strcpy(info_message, "[KARATEKA] Loaded ^B^UFighting Stance^C (Stance 1) Template.");
        } else if (karateka_preset_idx == 2) {
            // Ducking Stance (Stance 2)
            for (int r = 5; r <= 7; r++)
                for (int c = 6; c <= 8; c++) sprite_canvas[r][c] = 2;
            for (int r = 8; r <= 11; r++)
                for (int c = 4; c <= 10; c++) sprite_canvas[r][c] = 1;
            for (int c = 3; c <= 11; c++) {
                sprite_canvas[12][c] = 3;
                sprite_canvas[13][c] = 3;
            }
            strcpy(info_message, "[KARATEKA] Loaded ^B^UDucking Stance^C (Stance 2 - Low) Template.");
        } else if (karateka_preset_idx == 3) {
            // Jumping Stance (Stance 3)
            for (int r = 0; r <= 2; r++)
                for (int c = 6; c <= 8; c++) sprite_canvas[r][c] = 2;
            for (int r = 3; r <= 8; r++)
                for (int c = 5; c <= 9; c++) sprite_canvas[r][c] = 1;
            sprite_canvas[9][4] = 3; sprite_canvas[9][10] = 3;
            sprite_canvas[10][5] = 3; sprite_canvas[10][9] = 3;
            strcpy(info_message, "[KARATEKA] Loaded ^B^UJumping Stance^C (Stance 3 - High) Template.");
        }
        karateka_preset_idx = (karateka_preset_idx + 1) % 4;
    }
}

int main() {
    printf("[EDITOR] Booting Wayland Vulkan Sprite Tuner...\n");
    signal(SIGPIPE, SIG_IGN);
    setenv("TSFI_AB4H", "1", 1);

    VulkanSystem *s = create_vulkan_system();
    if (!s) return 1;
    lau_unseal_object(s);
    s->disable_ui_overlay = true;

    tsfi_input_set_key_hook((void*)editor_key_hook);

    int frame_counter = 0;
    AB4HPixel bg_grid = make_ab4h_pixel(0.12f, 0.12f, 0.18f, 1.0f);
    AB4HPixel grid_border = make_ab4h_pixel(0.3f, 0.3f, 0.45f, 1.0f);
    AB4HPixel panel_bg = make_ab4h_pixel(0.06f, 0.06f, 0.1f, 1.0f);

    while (!exit_requested) {
        int W = s->width;
        int H = s->height;

        AB4HPixel *pixels = (AB4HPixel *)s->paint_buffer->data;
        if (!pixels) {
            tsfi_raw_usleep(10000);
            continue;
        }

        // 1. Clear background to dark space
        memset(pixels, 0, W * H * sizeof(AB4HPixel));

        float scale_x = (float)W / 800.0f;
        float scale_y = (float)(H - 40) / 540.0f;

        // Logical Mouse coords mapping
        int mx = (int)(s->mouse_x / scale_x);
        int my = (int)(s->mouse_y / scale_y);

        // 2. Draw layout splits: Left Canvas Panel, Right Settings Panel
        int canvas_size = 480;
        int start_x = 30;
        int start_y = 20;

        // Draw Left Canvas background panel
        draw_rect_ab4h(pixels, W, H, (int)(start_x * scale_x), (int)(start_y * scale_y), (int)(canvas_size * scale_x), (int)(canvas_size * scale_y), panel_bg);
        
        // Grid calculations
        int cell_size = canvas_size / GRID_SIZE;
        int hover_r = -1, hover_c = -1;

        if (mx >= start_x && mx < start_x + canvas_size && my >= start_y && my < start_y + canvas_size) {
            hover_c = (mx - start_x) / cell_size;
            hover_r = (my - start_y) / cell_size;
            if (s->mouse_down && hover_r >= 0 && hover_r < GRID_SIZE && hover_c >= 0 && hover_c < GRID_SIZE) {
                sprite_canvas[hover_r][hover_c] = active_palette_idx;
            }
        }

        // Render Grid Cells
        for (int r = 0; r < GRID_SIZE; r++) {
            for (int c = 0; c < GRID_SIZE; c++) {
                int cx = start_x + c * cell_size;
                int cy = start_y + r * cell_size;
                AB4HPixel cell_color = bg_grid;
                
                uint8_t val = sprite_canvas[r][c];
                if (val != 0) {
                    cell_color = colors[val];
                } else if (r == hover_r && c == hover_c) {
                    // Hover overlay
                    cell_color = make_ab4h_pixel(0.4f, 0.4f, 0.6f, 0.5f);
                }

                // Draw cell interior
                draw_rect_ab4h(pixels, W, H, (int)(cx * scale_x) + 1, (int)(cy * scale_y) + 1, 
                               (int)(cell_size * scale_x) - 2, (int)(cell_size * scale_y) - 2, cell_color);

                // Draw cell grid lines
                if (c > 0) {
                    draw_line_aa(pixels, W, H, cx * scale_x, cy * scale_y, cx * scale_x, (cy + cell_size) * scale_y, grid_border, 1.0f);
                }
                if (r > 0) {
                    draw_line_aa(pixels, W, H, cx * scale_x, cy * scale_y, (cx + cell_size) * scale_x, cy * scale_y, grid_border, 1.0f);
                }
            }
        }

        // Right Panel: Tuning controls & Live scale previews
        int right_start_x = start_x + canvas_size + 40;
        int preview_y = 50;

        // Draw Swatches
        draw_string_ab4h(pixels, W, H, "^B^UPALETTE SELECTION^C", (int)(right_start_x * scale_x), (int)(50 * scale_y), colors[3]);
        
        char swatch_label[64];
        for (int p = 1; p <= 4; p++) {
            int sw_x = right_start_x + (p - 1) * 60;
            int sw_y = 90;
            AB4HPixel border = (active_palette_idx == p) ? colors[3] : make_ab4h_pixel(0.2f, 0.2f, 0.2f, 1.0f);
            draw_rect_ab4h(pixels, W, H, (int)(sw_x * scale_x), (int)(sw_y * scale_y), (int)(45 * scale_x), (int)(40 * scale_y), border);
            draw_rect_ab4h(pixels, W, H, (int)(sw_x * scale_x) + 3, (int)(sw_y * scale_y) + 3, (int)(39 * scale_x), (int)(34 * scale_y), colors[p]);
            sprintf(swatch_label, " %d", p);
            draw_string_ab4h(pixels, W, H, swatch_label, (int)((sw_x + 10) * scale_x), (int)((sw_y + 10) * scale_y), make_ab4h_pixel(1.0f, 1.0f, 1.0f, 1.0f));
        }

        // Draw Live Previews at 1x, 2x, 4x, 8x
        preview_y = 160;
        draw_string_ab4h(pixels, W, H, "^B^ULIVE RETRO PREVIEWS^C", (int)(right_start_x * scale_x), (int)(preview_y * scale_y), colors[1]);

        int p1_x = right_start_x;
        int p1_y = preview_y + 40;
        
        // 1x scale
        for (int r = 0; r < GRID_SIZE; r++) {
            for (int c = 0; c < GRID_SIZE; c++) {
                uint8_t val = sprite_canvas[r][c];
                if (val != 0) {
                    draw_rect_ab4h(pixels, W, H, (int)((p1_x + c) * scale_x), (int)((p1_y + r) * scale_y), (int)(1 * scale_x) + 1, (int)(1 * scale_y) + 1, colors[val]);
                }
            }
        }
        draw_string_ab4h(pixels, W, H, "1x", (int)((p1_x + 24) * scale_x), (int)((p1_y + 4) * scale_y), colors[3]);

        // 2x scale
        int p2_x = right_start_x + 80;
        int p2_y = p1_y;
        for (int r = 0; r < GRID_SIZE; r++) {
            for (int c = 0; c < GRID_SIZE; c++) {
                uint8_t val = sprite_canvas[r][c];
                if (val != 0) {
                    draw_rect_ab4h(pixels, W, H, (int)((p2_x + c * 2) * scale_x), (int)((p2_y + r * 2) * scale_y), (int)(2 * scale_x) + 1, (int)(2 * scale_y) + 1, colors[val]);
                }
            }
        }
        draw_string_ab4h(pixels, W, H, "2x", (int)((p2_x + 40) * scale_x), (int)((p2_y + 8) * scale_y), colors[3]);

        // 4x scale
        int p4_x = right_start_x;
        int p4_y = p1_y + 60;
        for (int r = 0; r < GRID_SIZE; r++) {
            for (int c = 0; c < GRID_SIZE; c++) {
                uint8_t val = sprite_canvas[r][c];
                if (val != 0) {
                    draw_rect_ab4h(pixels, W, H, (int)((p4_x + c * 4) * scale_x), (int)((p4_y + r * 4) * scale_y), (int)(4 * scale_x) + 1, (int)(4 * scale_y) + 1, colors[val]);
                }
            }
        }
        draw_string_ab4h(pixels, W, H, "4x", (int)((p4_x + 80) * scale_x), (int)((p4_y + 24) * scale_y), colors[3]);

        // Draw Interactive Guide and status text
        draw_string_ab4h(pixels, W, H, "^B^UCONTROLS GUIDE^C", (int)(right_start_x * scale_x), (int)(300 * scale_y), colors[3]);
        draw_string_ab4h(pixels, W, H, "[0-4]   Change Color Swatch", (int)(right_start_x * scale_x), (int)(330 * scale_y), colors[1]);
        draw_string_ab4h(pixels, W, H, "[L-CLK] Draw Pixel on Canvas", (int)(right_start_x * scale_x), (int)(360 * scale_y), colors[1]);
        draw_string_ab4h(pixels, W, H, "[C]     Clear Canvas Grid", (int)(right_start_x * scale_x), (int)(390 * scale_y), colors[1]);
        draw_string_ab4h(pixels, W, H, "[L]     Load Preset Template", (int)(right_start_x * scale_x), (int)(420 * scale_y), colors[1]);
        draw_string_ab4h(pixels, W, H, "[K]     Load Karateka Stances", (int)(right_start_x * scale_x), (int)(450 * scale_y), colors[1]);
        draw_string_ab4h(pixels, W, H, "[E]     Export Bytecode Array", (int)(right_start_x * scale_x), (int)(480 * scale_y), colors[1]);

        // Draw Footer Status Bar
        draw_rect_ab4h(pixels, W, H, 0, H - 40, W, 40, make_ab4h_pixel(0.01f, 0.01f, 0.06f, 1.0f));
        draw_line_aa(pixels, W, H, 0.0f, (float)(H - 40), (float)W, (float)(H - 40), colors[1], 1.5f);
        draw_string_ab4h(pixels, W, H, info_message, 20, H - 30, colors[3]);

        draw_frame(s);
        frame_counter++;
        tsfi_raw_usleep(16000);
    }

    printf("[EDITOR] Teardown Vulkan context.\n");
    lau_unseal_object(s);
    destroy_vulkan_system(s);
    
    extern _Atomic int g_teardown_in_progress;
    g_teardown_in_progress = 1;
    extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_free_all_active(void);
    lau_free_all_active();
    return 0;
}
