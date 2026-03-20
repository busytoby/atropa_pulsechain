#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <wayland-client.h>
#include "lau_memory.h"
#include "tsfi_fourier.h"
#include "tsfi_wavelet_arena.h"
#include "drm-lease-v1-client-protocol.h"

// --- TSFi DRM Lease Thunk Implementation ---

typedef struct {
    struct wl_display *display;
    struct wp_drm_lease_device_v1 *lease_device;
    struct wp_drm_lease_connector_v1 *target_connector;
    struct wp_drm_lease_v1 *active_lease;
    int unlocked_fd;
    bool granted;
} DrmLeaseContext;

// 1. Lease Listeners
static void lease_fd_handler(void *data, struct wp_drm_lease_v1 *lease, int32_t fd) {
    DrmLeaseContext *ctx = (DrmLeaseContext *)data;
    (void)lease;
    ctx->unlocked_fd = fd;
    ctx->granted = true;
    printf("[PASS] TSFi Hardware Lease GRANTED. Unlocked FD: %d\n", fd);
}
static void lease_finished_handler(void *data, struct wp_drm_lease_v1 *lease) {
    DrmLeaseContext *ctx = (DrmLeaseContext *)data;
    (void)lease;
    printf("[WARNING] Hardware Lease Revoked by Compositor.\n");
    ctx->granted = false;
}
static const struct wp_drm_lease_v1_listener lease_listener = { .lease_fd = lease_fd_handler, .finished = lease_finished_handler };

// 2. Connector Listeners
static void connector_name(void *data, struct wp_drm_lease_connector_v1 *c, const char *name) { (void)data;(void)c;(void)name; }
static void connector_desc(void *data, struct wp_drm_lease_connector_v1 *c, const char *desc) { (void)data;(void)c;(void)desc; }
static void connector_id(void *data, struct wp_drm_lease_connector_v1 *c, uint32_t id) { (void)data;(void)c;(void)id; }
static void connector_done(void *data, struct wp_drm_lease_connector_v1 *connector) {
    DrmLeaseContext *ctx = (DrmLeaseContext *)data;
    if (!ctx->target_connector) {
        ctx->target_connector = connector;
        printf("  -> [HW] Selected Connector for Lease.\n");
    }
}
static void connector_withdrawn(void *data, struct wp_drm_lease_connector_v1 *c) { (void)data;(void)c; }
static const struct wp_drm_lease_connector_v1_listener connector_listener = {
    .name = connector_name, .description = connector_desc, .connector_id = connector_id,
    .done = connector_done, .withdrawn = connector_withdrawn
};

// 3. Device Listeners
static void device_drm_fd(void *data, struct wp_drm_lease_device_v1 *d, int32_t fd) { (void)data;(void)d; close(fd); }
static void device_connector(void *data, struct wp_drm_lease_device_v1 *device, struct wp_drm_lease_connector_v1 *connector) {
    (void)device;
    wp_drm_lease_connector_v1_add_listener(connector, &connector_listener, data);
}
static void device_done(void *data, struct wp_drm_lease_device_v1 *device) {
    (void)device;
    DrmLeaseContext *ctx = (DrmLeaseContext *)data;
    if (ctx && ctx->lease_device && ctx->target_connector && !ctx->active_lease) {
        printf("[ACTION] Submitting Formal Hardware Lease Request...\n");
        struct wp_drm_lease_request_v1 *req = wp_drm_lease_device_v1_create_lease_request(ctx->lease_device);
        wp_drm_lease_request_v1_request_connector(req, ctx->target_connector);
        ctx->active_lease = wp_drm_lease_request_v1_submit(req);
        wp_drm_lease_v1_add_listener(ctx->active_lease, &lease_listener, ctx);
    }
}
static void device_released(void *data, struct wp_drm_lease_device_v1 *d) { (void)data;(void)d; }
static const struct wp_drm_lease_device_v1_listener device_listener = {
    .drm_fd = device_drm_fd, .connector = device_connector, .done = device_done, .released = device_released
};

static void registry_handler(void *data, struct wl_registry *registry, uint32_t id, const char *interface, uint32_t version) {
    DrmLeaseContext *ctx = (DrmLeaseContext *)data;
    (void)version;

    if (strcmp(interface, "wp_drm_lease_device_v1") == 0) {
        printf("  -> [HW] Discovery: Found wp_drm_lease_device_v1 (ID: %u). Binding...\n", id);
        ctx->lease_device = wl_registry_bind(registry, id, &wp_drm_lease_device_v1_interface, 1);
        wp_drm_lease_device_v1_add_listener(ctx->lease_device, &device_listener, ctx);
    } else if (strcmp(interface, "wl_compositor") == 0) {
        printf("  -> [WSI] Discovery: Found wl_compositor.\n");
    }
}

static void registry_remover(void *data, struct wl_registry *registry, uint32_t id) { (void)data;(void)registry;(void)id; }
static const struct wl_registry_listener registry_listener = { .global = registry_handler, .global_remove = registry_remover };

int tsfi_acquire_hardware_lease() {
    DrmLeaseContext ctx = {0};
    ctx.unlocked_fd = -1;

    ctx.display = wl_display_connect(NULL);
    if (!ctx.display) return -1;

    struct wl_registry *registry = wl_display_get_registry(ctx.display);
    wl_registry_add_listener(registry, &registry_listener, &ctx);
    
    // Physical Handshake Loop
    for (int i = 0; i < 500 && !ctx.granted; i++) {
        wl_display_dispatch(ctx.display);
        usleep(1000);
    }

    if (!ctx.granted) {
        printf("[FRACTURE] Hardware Lease REJECTED. Signalling XNOR Trap...\n");
        // Using the failure as an opportunity: Capture the fracture state
        TsfiWavelet W1 = {0}, W2 = {0};
        memset(W1.payload, 0xDE, 32); 
        memset(W2.payload, 0xAD, 32);
        extern void tsfi_wavelet_xnor_signal(void *manifold, const TsfiWavelet *W_Genie, const TsfiWavelet *W_SVDAG);
        tsfi_wavelet_xnor_signal(NULL, &W1, &W2); // Force fracture awareness
        
        wl_display_disconnect(ctx.display);
        return -1;
    }

    return ctx.unlocked_fd;
}
