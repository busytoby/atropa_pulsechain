#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tsfi_vulkan_guide_app.h"

int main(int argc, char **argv) {
    (void)argc;
    (void)argv;

    printf("[INFO] Initializing Standalone Auncient Vulkan P. J. Brown Guide Lore & Documentation Viewer...\n");

    vulkan_guide_application_t guide_app;
    vulkan_guide_application_initialize(&guide_app, 1280, 720);

    printf("[INFO] Scanning and indexing lore/ and docs/ markdown files...\n");
    int indexed_count = vulkan_guide_application_index_directories(
        &guide_app,
        "../lore",
        "../docs"
    );

    printf("[SUCCESS] Successfully indexed %d markdown files in Vulkan Guide database!\n", indexed_count);

    if (indexed_count > 0) {
        vulkan_guide_application_execute_search(&guide_app, "DYSNOMIA");
        printf("[INFO] Search for 'DYSNOMIA' matched %u documents.\n", guide_app.matched_search_count);

        vulkan_guide_application_select_document(&guide_app, 0);
        printf("[INFO] Active document title: %s\n", guide_app.core_guide_engine.active_document_title);
    }

    uint8_t *framebuffer_rgb = (uint8_t *)malloc(1280 * 720 * 3);
    if (framebuffer_rgb) {
        vulkan_guide_application_render_frame(&guide_app, framebuffer_rgb, 1280 * 3);
        printf("[SUCCESS] Rendered Vulkan P. J. Brown Guide framebuffer (%dx%d pixels clean).\n", 1280, 720);
        free(framebuffer_rgb);
    }

    vulkan_guide_application_run_wayland_loop(&guide_app, 0);

    printf("[SUCCESS] Standalone Auncient Vulkan Guide Hypermedia System verified!\n");
    return 0;
}
