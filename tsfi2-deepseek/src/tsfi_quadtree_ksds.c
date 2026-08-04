#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L

#include "tsfi_quadtree_ksds.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

bool tsfi_qt_ksds_write(
    const char *filepath,
    const char *tsv_header,
    const InteropQuadNode *nodes,
    size_t node_count,
    const uint8_t *records_data,
    size_t records_len
) {
    if (!filepath || !tsv_header || !nodes || node_count == 0) return false;

    FILE *f = fopen(filepath, "wb");
    if (!f) return false;

    // Write TSV header
    size_t header_len = strlen(tsv_header);
    fwrite(tsv_header, 1, header_len, f);
    fwrite("\n\n", 1, 2, f);

    // Sector alignment padding (512 bytes)
    size_t total_written = header_len + 2;
    size_t aligned_offset = ((total_written + 511) / 512) * 512;
    size_t padding = aligned_offset - total_written;
    if (padding > 0) {
        uint8_t *pad = calloc(1, padding);
        fwrite(pad, 1, padding, f);
        free(pad);
    }

    // Write Quadtree index nodes
    fwrite(nodes, sizeof(InteropQuadNode), node_count, f);

    // Write record data
    if (records_data && records_len > 0) {
        fwrite(records_data, 1, records_len, f);
    }

    fclose(f);
    return true;
}

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
) {
    if (!filepath) return false;

    FILE *f = fopen(filepath, "rb");
    if (!f) return false;

    fseek(f, 0, SEEK_END);
    long total_size = ftell(f);
    fseek(f, 0, SEEK_SET);

    uint8_t *buffer = malloc(total_size);
    if (!buffer) {
        fclose(f);
        return false;
    }
    size_t read_bytes = fread(buffer, 1, total_size, f);
    (void)read_bytes;
    fclose(f);

    // Find double newline boundary separating TSV header
    uint8_t *boundary = NULL;
    for (long i = 0; i < total_size - 1; i++) {
        if (buffer[i] == '\n' && buffer[i+1] == '\n') {
            boundary = buffer + i;
            break;
        }
    }
    if (!boundary) {
        free(buffer);
        return false;
    }

    size_t header_len = boundary - buffer;
    if (header_out && header_max > 0) {
        size_t cpy = (header_len < header_max - 1) ? header_len : header_max - 1;
        memcpy(header_out, buffer, cpy);
        header_out[cpy] = '\0';
    }

    size_t aligned_offset = ((header_len + 2 + 511) / 512) * 512;
    if (aligned_offset > (size_t)total_size) {
        free(buffer);
        return false;
    }

    // Parse counts from TSV header
    int quad_count = 0;
    int record_len = 0;
    char *hdr = malloc(header_len + 1);
    if (hdr) {
        memcpy(hdr, buffer, header_len);
        hdr[header_len] = '\0';
        char *line = strstr(hdr, "QuadtreeCount:");
        if (line) {
            sscanf(line, "QuadtreeCount:\t%d", &quad_count);
        }
        line = strstr(hdr, "RecordCount:");
        if (line) {
            sscanf(line, "RecordCount:\t%d", &record_len);
        }
        free(hdr);
    }

    if (quad_count <= 0) {
        free(buffer);
        return false;
    }

    // Read Quadtree nodes
    if (nodes_out && max_nodes > 0) {
        size_t copy_nodes = ((size_t)quad_count < max_nodes) ? (size_t)quad_count : max_nodes;
        memcpy(nodes_out, buffer + aligned_offset, copy_nodes * sizeof(InteropQuadNode));
    }
    if (node_count_out) {
        *node_count_out = quad_count;
    }

    // Read record data payload
    size_t rec_offset = aligned_offset + quad_count * sizeof(InteropQuadNode);
    if (rec_offset < (size_t)total_size) {
        size_t avail_rec_len = total_size - rec_offset;
        if (records_out && max_records > 0) {
            size_t cpy_rec = (avail_rec_len < max_records) ? avail_rec_len : max_records;
            memcpy(records_out, buffer + rec_offset, cpy_rec);
        }
        if (records_len_out) {
            *records_len_out = (int)avail_rec_len;
        }
    } else {
        if (records_len_out) {
            *records_len_out = 0;
        }
    }

    free(buffer);
    return true;
}
