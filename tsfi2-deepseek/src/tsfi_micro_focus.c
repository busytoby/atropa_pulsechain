#include "tsfi_micro_focus.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// COMP-5 Encoding: Native binary representation (little-endian on x86/ARM)
int tsfi_mf_comp5_encode(int64_t value, uint8_t *buffer, int size_bytes) {
    if (!buffer || (size_bytes != 1 && size_bytes != 2 && size_bytes != 4 && size_bytes != 8)) {
        return -1;
    }
    // Perform native store
    for (int i = 0; i < size_bytes; i++) {
        buffer[i] = (uint8_t)((value >> (8 * i)) & 0xFF);
    }
    return 0;
}

// COMP-5 Decoding: Native binary representation
int64_t tsfi_mf_comp5_decode(const uint8_t *buffer, int size_bytes, int is_signed) {
    if (!buffer || (size_bytes != 1 && size_bytes != 2 && size_bytes != 4 && size_bytes != 8)) {
        return 0;
    }
    uint64_t raw_val = 0;
    for (int i = 0; i < size_bytes; i++) {
        raw_val |= ((uint64_t)buffer[i] << (8 * i));
    }

    if (is_signed) {
        if (size_bytes == 1) return (int8_t)raw_val;
        if (size_bytes == 2) return (int16_t)raw_val;
        if (size_bytes == 4) return (int32_t)raw_val;
        return (int64_t)raw_val;
    }
    return (int64_t)raw_val;
}

// ISAM Record Writer: Write keys and values sequentially to .dat.bin files
int tsfi_mf_isam_write_record(const char *filepath_dat_bin, uint32_t key, const uint8_t *record_data, int record_len) {
    if (!filepath_dat_bin || !record_data || record_len <= 0) return -1;

    // Enforce Rule 13: Only allow .dat.bin extension
    int len = strlen(filepath_dat_bin);
    if (len < 8 || strcmp(filepath_dat_bin + len - 8, ".dat.bin") != 0) {
        return -2;
    }

    FILE *f = fopen(filepath_dat_bin, "ab");
    if (!f) return -3;

    // Write header: 4-byte Key, 4-byte length
    fwrite(&key, 1, 4, f);
    fwrite(&record_len, 1, 4, f);
    fwrite(record_data, 1, record_len, f);

    fclose(f);
    return 0;
}

// ISAM Record Reader: Search keys sequentially from .dat.bin files
int tsfi_mf_isam_read_record(const char *filepath_dat_bin, uint32_t key, uint8_t *record_out, int record_len) {
    if (!filepath_dat_bin || !record_out || record_len <= 0) return -1;

    // Enforce Rule 13
    int len = strlen(filepath_dat_bin);
    if (len < 8 || strcmp(filepath_dat_bin + len - 8, ".dat.bin") != 0) {
        return -2;
    }

    FILE *f = fopen(filepath_dat_bin, "rb");
    if (!f) return -3;

    uint32_t cur_key = 0;
    int cur_len = 0;

    while (fread(&cur_key, 1, 4, f) == 4) {
        if (fread(&cur_len, 1, 4, f) != 4) {
            fclose(f);
            return -4;
        }

        if (cur_key == key) {
            int to_read = (cur_len < record_len) ? cur_len : record_len;
            if (fread(record_out, 1, to_read, f) != (size_t)to_read) {
                fclose(f);
                return -5;
            }
            fclose(f);
            return 0;
        } else {
            // Seek past payload
            if (fseek(f, cur_len, SEEK_CUR) != 0) {
                fclose(f);
                return -6;
            }
        }
    }

    fclose(f);
    return -7; // Not found
}
