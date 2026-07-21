#ifndef TSFI_KNUTH_QUADTREE_H
#define TSFI_KNUTH_QUADTREE_H

#include <stdint.h>
#include <stddef.h>

typedef struct {
    uint64_t quad_node_address;
    char dat_bin_filename[128]; // Must end strictly in .DAT.BIN (Rule 13)
    uint32_t quadtree_depth;
} tsfi_knuth_quadtree_node_t;

/* Convert 2D spatial point (x, y) into a Knuth Base 2i Quadtree .dat.bin Slice Node */
int tsfi_knuth_quadtree_resolve_node(
    int32_t x_coord,
    int32_t y_coord,
    uint32_t depth,
    tsfi_knuth_quadtree_node_t *out_node
);

#endif // TSFI_KNUTH_QUADTREE_H
