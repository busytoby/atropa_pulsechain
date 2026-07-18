#include "tsfi_ramac_layout.h"
#include "tsfi_strategy_lang.h"
#include "tsfi_winchester_bridge.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846

#endif
tsfi_ramac_chs tsfi_ramac_index_to_chs(int index) {
    tsfi_ramac_chs chs;

    chs.cylinder = index / RAMAC_WORDS_PER_CYLINDER;
    int rem = index % RAMAC_WORDS_PER_CYLINDER;
    chs.head = rem / RAMAC_WORDS_PER_TRACK;
    rem = rem % RAMAC_WORDS_PER_TRACK;
    chs.sector = rem / RAMAC_WORDS;
    chs.word_offset = rem % RAMAC_WORDS;

    return chs;
}


int tsfi_ramac_chs_to_index(tsfi_ramac_chs chs) {
    return chs.cylinder * RAMAC_WORDS_PER_CYLINDER +
           chs.head * RAMAC_WORDS_PER_TRACK +
           chs.sector * RAMAC_WORDS +
           chs.word_offset;
}


double tsfi_ramac_calculate_seek(int from_index, int to_index) {
    tsfi_ramac_chs c1 = tsfi_ramac_index_to_chs(from_index);
    tsfi_ramac_chs c2 = tsfi_ramac_index_to_chs(to_index);

    double cylinder_seek = abs(c1.cylinder - c2.cylinder) * 1.5;
    double head_swap = (c1.head != c2.head) ? 0.8 : 0.0;
    
    // One-directional modular rotational delay representing physical disk spin
    int sector_dist = (c2.sector - c1.sector + 20) % 20;
    double rotational_delay = sector_dist * 5.0;

    return (cylinder_seek + head_swap + rotational_delay) * 1000.0; // Return in microseconds
}


int tsfi_ramac_layout_optimize(tsfi_dat *dat, const char *filepath) {
    if (!dat || !filepath) return -1;

    FILE *fp = fopen(filepath, "wb");
    if (!fp) return -1;

    fwrite("RMAC", 1, 4, fp);

    int capacity = dat->capacity;
    fwrite(&capacity, sizeof(int), 1, fp);

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
    int primary_slots = 45 * RAMAC_SECTORS;
    int slot = hash % primary_slots;
    
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
    int current_head = 0;

    while (disk[current_idx].is_active) {
        seek_time += tsfi_ramac_calculate_seek(current_head, current_idx);
        current_head = current_idx;

        if (strcmp(disk[current_idx].key, key) == 0) {
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
        seek_time += tsfi_ramac_calculate_seek(current_head, current_idx);
        strcpy(disk[current_idx].key, key);
        strcpy(disk[current_idx].value, value);
        disk[current_idx].is_active = 1;
        disk[current_idx].next_overflow_index = -1;
        if (out_total_seek_us) *out_total_seek_us = seek_time;
        return current_idx;
    }

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
        return -1;
    }

    seek_time += tsfi_ramac_calculate_seek(current_head, found_slot);
    strcpy(disk[found_slot].key, key);
    strcpy(disk[found_slot].value, value);
    disk[found_slot].is_active = 1;
    disk[found_slot].next_overflow_index = -1;

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

    const char *read_val = tsfi_ramac_search_record(disk, key, cylinder, &temp_seek);
    if (!read_val || strcmp(read_val, value) != 0) {
        return -1;
    }

    return 0;
}





int tsfi_ramac_inquiry_station(tsfi_ramac_record *disk, const char *command, char *response_out, int max_len) {
    if (!disk || !command || !response_out) return -1;
    
    char cmd[16] = {0};
    char arg1[32] = {0};
    char arg2[32] = {0};

    int parsed = sscanf(command, "%15s %31s %31s", cmd, arg1, arg2);
    if (parsed <= 0) return -1;

    if (strcmp(cmd, "QRY") == 0) {
        if (parsed < 2) return -1;
        double seek = 0.0;
        const char *val = tsfi_ramac_search_record(disk, arg1, 5, &seek);
        if (val) {
            snprintf(response_out, max_len, "KEY: %s VAL: %s SEEK: %.1f us", arg1, val, seek);
        } else {
            snprintf(response_out, max_len, "KEY: %s STATUS: NOT_FOUND", arg1);
        }
        return 0;
    }

    if (strcmp(cmd, "WRT") == 0) {
        if (parsed < 3) return -1;
        double seek = 0.0;
        int idx = tsfi_ramac_insert_record(disk, arg1, arg2, 5, &seek);
        if (idx != -1) {
            snprintf(response_out, max_len, "WRITE_SUCCESS INDEX: %d SEEK: %.1f us", idx, seek);
        } else {
            snprintf(response_out, max_len, "WRITE_FAILED (CYLINDER FULL)");
        }
        return 0;
    }

    if (strcmp(cmd, "PRT") == 0) {
        if (parsed < 2) return -1;
        int parity_ok = tsfi_ramac_check_parity(arg1);
        if (parity_ok) {
            snprintf(response_out, max_len, "PARITY CHECK: PASS");
        } else {
            snprintf(response_out, max_len, "PARITY CHECK: FAIL");
        }
        return 0;
    }

    return -1;
}


int tsfi_ramac_check_parity(const char *str) {
    if (!str) return 0;
    while (*str) {
        unsigned char c = *str++;
        int total_bits = 0;
        for (int b = 0; b < 8; b++) {
            if ((c >> b) & 1) total_bits++;
        }
        if (total_bits % 2 == 0) {
            return 0;
        }
    }
    return 1;
}


























































void tsfi_lgp30_flipflop_init(tsfi_lgp30_flipflop *ff) {
    if (!ff) return;
    ff->triode1_grid_v = -2.0;
    ff->triode1_plate_v = 150.0;
    ff->triode2_grid_v = 0.0;
    ff->triode2_plate_v = 50.0;
}


void tsfi_lgp30_flipflop_tick(tsfi_lgp30_flipflop *ff, double trigger_set_v, double trigger_reset_v, double dt) {
    if (!ff) return;

    double vcc = 200.0;
    double rl = 20000.0;
    double r1 = 100000.0;
    double r2 = 50000.0;
    double bias_v = -50.0;
    double i0 = 0.002;

    double target_g1 = (ff->triode2_plate_v * r2 + bias_v * r1) / (r1 + r2) + trigger_set_v;
    double target_g2 = (ff->triode1_plate_v * r2 + bias_v * r1) / (r1 + r2) + trigger_reset_v;

    ff->triode1_grid_v += (target_g1 - ff->triode1_grid_v) * (dt / 0.00001);
    ff->triode2_grid_v += (target_g2 - ff->triode2_grid_v) * (dt / 0.00001);
    if (ff->triode1_grid_v > 0.0) ff->triode1_grid_v = 0.0;
    if (ff->triode2_grid_v > 0.0) ff->triode2_grid_v = 0.0;

    double ip1 = 0.0;
    if (ff->triode1_grid_v > -6.0) {
        ip1 = i0 * pow(ff->triode1_grid_v + 6.0, 1.5);
    }
    double ip2 = 0.0;
    if (ff->triode2_grid_v > -6.0) {
        ip2 = i0 * pow(ff->triode2_grid_v + 6.0, 1.5);
    }

    double target_p1 = vcc - ip1 * rl;
    double target_p2 = vcc - ip2 * rl;
    if (target_p1 < 0.0) target_p1 = 0.0;
    if (target_p2 < 0.0) target_p2 = 0.0;

    ff->triode1_plate_v += (target_p1 - ff->triode1_plate_v) * (dt / 0.000002);
    ff->triode2_plate_v += (target_p2 - ff->triode2_plate_v) * (dt / 0.000002);
}























































void tsfi_univac_posting_init(tsfi_univac_posting_interpreter *interp) {
    if (!interp) return;
    memset(interp->retained_data, ' ', 80);
    interp->has_master_data = 0;
}


int tsfi_univac_posting_process(tsfi_univac_posting_interpreter *interp, const tsfi_ramac_card *card_in, tsfi_ramac_card *card_out) {
    if (!interp || !card_in || !card_out) return -1;
    
    // Check for control punch tags:
    // '*' = master card control punch: retain the data
    if (card_in->columns[0] == '*') {
        memcpy(interp->retained_data, &card_in->columns[1], 79);
        interp->retained_data[79] = ' '; // ensure boundary space padding
        interp->has_master_data = 1;
        // Output card gets the input card directly
        memcpy(card_out->columns, card_in->columns, 80);
        return 1; // Handled master
    }
    
    // '!' = clear control punch: clear the retained registers
    if (card_in->columns[0] == '!') {
        memset(interp->retained_data, ' ', 80);
        interp->has_master_data = 0;
        memcpy(card_out->columns, card_in->columns, 80);
        return 2; // Cleared
    }
    
    // Detail card: post the retained master data into columns 0-39, and details into columns 40-79
    if (interp->has_master_data) {
        memcpy(card_out->columns, interp->retained_data, 40);
        memcpy(&card_out->columns[40], &card_in->columns[40], 40);
    } else {
        memcpy(card_out->columns, card_in->columns, 80);
    }
    
    return 0; // Handled detail
}






void tsfi_winchester_socket_init(tsfi_winchester_socket_bridge *bridge, int port) {
    if (!bridge) return;
    bridge->listen_port = port;
    bridge->connection_active = 1;
    bridge->processed_packets = 0;
}


int tsfi_winchester_socket_route_event(tsfi_winchester_socket_bridge *bridge, const uint8_t *event_data, int len, void *pq) {
    if (!bridge || !event_data || len <= 0 || !pq) return -1;
    if (!bridge->connection_active) return -2;
    
    uint8_t keycode = event_data[0];
    TSFiPriorityQueue *queue = (TSFiPriorityQueue *)pq;
    
    tsfi_priority_queue_push(queue, 10, keycode, "");
    
    bridge->processed_packets++;
    return 0;
}




int tsfi_uniservo_init(tsfi_uniservo_tape *tape, const char *filepath) {
    if (!tape || !filepath) return -1;
    strncpy(tape->filepath, filepath, sizeof(tape->filepath) - 1);
    tape->filepath[sizeof(tape->filepath) - 1] = '\0';
    tape->current_block_pos = 0;
    tape->parity_errors = 0;
    
    FILE *f = fopen(filepath, "rb");
    if (!f) {
        tape->total_blocks = 0;
        return 0;
    }
    
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fclose(f);
    
    tape->total_blocks = (uint32_t)(size / 256);
    return 0;
}


int tsfi_uniservo_read_block(tsfi_uniservo_tape *tape, uint32_t block_idx, uint8_t *buffer, int buf_len) {
    if (!tape || !buffer || buf_len < 256) return -1;
    
    FILE *f = fopen(tape->filepath, "rb");
    if (!f) return -2;
    
    if (fseek(f, block_idx * 256, SEEK_SET) != 0) {
        fclose(f);
        return -3;
    }
    
    size_t read_bytes = fread(buffer, 1, 256, f);
    fclose(f);
    
    if (read_bytes < 256) return -4;
    
    uint8_t xor_sum = 0;
    for (int i = 0; i < 255; i++) {
        xor_sum ^= buffer[i];
    }
    
    if (buffer[255] != xor_sum) {
        tape->parity_errors++;
        return -5;
    }
    
    tape->current_block_pos = block_idx;
    return 0;
}


int tsfi_uniservo_write_block(tsfi_uniservo_tape *tape, uint32_t block_idx, const uint8_t *buffer, int buf_len) {
    if (!tape || !buffer || buf_len < 256) return -1;
    
    FILE *f = fopen(tape->filepath, "r+b");
    if (!f) {
        f = fopen(tape->filepath, "wb");
        if (!f) return -2;
    }
    
    if (fseek(f, block_idx * 256, SEEK_SET) != 0) {
        fclose(f);
        return -3;
    }
    
    uint8_t out_block[256];
    memcpy(out_block, buffer, 255);
    
    uint8_t xor_sum = 0;
    for (int i = 0; i < 255; i++) {
        xor_sum ^= out_block[i];
    }
    out_block[255] = xor_sum;
    
    size_t written_bytes = fwrite(out_block, 1, 256, f);
    fclose(f);
    
    if (written_bytes < 256) return -4;
    
    tape->current_block_pos = block_idx;
    if (block_idx >= tape->total_blocks) {
        tape->total_blocks = block_idx + 1;
    }
    return 0;
}











int tsfi_winchester_socket_route_to_zmm(tsfi_winchester_socket_bridge *bridge, const uint8_t *event_data, int len, TsfiZmmVmState *zmm) {
    if (!bridge || !event_data || len <= 0 || !zmm) return -1;
    if (!bridge->connection_active) return -2;
    
    uint8_t keycode = event_data[0];
    
    if (zmm->telem) {
        zmm->telem->zmm_val = keycode;
    }
    bridge->processed_packets++;
    return 0;
}


void tsfi_compool_init(tsfi_jovial_compool *cp) {
    if (!cp) return;
    cp->entry_count = 0;
    for (int i = 0; i < 16; i++) {
        cp->entries[i].var_name[0] = '\0';
        cp->entries[i].val = 0;
    }
}


int tsfi_compool_register(tsfi_jovial_compool *cp, const char *name, uint32_t val) {
    if (!cp || !name || cp->entry_count >= 16) return -1;
    
    for (int i = 0; i < cp->entry_count; i++) {
        if (strcmp(cp->entries[i].var_name, name) == 0) {
            cp->entries[i].val = val;
            return 0;
        }
    }
    
    strncpy(cp->entries[cp->entry_count].var_name, name, 31);
    cp->entries[cp->entry_count].var_name[31] = '\0';
    cp->entries[cp->entry_count].val = val;
    cp->entry_count++;
    return 0;
}


int tsfi_compool_lookup(const tsfi_jovial_compool *cp, const char *name, uint32_t *val_out) {
    if (!cp || !name || !val_out) return -1;
    for (int i = 0; i < cp->entry_count; i++) {
        if (strcmp(cp->entries[i].var_name, name) == 0) {
            *val_out = cp->entries[i].val;
            return 0;
        }
    }
    return -2;
}








#include <math.h>




















int tsfi_law_query(const tsfi_law_case *db, int db_size, const char *query_word, int results_out[8]) {
    if (!db || db_size <= 0 || !query_word || !results_out) return 0;
    int count = 0;
    for (int i = 0; i < db_size; i++) {
        if (strcmp(db[i].keyword, query_word) == 0) {
            results_out[count++] = db[i].case_id;
            if (count >= 8) break;
        }
    }
    return count;
}





uint64_t tsfi_double_to_cdc3600_float(double val) {
    union { double d; uint64_t u; } conv;
    conv.d = val;
    uint64_t sign = (conv.u >> 63) & 1ULL;
    int64_t exp = ((conv.u >> 52) & 0x7FFULL) - 1023 + 1024;
    if (exp < 0) exp = 0;
    if (exp > 0x7FF) exp = 0x7FF;
    uint64_t frac = (conv.u >> 16) & 0xFFFFFFFFFULL;
    return (sign << 47) | ((uint64_t)exp << 36) | frac;
}


















void tsfi_mackenzie_init(mackenzie_storage *store) {
    if (!store) return;
    store->current_tick = 0;
    for (int i = 0; i < 8; i++) {
        store->segments[i].sector_id = i;
        store->segments[i].access_count = 0;
        store->segments[i].last_access_tick = 0;
        store->segments[i].location = 0;
    }
}


int tsfi_mackenzie_access(mackenzie_storage *store, int sector_id) {
    if (!store || sector_id < 0 || sector_id >= 8) return -1;
    store->current_tick++;
    store->segments[sector_id].access_count++;
    store->segments[sector_id].last_access_tick = store->current_tick;
    return store->segments[sector_id].location;
}


int tsfi_mackenzie_migrate(mackenzie_storage *store, int age_threshold) {
    if (!store) return 0;
    int count = 0;
    for (int i = 0; i < 8; i++) {
        if (store->segments[i].location == 0) {
            int age = store->current_tick - store->segments[i].last_access_tick;
            if (age > age_threshold && store->segments[i].access_count < 5) {
                store->segments[i].location = 1;
                count++;
            }
        }
    }
    return count;
}






















uint64_t tsfi_bates_hash(const char *token, uint64_t salt) {
    if (!token) return 0;
    uint64_t h = salt ^ 953467954114363ULL;
    while (*token) {
        h = (h * 33) ^ (uint8_t)(*token);
        token++;
    }
    return h;
}


int tsfi_bates_authenticate(const char *token, uint64_t salt, uint64_t expected_hash) {
    uint64_t computed = tsfi_bates_hash(token, salt);
    return (computed == expected_hash) ? 0 : -1;
}



void tsfi_mis_init(mis_database *db) {
    if (!db) return;
    db->count = 0;
    for (int i = 0; i < 32; i++) {
        db->records[i].resource_name[0] = '\0';
        db->records[i].allocation_val = 0;
        db->records[i].parent_node_id = 0;
    }
}


int tsfi_mis_insert(mis_database *db, const char *name, uint32_t allocation, uint32_t parent_id) {
    if (!db || db->count >= 32 || !name) return -1;
    mis_record *r = &db->records[db->count++];
    strncpy(r->resource_name, name, sizeof(r->resource_name) - 1);
    r->resource_name[sizeof(r->resource_name) - 1] = '\0';
    r->allocation_val = allocation;
    r->parent_node_id = parent_id;
    return 0;
}


int tsfi_mis_query(const mis_database *db, uint32_t parent_id, uint32_t min_alloc, char *result_out, size_t max_len) {
    if (!db || !result_out || max_len == 0) return -1;
    result_out[0] = '\0';
    int match_count = 0;
    for (int i = 0; i < db->count; i++) {
        const mis_record *r = &db->records[i];
        if (r->parent_node_id == parent_id && r->allocation_val >= min_alloc) {
            if (match_count > 0) {
                strncat(result_out, ",", max_len - strlen(result_out) - 1);
            }
            strncat(result_out, r->resource_name, max_len - strlen(result_out) - 1);
            match_count++;
        }
    }
    return match_count;
}































void tsfi_mcs_init(tsfi_mcs_queue *q, const char *name) {
    if (!q) return;
    memset(q, 0, sizeof(tsfi_mcs_queue));
    strcpy(q->status_key, "00");
    pthread_mutex_init(&q->lock, NULL);
    if (name) {
        strncpy(q->queue_name, name, sizeof(q->queue_name) - 1);
    }
}


int tsfi_mcs_send(tsfi_mcs_queue *q, const char *msg, void *wmq_void) {
    if (!q || !msg) return -1;
    if (pthread_mutex_trylock(&q->lock) != 0) {
        strcpy(q->status_key, "40");
        return -4;
    }
    if (q->count >= 8) {
        strcpy(q->status_key, "10");
        pthread_mutex_unlock(&q->lock);
        return -2;
    }
    strncpy(q->messages[q->tail], msg, sizeof(q->messages[q->tail]) - 1);
    q->indicators[q->tail] = MCS_EMI;
    q->tail = (q->tail + 1) % 8;
    q->count++;
    strcpy(q->status_key, "00");
    pthread_mutex_unlock(&q->lock);
    TSFiWinchesterBridge *wmq = (TSFiWinchesterBridge *)wmq_void;
    if (wmq) {
        wmq->registers.status_reg = 1;
        wmq->registers.keycode_reg = 32;
        uint32_t packed_word = 0;
        memcpy(&packed_word, msg, (strlen(msg) < 4) ? strlen(msg) : 4);
        wmq->registers.data_reg = packed_word;
        tsfi_winchester_bridge_handshake(wmq);
    }
    return 0;
}


int tsfi_mcs_receive(tsfi_mcs_queue *q, char *msg_out, size_t max_len) {
    if (!q || !msg_out || max_len == 0) return -1;
    if (pthread_mutex_trylock(&q->lock) != 0) {
        strcpy(q->status_key, "40");
        return -4;
    }
    if (q->count == 0) {
        strcpy(q->status_key, "20");
        pthread_mutex_unlock(&q->lock);
        return -2;
    }
    strncpy(msg_out, q->messages[q->head], max_len - 1);
    msg_out[max_len - 1] = '\0';
    q->head = (q->head + 1) % 8;
    q->count--;
    strcpy(q->status_key, "00");
    pthread_mutex_unlock(&q->lock);
    return 0;
}


void tsfi_mcs_init_hierarchical(tsfi_mcs_queue *q, const char *q_name, const char *sq1, const char *sq2, const char *sq3) {
    if (!q) return;
    memset(q, 0, sizeof(tsfi_mcs_queue));
    strcpy(q->status_key, "00");
    pthread_mutex_init(&q->lock, NULL);
    if (q_name) strncpy(q->queue_name, q_name, sizeof(q->queue_name) - 1);
    if (sq1) strncpy(q->sub_queue1, sq1, sizeof(q->sub_queue1) - 1);
    if (sq2) strncpy(q->sub_queue2, sq2, sizeof(q->sub_queue2) - 1);
    if (sq3) strncpy(q->sub_queue3, sq3, sizeof(q->sub_queue3) - 1);
}


int tsfi_mcs_send_segment(tsfi_mcs_queue *q, const char *msg, uint8_t indicator, void *wmq_void) {
    if (!q || !msg) return -1;
    if (pthread_mutex_trylock(&q->lock) != 0) {
        strcpy(q->status_key, "40");
        return -4;
    }
    if (q->count >= 8) {
        strcpy(q->status_key, "10");
        pthread_mutex_unlock(&q->lock);
        return -2;
    }
    strncpy(q->messages[q->tail], msg, sizeof(q->messages[q->tail]) - 1);
    q->indicators[q->tail] = indicator;
    q->tail = (q->tail + 1) % 8;
    q->count++;
    strcpy(q->status_key, "00");
    pthread_mutex_unlock(&q->lock);
    TSFiWinchesterBridge *wmq = (TSFiWinchesterBridge *)wmq_void;
    if (wmq) {
        wmq->registers.status_reg = 1;
        wmq->registers.keycode_reg = 32;
        uint32_t packed_word = 0;
        memcpy(&packed_word, msg, (strlen(msg) < 4) ? strlen(msg) : 4);
        wmq->registers.data_reg = packed_word;
        tsfi_winchester_bridge_handshake(wmq);
    }
    return 0;
}


int tsfi_mcs_receive_segment(tsfi_mcs_queue *q, char *msg_out, size_t max_len, uint8_t *indicator_out) {
    if (!q || !msg_out || max_len == 0) return -1;
    if (pthread_mutex_trylock(&q->lock) != 0) {
        strcpy(q->status_key, "40");
        return -4;
    }
    if (q->count == 0) {
        strcpy(q->status_key, "20");
        pthread_mutex_unlock(&q->lock);
        return -2;
    }
    strncpy(msg_out, q->messages[q->head], max_len - 1);
    msg_out[max_len - 1] = '\0';
    if (indicator_out) {
        *indicator_out = q->indicators[q->head];
    }
    q->head = (q->head + 1) % 8;
    q->count--;
    strcpy(q->status_key, "00");
    pthread_mutex_unlock(&q->lock);
    return 0;
}


void tsfi_mcs_assembly_init(tsfi_mcs_assembly *buf) {
    if (!buf) return;
    memset(buf, 0, sizeof(tsfi_mcs_assembly));
}


int tsfi_mcs_assemble_next(tsfi_mcs_queue *q, tsfi_mcs_assembly *buf, char *msg_out, size_t max_len) {
    if (!q || !buf || !msg_out || max_len == 0) return -1;
    char temp_segment[128];
    uint8_t indicator = 0;
    int rx_res = tsfi_mcs_receive_segment(q, temp_segment, sizeof(temp_segment), &indicator);
    if (rx_res != 0) return rx_res;
    size_t seg_len = strlen(temp_segment);
    if (buf->assembly_len + seg_len + 1 > sizeof(buf->assembly_buffer)) {
        strcpy(q->status_key, "30");
        return -3;
    }
    memcpy(buf->assembly_buffer + buf->assembly_len, temp_segment, seg_len);
    buf->assembly_len += seg_len;
    buf->assembly_buffer[buf->assembly_len] = '\0';
    if (indicator == MCS_EMI || indicator == MCS_EGI) {
        strncpy(msg_out, buf->assembly_buffer, max_len - 1);
        msg_out[max_len - 1] = '\0';
        buf->assembly_len = 0;
        buf->assembly_buffer[0] = '\0';
        return 1;
    }
    return 0;
}


void tsfi_subschema_init(tsfi_subschema_map *map, const char *name, const char *rec, const char *set) {
    if (!map) return;
    memset(map, 0, sizeof(tsfi_subschema_map));
    if (name) strncpy(map->subschema_name, name, sizeof(map->subschema_name) - 1);
    if (rec) strncpy(map->record_name, rec, sizeof(map->record_name) - 1);
    if (set) strncpy(map->set_name, set, sizeof(map->set_name) - 1);
}


int tsfi_subschema_map_data(const tsfi_subschema_map *map, const uint8_t *db_record_data, int *registers_out) {
    if (!map || !db_record_data || !registers_out) return -1;
    for (int i = 0; i < map->field_count && i < 8; i++) {
        int offset = map->field_offsets[i];
        uint32_t val = (db_record_data[offset] << 24) |
                       (db_record_data[offset+1] << 16) |
                       (db_record_data[offset+2] << 8) |
                       db_record_data[offset+3];
        registers_out[i] = (int)val;
    }
    return 0;
}



















void tsfi_relational_tuple_init(tsfi_relational_tuple *t, const char *rel, const char *key) {
    if (!t) return;
    memset(t, 0, sizeof(tsfi_relational_tuple));
    if (rel) strncpy(t->relation_name, rel, sizeof(t->relation_name) - 1);
    if (key) strncpy(t->tuple_key, key, sizeof(t->tuple_key) - 1);
    t->resolved_record_id = -1;
}


int tsfi_relational_map_to_codasyl(const tsfi_relational_tuple *t, const tsfi_dbtg_realm_registry *realm_reg, int *db_status_out) {
    if (!t || !realm_reg || !db_status_out) return -1;
    int found_open_realm = 0;
    for (int i = 0; i < realm_reg->area_count; i++) {
        if (strcmp(realm_reg->areas[i].area_name, t->relation_name) == 0 && realm_reg->areas[i].is_open) {
            found_open_realm = 1;
            break;
        }
    }
    if (!found_open_realm) {
        *db_status_out = DB_STATUS_NOT_OPEN;
        return -2;
    }
    *db_status_out = DB_STATUS_OK;
    return 0;
}




























































