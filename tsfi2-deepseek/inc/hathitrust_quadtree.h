#ifndef HATHITRUST_QUADTREE_H
#define HATHITRUST_QUADTREE_H

#include <stdbool.h>
#include <stddef.h>

typedef struct {
    double x; // e.g. Publication date
    double y; // e.g. Normalized numeric ID/OCLC
    char htid[64];
    char title[128];
} HtrcQuadtreePoint;

typedef struct {
    double min_x, min_y, max_x, max_y;
    int points_count;
    HtrcQuadtreePoint points[4];
    
    // Child node file offsets inside the .dat.bin database
    long nw_offset;
    long ne_offset;
    long sw_offset;
    long se_offset;
} HtrcQuadtreeNode;

/*
 * Serializes and writes quadtree nodes to a binary .dat.bin file.
 * Parameters:
 *   filepath: The absolute path of the destination file (must end in .dat.bin).
 *   points: Array of points to index.
 *   count: Number of points.
 * Returns:
 *   true on successful serialization, false on error.
 */
bool hathitrust_quadtree_serialize(const char *filepath, const HtrcQuadtreePoint *points, size_t count);

/*
 * Zero-copy mmap reader for .dat.bin quadtree database files.
 */
const HtrcQuadtreeNode* hathitrust_quadtree_mmap_node(const char *filepath, size_t offset, size_t *out_file_size);

#endif /* HATHITRUST_QUADTREE_H */
