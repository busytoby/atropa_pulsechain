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

static struct wl_compositor *compositor = NULL;
static struct wl_shm *shm = NULL;
static struct wl_seat *seat = NULL;
static struct wl_keyboard *keyboard = NULL;
static struct xdg_wm_base *xdg_wm_base = NULL;
static struct wl_surface *surface = NULL;
static struct xdg_surface *xdg_surface = NULL;
static struct xdg_toplevel *xdg_toplevel = NULL;
static bool running = true;

// Vulkan dynamic bindings
static void *vulkan_lib = NULL;
static void *vk_instance = NULL;
static void *vk_surface_khr = NULL;
static void *vk_physical_device = NULL;
static void *vk_device = NULL;

typedef int (*PFN_vkCreateInstance)(const void *pCreateInfo, const void *pAllocator, void **pInstance);
typedef int (*PFN_vkCreateWaylandSurfaceKHR)(void *instance, const void *pCreateInfo, const void *pAllocator, void **pSurface);
typedef int (*PFN_vkEnumeratePhysicalDevices)(void *instance, uint32_t *pPhysicalDeviceCount, void **pPhysicalDevices);
typedef int (*PFN_vkCreateDevice)(void *physicalDevice, const void *pCreateInfo, const void *pAllocator, void **pDevice);
typedef void (*PFN_vkDestroyDevice)(void *device, const void *pAllocator);
typedef void (*PFN_vkDestroySurfaceKHR)(void *instance, void *surface, const void *pAllocator);
typedef void (*PFN_vkDestroyInstance)(void *instance, const void *pAllocator);

static PFN_vkCreateInstance pfn_vkCreateInstance = NULL;
static PFN_vkCreateWaylandSurfaceKHR pfn_vkCreateWaylandSurfaceKHR = NULL;
static PFN_vkEnumeratePhysicalDevices pfn_vkEnumeratePhysicalDevices = NULL;
static PFN_vkCreateDevice pfn_vkCreateDevice = NULL;
static PFN_vkDestroyDevice pfn_vkDestroyDevice = NULL;
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

// XDG Shell Base listeners (Pings)
static void xdg_wm_base_handle_ping(void *data, struct xdg_wm_base *xdg_wm_base, uint32_t serial) {
    (void)data;
    xdg_wm_base_pong(xdg_wm_base, serial);
    printf("[XDG] Ping received, Pong returned (serial %u).\n", serial);
}

static const struct xdg_wm_base_listener xdg_wm_base_listener = {
    .ping = xdg_wm_base_handle_ping,
};

// XDG Surface listeners
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
    if (state == 0) return;

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
    if (!vulkan_lib) {
        fprintf(stderr, "[VULKAN] Failed to load libvulkan shared library.\n");
        return false;
    }

    pfn_vkCreateInstance = (PFN_vkCreateInstance)dlsym(vulkan_lib, "vkCreateInstance");
    pfn_vkCreateWaylandSurfaceKHR = (PFN_vkCreateWaylandSurfaceKHR)dlsym(vulkan_lib, "vkCreateWaylandSurfaceKHR");
    pfn_vkEnumeratePhysicalDevices = (PFN_vkEnumeratePhysicalDevices)dlsym(vulkan_lib, "vkEnumeratePhysicalDevices");
    pfn_vkCreateDevice = (PFN_vkCreateDevice)dlsym(vulkan_lib, "vkCreateDevice");
    pfn_vkDestroyDevice = (PFN_vkDestroyDevice)dlsym(vulkan_lib, "vkDestroyDevice");
    pfn_vkDestroySurfaceKHR = (PFN_vkDestroySurfaceKHR)dlsym(vulkan_lib, "vkDestroySurfaceKHR");
    pfn_vkDestroyInstance = (PFN_vkDestroyInstance)dlsym(vulkan_lib, "vkDestroyInstance");

    if (!pfn_vkCreateInstance || !pfn_vkCreateWaylandSurfaceKHR || !pfn_vkEnumeratePhysicalDevices ||
        !pfn_vkCreateDevice || !pfn_vkDestroyDevice || !pfn_vkDestroySurfaceKHR || !pfn_vkDestroyInstance) {
        fprintf(stderr, "[VULKAN] Missing expected Vulkan API entry points.\n");
        dlclose(vulkan_lib);
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
        .sType = 4, // VK_STRUCTURE_TYPE_APPLICATION_INFO
        .pApplicationName = "CICS Terminal",
        .apiVersion = (1 << 22) // VK_API_VERSION_1_0
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
        .sType = 1, // VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO
        .pApplicationInfo = &app_info,
        .enabledExtensionCount = 2,
        .ppEnabledExtensionNames = extensions
    };

    if (pfn_vkCreateInstance(&inst_info, NULL, &vk_instance) != 0) {
        fprintf(stderr, "[VULKAN] vkCreateInstance failed.\n");
        return false;
    }
    printf("[VULKAN] Instance created dynamically.\n");

    struct {
        uint32_t sType;
        const void *pNext;
        uint32_t flags;
        struct wl_display *display;
        struct wl_surface *surface;
    } surf_info = {
        .sType = 1000006000, // VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR
        .display = wl_disp,
        .surface = wl_surf
    };

    if (pfn_vkCreateWaylandSurfaceKHR(vk_instance, &surf_info, NULL, &vk_surface_khr) != 0) {
        fprintf(stderr, "[VULKAN] vkCreateWaylandSurfaceKHR failed.\n");
        return false;
    }
    printf("[VULKAN] Wayland surface mapped dynamically.\n");

    uint32_t count = 0;
    pfn_vkEnumeratePhysicalDevices(vk_instance, &count, NULL);
    if (count == 0) {
        fprintf(stderr, "[VULKAN] No physical devices found.\n");
        return false;
    }

    void **devices = malloc(sizeof(void *) * count);
    pfn_vkEnumeratePhysicalDevices(vk_instance, &count, devices);
    vk_physical_device = devices[0];
    free(devices);
    printf("[VULKAN] Primary GPU physical device resolved.\n");

    float priority = 1.0f;
    struct {
        uint32_t sType;
        const void *pNext;
        uint32_t flags;
        uint32_t queueFamilyIndex;
        uint32_t queueCount;
        const float *pQueuePriorities;
    } q_info = {
        .sType = 2, // VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO
        .queueCount = 1,
        .pQueuePriorities = &priority
    };

    struct {
        uint32_t sType;
        const void *pNext;
        uint32_t flags;
        uint32_t queueCreateInfoCount;
        const void *pQueueCreateInfos;
        uint32_t enabledLayerCount;
        const char *const *ppEnabledLayerNames;
        uint32_t enabledExtensionCount;
        const char *const *ppEnabledExtensionNames;
        const void *pEnabledFeatures;
    } dev_info = {
        .sType = 3, // VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO
        .queueCreateInfoCount = 1,
        .pQueueCreateInfos = &q_info
    };

    if (pfn_vkCreateDevice(vk_physical_device, &dev_info, NULL, &vk_device) != 0) {
        fprintf(stderr, "[VULKAN] vkCreateDevice failed.\n");
        return false;
    }
    printf("[VULKAN] Logical VkDevice established dynamically.\n");

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
    printf("[WAYLAND] wl_display_connect established successfully.\n");

    struct wl_registry *registry = wl_display_get_registry(display);
    wl_registry_add_listener(registry, &registry_listener, NULL);
    
    wl_display_roundtrip(display);

    struct wl_buffer *buffer = NULL;
    if (compositor && xdg_wm_base && shm) {
        surface = wl_compositor_create_surface(compositor);
        xdg_surface = xdg_wm_base_get_xdg_surface(xdg_wm_base, surface);
        xdg_surface_add_listener(xdg_surface, &xdg_surface_listener, NULL);

        xdg_toplevel = xdg_surface_get_toplevel(xdg_surface);
        xdg_toplevel_add_listener(xdg_toplevel, &xdg_toplevel_listener, NULL);

        // Commit and roundtrip to set up initial configurations
        wl_surface_commit(surface);
        wl_display_roundtrip(display);
        printf("[WAYLAND] Surface and XDG toplevel window opened successfully.\n");

        if (init_vulkan_dynamic(display, surface)) {
            printf("[SUCCESS] Vulkan graphics context bound dynamically to Wayland compositor output.\n");
        }

        // Attach non-empty frame buffer to map/display the window physically on-screen
        uint32_t *pixels = NULL;
        buffer = create_shm_buffer(1280, 720, &pixels);
        if (buffer && pixels) {
            // Fill background with distinct dark blue terminal color
            for (int i = 0; i < 1280 * 720; i++) {
                pixels[i] = 0xFF0000AA; // ARGB full opacity blue
            }
            
            // Draw a beautiful screen visual representation (classic 3270 border and header box)
            for (int y = 20; y < 700; y++) {
                for (int x = 20; x < 1260; x++) {
                    if (y < 25 || y > 695 || x < 25 || x > 1255) {
                        pixels[y * 1280 + x] = 0xFFFFFFFF; // White frame border
                    }
                }
            }

            wl_surface_attach(surface, buffer, 0, 0);
            wl_surface_damage(surface, 0, 0, 1280, 720);
            wl_surface_commit(surface);
            wl_display_roundtrip(display);
            printf("[WAYLAND] SHM frame buffer attached and committed to show window.\n");
        }
    }

    printf("[INFO] Entering live interactive loop. Press keys inside the window to trigger CICS audits. Close the window to exit cleanly.\n");
    
    while (running) {
        if (wl_display_dispatch(display) < 0) {
            break;
        }
    }

    // Cleanup Vulkan dynamically bound resources
    if (vk_device && pfn_vkDestroyDevice) pfn_vkDestroyDevice(vk_device, NULL);
    if (vk_surface_khr && pfn_vkDestroySurfaceKHR) pfn_vkDestroySurfaceKHR(vk_instance, vk_surface_khr, NULL);
    if (vk_instance && pfn_vkDestroyInstance) pfn_vkDestroyInstance(vk_instance, NULL);
    if (vulkan_lib) dlclose(vulkan_lib);

    // Cleanup Wayland resources
    if (buffer) wl_buffer_destroy(buffer);
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

    printf("[WAYLAND] Connection disconnected cleanly.\n");
    printf("=============================================================\n");
    printf("WAYLAND VULKAN CICS TERMINAL ACCESS COMPLETE\n");
    printf("=============================================================\n");
    return 0;
}
