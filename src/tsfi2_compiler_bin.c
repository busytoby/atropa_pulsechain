#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L

#include "tsfi2_compiler_bin.h"
#include "tsfi2-deepseek/inc/tsfi_mainframe_computerworld.h"
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

    remove(filepath);
    tsfi_cw_vsam_ksds ksds;
    memset(&ksds, 0, sizeof(ksds));
    if (tsfi_cw_vsam_open(&ksds, filepath) != 0) {
        return false;
    }

    // Write binary program bytecode under key "PROG"
    if (tsfi_cw_vsam_write(&ksds, "PROG", bytecode, bytecode_len) != 0) {
        return false;
    }

    // Write metadata
    char entry_str[32];
    snprintf(entry_str, sizeof(entry_str), "0x%X", entry_point);
    if (tsfi_cw_vsam_write(&ksds, "ENT", (const uint8_t *)entry_str, strlen(entry_str)) != 0) {
        return false;
    }

    char count_str[32];
    snprintf(count_str, sizeof(count_str), "%u", instruction_count);
    if (tsfi_cw_vsam_write(&ksds, "CNT", (const uint8_t *)count_str, strlen(count_str)) != 0) {
        return false;
    }

    if (custom_keys && custom_values) {
        if (tsfi_cw_vsam_write(&ksds, "MKY", (const uint8_t *)custom_keys, strlen(custom_keys)) != 0) {
            return false;
        }
        if (tsfi_cw_vsam_write(&ksds, "MVL", (const uint8_t *)custom_values, strlen(custom_values)) != 0) {
            return false;
        }
    }

    uint64_t hash = calculate_fnv1a(bytecode, bytecode_len);

    char hash_str[32];
    snprintf(hash_str, sizeof(hash_str), "%llu", (unsigned long long)hash);
    if (tsfi_cw_vsam_write(&ksds, "HSH", (const uint8_t *)hash_str, strlen(hash_str)) != 0) {
        return false;
    }

    return true;
}

bool tsfi2_compile_to_dat_bin(
    const char *filepath,
    uint32_t entry_point,
    uint32_t instruction_count,
    const uint8_t *bytecode,
    size_t bytecode_len
) {
    return tsfi2_compile_to_dat_bin_ext(filepath, entry_point, instruction_count, NULL, NULL, bytecode, bytecode_len);
}
