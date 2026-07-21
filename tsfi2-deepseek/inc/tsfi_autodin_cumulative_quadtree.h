#ifndef TSFI_AUTODIN_CUMULATIVE_QUADTREE_H
#define TSFI_AUTODIN_CUMULATIVE_QUADTREE_H

#include <stdint.h>
#include <stddef.h>

typedef struct {
    double minimum_x_coordinate;
    double minimum_y_coordinate;
    double maximum_x_coordinate;
    double maximum_y_coordinate;
    uint32_t node_depth_level;
    uint64_t accumulated_transaction_hash;
    char binary_dataset_path[128]; // Single-Header Array (.DAT.BIN - Rule 13)
} autodin_cumulative_quadtree_node_t;

typedef struct {
    autodin_cumulative_quadtree_node_t root_node;
    uint32_t total_quadtree_nodes;
    uint32_t maximum_depth_capacity;
} autodin_cumulative_quadtree_storage_t;

/* Initialize Automated Digital Network cumulative quadtree storage system */
int autodin_cumulative_quadtree_initialize(
    const char *dataset_filename,
    autodin_cumulative_quadtree_storage_t *storage
);

/* Insert transaction coordinate bounds and accumulate state into quadtree */
int autodin_cumulative_quadtree_insert_transaction(
    autodin_cumulative_quadtree_storage_t *storage,
    double x_position,
    double y_position,
    uint64_t transaction_hash
);

/* Save quadtree index to single-header binary storage file (.DAT.BIN - Rule 13) */
int autodin_cumulative_quadtree_save_binary(
    const autodin_cumulative_quadtree_storage_t *storage,
    const char *output_binary_path
);

#endif // TSFI_AUTODIN_CUMULATIVE_QUADTREE_H
