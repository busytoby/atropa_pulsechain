#include "tsfi_ramac_layout.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

tsfi_ramac_chs tsfi_ramac_index_to_chs(int index) {
    tsfi_ramac_chs chs;
    int words_per_sector = RAMAC_WORDS;
    int words_per_track = RAMAC_SECTORS * words_per_sector;
    int words_per_cylinder = RAMAC_HEADS * words_per_track;

    chs.cylinder = index / words_per_cylinder;
    int rem = index % words_per_cylinder;
    chs.head = rem / words_per_track;
    rem = rem % words_per_track;
    chs.sector = rem / words_per_sector;
    chs.word_offset = rem % words_per_sector;

    return chs;
}

int tsfi_ramac_chs_to_index(tsfi_ramac_chs chs) {
    int words_per_sector = RAMAC_WORDS;
    int words_per_track = RAMAC_SECTORS * words_per_sector;
    int words_per_cylinder = RAMAC_HEADS * words_per_track;

    return chs.cylinder * words_per_cylinder +
           chs.head * words_per_track +
           chs.sector * words_per_sector +
           chs.word_offset;
}

double tsfi_ramac_calculate_seek(int from_index, int to_index) {
    tsfi_ramac_chs c1 = tsfi_ramac_index_to_chs(from_index);
    tsfi_ramac_chs c2 = tsfi_ramac_index_to_chs(to_index);

    // Physical model of IBM 305 RAMAC movement:
    // 1. Horizontal seek: Cylinder-to-cylinder arm movement.
    //    Average cylinder-to-cylinder seek: ~1.5 milliseconds per cylinder.
    double cylinder_seek = abs(c1.cylinder - c2.cylinder) * 1.5;

    // 2. Vertical seek: Disk surface head swap (if head index changes).
    //    Average head swap time: ~0.8 milliseconds.
    double head_swap = (c1.head != c2.head) ? 0.8 : 0.0;

    // 3. Rotational latency: Sector-to-sector delay.
    //    Assuming 600 RPM (100 ms per full rotation of 20 sectors).
    //    Rotational delay = 5.0 ms per sector distance.
    double rotational_delay = abs(c1.sector - c2.sector) * 5.0;

    return (cylinder_seek + head_swap + rotational_delay) * 1000.0; // Return in microseconds
}

int tsfi_ramac_layout_optimize(tsfi_dat *dat, const char *filepath) {
    if (!dat || !filepath) return -1;

    // Open target file for writing the RAMAC-optimized dat layout
    FILE *fp = fopen(filepath, "wb");
    if (!fp) return -1;

    // Write magic signature
    fwrite("RMAC", 1, 4, fp);

    // Calculate total layout words and size
    int capacity = dat->capacity;
    fwrite(&capacity, sizeof(int), 1, fp);

    // Write base and check arrays clustered as aligned structures
    fwrite(dat->base, sizeof(int), capacity, fp);
    fwrite(dat->check, sizeof(int), capacity, fp);

    fclose(fp);
    return 0;
}

int tsfi_ramac_hash_key(const char *key, int cylinder) {
    unsigned int hash = 5381;
    int c;
    while ((c = *key++)) {
        hash = ((hash << 5) + hash) + c;
    }
    // We restrict primary tracks to heads 0..44 (total 45 tracks per cylinder)
    int primary_slots = 45 * RAMAC_SECTORS;
    int slot = hash % primary_slots;
    
    // Return flat index within cylinder:
    // slot represents head * RAMAC_SECTORS + sector
    int head = slot / RAMAC_SECTORS;
    int sector = slot % RAMAC_SECTORS;

    tsfi_ramac_chs chs;
    chs.cylinder = cylinder;
    chs.head = head;
    chs.sector = sector;
    chs.word_offset = 0;

    return tsfi_ramac_chs_to_index(chs);
}

int tsfi_ramac_insert_record(tsfi_ramac_record *disk, const char *key, const char *value, int cylinder, double *out_total_seek_us) {
    int primary_idx = tsfi_ramac_hash_key(key, cylinder);
    int current_idx = primary_idx;
    int last_idx = -1;
    double seek_time = 0.0;
    int current_head = 0; // Assume start head

    // Traverse existing collision chain
    while (disk[current_idx].is_active) {
        seek_time += tsfi_ramac_calculate_seek(current_head, current_idx);
        current_head = current_idx;

        if (strcmp(disk[current_idx].key, key) == 0) {
            // Overwrite existing key
            strcpy(disk[current_idx].value, value);
            if (out_total_seek_us) *out_total_seek_us = seek_time;
            return current_idx;
        }
        last_idx = current_idx;
        if (disk[current_idx].next_overflow_index == -1) {
            break;
        }
        current_idx = disk[current_idx].next_overflow_index;
    }

    if (!disk[current_idx].is_active) {
        // Direct placement
        seek_time += tsfi_ramac_calculate_seek(current_head, current_idx);
        strcpy(disk[current_idx].key, key);
        strcpy(disk[current_idx].value, value);
        disk[current_idx].is_active = 1;
        disk[current_idx].next_overflow_index = -1;
        if (out_total_seek_us) *out_total_seek_us = seek_time;
        return current_idx;
    }

    // Find free slot in overflow area (heads 45..49) of the same cylinder
    tsfi_ramac_chs overflow_chs;
    overflow_chs.cylinder = cylinder;
    overflow_chs.word_offset = 0;

    int found_slot = -1;
    for (int h = 45; h < 50; h++) {
        overflow_chs.head = h;
        for (int s = 0; s < RAMAC_SECTORS; s++) {
            overflow_chs.sector = s;
            int test_idx = tsfi_ramac_chs_to_index(overflow_chs);
            if (!disk[test_idx].is_active) {
                found_slot = test_idx;
                break;
            }
        }
        if (found_slot != -1) break;
    }

    if (found_slot == -1) {
        // Cylinder overflow area full
        return -1;
    }

    // Write to overflow slot
    seek_time += tsfi_ramac_calculate_seek(current_head, found_slot);
    strcpy(disk[found_slot].key, key);
    strcpy(disk[found_slot].value, value);
    disk[found_slot].is_active = 1;
    disk[found_slot].next_overflow_index = -1;

    // Link the last record to this overflow slot
    disk[last_idx].next_overflow_index = found_slot;

    if (out_total_seek_us) *out_total_seek_us = seek_time;
    return found_slot;
}

const char* tsfi_ramac_search_record(tsfi_ramac_record *disk, const char *key, int cylinder, double *out_total_seek_us) {
    int primary_idx = tsfi_ramac_hash_key(key, cylinder);
    int current_idx = primary_idx;
    double seek_time = 0.0;
    int current_head = 0;

    while (current_idx != -1 && disk[current_idx].is_active) {
        seek_time += tsfi_ramac_calculate_seek(current_head, current_idx);
        current_head = current_idx;

        if (strcmp(disk[current_idx].key, key) == 0) {
            if (out_total_seek_us) *out_total_seek_us = seek_time;
            return disk[current_idx].value;
        }
        current_idx = disk[current_idx].next_overflow_index;
    }

    if (out_total_seek_us) *out_total_seek_us = seek_time;
    return NULL;
}

int tsfi_ramac_plugboard_route(const char *wiring, const uint8_t *src, uint8_t *dest, int max_len) {
    if (!wiring || !src || !dest) return -1;
    int src_s = 0, src_e = 0, dest_s = 0, dest_e = 0;
    if (sscanf(wiring, "%d..%d->%d..%d", &src_s, &src_e, &dest_s, &dest_e) != 4) {
        return -1;
    }

    if (src_s < 0 || src_e >= max_len || src_s > src_e) return -1;
    if (dest_s < 0 || dest_e >= max_len || dest_s > dest_e) return -1;

    int copy_len = src_e - src_s + 1;
    int dest_len = dest_e - dest_s + 1;
    int size_to_copy = (copy_len < dest_len) ? copy_len : dest_len;

    memcpy(dest + dest_s, src + src_s, size_to_copy);
    return size_to_copy;
}

int tsfi_ramac_write_verified(tsfi_ramac_record *disk, const char *key, const char *value, int cylinder) {
    double temp_seek = 0.0;
    int write_idx = tsfi_ramac_insert_record(disk, key, value, cylinder, &temp_seek);
    if (write_idx == -1) return -1;

    // Immediately trigger read-after-write verification
    const char *read_val = tsfi_ramac_search_record(disk, key, cylinder, &temp_seek);
    if (!read_val || strcmp(read_val, value) != 0) {
        return -1; // Parity check mismatch!
    }

    return 0; // Verified successfully
}
