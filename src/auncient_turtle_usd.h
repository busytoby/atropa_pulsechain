#ifndef AUNCIENT_TURTLE_USD_H
#define AUNCIENT_TURTLE_USD_H

#include <stdint.h>
#include <stdbool.h>
#include "../tsfi2-deepseek/inc/tsfi_riinterface.h"

// Struct representing a 3D coordinate point
typedef struct {
    float x;
    float y;
    float z;
} TurtlePoint3D;

// Runs the L-System expansion and generates a USDA file representing the turtle's path as a BasisCurves prim
bool auncient_turtle_generate_usd(const char *axiom, uint32_t depth, float step_length, float angle_delta_deg, const char *usda_filepath);

// Expanded interface incorporating RenderMan RiInterface and Hudson graphics/sound controls
bool auncient_turtle_generate_usd_with_ri(
    TSFiRiInterface *ri,
    const char *axiom,
    uint32_t depth,
    float step_length,
    float angle_delta_deg,
    const char *usda_filepath
);

typedef struct {
    float vgpr_x[4];
} TurtleVGPRState;

void auncient_turtle_promote_to_vgpr(float x, float y, float z, float angle, TurtleVGPRState *dest);

// Slices the generated points list into a binary quadtree serialized strictly as .dat.bin file (Rule 13)
bool auncient_turtle_slice_to_quadtree(const char *dat_bin_path);

uint32_t auncient_turtle_get_points_count(void);
const TurtlePoint3D *auncient_turtle_get_points_buffer(void);

uint32_t auncient_turtle_get_curve_count(void);
const uint32_t *auncient_turtle_get_curve_lengths(void);

#endif // AUNCIENT_TURTLE_USD_H
