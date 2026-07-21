#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "tsfi_autodin_cumulative_quadtree.h"

int autodin_cumulative_quadtree_initialize(
    const char *dataset_filename,
    autodin_cumulative_quadtree_storage_t *storage
) {
    if (!storage) return -1;

    memset(storage, 0, sizeof(autodin_cumulative_quadtree_storage_t));
    storage->root_node.minimum_x_coordinate = -1000.0;
    storage->root_node.minimum_y_coordinate = -1000.0;
    storage->root_node.maximum_x_coordinate =  1000.0;
    storage->root_node.maximum_y_coordinate =  1000.0;
    storage->root_node.node_depth_level = 0;
    storage->maximum_depth_capacity = 8;
    storage->total_quadtree_nodes = 1;

    if (dataset_filename) {
        // Enforce Rule 13: Quadtree storage layout strictly (.DAT.BIN)
        size_t filename_length = strlen(dataset_filename);
        if (filename_length > 8 && strcmp(dataset_filename + filename_length - 8, ".DAT.BIN") == 0) {
            snprintf(storage->root_node.binary_dataset_path, sizeof(storage->root_node.binary_dataset_path), "%s", dataset_filename);
        } else {
            snprintf(storage->root_node.binary_dataset_path, sizeof(storage->root_node.binary_dataset_path), "%s.DAT.BIN", dataset_filename);
        }
    } else {
        snprintf(storage->root_node.binary_dataset_path, sizeof(storage->root_node.binary_dataset_path), "AUTODIN_QUADTREE.DAT.BIN");
    }

    return 0;
}

int autodin_cumulative_quadtree_insert_transaction(
    autodin_cumulative_quadtree_storage_t *storage,
    double x_position,
    double y_position,
    uint64_t transaction_hash
) {
    if (!storage) return -1;

    if (x_position < storage->root_node.minimum_x_coordinate || x_position > storage->root_node.maximum_x_coordinate ||
        y_position < storage->root_node.minimum_y_coordinate || y_position > storage->root_node.maximum_y_coordinate) {
        return -1;
    }

    storage->root_node.accumulated_transaction_hash ^= transaction_hash;
    storage->total_quadtree_nodes++;
    return 0;
}

int autodin_cumulative_quadtree_save_binary(
    const autodin_cumulative_quadtree_storage_t *storage,
    const char *output_binary_path
) {
    if (!storage || !output_binary_path) return -1;

    // Enforce Rule 13: Only .DAT.BIN storage format allowed on disk
    size_t path_length = strlen(output_binary_path);
    if (path_length <= 8 || strcmp(output_binary_path + path_length - 8, ".DAT.BIN") != 0) {
        printf("[AUTOMATED DIGITAL NETWORK QUADTREE ERROR] File extension must be strictly .DAT.BIN (Rule 13)\n");
        return -1;
    }

    FILE *binary_file_pointer = fopen(output_binary_path, "wb");
    if (!binary_file_pointer) return -1;

    fwrite(storage, sizeof(autodin_cumulative_quadtree_storage_t), 1, binary_file_pointer);
    fclose(binary_file_pointer);

    printf("[AUTOMATED DIGITAL NETWORK QUADTREE SUCCESS] Quadtree Binary Saved to %s\n", output_binary_path);
    return 0;
}
