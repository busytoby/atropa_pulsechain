#ifndef TSFI_PARC_FIGMA_SUPER_H
#define TSFI_PARC_FIGMA_SUPER_H

#include <stdint.h>
#include "tsfi_parc_figma.h"

#define CONSTRAINT_MIN     0
#define CONSTRAINT_MAX     1
#define CONSTRAINT_CENTER  2
#define CONSTRAINT_SCALE   3
#define CONSTRAINT_STRETCH 4

/* 1. Figma Responsive Constraints Solver: Calculates new item position and size under parent frame resize */
void tsfi_parc_figma_apply_constraints(
    int parent_old_w, int parent_old_h,
    int parent_new_w, int parent_new_h,
    int constraint_x, int constraint_y,
    int *x, int *y, int *w, int *h
);

/* 2. Figma Vector Path Corner Smoothing: Generates bezier corner parameters based on corner radius */
void tsfi_parc_figma_corner_smooth(
    float p0_x, float p0_y,
    float p1_x, float p1_y,
    float p2_x, float p2_y,
    float corner_radius,
    float *out_ctrl_x, float *out_ctrl_y
);

/* 3. Figma Smart Selection & Tidy Up Grid: Uniformly spaces bounding boxes in a grid layout */
void tsfi_parc_figma_tidy_grid(
    tsfi_parc_layout_item_t *items, int item_count,
    int columns, int gap_x, int gap_y
);

#endif // TSFI_PARC_FIGMA_SUPER_H
