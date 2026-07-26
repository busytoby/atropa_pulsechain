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
    ['_'] = {0x40, 0x40, 0x40, 0x40, 0x40}
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
static int win_width = 1024;
static int win_height = 554;

// Doc
static char doc_buf[2048] = "# AUNCIENT MD EDITOR\n> TYPE YOUR DOCUMENT HERE\n";
static int doc_len = 46;

// ZL
static const char compressed_scroller[32] = "PAGADATA_2026_RETRO_C64_INTRO_";
static char decompressed_scroller[128];

// PL
static const char *parallax_scroller_text = "CONSPIRACY & SINGULAR HUNGARIAN TRIBUTE -- INTRODUCING MULTI-LAYER PARALLAX SCROLLER PATHS -- ";

// SL
static float sine_lut[256];

static const uint32_t color_cycle_lut[16] = {
    0xFF6A1B00, 0xFF8A2B00, 0xFFAA3B00, 0xFFCA4B00,
    0xFFEA5B00, 0xFFFF6B00, 0xFFFF8B20, 0xFFFFAB40,
    0xFFFF8B20, 0xFFFF6B00, 0xFFEA5B00, 0xFFCA4B00,
    0xFFAA3B00, 0xFF8A2B00, 0xFF6A1B00, 0xFF4A0A00
};

static const uint8_t log_volume_lut[16] = {
    0, 1, 1, 2, 2, 3, 4, 5, 6, 7, 9, 10, 12, 13, 14, 15
};

static const char charset_map_lut[128] = {
    ['a'] = 'A', ['b'] = 'B', ['c'] = 'C', ['d'] = 'D',
    ['e'] = 'E', ['f'] = 'F', ['g'] = 'G', ['h'] = 'H',
    ['i'] = 'I', ['j'] = 'J', ['k'] = 'K', ['l'] = 'L',
    ['m'] = 'M', ['n'] = 'N', ['o'] = 'O', ['p'] = 'P',
    ['q'] = 'Q', ['r'] = 'R', ['s'] = 'S', ['t'] = 'T',
    ['u'] = 'U', ['v'] = 'V', ['w'] = 'W', ['x'] = 'X',
    ['y'] = 'Y', ['z'] = 'Z'
};

static float retro_time = 0.0f;
static uint8_t vic_d012 = 130;
static uint8_t vic_d016 = 0;

static float loader_flash_time = 0.0f;
static float type_activity = 0.0f;

static int glitch_x = 0;
static int glitch_y = 0;

#include "auncient_wayland_audio.c"

// Pre-computed Morphological character frames (simulating pre-baked sprite assets)
typedef struct {
    uint16_t dilated[16];
    uint16_t eroded[16];
    uint16_t glints[16];
} MorphFrame;

static MorphFrame morph_cache[6][5];

static int current_water_idx = 0;
static int raymarch_mode = 0;
static int material_variant = 0;

static char key_history[3] = {'\0', '\0', '\0'};

typedef struct {
    uint32_t active_model;
    uint32_t material_variant;
    float rotation_angle;
    float camera_y;
    uint32_t starfield_count;
} USDStageRecord;

typedef struct {
    float rotation_angle;
    float camera_y;
    uint32_t active_model;
    float raster_intensity;
    uint32_t material_variant; // USD Material VariantSet selection!
    float padding[11];
} CoaxialUBO;

static CoaxialUBO active_ubo;

typedef struct {
    void (*render_scene)(uint32_t *pixels, int w, int h, const CoaxialUBO *ubo);
} HydraRenderDelegate;
#include "auncient_wayland_graphics.c"

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

typedef struct {
    float x;
    float y;
    float z;
    char glyph;
} Star;

static Star starfield[15];

typedef struct {
    float x;
    int y;
    float speed;
    char glyph;
    uint32_t color;
} ScrollStar;

static ScrollStar scroll_stars[30];

typedef struct {
    uint32_t transaction_id;
    uint32_t state_code;
    char action;
    uint32_t hash;
} EditorHistoryRecord;

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

static const uint8_t water_remove_raw[40][5] = {
    {96, 25, 0, 16, 255},
    {97, 25, 192, 15, 3},
    {98, 25, 192, 15, 3},
    {99, 25, 240, 15, 15},
    {100, 25, 252, 15, 63},
    {101, 25, 252, 15, 63},
    {110, 25, 0, 14, 255},
    {111, 25, 192, 13, 3},
    {168, 26, 192, 13, 3},
    {169, 26, 240, 13, 15},
    {170, 26, 252, 13, 63},
    {179, 26, 0, 12, 255},
    {180, 26, 192, 11, 3},
    {181, 26, 240, 11, 15},
    {182, 26, 252, 11, 63},
    {191, 26, 0, 10, 255},
    {248, 27, 192, 9, 3},
    {249, 27, 240, 9, 15},
    {2, 28, 0, 8, 255},
    {3, 28, 0, 8, 255},
    {4, 28, 0, 8, 255},
    {5, 28, 0, 8, 255},
    {6, 28, 0, 8, 255},
    {7, 28, 0, 8, 255},
    {64, 29, 0, 8, 255},
    {65, 29, 0, 8, 255},
    {66, 29, 0, 8, 255},
    {67, 29, 0, 8, 255},
    {68, 29, 0, 8, 255},
    {69, 29, 0, 8, 255},
    {70, 29, 0, 8, 255},
    {71, 29, 0, 8, 255},
    {128, 30, 0, 8, 255},
    {129, 30, 0, 8, 255},
    {130, 30, 0, 8, 255},
    {131, 30, 0, 8, 255},
    {132, 30, 0, 8, 255},
    {133, 30, 0, 8, 255},
    {134, 30, 0, 8, 255},
    {135, 30, 0, 8, 255}
};

// Simulated Self-Modifying Code (SMC) line remover and ZMM-style backup mirroring
static void simulate_smc_remove_next_line(uint32_t *active, uint32_t *backup, int win_w, int win_h, int ground, float flood_height) {
    if (flood_height <= 0.0f) return;
    
    // Determine table lookup index based on flood height
    int table_idx = (int)(flood_height * 1.5f);
    if (table_idx < 0) table_idx = 0;
    if (table_idx >= 40) table_idx = 39;
    current_water_idx = table_idx;
    
    // Extract parameters from raw water remove table configuration data
    uint8_t count = water_remove_raw[table_idx][3];
    uint32_t start_mask = ((uint32_t)water_remove_raw[table_idx][2] << 24) | 0x00FFFFFF;
    uint32_t end_mask = ((uint32_t)water_remove_raw[table_idx][4] << 24) | 0x00FFFFFF;
    
    int row_displacement = ground + 30 - (int)flood_height;
    if (row_displacement < 0 || row_displacement >= win_h) return;
    
    // Clear elements on both active screen and backup cache using simulated SMC loops and dynamic ripples
    for (int col = 0; col < win_w; col++) {
        // Calculate sinusoidal water ripple wave coordinates dynamically
        int ripple_y = row_displacement + (int)(sinf(retro_time * 6.0f + col * 0.06f) * 4.0f);
        if (ripple_y < 0) ripple_y = 0;
        if (ripple_y >= win_h) continue;
        
        int idx_boundary = ripple_y * win_w + col;
        
        if (col < 10) {
            active[idx_boundary] &= start_mask;
            backup[idx_boundary] &= start_mask;
        } else if (col < 10 + count * 8) {
            active[idx_boundary] = 0xFF051224; // Blue clear color at surface boundary
            backup[idx_boundary] = 0xFF051224;
        } else if (col < 10 + count * 8 + 10) {
            active[idx_boundary] &= end_mask;
            backup[idx_boundary] &= end_mask;
        }
        
        // Volumetric water body: Fill everything below the wavy surface with solid blue water
        for (int y = ripple_y + 1; y < win_h; y++) {
            active[y * win_w + col] = 0xFF051224;
            backup[y * win_w + col] = 0xFF051224;
        }
    }
}

// Pre-baked physics collision timeline frame structure mapping collision_20250902081037.inc
typedef struct {
    uint8_t delta_frame;
    uint8_t water_change;
    struct {
        uint8_t char_id;
        int16_t x;
        int16_t y;
    } updates[6];
    uint8_t update_count;
} PrebakedFrame;

static const PrebakedFrame prebaked_script[5] = {
    {0, 0, {{0, 100, 120}, {1, 160, 130}, {2, 220, 140}, {3, 280, 150}, {4, 340, 160}, {5, 400, 170}}, 6},
    {72, 0, {{1, 162, 148}, {2, 222, 148}, {3, 282, 148}}, 3},
    {13, 1, {{4, 342, 157}, {5, 402, 157}}, 2},
    {1, 1, {{0, 102, 145}, {1, 162, 145}}, 2},
    {1, 2, {{2, 222, 160}, {3, 282, 160}, {4, 342, 160}, {5, 402, 160}}, 4}
};

static inline float sd_capsule(float px, float py, float pz, float ax, float ay, float az, float bx, float by, float bz, float r) {
    float pax = px - ax, pay = py - ay, paz = pz - az;
    float bax = bx - ax, bay = by - ay, baz = bz - az;
    float h = (pax*bax + pay*bay + paz*baz) / (bax*bax + bay*bay + baz*baz);
    if (h < 0.0f) h = 0.0f;
    if (h > 1.0f) h = 1.0f;
    float dx = pax - bax * h;
    float dy = pay - bay * h;
    float dz = paz - baz * h;
    return sqrtf(dx*dx + dy*dy + dz*dz) - r;
}

static inline float sd_cactus(float px, float py, float pz) {
    float d1 = sd_capsule(px, py, pz, 0.0f, -3.5f, 0.0f, 0.0f, 3.5f, 0.0f, 0.8f);
    float d2 = sd_capsule(px, py, pz, 0.0f, 0.5f, 0.0f, -1.8f, 0.5f, 0.0f, 0.6f);
    float d3 = sd_capsule(px, py, pz, -1.8f, 0.5f, 0.0f, -1.8f, 2.5f, 0.0f, 0.6f);
    float d4 = sd_capsule(px, py, pz, 0.0f, -0.5f, 0.0f, 1.8f, -0.5f, 0.0f, 0.6f);
    float d5 = sd_capsule(px, py, pz, 1.8f, -0.5f, 0.0f, 1.8f, 1.5f, 0.0f, 0.6f);
    float d = d1;
    if (d2 < d) d = d2;
    if (d3 < d) d = d3;
    if (d4 < d) d = d4;
    if (d5 < d) d = d5;
    return d;
}

static inline float sd_letter_t(float px, float py, float pz) {
    float d1 = sd_capsule(px, py, pz, -1.5f, 2.0f, 0.0f, 1.5f, 2.0f, 0.0f, 0.5f);
    float d2 = sd_capsule(px, py, pz, 0.0f, -2.0f, 0.0f, 0.0f, 2.0f, 0.0f, 0.5f);
    return (d1 < d2) ? d1 : d2;
}

static inline uint32_t blend_color_add(uint32_t base, uint32_t add) {
    uint8_t r1 = (base >> 16) & 0xFF;
    uint8_t g1 = (base >> 8) & 0xFF;
    uint8_t b1 = base & 0xFF;
    
    uint8_t r2 = (add >> 16) & 0xFF;
    uint8_t g2 = (add >> 8) & 0xFF;
    uint8_t b2 = add & 0xFF;
    
    int r = r1 + r2; if (r > 255) r = 255;
    int g = g1 + g2; if (g > 255) g = 255;
    int b = b1 + b2; if (b > 255) b = 255;
    
    return (0xFF000000 | (r << 16) | (g << 8) | b);
}

static void hd_storm_render(uint32_t *pixels, int w, int h, const CoaxialUBO *ubo) {
    (void)ubo;
    for (int i = 0; i < 15; i++) {
        if (starfield[i].z > 0.1f) {
            int px = w / 2 + (int)((starfield[i].x / starfield[i].z) * (w / 2)) + glitch_x;
            int py = h / 2 + (int)((starfield[i].y / starfield[i].z) * (h / 2)) + glitch_y;
            
            if (px >= 0 && px < w && py >= 0 && py < h) {
                uint32_t star_color = 0xFF884400;
                if (starfield[i].z < 3.0f) star_color = 0xFFFFCC00;
                else if (starfield[i].z < 6.0f) star_color = 0xFFAA6600;
                
                draw_char(pixels, w, h, px, py, starfield[i].glyph, star_color, 2);
            }
        }
    }
}

static void hd_embree_render(uint32_t *pixels, int w, int h, const CoaxialUBO *ubo) {
    uint8_t ray_steps[32];
    float angle = ubo->rotation_angle;
    float cos_a = cosf(angle);
    float sin_a = sinf(angle);
    float ry = ubo->camera_y;
    int max_steps = 16;
    float abs_ry = fabsf(ry);
    if (abs_ry > 1.8f) {
        max_steps = 6;
    } else if (abs_ry > 1.0f) {
        max_steps = 10;
    }
    
    for (int col = 0; col < 32; col++) {
        float rx = (col - 15.5f) * 0.25f;
        float rz = -5.0f;
        int steps = 0;
        
        while (steps < max_steps) {
            steps++;
            float rot_x = rx * cos_a - rz * sin_a;
            float rot_z = rx * sin_a + rz * cos_a;
            
            float dist;
            if (ubo->active_model == 0) {
                dist = sd_cactus(rot_x, ry, rot_z);
            } else if (ubo->active_model == 1) {
                dist = sd_letter_t(rot_x, ry, rot_z);
            } else {
                float d_c = sd_cactus(rot_x, ry, rot_z);
                float d_l = sd_letter_t(rot_x, ry, rot_z);
                dist = (d_c < d_l) ? d_c : d_l;
            }
            
            if (dist < 0.05f) break;
            rz += dist;
            if (rz > 15.0f) { steps = max_steps; break; }
        }
        ray_steps[col] = (uint8_t)(steps * 31 / max_steps);
    }

    int osc_base_x = 100;
    int osc_base_y = 500;
    uint32_t trace_color = 0xFF00FFFF;
    if (ubo->material_variant == 1) {
        trace_color = 0xFFFF5555;
    } else if (ubo->material_variant == 2) {
        trace_color = 0xFFE6DFD3;
    }
    
    draw_string(pixels, w, h, osc_base_x + 480, osc_base_y - 15, "RAY", trace_color, 1);
    for (int col = 0; col < 32; col++) {
        int val = ray_steps[col];
        int py = osc_base_y - (val * 12 / 32);
        if (py >= 0 && py < h && (osc_base_x + 480 + col) < w) {
            pixels[py * w + osc_base_x + 480 + col] = trace_color;
        }
    }
}

static void redraw_screen(void) {
    if (!surface) return;
    
    struct timespec t_start;
    clock_gettime(CLOCK_MONOTONIC, &t_start);
    
    current_buffer_idx = 1 - current_buffer_idx;
    if (wl_buffers[current_buffer_idx]) {
        wl_buffer_destroy(wl_buffers[current_buffer_idx]);
    }
    
    uint32_t *pixels = NULL;
    wl_buffers[current_buffer_idx] = create_shm_buffer(win_width, win_height, &pixels);
    if (!wl_buffers[current_buffer_idx] || !pixels) return;
    
    static uint32_t *bg_cache = NULL;
    static int bg_cache_w = 0;
    static int bg_cache_h = 0;
    
    if (bg_cache == NULL || bg_cache_w != win_width || bg_cache_h != win_height) {
        if (bg_cache) free(bg_cache);
        bg_cache = malloc(win_width * win_height * sizeof(uint32_t));
        bg_cache_w = win_width;
        bg_cache_h = win_height;
        
        for (int y = 0; y < win_height; y++) {
            if (y >= 100 && y < 550 && (y % 24) == 0) {
                for (int x = 0; x < win_width; x++) {
                    bg_cache[y * win_width + x] = 0xFF000000;
                }
                continue;
            }
            
            uint32_t bg_color = 0xFF221100;
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
            
            for (int x = 0; x < win_width; x++) {
                bg_cache[y * win_width + x] = bg_color;
            }
        }
    }
    
    // Copy the background cache directly and overlay dynamic copper raster bars (Burglarized.prg style)
    for (int y = 0; y < win_height; y++) {
        int b1_dist = abs(y - (120 + (int)(sinf(retro_time * 1.8f) * 90.0f)));
        int b2_dist = abs(y - (240 + (int)(sinf(retro_time * 2.5f) * 70.0f)));
        int b3_dist = abs(y - (360 + (int)(sinf(retro_time * 1.3f) * 100.0f)));
        
        uint32_t add_color = 0;
        if (b1_dist < 15) {
            uint8_t intensity = (15 - b1_dist) * 8;
            add_color = blend_color_add(add_color, (intensity << 16) | ((intensity / 3) << 8)); // Glowing Red-Orange
        }
        if (b2_dist < 20) {
            uint8_t intensity = (20 - b2_dist) * 6;
            add_color = blend_color_add(add_color, ((intensity / 2) << 16) | (intensity << 8)); // Glowing Green-Gold
        }
        if (b3_dist < 12) {
            uint8_t intensity = (12 - b3_dist) * 12;
            add_color = blend_color_add(add_color, intensity); // Glowing Blue-Indigo
        }
        
        for (int x = 0; x < win_width; x++) {
            int idx = y * win_width + x;
            if (add_color != 0) {
                pixels[idx] = blend_color_add(bg_cache[idx], add_color);
            } else {
                pixels[idx] = bg_cache[idx];
            }
        }
    }
    
    // Draw scrolling background stars
    for (int i = 0; i < 30; i++) {
        int px = (int)scroll_stars[i].x + glitch_x;
        int py = scroll_stars[i].y + glitch_y;
        if (px >= 0 && px < win_width && py >= 0 && py < win_height) {
            draw_char(pixels, win_width, win_height, px, py, scroll_stars[i].glyph, scroll_stars[i].color, 1);
        }
    }

    static const HydraRenderDelegate hd_storm = { .render_scene = hd_storm_render };
    hd_storm.render_scene(pixels, win_width, win_height, &active_ubo);

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
            
            // Flexible Line Distance (FLD) accordion vertical stretching
            int fld_stretch = (int)(sinf(retro_time * 2.5f) * 3.0f * scale);
            int row_y = start_y + r * (8 * scale + fld_stretch);
            
            draw_char(pixels, win_width, win_height, 
                      start_x + c * 6 * scale + row_displace, 
                      row_y, 
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
    int char_spacing = 64; // Distance between characters (16 pixels * scale 4)
    
    // 6502-style C64 physics simulation
    static float sim_y[6] = {120.0f, 120.0f, 120.0f, 120.0f, 120.0f, 120.0f};
    static float sim_vy[6] = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
    static float sim_inflation[6] = {1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f};
    static float sim_scale_x[6] = {4.0f, 4.0f, 4.0f, 4.0f, 4.0f, 4.0f};
    static float sim_scale_y[6] = {4.0f, 4.0f, 4.0f, 4.0f, 4.0f, 4.0f};
    static float water_flood_height = 0.0f;
    static float last_sim_time = 0.0f;
    
    if (last_sim_time == 0.0f) {
        last_sim_time = retro_time;
    }
    float dt = retro_time - last_sim_time;
    if (dt > 0.1f) dt = 0.1f;
    last_sim_time = retro_time;
    
    int ground = 220;
    (void)sim_vy;
    
    // Playback pre-baked coordinate steps from the collision script
    static int prebaked_frame_idx = 0;
    static float last_frame_time = 0.0f;
    if (retro_time - last_frame_time > 0.12f) { // ~8 Hz update speed
        last_frame_time = retro_time;
        prebaked_frame_idx = (prebaked_frame_idx + 1) % 5;
        
        PrebakedFrame f = prebaked_script[prebaked_frame_idx];
        water_flood_height += f.water_change * 0.12f;
        if (water_flood_height > 25.0f) water_flood_height = 25.0f;
        
        for (int i = 0; i < f.update_count; i++) {
            int id = f.updates[i].char_id;
            if (id < 6) {
                sim_y[id] = f.updates[i].y;
                sim_scale_x[id] = 4.2f;
                sim_scale_y[id] = 3.8f;
                sim_inflation[id] = 2.2f;
            }
        }
    }
    
    // Smoothly decay scale parameters back to defaults
    for (int i = 0; i < 6; i++) {
        sim_inflation[i] += (1.0f - sim_inflation[i]) * dt * 2.0f;
        sim_scale_x[i] += (4.0f - sim_scale_x[i]) * dt * 4.0f;
        sim_scale_y[i] += (4.0f - sim_scale_y[i]) * dt * 4.0f;
    }
    
    // Apply simulated SMC remove_next_line to both active screen and backup cache
    simulate_smc_remove_next_line(pixels, bg_cache, win_width, win_height, ground, water_flood_height);

    // 1. Draw 3D Drop Shadow / Extrusion layer with matching wobble displacement
    for (int char_idx = 0; char_idx < 6; char_idx++) {
        int logo_start_y = (int)sim_y[char_idx];
        float elastic_scale_x = sim_scale_x[char_idx];
        float elastic_scale_y = sim_scale_y[char_idx];
        for (int r = 0; r < 16; r++) {
            uint16_t row_bits = bubble_font_tsfi2[char_idx][r];
            for (int c = 0; c < 16; c++) {
                if (row_bits & (1 << (15 - c))) {
                    int wobble_idx_x = (int)(retro_time * 90.0f + r * 12 + c * 6 + char_idx * 24) & 0xFF;
                    int wobble_idx_y = (int)(retro_time * 75.0f + r * 8 + c * 10 + char_idx * 30) & 0xFF;
                    int wobble_x = (int)(sine_lut[wobble_idx_x] * 3.0f);
                    int wobble_y = (int)(sine_lut[wobble_idx_y] * 3.0f);
                    
                    int pixel_x = logo_start_x + char_idx * char_spacing + c * elastic_scale_x + wobble_x + 6; // Shifted right
                    int pixel_y = logo_start_y + r * elastic_scale_y + wobble_y + 6; // Shifted down
                    for (int sy = 0; sy < (int)elastic_scale_y; sy++) {
                        for (int sx = 0; sx < (int)elastic_scale_x; sx++) {
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

    // 2. Draw Main Body with C64 Assembly-Aligned Morphological Bitwise Dilation, Erosion, and Specular Glints
    for (int char_idx = 0; char_idx < 6; char_idx++) {
        int logo_start_y = (int)sim_y[char_idx];
        float elastic_scale_x = sim_scale_x[char_idx];
        float elastic_scale_y = sim_scale_y[char_idx];
        
        // Look up pre-calculated morphological inflation stage frame from cache (simulating pre-baked sprite assets)
        int step_idx = (int)((sim_inflation[char_idx] - 1.0f) * 2.0f);
        if (step_idx < 0) step_idx = 0;
        if (step_idx > 4) step_idx = 4;
        
        MorphFrame *frame = &morph_cache[char_idx][step_idx];
        
        // Render characters based on bitwise morphological maps
        for (int r = 0; r < 16; r++) {
            for (int c = 0; c < 16; c++) {
                uint16_t mask = 1 << (15 - c);
                bool is_dilated = frame->dilated[r] & mask;
                if (!is_dilated) continue;
                
                int wobble_idx_x = (int)(retro_time * 90.0f + r * 12 + c * 6 + char_idx * 24) & 0xFF;
                int wobble_idx_y = (int)(retro_time * 75.0f + r * 8 + c * 10 + char_idx * 30) & 0xFF;
                int wobble_x = (int)(sine_lut[wobble_idx_x] * 3.0f);
                int wobble_y = (int)(sine_lut[wobble_idx_y] * 3.0f);
                
                int pixel_x = logo_start_x + char_idx * char_spacing + c * elastic_scale_x + wobble_x;
                int pixel_y = logo_start_y + r * elastic_scale_y + wobble_y;
                
                // Classify layer based on morphological level
                uint32_t pixel_color = 0x00000000;
                bool is_eroded = frame->eroded[r] & mask;
                bool is_source = bubble_font_tsfi2[char_idx][r] & mask;
                bool is_glint = frame->glints[r] & mask;
                
                if (is_glint) {
                    pixel_color = 0xFFFFFFFF; // High-gloss specular glint
                } else if (is_eroded) {
                    pixel_color = 0xFFFFCC00; // Inner Core (Gold)
                } else if (is_source) {
                    pixel_color = 0xFFFF6600; // Intermediate Slope (Orange)
                } else {
                    // Outer contour ring: Cycle color
                    int color_idx = (int)(retro_time * 15.0f + char_idx * 4 + c) & 0x0F;
                    pixel_color = color_cycle_lut[color_idx];
                }
                
                // Diagonal sheen sweep (from left to right)
                int sheen_pos = ((int)(retro_time * 200.0f)) % 900 - 200;
                int dist_to_sheen = abs((char_idx * char_spacing + c * 4 + r * 4) - sheen_pos);
                if (dist_to_sheen < 12) {
                    pixel_color = 0xFFFFFFFF;
                }
                
                int ground = 220;
                
                for (int sy = 0; sy < (int)elastic_scale_y; sy++) {
                    for (int sx = 0; sx < (int)elastic_scale_x; sx++) {
                        int px = pixel_x + sx;
                        int py = pixel_y + sy;
                        if (px >= 0 && px < win_width && py >= 0 && py < win_height) {
                            uint32_t *dest_pixel = &pixels[py * win_width + px];
                            uint32_t bg_val = bg_cache[py * win_width + px];
                            
                            // Horizon boundary mask
                            uint32_t boundary_mask = (py >= ground + 30) ? 0x00FFFFFF : 0xFFFFFFFF;
                            
                            if (is_glint && sx < 2 && sy < 2) {
                                *dest_pixel = (*dest_pixel & ~boundary_mask) | (0xFFFFFFFF & boundary_mask);
                            } else {
                                uint32_t blended_color = pixel_color;
                                if (boundary_mask != 0xFFFFFFFF) {
                                    blended_color = bg_val;
                                }
                                *dest_pixel = blended_color;
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
    
    for (int i = 0; i < 5; i++) {
        int idx = sorted_sprite_indices[i];
        draw_char(pixels, win_width, win_height, 
                  sprites[idx].x + glitch_x, 
                  sprites[idx].y + glitch_y, 
                  sprites[idx].glyph, 
                  sprites[idx].color, 3);
    }
    
    float p_scroll_speed = 60.0f;
    float p_scroll_x_total = retro_time * p_scroll_speed;
    int p_pixel_shift = (int)fmodf(p_scroll_x_total, 12.0f);
    int p_scroller_y = 60 + glitch_y;
    int len = strlen(parallax_scroller_text);
    int p_base_char_idx = len - ((int)(p_scroll_x_total / 12.0f) % len);
    
    for (int col = 0; col < 90; col++) {
        int char_idx = (p_base_char_idx + col) % len;
        char ch = parallax_scroller_text[char_idx];
        
        int dyn_scale = 2 + (int)(sinf(retro_time * 4.0f + col * 0.25f) * 0.6f);
        if (dyn_scale < 1) dyn_scale = 1;
        
        int px = 20 + col * 12 + p_pixel_shift + glitch_x;
        draw_char(pixels, win_width, win_height, px, p_scroller_y - (dyn_scale * 2), ch, 0xFFFFCC00, dyn_scale);
    }

    float scroll_x_speed = 30.0f;
    if (active_tune == 1) scroll_x_speed = 45.0f;
    else if (active_tune == 2) scroll_x_speed = 60.0f;
    else if (active_tune == 3) scroll_x_speed = 90.0f;
    
    float scroll_x_total = retro_time * scroll_x_speed;
    int base_char_idx = (int)(scroll_x_total / 24.0f) % strlen(decompressed_scroller);
    int pixel_shift = (int)fmodf(scroll_x_total, 24.0f);
    int scroller_y_base = win_height - 100 + glitch_y;
    
    int visible_cols = win_width / 24 + 2;
    for (int col = 0; col < visible_cols; col++) {
        int char_idx = (base_char_idx + col) % strlen(decompressed_scroller);
        char ch = decompressed_scroller[char_idx];
        
        int lut_step = (col * 10 + (int)(retro_time * 250.0f)) & 0xFF;
        int dy = (int)(sine_lut[lut_step] * 10.0f);
        
        uint32_t wash_color = color_cycle_lut[(col + (int)(retro_time * 20.0f)) & 0x0F];
        
        draw_char(pixels, win_width, win_height, 40 + col * 24 - pixel_shift + glitch_x, scroller_y_base + dy, ch, wash_color, 2);
    }
    
    draw_string(pixels, win_width, win_height, 100 + glitch_x, 30 + glitch_y, "AUNCIENT WAYLAND VULKAN MARKDOWN EDITOR", 0xFFFF8800, 2);
    
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
    draw_string(pixels, win_width, win_height, 100 + glitch_x, win_height - 65 + glitch_y, sid_buf, 0xFFFFFF00, 1);
 
    huc6280_psg.channels[0].freq = sid_chip.voices[0].freq / 2;
    huc6280_psg.channels[0].volume = (sid_chip.volume * 31) / 15;
    huc6280_psg.channels[0].pan = 0xDA;
    huc6280_psg.channels[1].freq = sid_chip.voices[1].freq / 2;
    huc6280_psg.channels[1].volume = (sid_chip.volume * 24) / 15;
    huc6280_psg.channels[1].pan = 0x24;
 
    for (int i = 0; i < 32; i++) {
        huc6280_psg.channels[0].waveform[i] = (uint8_t)(15.0f + 15.0f * sinf(i * 2.0f * M_PI / 32.0f));
        huc6280_psg.channels[1].waveform[i] = (uint8_t)(i < 16 ? i * 2 : (31 - i) * 2);
        huc6280_psg.channels[2].waveform[i] = (uint8_t)(i * 31 / 32);
        huc6280_psg.channels[3].waveform[i] = (uint8_t)(i < 16 ? 31 : 0);
        huc6280_psg.channels[4].waveform[i] = (uint8_t)((i + (int)(retro_time * 50.0f)) % 7 == 0 ? 25 : 5);
        huc6280_psg.channels[5].waveform[i] = (uint8_t)(i < 8 ? 31 : 0);
    }
    int osc_base_x = 120 + glitch_x;
    int osc_base_y = win_height - 80 + glitch_y;
    for (int c = 0; c < 6; c++) {
        // Draw oscilloscope label
        char osc_label[8];
        snprintf(osc_label, sizeof(osc_label), "CH%d", c + 1);
        draw_string(pixels, win_width, win_height, osc_base_x + c * 80, osc_base_y - 15, osc_label, 0xFF888888, 1);
        
        // Draw visual waveform line
        for (int col = 0; col < 32; col++) {
            int wave_val = huc6280_psg.channels[c].waveform[(col + (int)(retro_time * 40.0f)) % 32];
            int py = osc_base_y - (wave_val * 12 / 32); // Scale to 12px height
            if (py >= 0 && py < win_height && (osc_base_x + c * 80 + col) < win_width) {
                pixels[py * win_width + osc_base_x + c * 80 + col] = 0xFF00FF00; // Bright green scope line
            }
        }
    }
 
    // Populate Coaxial Uniform Buffer Object (UBO) fields
    active_ubo.rotation_angle = retro_time * 1.2f;
    active_ubo.camera_y = sinf(retro_time * 2.0f) * 2.2f;
    active_ubo.active_model = (uint32_t)raymarch_mode;
    active_ubo.raster_intensity = 1.0f;
    active_ubo.material_variant = (uint32_t)material_variant;
 
    // Invoke the composed Hydra Render Delegate to render 3D assets to screen
    static const HydraRenderDelegate hd_embree = { .render_scene = hd_embree_render };
    hd_embree.render_scene(pixels, win_width, win_height, &active_ubo);
 
    char psg_buf[256];
    snprintf(psg_buf, sizeof(psg_buf), 
             "HUDSON HUC6280 PSG | CH1: FREQ=0x%04X VOL=%2d PAN=0x%02X | CH2: FREQ=0x%04X VOL=%2d PAN=0x%02X",
             huc6280_psg.channels[0].freq, huc6280_psg.channels[0].volume, huc6280_psg.channels[0].pan,
             huc6280_psg.channels[1].freq, huc6280_psg.channels[1].volume, huc6280_psg.channels[1].pan);
    draw_string(pixels, win_width, win_height, 100 + glitch_x, win_height - 52 + glitch_y, psg_buf, 0xFF00FF00, 1);
 
    // Update TED registers dynamically
    ted_chip.ff09 = (ted_chip.ff09 + 1) & 0xFF; // Increment raster status flag (acknowledgment flow)
    ted_chip.ff0b = vic_d012; // Share VIC-II/TED raster scanline values
    ted_chip.ff15 = (uint8_t)(sid_chip.voices[1].freq >> 8);
    ted_chip.ff19 = (uint8_t)(sid_chip.voices[0].freq >> 8);
 
    char ted_buf[256];
    snprintf(ted_buf, sizeof(ted_buf),
             "TED MOS 8360 | ff06=0x%02X ff07=0x%02X ff09=0x%02X ff0b=%3d | WATER INDEX: %02d/40",
             ted_chip.ff06, ted_chip.ff07, ted_chip.ff09, ted_chip.ff0b,
             current_water_idx);
    draw_string(pixels, win_width, win_height, 100 + glitch_x, win_height - 39 + glitch_y, ted_buf, 0xFF00FFFF, 1);
 
    // Initials Anagram Mapping: Alternate "TSN" and "TNS" dynamically every second
    const char *initials = ((int)retro_time % 2 == 0) ? "TSN" : "TNS";
    
    struct timespec t_end;
    clock_gettime(CLOCK_MONOTONIC, &t_end);
    double latency_us = (t_end.tv_sec - t_start.tv_sec) * 1000000.0 + (t_end.tv_nsec - t_start.tv_nsec) / 1000.0;
    (void)latency_us;

    const char *m_names[3] = {"CAC", "LET", "UNI"};
    const char *v_names[3] = {"GLD", "CLY", "CHK"};
    char help_buf[256];
    snprintf(help_buf, sizeof(help_buf), 
             "VIC: d012=%d | RAY: %s (%s) | RAST: %3.0fus | 'B' VAR, 'S' SAVE | TR: %s", 
             vic_d012, m_names[raymarch_mode], v_names[material_variant], latency_us, initials);
    draw_string(pixels, win_width, win_height, 100 + glitch_x, win_height - 26 + glitch_y, help_buf, 0xFFFFCC00, 1);
    
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
    } else if (key == 50) { // 'M' key scancode
        raymarch_mode = (raymarch_mode + 1) % 3;
        printf("[RAY] Ray Marching Mode changed: %d\n", raymarch_mode);
        redraw_screen();
        return;
    } else if (key == 48) { // 'B' key scancode
        material_variant = (material_variant + 1) % 3;
        printf("[USD] Material Variant Set changed: %d\n", material_variant);
        redraw_screen();
        return;
    } else if (key == 31) { // 'S' key scancode
        USDStageRecord stage = {
            .active_model = (uint32_t)raymarch_mode,
            .material_variant = (uint32_t)material_variant,
            .rotation_angle = active_ubo.rotation_angle,
            .camera_y = active_ubo.camera_y,
            .starfield_count = 15
        };
        FILE *f_usd = fopen("assets/usd_stage.dat.bin", "wb");
        if (f_usd) {
            fwrite(&stage, sizeof(stage), 1, f_usd);
            fclose(f_usd);
            printf("[USD] Scene stage layer exported to assets/usd_stage.dat.bin\n");
            loader_flash_time = 0.5f;
        }
        redraw_screen();
        return;
    } else if (key == 4) {
        voice_active[2] = !voice_active[2];
        printf("[AUDIO] Voice 3 toggled: %s\n", voice_active[2] ? "ON" : "OFF");
        redraw_screen();
        return;
    }

    type_activity += 1.2f;
    if (type_activity > 5.0f) type_activity = 5.0f;

    char typed_char = '\0';
    if (key == 28) {
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
            
            loader_flash_time = 0.5f;
        }
        
        typed_char = '\n';
    } else if (key == 57) {
        typed_char = ' ';
    } else if (key == 14) {
        if (doc_len > 0) {
            doc_len--;
            doc_buf[doc_len] = '\0';
            redraw_screen();
        }
        return;
    } else {
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

    if (typed_char >= 'a' && typed_char <= 'z') {
        key_history[0] = key_history[1];
        key_history[1] = key_history[2];
        key_history[2] = typed_char;
        
        if (key_history[0] == 't' && key_history[1] == 's' && key_history[2] == 'n') {
            hidden_unlocked = true;
            active_tune = 3;
            printf("[INFO] Hidden SID Tune 3 Unlocked! Warp Scroll Enabled.\n");
        }
    }

    if (typed_char == '#') {
        active_tune = (active_tune + 1) % 3;
        redraw_screen();
        return;
    }

    if (typed_char && doc_len < (int)sizeof(doc_buf) - 1) {
        doc_buf[doc_len++] = typed_char;
        doc_buf[doc_len] = '\0';
        
        if (voice_active[0]) {
            sid_chip.voices[0].freq = 0x2200 + (doc_len * 32);
            sid_chip.voices[0].ctrl = 0x21;
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

    for (int i = 0; i < 256; i++) {
        sine_lut[i] = sinf((float)i * (2.0f * M_PI / 256.0f));
    }

    precompute_morph_cache();

    FILE *f_usd_load = fopen("assets/usd_stage.dat.bin", "rb");
    if (f_usd_load) {
        USDStageRecord stage;
        if (fread(&stage, sizeof(stage), 1, f_usd_load) == 1) {
            raymarch_mode = (int)stage.active_model;
            material_variant = (int)stage.material_variant;
            printf("[USD] Deserialized saved scene stage layer from assets/usd_stage.dat.bin\n");
        }
        fclose(f_usd_load);
    } else {
        USDStageRecord default_stage = {
            .active_model = 0,
            .material_variant = 0,
            .rotation_angle = 0.0f,
            .camera_y = 0.0f,
            .starfield_count = 15
        };
        FILE *f_usd_write = fopen("assets/usd_stage.dat.bin", "wb");
        if (f_usd_write) {
            fwrite(&default_stage, sizeof(default_stage), 1, f_usd_write);
            fclose(f_usd_write);
            printf("[USD] Created default scene stage containing Cactus asset at assets/usd_stage.dat.bin\n");
        }
    }

    FILE *f_usd_overlay = fopen("assets/usd_overlay.dat.bin", "rb");
    if (f_usd_overlay) {
        USDStageRecord overlay;
        if (fread(&overlay, sizeof(overlay), 1, f_usd_overlay) == 1) {
            raymarch_mode = (int)overlay.active_model;
            material_variant = (int)overlay.material_variant;
            printf("[USD] Composition Overlay Layer applied overrides from assets/usd_overlay.dat.bin\n");
        }
        fclose(f_usd_overlay);
    }

    for (int i = 0; i < 15; i++) {
        starfield[i].x = (float)((rand() % 200) - 100) / 10.0f;
        starfield[i].y = (float)((rand() % 200) - 100) / 10.0f;
        starfield[i].z = (float)(rand() % 100) / 10.0f + 0.5f;
        
        char glyphs[] = ".*+o";
        starfield[i].glyph = glyphs[rand() % 4];
    }

    for (int i = 0; i < 30; i++) {
        scroll_stars[i].x = (float)(rand() % 1280);
        scroll_stars[i].y = rand() % 720;
        scroll_stars[i].speed = 15.0f + (float)(rand() % 45);
        char glyphs[] = ".*+";
        scroll_stars[i].glyph = glyphs[rand() % 3];
        scroll_stars[i].color = (scroll_stars[i].speed > 35.0f) ? 0xFFFFCC00 : 0xFF884400;
    }

    sid_chip.voices[0].adsr[0] = 0x21;
    sid_chip.voices[0].adsr[1] = 0xF5;
    sid_chip.voices[1].adsr[0] = 0x42;
    sid_chip.voices[1].adsr[1] = 0x99;
    sid_chip.volume = 0x0F;
    sid_chip.hard_sync_enabled = true;
    sid_chip.ring_mod_enabled = true;

    ted_chip.ff06 = 0x28;
    ted_chip.ff07 = 0x18;
    ted_chip.ff12 = 0x20;
    ted_chip.ff14 = 0x78;
    ted_chip.ff16 = 0x41;

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
        
        int ret = poll(fds, 1, 16);
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
        
        struct timespec cur_ts;
        clock_gettime(CLOCK_MONOTONIC, &cur_ts);
        retro_time = (float)(cur_ts.tv_sec - start_ts.tv_sec) + 
                     (float)(cur_ts.tv_nsec - start_ts.tv_nsec) * 1e-9f;
        
        float dt = retro_time - last_time;
        last_time = retro_time;

        if (loader_flash_time > 0.0f) {
            loader_flash_time -= dt;
        }

        if (type_activity > 0.0f) {
            type_activity -= dt * 2.5f;
        }

        float star_speed = active_tune == 3 ? 6.0f : 2.5f;
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
