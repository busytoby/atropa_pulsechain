#define _DEFAULT_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <time.h>
#include <math.h>
#include <signal.h>
#include <linux/input-event-codes.h>

#include "lau_memory.h"
#include "vulkan/vulkan_render.h" 
#include "vulkan/vulkan_logic.h"
#include "vulkan/vulkan_main.h"
#include "vulkan/vulkan_input.h"
#include "tsfi_logic.h"
#include "tsfi_io.h"
#include "window_inc/tsfi_input.h"
#include "tsfi_paint.h"
#include "tsfi_svdag.h"

static volatile bool g_force_quit = false;
static volatile bool g_dirty = true;

typedef enum {
    TOOL_AIRBRUSH,
    TOOL_CALLIGRAPHY,
    TOOL_SMUDGE,
    TOOL_SPRAY_CAN,
    TOOL_CLONE,
    TOOL_FLOOD_FILL,
    TOOL_GRADIENT,
    TOOL_PAGE_PEEL,
    TOOL_MAX
} PaintTool;

static PaintTool g_active_tool = TOOL_AIRBRUSH;
static uint32_t g_brush_color = 0xFF00FF00; // Bright green
static float g_brush_pressure = 0.8f;
static int g_brush_radius = 15;

static uint32_t *g_canvas = NULL;
static uint32_t *g_temp_buffer = NULL;
static uint8_t *g_paper_texture = NULL;

static TSFiHelmholtzSVDAG *g_dag_flower = NULL;
static TSFiHelmholtzSVDAG *g_dag_bear = NULL;
static uint32_t *g_3d_viewport = NULL;
static float *g_3d_depth = NULL;

// Rendering options
static float g_rotation_t = 0.0f;
static bool g_auto_rotate = true;
static float g_water_tank_energy = 500.0f;
static uint32_t g_water_tank_depth = 5;
static float g_intent_w_user = 0.5f;
static float g_intent_w_deepseek = 0.3f;
static float g_intent_w_gemini = 0.2f;

static int g_prev_mouse_x = -1;
static int g_prev_mouse_y = -1;

static void handle_sigint(int sig) {
    (void)sig;
    g_force_quit = true;
}

// Simple font bitmap for drawing text overlays
static const uint8_t font_5x7[95][7] = {
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // space
    {0x04, 0x04, 0x04, 0x04, 0x00, 0x00, 0x04}, // !
    {0x09, 0x09, 0x00, 0x00, 0x00, 0x00, 0x00}, // "
    {0x0a, 0x0a, 0x1f, 0x0a, 0x1f, 0x0a, 0x0a}, // #
    {0x04, 0x0f, 0x14, 0x0e, 0x05, 0x1e, 0x04}, // $
    {0x18, 0x19, 0x02, 0x04, 0x08, 0x13, 0x03}, // %
    {0x0c, 0x12, 0x14, 0x08, 0x15, 0x12, 0x0d}, // &
    {0x0c, 0x04, 0x08, 0x00, 0x00, 0x00, 0x00}, // '
    {0x02, 0x04, 0x08, 0x08, 0x08, 0x04, 0x02}, // (
    {0x08, 0x04, 0x02, 0x02, 0x02, 0x04, 0x08}, // )
    {0x04, 0x15, 0x0e, 0x04, 0x0e, 0x15, 0x04}, // *
    {0x00, 0x04, 0x04, 0x1f, 0x04, 0x04, 0x00}, // +
    {0x00, 0x00, 0x00, 0x00, 0x0c, 0x04, 0x08}, // ,
    {0x00, 0x00, 0x00, 0x1f, 0x00, 0x00, 0x00}, // -
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04}, // .
    {0x00, 0x01, 0x02, 0x04, 0x08, 0x10, 0x00}, // /
    {0x0e, 0x11, 0x13, 0x15, 0x19, 0x11, 0x0e}, // 0
    {0x04, 0x0c, 0x04, 0x04, 0x04, 0x04, 0x0e}, // 1
    {0x0e, 0x11, 0x01, 0x02, 0x04, 0x08, 0x1f}, // 2
    {0x1f, 0x02, 0x04, 0x02, 0x01, 0x11, 0x0e}, // 3
    {0x02, 0x06, 0x0a, 0x12, 0x1f, 0x02, 0x02}, // 4
    {0x1f, 0x10, 0x1e, 0x01, 0x01, 0x11, 0x0e}, // 5
    {0x06, 0x08, 0x10, 0x1e, 0x11, 0x11, 0x0e}, // 6
    {0x1f, 0x01, 0x02, 0x04, 0x08, 0x08, 0x08}, // 7
    {0x0e, 0x11, 0x11, 0x0e, 0x11, 0x11, 0x0e}, // 8
    {0x0e, 0x11, 0x11, 0x0f, 0x01, 0x02, 0x0c}, // 9
    {0x00, 0x04, 0x00, 0x00, 0x00, 0x04, 0x00}, // :
    {0x00, 0x04, 0x00, 0x00, 0x0c, 0x04, 0x08}, // ;
    {0x02, 0x04, 0x08, 0x10, 0x08, 0x04, 0x02}, // <
    {0x00, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0x00}, // =
    {0x08, 0x04, 0x02, 0x01, 0x02, 0x04, 0x08}, // >
    {0x0e, 0x11, 0x01, 0x02, 0x04, 0x00, 0x04}, // ?
    {0x0e, 0x11, 0x17, 0x15, 0x15, 0x10, 0x0e}, // @
    {0x04, 0x0a, 0x11, 0x11, 0x1f, 0x11, 0x11}, // A
    {0x1e, 0x11, 0x11, 0x1e, 0x11, 0x11, 0x1e}, // B
    {0x0e, 0x11, 0x10, 0x10, 0x10, 0x11, 0x0e}, // C
    {0x1c, 0x12, 0x11, 0x11, 0x11, 0x12, 0x1c}, // D
    {0x1f, 0x10, 0x10, 0x1e, 0x10, 0x10, 0x1f}, // E
    {0x1f, 0x10, 0x10, 0x1e, 0x10, 0x10, 0x10}, // F
    {0x0e, 0x11, 0x10, 0x17, 0x11, 0x11, 0x0e}, // G
    {0x11, 0x11, 0x11, 0x1f, 0x11, 0x11, 0x11}, // H
    {0x0e, 0x04, 0x04, 0x04, 0x04, 0x04, 0x0e}, // I
    {0x1f, 0x02, 0x02, 0x02, 0x02, 0x12, 0x0c}, // J
    {0x11, 0x12, 0x14, 0x18, 0x14, 0x12, 0x11}, // K
    {0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x1f}, // L
    {0x11, 0x1b, 0x15, 0x15, 0x11, 0x11, 0x11}, // M
    {0x11, 0x11, 0x19, 0x15, 0x13, 0x11, 0x11}, // N
    {0x0e, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0e}, // O
    {0x1e, 0x11, 0x11, 0x1e, 0x10, 0x10, 0x10}, // P
    {0x0e, 0x11, 0x11, 0x11, 0x15, 0x12, 0x0d}, // Q
    {0x1e, 0x11, 0x11, 0x1e, 0x14, 0x12, 0x11}, // R
    {0x0f, 0x10, 0x10, 0x0e, 0x01, 0x01, 0x1e}, // S
    {0x1f, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04}, // T
    {0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0e}, // U
    {0x11, 0x11, 0x11, 0x11, 0x11, 0x0a, 0x04}, // V
    {0x11, 0x11, 0x11, 0x15, 0x15, 0x1b, 0x11}, // W
    {0x11, 0x11, 0x0a, 0x04, 0x0a, 0x11, 0x11}, // X
    {0x11, 0x11, 0x0a, 0x04, 0x04, 0x04, 0x04}, // Y
    {0x1f, 0x01, 0x02, 0x04, 0x08, 0x10, 0x1f}, // Z
    {0x0e, 0x08, 0x08, 0x08, 0x08, 0x08, 0x0e}, // [
    {0x00, 0x10, 0x08, 0x04, 0x02, 0x01, 0x00}, // backslash
    {0x0e, 0x02, 0x02, 0x02, 0x02, 0x02, 0x0e}, // ]
    {0x04, 0x0a, 0x11, 0x00, 0x00, 0x00, 0x00}, // ^
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1f}, // _
    {0x08, 0x04, 0x02, 0x00, 0x00, 0x00, 0x00}, // `
    {0x00, 0x00, 0x0e, 0x01, 0x0f, 0x11, 0x0f}, // a
    {0x10, 0x10, 0x16, 0x19, 0x11, 0x11, 0x1e}, // b
    {0x00, 0x00, 0x0e, 0x10, 0x10, 0x11, 0x0e}, // c
    {0x01, 0x01, 0x0d, 0x13, 0x11, 0x11, 0x0f}, // d
    {0x00, 0x00, 0x0e, 0x11, 0x1f, 0x10, 0x0f}, // e
    {0x06, 0x09, 0x08, 0x1c, 0x08, 0x08, 0x08}, // f
    {0x00, 0x00, 0x0f, 0x11, 0x11, 0x0f, 0x01}, // g
    {0x10, 0x10, 0x16, 0x19, 0x11, 0x11, 0x11}, // h
    {0x04, 0x00, 0x0c, 0x04, 0x04, 0x04, 0x0e}, // i
    {0x02, 0x00, 0x06, 0x02, 0x02, 0x12, 0x0c}, // j
    {0x10, 0x10, 0x12, 0x14, 0x18, 0x14, 0x12}, // k
    {0x0c, 0x04, 0x04, 0x04, 0x04, 0x04, 0x0e}, // l
    {0x00, 0x00, 0x1a, 0x15, 0x15, 0x11, 0x11}, // m
    {0x00, 0x00, 0x16, 0x19, 0x11, 0x11, 0x11}, // n
    {0x00, 0x00, 0x0e, 0x11, 0x11, 0x11, 0x0e}, // o
    {0x00, 0x00, 0x1e, 0x11, 0x11, 0x1e, 0x10}, // p
    {0x00, 0x00, 0x0d, 0x13, 0x11, 0x0f, 0x01}, // q
    {0x00, 0x00, 0x16, 0x19, 0x10, 0x10, 0x10}, // r
    {0x00, 0x00, 0x0f, 0x10, 0x0e, 0x01, 0x1e}, // s
    {0x08, 0x08, 0x1c, 0x08, 0x08, 0x09, 0x06}, // t
    {0x00, 0x00, 0x11, 0x11, 0x11, 0x13, 0x0d}, // u
    {0x00, 0x00, 0x11, 0x11, 0x11, 0x0a, 0x04}, // v
    {0x00, 0x00, 0x11, 0x15, 0x15, 0x1b, 0x11}, // w
    {0x00, 0x00, 0x11, 0x0a, 0x04, 0x0a, 0x11}, // x
    {0x00, 0x00, 0x11, 0x11, 0x0f, 0x01, 0x0e}, // y
    {0x00, 0x00, 0x1f, 0x02, 0x04, 0x08, 0x1f}  // z
};

static void draw_text_overlay(uint32_t *px, int w, int h, int rx, int ry, const char *text, uint32_t color) {
    while (*text) {
        char c = *text;
        if (c >= 32 && c <= 126) {
            int idx = c - 32;
            for (int row = 0; row < 7; row++) {
                int py = ry + row;
                if (py < 0 || py >= h) continue;
                uint8_t bits = font_5x7[idx][row];
                for (int col = 0; col < 5; col++) {
                    int px_x = rx + col;
                    if (px_x < 0 || px_x >= w) continue;
                    if (bits & (1 << (4 - col))) {
                        px[py * w + px_x] = color;
                    }
                }
            }
        }
        rx += 6;
        text++;
    }
}

static void draw_line_on_canvas(uint32_t *canvas, int w, int h, int x0, int y0, int x1, int y1, uint32_t color) {
    int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int err = dx + dy, e2;
    while (1) {
        if (x0 >= 0 && x0 < w && y0 >= 0 && y0 < h) {
            canvas[y0 * w + x0] = color;
        }
        if (x0 == x1 && y0 == y1) break;
        e2 = 2 * err;
        if (e2 >= dy) { err += dy; x0 += sx; }
        if (e2 <= dx) { err += dx; y0 += sy; }
    }
}

static void draw_circle_on_canvas(uint32_t *canvas, int w, int h, int cx, int cy, int radius, uint32_t color) {
    for (int dy = -radius; dy <= radius; dy++) {
        for (int dx = -radius; dx <= radius; dx++) {
            int x = cx + dx;
            int y = cy + dy;
            if (x >= 0 && x < w && y >= 0 && y < h) {
                if (dx*dx + dy*dy <= radius*radius) {
                    canvas[y * w + x] = color;
                }
            }
        }
    }
}

static void load_mainframe_dag_template(void) {
    if (!g_canvas) return;
    // Clear canvas
    memset(g_canvas, 0, 1024 * 768 * sizeof(uint32_t));

    // Draw connecting edges
    // CPU to steps
    draw_line_on_canvas(g_canvas, 1024, 768, 512, 120, 300, 250, 0xFF888888);
    draw_line_on_canvas(g_canvas, 1024, 768, 512, 120, 700, 250, 0xFF888888);
    
    // Steps to Storage and libraries
    draw_line_on_canvas(g_canvas, 1024, 768, 300, 250, 150, 420, 0xFF888888);
    draw_line_on_canvas(g_canvas, 1024, 768, 300, 250, 500, 420, 0xFF888888);
    draw_line_on_canvas(g_canvas, 1024, 768, 700, 250, 500, 420, 0xFF888888);
    draw_line_on_canvas(g_canvas, 1024, 768, 700, 250, 850, 420, 0xFF888888);

    // Outputs to Terminus
    draw_line_on_canvas(g_canvas, 1024, 768, 500, 420, 512, 600, 0xFF888888);
    draw_line_on_canvas(g_canvas, 1024, 768, 850, 420, 512, 600, 0xFF888888);

    // Draw Nodes
    draw_circle_on_canvas(g_canvas, 1024, 768, 512, 120, 25, 0xFFFF0000); // CPU Red
    draw_circle_on_canvas(g_canvas, 1024, 768, 300, 250, 22, 0xFF00FF00); // COBOL Green
    draw_circle_on_canvas(g_canvas, 1024, 768, 700, 250, 22, 0xFF00FF00); // LINKER Green
    draw_circle_on_canvas(g_canvas, 1024, 768, 150, 420, 20, 0xFF0000FF); // PUNCH CARD Blue
    draw_circle_on_canvas(g_canvas, 1024, 768, 500, 420, 20, 0xFF0000FF); // OBJ LIB Blue
    draw_circle_on_canvas(g_canvas, 1024, 768, 850, 420, 20, 0xFFFFFF00); // RAMAC DASD Yellow
    draw_circle_on_canvas(g_canvas, 1024, 768, 512, 600, 25, 0xFFFF00FF); // PRINTER Magenta

    // Draw labels on canvas using drawing pixels
    draw_text_overlay(g_canvas, 1024, 768, 500, 70, "SYS/370 CPU", 0xFFFFFFFF);
    draw_text_overlay(g_canvas, 1024, 768, 280, 210, "COBOL STEP", 0xFFFFFFFF);
    draw_text_overlay(g_canvas, 1024, 768, 680, 210, "LKED STEP", 0xFFFFFFFF);
    draw_text_overlay(g_canvas, 1024, 768, 110, 380, "SOURCE CARDS", 0xFFFFFFFF);
    draw_text_overlay(g_canvas, 1024, 768, 470, 380, "LOADLIB", 0xFFFFFFFF);
    draw_text_overlay(g_canvas, 1024, 768, 820, 380, "RAMAC DASD", 0xFFFFFFFF);
    draw_text_overlay(g_canvas, 1024, 768, 480, 640, "SYSOUT PRINTER", 0xFFFFFFFF);

    // Auto-voxelize to SVDAG
    if (g_dag_flower && g_canvas) {
        tsfi_svdag_voxelize_image(g_dag_flower, g_canvas, 1024, 768, 0.1f);
        tsfi_svdag_voxelize_image(g_dag_bear, g_canvas, 1024, 768, 0.1f);
        tsfi_svdag_execute_414d_water_tank(g_dag_flower, g_water_tank_energy, g_water_tank_depth);
        tsfi_svdag_execute_414d_water_tank(g_dag_bear, g_water_tank_energy, g_water_tank_depth);
        tsfi_proteo_screen_candidates_avx512(g_dag_flower, 4);
    }
}

static void draw_toolbox(uint32_t *px, int w, int h) {
    // Draw sidebar panel background
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < 200; x++) {
            px[y * w + x] = 0xFF2A2A2A; // Slate gray background for toolbox
        }
        px[y * w + 200] = 0xFF444444; // Border line
    }

    draw_text_overlay(px, w, h, 10, 15, "TSFi PAINTBOX TOOLS", 0xFFFFFFFF);
    draw_text_overlay(px, w, h, 10, 28, "-------------------", 0xFF888888);

    const char *tool_names[] = {
        "[1] Airbrush",
        "[2] Calligraphy",
        "[3] Wet Smudge",
        "[4] Spray Can",
        "[5] Clone Tool",
        "[6] Flood Fill",
        "[7] Gradient",
        "[8] Page Peel"
    };

    for (int i = 0; i < (int)TOOL_MAX; i++) {
        uint32_t color = ((int)g_active_tool == i) ? 0xFF00FF00 : 0xFFCCCCCC;
        draw_text_overlay(px, w, h, 15, 45 + i * 22, tool_names[i], color);
    }

    draw_text_overlay(px, w, h, 10, 230, "SVDAG MANIPULATIONS", 0xFFFFFFFF);
    draw_text_overlay(px, w, h, 10, 243, "-------------------", 0xFF888888);

    draw_text_overlay(px, w, h, 15, 260, "[V] Voxelize Canvas", 0xFFFFD700);
    draw_text_overlay(px, w, h, 15, 275, "[M] Load Mainframe DAG", 0xFFFFD700);
    draw_text_overlay(px, w, h, 15, 290, "[R] Toggle Auto-Rotate", g_auto_rotate ? 0xFF00FF00 : 0xFFCCCCCC);
    
    char val_buf[64];
    snprintf(val_buf, sizeof(val_buf), "[E/D] Energy Pour: %.1f", g_water_tank_energy);
    draw_text_overlay(px, w, h, 15, 310, val_buf, 0xFFCCCCCC);

    snprintf(val_buf, sizeof(val_buf), "[T/G] Target Depth: %d", g_water_tank_depth);
    draw_text_overlay(px, w, h, 15, 330, val_buf, 0xFFCCCCCC);

    draw_text_overlay(px, w, h, 10, 360, "Intent Norm Weights", 0xFFFFFFFF);
    snprintf(val_buf, sizeof(val_buf), "User (U/I): %.2f", g_intent_w_user);
    draw_text_overlay(px, w, h, 15, 380, val_buf, 0xFFCCCCCC);
    snprintf(val_buf, sizeof(val_buf), "DeepSeek (J/K): %.2f", g_intent_w_deepseek);
    draw_text_overlay(px, w, h, 15, 400, val_buf, 0xFFCCCCCC);
    snprintf(val_buf, sizeof(val_buf), "Gemini (O/P): %.2f", g_intent_w_gemini);
    draw_text_overlay(px, w, h, 15, 420, val_buf, 0xFFCCCCCC);

    // Display brush size and pressure status
    snprintf(val_buf, sizeof(val_buf), "Brush Radius: %d (UP/DN)", g_brush_radius);
    draw_text_overlay(px, w, h, 10, 450, val_buf, 0xFFEAEAEA);

    snprintf(val_buf, sizeof(val_buf), "Brush Press: %.2f", g_brush_pressure);
    draw_text_overlay(px, w, h, 10, 470, val_buf, 0xFFEAEAEA);

    draw_text_overlay(px, w, h, 10, 495, "Color (C to Cycle):", 0xFF888888);
    for (int r = 0; r < 20; r++) {
        for (int c = 10; c < 50; c++) {
            px[(515 + r) * w + c] = g_brush_color;
        }
    }
}

static void application_resize_hook(void *data, int32_t width, int32_t height) {
    (void)data; (void)width; (void)height;
    g_dirty = true;
}

static void application_key_hook(void *data, uint32_t serial, uint32_t time, uint32_t key, uint32_t state) {
    (void)data; (void)serial; (void)time; 
    if (state == 0) return; // Release

    if (key == KEY_Q || key == KEY_ESC) {
        g_force_quit = true;
    }

    // Select tools via number keys
    if (key == KEY_1) g_active_tool = TOOL_AIRBRUSH;
    if (key == KEY_2) g_active_tool = TOOL_CALLIGRAPHY;
    if (key == KEY_3) g_active_tool = TOOL_SMUDGE;
    if (key == KEY_4) g_active_tool = TOOL_SPRAY_CAN;
    if (key == KEY_5) g_active_tool = TOOL_CLONE;
    if (key == KEY_6) g_active_tool = TOOL_FLOOD_FILL;
    if (key == KEY_7) g_active_tool = TOOL_GRADIENT;
    if (key == KEY_8) g_active_tool = TOOL_PAGE_PEEL;

    // Toggle Auto-Rotate
    if (key == KEY_R) {
        g_auto_rotate = !g_auto_rotate;
    }

    // Load Mainframe JCL Job-Dataset dependency DAG
    if (key == KEY_M) {
        load_mainframe_dag_template();
        printf("[PAINT SVDAG] Loaded Mainframe JCL dependency DAG onto ZMM substrate.\n");
    }

    // Voxelize Canvas
    if (key == KEY_V) {
        if (g_dag_flower && g_canvas) {
            tsfi_svdag_voxelize_image(g_dag_flower, g_canvas, 1024, 768, 0.1f);
            tsfi_svdag_voxelize_image(g_dag_bear, g_canvas, 1024, 768, 0.1f);
            // Apply water tank LOD energy optimizations
            tsfi_svdag_execute_414d_water_tank(g_dag_flower, g_water_tank_energy, g_water_tank_depth);
            tsfi_svdag_execute_414d_water_tank(g_dag_bear, g_water_tank_energy, g_water_tank_depth);
            // Re-screen templates using proteo screener
            tsfi_proteo_screen_candidates_avx512(g_dag_flower, 4);
            printf("[PAINT SVDAG] Voxelized canvas & Screened candidates on ZMM substrate.\n");
        }
    }

    // Energy Pour adjust
    if (key == KEY_E) {
        g_water_tank_energy += 50.0f;
    }
    if (key == KEY_D) {
        g_water_tank_energy -= 50.0f;
        if (g_water_tank_energy < 50.0f) g_water_tank_energy = 50.0f;
    }

    // Target depth adjust
    if (key == KEY_T) {
        g_water_tank_depth++;
        if (g_water_tank_depth > 12) g_water_tank_depth = 12;
    }
    if (key == KEY_G) {
        g_water_tank_depth--;
        if (g_water_tank_depth < 1) g_water_tank_depth = 1;
    }

    // Intent weights adjust
    if (key == KEY_U) g_intent_w_user += 0.05f;
    if (key == KEY_I) { g_intent_w_user -= 0.05f; if (g_intent_w_user < 0.0f) g_intent_w_user = 0.0f; }
    if (key == KEY_J) g_intent_w_deepseek += 0.05f;
    if (key == KEY_K) { g_intent_w_deepseek -= 0.05f; if (g_intent_w_deepseek < 0.0f) g_intent_w_deepseek = 0.0f; }
    if (key == KEY_O) g_intent_w_gemini += 0.05f;
    if (key == KEY_P) { g_intent_w_gemini -= 0.05f; if (g_intent_w_gemini < 0.0f) g_intent_w_gemini = 0.0f; }

    // Brush radius control
    if (key == KEY_UP) {
        g_brush_radius += 2;
        if (g_brush_radius > 100) g_brush_radius = 100;
    }
    if (key == KEY_DOWN) {
        g_brush_radius -= 2;
        if (g_brush_radius < 1) g_brush_radius = 1;
    }

    // Clear Canvas
    if (key == KEY_K) {
        memset(g_canvas, 0, 1024 * 768 * sizeof(uint32_t));
    }

    // Cycle brush color
    if (key == KEY_C) {
        if (g_brush_color == 0xFF00FF00) g_brush_color = 0xFFFF0000; // Red
        else if (g_brush_color == 0xFFFF0000) g_brush_color = 0xFF0000FF; // Blue
        else if (g_brush_color == 0xFF0000FF) g_brush_color = 0xFFFFFF00; // Yellow
        else if (g_brush_color == 0xFFFFFF00) g_brush_color = 0xFFFFFFFF; // White
        else g_brush_color = 0xFF00FF00;
    }

    g_dirty = true;
}

int main(int argc, char **argv) {
    (void)argc; (void)argv;
    signal(SIGINT, handle_sigint);

    // Initialize global font registry and input system
    extern void tsfi_font_registry_init(void *reg);
    void *g_registry = lau_memalign_wired(512, 1024);
    tsfi_font_registry_init(g_registry);
    tsfi_input_init();

    // Create wayland/vulkan system
    VulkanSystem *s = create_vulkan_system();
    if (!s) return 1;
    s->external_render_enabled = true;
    xdg_toplevel_set_title(s->xdg_toplevel, "TSFi Auncient Vulkan Paintbox & SVDAG Realizer");

    tsfi_input_set_key_hook(application_key_hook);
    tsfi_input_set_resize_hook(application_resize_hook);

    // Allocate painting canvas substrate (1024x768)
    g_canvas = (uint32_t*)lau_malloc_wired(1024 * 768 * sizeof(uint32_t));
    g_temp_buffer = (uint32_t*)lau_malloc_wired(1024 * 768 * sizeof(uint32_t));
    g_paper_texture = (uint8_t*)lau_malloc_wired(1024 * 768);

    // Create the SVDAG taste tree instances
    g_dag_flower = tsfi_svdag_create(4096);
    g_dag_bear = tsfi_svdag_create(4096);
    
    // Allocate 3D Viewport buffers (400x400)
    g_3d_viewport = (uint32_t*)lau_malloc_wired(400 * 400 * sizeof(uint32_t));
    g_3d_depth = (float*)lau_malloc_wired(400 * 400 * sizeof(float));

    if (!g_canvas || !g_temp_buffer || !g_paper_texture || !g_dag_flower || !g_dag_bear || !g_3d_viewport || !g_3d_depth) {
        fprintf(stderr, "Failed to allocate memory substrate for Paint canvas / SVDAG\n");
        return 1;
    }

    memset(g_canvas, 0, 1024 * 768 * sizeof(uint32_t));
    memset(g_temp_buffer, 0, 1024 * 768 * sizeof(uint32_t));

    // Generate textured paper substrate profile
    for (int i = 0; i < 1024 * 768; i++) {
        g_paper_texture[i] = (uint8_t)(rand() % 40 + 215); // Noise profile [215, 255]
    }

    tsfi_io_printf(stdout, "[PAINT] Auncient Wayland-Vulkan Paint & SVDAG Realizer Program Active.\n");

    while (s->running && !g_force_quit) {
        wl_display_dispatch_pending(s->display);

        int W = s->paint_buffer->width, H = s->paint_buffer->height;
        uint32_t *px = (uint32_t*)s->paint_buffer->data;

        // Perform drawing if mouse is down and we are inside the canvas region (200 < x < W/2)
        int canvas_right_limit = W / 2;
        if (s->mouse_down && s->mouse_x > 200 && s->mouse_x < canvas_right_limit) {
            // Map window mouse coordinate back to canvas coordinates
            int canvas_w = canvas_right_limit - 200;
            int cx = (s->mouse_x - 200) * 1024 / canvas_w;
            int cy = s->mouse_y * 768 / H;

            if (g_prev_mouse_x == -1) {
                g_prev_mouse_x = cx;
                g_prev_mouse_y = cy;
            }

            // Route to correct Quantel paint module
            switch (g_active_tool) {
                case TOOL_AIRBRUSH:
                    tsfi_quantel_paintbox_airbrush(g_canvas, 1024, 768, cx, cy, g_brush_radius, g_brush_pressure, g_brush_color);
                    break;
                case TOOL_CALLIGRAPHY:
                    tsfi_quantel_paintbox_tablet_brush(g_canvas, 1024, 768, cx, cy, g_brush_radius, g_brush_pressure, 0.3f, 0.785f, g_brush_color);
                    break;
                case TOOL_SMUDGE:
                    tsfi_quantel_paintbox_wet_paint(g_canvas, 1024, 768, cx, cy, g_brush_radius, 0.2f, &g_brush_color);
                    break;
                case TOOL_SPRAY_CAN:
                    tsfi_quantel_paintbox_spray_can(g_canvas, 1024, 768, cx, cy, g_brush_radius, g_brush_pressure, g_brush_color);
                    break;
                case TOOL_CLONE:
                    tsfi_quantel_paintbox_clone(g_canvas, 1024, 768, cx, cy, 100, 100, g_brush_radius, 0.5f);
                    break;
                case TOOL_FLOOD_FILL:
                    tsfi_quantel_paintbox_flood_fill(g_canvas, 1024, 768, cx, cy, g_brush_color, 15.0f);
                    break;
                case TOOL_GRADIENT:
                    tsfi_quantel_paintbox_gradient(g_canvas, 1024, 768, 200, 0, cx, cy, g_brush_color, 0xFF000000, "linear");
                    break;
                case TOOL_PAGE_PEEL:
                    memcpy(g_temp_buffer, g_canvas, 1024 * 768 * sizeof(uint32_t));
                    tsfi_quantel_mirage_page_peel_transition(g_temp_buffer, g_canvas, g_canvas, 1024, 768, (float)cx / 1024.0f, 100.0f);
                    break;
                default:
                    break;
            }

            g_prev_mouse_x = cx;
            g_prev_mouse_y = cy;
            g_dirty = true;
        } else {
            g_prev_mouse_x = -1;
            g_prev_mouse_y = -1;
        }

        if (g_auto_rotate) {
            g_rotation_t += 0.005f;
            if (g_rotation_t > 1.0f) g_rotation_t -= 1.0f;
            g_dirty = true;
        }

        if (g_dirty || true) {
            // Apply trilateral intent modifications to the SVDAG
            tsfi_svdag_calculate_intent(g_dag_flower, g_intent_w_user, g_intent_w_deepseek, g_intent_w_gemini);
            tsfi_svdag_calculate_intent(g_dag_bear, g_intent_w_user, g_intent_w_deepseek, g_intent_w_gemini);

            // Render the path-traced SVDAG viewport
            for (int i = 0; i < 400 * 400; i++) {
                g_3d_viewport[i] = 0xFF14020B; // Deep dark purple background for ray tracer
                g_3d_depth[i] = 10.0f; // Reset depth
            }
            tsfi_svdag_path_trace(g_3d_viewport, g_3d_depth, g_dag_flower, g_dag_bear, 400, 400, g_rotation_t, 0.5f, 0.5f, 0.5f);

            // Render 2D canvas and copy SVDAG 3D viewport side-by-side
            for (int y = 0; y < H; y++) {
                int canvas_y = y * 768 / H;
                if (canvas_y >= 768) canvas_y = 767;

                int canvas_w = canvas_right_limit - 200;

                // 1. Draw Left Side: 2D Canvas
                for (int x = 200; x < canvas_right_limit; x++) {
                    int canvas_x = (x - 200) * 1024 / canvas_w;
                    if (canvas_x >= 1024) canvas_x = 1023;

                    uint32_t c_pixel = g_canvas[canvas_y * 1024 + canvas_x];
                    if ((c_pixel & 0xFFFFFF) == 0) {
                        uint8_t tex = g_paper_texture[canvas_y * 1024 + canvas_x];
                        px[y * W + x] = 0xFF000000 | (tex << 16) | (tex << 8) | tex;
                    } else {
                        px[y * W + x] = c_pixel;
                    }
                }

                // 2. Draw Right Side: 3D Viewport
                int viewport_start_x = canvas_right_limit;
                int viewport_y = y * 400 / H;
                if (viewport_y >= 400) viewport_y = 399;

                for (int x = viewport_start_x; x < W; x++) {
                    int viewport_x = (x - viewport_start_x) * 400 / (W - viewport_start_x);
                    if (viewport_x >= 400) viewport_x = 399;

                    px[y * W + x] = g_3d_viewport[viewport_y * 400 + viewport_x];
                }
            }

            draw_toolbox(px, W, H);
            draw_frame(s);
            g_dirty = false;
        }

        usleep(16000); // ~60fps
    }

    lau_free(g_canvas);
    lau_free(g_temp_buffer);
    lau_free(g_paper_texture);
    tsfi_svdag_destroy(g_dag_flower);
    tsfi_svdag_destroy(g_dag_bear);
    lau_free(g_3d_viewport);
    lau_free(g_3d_depth);
    destroy_vulkan_system(s);

    return 0;
}
