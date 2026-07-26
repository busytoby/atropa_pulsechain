#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include <poll.h>
#include <fcntl.h>
#include <math.h>
#include <sys/mman.h>
#include <dlfcn.h>
#include <wayland-client.h>
#include "xdg-shell-client-protocol.h"
#include "../src/auncient_timeline_autodin.h"

// Font definition (5x7 bits per character)
static const uint8_t font5x7[128][5] = {
    [' '] = {0x00, 0x00, 0x00, 0x00, 0x00},
    ['!'] = {0x00, 0x00, 0x5f, 0x00, 0x00},
    ['#'] = {0x14, 0x7f, 0x14, 0x7f, 0x14},
    ['$'] = {0x24, 0x2a, 0x7f, 0x2a, 0x12},
    ['%'] = {0x23, 0x13, 0x08, 0x64, 0x62},
    ['&'] = {0x36, 0x49, 0x55, 0x22, 0x50},
    ['('] = {0x00, 0x1c, 0x22, 0x41, 0x00},
    [')'] = {0x00, 0x41, 0x22, 0x1c, 0x00},
    ['*'] = {0x14, 0x08, 0x3e, 0x08, 0x14},
    ['+'] = {0x08, 0x08, 0x3e, 0x08, 0x08},
    [','] = {0x00, 0x50, 0x30, 0x00, 0x00},
    ['-'] = {0x08, 0x08, 0x08, 0x08, 0x08},
    ['.'] = {0x00, 0x60, 0x60, 0x00, 0x00},
    ['/'] = {0x20, 0x10, 0x08, 0x04, 0x02},
    ['0'] = {0x3e, 0x51, 0x49, 0x45, 0x3e},
    ['1'] = {0x00, 0x42, 0x7f, 0x40, 0x00},
    ['2'] = {0x42, 0x61, 0x51, 0x49, 0x46},
    ['3'] = {0x21, 0x41, 0x45, 0x4b, 0x31},
    ['4'] = {0x18, 0x14, 0x12, 0x7f, 0x10},
    ['5'] = {0x27, 0x45, 0x45, 0x45, 0x39},
    ['6'] = {0x3c, 0x4a, 0x49, 0x49, 0x30},
    ['7'] = {0x01, 0x71, 0x09, 0x05, 0x03},
    ['8'] = {0x36, 0x49, 0x49, 0x49, 0x36},
    ['9'] = {0x06, 0x49, 0x49, 0x29, 0x1e},
    [':'] = {0x00, 0x36, 0x36, 0x00, 0x00},
    [';'] = {0x00, 0x56, 0x36, 0x00, 0x00},
    ['<'] = {0x08, 0x14, 0x22, 0x41, 0x00},
    ['='] = {0x14, 0x14, 0x14, 0x14, 0x14},
    ['>'] = {0x00, 0x41, 0x22, 0x14, 0x08},
    ['?'] = {0x02, 0x01, 0x51, 0x09, 0x06},
    ['@'] = {0x32, 0x49, 0x79, 0x41, 0x3e},
    ['A'] = {0x7e, 0x11, 0x11, 0x11, 0x7e},
    ['B'] = {0x7f, 0x49, 0x49, 0x49, 0x36},
    ['C'] = {0x3e, 0x41, 0x41, 0x41, 0x22},
    ['D'] = {0x7f, 0x41, 0x41, 0x22, 0x1c},
    ['E'] = {0x7f, 0x49, 0x49, 0x49, 0x41},
    ['F'] = {0x7f, 0x09, 0x09, 0x09, 0x01},
    ['G'] = {0x3e, 0x41, 0x49, 0x49, 0x7a},
    ['H'] = {0x7f, 0x08, 0x08, 0x08, 0x7f},
    ['I'] = {0x00, 0x41, 0x7f, 0x41, 0x00},
    ['J'] = {0x20, 0x40, 0x41, 0x3f, 0x01},
    ['K'] = {0x7f, 0x08, 0x14, 0x22, 0x41},
    ['L'] = {0x7f, 0x40, 0x40, 0x40, 0x40},
    ['M'] = {0x7f, 0x02, 0x0c, 0x02, 0x7f},
    ['N'] = {0x7f, 0x04, 0x08, 0x10, 0x7f},
    ['O'] = {0x3e, 0x41, 0x41, 0x41, 0x3e},
    ['P'] = {0x7f, 0x09, 0x09, 0x09, 0x06},
    ['Q'] = {0x3e, 0x41, 0x51, 0x21, 0x5e},
    ['R'] = {0x7f, 0x09, 0x19, 0x29, 0x46},
    ['S'] = {0x46, 0x49, 0x49, 0x49, 0x31},
    ['T'] = {0x01, 0x01, 0x7f, 0x01, 0x01},
    ['U'] = {0x3f, 0x40, 0x40, 0x40, 0x3f},
    ['V'] = {0x1f, 0x20, 0x40, 0x20, 0x1f},
    ['W'] = {0x3f, 0x40, 0x38, 0x40, 0x3f},
    ['X'] = {0x63, 0x14, 0x08, 0x14, 0x63},
    ['Y'] = {0x07, 0x08, 0x70, 0x08, 0x07},
    ['Z'] = {0x61, 0x51, 0x49, 0x45, 0x43},
    ['['] = {0x00, 0x7f, 0x41, 0x41, 0x00},
    [']'] = {0x00, 0x41, 0x41, 0x7f, 0x00},
    ['_'] = {0x40, 0x40, 0x40, 0x40, 0x40},
    ['a'] = {0x20, 0x54, 0x54, 0x54, 0x78},
    ['b'] = {0x7f, 0x48, 0x44, 0x44, 0x38},
    ['c'] = {0x38, 0x44, 0x44, 0x44, 0x20},
    ['d'] = {0x38, 0x44, 0x44, 0x48, 0x7f},
    ['e'] = {0x38, 0x54, 0x54, 0x54, 0x18},
    ['f'] = {0x08, 0x7e, 0x09, 0x01, 0x02},
    ['g'] = {0x0c, 0x52, 0x52, 0x52, 0x3e},
    ['h'] = {0x7f, 0x08, 0x04, 0x04, 0x78},
    ['i'] = {0x00, 0x44, 0x7d, 0x40, 0x00},
    ['j'] = {0x20, 0x40, 0x44, 0x3d, 0x00},
    ['k'] = {0x7f, 0x10, 0x28, 0x44, 0x00},
    ['l'] = {0x00, 0x41, 0x7f, 0x40, 0x00},
    ['m'] = {0x7c, 0x04, 0x18, 0x04, 0x78},
    ['n'] = {0x7c, 0x08, 0x04, 0x04, 0x78},
    ['o'] = {0x38, 0x44, 0x44, 0x44, 0x38},
    ['p'] = {0x7c, 0x14, 0x14, 0x14, 0x08},
    ['q'] = {0x08, 0x14, 0x14, 0x18, 0x7c},
    ['r'] = {0x7c, 0x08, 0x04, 0x04, 0x08},
    ['s'] = {0x48, 0x54, 0x54, 0x54, 0x20},
    ['t'] = {0x04, 0x3f, 0x44, 0x40, 0x20},
    ['u'] = {0x3c, 0x40, 0x40, 0x20, 0x7c},
    ['v'] = {0x1c, 0x20, 0x40, 0x20, 0x1c},
    ['w'] = {0x3c, 0x40, 0x30, 0x40, 0x3c},
    ['x'] = {0x44, 0x28, 0x10, 0x28, 0x44},
    ['y'] = {0x0c, 0x50, 0x50, 0x50, 0x3c},
    ['z'] = {0x44, 0x64, 0x54, 0x4c, 0x44}
};

static struct wl_compositor *compositor = NULL;
static struct wl_shm *shm = NULL;
static struct wl_seat *seat = NULL;
static struct wl_keyboard *keyboard = NULL;
static struct xdg_wm_base *xdg_wm_base = NULL;
static struct wl_surface *surface = NULL;
static struct xdg_surface *xdg_surface = NULL;
static struct xdg_toplevel *xdg_toplevel = NULL;
static struct wl_buffer *wl_buffers[2] = {NULL, NULL};
static int current_buffer_idx = 0;
static bool running = true;

// Window dimensions
static int win_width = 1280;
static int win_height = 720;

// Editor Document buffer (Markdown input)
static char doc_buf[2048] = "# AUNCIENT MD EDITOR\n> TYPE YOUR DOCUMENT HERE\n";
static int doc_len = 46;

// Compressed scroller payload text (Decompression simulation LUT)
static const char compressed_scroller[32] = "PAGADATA_2026_RETRO_C64_INTRO_";
static char decompressed_scroller[128];

// Parallax Scroller text: Singular / HHH Tribute scroll
static const char *parallax_scroller_text = "CONSPIRACY & SINGULAR HUNGARIAN TRIBUTE -- INTRODUCING MULTI-LAYER PARALLAX SCROLLER PATHS -- ";

// Static 256-entry Sine Lookup Table (LUT)
static float sine_lut[256];

// Color Cycle Palette LUT (Sunset Theme: Warm colors)
static const uint32_t color_cycle_lut[16] = {
    0xFF6A1B00, 0xFF8A2B00, 0xFFAA3B00, 0xFFCA4B00,
    0xFFEA5B00, 0xFFFF6B00, 0xFFFF8B20, 0xFFFFAB40,
    0xFFFF8B20, 0xFFFF6B00, 0xFFEA5B00, 0xFFCA4B00,
    0xFFAA3B00, 0xFF8A2B00, 0xFF6A1B00, 0xFF4A0A00
};

// Logarithmic SID Volume Fader LUT
static const uint8_t log_volume_lut[16] = {
    0, 1, 1, 2, 2, 3, 4, 5, 6, 7, 9, 10, 12, 13, 14, 15
};

// PETSCII Custom Charset Translation Map LUT
static const char charset_map_lut[128] = {
    ['a'] = 'A', ['b'] = 'B', ['c'] = 'C', ['d'] = 'D',
    ['e'] = 'E', ['f'] = 'F', ['g'] = 'G', ['h'] = 'H',
    ['i'] = 'I', ['j'] = 'J', ['k'] = 'K', ['l'] = 'L',
    ['m'] = 'M', ['n'] = 'N', ['o'] = 'O', ['p'] = 'P',
    ['q'] = 'Q', ['r'] = 'R', ['s'] = 'S', ['t'] = 'T',
    ['u'] = 'U', ['v'] = 'V', ['w'] = 'W', ['x'] = 'X',
    ['y'] = 'Y', ['z'] = 'Z'
};

// VIC-II Simulated Register state maps
static float retro_time = 0.0f;
static uint8_t vic_d012 = 130;  // Raster split scanline
static uint8_t vic_d016 = 0;    // Fine scroll shift (0-7 pixels)

// Interactive Loader state variables
static float loader_flash_time = 0.0f;
static float type_activity = 0.0f;

// Glitch Screen Shake displacement coordinates
static int glitch_x = 0;
static int glitch_y = 0;

// SID Audio Chip Register Emulation
typedef struct {
    uint16_t freq;
    uint16_t pw;
    uint8_t ctrl;
    uint8_t adsr[2];
} SidVoice;

static struct {
    SidVoice voices[3];
    uint16_t filter_freq;
    uint8_t filter_ctrl;
    uint8_t volume;
    // Hard Sync & Ring Mod Emulator registers
    bool hard_sync_enabled;
    bool ring_mod_enabled;
} sid_chip;

static int sid_arp_step = 0;
static bool voice_active[3] = {true, true, true};

// 4 different compiled SID tunes (Tune 3 is hidden!)
static const uint16_t sid_tunes[4][3] = {
    {0x1100, 0x1500, 0x1A00}, // Tune 0: C-4, E-4, G-4
    {0x1200, 0x1600, 0x1C00}, // Tune 1: D-4, F-4, A-4
    {0x1400, 0x1800, 0x1E00}, // Tune 2: E-4, G-4, B-4
    {0x0800, 0x0C00, 0x1100}  // Tune 3 (Hidden): C-3, G-3, C-4
};

static int active_tune = 0;
static bool hidden_unlocked = false;

// Sequential key history to unlock hidden tune ("tsn")
static char key_history[3] = {'\0', '\0', '\0'};

// PETSCII Western Desert Artwork Split Screen (Cactus & Sunset Sunset mesa)
static const char *western_desert_art[6] = {
    "      #      .   .      ",
    "    #####    .   .      ",
    "      #    *       *    ",
    "    ###       ###       ",
    "   #####     #####      ",
    "========================"
};

// 16x16 Bubble Font Bitmaps for TSFi/2
static const uint16_t bubble_font_tsfi2[6][16] = {
    // T
    {0x3FFC, 0x7FFE, 0xFFFF, 0xE3C7, 0x03C0, 0x03C0, 0x03C0, 0x03C0, 0x03C0, 0x03C0, 0x03C0, 0x03C0, 0x03C0, 0x03C0, 0x03C0, 0x0180},
    // S
    {0x3FFC, 0x7FFE, 0xC003, 0xC000, 0x7FE0, 0x3FF8, 0x01FC, 0x007E, 0x000F, 0xC007, 0xC003, 0xE007, 0x7FFE, 0x7FFE, 0x3FFC, 0x0000},
    // F
    {0xFFFF, 0xFFFF, 0xC000, 0xC000, 0xFFF0, 0xFFF0, 0xC000, 0xC000, 0xC000, 0xC000, 0xC000, 0xC000, 0xC000, 0xC000, 0xC000, 0x0000},
    // i
    {0x0180, 0x03C0, 0x03C0, 0x0180, 0x0000, 0x0000, 0x0180, 0x03C0, 0x03C0, 0x03C0, 0x03C0, 0x03C0, 0x03C0, 0x03C0, 0x03C0, 0x0180},
    // /
    {0x000F, 0x001F, 0x003E, 0x007C, 0x00F8, 0x01F0, 0x03E0, 0x07C0, 0x0F80, 0x1F00, 0x3E00, 0x7C00, 0xF800, 0xF000, 0xE000, 0x0000},
    // 2
    {0x3FFC, 0x7FFE, 0xC007, 0x000F, 0x001E, 0x003C, 0x0078, 0x00F0, 0x01E0, 0x03C0, 0x0780, 0x0F00, 0xFFFF, 0xFFFF, 0xFFFF, 0x0000}
};

// Multiplexed Sprite Structure
typedef struct {
    int x;
    int y;
    char glyph;
    uint32_t color;
} MultiplexSprite;

static MultiplexSprite sprites[5] = {
    {100, 150, 'A', 0xFFFF00FF},
    {200, 250, 'B', 0xFF00FFFF},
    {300, 100, 'C', 0xFFFFFF00},
    {150, 300, 'D', 0xFF00FF00},
    {250, 80,  'E', 0xFFFF5555}
};

static int sorted_sprite_indices[5] = {0, 1, 2, 3, 4};

// 3D Starfield Array
typedef struct {
    float x;
    float y;
    float z;
    char glyph;
} Star;

static Star starfield[15];

// 2D horizontal scrolling background stars
typedef struct {
    float x;
    int y;
    float speed;
    char glyph;
    uint32_t color;
} ScrollStar;

static ScrollStar scroll_stars[30];

// Binary History Record (No Mocking)
typedef struct {
    uint32_t transaction_id;
    uint32_t state_code;
    char action;
    uint32_t hash;
} EditorHistoryRecord;

// Helper for allocating SHM pool memory
static struct wl_buffer *create_shm_buffer(int width, int height, uint32_t **out_pixels) {
    int stride = width * 4;
    int size = stride * height;
    
    int fd = memfd_create("shm-editor", MFD_CLOEXEC);
    if (fd < 0) return NULL;
    if (ftruncate(fd, size) < 0) {
        close(fd);
        return NULL;
    }
    
    *out_pixels = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (*out_pixels == MAP_FAILED) {
        close(fd);
        return NULL;
    }
    
    struct wl_shm_pool *pool = wl_shm_create_pool(shm, fd, size);
    struct wl_buffer *buffer = wl_shm_pool_create_buffer(pool, 0, width, height, stride, WL_SHM_FORMAT_ARGB8888);
    wl_shm_pool_destroy(pool);
    close(fd);
    
    return buffer;
}

// Drawing text helpers
static void draw_char(uint32_t *pixels, int w, int h, int start_x, int start_y, uint8_t ch, uint32_t color, int scale) {
    if (ch >= 128) return;
    const uint8_t *glyph = font5x7[(int)ch];
    for (int col = 0; col < 5; col++) {
        uint8_t byte = glyph[col];
        for (int row = 0; row < 7; row++) {
            if (byte & (1 << row)) {
                for (int sy = 0; sy < scale; sy++) {
                    for (int sx = 0; sx < scale; sx++) {
                        int px = start_x + col * scale + sx;
                        int py = start_y + row * scale + sy;
                        if (px >= 0 && px < w && py >= 0 && py < h) {
                            pixels[py * w + px] = color;
                        }
                    }
                }
            }
        }
    }
}

static void draw_string(uint32_t *pixels, int w, int h, int start_x, int start_y, const char *str, uint32_t color, int scale) {
    int cur_x = start_x;
    while (*str) {
        draw_char(pixels, w, h, cur_x, start_y, *str, color, scale);
        cur_x += 6 * scale;
        str++;
    }
}

// Redraw screen with real-time PETSCII / C64 effects
static void redraw_screen(void) {
    if (!surface) return;
    
    current_buffer_idx = 1 - current_buffer_idx;
    if (wl_buffers[current_buffer_idx]) {
        wl_buffer_destroy(wl_buffers[current_buffer_idx]);
    }
    
    uint32_t *pixels = NULL;
    wl_buffers[current_buffer_idx] = create_shm_buffer(win_width, win_height, &pixels);
    if (!wl_buffers[current_buffer_idx] || !pixels) return;
    
    // Background raster scanline coloring loop
    for (int y = 0; y < win_height; y++) {
        if (y >= 100 && y < 550 && (y % 24) == 0) {
            for (int x = 0; x < win_width; x++) {
                pixels[y * win_width + x] = 0xFF000000;
            }
            continue;
        }

        uint32_t bg_color = 0xFF221100; // Warm dark base
        
        if (loader_flash_time > 0.0f && (y < (int)vic_d012 * 3 || y > win_height - 100)) {
            int rand_color_idx = (int)(y * 0.15f + retro_time * 800.0f + (rand() % 4)) & 0x0F;
            bg_color = color_cycle_lut[rand_color_idx];
        } else {
            // Calm, normal sunset gradient based on vertical coordinate y
            float factor = (float)y / (float)win_height;
            uint8_t r_val, g_val, b_val;
            if (factor < 0.6f) {
                float t = factor / 0.6f;
                r_val = (uint8_t)(0x1A * (1.0f - t) + 0xFF * t);
                g_val = (uint8_t)(0x00 * (1.0f - t) + 0x55 * t);
                b_val = (uint8_t)(0x2A * (1.0f - t) + 0x00 * t);
            } else {
                float t = (factor - 0.6f) / 0.4f;
                r_val = (uint8_t)(0xFF * (1.0f - t) + 0x1C * t);
                g_val = (uint8_t)(0x55 * (1.0f - t) + 0x0B * t);
                b_val = (uint8_t)(0x00 * (1.0f - t) + 0x00 * t);
            }
            bg_color = (0xFF << 24) | (r_val << 16) | (g_val << 8) | b_val;
        }
        
        for (int x = 0; x < win_width; x++) {
            pixels[y * win_width + x] = bg_color;
        }
    }
    
    // Draw 2D scrolling background stars (Parallax background depth layer)
    for (int i = 0; i < 30; i++) {
        int px = (int)scroll_stars[i].x + glitch_x;
        int py = scroll_stars[i].y + glitch_y;
        if (px >= 0 && px < win_width && py >= 0 && py < win_height) {
            draw_char(pixels, win_width, win_height, px, py, scroll_stars[i].glyph, scroll_stars[i].color, 1);
        }
    }

    // Draw 3D PETSCII Starfield behind the document text
    for (int i = 0; i < 15; i++) {
        if (starfield[i].z > 0.1f) {
            int px = win_width / 2 + (int)((starfield[i].x / starfield[i].z) * (win_width / 2)) + glitch_x;
            int py = win_height / 2 + (int)((starfield[i].y / starfield[i].z) * (win_height / 2)) + glitch_y;
            
            if (px >= 0 && px < win_width && py >= 0 && py < win_height) {
                uint32_t star_color = 0xFF884400;
                if (starfield[i].z < 3.0f) star_color = 0xFFFFCC00;
                else if (starfield[i].z < 6.0f) star_color = 0xFFAA6600;
                
                draw_char(pixels, win_width, win_height, px, py, starfield[i].glyph, star_color, 2);
            }
        }
    }

    // Infinite Zooming Tunnel/Zoomer Effect
    int tunnel_center_x = win_width / 2;
    int tunnel_center_y = win_height / 2;
    for (int i = 0; i < 4; i++) {
        float zoom_rad = fmodf(retro_time * 80.0f + i * 90.0f, 360.0f);
        int w_size = (int)(zoom_rad * 1.6f);
        int h_size = (int)(zoom_rad * 0.9f);
        
        for (int tx = -w_size; tx <= w_size; tx += 12) {
            draw_char(pixels, win_width, win_height, tunnel_center_x + tx + glitch_x, tunnel_center_y - h_size + glitch_y, '=', 0xFF331100, 1);
            draw_char(pixels, win_width, win_height, tunnel_center_x + tx + glitch_x, tunnel_center_y + h_size + glitch_y, '=', 0xFF331100, 1);
        }
        for (int ty = -h_size; ty <= h_size; ty += 12) {
            draw_char(pixels, win_width, win_height, tunnel_center_x - w_size + glitch_x, tunnel_center_y + ty + glitch_y, '|', 0xFF331100, 1);
            draw_char(pixels, win_width, win_height, tunnel_center_x + w_size + glitch_x, tunnel_center_y + ty + glitch_y, '|', 0xFF331100, 1);
        }
    }

    // Audio Peak Border Pulsing
    int base_scale = win_width / 280;
    if (base_scale < 1) base_scale = 1;
    if (base_scale > 6) base_scale = 6;
    
    int scale = base_scale;
    if (sid_chip.volume > 8) {
        scale = base_scale + 1;
    }
    
    // Split Screen Layout: Art in top half, Editor text grid in bottom half
    int grid_w = 40 * 6 * scale;
    int start_x = (win_width - grid_w) / 2 + glitch_x;
    int start_y = win_height / 2 + 10 + glitch_y; // Lower half
    if (start_x < 20) start_x = 20;
    if (start_y < win_height / 2) start_y = win_height / 2;
    
    // Parse Markdown to ANSI grid (40 columns, 7 rows for split screen)
    char ansi_grid[280];
    memset(ansi_grid, ' ', sizeof(ansi_grid));
    auncient_parse_markdown_to_ansi(doc_buf, ansi_grid, 40, 7, 0);
    
    int cursor_r = doc_len / 40;
    int cursor_c = doc_len % 40;
    
    // Render bottom half editor grid
    for (int r = 0; r < 7; r++) {
        int fine_shift = (vic_d016 & 0x07);
        int wave_idx = (int)(r * 15.0f + retro_time * 150.0f) & 0xFF;
        int row_displace = (int)(sine_lut[wave_idx] * 3.0f * scale) + fine_shift;
        
        for (int c = 0; c < 40; c++) {
            char ch = ansi_grid[r * 40 + c];
            
            // Apply raymarched sphere overlay inside empty cells
            float nx = ((float)c - 20.0f) / 10.0f;
            float ny = ((float)r - 3.5f) / 3.0f;
            float time_sin = sine_lut[(int)(retro_time * 120.0f) & 0xFF];
            float sphere_radius = 1.0f + time_sin * 0.2f;
            float dist_from_sphere = sqrtf(nx*nx + ny*ny) - sphere_radius;
            
            if (dist_from_sphere < 0.0f && ch == ' ') {
                char sphere_shades[] = "%#*+=-:. ";
                int shade_idx = (int)(fabsf(dist_from_sphere) * 8.0f);
                if (shade_idx < 0) shade_idx = 0;
                if (shade_idx > 8) shade_idx = 8;
                ch = sphere_shades[shade_idx];
            }
            
            int idx1 = (int)(c * 12.0f + retro_time * 180.0f) & 0xFF;
            int idx2 = (int)(r * 18.0f + retro_time * 120.0f) & 0xFF;
            float wave1 = sine_lut[idx1];
            float wave2 = sine_lut[idx2];
            float plasma = (wave1 + wave2) / 2.0f;
            
            if (ch == ' ') {
                char plasma_chars[] = " .:-=+*";
                int p_idx = (int)((plasma + 1.0f) * 3.0f);
                if (p_idx < 0) p_idx = 0;
                if (p_idx > 6) p_idx = 6;
                ch = plasma_chars[p_idx];
            } else {
                char mapped = charset_map_lut[(uint8_t)ch];
                if (mapped) ch = mapped;
            }
            
            int dist_r = r - cursor_r;
            int dist_c = c - cursor_c;
            float dist = sqrtf((float)(dist_r * dist_r + dist_c * dist_c));
            
            uint32_t color = 0xFFFFCC00; // Warm gold default
            if (type_activity > 1.0f) {
                int type_color_idx = (int)(retro_time * 25.0f + c) & 0x0F;
                color = color_cycle_lut[type_color_idx];
            } else {
                if (dist < 4.0f) {
                    color = 0xFFFF8800; // Flame orange near cursor
                } else if (dist < 8.0f) {
                    color = 0xFFCC4400; // Dark red
                } else {
                    color = 0xFF882200; // Muted dark brown
                }
            }
            
            if (ch == '|') color = 0xFFFFFF00;
            else if (ch == '*') color = 0xFFFF5500;
            
            draw_char(pixels, win_width, win_height, 
                      start_x + c * 6 * scale + row_displace, 
                      start_y + r * 8 * scale, 
                      ch, color, scale);
        }
    }
    
    // Render top half Western Desert Artwork (Cactus & mesa Sunset)
    int art_start_x = (win_width - 24 * 12) / 2 + glitch_x;
    int art_start_y = 120 + glitch_y;
    for (int pr = 0; pr < 6; pr++) {
        for (int pc = 0; pc < 24; pc++) {
            char symbol = western_desert_art[pr][pc];
            if (symbol != ' ') {
                uint32_t art_color = 0xFF00FF00; // Green cactus
                if (symbol == '*') art_color = 0xFFFFCC00; // Yellow sun
                else if (symbol == '.') art_color = 0xFFFF5500; // Red sunset dots
                else if (symbol == '=') art_color = 0xFF884400; // Brown ground
                
                draw_char(pixels, win_width, win_height, 
                          art_start_x + pc * 12, 
                          art_start_y + pr * 16, 
                          symbol, art_color, 2);
            }
        }
    }

    // Render TSFi/2 bubble logo centered above the cactus art (16x16 pixel bitmaps with 3D layers)
    int logo_start_x = art_start_x - 50;
    int logo_start_y = 120 + glitch_y;
    int char_spacing = 64; // Distance between characters (16 pixels * scale 4)
    
    // 1. Draw 3D Drop Shadow / Extrusion layer with matching wobble displacement
    for (int char_idx = 0; char_idx < 6; char_idx++) {
        for (int r = 0; r < 16; r++) {
            uint16_t row_bits = bubble_font_tsfi2[char_idx][r];
            for (int c = 0; c < 16; c++) {
                if (row_bits & (1 << (15 - c))) {
                    int wobble_idx_x = (int)(retro_time * 90.0f + r * 12 + c * 6 + char_idx * 24) & 0xFF;
                    int wobble_idx_y = (int)(retro_time * 75.0f + r * 8 + c * 10 + char_idx * 30) & 0xFF;
                    int wobble_x = (int)(sine_lut[wobble_idx_x] * 3.0f);
                    int wobble_y = (int)(sine_lut[wobble_idx_y] * 3.0f);
                    
                    int pixel_x = logo_start_x + char_idx * char_spacing + c * 4 + wobble_x + 6; // Shifted right
                    int pixel_y = logo_start_y + r * 4 + wobble_y + 6; // Shifted down
                    for (int sy = 0; sy < 4; sy++) {
                        for (int sx = 0; sx < 4; sx++) {
                            int px = pixel_x + sx;
                            int py = pixel_y + sy;
                            if (px >= 0 && px < win_width && py >= 0 && py < win_height) {
                                pixels[py * win_width + px] = 0xFF1C0500; // Deep shadow
                            }
                        }
                    }
                }
            }
        }
    }

    // 2. Draw Main Body with Dynamic Volumetric Distance Field Inflation and Glossy Specular Highlights
    for (int char_idx = 0; char_idx < 6; char_idx++) {
        for (int r = 0; r < 16; r++) {
            uint16_t row_bits = bubble_font_tsfi2[char_idx][r];
            uint16_t prev_row_bits = (r > 0) ? bubble_font_tsfi2[char_idx][r - 1] : 0;
            for (int c = 0; c < 16; c++) {
                if (row_bits & (1 << (15 - c))) {
                    int wobble_idx_x = (int)(retro_time * 90.0f + r * 12 + c * 6 + char_idx * 24) & 0xFF;
                    int wobble_idx_y = (int)(retro_time * 75.0f + r * 8 + c * 10 + char_idx * 30) & 0xFF;
                    int wobble_x = (int)(sine_lut[wobble_idx_x] * 3.0f);
                    int wobble_y = (int)(sine_lut[wobble_idx_y] * 3.0f);
                    
                    int pixel_x = logo_start_x + char_idx * char_spacing + c * 4 + wobble_x;
                    int pixel_y = logo_start_y + r * 4 + wobble_y;
                    
                    // Calculate distance to nearest empty pixel (volumetric inflation depth)
                    int dist_sq = 16;
                    for (int dr = -2; dr <= 2; dr++) {
                        for (int dc = -2; dc <= 2; dc++) {
                            int nr = r + dr;
                            int nc = c + dc;
                            bool is_inside = (nr >= 0 && nr < 16 && nc >= 0 && nc < 16);
                            bool is_filled = is_inside && (bubble_font_tsfi2[char_idx][nr] & (1 << (15 - nc)));
                            if (!is_filled) {
                                int d = dr * dr + dc * dc;
                                if (d < dist_sq) dist_sq = d;
                            }
                        }
                    }
                    
                    // Detect if this is a top-left edge pixel (empty to top or left)
                    bool is_top_edge = (r == 0) || !(prev_row_bits & (1 << (15 - c)));
                    bool is_left_edge = (c == 0) || !(row_bits & (1 << (15 - (c - 1))));
                    bool is_glossy = is_top_edge && is_left_edge;
                    
                    // Color calculation based on inflation depth
                    int color_idx = (int)(retro_time * 15.0f + char_idx * 4 + c) & 0x0F;
                    uint32_t border_color = color_cycle_lut[color_idx];
                    uint32_t pixel_color = border_color;
                    
                    if (dist_sq > 2) {
                        // Inflated center: Golden yellow
                        pixel_color = 0xFFFFCC00;
                    } else if (dist_sq > 1) {
                        // Intermediate slope: Orange
                        pixel_color = 0xFFFF6600;
                    }
                    
                    for (int sy = 0; sy < 4; sy++) {
                        for (int sx = 0; sx < 4; sx++) {
                            int px = pixel_x + sx;
                            int py = pixel_y + sy;
                            if (px >= 0 && px < win_width && py >= 0 && py < win_height) {
                                if (is_glossy && sx < 2 && sy < 2) {
                                    pixels[py * win_width + px] = 0xFFFFFFFF; // Specular highlight glint
                                } else {
                                    pixels[py * win_width + px] = pixel_color;
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    // Render PETSCII Screen Border Frames scrolling around the bottom editor text canvas margins
    int frame_y_top = start_y - 12 * scale;
    int frame_y_bottom = start_y + 7 * 8 * scale + 4 * scale;
    for (int col = 0; col < 40; col++) {
        int frame_char_idx = (col + (int)(retro_time * 12.0f)) % 6;
        char frame_chars[] = "[=#=-]";
        char ch = frame_chars[frame_char_idx];
        draw_char(pixels, win_width, win_height, start_x + col * 6 * scale, frame_y_top, ch, 0xFFFF8800, scale);
        draw_char(pixels, win_width, win_height, start_x + col * 6 * scale, frame_y_bottom, ch, 0xFFFF8800, scale);
    }

    // Update Sprite Coordinates dynamically (Tracing a sinusoidal trailing path - Western style)
    for (int i = 0; i < 5; i++) {
        sprites[i].x = 100 + i * 200 + (int)(retro_time * 60.0f) % (win_width - 200);
        int wave_idx = ((int)(retro_time * 80.0f) + i * 40) & 0xFF;
        sprites[i].y = win_height / 2 + (int)(sine_lut[wave_idx] * 80.0f);
    }
    
    // Sort Sprites by Y Coordinate (VIC-II Multiplexer simulation sorting LUT)
    for (int i = 0; i < 5; i++) sorted_sprite_indices[i] = i;
    for (int i = 0; i < 4; i++) {
        for (int j = i + 1; j < 5; j++) {
            if (sprites[sorted_sprite_indices[i]].y > sprites[sorted_sprite_indices[j]].y) {
                int temp = sorted_sprite_indices[i];
                sorted_sprite_indices[i] = sorted_sprite_indices[j];
                sorted_sprite_indices[j] = temp;
            }
        }
    }
    
    // Draw sorted multiplexed sprites on canvas
    for (int i = 0; i < 5; i++) {
        int idx = sorted_sprite_indices[i];
        draw_char(pixels, win_width, win_height, 
                  sprites[idx].x + glitch_x, 
                  sprites[idx].y + glitch_y, 
                  sprites[idx].glyph, 
                  sprites[idx].color, 3);
    }
    
    // 1. Top Parallax Scroller moving RIGHT (Smooth right-scrolling math)
    float p_scroll_speed = 60.0f;
    float p_scroll_x_total = retro_time * p_scroll_speed;
    int p_pixel_shift = (int)fmodf(p_scroll_x_total, 12.0f);
    int p_scroller_y = 60 + glitch_y;
    int len = strlen(parallax_scroller_text);
    int p_base_char_idx = len - ((int)(p_scroll_x_total / 12.0f) % len);
    for (int col = 0; col < 90; col++) {
        int char_idx = (p_base_char_idx + col) % len;
        char ch = parallax_scroller_text[char_idx];
        draw_char(pixels, win_width, win_height, 20 + col * 12 + p_pixel_shift + glitch_x, p_scroller_y, ch, 0xFFFFCC00, 2);
    }

    // 2. Bottom Main Scroller moving LEFT
    float scroll_x_speed = 30.0f;
    if (active_tune == 1) scroll_x_speed = 45.0f;
    else if (active_tune == 2) scroll_x_speed = 60.0f;
    else if (active_tune == 3) scroll_x_speed = 90.0f;
    
    float scroll_x_total = retro_time * scroll_x_speed;
    int base_char_idx = (int)(scroll_x_total / 18.0f) % strlen(decompressed_scroller);
    int pixel_shift = (int)fmodf(scroll_x_total, 18.0f);
    int scroller_y_base = win_height - 120 + glitch_y;
    
    for (int col = 0; col < 70; col++) {
        int char_idx = (base_char_idx + col) % strlen(decompressed_scroller);
        char ch = decompressed_scroller[char_idx];
        
        int lut_step = (col * 10 + (int)(retro_time * 250.0f)) & 0xFF;
        int dy = (int)(sine_lut[lut_step] * 15.0f);
        
        draw_char(pixels, win_width, win_height, 40 + col * 18 - pixel_shift + glitch_x, scroller_y_base + dy, ch, 0xFFFF5500, 3);
    }
    
    // Display header details
    draw_string(pixels, win_width, win_height, 100 + glitch_x, 30 + glitch_y, "AUNCIENT WAYLAND VULKAN MARKDOWN EDITOR", 0xFFFF8800, 2);
    
    // Render Simulated SID Chip register state array and active tune info
    char sid_buf[256];
    snprintf(sid_buf, sizeof(sid_buf), 
             "SID TUNE %d %s | FREQ=0x%04X PW=0x%04X ADSR=0x%02X%02X VOL=%d | SYNC:%s RM:%s | V1:%s V2:%s V3:%s", 
             active_tune, (active_tune == 3) ? "(HIDDEN UNLOCKED!)" : "(ACTIVE)",
             sid_chip.voices[0].freq, sid_chip.voices[1].pw,
             sid_chip.voices[0].adsr[0], sid_chip.voices[0].adsr[1],
             sid_chip.volume,
             sid_chip.hard_sync_enabled ? "ON" : "OFF",
             sid_chip.ring_mod_enabled ? "ON" : "OFF",
             voice_active[0] ? "ON" : "OFF",
             voice_active[1] ? "ON" : "OFF",
             voice_active[2] ? "ON" : "OFF");
    draw_string(pixels, win_width, win_height, 100 + glitch_x, win_height - 65 + glitch_y, sid_buf, 0xFFFFFF00, 2);

    // Initials Anagram Mapping: Alternate "TSN" and "TNS" dynamically every second
    const char *initials = ((int)retro_time % 2 == 0) ? "TSN" : "TNS";
    char help_buf[256];
    snprintf(help_buf, sizeof(help_buf), 
             "VIC-II: d012=%d - TRIBUTE: %s - PRESS '#' FOR TUNES | KEYS '1'-'3' TO MUTE VOICES", 
             vic_d012, initials);
    draw_string(pixels, win_width, win_height, 100 + glitch_x, win_height - 35 + glitch_y, help_buf, 0xFFFFCC00, 2);
    
    wl_surface_attach(surface, wl_buffers[current_buffer_idx], 0, 0);
    wl_surface_damage(surface, 0, 0, win_width, win_height);
    wl_surface_commit(surface);
}

// XDG Shell Base listeners
static void xdg_wm_base_handle_ping(void *data, struct xdg_wm_base *xdg_wm_base, uint32_t serial) {
    (void)data;
    xdg_wm_base_pong(xdg_wm_base, serial);
}

static const struct xdg_wm_base_listener xdg_wm_base_listener = {
    .ping = xdg_wm_base_handle_ping,
};

// XDG Surface listeners
static void xdg_surface_handle_configure(void *data, struct xdg_surface *xdg_surface, uint32_t serial) {
    (void)data;
    xdg_surface_ack_configure(xdg_surface, serial);
}

static const struct xdg_surface_listener xdg_surface_listener = {
    .configure = xdg_surface_handle_configure,
};

static void xdg_toplevel_handle_configure(void *data, struct xdg_toplevel *xdg_toplevel, int32_t width, int32_t height, struct wl_array *states) {
    (void)data; (void)xdg_toplevel; (void)states;
    if (width > 0 && height > 0) {
        win_width = width;
        win_height = height;
        redraw_screen();
    }
}

static void xdg_toplevel_handle_close(void *data, struct xdg_toplevel *xdg_toplevel) {
    (void)data; (void)xdg_toplevel;
    running = false;
}

static void xdg_toplevel_handle_configure_bounds(void *data, struct xdg_toplevel *xdg_toplevel, int32_t width, int32_t height) {
    (void)data; (void)xdg_toplevel; (void)width; (void)height;
}

static void xdg_toplevel_handle_wm_capabilities(void *data, struct xdg_toplevel *xdg_toplevel, struct wl_array *capabilities) {
    (void)data; (void)xdg_toplevel; (void)capabilities;
}

static const struct xdg_toplevel_listener xdg_toplevel_listener = {
    .configure = xdg_toplevel_handle_configure,
    .close = xdg_toplevel_handle_close,
    .configure_bounds = xdg_toplevel_handle_configure_bounds,
    .wm_capabilities = xdg_toplevel_handle_wm_capabilities,
};

// Keyboard listeners
static void keyboard_handle_keymap(void *data, struct wl_keyboard *wl_keyboard, uint32_t format, int32_t fd, uint32_t size) {
    (void)data; (void)wl_keyboard; (void)format; (void)size;
    close(fd);
}

static void keyboard_handle_enter(void *data, struct wl_keyboard *wl_keyboard, uint32_t serial, struct wl_surface *surface, struct wl_array *keys) {
    (void)data; (void)wl_keyboard; (void)serial; (void)surface; (void)keys;
}

static void keyboard_handle_leave(void *data, struct wl_keyboard *wl_keyboard, uint32_t serial, struct wl_surface *surface) {
    (void)data; (void)wl_keyboard; (void)serial; (void)surface;
}

static void keyboard_handle_key(void *data, struct wl_keyboard *wl_keyboard, uint32_t serial, uint32_t time, uint32_t key, uint32_t state) {
    (void)data; (void)wl_keyboard; (void)serial; (void)time;
    if (state == 0) return;

    if (key == 1) { // ESC key scancode -> Exit
        running = false;
        printf("[INFO] Escape key pressed. Editor shutting down...\n");
        return;
    }

    // Toggle simulated audio voices (scancodes: 2='1', 3='2', 4='3')
    if (key == 2) {
        voice_active[0] = !voice_active[0];
        printf("[AUDIO] Voice 1 toggled: %s\n", voice_active[0] ? "ON" : "OFF");
        redraw_screen();
        return;
    } else if (key == 3) {
        voice_active[1] = !voice_active[1];
        printf("[AUDIO] Voice 2 toggled: %s\n", voice_active[1] ? "ON" : "OFF");
        redraw_screen();
        return;
    } else if (key == 4) {
        voice_active[2] = !voice_active[2];
        printf("[AUDIO] Voice 3 toggled: %s\n", voice_active[2] ? "ON" : "OFF");
        redraw_screen();
        return;
    }

    // Keystroke activity acceleration
    type_activity += 1.2f;
    if (type_activity > 5.0f) type_activity = 5.0f;

    char typed_char = '\0';
    if (key == 28) { // Enter key -> Newline / Commit history transaction
        uint32_t hash = 2166136261U;
        for (int i = 0; i < doc_len; i++) {
            hash ^= (uint8_t)doc_buf[i];
            hash *= 16777619U;
        }
        
        EditorHistoryRecord rec = {
            .transaction_id = 0x4001,
            .state_code = doc_len,
            .action = 'E',
            .hash = hash
        };
        
        FILE *f = fopen("assets/editor_history.dat.bin", "ab");
        if (f) {
            fwrite(&rec, sizeof(rec), 1, f);
            fclose(f);
            printf("[LEDGER] Transaction committed to assets/editor_history.dat.bin\n");
            
            // Trigger 0.5s Fast Loader flash on write transaction
            loader_flash_time = 0.5f;
        }
        
        typed_char = '\n';
    } else if (key == 57) { // Space
        typed_char = ' ';
    } else if (key == 14) { // Backspace
        if (doc_len > 0) {
            doc_len--;
            doc_buf[doc_len] = '\0';
            redraw_screen();
        }
        return;
    } else {
        // Map common letter keys
        switch (key) {
            case 30: typed_char = 'a'; break;
            case 48: typed_char = 'b'; break;
            case 46: typed_char = 'c'; break;
            case 32: typed_char = 'd'; break;
            case 18: typed_char = 'e'; break;
            case 33: typed_char = 'f'; break;
            case 34: typed_char = 'g'; break;
            case 35: typed_char = 'h'; break;
            case 23: typed_char = 'i'; break;
            case 36: typed_char = 'j'; break;
            case 37: typed_char = 'k'; break;
            case 38: typed_char = 'l'; break;
            case 50: typed_char = 'm'; break;
            case 49: typed_char = 'n'; break;
            case 24: typed_char = 'o'; break;
            case 25: typed_char = 'p'; break;
            case 16: typed_char = 'q'; break;
            case 19: typed_char = 'r'; break;
            case 31: typed_char = 's'; break;
            case 20: typed_char = 't'; break;
            case 22: typed_char = 'u'; break;
            case 47: typed_char = 'v'; break;
            case 17: typed_char = 'w'; break;
            case 45: typed_char = 'x'; break;
            case 21: typed_char = 'y'; break;
            case 44: typed_char = 'z'; break;
            case 41: typed_char = '#'; break;
            case 52: typed_char = '>'; break;
            default: typed_char = '\0'; break;
        }
    }

    // Capture key history for secret code unlock ("tsn")
    if (typed_char >= 'a' && typed_char <= 'z') {
        key_history[0] = key_history[1];
        key_history[1] = key_history[2];
        key_history[2] = typed_char;
        
        if (key_history[0] == 't' && key_history[1] == 's' && key_history[2] == 'n') {
            hidden_unlocked = true;
            active_tune = 3; // Trigger Hidden Tune
            printf("[INFO] Hidden SID Tune 3 Unlocked! Warp Scroll Enabled.\n");
        }
    }

    // Cycled tune manually via '#' key
    if (typed_char == '#') {
        active_tune = (active_tune + 1) % 3; // Cycle normal tunes 0, 1, 2
        redraw_screen();
        return;
    }

    if (typed_char && doc_len < (int)sizeof(doc_buf) - 1) {
        doc_buf[doc_len++] = typed_char;
        doc_buf[doc_len] = '\0';
        
        // Trigger pitch sweeps on simulated SID Voice 1 when typing
        if (voice_active[0]) {
            sid_chip.voices[0].freq = 0x2200 + (doc_len * 32);
            sid_chip.voices[0].ctrl = 0x21; // Triangle waveform start
        }
        
        redraw_screen();
    }
}

static void keyboard_handle_modifiers(void *data, struct wl_keyboard *wl_keyboard, uint32_t serial, uint32_t mods_depressed, uint32_t mods_latched, uint32_t mods_locked, uint32_t group) {
    (void)data; (void)wl_keyboard; (void)serial; (void)mods_depressed; (void)mods_latched; (void)mods_locked; (void)group;
}

static void keyboard_handle_repeat_info(void *data, struct wl_keyboard *wl_keyboard, int32_t rate, int32_t delay) {
    (void)data; (void)wl_keyboard; (void)rate; (void)delay;
}

static const struct wl_keyboard_listener keyboard_listener = {
    .keymap = keyboard_handle_keymap,
    .enter = keyboard_handle_enter,
    .leave = keyboard_handle_leave,
    .key = keyboard_handle_key,
    .modifiers = keyboard_handle_modifiers,
    .repeat_info = keyboard_handle_repeat_info,
};

// Seat capabilities listener
static void seat_handle_capabilities(void *data, struct wl_seat *wl_seat, uint32_t capabilities) {
    (void)data;
    if (capabilities & WL_SEAT_CAPABILITY_KEYBOARD) {
        keyboard = wl_seat_get_keyboard(wl_seat);
        wl_keyboard_add_listener(keyboard, &keyboard_listener, NULL);
    }
}

static void seat_handle_name(void *data, struct wl_seat *wl_seat, const char *name) {
    (void)data; (void)wl_seat; (void)name;
}

static const struct wl_seat_listener seat_listener = {
    .capabilities = seat_handle_capabilities,
    .name = seat_handle_name,
};

// Registry handler
static void registry_handle_global(void *data, struct wl_registry *registry, uint32_t name, const char *interface, uint32_t version) {
    (void)data;
    if (strcmp(interface, "wl_compositor") == 0) {
        compositor = wl_registry_bind(registry, name, &wl_compositor_interface, version);
    } else if (strcmp(interface, "wl_shm") == 0) {
        shm = wl_registry_bind(registry, name, &wl_shm_interface, 1);
    } else if (strcmp(interface, "wl_seat") == 0) {
        seat = wl_registry_bind(registry, name, &wl_seat_interface, version);
        wl_seat_add_listener(seat, &seat_listener, NULL);
    } else if (strcmp(interface, "xdg_wm_base") == 0) {
        xdg_wm_base = wl_registry_bind(registry, name, &xdg_wm_base_interface, version);
        xdg_wm_base_add_listener(xdg_wm_base, &xdg_wm_base_listener, NULL);
    }
}

static void registry_handle_global_remove(void *data, struct wl_registry *registry, uint32_t name) {
    (void)data; (void)registry; (void)name;
}

static const struct wl_registry_listener registry_listener = {
    .global = registry_handle_global,
    .global_remove = registry_handle_global_remove,
};

int main(void) {
    srand(time(NULL));

    // Decruncher relocation simulation: copy payload with decompression logs
    printf("[DECRUNCH] Initiating in-place decompressed payload relocation...\n");
    char decrunch_temp[128];
    int de_idx = 0;
    for (int i = 0; i < 4; i++) {
        for (int k = 0; k < 30; k++) {
            if (compressed_scroller[k] == '\0') break;
            decrunch_temp[de_idx++] = compressed_scroller[k];
        }
    }
    decrunch_temp[de_idx] = '\0';
    memcpy(decompressed_scroller, decrunch_temp, de_idx + 1);
    printf("[DECRUNCH] Relocated %d bytes from 0x2000 to 0x0800\n", de_idx);

    // Precalculate Sine Lookup Table (LUT) covering full wave cycle
    for (int i = 0; i < 256; i++) {
        sine_lut[i] = sinf((float)i * (2.0f * M_PI / 256.0f));
    }

    // Initialize 3D Starfield coordinates
    for (int i = 0; i < 15; i++) {
        starfield[i].x = (float)((rand() % 200) - 100) / 10.0f;
        starfield[i].y = (float)((rand() % 200) - 100) / 10.0f;
        starfield[i].z = (float)(rand() % 100) / 10.0f + 0.5f;
        
        char glyphs[] = ".*+o";
        starfield[i].glyph = glyphs[rand() % 4];
    }

    // Initialize 2D horizontal scrolling background stars
    for (int i = 0; i < 30; i++) {
        scroll_stars[i].x = (float)(rand() % 1280);
        scroll_stars[i].y = rand() % 720;
        scroll_stars[i].speed = 15.0f + (float)(rand() % 45);
        char glyphs[] = ".*+";
        scroll_stars[i].glyph = glyphs[rand() % 3];
        scroll_stars[i].color = (scroll_stars[i].speed > 35.0f) ? 0xFFFFCC00 : 0xFF884400;
    }

    // Initialize simulated SID chip register defaults
    sid_chip.voices[0].adsr[0] = 0x21; // Attack / Decay
    sid_chip.voices[0].adsr[1] = 0xF5; // Sustain / Release
    sid_chip.voices[1].adsr[0] = 0x42;
    sid_chip.voices[1].adsr[1] = 0x99;
    sid_chip.volume = 0x0F;
    sid_chip.hard_sync_enabled = true;
    sid_chip.ring_mod_enabled = true;

    struct wl_display *display = wl_display_connect(NULL);
    if (!display) {
        fprintf(stderr, "[ERROR] Connect to Wayland compositor failed.\n");
        return 1;
    }

    struct wl_registry *registry = wl_display_get_registry(display);
    wl_registry_add_listener(registry, &registry_listener, NULL);
    
    wl_display_roundtrip(display);

    if (compositor && xdg_wm_base && shm) {
        surface = wl_compositor_create_surface(compositor);
        xdg_surface = xdg_wm_base_get_xdg_surface(xdg_wm_base, surface);
        xdg_surface_add_listener(xdg_surface, &xdg_surface_listener, NULL);

        xdg_toplevel = xdg_surface_get_toplevel(xdg_surface);
        xdg_toplevel_add_listener(xdg_toplevel, &xdg_toplevel_listener, NULL);

        wl_surface_commit(surface);
        wl_display_roundtrip(display);
        redraw_screen();
    }

    struct pollfd fds[1] = {
        { .fd = wl_display_get_fd(display), .events = POLLIN }
    };

    struct timespec start_ts;
    clock_gettime(CLOCK_MONOTONIC, &start_ts);
    float last_time = 0.0f;

    while (running) {
        if (wl_display_get_error(display)) {
            break;
        }
        
        while (wl_display_prepare_read(display) != 0) {
            wl_display_dispatch_pending(display);
        }
        wl_display_flush(display);
        
        int ret = poll(fds, 1, 16); // ~60 FPS update tick
        if (ret > 0) {
            if (fds[0].revents & POLLIN) {
                if (wl_display_read_events(display) < 0) {
                    break;
                }
                wl_display_dispatch_pending(display);
            } else {
                wl_display_cancel_read(display);
            }
        } else {
            wl_display_cancel_read(display);
        }
        
        // Progress active timeline based on real elapsed time
        struct timespec cur_ts;
        clock_gettime(CLOCK_MONOTONIC, &cur_ts);
        retro_time = (float)(cur_ts.tv_sec - start_ts.tv_sec) + 
                     (float)(cur_ts.tv_nsec - start_ts.tv_nsec) * 1e-9f;
        
        float dt = retro_time - last_time;
        last_time = retro_time;

        // Decrease active loader flash duration
        if (loader_flash_time > 0.0f) {
            loader_flash_time -= dt;
        }

        // Decay keystroke activity level
        if (type_activity > 0.0f) {
            type_activity -= dt * 2.5f;
        }

        // Animate 3D Starfield coordinates
        float star_speed = active_tune == 3 ? 6.0f : 2.5f; // Warp speed sync
        for (int i = 0; i < 15; i++) {
            starfield[i].z -= dt * star_speed;
            if (starfield[i].z <= 0.1f) {
                starfield[i].x = (float)((rand() % 200) - 100) / 10.0f;
                starfield[i].y = (float)((rand() % 200) - 100) / 10.0f;
                starfield[i].z = 10.0f; // Reset to far boundary
            }
        }
        // Animate 2D scrolling stars
        for (int i = 0; i < 30; i++) {
            scroll_stars[i].x -= dt * scroll_stars[i].speed;
            if (scroll_stars[i].x < 0.0f) {
                scroll_stars[i].x = (float)win_width;
                scroll_stars[i].y = rand() % win_height;
            }
        }

        // Run VIC-II screen shake displacement glitches only during typing activity to keep layout smooth
        if (type_activity > 1.0f && ((int)(retro_time * 12.0f) % 2 == 0)) {
            glitch_x = (rand() % 3) - 1; // Subtle horizontal bump
            glitch_y = (rand() % 3) - 1; // Subtle vertical bump
        } else {
            glitch_x = 0;
            glitch_y = 0;
        }

        // Run simulated 50Hz SID playroutine updates
        static float last_sid_tick = 0.0f;
        if (retro_time - last_sid_tick >= 0.020f) { // 50Hz Interval
            last_sid_tick = retro_time;
            
            // 3-note arpeggiator step loop based on active tune selection (Skip Voice 1 updates if muted)
            if (voice_active[0]) {
                sid_arp_step = (sid_arp_step + 1) % 3;
                sid_chip.voices[0].freq = sid_tunes[active_tune][sid_arp_step] + (int)(sinf(retro_time * 5.0f) * 200.0f);
            } else {
                sid_chip.voices[0].freq = 0;
            }
            
            // Hard Sync and Ring Modulation sweep simulation on Voice 2
            if (voice_active[1]) {
                uint16_t base_pw = 0x0400 + (uint16_t)(sinf(retro_time * 3.0f) * 512.0f + 512.0f);
                if (sid_chip.ring_mod_enabled && voice_active[0]) {
                    // Multiply/modulate Voice 2 frequency with Voice 1 frequency (Ring modulation)
                    sid_chip.voices[1].pw = base_pw + (sid_chip.voices[0].freq >> 2);
                } else {
                    sid_chip.voices[1].pw = base_pw;
                }
                
                // Hard sync: sweep frequency higher and reset phase
                if (sid_chip.hard_sync_enabled) {
                    sid_chip.voices[1].freq = sid_chip.voices[0].freq * 2;
                } else {
                    sid_chip.voices[1].freq = 0x1100;
                }
            } else {
                sid_chip.voices[1].pw = 0;
                sid_chip.voices[1].freq = 0;
            }
            
            // Logarithmic SID Volume Fader simulation
            int fade_index = ((int)(retro_time * 4.0f)) & 0x0F;
            sid_chip.volume = voice_active[2] ? log_volume_lut[fade_index] : 0; // Map volume logarithmic fade curve LUT
        }
        
        redraw_screen();
    }

    // Cleanup Wayland resources
    for (int i = 0; i < 2; i++) {
        if (wl_buffers[i]) wl_buffer_destroy(wl_buffers[i]);
    }
    if (xdg_toplevel) xdg_toplevel_destroy(xdg_toplevel);
    if (xdg_surface) xdg_surface_destroy(xdg_surface);
    if (surface) wl_surface_destroy(surface);
    if (keyboard) wl_keyboard_destroy(keyboard);
    if (seat) wl_seat_destroy(seat);
    if (compositor) wl_compositor_destroy(compositor);
    if (xdg_wm_base) xdg_wm_base_destroy(xdg_wm_base);
    if (shm) wl_shm_destroy(shm);
    wl_registry_destroy(registry);
    wl_display_disconnect(display);

    return 0;
}
