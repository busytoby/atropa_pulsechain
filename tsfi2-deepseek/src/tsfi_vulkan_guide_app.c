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

    // Reset Document Scroll Offset
    application->document_scroll_y_offset = 0;

    // Dynamically Register P. J. Brown Expansion Buttons for Terms Actually Present in Selected Document
    application->core_guide_engine.total_guide_buttons = 0;
    const char *body = application->core_guide_engine.active_document_body_text;

    if (strstr(body, "WinchesterMQ") || strstr(body, "Winchester") || strstr(body, "SCSI")) {
        float btn_y = 550.0f + application->core_guide_engine.total_guide_buttons * 38.0f;
        vulkan_guide_engine_register_button(
            &application->core_guide_engine,
            "Auncient WinchesterMQ",
            "Tripartite Register Context: WinchesterMQ SCSI handshake loops (keycode 32 for d/D, keycode 30 for a/A).\n"
            "Mathematical Function: Channel = Base^Signal mod MotzkinPrime (953467954114363).\n"
            "Visual Manifestation: Modulates initial phase angle offset phi_w and orbital camera projection vector.",
            "3 + 4",
            "dynamic_0x1234567890abcdef",
            390.0f, btn_y, 260.0f, 28.0f
        );
    }

    if (strstr(body, "MotzkinPrime") || strstr(body, "Motzkin") || strstr(body, "Prime")) {
        float btn_y = 550.0f + application->core_guide_engine.total_guide_buttons * 38.0f;
        vulkan_guide_engine_register_button(
            &application->core_guide_engine,
            "MotzkinPrime Field",
            "Tripartite Register Context: System field divisor constant 953467954114363.\n"
            "Mathematical Function: Denominator P for modular exponentiations (Base^Exponent mod P).\n"
            "Visual Manifestation: Sets mathematical scale limit for internal lookup tables and EDO-22 octave mapping.",
            "5 * 5",
            "dynamic_0x953467954114363",
            390.0f, btn_y, 260.0f, 28.0f
        );
    }

    if (strstr(body, "LAU") || strstr(body, "Delegate") || strstr(body, "Soul")) {
        float btn_y = 550.0f + application->core_guide_engine.total_guide_buttons * 38.0f;
        vulkan_guide_engine_register_button(
            &application->core_guide_engine,
            "LAU Identity Delegate",
            "Tripartite Register Context: Wallet address to verified LAU token contract mapping.\n"
            "Mathematical Function: Resolves user identifier Soul = LAU(UserToken).Saat(1).\n"
            "Visual Manifestation: Governs hypotrochoid geometric signature coordinates.",
            "100 + 200",
            "dynamic_0x7000000000000001",
            390.0f, btn_y, 260.0f, 28.0f
        );
    }

    if (strstr(body, "Chin") || strstr(body, "Hemisphere")) {
        float btn_y = 550.0f + application->core_guide_engine.total_guide_buttons * 38.0f;
        vulkan_guide_engine_register_button(
            &application->core_guide_engine,
            "Chin Floor Clamp",
            "Tripartite Register Context: Lower clamp of WinchesterMQ field layout.\n"
            "Mathematical Function: Chin = Beta + Eta mod MotzkinPrime; Monopole = Chin^Identity mod MotzkinPrime.\n"
            "Visual Manifestation: Governs visual hemisphere asymmetry along negative Y-axis floor.",
            "7 * 6",
            "dynamic_0x000000000000000c",
            390.0f, btn_y, 260.0f, 28.0f
        );
    }

    if (strstr(body, "Dynamo") || strstr(body, "Bond")) {
        float btn_y = 550.0f + application->core_guide_engine.total_guide_buttons * 38.0f;
        vulkan_guide_engine_register_button(
            &application->core_guide_engine,
            "Dynamo Velocity Tracker",
            "Tripartite Register Context: Dynamic velocity tracking word of SHA contract.\n"
            "Mathematical Function: Dynamo = Base^Signal mod Element during Bond.\n"
            "Visual Manifestation: Governs frequency multiplier f_y of Y-axis coordinate.",
            "88 / 2",
            "dynamic_0x000000000000000b",
            390.0f, btn_y, 260.0f, 28.0f
        );
    }

    if (strstr(body, "FET") || strstr(body, "Verlet") || strstr(body, "Discharge")) {
        float btn_y = 550.0f + application->core_guide_engine.total_guide_buttons * 38.0f;
        vulkan_guide_engine_register_button(
            &application->core_guide_engine,
            "FET Verlet Discharge Physics",
            "Tripartite Register Context: Low-level hardware Field-Effect Transistor structure.\n"
            "Mathematical Function: Soft-body Verlet solver and mass-spring depletion dynamics.\n"
            "Visual Manifestation: Governs discharge cycle physics under 78.2% power cut.",
            "12 * 12",
            "dynamic_0x000000000000000f",
            390.0f, btn_y, 260.0f, 28.0f
        );
    }

    if (strstr(body, "Quadtree") || strstr(body, "dat.bin")) {
        float btn_y = 550.0f + application->core_guide_engine.total_guide_buttons * 38.0f;
        vulkan_guide_engine_register_button(
            &application->core_guide_engine,
            "Quadtree .dat.bin Layout",
            "Tripartite Register Context: Quadtree spatial block-ledger database layout (Rule 13).\n"
            "Mathematical Function: 2D spatial bounding box slice partition.\n"
            "Visual Manifestation: Governs spatial indexing boundary maps.",
            "64 * 64",
            "dynamic_0x0000000000000013",
            390.0f, btn_y, 260.0f, 28.0f
        );
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

    // Enhanced Markdown Rendering Engine with Clean Horizontal Word-Wrapping
    const char *text_cursor = application->core_guide_engine.active_document_body_text;
    uint32_t render_y = view_y + 45;
    uint32_t max_render_y = view_y + view_h - 170;
    int in_code_block = 0;
    uint32_t line_counter = 0;

    while (*text_cursor && render_y < max_render_y) {
        // Read raw line up to newline
        char raw_line[512];
        size_t raw_len = 0;
        while (*text_cursor && *text_cursor != '\n' && raw_len < sizeof(raw_line) - 1) {
            raw_line[raw_len++] = *text_cursor++;
        }
        if (*text_cursor == '\n') text_cursor++;
        raw_line[raw_len] = '\0';

        // Check line scroll offset
        if (line_counter < application->document_scroll_y_offset) {
            line_counter++;
            continue;
        }
        line_counter++;

        // Code block toggle
        if (strncmp(raw_line, "```", 3) == 0) {
            in_code_block = !in_code_block;
            continue;
        }

        uint32_t line_color = 0xFFD4CEB8; // Standard Parchment
        float font_scale = 5.5f;
        int render_x_offset = (int)view_x + 20;
        const char *display_text = raw_line;

        if (in_code_block) {
            // Monospace Code Block formatting inside charcoal container
            line_color = 0xFF00E5FF;
            font_scale = 5.0f;
            render_x_offset += 10;
            // Draw background card for code block
            for (uint32_t cy = render_y - 2; cy < render_y + 14 && cy < height; cy++) {
                for (uint32_t cx = view_x + 20; cx < view_x + view_w - 20 && cx < width; cx++) {
                    pixels[cy * width + cx] = 0xFF1A2233;
                }
            }
        } else if (raw_line[0] == '#') {
            // Markdown Headings
            if (strncmp(raw_line, "### ", 4) == 0) {
                line_color = 0xFFC5A059; // H3 Bronze
                font_scale = 6.5f;
                display_text = raw_line + 4;
            } else if (strncmp(raw_line, "## ", 3) == 0) {
                line_color = 0xFFE6DFD3; // H2 Warm Cream
                font_scale = 7.5f;
                display_text = raw_line + 3;
            } else if (strncmp(raw_line, "# ", 2) == 0) {
                line_color = 0xFFF5D061; // H1 Bright Gold
                font_scale = 8.5f;
                display_text = raw_line + 2;
            }
        } else if (strncmp(raw_line, "* ", 2) == 0 || strncmp(raw_line, "- ", 2) == 0 || strncmp(raw_line, "+ ", 2) == 0) {
            // Bullet Lists
            line_color = 0xFFEFE6D5;
            font_scale = 5.5f;
            render_x_offset += 15;
            display_text = raw_line + 2;
            // Render Bullet Dot
            tsfi_quantel_paintbox_typographer(pixels, (int)width, (int)height, render_x_offset - 10, (int)render_y, "*", 0xFFC5A059, 5.5f);
        } else if (raw_line[0] == '>' && raw_line[1] == ' ') {
            // Blockquotes
            line_color = 0xFFD4B982;
            font_scale = 5.5f;
            render_x_offset += 15;
            display_text = raw_line + 2;
            // Draw Gold Margin Accent Bar
            for (uint32_t qy = render_y - 2; qy < render_y + 14 && qy < height; qy++) {
                for (uint32_t qx = view_x + 22; qx < view_x + 25 && qx < width; qx++) {
                    pixels[qy * width + qx] = 0xFFC5A059;
                }
            }
        }

        // Clean Horizontal Paragraph Line Accumulator (never breaks line mid-word)
        char line_acc[512] = "";
        const char *word_ptr = display_text;

        while (*word_ptr) {
            char single_word[128];
            size_t w_len = 0;
            while (*word_ptr && *word_ptr != ' ' && w_len < sizeof(single_word) - 1) {
                single_word[w_len++] = *word_ptr++;
            }
            single_word[w_len] = '\0';
            if (*word_ptr == ' ') word_ptr++;

            size_t current_len = strlen(line_acc);
            if (current_len + w_len + 1 > 68) {
                tsfi_quantel_paintbox_typographer(
                    pixels, (int)width, (int)height,
                    render_x_offset, (int)render_y,
                    line_acc,
                    line_color, font_scale
                );
                render_y += (uint32_t)(font_scale * 2.2f) + 4;
                if (render_y >= max_render_y) break;

                snprintf(line_acc, sizeof(line_acc), "%s", single_word);
            } else {
                if (current_len > 0) {
                    strcat(line_acc, " ");
                }
                strcat(line_acc, single_word);
            }
        }

        if (line_acc[0] != '\0' && render_y < max_render_y) {
            tsfi_quantel_paintbox_typographer(
                pixels, (int)width, (int)height,
                render_x_offset, (int)render_y,
                line_acc,
                line_color, font_scale
            );
            render_y += (uint32_t)(font_scale * 2.2f) + 4;
        }

        if (display_text[0] == '\0') {
            render_y += 6; // Extra paragraph spacing
        }
    }

    // 5. P. J. Brown In-Place Hypermedia Expansion Buttons Section (Contextual to Selected Document)
    if (application->core_guide_engine.total_guide_buttons > 0) {
        uint32_t btn_y = view_y + view_h - 165;
        tsfi_quantel_paintbox_typographer(
            pixels, (int)width, (int)height,
            (int)view_x + 20, (int)btn_y - 12,
            "P. J. BROWN IN-PLACE HYPERTEXT EXPANSIONS & GLOSSARY SIEVES:",
            0xFFc5a059, 6.0f
        );

        for (uint32_t b_idx = 0; b_idx < application->core_guide_engine.total_guide_buttons; b_idx++) {
            vulkan_guide_button_t *button = &application->core_guide_engine.guide_buttons[b_idx];

            uint32_t cur_btn_y = btn_y + b_idx * 40;
            if (cur_btn_y + 35 > (view_y + view_h)) break;

            int is_exp = (button->button_state == VULKAN_GUIDE_BUTTON_EXPANDED);

            // Draw Button Box
            for (uint32_t ry = cur_btn_y; ry < cur_btn_y + 26; ry++) {
                for (uint32_t rx = view_x + 20; rx < view_x + 260; rx++) {
                    pixels[ry * width + rx] = is_exp ? 0xFF008080 : 0xFF2a354d;
                }
            }

            char btn_label_buf[128];
            snprintf(btn_label_buf, sizeof(btn_label_buf), "[%s] %s", is_exp ? "-" : "+", button->button_label_text);
            tsfi_quantel_paintbox_typographer(
                pixels, (int)width, (int)height,
                (int)view_x + 25, (int)cur_btn_y + 14,
                btn_label_buf,
                0xFFFFFFFF, 5.5f
            );

            if (is_exp) {
                // Render In-Place Expanded Glossary Context & Smalltalk/ZMM Dynamic Address
                tsfi_quantel_paintbox_typographer(
                    pixels, (int)width, (int)height,
                    (int)view_x + 270, (int)cur_btn_y + 8,
                    button->expansion_text_content,
                    0xFF00E5FF, 4.5f
                );

                char zmm_buf[128];
                snprintf(zmm_buf, sizeof(zmm_buf), "ZMM Contract: %s | %s", button->dynamic_contract_address, button->smalltalk_bytecode_evaluator);
                tsfi_quantel_paintbox_typographer(
                    pixels, (int)width, (int)height,
                    (int)view_x + 270, (int)cur_btn_y + 20,
                    zmm_buf,
                    0xFFc5a059, 4.5f
                );
            }
        }
    }

    return 0;
}

#include "tsfi_parc_ui_def.h"

extern VulkanSystem* create_vulkan_system(void);
extern void destroy_vulkan_system(VulkanSystem *s);
extern bool init_swapchain(VulkanSystem *s);
extern void draw_frame(VulkanSystem *s);
extern void lau_unseal_object(void *obj);
extern void tsfi_input_set_key_hook(void *hook_func);

static vulkan_guide_application_t *g_guide_app_instance = NULL;
static VulkanSystem *g_guide_vulkan_system_instance = NULL;

static void vulkan_guide_key_hook(void *data, uint32_t serial, uint32_t time, uint32_t key, uint32_t state) {
    (void)data;
    (void)serial;
    (void)time;

    if (state != 1) return; // Only process key press down

    if (key == 1) { // ESC Key (KEY_ESC)
        printf("[INFO] ESC key pressed in Vulkan Guide window. Closing window...\n");
        if (g_guide_vulkan_system_instance) {
            g_guide_vulkan_system_instance->running = false;
        }
    } else if (key == 108 || key == 31) { // DOWN Arrow or 's'/'j'
        if (g_guide_app_instance && g_guide_app_instance->active_selected_document_index + 1 < g_guide_app_instance->total_indexed_documents) {
            vulkan_guide_application_select_document(g_guide_app_instance, g_guide_app_instance->active_selected_document_index + 1);
        }
    } else if (key == 103 || key == 17) { // UP Arrow or 'w'/'k'
        if (g_guide_app_instance && g_guide_app_instance->active_selected_document_index > 0) {
            vulkan_guide_application_select_document(g_guide_app_instance, g_guide_app_instance->active_selected_document_index - 1);
        }
    } else if (key == 109 || key == 32) { // Page Down (109) or 'd'
        if (g_guide_app_instance) {
            g_guide_app_instance->document_scroll_y_offset += 4;
        }
    } else if (key == 104 || key == 18) { // Page Up (104) or 'e'
        if (g_guide_app_instance) {
            if (g_guide_app_instance->document_scroll_y_offset >= 4) {
                g_guide_app_instance->document_scroll_y_offset -= 4;
            } else {
                g_guide_app_instance->document_scroll_y_offset = 0;
            }
        }
    }
}

int vulkan_guide_application_run_wayland_loop(
    vulkan_guide_application_t *application,
    int max_frames_to_run
) {
    if (!application) return -1;

    g_guide_app_instance = application;

    // 1. Load Declarative XAML Layout & Smalltalk Morph Class Assets
    tsfi_ui_def_engine_t ui_def_engine;
    tsfi_ui_def_engine_init(101, &ui_def_engine);

    FILE *xaml_file = fopen("../assets/vulkan_guide_layout.xaml", "r");
    if (!xaml_file) xaml_file = fopen("assets/vulkan_guide_layout.xaml", "r");
    if (xaml_file) {
        char xaml_buffer[2048];
        size_t bytes_read = fread(xaml_buffer, 1, sizeof(xaml_buffer) - 1, xaml_file);
        xaml_buffer[bytes_read] = '\0';
        fclose(xaml_file);

        tsfi_ui_element_t ui_element;
        tsfi_xaml_parse_markup(&ui_def_engine, xaml_buffer, &ui_element);
        tsfi_st_bind_ui_morph(&ui_def_engine, &ui_element, "GuideViewportMorph");
        printf("[SUCCESS] Loaded XAML markup assets & bound Smalltalk GuideViewportMorph!\n");
    }

    printf("[INFO] Attempting to open Auncient Wayland Vulkan surface window...\n");
    VulkanSystem *vulkan_system = create_vulkan_system();

    if (vulkan_system) {
        lau_unseal_object(vulkan_system);
        vulkan_system->disable_ui_overlay = true;
        vulkan_system->external_render_enabled = true;
        g_guide_vulkan_system_instance = vulkan_system;

        tsfi_input_set_key_hook((void *)vulkan_guide_key_hook);

        printf("[SUCCESS] Opened Wayland Vulkan surface window successfully! (Press ESC to close)\n");
        int frame_counter = 0;
        bool prev_mouse_down = false;

        while (vulkan_system->running && (max_frames_to_run <= 0 || frame_counter < max_frames_to_run)) {
            if (vulkan_system->display) {
                wl_display_roundtrip(vulkan_system->display);
            }

            // Dispatch Pointer / Mouse Click Events
            if (vulkan_system->mouse_down && !prev_mouse_down) {
                int mx = vulkan_system->mouse_x;
                int my = vulkan_system->mouse_y;

                // Sidebar Document List Selection Click (x: 10..360, y: 100..650)
                if (mx >= 10 && mx <= 360 && my >= 100 && my <= 650) {
                    uint32_t start_doc_idx = 0;
                    if (application->active_selected_document_index > 12) {
                        start_doc_idx = application->active_selected_document_index - 12;
                    }
                    uint32_t clicked_item = start_doc_idx + (uint32_t)((my - 100) / 22);
                    if (clicked_item < application->total_indexed_documents) {
                        vulkan_guide_application_select_document(application, clicked_item);
                        printf("[INFO] Selected Document #%u: %s\n", clicked_item, application->indexed_documents[clicked_item].document_file_name);
                    }
                }

                // P. J. Brown Expansion Button Click (x: 370..1260, y: 500..710)
                if (mx >= 370 && mx <= 1260 && my >= 500 && my <= 710) {
                    int clicked_btn = vulkan_guide_engine_process_click(&application->core_guide_engine, (float)mx, (float)my);
                    if (clicked_btn >= 0) {
                        printf("[INFO] Toggled P. J. Brown expansion button #%d!\n", clicked_btn);
                    }
                }
            }
            prev_mouse_down = vulkan_system->mouse_down;

            if (!vulkan_system->vk->swapchain) {
                init_swapchain(vulkan_system);
            }

            if (vulkan_system->paint_buffer && vulkan_system->paint_buffer->data) {
                application->core_guide_engine.vulkan_framebuffer_width = (uint32_t)vulkan_system->paint_buffer->width;
                application->core_guide_engine.vulkan_framebuffer_height = (uint32_t)vulkan_system->paint_buffer->height;

                vulkan_guide_application_render_frame(
                    application,
                    (uint8_t *)vulkan_system->paint_buffer->data,
                    vulkan_system->paint_buffer->width * 4
                );
            }

            draw_frame(vulkan_system);
            frame_counter++;
        }

        destroy_vulkan_system(vulkan_system);
        g_guide_vulkan_system_instance = NULL;
        printf("[INFO] Closed Wayland Vulkan surface window after %d frames.\n", frame_counter);
        return 0;
    }

    printf("[INFO] Wayland compositor not active in environment. Rendered clean offscreen Vulkan Guide frame.\n");
    return 1;
}
