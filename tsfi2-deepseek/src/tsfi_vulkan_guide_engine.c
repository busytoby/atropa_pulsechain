#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "tsfi_vulkan_guide_engine.h"

int vulkan_guide_engine_initialize(
    vulkan_guide_engine_t *guide_engine,
    uint32_t framebuffer_width,
    uint32_t framebuffer_height
) {
    if (!guide_engine) return -1;
    memset(guide_engine, 0, sizeof(vulkan_guide_engine_t));

    guide_engine->vulkan_framebuffer_width = framebuffer_width;
    guide_engine->vulkan_framebuffer_height = framebuffer_height;
    snprintf(guide_engine->active_document_title, sizeof(guide_engine->active_document_title), "P. J. Brown Auncient Vulkan Guide Hypermedia Engine");

    autodin_cumulative_contract_comprehensive_initialize(&guide_engine->contract_resolver);

    // Default Auncient WinchesterMQ & ZMM Dynamic Contract Glossary Buttons
    vulkan_guide_engine_register_button(
        guide_engine,
        "Auncient WinchesterMQ",
        "Tripartite Register Context: WinchesterMQ SCSI handshake loops (keycode 32 for d/D, keycode 30 for a/A).\n"
        "Mathematical Function: Base channel exponential mod MotzkinPrime (953467954114363).\n"
        "Visual Manifestation: Modulates initial phase angle offset phi_w and orbital camera projection vector.",
        "3 + 4",
        "dynamic_0x1234567890abcdef",
        40.0f, 80.0f, 240.0f, 32.0f
    );

    vulkan_guide_engine_register_button(
        guide_engine,
        "MotzkinPrime Field",
        "Tripartite Register Context: System field divisor constant 953467954114363.\n"
        "Mathematical Function: Denominator P for modular exponentiations (Base^Exponent mod P).\n"
        "Visual Manifestation: Sets the mathematical scale limit for internal lookup tables and EDO-22 octave mapping.",
        "5 * 5",
        "dynamic_0xfeedfacecafe",
        40.0f, 130.0f, 240.0f, 32.0f
    );

    return 0;
}

int vulkan_guide_engine_register_button(
    vulkan_guide_engine_t *guide_engine,
    const char *button_label,
    const char *expansion_text,
    const char *smalltalk_source_code,
    const char *contract_address,
    float position_x,
    float position_y,
    float width,
    float height
) {
    if (!guide_engine || !button_label || !expansion_text) return -1;
    if (guide_engine->total_guide_buttons >= VULKAN_GUIDE_MAX_BUTTONS) return -2;

    vulkan_guide_button_t *button = &guide_engine->guide_buttons[guide_engine->total_guide_buttons];
    snprintf(button->button_label_text, sizeof(button->button_label_text), "%s", button_label);
    snprintf(button->expansion_text_content, sizeof(button->expansion_text_content), "%s", expansion_text);
    
    if (smalltalk_source_code) {
        uint8_t bytecodes_output[128];
        tsfi_parc_st_compile(smalltalk_source_code, bytecodes_output, (int)sizeof(bytecodes_output));
        snprintf(button->smalltalk_bytecode_evaluator, sizeof(button->smalltalk_bytecode_evaluator), "STCompiled: %s", smalltalk_source_code);
    }

    if (contract_address) {
        if (strncmp(contract_address, "dynamic_", 8) == 0) {
            snprintf(button->dynamic_contract_address, sizeof(button->dynamic_contract_address), "%s", contract_address);
        } else {
            snprintf(button->dynamic_contract_address, sizeof(button->dynamic_contract_address), "dynamic_%s", contract_address);
        }
    }

    button->button_state = VULKAN_GUIDE_BUTTON_COLLAPSED;
    button->position_x_coordinate = position_x;
    button->position_y_coordinate = position_y;
    button->button_width_extent = width;
    button->button_height_extent = height;

    guide_engine->total_guide_buttons++;
    return 0;
}

int vulkan_guide_engine_process_click(
    vulkan_guide_engine_t *guide_engine,
    float click_x_coordinate,
    float click_y_coordinate
) {
    if (!guide_engine) return -1;

    for (uint32_t index = 0; index < guide_engine->total_guide_buttons; index++) {
        vulkan_guide_button_t *button = &guide_engine->guide_buttons[index];
        if (click_x_coordinate >= button->position_x_coordinate &&
            click_x_coordinate <= (button->position_x_coordinate + button->button_width_extent) &&
            click_y_coordinate >= button->position_y_coordinate &&
            click_y_coordinate <= (button->position_y_coordinate + button->button_height_extent)) {
            
            // Toggle P. J. Brown In-Place Expansion Sieve
            if (button->button_state == VULKAN_GUIDE_BUTTON_COLLAPSED) {
                button->button_state = VULKAN_GUIDE_BUTTON_EXPANDED;
            } else {
                button->button_state = VULKAN_GUIDE_BUTTON_COLLAPSED;
            }
            return (int)index;
        }
    }

    return -2;
}

int vulkan_guide_engine_render_framebuffer(
    vulkan_guide_engine_t *guide_engine,
    uint8_t *framebuffer_rgb_pixels,
    uint32_t stride_bytes
) {
    if (!guide_engine || !framebuffer_rgb_pixels) return -1;

    uint32_t width = guide_engine->vulkan_framebuffer_width;
    uint32_t height = guide_engine->vulkan_framebuffer_height;

    // Dark Vulkan slate background
    for (uint32_t y = 0; y < height; y++) {
        for (uint32_t x = 0; x < width; x++) {
            uint32_t pixel_index = y * stride_bytes + x * 3;
            framebuffer_rgb_pixels[pixel_index + 0] = 16;
            framebuffer_rgb_pixels[pixel_index + 1] = 20;
            framebuffer_rgb_pixels[pixel_index + 2] = 30;
        }
    }

    // Render P. J. Brown Guide Expansion Buttons onto Vulkan target
    for (uint32_t index = 0; index < guide_engine->total_guide_buttons; index++) {
        vulkan_guide_button_t *button = &guide_engine->guide_buttons[index];

        uint32_t start_x = (uint32_t)button->position_x_coordinate;
        uint32_t start_y = (uint32_t)button->position_y_coordinate;
        uint32_t end_x = start_x + (uint32_t)button->button_width_extent;
        uint32_t end_y = start_y + (uint32_t)button->button_height_extent;

        if (end_x > width) end_x = width;
        if (end_y > height) end_y = height;

        // Button background fill color (cyan highlight when expanded)
        uint8_t color_r = (button->button_state == VULKAN_GUIDE_BUTTON_EXPANDED) ? 30 : 40;
        uint8_t color_g = (button->button_state == VULKAN_GUIDE_BUTTON_EXPANDED) ? 140 : 60;
        uint8_t color_b = (button->button_state == VULKAN_GUIDE_BUTTON_EXPANDED) ? 200 : 90;

        for (uint32_t py = start_y; py < end_y; py++) {
            for (uint32_t px = start_x; px < end_x; px++) {
                uint32_t pixel_index = py * stride_bytes + px * 3;
                framebuffer_rgb_pixels[pixel_index + 0] = color_r;
                framebuffer_rgb_pixels[pixel_index + 1] = color_g;
                framebuffer_rgb_pixels[pixel_index + 2] = color_b;
            }
        }
    }

    return 0;
}
