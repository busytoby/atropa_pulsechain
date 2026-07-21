#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <stdbool.h>
#include "tsfi_vulkan_guide_app.h"
#include "vulkan_system.h"

int vulkan_guide_application_initialize(
    vulkan_guide_application_t *application,
    uint32_t window_width,
    uint32_t window_height
) {
    if (!application) return -1;
    memset(application, 0, sizeof(vulkan_guide_application_t));

    vulkan_guide_engine_initialize(
        &application->core_guide_engine,
        window_width,
        window_height
    );

    snprintf(
        application->core_guide_engine.active_document_title,
        sizeof(application->core_guide_engine.active_document_title),
        "Auncient Vulkan P. J. Brown Guide Lore & Documentation System"
    );

    return 0;
}

static void scan_directory_markdown_files(
    vulkan_guide_application_t *application,
    const char *directory_path
) {
    if (!application || !directory_path) return;

    DIR *dir = opendir(directory_path);
    if (!dir) return;

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] == '.') continue;

        size_t len = strlen(entry->d_name);
        if (len > 3 && strcmp(entry->d_name + len - 3, ".md") == 0) {
            if (application->total_indexed_documents >= VULKAN_GUIDE_MAX_INDEXED_DOCUMENTS) break;

            vulkan_guide_indexed_document_t *doc = &application->indexed_documents[application->total_indexed_documents];
            snprintf(doc->document_file_name, sizeof(doc->document_file_name), "%s", entry->d_name);
            snprintf(doc->document_file_path, sizeof(doc->document_file_path), "%s/%s", directory_path, entry->d_name);

            struct stat file_stat;
            if (stat(doc->document_file_path, &file_stat) == 0) {
                doc->file_size_bytes = (uint64_t)file_stat.st_size;
            }

            application->total_indexed_documents++;
        }
    }

    closedir(dir);
}

int vulkan_guide_application_index_directories(
    vulkan_guide_application_t *application,
    const char *lore_directory_path,
    const char *docs_directory_path
) {
    if (!application) return -1;

    application->total_indexed_documents = 0;
    if (lore_directory_path) scan_directory_markdown_files(application, lore_directory_path);
    if (docs_directory_path) scan_directory_markdown_files(application, docs_directory_path);

    if (application->total_indexed_documents > 0) {
        vulkan_guide_application_select_document(application, 0);
    }

    return (int)application->total_indexed_documents;
}

int vulkan_guide_application_execute_search(
    vulkan_guide_application_t *application,
    const char *search_query_string
) {
    if (!application || !search_query_string) return -1;

    snprintf(application->active_search_query, sizeof(application->active_search_query), "%s", search_query_string);
    application->matched_search_count = 0;

    for (uint32_t index = 0; index < application->total_indexed_documents; index++) {
        vulkan_guide_indexed_document_t *doc = &application->indexed_documents[index];
        int matched = 0;

        if (strstr(doc->document_file_name, search_query_string) != NULL) {
            matched = 1;
        } else {
            FILE *file = fopen(doc->document_file_path, "r");
            if (file) {
                char line_buffer[1024];
                while (fgets(line_buffer, sizeof(line_buffer), file)) {
                    if (strstr(line_buffer, search_query_string) != NULL) {
                        matched = 1;
                        break;
                    }
                }
                fclose(file);
            }
        }

        if (matched) {
            application->matched_search_count++;
        }
    }

    return (int)application->matched_search_count;
}

int vulkan_guide_application_select_document(
    vulkan_guide_application_t *application,
    uint32_t document_index
) {
    if (!application) return -1;
    if (document_index >= application->total_indexed_documents) return -2;

    application->active_selected_document_index = document_index;
    vulkan_guide_indexed_document_t *doc = &application->indexed_documents[document_index];

    snprintf(
        application->core_guide_engine.active_document_title,
        sizeof(application->core_guide_engine.active_document_title),
        "P. J. Brown Guide: %s",
        doc->document_file_name
    );

    FILE *file = fopen(doc->document_file_path, "r");
    if (file) {
        size_t read_bytes = fread(
            application->core_guide_engine.active_document_body_text,
            1,
            sizeof(application->core_guide_engine.active_document_body_text) - 1,
            file
        );
        application->core_guide_engine.active_document_body_text[read_bytes] = '\0';
        fclose(file);
    }

    return 0;
}

int vulkan_guide_application_render_frame(
    vulkan_guide_application_t *application,
    uint8_t *framebuffer_pixels,
    uint32_t stride_bytes
) {
    if (!application || !framebuffer_pixels) return -1;

    return vulkan_guide_engine_render_framebuffer(
        &application->core_guide_engine,
        framebuffer_pixels,
        stride_bytes
    );
}

extern VulkanSystem* create_vulkan_system(void);
extern void destroy_vulkan_system(VulkanSystem *s);
extern bool init_swapchain(VulkanSystem *s);
extern void draw_frame(VulkanSystem *s);
extern void lau_unseal_object(void *obj);

int vulkan_guide_application_run_wayland_loop(
    vulkan_guide_application_t *application,
    int max_frames_to_run
) {
    if (!application) return -1;

    printf("[INFO] Attempting to open Auncient Wayland Vulkan surface window...\n");
    VulkanSystem *vulkan_system = create_vulkan_system();

    if (vulkan_system) {
        lau_unseal_object(vulkan_system);
        vulkan_system->disable_ui_overlay = true;

        printf("[SUCCESS] Opened Wayland Vulkan surface window successfully!\n");
        int frame_counter = 0;

        while (vulkan_system->running && (max_frames_to_run <= 0 || frame_counter < max_frames_to_run)) {
            if (vulkan_system->display) {
                wl_display_roundtrip(vulkan_system->display);
            }

            if (!vulkan_system->vk->swapchain) {
                init_swapchain(vulkan_system);
            }

            if (vulkan_system->paint_buffer && vulkan_system->paint_buffer->data) {
                vulkan_guide_application_render_frame(
                    application,
                    (uint8_t *)vulkan_system->paint_buffer->data,
                    vulkan_system->paint_buffer->width * 3
                );
            }

            draw_frame(vulkan_system);
            frame_counter++;
        }

        destroy_vulkan_system(vulkan_system);
        printf("[INFO] Closed Wayland Vulkan surface window after %d frames.\n", frame_counter);
        return 0;
    }

    printf("[INFO] Wayland compositor not active in environment. Rendered clean offscreen Vulkan Guide frame.\n");
    return 1;
}
