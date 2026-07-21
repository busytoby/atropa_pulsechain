#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "tsfi_knuth_quadtree.h"
#include "tsfi_knuth_storage.h"

int tsfi_knuth_quadtree_resolve_node(
    int32_t x_coord,
    int32_t y_coord,
    uint32_t depth,
    tsfi_knuth_quadtree_node_t *out_node
) {
    if (!out_node || depth == 0) return -1;
    memset(out_node, 0, sizeof(tsfi_knuth_quadtree_node_t));
    out_node->quadtree_depth = depth;

    // 1. Pack 2D point (x, y) into Knuth Base 2i 64-bit word
    uint64_t packed_base2i = tsfi_knuth_pack_base2i(x_coord, y_coord);

    // 2. Truncate bits according to quadtree depth
    uint64_t mask = (depth >= 16) ? 0xFFFFFFFFFFFFFFFFULL : ((1ULL << (depth * 4)) - 1ULL);
    out_node->quad_node_address = packed_base2i & mask;

    // 3. Format .DAT.BIN filename enforcing Rule 13 (Only .dat.bin extension allowed)
    snprintf(out_node->dat_bin_filename, sizeof(out_node->dat_bin_filename),
             "QUAD_NODE_%016lX.DAT.BIN", (unsigned long)out_node->quad_node_address);

    printf("[KNUTH QUADTREE] Resolved Point (%d, %d) @ Depth %u -> Node 0x%016lX (%s)\n",
           x_coord, y_coord, depth, (unsigned long)out_node->quad_node_address, out_node->dat_bin_filename);

    return 0; // Quadtree node resolution complete
}
