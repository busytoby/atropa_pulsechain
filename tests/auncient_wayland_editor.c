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

// Retro demo animation parameters
static float retro_time = 0.0f;
static const char *scroller_text = "PAGADATA 2026 RETRO C64 INTRO -- TISSEPAUSE BY SINATRA BRINGS PETSCII LINE AND CHARACTER BASED DRAWING BACK -- ";

// VIC-II Simulated Register state maps
static uint8_t vic_d012 = 130;  // Raster split scanline
static uint8_t vic_d016 = 0;    // Fine scroll shift (0-7 pixels)
static uint8_t vic_d020 = 0x05; // Border Color (Purple)
static uint8_t vic_d021 = 0x06; // Background Color (Blue)

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
} sid_chip;

static int sid_arp_step = 0;

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
    
    // Simulate VIC-II raster scanline color splits (Teledeltos C64 style)
    for (int y = 0; y < win_height; y++) {
        uint32_t bg_color = 0xFF222222; // default
        
        // Raster split scanline evaluation (using vic_d012 register simulation)
        if (y < (int)vic_d012 * 3) {
            bg_color = (vic_d020 == 0x05) ? 0xFF3a1a4a : 0xFF220033; // Border color split 1
        } else {
            bg_color = (vic_d021 == 0x06) ? 0xFF1b253d : 0xFF112222; // Background color split 2
            
            // Dynamic raster wave bands
            float wave = sinf((float)y * 0.015f + retro_time * 6.0f);
            if (wave > 0.85f) {
                bg_color = 0xFF440055;
            }
        }
        
        for (int x = 0; x < win_width; x++) {
            pixels[y * win_width + x] = bg_color;
        }
    }
    
    // Calculate dynamic scaling factor
    int scale = win_width / 280;
    if (scale < 1) scale = 1;
    if (scale > 6) scale = 6;
    
    // Center document grid dynamically
    int grid_w = 40 * 6 * scale;
    int grid_h = 15 * 8 * scale;
    int start_x = (win_width - grid_w) / 2;
    int start_y = (win_height - grid_h) / 2;
    if (start_x < 20) start_x = 20;
    if (start_y < 80) start_y = 80;
    
    // Parse Markdown to ANSI grid (40 columns, 15 rows)
    char ansi_grid[600];
    memset(ansi_grid, ' ', sizeof(ansi_grid));
    auncient_parse_markdown_to_ansi(doc_buf, ansi_grid, 40, 15, 0);
    
    // Render the parsed ANSI grid onto the pixel canvas
    for (int r = 0; r < 15; r++) {
        // PETSCII-Style Border Split Transitions (using sin + dynamic vic_d016 scroll shift)
        int fine_shift = (vic_d016 & 0x07);
        int row_displace = (int)(sinf((float)r * 0.6f + retro_time * 4.0f) * 3.0f * scale) + fine_shift;
        
        for (int c = 0; c < 40; c++) {
            char ch = ansi_grid[r * 40 + c];
            uint32_t color = 0xFFCCCCCC;
            
            if (ch == '|') color = 0xFF00FFFF;
            else if (ch == '*') color = 0xFFFF00FF;
            else if (ch == '=') color = 0xFFFFFF00;
            
            draw_char(pixels, win_width, win_height, 
                      start_x + c * 6 * scale + row_displace, 
                      start_y + r * 8 * scale, 
                      ch, color, scale);
        }
    }
    
    // Dynamic scroller speed linked to the active SID tune selection (Interactive speed sync)
    float scroll_x_speed = 30.0f;
    if (active_tune == 1) scroll_x_speed = 45.0f;
    else if (active_tune == 2) scroll_x_speed = 60.0f;
    else if (active_tune == 3) scroll_x_speed = 90.0f; // Hidden Warp speed
    
    float scroll_x_total = retro_time * scroll_x_speed;
    int base_char_idx = (int)(scroll_x_total / 18.0f) % strlen(scroller_text);
    int pixel_shift = (int)fmodf(scroll_x_total, 18.0f);
    int scroller_y_base = win_height - 120;
    
    for (int col = 0; col < 70; col++) {
        int char_idx = (base_char_idx + col) % strlen(scroller_text);
        char ch = scroller_text[char_idx];
        int dy = (int)(sinf((float)col * 0.25f + retro_time * 10.0f) * 15.0f);
        draw_char(pixels, win_width, win_height, 40 + col * 18 - pixel_shift, scroller_y_base + dy, ch, 0xFF00FF00, 3);
    }
    
    // Display header details
    draw_string(pixels, win_width, win_height, 100, 30, "AUNCIENT WAYLAND VULKAN MARKDOWN EDITOR", 0xFF00FF00, 2);
    
    // Render Simulated SID Chip register state array and active tune info
    char sid_buf[256];
    snprintf(sid_buf, sizeof(sid_buf), 
             "SID TUNE %d %s | FREQ=0x%04X PW=0x%04X ADSR=0x%02X%02X", 
             active_tune, (active_tune == 3) ? "(HIDDEN UNLOCKED!)" : "(ACTIVE)",
             sid_chip.voices[0].freq, sid_chip.voices[1].pw,
             sid_chip.voices[0].adsr[0], sid_chip.voices[0].adsr[1]);
    draw_string(pixels, win_width, win_height, 100, win_height - 65, sid_buf, 0xFF00FFFF, 2);

    char help_buf[256];
    snprintf(help_buf, sizeof(help_buf), 
             "SCALE: %d - VIC-II: d012=%d d016=%d - PRESS '#' TO CYCLE TUNES - TYPE 'TSN'", 
             scale, vic_d012, vic_d016);
    draw_string(pixels, win_width, win_height, 100, win_height - 35, help_buf, 0xFFFFFF00, 2);
    
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
        sid_chip.voices[0].freq = 0x2200 + (doc_len * 32);
        sid_chip.voices[0].ctrl = 0x21; // Triangle waveform start
        
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
    // Initialize simulated SID chip register defaults
    sid_chip.voices[0].adsr[0] = 0x21; // Attack / Decay
    sid_chip.voices[0].adsr[1] = 0xF5; // Sustain / Release
    sid_chip.voices[1].adsr[0] = 0x42;
    sid_chip.voices[1].adsr[1] = 0x99;
    sid_chip.volume = 0x0F;

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
        
        // Mutate simulated VIC-II hardware register states
        vic_d016 = (uint8_t)(retro_time * 8.0f) & 0x07; // 0-7 pixel fine scroll shift
        vic_d012 = 120 + (uint8_t)(sinf(retro_time * 2.0f) * 20.0f); // Modulate raster split line
        
        // Run simulated 50Hz SID playroutine updates
        static float last_sid_tick = 0.0f;
        if (retro_time - last_sid_tick >= 0.020f) { // 50Hz Interval
            last_sid_tick = retro_time;
            
            // 3-note arpeggiator step loop based on active tune selection
            sid_arp_step = (sid_arp_step + 1) % 3;
            sid_chip.voices[0].freq = sid_tunes[active_tune][sid_arp_step] + (int)(sinf(retro_time * 5.0f) * 200.0f);
            
            // Pulse Width LFO sweep modulation on Voice 2
            sid_chip.voices[1].pw = 0x0400 + (uint16_t)(sinf(retro_time * 3.0f) * 512.0f + 512.0f);
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
