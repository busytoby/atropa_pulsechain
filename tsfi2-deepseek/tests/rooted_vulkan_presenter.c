#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/mman.h>
#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include <poll.h>
#include <jpeglib.h>
#include <setjmp.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/stat.h>

#include "node_interop.h"
#include "tsfi_jpeg_encoder.h"
#include "lau_memory.h"
#include "tsfi_raw.h"
#include "window_inc/vulkan_struct.h"
#include "tsfi_dom.h"
#include "tsfi_zmm_vm.h"
#include "../src/lau_yul_thunk.h"

extern PFN_vkVoidFunction tsfi_vkGetInstanceProcAddr(VkInstance instance, const char* pName);
extern void tsfi_wire_firmware_init(void);
void tsfi_dl_thunks_teardown(void);
void lau_registry_teardown(void);
void lau_free_all_active(void);

extern bool tsfi_mozilla_wmq_bridge_init(const char *so_path);
extern void tsfi_mozilla_wmq_bridge_tick(TsfiZmmVmState *vm_state);
extern void tsfi_mozilla_wmq_bridge_destroy(void);

typedef int (VKAPI_CALL *PFN_tsfi_drmModeAddPlane)(uint32_t plane_id, size_t buffer_size);
typedef void* (VKAPI_CALL *PFN_tsfi_drmModeGetVirtualPlaneBuffer)(uint32_t plane_id);
typedef void (VKAPI_CALL *PFN_tsfi_zmm_set_scanout_buffer)(void* ptr, int w, int h);
typedef void (VKAPI_CALL *PFN_tsfi_drmModeFreeVirtualPlanes)(void);

#include "rooted_wayland_events.h"

extern const uint8_t *g_active_video_frame;
extern int g_active_video_w;
extern int g_active_video_h;

#include "tsfi_media_decode.h"
TsfiMediaDecoder *g_video_decoder = NULL;
uint8_t *g_video_rgba_buf = NULL;
int g_video_w = 0;
int g_video_h = 0;

TsfiZmmVmState g_zmm_vm;
pthread_mutex_t g_zmm_vm_mutex = PTHREAD_MUTEX_INITIALIZER;

static void fill_hex_uint256(char *dest, uint64_t val) {
    memset(dest, '0', 64);
    char tmp[32];
    int len = snprintf(tmp, sizeof(tmp), "%lx", val);
    memcpy(dest + 64 - len, tmp, len);
    dest[64] = '\0';
}

#define MAX_W 3840
#define MAX_H 2160

static int g_dest_x = 410;
static int g_dest_y = 80;
static int g_dest_w = 0;
static int g_dest_h = 0;

void commit_wayland_surface(int fd_wl, uint32_t surf, uint32_t bid) {
    uint32_t attach_args[] = {bid, 0, 0};
    send_msg(fd_wl, surf, WL_SURFACE_ATTACH, attach_args, 12, -1);
    uint32_t damage[] = {0, 0, g_w, g_h};
    send_msg(fd_wl, surf, WL_SURFACE_DAMAGE, damage, 16, -1);
    send_msg(fd_wl, surf, WL_SURFACE_COMMIT, NULL, 0, -1);
}

void run_rooted_zmm_tick() {
    if (!g_scanout_px) return;
    
    char cmd[1024];
    char hex_mx[65], hex_my[65], hex_click[65];
    
    uint16_t vm_mx = (uint16_t)((float)g_mouse_x * 320.0f / (float)g_w);
    uint16_t vm_my = (uint16_t)((float)g_mouse_y * 300.0f / (float)g_h);
    
    fill_hex_uint256(hex_mx, vm_mx);
    fill_hex_uint256(hex_my, vm_my);
    fill_hex_uint256(hex_click, g_click_state);

    snprintf(cmd, sizeof(cmd), "YULEXEC \"MicroUI\", \"8b5c90d2%s%s%s\"", hex_mx, hex_my, hex_click);
    tsfi_zmm_vm_exec(&g_zmm_vm, cmd);
}

// Monochrome 8x8 font for rendering text directly without parser/HTML
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

static void draw_char(uint32_t *pixels, int w, int h, int rx, int ry, unsigned char c, uint32_t color, int scale) {
    if (c < 32 || c > 127) return;
    int idx = c - 32;
    int char_w = 8 * scale;
    int char_h = 8 * scale;
    for (int dy = 0; dy < char_h; dy++) {
        int y = ry + dy;
        if (y < 0 || y >= h) continue;
        uint32_t *row_pixels = pixels + y * w;
        int src_y = dy / scale;
        uint8_t row_bits = font_8x8[idx][src_y & 7];
        for (int dx = 0; dx < char_w; dx++) {
            int x = rx + dx;
            if (x < 0 || x >= w) continue;
            int src_x = dx / scale;
            if ((row_bits >> (7 - src_x)) & 1) {
                row_pixels[x] = color;
            }
        }
    }
}

static void draw_text(uint32_t *pixels, int w, int h, int rx, int ry, const char *text, uint32_t color, int scale) {
    int cur_x = rx;
    while (*text) {
        draw_char(pixels, w, h, cur_x, ry, *text, color, scale);
        cur_x += 8 * scale;
        text++;
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

TsfiNode *g_dashboard_dom = NULL;

static void render_native_dashboard(uint32_t *pixels, int w, int h) {
    static int g_demo_tick = 0;
    g_demo_tick = (g_demo_tick + 1) % 2000;

    // 1. Draw solid dark background (#0f0f0f -> 0xFF0F0F0F)
    draw_rect(pixels, w, h, 0, 0, w, h, 0xFF0F0F0F);

    // 2. YouTube Header (y=0 to y=60)
    // Logo text "YouTube" in red
    draw_text(pixels, w, h, 20, 15, "YouTube", 0xFFFF0000, 3);
    // Search input background outline/box at x=250, y=10, w=500, h=40
    draw_rect(pixels, w, h, 250, 10, 500, 40, 0xFF121212);
    // Draw search text inside search box
    const char *query = "";
    if (g_demo_tick >= 500) {
        query = "atropa";
    } else {
        if (g_demo_tick >= 360) query = "atropa";
        else if (g_demo_tick >= 300) query = "atrop";
        else if (g_demo_tick >= 240) query = "atro";
        else if (g_demo_tick >= 180) query = "atr";
        else if (g_demo_tick >= 120) query = "at";
        else if (g_demo_tick >= 60) query = "a";
    }
    draw_text(pixels, w, h, 260, 20, query, 0xFFF1F1F1, 2);
    // Search button background at x=760, y=10, w=80, h=40
    draw_rect(pixels, w, h, 760, 10, 80, 40, 0xFF303030);
    draw_text(pixels, w, h, 772, 22, "Search", 0xFFFFFFFF, 1);
    
    // Bottom line of header
    draw_rect(pixels, w, h, 0, 60, w, 1, 0xFF2F2F2F);

    // 3. Filter Bar (y=70 to y=110)
    if (g_demo_tick >= 1000) {
        // Active filter badge (#FF0000 background)
        draw_rect(pixels, w, h, 20, 75, 160, 30, 0xFFFF0000);
        draw_text(pixels, w, h, 25, 83, "Filter: This Week", 0xFFFFFFFF, 1);
        draw_text(pixels, w, h, 200, 83, "124 results found this week for \"atropa\"", 0xFFAAAAAA, 1);
    } else {
        draw_rect(pixels, w, h, 20, 75, 100, 30, 0xFF303030);
        draw_text(pixels, w, h, 35, 83, "Filters", 0xFFFFFFFF, 1);
        draw_text(pixels, w, h, 140, 83, "About 4,210 results for \"atropa\"", 0xFFAAAAAA, 1);
    }

    // 4. Main content drawing (Videos dashboard list)
    if (g_demo_tick < 500) {
        // STATE_TYPING: Big single video display
        // Thumbnail (video box at x=20, y=130, w=640, h=360)
        if (g_active_video_frame) {
            draw_video_frame(pixels, w, h, 20, 130, 640, 360, g_active_video_frame, g_active_video_w, g_active_video_h);
        } else {
            draw_rect(pixels, w, h, 20, 130, 640, 360, 0xFF1A1A1A);
        }
        draw_text(pixels, w, h, 20, 510, "Atropa Pulsechain: The Helmholtz Web Launch", 0xFFFFFFFF, 3);
        draw_text(pixels, w, h, 20, 545, "12K views * Oct 15, 2026", 0xFFAAAAAA, 2);
    } 
    else if (g_demo_tick >= 500 && g_demo_tick < 1000) {
        // STATE_RESULTS: Search results list
        // Video 1
        draw_rect(pixels, w, h, 20, 130, 950, 120, 0xFF111111);
        // Video 1 thumbnail
        draw_rect(pixels, w, h, 30, 135, 200, 110, 0xFF333333);
        draw_text(pixels, w, h, 250, 150, "Atropa Pulsechain: Complete Auncient Guide (2026)", 0xFFFFFFFF, 2);
        draw_text(pixels, w, h, 250, 185, "Auncient Tech * 85K views * 3 months ago", 0xFFAAAAAA, 1);

        // Video 2
        draw_rect(pixels, w, h, 20, 270, 950, 120, 0xFF111111);
        // Video 2 thumbnail
        draw_rect(pixels, w, h, 30, 275, 200, 110, 0xFF333333);
        draw_text(pixels, w, h, 250, 290, "Atropa DNA Tracker Tutorial", 0xFFFFFFFF, 2);
        draw_text(pixels, w, h, 250, 325, "Auncient Tech * 12K views * 2 weeks ago", 0xFFAAAAAA, 1);
    }
    else if (g_demo_tick >= 1000 && g_demo_tick < 1500) {
        // STATE_FILTER_SHOWN: Upload date dropdown sidebar
        draw_rect(pixels, w, h, 20, 130, 980, 140, 0xFF161616);
        draw_text(pixels, w, h, 35, 145, "UPLOAD DATE", 0xFFAAAAAA, 1);
        draw_text(pixels, w, h, 35, 175, "Today", 0xFFFFFFFF, 1);
        draw_text(pixels, w, h, 35, 205, "This week <-", 0xFFFF0000, 1);
    }
    else {
        // STATE_FILTERED_RESULTS: Videos from this week
        // Video 1 (Atropa Pulsechain: New Epoch Launches This Week)
        draw_rect(pixels, w, h, 20, 130, 950, 120, 0xFF111111);
        if (g_active_video_frame) {
            draw_video_frame(pixels, w, h, 30, 135, 200, 110, g_active_video_frame, g_active_video_w, g_active_video_h);
        } else {
            draw_rect(pixels, w, h, 30, 135, 200, 110, 0xFF333333);
        }
        draw_text(pixels, w, h, 250, 150, "Atropa Pulsechain: Segment 0", 0xFFFF0000, 2);
        draw_text(pixels, w, h, 250, 185, "Auncient Tech * 3.2K views * 2 days ago * NEW", 0xFFAAAAAA, 1);

        // Video 2 (Pulsechain: Atropa's Helmholtz Resonance this week)
        draw_rect(pixels, w, h, 20, 270, 950, 120, 0xFF111111);
        draw_rect(pixels, w, h, 30, 275, 200, 110, 0xFF333333);
        draw_text(pixels, w, h, 250, 290, "Pulsechain: Atropa's Helmholtz Resonance this week", 0xFFFFFFFF, 2);
        draw_text(pixels, w, h, 250, 325, "Helmholtz Spec * 850 views * 5 days ago * NEW", 0xFFAAAAAA, 1);
    }
}

bool update_and_present(int fd_wl, uint32_t surf, uint32_t bid, bool force_redraw) {
    (void)force_redraw;
    if (g_scanout_px) {
        if (is_projector) {
            for (int i = 0; i < g_w * g_h; i++) {
                g_scanout_px[i] = 0xFF000000;
            }
        } else {
            render_native_dashboard(g_scanout_px, g_w, g_h);
        }
        
        pthread_mutex_lock(&g_zmm_vm_mutex);
        run_rooted_zmm_tick();
        tsfi_mozilla_wmq_bridge_tick(&g_zmm_vm);
        pthread_mutex_unlock(&g_zmm_vm_mutex);

        // Runtime validation against the expected YouTube frame
        if (g_active_video_frame && g_active_video_w > 0 && g_active_video_h > 0) {
            static int log_ticks = 0;
            if (log_ticks++ % 30 == 0) {
                // Save current screenshot to latest_frame.jpg for automated validation verification
                uint8_t *rgb24 = (uint8_t *)malloc(g_w * g_h * 3);
                if (rgb24) {
                    for (int i = 0; i < g_w * g_h; i++) {
                        uint32_t pixel = g_scanout_px[i];
                        rgb24[i * 3 + 0] = (pixel >> 16) & 0xFF; // R
                        rgb24[i * 3 + 1] = (pixel >> 8) & 0xFF;  // G
                        rgb24[i * 3 + 2] = pixel & 0xFF;         // B
                    }
                    unsigned char *jpeg_buf = NULL;
                    unsigned long jpeg_size = 0;
                    int enc_res = tsfi_jpeg_encode(&jpeg_buf, &jpeg_size, rgb24, g_w, g_h, 90);
                    if (enc_res == 0) {
                        const char *abs_path = "/home/mariarahel/src/tsfi2/atropa_pulsechain/frontend/latest_frame.jpg";
                        FILE *f = fopen(abs_path, "wb");
                        if (f) {
                            fwrite(jpeg_buf, 1, jpeg_size, f);
                            fclose(f);
                            printf("[Auncient Presenter] Saved screenshot to %s (%lu bytes)\n", abs_path, jpeg_size);
                        }
                        tsfi_free_buffer(jpeg_buf);
                    }
                    free(rgb24);
                }
            }
        }
        
        commit_wayland_surface(fd_wl, surf, bid);
        return true;
    }
    return false;
}

static void* run_presenter_thread(void* arg) {
    (void)arg;
    tsfi_wire_firmware_init();

    printf("[Auncient Presenter] Initializing global ZMM VM...\n");
    fflush(stdout);
    tsfi_zmm_vm_init(&g_zmm_vm);

    tsfi_zmm_vm_exec(&g_zmm_vm, "YULINIT \"MicroUI\", \"../solidity/bin/microui.yul\", 256");
    tsfi_zmm_vm_exec(&g_zmm_vm, "YULINIT \"MicroUI\", \"solidity/bin/microui.yul\", 256");
    tsfi_zmm_vm_exec(&g_zmm_vm, "YULINIT \"WinchesterMQ\", \"../solidity/bin/WinchesterMQ.yul\", 0xccb077a0");
    tsfi_zmm_vm_exec(&g_zmm_vm, "YULINIT \"WinchesterMQ\", \"solidity/bin/WinchesterMQ.yul\", 0xccb077a0");
    tsfi_zmm_vm_exec(&g_zmm_vm, "YULINIT \"laufactory\", \"../solidity/bin/laufactory.yul\", 0x0EB4EE7d5Ff28cbF68565A174f7E5e186c36B4b3");
    tsfi_zmm_vm_exec(&g_zmm_vm, "YULINIT \"laufactory\", \"solidity/bin/laufactory.yul\", 0x0EB4EE7d5Ff28cbF68565A174f7E5e186c36B4b3");
    
    tsfi_zmm_vm_exec(&g_zmm_vm, "YULEXEC \"laufactory\", \"c55c7075"
        "0000000000000000000000000000000000000000000000000000000000000040" // Offset of name
        "0000000000000000000000000000000000000000000000000000000000000080" // Offset of symbol
        "000000000000000000000000000000000000000000000000000000000000000e" // Length of name "ROOTED Browser" (14 bytes)
        "524f4f5445442042726f77736572000000000000000000000000000000000000" // Data "ROOTED Browser"
        "0000000000000000000000000000000000000000000000000000000000000006" // Length of symbol "ROOTED" (6 bytes)
        "524f4f5445440000000000000000000000000000000000000000000000000000\""); // Data "ROOTED"
    
    printf("[Auncient Presenter] ZMM VM diagnostics, WinchesterMQ, and legal immutable LAU ROOTED Browser username setup completed.\n");
    fflush(stdout);

    printf("[Auncient Presenter] Initializing Mozilla WMQ bridge...\n");
    tsfi_mozilla_wmq_bridge_init("./libtsfi2.so");

    PFN_vkCreateInstance pvkCreateInstance = (PFN_vkCreateInstance)tsfi_vkGetInstanceProcAddr(NULL, "vkCreateInstance");
    PFN_vkCreateDevice pvkCreateDevice = (PFN_vkCreateDevice)tsfi_vkGetInstanceProcAddr(NULL, "vkCreateDevice");
    PFN_vkGetDeviceQueue pvkGetDeviceQueue = (PFN_vkGetDeviceQueue)tsfi_vkGetInstanceProcAddr(NULL, "vkGetDeviceQueue");
    PFN_tsfi_zmm_set_scanout_buffer ptsfi_zmm_set_scanout_buffer = (PFN_tsfi_zmm_set_scanout_buffer)tsfi_vkGetInstanceProcAddr(NULL, "tsfi_zmm_set_scanout_buffer");

    if (!pvkCreateInstance || !pvkCreateDevice || !pvkGetDeviceQueue || !ptsfi_zmm_set_scanout_buffer) {
        fprintf(stderr, "[Auncient Presenter ERR] Failed to resolve critical Vulkan/ZMM function pointers!\n");
        return NULL;
    }

    VkInstance instance; VkInstanceCreateInfo inst_info = {0}; pvkCreateInstance(&inst_info, NULL, &instance);
    VkDevice device; VkDeviceCreateInfo dev_info = {0}; pvkCreateDevice((VkPhysicalDevice)0x2000, &dev_info, NULL, &device);
    VkQueue queue; pvkGetDeviceQueue(device, 0, 0, &queue);

    const char *run = getenv("XDG_RUNTIME_DIR"), *disp = getenv("WAYLAND_DISPLAY");
    if (!run) return NULL; 
    if (!disp) disp = "wayland-0";
    char path[108]; snprintf(path, 108, "%s/%s", run, disp);
    int fd = socket(AF_UNIX, SOCK_STREAM, 0);
    struct sockaddr_un addr = {0}; addr.sun_family = AF_UNIX;
    snprintf(addr.sun_path, sizeof(addr.sun_path), "%s", path);
    if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) return NULL;

    uint32_t reg_args[] = {WL_REGISTRY_ID}; send_msg(fd, WL_DISPLAY_ID, WL_DISPLAY_GET_REGISTRY, reg_args, 4, -1);
    for (int i = 0; i < 100; i++) { process_events(fd, 0, NULL); tsfi_raw_usleep(2000); }
    if (!global_xdg_id) return NULL;

    uint32_t cid = next_id++, sid = next_id++, xid = next_id++;
    xdg_wm_base_id = xid;
    uint32_t *b = (uint32_t*)malloc(256); int off;
    off = 0; b[off++] = global_compositor_id; off += pack_string(&b[off], "wl_compositor"); b[off++] = 4; b[off++] = cid;
    send_msg(fd, WL_REGISTRY_ID, WL_REGISTRY_BIND, b, off * 4, -1);
    off = 0; b[off++] = global_shm_id; off += pack_string(&b[off], "wl_shm"); b[off++] = 1; b[off++] = sid;
    send_msg(fd, WL_REGISTRY_ID, WL_REGISTRY_BIND, b, off * 4, -1);
    off = 0; b[off++] = global_xdg_id; off += pack_string(&b[off], "xdg_wm_base"); b[off++] = 1; b[off++] = xid;
    send_msg(fd, WL_REGISTRY_ID, WL_REGISTRY_BIND, b, off * 4, -1);

    if (global_seat_name) {
        seat_id = next_id++;
        off = 0;
        b[off++] = global_seat_name;
        off += pack_string(&b[off], "wl_seat");
        b[off++] = 1; // version
        b[off++] = seat_id;
        send_msg(fd, WL_REGISTRY_ID, WL_REGISTRY_BIND, b, off * 4, -1);

        pointer_id = next_id++;
        uint32_t p_args[] = {pointer_id};
        send_msg(fd, seat_id, 0, p_args, 4, -1); // get_pointer

        keyboard_id = next_id++;
        uint32_t k_args[] = {keyboard_id};
        send_msg(fd, seat_id, 1, k_args, 4, -1); // get_keyboard

        for (int i = 0; i < 50; i++) {
            process_events(fd, 0, NULL);
            tsfi_raw_usleep(2000);
        }
    }

    surf_id = next_id++;
    uint32_t surf = surf_id, xsurf = next_id++, top = next_id++;
    uint32_t s_args[] = {surf}; send_msg(fd, cid, WL_COMPOSITOR_CREATE_SURFACE, s_args, 4, -1);
    uint32_t xs_args[] = {xsurf, surf}; send_msg(fd, xid, XDG_WM_BASE_GET_XDG_SURFACE, xs_args, 8, -1);
    uint32_t t_args[] = {top}; send_msg(fd, xsurf, XDG_SURFACE_GET_TOPLEVEL, t_args, 4, -1);

    uint32_t *title_buf = (uint32_t*)malloc(256);
    int title_off = 0;
    title_off += pack_string(title_buf, "Auncient Web");
    send_msg(fd, top, 2, title_buf, title_off * 4, -1); // 2 = set_title
    
    int app_off = 0;
    app_off += pack_string(title_buf, "auncient-web");
    send_msg(fd, top, 3, title_buf, app_off * 4, -1); // 3 = set_app_id
    free(title_buf);

    int str = g_w * 4; size_t sz = MAX_W * MAX_H * 4;
    int mfd = memfd_create("tsfi_scanout", MFD_CLOEXEC);
    if (mfd < 0 || ftruncate(mfd, sz) < 0) return NULL;
    uint32_t *scanout_px = (uint32_t*)mmap(NULL, sz, PROT_READ|PROT_WRITE, MAP_SHARED, mfd, 0);
    for (int i = 0; i < MAX_W * MAX_H; i++) {
        scanout_px[i] = 0xFF0B0214; 
    }
    
    pid_val = next_id++; bid_val = next_id++;
    uint32_t p_args[] = {pid_val, (uint32_t)sz}; send_msg(fd, sid, WL_SHM_CREATE_POOL, p_args, 8, mfd);
    uint32_t bf_args[] = {bid_val, 0, (uint32_t)g_w, (uint32_t)g_h, (uint32_t)str, 1}; send_msg(fd, pid_val, WL_SHM_POOL_CREATE_BUFFER, bf_args, 24, -1); // 1 = XRGB8888
    
    send_msg(fd, surf, WL_SURFACE_COMMIT, NULL, 0, -1);

    g_scanout_px = scanout_px;
    g_ptsfi_zmm_set_scanout_buffer = ptsfi_zmm_set_scanout_buffer;
    ptsfi_zmm_set_scanout_buffer(scanout_px, g_w, g_h);

    VkSwapchainKHR mock_swapchain = (VkSwapchainKHR)0xB000;
    uint32_t imageIndex = 0;
    VkPresentInfoKHR presentInfo = {0};
    presentInfo.sType = (VkStructureType)8; 
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &mock_swapchain;
    presentInfo.pImageIndices = &imageIndex;
    (void)presentInfo;
    printf("[Auncient Presenter] Waiting for initial Wayland configure event...\n");
    bool initial_configured = false;
    for (int i = 0; i < 100; i++) {
        process_events(fd, xsurf, &initial_configured);
        if (initial_configured) {
            uint32_t a_args[] = {bid_val, 0, 0};
            send_msg(fd, surf, WL_SURFACE_ATTACH, a_args, 12, -1);
            render_native_dashboard(scanout_px, g_w, g_h);
            send_msg(fd, surf, WL_SURFACE_COMMIT, NULL, 0, -1);
            break;
        }
        tsfi_raw_usleep(10000);
    }

    printf("[Auncient Presenter] Frame presenter ready. Streaming starting...\n");
    fflush(stdout);

    if (is_projector) {
        int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
        fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);
    }

    struct pollfd fds[2] = {0};
    fds[0].fd = fd;
    fds[0].events = POLLIN;
    fds[1].fd = is_projector ? STDIN_FILENO : -1;
    fds[1].events = POLLIN;

    enum {
        STATE_READ_LEN,
        STATE_READ_DATA,
        STATE_READ_CMD
    } stream_state = STATE_READ_LEN;

    size_t target_len = 20;
    size_t total_read = 0;
    static uint8_t input_buf[2 * 1024 * 1024]; 
    uint32_t jpeg_len = 0;

    g_video_decoder = tsfi_media_decoder_create("segment_0.mp4");
    if (!g_video_decoder) {
        g_video_decoder = tsfi_media_decoder_create("tsfi2-deepseek/segment_0.mp4");
    }
    if (g_video_decoder) {
        g_video_rgba_buf = (uint8_t *)malloc(1920 * 1080 * 4);
        if (g_video_rgba_buf) {
            int vw = 0, vh = 0;
            for (int i = 0; i < 300; i++) {
                tsfi_media_decoder_next_frame(g_video_decoder, &vw, &vh, g_video_rgba_buf);
            }
            g_video_w = vw;
            g_video_h = vh;
            g_active_video_frame = g_video_rgba_buf;
            g_active_video_w = vw;
            g_active_video_h = vh;

            long non_zero = 0;
            for (int i = 0; i < vw * vh * 4; i++) {
                if (g_video_rgba_buf[i] > 10) non_zero++;
            }
            printf("[Auncient Presenter] Fast-forwarded frame: %dx%d, non-zero bytes (val > 10): %ld\n", vw, vh, non_zero);
        }
    }

    while (1) {
        extern bool g_video_playing;
        extern double g_video_current_time;
        extern double g_video_duration;

        if (g_video_playing && g_video_decoder && g_video_rgba_buf) {
            int vw = 0, vh = 0;
            if (tsfi_media_decoder_next_frame(g_video_decoder, &vw, &vh, g_video_rgba_buf)) {
                g_video_w = vw;
                g_video_h = vh;
                g_active_video_frame = g_video_rgba_buf;
                g_active_video_w = vw;
                g_active_video_h = vh;
                g_input_dirty = true;
                g_video_current_time += 0.033; 
                if (g_video_current_time > g_video_duration) {
                    g_video_current_time = 0.0;
                }
            } else {
                tsfi_media_decoder_destroy(g_video_decoder);
                g_video_decoder = tsfi_media_decoder_create("segment_0.mp4");
                if (!g_video_decoder) {
                    g_video_decoder = tsfi_media_decoder_create("tsfi2-deepseek/segment_0.mp4");
                }
                g_video_current_time = 0.0;
            }
        }

        int ret = poll(fds, 2, 10);
        if (ret < 0) {
            if (errno == EINTR) continue;
            break;
        }

        if (fds[0].revents & POLLIN) {
            bool conf = false;
            int pev;
            while ((pev = process_events(fd, xsurf, &conf)) > 0) {
                // drain events
            }
            if (conf) {
                update_and_present(fd, surf, bid_val, true);
            }
        }

        if (fds[1].revents & POLLIN) {
            ssize_t bytes = read(STDIN_FILENO, input_buf + total_read, target_len - total_read);
            if (bytes < 0) {
                if (errno != EAGAIN && errno != EWOULDBLOCK) {
                    fprintf(stderr, "[Auncient Presenter] STDIN read error: %s\n", strerror(errno));
                    goto out;
                }
            } else if (bytes == 0) {
                printf("[Auncient Presenter] STDIN EOF reached. Exiting loop.\n");
                goto out;
            } else {
                total_read += bytes;
                if (total_read >= target_len) {
                    if (stream_state == STATE_READ_LEN) {
                        memcpy(&jpeg_len, input_buf, 4);
                        memcpy(&g_dest_x, input_buf + 4, 4);
                        memcpy(&g_dest_y, input_buf + 8, 4);
                        memcpy(&g_dest_w, input_buf + 12, 4);
                        memcpy(&g_dest_h, input_buf + 16, 4);

                        if (jpeg_len & 0x80000000) {
                            uint32_t payload_len = jpeg_len & 0x7FFFFFFF;
                            if (payload_len > sizeof(input_buf)) {
                                fprintf(stderr, "[Auncient Presenter ERR] Command payload length %u exceeds buffer size!\n", payload_len);
                                goto out;
                            }
                            stream_state = STATE_READ_CMD;
                            target_len = payload_len;
                            total_read = 0;
                        } else {
                            stream_state = STATE_READ_LEN;
                            target_len = 20;
                            total_read = 0;
                        }
                    } else if (stream_state == STATE_READ_DATA) {
                        stream_state = STATE_READ_LEN;
                        target_len = 20;
                        total_read = 0;
                    } else if (stream_state == STATE_READ_CMD) {
                        if (target_len < sizeof(input_buf)) {
                            input_buf[target_len] = '\0';
                        } else {
                            input_buf[sizeof(input_buf) - 1] = '\0';
                        }
                        FILE *py_proc = popen("python3 scripts/set_clipboard.py", "w");
                        if (py_proc) {
                            fwrite(input_buf, 1, target_len, py_proc);
                            pclose(py_proc);
                        }
                        stream_state = STATE_READ_LEN;
                        target_len = 20;
                        total_read = 0;
                    }
                }
            }
        }
        if (g_input_dirty) {
            g_input_dirty = false;
            update_and_present(fd, surf, bid_val, true);
        } else {
            update_and_present(fd, surf, bid_val, false);
        }
    }
out:
    printf("[Auncient Presenter] Cleaning up virtual planes...\n");
    free(b);
    PFN_vkDestroyDevice pvkDestroyDevice = (PFN_vkDestroyDevice)tsfi_vkGetInstanceProcAddr(NULL, "vkDestroyDevice");
    pvkDestroyDevice(device, NULL);
    PFN_vkDestroyInstance pvkDestroyInstance = (PFN_vkDestroyInstance)tsfi_vkGetInstanceProcAddr(NULL, "vkDestroyInstance");
    pvkDestroyInstance(instance, NULL);

    tsfi_zmm_vm_destroy(&g_zmm_vm);
    tsfi_dl_thunks_teardown();
    lau_registry_teardown();
    lau_free_all_active();
    return NULL;
}

int main(int argc, char **argv) {
    is_projector = false;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--projector") == 0) {
            is_projector = true;
        }
    }
    g_dest_x = 0;
    g_dest_y = 0;
    g_dest_w = g_w;
    g_dest_h = g_h;
    printf("[Auncient Presenter] Initializing Vulkan and Wayland loops... (projector=%d)\n", is_projector);

    pthread_t presenter_thread_handle;
    if (pthread_create(&presenter_thread_handle, NULL, run_presenter_thread, NULL) != 0) {
        fprintf(stderr, "[Auncient Presenter ERR] Failed to create presenter loop thread!\n");
        return 1;
    }
    pthread_detach(presenter_thread_handle);

    printf("[Auncient Presenter] Spawning embedded Node.js engine on main thread...\n");
    const char *script_path = "scripts/embedded_browser_controller.js";
    if (access(script_path, F_OK) != 0) {
        script_path = "../scripts/embedded_browser_controller.js";
    }
    char* args[] = { (char*)"node", (char*)script_path, NULL };
    start_embedded_node(2, args);
    printf("[Auncient Presenter] Embedded Node.js engine exited.\n");
    return 0;
}
