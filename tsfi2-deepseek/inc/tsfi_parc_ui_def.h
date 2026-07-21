#ifndef TSFI_PARC_UI_DEF_H
#define TSFI_PARC_UI_DEF_H

#include <stdint.h>
#include <stddef.h>
#include "tsfi_parc_smalltalk.h"
#include "tsfi_parc_figma.h"
#include "tsfi_parc_figma_super.h"
#include "tsfi_parc_figma_vector.h"

#define MAX_UI_ELEMENTS 64

typedef enum {
    UI_ELEMENT_UNKNOWN = 0,
    UI_ELEMENT_GRID,
    UI_ELEMENT_BUTTON,
    UI_ELEMENT_PATH,
    UI_ELEMENT_TEXTBLOCK
} tsfi_ui_element_type_t;

typedef struct {
    uint32_t element_id;
    tsfi_ui_element_type_t type;
    char tag_name[32];
    int x, y, w, h;
    float corner_radius;
    uint32_t fill_color;
    char content_text[64];
    oop_t smalltalk_morph_oop;
} tsfi_ui_element_t;

typedef struct {
    uint32_t ui_def_id;
    tsfi_ui_element_t elements[MAX_UI_ELEMENTS];
    size_t element_count;
    tsfi_parc_smalltalk_vm_t st_vm;
    uint32_t evm_gas_units;             // 280 Gas / Auncient Ether Units
    double fet_power_watts;             // 0.0109 W under 78.2% Power Cut
    char tape_dat_bin[128];             // Single-Header Array (.DAT.BIN - Rule 13)
} tsfi_ui_def_engine_t;

/* Initialize XAML & Smalltalk Dynamic UI Engine */
int tsfi_ui_def_engine_init(
    uint32_t ui_def_id,
    tsfi_ui_def_engine_t *engine
);

/* Parse XAML declarative UI markup string and instantiate UI elements */
int tsfi_xaml_parse_markup(
    tsfi_ui_def_engine_t *engine,
    const char *xaml_markup,
    tsfi_ui_element_t *out_element
);

/* Bind parsed XAML UI node to Smalltalk-76 Morph instance (oop_t) */
int tsfi_st_bind_ui_morph(
    tsfi_ui_def_engine_t *engine,
    tsfi_ui_element_t *element,
    const char *morph_class_name
);

/* Synchronize UI elements to Vulkan GPU pixel buffer using Figma Vector De Casteljau rasterizer */
int tsfi_ui_sync_vulkan_buffer(
    tsfi_ui_def_engine_t *engine,
    uint32_t *pixels,
    int width,
    int height
);

#endif // TSFI_PARC_UI_DEF_H
