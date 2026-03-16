#ifndef TSFI_OTTYPE_H
#define TSFI_OTTYPE_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "lau_memory.h"
#include "tsfi_math.h"
#include "tsfi_wave15.h"

// Forward declaration to avoid circular include
typedef struct {
    const uint8_t *data;
    size_t size;
    uint32_t charstrings_secret;
    uint32_t global_subrs_secret;
    uint32_t local_subrs_secret;
    uint8_t secret_size;
} tsfi_cff_inner_t;

typedef struct {
    float x, y;
    uint8_t type; // 0: move, 1: line, 2: quad, 3: cubic
} tsfi_point_t;

typedef struct {
    tsfi_point_t *points;
    uint32_t num_points;
    uint16_t *contour_ends;
    uint16_t num_contours;
    int16_t xMin, yMin, xMax, yMax;
    uint32_t capacity;
    
    // Reuse buffers for parsing to avoid lau_malloc/lau_free contention
    uint8_t *temp_flags;
    int16_t *temp_x;
    int16_t *temp_y;
    uint16_t *temp_ends;
    uint32_t temp_cap_pts;
    uint32_t temp_cap_contours;
} tsfi_glyph_geom_t;

typedef struct {
    uint8_t *data;
    size_t size;
    uint32_t num_glyphs;
    uint32_t cmap_secret;
    uint32_t os2_secret;
    uint32_t glyf_secret;
    uint32_t location_secret;
    uint32_t cff_secret;
    uint32_t gsub_secret;
    uint32_t gpos_secret;
    uint32_t gdef_secret;
    uint32_t jstf_secret;
    uint32_t base_secret;
    uint32_t math_secret;
    uint32_t fvar_secret;
    uint32_t avar_secret;
    uint32_t hvar_secret;
    uint32_t vvar_secret;
    uint32_t mvar_secret;
    uint32_t gvar_secret;
    uint32_t hmtx_secret;
    uint32_t head_secret;
    uint32_t vhea_secret;
    uint32_t vmtx_secret;
    uint32_t ltsh_secret;
    uint32_t vdmx_secret;
    uint32_t hdmx_secret;
    uint16_t num_h_metrics;
    uint16_t num_v_metrics;
    uint16_t num_axes;
    uint16_t family_class;
    int16_t ascent;
    int16_t descent;
    int16_t line_gap;
    int16_t vert_ascent;
    int16_t vert_descent;
    int16_t vert_line_gap;
    uint16_t advance_width_max;
    uint16_t advance_height_max;
    int16_t min_lsb;
    int16_t min_rsb;
    int16_t min_tsb;
    int16_t min_bsb;
    int16_t max_x_extent;
    int16_t max_y_extent;
    int16_t caret_slope_rise;
    int16_t caret_slope_run;
    int16_t caret_secret;
    int16_t location_format;
    uint16_t max_points;
    uint16_t max_contours;
    uint16_t max_composite_points;
    uint16_t max_composite_contours;
    uint16_t max_zones;
    uint16_t max_twilight_points;
    uint16_t max_storage;
    uint16_t max_function_defs;
    uint16_t max_instruction_defs;
    uint16_t max_stack_elements;
    uint16_t max_size_of_instructions;
    uint16_t max_component_elements;
    uint16_t max_component_depth;
    bool is_cff;
    tsfi_cff_inner_t cff;
} tsfi_ottype_t;

#include "tsfi_font_shaping.h"

// Legacy typedef for compatibility while refactoring
typedef TSFiShapedGlyph tsfi_shaped_glyph_t;

typedef enum {
    TSFI_JOIN_U = 0, // Non-joining
    TSFI_JOIN_R = 1, // Right-joining
    TSFI_JOIN_D = 2, // Dual-joining
    TSFI_JOIN_C = 3  // Join-causing (e.g. ZWJ)
} tsfi_joining_type_t;

bool tsfi_ottype_init(tsfi_ottype_t *ot, const uint8_t *data, size_t size, uint32_t secret);
uint32_t tsfi_ottype_get_glyph_index(tsfi_ottype_t *ot, uint32_t codepoint, uint32_t variation_selector);
uint32_t tsfi_ottype_get_glyph_location(tsfi_ottype_t *ot, uint32_t glyph_index);
int tsfi_ottype_shape(tsfi_ottype_t *ot, const uint32_t *codepoints, int count, tsfi_shaped_glyph_t *shaped, int max_shaped, const uint32_t *opt_features, int num_opt_features);
int tsfi_ottype_justify(tsfi_ottype_t *ot, const uint32_t *codepoints, int count, float target_width, tsfi_shaped_glyph_t *shaped, int max_shaped);
bool tsfi_ottype_get_glyph_metrics(tsfi_ottype_t *ot, uint32_t glyph_index, int *advance, int *lsb);
bool tsfi_ottype_get_vertical_glyph_metrics(tsfi_ottype_t *ot, uint32_t glyph_index, int *advance, int *tsb);
bool tsfi_ottype_get_cursive_anchors(tsfi_ottype_t *ot, uint32_t gid, float *entry_x, float *entry_y, float *exit_x, float *exit_y);
bool tsfi_ottype_get_glyph_geom(tsfi_ottype_t *ot, uint32_t glyph_index, tsfi_glyph_geom_t *geom);
int tsfi_ottype_get_jstf_extenders(tsfi_ottype_t *ot, uint32_t script_tag, uint16_t *extender_gids, int max_extenders);
void tsfi_ottype_free_geom(tsfi_glyph_geom_t *geom);

#include "tsfi_k0rn_ops.h"
K0RnStream* tsfi_ottype_compile_dna(const tsfi_glyph_geom_t *geom);

#endif
