#ifndef TSFI_VULKAN_GUIDE_ENGINE_H
#define TSFI_VULKAN_GUIDE_ENGINE_H

#include <stdint.h>
#include <stddef.h>
#include "tsfi_parc_stcomp.h"
#include "tsfi_autodin_cumulative_contract_comprehensive.h"

#define VULKAN_GUIDE_MAX_BUTTONS 64
#define VULKAN_GUIDE_MAX_TEXT_BYTES 16384

typedef enum {
    VULKAN_GUIDE_BUTTON_COLLAPSED = 0,
    VULKAN_GUIDE_BUTTON_EXPANDED = 1
} vulkan_guide_button_state_t;

typedef struct {
    char button_label_text[64];
    char expansion_text_content[1024];
    char smalltalk_bytecode_evaluator[256];
    char dynamic_contract_address[64]; // dynamic_<address> - Rule 9
    vulkan_guide_button_state_t button_state;
    float position_x_coordinate;
    float position_y_coordinate;
    float button_width_extent;
    float button_height_extent;
} vulkan_guide_button_t;

typedef struct {
    vulkan_guide_button_t guide_buttons[VULKAN_GUIDE_MAX_BUTTONS];
    uint32_t total_guide_buttons;
    char active_document_title[128];
    char active_document_body_text[VULKAN_GUIDE_MAX_TEXT_BYTES];
    uint32_t vulkan_framebuffer_width;
    uint32_t vulkan_framebuffer_height;
    autodin_cumulative_contract_comprehensive_resolver_t contract_resolver;
} vulkan_guide_engine_t;

/* Initialize Vulkan P. J. Brown Guide Engine replacing Lore Workshop */
int vulkan_guide_engine_initialize(
    vulkan_guide_engine_t *guide_engine,
    uint32_t framebuffer_width,
    uint32_t framebuffer_height
);

/* Register in-place expansion button for Guide hypermedia document */
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
);

/* Process click event on Vulkan Guide document viewport, toggling P. J. Brown in-place expansion */
int vulkan_guide_engine_process_click(
    vulkan_guide_engine_t *guide_engine,
    float click_x_coordinate,
    float click_y_coordinate
);

/* Render P. J. Brown Guide hypertext document onto Vulkan target framebuffer RGB buffer */
int vulkan_guide_engine_render_framebuffer(
    vulkan_guide_engine_t *guide_engine,
    uint8_t *framebuffer_rgb_pixels,
    uint32_t stride_bytes
);

#endif // TSFI_VULKAN_GUIDE_ENGINE_H
