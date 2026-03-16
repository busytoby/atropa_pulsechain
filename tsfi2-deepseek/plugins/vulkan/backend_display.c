#include "vulkan_main.h"
#include "vulkan_logic.h"
#include "vulkan_init.h"
#include "window_inc/vulkan_config.h"
#include "lau_memory.h"
#include "tsfi_wiring.h"
#include "tsfi_broadcaster.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// Forward declarations
void init_vty_input(VulkanSystem *s);
void vty_input_poll(VulkanSystem *s);

// Shim for logic polling (dirty but works for now)
void vty_logic_epoch(int *ver) {
    VulkanSystem *s = get_vulkan_system();
    if (s && s->running) {
        vty_input_poll(s);
        // Delegate to original logic
        vulkan_logic_epoch(ver);
    }
}

VulkanSystem* create_vulkan_display_system() {
    printf("[TSFI_VULKAN] Creating Direct Display (VTY) System...\n");
    
    VulkanSystem *s = (VulkanSystem *)lau_malloc_wired(sizeof(VulkanSystem));
    if (!s) return NULL;
    memset(s, 0, sizeof(VulkanSystem));
    s->backend_type = VULKAN_BACKEND_DIRECT_DISPLAY;
    
    s->width = TSFI_WINDOW_WIDTH_DEFAULT;
    s->height = TSFI_WINDOW_HEIGHT_DEFAULT;

    s->paint_buffer = create_staging_buffer(s->width, s->height);
    if (s->paint_buffer) {
        staging_clear_avx512(s->paint_buffer, 0xFF000000); 
    }

    LauSystemHeader *h = (LauSystemHeader *)((char *)s - offsetof(LauWiredHeader, payload));
    h->resonance_as_status = lau_strdup("VK_INIT_VTY");
    
    TSFiLogicTable vulkan_inner_logic = {
        .logic_epoch = vty_logic_epoch, // Intercept epoch for input polling
        .logic_state = vulkan_logic_state,
        .logic_directive = vulkan_logic_directive,
        .logic_scramble = vulkan_logic_scramble,
        .logic_provenance = vulkan_logic_provenance
    };
    lau_wire_system((WaveSystem*)s, h, &vulkan_inner_logic);

    s->vk = init_vulkan_display();
    if (!s->vk) {
        lau_free(s);
        return NULL;
    }

    // Initialize Input
    init_vty_input(s);

    init_vk_swapchain(s->vk, s->width, s->height);
    
    s->running = true;
    set_vulkan_system(s);
    return s;
}