#include "tsfi_mainframe_decnet.h"

void tsfi_decnet_init(tsfi_decnet_router *router, uint16_t local_id) {
    if (!router) return;
    router->local_node_id = local_id;
    router->route_count = 0;
    router->routing_table[0].node_id = local_id;
    router->routing_table[0].next_hop = local_id;
    router->routing_table[0].cost = 0;
    router->routing_table[0].age = 0;
    router->route_count = 1;
}

int tsfi_decnet_add_neighbor(tsfi_decnet_router *router, uint16_t neighbor_id, uint16_t cost) {
    if (!router) return -1;
    if (router->route_count >= 32) return -2;
    router->routing_table[router->route_count].node_id = neighbor_id;
    router->routing_table[router->route_count].next_hop = neighbor_id;
    router->routing_table[router->route_count].cost = cost;
    router->routing_table[router->route_count].age = 0;
    router->route_count++;
    return 0;
}

int tsfi_decnet_update_route(tsfi_decnet_router *router, uint16_t dest_id, uint16_t next_hop, uint16_t cost) {
    if (!router) return -1;
    for (int i = 0; i < router->route_count; i++) {
        if (router->routing_table[i].node_id == dest_id) {
            if (cost < router->routing_table[i].cost) {
                router->routing_table[i].next_hop = next_hop;
                router->routing_table[i].cost = cost;
                router->routing_table[i].age = 0;
            }
            return 0;
        }
    }
    if (router->route_count >= 32) return -2;
    router->routing_table[router->route_count].node_id = dest_id;
    router->routing_table[router->route_count].next_hop = next_hop;
    router->routing_table[router->route_count].cost = cost;
    router->routing_table[router->route_count].age = 0;
    router->route_count++;
    return 0;
}

int tsfi_decnet_route_packet(tsfi_decnet_router *router, uint16_t dest_id, uint16_t *next_hop_out) {
    if (!router || !next_hop_out) return -1;
    for (int i = 0; i < router->route_count; i++) {
        if (router->routing_table[i].node_id == dest_id) {
            *next_hop_out = router->routing_table[i].next_hop;
            return 0;
        }
    }
    return -2;
}

uint16_t tsfi_sdlc_crc16(const uint8_t *data, size_t len) {
    if (!data) return 0;
    uint16_t crc = 0xFFFF;
    for (size_t i = 0; i < len; i++) {
        crc ^= (data[i] << 8);
        for (int j = 0; j < 8; j++) {
            if (crc & 0x8000) {
                crc = (crc << 1) ^ 0x1021;
            } else {
                crc <<= 1;
            }
        }
    }
    return crc;
}

int tsfi_sdlc_serialize(const tsfi_sdlc_frame *frame, uint8_t *buf_out, size_t *len_out) {
    if (!frame || !buf_out || !len_out) return -1;
    if (frame->payload_len > 256) return -2;
    
    buf_out[0] = 0x7E;
    buf_out[1] = frame->address;
    buf_out[2] = frame->control;
    memcpy(buf_out + 3, frame->payload, frame->payload_len);
    
    uint16_t crc = tsfi_sdlc_crc16(buf_out + 1, 2 + frame->payload_len);
    
    size_t offset = 3 + frame->payload_len;
    buf_out[offset] = (crc >> 8) & 0xFF;
    buf_out[offset + 1] = crc & 0xFF;
    buf_out[offset + 2] = 0x7E;
    
    *len_out = offset + 3;
    return 0;
}

int tsfi_sdlc_deserialize(const uint8_t *buf, size_t len, tsfi_sdlc_frame *frame_out) {
    if (!buf || !frame_out || len < 6) return -1;
    if (buf[0] != 0x7E || buf[len - 1] != 0x7E) return -2;
    
    frame_out->address = buf[1];
    frame_out->control = buf[2];
    frame_out->payload_len = len - 6;
    if (frame_out->payload_len > 256) return -3;
    
    memcpy(frame_out->payload, buf + 3, frame_out->payload_len);
    
    uint16_t crc_rx = (buf[len - 3] << 8) | buf[len - 2];
    uint16_t crc_calc = tsfi_sdlc_crc16(buf + 1, 2 + frame_out->payload_len);
    frame_out->fcs = crc_rx;
    
    if (crc_rx != crc_calc) return -4;
    return 0;
}

void tsfi_sdlc_hub_init(tsfi_sdlc_hub *hub) {
    if (!hub) return;
    hub->connected_count = 0;
}

int tsfi_sdlc_hub_connect(tsfi_sdlc_hub *hub, uint8_t addr) {
    if (!hub) return -1;
    if (hub->connected_count >= 8) return -2;
    hub->connected_addresses[hub->connected_count++] = addr;
    return 0;
}

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

int tsfi_decnet_bridge_send_update(tsfi_decnet_router *router, uint16_t dest_node, tsfi_sdlc_hub *hub, uint8_t sdlc_addr) {
    if (!router || !hub) return -1;
    
    uint16_t cost = 0xFFFF;
    uint16_t next_hop = 0xFFFF;
    int found = 0;
    for (int i = 0; i < router->route_count; i++) {
        if (router->routing_table[i].node_id == dest_node) {
            cost = router->routing_table[i].cost;
            next_hop = router->routing_table[i].next_hop;
            found = 1;
            break;
        }
    }
    if (!found) return -2;
    
    tsfi_sdlc_frame frame;
    frame.address = sdlc_addr;
    frame.control = SDLC_CMD_POLL;
    frame.payload_len = 6;
    frame.payload[0] = (dest_node >> 8) & 0xFF;
    frame.payload[1] = dest_node & 0xFF;
    frame.payload[2] = (next_hop >> 8) & 0xFF;
    frame.payload[3] = next_hop & 0xFF;
    frame.payload[4] = (cost >> 8) & 0xFF;
    frame.payload[5] = cost & 0xFF;
    
    uint8_t tx_buf[512];
    size_t tx_len = 0;
    if (tsfi_sdlc_serialize(&frame, tx_buf, &tx_len) != 0) return -3;
    
    uint8_t rx_buf[512];
    size_t rx_len = 0;
    if (tsfi_sdlc_hub_broadcast(hub, tx_buf, tx_len, sdlc_addr, rx_buf, &rx_len) != 0) return -4;
    
    return 0;
}

int tsfi_decnet_bridge_receive_update(tsfi_decnet_router *router, const uint8_t *sdlc_frame_data, size_t len) {
    if (!router || !sdlc_frame_data || len == 0) return -1;
    
    tsfi_sdlc_frame frame;
    if (tsfi_sdlc_deserialize(sdlc_frame_data, len, &frame) != 0) return -2;
    if (frame.payload_len < 6) return -3;
    
    uint16_t dest_node = (frame.payload[0] << 8) | frame.payload[1];
    uint16_t next_hop = (frame.payload[2] << 8) | frame.payload[3];
    uint16_t cost = (frame.payload[4] << 8) | frame.payload[5];
    
    return tsfi_decnet_update_route(router, dest_node, next_hop, cost + 1);
}

void tsfi_sdlc_link_init(tsfi_sdlc_link_state *link) {
    if (!link) return;
    link->ns = 0;
    link->nr = 0;
    link->link_established = 0;
}

int tsfi_sdlc_link_handshake(tsfi_sdlc_link_state *primary, tsfi_sdlc_link_state *secondary, uint8_t command) {
    if (!primary || !secondary) return -1;
    if (command == SDLC_CMD_SNRM) {
        primary->link_established = 0;
        secondary->link_established = 1;
        primary->link_established = 1;
        return SDLC_CMD_UA;
    } else if (command == SDLC_CMD_DISC) {
        primary->link_established = 0;
        secondary->link_established = 0;
        return SDLC_CMD_UA;
    }
    return -2;
}

int tsfi_sdlc_verify_sequence(tsfi_sdlc_link_state *link, uint8_t ns_received, uint8_t nr_received) {
    if (!link) return -1;
    if (ns_received != link->nr) {
        return -2;
    }
    link->nr = (link->nr + 1) % 8;
    link->ns = nr_received;
    return 0;
}

void tsfi_sdlc_scheduler_init(tsfi_sdlc_scheduler *sched) {
    if (!sched) return;
    sched->station_count = 0;
    sched->current_index = 0;
}

int tsfi_sdlc_scheduler_add(tsfi_sdlc_scheduler *sched, uint8_t addr) {
    if (!sched) return -1;
    if (sched->station_count >= 8) return -2;
    sched->stations[sched->station_count++] = addr;
    return 0;
}

uint8_t tsfi_sdlc_scheduler_next(tsfi_sdlc_scheduler *sched) {
    if (!sched || sched->station_count == 0) return 0x00;
    uint8_t addr = sched->stations[sched->current_index];
    sched->current_index = (sched->current_index + 1) % sched->station_count;
    return addr;
}

int tsfi_sdlc_generate_frmr(uint8_t invalid_ctrl, uint8_t expected_nr, uint8_t actual_nr, uint8_t *frmr_data_out) {
    if (!frmr_data_out) return -1;
    frmr_data_out[0] = invalid_ctrl;
    frmr_data_out[1] = (expected_nr << 5) | (actual_nr << 1) | 0x01;
    frmr_data_out[2] = 0x08;
    return 0;
}

void tsfi_sdlc_timer_start(tsfi_sdlc_t1_timer *timer) {
    if (!timer) return;
    timer->timer_active = 1;
    timer->timeout_count = 0;
}

int tsfi_sdlc_timer_tick(tsfi_sdlc_t1_timer *timer, int elapsed_ms, int max_wait_ms) {
    if (!timer) return -1;
    if (!timer->timer_active) return 0;
    if (elapsed_ms >= max_wait_ms) {
        timer->timeout_count++;
        timer->timer_active = 0;
        return 1;
    }
    return 0;
}

void tsfi_sdlc_line_init(tsfi_sdlc_line_mode *line) {
    if (!line) return;
    line->tx_enabled = 1;
    line->rx_enabled = 0;
    line->turnaround_requested = 0;
}

int tsfi_sdlc_line_turnaround(tsfi_sdlc_line_mode *line) {
    if (!line) return -1;
    int temp = line->tx_enabled;
    line->tx_enabled = line->rx_enabled;
    line->rx_enabled = temp;
    line->turnaround_requested = 0;
    return 0;
}

void tsfi_decnet_nsp_init(tsfi_decnet_nsp_link *link, uint16_t local_id) {
    if (!link) return;
    link->local_link_id = local_id;
    link->remote_link_id = 0;
    link->state = 0;
}

int tsfi_decnet_nsp_connect(tsfi_decnet_nsp_link *local, tsfi_decnet_nsp_link *remote) {
    if (!local || !remote) return -1;
    local->state = 1;
    remote->remote_link_id = local->local_link_id;
    remote->state = 2;
    local->remote_link_id = remote->local_link_id;
    local->state = 2;
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

void tsfi_decnet_dir_init(tsfi_decnet_directory *dir) {
    if (!dir) return;
    dir->entry_count = 0;
}

int tsfi_decnet_dir_add(tsfi_decnet_directory *dir, const char *name, uint16_t node_id) {
    if (!dir || !name) return -1;
    if (dir->entry_count >= 16) return -2;
    strncpy(dir->entries[dir->entry_count].name, name, 15);
    dir->entries[dir->entry_count].name[15] = '\0';
    dir->entries[dir->entry_count].node_id = node_id;
    dir->entry_count++;
    return 0;
}

int tsfi_decnet_dir_resolve(tsfi_decnet_directory *dir, const char *name, uint16_t *node_id_out) {
    if (!dir || !name || !node_id_out) return -1;
    for (int i = 0; i < dir->entry_count; i++) {
        if (strcmp(dir->entries[i].name, name) == 0) {
            *node_id_out = dir->entries[i].node_id;
            return 0;
        }
    }
    return -2;
}

int tsfi_sdlc_update_buffer_status(tsfi_sdlc_link_state *link, int buffer_busy) {
    if (!link) return -1;
    return buffer_busy ? SDLC_CMD_RNR : SDLC_CMD_RR;
}

int tsfi_decnet_forward_packet(tsfi_decnet_router *router, uint16_t src, uint16_t dest, uint16_t *next_hop_out) {
    if (!router || !next_hop_out) return -1;
    (void)src;
    if (dest == router->local_node_id) {
        *next_hop_out = router->local_node_id;
        return 0;
    }
    return tsfi_decnet_route_packet(router, dest, next_hop_out);
}

int tsfi_decnet_exchange_hello(tsfi_decnet_router *local, tsfi_decnet_router *remote, uint16_t link_cost) {
    if (!local || !remote) return -1;
    int r1 = tsfi_decnet_add_neighbor(local, remote->local_node_id, link_cost);
    int r2 = tsfi_decnet_add_neighbor(remote, local->local_node_id, link_cost);
    return (r1 == 0 && r2 == 0) ? 0 : -2;
}

int tsfi_decnet_serialize_header(const tsfi_decnet_header *hdr, uint8_t *buf, size_t *len_out) {
    if (!hdr || !buf || !len_out) return -1;
    buf[0] = hdr->flags;
    buf[1] = (hdr->dst_node >> 8) & 0xFF;
    buf[2] = hdr->dst_node & 0xFF;
    buf[3] = (hdr->src_node >> 8) & 0xFF;
    buf[4] = hdr->src_node & 0xFF;
    buf[5] = hdr->forward_count;
    *len_out = 6;
    return 0;
}

int tsfi_decnet_deserialize_header(const uint8_t *buf, size_t len, tsfi_decnet_header *hdr_out) {
    if (!buf || !hdr_out || len < 6) return -1;
    hdr_out->flags = buf[0];
    hdr_out->dst_node = (buf[1] << 8) | buf[2];
    hdr_out->src_node = (buf[3] << 8) | buf[4];
    hdr_out->forward_count = buf[5];
    return 0;
}

void tsfi_sdlc_queue_init(tsfi_sdlc_tx_queue *q) {
    if (!q) return;
    q->count = 0;
}

int tsfi_sdlc_queue_push(tsfi_sdlc_tx_queue *q, const tsfi_sdlc_frame *f) {
    if (!q || !f) return -1;
    if (q->count >= 4) return -2;
    q->frames[q->count++] = *f;
    return 0;
}

int tsfi_sdlc_queue_ack(tsfi_sdlc_tx_queue *q, uint8_t nr) {
    if (!q) return -1;
    if (q->count > 0 && nr > 0) {
        q->count = 0;
        return 1;
    }
    return 0;
}

int tsfi_decnet_age_routes(tsfi_decnet_router *router) {
    if (!router) return -1;
    int active_routes = 0;
    for (int i = 0; i < router->route_count; i++) {
        if (router->routing_table[i].node_id == router->local_node_id) {
            active_routes++;
            continue;
        }
        router->routing_table[i].age++;
        if (router->routing_table[i].age >= 3) {
            router->routing_table[i] = router->routing_table[router->route_count - 1];
            router->route_count--;
            i--;
        } else {
            active_routes++;
        }
    }
    return active_routes;
}

int tsfi_sdlc_bit_stuff(const uint8_t *in, size_t in_len, uint8_t *out, size_t *out_len) {
    if (!in || !out || !out_len) return -1;
    size_t out_bit_idx = 0;
    int consecutive_ones = 0;
    for (size_t i = 0; i < in_len; i++) {
        for (int bit = 7; bit >= 0; bit--) {
            int val = (in[i] >> bit) & 0x01;
            size_t out_byte = out_bit_idx / 8;
            int out_bit = 7 - (out_bit_idx % 8);
            if (out_bit == 7) out[out_byte] = 0;
            out[out_byte] |= (val << out_bit);
            out_bit_idx++;
            if (val == 1) {
                consecutive_ones++;
                if (consecutive_ones == 5) {
                    size_t z_byte = out_bit_idx / 8;
                    int z_bit = 7 - (out_bit_idx % 8);
                    if (z_bit == 7) out[z_byte] = 0;
                    out_bit_idx++;
                    consecutive_ones = 0;
                }
            } else {
                consecutive_ones = 0;
            }
        }
    }
    *out_len = (out_bit_idx + 7) / 8;
    return 0;
}

int tsfi_sdlc_bit_unstuff(const uint8_t *in, size_t in_len, uint8_t *out, size_t *out_len) {
    if (!in || !out || !out_len) return -1;
    size_t out_bit_idx = 0;
    int consecutive_ones = 0;
    size_t total_bits = in_len * 8;
    for (size_t in_bit_idx = 0; in_bit_idx < total_bits; in_bit_idx++) {
        size_t in_byte = in_bit_idx / 8;
        int in_bit = 7 - (in_bit_idx % 8);
        int val = (in[in_byte] >> in_bit) & 0x01;
        
        size_t out_byte = out_bit_idx / 8;
        int out_bit = 7 - (out_bit_idx % 8);
        if (out_bit == 7) out[out_byte] = 0;
        out[out_byte] |= (val << out_bit);
        out_bit_idx++;
        
        if (val == 1) {
            consecutive_ones++;
            if (consecutive_ones == 5) {
                in_bit_idx++;
                consecutive_ones = 0;
            }
        } else {
            consecutive_ones = 0;
        }
    }
    *out_len = out_bit_idx / 8;
    return 0;
}

int tsfi_sdlc_detect_abort(const uint8_t *bit_stream, size_t bit_len) {
    if (!bit_stream) return -1;
    int consecutive_ones = 0;
    for (size_t i = 0; i < bit_len; i++) {
        size_t byte_idx = i / 8;
        int bit_idx = 7 - (i % 8);
        int val = (bit_stream[byte_idx] >> bit_idx) & 0x01;
        if (val == 1) {
            consecutive_ones++;
            if (consecutive_ones >= 7) {
                return 1;
            }
        } else {
            consecutive_ones = 0;
        }
    }
    return 0;
}

int tsfi_decnet_hop_check(tsfi_decnet_header *hdr) {
    if (!hdr) return -1;
    hdr->forward_count++;
    if (hdr->forward_count > 15) {
        return -2;
    }
    return 0;
}

void tsfi_decnet_nsp_flow_init(tsfi_decnet_nsp_flow *flow, int initial_credits) {
    if (!flow) return;
    flow->tx_credits = initial_credits;
    flow->rx_credits = initial_credits;
}

int tsfi_decnet_nsp_flow_consume(tsfi_decnet_nsp_flow *flow) {
    if (!flow) return -1;
    if (flow->tx_credits <= 0) return -2;
    flow->tx_credits--;
    return 0;
}

void tsfi_decnet_nsp_flow_grant(tsfi_decnet_nsp_flow *flow, int credits) {
    if (!flow) return;
    flow->tx_credits += credits;
}

void tsfi_sdlc_mux_init(tsfi_sdlc_multiplexer *mux) {
    if (!mux) return;
    mux->count = 0;
}

int tsfi_sdlc_mux_add(tsfi_sdlc_multiplexer *mux, uint8_t addr) {
    if (!mux) return -1;
    if (mux->count >= 4) return -2;
    mux->secondary_addresses[mux->count++] = addr;
    return 0;
}

int tsfi_sdlc_mux_route(tsfi_sdlc_multiplexer *mux, uint8_t addr) {
    if (!mux) return -1;
    for (int i = 0; i < mux->count; i++) {
        if (mux->secondary_addresses[i] == addr) {
            return i;
        }
    }
    return -2;
}

void tsfi_decnet_reassembler_init(tsfi_decnet_reassembler *r, int expected) {
    if (!r) return;
    r->len = 0;
    r->expected_segments = expected;
    r->current_segments = 0;
}

int tsfi_decnet_reassembler_add(tsfi_decnet_reassembler *r, const uint8_t *segment, size_t seg_len) {
    if (!r || !segment) return -1;
    if (r->len + seg_len > 1024) return -2;
    memcpy(r->buffer + r->len, segment, seg_len);
    r->len += seg_len;
    r->current_segments++;
    if (r->current_segments >= r->expected_segments) {
        return 1;
    }
    return 0;
}

int tsfi_decnet_nsp_send_interrupt(tsfi_decnet_nsp_link *link, const uint8_t *data, size_t len, uint8_t *pkt_out, size_t *pkt_len) {
    if (!link || !data || !pkt_out || !pkt_len) return -1;
    if (len > 256) return -2;
    pkt_out[0] = NSP_MSG_INTERRUPT;
    pkt_out[1] = (link->remote_link_id >> 8) & 0xFF;
    pkt_out[2] = link->remote_link_id & 0xFF;
    memcpy(pkt_out + 3, data, len);
    *pkt_len = 3 + len;
    return 0;
}

void tsfi_nsp_reorder_init(tsfi_nsp_reorder_queue *q) {
    if (!q) return;
    q->count = 0;
    for (int i = 0; i < 8; i++) {
        q->items[i].valid = 0;
    }
}

int tsfi_nsp_reorder_add(tsfi_nsp_reorder_queue *q, uint8_t seq, const uint8_t *data, size_t len) {
    if (!q || !data) return -1;
    if (len > 256) return -2;
    if (q->count >= 8) return -3;
    
    int insert_pos = q->count;
    for (int i = 0; i < q->count; i++) {
        if (seq < q->items[i].seq_num) {
            insert_pos = i;
            break;
        }
    }
    
    for (int i = q->count; i > insert_pos; i--) {
        q->items[i] = q->items[i - 1];
    }
    
    q->items[insert_pos].seq_num = seq;
    q->items[insert_pos].len = len;
    memcpy(q->items[insert_pos].data, data, len);
    q->items[insert_pos].valid = 1;
    q->count++;
    return 0;
}

int tsfi_decnet_nsp_probe(tsfi_decnet_nsp_link *link, int ticks_elapsed, int timeout_ticks) {
    if (!link) return -1;
    if (link->state != 2) return -2;
    if (ticks_elapsed >= timeout_ticks) {
        link->state = 0;
        return 1;
    }
    return 0;
}

int tsfi_nsp_serialize_control(const tsfi_nsp_control_msg *msg, uint8_t *buf, size_t *len_out) {
    if (!msg || !buf || !len_out) return -1;
    buf[0] = msg->command;
    buf[1] = (msg->src_link >> 8) & 0xFF;
    buf[2] = msg->src_link & 0xFF;
    buf[3] = (msg->dst_link >> 8) & 0xFF;
    buf[4] = msg->dst_link & 0xFF;
    buf[5] = msg->services;
    *len_out = 6;
    return 0;
}

int tsfi_nsp_deserialize_control(const uint8_t *buf, size_t len, tsfi_nsp_control_msg *msg_out) {
    if (!buf || !msg_out || len < 6) return -1;
    msg_out->command = buf[0];
    msg_out->src_link = (buf[1] << 8) | buf[2];
    msg_out->dst_link = (buf[3] << 8) | buf[4];
    msg_out->services = buf[5];
    return 0;
}

void tsfi_nsp_retransmit_start(tsfi_nsp_retransmit_timer *timer) {
    if (!timer) return;
    timer->active = 1;
    timer->elapsed = 0;
    timer->retries = 0;
}

int tsfi_nsp_retransmit_tick(tsfi_nsp_retransmit_timer *timer, int elapsed_ms, int timeout_ms, int max_retries) {
    if (!timer) return -1;
    if (!timer->active) return 0;
    timer->elapsed += elapsed_ms;
    if (timer->elapsed >= timeout_ms) {
        timer->retries++;
        timer->elapsed = 0;
        if (timer->retries > max_retries) {
            timer->active = 0;
            return -2;
        }
        return 1;
    }
    return 0;
}

void tsfi_nsp_subchannel_init(tsfi_nsp_subchannel *sub, uint16_t link_id) {
    if (!sub) return;
    sub->normal_subchannel = link_id;
    sub->interrupt_subchannel = link_id + 1000;
}

void tsfi_ibm3705_init(tsfi_ibm3705_ctc *ctc) {
    if (!ctc) return;
    ctc->command_register = 0x00;
    ctc->status_register = 0x00;
    ctc->interrupt_pending = 0;
}

int tsfi_ibm3705_write_cmd(tsfi_ibm3705_ctc *ctc, uint8_t cmd) {
    if (!ctc) return -1;
    ctc->command_register = cmd;
    ctc->status_register |= 0x02;
    ctc->interrupt_pending = 1;
    return 0;
}

uint8_t tsfi_ibm3705_read_status(tsfi_ibm3705_ctc *ctc) {
    if (!ctc) return 0xFF;
    ctc->interrupt_pending = 0;
    return ctc->status_register;
}

void tsfi_ibm3270_init(tsfi_ibm3270_terminal *term) {
    if (!term) return;
    memset(term->screen_buffer, ' ', 1920);
    term->cursor_position = 0;
    term->buffer_updated = 0;
}

int tsfi_ibm3270_write_char(tsfi_ibm3270_terminal *term, char c, int pos) {
    if (!term) return -1;
    if (pos < 0 || pos >= 1920) return -2;
    term->screen_buffer[pos] = c;
    term->cursor_position = pos + 1;
    term->buffer_updated = 1;
    return 0;
}

int tsfi_vtam_serialize_piu(const tsfi_vtam_piu_header *piu, const uint8_t *payload, size_t pay_len, uint8_t *buf_out, size_t *len_out) {
    if (!piu || !buf_out || !len_out) return -1;
    if (pay_len > 256) return -2;
    
    buf_out[0] = piu->th_fid;
    buf_out[1] = piu->th_daf;
    buf_out[2] = piu->th_saf;
    buf_out[3] = (piu->th_snf >> 8) & 0xFF;
    buf_out[4] = piu->th_snf & 0xFF;
    buf_out[5] = piu->rh_type;
    
    if (payload && pay_len > 0) {
        memcpy(buf_out + 6, payload, pay_len);
    }
    *len_out = 6 + pay_len;
    return 0;
}

int tsfi_vtam_deserialize_piu(const uint8_t *buf, size_t len, tsfi_vtam_piu_header *piu_out, uint8_t *pay_out, size_t *pay_len_out) {
    if (!buf || !piu_out || len < 6) return -1;
    
    piu_out->th_fid = buf[0];
    piu_out->th_daf = buf[1];
    piu_out->th_saf = buf[2];
    piu_out->th_snf = (buf[3] << 8) | buf[4];
    piu_out->rh_type = buf[5];
    
    if (pay_out && pay_len_out) {
        *pay_len_out = len - 6;
        if (*pay_len_out > 0) {
            memcpy(pay_out, buf + 6, *pay_len_out);
        }
    }
    return 0;
}

void tsfi_sdlc_loop_init(tsfi_sdlc_loop_station *station, uint8_t addr) {
    if (!station) return;
    station->loop_address = addr;
    station->active_repeater = 1;
    station->has_token = 0;
}

int tsfi_sdlc_loop_forward(tsfi_sdlc_loop_station *station, uint8_t *frame_bits, size_t bit_len) {
    if (!station || !frame_bits) return -1;
    for (size_t i = 0; i < bit_len / 8; i++) {
        if (frame_bits[i] == 0x7F) {
            station->has_token = 1;
            frame_bits[i] = 0xFF;
            return 1;
        }
    }
    return 0;
}

void tsfi_ibm3705_scanner_init(tsfi_ibm3705_scanner *scanner) {
    if (!scanner) return;
    scanner->clock_phase = 0;
    scanner->last_bit = 0;
    scanner->bit_synced = 0;
}

int tsfi_ibm3705_scanner_sample(tsfi_ibm3705_scanner *scanner, int pin_value) {
    if (!scanner) return -1;
    scanner->clock_phase = (scanner->clock_phase + 1) % 4;
    if (scanner->clock_phase == 2) {
        int sampled = pin_value;
        if (sampled != scanner->last_bit) {
            scanner->bit_synced = 1;
        }
        scanner->last_bit = sampled;
        return sampled;
    }
    return -2;
}

void tsfi_sna_route_init(tsfi_sna_route *route, uint8_t vr, uint8_t er, int prio) {
    if (!route) return;
    route->virtual_route_id = vr;
    route->explicit_route_id = er;
    route->transmission_priority = prio;
}

int tsfi_sna_route_prioritize(const tsfi_sna_route *route, int load_factor) {
    if (!route) return -1;
    return route->transmission_priority * 10 - load_factor;
}
