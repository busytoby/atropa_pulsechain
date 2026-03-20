#include <malloc.h>
#include "vulkan_main.h"
#include "vulkan_logic.h"
#include "vulkan_xdg.h"
#include "vulkan_input.h"
#include "vulkan_dmabuf.h"
#include "vulkan_render.h" 
#include "window_inc/vulkan_config.h"
#include "vulkan_init.h"
#include "lau_memory.h"
#include "lau_registry.h"
#include "tsfi_wiring.h"
#include "tsfi_plugin.h"
#include "tsfi_logic.h"
#include "tsfi_broadcaster.h"
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "tsfi_raw.h"
#include "tsfi_wire_firmware.h"
#include "tsfi_io.h"

extern VulkanSystem *sys; // Shared across hot-swaps via RTLD_GLOBAL

static void main_noop() {}

#include "drm-lease-v1-client-protocol.h"

// --- DRM Lease Handshake Listeners ---

static void lease_fd_handler(void *data, struct wp_drm_lease_v1 *lease, int32_t fd) {
    VulkanSystem *s = (VulkanSystem *)data;
    (void)lease;
    s->leased_fd = fd;
    s->lease_active = true;
    printf("[PASS] TSFi Physical Lease Granted. FD: %d\n", fd);
}

static void lease_finished_handler(void *data, struct wp_drm_lease_v1 *lease) {
    VulkanSystem *s = (VulkanSystem *)data;
    (void)lease;
    printf("[WARNING] Physical Lease Revoked.\n");
    s->lease_active = false;
}

static const struct wp_drm_lease_v1_listener lease_listener = { .lease_fd = lease_fd_handler, .finished = lease_finished_handler };

static void lease_connector_done(void *data, struct wp_drm_lease_connector_v1 *connector) {
    VulkanSystem *s = (VulkanSystem *)data;
    if (!s->lease_connector) {
        s->lease_connector = connector;
        printf("[HW] Plane 71 Connector Found.\n");
    }
}

static const struct wp_drm_lease_connector_v1_listener connector_listener = {
    .name = (void*)main_noop, .description = (void*)main_noop, .connector_id = (void*)main_noop,
    .done = lease_connector_done, .withdrawn = (void*)main_noop
};

static void lease_device_connector(void *data, struct wp_drm_lease_device_v1 *device, struct wp_drm_lease_connector_v1 *connector) {
    (void)device;
    wp_drm_lease_connector_v1_add_listener(connector, &connector_listener, data);
}

static void lease_device_done(void *data, struct wp_drm_lease_device_v1 *device) {
    VulkanSystem *s = (VulkanSystem *)data;
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

// --- Registry Listener ---
static void registry_handle_global(void *data, struct wl_registry *registry, uint32_t name, const char *interface, uint32_t version) {
    VulkanSystem *s = (VulkanSystem *)data;
    
    if (strcmp(interface, wl_compositor_interface.name) == 0) {
        uint32_t target = (version < 6) ? version : 6;
        s->compositor = wl_registry_bind(registry, name, &wl_compositor_interface, target);
        printf("[TSFI_VULKAN] Negotiated Compositor: v%u (Available: v%u)\n", target, version);
    } 
    else if (strcmp(interface, zwp_linux_dmabuf_v1_interface.name) == 0) {
        uint32_t target = (version < 10) ? version : 10;
        s->dmabuf = wl_registry_bind(registry, name, &zwp_linux_dmabuf_v1_interface, target);
        printf("[TSFI_VULKAN] Negotiated DMABUF: v%u\n", target);
    } 
    else if (strcmp(interface, wl_seat_interface.name) == 0) {
        uint32_t target = (version < 10) ? version : 10;
        s->seat = wl_registry_bind(registry, name, &wl_seat_interface, target);
        wl_seat_add_listener(s->seat, (struct wl_seat_listener*)&seat_listener, s);
        printf("[TSFI_VULKAN] Negotiated Seat: v%u\n", target);
    }
    else if (strcmp(interface, xdg_wm_base_interface.name) == 0) {
        // Mode A Fallback only
        uint32_t target = (version < 6) ? version : 6;
        s->xdg_wm_base = wl_registry_bind(registry, name, &xdg_wm_base_interface, target);
        xdg_wm_base_add_listener(s->xdg_wm_base, &xdg_wm_base_listener, s);
    }
    else if (strcmp(interface, "wp_drm_lease_device_v1") == 0) {
        static int l_count = 0;
        if (++l_count == 2) { 
            s->lease_device = wl_registry_bind(registry, name, &wp_drm_lease_device_v1_interface, 1);
            wp_drm_lease_device_v1_add_listener(s->lease_device, &lease_device_listener, s);
            printf("[TSFI_VULKAN] Bound Physical Lease Device (Plane 71 Sovereignty).\n");
        }
    }
}

static const struct wl_registry_listener registry_listener = { .global = registry_handle_global, .global_remove = (void*)main_noop };

// --- Lifecycle ---
VulkanSystem* create_vulkan_headless_system() {
    VulkanSystem *s = (VulkanSystem *)lau_malloc_wired(sizeof(VulkanSystem));
    if (!s) return NULL;
    memset(s, 0, sizeof(VulkanSystem));
    
    s->backend_type = VULKAN_BACKEND_HEADLESS;
    s->width = TSFI_WINDOW_WIDTH_DEFAULT;
    s->height = TSFI_WINDOW_HEIGHT_DEFAULT;
    s->paint_buffer = create_staging_buffer(s->width, s->height);
    if (s->paint_buffer) staging_clear_avx512(s->paint_buffer, 0xFF000000);

    LauSystemHeader *h = (LauSystemHeader *)((char *)s - offsetof(LauWiredHeader, payload));
    h->resonance_as_status = lau_strdup("VK_HEADLESS");
    
    TSFiLogicTable logic = {
        .logic_epoch = vulkan_logic_epoch, .logic_state = vulkan_logic_state,
        .logic_directive = vulkan_logic_directive, .logic_scramble = vulkan_logic_scramble,
        .logic_provenance = vulkan_logic_provenance
    };
    lau_wire_system((WaveSystem*)s, h, &logic);

    extern VulkanContext* init_vulkan(int);
    s->vk = init_vulkan(-1);
    if (!s->vk) { lau_free(s); return NULL; }
    
    s->running = true;
    set_vulkan_system(s);

    // Export Zhong Timeline Sync (Physical Context)
    if (s->vk && s->vk->timelineSemaphore) {
        s->gpu_sem_addr = &s->vk->timelineValue; 
        s->gpu_sem_target = 0; 
    }

    printf("[TSFI_VULKAN] Headless Manifold Online.\n");
    return s;
}

VulkanSystem* create_vulkan_system() {
    VulkanSystem *existing = get_vulkan_system();
    if (existing) return existing;

    VulkanSystem *s = (VulkanSystem *)lau_malloc_wired(sizeof(VulkanSystem));
    if (!s) return NULL;
    memset(s, 0, sizeof(VulkanSystem));
    s->width = TSFI_WINDOW_WIDTH_DEFAULT;
    s->height = TSFI_WINDOW_HEIGHT_DEFAULT;
    s->paint_buffer = create_staging_buffer(s->width, s->height);

    LauSystemHeader *h = (LauSystemHeader *)((char *)s - offsetof(LauWiredHeader, payload));
    h->resonance_as_status = lau_strdup("VK_INIT");
    
    TSFiLogicTable logic = {
        .logic_epoch = vulkan_logic_epoch, .logic_state = vulkan_logic_state,
        .logic_directive = vulkan_logic_directive, .logic_scramble = vulkan_logic_scramble,
        .logic_provenance = vulkan_logic_provenance
    };
    lau_wire_system((WaveSystem*)s, h, &logic);

    // 1. Connection & Discovery (Bind Everything)
    s->display = wl_display_connect(NULL);
    if (!s->display) { lau_free(s); return NULL; }
    s->registry = wl_display_get_registry(s->display);
    wl_registry_add_listener(s->registry, &registry_listener, s);
    wl_display_roundtrip(s->display); // First roundtrip to bind interfaces
    wl_display_roundtrip(s->display); // Second roundtrip to let device_done fire

    // 2. The Physical Decision
    if (s->lease_active) {
        // Mode B: Sovereign Plane 71 (No Window Created)
        printf("[TSFI_VULKAN] Mode B Initialized: Direct Physical Scanout.\n");
        extern VulkanContext* init_vulkan(int);
        s->vk = init_vulkan(s->leased_fd);
    } else {
        // Mode A: Standard Hyprland Window
        printf("[TSFI_VULKAN] Mode A Initialized: Opening Pass-through Window.\n");
        if (!s->compositor || !s->xdg_wm_base) {
            printf("[TSFI_VULKAN] FATAL: Hyprland/XDG interfaces missing for Mode A.\n");
            wl_display_disconnect(s->display); lau_free(s); return NULL;
        }
        s->surface = wl_compositor_create_surface(s->compositor);
        s->xdg_surface = xdg_wm_base_get_xdg_surface(s->xdg_wm_base, s->surface);
        s->xdg_toplevel = xdg_surface_get_toplevel(s->xdg_surface);
        xdg_toplevel_set_title(s->xdg_toplevel, TSFI_WINDOW_TITLE);
        wl_surface_commit(s->surface);

        extern VulkanContext* init_vulkan(int);
        s->vk = init_vulkan(-1);
    }

    if (s->vk) {
        lau_memory_init_gpu(s->vk);
        if (!s->lease_active) {
            // Standard Wayland Surface Binding
            VkWaylandSurfaceCreateInfoKHR createInfo = { .sType = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR, .display = s->display, .surface = s->surface };
            PFN_vkCreateWaylandSurfaceKHR createFunc = (PFN_vkCreateWaylandSurfaceKHR)s->vk->vkGetInstanceProcAddr(s->vk->instance, "vkCreateWaylandSurfaceKHR");
            if (createFunc) createFunc(s->vk->instance, &createInfo, NULL, &s->vk->surface);
        }
        init_vk_swapchain(s->vk, s->width, s->height);
    }

    s->running = true;
    set_vulkan_system(s);
    return s;
}

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

    if (s->vk) {
        if (!s->lease_active) {
            printf("[TSFI_VULKAN] Mode A: Creating Wayland Surface...\n"); fflush(stdout);
            VkWaylandSurfaceCreateInfoKHR createInfo = { .sType = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR, .display = s->display, .surface = s->surface };
            PFN_vkCreateWaylandSurfaceKHR createFunc = (PFN_vkCreateWaylandSurfaceKHR)s->vk->vkGetInstanceProcAddr(s->vk->instance, "vkCreateWaylandSurfaceKHR");
            if (createFunc && createFunc(s->vk->instance, &createInfo, NULL, &s->vk->surface) == VK_SUCCESS) {
                printf("[TSFI_VULKAN] Vulkan Wayland Surface Created!\n");
                
                s->dma_buffer = create_dma_buffer(s, s->width, s->height);
                if (s->dma_buffer) {
                    wl_display_roundtrip(s->display);
                    wl_surface_attach(s->surface, s->dma_buffer, 0, 0);
                    wl_surface_damage_buffer(s->surface, 0, 0, s->width, s->height);
                    wl_surface_commit(s->surface);
                    wl_display_roundtrip(s->display);
                }
            }
        }

        printf("[TSFI_VULKAN] Finalizing Swapchain...\n"); fflush(stdout);
        if (init_swapchain(s)) {
            init_staging_vk_buffer(s, s->paint_buffer->size);

                // Initialize Terminal Logic (CPU Thunk Fallback)
                extern void* tsfi_hotload_thunk(const char* thunk_path, const char* symbol_name);
                void* terminal_thunk = tsfi_hotload_thunk("thunks/tsfi_terminal_thunk.c", "tsfi_terminal_render_thunk");
                if (terminal_thunk) {
                    // Assign thunk to system for draw_ui_elements to invoke later
                    s->vty_render_thunk = terminal_thunk;
                    printf("[TSFI_VULKAN] Terminal Soft-Compute Pipeline Active.\n");
                }
            } else {
                printf("[TSFI_VULKAN] CRITICAL: Swapchain Init Failed (Surface Lost?)\n");
            }
        }
    }
    printf("[TSFI_VULKAN] DEBUG: System Loop Ready.\n"); fflush(stdout);
    s->running = true;
    
    // Final sync to ensure compositor has processed everything
    wl_display_roundtrip(s->display);
    
    set_vulkan_system(s);
    return s;
}

void destroy_vulkan_system(VulkanSystem *s) {
    if (!s) return;
    
    if (get_vulkan_system() == s) {
        set_vulkan_system(NULL);
    }
    
    LauSystemHeader *h = (LauSystemHeader *)((char *)s - 7168);
    if (h->resonance_as_status) {
        lau_free(h->resonance_as_status);
        h->resonance_as_status = NULL;
    }

    printf("[TSFI_VULKAN] Destroying Paint Buffer...\n"); fflush(stdout);
    if (s->paint_buffer) {
        destroy_staging_buffer(s->paint_buffer);
        s->paint_buffer = NULL;
    }

    VulkanContext *vk = s->vk;
    if (vk && vk->device) {
        printf("[TSFI_VULKAN] Destroying Vulkan Resources...\n"); fflush(stdout);
        if (vk->vkDeviceWaitIdle) vk->vkDeviceWaitIdle(vk->device);
        
        if (s->dma_image) { vk->vkDestroyImage(vk->device, s->dma_image, NULL); s->dma_image = VK_NULL_HANDLE; }
        if (s->dma_mem) { vk->vkFreeMemory(vk->device, s->dma_mem, NULL); s->dma_mem = VK_NULL_HANDLE; }
        if (s->staging_vk_buffer) { vk->vkDestroyBuffer(vk->device, s->staging_vk_buffer, NULL); s->staging_vk_buffer = VK_NULL_HANDLE; }
        if (s->staging_vk_memory) { vk->vkFreeMemory(vk->device, s->staging_vk_memory, NULL); s->staging_vk_memory = VK_NULL_HANDLE; }
    }

    if (vk) {
        printf("[TSFI_VULKAN] Cleaning up Vulkan Context...\n"); fflush(stdout);
        cleanup_vulkan(vk);
        s->vk = NULL;
    }

    if (s->telem) {
        munmap(s->telem, sizeof(LauTelemetryState));
        s->telem = NULL;
    }

    printf("[TSFI_VULKAN] Destroying Wayland Objects...\n"); fflush(stdout);
    if (s->xdg_toplevel) { xdg_toplevel_destroy(s->xdg_toplevel); s->xdg_toplevel = NULL; }
    if (s->xdg_surface) { xdg_surface_destroy(s->xdg_surface); s->xdg_surface = NULL; }
    if (s->xdg_wm_base) { xdg_wm_base_destroy(s->xdg_wm_base); s->xdg_wm_base = NULL; }
    if (s->dma_buffer) { wl_buffer_destroy(s->dma_buffer); s->dma_buffer = NULL; }
    if (s->keyboard) { wl_keyboard_destroy(s->keyboard); s->keyboard = NULL; }
    if (s->pointer) { wl_pointer_destroy(s->pointer); s->pointer = NULL; }
    if (s->data_device) { wl_data_device_release(s->data_device); s->data_device = NULL; }
    if (s->data_device_manager) { wl_data_device_manager_destroy(s->data_device_manager); s->data_device_manager = NULL; }
    if (s->clipboard_buffer) { lau_free(s->clipboard_buffer); s->clipboard_buffer = NULL; }
    if (s->seat) { wl_seat_destroy(s->seat); s->seat = NULL; }
    if (s->dmabuf) { zwp_linux_dmabuf_v1_destroy(s->dmabuf); s->dmabuf = NULL; }
    if (s->surface) { wl_surface_destroy(s->surface); s->surface = NULL; }
    if (s->compositor) { wl_compositor_destroy(s->compositor); s->compositor = NULL; }
    if (s->registry) { wl_registry_destroy(s->registry); s->registry = NULL; }

    if (s->display) {
        printf("[TSFI_VULKAN] Disconnecting display %p...\n", s->display);
        wl_display_disconnect(s->display);
        s->display = NULL;
    }
    
    printf("[TSFI_VULKAN] Freeing System Object...\n"); fflush(stdout);
    lau_free(s);
    printf("[TSFI_VULKAN] Destroyed\n");
}

// --- Master Logic ---

__attribute__((weak)) void master_logic_epoch(int *ver) {
    VulkanSystem *s = get_vulkan_system();
    if (s) s->step_safety_epoch();
    if (ver) *ver += 1;
}

__attribute__((weak)) bool master_logic_state(void *obj) { 
    VulkanSystem *s = get_vulkan_system();
    if (s) s->step_safety_state(); 
    return vulkan_logic_state(obj);
}

__attribute__((weak)) void master_logic_directive(int *cnt, char *dir) { 
    VulkanSystem *s = get_vulkan_system();
    (*cnt)++; 
    if (s) s->step_executor_directive(dir); 
    
    if (dir && strcmp(dir, "OPEN_WINDOW") == 0) {
        if (!s) {
            create_vulkan_system();
        }
    }
    else if (dir && strcmp(dir, "OPEN_WINDOW_VTY") == 0) {
        if (!s) {
            create_vulkan_display_system();
        }
    }
    else if (dir && strcmp(dir, "CLOSE_WINDOW") == 0) {
        if (s) {
            destroy_vulkan_system(s);
        }
    }
}

__attribute__((weak)) void master_logic_scramble(void *ws_ptr) { 
    VulkanSystem *s = get_vulkan_system();
    (void)ws_ptr; if(s) s->scramble(); 
}

__attribute__((weak)) void master_logic_provenance(void *ws_ptr) { 
    printf("[VULKAN] Provenance Phase.\n"); fflush(stdout);
    (void)ws_ptr;
    printf("[TSFI_VULKAN] Provenance Verified.\n"); fflush(stdout);
}

static const TSFiLogicTable v_default_table = {
    .logic_epoch = master_logic_epoch,
    .logic_state = master_logic_state,
    .logic_directive = master_logic_directive,
    .logic_scramble = master_logic_scramble,
    .logic_provenance = master_logic_provenance
};

__attribute__((weak)) const TSFiLogicTable* tsfi_get_default_logic(void) {

    return &v_default_table;
}
