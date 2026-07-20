#ifndef TSFI_PARC_FIGMA_H
#define TSFI_PARC_FIGMA_H

#include <stdint.h>

typedef struct {
    float x;
    float y;
} tsfi_parc_vector_node_t;

typedef struct {
    int start_node_idx;
    int end_node_idx;
} tsfi_parc_vector_edge_t;

typedef struct {
    int x;
    int y;
    int w;
    int h;
} tsfi_parc_layout_item_t;

typedef struct {
    int x;
    int y;
    char label[16];
    uint32_t color;
} tsfi_parc_figma_cursor_t;

/* 1. Figma Vector Networks: Draw nodes and arbitrary multi-directional joining edges */
void tsfi_parc_figma_draw_vector_network(
    uint32_t *pixels, int width, int height,
    const tsfi_parc_vector_node_t *nodes, int node_count,
    const tsfi_parc_vector_edge_t *edges, int edge_count,
    uint32_t color
);

/* 2. Figma Auto Layout Flex Engine: Automatically arranges layout items in horizontal or vertical flow */
void tsfi_parc_figma_auto_layout(
    tsfi_parc_layout_item_t *items, int item_count,
    int is_vertical, int padding_x, int padding_y, int gap,
    int container_w, int container_h
);

/* 3. Figma Collaborative Multi-Cursor Mixer: Blends multiple users' cursor markers and labels */
void tsfi_parc_figma_draw_cursors(
    uint32_t *pixels, int width, int height,
    const tsfi_parc_figma_cursor_t *cursors, int cursor_count
);

#endif // TSFI_PARC_FIGMA_H
