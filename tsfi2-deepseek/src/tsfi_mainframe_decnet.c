#include "tsfi_mainframe_decnet.h"
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>

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

void tsfi_ibm3705_ncp_init(tsfi_ibm3705_ncp *ncp) {
    if (!ncp) return;
    ncp->active_partition = 0;
    for (int i = 0; i < 4; i++) {
        ncp->partition_busy[i] = 0;
    }
}

int tsfi_ibm3705_ncp_switch(tsfi_ibm3705_ncp *ncp, int partition_id) {
    if (!ncp || partition_id < 0 || partition_id >= 4) return -1;
    if (ncp->partition_busy[partition_id]) return -2;
    ncp->active_partition = partition_id;
    return 0;
}

void tsfi_sna_session_init(tsfi_sna_session *sess) {
    if (!sess) return;
    sess->session_active = 0;
    sess->traffic_started = 0;
}

int tsfi_sna_session_handshake(tsfi_sna_session *sess, uint8_t cmd) {
    if (!sess) return -1;
    if (cmd == SNA_CMD_BIND) {
        sess->session_active = 1;
        sess->traffic_started = 0;
        return 0;
    } else if (cmd == SNA_CMD_UNBIND) {
        sess->session_active = 0;
        sess->traffic_started = 0;
        return 0;
    } else if (cmd == SNA_CMD_SDT) {
        if (!sess->session_active) return -2;
        sess->traffic_started = 1;
        return 0;
    }
    return -3;
}

void tsfi_s370_channel_status_init(tsfi_s370_channel_status *chan) {
    if (!chan) return;
    chan->status_byte = 0x00;
    chan->sense_byte = 0x00;
}

void tsfi_s370_channel_set_error(tsfi_s370_channel_status *chan, uint8_t sense) {
    if (!chan) return;
    chan->status_byte = 0x01;
    chan->sense_byte = sense;
}

void tsfi_msnf_init(tsfi_msnf_cdrm *cdrm, uint16_t local_id) {
    if (!cdrm) return;
    cdrm->local_domain_id = local_id;
    cdrm->remote_domain_id = 0;
    cdrm->session_state = 0;
}

int tsfi_msnf_establish_session(tsfi_msnf_cdrm *cdrm, uint16_t remote_id) {
    if (!cdrm) return -1;
    cdrm->remote_domain_id = remote_id;
    cdrm->session_state = 2;
    return 0;
}

int tsfi_x25_encapsulate_sdlc(const uint8_t *sdlc_buf, size_t sdlc_len, uint8_t lci, tsfi_x25_packet *x25_out) {
    if (!sdlc_buf || !x25_out || sdlc_len > 256) return -1;
    x25_out->gfi_lci = 0x10 | (lci & 0x0F);
    x25_out->packet_type = 0x01;
    memcpy(x25_out->payload, sdlc_buf, sdlc_len);
    x25_out->payload_len = sdlc_len;
    return 0;
}

int tsfi_x25_decapsulate_sdlc(const tsfi_x25_packet *x25, uint8_t *sdlc_buf_out, size_t *sdlc_len_out) {
    if (!x25 || !sdlc_buf_out || !sdlc_len_out) return -1;
    memcpy(sdlc_buf_out, x25->payload, x25->payload_len);
    *sdlc_len_out = x25->payload_len;
    return 0;
}

void tsfi_ibm8100_init(tsfi_ibm8100_dpcx *node, uint16_t term_id) {
    if (!node) return;
    node->terminal_id = term_id;
    node->local_accumulator = 0;
    node->sync_pending = 0;
}

int tsfi_ibm8100_process_local(tsfi_ibm8100_dpcx *node, int val) {
    if (!node) return -1;
    node->local_accumulator += val;
    node->sync_pending = 1;
    return 0;
}

int tsfi_ibm8100_sync_host(tsfi_ibm8100_dpcx *node, uint8_t *sync_frame_out, size_t *len_out) {
    if (!node || !sync_frame_out || !len_out) return -1;
    if (!node->sync_pending) return -2;
    
    sync_frame_out[0] = 0x81;
    sync_frame_out[1] = (node->terminal_id >> 8) & 0xFF;
    sync_frame_out[2] = node->terminal_id & 0xFF;
    sync_frame_out[3] = (node->local_accumulator >> 24) & 0xFF;
    sync_frame_out[4] = (node->local_accumulator >> 16) & 0xFF;
    sync_frame_out[5] = (node->local_accumulator >> 8) & 0xFF;
    sync_frame_out[6] = node->local_accumulator & 0xFF;
    
    *len_out = 7;
    node->sync_pending = 0;
    return 0;
}

int tsfi_msnf_send_tree_op(tsfi_msnf_cdrm *cdrm, tsfi_tcp_connection *conn, const tsfi_23tree_msg *msg, uint8_t *pkt_out, size_t *len_out) {
    if (!cdrm || !conn || !msg || !pkt_out || !len_out) return -1;
    if (!conn->connected || cdrm->session_state != 2) return -2;
    
    pkt_out[0] = 0xCD;
    pkt_out[1] = msg->command_type & 0xFF;
    pkt_out[2] = (msg->key >> 24) & 0xFF;
    pkt_out[3] = (msg->key >> 16) & 0xFF;
    pkt_out[4] = (msg->key >> 8) & 0xFF;
    pkt_out[5] = msg->key & 0xFF;
    pkt_out[6] = (msg->value >> 24) & 0xFF;
    pkt_out[7] = (msg->value >> 16) & 0xFF;
    pkt_out[8] = (msg->value >> 8) & 0xFF;
    pkt_out[9] = msg->value & 0xFF;
    
    *len_out = 10;
    return 0;
}

int tsfi_msnf_recv_tree_op(tsfi_msnf_cdrm *cdrm, tsfi_tcp_connection *conn, const uint8_t *pkt, size_t len, tsfi_23tree_msg *msg_out) {
    if (!cdrm || !conn || !pkt || !msg_out || len < 10) return -1;
    if (pkt[0] != 0xCD) return -3;
    
    msg_out->command_type = pkt[1];
    msg_out->key = (pkt[2] << 24) | (pkt[3] << 16) | (pkt[4] << 8) | pkt[5];
    msg_out->value = (pkt[6] << 24) | (pkt[7] << 16) | (pkt[8] << 8) | pkt[9];
    return 0;
}

void tsfi_sna_crypto_init(tsfi_sna_crypto *crypto, const uint8_t *key) {
    if (!crypto) return;
    if (key) {
        memcpy(crypto->session_key, key, 8);
        crypto->encryption_enabled = 1;
    } else {
        memset(crypto->session_key, 0, 8);
        crypto->encryption_enabled = 0;
    }
}

int tsfi_sna_encrypt(tsfi_sna_crypto *crypto, const uint8_t *plain, size_t len, uint8_t *cipher) {
    if (!crypto || !plain || !cipher) return -1;
    if (!crypto->encryption_enabled) {
        memcpy(cipher, plain, len);
        return 0;
    }
    for (size_t i = 0; i < len; i++) {
        cipher[i] = plain[i] ^ crypto->session_key[i % 8];
    }
    return 0;
}

int tsfi_sna_decrypt(tsfi_sna_crypto *crypto, const uint8_t *cipher, size_t len, uint8_t *plain) {
    if (!crypto || !cipher || !plain) return -1;
    if (!crypto->encryption_enabled) {
        memcpy(plain, cipher, len);
        return 0;
    }
    for (size_t i = 0; i < len; i++) {
        plain[i] = cipher[i] ^ crypto->session_key[i % 8];
    }
    return 0;
}

void tsfi_vtam_nto_init(tsfi_vtam_nto *nto) {
    if (!nto) return;
    nto->active = 1;
}

int tsfi_vtam_nto_translate(tsfi_vtam_nto *nto, const char *tty_in, size_t len, uint8_t *lu_out, size_t *out_len) {
    if (!nto || !tty_in || !lu_out || !out_len) return -1;
    if (!nto->active) return -2;
    if (len > 250) return -3;
    
    lu_out[0] = 0x11;
    lu_out[1] = len & 0xFF;
    memcpy(lu_out + 2, tty_in, len);
    *out_len = 2 + len;
    return 0;
}

void tsfi_sna_pacing_init(tsfi_sna_pacing *pacing, int initial_window) {
    if (!pacing) return;
    pacing->window_size = initial_window;
    pacing->max_window = initial_window * 2;
    pacing->congestion_detected = 0;
}

int tsfi_sna_pacing_adjust(tsfi_sna_pacing *pacing, int congestion_flag) {
    if (!pacing) return -1;
    pacing->congestion_detected = congestion_flag;
    if (congestion_flag) {
        pacing->window_size /= 2;
        if (pacing->window_size < 1) pacing->window_size = 1;
    } else {
        pacing->window_size++;
        if (pacing->window_size > pacing->max_window) {
            pacing->window_size = pacing->max_window;
        }
    }
    return pacing->window_size;
}

void tsfi_vtam_lu_registry_init(tsfi_vtam_lu_registry *reg) {
    if (!reg) return;
    reg->count = 0;
    for (int i = 0; i < 16; i++) {
        reg->lus[i].active = 0;
        reg->lus[i].lu_address = 0;
        reg->lus[i].lu_type = 0;
        reg->lus[i].resource_name[0] = '\0';
    }
}

int tsfi_vtam_lu_registry_add(tsfi_vtam_lu_registry *reg, uint16_t addr, uint8_t type, const char *name) {
    if (!reg || !name) return -1;
    if (reg->count >= 16) return -2;
    for (int i = 0; i < reg->count; i++) {
        if (reg->lus[i].lu_address == addr) return -3;
    }
    tsfi_vtam_lu *lu = &reg->lus[reg->count];
    lu->lu_address = addr;
    lu->lu_type = type;
    lu->active = 1;
    strncpy(lu->resource_name, name, 31);
    lu->resource_name[31] = '\0';
    reg->count++;
    return 0;
}

int tsfi_vtam_lu_registry_route(tsfi_vtam_lu_registry *reg, uint16_t addr, uint8_t *data, size_t len) {
    if (!reg || !data) return -1;
    for (int i = 0; i < reg->count; i++) {
        if (reg->lus[i].lu_address == addr && reg->lus[i].active) {
            if (len > 0) {
                data[0] = reg->lus[i].lu_type;
            }
            return i;
        }
    }
    return -2;
}

int tsfi_vtam_lu_bridge_winchester(tsfi_vtam_lu_registry *reg, uint16_t addr, uint8_t *scsi_status, uint8_t *data_reg, uint8_t *keycode_reg) {
    if (!reg || !scsi_status || !data_reg || !keycode_reg) return -1;
    for (int i = 0; i < reg->count; i++) {
        if (reg->lus[i].lu_address == addr && reg->lus[i].active) {
            *scsi_status = 0x03;
            *data_reg = reg->lus[i].lu_type;
            *keycode_reg = 32;
            return i;
        }
    }
    return -2;
}

int tsfi_vtam_lu_bridge_coaxial(tsfi_vtam_lu_registry *reg, uint16_t addr, int *coax_phase, int *coax_signal) {
    if (!reg || !coax_phase || !coax_signal) return -1;
    for (int i = 0; i < reg->count; i++) {
        if (reg->lus[i].lu_address == addr && reg->lus[i].active) {
            *coax_phase = (int)(reg->lus[i].lu_address % 360);
            *coax_signal = (int)reg->lus[i].lu_type;
            return i;
        }
    }
    return -2;
}

void tsfi_nau_session_init(tsfi_nau_session *sess) {
    if (!sess) return;
    sess->sscp_id = 0;
    sess->pu_id = 0;
    sess->lu_id = 0;
    sess->session_active = 0;
}

int tsfi_nau_session_bind(tsfi_nau_session *sess, uint16_t sscp, uint16_t pu, uint16_t lu) {
    if (!sess) return -1;
    sess->sscp_id = sscp;
    sess->pu_id = pu;
    sess->lu_id = lu;
    sess->session_active = 1;
    return 0;
}

void tsfi_3705_tg_init(tsfi_3705_tg_reorder *tg) {
    if (!tg) return;
    tg->expected_tg_seq = 0;
    tg->received_tg_seq = 0;
    tg->out_of_sequence_count = 0;
}

int tsfi_3705_tg_process(tsfi_3705_tg_reorder *tg, uint16_t seq) {
    if (!tg) return -1;
    tg->received_tg_seq = seq;
    if (seq != tg->expected_tg_seq) {
        tg->out_of_sequence_count++;
        tg->expected_tg_seq = seq + 1;
        return 1;
    }
    tg->expected_tg_seq++;
    return 0;
}

void tsfi_sna_er_init(tsfi_sna_er_route *er, uint8_t route_num) {
    if (!er) return;
    er->route_number = route_num;
    er->active = 0;
}

int tsfi_sna_er_activate(tsfi_sna_er_route *er) {
    if (!er) return -1;
    er->active = 1;
    return 0;
}

int tsfi_sna_er_deactivate(tsfi_sna_er_route *er) {
    if (!er) return -1;
    er->active = 0;
    return 0;
}

void tsfi_mainframe_connection_status(const tsfi_msnf_cdrm *cdrm, const tsfi_nau_session *nau, const tsfi_sna_er_route *er, char *report_out, size_t max_len) {
    if (!report_out || max_len == 0) return;
    snprintf(report_out, max_len,
             "MAINFRAME CONNECTION STATUS\n"
             "---------------------------\n"
             "CDRM Session State: %s (Local: 0x%04X, Remote: 0x%04X)\n"
             "NAU Session State : %s (SSCP: 0x%04X, PU: 0x%04X, LU: 0x%04X)\n"
             "Explicit Route State: %s (Route Number: %d)\n",
             cdrm ? (cdrm->session_state == 2 ? "ACTIVE" : "INACTIVE") : "UNKNOWN",
             cdrm ? cdrm->local_domain_id : 0,
             cdrm ? cdrm->remote_domain_id : 0,
             nau ? (nau->session_active ? "ACTIVE" : "INACTIVE") : "UNKNOWN",
             nau ? nau->sscp_id : 0,
             nau ? nau->pu_id : 0,
             nau ? nau->lu_id : 0,
             er ? (er->active ? "ACTIVE" : "INACTIVE") : "UNKNOWN",
             er ? er->route_number : 0);
}

void tsfi_sna_stage_pacing_init(tsfi_sna_stage_pacing *pacing, int window) {
    if (!pacing) return;
    pacing->stage_window = window;
    pacing->credits_left = window;
}

int tsfi_sna_stage_pacing_consume(tsfi_sna_stage_pacing *pacing) {
    if (!pacing) return -1;
    if (pacing->credits_left <= 0) return -2;
    pacing->credits_left--;
    return 0;
}

void tsfi_sna_stage_pacing_response(tsfi_sna_stage_pacing *pacing) {
    if (!pacing) return;
    pacing->credits_left = pacing->stage_window;
}

int tsfi_sna_serialize_fmh(const tsfi_sna_fmh *fmh, uint8_t *buf, size_t *len_out) {
    if (!fmh || !buf || !len_out) return -1;
    buf[0] = fmh->fmh_type;
    buf[1] = fmh->fmh_len;
    buf[2] = (fmh->destination_id >> 8) & 0xFF;
    buf[3] = fmh->destination_id & 0xFF;
    *len_out = 4;
    return 0;
}

int tsfi_sna_deserialize_fmh(const uint8_t *buf, size_t len, tsfi_sna_fmh *fmh_out) {
    if (!buf || !fmh_out || len < 4) return -1;
    fmh_out->fmh_type = buf[0];
    fmh_out->fmh_len = buf[1];
    fmh_out->destination_id = (buf[2] << 8) | buf[3];
    return 0;
}

void tsfi_sscp_lu_init(tsfi_sscp_lu_session *sess) {
    if (!sess) return;
    sess->lu_active = 0;
}

int tsfi_sscp_lu_control(tsfi_sscp_lu_session *sess, uint8_t cmd) {
    if (!sess) return -1;
    if (cmd == SNA_CMD_ACTLU) {
        sess->lu_active = 1;
        return 0;
    } else if (cmd == SNA_CMD_DACTLU) {
        sess->lu_active = 0;
        return 0;
    }
    return -2;
}

void tsfi_sna_chain_init(tsfi_sna_chain_assembler *assembler) {
    if (!assembler) return;
    assembler->len = 0;
    assembler->assembly_complete = 0;
}

int tsfi_sna_chain_add(tsfi_sna_chain_assembler *assembler, uint8_t chain_indicator, const uint8_t *data, size_t len) {
    if (!assembler || !data) return -1;
    if (assembler->len + len > 1024) return -2;
    memcpy(assembler->buffer + assembler->len, data, len);
    assembler->len += len;
    if (chain_indicator == SNA_CHAIN_LIC || chain_indicator == SNA_CHAIN_OIC) {
        assembler->assembly_complete = 1;
        return 1;
    }
    assembler->assembly_complete = 0;
    return 0;
}

void tsfi_sna_bind_profile_init(tsfi_sna_bind_profile *profile) {
    if (!profile) return;
    profile->profile_id = 1;
    profile->pacing_in = 2;
    profile->pacing_out = 2;
    profile->duplex_mode = 0;
}

int tsfi_sna_bind_profile_negotiate(tsfi_sna_bind_profile *local, const tsfi_sna_bind_profile *requested) {
    if (!local || !requested) return -1;
    local->profile_id = requested->profile_id;
    if (requested->pacing_in < local->pacing_in) {
        local->pacing_in = requested->pacing_in;
    }
    if (requested->pacing_out < local->pacing_out) {
        local->pacing_out = requested->pacing_out;
    }
    local->duplex_mode = requested->duplex_mode;
    return 0;
}

void tsfi_sna_tg_failover_init(tsfi_sna_tg_failover *failover) {
    if (!failover) return;
    failover->active_links = 3;
    failover->backup_route_active = 0;
}

int tsfi_sna_tg_link_fail(tsfi_sna_tg_failover *failover, int link_id) {
    if (!failover) return -1;
    (void)link_id;
    if (failover->active_links > 0) {
        failover->active_links--;
    }
    if (failover->active_links == 0) {
        failover->backup_route_active = 1;
        return 1;
    }
    return 0;
}

void tsfi_sna_bracket_init(tsfi_sna_bracket *b) {
    if (!b) return;
    b->bracket_active = 0;
}

int tsfi_sna_bracket_process(tsfi_sna_bracket *b, int begin_bracket, int end_bracket) {
    if (!b) return -1;
    if (begin_bracket) {
        b->bracket_active = 1;
    }
    if (end_bracket) {
        b->bracket_active = 0;
    }
    return 0;
}

void tsfi_sna_hdx_init(tsfi_sna_hdx *hdx, int initial_turn) {
    if (!hdx) return;
    hdx->my_turn = initial_turn;
}

int tsfi_sna_hdx_process(tsfi_sna_hdx *hdx, int receive_cd, int send_cd) {
    if (!hdx) return -1;
    if (receive_cd) {
        hdx->my_turn = 1;
    }
    if (send_cd) {
        hdx->my_turn = 0;
    }
    return 0;
}

void tsfi_sna_response_init(tsfi_sna_response_tracker *tracker) {
    if (!tracker) return;
    tracker->dr1_requested = 0;
    tracker->dr2_requested = 0;
    tracker->exception_response_only = 0;
}

int tsfi_sna_response_parse(tsfi_sna_response_tracker *tracker, uint8_t rh_byte) {
    if (!tracker) return -1;
    tracker->dr1_requested = (rh_byte & 0x01) ? 1 : 0;
    tracker->dr2_requested = (rh_byte & 0x02) ? 1 : 0;
    tracker->exception_response_only = (rh_byte & 0x04) ? 1 : 0;
    return 0;
}

uint8_t tsfi_sna_piu_bcc(const uint8_t *data, size_t len) {
    if (!data) return 0;
    uint8_t bcc = 0;
    for (size_t i = 0; i < len; i++) {
        bcc ^= data[i];
    }
    return bcc;
}

int tsfi_sna_serialize_session_cmd(const tsfi_sna_session_cmd *cmd, uint8_t *buf, size_t *len_out) {
    if (!cmd || !buf || !len_out) return -1;
    buf[0] = cmd->command_code;
    buf[1] = (cmd->local_addr >> 8) & 0xFF;
    buf[2] = cmd->local_addr & 0xFF;
    buf[3] = (cmd->remote_addr >> 8) & 0xFF;
    buf[4] = cmd->remote_addr & 0xFF;
    *len_out = 5;
    return 0;
}

int tsfi_sna_deserialize_session_cmd(const uint8_t *buf, size_t len, tsfi_sna_session_cmd *cmd_out) {
    if (!buf || !cmd_out || len < 5) return -1;
    cmd_out->command_code = buf[0];
    cmd_out->local_addr = (buf[1] << 8) | buf[2];
    cmd_out->remote_addr = (buf[3] << 8) | buf[4];
    return 0;
}

void tsfi_sna_key_rotation_init(tsfi_sna_key_rotation *rot, const uint8_t *dist_key) {
    if (!rot || !dist_key) return;
    memcpy(rot->distribution_key, dist_key, 8);
    memset(rot->current_session_key, 0, 8);
}

int tsfi_sna_rotate_key(tsfi_sna_key_rotation *rot, const uint8_t *encrypted_new_key) {
    if (!rot || !encrypted_new_key) return -1;
    for (int i = 0; i < 8; i++) {
        rot->current_session_key[i] = encrypted_new_key[i] ^ rot->distribution_key[i];
    }
    return 0;
}

int tsfi_sna_serialize_th(const tsfi_sna_th *th, uint8_t *buf, size_t *len_out) {
    if (!th || !buf || !len_out) return -1;
    if (th->fid_type == SNA_FID_TYPE0 || th->fid_type == SNA_FID_TYPE1) {
        buf[0] = (th->fid_type << 4) | (th->mpf & 0x0F);
        buf[1] = 0;
        buf[2] = (th->daf >> 8) & 0xFF;
        buf[3] = th->daf & 0xFF;
        buf[4] = (th->oaf >> 8) & 0xFF;
        buf[5] = th->oaf & 0xFF;
        buf[6] = (th->sn >> 8) & 0xFF;
        buf[7] = th->sn & 0xFF;
        buf[8] = 0;
        buf[9] = 0;
        *len_out = 10;
    } else if (th->fid_type == SNA_FID_TYPE2) {
        buf[0] = (SNA_FID_TYPE2 << 4) | (th->mpf & 0x0F);
        buf[1] = 0;
        buf[2] = th->daf & 0xFF;
        buf[3] = th->oaf & 0xFF;
        buf[4] = (th->sn >> 8) & 0xFF;
        buf[5] = th->sn & 0xFF;
        *len_out = 6;
    } else if (th->fid_type == SNA_FID_TYPE3 || th->fid_type == SNA_FID_TYPEF) {
        buf[0] = (th->fid_type << 4) | (th->mpf & 0x0F);
        buf[1] = th->daf & 0xFF;
        *len_out = 2;
    } else {
        buf[0] = (SNA_FID_TYPE4 << 4) | (th->mpf & 0x0F);
        buf[1] = 0;
        buf[2] = (th->daf >> 8) & 0xFF;
        buf[3] = th->daf & 0xFF;
        buf[4] = (th->oaf >> 8) & 0xFF;
        buf[5] = th->oaf & 0xFF;
        buf[6] = (th->sn >> 8) & 0xFF;
        buf[7] = th->sn & 0xFF;
        *len_out = 8;
    }
    return 0;
}

int tsfi_sna_deserialize_th(const uint8_t *buf, size_t len, tsfi_sna_th *th_out) {
    if (!buf || !th_out || len < 2) return -1;
    th_out->fid_type = (buf[0] >> 4) & 0x0F;
    th_out->mpf = buf[0] & 0x0F;
    if (th_out->fid_type == SNA_FID_TYPE0 || th_out->fid_type == SNA_FID_TYPE1) {
        if (len < 10) return -2;
        th_out->daf = (buf[2] << 8) | buf[3];
        th_out->oaf = (buf[4] << 8) | buf[5];
        th_out->sn = (buf[6] << 8) | buf[7];
    } else if (th_out->fid_type == SNA_FID_TYPE2) {
        if (len < 6) return -3;
        th_out->daf = buf[2];
        th_out->oaf = buf[3];
        th_out->sn = (buf[4] << 8) | buf[5];
    } else if (th_out->fid_type == SNA_FID_TYPE3 || th_out->fid_type == SNA_FID_TYPEF) {
        th_out->daf = buf[1];
        th_out->oaf = 0;
        th_out->sn = 0;
    } else {
        if (len < 8) return -4;
        th_out->daf = (buf[2] << 8) | buf[3];
        th_out->oaf = (buf[4] << 8) | buf[5];
        th_out->sn = (buf[6] << 8) | buf[7];
    }
    return 0;
}

int tsfi_sna_serialize_rh(const tsfi_sna_rh *rh, uint8_t *buf, size_t *len_out) {
    if (!rh || !buf || !len_out) return -1;
    buf[0] = ((rh->ru_category & 0x03) << 6) |
             ((rh->is_response & 0x01) << 5) |
             ((rh->format_indicator & 0x01) << 4) |
             ((rh->sense_data_included & 0x01) << 3);
    buf[1] = ((rh->begin_chain & 0x01) << 7) |
             ((rh->end_chain & 0x01) << 6) |
             ((rh->dr1_indicator & 0x01) << 5) |
             ((rh->dr2_indicator & 0x01) << 4) |
             ((rh->exception_response & 0x01) << 3);
    buf[2] = ((rh->change_direction & 0x01) << 7) |
             ((rh->begin_bracket & 0x01) << 6) |
             ((rh->end_bracket & 0x01) << 5);
    *len_out = 3;
    return 0;
}

int tsfi_sna_deserialize_rh(const uint8_t *buf, size_t len, tsfi_sna_rh *rh_out) {
    if (!buf || !rh_out || len < 3) return -1;
    rh_out->ru_category = (buf[0] >> 6) & 0x03;
    rh_out->is_response = (buf[0] >> 5) & 0x01;
    rh_out->format_indicator = (buf[0] >> 4) & 0x01;
    rh_out->sense_data_included = (buf[0] >> 3) & 0x01;
    rh_out->begin_chain = (buf[1] >> 7) & 0x01;
    rh_out->end_chain = (buf[1] >> 6) & 0x01;
    rh_out->dr1_indicator = (buf[1] >> 5) & 0x01;
    rh_out->dr2_indicator = (buf[1] >> 4) & 0x01;
    rh_out->exception_response = (buf[1] >> 3) & 0x01;
    rh_out->change_direction = (buf[2] >> 7) & 0x01;
    rh_out->begin_bracket = (buf[2] >> 6) & 0x01;
    rh_out->end_bracket = (buf[2] >> 5) & 0x01;
    return 0;
}

int tsfi_sna_map_lu_type(uint8_t unified_type) {
    switch (unified_type) {
        case LU_TYPE_FILE:     return SNA_LU_TYPE1;
        case LU_TYPE_SOCKET:   return SNA_LU_TYPE62;
        case LU_TYPE_TERMINAL: return SNA_LU_TYPE2;
        case LU_TYPE_DISK:     return SNA_LU_TYPE0;
        default:               return SNA_LU_TYPE0;
    }
}

const char *tsfi_sna_resolve_sense(uint16_t sense_code) {
    switch (sense_code) {
        case SNA_SENSE_RESOURCE_UNAVAILABLE: return "Resource Not Available";
        case SNA_SENSE_SESSION_LIMIT:        return "Session Limit Exceeded";
        case SNA_SENSE_END_USER_UNAVAILABLE: return "End-User Not Available";
        case SNA_SENSE_PATH_OUT_OF_ORDER:    return "Path Out of Order";
        default:                             return "Unknown/General Protocol Error";
    }
}

int tsfi_sna_package_piu(const tsfi_sna_th *th, const tsfi_sna_rh *rh, const uint8_t *ru_data, size_t ru_len, uint8_t *piu_out, size_t *piu_len) {
    if (!th || !rh || !piu_out || !piu_len) return -1;
    size_t th_len = 0;
    if (tsfi_sna_serialize_th(th, piu_out, &th_len) != 0) return -2;
    size_t rh_len = 0;
    if (tsfi_sna_serialize_rh(rh, piu_out + th_len, &rh_len) != 0) return -3;
    if (ru_data && ru_len > 0) {
        memcpy(piu_out + th_len + rh_len, ru_data, ru_len);
    } else {
        ru_len = 0;
    }
    size_t payload_len = th_len + rh_len + ru_len;
    uint8_t bcc = tsfi_sna_piu_bcc(piu_out, payload_len);
    piu_out[payload_len] = bcc;
    *piu_len = payload_len + 1;
    return 0;
}

int tsfi_sna_parse_piu(const uint8_t *piu, size_t piu_len, tsfi_sna_th *th_out, tsfi_sna_rh *rh_out, uint8_t *ru_out, size_t *ru_len) {
    if (!piu || piu_len < 10 || !th_out || !rh_out || !ru_out || !ru_len) return -1;
    uint8_t rx_bcc = piu[piu_len - 1];
    if (tsfi_sna_piu_bcc(piu, piu_len - 1) != rx_bcc) return -2;
    if (tsfi_sna_deserialize_th(piu, piu_len - 1, th_out) != 0) return -3;
    size_t th_len = (th_out->fid_type == SNA_FID_TYPE2) ? 6 : 8;
    if (tsfi_sna_deserialize_rh(piu + th_len, piu_len - th_len - 1, rh_out) != 0) return -4;
    size_t rh_len = 3;
    *ru_len = piu_len - th_len - rh_len - 1;
    if (*ru_len > 0) {
        memcpy(ru_out, piu + th_len + rh_len, *ru_len);
    }
    return 0;
}

void tsfi_usenet_init(tsfi_usenet_article *art, const char *group, uint32_t art_num, const char *subject, const char *body) {
    if (!art) return;
    art->article_number = art_num;
    if (group) {
        strncpy(art->newsgroup, group, 63);
        art->newsgroup[63] = '\0';
    } else {
        art->newsgroup[0] = '\0';
    }
    if (subject) {
        strncpy(art->subject, subject, 63);
        art->subject[63] = '\0';
    } else {
        art->subject[0] = '\0';
    }
    if (body) {
        strncpy(art->body, body, 255);
        art->body[255] = '\0';
    } else {
        art->body[0] = '\0';
    }
}

int tsfi_usenet_store_bin(const tsfi_usenet_article *art, uint8_t *buf, size_t *len_out) {
    if (!art || !buf || !len_out) return -1;
    memcpy(buf, art, sizeof(tsfi_usenet_article));
    *len_out = sizeof(tsfi_usenet_article);
    return 0;
}

int tsfi_usenet_retrieve_bin(const uint8_t *buf, size_t len, tsfi_usenet_article *art_out) {
    if (!buf || !art_out || len < sizeof(tsfi_usenet_article)) return -1;
    memcpy(art_out, buf, sizeof(tsfi_usenet_article));
    return 0;
}

void tsfi_sna_map_th_mpf_to_rh_chain(uint8_t mpf, tsfi_sna_rh *rh) {
    if (!rh) return;
    switch (mpf & 0x03) {
        case 0x02:
            rh->begin_chain = 1;
            rh->end_chain = 0;
            break;
        case 0x00:
            rh->begin_chain = 0;
            rh->end_chain = 0;
            break;
        case 0x01:
            rh->begin_chain = 0;
            rh->end_chain = 1;
            break;
        case 0x03:
        default:
            rh->begin_chain = 1;
            rh->end_chain = 1;
            break;
    }
}

void tsfi_zvm_gcs_init(tsfi_zvm_gcs *gcs, int vmid, const char *seg_name) {
    if (!gcs) return;
    gcs->vmid = vmid;
    gcs->vtam_active = 0;
    if (seg_name) {
        strncpy(gcs->shared_segment_name, seg_name, 15);
        gcs->shared_segment_name[15] = '\0';
    } else {
        gcs->shared_segment_name[0] = '\0';
    }
}

void tsfi_zvm_gcs_set_vtam(tsfi_zvm_gcs *gcs, int active) {
    if (gcs) {
        gcs->vtam_active = active;
    }
}

void tsfi_zvm_pvm_init(tsfi_zvm_pvm *pvm) {
    if (!pvm) return;
    pvm->source_lu = 0;
    pvm->target_lu = 0;
    pvm->session_id = 0;
    pvm->is_active = 0;
}

int tsfi_zvm_pvm_route(tsfi_zvm_pvm *pvm, int src, int target) {
    if (!pvm) return -1;
    pvm->source_lu = src;
    pvm->target_lu = target;
    pvm->session_id = (src << 8) | (target & 0xFF);
    pvm->is_active = 1;
    return 0;
}

void tsfi_zvm_rscs_init(tsfi_zvm_rscs_spool *spool, const char *filename, int file_id, int lu_type, int size) {
    if (!spool) return;
    spool->file_id = file_id;
    spool->lu_type = lu_type;
    spool->size_bytes = size;
    if (filename) {
        strncpy(spool->spool_file_name, filename, 31);
        spool->spool_file_name[31] = '\0';
    } else {
        spool->spool_file_name[0] = '\0';
    }
}

void tsfi_zvm_vscs_init(tsfi_zvm_vscs *vscs) {
    if (!vscs) return;
    vscs->terminal_lu = 0;
    vscs->target_vmid = 0;
    vscs->is_attached = 0;
}

int tsfi_zvm_vscs_attach(tsfi_zvm_vscs *vscs, int term_lu, int vmid) {
    if (!vscs) return -1;
    vscs->terminal_lu = term_lu;
    vscs->target_vmid = vmid;
    vscs->is_attached = 1;
    return 0;
}

int tsfi_appc_allocate(tsfi_appc_conversation *conv, int local_lu, int partner_lu) {
    if (!conv) return -1;
    conv->conversation_id = (local_lu << 16) | (partner_lu & 0xFFFF);
    conv->state = 0;
    return 0;
}

int tsfi_appc_send_data(tsfi_appc_conversation *conv, const uint8_t *data, size_t len) {
    if (!conv || !data || len == 0) return -1;
    conv->state = 1;
    return 0;
}

int tsfi_appc_receive_data(tsfi_appc_conversation *conv, uint8_t *buf, size_t *len_out) {
    if (!conv || !buf || !len_out) return -1;
    conv->state = 2;
    *len_out = 0;
    return 0;
}

int tsfi_appc_deallocate(tsfi_appc_conversation *conv) {
    if (!conv) return -1;
    conv->state = 3;
    return 0;
}

int tsfi_3270_format_usenet_list(const tsfi_usenet_article *articles, size_t count, uint8_t *ebcdic_buf, size_t *len_out) {
    if (!articles || count == 0 || !ebcdic_buf || !len_out) return -1;
    ebcdic_buf[0] = 0x11;
    ebcdic_buf[1] = 0x40;
    ebcdic_buf[2] = 0x40;
    ebcdic_buf[3] = 0x1D;
    ebcdic_buf[4] = 0x00;
    size_t offset = 5;
    for (size_t i = 0; i < count && offset < 500; i++) {
        int written = snprintf((char *)(ebcdic_buf + offset), 512 - offset, "[#%u] %s: %s\n",
                               articles[i].article_number, articles[i].newsgroup, articles[i].subject);
        if (written > 0) {
            offset += written;
        }
    }
    *len_out = offset;
    return 0;
}

int tsfi_usenet_sign_article(const tsfi_usenet_article *art, const uint8_t *private_key, tsfi_usenet_signature *sig_out) {
    if (!art || !private_key || !sig_out) return -1;
    uint8_t hash = 0;
    const uint8_t *art_bytes = (const uint8_t *)art;
    for (size_t i = 0; i < sizeof(tsfi_usenet_article); i++) {
        hash ^= art_bytes[i];
    }
    for (int i = 0; i < 32; i++) {
        sig_out->r[i] = hash ^ private_key[i % 32] ^ i;
        sig_out->s[i] = hash ^ private_key[(i + 1) % 32] ^ (i * 2);
    }
    return 0;
}

int tsfi_usenet_verify_article(const tsfi_usenet_article *art, const uint8_t *public_key, const tsfi_usenet_signature *sig) {
    if (!art || !public_key || !sig) return -1;
    uint8_t hash = 0;
    const uint8_t *art_bytes = (const uint8_t *)art;
    for (size_t i = 0; i < sizeof(tsfi_usenet_article); i++) {
        hash ^= art_bytes[i];
    }
    for (int i = 0; i < 32; i++) {
        uint8_t expected_r = hash ^ (public_key[i % 32] ^ 0xAA) ^ i;
        if (sig->r[i] != expected_r) {
            return -2;
        }
    }
    return 0;
}

void tsfi_bna_node_init(tsfi_bna_node *node, int id, const char *host) {
    if (!node) return;
    node->node_id = id;
    node->bna_active = 1;
    if (host) {
        strncpy(node->hostname, host, 15);
        node->hostname[15] = '\0';
    } else {
        node->hostname[0] = '\0';
    }
}

void tsfi_ddp_bridge_init(tsfi_ddp_bridge *bridge) {
    if (!bridge) return;
    bridge->sna_lu_count = 0;
    bridge->decnet_node_count = 0;
    bridge->bna_node_count = 0;
    bridge->coaxial_carrier_frequency = 10.0f;
}

int tsfi_ddp_bridge_status(const tsfi_ddp_bridge *bridge, char *status_out, size_t max_len) {
    if (!bridge || !status_out || max_len == 0) return -1;
    snprintf(status_out, max_len, "DDP Bridge: SNA LUs=%d, DECnet Nodes=%d, BNA Nodes=%d, Coaxial Freq=%.1fMHz",
             bridge->sna_lu_count, bridge->decnet_node_count, bridge->bna_node_count,
             bridge->coaxial_carrier_frequency);
    return 0;
}

int tsfi_braille_translate(const char *ascii_in, uint8_t *braille_out, size_t *len_out) {
    if (!ascii_in || !braille_out || !len_out) return -1;
    size_t i = 0;
    for (; ascii_in[i] != '\0' && i < 256; i++) {
        char c = ascii_in[i];
        if (c >= 'A' && c <= 'Z') {
            braille_out[i] = (c - 'A' + 1) & 0x3F;
        } else if (c >= 'a' && c <= 'z') {
            braille_out[i] = (c - 'a' + 1) & 0x3F;
        } else if (c == ' ') {
            braille_out[i] = 0x00;
        } else {
            braille_out[i] = 0x2A;
        }
    }
    *len_out = i;
    return 0;
}

void tsfi_bubble_init(tsfi_bubble_memory *bm) {
    if (!bm) return;
    bm->active_track = 0;
    bm->write_latch = 0;
    memset(bm->bubble_data, 0, sizeof(bm->bubble_data));
}

int tsfi_bubble_read(tsfi_bubble_memory *bm, uint32_t track, uint8_t *val) {
    if (!bm || !val || track >= 256) return -1;
    bm->active_track = track;
    *val = bm->bubble_data[track];
    return 0;
}

int tsfi_bubble_write(tsfi_bubble_memory *bm, uint32_t track, uint8_t val) {
    if (!bm || track >= 256) return -1;
    bm->active_track = track;
    bm->bubble_data[track] = val;
    return 0;
}

void tsfi_adds_init(tsfi_adds_terminal *term) {
    if (!term) return;
    memset(term->screen_buffer, ' ', sizeof(term->screen_buffer));
    term->cursor_pos = 0;
}

int tsfi_adds_write_char(tsfi_adds_terminal *term, uint8_t ebcdic_char) {
    if (!term || term->cursor_pos >= 80 * 24) return -1;
    char ascii_val = ' ';
    if (ebcdic_char == 0x40) {
        ascii_val = ' ';
    } else if (ebcdic_char >= 0xC1 && ebcdic_char <= 0xC9) {
        ascii_val = 'A' + (ebcdic_char - 0xC1);
    } else {
        ascii_val = '?';
    }
    term->screen_buffer[term->cursor_pos++] = ascii_val;
    return 0;
}

void tsfi_plato_init(tsfi_cdc_plato *plato) {
    if (!plato) return;
    plato->terminal_connected = 1;
    plato->keystroke_count = 0;
}

int tsfi_plato_process_key(tsfi_cdc_plato *plato, uint8_t keycode) {
    if (!plato || !plato->terminal_connected) return -1;
    plato->keystroke_count++;
    return (keycode == 32) ? 1 : 0;
}

void tsfi_mis_budget_init(tsfi_mis_budget *mb, float cpu_r, float mem_r, float storage_r, float limit) {
    if (!mb) return;
    mb->cpu_rate = cpu_r;
    mb->memory_rate = mem_r;
    mb->storage_rate = storage_r;
    mb->budget_limit = limit;
    mb->current_cost = 0.0f;
}

float tsfi_mis_calculate_cost(tsfi_mis_budget *mb, float cpu_hours, float mem_gb_hours, float storage_gb) {
    if (!mb) return 0.0f;
    float cost = (cpu_hours * mb->cpu_rate) + (mem_gb_hours * mb->memory_rate) + (storage_gb * mb->storage_rate);
    mb->current_cost += cost;
    return cost;
}

int tsfi_mis_is_over_budget(const tsfi_mis_budget *mb) {
    if (!mb) return 0;
    return (mb->current_cost > mb->budget_limit) ? 1 : 0;
}

void tsfi_sdc_init(tsfi_sdc_crypto *sdc, uint32_t mask, const uint8_t *m_key) {
    if (!sdc) return;
    sdc->auth_mask = mask;
    if (m_key) {
        memcpy(sdc->master_key, m_key, 16);
    } else {
        memset(sdc->master_key, 0, 16);
    }
}

int tsfi_sdc_validate_record(const tsfi_sdc_crypto *sdc, const uint8_t *record_data, size_t len, uint32_t signature) {
    if (!sdc || !record_data || len == 0) return -1;
    uint32_t checksum = sdc->auth_mask;
    for (size_t i = 0; i < len; i++) {
        checksum ^= record_data[i];
        checksum ^= sdc->master_key[i % 16];
    }
    return (checksum == signature) ? 0 : -2;
}

int tsfi_sdc_acm_authorize(const tsfi_sdc_acm *user_acm, int record_classification, int request_write) {
    if (!user_acm) return -1;
    if (user_acm->clearance_level < record_classification) {
        return -2;
    }
    if (request_write && !user_acm->write_privilege) {
        return -3;
    }
    return 0;
}

void tsfi_sdc_handshake_init(tsfi_sdc_handshake *hs, uint32_t secret) {
    if (!hs) return;
    hs->private_secret = secret;
    hs->derived_session_key = 0;
}

uint32_t tsfi_sdc_handshake_exchange(tsfi_sdc_handshake *hs, uint32_t foreign_derived) {
    if (!hs) return 0;
    uint64_t session = ((uint64_t)hs->private_secret * foreign_derived) % 953467954114363ULL;
    hs->derived_session_key = (uint32_t)(session & 0xFFFFFFFF);
    return hs->derived_session_key;
}

int tsfi_sdc_lisp_filter(const char *query_expr, const char *record_key, const char *record_val) {
    if (!query_expr || !record_key || !record_val) return -1;
    if (strstr(query_expr, "EQUAL") != NULL) {
        if (strstr(query_expr, record_key) != NULL && strstr(query_expr, record_val) != NULL) {
            return 1;
        }
    }
    return 0;
}

int tsfi_s240_audit(const char *user, int action_type, const char *resource, char *log_out, size_t max_len) {
    if (!user || !resource || !log_out || max_len == 0) return -1;
    const char *action_str = "UNKNOWN";
    if (action_type == S240_ACCESS) action_str = "UNAUTHORIZED_ACCESS";
    else if (action_type == S240_MODIFY) action_str = "UNAUTHORIZED_MODIFICATION";
    else if (action_type == S240_DESTROY) action_str = "UNAUTHORIZED_DESTRUCTION";
    
    snprintf(log_out, max_len, "S240_AUDIT: User=%s, Action=%s, Resource=%s", user, action_str, resource);
    return 0;
}

int tsfi_cpa_verify(const tsfi_cpa_audit *audit, float allowed_variance) {
    if (!audit) return -1;
    float diff = audit->total_ledger_sum - audit->total_rendered_sum;
    if (diff < 0.0f) diff = -diff;
    return (diff <= allowed_variance) ? 0 : -2;
}

int tsfi_apollo_serialize(const tsfi_apollo_frame *frame, uint8_t *buf, size_t *len_out) {
    if (!frame || !buf || !len_out) return -1;
    buf[0] = (frame->ring_id >> 8) & 0xFF;
    buf[1] = frame->ring_id & 0xFF;
    buf[2] = frame->source_node;
    buf[3] = frame->dest_node;
    buf[4] = frame->control_token;
    *len_out = 5;
    return 0;
}

int tsfi_apollo_deserialize(const uint8_t *buf, size_t len, tsfi_apollo_frame *frame_out) {
    if (!buf || !frame_out || len < 5) return -1;
    frame_out->ring_id = (buf[0] << 8) | buf[1];
    frame_out->source_node = buf[2];
    frame_out->dest_node = buf[3];
    frame_out->control_token = buf[4];
    return 0;
}

int tsfi_apollo_control_synth_bird_call(const tsfi_apollo_frame *frame, float *frequency_sweep_out, size_t *sweep_points_out) {
    if (!frame || !frequency_sweep_out || !sweep_points_out) return -1;
    size_t points = 100;
    float start_freq = 2000.0f + (frame->source_node * 10.0f);
    float end_freq = 6000.0f + (frame->dest_node * 20.0f);
    for (size_t i = 0; i < points; i++) {
        float t = (float)i / (float)(points - 1);
        frequency_sweep_out[i] = start_freq + (end_freq - start_freq) * t;
    }
    *sweep_points_out = points;
    return 0;
}

int tsfi_apollo_spool_phonemes(const tsfi_apollo_frame *frame, const char *text, tsfi_apollo_phoneme *phonemes_out, size_t *count_out) {
    if (!frame || !text || !phonemes_out || !count_out) return -1;
    size_t i = 0;
    for (; text[i] != '\0' && i < 128; i++) {
        phonemes_out[i].phoneme_char = text[i];
        phonemes_out[i].pitch_frequency = 300.0f + (frame->ring_id * 2.0f) + (text[i] * 3.0f);
        phonemes_out[i].amplitude = 0.8f;
    }
    *count_out = i;
    return 0;
}

int tsfi_apollo_render_soundscape(const tsfi_apollo_soundscape_node *nodes, size_t node_count, float *mixed_signal_out, size_t points) {
    if (!nodes || node_count == 0 || !mixed_signal_out || points == 0) return -1;
    memset(mixed_signal_out, 0, points * sizeof(float));
    for (size_t i = 0; i < points; i++) {
        float sample = 0.0f;
        for (size_t n = 0; n < node_count; n++) {
            float dist = sqrtf(nodes[n].x_pos * nodes[n].x_pos + nodes[n].y_pos * nodes[n].y_pos) + 1.0f;
            float attenuation = 1.0f / dist;
            sample += sinf((float)i * 0.1f * (float)(nodes[n].node_id % 10)) * nodes[n].volume_level * attenuation;
        }
        mixed_signal_out[i] = sample / (float)node_count;
    }
    return 0;
}

int tsfi_apollo_dfs_resolve(const char *global_path, char *resolved_node_out, char *local_path_out, size_t max_len) {
    if (!global_path || !resolved_node_out || !local_path_out || max_len == 0) return -1;
    if (global_path[0] == '/' && global_path[1] == '/') {
        const char *node_start = global_path + 2;
        const char *slash = strchr(node_start, '/');
        if (slash) {
            size_t node_len = slash - node_start;
            if (node_len >= max_len) node_len = max_len - 1;
            memcpy(resolved_node_out, node_start, node_len);
            resolved_node_out[node_len] = '\0';
            strncpy(local_path_out, slash, max_len - 1);
            local_path_out[max_len - 1] = '\0';
            return 0;
        }
    }
    return -2;
}

void tsfi_apollo_dm_init(tsfi_apollo_dm_pad *pad, int id, int w, int h, const char *stream) {
    if (!pad) return;
    pad->pad_id = id;
    pad->width = w;
    pad->height = h;
    if (stream) {
        strncpy(pad->stream_association, stream, 31);
        pad->stream_association[31] = '\0';
    } else {
        pad->stream_association[0] = '\0';
    }
}

void tsfi_aegis_pcb_init(tsfi_aegis_pcb *pcb, int pid, int priority, const char *name) {
    if (!pcb) return;
    pcb->process_id = pid;
    pcb->priority = priority;
    if (name) {
        strncpy(pcb->process_name, name, 15);
        pcb->process_name[15] = '\0';
    } else {
        pcb->process_name[0] = '\0';
    }
}

void tsfi_apollo_ring_reg_init(tsfi_apollo_ring_register *reg) {
    if (!reg) return;
    reg->node_bypass = 0;
    reg->token_held = 0;
    reg->packets_transmitted = 0;
}

uint8_t tsfi_secded_encode(uint8_t data) {
    uint8_t d0 = (data >> 0) & 1;
    uint8_t d1 = (data >> 1) & 1;
    uint8_t d2 = (data >> 2) & 1;
    uint8_t d3 = (data >> 3) & 1;
    uint8_t d4 = (data >> 4) & 1;
    uint8_t d5 = (data >> 5) & 1;
    uint8_t d6 = (data >> 6) & 1;
    uint8_t d7 = (data >> 7) & 1;

    uint8_t p1 = d0 ^ d1 ^ d3 ^ d4 ^ d6;
    uint8_t p2 = d0 ^ d2 ^ d3 ^ d5 ^ d6;
    uint8_t p3 = d1 ^ d2 ^ d3 ^ d7;
    uint8_t p4 = d4 ^ d5 ^ d6 ^ d7;

    return (p1 << 0) | (p2 << 1) | (p3 << 2) | (p4 << 3);
}

int tsfi_secded_decode(uint8_t data, uint8_t parity, uint8_t *corrected_data_out) {
    if (!corrected_data_out) return -1;
    uint8_t expected = tsfi_secded_encode(data);
    uint8_t syndrome = expected ^ parity;
    if (syndrome == 0) {
        *corrected_data_out = data;
        return 0;
    }
    if (syndrome == 0x03) {
        *corrected_data_out = data ^ (1 << 0);
        return 1;
    }
    *corrected_data_out = data;
    return -2;
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

int tsfi_cyclades_serialize(const tsfi_cyclades_header *hdr, uint8_t *buf, size_t *len_out) {
    if (!hdr || !buf || !len_out) return -1;
    buf[0] = hdr->src_node;
    buf[1] = hdr->dest_node;
    buf[2] = (hdr->seq_num >> 8) & 0xFF;
    buf[3] = hdr->seq_num & 0xFF;
    buf[4] = hdr->flags;
    *len_out = 5;
    return 0;
}

int tsfi_cyclades_deserialize(const uint8_t *buf, size_t len, tsfi_cyclades_header *hdr_out) {
    if (!buf || !hdr_out || len < 5) return -1;
    hdr_out->src_node = buf[0];
    hdr_out->dest_node = buf[1];
    hdr_out->seq_num = (buf[2] << 8) | buf[3];
    hdr_out->flags = buf[4];
    return 0;
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

void tsfi_cyclades_conn_init(tsfi_cyclades_connection *conn) {
    if (!conn) return;
    conn->state = CYCLADES_STATE_CLOSED;
    conn->local_seq = 100;
    conn->remote_seq = 0;
}

int tsfi_cyclades_process_packet(tsfi_cyclades_connection *conn, const tsfi_cyclades_header *packet_in, tsfi_cyclades_header *packet_out) {
    if (!conn || !packet_in || !packet_out) return -1;
    if (conn->state == CYCLADES_STATE_CLOSED && (packet_in->flags == 0x02)) {
        conn->state = CYCLADES_STATE_SYN_RCVD;
        conn->remote_seq = packet_in->seq_num;
        packet_out->flags = 0x12;
        packet_out->seq_num = conn->local_seq;
        packet_out->src_node = packet_in->dest_node;
        packet_out->dest_node = packet_in->src_node;
        return 0;
    } else if (conn->state == CYCLADES_STATE_SYN_RCVD && (packet_in->flags == 0x10)) {
        conn->state = CYCLADES_STATE_ESTABLISHED;
        conn->local_seq++;
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

int tsfi_cyclades_window_verify(const tsfi_cyclades_window *win, uint16_t seq) {
    if (!win) return -1;
    uint16_t offset = seq - win->window_start;
    return (offset < win->window_size) ? 0 : -2;
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
