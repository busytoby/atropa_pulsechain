#include "tsfi_mainframe_decnet.h"
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>


int tsfi_sdlc_hub_broadcast(tsfi_sdlc_hub *hub, const uint8_t *frame_data, size_t len, uint8_t rx_addr, uint8_t *buf_out, size_t *len_out) {
    if (!hub || !frame_data || len == 0 || !buf_out || !len_out) return -1;
    int found = 0;
    for (int i = 0; i < hub->connected_count; i++) {
        if (hub->connected_addresses[i] == rx_addr) {
            found = 1;
            break;
        }
    }
    if (!found) return -2;
    
    memcpy(buf_out, frame_data, len);
    *len_out = len;
    return 0;
}

int tsfi_sdlc_hub_broadcast_group(tsfi_sdlc_hub *hub, const uint8_t *frame_data, size_t len, uint8_t rx_addr, uint8_t *buf_out, size_t *len_out) {
    if (!hub || !frame_data || len == 0 || !buf_out || !len_out) return -1;
    if (rx_addr == 0xFF) {
        memcpy(buf_out, frame_data, len);
        *len_out = len;
        return 0;
    }
    return tsfi_sdlc_hub_broadcast(hub, frame_data, len, rx_addr, buf_out, len_out);
}

void tsfi_roscoe_init(tsfi_roscoe_library *lib) {
    if (!lib) return;
    lib->member_count = 0;
    memset(lib->members, 0, sizeof(lib->members));
}

int tsfi_roscoe_add_member(tsfi_roscoe_library *lib, const char *name) {
    if (!lib || !name || lib->member_count >= 16) return -1;
    strncpy(lib->members[lib->member_count].member_name, name, 7);
    lib->members[lib->member_count].member_name[7] = '\0';
    lib->members[lib->member_count].version = 1;
    lib->members[lib->member_count].locked = 0;
    lib->member_count++;
    return 0;
}

int tsfi_roscoe_lock_member(tsfi_roscoe_library *lib, const char *name, int lock_state) {
    if (!lib || !name) return -1;
    for (int i = 0; i < lib->member_count; i++) {
        if (strcmp(lib->members[i].member_name, name) == 0) {
            lib->members[i].locked = lock_state;
            return 0;
        }
    }
    return -2;
}

int tsfi_swift_parse(const char *raw_telex, tsfi_swift_message *msg_out) {
    if (!raw_telex || !msg_out) return -1;
    const char *s_bic = strstr(raw_telex, "{1:F01");
    const char *r_bic = strstr(raw_telex, "{2:I103");
    const char *mt = strstr(raw_telex, "MT:");
    const char *amt = strstr(raw_telex, "AMT:");
    
    if (s_bic && r_bic && mt && amt) {
        const char *s_end = strchr(s_bic, '}');
        if (s_end) {
            size_t len = s_end - (s_bic + 6);
            if (len > 15) len = 15;
            memcpy(msg_out->sender_bic, s_bic + 6, len);
            msg_out->sender_bic[len] = '\0';
        }
        const char *r_end = strchr(r_bic, '}');
        if (r_end) {
            size_t len = r_end - (r_bic + 7);
            if (len > 15) len = 15;
            memcpy(msg_out->receiver_bic, r_bic + 7, len);
            msg_out->receiver_bic[len] = '\0';
        }
        strncpy(msg_out->message_type, mt + 3, 3);
        msg_out->message_type[3] = '\0';
        msg_out->amount = atof(amt + 4);
        return 0;
    }
    return -2;
}

int tsfi_swift_generate_trailer(const char *block4_text, char *block5_out, size_t max_len) {
    if (!block4_text || !block5_out || max_len < 32) return -1;
    uint32_t checksum = 0xABCD0123;
    for (size_t i = 0; block4_text[i] != '\0'; i++) {
        checksum ^= block4_text[i];
        checksum = (checksum << 5) | (checksum >> 27);
    }
    snprintf(block5_out, max_len, "{5:MAC-%08X}", checksum);
    return 0;
}

int tsfi_swift_verify_trailer(const char *block4_text, const char *block5_trailer) {
    if (!block4_text || !block5_trailer) return -1;
    char expected_trailer[32];
    if (tsfi_swift_generate_trailer(block4_text, expected_trailer, sizeof(expected_trailer)) == 0) {
        return (strcmp(expected_trailer, block5_trailer) == 0) ? 0 : -2;
    }
    return -3;
}

int tsfi_swift_parse_block1(const char *raw_block1, tsfi_swift_block1 *b1_out) {
    if (!raw_block1 || !b1_out) return -1;
    const char *p = strstr(raw_block1, "{1:");
    if (p) {
        p += 3;
        b1_out->application_id = p[0];
        memcpy(b1_out->service_id, p + 1, 2);
        b1_out->service_id[2] = '\0';
        memcpy(b1_out->sender_lt, p + 3, 12);
        b1_out->sender_lt[12] = '\0';
        memcpy(b1_out->session_num, p + 15, 4);
        b1_out->session_num[4] = '\0';
        memcpy(b1_out->seq_num, p + 19, 6);
        b1_out->seq_num[6] = '\0';
        return 0;
    }
    return -2;
}

int tsfi_rb_gateway_route(const tsfi_rb_packet *pkt, void *out_struct) {
    if (!pkt || !out_struct) return -1;
    if (pkt->color == RB_COLOR_RED) {
        return tsfi_cyclades_deserialize(pkt->payload, pkt->data_len, (tsfi_cyclades_header *)out_struct);
    } else if (pkt->color == RB_COLOR_BLACK) {
        return tsfi_swift_parse_block1((const char *)pkt->payload, (tsfi_swift_block1 *)out_struct);
    }
    return -2;
}

void tsfi_rb_23_init(tsfi_rb_23_node *node) {
    if (!node) return;
    node->is_three_node = 0;
    node->swift_keys[0] = 0;
    node->swift_keys[1] = 0;
    node->cyclades_buffer_count = 0;
}

int tsfi_rb_23_insert(tsfi_rb_23_node *node, uint32_t swift_key, uint32_t cyclades_data) {
    if (!node) return -1;
    if (!node->is_three_node) {
        if (node->swift_keys[0] == 0) {
            node->swift_keys[0] = swift_key;
        } else {
            if (swift_key < node->swift_keys[0]) {
                node->swift_keys[1] = node->swift_keys[0];
                node->swift_keys[0] = swift_key;
            } else {
                node->swift_keys[1] = swift_key;
            }
            node->is_three_node = 1;
        }
        node->cyclades_buffer_count += cyclades_data;
        return 0;
    }
    return -2;
}

float tsfi_gibson_mix_calculate_mips(const tsfi_gibson_mix_input *input) {
    if (!input) return 0.0f;
    double total_time_us = 
        input->load_store_count * 1.5 +
        input->add_sub_count * 1.0 +
        input->multiply_count * 5.0 +
        input->divide_count * 12.0 +
        input->branch_count * 2.0 +
        input->logic_count * 0.8;
    
    double total_instructions = 
        input->load_store_count +
        input->add_sub_count +
        input->multiply_count +
        input->divide_count +
        input->branch_count +
        input->logic_count;

    if (total_time_us == 0.0) return 0.0f;
    return (float)(total_instructions / total_time_us);
}

void tsfi_cad_transform_line(const tsfi_cad_line *line_in, float scale, float tx, float ty, tsfi_cad_line *line_out) {
    if (!line_in || !line_out) return;
    line_out->start.x = line_in->start.x * scale + tx;
    line_out->start.y = line_in->start.y * scale + ty;
    line_out->end.x = line_in->end.x * scale + tx;
    line_out->end.y = line_in->end.y * scale + ty;
    line_out->color = line_in->color;
}

float tsfi_optimizer_estimate_cost(const tsfi_optimizer_input *input) {
    if (!input) return -1.0f;
    if (input->has_index) {
        return 1.0f + (float)input->total_pages * input->selectivity;
    } else {
        return (float)input->total_pages;
    }
}

int tsfi_eft_audit_transaction(const tsfi_eft_transaction *tx, float max_latency_ms) {
    if (!tx) return -1;
    if (tx->latency_ms <= max_latency_ms && (tx->auth_flags & 0x03) == 0x03) {
        return 0;
    }
    return -2;
}

void tsfi_dp_registry_init(tsfi_dp_registry *reg) {
    if (!reg) return;
    reg->count = 0;
    memset(reg->professionals, 0, sizeof(reg->professionals));
}

int tsfi_dp_registry_add(tsfi_dp_registry *reg, const char *name, int years, int lang_prof, int cert) {
    if (!reg || !name || reg->count >= 16) return -1;
    strncpy(reg->professionals[reg->count].employee_name, name, 31);
    reg->professionals[reg->count].employee_name[31] = '\0';
    reg->professionals[reg->count].years_experience = years;
    reg->professionals[reg->count].strategic_lang_proficient = lang_prof;
    reg->professionals[reg->count].certified = cert;
    reg->count++;
    return 0;
}

int tsfi_ramac_join_dp_roscoe(const tsfi_dp_registry *dp_reg, const tsfi_roscoe_library *roscoe_lib, tsfi_dp_roscoe_join_row *out_rows, size_t *out_count) {
    if (!dp_reg || !roscoe_lib || !out_rows || !out_count) return -1;
    size_t count = 0;
    for (int i = 0; i < dp_reg->count; i++) {
        for (int j = 0; j < roscoe_lib->member_count; j++) {
            if (i == j) {
                strncpy(out_rows[count].employee_name, dp_reg->professionals[i].employee_name, 31);
                out_rows[count].employee_name[31] = '\0';
                strncpy(out_rows[count].member_name, roscoe_lib->members[j].member_name, 7);
                out_rows[count].member_name[7] = '\0';
                out_rows[count].locked = roscoe_lib->members[j].locked;
                count++;
            }
        }
    }
    *out_count = count;
    return 0;
}

void tsfi_eft_batch_init(tsfi_eft_batch *batch) {
    if (!batch) return;
    batch->batch_count = 0;
    memset(batch->batch_txs, 0, sizeof(batch->batch_txs));
}

int tsfi_eft_batch_add(tsfi_eft_batch *batch, const tsfi_eft_transaction *tx) {
    if (!batch || !tx || batch->batch_count >= 8) return -1;
    batch->batch_txs[batch->batch_count] = *tx;
    batch->batch_count++;
    return 0;
}

int tsfi_eft_batch_settle(tsfi_eft_batch *batch, float *total_amount_out) {
    if (!batch || !total_amount_out) return -1;
    float sum = 0.0f;
    for (int i = 0; i < batch->batch_count; i++) {
        sum += batch->batch_txs[i].amount;
    }
    *total_amount_out = sum;
    batch->batch_count = 0;
    return 0;
}

float tsfi_market_calculate_growth(const tsfi_market_company *company) {
    if (!company || company->previous_revenue == 0.0) return 0.0f;
    return (float)((company->revenue - company->previous_revenue) / company->previous_revenue);
}

float tsfi_market_calculate_share(const tsfi_market_company *companies, size_t count, size_t index) {
    if (!companies || count == 0 || index >= count) return 0.0f;
    double total = 0.0;
    for (size_t i = 0; i < count; i++) {
        total += companies[i].revenue;
    }
    if (total == 0.0) return 0.0f;
    return (float)(companies[index].revenue / total);
}

void tsfi_bank_vault_init(tsfi_bank_vault *vault, uint16_t pin) {
    if (!vault) return;
    vault->key_inserted = 0;
    vault->expected_pin = pin;
    vault->failed_attempts = 0;
    vault->vault_locked = 0;
}

int tsfi_bank_vault_unlock(tsfi_bank_vault *vault, uint16_t pin) {
    if (!vault) return -1;
    if (vault->vault_locked) return -3;
    if (!vault->key_inserted) return -4;
    if (vault->expected_pin == pin) {
        vault->failed_attempts = 0;
        return 0;
    }
    vault->failed_attempts++;
    if (vault->failed_attempts >= 3) {
        vault->vault_locked = 1;
    }
    return -2;
}

void tsfi_eft_monitor_init(tsfi_eft_monitor *mon, double max_lat) {
    if (!mon) return;
    mon->expected_tx_id = 0;
    mon->dropped_tx_count = 0;
    mon->max_latency_ms = max_lat;
}

int tsfi_eft_monitor_check(tsfi_eft_monitor *mon, const tsfi_eft_transaction *tx) {
    if (!mon || !tx) return -1;
    if (mon->expected_tx_id == 0) {
        mon->expected_tx_id = tx->transaction_id;
    }
    if (tx->transaction_id > mon->expected_tx_id) {
        mon->dropped_tx_count += (tx->transaction_id - mon->expected_tx_id);
    }
    mon->expected_tx_id = tx->transaction_id + 1;
    if (tx->latency_ms > mon->max_latency_ms) {
        return -2;
    }
    return 0;
}

void tsfi_eft_guard_init(tsfi_eft_guard *guard, float max_daily, float single_limit) {
    if (!guard) return;
    guard->daily_total = 0.0f;
    guard->max_daily_limit = max_daily;
    guard->single_tx_limit = single_limit;
}

int tsfi_eft_guard_check(tsfi_eft_guard *guard, float tx_amount) {
    if (!guard) return -1;
    if (tx_amount > guard->single_tx_limit) return -2;
    if (guard->daily_total + tx_amount > guard->max_daily_limit) return -3;
    guard->daily_total += tx_amount;
    return 0;
}

void tsfi_bank_term_init(tsfi_bank_terminal *term, uint32_t master) {
    if (!term) return;
    term->master_key = master;
    term->session_key = 0;
    term->session_active = 0;
}

int tsfi_bank_term_rotate_key(tsfi_bank_terminal *term, uint32_t challenge, uint32_t response) {
    if (!term) return -1;
    uint32_t expected = challenge ^ term->master_key;
    if (response == expected) {
        term->session_key = challenge + 0x12345;
        term->session_active = 1;
        return 0;
    }
    term->session_active = 0;
    return -2;
}

int tsfi_pki_rotate_key(uint8_t public_keys[11][32], int target_index, const uint8_t *new_key, const uint8_t signatures[11][32], int sig_count) {
    if (!public_keys || target_index < 0 || target_index >= 11 || !new_key || !signatures) return -1;
    int valid_sigs = 0;
    for (int i = 0; i < sig_count; i++) {
        for (int k = 0; k < 11; k++) {
            if (memcmp(public_keys[k], signatures[i], 32) == 0) {
                valid_sigs++;
                break;
            }
        }
    }
    if (valid_sigs < 6) return -2;
    memcpy(public_keys[target_index], new_key, 32);
    return 0;
}

void tsfi_migration_init(tsfi_migration_planner *planner) {
    if (!planner) return;
    planner->current_phase = 1;
    planner->network_nodes_ready = 0;
    planner->compatibility_tests_passed = 0;
}

int tsfi_migration_advance(tsfi_migration_planner *planner) {
    if (!planner) return -1;
    if (planner->current_phase == 1) {
        if (planner->compatibility_tests_passed >= 5) {
            planner->current_phase = 2;
            return 0;
        }
        return -2;
    } else if (planner->current_phase == 2) {
        if (planner->network_nodes_ready >= 10) {
            planner->current_phase = 3;
            return 0;
        }
        return -3;
    }
    return 1;
}

void tsfi_vredestein_init(tsfi_vredestein_controller *ctrl) {
    if (!ctrl) return;
    ctrl->write_in_progress = 0;
    ctrl->dirty_flag = 0;
    ctrl->rollback_executed = 0;
}

int tsfi_vredestein_commit(tsfi_vredestein_controller *ctrl) {
    if (!ctrl) return -1;
    if (ctrl->write_in_progress) {
        ctrl->write_in_progress = 0;
        ctrl->dirty_flag = 0;
        return 0;
    }
    return -2;
}

int tsfi_vredestein_rollback(tsfi_vredestein_controller *ctrl) {
    if (!ctrl) return -1;
    if (ctrl->write_in_progress || ctrl->dirty_flag) {
        ctrl->write_in_progress = 0;
        ctrl->dirty_flag = 0;
        ctrl->rollback_executed = 1;
        return 0;
    }
    return -2;
}

void tsfi_consensus_init(tsfi_consensus_engine *eng) {
    if (!eng) return;
    eng->node_count = 0;
    eng->global_state = 0;
    memset(eng->nodes, 0, sizeof(eng->nodes));
}

int tsfi_consensus_add_node(tsfi_consensus_engine *eng, int node_id, int vote) {
    if (!eng || eng->node_count >= 4) return -1;
    eng->nodes[eng->node_count].node_id = node_id;
    eng->nodes[eng->node_count].vote_commit = vote;
    eng->nodes[eng->node_count].received_prepare = 1;
    eng->nodes[eng->node_count].current_state = 1;
    eng->node_count++;
    return 0;
}

int tsfi_consensus_execute(tsfi_consensus_engine *eng) {
    if (!eng || eng->node_count == 0) return -1;
    int all_commit = 1;
    for (int i = 0; i < eng->node_count; i++) {
        if (!eng->nodes[i].vote_commit) {
            all_commit = 0;
            break;
        }
    }
    if (all_commit) {
        eng->global_state = 1;
        for (int i = 0; i < eng->node_count; i++) {
            eng->nodes[i].current_state = 2;
        }
    } else {
        eng->global_state = 2;
        for (int i = 0; i < eng->node_count; i++) {
            eng->nodes[i].current_state = 3;
        }
    }
    return 0;
}

int tsfi_decnet_broadcast_consensus(const tsfi_decnet_router *router, tsfi_consensus_engine *eng) {
    if (!router || !eng) return -1;
    int count = 0;
    for (int i = 0; i < router->route_count; i++) {
        if (eng->node_count >= 4) break;
        int neighbor_node = router->routing_table[i].node_id;
        if (neighbor_node == router->local_node_id) continue;
        int exists = 0;
        for (int k = 0; k < eng->node_count; k++) {
            if (eng->nodes[k].node_id == neighbor_node) {
                exists = 1;
                break;
            }
        }
        if (!exists) {
            tsfi_consensus_add_node(eng, neighbor_node, 1);
            count++;
        }
    }
    return count;
}

int tsfi_vredestein_process_consensus(tsfi_vredestein_controller *ctrl, const tsfi_consensus_engine *eng) {
    if (!ctrl || !eng) return -1;
    if (eng->global_state == 1) {
        return tsfi_vredestein_commit(ctrl);
    } else if (eng->global_state == 2) {
        return tsfi_vredestein_rollback(ctrl);
    }
    return -3;
}

void tsfi_failover_init(tsfi_failover_group *group) {
    if (!group) return;
    group->node_count = 0;
    group->active_primary_id = -1;
    memset(group->nodes, 0, sizeof(group->nodes));
}

int tsfi_failover_add_node(tsfi_failover_group *group, int node_id, int is_primary) {
    if (!group || group->node_count >= 4) return -1;
    group->nodes[group->node_count].node_id = node_id;
    group->nodes[group->node_count].is_active = 1;
    group->nodes[group->node_count].last_heartbeat_tick = 0;
    group->nodes[group->node_count].is_primary = is_primary;
    if (is_primary) {
        group->active_primary_id = node_id;
    }
    group->node_count++;
    return 0;
}

int tsfi_failover_tick(tsfi_failover_group *group, int current_tick, int max_missed_ticks) {
    if (!group) return -1;
    int primary_idx = -1;
    for (int i = 0; i < group->node_count; i++) {
        if (group->nodes[i].node_id == group->active_primary_id) {
            primary_idx = i;
            break;
        }
    }
    if (primary_idx != -1) {
        int missed = current_tick - group->nodes[primary_idx].last_heartbeat_tick;
        if (missed > max_missed_ticks) {
            group->nodes[primary_idx].is_active = 0;
            int promoted = 0;
            for (int i = 0; i < group->node_count; i++) {
                if (group->nodes[i].is_active && !group->nodes[i].is_primary) {
                    group->active_primary_id = group->nodes[i].node_id;
                    promoted = 1;
                    break;
                }
            }
            if (!promoted) {
                group->active_primary_id = -1;
            }
            return 1;
        }
    }
    return 0;
}

void tsfi_cyclades_ts_init(tsfi_cyclades_ts_conn *conn, int conn_id, uint16_t port) {
    if (!conn) return;
    conn->connection_id = conn_id;
    conn->state = CYCLADES_STATE_CLOSED;
    conn->local_port = port;
    conn->remote_port = 0;
}

int tsfi_cyclades_ts_transition(tsfi_cyclades_ts_conn *conn, int event) {
    if (!conn) return -1;
    switch (conn->state) {
        case CYCLADES_STATE_CLOSED:
            if (event == CYCLADES_EVENT_ACTIVE_OPEN) {
                conn->state = CYCLADES_STATE_SYN_SENT;
                return 0;
            }
            break;
        case CYCLADES_STATE_LISTEN:
            if (event == CYCLADES_EVENT_RCV_SYN) {
                conn->state = CYCLADES_STATE_SYN_RCVD;
                return 0;
            }
            break;
        case CYCLADES_STATE_SYN_SENT:
            if (event == CYCLADES_EVENT_RCV_SYN) {
                conn->state = CYCLADES_STATE_ESTABLISHED;
                return 0;
            }
            break;
        case CYCLADES_STATE_SYN_RCVD:
            if (event == CYCLADES_EVENT_SEND_SYN_ACK) {
                conn->state = CYCLADES_STATE_ESTABLISHED;
                return 0;
            }
            break;
        case CYCLADES_STATE_ESTABLISHED:
            if (event == CYCLADES_EVENT_CLOSE) {
                conn->state = CYCLADES_STATE_FIN_WAIT;
                return 0;
            }
            break;
    }
    return -2;
}

void tsfi_token_ring_init(tsfi_token_ring *ring) {
    if (!ring) return;
    ring->token_priority = 0;
    ring->token_held = 0;
    ring->active_station_id = -1;
}

int tsfi_token_ring_pass(tsfi_token_ring *ring, tsfi_tr_station *stations, size_t count) {
    if (!ring || !stations || count == 0) return -1;
    int start_idx = 0;
    for (size_t i = 0; i < count; i++) {
        if (stations[i].station_id == ring->active_station_id) {
            start_idx = (int)(i + 1) % (int)count;
            break;
        }
    }
    for (size_t i = 0; i < count; i++) {
        int idx = (start_idx + (int)i) % (int)count;
        if (stations[idx].pending_frame && stations[idx].frame_priority >= ring->token_priority) {
            ring->active_station_id = stations[idx].station_id;
            ring->token_held = 1;
            stations[idx].pending_frame = 0;
            return 0;
        }
    }
    ring->token_held = 0;
    return 1;
}

void tsfi_sni_gateway_init(tsfi_sni_gateway *gw) {
    if (!gw) return;
    gw->map_count = 0;
    memset(gw->maps, 0, sizeof(gw->maps));
}

int tsfi_sni_gateway_add(tsfi_sni_gateway *gw, uint16_t local_lu, uint16_t target_lu) {
    if (!gw || gw->map_count >= 8) return -1;
    gw->maps[gw->map_count].local_lu_address = local_lu;
    gw->maps[gw->map_count].target_lu_address = target_lu;
    gw->maps[gw->map_count].session_active = 1;
    gw->map_count++;
    return 0;
}

int tsfi_sni_gateway_translate(const tsfi_sni_gateway *gw, uint16_t local_lu, uint16_t *target_lu_out) {
    if (!gw || !target_lu_out) return -1;
    for (int i = 0; i < gw->map_count; i++) {
        if (gw->maps[i].local_lu_address == local_lu && gw->maps[i].session_active) {
            *target_lu_out = gw->maps[i].target_lu_address;
            return 0;
        }
    }
    return -2;
}

int tsfi_cas_filter(tsfi_cas_page *pages, size_t count, const char *search_term) {
    if (!pages || count == 0 || !search_term) return -1;
    int matches = 0;
    for (size_t i = 0; i < count; i++) {
        if (strstr(pages[i].data_payload, search_term) != NULL) {
            pages[i].match_tag = 1;
            matches++;
        } else {
            pages[i].match_tag = 0;
        }
    }
    return matches;
}

void tsfi_ibm3880_init(tsfi_ibm3880_cache *cache) {
    if (!cache) return;
    cache->cache_hits = 0;
    cache->cache_misses = 0;
    cache->active_count = 0;
    memset(cache->cached_addresses, 0, sizeof(cache->cached_addresses));
}

int tsfi_ibm3880_access(tsfi_ibm3880_cache *cache, uint32_t address, int is_write) {
    if (!cache) return -1;
    int found_idx = -1;
    for (int i = 0; i < cache->active_count; i++) {
        if (cache->cached_addresses[i] == address) {
            found_idx = i;
            break;
        }
    }
    if (found_idx != -1) {
        cache->cache_hits++;
        return 0;
    }
    cache->cache_misses++;
    if (!is_write) {
        if (cache->active_count < 4) {
            cache->cached_addresses[cache->active_count] = address;
            cache->active_count++;
        } else {
            cache->cached_addresses[0] = address;
        }
    }
    return 1;
}

int tsfi_cached_cas_filter(tsfi_ibm3880_cache *cache, tsfi_cas_page *pages, size_t count, const char *search_term, int *cache_hits_out) {
    if (!cache || !pages || count == 0 || !search_term || !cache_hits_out) return -1;
    int hits = 0;
    for (size_t i = 0; i < count; i++) {
        uint32_t addr = (uint32_t)pages[i].page_id;
        int res = tsfi_ibm3880_access(cache, addr, 0);
        if (res == 0) {
            hits++;
        }
    }
    *cache_hits_out = hits;
    return tsfi_cas_filter(pages, count, search_term);
}

int tsfi_posix_to_svc_open(const char *path, int flags, tsfi_svc_translation *trans_out) {
    if (!path || !trans_out) return -1;
    int hash = 0;
    for (int i = 0; path[i] != '\0'; i++) {
        hash = (hash * 31) + path[i];
    }
    trans_out->posix_fd = 3;
    trans_out->mainframe_ddname_hash = hash;
    trans_out->status = (flags & 0x02) ? 1 : 0;
    return 0;
}

int tsfi_db2_insert_key(tsfi_db2_index_page *left, tsfi_db2_index_page *right, int key, int *split_occurred) {
    if (!left || !right || !split_occurred) return -1;
    *split_occurred = 0;
    if (left->key_count < 4) {
        int pos = left->key_count;
        while (pos > 0 && left->keys[pos - 1] > key) {
            left->keys[pos] = left->keys[pos - 1];
            pos--;
        }
        left->keys[pos] = key;
        left->key_count++;
        return 0;
    }
    *split_occurred = 1;
    left->sibling_page_id = 999;
    int temp_keys[5];
    int pos = 4;
    memcpy(temp_keys, left->keys, sizeof(left->keys));
    while (pos > 0 && temp_keys[pos - 1] > key) {
        temp_keys[pos] = temp_keys[pos - 1];
        pos--;
    }
    temp_keys[pos] = key;
    left->key_count = 2;
    left->keys[0] = temp_keys[0];
    left->keys[1] = temp_keys[1];
    right->key_count = 3;
    right->keys[0] = temp_keys[2];
    right->keys[1] = temp_keys[3];
    right->keys[2] = temp_keys[4];
    return 0;
}

int tsfi_cad_search_components(const tsfi_cad_component *components, size_t count, const char *meta_query, int *matches_out) {
    if (!components || count == 0 || !meta_query || !matches_out) return -1;
    int matches = 0;
    for (size_t i = 0; i < count; i++) {
        if (strstr(components[i].metadata, meta_query) != NULL) {
            matches++;
        }
    }
    *matches_out = matches;
    return 0;
}

int tsfi_cad_cache_projection(tsfi_ibm3880_cache *cache, tsfi_cad_projection *proj, uint32_t frame_address) {
    if (!cache || !proj) return -1;
    proj->frame_address = frame_address;
    return tsfi_ibm3880_access(cache, frame_address, 0);
}

void tsfi_coax_assemble(tsfi_coax_frame *frame, const uint8_t *data) {
    if (!frame || !data) return;
    frame->sync_pattern[0] = 0xAA;
    frame->sync_pattern[1] = 0x55;
    frame->sync_pattern[2] = 0xAA;
    frame->sync_pattern[3] = 0x55;
    memcpy(frame->payload, data, 32);
    
    uint16_t crc = 0xFFFF;
    for (int i = 0; i < 32; i++) {
        crc ^= frame->payload[i];
        for (int j = 0; j < 8; j++) {
            if (crc & 1) {
                crc = (crc >> 1) ^ 0xA001;
            } else {
                crc >>= 1;
            }
        }
    }
    frame->crc = crc;
}

int tsfi_coax_verify(const tsfi_coax_frame *frame) {
    if (!frame) return -1;
    if (frame->sync_pattern[0] != 0xAA || frame->sync_pattern[1] != 0x55 ||
        frame->sync_pattern[2] != 0xAA || frame->sync_pattern[3] != 0x55) {
        return -2;
    }
    uint16_t crc = 0xFFFF;
    for (int i = 0; i < 32; i++) {
        crc ^= frame->payload[i];
        for (int j = 0; j < 8; j++) {
            if (crc & 1) {
                crc = (crc >> 1) ^ 0xA001;
            } else {
                crc >>= 1;
            }
        }
    }
    return (crc == frame->crc) ? 0 : -3;
}

void tsfi_des_vault_init(tsfi_des_key_vault *vault, const uint8_t *kek) {
    if (!vault) return;
    vault->rotations_count = 0;
    if (kek) {
        memcpy(vault->master_kek, kek, 8);
    } else {
        memset(vault->master_kek, 0, 8);
    }
    memset(vault->active_session_key, 0, 8);
}

int tsfi_des_rotate_session_key(tsfi_des_key_vault *vault) {
    if (!vault) return -1;
    for (int i = 0; i < 8; i++) {
        vault->active_session_key[i] = vault->master_kek[i] ^ (uint8_t)(vault->rotations_count + i);
    }
    vault->rotations_count++;
    return 0;
}

int tsfi_cad_map_vulkan_buffer(struct VulkanContext *vk, const tsfi_cad_projection *proj, void *vk_mapped_memory) {
    if (!proj || !vk_mapped_memory) return -1;
    (void)vk;
    float *dest = (float*)vk_mapped_memory;
    for (int i = 0; i < proj->coordinate_count && i < 32; i++) {
        dest[i * 2] = proj->projected_x[i];
        dest[i * 2 + 1] = proj->projected_y[i];
    }
    return 0;
}

int tsfi_lockstep_evaluate(tsfi_lockstep_cpu *cpu, uint32_t state_a, uint32_t state_b) {
    if (!cpu) return -1;
    cpu->reg_a = state_a;
    cpu->reg_b = state_b;
    if (state_a != state_b) {
        cpu->divergence_detected = 1;
        return 1;
    }
    cpu->divergence_detected = 0;
    return 0;
}

int tsfi_audit_constraint(const tsfi_dictionary_constraint *constraints, size_t count, int column_id, int val) {
    if (!constraints || count == 0) return -1;
    for (size_t i = 0; i < count; i++) {
        if (constraints[i].column_id == column_id) {
            if (val < constraints[i].min_val || val > constraints[i].max_val) {
                return -2;
            }
            return 0;
        }
    }
    return -3;
}

void tsfi_sna_path_init(tsfi_sna_path_control *pc) {
    if (!pc) return;
    pc->route_count = 0;
    memset(pc->routes, 0, sizeof(pc->routes));
}

int tsfi_sna_path_add(tsfi_sna_path_control *pc, uint8_t subarea, uint8_t element, int cost) {
    if (!pc || pc->route_count >= 8) return -1;
    pc->routes[pc->route_count].subarea_id = subarea;
    pc->routes[pc->route_count].element_id = element;
    pc->routes[pc->route_count].explicit_route_cost = cost;
    pc->route_count++;
    return 0;
}

int tsfi_sna_path_route(const tsfi_sna_path_control *pc, uint8_t subarea, int *cost_out) {
    if (!pc || !cost_out) return -1;
    for (int i = 0; i < pc->route_count; i++) {
        if (pc->routes[i].subarea_id == subarea) {
            *cost_out = pc->routes[i].explicit_route_cost;
            return 0;
        }
    }
    return -2;
}

void tsfi_vtam_buf_init(tsfi_vtam_buf_session *sess, int sess_id) {
    if (!sess) return;
    sess->session_id = sess_id;
    sess->buffer_allocation = 0;
    sess->data_flow_state = -1;
}

int tsfi_vtam_buf_bind(tsfi_vtam_buf_session *sess, int buffer_size) {
    if (!sess || buffer_size <= 0) return -1;
    sess->buffer_allocation = buffer_size;
    sess->data_flow_state = 1;
    return 0;
}

void tsfi_coax_controller_init(tsfi_coax_controller *ctrl) {
    if (!ctrl) return;
    ctrl->device_count = 0;
    memset(ctrl->devices, 0, sizeof(ctrl->devices));
}

int tsfi_coax_controller_poll(tsfi_coax_controller *ctrl, int *active_device_id_out) {
    if (!ctrl || !active_device_id_out) return -1;
    for (int i = 0; i < ctrl->device_count; i++) {
        ctrl->devices[i].poll_count++;
        if (ctrl->devices[i].status_register & 0x01) {
            *active_device_id_out = ctrl->devices[i].device_id;
            return 0;
        }
    }
    *active_device_id_out = -1;
    return 1;
}

int tsfi_scsi_authorize_transaction(tsfi_scsi_transaction *tx, const uint8_t *expected_hash) {
    if (!tx || !expected_hash) return -1;
    if (memcmp(tx->payload_hash, expected_hash, 32) == 0) {
        tx->signature_verified = 1;
        tx->is_fips_compliant = 1;
        return 0;
    }
    tx->signature_verified = 0;
    tx->is_fips_compliant = 0;
    return -2;
}

int tsfi_scsi_coax_bridge_transfer(tsfi_scsi_transaction *tx, tsfi_coax_controller *coax_ctrl, int *selected_device_id_out) {
    if (!tx || !coax_ctrl || !selected_device_id_out) return -1;
    int active_dev = -1;
    int res = tsfi_coax_controller_poll(coax_ctrl, &active_dev);
    if (res == 0 && active_dev != -1) {
        tx->signature_verified = 1;
        tx->is_fips_compliant = 1;
        *selected_device_id_out = active_dev;
        return 0;
    }
    *selected_device_id_out = -1;
    return 1;
}

int tsfi_scsi_coax_bridge_send_frame(tsfi_scsi_transaction *tx, tsfi_coax_controller *coax_ctrl, tsfi_coax_frame *frame_out, int *selected_device_id_out) {
    if (!tx || !coax_ctrl || !frame_out || !selected_device_id_out) return -1;
    int res = tsfi_scsi_coax_bridge_transfer(tx, coax_ctrl, selected_device_id_out);
    if (res == 0) {
        tsfi_coax_assemble(frame_out, tx->payload_hash);
        return 0;
    }
    return res;
}

int tsfi_scsi_coax_bridge_send_encrypted_frame(tsfi_scsi_transaction *tx, tsfi_coax_controller *coax_ctrl, tsfi_des_key_vault *vault, tsfi_coax_frame *frame_out, int *selected_device_id_out) {
    if (!tx || !coax_ctrl || !vault || !frame_out || !selected_device_id_out) return -1;
    int res = tsfi_scsi_coax_bridge_transfer(tx, coax_ctrl, selected_device_id_out);
    if (res == 0) {
        uint8_t enc_payload[32];
        for (int i = 0; i < 32; i++) {
            enc_payload[i] = tx->payload_hash[i] ^ vault->active_session_key[i % 8];
        }
        tsfi_coax_assemble(frame_out, enc_payload);
        return 0;
    }
    return res;
}

int tsfi_appc_coax_bridge_coordinate(tsfi_appc_conversation *conv, tsfi_scsi_transaction *tx, tsfi_coax_controller *coax_ctrl, tsfi_des_key_vault *vault, tsfi_coax_frame *frame_out, int *selected_device_id_out) {
    if (!conv || !tx || !coax_ctrl || !vault || !frame_out || !selected_device_id_out) return -1;
    if (conv->state != 0 && conv->state != 1) {
        return -2;
    }
    int res = tsfi_scsi_coax_bridge_send_encrypted_frame(tx, coax_ctrl, vault, frame_out, selected_device_id_out);
    if (res == 0) {
        conv->state = 1;
        return 0;
    }
    return res;
}

int tsfi_appc_consensus_commit(tsfi_appc_conversation *conv, int consensus_success) {
    if (!conv) return -1;
    if (consensus_success) {
        conv->state = 3; // DEALLOCATED
    } else {
        conv->state = 0; // ALLOCATED
    }
    return 0;
}

int tsfi_appc_lockstep_abort_check(tsfi_appc_conversation *conv, const tsfi_lockstep_cpu *cpu) {
    if (!conv || !cpu) return -1;
    if (cpu->divergence_detected) {
        conv->state = 3;
        return 1;
    }
    return 0;
}

int tsfi_appc_audit_transaction(tsfi_appc_conversation *conv, const tsfi_dictionary_constraint *constraints, size_t count, int column_id, int val) {
    if (!conv || !constraints) return -1;
    int res = tsfi_audit_constraint(constraints, count, column_id, val);
    if (res != 0) {
        conv->state = 3;
        return res;
    }
    return tsfi_audit_constraint(constraints, count, column_id, val);
}

void tsfi_cua_terminal_init(tsfi_cua_terminal *term) {
    if (!term) return;
    term->node_count = 0;
    term->active_transfers = 0;
    memset(term->nodes, 0, sizeof(term->nodes));
}

int tsfi_cua_terminal_add_node(tsfi_cua_terminal *term, const char *name, int row, int col) {
    if (!term || !name || term->node_count >= 16) return -1;
    strncpy(term->nodes[term->node_count].name, name, 15);
    term->nodes[term->node_count].name[15] = '\0';
    term->nodes[term->node_count].state = 0; // IDLE
    term->nodes[term->node_count].row = row;
    term->nodes[term->node_count].col = col;
    term->node_count++;
    return 0;
}

int tsfi_cua_terminal_set_transfer(tsfi_cua_terminal *term, const char *node_name, int state) {
    if (!term || !node_name) return -1;
    for (int i = 0; i < term->node_count; i++) {
        if (strcmp(term->nodes[i].name, node_name) == 0) {
            term->nodes[i].state = state;
            if (state == 1) {
                term->active_transfers++;
            }
            return 0;
        }
    }
    return -2;
}

int tsfi_cua_terminal_render(const tsfi_cua_terminal *term, char *screen_buf, size_t max_len) {
    if (!term || !screen_buf || max_len < 2000) return -1;

    char grid[24][80];
    memset(grid, ' ', sizeof(grid));

    // Draw CUA window borders
    for (int c = 0; c < 80; c++) {
        grid[0][c] = '=';
        grid[2][c] = '-';
        grid[22][c] = '-';
        grid[23][c] = '=';
    }

    // Titles and Headers
    const char *title = " AMERITECH CUA NODE MONITOR - S/370 NETWORK STATUS ";
    size_t title_len = strlen(title);
    memcpy(&grid[0][(80 - title_len) / 2], title, title_len);

    const char *action_bar = " F1=Help   F3=Exit   F5=Refresh   F9=Transfer ";
    memcpy(&grid[1][2], action_bar, strlen(action_bar));

    // Render nodes onto the grid
    for (int i = 0; i < term->node_count; i++) {
        const tsfi_cua_node *n = &term->nodes[i];
        if (n->row >= 3 && n->row < 22 && n->col >= 0 && n->col < 70) {
            // Write name
            size_t name_len = strlen(n->name);
            memcpy(&grid[n->row][n->col], n->name, name_len);
            
            // Draw state block next to the name
            int block_col = n->col + (int)name_len + 1;
            if (n->state == 1) {
                // RED TRANSFER block
                memcpy(&grid[n->row][block_col], "[RED_TRANSFER]", 14);
            } else if (n->state == 2) {
                // AMERITECH BUSY block
                memcpy(&grid[n->row][block_col], "[BUSY_SIGNAL]", 13);
            } else {
                // IDLE BLACK block
                memcpy(&grid[n->row][block_col], "[BLACK_NODE]", 12);
            }
        }
    }

    // Convert grid to flat buffer
    char *p = screen_buf;
    for (int r = 0; r < 24; r++) {
        memcpy(p, grid[r], 80);
        p += 80;
        *p = '\n';
        p++;
    }
    *p = '\0';

    return 0;
}

