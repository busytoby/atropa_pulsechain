#include "vulkan_xdg.h"
#include "vulkan_input.h"
#include "tsfi_broadcaster.h"
#include "lau_memory.h"
#include <stdio.h>

// Forward decl for dispatcher if not in header
void tsfi_input_dispatch_resize(void *data, int32_t width, int32_t height);

// --- XDG Shell Listeners ---
static void xdg_wm_base_ping(void *data, struct xdg_wm_base *xdg_wm_base, uint32_t serial) {
    (void)data;
    xdg_wm_base_pong(xdg_wm_base, serial);
}

const struct xdg_wm_base_listener xdg_wm_base_listener = {
    .ping = xdg_wm_base_ping,
};

static void xdg_surface_configure(void *data, struct xdg_surface *xdg_surface, uint32_t serial) {
    VulkanSystem *s = (VulkanSystem *)data;
    printf("[TSFI_VULKAN] xdg_surface_configure called (serial: %u)\n", serial); fflush(stdout);
    xdg_surface_ack_configure(xdg_surface, serial);
    if (s && s->surface) {
        wl_surface_commit(s->surface);
    }
}

const struct xdg_surface_listener xdg_surface_listener = {
    .configure = xdg_surface_configure,
};

static void xdg_toplevel_configure(void *data, struct xdg_toplevel *xdg_toplevel, int32_t width, int32_t height, struct wl_array *states) {
    VulkanSystem *s = (VulkanSystem *)data;
    (void)xdg_toplevel;
    
    extern LauMetadata* lau_registry_find(void *ptr);
    LauMetadata *m = lau_registry_find(s);
    bool was_sealed = (m && (m->alloc_size & (1ULL << 55)));

    if (was_sealed) {
        lau_unseal_object(s);
    }

    if (width > 0 && height > 0) {
        printf("[TSFI_VULKAN] XDG Toplevel Configure: %dx%d\n", width, height); fflush(stdout);
        s->width = width;
        s->height = height;
    }

    uint32_t *state;
    s->maximized = false;
    wl_array_for_each(state, states) {
        if (*state == XDG_TOPLEVEL_STATE_MAXIMIZED) {
            s->maximized = true;
        }
    }
    
    if (s->maximized) {
        printf("[TSFI_VULKAN] Window Maximized.\n");
    }

    tsfi_input_dispatch_resize(s, s->width, s->height);

    if (was_sealed) {
        lau_seal_object(s);
    }
}

static void xdg_toplevel_close(void *data, struct xdg_toplevel *xdg_toplevel) {
    VulkanSystem *s = (VulkanSystem *)data;
    (void)xdg_toplevel;
    printf("[TSFI_VULKAN] Window Close Requested.\n");
    
    extern LauMetadata* lau_registry_find(void *ptr);
    LauMetadata *m = lau_registry_find(s);
    bool was_sealed = (m && (m->alloc_size & (1ULL << 55)));

    if (was_sealed) {
        lau_unseal_object(s);
    }
    s->running = false;
    if (was_sealed) {
        lau_seal_object(s);
    }
    
    extern LauBroadcaster* get_active_broadcaster(void);
    LauBroadcaster *lb = get_active_broadcaster();
    if (lb) {
        LauMetadata *lm = lau_registry_find(lb);
        bool lb_sealed = (lm && (lm->alloc_size & (1ULL << 55)));
        if (lb_sealed) {
            lau_unseal_object(lb);
        }
        lb->stream_active = false;
        if (lb_sealed) {
            lau_seal_object(lb);
        }
    }
}

static void xdg_toplevel_configure_bounds(void *data, struct xdg_toplevel *xdg_toplevel, int32_t width, int32_t height) {
    (void)data; (void)xdg_toplevel; (void)width; (void)height;
}

static void xdg_toplevel_wm_capabilities(void *data, struct xdg_toplevel *xdg_toplevel, struct wl_array *capabilities) {
    (void)data; (void)xdg_toplevel; (void)capabilities;
}

const struct xdg_toplevel_listener xdg_toplevel_listener = {
    .configure = xdg_toplevel_configure,
    .close = xdg_toplevel_close,
    .configure_bounds = xdg_toplevel_configure_bounds,
    .wm_capabilities = xdg_toplevel_wm_capabilities,
};