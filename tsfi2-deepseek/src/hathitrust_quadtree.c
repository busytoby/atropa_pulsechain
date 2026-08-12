#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L

#include "hathitrust_quadtree.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static bool has_dat_bin_extension(const char *path) {
    size_t len = strlen(path);
    if (len < 8) return false;
    return strcmp(path + len - 8, ".dat.bin") == 0;
}

static void build_and_write_node(FILE *f, const HtrcQuadtreePoint *points, size_t count, double min_x, double min_y, double max_x, double max_y) {
    HtrcQuadtreeNode node;
    memset(&node, 0, sizeof(node));
    node.min_x = min_x;
    node.min_y = min_y;
    node.max_x = max_x;
    node.max_y = max_y;
    
    // Boundary check / partition
    size_t inside_count = 0;
    HtrcQuadtreePoint inside_points[100]; // temporary limit for simple implementation
    
    for (size_t i = 0; i < count; i++) {
        if (points[i].x >= min_x && points[i].x <= max_x &&
            points[i].y >= min_y && points[i].y <= max_y) {
            if (inside_count < 100) {
                inside_points[inside_count++] = points[i];
            }
        }
    }
    
    if (inside_count <= 4) {
        node.points_count = (int)inside_count;
        for (size_t i = 0; i < inside_count; i++) {
            node.points[i] = inside_points[i];
        }
        fwrite(&node, sizeof(HtrcQuadtreeNode), 1, f);
    } else {
        // Subdivide
        double mid_x = min_x + (max_x - min_x) / 2.0;
        double mid_y = min_y + (max_y - min_y) / 2.0;
        
        long current_offset = ftell(f);
        // Write placeholder node
        fwrite(&node, sizeof(HtrcQuadtreeNode), 1, f);
        
        // NW
        node.nw_offset = ftell(f);
        build_and_write_node(f, inside_points, inside_count, min_x, mid_y, mid_x, max_y);
        
        // NE
        node.ne_offset = ftell(f);
        build_and_write_node(f, inside_points, inside_count, mid_x, mid_y, max_x, max_y);
        
        // SW
        node.sw_offset = ftell(f);
        build_and_write_node(f, inside_points, inside_count, min_x, min_y, mid_x, mid_y);
        
        // SE
        node.se_offset = ftell(f);
        build_and_write_node(f, inside_points, inside_count, mid_x, min_y, max_x, mid_y);
        
        // Update offsets in parent node
        fseek(f, current_offset, SEEK_SET);
        fwrite(&node, sizeof(HtrcQuadtreeNode), 1, f);
        fseek(f, 0, SEEK_END);
    }
}

bool hathitrust_quadtree_serialize(const char *filepath, const HtrcQuadtreePoint *points, size_t count) {
    if (!filepath || !points || count == 0) return false;
    if (!has_dat_bin_extension(filepath)) {
        fprintf(stderr, "[ERROR] Quadtree indexer only supports .dat.bin extension.\n");
        return false;
    }
    
    FILE *f = fopen(filepath, "wb");
    if (!f) return false;
    
    // Determine global boundaries
    double min_x = points[0].x, max_x = points[0].x;
    double min_y = points[0].y, max_y = points[0].y;
    
    for (size_t i = 1; i < count; i++) {
        if (points[i].x < min_x) min_x = points[i].x;
        if (points[i].x > max_x) max_x = points[i].x;
        if (points[i].y < min_y) min_y = points[i].y;
        if (points[i].y > max_y) max_y = points[i].y;
    }
    
    build_and_write_node(f, points, count, min_x, min_y, max_x, max_y);
    
    fclose(f);
    return true;
}

#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

const HtrcQuadtreeNode* hathitrust_quadtree_mmap_node(const char *filepath, size_t offset, size_t *out_file_size) {
    if (!filepath || !has_dat_bin_extension(filepath)) return NULL;

    int fd = open(filepath, O_RDONLY);
    if (fd < 0) return NULL;

    struct stat st;
    if (fstat(fd, &st) < 0) {
        close(fd);
        return NULL;
    }

    if (out_file_size) *out_file_size = (size_t)st.st_size;

    void *map = mmap(NULL, st.st_size, PROT_READ, MAP_SHARED, fd, 0);
    close(fd);

    if (map == MAP_FAILED) return NULL;
    if (offset + sizeof(HtrcQuadtreeNode) > (size_t)st.st_size) {
        munmap(map, st.st_size);
        return NULL;
    }

    return (const HtrcQuadtreeNode*)((const char*)map + offset);
}
