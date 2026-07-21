#ifndef TSFI_VULKAN_GUIDE_APP_H
#define TSFI_VULKAN_GUIDE_APP_H

#include <stdint.h>
#include <stddef.h>
#include "tsfi_vulkan_guide_engine.h"

#define VULKAN_GUIDE_MAX_INDEXED_DOCUMENTS 1024
#define VULKAN_GUIDE_SEARCH_QUERY_MAX_BYTES 256

typedef struct {
    char document_file_path[256];
    char document_file_name[128];
    uint64_t file_size_bytes;
    uint32_t total_lore_buttons;
} vulkan_guide_indexed_document_t;

typedef struct {
    vulkan_guide_engine_t core_guide_engine;
    vulkan_guide_indexed_document_t indexed_documents[VULKAN_GUIDE_MAX_INDEXED_DOCUMENTS];
    uint32_t total_indexed_documents;
    char active_search_query[VULKAN_GUIDE_SEARCH_QUERY_MAX_BYTES];
    uint32_t active_selected_document_index;
    uint32_t matched_search_count;
} vulkan_guide_application_t;

/* Initialize Standalone Vulkan P. J. Brown Guide Lore & Documentation Application */
int vulkan_guide_application_initialize(
    vulkan_guide_application_t *application,
    uint32_t window_width,
    uint32_t window_height
);

/* Automatically scan and index lore/ and docs/ markdown file directory paths */
int vulkan_guide_application_index_directories(
    vulkan_guide_application_t *application,
    const char *lore_directory_path,
    const char *docs_directory_path
);

/* Execute lexical search filter across indexed lore and documentation files */
int vulkan_guide_application_execute_search(
    vulkan_guide_application_t *application,
    const char *search_query_string
);

/* Select and display active indexed document in Vulkan P. J. Brown Guide viewport */
int vulkan_guide_application_select_document(
    vulkan_guide_application_t *application,
    uint32_t document_index
);

/* Render standalone Vulkan Guide application frame onto RGB buffer */
int vulkan_guide_application_render_frame(
    vulkan_guide_application_t *application,
    uint8_t *framebuffer_pixels,
    uint32_t stride_bytes
);

/* Run standalone Wayland Vulkan window presentation and input loop */
int vulkan_guide_application_run_wayland_loop(
    vulkan_guide_application_t *application,
    int max_frames_to_run
);

#endif // TSFI_VULKAN_GUIDE_APP_H
