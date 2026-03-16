#include "vulkan_input.h"
#include "lau_memory.h"
#include <stdio.h>
#include <unistd.h>
#include <linux/input.h>
#include <fcntl.h>
#include <string.h>
#include "window_inc/tsfi_input.h"
#include "tsfi_wire_firmware.h"
#include "tsfi_raw.h"

static void input_noop() {}

// --- Clipboard Handling ---
static struct wl_data_offer *g_current_offer = NULL;
static char *g_offer_mime = NULL;

static void data_offer_handle_offer(void *data, struct wl_data_offer *offer, const char *mime_type) {
    (void)data; (void)offer;
    if (strcmp(mime_type, "text/plain;charset=utf-8") == 0 || strcmp(mime_type, "text/plain") == 0 || strcmp(mime_type, "UTF8_STRING") == 0) {
        if (g_offer_mime) lau_free(g_offer_mime);
        g_offer_mime = lau_strdup(mime_type);
    }
}

static void data_offer_handle_source_actions(void *data, struct wl_data_offer *offer, uint32_t source_actions) {
    (void)data; (void)offer; (void)source_actions;
}

static void data_offer_handle_action(void *data, struct wl_data_offer *offer, uint32_t dnd_action) {
    (void)data; (void)offer; (void)dnd_action;
}

static const struct wl_data_offer_listener data_offer_listener = {
    .offer = data_offer_handle_offer,
    .source_actions = data_offer_handle_source_actions,
    .action = data_offer_handle_action,
};

static void data_device_handle_data_offer(void *data, struct wl_data_device *device, struct wl_data_offer *offer) {
    (void)data; (void)device;
    wl_data_offer_add_listener(offer, &data_offer_listener, NULL);
}

static void data_device_handle_enter(void *data, struct wl_data_device *device, uint32_t serial, struct wl_surface *surface, wl_fixed_t x, wl_fixed_t y, struct wl_data_offer *offer) {
    (void)data; (void)device; (void)serial; (void)surface; (void)x; (void)y; (void)offer;
}

static void data_device_handle_leave(void *data, struct wl_data_device *device) {
    (void)data; (void)device;
}

static void data_device_handle_motion(void *data, struct wl_data_device *device, uint32_t time, wl_fixed_t x, wl_fixed_t y) {
    (void)data; (void)device; (void)time; (void)x; (void)y;
}

static void data_device_handle_drop(void *data, struct wl_data_device *device) {
    (void)data; (void)device;
}

static void data_device_handle_selection(void *data, struct wl_data_device *device, struct wl_data_offer *offer) {
    (void)data; (void)device;
    if (g_current_offer && g_current_offer != offer) {
        wl_data_offer_destroy(g_current_offer);
    }
    g_current_offer = offer;
    if (!offer) {
        if (g_offer_mime) { lau_free(g_offer_mime); g_offer_mime = NULL; }
    }
}

const struct wl_data_device_listener data_device_listener = {
    .data_offer = data_device_handle_data_offer,
    .enter = data_device_handle_enter,
    .leave = data_device_handle_leave,
    .motion = data_device_handle_motion,
    .drop = data_device_handle_drop,
    .selection = data_device_handle_selection,
};

void tsfi_input_request_paste(VulkanSystem *s) {
    if (!g_current_offer || !g_offer_mime) return;

    int fds[2];
    if (pipe(fds) == -1) return;

    int flags = fcntl(fds[0], F_GETFL, 0);
    fcntl(fds[0], F_SETFL, flags | O_NONBLOCK);

    wl_data_offer_receive(g_current_offer, g_offer_mime, fds[1]);
    close(fds[1]);
    wl_display_flush(s->display);
    
    char buffer[4096];
    int attempts = 0;
    while (attempts < 50) {
        wl_display_dispatch(s->display);
        ssize_t n = read(fds[0], buffer, sizeof(buffer) - 1);
        if (n > 0) {
            buffer[n] = 0;
            if (s->clipboard_buffer) lau_free(s->clipboard_buffer);
            s->clipboard_buffer = lau_strdup(buffer);
            break;
        }
        if (n == 0) break; 
        tsfi_raw_usleep(10000);
        attempts++;
    }
    close(fds[0]);
}

// --- Global Input Hook System ---
typedef void (*tsfi_key_hook_t)(void *data, uint32_t serial, uint32_t time, uint32_t key, uint32_t state);
static tsfi_key_hook_t g_application_key_hook = NULL;

typedef void (*tsfi_resize_hook_t)(void *data, int32_t width, int32_t height);
static tsfi_resize_hook_t g_application_resize_hook = NULL;

void tsfi_input_set_key_hook(void *hook_func) {
    g_application_key_hook = (tsfi_key_hook_t)hook_func;
    printf("[VULKAN_INPUT] Application Key Hook Registered.\n");
}

void tsfi_input_set_resize_hook(void *hook_func) {
    g_application_resize_hook = (tsfi_resize_hook_t)hook_func;
    printf("[VULKAN_INPUT] Application Resize Hook Registered.\n");
}

void tsfi_input_dispatch_resize(void *data, int32_t width, int32_t height) {
    if (g_application_resize_hook) {
        g_application_resize_hook(data, width, height);
    }
}

static void pointer_handle_enter(void *data, struct wl_pointer *pointer, uint32_t serial, struct wl_surface *surface, wl_fixed_t sx, wl_fixed_t sy) {
    (void)pointer; (void)serial; (void)surface;
    VulkanSystem *s = (VulkanSystem *)data;
    s->mouse_x = wl_fixed_to_int(sx);
    s->mouse_y = wl_fixed_to_int(sy);
}

static void pointer_handle_leave(void *data, struct wl_pointer *pointer, uint32_t serial, struct wl_surface *surface) {
    (void)data; (void)pointer; (void)serial; (void)surface;
}

static void pointer_handle_motion(void *data, struct wl_pointer *pointer, uint32_t time, wl_fixed_t sx, wl_fixed_t sy) {
    (void)pointer; (void)time;
    VulkanSystem *s = (VulkanSystem *)data;
    s->mouse_x = wl_fixed_to_int(sx);
    s->mouse_y = wl_fixed_to_int(sy);

    if (s->mouse_down && s->paint_buffer) {
        uint32_t *pixels = (uint32_t *)s->paint_buffer->data;
        int radius = 5;
        for (int dy = -radius; dy <= radius; dy++) {
            for (int dx = -radius; dx <= radius; dx++) {
                int px = s->mouse_x + dx;
                int py = s->mouse_y + dy;
                if (px >= 0 && px < (int)s->paint_buffer->width && py >= 0 && py < (int)s->paint_buffer->height) {
                    if (dx*dx + dy*dy <= radius*radius) {
                        pixels[py * s->paint_buffer->width + px] = 0xFFFFFFFF; // White
                    }
                }
            }
        }
    }
}

static void pointer_handle_button(void *data, struct wl_pointer *pointer, uint32_t serial, uint32_t time, uint32_t button, uint32_t state) {
    (void)pointer; (void)serial; (void)time; (void)button;
    VulkanSystem *s = (VulkanSystem *)data;
    s->mouse_down = (state == WL_POINTER_BUTTON_STATE_PRESSED);
}

const struct wl_pointer_listener pointer_listener = {
    .enter = pointer_handle_enter,
    .leave = pointer_handle_leave,
    .motion = pointer_handle_motion,
    .button = pointer_handle_button,
    .axis = (void*)input_noop,
    .frame = (void*)input_noop,
    .axis_source = (void*)input_noop,
    .axis_stop = (void*)input_noop,
    .axis_discrete = (void*)input_noop,
    .axis_value120 = (void*)input_noop,
    .axis_relative_direction = (void*)input_noop,
};

static void keyboard_handle_keymap(void *data, struct wl_keyboard *keyboard, uint32_t format, int32_t fd, uint32_t size) {
    (void)data; (void)keyboard; (void)format; (void)size;
    close(fd);
}

static void keyboard_handle_enter(void *data, struct wl_keyboard *keyboard, uint32_t serial, struct wl_surface *surface, struct wl_array *keys) {
    (void)data; (void)keyboard; (void)serial; (void)surface; (void)keys;
}

static void keyboard_handle_leave(void *data, struct wl_keyboard *keyboard, uint32_t serial, struct wl_surface *surface) {
    (void)data; (void)keyboard; (void)serial; (void)surface;
}

#include "tsfi_broadcaster.h"

extern uint32_t active_modifiers;
extern void vulkan_logic_directive(int *cnt, char *dir);

void keyboard_handle_key(void *data, struct wl_keyboard *keyboard, uint32_t serial, uint32_t time, uint32_t key, uint32_t state) {
    (void)keyboard; (void)serial; (void)time;
    VulkanSystem *s = (VulkanSystem *)data;
    bool pressed = (state == WL_KEYBOARD_KEY_STATE_PRESSED);
    
    // 1. Dispatch to Application Hook (Mandatory for Typewriter)
    if (g_application_key_hook) {
        g_application_key_hook(data, serial, time, key, state);
    }
    
    // 2. Standard Logic
    tsfi_input_update_key(key, pressed);
    
    if (pressed) {
        s->last_key_scancode = key;
        bool ctrl = (active_modifiers & 4); 
        
        if (key == KEY_PAUSE) {
            s->edit_mode = !s->edit_mode;
            printf("[TSFI_VULKAN] Edit Mode: %s\n", s->edit_mode ? "ON" : "OFF");
        } else if (key == KEY_ESC || key == 1) {
            printf("[DEBUG] ESC detected in Vulkan window input.\n");
            extern LauBroadcaster* get_active_broadcaster(void);
            LauBroadcaster *lb = get_active_broadcaster();
            if (lb) {
                printf("[TSFI_VULKAN] ESC pressed in window. Terminating stream.\n");
                lb->stream_active = false;
            }
        } else if (ctrl && key == KEY_C) {
            printf("[DEBUG] Ctrl+C detected in Vulkan window input.\n");
            extern LauBroadcaster* get_active_broadcaster(void);
            LauBroadcaster *lb = get_active_broadcaster();
            if (lb) {
                printf("[TSFI_VULKAN] Ctrl+C pressed in window. Terminating stream.\n");
                lb->stream_active = false;
            }
        } else if (ctrl && key == KEY_V) {
            printf("[TSFI_VULKAN] Ctrl+V: Triggering ANALYZE_VISUALS...\n");
            vulkan_logic_directive(s->counter, "ANALYZE_VISUALS");
        } else if (ctrl && key == KEY_S) {
            printf("[TSFI_VULKAN] Ctrl+S: Triggering SCREENSHOT...\n");
            vulkan_logic_directive(s->counter, "SCREENSHOT");
        } else {
            uint32_t utf32 = tsfi_input_map_to_utf32(key);
            if (utf32) {
                printf("[TSFI_VULKAN] Input (UTF-32): 0x%X\n", utf32);
                LauWireFirmware *fw = tsfi_wire_firmware_get();
                if (fw && fw->wavefront.stdin_log.buffer && fw->wavefront.stdin_log.head && fw->wavefront.stdin_log.tail) {
                    char ascii_char = (char)utf32;
                    if (key == 28) ascii_char = '\n'; // KEY_ENTER
                    else if (key == 14) ascii_char = '\b'; // KEY_BACKSPACE
                    
                    _Atomic uint32_t *head_ptr = (_Atomic uint32_t *)fw->wavefront.stdin_log.head;
                    _Atomic uint32_t *tail_ptr = (_Atomic uint32_t *)fw->wavefront.stdin_log.tail;
                    uint32_t head = atomic_load_explicit(head_ptr, memory_order_relaxed);
                    uint32_t tail = atomic_load_explicit(tail_ptr, memory_order_acquire);
                    
                    uint32_t next = (head + 1) % 4096;
                    if (next != tail) {
                        fw->wavefront.stdin_log.buffer[head] = ascii_char;
                        atomic_store_explicit(head_ptr, next, memory_order_release);
                    }
                }
            }
        }
    }
}

static void keyboard_handle_modifiers(void *data, struct wl_keyboard *keyboard, uint32_t serial, uint32_t mods_depressed, uint32_t mods_latched, uint32_t mods_locked, uint32_t group) {
    (void)data; (void)keyboard; (void)serial; (void)mods_depressed; (void)mods_latched; (void)mods_locked; (void)group;
}

static void keyboard_handle_repeat_info(void *data, struct wl_keyboard *keyboard, int32_t rate, int32_t delay) {
    (void)data; (void)keyboard; (void)rate; (void)delay;
}

const struct wl_keyboard_listener keyboard_listener = {
    .keymap = keyboard_handle_keymap,
    .enter = keyboard_handle_enter,
    .leave = keyboard_handle_leave,
    .key = keyboard_handle_key,
    .modifiers = keyboard_handle_modifiers,
    .repeat_info = keyboard_handle_repeat_info
};

static void seat_handle_name(void *data, struct wl_seat *seat, const char *name) {
    (void)data; (void)seat;
    printf("[TSFI_VULKAN] Seat Name: %s\n", name);
}

static void seat_handle_release(void *data, struct wl_seat *seat) {
    (void)data; (void)seat;
    printf("[TSFI_VULKAN] Seat Released (v10).\n");
}

static void seat_handle_capabilities(void *data, struct wl_seat *seat, uint32_t caps) {
    VulkanSystem *s = (VulkanSystem *)data;
    if ((caps & WL_SEAT_CAPABILITY_KEYBOARD) && !s->keyboard) {
        s->keyboard = wl_seat_get_keyboard(seat);
        wl_keyboard_add_listener(s->keyboard, &keyboard_listener, s);
        printf("[TSFI_VULKAN] Keyboard Attached.\n");
    }
    if ((caps & WL_SEAT_CAPABILITY_POINTER) && !s->pointer) {
        s->pointer = wl_seat_get_pointer(seat);
        wl_pointer_add_listener(s->pointer, &pointer_listener, s);
        printf("[TSFI_VULKAN] Pointer Attached.\n");
    }
}

const struct wl_seat_listener_v10 seat_listener = { 
    .capabilities = seat_handle_capabilities, 
    .name = seat_handle_name,
    .release = seat_handle_release
};
