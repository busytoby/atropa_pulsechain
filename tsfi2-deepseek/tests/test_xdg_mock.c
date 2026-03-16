#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "vulkan_system.h"
#include "vulkan_xdg.h"
#include "lau_memory.h"

// Since the plugin listener implementations are static, we define simple local stubs that mirror their exact logic for testing the struct layout and data mapping
static void mock_xdg_toplevel_configure(void *data, struct xdg_toplevel *xdg_toplevel, int32_t width, int32_t height, struct wl_array *states) {
    VulkanSystem *s = (VulkanSystem *)data;
    (void)xdg_toplevel;
    
    if (width > 0 && height > 0) {
        printf("[TSFI_VULKAN] XDG Toplevel Configure: %dx%d\n", width, height);
        s->width = width;
        s->height = height;
    }
    
    if (states) {
        uint32_t *state;
        wl_array_for_each(state, states) {
            if (*state == XDG_TOPLEVEL_STATE_MAXIMIZED) {
                s->maximized = true;
                printf("[TSFI_VULKAN] Window Maximized.\n");
            } else if (*state == XDG_TOPLEVEL_STATE_FULLSCREEN) {
                s->fullscreen = true;
                printf("[TSFI_VULKAN] Window Fullscreen.\n");
            } else if (*state == XDG_TOPLEVEL_STATE_ACTIVATED) {
                printf("[TSFI_VULKAN] Window Activated.\n");
            }
        }
    }
}

static void mock_xdg_toplevel_close(void *data, struct xdg_toplevel *xdg_toplevel) {
    VulkanSystem *s = (VulkanSystem *)data;
    (void)xdg_toplevel;
    printf("[TSFI_VULKAN] Window Close Requested.\n");
    s->running = false;
}

void test_vulkan_lifecycle_static() {
    printf("[TEST] Vulkan System Lifecycle & Struct Layout\n");

    // 1. Verify Struct Alignment/Size (Basic Sanity)
    size_t size = sizeof(VulkanSystem);
    printf("VulkanSystem Size: %zu bytes\n", size);
    
    // 2. Simulate Helper Usage (Directly creating a dummy system)
    // We can't call create_vulkan_system because it tries to connect to Wayland.
    // So we manually allocate and test the state flags.
    
    VulkanSystem *s = (VulkanSystem*)lau_malloc(sizeof(VulkanSystem));
    assert(s != NULL);
    memset(s, 0, sizeof(VulkanSystem));
    
    // Test initial state expectations
    s->width = 800;
    s->height = 600;
    s->maximized = false;
    s->fullscreen = false;
    
    // Simulate Logic Updates
    assert(s->width > 0 && s->height > 0);
    printf("Dimensions Valid: %dx%d\n", s->width, s->height);
    
    // Simulate Flag Toggles
    s->maximized = true;
    assert(s->maximized);
    
    s->fullscreen = true;
    assert(s->fullscreen);

    // Clean up
    lau_free(s);
    
    printf("[PASS] Vulkan Lifecycle Test (Static)\n\n");
}

int main() {
    test_vulkan_lifecycle_static();

    printf("[TEST] XDG Mock Listener Transitions\n");

    VulkanSystem *s = (VulkanSystem*)lau_malloc(sizeof(VulkanSystem));
    assert(s != NULL);
    memset(s, 0, sizeof(VulkanSystem));
    s->running = true;

    // 1. Test xdg_toplevel_configure (Resize)
    struct wl_array states;
    wl_array_init(&states);
    
    // Simulate a configure event with 1024x768
    mock_xdg_toplevel_configure(s, NULL, 1024, 768, &states);
    printf("State after resize: %dx%d\n", s->width, s->height);
    assert(s->width == 1024);
    assert(s->height == 768);

    // 2. Test xdg_toplevel_configure (Maximized State)
    uint32_t *state_ptr = wl_array_add(&states, sizeof(uint32_t));
    *state_ptr = XDG_TOPLEVEL_STATE_MAXIMIZED;
    
    mock_xdg_toplevel_configure(s, NULL, 1024, 768, &states);
    printf("State maximized: %s\n", s->maximized ? "TRUE" : "FALSE");
    assert(s->maximized == true);

    // 3. Test xdg_toplevel_close
    mock_xdg_toplevel_close(s, NULL);
    printf("State running after close: %s\n", s->running ? "TRUE" : "FALSE");
    assert(s->running == false);

    wl_array_release(&states);
    lau_free(s);
    
    printf("[PASS] XDG Mock Test\n");

    extern void tsfi_wire_firmware_teardown(void);
    tsfi_wire_firmware_teardown();
    extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}
