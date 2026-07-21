#ifndef TSFI_ALGOL_COBOL_VULKAN_H
#define TSFI_ALGOL_COBOL_VULKAN_H

#include <stdint.h>
#include <stddef.h>

typedef struct {
    uint32_t bridge_id;
    uint32_t draw_call_count;
    uint32_t evm_gas_units;             // 270 Gas / Auncient Ether Units
    double fet_power_watts;             // 0.0109 W under 78.2% Power Cut
    char tape_dat_bin[128];             // Single-Header Array (.DAT.BIN - Rule 13)
} tsfi_algol_cobol_vk_bridge_t;

/* Initialize ALGOL-61 & COBOL Vulkan Drawing Bridge */
int tsfi_algol_cobol_vk_bridge_init(
    uint32_t bridge_id,
    tsfi_algol_cobol_vk_bridge_t *bridge
);

/* ALGOL-61 Vulkan Clear Screen Procedure */
int tsfi_algol61_vk_clear(
    tsfi_algol_cobol_vk_bridge_t *bridge,
    float r, float g, float b, float a
);

/* ALGOL-61 Vulkan Draw Triangle Procedure */
int tsfi_algol61_vk_draw_triangle(
    tsfi_algol_cobol_vk_bridge_t *bridge,
    float x1, float y1,
    float x2, float y2,
    float x3, float y3,
    uint32_t color_hex
);

/* COBOL PERFORM VULKAN-DRAW-RECTANGLE Sentence */
int tsfi_cobol_perform_vk_draw_rect(
    tsfi_algol_cobol_vk_bridge_t *bridge,
    float x, float y,
    float width, float height,
    uint32_t color_hex
);

/* COBOL PERFORM VULKAN-PRESENT-FRAME Sentence */
int tsfi_cobol_perform_vk_present(
    tsfi_algol_cobol_vk_bridge_t *bridge
);

#endif // TSFI_ALGOL_COBOL_VULKAN_H
