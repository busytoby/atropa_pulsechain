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
void tsfi_rwcs_init(tsfi_rwcs_report *rep, int page_limit, int line_limit) {
    if (!rep) return;
    rep->page_limit = page_limit;
    rep->line_limit = line_limit;
    rep->current_page = 1;
    rep->current_line = 0;
    rep->group_total = 0.0;
    rep->final_total = 0.0;
}

int tsfi_rwcs_write_header(tsfi_rwcs_report *rep, char *out, size_t max_len) {
    if (!rep || !out || max_len < 128) return -1;
    int written = snprintf(out, max_len,
                           "=========================================\n"
                           "COBOL FINANCIAL AUDIT REPORT    PAGE: %02d\n"
                           "=========================================\n"
                           "ITEM-ID    ITEM-NAME             AMOUNT\n"
                           "-----------------------------------------\n",
                           rep->current_page);
    rep->current_line = 5;
    return written;
}

int tsfi_rwcs_process_item(tsfi_rwcs_report *rep, char *out, size_t max_len, int item_id, const char *name, double amount) {
    if (!rep || !out || max_len < 64) return -1;
    if (rep->current_line >= rep->line_limit) {
        rep->current_page++;
        int header_len = tsfi_rwcs_write_header(rep, out, max_len);
        int item_len = snprintf(out + header_len, max_len - header_len,
                                "%06d     %-20.20s  %10.2f\n",
                                item_id, name, amount);
        rep->group_total += amount;
        rep->final_total += amount;
        rep->current_line++;
        return header_len + item_len;
    } else {
        int item_len = snprintf(out, max_len,
                                "%06d     %-20.20s  %10.2f\n",
                                item_id, name, amount);
        rep->group_total += amount;
        rep->final_total += amount;
        rep->current_line++;
        return item_len;
    }
}

int tsfi_rwcs_control_break(tsfi_rwcs_report *rep, char *out, size_t max_len) {
    if (!rep || !out || max_len < 64) return -1;
    int written = snprintf(out, max_len,
                           "-----------------------------------------\n"
                           "SUBTOTAL:                         %10.2f\n"
                           "=========================================\n",
                           rep->group_total);
    rep->group_total = 0.0;
    rep->current_line += 3;
    return written;
}

int tsfi_rwcs_write_final(tsfi_rwcs_report *rep, char *out, size_t max_len) {
    if (!rep || !out || max_len < 64) return -1;
    int written = snprintf(out, max_len,
                           "-----------------------------------------\n"
                           "GRAND TOTAL:                      %10.2f\n"
                           "=========================================\n",
                           rep->final_total);
    rep->current_line += 3;
    return written;
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

void tsfi_dbtg_currency_init(tsfi_dbtg_currency *cur) {
    if (!cur) return;
    cur->current_run_unit = -1;
    for (int i = 0; i < 8; i++) {
        cur->current_record_type[i] = -1;
        cur->current_set_type[i] = -1;
    }
}

void tsfi_dbtg_currency_update(tsfi_dbtg_currency *cur, int run_unit, int record_type, int set_type) {
    if (!cur) return;
    cur->current_run_unit = run_unit;
    if (record_type >= 0 && record_type < 8) {
        cur->current_record_type[record_type] = run_unit;
    }
    if (set_type >= 0 && set_type < 8) {
        cur->current_set_type[set_type] = run_unit;
    }
}

void tsfi_dbtg_realm_init(tsfi_dbtg_realm_registry *reg) {
    if (!reg) return;
    memset(reg, 0, sizeof(tsfi_dbtg_realm_registry));
}

int tsfi_dbtg_realm_register(tsfi_dbtg_realm_registry *reg, const char *name) {
    if (!reg || !name) return -1;
    if (reg->area_count >= 4) return -2;
    strncpy(reg->areas[reg->area_count].area_name, name, sizeof(reg->areas[reg->area_count].area_name) - 1);
    reg->areas[reg->area_count].lock_mode = DBTG_LOCK_NONE;
    reg->areas[reg->area_count].is_open = 0;
    reg->area_count++;
    return 0;
}

int tsfi_dbtg_realm_open(tsfi_dbtg_realm_registry *reg, const char *name, int lock_mode) {
    if (!reg || !name) return -1;
    for (int i = 0; i < reg->area_count; i++) {
        if (strcmp(reg->areas[i].area_name, name) == 0) {
            if (reg->areas[i].is_open) return -3;
            reg->areas[i].lock_mode = lock_mode;
            reg->areas[i].is_open = 1;
            return 0;
        }
    }
    return -4;
}

int tsfi_dbtg_realm_close(tsfi_dbtg_realm_registry *reg, const char *name) {
    if (!reg || !name) return -1;
    for (int i = 0; i < reg->area_count; i++) {
        if (strcmp(reg->areas[i].area_name, name) == 0) {
            if (!reg->areas[i].is_open) return -3;
            reg->areas[i].lock_mode = DBTG_LOCK_NONE;
            reg->areas[i].is_open = 0;
            return 0;
        }
    }
    return -4;
}

void tsfi_dbtg_exception_init(tsfi_dbtg_exception_context *ctx) {
    if (!ctx) return;
    memset(ctx, 0, sizeof(tsfi_dbtg_exception_context));
    ctx->db_status = DB_STATUS_OK;
}

int tsfi_dbtg_validate_action(tsfi_dbtg_exception_context *ctx, const tsfi_dbtg_realm_registry *reg, const char *realm_name, int is_write_action) {
    if (!ctx || !reg || !realm_name) return -1;
    for (int i = 0; i < reg->area_count; i++) {
        if (strcmp(reg->areas[i].area_name, realm_name) == 0) {
            if (!reg->areas[i].is_open) {
                ctx->db_status = DB_STATUS_NOT_OPEN;
                strncpy(ctx->failing_realm, realm_name, sizeof(ctx->failing_realm) - 1);
                ctx->exception_triggered = 1;
                return -2;
            }
            if (is_write_action) {
                if (reg->areas[i].lock_mode == DBTG_LOCK_RETRIEVAL ||
                    reg->areas[i].lock_mode == DBTG_LOCK_EXCLUSIVE_RETRIEVAL) {
                    ctx->db_status = DB_STATUS_LOCK_VIOLATION;
                    strncpy(ctx->failing_realm, realm_name, sizeof(ctx->failing_realm) - 1);
                    ctx->exception_triggered = 1;
                    return -3;
                }
            }
            ctx->db_status = DB_STATUS_OK;
            ctx->exception_triggered = 0;
            return 0;
        }
    }
    ctx->db_status = DB_STATUS_NOT_OPEN;
    strncpy(ctx->failing_realm, realm_name, sizeof(ctx->failing_realm) - 1);
    ctx->exception_triggered = 1;
    return -4;
}

void tsfi_dbtg_set_membership_init(tsfi_dbtg_set_membership *sm, const char *name, int ins_mode, int ret_mode) {
    if (!sm) return;
    memset(sm, 0, sizeof(tsfi_dbtg_set_membership));
    if (name) strncpy(sm->set_name, name, sizeof(sm->set_name) - 1);
    sm->insertion_mode = ins_mode;
    sm->retention_mode = ret_mode;
}

int tsfi_dbtg_validate_disconnect(const tsfi_dbtg_set_membership *sm, int *db_status_out) {
    if (!sm || !db_status_out) return -1;
    if (sm->retention_mode == DBTG_RETENTION_MANDATORY) {
        *db_status_out = DB_STATUS_MEMBER_ERR;
        return -2;
    }
    *db_status_out = DB_STATUS_OK;
    return 0;
}

int tsfi_dbtg_navigate_set(int current_idx, int record_count, int direction_mode) {
    if (record_count <= 0) return -1;
    switch (direction_mode) {
        case DBTG_NAV_FIRST:
            return 0;
        case DBTG_NAV_LAST:
            return record_count - 1;
        case DBTG_NAV_NEXT:
            return (current_idx + 1) % record_count;
        case DBTG_NAV_PRIOR:
            return (current_idx - 1 + record_count) % record_count;
        default:
            return -1;
    }
}

int tsfi_dbtg_insert_sorted(tsfi_dbtg_sorted_record *records, int current_count, int max_capacity, int record_id, int sort_key) {
    if (!records || current_count < 0 || current_count >= max_capacity) return -1;
    int insert_idx = 0;
    while (insert_idx < current_count && records[insert_idx].sort_key < sort_key) {
        insert_idx++;
    }
    for (int i = current_count; i > insert_idx; i--) {
        records[i] = records[i - 1];
    }
    records[insert_idx].record_id = record_id;
    records[insert_idx].sort_key = sort_key;
    return current_count + 1;
}

void tsfi_dbtg_resolve_calc(const char *key, uint32_t total_pages, uint32_t slots_per_page, tsfi_dbtg_calc_address *address_out) {
    if (!key || total_pages == 0 || slots_per_page == 0 || !address_out) return;
    uint32_t hash = 2166136261U;
    while (*key) {
        hash ^= (uint8_t)*key++;
        hash *= 16777619U;
    }
    address_out->target_page = hash % total_pages;
    address_out->target_slot = (hash / total_pages) % slots_per_page;
}

void tsfi_dbtg_dml_tracker_init(tsfi_dbtg_dml_tracker *tracker) {
    if (!tracker) return;
    memset(tracker, 0, sizeof(tsfi_dbtg_dml_tracker));
}

int tsfi_dbtg_execute_dml(tsfi_dbtg_dml_tracker *tracker, int verb_opcode) {
    if (!tracker) return -1;
    switch (verb_opcode) {
        case DBTG_VERB_STORE:
            tracker->store_count++;
            break;
        case DBTG_VERB_GET:
            tracker->get_count++;
            break;
        case DBTG_VERB_MODIFY:
            tracker->modify_count++;
            break;
        case DBTG_VERB_ERASE:
            tracker->erase_count++;
            break;
        case DBTG_VERB_CONNECT:
            tracker->connect_count++;
            break;
        case DBTG_VERB_DISCONNECT:
            tracker->disconnect_count++;
            break;
        default:
            return -2;
    }
    return 0;
}

void tsfi_s370_vs_dat_init(tsfi_s370_vs_dat *dat) {
    if (!dat) return;
    memset(dat, 0, sizeof(tsfi_s370_vs_dat));
}

int tsfi_s370_vs_dat_translate(const tsfi_s370_vs_dat *dat, uint32_t virtual_address, uint32_t *physical_address_out) {
    if (!dat || !physical_address_out) return -1;
    uint32_t seg_idx = (virtual_address >> 12) & 0x0F;
    uint32_t page_idx = (virtual_address >> 8) & 0x0F;
    uint32_t offset = virtual_address & 0xFF;
    if (dat->segment_table[seg_idx] == 0) {
        return -2;
    }
    uint32_t page_frame = dat->page_tables[seg_idx][page_idx];
    if (page_frame == 0) {
        return -3;
    }
    *physical_address_out = page_frame + offset;
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

void tsfi_structured_analyze_script(const char *script, tsfi_structured_analysis_report *report) {
    if (!script || !report) return;
    memset(report, 0, sizeof(tsfi_structured_analysis_report));
    char script_copy[2048];
    strncpy(script_copy, script, 2047);
    script_copy[2047] = '\0';
    const char *delims = " ;(),{}\"\n\r\t.";
    char labels[32][32];
    int label_count = 0;
    char *tokens[256];
    int token_count = 0;
    char *tok = strtok(script_copy, delims);
    while (tok && token_count < 256) {
        tokens[token_count++] = tok;
        tok = strtok(NULL, delims);
    }
    for (int i = 0; i < token_count; i++) {
        size_t len = strlen(tokens[i]);
        if (len > 1 && tokens[i][len - 1] == ':') {
            char lbl[32];
            size_t copy_len = (len - 1 < 31) ? len - 1 : 31;
            memcpy(lbl, tokens[i], copy_len);
            lbl[copy_len] = '\0';
            if (label_count < 32) {
                memcpy(labels[label_count], lbl, copy_len + 1);
                label_count++;
            }
        }
        if (i + 1 < token_count &&
            ((strcasecmp(tokens[i], "GO") == 0 && strcasecmp(tokens[i + 1], "TO") == 0) ||
             (strcasecmp(tokens[i], "GOTO") == 0))) {
            report->goto_count++;
            char *target = (strcasecmp(tokens[i], "GOTO") == 0) ? tokens[i + 1] : tokens[i + 2];
            for (int k = 0; k < label_count; k++) {
                if (strcmp(labels[k], target) == 0) {
                    report->backward_jmp_detected = 1;
                    break;
                }
            }
        }
    }
}

void tsfi_vtam_session_init(tsfi_vtam_session *session, const char *lu_name) {
    if (!session) return;
    memset(session, 0, sizeof(tsfi_vtam_session));
    if (lu_name) strncpy(session->logical_unit, lu_name, sizeof(session->logical_unit) - 1);
    session->session_state = VTAM_STATE_INIT;
}

int tsfi_vtam_session_handshake(tsfi_vtam_session *session, int event) {
    if (!session) return -1;
    switch (session->session_state) {
        case VTAM_STATE_INIT:
            if (event == VTAM_EV_BIND) {
                session->session_state = VTAM_STATE_NEGOTIATE;
                return 0;
            }
            break;
        case VTAM_STATE_NEGOTIATE:
            if (event == VTAM_EV_SDT) {
                session->session_state = VTAM_STATE_ACTIVE;
                return 0;
            }
            break;
        case VTAM_STATE_ACTIVE:
            if (event == VTAM_EV_UNBIND) {
                session->session_state = VTAM_STATE_TERMINATED;
                return 0;
            }
            break;
        default:
            break;
    }
    return -2;
}

int tsfi_vtam_session_send(tsfi_vtam_session *session, const char *data, int data_len) {
    if (!session || !data || data_len <= 0) return -1;
    if (session->session_state != VTAM_STATE_ACTIVE) {
        return -3;
    }
    session->bytes_transmitted += data_len;
    return data_len;
}

void tsfi_cms_session_init(tsfi_cms_session *sess) {
    if (!sess) return;
    memset(sess, 0, sizeof(tsfi_cms_session));
}

int tsfi_cms_execute_command(tsfi_cms_session *sess, const char *cmd) {
    if (!sess || !cmd) return -1;
    if (strcasecmp(cmd, "IPL CMS") == 0) {
        sess->is_booted = 1;
        sess->last_command_status = 0;
        return 0;
    }
    if (strcasecmp(cmd, "ACCESS 191 A") == 0) {
        if (!sess->is_booted) {
            sess->last_command_status = -2;
            return -2;
        }
        sess->disk_mounted_191 = 1;
        sess->last_command_status = 0;
        return 0;
    }
    if (strcasecmp(cmd, "LISTFILE") == 0) {
        if (!sess->disk_mounted_191) {
            sess->last_command_status = -3;
            return -3;
        }
        sess->last_command_status = 0;
        return 0;
    }
    sess->last_command_status = -1;
    return -1;
}

void tsfi_cp_session_init(tsfi_cp_session *sess) {
    if (!sess) return;
    memset(sess, 0, sizeof(tsfi_cp_session));
    sess->virtual_storage_kb = 4096;
    sess->spool_class = ' ';
    sess->last_cp_status = 0;
}

int tsfi_cp_execute_command(tsfi_cp_session *sess, const char *cmd) {
    if (!sess || !cmd) return -1;
    if (strcasecmp(cmd, "DEFINE STORAGE 16M") == 0) {
        sess->virtual_storage_kb = 16384;
        sess->last_cp_status = 0;
        return 0;
    }
    if (strcasecmp(cmd, "SPOOL PUNCH CLASS A") == 0) {
        sess->spool_class = 'A';
        sess->last_cp_status = 0;
        return 0;
    }
    if (strcasecmp(cmd, "QUERY VIRTUAL") == 0) {
        sess->last_cp_status = 0;
        return 0;
    }
    sess->last_cp_status = -1;
    return -1;
}

void tsfi_cp_spool_queue_init(tsfi_cp_spool_queue *q) {
    if (!q) return;
    memset(q, 0, sizeof(tsfi_cp_spool_queue));
}

int tsfi_cp_spool_push(tsfi_cp_spool_queue *q, const char *data) {
    if (!q || !data || q->count >= MAX_SPOOL_CARDS) return -1;
    strncpy(q->queue[q->tail].card_data, data, sizeof(q->queue[q->tail].card_data) - 1);
    q->queue[q->tail].card_data[sizeof(q->queue[q->tail].card_data) - 1] = '\0';
    q->tail = (q->tail + 1) % MAX_SPOOL_CARDS;
    q->count++;
    return 0;
}

int tsfi_cp_spool_pop(tsfi_cp_spool_queue *q, char *data_out) {
    if (!q || !data_out || q->count <= 0) return -1;
    strncpy(data_out, q->queue[q->head].card_data, 79);
    data_out[79] = '\0';
    q->head = (q->head + 1) % MAX_SPOOL_CARDS;
    q->count--;
    return 0;
}

void tsfi_iucv_broker_init(tsfi_iucv_broker *broker) {
    if (!broker) return;
    memset(broker, 0, sizeof(tsfi_iucv_broker));
}

int tsfi_iucv_connect(tsfi_iucv_broker *broker, const char *src, const char *dest) {
    if (!broker || !src || !dest) return -1;
    for (int i = 0; i < MAX_IUCV_PATHS; i++) {
        if (broker->paths[i].status == IUCV_PATH_FREE) {
            broker->paths[i].path_id = i;
            strncpy(broker->paths[i].source_user, src, sizeof(broker->paths[i].source_user) - 1);
            strncpy(broker->paths[i].dest_user, dest, sizeof(broker->paths[i].dest_user) - 1);
            broker->paths[i].status = IUCV_PATH_ACTIVE;
            broker->paths[i].message_pending = 0;
            return i;
        }
    }
    return -2;
}

int tsfi_iucv_send(tsfi_iucv_broker *broker, int path_id, const char *msg) {
    if (!broker || path_id < 0 || path_id >= MAX_IUCV_PATHS) return -1;
    if (broker->paths[path_id].status != IUCV_PATH_ACTIVE) return -2;
    strncpy(broker->paths[path_id].buffered_message, msg, sizeof(broker->paths[path_id].buffered_message) - 1);
    broker->paths[path_id].buffered_message[sizeof(broker->paths[path_id].buffered_message) - 1] = '\0';
    broker->paths[path_id].message_pending = 1;
    return 0;
}

int tsfi_iucv_receive(tsfi_iucv_broker *broker, int path_id, char *msg_out) {
    if (!broker || path_id < 0 || path_id >= MAX_IUCV_PATHS || !msg_out) return -1;
    if (broker->paths[path_id].status != IUCV_PATH_ACTIVE || !broker->paths[path_id].message_pending) return -2;
    strncpy(msg_out, broker->paths[path_id].buffered_message, 63);
    msg_out[63] = '\0';
    broker->paths[path_id].message_pending = 0;
    return 0;
}

void tsfi_cp_console_spool_init(tsfi_cp_console_spool *spool) {
    if (!spool) return;
    memset(spool, 0, sizeof(tsfi_cp_console_spool));
}

void tsfi_cp_console_spool_start(tsfi_cp_console_spool *spool) {
    if (!spool) return;
    spool->is_spooling = 1;
    spool->log_len = 0;
    spool->log_buffer[0] = '\0';
}

void tsfi_cp_console_spool_write(tsfi_cp_console_spool *spool, const char *text) {
    if (!spool || !text || !spool->is_spooling) return;
    int len = strlen(text);
    if (spool->log_len + len < 511) {
        memcpy(spool->log_buffer + spool->log_len, text, len);
        spool->log_len += len;
        spool->log_buffer[spool->log_len] = '\0';
    }
}

void tsfi_cp_console_spool_stop(tsfi_cp_console_spool *spool) {
    if (!spool) return;
    spool->is_spooling = 0;
}

void tsfi_cp_scheduler_init(tsfi_cp_scheduler *sched) {
    if (!sched) return;
    memset(sched, 0, sizeof(tsfi_cp_scheduler));
}

int tsfi_cp_scheduler_register(tsfi_cp_scheduler *sched, const char *name, int initial_q) {
    if (!sched || !name || sched->task_count >= 8) return -1;
    strncpy(sched->tasks[sched->task_count].vm_name, name, sizeof(sched->tasks[sched->task_count].vm_name) - 1);
    sched->tasks[sched->task_count].vm_name[sizeof(sched->tasks[sched->task_count].vm_name) - 1] = '\0';
    sched->tasks[sched->task_count].queue_type = initial_q;
    sched->tasks[sched->task_count].cpu_cycles_used = 0;
    sched->task_count++;
    return 0;
}

int tsfi_cp_scheduler_dispatch(tsfi_cp_scheduler *sched, int task_idx, int cycles) {
    if (!sched || task_idx < 0 || task_idx >= sched->task_count) return -1;
    sched->tasks[task_idx].cpu_cycles_used += cycles;
    if (sched->tasks[task_idx].queue_type == VM_QUEUE_Q1 && cycles > 50) {
        sched->tasks[task_idx].queue_type = VM_QUEUE_Q2;
    }
    return 0;
}

void tsfi_cp_directory_init(tsfi_cp_directory *dir) {
    if (!dir) return;
    memset(dir, 0, sizeof(tsfi_cp_directory));
}

int tsfi_cp_directory_add(tsfi_cp_directory *dir, const char *uid, char priv, uint32_t max_store) {
    if (!dir || !uid || dir->entry_count >= 8) return -1;
    strncpy(dir->entries[dir->entry_count].userid, uid, sizeof(dir->entries[dir->entry_count].userid) - 1);
    dir->entries[dir->entry_count].userid[sizeof(dir->entries[dir->entry_count].userid) - 1] = '\0';
    dir->entries[dir->entry_count].privilege_class = priv;
    dir->entries[dir->entry_count].max_storage_kb = max_store;
    dir->entry_count++;
    return 0;
}

int tsfi_cp_directory_check(const tsfi_cp_directory *dir, const char *uid, char required_priv) {
    if (!dir || !uid) return -1;
    for (int i = 0; i < dir->entry_count; i++) {
        if (strcmp(dir->entries[i].userid, uid) == 0) {
            if (required_priv == 'A' && dir->entries[i].privilege_class != 'A') {
                return -2;
            }
            return 0;
        }
    }
    return -1;
}

void tsfi_cp_attachment_init(tsfi_cp_attachment_manager *mgr) {
    if (!mgr) return;
    memset(mgr, 0, sizeof(tsfi_cp_attachment_manager));
}

int tsfi_cp_attachment_register(tsfi_cp_attachment_manager *mgr, uint32_t phys_addr) {
    if (!mgr || mgr->device_count >= 8) return -1;
    mgr->devices[mgr->device_count].physical_address = phys_addr;
    mgr->devices[mgr->device_count].virtual_address = 0;
    mgr->devices[mgr->device_count].is_attached = 0;
    memset(mgr->devices[mgr->device_count].dedicated_user, 0, 16);
    mgr->device_count++;
    return 0;
}

int tsfi_cp_attach(tsfi_cp_attachment_manager *mgr, uint32_t phys_addr, const char *uid, uint32_t virt_addr) {
    if (!mgr || !uid) return -1;
    for (int i = 0; i < mgr->device_count; i++) {
        if (mgr->devices[i].physical_address == phys_addr) {
            if (mgr->devices[i].is_attached) {
                return -2;
            }
            mgr->devices[i].virtual_address = virt_addr;
            strncpy(mgr->devices[i].dedicated_user, uid, 15);
            mgr->devices[i].dedicated_user[15] = '\0';
            mgr->devices[i].is_attached = 1;
            return 0;
        }
    }
    return -3;
}

int tsfi_cp_detach(tsfi_cp_attachment_manager *mgr, uint32_t virt_addr, const char *uid) {
    if (!mgr || !uid) return -1;
    for (int i = 0; i < mgr->device_count; i++) {
        if (mgr->devices[i].is_attached && mgr->devices[i].virtual_address == virt_addr) {
            if (strcmp(mgr->devices[i].dedicated_user, uid) != 0) {
                return -2;
            }
            mgr->devices[i].virtual_address = 0;
            memset(mgr->devices[i].dedicated_user, 0, 16);
            mgr->devices[i].is_attached = 0;
            return 0;
        }
    }
    return -3;
}

void tsfi_cp_smsg_init(tsfi_cp_smsg_receiver *rcv, const char *uid) {
    if (!rcv) return;
    memset(rcv, 0, sizeof(tsfi_cp_smsg_receiver));
    if (uid) {
        strncpy(rcv->userid, uid, sizeof(rcv->userid) - 1);
        rcv->userid[sizeof(rcv->userid) - 1] = '\0';
    }
    rcv->smsg_enabled = 1;
}

int tsfi_cp_smsg_send(tsfi_cp_smsg_receiver *rcv, const char *msg) {
    if (!rcv || !msg) return -1;
    if (!rcv->smsg_enabled) {
        return -1;
    }
    if (rcv->count >= 8) {
        return -2;
    }
    strncpy(rcv->queue[rcv->tail].payload, msg, sizeof(rcv->queue[rcv->tail].payload) - 1);
    rcv->queue[rcv->tail].payload[sizeof(rcv->queue[rcv->tail].payload) - 1] = '\0';
    rcv->tail = (rcv->tail + 1) % 8;
    rcv->count++;
    return 0;
}

int tsfi_cp_smsg_receive(tsfi_cp_smsg_receiver *rcv, char *msg_out) {
    if (!rcv || !msg_out || rcv->count <= 0) return -1;
    strncpy(msg_out, rcv->queue[rcv->head].payload, 63);
    msg_out[63] = '\0';
    rcv->head = (rcv->head + 1) % 8;
    rcv->count--;
    return 0;
}

void tsfi_cp_vcpu_init(tsfi_cp_vcpu *vcpu) {
    if (!vcpu) return;
    memset(vcpu, 0, sizeof(tsfi_cp_vcpu));
    vcpu->state = VCPU_STOPPED;
    vcpu->psw_instruction_address = 0x00000000;
    vcpu->psw_mask = 0x00000000;
}

int tsfi_cp_vcpu_control(tsfi_cp_vcpu *vcpu, const char *action) {
    if (!vcpu || !action) return -1;
    if (strcasecmp(action, "START") == 0) {
        vcpu->state = VCPU_RUNNING;
        return 0;
    }
    if (strcasecmp(action, "STOP") == 0) {
        vcpu->state = VCPU_STOPPED;
        return 0;
    }
    if (strcasecmp(action, "RESET") == 0) {
        vcpu->state = VCPU_STOPPED;
        vcpu->psw_instruction_address = 0x00000000;
        vcpu->psw_mask = 0x00000000;
        return 0;
    }
    return -1;
}

void tsfi_cp_printer_init(tsfi_cp_spool_printer *prt) {
    if (!prt) return;
    memset(prt, 0, sizeof(tsfi_cp_spool_printer));
    prt->page_count = 1;
}

int tsfi_cp_printer_write_record(tsfi_cp_spool_printer *prt, const char *record) {
    if (!prt || !record || record[0] == '\0') return -1;
    char ctrl = record[0];
    switch (ctrl) {
        case '1':
            prt->line_count = 0;
            prt->page_count++;
            prt->last_skip_count = 0;
            break;
        case ' ':
            prt->line_count += 1;
            prt->last_skip_count = 1;
            break;
        case '0':
            prt->line_count += 2;
            prt->last_skip_count = 2;
            break;
        case '-':
            prt->line_count += 3;
            prt->last_skip_count = 3;
            break;
        default:
            prt->line_count += 1;
            prt->last_skip_count = 1;
            break;
    }
    return 0;
}

void tsfi_cp_spool_queue_v2_init(tsfi_cp_spool_queue_v2 *q, char reader_class) {
    if (!q) return;
    memset(q, 0, sizeof(tsfi_cp_spool_queue_v2));
    q->active_reader_class = reader_class;
}

int tsfi_cp_spool_push_v2(tsfi_cp_spool_queue_v2 *q, const char *data, char card_class) {
    if (!q || !data || q->count >= MAX_SPOOL_CARDS_V2) return -1;
    strncpy(q->queue[q->count].card_data, data, sizeof(q->queue[q->count].card_data) - 1);
    q->queue[q->count].card_data[sizeof(q->queue[q->count].card_data) - 1] = '\0';
    q->queue[q->count].spool_class = card_class;
    q->count++;
    return 0;
}

int tsfi_cp_spool_pop_v2(tsfi_cp_spool_queue_v2 *q, char *data_out) {
    if (!q || !data_out || q->count <= 0) return -1;
    for (int i = 0; i < q->count; i++) {
        if (q->active_reader_class == '*' || q->queue[i].spool_class == q->active_reader_class) {
            strncpy(data_out, q->queue[i].card_data, 79);
            data_out[79] = '\0';
            for (int j = i; j < q->count - 1; j++) {
                q->queue[j] = q->queue[j + 1];
            }
            q->count--;
            return 0;
        }
    }
    return -2;
}

void tsfi_cp_spool_queue_v3_init(tsfi_cp_spool_queue_v3 *q) {
    if (!q) return;
    memset(q, 0, sizeof(tsfi_cp_spool_queue_v3));
}

int tsfi_cp_spool_push_v3(tsfi_cp_spool_queue_v3 *q, const char *data, int file_id) {
    if (!q || !data || q->count >= MAX_SPOOL_CARDS_V3) return -1;
    strncpy(q->queue[q->count].card_data, data, sizeof(q->queue[q->count].card_data) - 1);
    q->queue[q->count].card_data[sizeof(q->queue[q->count].card_data) - 1] = '\0';
    q->queue[q->count].file_id = file_id;
    q->queue[q->count].is_held = 0;
    q->count++;
    return 0;
}

int tsfi_cp_spool_hold(tsfi_cp_spool_queue_v3 *q, int file_id, int hold) {
    if (!q) return -1;
    for (int i = 0; i < q->count; i++) {
        if (q->queue[i].file_id == file_id) {
            q->queue[i].is_held = hold;
            return 0;
        }
    }
    return -1;
}

int tsfi_cp_spool_pop_v3(tsfi_cp_spool_queue_v3 *q, char *data_out) {
    if (!q || !data_out || q->count <= 0) return -1;
    for (int i = 0; i < q->count; i++) {
        if (!q->queue[i].is_held) {
            strncpy(data_out, q->queue[i].card_data, 79);
            data_out[79] = '\0';
            for (int j = i; j < q->count - 1; j++) {
                q->queue[j] = q->queue[j + 1];
            }
            q->count--;
            return 0;
        }
    }
    return -2;
}

void tsfi_cp_purge_stats_init(tsfi_cp_purge_stats *stats) {
    if (!stats) return;
    memset(stats, 0, sizeof(tsfi_cp_purge_stats));
}

int tsfi_cp_execute_purge(tsfi_cp_purge_stats *stats, tsfi_cp_spool_queue_v3 *rdr, tsfi_cp_spool_printer *prt, const char *cmd) {
    if (!stats || !cmd) return -1;
    if (strcasecmp(cmd, "PURGE RDR") == 0) {
        if (rdr) {
            stats->reader_count = rdr->count;
            stats->total_purged += rdr->count;
            rdr->count = 0;
            memset(rdr->queue, 0, sizeof(rdr->queue));
        }
        return 0;
    }
    if (strcasecmp(cmd, "PURGE ALL") == 0) {
        if (rdr) {
            stats->reader_count = rdr->count;
            stats->total_purged += rdr->count;
            rdr->count = 0;
            memset(rdr->queue, 0, sizeof(rdr->queue));
        }
        if (prt) {
            stats->printer_count = prt->line_count;
            stats->total_purged += prt->line_count;
            prt->line_count = 0;
            prt->page_count = 1;
            prt->last_skip_count = 0;
        }
        return 0;
    }
    return -1;
}

void tsfi_cp_query_init(tsfi_cp_query_manager *mgr) {
    if (!mgr) return;
    mgr->simulated_connect_time_sec = 3600;
    mgr->active_user_count = 2;
}

int tsfi_cp_query_execute(tsfi_cp_query_manager *mgr, const char *query_cmd, char *out_buf, int out_max) {
    if (!mgr || !query_cmd || !out_buf || out_max <= 0) return -1;
    if (strcasecmp(query_cmd, "QUERY TIME") == 0) {
        snprintf(out_buf, out_max, "CONNECT TIME: %u SEC", mgr->simulated_connect_time_sec);
        return 0;
    }
    if (strcasecmp(query_cmd, "QUERY NAMES") == 0) {
        snprintf(out_buf, out_max, "ACTIVE USERS: %d", mgr->active_user_count);
        return 0;
    }
    return -1;
}

int tsfi_cp_msg_send(const tsfi_cp_directory *dir, const char *sender, const char *recipient, const char *msg_text, char *out_terminal_buf, int out_terminal_max) {
    if (!dir || !sender || !recipient || !msg_text || !out_terminal_buf || out_terminal_max <= 0) return -1;
    
    int found = 0;
    for (int i = 0; i < dir->entry_count; i++) {
        if (strcmp(dir->entries[i].userid, recipient) == 0) {
            found = 1;
            break;
        }
    }
    
    if (found) {
        snprintf(out_terminal_buf, out_terminal_max, "MSG FROM %s: %s", sender, msg_text);
        return 0;
    } else {
        snprintf(out_terminal_buf, out_terminal_max, "ERR: USER %s NOT LOGGED ON", recipient);
        return -1;
    }
}

int tsfi_cp_warning_broadcast(const tsfi_cp_directory *dir, const char *sender_uid, const char *warn_text, char out_terminals[8][128], int *broadcast_count) {
    if (!dir || !sender_uid || !warn_text || !out_terminals || !broadcast_count) return -1;
    
    int priv_ok = tsfi_cp_directory_check(dir, sender_uid, 'A');
    if (priv_ok != 0) {
        return -2;
    }
    
    *broadcast_count = 0;
    for (int i = 0; i < dir->entry_count; i++) {
        snprintf(out_terminals[i], 127, "WARN: %s", warn_text);
        out_terminals[i][127] = '\0';
        (*broadcast_count)++;
    }
    return 0;
}

void tsfi_cp_sleep_init(tsfi_cp_terminal_sleep *t) {
    if (!t) return;
    t->is_sleeping = 0;
    t->remaining_seconds = 0;
}

int tsfi_cp_sleep_start(tsfi_cp_terminal_sleep *t, int seconds) {
    if (!t || seconds <= 0) return -1;
    t->is_sleeping = 1;
    t->remaining_seconds = seconds;
    return 0;
}

int tsfi_cp_sleep_tick(tsfi_cp_terminal_sleep *t) {
    if (!t || !t->is_sleeping) return 0;
    t->remaining_seconds--;
    if (t->remaining_seconds <= 0) {
        t->remaining_seconds = 0;
        t->is_sleeping = 0;
    }
    return t->remaining_seconds;
}

int tsfi_cp_sleep_interrupt(tsfi_cp_terminal_sleep *t) {
    if (!t) return -1;
    t->is_sleeping = 0;
    t->remaining_seconds = 0;
    return 0;
}

void tsfi_cp_active_session_init(tsfi_cp_active_session *sess, const char *uid) {
    if (!sess) return;
    memset(sess, 0, sizeof(tsfi_cp_active_session));
    if (uid) {
        strncpy(sess->userid, uid, sizeof(sess->userid) - 1);
        sess->userid[sizeof(sess->userid) - 1] = '\0';
    }
    sess->is_connected = 1;
}

int tsfi_cp_active_session_disconnect(tsfi_cp_active_session *sess) {
    if (!sess) return -1;
    sess->is_connected = 0;
    return 0;
}

int tsfi_cp_active_session_connect(tsfi_cp_active_session *sess) {
    if (!sess) return -1;
    sess->is_connected = 1;
    return 0;
}

int tsfi_cp_active_session_dispatch(tsfi_cp_active_session *sess, int cycles) {
    if (!sess) return -1;
    sess->background_cycles_run += cycles;
    return sess->background_cycles_run;
}

int tsfi_ramac_verify_and_isolate_space_charge(tsfi_ramac_acc_model *model, double current, double voltage, double distance) {
    if (!model) return -1;
    if (voltage > 0.0001 && distance > 0.0001) {
        double ratio = (current * distance * distance) / (voltage * sqrt(voltage));
        // If Child-Langmuir law with perveance 2.5 is matched (implies empirical preference), intercept and isolate
        if (fabs(ratio - 2.5) < 0.0001) {
            model->isolation_trap = (int64_t)(current * 1000.0);
            return -2; // Intercepted and isolated
        }
    }
    return 0;
}

