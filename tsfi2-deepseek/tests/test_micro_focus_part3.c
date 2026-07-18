#include <stdio.h>
#include <assert.h>
#include "tsfi_cade_imf.h"

int run_nato_stanag_tests(void) {
    // 250. Verify NATO STANAG 5066 Header Validator
    printf("[TEST] Validating NATO STANAG 5066 Header Validator...\n");
    const unsigned char s5066_hdr[] = {0x90, 0xEB, 0x05, 0x00, 0x10};
    int s5066_valid = -1;
    int s5066_res = tsfi_mf_nato_verify_stanag5066_header(s5066_hdr, sizeof(s5066_hdr), &s5066_valid);
    assert(s5066_res == 0);
    assert(s5066_valid == 1);
    printf("  [PASS] NATO STANAG 5066 Header Validator verified.\n");

    // 251. Verify NATO STANAG 5066 Kermit Payload Validator
    printf("[TEST] Validating NATO STANAG 5066 Kermit Payload Validator...\n");
    const unsigned char kermit_pkt[] = {0x01, 35, 35, 35, 74};
    int kermit_valid = -1;
    int kermit_res = tsfi_mf_nato_verify_kermit_payload(kermit_pkt, sizeof(kermit_pkt), &kermit_valid);
    assert(kermit_res == 0);
    assert(kermit_valid == 1);
    printf("  [PASS] NATO STANAG 5066 Kermit Payload Validator verified.\n");

    // 252. Verify NATO STANAG 5066 Client Protocol Data Unit (C_PDU) Type Matcher
    printf("[TEST] Validating NATO STANAG 5066 C_PDU Type Matcher...\n");
    int cpdu_type_valid = -1;
    int cpdu_type_res = tsfi_mf_nato_verify_cpdu_type(3, &cpdu_type_valid);
    assert(cpdu_type_res == 0);
    assert(cpdu_type_valid == 1);
    printf("  [PASS] NATO STANAG 5066 C_PDU Type Matcher verified.\n");

    // 253. Verify NATO STANAG 5066 HF MTU Segment Size Validator
    printf("[TEST] Validating NATO STANAG 5066 HF MTU Segment Size Validator...\n");
    int seg_size_valid = -1;
    int seg_size_res = tsfi_mf_nato_verify_segment_size(1024, &seg_size_valid);
    assert(seg_size_res == 0);
    assert(seg_size_valid == 1);
    printf("  [PASS] NATO STANAG 5066 HF MTU Segment Size Validator verified.\n");

    // 254. Verify NATO STANAG 5066 Service Access Point (SAP) Number Matcher
    printf("[TEST] Validating NATO STANAG 5066 SAP Matcher...\n");
    int sap_valid = -1;
    int sap_res = tsfi_mf_nato_verify_sap_number(12, &sap_valid);
    assert(sap_res == 0);
    assert(sap_valid == 1);
    printf("  [PASS] NATO STANAG 5066 SAP Matcher verified.\n");

    // 255. Verify NATO STANAG 5066 ARQ Window Size Validator
    printf("[TEST] Validating NATO STANAG 5066 ARQ Window Size Validator...\n");
    int window_valid = -1;
    int window_res = tsfi_mf_nato_verify_arq_window(64, &window_valid);
    assert(window_res == 0);
    assert(window_valid == 1);
    printf("  [PASS] NATO STANAG 5066 ARQ Window Size Validator verified.\n");

    // 256. Verify NATO STANAG 5066 Priority Level Checker
    printf("[TEST] Validating NATO STANAG 5066 Priority Level Checker...\n");
    int priority_valid = -1;
    int priority_res = tsfi_mf_nato_verify_priority_level(8, &priority_valid);
    assert(priority_res == 0);
    assert(priority_valid == 1);
    printf("  [PASS] NATO STANAG 5066 Priority Level Checker verified.\n");

    // 257. Verify NATO STANAG 5066 Sync Sequence Type Matcher
    printf("[TEST] Validating NATO STANAG 5066 Sync Sequence Matcher...\n");
    int sync_seq_valid = -1;
    int sync_seq_res = tsfi_mf_nato_match_sync_sequence(0x90EB, &sync_seq_valid);
    assert(sync_seq_res == 0);
    assert(sync_seq_valid == 1);
    printf("  [PASS] NATO STANAG 5066 Sync Sequence Matcher verified.\n");

    // 258. Verify NATO STANAG 5066 Client Protocol Data Unit (C_PDU) Length Checker
    printf("[TEST] Validating NATO STANAG 5066 C_PDU Length Checker...\n");
    int cpdu_len_valid = -1;
    int cpdu_len_res = tsfi_mf_nato_verify_cpdu_length(2048, &cpdu_len_valid);
    assert(cpdu_len_res == 0);
    assert(cpdu_len_valid == 1);
    printf("  [PASS] NATO STANAG 5066 C_PDU Length Checker verified.\n");

    // 259. Verify NATO STANAG 5066 Transmission Mode Selector
    printf("[TEST] Validating NATO STANAG 5066 Tx Mode Validator...\n");
    int tx_mode_valid = -1;
    int tx_mode_res = tsfi_mf_nato_verify_tx_mode(1, &tx_mode_valid);
    assert(tx_mode_res == 0);
    assert(tx_mode_valid == 1);
    printf("  [PASS] NATO STANAG 5066 Tx Mode Validator verified.\n");

    // 260. Verify NATO STANAG 5066 Short-TIN Node Address Validator
    printf("[TEST] Validating NATO STANAG 5066 Short-TIN Address Matcher...\n");
    int short_tin_valid = -1;
    int short_tin_res = tsfi_mf_nato_verify_short_tin(50000, &short_tin_valid);
    assert(short_tin_res == 0);
    assert(short_tin_valid == 1);
    printf("  [PASS] NATO STANAG 5066 Short-TIN Address Matcher verified.\n");

    // 261. Verify NATO STANAG 5066 Segmented Transfer Offset Matcher
    printf("[TEST] Validating NATO STANAG 5066 Segmented Offset Matcher...\n");
    int seg_offset_valid = -1;
    int seg_offset_res = tsfi_mf_nato_verify_segment_offset(1000, 500, 2000, &seg_offset_valid);
    assert(seg_offset_res == 0);
    assert(seg_offset_valid == 1);
    printf("  [PASS] NATO STANAG 5066 Segmented Offset Matcher verified.\n");

    // 262. Verify NATO Short-TIN IRS Exclusivity Matcher
    printf("[TEST] Validating NATO Short-TIN IRS Exclusivity Matcher...\n");
    int tin_isolated = -1;
    int tin_isol_res = tsfi_mf_cross_verify_tin_isolation("777665555", &tin_isolated);
    assert(tin_isol_res == 0);
    assert(tin_isolated == 1);
    int broadcast_tin_isolated = -1;
    int broadcast_tin_res = tsfi_mf_cross_verify_tin_isolation("67108863", &broadcast_tin_isolated);
    assert(broadcast_tin_res == 0);
    assert(broadcast_tin_isolated == 0);
    printf("  [PASS] NATO Short-TIN IRS Exclusivity Matcher verified.\n");

    // 263. Verify NATO STANAG 5066 SAP Prioritization Matcher
    printf("[TEST] Validating NATO STANAG 5066 SAP Prioritization Matcher...\n");
    int sap_pri_valid = -1;
    int sap_pri_res = tsfi_mf_nato_verify_sap_priority(0, 14, &sap_pri_valid);
    assert(sap_pri_res == 0);
    assert(sap_pri_valid == 1);
    printf("  [PASS] NATO STANAG 5066 SAP Prioritization Matcher verified.\n");

    // 264. Verify NATO STANAG 5066 Frame Sequence wrap-Around Validator
    printf("[TEST] Validating NATO STANAG 5066 Frame Sequence wrap-Around Validator...\n");
    int frame_seq_valid = -1;
    int frame_seq_res = tsfi_mf_nato_verify_frame_sequence(127, 0, &frame_seq_valid);
    assert(frame_seq_res == 0);
    assert(frame_seq_valid == 1);
    printf("  [PASS] NATO STANAG 5066 Frame Sequence wrap-Around Validator verified.\n");

    // 265. Verify NATO STANAG 5066 HF Modem Data Rate Matcher
    printf("[TEST] Validating NATO STANAG 5066 Data Rate Matcher...\n");
    int rate_valid = -1;
    int rate_res = tsfi_mf_nato_verify_data_rate(2400, &rate_valid);
    assert(rate_res == 0);
    assert(rate_valid == 1);
    printf("  [PASS] NATO STANAG 5066 Data Rate Matcher verified.\n");

    // 266. Verify NATO STANAG 5066 ARQ Frame Retry Limit Checker
    printf("[TEST] Validating NATO STANAG 5066 Retry Limit Checker...\n");
    int retry_valid = -1;
    int retry_res = tsfi_mf_nato_verify_retry_limit(10, &retry_valid);
    assert(retry_res == 0);
    assert(retry_valid == 1);
    printf("  [PASS] NATO STANAG 5066 Retry Limit Checker verified.\n");

    // 267. Verify NATO STANAG 5066 Client Interface Version Matcher
    printf("[TEST] Validating NATO STANAG 5066 Client Interface Version Matcher...\n");
    int version_valid = -1;
    int version_res = tsfi_mf_nato_verify_interface_version(1, &version_valid);
    assert(version_res == 0);
    assert(version_valid == 1);
    printf("  [PASS] NATO STANAG 5066 Client Interface Version Matcher verified.\n");

    // 268. Verify NATO STANAG 5066 Frame Check Sequence (FCS) Type Checker
    printf("[TEST] Validating NATO STANAG 5066 FCS Type Checker...\n");
    int fcs_type_valid = -1;
    int fcs_type_res = tsfi_mf_nato_verify_fcs_type(1, &fcs_type_valid);
    assert(fcs_type_res == 0);
    assert(fcs_type_valid == 1);
    printf("  [PASS] NATO STANAG 5066 FCS Type Checker verified.\n");

    // 269. Verify NATO STANAG 5066 Client Handshake Confirmation Matcher
    printf("[TEST] Validating NATO STANAG 5066 Handshake Confirmation Matcher...\n");
    int handshake_status_valid = -1;
    int handshake_status_res = tsfi_mf_nato_verify_handshake_confirmation(1, &handshake_status_valid);
    assert(handshake_status_res == 0);
    assert(handshake_status_valid == 1);
    printf("  [PASS] NATO STANAG 5066 Handshake Confirmation Matcher verified.\n");

    // 270. Verify NATO STANAG 5066 Burst Packet Segment Count Validator
    printf("[TEST] Validating NATO STANAG 5066 Burst Limit Validator...\n");
    int burst_valid = -1;
    int burst_res = tsfi_mf_nato_verify_burst_limit(64, &burst_valid);
    assert(burst_res == 0);
    assert(burst_valid == 1);
    printf("  [PASS] NATO STANAG 5066 Burst Limit Validator verified.\n");

    // 271. Verify NATO STANAG 5066 Interface Data Flow Direction Matcher
    printf("[TEST] Validating NATO STANAG 5066 Flow Direction Matcher...\n");
    int flow_valid = -1;
    int flow_res = tsfi_mf_nato_verify_flow_direction(1, &flow_valid);
    assert(flow_res == 0);
    assert(flow_valid == 1);
    printf("  [PASS] NATO STANAG 5066 Flow Direction Matcher verified.\n");

    // 272. Verify NATO STANAG 5066 Link-Layer Multi-Scan Selector
    printf("[TEST] Validating NATO STANAG 5066 Multi-Scan Selector...\n");
    int scan_valid = -1;
    int scan_res = tsfi_mf_nato_verify_multiscan_mode(1, &scan_valid);
    assert(scan_res == 0);
    assert(scan_valid == 1);
    printf("  [PASS] NATO STANAG 5066 Multi-Scan Selector verified.\n");

    // 273. Verify NATO STANAG 5066 Interface Data Type Selector
    printf("[TEST] Validating NATO STANAG 5066 Data Type Matcher...\n");
    int type_valid = -1;
    int type_res = tsfi_mf_nato_verify_data_type(1, &type_valid);
    assert(type_res == 0);
    assert(type_valid == 1);
    printf("  [PASS] NATO STANAG 5066 Data Type Matcher verified.\n");

    // 274. Verify NATO STANAG 5066 Link-Layer Compression Mode Validator
    printf("[TEST] Validating NATO STANAG 5066 Compression Mode Validator...\n");
    int compression_valid = -1;
    int compression_res = tsfi_mf_nato_verify_compression_mode(1, &compression_valid);
    assert(compression_res == 0);
    assert(compression_valid == 1);
    printf("  [PASS] NATO STANAG 5066 Compression Mode Validator verified.\n");

    // 275. Verify NATO STANAG 5066 Interface Flow Control State Matcher
    printf("[TEST] Validating NATO STANAG 5066 Flow Control State Matcher...\n");
    int flow_ctrl_valid = -1;
    int flow_ctrl_res = tsfi_mf_nato_verify_flow_control(1, &flow_ctrl_valid);
    assert(flow_ctrl_res == 0);
    assert(flow_ctrl_valid == 1);
    printf("  [PASS] NATO STANAG 5066 Flow Control State Matcher verified.\n");

    // 276. Verify NATO STANAG 5066 Delivery Confirmation Mode Selector
    printf("[TEST] Validating NATO STANAG 5066 Delivery Confirmation Matcher...\n");
    int deliv_confirm_valid = -1;
    int deliv_confirm_res = tsfi_mf_nato_verify_delivery_confirmation(1, &deliv_confirm_valid);
    assert(deliv_confirm_res == 0);
    assert(deliv_confirm_valid == 1);
    printf("  [PASS] NATO STANAG 5066 Delivery Confirmation Matcher verified.\n");

    // 277. Verify NATO STANAG 5066 Interface Backpressure Timeout Validator
    printf("[TEST] Validating NATO STANAG 5066 Backpressure Timeout Validator...\n");
    int backpressure_valid = -1;
    int backpressure_res = tsfi_mf_nato_verify_backpressure_timeout(60, &backpressure_valid);
    assert(backpressure_res == 0);
    assert(backpressure_valid == 1);
    printf("  [PASS] NATO STANAG 5066 Backpressure Timeout Validator verified.\n");

    // 278. Verify NATO STANAG 5066 Link-Layer Max Frame Size Checker
    printf("[TEST] Validating NATO STANAG 5066 Max Frame Size Checker...\n");
    int max_frame_valid = -1;
    int max_frame_res = tsfi_mf_nato_verify_max_frame_size(1024, &max_frame_valid);
    assert(max_frame_res == 0);
    assert(max_frame_valid == 1);
    printf("  [PASS] NATO STANAG 5066 Max Frame Size Checker verified.\n");

    // 279. Verify NATO STANAG 5066 Client Transmit Buffer Size Matcher
    printf("[TEST] Validating NATO STANAG 5066 Client Transmit Buffer Size Matcher...\n");
    int buffer_size_valid = -1;
    int buffer_size_res = tsfi_mf_nato_verify_buffer_size(32768, &buffer_size_valid);
    assert(buffer_size_res == 0);
    assert(buffer_size_valid == 1);
    printf("  [PASS] NATO STANAG 5066 Client Transmit Buffer Size Matcher verified.\n");

    // 280. Verify NATO STANAG 5066 Max Segment Lifetime Checker
    printf("[TEST] Validating NATO STANAG 5066 Max Segment Lifetime Checker...\n");
    int lifetime_valid = -1;
    int lifetime_res = tsfi_mf_nato_verify_segment_lifetime(1800, &lifetime_valid);
    assert(lifetime_res == 0);
    assert(lifetime_valid == 1);
    printf("  [PASS] NATO STANAG 5066 Max Segment Lifetime Checker verified.\n");

    // 281. Verify NATO STANAG 5066 Connection Rejection Reason Matcher
    printf("[TEST] Validating NATO STANAG 5066 Connection Rejection Reason Matcher...\n");
    int reason_valid = -1;
    int reason_res = tsfi_mf_nato_verify_rejection_reason(3, &reason_valid);
    assert(reason_res == 0);
    assert(reason_valid == 1);
    printf("  [PASS] NATO STANAG 5066 Connection Rejection Reason Matcher verified.\n");

    // 282. Verify NATO STANAG 5066 Link-Layer Packet Delay Selector
    printf("[TEST] Validating NATO STANAG 5066 Packet Delay Selector...\n");
    int delay_valid = -1;
    int delay_res = tsfi_mf_nato_verify_packet_delay(60, &delay_valid);
    assert(delay_res == 0);
    assert(delay_valid == 1);
    printf("  [PASS] NATO STANAG 5066 Packet Delay Selector verified.\n");

    // 283. Verify NATO STANAG 5066 Flow Control Buffer High-Water Mark Matcher
    printf("[TEST] Validating NATO STANAG 5066 High-Water Mark Matcher...\n");
    int high_water_valid = -1;
    int high_water_res = tsfi_mf_nato_verify_high_water_mark(80, &high_water_valid);
    assert(high_water_res == 0);
    assert(high_water_valid == 1);
    printf("  [PASS] NATO STANAG 5066 High-Water Mark Matcher verified.\n");

    // 284. Verify NATO STANAG 5066 Max Segment Retransmission Timeout Validator
    printf("[TEST] Validating NATO STANAG 5066 Retransmit Timeout Validator...\n");
    int retransmit_timeout_valid = -1;
    int retransmit_timeout_res = tsfi_mf_nato_verify_retransmit_timeout(300, &retransmit_timeout_valid);
    assert(retransmit_timeout_res == 0);
    assert(retransmit_timeout_valid == 1);
    printf("  [PASS] NATO STANAG 5066 Retransmit Timeout Validator verified.\n");

    // 285. Verify NATO STANAG 5066 Flow Control Buffer Low-Water Mark Matcher
    printf("[TEST] Validating NATO STANAG 5066 Low-Water Mark Matcher...\n");
    int low_water_valid = -1;
    int low_water_res = tsfi_mf_nato_verify_low_water_mark(30, &low_water_valid);
    assert(low_water_res == 0);
    assert(low_water_valid == 1);
    printf("  [PASS] NATO STANAG 5066 Low-Water Mark Matcher verified.\n");

    // 286. Verify NATO STANAG 5066 Multi-Scan Frame Count Validator
    printf("[TEST] Validating NATO STANAG 5066 Multi-Scan Frame Count Validator...\n");
    int multiscan_count_valid = -1;
    int multiscan_count_res = tsfi_mf_nato_verify_multiscan_frame_count(16, &multiscan_count_valid);
    assert(multiscan_count_res == 0);
    assert(multiscan_count_valid == 1);
    printf("  [PASS] NATO STANAG 5066 Multi-Scan Frame Count Validator verified.\n");

    // 287. Verify NATO STANAG 5066 Client Idle Timeout Matcher
    printf("[TEST] Validating NATO STANAG 5066 Client Idle Timeout Matcher...\n");
    int idle_timeout_valid = -1;
    int idle_timeout_res = tsfi_mf_nato_verify_idle_timeout(300, &idle_timeout_valid);
    assert(idle_timeout_res == 0);
    assert(idle_timeout_valid == 1);
    printf("  [PASS] NATO STANAG 5066 Client Idle Timeout Matcher verified.\n");

    // 288. Verify NATO STANAG 5066 Transmit Delay Interval Validator
    printf("[TEST] Validating NATO STANAG 5066 Transmit Delay Interval Validator...\n");
    int delay_interval_valid = -1;
    int delay_interval_res = tsfi_mf_nato_verify_delay_interval(5, &delay_interval_valid);
    assert(delay_interval_res == 0);
    assert(delay_interval_valid == 1);
    printf("  [PASS] NATO STANAG 5066 Transmit Delay Interval Validator verified.\n");

    // 289. Verify NATO STANAG 5066 Flow Control Buffer Low-Water Mark Mode Selector
    printf("[TEST] Validating NATO STANAG 5066 Low-Water Mode Selector...\n");
    int low_water_mode_valid = -1;
    int low_water_mode_res = tsfi_mf_nato_verify_low_water_mode(1, &low_water_mode_valid);
    assert(low_water_mode_res == 0);
    assert(low_water_mode_valid == 1);
    printf("  [PASS] NATO STANAG 5066 Low-Water Mode Selector verified.\n");

    // 290. Verify NATO STANAG 5066 Dynamic Segment Lifetime Adaptation Selector
    printf("[TEST] Validating NATO STANAG 5066 Lifetime Adaptation Selector...\n");
    int adapt_mode_valid = -1;
    int adapt_mode_res = tsfi_mf_nato_verify_adaptation_mode(2, &adapt_mode_valid);
    assert(adapt_mode_res == 0);
    assert(adapt_mode_valid == 1);
    printf("  [PASS] NATO STANAG 5066 Lifetime Adaptation Selector verified.\n");

    // 291. Verify NATO Link-Layer Subnetwork Profile Identifier Validator
    printf("[TEST] Validating NATO Subnetwork Profile Identifier Validator...\n");
    int profile_valid = -1;
    int profile_res = tsfi_mf_nato_verify_subnetwork_profile(1, &profile_valid);
    assert(profile_res == 0);
    assert(profile_valid == 1);
    printf("  [PASS] NATO Subnetwork Profile Identifier Validator verified.\n");

    // 292. Verify NATO Link-Layer Multi-Scan Retry Delay Matcher
    printf("[TEST] Validating NATO Multi-Scan Retry Delay Matcher...\n");
    int retry_delay_valid = -1;
    int retry_delay_res = tsfi_mf_nato_verify_retry_delay(200, &retry_delay_valid);
    assert(retry_delay_res == 0);
    assert(retry_delay_valid == 1);
    printf("  [PASS] NATO Multi-Scan Retry Delay Matcher verified.\n");

    // 293. Verify NATO Link-Layer Subnetwork Profile Override Flag Matcher
    printf("[TEST] Validating NATO Subnetwork Profile Override Flag Matcher...\n");
    int override_valid = -1;
    int override_res = tsfi_mf_nato_verify_profile_override(1, &override_valid);
    assert(override_res == 0);
    assert(override_valid == 1);
    printf("  [PASS] NATO Subnetwork Profile Override Flag Matcher verified.\n");

    // 294. Verify NATO Link-Layer Multi-Scan Collision Backoff Multiplier Validator
    printf("[TEST] Validating NATO Multi-Scan Collision Backoff Multiplier Validator...\n");
    int backoff_multiplier_valid = -1;
    int backoff_multiplier_res = tsfi_mf_nato_verify_backoff_multiplier(2, &backoff_multiplier_valid);
    assert(backoff_multiplier_res == 0);
    assert(backoff_multiplier_valid == 1);
    printf("  [PASS] NATO Multi-Scan Collision Backoff Multiplier Validator verified.\n");

    // 295. Verify NATO Link-Layer Broadcast Multi-Cast Address Validator
    printf("[TEST] Validating NATO Broadcast Address Validator...\n");
    int broadcast_address_valid = -1;
    int broadcast_address_res = tsfi_mf_nato_verify_broadcast_address(0x3FFFFFF, &broadcast_address_valid);
    assert(broadcast_address_res == 0);
    assert(broadcast_address_valid == 1);
    printf("  [PASS] NATO Broadcast Address Validator verified.\n");

    // 296. Verify NATO Link-Layer Broadcast Window Constraint Matcher
    printf("[TEST] Validating NATO Broadcast Window Constraint Matcher...\n");
    int broadcast_window_valid = -1;
    int broadcast_window_res = tsfi_mf_nato_verify_broadcast_window(0, 0, &broadcast_window_valid);
    assert(broadcast_window_res == 0);
    assert(broadcast_window_valid == 1);
    printf("  [PASS] NATO Broadcast Window Constraint Matcher verified.\n");

    // 297. Verify NATO Link-Layer Broadcast Collision Backoff Limit Validator
    printf("[TEST] Validating NATO Broadcast Collision Backoff Limit Validator...\n");
    int backoff_limit_valid = -1;
    int backoff_limit_res = tsfi_mf_nato_verify_broadcast_backoff_limit(8, &backoff_limit_valid);
    assert(backoff_limit_res == 0);
    assert(backoff_limit_valid == 1);
    printf("  [PASS] NATO Broadcast Collision Backoff Limit Validator verified.\n");

    // 298. Verify NATO Link-Layer Broadcast Queue Capacity Checker
    printf("[TEST] Validating NATO Broadcast Queue Capacity Checker...\n");
    int queue_capacity_valid = -1;
    int queue_capacity_res = tsfi_mf_nato_verify_broadcast_queue_capacity(100, &queue_capacity_valid);
    assert(queue_capacity_res == 0);
    assert(queue_capacity_valid == 1);
    printf("  [PASS] NATO Broadcast Queue Capacity Checker verified.\n");

    // 299. Verify NATO Link-Layer Broadcast Collision Backoff Algorithm Selector
    printf("[TEST] Validating NATO Broadcast Backoff Algorithm Selector...\n");
    int backoff_algo_valid = -1;
    int backoff_algo_res = tsfi_mf_nato_verify_backoff_algorithm(1, &backoff_algo_valid);
    assert(backoff_algo_res == 0);
    assert(backoff_algo_valid == 1);
    printf("  [PASS] NATO Broadcast Backoff Algorithm Selector verified.\n");

    // 300. Verify NATO Link-Layer Multi-Scan Channel Busy Threshold Matcher
    printf("[TEST] Validating NATO Channel Busy Threshold Matcher...\n");
    int busy_threshold_valid = -1;
    int busy_threshold_res = tsfi_mf_nato_verify_channel_busy_threshold(-85, &busy_threshold_valid);
    assert(busy_threshold_res == 0);
    assert(busy_threshold_valid == 1);
    printf("  [PASS] NATO Channel Busy Threshold Matcher verified.\n");

    // 301. Verify NATO Link-Layer Multi-Scan Channel Free RSSI Threshold Matcher
    printf("[TEST] Validating NATO Channel Free Threshold Matcher...\n");
    int free_threshold_valid = -1;
    int free_threshold_res = tsfi_mf_nato_verify_channel_free_threshold(-95, &free_threshold_valid);
    assert(free_threshold_res == 0);
    assert(free_threshold_valid == 1);
    printf("  [PASS] NATO Channel Free Threshold Matcher verified.\n");

    // 302. Verify NATO Link-Layer Broadcast Minimum Repeat Count Validator
    printf("[TEST] Validating NATO Broadcast Repeats Validator...\n");
    int repeats_valid = -1;
    int repeats_res = tsfi_mf_nato_verify_broadcast_repeats(3, &repeats_valid);
    assert(repeats_res == 0);
    assert(repeats_valid == 1);
    printf("  [PASS] NATO Broadcast Repeats Validator verified.\n");

    return 0;
}
