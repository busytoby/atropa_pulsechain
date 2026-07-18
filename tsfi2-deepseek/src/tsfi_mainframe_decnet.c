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
            if (cost < router->routing_table[i].cost || router->routing_table[i].next_hop == next_hop) {
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

int tsfi_usenet_article_to_piu(const tsfi_usenet_article *art, const tsfi_sna_th *th, const tsfi_sna_rh *rh, uint8_t *piu_out, size_t *piu_len) {
    if (!art || !th || !rh || !piu_out || !piu_len) return -1;
    uint8_t raw_buf[sizeof(tsfi_usenet_article)];
    size_t raw_len = 0;
    if (tsfi_usenet_store_bin(art, raw_buf, &raw_len) != 0) return -2;
    return tsfi_sna_package_piu(th, rh, raw_buf, raw_len, piu_out, piu_len);
}

int tsfi_usenet_piu_to_article(const uint8_t *piu, size_t piu_len, tsfi_sna_th *th_out, tsfi_sna_rh *rh_out, tsfi_usenet_article *art_out) {
    if (!piu || piu_len == 0 || !th_out || !rh_out || !art_out) return -1;
    uint8_t raw_buf[1024];
    size_t raw_len = 0;
    int parse_res = tsfi_sna_parse_piu(piu, piu_len, th_out, rh_out, raw_buf, &raw_len);
    if (parse_res != 0) return parse_res;
    return tsfi_usenet_retrieve_bin(raw_buf, raw_len, art_out);
}

int tsfi_usenet_match_subscription(const tsfi_usenet_subscription *subs, size_t count, const char *newsgroup, uint16_t *lu_addr_out) {
    if (!subs || !newsgroup || !lu_addr_out) return -1;
    for (size_t i = 0; i < count; i++) {
        if (subs[i].active && strcmp(subs[i].newsgroup, newsgroup) == 0) {
            *lu_addr_out = subs[i].lu_addr;
            return 0;
        }
    }
    return -2;
}

int tsfi_usenet_pacing_check(tsfi_usenet_pacing *pacing, uint32_t current_time) {
    if (!pacing) return -1;
    uint32_t elapsed = current_time - pacing->last_packet_time;
    if (pacing->interval_ms > 0) {
        int gained = elapsed / pacing->interval_ms;
        if (gained > 0) {
            pacing->current_tokens += gained;
            if (pacing->current_tokens > pacing->max_burst) {
                pacing->current_tokens = pacing->max_burst;
            }
            pacing->last_packet_time += gained * pacing->interval_ms;
        }
    }
    if (pacing->current_tokens <= 0) {
        return -2;
    }
    pacing->current_tokens--;
    return 0;
}

int tsfi_usenet_cdrm_replicate(tsfi_msnf_cdrm *cdrm, const tsfi_usenet_article *art, uint8_t *piu_out, size_t *piu_len) {
    if (!cdrm || !art || !piu_out || !piu_len) return -1;
    if (cdrm->session_state != 2) return -2;
    
    tsfi_sna_th th;
    th.fid_type = SNA_FID_TYPE4;
    th.mpf = 3;
    th.daf = cdrm->remote_domain_id;
    th.oaf = cdrm->local_domain_id;
    th.sn = art->article_number;
    
    tsfi_sna_rh rh;
    memset(&rh, 0, sizeof(rh));
    rh.begin_chain = 1;
    rh.end_chain = 1;
    
    return tsfi_usenet_article_to_piu(art, &th, &rh, piu_out, piu_len);
}

void tsfi_usenet_history_init(tsfi_usenet_history *history) {
    if (!history) return;
    memset(history->article_history, 0, sizeof(history->article_history));
    history->count = 0;
}

int tsfi_usenet_history_check_and_add(tsfi_usenet_history *history, uint32_t article_number) {
    if (!history) return -1;
    for (size_t i = 0; i < history->count; i++) {
        if (history->article_history[i] == article_number) {
            return -2;
        }
    }
    if (history->count < 256) {
        history->article_history[history->count++] = article_number;
    } else {
        for (int i = 1; i < 256; i++) {
            history->article_history[i - 1] = history->article_history[i];
        }
        history->article_history[255] = article_number;
    }
    return 0;
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















