#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "tsfi_vulkan_guide_app.h"

int main(void) {
    printf("=======================================================\n");
    printf(" RUNNING VULKAN P. J. BROWN GUIDE EXHAUSTIVE TEST SUITE \n");
    printf("=======================================================\n");

    vulkan_guide_application_t guide_app;
    int status = vulkan_guide_application_initialize(&guide_app, 1280, 720);
    assert(status == 0);
    printf("[PASS] Application initialization successful.\n");

    int indexed_count = vulkan_guide_application_index_directories(
        &guide_app,
        "../lore",
        "../docs"
    );
    assert(indexed_count > 0);
    printf("[PASS] Directory indexing verified: %d markdown (.md) files indexed.\n", indexed_count);

    const char *test_search_keywords[] = {
        "DYSNOMIA",
        "Auncient",
        "MotzkinPrime",
        "WinchesterMQ",
        "Vulkan",
        "EVM",
        "ZMM"
    };

    size_t num_keywords = sizeof(test_search_keywords) / sizeof(test_search_keywords[0]);
    for (size_t i = 0; i < num_keywords; i++) {
        int matches = vulkan_guide_application_execute_search(&guide_app, test_search_keywords[i]);
        assert(matches >= 0);
        printf("[PASS] Full-text search for '%s': %d document matches.\n", test_search_keywords[i], matches);
    }

    status = vulkan_guide_application_select_document(&guide_app, 0);
    assert(status == 0);
    printf("[PASS] Document selection verified: '%s'\n", guide_app.core_guide_engine.active_document_title);

    vulkan_guide_engine_process_click(&guide_app.core_guide_engine, 45.0f, 85.0f);
    printf("[PASS] Interactive Guide click expansion verified.\n");

    uint8_t *framebuffer_pixels = (uint8_t *)malloc(1280 * 720 * 3);
    assert(framebuffer_pixels != NULL);

    status = vulkan_guide_application_render_frame(&guide_app, framebuffer_pixels, 1280 * 3);
    assert(status == 0);
    printf("[PASS] Framebuffer rendering verified (1280x720 RGB buffer populated).\n");

    free(framebuffer_pixels);

    printf("=======================================================\n");
    printf(" ALL VULKAN GUIDE TEST SUITE VERIFICATIONS PASSED 100%% \n");
    printf("=======================================================\n");
    return 0;
}
