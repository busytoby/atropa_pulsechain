#ifndef TSFI_PARC_SCHRACK_H
#define TSFI_PARC_SCHRACK_H

#include <stdint.h>
#include <stdbool.h>

/* G. F. Schrack Quadtree Directional Neighbor Enums */
typedef enum {
    SCHRACK_DIR_NORTH = 0,
    SCHRACK_DIR_SOUTH = 1,
    SCHRACK_DIR_EAST  = 2,
    SCHRACK_DIR_WEST  = 3,
    SCHRACK_DIR_NORTH_EAST = 4,
    SCHRACK_DIR_NORTH_WEST = 5,
    SCHRACK_DIR_SOUTH_EAST = 6,
    SCHRACK_DIR_SOUTH_WEST = 7
} tsfi_schrack_direction_t;

/* G. F. Schrack Quadcode Locational Code Structure */
typedef struct {
    uint64_t locational_code;
    uint32_t quadtree_depth;
    uint32_t coordinate_x;
    uint32_t coordinate_y;
} tsfi_schrack_quadcode_t;

/* Schrack Bitwise Integer Dilation: Interleaves bits of X and Y coordinates into 64-bit Morton quadcode */
uint64_t tsfi_schrack_integer_dilation(uint32_t coordinate_x, uint32_t coordinate_y);

/* Schrack Bitwise Integer Contraction: De-interleaves 64-bit Morton quadcode back into (X, Y) coordinates */
void tsfi_schrack_integer_contraction(uint64_t quadcode, uint32_t *coordinate_x_out, uint32_t *coordinate_y_out);

/* Schrack Constant-Time O(1) Equal-Size Neighbor Finding algorithm */
int tsfi_schrack_find_equal_neighbor(
    uint64_t input_quadcode,
    uint32_t quadtree_depth,
    tsfi_schrack_direction_t direction,
    uint64_t *neighbor_quadcode_out
);

/* Convert Morton Quadcode to Hilbert-Schrack Space-Filling Curve Index */
uint64_t tsfi_schrack_morton_to_hilbert(uint64_t morton_code, uint32_t quadtree_depth);

/* Quadtree .dat.bin Slice Spatial Index Verification (Rule 13) */
int tsfi_schrack_verify_quadtree_slice_asset(const char *quadtree_bin_filepath);

#endif // TSFI_PARC_SCHRACK_H
