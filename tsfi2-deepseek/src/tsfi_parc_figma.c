#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "tsfi_parc_figma.h"

// Helper: draw simple line segment
static void draw_line(uint32_t *pixels, int width, int height, int x0, int y0, int x1, int y1, uint32_t color) {
    int dx = abs(x1 - x0);
    int dy = abs(y1 - y0);
    int sx = x0 < x1 ? 1 : -1;
    int sy = y0 < y1 ? 1 : -1;
    int err = dx - dy;

    while (1) {
        if (x0 >= 0 && x0 < width && y0 >= 0 && y0 < height) {
            pixels[y0 * width + x0] = color;
        }
        if (x0 == x1 && y0 == y1) break;
        int e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x0 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y0 += sy;
        }
    }
}

void tsfi_parc_figma_draw_vector_network(
    uint32_t *pixels, int width, int height,
    const tsfi_parc_vector_node_t *nodes, int node_count,
    const tsfi_parc_vector_edge_t *edges, int edge_count,
    uint32_t color
) {
    if (!pixels || !nodes || !edges || width <= 0 || height <= 0) return;

    for (int i = 0; i < edge_count; i++) {
        int s_idx = edges[i].start_node_idx;
        int e_idx = edges[i].end_node_idx;
        if (s_idx >= 0 && s_idx < node_count && e_idx >= 0 && e_idx < node_count) {
            draw_line(pixels, width, height,
                      (int)nodes[s_idx].x, (int)nodes[s_idx].y,
                      (int)nodes[e_idx].x, (int)nodes[e_idx].y, color);
        }
    }
}

void tsfi_parc_figma_auto_layout(
    tsfi_parc_layout_item_t *items, int item_count,
    int is_vertical, int padding_x, int padding_y, int gap,
    int container_w, int container_h
) {
    if (!items || item_count <= 0) return;

    int current_pos = is_vertical ? padding_y : padding_x;

    for (int i = 0; i < item_count; i++) {
        if (is_vertical) {
            items[i].x = padding_x;
            items[i].y = current_pos;
            current_pos += items[i].h + gap;
            
            // Constrain to container
            if (items[i].x + items[i].w > container_w - padding_x) {
                items[i].w = container_w - padding_x - items[i].x;
            }
        } else {
            items[i].x = current_pos;
            items[i].y = padding_y;
            current_pos += items[i].w + gap;

            if (items[i].y + items[i].h > container_h - padding_y) {
                items[i].h = container_h - padding_y - items[i].y;
            }
        }
    }
}

void tsfi_parc_figma_draw_cursors(
    uint32_t *pixels, int width, int height,
    const tsfi_parc_figma_cursor_t *cursors, int cursor_count
) {
    if (!pixels || !cursors || width <= 0 || height <= 0) return;

    for (int c = 0; c < cursor_count; c++) {
        int cx = cursors[c].x;
        int cy = cursors[c].y;

        // Draw a simple 8x8 triangular colored cursor pointer
        for (int dy = 0; dy < 8; dy++) {
            for (int dx = 0; dx <= dy; dx++) {
                int px = cx + dx;
                int py = cy + dy;
                if (px >= 0 && px < width && py >= 0 && py < height) {
                    pixels[py * width + px] = cursors[c].color;
                }
            }
        }

        // Draw a basic background tag for label text
        int label_len = (int)strlen(cursors[c].label);
        int tag_w = label_len * 6 + 4;
        int tag_h = 10;
        int tag_x = cx + 8;
        int tag_y = cy + 8;

        for (int dy = 0; dy < tag_h; dy++) {
            for (int dx = 0; dx < tag_w; dx++) {
                int px = tag_x + dx;
                int py = tag_y + dy;
                if (px >= 0 && px < width && py >= 0 && py < height) {
                    pixels[py * width + px] = cursors[c].color; // Draw solid label box
                }
            }
        }
    }
}
