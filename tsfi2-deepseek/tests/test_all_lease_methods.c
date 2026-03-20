#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <wayland-client.h>
#include <xf86drm.h>
#include <xf86drmMode.h>
#include "drm-lease-v1-client-protocol.h"

// --- Global Audit State ---
typedef struct {
    struct wl_display *display;
    struct wp_drm_lease_device_v1 *lease_device;
    int lease_count;
    bool success;
} AuditContext;

// --- Method A: Wayland Protocol Handlers ---
static void audit_nop() {}

static void lease_fd_handler(void *data, struct wp_drm_lease_v1 *lease, int32_t fd) {
    AuditContext *ctx = (AuditContext *)data;
    printf("[METHOD A] SUCCESS: Wayland Lease granted. FD: %d\n", fd);
    ctx->success = true;
    close(fd);
}
static void lease_finished(void *data, struct wp_drm_lease_v1 *lease) { (void)data;(void)lease; }
static const struct wp_drm_lease_v1_listener lease_listener = { .lease_fd = lease_fd_handler, .finished = lease_finished };

static void connector_done(void *data, struct wp_drm_lease_connector_v1 *c) {
    AuditContext *ctx = (AuditContext *)data;
    printf("  -> [HW] Found Leaseable Connector. Requesting...\n");
    struct wp_drm_lease_request_v1 *req = wp_drm_lease_device_v1_create_lease_request(ctx->lease_device);
    wp_drm_lease_request_v1_request_connector(req, c);
    struct wp_drm_lease_v1 *lease = wp_drm_lease_request_v1_submit(req);
    wp_drm_lease_v1_add_listener(lease, &lease_listener, ctx);
}
static const struct wp_drm_lease_connector_v1_listener conn_listener = { 
    .name=(void*)audit_nop, .description=(void*)audit_nop, .connector_id=(void*)audit_nop, 
    .done=connector_done, .withdrawn=(void*)audit_nop 
};

static void device_connector(void *data, struct wp_drm_lease_device_v1 *d, struct wp_drm_lease_connector_v1 *c) {
    wp_drm_lease_connector_v1_add_listener(c, &conn_listener, data);
}
static void device_done(void *data, struct wp_drm_lease_device_v1 *d) { (void)data;(void)d; }
static const struct wp_drm_lease_device_v1_listener dev_listener = { 
    .drm_fd=(void*)audit_nop, .connector=device_connector, .done=device_done, .released=(void*)audit_nop 
};

static void registry_handle(void *data, struct wl_registry *reg, uint32_t id, const char *iface, uint32_t ver) {
    AuditContext *ctx = (AuditContext *)data;
    if (strcmp(iface, "wp_drm_lease_device_v1") == 0) {
        printf("[METHOD A] Found Lease Device Global (ID: %u). Binding...\n", id);
        ctx->lease_device = wl_registry_bind(reg, id, &wp_drm_lease_device_v1_interface, 1);
        wp_drm_lease_device_v1_add_listener(ctx->lease_device, &dev_listener, ctx);
    }
}
static const struct wl_registry_listener reg_listener = { .global = registry_handle, .global_remove = (void*)free };

// --- Main Audit Routine ---
void run_lease_audit() {
    printf("\n=== TSFi Exhaustive Hardware Lease Audit ===\n");

    // --- TEST 1: Direct DRM Master Seizure (Method C) ---
    const char *nodes[] = {"/dev/dri/card0", "/dev/dri/card1", "/dev/dri/card2"};
    for (int i = 0; i < 3; i++) {
        int fd = open(nodes[i], O_RDWR | O_CLOEXEC);
        if (fd < 0) continue;
        
        printf("[METHOD C] Testing Direct Master Seizure on %s...\n", nodes[i]);
        if (drmSetMaster(fd) == 0) {
            printf("[METHOD C] !!! SOVEREIGN SUCCESS !!! Acquired DRM_MASTER on %s.\n", nodes[i]);
            drmDropMaster(fd);
        } else {
            printf("[METHOD C] FAILED: Permission Denied (Not root or already seized).\n");
        }
        close(fd);
    }

    // --- TEST 2: Wayland Lease Handshake (Method A) ---
    printf("\n[METHOD A] Testing Wayland Lease Handshake...\n");
    AuditContext ctx = {0};
    ctx.display = wl_display_connect(NULL);
    if (ctx.display) {
        struct wl_registry *reg = wl_display_get_registry(ctx.display);
        wl_registry_add_listener(reg, &reg_listener, &ctx);
        
        // Manual Event Pump
        for (int i = 0; i < 1000 && !ctx.success; i++) {
            while (wl_display_prepare_read(ctx.display) != 0) {
                wl_display_dispatch_pending(ctx.display);
            }
            wl_display_flush(ctx.display);
            wl_display_read_events(ctx.display);
            wl_display_dispatch_pending(ctx.display);
            usleep(1000);
            
            // Log heartbeat to show the loop is alive
            if (i % 100 == 0) { printf("  [PULSE] Handshake Attempt %d...\n", i); fflush(stdout); }
        }
        
        if (!ctx.success) printf("[METHOD A] FAILED: Handshake timed out or rejected.\n");
        wl_display_disconnect(ctx.display);
    } else {
        printf("[METHOD A] FAILED: Could not connect to Wayland.\n");
    }

    // --- TEST 3: Kernel Direct Lease (Method B) ---
    for (int i = 0; i < 3; i++) {
        int fd = open(nodes[i], O_RDWR | O_CLOEXEC);
        if (fd < 0) continue;
        printf("\n[METHOD B] Testing Direct Kernel Lease on %s...\n", nodes[i]);
        
        // Attempt to lease Plane 71 specifically
        uint32_t objects[] = { 71 }; // Plane ID 71
        uint32_t lease_id;
        int lease_fd = drmModeCreateLease(fd, objects, 1, 0, &lease_id);
        if (lease_fd >= 0) {
            printf("[METHOD B] !!! KERNEL SUCCESS !!! Lease granted for Plane 71. FD: %d\n", lease_fd);
            close(lease_fd);
        } else {
            printf("[METHOD B] FAILED: Kernel rejected direct lease request.\n");
        }
        close(fd);
    }
}

int main() {
    run_lease_audit();
    printf("\n=== Audit Concluded ===\n");
    return 0;
}
