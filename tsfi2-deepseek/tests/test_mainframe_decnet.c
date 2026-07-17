#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "tsfi_mainframe_decnet.h"

int main(void) {
    printf("=============================================================\n");
    printf("UNIT TESTS: DISTRIBUTED DECNET-OVER-SDLC NETWORKING\n");
    printf("=============================================================\n");

    // 1. Initialize Routers
    printf("[Test] Initializing Router Nodes...\n");
    tsfi_decnet_router router_a;
    tsfi_decnet_router router_b;
    tsfi_decnet_router router_c;
    
    tsfi_decnet_init(&router_a, 10);
    tsfi_decnet_init(&router_b, 20);
    tsfi_decnet_init(&router_c, 30);
    
    assert(router_a.local_node_id == 10);
    assert(router_b.local_node_id == 20);
    assert(router_c.local_node_id == 30);
    printf("  [PASS] Node initializations verified.\n");

    // 2. Setup SDLC Link Handshakes
    printf("[Test] Performing SDLC SNRM/UA Link Handshake...\n");
    tsfi_sdlc_link_state link_ab;
    tsfi_sdlc_link_state link_ba;
    tsfi_sdlc_link_init(&link_ab);
    tsfi_sdlc_link_init(&link_ba);
    
    assert(tsfi_sdlc_link_handshake(&link_ab, &link_ba, SDLC_CMD_SNRM) == SDLC_CMD_UA);
    assert(link_ab.link_established == 1);
    assert(link_ba.link_established == 1);
    printf("  [PASS] Connection handshake established successfully.\n");

    // 3. Multi-point SDLC Hub Routing
    printf("[Test] Connecting Nodes to Virtual Coaxial Hub...\n");
    tsfi_sdlc_hub hub;
    tsfi_sdlc_hub_init(&hub);
    assert(tsfi_sdlc_hub_connect(&hub, 0x11) == 0); // Node A
    assert(tsfi_sdlc_hub_connect(&hub, 0x22) == 0); // Node B
    assert(tsfi_sdlc_hub_connect(&hub, 0x33) == 0); // Node C
    printf("  [PASS] Hub attachments verified.\n");

    // 4. Name Resolution
    printf("[Test] Resolving VAX Node Names...\n");
    tsfi_decnet_directory dir;
    tsfi_decnet_dir_init(&dir);
    assert(tsfi_decnet_dir_add(&dir, "ALPHA_VAX", 10) == 0);
    assert(tsfi_decnet_dir_add(&dir, "BETA_VAX", 20) == 0);
    
    uint16_t resolved = 0;
    assert(tsfi_decnet_dir_resolve(&dir, "ALPHA_VAX", &resolved) == 0);
    assert(resolved == 10);
    printf("  [PASS] Directory resolution verified.\n");

    // 5. DECnet Dynamic Hello Discovery
    printf("[Test] Performing Keep-Alive Hello Discoveries...\n");
    // Connect A <-> B (cost 2)
    assert(tsfi_decnet_exchange_hello(&router_a, &router_b, 2) == 0);
    // Connect B <-> C (cost 4)
    assert(tsfi_decnet_exchange_hello(&router_b, &router_c, 4) == 0);
    
    uint16_t hop = 0;
    // Route from A to C: should go through B (hop 20)
    // First, propagate route updates from B to A
    assert(tsfi_decnet_update_route(&router_a, 30, 20, 6) == 0); // total cost 2 + 4 = 6
    assert(tsfi_decnet_route_packet(&router_a, 30, &hop) == 0);
    assert(hop == 20);
    printf("  [PASS] Dynamic cost-vector discovery and routing verified.\n");

    // 6. Packet Forwarding Header
    printf("[Test] Serializing Multi-hop Data Headers...\n");
    tsfi_decnet_header hdr;
    hdr.flags = 0x02;
    hdr.dst_node = 30;
    hdr.src_node = 10;
    hdr.forward_count = 0;
    
    uint8_t pkt_buf[32];
    size_t pkt_len = 0;
    assert(tsfi_decnet_serialize_header(&hdr, pkt_buf, &pkt_len) == 0);
    
    tsfi_decnet_header rx_hdr;
    assert(tsfi_decnet_deserialize_header(pkt_buf, pkt_len, &rx_hdr) == 0);
    assert(rx_hdr.dst_node == 30);
    assert(rx_hdr.src_node == 10);
    printf("  [PASS] Header codecs verified.\n");

    // 7. SDLC Bit Stuffing/Unstuffing
    printf("[Test] Verifying SDLC Bit Stuffing...\n");
    uint8_t raw_payload[] = { 0x3F, 0x00 }; // 0x3F has six consecutive ones (00111111)
    uint8_t stuffed[8];
    size_t stuffed_len = 0;
    assert(tsfi_sdlc_bit_stuff(raw_payload, sizeof(raw_payload), stuffed, &stuffed_len) == 0);
    
    uint8_t unstuffed[8];
    size_t unstuffed_len = 0;
    assert(tsfi_sdlc_bit_unstuff(stuffed, stuffed_len, unstuffed, &unstuffed_len) == 0);
    assert(unstuffed_len >= sizeof(raw_payload));
    assert(memcmp(raw_payload, unstuffed, sizeof(raw_payload)) == 0);
    printf("  [PASS] Bit stuffing zero insertion and extraction verified.\n");

    // 8. SDLC Abort frame detection
    printf("[Test] Verifying SDLC Abort Signal Detection...\n");
    uint8_t abort_stream[] = { 0xFE, 0x00 }; // 0xFE has seven consecutive ones (11111110)
    assert(tsfi_sdlc_detect_abort(abort_stream, 16) == 1);
    
    uint8_t safe_stream[] = { 0x7E, 0x00 }; // 0x7E has six ones (01111110)
    assert(tsfi_sdlc_detect_abort(safe_stream, 16) == 0);
    printf("  [PASS] Abort signaling frame detection verified.\n");

    // 9. DECnet Hop check (TTL loop protection)
    printf("[Test] Verifying DECnet Hop Limits...\n");
    tsfi_decnet_header hop_hdr;
    hop_hdr.forward_count = 14;
    assert(tsfi_decnet_hop_check(&hop_hdr) == 0);
    assert(hop_hdr.forward_count == 15);
    // Exceeding 15 triggers discard error (-2)
    assert(tsfi_decnet_hop_check(&hop_hdr) == -2);
    printf("  [PASS] Hop limit count loop discards verified.\n");

    // 10. DECnet NSP Segment Credit Flow Control
    printf("[Test] Verifying DECnet NSP Segment Flow Control...\n");
    tsfi_decnet_nsp_flow flow;
    tsfi_decnet_nsp_flow_init(&flow, 2);
    assert(flow.tx_credits == 2);
    assert(tsfi_decnet_nsp_flow_consume(&flow) == 0);
    assert(tsfi_decnet_nsp_flow_consume(&flow) == 0);
    // Out of credits
    assert(tsfi_decnet_nsp_flow_consume(&flow) == -2);
    tsfi_decnet_nsp_flow_grant(&flow, 5);
    assert(flow.tx_credits == 5);
    printf("  [PASS] NSP segment credit window flow control verified.\n");

    // 11. SDLC Secondary Station Multiplexing
    printf("[Test] Verifying SDLC Secondary Station Multiplexing...\n");
    tsfi_sdlc_multiplexer mux;
    tsfi_sdlc_mux_init(&mux);
    assert(tsfi_sdlc_mux_add(&mux, 0x12) == 0);
    assert(tsfi_sdlc_mux_add(&mux, 0x34) == 0);
    assert(tsfi_sdlc_mux_route(&mux, 0x12) == 0);
    assert(tsfi_sdlc_mux_route(&mux, 0x34) == 1);
    assert(tsfi_sdlc_mux_route(&mux, 0x99) == -2);
    printf("  [PASS] Station address multiplexing routes verified.\n");

    // 12. DECnet NSP Message Segmentation and Reassembly
    printf("[Test] Verifying DECnet NSP Message Reassembly...\n");
    tsfi_decnet_reassembler reasm;
    tsfi_decnet_reassembler_init(&reasm, 3);
    assert(tsfi_decnet_reassembler_add(&reasm, (uint8_t*)"PART_1_", 7) == 0);
    assert(tsfi_decnet_reassembler_add(&reasm, (uint8_t*)"PART_2_", 7) == 0);
    // Third segment completes reassembly
    assert(tsfi_decnet_reassembler_add(&reasm, (uint8_t*)"FINAL", 5) == 1);
    assert(reasm.len == 19);
    assert(memcmp(reasm.buffer, "PART_1_PART_2_FINAL", 19) == 0);
    printf("  [PASS] Message segment accumulation and reassembly verified.\n");

    // 13. NSP Interrupt Messages
    printf("[Test] Verifying NSP Interrupt Messages...\n");
    tsfi_decnet_nsp_link nsp_link;
    nsp_link.remote_link_id = 999;
    uint8_t int_pkt[128];
    size_t int_pkt_len = 0;
    assert(tsfi_decnet_nsp_send_interrupt(&nsp_link, (uint8_t*)"ALARM", 5, int_pkt, &int_pkt_len) == 0);
    assert(int_pkt[0] == NSP_MSG_INTERRUPT);
    assert(((int_pkt[1] << 8) | int_pkt[2]) == 999);
    assert(memcmp(int_pkt + 3, "ALARM", 5) == 0);
    printf("  [PASS] NSP high-priority interrupt paths verified.\n");

    // 14. NSP Out-of-Order Queue Reordering
    printf("[Test] Verifying NSP Out-of-Order Queue...\n");
    tsfi_nsp_reorder_queue reorder_q;
    tsfi_nsp_reorder_init(&reorder_q);
    
    // Add segment sequence 2 first, then 1 (out-of-order)
    assert(tsfi_nsp_reorder_add(&reorder_q, 2, (uint8_t*)"SECOND", 6) == 0);
    assert(tsfi_nsp_reorder_add(&reorder_q, 1, (uint8_t*)"FIRST", 5) == 0);
    
    // Queue should sort them: seq 1 first, then seq 2
    assert(reorder_q.items[0].seq_num == 1);
    assert(reorder_q.items[1].seq_num == 2);
    assert(memcmp(reorder_q.items[0].data, "FIRST", 5) == 0);
    assert(memcmp(reorder_q.items[1].data, "SECOND", 6) == 0);
    printf("  [PASS] NSP out-of-order frame resequencing verified.\n");

    // 15. NSP Link Probe Keep-Alives
    printf("[Test] Verifying NSP Link Probes...\n");
    tsfi_decnet_nsp_link probe_link;
    probe_link.state = 2; // Active
    
    // Elapsed ticks less than timeout
    assert(tsfi_decnet_nsp_probe(&probe_link, 5, 10) == 0);
    assert(probe_link.state == 2);
    
    // Elapsed ticks equal to timeout: connection resets to 0 (CLOSED)
    assert(tsfi_decnet_nsp_probe(&probe_link, 10, 10) == 1);
    assert(probe_link.state == 0);
    printf("  [PASS] NSP logical session keep-alive timeout verified.\n");

    // 16. NSP Control Message Codec Verification
    printf("[Test] Verifying NSP CI/CC Handshake Codecs...\n");
    tsfi_nsp_control_msg tx_msg;
    tx_msg.command = NSP_CMD_CI;
    tx_msg.src_link = 1001;
    tx_msg.dst_link = 2002;
    tx_msg.services = 0x05;
    
    uint8_t c_buf[32];
    size_t c_len = 0;
    assert(tsfi_nsp_serialize_control(&tx_msg, c_buf, &c_len) == 0);
    assert(c_len == 6);
    
    tsfi_nsp_control_msg rx_msg;
    assert(tsfi_nsp_deserialize_control(c_buf, c_len, &rx_msg) == 0);
    assert(rx_msg.command == NSP_CMD_CI);
    assert(rx_msg.src_link == 1001);
    assert(rx_msg.dst_link == 2002);
    assert(rx_msg.services == 0x05);
    printf("  [PASS] NSP Connect-Initiate codec formatting verified.\n");

    // 17. NSP Transport Retransmit Timer Verification
    printf("[Test] Verifying NSP Transport Retransmission Timer...\n");
    tsfi_nsp_retransmit_timer r_timer;
    tsfi_nsp_retransmit_start(&r_timer);
    assert(r_timer.active == 1);
    assert(r_timer.elapsed == 0);
    
    // Ticks below timeout: no action
    assert(tsfi_nsp_retransmit_tick(&r_timer, 200, 1000, 3) == 0);
    // Ticks matching/exceeding timeout: triggers retransmit retry (1)
    assert(tsfi_nsp_retransmit_tick(&r_timer, 900, 1000, 3) == 1);
    assert(r_timer.retries == 1);
    
    // Multiple timeouts exceed retry limit: drops connection (-2)
    assert(tsfi_nsp_retransmit_tick(&r_timer, 1050, 1000, 3) == 1); // retry 2
    assert(tsfi_nsp_retransmit_tick(&r_timer, 1050, 1000, 3) == 1); // retry 3
    assert(tsfi_nsp_retransmit_tick(&r_timer, 1050, 1000, 3) == -2); // exceeds max_retries = 3
    assert(r_timer.active == 0);
    printf("  [PASS] NSP segment retry limit disconnects verified.\n");

    // 18. NSP Subchannel ID Multiplexing Verification
    printf("[Test] Verifying NSP Subchannel ID Splits...\n");
    tsfi_nsp_subchannel sub;
    tsfi_nsp_subchannel_init(&sub, 555);
    assert(sub.normal_subchannel == 555);
    assert(sub.interrupt_subchannel == 1555);
    printf("  [PASS] NSP subchannel allocation multiplexing verified.\n");

    // 19. IBM 3705 Communications Controller Channel Handshake Verification
    printf("[Test] Verifying IBM 3705 Channel Handshake...\n");
    tsfi_ibm3705_ctc ctc;
    tsfi_ibm3705_init(&ctc);
    assert(ctc.interrupt_pending == 0);
    // Write CTC command: sets device busy status and asserts interrupt request
    assert(tsfi_ibm3705_write_cmd(&ctc, 0x01) == 0);
    assert(ctc.interrupt_pending == 1);
    assert((tsfi_ibm3705_read_status(&ctc) & 0x02) == 0x02);
    assert(ctc.interrupt_pending == 0); // Cleared on status read
    printf("  [PASS] IBM 3705 controller bus handshakes verified.\n");

    // 20. IBM 3270 Visual Terminal Cluster Controller Verification
    printf("[Test] Verifying IBM 3270 Display Screen Updates...\n");
    tsfi_ibm3270_terminal terminal;
    tsfi_ibm3270_init(&terminal);
    assert(terminal.screen_buffer[100] == ' ');
    // Write char to visual screen buffer
    assert(tsfi_ibm3270_write_char(&terminal, 'A', 100) == 0);
    assert(terminal.screen_buffer[100] == 'A');
    assert(terminal.cursor_position == 101);
    assert(terminal.buffer_updated == 1);
    printf("  [PASS] IBM 3270 screen buffer display maps verified.\n");

    // 21. SNA/VTAM Path Information Unit (PIU) Header Codec Verification
    printf("[Test] Verifying SNA/VTAM PIU Packet Serialization...\n");
    tsfi_vtam_piu_header piu_tx;
    piu_tx.th_fid = 0x02; // Format Identification 2
    piu_tx.th_daf = 0x1A;
    piu_tx.th_saf = 0x2B;
    piu_tx.th_snf = 12345;
    piu_tx.rh_type = 0x88;
    
    uint8_t piu_buf[256];
    size_t piu_len = 0;
    assert(tsfi_vtam_serialize_piu(&piu_tx, (uint8_t*)"SNA_DATA", 8, piu_buf, &piu_len) == 0);
    assert(piu_len == 14); // 6 header bytes + 8 payload bytes
    
    tsfi_vtam_piu_header piu_rx;
    uint8_t r_pay[128];
    size_t r_pay_len = 0;
    assert(tsfi_vtam_deserialize_piu(piu_buf, piu_len, &piu_rx, r_pay, &r_pay_len) == 0);
    assert(piu_rx.th_fid == 0x02);
    assert(piu_rx.th_daf == 0x1A);
    assert(piu_rx.th_saf == 0x2B);
    assert(piu_rx.th_snf == 12345);
    assert(piu_rx.rh_type == 0x88);
    assert(r_pay_len == 8);
    assert(memcmp(r_pay, "SNA_DATA", 8) == 0);
    printf("  [PASS] SNA/VTAM packet encapsulation formatting verified.\n");

    // 22. IBM SDLC Loop Mode Verification
    printf("[Test] Verifying IBM SDLC Loop Mode...\n");
    tsfi_sdlc_loop_station station;
    tsfi_sdlc_loop_init(&station, 0xC0);
    assert(station.has_token == 0);
    
    uint8_t loop_bits[] = { 0x7F }; // Token pattern
    assert(tsfi_sdlc_loop_forward(&station, loop_bits, 8) == 1);
    assert(station.has_token == 1);
    assert(loop_bits[0] == 0xFF); // Marked busy on token capture
    printf("  [PASS] SDLC loop active token capture verified.\n");

    // 23. IBM 3705 Scanner Bit-Sampling Verification
    printf("[Test] Verifying IBM 3705 Scanner Bit-Sampling...\n");
    tsfi_ibm3705_scanner scanner;
    tsfi_ibm3705_scanner_init(&scanner);
    assert(scanner.clock_phase == 0);
    
    // Sample pins over clock phases: clock phase 2 samples the value
    assert(tsfi_ibm3705_scanner_sample(&scanner, 1) == -2); // Phase 1
    assert(tsfi_ibm3705_scanner_sample(&scanner, 1) == 1);  // Phase 2: Samples
    assert(tsfi_ibm3705_scanner_sample(&scanner, 1) == -2); // Phase 3
    assert(tsfi_ibm3705_scanner_sample(&scanner, 1) == -2); // Phase 0
    printf("  [PASS] IBM 3705 scanner sampling synchronization verified.\n");

    // 24. SNA Virtual Route and Explicit Route Path engine Verification
    printf("[Test] Verifying SNA Route Prioritization...\n");
    tsfi_sna_route route;
    tsfi_sna_route_init(&route, 1, 2, 3); // VR=1, ER=2, priority=3
    assert(route.virtual_route_id == 1);
    assert(route.transmission_priority == 3);
    // Prioritization check: priority * 10 - load_factor = 3 * 10 - 5 = 25
    assert(tsfi_sna_route_prioritize(&route, 5) == 25);
    printf("  [PASS] SNA virtual route prioritization engines verified.\n");

    // 25. IBM 3705 EP/NCP Partition State Machine Verification
    printf("[Test] Verifying IBM 3705 NCP Partition Switching...\n");
    tsfi_ibm3705_ncp ncp;
    tsfi_ibm3705_ncp_init(&ncp);
    assert(ncp.active_partition == 0);
    ncp.partition_busy[2] = 1;
    // Switching to busy partition fails
    assert(tsfi_ibm3705_ncp_switch(&ncp, 2) == -2);
    // Switching to free partition succeeds
    assert(tsfi_ibm3705_ncp_switch(&ncp, 3) == 0);
    assert(ncp.active_partition == 3);
    printf("  [PASS] IBM 3705 EP/NCP partition states verified.\n");

    // 26. SNA Session BIND/SDT Handshake Verification
    printf("[Test] Verifying SNA Session Handshakes...\n");
    tsfi_sna_session sess;
    tsfi_sna_session_init(&sess);
    assert(sess.session_active == 0);
    // SDT before BIND fails
    assert(tsfi_sna_session_handshake(&sess, SNA_CMD_SDT) == -2);
    assert(tsfi_sna_session_handshake(&sess, SNA_CMD_BIND) == 0);
    assert(sess.session_active == 1);
    assert(tsfi_sna_session_handshake(&sess, SNA_CMD_SDT) == 0);
    assert(sess.traffic_started == 1);
    printf("  [PASS] SNA BIND and SDT session handshakes verified.\n");

    // 27. S/370 Channel Status & Sense Bytes Verification
    printf("[Test] Verifying S/370 Channel Registers...\n");
    tsfi_s370_channel_status chan;
    tsfi_s370_channel_status_init(&chan);
    assert(chan.status_byte == 0x00);
    assert(chan.sense_byte == 0x00);
    // Trigger I/O channel error (Sense byte 0x10 = Bus Out Check)
    tsfi_s370_channel_set_error(&chan, 0x10);
    assert(chan.status_byte == 0x01); // Unit Check
    assert(chan.sense_byte == 0x10);
    printf("  [PASS] S/370 channel adapter status and sense registers verified.\n");

    printf("[PASS] All distributed networking unit tests executed successfully!\n");
    printf("=============================================================\n");
    return 0;
}
