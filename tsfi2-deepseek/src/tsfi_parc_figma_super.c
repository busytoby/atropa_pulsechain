#include "tsfi_parc_figma_super.h"

void tsfi_parc_figma_apply_constraints(
    int parent_old_w, int parent_old_h,
    int parent_new_w, int parent_new_h,
    int constraint_x, int constraint_y,
    int *x, int *y, int *w, int *h
) {
    if (!x || !y || !w || !h || parent_old_w <= 0 || parent_old_h <= 0) return;

    // X Constraint Solving
    if (constraint_x == CONSTRAINT_MAX) { // Right aligned
        int dist_to_right = parent_old_w - (*x + *w);
        *x = parent_new_w - *w - dist_to_right;
    } else if (constraint_x == CONSTRAINT_CENTER) {
        float center_ratio = (float)(*x + *w / 2) / (float)parent_old_w;
        int new_center = (int)(center_ratio * parent_new_w);
        *x = new_center - *w / 2;
    } else if (constraint_x == CONSTRAINT_SCALE) {
        float x_ratio = (float)*x / (float)parent_old_w;
        float w_ratio = (float)*w / (float)parent_old_w;
        *x = (int)(x_ratio * parent_new_w);
        *w = (int)(w_ratio * parent_new_w);
    } else if (constraint_x == CONSTRAINT_STRETCH) {
        int right_dist = parent_old_w - (*x + *w);
        *w = parent_new_w - *x - right_dist;
    }

    // Y Constraint Solving
    if (constraint_y == CONSTRAINT_MAX) { // Bottom aligned
        int dist_to_bottom = parent_old_h - (*y + *h);
        *y = parent_new_h - *h - dist_to_bottom;
    } else if (constraint_y == CONSTRAINT_CENTER) {
        float center_ratio = (float)(*y + *h / 2) / (float)parent_old_h;
        int new_center = (int)(center_ratio * parent_new_h);
        *y = new_center - *h / 2;
    } else if (constraint_y == CONSTRAINT_SCALE) {
        float y_ratio = (float)*y / (float)parent_old_h;
        float h_ratio = (float)*h / (float)parent_old_h;
        *y = (int)(y_ratio * parent_new_h);
        *h = (int)(h_ratio * parent_new_h);
    } else if (constraint_y == CONSTRAINT_STRETCH) {
        int bottom_dist = parent_old_h - (*y + *h);
        *h = parent_new_h - *y - bottom_dist;
    }
}

void tsfi_parc_figma_corner_smooth(
    float p0_x, float p0_y,
    float p1_x, float p1_y,
    float p2_x, float p2_y,
    float corner_radius,
    float *out_ctrl_x, float *out_ctrl_y
) {
    if (!out_ctrl_x || !out_ctrl_y) return;
    (void)p0_x; (void)p0_y; (void)p2_x; (void)p2_y;

    // Approximates the smooth Bezier control point offset based on corner radius
    *out_ctrl_x = p1_x - (corner_radius * 0.552f);
    *out_ctrl_y = p1_y + (corner_radius * 0.552f);
}

void tsfi_parc_figma_tidy_grid(
    tsfi_parc_layout_item_t *items, int item_count,
    int columns, int gap_x, int gap_y
) {
    if (!items || item_count <= 0 || columns <= 0) return;

    int cell_w = items[0].w;
    int cell_h = items[0].h;

    for (int i = 0; i < item_count; i++) {
        int r = i / columns;
        int c = i % columns;
        items[i].x = c * (cell_w + gap_x);
        items[i].y = r * (cell_h + gap_y);
    }
}
