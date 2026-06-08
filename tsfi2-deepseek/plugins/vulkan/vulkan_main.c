#include "vulkan_main.h"
#include "vulkan_render.h"
#include "vulkan_logic.h"
#include "vulkan_init.h"
#include "vulkan_dmabuf.h"
#include "window_inc/vulkan_config.h"
#include "tsfi_wire_firmware.h"
#include "lau_memory.h"
#include "lau_telemetry.h"
#include "tsfi_wiring.h"
#include "tsfi_broadcaster.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dlfcn.h>
#include <stdbool.h>
#include <stdatomic.h>
#include <sys/mman.h>
#include "drm-lease-v1-client-protocol.h"

extern void set_vulkan_system(VulkanSystem *s);
extern VulkanSystem* get_vulkan_system();

static void main_noop() {}

// --- Lease Listeners ---
static void lease_fd_handler(void *data, struct wp_drm_lease_v1 *lease, int32_t fd) {
    VulkanSystem *s = (VulkanSystem *)data;
    (void)lease; s->leased_fd = fd; s->lease_active = true;
    printf("[PASS] TSFi Physical Lease Granted. FD: %d\n", fd);
}
static void lease_finished_handler(void *data, struct wp_drm_lease_v1 *lease) {
    VulkanSystem *s = (VulkanSystem *)data; (void)lease;
    printf("[WARNING] Physical Lease Revoked.\n"); s->lease_active = false;
}
static const struct wp_drm_lease_v1_listener lease_listener = { .lease_fd = lease_fd_handler, .finished = lease_finished_handler };

static void lease_connector_done(void *data, struct wp_drm_lease_connector_v1 *connector) {
    VulkanSystem *s = (VulkanSystem *)data;
    if (!s->lease_connector) { s->lease_connector = connector; printf("[HW] Plane 71 Connector Found.\n"); }
}
static const struct wp_drm_lease_connector_v1_listener connector_listener = {
    .name = (void*)main_noop, .description = (void*)main_noop, .connector_id = (void*)main_noop,
    .done = lease_connector_done, .withdrawn = (void*)main_noop
};

static void lease_device_connector(void *data, struct wp_drm_lease_device_v1 *device, struct wp_drm_lease_connector_v1 *connector) {
    (void)device; wp_drm_lease_connector_v1_add_listener(connector, &connector_listener, data);
}
static void lease_device_done(void *data, struct wp_drm_lease_device_v1 *device) {
    VulkanSystem *s = (VulkanSystem *)data; (void)device;
    if (s->lease_device && s->lease_connector && !s->active_lease) {
        printf("[ACTION] Submitting Formal Lease Request for Plane 71...\n");
        struct wp_drm_lease_request_v1 *req = wp_drm_lease_device_v1_create_lease_request(s->lease_device);
        wp_drm_lease_request_v1_request_connector(req, s->lease_connector);
        s->active_lease = wp_drm_lease_request_v1_submit(req);
        wp_drm_lease_v1_add_listener(s->active_lease, &lease_listener, s);
    }
}
static const struct wp_drm_lease_device_v1_listener lease_device_listener = {
    .drm_fd = (void*)main_noop, .connector = lease_device_connector, .done = lease_device_done, .released = (void*)main_noop
};

static void registry_handle_global(void *data, struct wl_registry *registry, uint32_t name, const char *interface, uint32_t version) {
    VulkanSystem *s = (VulkanSystem *)data;
    if (strcmp(interface, wl_compositor_interface.name) == 0) {
        s->compositor = wl_registry_bind(registry, name, &wl_compositor_interface, version < 6 ? version : 6);
    } else if (strcmp(interface, xdg_wm_base_interface.name) == 0) {
        s->xdg_wm_base = wl_registry_bind(registry, name, &xdg_wm_base_interface, version < 6 ? version : 6);
        extern const struct xdg_wm_base_listener xdg_wm_base_listener;
        xdg_wm_base_add_listener(s->xdg_wm_base, &xdg_wm_base_listener, s);
    } else if (strcmp(interface, wl_seat_interface.name) == 0) {
        s->seat = wl_registry_bind(registry, name, &wl_seat_interface, version < 7 ? version : 7);
        extern const struct wl_seat_listener_v10 seat_listener;
        wl_seat_add_listener(s->seat, (const struct wl_seat_listener *)&seat_listener, s);
    } else if (strcmp(interface, "wp_drm_lease_device_v1") == 0) {
        static int l_count = 0;
        if (++l_count == 2) {
            s->lease_device = wl_registry_bind(registry, name, &wp_drm_lease_device_v1_interface, 1);
            wp_drm_lease_device_v1_add_listener(s->lease_device, &lease_device_listener, s);
        }
    }
}
static const struct wl_registry_listener registry_listener = { .global = registry_handle_global, .global_remove = (void*)main_noop };

VulkanSystem* create_vulkan_system() {
    VulkanSystem *existing = get_vulkan_system();
    if (existing) return existing;

    VulkanSystem *s = (VulkanSystem *)lau_malloc_wired(sizeof(VulkanSystem));
    if (!s) return NULL;
    memset(s, 0, sizeof(VulkanSystem));
    
    const char *ab4h_env = getenv("TSFI_AB4H");
    if (ab4h_env && strcmp(ab4h_env, "1") == 0) {
        s->is_ab4h = true;
    }
    
    s->width = TSFI_WINDOW_WIDTH_DEFAULT;
    s->height = TSFI_WINDOW_HEIGHT_DEFAULT;
    s->paint_buffer = create_staging_buffer(s->width, s->height);

    LauSystemHeader *h = (LauSystemHeader *)((char *)s - 8192);
    h->resonance_as_status = lau_strdup("VK_INIT");


    s->display = wl_display_connect(NULL);
    if (!s->display) { lau_free(s); return NULL; }
    s->registry = wl_display_get_registry(s->display);
    wl_registry_add_listener(s->registry, &registry_listener, s);
    wl_display_roundtrip(s->display);
    wl_display_roundtrip(s->display);

    extern VulkanContext* init_vulkan(int);
    s->vk = init_vulkan(s->lease_active ? s->leased_fd : -1);
    if (!s->vk) { wl_display_disconnect(s->display); lau_free(s); return NULL; }

    tsfi_wire_firmware_init();
    LauWireFirmware *fw = tsfi_wire_firmware_get();
    if (fw) {
        s->wavefront.stdout_log.buffer = (char*)(uintptr_t)fw->rtl.log_stdout_ptr;
        s->wavefront.stdout_log.head = &fw->rtl.log_stdout_head;
        s->wavefront.stdout_log.tail = &fw->rtl.log_stdout_tail;
        s->wavefront.stdout_log.valid_head = &fw->rtl.log_stdout_valid_head;
        s->wavefront.stdout_log.lock = (_Atomic uint32_t *)&fw->rtl.log_stdout_lock;
        s->wavefront.stdout_log.capacity = 4096;
        s->wavefront.stdin_log.buffer = (char*)(uintptr_t)fw->rtl.log_stdin_ptr;
        s->wavefront.stdin_log.head = &fw->rtl.log_stdin_head;
        s->wavefront.stdin_log.tail = &fw->rtl.log_stdin_tail;
        s->wavefront.stdin_log.lock = (_Atomic uint32_t *)&fw->rtl.log_stdin_lock;
        s->wavefront.stdin_log.capacity = 4096;
    }

    if (!s->lease_active) {
        if (!s->compositor || !s->xdg_wm_base) {
            printf("[TSFI_VULKAN] Wayland compositor or xdg_wm_base not found, failing create_vulkan_system to fallback to headless.\n");
            if (s->vk) cleanup_vulkan(s->vk);
            if (s->display) wl_display_disconnect(s->display);
            if (s->paint_buffer) destroy_staging_buffer(s->paint_buffer);
            lau_free(s);
            return NULL;
        }
        s->surface = wl_compositor_create_surface(s->compositor);
        s->xdg_surface = xdg_wm_base_get_xdg_surface(s->xdg_wm_base, s->surface);
        s->xdg_toplevel = xdg_surface_get_toplevel(s->xdg_surface);
        extern const struct xdg_surface_listener xdg_surface_listener;
        extern const struct xdg_toplevel_listener xdg_toplevel_listener;
        xdg_surface_add_listener(s->xdg_surface, &xdg_surface_listener, s);
        xdg_toplevel_add_listener(s->xdg_toplevel, &xdg_toplevel_listener, s);
        xdg_toplevel_set_title(s->xdg_toplevel, TSFI_WINDOW_TITLE);
        wl_surface_commit(s->surface);
        extern const VkAllocationCallbacks tsfi_alloc_callbacks;
        VkWaylandSurfaceCreateInfoKHR createInfo = { .sType = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR, .display = s->display, .surface = s->surface };
        PFN_vkCreateWaylandSurfaceKHR createFunc = (PFN_vkCreateWaylandSurfaceKHR)s->vk->vkGetInstanceProcAddr(s->vk->instance, "vkCreateWaylandSurfaceKHR");
        if (createFunc) createFunc(s->vk->instance, &createInfo, &tsfi_alloc_callbacks, &s->vk->surface);
    }

    s->running = true;
    set_vulkan_system(s);

    TSFiLogicTable logic = { .logic_epoch = vulkan_logic_epoch, .logic_state = vulkan_logic_state, .logic_directive = vulkan_logic_directive, .logic_scramble = vulkan_logic_scramble, .logic_provenance = vulkan_logic_provenance };
    lau_wire_system((WaveSystem*)s, h, &logic);

    return s;
}

void destroy_vulkan_system(VulkanSystem *s) {
    if (!s) return;
    if (get_vulkan_system() == s) set_vulkan_system(NULL);
    if (s->vk) cleanup_vulkan(s->vk);
    if (s->paint_buffer) destroy_staging_buffer(s->paint_buffer);
    
    // Destroy Wayland Proxies
    if (s->xdg_toplevel) xdg_toplevel_destroy(s->xdg_toplevel);
    if (s->xdg_surface) xdg_surface_destroy(s->xdg_surface);
    if (s->xdg_wm_base) xdg_wm_base_destroy(s->xdg_wm_base);
    if (s->surface) wl_surface_destroy(s->surface);
    if (s->compositor) wl_compositor_destroy(s->compositor);
    if (s->active_lease) wp_drm_lease_v1_destroy(s->active_lease);
    if (s->lease_connector) wp_drm_lease_connector_v1_destroy(s->lease_connector);
    if (s->lease_device) wp_drm_lease_device_v1_destroy(s->lease_device);
    if (s->registry) wl_registry_destroy(s->registry);
    if (s->display) wl_display_disconnect(s->display);


    
    if (s->leased_fd >= 0) {
        close(s->leased_fd);
    }

    extern void lau_final_cleanup(WaveSystem *ws, int sfd);
    lau_final_cleanup((WaveSystem*)s, s->leased_fd);

    lau_free(s);
}
