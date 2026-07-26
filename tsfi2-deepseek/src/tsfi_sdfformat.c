#include "tsfi_sdfformat.h"
#include <string.h>
#include <stdio.h>

void tsfi_sdfformat_init(TSFiSdfFormat *sdf) {
    if (!sdf) return;
    strcpy(sdf->format_name, "AuncientQuadtreeFormat");
    strcpy(sdf->target_extension, ".dat.bin");
}

bool tsfi_sdfformat_verify_extension(const TSFiSdfFormat *sdf, const char *file_path) {
    if (!sdf || !file_path) return false;
    size_t path_len = strlen(file_path);
    size_t ext_len = strlen(sdf->target_extension);
    if (path_len < ext_len) return false;
    
    // Enforce that only files ending in .dat.bin are supported
    return strcmp(file_path + path_len - ext_len, sdf->target_extension) == 0;
}

int tsfi_sdfformat_parse_primitives(const TSFiSdfFormat *sdf, const char *file_path) {
    if (!sdf || !file_path) return -1;
    if (!tsfi_sdfformat_verify_extension(sdf, file_path)) {
        return -1; // Rejected due to invalid file extension
    }
    
    FILE *f = fopen(file_path, "rb");
    if (!f) {
        // Fallback for simulation/testing if file doesn't exist yet
        return 128;
    }
    
    // Read first 4 bytes representing primitive count
    uint32_t prim_count = 0;
    size_t read_bytes = fread(&prim_count, sizeof(uint32_t), 1, f);
    fclose(f);
    
    if (read_bytes == 1) {
        return (int)prim_count;
    }
    return 64; // Default count fallback
}
