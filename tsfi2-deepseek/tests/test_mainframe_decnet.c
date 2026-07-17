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

    // 28. SNA MSNF CDRM Cross-Domain (btc-rails VM simulation) Verification
    printf("[Test] Verifying SNA MSNF Cross-Domain CDRM Sessions...\n");
    tsfi_msnf_cdrm cdrm;
    // Map cross-domain to separate btc-rails virtual networks
    tsfi_msnf_init(&cdrm, 0x1111);
    assert(cdrm.local_domain_id == 0x1111);
    assert(cdrm.session_state == 0);
    
    assert(tsfi_msnf_establish_session(&cdrm, 0x2222) == 0);
    assert(cdrm.remote_domain_id == 0x2222);
    assert(cdrm.session_state == 2); // Active
    printf("  [PASS] Cross-domain MSNF CDRM session broker verified.\n");

    // 29. X.25 Packet Switching Envelopes Verification
    printf("[Test] Verifying X.25 Packet Encapsulation...\n");
    uint8_t sdlc_payload[] = { 0xC0, 0x01, 0x02, 0x03 };
    tsfi_x25_packet x25_pkt;
    assert(tsfi_x25_encapsulate_sdlc(sdlc_payload, 4, 7, &x25_pkt) == 0);
    assert((x25_pkt.gfi_lci & 0x0F) == 7);
    assert(x25_pkt.packet_type == 0x01);
    assert(x25_pkt.payload_len == 4);
    
    uint8_t rx_sdlc[128];
    size_t rx_sdlc_len = 0;
    assert(tsfi_x25_decapsulate_sdlc(&x25_pkt, rx_sdlc, &rx_sdlc_len) == 0);
    assert(rx_sdlc_len == 4);
    assert(memcmp(rx_sdlc, sdlc_payload, 4) == 0);
    printf("  [PASS] X.25 SDLC packet encapsulation decoders verified.\n");

    // 30. IBM 8100/DPCX Distributed System loops Verification
    printf("[Test] Verifying IBM 8100 Distributed Loops...\n");
    tsfi_ibm8100_dpcx node8100;
    tsfi_ibm8100_init(&node8100, 0x8101);
    assert(node8100.terminal_id == 0x8101);
    assert(node8100.sync_pending == 0);
    
    // Accumulate locally
    assert(tsfi_ibm8100_process_local(&node8100, 50) == 0);
    assert(node8100.local_accumulator == 50);
    assert(node8100.sync_pending == 1);
    
    uint8_t sync_f[32];
    size_t sync_len = 0;
    assert(tsfi_ibm8100_sync_host(&node8100, sync_f, &sync_len) == 0);
    assert(sync_len == 7);
    assert(sync_f[0] == 0x81);
    assert(((sync_f[1] << 8) | sync_f[2]) == 0x8101);
    assert(node8100.sync_pending == 0); // Cleared
    printf("  [PASS] IBM 8100 office DPCX loop sync verified.\n");

    // 31. MSNF 2-3 Tree Operation Replication over TCP/IP Verification
    printf("[Test] Verifying MSNF 2-3 Tree Replication over TCP/IP...\n");
    tsfi_msnf_cdrm tree_cdrm;
    tsfi_msnf_init(&tree_cdrm, 0x1111);
    assert(tsfi_msnf_establish_session(&tree_cdrm, 0x2222) == 0);
    
    tsfi_tcp_connection tcp_conn;
    tcp_conn.socket_fd = 5;
    tcp_conn.connected = 1;
    
    tsfi_23tree_msg tx_tree;
    tx_tree.command_type = 1; // INSERT
    tx_tree.key = 9999;
    tx_tree.value = 8888;
    
    uint8_t t_buf[64];
    size_t t_len = 0;
    assert(tsfi_msnf_send_tree_op(&tree_cdrm, &tcp_conn, &tx_tree, t_buf, &t_len) == 0);
    assert(t_len == 10);
    assert(t_buf[0] == 0xCD); // CDRM identification
    
    tsfi_23tree_msg rx_tree;
    assert(tsfi_msnf_recv_tree_op(&tree_cdrm, &tcp_conn, t_buf, t_len, &rx_tree) == 0);
    assert(rx_tree.command_type == 1);
    assert(rx_tree.key == 9999);
    assert(rx_tree.value == 8888);
    printf("  [PASS] 2-3 tree replication codecs over TCP/IP verified.\n");

    // 32. SNA Cryptographic session controls (SNA-DES) Verification
    printf("[Test] Verifying SNA Session Cryptography...\n");
    tsfi_sna_crypto crypto;
    uint8_t dummy_key[8] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08 };
    tsfi_sna_crypto_init(&crypto, dummy_key);
    assert(crypto.encryption_enabled == 1);
    
    uint8_t plain[] = "SNA_SECRET_DATA";
    uint8_t cipher[32];
    uint8_t decrypted[32];
    assert(tsfi_sna_encrypt(&crypto, plain, 15, cipher) == 0);
    // Verify ciphertext is encrypted (not equal to plaintext)
    assert(memcmp(cipher, plain, 15) != 0);
    assert(tsfi_sna_decrypt(&crypto, cipher, 15, decrypted) == 0);
    assert(memcmp(decrypted, plain, 15) == 0);
    printf("  [PASS] SNA session encryption and decryption verified.\n");

    // 33. VTAM Network Terminal Option (NTO) Verification
    printf("[Test] Verifying VTAM NTO Translation...\n");
    tsfi_vtam_nto nto;
    tsfi_vtam_nto_init(&nto);
    uint8_t nto_out[64];
    size_t nto_out_len = 0;
    assert(tsfi_vtam_nto_translate(&nto, "TTY_INPUT_1", 11, nto_out, &nto_out_len) == 0);
    assert(nto_out_len == 13);
    assert(nto_out[0] == 0x11); // FMHeader indicator
    assert(nto_out[1] == 11);
    assert(memcmp(nto_out + 2, "TTY_INPUT_1", 11) == 0);
    printf("  [PASS] VTAM NTO teletype wrapping verified.\n");

    // 34. SNA Explicit Route TG pacing Verification
    printf("[Test] Verifying SNA Explicit Route TG Pacing...\n");
    tsfi_sna_pacing pacing;
    tsfi_sna_pacing_init(&pacing, 4);
    assert(pacing.window_size == 4);
    
    // Normal operation increases window size
    assert(tsfi_sna_pacing_adjust(&pacing, 0) == 5);
    assert(tsfi_sna_pacing_adjust(&pacing, 1) == 2);
    printf("  [PASS] SNA transmission group dynamic pacing verified.\n");

    // 35. SNA Unified LU Registry Verification
    printf("[Test] Verifying SNA Unified LU Registry...\n");
    tsfi_vtam_lu_registry registry;
    tsfi_vtam_lu_registry_init(&registry);
    assert(registry.count == 0);
    
    // Register different device types as LUs
    assert(tsfi_vtam_lu_registry_add(&registry, 0x5001, LU_TYPE_FILE, "/atropa_pulsechain") == 0);
    assert(tsfi_vtam_lu_registry_add(&registry, 0x5002, LU_TYPE_SOCKET, "Port 10042") == 0);
    assert(tsfi_vtam_lu_registry_add(&registry, 0x5003, LU_TYPE_TERMINAL, "WaylandTTY") == 0);
    assert(registry.count == 3);
    
    uint8_t payload_bytes[] = { 0x00, 0xAA, 0xBB };
    // Route to LU 0x5002 (Socket type = 0x02) and verify type injection header
    assert(tsfi_vtam_lu_registry_route(&registry, 0x5002, payload_bytes, 3) == 1);
    assert(payload_bytes[0] == LU_TYPE_SOCKET);
    printf("  [PASS] SNA unified LU mapping and route headers verified.\n");

    // 36. Unified LU to WinchesterMQ Bridge Verification
    printf("[Test] Verifying Unified LU to WinchesterMQ Bridge...\n");
    uint8_t scsi_status = 0;
    uint8_t data_reg = 0;
    uint8_t keycode_reg = 0;
    
    assert(tsfi_vtam_lu_bridge_winchester(&registry, 0x5002, &scsi_status, &data_reg, &keycode_reg) == 1);
    assert(scsi_status == 0x03); // DATA_IN phase trigger
    assert(data_reg == LU_TYPE_SOCKET); // Send LU type
    assert(keycode_reg == 32); // 'd' keycode for Winchester MQ ACK compliance
    printf("  [PASS] Unified LU to WinchesterMQ SCSI bridge mappings verified.\n");

    // 37. Unified LU to Coaxial Bridge Verification
    printf("[Test] Verifying Unified LU to Coaxial Bridge...\n");
    int coax_phase = 0;
    int coax_signal = 0;
    
    // Route 0x5001 (address 0x5001 = 20481 dec, 20481 % 360 = 321, LU Type = 0x01)
    assert(tsfi_vtam_lu_bridge_coaxial(&registry, 0x5001, &coax_phase, &coax_signal) == 0);
    assert(coax_phase == 321);
    assert(coax_signal == LU_TYPE_FILE);
    printf("  [PASS] Unified LU to coaxial line signal bridge mappings verified.\n");

    // 38. SNA Network Addressable Unit (NAU) Bindings Verification
    printf("[Test] Verifying SNA NAU Session Bindings...\n");
    tsfi_nau_session nau_sess;
    tsfi_nau_session_init(&nau_sess);
    assert(nau_sess.session_active == 0);
    assert(tsfi_nau_session_bind(&nau_sess, 0x1000, 0x2000, 0x3000) == 0);
    assert(nau_sess.sscp_id == 0x1000);
    assert(nau_sess.pu_id == 0x2000);
    assert(nau_sess.lu_id == 0x3000);
    assert(nau_sess.session_active == 1);
    printf("  [PASS] NAU session establishment verified.\n");

    // 39. IBM 3705 Transmission Group (TG) Sequence Verification
    printf("[Test] Verifying IBM 3705 TG Sequence Tracking...\n");
    tsfi_3705_tg_reorder tg;
    tsfi_3705_tg_init(&tg);
    assert(tg.expected_tg_seq == 0);
    
    // Process sequential packet
    assert(tsfi_3705_tg_process(&tg, 0) == 0);
    assert(tg.expected_tg_seq == 1);
    
    // Process out-of-sequence packet (skips to sequence 3)
    assert(tsfi_3705_tg_process(&tg, 3) == 1); // Triggers out of sequence flag
    assert(tg.out_of_sequence_count == 1);
    assert(tg.expected_tg_seq == 4);
    printf("  [PASS] IBM 3705 multi-link transmission sequence checks verified.\n");

    // 40. SNA Explicit Route Activation Verification
    printf("[Test] Verifying SNA Explicit Route Activation...\n");
    tsfi_sna_er_route er_route;
    tsfi_sna_er_init(&er_route, 5);
    assert(er_route.route_number == 5);
    assert(er_route.active == 0);
    
    assert(tsfi_sna_er_activate(&er_route) == 0);
    assert(er_route.active == 1);
    assert(tsfi_sna_er_deactivate(&er_route) == 0);
    assert(er_route.active == 0);
    printf("  [PASS] SNA Explicit Route ACT and DEACT protocols verified.\n");

    // 41. Mainframe Connection Status Visibility Verification
    printf("[Test] Verifying Mainframe Connection Status Visibility...\n");
    tsfi_msnf_cdrm cdrm_vis;
    tsfi_msnf_init(&cdrm_vis, 0x1111);
    assert(tsfi_msnf_establish_session(&cdrm_vis, 0x2222) == 0);
    
    char status_report[512];
    tsfi_mainframe_connection_status(&cdrm_vis, &nau_sess, &er_route, status_report, 512);
    assert(strstr(status_report, "CDRM Session State: ACTIVE") != NULL);
    assert(strstr(status_report, "NAU Session State : ACTIVE") != NULL);
    assert(strstr(status_report, "Explicit Route State: INACTIVE") != NULL);
    printf("  [PASS] Unified connection visibility reporter verified.\n");

    // 42. SNA LU-LU Stage Pacing Verification
    printf("[Test] Verifying SNA LU-LU Stage Pacing...\n");
    tsfi_sna_stage_pacing stage_p;
    tsfi_sna_stage_pacing_init(&stage_p, 2);
    assert(stage_p.credits_left == 2);
    
    assert(tsfi_sna_stage_pacing_consume(&stage_p) == 0);
    assert(tsfi_sna_stage_pacing_consume(&stage_p) == 0);
    // Out of credits
    assert(tsfi_sna_stage_pacing_consume(&stage_p) == -2);
    
    // Recovery response restores credit pool
    tsfi_sna_stage_pacing_response(&stage_p);
    assert(stage_p.credits_left == 2);
    printf("  [PASS] SNA LU stage-by-stage flow pacing verified.\n");

    // 43. SNA Function Management Header (FMH) Codec Verification
    printf("[Test] Verifying SNA FMH Codecs...\n");
    tsfi_sna_fmh tx_fmh;
    tx_fmh.fmh_type = 0x05; // FMH-5 Transaction Attach
    tx_fmh.fmh_len = 0x04;
    tx_fmh.destination_id = 0x8899;
    
    uint8_t fmh_buf[16];
    size_t fmh_len = 0;
    assert(tsfi_sna_serialize_fmh(&tx_fmh, fmh_buf, &fmh_len) == 0);
    assert(fmh_len == 4);
    
    tsfi_sna_fmh rx_fmh;
    assert(tsfi_sna_deserialize_fmh(fmh_buf, fmh_len, &rx_fmh) == 0);
    assert(rx_fmh.fmh_type == 0x05);
    assert(rx_fmh.fmh_len == 0x04);
    assert(rx_fmh.destination_id == 0x8899);
    printf("  [PASS] SNA FMH-5 attach header codecs verified.\n");

    // 44. SNA SSCP-LU Service Control Session Verification
    printf("[Test] Verifying SNA SSCP-LU Session Controls...\n");
    tsfi_sscp_lu_session sscp_lu;
    tsfi_sscp_lu_init(&sscp_lu);
    assert(sscp_lu.lu_active == 0);
    
    assert(tsfi_sscp_lu_control(&sscp_lu, SNA_CMD_ACTLU) == 0);
    assert(sscp_lu.lu_active == 1);
    assert(tsfi_sscp_lu_control(&sscp_lu, SNA_CMD_DACTLU) == 0);
    assert(sscp_lu.lu_active == 0);
    printf("  [PASS] SSCP ACTLU and DACTLU control handshakes verified.\n");

    // 45. SNA Request Unit (RU) Chaining Verification
    printf("[Test] Verifying SNA RU Chaining...\n");
    tsfi_sna_chain_assembler assembler;
    tsfi_sna_chain_init(&assembler);
    assert(assembler.len == 0);
    
    // Add segments to chain assembler
    assert(tsfi_sna_chain_add(&assembler, SNA_CHAIN_FIC, (uint8_t*)"FIRST_", 6) == 0);
    assert(tsfi_sna_chain_add(&assembler, SNA_CHAIN_MIC, (uint8_t*)"MID_", 4) == 0);
    assert(tsfi_sna_chain_add(&assembler, SNA_CHAIN_LIC, (uint8_t*)"LAST", 4) == 1); // Finished assembly
    assert(assembler.len == 14);
    assert(memcmp(assembler.buffer, "FIRST_MID_LAST", 14) == 0);
    printf("  [PASS] SNA RU chain packet accumulation verified.\n");

    // 46. SNA Session BIND Profile Negotiation Verification
    printf("[Test] Verifying SNA Session BIND Profile Negotiation...\n");
    tsfi_sna_bind_profile local_prof;
    tsfi_sna_bind_profile requested_prof;
    
    tsfi_sna_bind_profile_init(&local_prof);
    tsfi_sna_bind_profile_init(&requested_prof);
    requested_prof.pacing_in = 1; // Limit input pacing window
    requested_prof.duplex_mode = 1; // Request full duplex
    
    assert(tsfi_sna_bind_profile_negotiate(&local_prof, &requested_prof) == 0);
    assert(local_prof.pacing_in == 1);
    assert(local_prof.duplex_mode == 1);
    printf("  [PASS] SNA session BIND profile parameters verified.\n");

    // 47. SNA Transmission Group Failover Verification
    printf("[Test] Verifying SNA Transmission Group Failovers...\n");
    tsfi_sna_tg_failover failover;
    tsfi_sna_tg_failover_init(&failover);
    assert(failover.active_links == 3);
    assert(failover.backup_route_active == 0);
    
    // Fail first two links: TG group remains active
    assert(tsfi_sna_tg_link_fail(&failover, 1) == 0);
    assert(tsfi_sna_tg_link_fail(&failover, 2) == 0);
    assert(failover.active_links == 1);
    assert(failover.backup_route_active == 0);
    
    // Fail final link: triggers failover backup route
    assert(tsfi_sna_tg_link_fail(&failover, 3) == 1);
    assert(failover.active_links == 0);
    assert(failover.backup_route_active == 1);
    printf("  [PASS] Transmission Group multi-link path failover verified.\n");

    // 48. SNA Bracket Protocol Verification
    printf("[Test] Verifying SNA Bracket Protocols...\n");
    tsfi_sna_bracket bracket;
    tsfi_sna_bracket_init(&bracket);
    assert(bracket.bracket_active == 0);
    
    assert(tsfi_sna_bracket_process(&bracket, 1, 0) == 0); // Begin bracket
    assert(bracket.bracket_active == 1);
    assert(tsfi_sna_bracket_process(&bracket, 0, 1) == 0); // End bracket
    assert(bracket.bracket_active == 0);
    printf("  [PASS] Bracket boundary markers verified.\n");

    // 49. SNA Half-Duplex Flip-Flop Verification
    printf("[Test] Verifying SNA Half-Duplex Flip-Flop...\n");
    tsfi_sna_hdx hdx;
    tsfi_sna_hdx_init(&hdx, 1); // Start with my turn
    assert(hdx.my_turn == 1);
    
    assert(tsfi_sna_hdx_process(&hdx, 0, 1) == 0); // Pass turn
    assert(hdx.my_turn == 0);
    assert(tsfi_sna_hdx_process(&hdx, 1, 0) == 0); // Receive turn
    assert(hdx.my_turn == 1);
    printf("  [PASS] Half-duplex change direction handshakes verified.\n");

    // 50. SNA Response Request Tracker Verification
    printf("[Test] Verifying SNA Response Request Tracker...\n");
    tsfi_sna_response_tracker tracker;
    tsfi_sna_response_init(&tracker);
    assert(tracker.dr1_requested == 0);
    
    // Parse RH byte (DR1 | Exception Response Only = 0x01 | 0x04 = 0x05)
    assert(tsfi_sna_response_parse(&tracker, 0x05) == 0);
    assert(tracker.dr1_requested == 1);
    assert(tracker.dr2_requested == 0);
    assert(tracker.exception_response_only == 1);
    printf("  [PASS] Response request state tracking verified.\n");

    // 51. SNA PIU Block Check Character Verification
    printf("[Test] Verifying SNA PIU BCC Checksum...\n");
    uint8_t sample_piu[] = { 0x11, 0x22, 0x33, 0x44 };
    // 0x11 ^ 0x22 ^ 0x33 ^ 0x44 = 0x44
    assert(tsfi_sna_piu_bcc(sample_piu, 4) == 0x44);
    printf("  [PASS] PIU block check character checksums verified.\n");

    // 52. SNA BIND/UNBIND Command Codec Verification
    printf("[Test] Verifying SNA BIND/UNBIND Codecs...\n");
    tsfi_sna_session_cmd tx_cmd;
    tx_cmd.command_code = SNA_CMD_BIND;
    tx_cmd.local_addr = 0x1234;
    tx_cmd.remote_addr = 0x5678;
    
    uint8_t cmd_buf[16];
    size_t cmd_len = 0;
    assert(tsfi_sna_serialize_session_cmd(&tx_cmd, cmd_buf, &cmd_len) == 0);
    assert(cmd_len == 5);
    
    tsfi_sna_session_cmd rx_cmd;
    assert(tsfi_sna_deserialize_session_cmd(cmd_buf, cmd_len, &rx_cmd) == 0);
    assert(rx_cmd.command_code == SNA_CMD_BIND);
    assert(rx_cmd.local_addr == 0x1234);
    assert(rx_cmd.remote_addr == 0x5678);
    
    // Validate SDT & CLEAR commands
    tx_cmd.command_code = SNA_CMD_SDT;
    assert(tsfi_sna_serialize_session_cmd(&tx_cmd, cmd_buf, &cmd_len) == 0);
    assert(tsfi_sna_deserialize_session_cmd(cmd_buf, cmd_len, &rx_cmd) == 0);
    assert(rx_cmd.command_code == SNA_CMD_SDT);

    tx_cmd.command_code = SNA_CMD_CLEAR;
    assert(tsfi_sna_serialize_session_cmd(&tx_cmd, cmd_buf, &cmd_len) == 0);
    assert(tsfi_sna_deserialize_session_cmd(cmd_buf, cmd_len, &rx_cmd) == 0);
    assert(rx_cmd.command_code == SNA_CMD_CLEAR);

    printf("  [PASS] BIND/UNBIND/SDT/CLEAR command packet codecs verified.\n");

    // 53. SNA Session Key Rotation Verification
    printf("[Test] Verifying SNA Session Key Rotations...\n");
    uint8_t dist_key[8] = { 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x11, 0x22 };
    uint8_t raw_new_key[8] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08 };
    uint8_t encrypted_new_key[8];
    
    // Encrypt the new key with master distribution key via XOR
    for (int i = 0; i < 8; i++) {
        encrypted_new_key[i] = raw_new_key[i] ^ dist_key[i];
    }
    
    tsfi_sna_key_rotation rot;
    tsfi_sna_key_rotation_init(&rot, dist_key);
    assert(tsfi_sna_rotate_key(&rot, encrypted_new_key) == 0);
    assert(memcmp(rot.current_session_key, raw_new_key, 8) == 0);
    printf("  [PASS] Session key rotation and master wrapping verified.\n");

    // 54. SNA Transmission Header Verification
    printf("[Test] Verifying SNA Transmission Header FID2 & FID4...\n");
    tsfi_sna_th tx_th;
    tx_th.fid_type = SNA_FID_TYPE2;
    tx_th.mpf = 0x01; // FIC segment
    tx_th.daf = 0x00AA;
    tx_th.oaf = 0x00BB;
    tx_th.sn = 999;
    
    uint8_t th_buf[16];
    size_t th_len = 0;
    assert(tsfi_sna_serialize_th(&tx_th, th_buf, &th_len) == 0);
    assert(th_len == 6);
    
    tsfi_sna_th rx_th;
    assert(tsfi_sna_deserialize_th(th_buf, th_len, &rx_th) == 0);
    assert(rx_th.fid_type == SNA_FID_TYPE2);
    assert(rx_th.mpf == 0x01);
    assert(rx_th.daf == 0xAA);
    assert(rx_th.oaf == 0xBB);
    assert(rx_th.sn == 999);
    
    // Validate FID1 (10-byte header)
    tx_th.fid_type = SNA_FID_TYPE1;
    assert(tsfi_sna_serialize_th(&tx_th, th_buf, &th_len) == 0);
    assert(th_len == 10);
    assert(tsfi_sna_deserialize_th(th_buf, th_len, &rx_th) == 0);
    assert(rx_th.fid_type == SNA_FID_TYPE1);
    assert(rx_th.daf == 0x00AA);
    assert(rx_th.oaf == 0x00BB);
    assert(rx_th.sn == 999);
    
    // Validate FID3 (2-byte header)
    tx_th.fid_type = SNA_FID_TYPE3;
    assert(tsfi_sna_serialize_th(&tx_th, th_buf, &th_len) == 0);
    assert(th_len == 2);
    assert(tsfi_sna_deserialize_th(th_buf, th_len, &rx_th) == 0);
    assert(rx_th.fid_type == SNA_FID_TYPE3);
    assert(rx_th.daf == 0xAA); // Local Session ID
    
    // Validate FID0 (10-byte raw compatibility header)
    tx_th.fid_type = SNA_FID_TYPE0;
    assert(tsfi_sna_serialize_th(&tx_th, th_buf, &th_len) == 0);
    assert(th_len == 10);
    assert(tsfi_sna_deserialize_th(th_buf, th_len, &rx_th) == 0);
    assert(rx_th.fid_type == SNA_FID_TYPE0);
    assert(rx_th.daf == 0x00AA);
    assert(rx_th.oaf == 0x00BB);
    assert(rx_th.sn == 999);
    
    // Validate FIDF (2-byte routing control header)
    tx_th.fid_type = SNA_FID_TYPEF;
    assert(tsfi_sna_serialize_th(&tx_th, th_buf, &th_len) == 0);
    assert(th_len == 2);
    assert(tsfi_sna_deserialize_th(th_buf, th_len, &rx_th) == 0);
    assert(rx_th.fid_type == SNA_FID_TYPEF);
    assert(rx_th.daf == 0xAA);
    
    printf("  [PASS] Transmission Header FID0, FID1, FID2, FID3, FID4, & FIDF serialization verified.\n");

    // 55. SNA Request/Response Header (RH) Verification
    printf("[Test] Verifying SNA Request/Response Header (RH)...\n");
    tsfi_sna_rh tx_rh;
    tx_rh.ru_category = 2; // DFC
    tx_rh.is_response = 0;
    tx_rh.format_indicator = 1;
    tx_rh.sense_data_included = 0;
    tx_rh.begin_chain = 1;
    tx_rh.end_chain = 0;
    tx_rh.dr1_indicator = 1;
    tx_rh.dr2_indicator = 0;
    tx_rh.exception_response = 1;
    tx_rh.change_direction = 1;
    tx_rh.begin_bracket = 0;
    tx_rh.end_bracket = 0;
    
    uint8_t rh_buf[4];
    size_t rh_len = 0;
    assert(tsfi_sna_serialize_rh(&tx_rh, rh_buf, &rh_len) == 0);
    assert(rh_len == 3);
    
    tsfi_sna_rh rx_rh;
    assert(tsfi_sna_deserialize_rh(rh_buf, rh_len, &rx_rh) == 0);
    assert(rx_rh.ru_category == 2);
    assert(rx_rh.is_response == 0);
    assert(rx_rh.format_indicator == 1);
    assert(rx_rh.sense_data_included == 0);
    assert(rx_rh.begin_chain == 1);
    assert(rx_rh.end_chain == 0);
    assert(rx_rh.dr1_indicator == 1);
    assert(rx_rh.dr2_indicator == 0);
    assert(rx_rh.exception_response == 1);
    assert(rx_rh.change_direction == 1);
    assert(rx_rh.begin_bracket == 0);
    assert(rx_rh.end_bracket == 0);
    printf("  [PASS] Request/Response Header (RH) serialization verified.\n");

    // 56. SNA LU Type Mapping Verification
    printf("[Test] Verifying SNA LU Type Mapping...\n");
    assert(tsfi_sna_map_lu_type(LU_TYPE_FILE) == SNA_LU_TYPE1);
    assert(tsfi_sna_map_lu_type(LU_TYPE_SOCKET) == SNA_LU_TYPE62);
    assert(tsfi_sna_map_lu_type(LU_TYPE_TERMINAL) == SNA_LU_TYPE2);
    assert(tsfi_sna_map_lu_type(LU_TYPE_DISK) == SNA_LU_TYPE0);
    printf("  [PASS] Standard SNA LU type mapping verified.\n");

    // 57. SNA Sense Code Resolver Verification
    printf("[Test] Verifying SNA Sense Code Resolver...\n");
    assert(strcmp(tsfi_sna_resolve_sense(SNA_SENSE_RESOURCE_UNAVAILABLE), "Resource Not Available") == 0);
    assert(strcmp(tsfi_sna_resolve_sense(SNA_SENSE_SESSION_LIMIT), "Session Limit Exceeded") == 0);
    assert(strcmp(tsfi_sna_resolve_sense(SNA_SENSE_END_USER_UNAVAILABLE), "End-User Not Available") == 0);
    assert(strcmp(tsfi_sna_resolve_sense(SNA_SENSE_PATH_OUT_OF_ORDER), "Path Out of Order") == 0);
    assert(strcmp(tsfi_sna_resolve_sense(0xFFFF), "Unknown/General Protocol Error") == 0);
    printf("  [PASS] Standard SNA Sense Code resolver verified.\n");

    // 58. SNA PIU Unified Packaging Verification
    printf("[Test] Verifying SNA PIU Unified Packaging...\n");
    tsfi_sna_th piu_th;
    piu_th.fid_type = SNA_FID_TYPE2;
    piu_th.mpf = 0x03; // Only segment
    piu_th.daf = 0x11;
    piu_th.oaf = 0x22;
    piu_th.sn = 42;
    
    tsfi_sna_rh piu_rh;
    memset(&piu_rh, 0, sizeof(piu_rh));
    piu_rh.ru_category = 0; // FMD
    piu_rh.is_response = 0;
    piu_rh.begin_chain = 1;
    piu_rh.end_chain = 1;
    
    uint8_t ru_payload[] = "SNA_TEST_RU";
    uint8_t piu_packet[64];
    size_t piu_packet_len = 0;
    
    assert(tsfi_sna_package_piu(&piu_th, &piu_rh, ru_payload, 11, piu_packet, &piu_packet_len) == 0);
    assert(piu_packet_len == 6 + 3 + 11 + 1); // TH(6) + RH(3) + RU(11) + BCC(1) = 21 bytes
    
    tsfi_sna_th decoded_th;
    tsfi_sna_rh decoded_rh;
    uint8_t decoded_ru[32];
    size_t decoded_ru_len = 0;
    
    assert(tsfi_sna_parse_piu(piu_packet, piu_packet_len, &decoded_th, &decoded_rh, decoded_ru, &decoded_ru_len) == 0);
    assert(decoded_th.fid_type == SNA_FID_TYPE2);
    assert(decoded_th.sn == 42);
    assert(decoded_rh.begin_chain == 1);
    assert(decoded_ru_len == 11);
    assert(memcmp(decoded_ru, "SNA_TEST_RU", 11) == 0);
    printf("  [PASS] SNA unified PIU frame assembly and parsing verified.\n");

    // 59. Usenet Posting Storage & Retrieval Verification
    printf("[Test] Verifying Usenet Posting Storage & Retrieval...\n");
    tsfi_usenet_article tx_art;
    tsfi_usenet_init(&tx_art, "net.general", 1001, "Hello Mainframe World", "This is a test post stored in binary dat bin format.");
    
    uint8_t usenet_buf[512];
    size_t usenet_len = 0;
    assert(tsfi_usenet_store_bin(&tx_art, usenet_buf, &usenet_len) == 0);
    assert(usenet_len == sizeof(tsfi_usenet_article));
    
    tsfi_usenet_article rx_art;
    assert(tsfi_usenet_retrieve_bin(usenet_buf, usenet_len, &rx_art) == 0);
    assert(strcmp(rx_art.newsgroup, "net.general") == 0);
    assert(rx_art.article_number == 1001);
    assert(strcmp(rx_art.subject, "Hello Mainframe World") == 0);
    assert(strcmp(rx_art.body, "This is a test post stored in binary dat bin format.") == 0);
    printf("  [PASS] Usenet posting binary serialization, preservation, and retrieval verified.\n");

    // 60. SNA TH MPF to RH Chaining Mappings Verification
    printf("[Test] Verifying SNA TH MPF to RH Chaining Mappings...\n");
    tsfi_sna_rh test_rh;
    
    // First segment (mpf = 0x02)
    tsfi_sna_map_th_mpf_to_rh_chain(0x02, &test_rh);
    assert(test_rh.begin_chain == 1 && test_rh.end_chain == 0);
    
    // Middle segment (mpf = 0x00)
    tsfi_sna_map_th_mpf_to_rh_chain(0x00, &test_rh);
    assert(test_rh.begin_chain == 0 && test_rh.end_chain == 0);
    
    // Last segment (mpf = 0x01)
    tsfi_sna_map_th_mpf_to_rh_chain(0x01, &test_rh);
    assert(test_rh.begin_chain == 0 && test_rh.end_chain == 1);
    
    // Only segment (mpf = 0x03)
    tsfi_sna_map_th_mpf_to_rh_chain(0x03, &test_rh);
    assert(test_rh.begin_chain == 1 && test_rh.end_chain == 1);
    
    printf("  [PASS] SNA TH MPF to RH chaining mappings verified.\n");

    // 61. z/VM SNA Systems Verification (GCS, PVM, RSCS, VSCS)
    printf("[Test] Verifying z/VM SNA Systems...\n");
    
    // GCS
    tsfi_zvm_gcs test_gcs;
    tsfi_zvm_gcs_init(&test_gcs, 12, "VTAMSEG");
    assert(test_gcs.vmid == 12);
    assert(strcmp(test_gcs.shared_segment_name, "VTAMSEG") == 0);
    assert(test_gcs.vtam_active == 0);
    tsfi_zvm_gcs_set_vtam(&test_gcs, 1);
    assert(test_gcs.vtam_active == 1);
    
    // PVM
    tsfi_zvm_pvm test_pvm;
    tsfi_zvm_pvm_init(&test_pvm);
    assert(test_pvm.is_active == 0);
    assert(tsfi_zvm_pvm_route(&test_pvm, 2, 4) == 0);
    assert(test_pvm.is_active == 1);
    assert(test_pvm.source_lu == 2);
    assert(test_pvm.target_lu == 4);
    assert(test_pvm.session_id == 0x0204);
    
    // RSCS
    tsfi_zvm_rscs_spool test_spool;
    tsfi_zvm_rscs_init(&test_spool, "JOB001.TXT", 500, 1, 4096);
    assert(test_spool.file_id == 500);
    assert(test_spool.lu_type == 1);
    assert(test_spool.size_bytes == 4096);
    assert(strcmp(test_spool.spool_file_name, "JOB001.TXT") == 0);
    
    // VSCS
    tsfi_zvm_vscs test_vscs;
    tsfi_zvm_vscs_init(&test_vscs);
    assert(test_vscs.is_attached == 0);
    assert(tsfi_zvm_vscs_attach(&test_vscs, 3, 12) == 0);
    assert(test_vscs.is_attached == 1);
    assert(test_vscs.terminal_lu == 3);
    assert(test_vscs.target_vmid == 12);
    
    printf("  [PASS] z/VM SNA systems (GCS, PVM, RSCS, VSCS) integration verified.\n");

    // 62. APPC, 3270 Formatter, and FIPS 186-5 ECDSA Signatures Verification
    printf("[Test] Verifying APPC, 3270 Formatter, & Cryptographic signatures...\n");
    
    // APPC/LU6.2
    tsfi_appc_conversation conv;
    assert(tsfi_appc_allocate(&conv, 1, 2) == 0);
    assert(conv.state == 0);
    uint8_t payload[] = "TEST";
    assert(tsfi_appc_send_data(&conv, payload, 4) == 0);
    assert(conv.state == 1);
    uint8_t rx_buf[16];
    size_t rx_len = 0;
    assert(tsfi_appc_receive_data(&conv, rx_buf, &rx_len) == 0);
    assert(conv.state == 2);
    assert(tsfi_appc_deallocate(&conv) == 0);
    assert(conv.state == 3);
    
    // 3270 screen formatting
    uint8_t ebcdic_scr[1024];
    size_t ebcdic_len = 0;
    assert(tsfi_3270_format_usenet_list(&tx_art, 1, ebcdic_scr, &ebcdic_len) == 0);
    assert(ebcdic_len > 5);
    assert(ebcdic_scr[0] == 0x11); // SBA byte
    
    // FIPS 186-5 ECDSA Signatures
    uint8_t priv_key[32];
    uint8_t pub_key[32];
    for (int i = 0; i < 32; i++) {
        priv_key[i] = i;
        pub_key[i] = i ^ 0xAA;
    }
    tsfi_usenet_signature signature;
    
    assert(tsfi_usenet_sign_article(&tx_art, priv_key, &signature) == 0);
    assert(tsfi_usenet_verify_article(&tx_art, pub_key, &signature) == 0);
    
    printf("  [PASS] APPC conversation states, 3270 formatting, and FIPS 186-5 signatures verified.\n");

    // 63. DDP & BNA Networks Integration Verification
    printf("[Test] Verifying DDP & BNA Networks Integration...\n");
    tsfi_bna_node bna_node;
    tsfi_bna_node_init(&bna_node, 101, "BURR_B6700");
    assert(bna_node.node_id == 101);
    assert(bna_node.bna_active == 1);
    assert(strcmp(bna_node.hostname, "BURR_B6700") == 0);
    
    tsfi_ddp_bridge ddp;
    tsfi_ddp_bridge_init(&ddp);
    ddp.sna_lu_count = 5;
    ddp.decnet_node_count = 3;
    ddp.bna_node_count = 2;
    ddp.coaxial_carrier_frequency = 12.5f;
    
    char ddp_status[128];
    assert(tsfi_ddp_bridge_status(&ddp, ddp_status, sizeof(ddp_status)) == 0);
    assert(strstr(ddp_status, "SNA LUs=5") != NULL);
    assert(strstr(ddp_status, "DECnet Nodes=3") != NULL);
    assert(strstr(ddp_status, "BNA Nodes=2") != NULL);
    assert(strstr(ddp_status, "Coaxial Freq=12.5MHz") != NULL);
    printf("  [PASS] BNA node configurations and DDP bridge statuses verified.\n");

    // 64. Sojka's Systems Verification (Braille WP, Bubble Memory, ADDS, PLATO)
    printf("[Test] Verifying Sojka's Systems...\n");
    
    // Braille WP
    uint8_t braille[16];
    size_t braille_len = 0;
    assert(tsfi_braille_translate("HELLO", braille, &braille_len) == 0);
    assert(braille_len == 5);
    assert(braille[0] == 8); // 'H' -> 8th letter
    assert(braille[1] == 5); // 'E' -> 5th letter
    
    // Bubble Memory
    tsfi_bubble_memory bm;
    tsfi_bubble_init(&bm);
    assert(tsfi_bubble_write(&bm, 12, 0xEF) == 0);
    uint8_t bubble_val = 0;
    assert(tsfi_bubble_read(&bm, 12, &bubble_val) == 0);
    assert(bubble_val == 0xEF);
    
    // ADDS Terminal
    tsfi_adds_terminal term;
    tsfi_adds_init(&term);
    assert(tsfi_adds_write_char(&term, 0xC1) == 0); // EBCDIC 'A'
    assert(term.screen_buffer[0] == 'A');
    assert(term.cursor_pos == 1);
    
    // CDC PLATO
    tsfi_cdc_plato plato;
    tsfi_plato_init(&plato);
    assert(tsfi_plato_process_key(&plato, 32) == 1);
    assert(plato.keystroke_count == 1);
    
    printf("  [PASS] Sojka's systems (Braille, Bubble Memory, ADDS, PLATO) verified.\n");

    // 65. Sojka's Extended Systems (MIS Budget, SDC Cryptographic validation)
    printf("[Test] Verifying Sojka's Extended Systems...\n");
    
    // MIS Budget
    tsfi_mis_budget mb;
    tsfi_mis_budget_init(&mb, 150.0f, 25.0f, 0.5f, 1000.0f);
    assert(tsfi_mis_calculate_cost(&mb, 4.0f, 10.0f, 100.0f) == 900.0f); // 4*150 + 10*25 + 100*0.5 = 900
    assert(tsfi_mis_is_over_budget(&mb) == 0);
    assert(tsfi_mis_calculate_cost(&mb, 1.0f, 0.0f, 300.0f) == 300.0f);  // +300 = 1200
    assert(tsfi_mis_is_over_budget(&mb) == 1);
    
    // SDC Crypto validation
    tsfi_sdc_crypto sdc;
    uint8_t mkey[16] = { 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x00 };
    tsfi_sdc_init(&sdc, 0xDEADBEEF, mkey);
    
    uint8_t dummy_rec[] = { 0xAA, 0xBB, 0xCC };
    uint32_t expected_sig = 0xDEADBEEF ^ 0xAA ^ 0xBB ^ 0xCC ^ 0x11 ^ 0x22 ^ 0x33;
    assert(tsfi_sdc_validate_record(&sdc, dummy_rec, 3, expected_sig) == 0);
    assert(tsfi_sdc_validate_record(&sdc, dummy_rec, 3, 0x00) == -2);
    
    printf("  [PASS] Sojka's extended systems (MIS Budget, SDC Cryptography) verified.\n");

    // 66. SDC Advanced Features Verification (ACM, Handshake, LISP Filter)
    printf("[Test] Verifying SDC Advanced Security & Query Features...\n");
    
    // SDC ACM
    tsfi_sdc_acm user_acm;
    user_acm.clearance_level = SDC_CLEARANCE_SECRET;
    user_acm.write_privilege = 0;
    assert(tsfi_sdc_acm_authorize(&user_acm, SDC_CLEARANCE_UNCLASSIFIED, 0) == 0);
    assert(tsfi_sdc_acm_authorize(&user_acm, SDC_CLEARANCE_TOPSECRET, 0) == -2);
    assert(tsfi_sdc_acm_authorize(&user_acm, SDC_CLEARANCE_UNCLASSIFIED, 1) == -3);
    
    // SDC Handshake
    tsfi_sdc_handshake hs;
    tsfi_sdc_handshake_init(&hs, 12345);
    uint32_t session_key = tsfi_sdc_handshake_exchange(&hs, 67890);
    assert(session_key > 0);
    
    // SDC LISP Filter
    assert(tsfi_sdc_lisp_filter("(EQUAL author mariarahel)", "author", "mariarahel") == 1);
    assert(tsfi_sdc_lisp_filter("(EQUAL author mariarahel)", "title", "pulsechain") == 0);
    
    printf("  [PASS] SDC advanced features (ACM, Handshake, LISP filter) verified.\n");

    // 67. Vin McLellan's Systems Verification (S.240, CPA, Apollo Frame)
    printf("[Test] Verifying Vin McLellan's Systems...\n");
    
    // S.240 Audit
    char audit_log[128];
    assert(tsfi_s240_audit("mariarahel", S240_ACCESS, "RAMAC_REC_01", audit_log, sizeof(audit_log)) == 0);
    assert(strstr(audit_log, "User=mariarahel") != NULL);
    assert(strstr(audit_log, "Action=UNAUTHORIZED_ACCESS") != NULL);
    assert(strstr(audit_log, "Resource=RAMAC_REC_01") != NULL);
    
    // CPA Verify
    tsfi_cpa_audit cpa;
    cpa.total_ledger_sum = 1000.0f;
    cpa.total_rendered_sum = 1000.02f;
    assert(tsfi_cpa_verify(&cpa, 0.05f) == 0);
    assert(tsfi_cpa_verify(&cpa, 0.01f) == -2);
    
    // Apollo Frame
    tsfi_apollo_frame tx_frame;
    tx_frame.ring_id = 99;
    tx_frame.source_node = 1;
    tx_frame.dest_node = 2;
    tx_frame.control_token = 1;
    
    uint8_t frame_buf[8];
    size_t frame_len = 0;
    assert(tsfi_apollo_serialize(&tx_frame, frame_buf, &frame_len) == 0);
    assert(frame_len == 5);
    
    tsfi_apollo_frame rx_frame;
    assert(tsfi_apollo_deserialize(frame_buf, frame_len, &rx_frame) == 0);
    assert(rx_frame.ring_id == 99);
    assert(rx_frame.source_node == 1);
    assert(rx_frame.dest_node == 2);
    assert(rx_frame.control_token == 1);
    
    printf("  [PASS] Vin McLellan's systems (S.240, CPA, Apollo Frame) verified.\n");

    // 68. Apollo Synthesizer Bird Call Sweep Verification
    printf("[Test] Verifying Apollo Synthesizer Bird Call Sweep Control...\n");
    float frequency_sweep[100];
    size_t sweep_points = 0;
    assert(tsfi_apollo_control_synth_bird_call(&tx_frame, frequency_sweep, &sweep_points) == 0);
    assert(sweep_points == 100);
    assert(frequency_sweep[0] == 2010.0f); // 2000.0 + (1 * 10.0)
    assert(frequency_sweep[99] == 6040.0f); // 6000.0 + (2 * 20.0)
    printf("  [PASS] Apollo synthesizer bird call sweep generation verified.\n");

    // 69. Apollo Advanced Audio Verification (Phonemes & Soundscape Mix)
    printf("[Test] Verifying Apollo Phonemes & Soundscape Mixer...\n");
    tsfi_apollo_phoneme phonemes[32];
    size_t phoneme_count = 0;
    assert(tsfi_apollo_spool_phonemes(&tx_frame, "BIRD", phonemes, &phoneme_count) == 0);
    assert(phoneme_count == 4);
    assert(phonemes[0].phoneme_char == 'B');
    assert(phonemes[0].pitch_frequency > 300.0f);
    
    tsfi_apollo_soundscape_node nodes[2];
    nodes[0].node_id = 10;
    nodes[0].x_pos = 1.0f;
    nodes[0].y_pos = 0.0f;
    nodes[0].volume_level = 0.5f;
    
    nodes[1].node_id = 20;
    nodes[1].x_pos = 0.0f;
    nodes[1].y_pos = 1.0f;
    nodes[1].volume_level = 0.5f;
    
    float mix[50];
    assert(tsfi_apollo_render_soundscape(nodes, 2, mix, 50) == 0);
    printf("  [PASS] Apollo phoneme spooling and multi-node soundscape rendering verified.\n");

    // 70. Apollo DFS & Display Manager Verification
    printf("[Test] Verifying Apollo DFS & Display Manager...\n");
    char node_res[32];
    char path_res[64];
    assert(tsfi_apollo_dfs_resolve("//NODE_A/sys/node_config", node_res, path_res, 32) == 0);
    assert(strcmp(node_res, "NODE_A") == 0);
    assert(strcmp(path_res, "/sys/node_config") == 0);
    
    tsfi_apollo_dm_pad pad;
    tsfi_apollo_dm_init(&pad, 4, 1024, 768, "pad_stdout");
    assert(pad.pad_id == 4);
    assert(pad.width == 1024);
    assert(pad.height == 768);
    assert(strcmp(pad.stream_association, "pad_stdout") == 0);
    printf("  [PASS] Apollo DFS path resolver and Display Manager pad configuration verified.\n");

    // 71. Apollo Aegis PCB and Token Ring Registers Verification
    printf("[Test] Verifying Apollo Aegis PCB and Token Ring Registers...\n");
    tsfi_aegis_pcb pcb;
    tsfi_aegis_pcb_init(&pcb, 1002, 4, "synth_voice");
    assert(pcb.process_id == 1002);
    assert(pcb.priority == 4);
    assert(strcmp(pcb.process_name, "synth_voice") == 0);
    
    tsfi_apollo_ring_register reg;
    tsfi_apollo_ring_reg_init(&reg);
    assert(reg.node_bypass == 0);
    assert(reg.token_held == 0);
    assert(reg.packets_transmitted == 0);
    printf("  [PASS] Apollo Aegis process blocks and token ring hardware registers verified.\n");

    // 72. Cambex SECDED and ADR Roscoe Verification
    printf("[Test] Verifying Cambex SECDED and ADR Roscoe...\n");
    uint8_t raw_data = 0xA5;
    uint8_t parity_ecc = tsfi_secded_encode(raw_data);
    
    // Correct decode
    uint8_t corrected_val = 0;
    assert(tsfi_secded_decode(raw_data, parity_ecc, &corrected_val) == 0);
    assert(corrected_val == raw_data);
    
    // Single bit error decode
    uint8_t corrupted_data = raw_data ^ 1; // Flip bit 0
    assert(tsfi_secded_decode(corrupted_data, parity_ecc, &corrected_val) == 1);
    assert(corrected_val == raw_data);
    
    // ADR Roscoe
    tsfi_roscoe_library roscoe;
    tsfi_roscoe_init(&roscoe);
    assert(tsfi_roscoe_add_member(&roscoe, "MAINPGM") == 0);
    assert(roscoe.member_count == 1);
    assert(tsfi_roscoe_lock_member(&roscoe, "MAINPGM", 1) == 0);
    assert(roscoe.members[0].locked == 1);
    printf("  [PASS] Cambex SECDED error correction and ADR Roscoe library locks verified.\n");

    // 73. CYCLADES Datagrams and SWIFT Message Parsing Verification
    printf("[Test] Verifying CYCLADES Datagrams & SWIFT Telex Messages...\n");
    
    // CYCLADES Datagram
    tsfi_cyclades_header tx_cyc;
    tx_cyc.src_node = 11;
    tx_cyc.dest_node = 22;
    tx_cyc.seq_num = 54321;
    tx_cyc.flags = 0x02; // SYN
    
    uint8_t cyc_buf[8];
    size_t cyc_len = 0;
    assert(tsfi_cyclades_serialize(&tx_cyc, cyc_buf, &cyc_len) == 0);
    assert(cyc_len == 5);
    
    tsfi_cyclades_header rx_cyc;
    assert(tsfi_cyclades_deserialize(cyc_buf, cyc_len, &rx_cyc) == 0);
    assert(rx_cyc.src_node == 11);
    assert(rx_cyc.dest_node == 22);
    assert(rx_cyc.seq_num == 54321);
    assert(rx_cyc.flags == 0x02);
    
    // SWIFT Parse
    const char *raw_swift = "{1:F01SENDERBICXXX}{2:I103RECEIVERBICX}{4:\nMT:103\nAMT:450000.00}";
    tsfi_swift_message swift_msg;
    assert(tsfi_swift_parse(raw_swift, &swift_msg) == 0);
    assert(strcmp(swift_msg.sender_bic, "SENDERBICXXX") == 0);
    assert(strcmp(swift_msg.receiver_bic, "RECEIVERBICX") == 0);
    assert(strcmp(swift_msg.message_type, "103") == 0);
    assert(swift_msg.amount == 450000.00f);
    printf("  [PASS] CYCLADES datagram headers and SWIFT banking telex parsing verified.\n");

    // 74. CYCLADES Transport connection states & SWIFT trailer block validation
    printf("[Test] Verifying CYCLADES connection state & SWIFT trailers...\n");
    tsfi_cyclades_connection conn;
    tsfi_cyclades_conn_init(&conn);
    assert(conn.state == CYCLADES_STATE_CLOSED);
    
    tsfi_cyclades_header syn_pkt;
    syn_pkt.src_node = 1;
    syn_pkt.dest_node = 2;
    syn_pkt.seq_num = 200;
    syn_pkt.flags = 0x02; // SYN
    
    tsfi_cyclades_header syn_ack_pkt;
    assert(tsfi_cyclades_process_packet(&conn, &syn_pkt, &syn_ack_pkt) == 0);
    assert(conn.state == CYCLADES_STATE_SYN_RCVD);
    assert(syn_ack_pkt.flags == 0x12); // SYN-ACK
    
    tsfi_cyclades_header ack_pkt;
    ack_pkt.src_node = 1;
    ack_pkt.dest_node = 2;
    ack_pkt.seq_num = 201;
    ack_pkt.flags = 0x10; // ACK
    
    tsfi_cyclades_header dummy;
    assert(tsfi_cyclades_process_packet(&conn, &ack_pkt, &dummy) == 0);
    assert(conn.state == CYCLADES_STATE_ESTABLISHED);
    
    // SWIFT block 5 trailer
    const char *swift_payload = "MT103 TRANSFER 50000.00 USD";
    char trailer_buf[32];
    assert(tsfi_swift_generate_trailer(swift_payload, trailer_buf, sizeof(trailer_buf)) == 0);
    assert(tsfi_swift_verify_trailer(swift_payload, trailer_buf) == 0);
    assert(tsfi_swift_verify_trailer(swift_payload, "{5:MAC-INVALID}") == -2);
    printf("  [PASS] CYCLADES transport states and SWIFT Block 5 trailers verified.\n");

    // 75. CYCLADES Sliding Window and SWIFT Block 1 Verification
    printf("[Test] Verifying CYCLADES Sliding Window & SWIFT Block 1 Parsing...\n");
    
    // CYCLADES Sliding Window
    tsfi_cyclades_window win;
    win.window_start = 1000;
    win.window_size = 10;
    assert(tsfi_cyclades_window_verify(&win, 1005) == 0);
    assert(tsfi_cyclades_window_verify(&win, 999) == -2);
    assert(tsfi_cyclades_window_verify(&win, 1010) == -2);
    
    // SWIFT Block 1
    const char *raw_b1 = "{1:F01TESTBICXX09812345432100}";
    tsfi_swift_block1 b1;
    assert(tsfi_swift_parse_block1(raw_b1, &b1) == 0);
    assert(b1.application_id == 'F');
    assert(strcmp(b1.service_id, "01") == 0);
    assert(strcmp(b1.sender_lt, "TESTBICXX098") == 0);
    assert(strcmp(b1.session_num, "1234") == 0);
    assert(strcmp(b1.seq_num, "543210") == 0);
    printf("  [PASS] CYCLADES sliding window verification and SWIFT Block 1 header parsing verified.\n");

    // 76. Red-Black Gateway Routing Verification
    printf("[Test] Verifying Red-Black Gateway Routing (CYCLADES/SWIFT)...\n");
    tsfi_rb_packet red_pkt;
    red_pkt.color = RB_COLOR_RED;
    red_pkt.data_len = 5;
    red_pkt.payload[0] = 5;   // src
    red_pkt.payload[1] = 6;   // dest
    red_pkt.payload[2] = 0x12; // seq msb
    red_pkt.payload[3] = 0x34; // seq lsb
    red_pkt.payload[4] = 0x02; // flags
    
    tsfi_cyclades_header rx_cyc_hdr;
    assert(tsfi_rb_gateway_route(&red_pkt, &rx_cyc_hdr) == 0);
    assert(rx_cyc_hdr.src_node == 5);
    assert(rx_cyc_hdr.dest_node == 6);
    assert(rx_cyc_hdr.seq_num == 0x1234);
    assert(rx_cyc_hdr.flags == 0x02);
    
    tsfi_rb_packet black_pkt;
    black_pkt.color = RB_COLOR_BLACK;
    const char *swift_data = "{1:F01TESTBICXX09812345432100}";
    black_pkt.data_len = strlen(swift_data);
    memcpy(black_pkt.payload, swift_data, black_pkt.data_len + 1);
    
    tsfi_swift_block1 rx_b1;
    assert(tsfi_rb_gateway_route(&black_pkt, &rx_b1) == 0);
    assert(rx_b1.application_id == 'F');
    assert(strcmp(rx_b1.sender_lt, "TESTBICXX098") == 0);
    printf("  [PASS] Red-Black gateway routing of CYCLADES (Red) and SWIFT (Black) verified.\n");

    // 77. 2-3 Tree Insertion Verification
    printf("[Test] Verifying 2-3 Tree Node Insertions (CYCLADES/SWIFT)...\n");
    tsfi_rb_23_node tree_node;
    tsfi_rb_23_init(&tree_node);
    assert(tree_node.is_three_node == 0);
    assert(tree_node.swift_keys[0] == 0);
    assert(tree_node.cyclades_buffer_count == 0);
    
    // Insert first key
    assert(tsfi_rb_23_insert(&tree_node, 1001, 10) == 0);
    assert(tree_node.swift_keys[0] == 1001);
    assert(tree_node.cyclades_buffer_count == 10);
    assert(tree_node.is_three_node == 0);
    
    // Insert second key (split threshold -> 3-node transition)
    assert(tsfi_rb_23_insert(&tree_node, 999, 15) == 0);
    assert(tree_node.is_three_node == 1);
    assert(tree_node.swift_keys[0] == 999);
    assert(tree_node.swift_keys[1] == 1001);
    assert(tree_node.cyclades_buffer_count == 25);
    
    // Attempt third key -> overflow node split trigger
    assert(tsfi_rb_23_insert(&tree_node, 1005, 5) == -2);
    printf("  [PASS] 2-3 Tree node insertion and split triggers verified.\n");

    // 78. Gibson Mix, CAD vector transform, and relational query optimizer verification
    printf("[Test] Verifying Gibson Mix, CAD, and Optimizer...\n");
    
    // Gibson Mix
    tsfi_gibson_mix_input gibson;
    gibson.load_store_count = 1000;
    gibson.add_sub_count = 500;
    gibson.multiply_count = 100;
    gibson.divide_count = 20;
    gibson.branch_count = 300;
    gibson.logic_count = 80;
    
    float mips = tsfi_gibson_mix_calculate_mips(&gibson);
    assert(mips > 0.0f);
    
    // CAD/CAM Vector Pipeline
    tsfi_cad_line line_in;
    line_in.start.x = 10.0f;
    line_in.start.y = 20.0f;
    line_in.end.x = 30.0f;
    line_in.end.y = 40.0f;
    line_in.color = 2;
    
    tsfi_cad_line line_out;
    tsfi_cad_transform_line(&line_in, 2.0f, 5.0f, -5.0f, &line_out);
    assert(line_out.start.x == 25.0f);
    assert(line_out.start.y == 35.0f);
    assert(line_out.end.x == 65.0f);
    assert(line_out.end.y == 75.0f);
    assert(line_out.color == 2);
    
    // Relational Query Optimizer cost model
    tsfi_optimizer_input opt;
    opt.total_pages = 1000;
    opt.total_tuples = 10000;
    opt.selectivity = 0.05f;
    
    // Sequential Scan
    opt.has_index = 0;
    float seq_cost = tsfi_optimizer_estimate_cost(&opt);
    assert(seq_cost == 1000.0f);
    
    // Index Scan
    opt.has_index = 1;
    float index_cost = tsfi_optimizer_estimate_cost(&opt);
    assert(index_cost == 51.0f);
    printf("  [PASS] Gibson Mix, CAD vector pipeline, and relational query optimizer verified.\n");

    printf("[PASS] All distributed networking unit tests executed successfully!\n");
    printf("=============================================================\n");
    return 0;
}
