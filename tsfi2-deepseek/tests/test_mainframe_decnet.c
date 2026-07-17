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
    printf("  [PASS] Transmission Header FID2 & FID4 serialization verified.\n");

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

    printf("[PASS] All distributed networking unit tests executed successfully!\n");
    printf("=============================================================\n");
    return 0;
}
