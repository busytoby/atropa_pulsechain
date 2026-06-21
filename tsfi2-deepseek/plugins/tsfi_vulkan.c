// TSFi Vulkan Plugin - Unity Build Wrapper
// This file aggregates all modular components into a single compilation unit
// for the hotloader, ensuring all symbols are resolved and logic is unified.
#include "vulkan/vulkan_logic.h"
#include "tsfi_staging.c"
#include "vulkan_init.c"

#include "vulkan/vulkan_input.c"
#include "vulkan/vulkan_dmabuf.c"
#include "vulkan/vulkan_xdg.c"
#include "vulkan/vulkan_render.c"
#include "vulkan/vulkan_logic.c"
#include "vulkan/backend_display.c"
#include "vulkan/input_vty.c"
#include "vulkan/vulkan_main.c"

// --- Hotloader master logic entry points ---
void master_logic_epoch(int *ver) {
    VulkanSystem *s = get_vulkan_system();
    if (!s) {
        printf("[TSFI_VULKAN] Auto-initializing Vulkan system on first epoch...\n");
        s = create_vulkan_system();
        if (!s) {
            printf("[TSFI_VULKAN] Wayland connection failed, creating headless Vulkan system...\n");
            s = (VulkanSystem *)lau_malloc_wired(sizeof(VulkanSystem));
            if (s) {
                memset(s, 0, sizeof(VulkanSystem));
                s->backend_type = VULKAN_BACKEND_HEADLESS;
                s->width = 320;
                s->height = 200;
                s->paint_buffer = create_staging_buffer(s->width, s->height);
                if (s->paint_buffer) {
                    staging_clear_avx512(s->paint_buffer, 0xFF000000); 
                }
                LauSystemHeader *h = (LauSystemHeader *)((char *)s - 8192);
                h->resonance_as_status = lau_strdup("VK_INIT_HEADLESS");
                
                extern VulkanContext* init_vulkan(int);
                s->vk = init_vulkan(-1);
                if (s->vk) {
                    s->running = true;
                    set_vulkan_system(s);
                    
                    TSFiLogicTable logic = { 
                        .logic_epoch = vulkan_logic_epoch, 
                        .logic_state = vulkan_logic_state, 
                        .logic_directive = vulkan_logic_directive, 
                        .logic_scramble = vulkan_logic_scramble, 
                        .logic_provenance = vulkan_logic_provenance 
                    };
                    lau_wire_system((WaveSystem*)s, h, &logic);
                    printf("[TSFI_VULKAN] Headless Vulkan system successfully initialized and running.\n");
                } else {
                    lau_free(s);
                    s = NULL;
                    printf("[TSFI_VULKAN] Headless Vulkan initialization failed.\n");
                }
            }
        }
    }
    vulkan_logic_epoch(ver);
}

bool master_logic_state(void *obj) {
    return vulkan_logic_state(obj);
}

void master_logic_directive(int *cnt, char *dir) {
    vulkan_logic_directive(cnt, dir);
}

void master_logic_scramble(void *ws_ptr) {
    vulkan_logic_scramble(ws_ptr);
}

void master_logic_provenance(void *ws_ptr) {
    vulkan_logic_provenance(ws_ptr);
}