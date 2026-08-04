#ifndef TSFI_QUADTREE_KSDS_H
#define TSFI_QUADTREE_KSDS_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "scripts/libantigravity_interop.h"

/*
 * Standard Quadtree KSDS disk database layout helper functions.
 */
bool tsfi_qt_ksds_write(
    const char *filepath,
    const char *tsv_header,
    const InteropQuadNode *nodes,
    size_t node_count,
    const uint8_t *records_data,
    size_t records_len
);

bool tsfi_qt_ksds_read(
    const char *filepath,
    char *header_out,
    size_t header_max,
    InteropQuadNode *nodes_out,
    size_t max_nodes,
    int *node_count_out,
    uint8_t *records_out,
    size_t max_records,
    int *records_len_out
);

bool tsfi_qt_ksds_aix_write(
    const char *filepath,
    const char *tsv_header,
    const InteropQuadNode *nodes,
    size_t node_count,
    const uint32_t *target_offsets,
    size_t target_count
);

bool tsfi_qt_ksds_aix_query(
    const char *aix_filepath,
    const char *primary_filepath,
    uint32_t secondary_x,
    uint32_t secondary_y,
    uint8_t *record_out,
    size_t max_record,
    int *record_len_out
);

#endif /* TSFI_QUADTREE_KSDS_H */
