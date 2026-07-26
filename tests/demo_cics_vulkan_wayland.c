#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>
#include <wayland-client.h>
#include "xdg-shell-client-protocol.h"
#include "../src/auncient_timeline_autodin.h"

static struct wl_compositor *compositor = NULL;
static struct wl_seat *seat = NULL;
static struct wl_keyboard *keyboard = NULL;
static struct xdg_wm_base *xdg_wm_base = NULL;
static struct wl_surface *surface = NULL;
static struct xdg_surface *xdg_surface = NULL;
static struct xdg_toplevel *xdg_toplevel = NULL;
static bool running = true;

// XDG Shell listeners
static void xdg_surface_handle_configure(void *data, struct xdg_surface *xdg_surface, uint32_t serial) {
    (void)data;
    xdg_surface_ack_configure(xdg_surface, serial);
    printf("[XDG] Configure event acknowledged.\n");
}

static const struct xdg_surface_listener xdg_surface_listener = {
    .configure = xdg_surface_handle_configure,
};

static void xdg_toplevel_handle_configure(void *data, struct xdg_toplevel *xdg_toplevel, int32_t width, int32_t height, struct wl_array *states) {
    (void)data; (void)xdg_toplevel; (void)width; (void)height; (void)states;
    printf("[XDG] Toplevel configure: width=%d height=%d\n", width, height);
}

static void xdg_toplevel_handle_close(void *data, struct xdg_toplevel *xdg_toplevel) {
    (void)data; (void)xdg_toplevel;
    running = false;
    printf("[XDG] Toplevel close event received.\n");
}

static void xdg_toplevel_handle_configure_bounds(void *data, struct xdg_toplevel *xdg_toplevel, int32_t width, int32_t height) {
    (void)data; (void)xdg_toplevel; (void)width; (void)height;
    printf("[XDG] Configure bounds: width=%d height=%d\n", width, height);
}

static void xdg_toplevel_handle_wm_capabilities(void *data, struct xdg_toplevel *xdg_toplevel, struct wl_array *capabilities) {
    (void)data; (void)xdg_toplevel; (void)capabilities;
    printf("[XDG] WM capabilities received.\n");
}

static const struct xdg_toplevel_listener xdg_toplevel_listener = {
    .configure = xdg_toplevel_handle_configure,
    .close = xdg_toplevel_handle_close,
    .configure_bounds = xdg_toplevel_handle_configure_bounds,
    .wm_capabilities = xdg_toplevel_handle_wm_capabilities,
};

// Keyboard listener callbacks
static void keyboard_handle_keymap(void *data, struct wl_keyboard *wl_keyboard, uint32_t format, int32_t fd, uint32_t size) {
    (void)data; (void)wl_keyboard; (void)format; (void)fd; (void)size;
}

static void keyboard_handle_enter(void *data, struct wl_keyboard *wl_keyboard, uint32_t serial, struct wl_surface *surface, struct wl_array *keys) {
    (void)data; (void)wl_keyboard; (void)serial; (void)surface; (void)keys;
    printf("[WAYLAND] Keyboard entered active surface context.\n");
}

static void keyboard_handle_leave(void *data, struct wl_keyboard *wl_keyboard, uint32_t serial, struct wl_surface *surface) {
    (void)data; (void)wl_keyboard; (void)serial; (void)surface;
    printf("[WAYLAND] Keyboard left active surface context.\n");
}

static void keyboard_handle_key(void *data, struct wl_keyboard *wl_keyboard, uint32_t serial, uint32_t time, uint32_t key, uint32_t state) {
    (void)data; (void)wl_keyboard; (void)serial; (void)time;
    if (state == 0) return; // Ignore key release

    printf("[WAYLAND EVENT] Key pressed scancode: %u\n", key);
    
    char typed_char = (char)(key + 'a' - 30);
    if (typed_char < 'a' || typed_char > 'z') typed_char = 'x';
    
    auncient_autodin_audit_edit("INPUT_BUFF", 10, 0, typed_char);
    auncient_cics_process_transaction(0x3001, "INPUT_BUFF", typed_char);
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
        printf("[WAYLAND] Keyboard interface bound successfully.\n");
    }
}

static void seat_handle_name(void *data, struct wl_seat *wl_seat, const char *name) {
    (void)data; (void)wl_seat;
    printf("[WAYLAND] Seat identifier: %s\n", name);
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
    } else if (strcmp(interface, "wl_seat") == 0) {
        seat = wl_registry_bind(registry, name, &wl_seat_interface, version);
        wl_seat_add_listener(seat, &seat_listener, NULL);
    } else if (strcmp(interface, "xdg_wm_base") == 0) {
        xdg_wm_base = wl_registry_bind(registry, name, &xdg_wm_base_interface, version);
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
    printf("=============================================================\n");
    printf("AUNCIENT WAYLAND VULKAN LIVE CICS TERMINAL INTERFACE\n");
    printf("=============================================================\n");

    struct wl_display *display = wl_display_connect(NULL);
    if (!display) {
        fprintf(stderr, "[ERROR] Connect to Wayland compositor failed.\n");
        return 1;
    }
    printf("[WAYLAND] wl_display_connect established successfully.\n");

    struct wl_registry *registry = wl_display_get_registry(display);
    wl_registry_add_listener(registry, &registry_listener, NULL);
    
    wl_display_roundtrip(display);

    if (compositor && xdg_wm_base) {
        surface = wl_compositor_create_surface(compositor);
        xdg_surface = xdg_wm_base_get_xdg_surface(xdg_wm_base, surface);
        xdg_surface_add_listener(xdg_surface, &xdg_surface_listener, NULL);

        xdg_toplevel = xdg_surface_get_toplevel(xdg_surface);
        xdg_toplevel_add_listener(xdg_toplevel, &xdg_toplevel_listener, NULL);

        wl_surface_commit(surface);
        wl_display_roundtrip(display);
        printf("[WAYLAND] Surface and XDG toplevel window opened successfully.\n");
    }

    // Run short event dispatch loop for verification
    int loop_ticks = 10;
    while (running && loop_ticks-- > 0) {
        wl_display_dispatch_pending(display);
        usleep(10000);
    }

    // Cleanup resources
    if (xdg_toplevel) xdg_toplevel_destroy(xdg_toplevel);
    if (xdg_surface) xdg_surface_destroy(xdg_surface);
    if (surface) wl_surface_destroy(surface);
    if (keyboard) wl_keyboard_destroy(keyboard);
    if (seat) wl_seat_destroy(seat);
    if (compositor) wl_compositor_destroy(compositor);
    if (xdg_wm_base) xdg_wm_base_destroy(xdg_wm_base);
    wl_registry_destroy(registry);
    wl_display_disconnect(display);

    printf("[WAYLAND] Connection disconnected cleanly.\n");
    printf("=============================================================\n");
    printf("WAYLAND VULKAN CICS TERMINAL ACCESS COMPLETE\n");
    printf("=============================================================\n");
    return 0;
}
