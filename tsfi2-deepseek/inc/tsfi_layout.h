#ifndef TSFI_LAYOUT_H
#define TSFI_LAYOUT_H

#include "tsfi_dom.h"

typedef enum {
    TSFI_BOX_BLOCK,
    TSFI_BOX_INLINE,
    TSFI_BOX_FLEX,
    TSFI_BOX_VIDEO
} TsfiBoxType;

typedef struct TsfiLayoutBox {
    TsfiBoxType type;
    TsfiNode *node;

    // Computed absolute bounds
    float x;
    float y;
    float w;
    float h;

    // Box model details (resolved styles)
    float margin_top, margin_bottom, margin_left, margin_right;
    float padding_top, padding_bottom, padding_left, padding_right;
    float border_top, border_bottom, border_left, border_right;

    // Layout tree pointers
    struct TsfiLayoutBox *parent;
    struct TsfiLayoutBox *first_child;
    struct TsfiLayoutBox *last_child;
    struct TsfiLayoutBox *prev_sibling;
    struct TsfiLayoutBox *next_sibling;
} TsfiLayoutBox;

// Layout lifecycle
TsfiLayoutBox* tsfi_layout_box_create(TsfiNode *node);
void tsfi_layout_box_free(TsfiLayoutBox *box);
void tsfi_layout_add_child(TsfiLayoutBox *parent, TsfiLayoutBox *child);

// Builds the layout tree recursively from the DOM tree, matching inline styles
TsfiLayoutBox* tsfi_layout_tree_build(TsfiNode *dom_node);

// Runs geometry reflow solver on the layout tree
void tsfi_layout_solve(TsfiLayoutBox *box, float containing_x, float containing_y, float containing_w, float containing_h);

// Debug prints layout tree coordinates
void tsfi_layout_print(const TsfiLayoutBox *box, int depth);

#endif // TSFI_LAYOUT_H
