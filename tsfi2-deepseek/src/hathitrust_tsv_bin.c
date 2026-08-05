#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L

#include "hathitrust_tsv_bin.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static uint64_t calculate_fnv1a(const uint8_t *data, size_t len) {
    uint64_t hash = 14695981039346656037ULL;
    for (size_t i = 0; i < len; i++) {
        hash ^= data[i];
        hash *= 1099511628211ULL;
    }
    return hash;
}

static bool verify_extension(const char *filepath) {
    size_t len = strlen(filepath);
    if (len < 8) return false;
    return (strcmp(filepath + len - 8, ".dat.bin") == 0);
}

bool hathitrust_tsv_bin_serialize(
    const char *filepath,
    const char *tsv_header,
    const uint8_t *payload,
    size_t payload_len
) {
    if (!filepath || !tsv_header || !payload || !verify_extension(filepath)) {
        return false;
    }
    
    size_t header_len = strlen(tsv_header);
    size_t total_written = header_len + 2;
    size_t aligned_offset = ((total_written + 511) / 512) * 512;
    size_t data_size = aligned_offset + payload_len;
    
    uint8_t *buffer = calloc(1, data_size);
    if (!buffer) return false;
    
    // Copy header
    memcpy(buffer, tsv_header, header_len);
    buffer[header_len] = '\n';
    buffer[header_len + 1] = '\n';
    
    // Copy payload after alignment padding
    memcpy(buffer + aligned_offset, payload, payload_len);
    
    // Calculate FNV-1a checksum
    uint64_t checksum = calculate_fnv1a(buffer, data_size);
    
    FILE *f = fopen(filepath, "wb");
    if (!f) {
        free(buffer);
        return false;
    }
    
    // Write full data buffer
    fwrite(buffer, 1, data_size, f);
    free(buffer);
    
    // Write checksum at the end
    fwrite(&checksum, 1, sizeof(checksum), f);
    
    fclose(f);
    return true;
}

bool hathitrust_tsv_bin_deserialize(const char *filepath, HtrcTsvBinBlock *out_block) {
    if (!filepath || !out_block || !verify_extension(filepath)) {
        return false;
    }
    
    FILE *f = fopen(filepath, "rb");
    if (!f) return false;
    
    fseek(f, 0, SEEK_END);
    long total_size = ftell(f);
    if (total_size < 16) { // Min header + boundary + checksum
        fclose(f);
        return false;
    }
    
    fseek(f, 0, SEEK_SET);
    uint8_t *buffer = malloc(total_size);
    if (!buffer) {
        fclose(f);
        return false;
    }
    size_t read_bytes = fread(buffer, 1, total_size, f);
    (void)read_bytes;
    fclose(f);
    
    // Verify checksum in last 8 bytes
    size_t data_len = total_size - 8;
    uint64_t file_checksum;
    memcpy(&file_checksum, buffer + data_len, 8);
    
    uint64_t computed = calculate_fnv1a(buffer, data_len);
    if (computed != file_checksum) {
        free(buffer);
        return false;
    }
    
    // Find boundary separator
    uint8_t *boundary = NULL;
    for (size_t i = 0; i < data_len - 1; i++) {
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
    bool originally_ended_with_newline = (boundary + 2 < buffer + data_len && boundary[2] == '\n');
    size_t alloc_len = originally_ended_with_newline ? header_len + 1 : header_len;
    
    out_block->tsv_header = malloc(alloc_len + 1);
    if (!out_block->tsv_header) {
        free(buffer);
        return false;
    }
    memcpy(out_block->tsv_header, buffer, header_len);
    if (originally_ended_with_newline) {
        out_block->tsv_header[header_len] = '\n';
        out_block->tsv_header[header_len + 1] = '\n'; // wait, index was alloc_len which is header_len + 1, so out_block->tsv_header[header_len] = '\n', out_block->tsv_header[header_len + 1] = '\0'
    } else {
        out_block->tsv_header[header_len] = '\0';
    }
    
    // Correctly null terminate the allocated length
    out_block->tsv_header[alloc_len] = '\0';
    out_block->header_len = alloc_len;
    
    size_t total_written = originally_ended_with_newline ? header_len + 3 : header_len + 2;
    size_t aligned_offset = ((total_written + 511) / 512) * 512;
    
    if (aligned_offset > data_len) {
        free(out_block->tsv_header);
        free(buffer);
        return false;
    }
    
    out_block->binary_len = data_len - aligned_offset;
    out_block->binary_payload = malloc(out_block->binary_len);
    if (!out_block->binary_payload) {
        free(out_block->tsv_header);
        free(buffer);
        return false;
    }
    memcpy(out_block->binary_payload, buffer + aligned_offset, out_block->binary_len);
    out_block->checksum = file_checksum;
    
    free(buffer);
    return true;
}

void hathitrust_tsv_bin_free_block(HtrcTsvBinBlock *block) {
    if (!block) return;
    free(block->tsv_header);
    free(block->binary_payload);
    memset(block, 0, sizeof(HtrcTsvBinBlock));
}
