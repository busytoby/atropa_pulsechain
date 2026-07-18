#include "tsfi_hogan.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int tsfi_hogan_write_seq_record(const char *filepath, const uint8_t *payload, size_t size) {
    // Enforce Rule 13: file extension must end with .dat.bin
    const char *ext = strrchr(filepath, '.');
    if (!ext || strcmp(ext, ".bin") != 0) {
        if (!ext || strcmp(ext - 4, ".dat.bin") != 0) {
            return -3; // Invalid extension
        }
    }

    FILE *f = fopen(filepath, "ab");
    if (!f) return -1;
    
    uint32_t record_len = (uint32_t)size;
    if (fwrite(&record_len, sizeof(uint32_t), 1, f) != 1) {
        fclose(f);
        return -2;
    }
    if (fwrite(payload, 1, size, f) != size) {
        fclose(f);
        return -2;
    }
    
    fclose(f);
    return 0;
}

int tsfi_hogan_read_seq_record(const char *filepath, size_t index, uint8_t *payload_out, size_t *size_out) {
    FILE *f = fopen(filepath, "rb");
    if (!f) return -1;
    
    for (size_t i = 0; i <= index; i++) {
        uint32_t record_len = 0;
        if (fread(&record_len, sizeof(uint32_t), 1, f) != 1) {
            fclose(f);
            return -2; // Out of bounds / EOF
        }
        
        if (i == index) {
            if (fread(payload_out, 1, record_len, f) != record_len) {
                fclose(f);
                return -3;
            }
            *size_out = record_len;
            fclose(f);
            return 0;
        } else {
            // Seek past this record to read the next one
            if (fseek(f, record_len, SEEK_CUR) != 0) {
                fclose(f);
                return -4;
            }
        }
    }
    
    fclose(f);
    return -5;
}

void tsfi_hogan_init_dict(hogan_record_dict *dict) {
    memset(dict, 0, sizeof(hogan_record_dict));
}

int tsfi_hogan_add_field(hogan_record_dict *dict, const char *name, uint32_t offset, hogan_field_type type) {
    if (dict->field_count >= HOGAN_MAX_FIELDS) {
        return -1;
    }
    hogan_field_def *field = &dict->fields[dict->field_count++];
    strncpy(field->name, name, sizeof(field->name) - 1);
    field->name[sizeof(field->name) - 1] = '\0';
    field->offset = offset;
    field->type = type;
    return 0;
}

int tsfi_hogan_resolve_uint32(const hogan_record_dict *dict, const uint8_t *payload, const char *field_name, uint32_t *val_out) {
    for (size_t i = 0; i < dict->field_count; i++) {
        const hogan_field_def *field = &dict->fields[i];
        if (strcmp(field->name, field_name) == 0) {
            if (field->type != FIELD_TYPE_UINT32) return -1;
            memcpy(val_out, payload + field->offset, sizeof(uint32_t));
            return 0;
        }
    }
    return -2; // Not found
}

int tsfi_hogan_resolve_uint64(const hogan_record_dict *dict, const uint8_t *payload, const char *field_name, uint64_t *val_out) {
    for (size_t i = 0; i < dict->field_count; i++) {
        const hogan_field_def *field = &dict->fields[i];
        if (strcmp(field->name, field_name) == 0) {
            if (field->type != FIELD_TYPE_UINT64) return -1;
            memcpy(val_out, payload + field->offset, sizeof(uint64_t));
            return 0;
        }
    }
    return -2; // Not found
}

int tsfi_hogan_write_account_index(const char *filepath, uint32_t account_id, uint32_t sector_offset) {
    hogan_index_entry entry;
    entry.account_id = account_id;
    entry.sector_offset = sector_offset;
    
    return tsfi_hogan_write_seq_record(filepath, (const uint8_t *)&entry, sizeof(hogan_index_entry));
}

int tsfi_hogan_lookup_account_offset(const char *filepath, uint32_t account_id, uint32_t *sector_offset_out) {
    uint8_t buf[sizeof(hogan_index_entry)];
    size_t size = 0;
    size_t index = 0;
    
    while (tsfi_hogan_read_seq_record(filepath, index, buf, &size) == 0) {
        if (size != sizeof(hogan_index_entry)) {
            return -2; // Corrupt record size
        }
        
        const hogan_index_entry *entry = (const hogan_index_entry *)buf;
        if (entry->account_id == account_id) {
            *sector_offset_out = entry->sector_offset;
            return 0; // Found
        }
        index++;
    }
    
    return -1; // Not found
}
