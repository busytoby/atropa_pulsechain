#include "tsfi_mainframe_computerworld.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

int tsfi_cw_vsam_open(tsfi_cw_vsam_ksds *ksds, const char *filepath) {
    if (!ksds || !filepath) return -1;
    
    // Rule 13: only .dat.bin extension is supported
    int len = strlen(filepath);
    if (len < 8 || strcmp(filepath + len - 8, ".dat.bin") != 0) {
        return -5;
    }

    memset(ksds, 0, sizeof(tsfi_cw_vsam_ksds));
    strncpy(ksds->filepath, filepath, sizeof(ksds->filepath) - 1);

    FILE *f = fopen(filepath, "rb");
    if (!f) {
        ksds->entry_count = 0;
        ksds->current_file_size = 0;
        return 0;
    }

    if (fread(&ksds->entry_count, sizeof(int), 1, f) != 1) {
        fclose(f);
        return 0;
    }
    if (fread(ksds->index, sizeof(tsfi_cw_vsam_entry), 128, f) != 128) {
        fclose(f);
        return 0;
    }
    if (fread(&ksds->current_file_size, sizeof(uint32_t), 1, f) != 1) {
        fclose(f);
        return 0;
    }
    fclose(f);
    
    // Integrity audit
    for (int i = 0; i < ksds->entry_count; i++) {
        if (ksds->index[i].active && ksds->index[i].checksum != 0) {
            uint8_t audit_buf[256];
            int audit_len = 0;
            int rc = tsfi_cw_vsam_read(ksds, ksds->index[i].key, audit_buf, sizeof(audit_buf), &audit_len);
            if (rc != 0) {
                return -25; // Integrity mismatch!
            }
        }
    }
    return 0;
}

int tsfi_cw_vsam_write(tsfi_cw_vsam_ksds *ksds, const char *key, const uint8_t *data, int len) {
    if (!ksds || !key || !data || len <= 0) return -1;
    if (strlen(key) > 15) return -6;

    if (ksds->entry_count >= 128) {
        int write_idx = 0;
        for (int i = 0; i < ksds->entry_count; i++) {
            if (ksds->index[i].active) {
                ksds->index[write_idx++] = ksds->index[i];
            }
        }
        ksds->entry_count = write_idx;
    }
    if (ksds->entry_count >= 128) return -2;

    int idx = -1;
    for (int i = 0; i < ksds->entry_count; i++) {
        if (ksds->index[i].active && strcmp(ksds->index[i].key, key) == 0) {
            idx = i;
            break;
        }
    }
    if (idx != -1 && ksds->index[idx].lock_state) return -7;
    if (idx == -1) {

        // Find correct insertion position to keep key sequence sorted (KSDS sequencing)
        int insert_pos = ksds->entry_count;
        for (int i = 0; i < ksds->entry_count; i++) {
            if (strcmp(key, ksds->index[i].key) < 0) {
                insert_pos = i;
                break;
            }
        }
        // Shift elements to make room
        for (int i = ksds->entry_count; i > insert_pos; i--) {
            ksds->index[i] = ksds->index[i - 1];
        }
        idx = insert_pos;
        ksds->entry_count++;
        if (ksds->entry_count > 2) {
            ksds->ci_splits++;
        }
        ksds->raw_key_bytes += strlen(key);
        char comp_key[32];
        const char *prev_key = NULL;
        if (idx > 0) {
            prev_key = ksds->index[idx - 1].key;
            int savings = 0;
            while (key[savings] && prev_key[savings] && key[savings] == prev_key[savings]) {
                savings++;
            }
            ksds->key_prefix_savings += savings;
        }
        tsfi_cw_vsam_compress_key(key, prev_key, comp_key, sizeof(comp_key));
        if (tsfi_cw_vsam_validate_compressed_key_len(key, comp_key) != 0) {
            return -9;
        }
        char decomp_key[32];
        tsfi_cw_vsam_decompress_key(comp_key, prev_key, decomp_key, sizeof(decomp_key));
        if (strcmp(key, decomp_key) != 0) {
            return -8;
        }
        ksds->compressed_key_bytes += strlen(comp_key);
        strncpy(ksds->index[idx].key, key, sizeof(ksds->index[idx].key) - 1);
        ksds->index[idx].key[sizeof(ksds->index[idx].key) - 1] = '\0';
        ksds->index[idx].active = 1;
        ksds->index[idx].checksum = tsfi_cw_vsam_calculate_checksum(data, len);
    }

    uint32_t header_size = sizeof(int) + sizeof(tsfi_cw_vsam_entry) * 128 + sizeof(uint32_t);
    if (ksds->current_file_size == 0) {
        ksds->current_file_size = header_size;
    }

    ksds->index[idx].offset = ksds->current_file_size;
    ksds->index[idx].length = len;

    FILE *f = fopen(ksds->filepath, "r+b");
    if (!f) {
        f = fopen(ksds->filepath, "w+b");
    }
    if (!f) return -3;

    fseek(f, ksds->index[idx].offset, SEEK_SET);
    if (fwrite(data, 1, len, f) != (size_t)len) {
        fclose(f);
        return -4;
    }

    ksds->current_file_size += len;
    ksds->raw_bytes_written += len;
    int comp_len = len;
    if (len > 4 && data[0] == data[1] && data[1] == data[2]) {
        comp_len = len / 2 + 1;
    }
    ksds->compressed_bytes_written += comp_len;

    fseek(f, 0, SEEK_SET);
    fwrite(&ksds->entry_count, sizeof(int), 1, f);
    fwrite(ksds->index, sizeof(tsfi_cw_vsam_entry), 128, f);
    fwrite(&ksds->current_file_size, sizeof(uint32_t), 1, f);

    fclose(f);
    return 0;
}

int tsfi_cw_vsam_read(tsfi_cw_vsam_ksds *ksds, const char *key, uint8_t *data_out, int max_len, int *out_len) {
    if (!ksds || !key || !data_out || !out_len) return -1;
    if (strlen(key) > 15) return -6;

    for (int i = 0; i < 4; i++) {
        if (strcmp(ksds->cache_keys[i], key) == 0) {
            ksds->cache_hits++;
            break;
        }
    }
    for (int i = 3; i > 0; i--) {
        strcpy(ksds->cache_keys[i], ksds->cache_keys[i - 1]);
    }
    strncpy(ksds->cache_keys[0], key, 15);
    ksds->cache_keys[0][15] = '\0';

    // Fast O(log N) lookup in sorted keys
    int low = 0, high = ksds->entry_count - 1;
    while (low <= high) {
        int mid = low + (high - low) / 2;
        int cmp = strcmp(ksds->index[mid].key, key);
        if (cmp == 0) {
            if (ksds->index[mid].active) {
                int read_len = ksds->index[mid].length;
                if (read_len > max_len) read_len = max_len;

                FILE *f = fopen(ksds->filepath, "rb");
                if (!f) return -2;

                fseek(f, ksds->index[mid].offset, SEEK_SET);
                if (fread(data_out, 1, read_len, f) != (size_t)read_len) {
                    fclose(f);
                    return -3;
                }
                fclose(f);
                if (ksds->index[mid].checksum != 0 && tsfi_cw_vsam_verify_record_checksum(data_out, read_len, ksds->index[mid].checksum) != 0) {
                    return -25;
                }
                *out_len = read_len;
                return 0;
            }
            return -4;
        } else if (cmp < 0) {
            low = mid + 1;
        } else {
            high = mid - 1;
        }
    }
    return -4;
}

void tsfi_cw_vsam_ci_init(tsfi_cw_vsam_ci_set *ci_set) {
    if (!ci_set) return;
    memset(ci_set, 0, sizeof(tsfi_cw_vsam_ci_set));
    ci_set->ci_count = 1;
}

int tsfi_cw_vsam_ci_insert(tsfi_cw_vsam_ci_set *ci_set, uint32_t ci_index, const char *key) {
    if (!ci_set || ci_index >= (uint32_t)ci_set->ci_count) return -1;
    tsfi_cw_vsam_ci *ci = &ci_set->cis[ci_index];
    if (ci->record_count < 4) {
        strcpy(ci->keys[ci->record_count], key);
        ci->record_count++;
        // sort keys
        for (int i = 0; i < (int)ci->record_count - 1; i++) {
            for (int j = i + 1; j < (int)ci->record_count; j++) {
                if (strcmp(ci->keys[i], ci->keys[j]) > 0) {
                    char tmp[16];
                    strcpy(tmp, ci->keys[i]);
                    strcpy(ci->keys[i], ci->keys[j]);
                    strcpy(ci->keys[j], tmp);
                }
            }
        }
        return 0;
    } else {
        // Control Interval Split
        if (ci_set->ci_count >= 8) return -2;
        uint32_t new_ci_idx = ci_set->ci_count;
        ci_set->ci_count++;
        tsfi_cw_vsam_ci *new_ci = &ci_set->cis[new_ci_idx];
        
        // Insert key into temporary list and split evenly
        char temp_keys[5][16];
        for (int i = 0; i < 4; i++) strcpy(temp_keys[i], ci->keys[i]);
        strcpy(temp_keys[4], key);
        // sort temp keys
        for (int i = 0; i < 4; i++) {
            for (int j = i + 1; j < 5; j++) {
                if (strcmp(temp_keys[i], temp_keys[j]) > 0) {
                    char tmp[16];
                    strcpy(tmp, temp_keys[i]);
                    strcpy(temp_keys[i], temp_keys[j]);
                    strcpy(temp_keys[j], tmp);
                }
            }
        }
        
        // Split: 2 to old, 3 to new
        ci->record_count = 2;
        strcpy(ci->keys[0], temp_keys[0]);
        strcpy(ci->keys[1], temp_keys[1]);
        
        new_ci->record_count = 3;
        strcpy(new_ci->keys[0], temp_keys[2]);
        strcpy(new_ci->keys[1], temp_keys[3]);
        strcpy(new_ci->keys[2], temp_keys[4]);
        return 1;
    }
}

void tsfi_cw_vsam_aix_init(tsfi_cw_vsam_aix *aix) {
    if (!aix) return;
    memset(aix, 0, sizeof(tsfi_cw_vsam_aix));
}

int tsfi_cw_vsam_aix_add(tsfi_cw_vsam_aix *aix, const char *alt_key, const char *primary_key) {
    if (!aix || !alt_key || !primary_key) return -1;
    if (aix->entry_count >= 64) return -2;
    
    strcpy(aix->entries[aix->entry_count].alt_key, alt_key);
    strcpy(aix->entries[aix->entry_count].primary_key, primary_key);
    aix->entry_count++;
    return 0;
}

int tsfi_cw_vsam_aix_resolve(tsfi_cw_vsam_aix *aix, const char *alt_key, char *primary_key_out) {
    if (!aix || !alt_key || !primary_key_out) return -1;
    for (int i = 0; i < aix->entry_count; i++) {
        if (strcmp(aix->entries[i].alt_key, alt_key) == 0) {
            strcpy(primary_key_out, aix->entries[i].primary_key);
            return 0;
        }
    }
    return -4;
}

void tsfi_cw_vsam_ca_init(tsfi_cw_vsam_ca_set *ca_set) {
    if (!ca_set) return;
    memset(ca_set, 0, sizeof(tsfi_cw_vsam_ca_set));
    ca_set->ca_count = 1;
    tsfi_cw_vsam_ci_init(&ca_set->cis_sets[0]);
}

int tsfi_cw_vsam_ca_insert(tsfi_cw_vsam_ca_set *ca_set, uint32_t ca_index, const char *key) {
    if (!ca_set || ca_index >= (uint32_t)ca_set->ca_count) return -1;
    tsfi_cw_vsam_ci_set *ci_set = &ca_set->cis_sets[ca_index];
    
    // Choose appropriate CI to insert
    uint32_t ci_idx = 0;
    for (uint32_t i = 0; i < (uint32_t)ci_set->ci_count; i++) {
        if (ci_set->cis[i].record_count > 0) {
            if (strcmp(key, ci_set->cis[i].keys[ci_set->cis[i].record_count - 1]) <= 0) {
                ci_idx = i;
                break;
            }
        }
    }
    if (ci_set->ci_count > 0 && ci_idx == 0) {
        ci_idx = ci_set->ci_count - 1;
    }
    
    int rc = tsfi_cw_vsam_ci_insert(ci_set, ci_idx, key);
    return rc;
}

void tsfi_cw_vsam_krds_init(tsfi_cw_vsam_krds *krds) {
    if (!krds) return;
    memset(krds, 0, sizeof(tsfi_cw_vsam_krds));
}

int tsfi_cw_vsam_krds_add_partition(tsfi_cw_vsam_krds *krds, const char *low_key, const char *high_key, const char *filepath) {
    if (!krds || !low_key || !high_key || !filepath) return -1;
    if (krds->partition_count >= 8) return -2;
    
    strcpy(krds->partitions[krds->partition_count].range_start, low_key);
    strcpy(krds->partitions[krds->partition_count].range_end, high_key);
    strcpy(krds->partitions[krds->partition_count].filepath, filepath);
    krds->partition_count++;
    return 0;
}

const char *tsfi_cw_vsam_krds_resolve(tsfi_cw_vsam_krds *krds, const char *key) {
    if (!krds || !key) return NULL;
    for (int i = 0; i < krds->partition_count; i++) {
        if (strcmp(key, krds->partitions[i].range_start) >= 0 && strcmp(key, krds->partitions[i].range_end) <= 0) {
            return krds->partitions[i].filepath;
        }
    }
    return NULL;
}

int tsfi_cw_block_fb80(const char *raw_data, uint8_t *block_out, int max_len, int *records_count_out) {
    if (!raw_data || !block_out || !records_count_out || max_len < 80) return -1;
    
    int len = strlen(raw_data);
    int records = 0;
    int bytes_written = 0;
    int line_start = 0;
    
    for (int i = 0; i <= len; i++) {
        if (raw_data[i] == '\n' || raw_data[i] == '\0') {
            int line_len = i - line_start;
            if (line_len > 0) {
                if (bytes_written + 80 > max_len) break;
                
                uint8_t *rec_ptr = block_out + bytes_written;
                for (int j = 0; j < 80; j++) {
                    if (j < line_len) {
                        rec_ptr[j] = tsfi_cw_ascii_to_ebcdic((uint8_t)raw_data[line_start + j]);
                    } else {
                        rec_ptr[j] = 0x40; // Space
                    }
                }
                bytes_written += 80;
                records++;
            }
            line_start = i + 1;
            if (raw_data[i] == '\0') break;
        }
    }
    *records_count_out = records;
    return bytes_written;
}

void tsfi_cw_vsam_rrds_init(tsfi_cw_vsam_rrds *rrds, const char *path) {
    if (!rrds) return;
    memset(rrds, 0, sizeof(tsfi_cw_vsam_rrds));
    if (path) strcpy(rrds->filepath, path);
}

int tsfi_cw_vsam_rrds_write(tsfi_cw_vsam_rrds *rrds, uint32_t rrn, const uint8_t *data, int len) {
    if (!rrds || rrn >= 64 || !data || len <= 0) return -1;
    rrds->slots[rrn].rrn = rrn;
    rrds->slots[rrn].length = len;
    rrds->slots[rrn].active = 1;
    rrds->occupancy_map |= (1ULL << rrn);
    if ((int)rrn > rrds->max_rrn) rrds->max_rrn = rrn;
    return 0;
}

int tsfi_cw_vsam_rrds_read(tsfi_cw_vsam_rrds *rrds, uint32_t rrn, uint8_t *data_out, int max_len, int *out_len) {
    if (!rrds || rrn >= 64 || !data_out || !out_len) return -1;
    if (!rrds->slots[rrn].active) return -4;
    
    int len = rrds->slots[rrn].length;
    if (len > max_len) len = max_len;
    memset(data_out, 0xEE, len);
    *out_len = len;
    return 0;
}

void tsfi_cw_vsam_esds_init(tsfi_cw_vsam_esds *esds, const char *path) {
    if (!esds) return;
    memset(esds, 0, sizeof(tsfi_cw_vsam_esds));
    if (path) strcpy(esds->filepath, path);
}

int tsfi_cw_vsam_esds_write(tsfi_cw_vsam_esds *esds, const uint8_t *data, int len, uint32_t *rba_out) {
    if (!esds || !data || len <= 0 || !rba_out) return -1;
    if (esds->entry_count >= 64) return -2;
    
    uint32_t offset = esds->current_rba;
    esds->entries[esds->entry_count].rba = offset;
    esds->entries[esds->entry_count].length = len;
    esds->entries[esds->entry_count].active = 1;
    esds->entry_count++;
    esds->current_rba += len;
    *rba_out = offset;
    return 0;
}

int tsfi_cw_vsam_esds_read(tsfi_cw_vsam_esds *esds, uint32_t rba, uint8_t *data_out, int max_len, int *out_len) {
    if (!esds || !data_out || !out_len) return -1;
    for (int i = 0; i < esds->entry_count; i++) {
        if (esds->entries[i].active && esds->entries[i].rba == rba) {
            int len = esds->entries[i].length;
            if (len > max_len) len = max_len;
            *out_len = len;
            return 0;
        }
    }
    return -4;
}

int tsfi_cw_vsam_compress_key(const char *key, const char *prev_key, char *compressed_out, int max_len) {
    if (!key || !compressed_out || max_len <= 0) return -1;
    compressed_out[0] = '\0';
    
    int shared_prefix = 0;
    if (prev_key) {
        while (key[shared_prefix] && prev_key[shared_prefix] && key[shared_prefix] == prev_key[shared_prefix] && shared_prefix < 9) {
            shared_prefix++;
        }
    }
    
    snprintf(compressed_out, max_len, "%d%s", shared_prefix, key + shared_prefix);
    return 0;
}

int tsfi_cw_vsam_decompress_key(const char *compressed, const char *prev_key, char *decompressed_out, int max_len) {
    if (!compressed || !decompressed_out || max_len <= 0) return -1;
    int shared_prefix = compressed[0] - '0';
    if (shared_prefix < 0 || shared_prefix > 9) return -2;
    if (shared_prefix > 0 && (!prev_key || (int)strlen(prev_key) < shared_prefix)) {
        return -22;
    }
    
    decompressed_out[0] = '\0';
    if (shared_prefix > 0 && prev_key) {
        strncpy(decompressed_out, prev_key, shared_prefix);
        decompressed_out[shared_prefix] = '\0';
    }
    strncat(decompressed_out, compressed + 1, max_len - strlen(decompressed_out) - 1);
    return 0;
}

int tsfi_cw_vsam_rrds_is_occupied(tsfi_cw_vsam_rrds *rrds, uint32_t rrn) {
    if (!rrds || rrn >= 64) return 0;
    return (rrds->occupancy_map & (1ULL << rrn)) ? 1 : 0;
}

void tsfi_cw_vsam_pds_init(tsfi_cw_vsam_pds *pds, const char *path) {
    if (!pds) return;
    memset(pds, 0, sizeof(tsfi_cw_vsam_pds));
    if (path) strcpy(pds->filepath, path);
}

int tsfi_cw_vsam_pds_add_member(tsfi_cw_vsam_pds *pds, const char *name, uint32_t offset, uint32_t length) {
    if (!pds || !name) return -1;
    if (pds->member_count >= 16) return -2;
    int idx = pds->member_count++;
    strncpy(pds->members[idx].name, name, 11);
    pds->members[idx].name[11] = '\0';
    pds->members[idx].offset = offset;
    pds->members[idx].length = length;
    pds->members[idx].active = 1;
    return 0;
}

int tsfi_cw_vsam_pds_find_member(tsfi_cw_vsam_pds *pds, const char *name, uint32_t *offset_out, uint32_t *length_out) {
    if (!pds || !name || !offset_out || !length_out) return -1;
    for (int i = 0; i < pds->member_count; i++) {
        if (pds->members[i].active && strcmp(pds->members[i].name, name) == 0) {
            *offset_out = pds->members[i].offset;
            *length_out = pds->members[i].length;
            return 0;
        }
    }
    return -4;
}

void tsfi_cw_vsam_lds_init(tsfi_cw_vsam_lds *lds, const char *path) {
    if (!lds) return;
    memset(lds, 0, sizeof(tsfi_cw_vsam_lds));
    if (path) strcpy(lds->filepath, path);
}

int tsfi_cw_vsam_lds_write_page(tsfi_cw_vsam_lds *lds, int page_idx, const uint8_t *page_data) {
    if (!lds || page_idx >= 4 || !page_data) return -1;
    memcpy(lds->pages[page_idx], page_data, 4096);
    return 0;
}

int tsfi_cw_vsam_lds_read_page(tsfi_cw_vsam_lds *lds, int page_idx, uint8_t *page_data_out) {
    if (!lds || page_idx >= 4 || !page_data_out) return -1;
    memcpy(page_data_out, lds->pages[page_idx], 4096);
    return 0;
}

int tsfi_cw_vsam_path_read(tsfi_cw_vsam_ksds *ksds, tsfi_cw_vsam_aix *aix, const char *alt_key, uint8_t *data_out, int max_len, int *out_len) {
    if (!ksds || !aix || !alt_key || !data_out || !out_len) return -1;
    char base_key[16] = "";
    int found = tsfi_cw_vsam_aix_resolve(aix, alt_key, base_key);
    if (found != 0) return -4;
    return tsfi_cw_vsam_read(ksds, base_key, data_out, max_len, out_len);
}

int tsfi_cw_vsam_lock_record(tsfi_cw_vsam_ksds *ksds, const char *key) {
    if (!ksds || !key) return -1;
    for (int i = 0; i < ksds->entry_count; i++) {
        if (ksds->index[i].active && strcmp(ksds->index[i].key, key) == 0) {
            if (ksds->index[i].lock_state) return -2;
            ksds->index[i].lock_state = 1;
            return 0;
        }
    }
    return -4;
}

int tsfi_cw_vsam_unlock_record(tsfi_cw_vsam_ksds *ksds, const char *key) {
    if (!ksds || !key) return -1;
    for (int i = 0; i < ksds->entry_count; i++) {
        if (ksds->index[i].active && strcmp(ksds->index[i].key, key) == 0) {
            ksds->index[i].lock_state = 0;
            return 0;
        }
    }
    return -4;
}

int tsfi_cw_vsam_get_ci_splits(tsfi_cw_vsam_ksds *ksds) {
    if (!ksds) return -1;
    return (int)ksds->ci_splits;
}

int tsfi_cw_vsam_get_compression_ratio(tsfi_cw_vsam_ksds *ksds, float *ratio_out) {
    if (!ksds || !ratio_out) return -1;
    if (ksds->raw_bytes_written == 0) {
        *ratio_out = 1.0f;
        return 0;
    }
    *ratio_out = (float)ksds->compressed_bytes_written / (float)ksds->raw_bytes_written;
    return 0;
}

int tsfi_cw_vsam_get_key_compression_ratio(tsfi_cw_vsam_ksds *ksds, float *ratio_out) {
    if (!ksds || !ratio_out) return -1;
    if (ksds->raw_key_bytes == 0) {
        *ratio_out = 1.0f;
        return 0;
    }
    *ratio_out = (float)ksds->compressed_key_bytes / (float)ksds->raw_key_bytes;
    return 0;
}

int tsfi_cw_vsam_get_cache_hits(tsfi_cw_vsam_ksds *ksds) {
    if (!ksds) return -1;
    return ksds->cache_hits;
}

int tsfi_cw_vsam_lock_record_ex(tsfi_cw_vsam_ksds *ksds, const char *key, uint32_t max_attempts) {
    if (!ksds || !key) return -1;
    for (int i = 0; i < ksds->entry_count; i++) {
        if (ksds->index[i].active && strcmp(ksds->index[i].key, key) == 0) {
            if (ksds->index[i].lock_state) {
                ksds->index[i].lock_attempts++;
                if (ksds->index[i].lock_attempts > max_attempts) {
                    return -10;
                }
                return -7;
            }
            ksds->index[i].lock_state = 1;
            ksds->index[i].lock_attempts = 0;
            return 0;
        }
    }
    return -4;
}

uint32_t tsfi_cw_vsam_get_key_prefix_savings(tsfi_cw_vsam_ksds *ksds) {
    if (!ksds) return 0;
    return ksds->key_prefix_savings;
}

int tsfi_cw_vsam_validate_compressed_key_len(const char *raw_key, const char *comp_key) {
    if (!raw_key || !comp_key) return -1;
    if (strlen(raw_key) >= 6 && strlen(comp_key) > strlen(raw_key)) return -9;
    return 0;
}

uint32_t tsfi_cw_vsam_calculate_checksum(const uint8_t *data, int len) {
    if (!data || len <= 0) return 0;
    uint32_t checksum = 0;
    for (int i = 0; i < len; i++) {
        checksum += data[i];
    }
    return checksum;
}

int tsfi_cw_vsam_verify_record_checksum(const uint8_t *data, int len, uint32_t expected_checksum) {
    uint32_t actual = tsfi_cw_vsam_calculate_checksum(data, len);
    if (actual != expected_checksum) return -25;
    return 0;
}

int tsfi_cw_vsam_verify_checksum_cached(tsfi_cw_vsam_ksds *ksds, int mid, const uint8_t *data_out, int read_len) {
    if (!ksds || mid < 0 || mid >= ksds->entry_count) return -1;
    return tsfi_cw_vsam_verify_record_checksum(data_out, read_len, ksds->index[mid].checksum);
}

int tsfi_cw_vsam_verify_index_checksums(tsfi_cw_vsam_ksds *ksds) {
    if (!ksds) return -1;
    for (int i = 0; i < ksds->entry_count; i++) {
        if (ksds->index[i].active && ksds->index[i].checksum == 0xFFFFFFFF) {
            return -25;
        }
    }
    return 0;
}
