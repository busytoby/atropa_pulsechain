#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L

#include "tsfi2_compiler_bin.h"
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

bool tsfi2_compile_to_dat_bin(
    const char *filepath,
    uint32_t entry_point,
    uint32_t instruction_count,
    const uint8_t *bytecode,
    size_t bytecode_len
) {
    if (!filepath || !bytecode || bytecode_len == 0 || !verify_extension(filepath)) {
        return false;
    }
    
    char tsv_header[256];
    int written = snprintf(tsv_header, sizeof(tsv_header),
                           "EntryAddress\tInstructionCount\n0x%X\t%u\n",
                           entry_point, instruction_count);
                           
    if (written <= 0 || (size_t)written >= sizeof(tsv_header)) {
        return false;
    }
    
    size_t header_len = strlen(tsv_header);
    size_t total_written = header_len + 2;
    size_t aligned_offset = ((total_written + 511) / 512) * 512;
    size_t data_size = aligned_offset + bytecode_len;
    
    uint8_t *buffer = calloc(1, data_size);
    if (!buffer) return false;
    
    // Copy TSV header
    memcpy(buffer, tsv_header, header_len);
    buffer[header_len] = '\n';
    buffer[header_len + 1] = '\n';
    
    // Copy bytecode payload after alignment padding
    memcpy(buffer + aligned_offset, bytecode, bytecode_len);
    
    // Calculate checksum of complete data block
    uint64_t checksum = calculate_fnv1a(buffer, data_size);
    
    FILE *f = fopen(filepath, "wb");
    if (!f) {
        free(buffer);
        return false;
    }
    
    // Write data block and checksum
    fwrite(buffer, 1, data_size, f);
    free(buffer);
    
    fwrite(&checksum, 1, sizeof(checksum), f);
    fclose(f);
    
    return true;
}

bool tsfi2_compile_to_dat_bin_ext(
    const char *filepath,
    uint32_t entry_point,
    uint32_t instruction_count,
    const char *custom_keys,
    const char *custom_values,
    const uint8_t *bytecode,
    size_t bytecode_len
) {
    if (!filepath || !bytecode || bytecode_len == 0 || !verify_extension(filepath)) {
        return false;
    }
    
    char tsv_header[512];
    int written;
    if (custom_keys && custom_values) {
        written = snprintf(tsv_header, sizeof(tsv_header),
                           "EntryAddress\tInstructionCount\t%s\n0x%X\t%u\t%s\n",
                           custom_keys, entry_point, instruction_count, custom_values);
    } else {
        written = snprintf(tsv_header, sizeof(tsv_header),
                           "EntryAddress\tInstructionCount\n0x%X\t%u\n",
                           entry_point, instruction_count);
    }
                           
    if (written <= 0 || (size_t)written >= sizeof(tsv_header)) {
        return false;
    }
    
    size_t header_len = strlen(tsv_header);
    size_t total_written = header_len + 2;
    size_t aligned_offset = ((total_written + 511) / 512) * 512;
    size_t data_size = aligned_offset + bytecode_len;
    
    uint8_t *buffer = calloc(1, data_size);
    if (!buffer) return false;
    
    memcpy(buffer, tsv_header, header_len);
    buffer[header_len] = '\n';
    buffer[header_len + 1] = '\n';
    
    memcpy(buffer + aligned_offset, bytecode, bytecode_len);
    
    uint64_t checksum = calculate_fnv1a(buffer, data_size);
    
    FILE *f = fopen(filepath, "wb");
    if (!f) {
        free(buffer);
        return false;
    }
    
    fwrite(buffer, 1, data_size, f);
    free(buffer);
    
    fwrite(&checksum, 1, sizeof(checksum), f);
    fclose(f);
    
    return true;
}
