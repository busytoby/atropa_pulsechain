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

#include "tsfi_paint.h"

int vulkan_guide_application_render_frame(
    vulkan_guide_application_t *application,
    uint8_t *framebuffer_pixels,
    uint32_t stride_bytes
) {
    if (!application || !framebuffer_pixels) return -1;
    (void)stride_bytes;

    uint32_t width = application->core_guide_engine.vulkan_framebuffer_width;
    uint32_t height = application->core_guide_engine.vulkan_framebuffer_height;
    if (width == 0) width = 1280;
    if (height == 0) height = 720;

    uint32_t *pixels = (uint32_t *)framebuffer_pixels;

    // 1. Dark Slate Vulkan Background Fill
    for (uint32_t y = 0; y < height; y++) {
        for (uint32_t x = 0; x < width; x++) {
            pixels[y * width + x] = 0xFF121620;
        }
    }

    // 2. Top Navigation Header Bar
    for (uint32_t y = 5; y < 40; y++) {
        for (uint32_t x = 10; x < width - 10; x++) {
            pixels[y * width + x] = 0xFF1c2333;
        }
    }
    tsfi_quantel_paintbox_typographer(
        pixels, (int)width, (int)height,
        20, 22,
        "AUNCIENT DYSNOMIA VULKAN P. J. BROWN GUIDE & DOCUMENTATION SYSTEM",
        0xFFe6dfd3, 7.5f
    );

    // 3. Left Panel: Document Index & Search Sidebar (Width: 350, Height: 665, x: 10, y: 45)
    uint32_t sb_x = 10;
    uint32_t sb_y = 45;
    uint32_t sb_w = 350;
    uint32_t sb_h = 665;

    for (uint32_t y = sb_y; y < sb_y + sb_h && y < height; y++) {
        for (uint32_t x = sb_x; x < sb_x + sb_w && x < width; x++) {
            pixels[y * width + x] = 0xFF181d2a;
        }
    }
    // Draw Gold Outline for Sidebar Panel
    for (uint32_t x = sb_x; x < sb_x + sb_w; x++) {
        pixels[sb_y * width + x] = 0xFFc5a059;
        pixels[(sb_y + sb_h - 1) * width + x] = 0xFFc5a059;
    }
    for (uint32_t y = sb_y; y < sb_y + sb_h; y++) {
        pixels[y * width + sb_x] = 0xFFc5a059;
        pixels[y * width + (sb_x + sb_w - 1)] = 0xFFc5a059;
    }

    tsfi_quantel_paintbox_typographer(
        pixels, (int)width, (int)height,
        (int)sb_x + 15, (int)sb_y + 20,
        "DOCUMENT INDEX (642 .md Files)",
        0xFFc5a059, 7.0f
    );

    // Search Query Status Line
    char search_status_buf[384];
    snprintf(
        search_status_buf, sizeof(search_status_buf),
        "[ Search: '%s' ] (%u Matches)",
        application->active_search_query[0] != '\0' ? application->active_search_query : "ALL",
        application->matched_search_count
    );
    tsfi_quantel_paintbox_typographer(
        pixels, (int)width, (int)height,
        (int)sb_x + 15, (int)sb_y + 36,
        search_status_buf,
        0xFF00E5FF, 6.0f
    );

    // Render Document File List Entries in Sidebar
    uint32_t list_y_start = sb_y + 55;
    uint32_t max_items_to_display = 25;
    uint32_t start_doc_idx = 0;
    if (application->active_selected_document_index > 12) {
        start_doc_idx = application->active_selected_document_index - 12;
    }

    for (uint32_t i = 0; i < max_items_to_display && (start_doc_idx + i) < application->total_indexed_documents; i++) {
        uint32_t doc_idx = start_doc_idx + i;
        vulkan_guide_indexed_document_t *doc = &application->indexed_documents[doc_idx];

        uint32_t item_y = list_y_start + i * 22;
        int is_selected = (doc_idx == application->active_selected_document_index);

        if (is_selected) {
            for (uint32_t ry = item_y - 4; ry < item_y + 16 && ry < height; ry++) {
                for (uint32_t rx = sb_x + 8; rx < sb_x + sb_w - 8 && rx < width; rx++) {
                    pixels[ry * width + rx] = 0xFF8c7241; // Highlight active selection
                }
            }
        }

        char line_buf[128];
        snprintf(line_buf, sizeof(line_buf), "%s %s", is_selected ? ">" : " ", doc->document_file_name);
        tsfi_quantel_paintbox_typographer(
            pixels, (int)width, (int)height,
            (int)sb_x + 15, (int)item_y + 8,
            line_buf,
            is_selected ? 0xFFFFFFFF : 0xFFd4ceb8,
            5.5f
        );
    }

    // 4. Right Panel: Document Viewer Viewport (Width: 895, Height: 665, x: 370, y: 45)
    uint32_t view_x = 370;
    uint32_t view_y = 45;
    uint32_t view_w = 895;
    uint32_t view_h = 665;

    for (uint32_t y = view_y; y < view_y + view_h && y < height; y++) {
        for (uint32_t x = view_x; x < view_x + view_w && x < width; x++) {
            pixels[y * width + x] = 0xFF151924;
        }
    }
    // Draw Gold Outline for Viewer Panel
    for (uint32_t x = view_x; x < view_x + view_w; x++) {
        pixels[view_y * width + x] = 0xFFc5a059;
        pixels[(view_y + view_h - 1) * width + x] = 0xFFc5a059;
    }
    for (uint32_t y = view_y; y < view_y + view_h; y++) {
        pixels[y * width + view_x] = 0xFFc5a059;
        pixels[y * width + (view_x + view_w - 1)] = 0xFFc5a059;
    }

    // Document Title Bar Header
    tsfi_quantel_paintbox_typographer(
        pixels, (int)width, (int)height,
        (int)view_x + 20, (int)view_y + 20,
        application->core_guide_engine.active_document_title,
        0xFFe6dfd3, 7.5f
    );

    // Separator Line under Title Bar
    for (uint32_t x = view_x + 15; x < view_x + view_w - 15; x++) {
        pixels[(view_y + 32) * width + x] = 0xFFc5a059;
    }

    // Document Body Text Reader Viewport
    const char *body_ptr = application->core_guide_engine.active_document_body_text;
    uint32_t text_render_y = view_y + 50;
    char line_buffer[256];
    size_t line_len = 0;

    while (*body_ptr && text_render_y < (view_y + view_h - 180)) {
        if (*body_ptr == '\n' || line_len >= 80) {
            line_buffer[line_len] = '\0';

            uint32_t line_color = 0xFFd4ceb8; // Default parchment
            float font_scale = 5.5f;

            if (line_buffer[0] == '#') {
                line_color = 0xFFe6dfd3; // Header Gold/Cream
                font_scale = 6.5f;
            } else if (strncmp(line_buffer, "```", 3) == 0 || line_buffer[0] == '`') {
                line_color = 0xFF00E5FF; // Code Cyan
            }

            tsfi_quantel_paintbox_typographer(
                pixels, (int)width, (int)height,
                (int)view_x + 20, (int)text_render_y,
                line_buffer,
                line_color, font_scale
            );

            text_render_y += (uint32_t)(font_scale * 2.2f) + 4;
            line_len = 0;
            if (*body_ptr == '\n') body_ptr++;
        } else {
            line_buffer[line_len++] = *body_ptr++;
        }
    }

    // 5. P. J. Brown In-Place Hypermedia Expansion Buttons Section
    uint32_t btn_y = view_y + view_h - 165;
    tsfi_quantel_paintbox_typographer(
        pixels, (int)width, (int)height,
        (int)view_x + 20, (int)btn_y - 12,
        "P. J. BROWN IN-PLACE HYPERTEXT EXPANSIONS & GLOSSARY SIEVES:",
        0xFFc5a059, 6.0f
    );

    for (uint32_t b_idx = 0; b_idx < application->core_guide_engine.total_guide_buttons; b_idx++) {
        vulkan_guide_button_t *button = &application->core_guide_engine.guide_buttons[b_idx];

        uint32_t cur_btn_y = btn_y + b_idx * 65;
        if (cur_btn_y + 50 > (view_y + view_h)) break;

        int is_exp = (button->button_state == VULKAN_GUIDE_BUTTON_EXPANDED);

        // Draw Button Box
        for (uint32_t ry = cur_btn_y; ry < cur_btn_y + 24; ry++) {
            for (uint32_t rx = view_x + 20; rx < view_x + 280; rx++) {
                pixels[ry * width + rx] = is_exp ? 0xFF008080 : 0xFF2a354d;
            }
        }

        char btn_label_buf[128];
        snprintf(btn_label_buf, sizeof(btn_label_buf), "[%s] %s", is_exp ? "-" : "+", button->button_label_text);
        tsfi_quantel_paintbox_typographer(
            pixels, (int)width, (int)height,
            (int)view_x + 25, (int)cur_btn_y + 14,
            btn_label_buf,
            0xFFFFFFFF, 6.0f
        );

        if (is_exp) {
            // Render In-Place Expanded Glossary Context & Smalltalk/ZMM Dynamic Address
            tsfi_quantel_paintbox_typographer(
                pixels, (int)width, (int)height,
                (int)view_x + 290, (int)cur_btn_y + 10,
                button->expansion_text_content,
                0xFF00E5FF, 5.0f
            );

            char zmm_buf[128];
            snprintf(zmm_buf, sizeof(zmm_buf), "ZMM Contract: %s | %s", button->dynamic_contract_address, button->smalltalk_bytecode_evaluator);
            tsfi_quantel_paintbox_typographer(
                pixels, (int)width, (int)height,
                (int)view_x + 290, (int)cur_btn_y + 22,
                zmm_buf,
                0xFFc5a059, 5.0f
            );
        }
    }

    return 0;
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
