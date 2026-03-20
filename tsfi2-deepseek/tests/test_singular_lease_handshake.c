#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <wayland-client.h>
#include "drm-lease-v1-client-protocol.h"

// --- Minimal Context ---
#define MAX_DEVICES 8
struct lease_context {
    struct wl_display *display;
    struct wp_drm_lease_device_v1 *devices[MAX_DEVICES];
    int device_count;
    int connector_count;
    int leased_fd;
    bool done;
};

struct lease_context *g_probe_ctx = NULL;

// 1. Lease Callbacks
static void lease_fd(void *data, struct wp_drm_lease_v1 *lease, int32_t fd) {
    struct lease_context *ctx = data;
    if (ctx->leased_fd < 0) {
        ctx->leased_fd = fd;
        ctx->done = true;
        printf("[PROBER] !!! SUCCESS !!! Lease GRANTED on FD: %d\n", fd);
    } else {
        close(fd);
    }
}
static void lease_finished(void *data, struct wp_drm_lease_v1 *lease) {
    printf("[PROBER] -> Lease Request Resolved/Rejected.\n");
}
static struct wp_drm_lease_v1_listener lease_listener = { .lease_fd = lease_fd, .finished = lease_finished };

static void handshake_nop(void *a, ...) { (void)a; }

// 2. Connector Callback
static void connector_done(void *data, struct wp_drm_lease_connector_v1 *c) {
    struct wp_drm_lease_device_v1 *d = (struct wp_drm_lease_device_v1 *)data;
    printf("  -> [PROBER] Found Connector %p on Device %p. Submitting...\n", (void*)c, (void*)d);
    struct wp_drm_lease_request_v1 *req = wp_drm_lease_device_v1_create_lease_request(d);
    wp_drm_lease_request_v1_request_connector(req, c);
    struct wp_drm_lease_v1 *lease = wp_drm_lease_request_v1_submit(req);
    wp_drm_lease_v1_add_listener(lease, &lease_listener, g_probe_ctx);
}
static struct wp_drm_lease_connector_v1_listener connector_listener = { 
    .name=(void*)handshake_nop, .description=(void*)handshake_nop, .connector_id=(void*)handshake_nop, 
    .done=connector_done, .withdrawn=(void*)handshake_nop 
};

// 3. Device Callbacks
static void device_drm_fd(void *data, struct wp_drm_lease_device_v1 *d, int32_t fd) { (void)data;(void)d; if(fd>=0) close(fd); }
static void device_connector(void *data, struct wp_drm_lease_device_v1 *d, struct wp_drm_lease_connector_v1 *c) {
    struct lease_context *ctx = g_probe_ctx;
    ctx->connector_count++;
    printf("[PROBER] -> Device %p advertised Connector %p. (Total: %d)\n", (void*)d, (void*)c, ctx->connector_count);
    wp_drm_lease_connector_v1_add_listener(c, &connector_listener, d);
}
static void device_done(void *data, struct wp_drm_lease_device_v1 *d) {
    struct lease_context *ctx = g_probe_ctx;
    printf("[PROBER] -> Device %p Handshake Protocol Ready.\n", (void*)d);
    if (ctx->connector_count == 0) {
        printf("[PROBER] -> WARNING: Device reported DONE with 0 connectors.\n");
        static int d_cnt = 0;
        if (++d_cnt >= ctx->device_count) ctx->done = true;
    }
}
static struct wp_drm_lease_device_v1_listener device_listener = { 
    .drm_fd=device_drm_fd, .connector=device_connector, .done=device_done, .released=(void*)handshake_nop 
};

// 4. Registry Callback
static void registry_handle(void *data, struct wl_registry *reg, uint32_t id, const char *iface, uint32_t ver) {
    struct lease_context *ctx = data;
    if (strcmp(iface, "wp_drm_lease_device_v1") == 0) {
        static int l_count = 0;
        l_count++;
        if (l_count == 2) { // Targeting the second advertised device
            printf("[PROBER] Discovery: Targeting Secondary Lease Device (ID: %u). Binding...\n", id);
            ctx->devices[0] = wl_registry_bind(reg, id, &wp_drm_lease_device_v1_interface, 1);
            wp_drm_lease_device_v1_add_listener(ctx->devices[0], &device_listener, ctx->devices[0]);
            ctx->device_count = 1;
            wl_display_roundtrip(ctx->display);
        } else {
            printf("[PROBER] Discovery: Bypassing Primary Lease Device (ID: %u).\n", id);
        }
    }
}
static void registry_remover(void *data, struct wl_registry *reg, uint32_t id) { (void)data;(void)reg;(void)id; }
static struct wl_registry_listener registry_listener = { .global = registry_handle, .global_remove = registry_remover };

int main() {
    printf("=== TSFi Singular Lease Handshake Test (Watchdog) ===\n");
    struct lease_context ctx = { .leased_fd = -1, .done = false, .device_count = 0 };
    g_probe_ctx = &ctx;

    ctx.display = wl_display_connect(NULL);
    if (!ctx.display) { printf("[FAIL] Wayland connect failed.\n"); return 1; }

    struct wl_registry *reg = wl_display_get_registry(ctx.display);
    wl_registry_add_listener(reg, &registry_listener, &ctx);

    // --- WATCHDOG PUMP ---
    printf("[PROBER] Starting Watchdog Pump (10s timeout)...\n");
    for (int i = 0; i < 100 && !ctx.done; i++) {
        if (wl_display_dispatch(ctx.display) < 0) break;
        usleep(100000); 
        if (i % 10 == 0) { printf("  [PULSE] Handshake Alive (%d/100)\n", i); fflush(stdout); }
    }

    if (ctx.leased_fd >= 0) {
        printf("[RESULT] Sovereign Path Validated.\n");
        close(ctx.leased_fd);
    } else {
        printf("[RESULT] Sovereign Path Blocked by Compositor Silence.\n");
    }

    wl_display_disconnect(ctx.display);
    return 0;
}
