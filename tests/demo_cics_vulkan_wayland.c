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
// Input buffer
static char input_buf[64] = "INPUT_VAL";
static int input_len = 9;

// Vulkan dynamic bindings
static void *vulkan_lib = NULL;
static void *vk_instance = NULL;
static void *vk_surface_khr = NULL;

typedef int (*PFN_vkCreateInstance)(const void *pCreateInfo, const void *pAllocator, void **pInstance);
typedef int (*PFN_vkCreateWaylandSurfaceKHR)(void *instance, const void *pCreateInfo, const void *pAllocator, void **pSurface);
typedef void (*PFN_vkDestroySurfaceKHR)(void *instance, void *surface, const void *pAllocator);
typedef void (*PFN_vkDestroyInstance)(void *instance, const void *pAllocator);

static PFN_vkCreateInstance pfn_vkCreateInstance = NULL;
static PFN_vkCreateWaylandSurfaceKHR pfn_vkCreateWaylandSurfaceKHR = NULL;
static PFN_vkDestroySurfaceKHR pfn_vkDestroySurfaceKHR = NULL;
static PFN_vkDestroyInstance pfn_vkDestroyInstance = NULL;

// SHM Buffer Helper
static struct wl_buffer *create_shm_buffer(int width, int height, uint32_t **out_pixels) {
    int stride = width * 4;
    int size = stride * height;
    
    int fd = memfd_create("shm-cics", MFD_CLOEXEC);
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

// Redraw current window contents
static void redraw_screen(void) {
    if (!surface) return;
    
    // Cycle double buffers
    current_buffer_idx = 1 - current_buffer_idx;
    if (wl_buffers[current_buffer_idx]) {
        wl_buffer_destroy(wl_buffers[current_buffer_idx]);
    }
    
    uint32_t *pixels = NULL;
    wl_buffers[current_buffer_idx] = create_shm_buffer(win_width, win_height, &pixels);
    if (!wl_buffers[current_buffer_idx] || !pixels) return;
    
    // Background fill (CICS Blue)
    for (int i = 0; i < win_width * win_height; i++) {
        pixels[i] = 0xFF0000AA;
    }
    
    // Draw boundary frames
    for (int y = 20; y < win_height - 20; y++) {
        for (int x = 20; x < win_width - 20; x++) {
            if (y < 25 || y > win_height - 25 || x < 25 || x > win_width - 25) {
                pixels[y * win_width + x] = 0xFFFFFFFF;
            }
        }
    }
    
    // Render static CICS IBM 3270 text fields
    draw_string(pixels, win_width, win_height, 100, 80, "IBM 3270 CICS ONLINE SESSION", 0xFFFFFF00, 3);
    draw_string(pixels, win_width, win_height, 100, 150, "ENTER TRANSACTION PARAMETERS:", 0xFFFFFFFF, 2);
    
    draw_string(pixels, win_width, win_height, 100, 220, "TRANS ID  ===> [ 0x3001 ]", 0xFF00FF00, 2);
    draw_string(pixels, win_width, win_height, 100, 270, "REC KEY   ===> [ INPUT_BUFF ]", 0xFF00FF00, 2);
    draw_string(pixels, win_width, win_height, 100, 320, "ACTION    ===> [ W ]", 0xFF00FF00, 2);
    
    // Display prompt input buffer
    char prompt_display[128];
    snprintf(prompt_display, sizeof(prompt_display), "INPUT BUFFER ===> [ %s_ ]", input_buf);
    draw_string(pixels, win_width, win_height, 100, 420, prompt_display, 0xFFFF00FF, 2);
    
    draw_string(pixels, win_width, win_height, 100, 500, "PRESS ANY KEY TO TYPE - PRESS ENTER TO TRANSMIT CICS BLOCK", 0xFFFFFF00, 2);
    
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
        printf("[XDG] Resizing window buffer to: %dx%d\n", width, height);
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
    (void)data; (void)wl_keyboard; (void)format; (void)fd; (void)size;
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

    // Map scancodes to simple ascii chars
    char typed_char = '\0';
    if (key >= 2 && key <= 11) {
        typed_char = (key == 11) ? '0' : (char)('1' + (key - 2));
    } else if (key == 28) { // Enter key -> Commit Transaction
        printf("[CICS] Transmitting transaction block payload!\n");
        auncient_autodin_audit_edit(input_buf, input_len, 0, 'W');
        auncient_cics_process_transaction(0x3001, "INPUT_BUFF", 'W');
        input_len = 0;
        input_buf[0] = '\0';
        redraw_screen();
        return;
    } else if (key == 14) { // Backspace
        if (input_len > 0) {
            input_len--;
            input_buf[input_len] = '\0';
            redraw_screen();
        }
        return;
    } else {
        // Map common letter keys
        switch (key) {
            case 30: typed_char = 'A'; break;
            case 48: typed_char = 'B'; break;
            case 46: typed_char = 'C'; break;
            case 32: typed_char = 'D'; break;
            case 18: typed_char = 'E'; break;
            case 33: typed_char = 'F'; break;
            case 34: typed_char = 'G'; break;
            case 35: typed_char = 'H'; break;
            case 23: typed_char = 'I'; break;
            case 36: typed_char = 'J'; break;
            case 37: typed_char = 'K'; break;
            case 38: typed_char = 'L'; break;
            case 50: typed_char = 'M'; break;
            case 49: typed_char = 'N'; break;
            case 24: typed_char = 'O'; break;
            case 25: typed_char = 'P'; break;
            case 16: typed_char = 'Q'; break;
            case 19: typed_char = 'R'; break;
            case 31: typed_char = 'S'; break;
            case 20: typed_char = 'T'; break;
            case 22: typed_char = 'U'; break;
            case 47: typed_char = 'V'; break;
            case 17: typed_char = 'W'; break;
            case 45: typed_char = 'X'; break;
            case 21: typed_char = 'Y'; break;
            case 44: typed_char = 'Z'; break;
            default: typed_char = '_'; break;
        }
    }

    if (typed_char && input_len < (int)sizeof(input_buf) - 1) {
        input_buf[input_len++] = typed_char;
        input_buf[input_len] = '\0';
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

// Dynamic Vulkan loading
static bool init_vulkan_dynamic(struct wl_display *wl_disp, struct wl_surface *wl_surf) {
    vulkan_lib = dlopen("libvulkan.so.1", RTLD_NOW);
    if (!vulkan_lib) {
        vulkan_lib = dlopen("libvulkan.so", RTLD_NOW);
    }
    if (!vulkan_lib) return false;

    pfn_vkCreateInstance = (PFN_vkCreateInstance)dlsym(vulkan_lib, "vkCreateInstance");
    pfn_vkCreateWaylandSurfaceKHR = (PFN_vkCreateWaylandSurfaceKHR)dlsym(vulkan_lib, "vkCreateWaylandSurfaceKHR");
    pfn_vkDestroySurfaceKHR = (PFN_vkDestroySurfaceKHR)dlsym(vulkan_lib, "vkDestroySurfaceKHR");
    pfn_vkDestroyInstance = (PFN_vkDestroyInstance)dlsym(vulkan_lib, "vkDestroyInstance");

    if (!pfn_vkCreateInstance || !pfn_vkCreateWaylandSurfaceKHR || !pfn_vkDestroySurfaceKHR || !pfn_vkDestroyInstance) {
        dlclose(vulkan_lib);
        vulkan_lib = NULL;
        return false;
    }

    const char *extensions[] = {
        "VK_KHR_surface",
        "VK_KHR_wayland_surface"
    };

    struct {
        uint32_t sType;
        const void *pNext;
        const char *pApplicationName;
        uint32_t applicationVersion;
        const char *pEngineName;
        uint32_t engineVersion;
        uint32_t apiVersion;
    } app_info = {
        .sType = 4,
        .pApplicationName = "CICS Terminal",
        .apiVersion = (1 << 22)
    };

    struct {
        uint32_t sType;
        const void *pNext;
        uint32_t flags;
        const void *pApplicationInfo;
        uint32_t enabledLayerCount;
        const char *const *ppEnabledLayerNames;
        uint32_t enabledExtensionCount;
        const char *const *ppEnabledExtensionNames;
    } inst_info = {
        .sType = 1,
        .pApplicationInfo = &app_info,
        .enabledExtensionCount = 2,
        .ppEnabledExtensionNames = extensions
    };

    if (pfn_vkCreateInstance(&inst_info, NULL, &vk_instance) != 0) {
        return false;
    }

    struct {
        uint32_t sType;
        const void *pNext;
        uint32_t flags;
        struct wl_display *display;
        struct wl_surface *surface;
    } surf_info = {
        .sType = 1000006000,
        .display = wl_disp,
        .surface = wl_surf
    };

    if (pfn_vkCreateWaylandSurfaceKHR(vk_instance, &surf_info, NULL, &vk_surface_khr) != 0) {
        return false;
    }

    return true;
}

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT WAYLAND VULKAN LIVE CICS TERMINAL INTERFACE\n");
    printf("=============================================================\n");

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

        init_vulkan_dynamic(display, surface);
        redraw_screen();
    }

    while (running) {
        if (wl_display_dispatch(display) < 0) {
            break;
        }
    }

    // Cleanup Vulkan
    if (vk_surface_khr && pfn_vkDestroySurfaceKHR) pfn_vkDestroySurfaceKHR(vk_instance, vk_surface_khr, NULL);
    if (vk_instance && pfn_vkDestroyInstance) pfn_vkDestroyInstance(vk_instance, NULL);
    if (vulkan_lib) dlclose(vulkan_lib);

    // Cleanup Wayland
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
