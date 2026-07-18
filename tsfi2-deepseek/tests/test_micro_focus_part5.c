#include <stdio.h>
#include <assert.h>
#include "tsfi_cade_imf.h"

int run_nato_stanag_tests_part5(void) {
    // 416. Verify NATO Link-Layer Broadcast Collision Backoff Slot Alignment Boundary Source Offset Range Limit Value Margin Range Option Selector Option Margin Range Option Selector Option Margin Limit Range Option Selector Margin Range Option Selector Margin Range Option Selector Margin Range Option Matcher
    printf("[TEST] Validating NATO Broadcast Backoff Slot Boundary Source Offset Range Limit Value Margin Range Option Selector Option Margin Range Option Selector Option Margin Limit Range Option Selector Margin Range Option Selector Margin Range Option Selector Margin Range Option Matcher...\n");
    int slot_boundary_source_offset_range_limit_val_margin_range_option_selector_option_margin_range_option_selector_option_margin_limit_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_valid = -1;
    int slot_boundary_source_offset_range_limit_val_margin_range_option_selector_option_margin_range_option_selector_option_margin_limit_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_res = tsfi_mf_nato_verify_slot_boundary_source_offset_range_limit_val_margin_range_option_selector_option_margin_range_option_selector_option_margin_limit_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option(2, &slot_boundary_source_offset_range_limit_val_margin_range_option_selector_option_margin_range_option_selector_option_margin_limit_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_valid);
    assert(slot_boundary_source_offset_range_limit_val_margin_range_option_selector_option_margin_range_option_selector_option_margin_limit_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_res == 0);
    assert(slot_boundary_source_offset_range_limit_val_margin_range_option_selector_option_margin_range_option_selector_option_margin_limit_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_valid == 1);
    printf("  [PASS] NATO Broadcast Backoff Slot Boundary Source Offset Range Limit Value Margin Range Option Selector Option Margin Range Option Selector Option Margin Limit Range Option Selector Margin Range Option Selector Margin Range Option Selector Margin Range Option Matcher verified.\n");

    // 417. Verify NATO Link-Layer Broadcast Channel Busy Hysteresis RSSI Quiet Sample Threshold Limit Margin Range Option Selector Margin Range Option Selector Margin Range Option Selector Margin Range Option Selector Margin Range Option Selector Margin Range Option Selector Margin Range Matcher
    printf("[TEST] Validating NATO Broadcast Busy RSSI Quiet Sample Threshold Limit Margin Range Option Selector Margin Range Option Selector Margin Range Option Selector Margin Range Option Selector Margin Range Option Selector Margin Range Option Selector Margin Range Matcher...\n");
    int rssi_quiet_sample_threshold_limit_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_valid = -1;
    int rssi_quiet_sample_threshold_limit_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_res = tsfi_mf_nato_verify_rssi_quiet_sample_threshold_limit_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range(4, &rssi_quiet_sample_threshold_limit_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_valid);
    assert(rssi_quiet_sample_threshold_limit_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_res == 0);
    assert(rssi_quiet_sample_threshold_limit_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_valid == 1);
    printf("  [PASS] NATO Broadcast Busy RSSI Quiet Sample Threshold Limit Margin Range Option Selector Margin Range Option Selector Margin Range Option Selector Margin Range Option Selector Margin Range Option Selector Margin Range Option Selector Margin Range Matcher verified.\n");

    // 418. Verify NATO Link-Layer Broadcast Collision Backoff Slot Alignment Boundary Source Offset Range Limit Value Margin Range Option Selector Option Margin Range Option Selector Option Margin Limit Range Option Selector Margin Range Option Selector Margin Range Option Selector Margin Range Option Selector Margin Matcher
    printf("[TEST] Validating NATO Broadcast Backoff Slot Boundary Source Offset Range Limit Value Margin Range Option Selector Option Margin Range Option Selector Option Margin Limit Range Option Selector Margin Range Option Selector Margin Range Option Selector Margin Range Option Selector Margin Matcher...\n");
    int slot_boundary_source_offset_range_limit_val_margin_range_option_selector_option_margin_range_option_selector_option_margin_limit_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_valid = -1;
    int slot_boundary_source_offset_range_limit_val_margin_range_option_selector_option_margin_range_option_selector_option_margin_limit_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_res = tsfi_mf_nato_verify_slot_boundary_source_offset_range_limit_val_margin_range_option_selector_option_margin_range_option_selector_option_margin_limit_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin(20, &slot_boundary_source_offset_range_limit_val_margin_range_option_selector_option_margin_range_option_selector_option_margin_limit_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_valid);
    assert(slot_boundary_source_offset_range_limit_val_margin_range_option_selector_option_margin_range_option_selector_option_margin_limit_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_res == 0);
    assert(slot_boundary_source_offset_range_limit_val_margin_range_option_selector_option_margin_range_option_selector_option_margin_limit_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_valid == 1);
    printf("  [PASS] NATO Broadcast Backoff Slot Boundary Source Offset Range Limit Value Margin Range Option Selector Option Margin Range Option Selector Option Margin Limit Range Option Selector Margin Range Option Selector Margin Range Option Selector Margin Range Option Selector Margin Matcher verified.\n");

    // 419. Verify NATO Link-Layer Broadcast Channel Busy Hysteresis RSSI Quiet Sample Threshold Limit Margin Range Option Selector Margin Range Option Selector Margin Range Option Selector Margin Range Option Selector Margin Range Option Selector Margin Range Option Selector Margin Range Option Selector Matcher
    printf("[TEST] Validating NATO Broadcast Busy RSSI Quiet Sample Threshold Limit Margin Range Option Selector Margin Range Option Selector Margin Range Option Selector Margin Range Option Selector Margin Range Option Selector Margin Range Option Selector Margin Range Option Selector Matcher...\n");
    int rssi_quiet_sample_threshold_limit_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_valid = -1;
    int rssi_quiet_sample_threshold_limit_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_res = tsfi_mf_nato_verify_rssi_quiet_sample_threshold_limit_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector(4, &rssi_quiet_sample_threshold_limit_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_valid);
    assert(rssi_quiet_sample_threshold_limit_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_res == 0);
    assert(rssi_quiet_sample_threshold_limit_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_valid == 1);
    printf("  [PASS] NATO Broadcast Busy RSSI Quiet Sample Threshold Limit Margin Range Option Selector Margin Range Option Selector Margin Range Option Selector Margin Range Option Selector Margin Range Option Selector Margin Range Option Selector Margin Range Option Selector Matcher verified.\n");

    // 420. Verify NATO Link-Layer Broadcast Collision Backoff Slot Alignment Boundary Source Offset Range Limit Value Margin Range Option Selector Option Margin Range Option Selector Option Margin Limit Range Option Selector Margin Range Option Selector Margin Range Option Selector Margin Range Option Selector Margin Range Matcher
    printf("[TEST] Validating NATO Broadcast Backoff Slot Boundary Source Offset Range Limit Value Margin Range Option Selector Option Margin Range Option Selector Option Margin Limit Range Option Selector Margin Range Option Selector Margin Range Option Selector Margin Range Option Selector Margin Range Matcher...\n");
    int slot_boundary_source_offset_range_limit_val_margin_range_option_selector_option_margin_range_option_selector_option_margin_limit_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_valid = -1;
    int slot_boundary_source_offset_range_limit_val_margin_range_option_selector_option_margin_range_option_selector_option_margin_limit_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_res = tsfi_mf_nato_verify_slot_boundary_source_offset_range_limit_val_margin_range_option_selector_option_margin_range_option_selector_option_margin_limit_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range(20, &slot_boundary_source_offset_range_limit_val_margin_range_option_selector_option_margin_range_option_selector_option_margin_limit_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_valid);
    assert(slot_boundary_source_offset_range_limit_val_margin_range_option_selector_option_margin_range_option_selector_option_margin_limit_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_res == 0);
    assert(slot_boundary_source_offset_range_limit_val_margin_range_option_selector_option_margin_range_option_selector_option_margin_limit_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_valid == 1);
    printf("  [PASS] NATO Broadcast Backoff Slot Boundary Source Offset Range Limit Value Margin Range Option Selector Option Margin Range Option Selector Option Margin Limit Range Option Selector Margin Range Option Selector Margin Range Option Selector Margin Range Option Selector Margin Range Matcher verified.\n");

    // 421. Verify NATO Link-Layer Broadcast Channel Busy Hysteresis RSSI Quiet Sample Threshold Limit Margin Range Option Selector Margin Range Option Selector Margin Range Option Selector Margin Range Option Selector Margin Range Option Selector Margin Range Option Selector Margin Range Option Selector Margin Matcher
    printf("[TEST] Validating NATO Broadcast Busy RSSI Quiet Sample Threshold Limit Margin Range Option Selector Margin Range Option Selector Margin Range Option Selector Margin Range Option Selector Margin Range Option Selector Margin Range Option Selector Margin Range Option Selector Margin Matcher...\n");
    int rssi_quiet_sample_threshold_limit_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_valid = -1;
    int rssi_quiet_sample_threshold_limit_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_res = tsfi_mf_nato_verify_rssi_quiet_sample_threshold_limit_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin(4, &rssi_quiet_sample_threshold_limit_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_valid);
    assert(rssi_quiet_sample_threshold_limit_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_res == 0);
    assert(rssi_quiet_sample_threshold_limit_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_valid == 1);
    printf("  [PASS] NATO Broadcast Busy RSSI Quiet Sample Threshold Limit Margin Range Option Selector Margin Range Option Selector Margin Range Option Selector Margin Range Option Selector Margin Range Option Selector Margin Range Option Selector Margin Range Option Selector Margin Matcher verified.\n");

    // 422. Verify NATO Link-Layer Broadcast Collision Backoff Slot Alignment Boundary Source Offset Range Limit Value Margin Range Option Selector Option Margin Range Option Selector Option Margin Limit Range Option Selector Margin Range Option Selector Margin Range Option Selector Margin Range Option Selector Margin Range Option Matcher
    printf("[TEST] Validating NATO Broadcast Backoff Slot Boundary Source Offset Range Limit Value Margin Range Option Selector Option Margin Range Option Selector Option Margin Limit Range Option Selector Margin Range Option Selector Margin Range Option Selector Margin Range Option Selector Margin Range Option Matcher...\n");
    int slot_boundary_source_offset_range_limit_val_margin_range_option_selector_option_margin_range_option_selector_option_margin_limit_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_valid = -1;
    int slot_boundary_source_offset_range_limit_val_margin_range_option_selector_option_margin_range_option_selector_option_margin_limit_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_res = tsfi_mf_nato_verify_slot_boundary_source_offset_range_limit_val_margin_range_option_selector_option_margin_range_option_selector_option_margin_limit_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option(2, &slot_boundary_source_offset_range_limit_val_margin_range_option_selector_option_margin_range_option_selector_option_margin_limit_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_valid);
    assert(slot_boundary_source_offset_range_limit_val_margin_range_option_selector_option_margin_range_option_selector_option_margin_limit_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_res == 0);
    assert(slot_boundary_source_offset_range_limit_val_margin_range_option_selector_option_margin_range_option_selector_option_margin_limit_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_valid == 1);
    printf("  [PASS] NATO Broadcast Backoff Slot Boundary Source Offset Range Limit Value Margin Range Option Selector Option Margin Range Option Selector Option Margin Limit Range Option Selector Margin Range Option Selector Margin Range Option Selector Margin Range Option Selector Margin Range Option Matcher verified.\n");

    // 423. Verify NATO Link-Layer Broadcast Channel Busy Hysteresis RSSI Quiet Sample Threshold Limit Margin Range Option Selector Margin Range Option Selector Margin Range Option Selector Margin Range Option Selector Margin Range Option Selector Margin Range Option Selector Margin Range Option Selector Margin Range Matcher
    printf("[TEST] Validating NATO Broadcast Busy RSSI Quiet Sample Threshold Limit Margin Range Option Selector Margin Range Option Selector Margin Range Option Selector Margin Range Option Selector Margin Range Option Selector Margin Range Option Selector Margin Range Option Selector Margin Range Matcher...\n");
    int rssi_quiet_sample_threshold_limit_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_valid = -1;
    int rssi_quiet_sample_threshold_limit_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_res = tsfi_mf_nato_verify_rssi_quiet_sample_threshold_limit_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range(4, &rssi_quiet_sample_threshold_limit_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_valid);
    assert(rssi_quiet_sample_threshold_limit_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_res == 0);
    assert(rssi_quiet_sample_threshold_limit_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_valid == 1);
    printf("  [PASS] NATO Broadcast Busy RSSI Quiet Sample Threshold Limit Margin Range Option Selector Margin Range Option Selector Margin Range Option Selector Margin Range Option Selector Margin Range Option Selector Margin Range Option Selector Margin Range Option Selector Margin Range Matcher verified.\n");

    // 424. Verify NATO Link-Layer Broadcast Collision Backoff Slot Alignment Boundary Source Offset Range Limit Value Margin Range Option Selector Option Margin Range Option Selector Option Margin Limit Range Option Selector Margin Range Option Selector Margin Range Option Selector Margin Range Option Selector Margin Range Option Selector Matcher
    printf("[TEST] Validating NATO Broadcast Backoff Slot Boundary Source Offset Range Limit Value Margin Range Option Selector Option Margin Range Option Selector Option Margin Limit Range Option Selector Margin Range Option Selector Margin Range Option Selector Margin Range Option Selector Margin Range Option Selector Matcher...\n");
    int slot_boundary_source_offset_range_limit_val_margin_range_option_selector_option_margin_range_option_selector_option_margin_limit_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_valid = -1;
    int slot_boundary_source_offset_range_limit_val_margin_range_option_selector_option_margin_range_option_selector_option_margin_limit_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_res = tsfi_mf_nato_verify_slot_boundary_source_offset_range_limit_val_margin_range_option_selector_option_margin_range_option_selector_option_margin_limit_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector(4, &slot_boundary_source_offset_range_limit_val_margin_range_option_selector_option_margin_range_option_selector_option_margin_limit_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_valid);
    assert(slot_boundary_source_offset_range_limit_val_margin_range_option_selector_option_margin_range_option_selector_option_margin_limit_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_res == 0);
    assert(slot_boundary_source_offset_range_limit_val_margin_range_option_selector_option_margin_range_option_selector_option_margin_limit_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_valid == 1);
    printf("  [PASS] NATO Broadcast Backoff Slot Boundary Source Offset Range Limit Value Margin Range Option Selector Option Margin Range Option Selector Option Margin Limit Range Option Selector Margin Range Option Selector Margin Range Option Selector Margin Range Option Selector Margin Range Option Selector Matcher verified.\n");

    // 425. Verify NATO Link-Layer Broadcast Channel Busy Hysteresis RSSI Quiet Sample Threshold Limit Margin Range Option Selector Margin Range Option Selector Margin Range Option Selector Margin Range Option Selector Margin Range Option Selector Margin Range Option Selector Margin Range Option Selector Margin Range Option Matcher
    printf("[TEST] Validating NATO Broadcast Busy RSSI Quiet Sample Threshold Limit Margin Range Option Selector Margin Range Option Selector Margin Range Option Selector Margin Range Option Selector Margin Range Option Selector Margin Range Option Selector Margin Range Option Selector Margin Range Option Matcher...\n");
    int rssi_quiet_sample_threshold_limit_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_valid = -1;
    int rssi_quiet_sample_threshold_limit_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_res = tsfi_mf_nato_verify_rssi_quiet_sample_threshold_limit_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option(1, &rssi_quiet_sample_threshold_limit_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_valid);
    assert(rssi_quiet_sample_threshold_limit_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_res == 0);
    assert(rssi_quiet_sample_threshold_limit_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_valid == 1);
    printf("  [PASS] NATO Broadcast Busy RSSI Quiet Sample Threshold Limit Margin Range Option Selector Margin Range Option Selector Margin Range Option Selector Margin Range Option Selector Margin Range Option Selector Margin Range Option Selector Margin Range Option Selector Margin Range Option Matcher verified.\n");

    // 426. Verify NATO Link-Layer Broadcast Collision Backoff Slot Alignment Boundary Source Offset Range Limit Value Margin Range Option Selector Option Margin Range Option Selector Option Margin Limit Range Option Selector Margin Range Option Selector Margin Range Option Selector Margin Range Option Selector Margin Range Option Selector Margin Matcher
    printf("[TEST] Validating NATO Broadcast Backoff Slot Boundary Source Offset Range Limit Value Margin Range Option Selector Option Margin Range Option Selector Option Margin Limit Range Option Selector Margin Range Option Selector Margin Range Option Selector Margin Range Option Selector Margin Range Option Selector Margin Matcher...\n");
    int slot_boundary_source_offset_range_limit_val_margin_range_option_selector_option_margin_range_option_selector_option_margin_limit_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_valid = -1;
    int slot_boundary_source_offset_range_limit_val_margin_range_option_selector_option_margin_range_option_selector_option_margin_limit_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_res = tsfi_mf_nato_verify_slot_boundary_source_offset_range_limit_val_margin_range_option_selector_option_margin_range_option_selector_option_margin_limit_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin(20, &slot_boundary_source_offset_range_limit_val_margin_range_option_selector_option_margin_range_option_selector_option_margin_limit_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_valid);
    assert(slot_boundary_source_offset_range_limit_val_margin_range_option_selector_option_margin_range_option_selector_option_margin_limit_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_res == 0);
    assert(slot_boundary_source_offset_range_limit_val_margin_range_option_selector_option_margin_range_option_selector_option_margin_limit_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_valid == 1);
    printf("  [PASS] NATO Broadcast Backoff Slot Boundary Source Offset Range Limit Value Margin Range Option Selector Option Margin Range Option Selector Option Margin Limit Range Option Selector Margin Range Option Selector Margin Range Option Selector Margin Range Option Selector Margin Range Option Selector Margin Matcher verified.\n");

    // 427. Verify NATO Link-Layer Broadcast Channel Busy Hysteresis RSSI Quiet Sample Threshold Limit Margin Range Option Selector Margin Range Option Selector Margin Range Option Selector Margin Range Option Selector Margin Range Option Selector Margin Range Option Selector Margin Range Option Selector Margin Range Option Selector Matcher
    printf("[TEST] Validating NATO Broadcast Busy RSSI Quiet Sample Threshold Limit Margin Range Option Selector Margin Range Option Selector Margin Range Option Selector Margin Range Option Selector Margin Range Option Selector Margin Range Option Selector Margin Range Option Selector Margin Range Option Selector Matcher...\n");
    int rssi_quiet_sample_threshold_limit_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_valid = -1;
    int rssi_quiet_sample_threshold_limit_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_res = tsfi_mf_nato_verify_rssi_quiet_sample_threshold_limit_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector(4, &rssi_quiet_sample_threshold_limit_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_valid);
    assert(rssi_quiet_sample_threshold_limit_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_res == 0);
    assert(rssi_quiet_sample_threshold_limit_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_margin_range_option_selector_valid == 1);
    // Verify SIS Flow Control state verifier
    printf("[TEST] Validating NATO SIS Flow Control...\n");
    int sis_fc_valid = -1;
    int sis_fc_res = tsfi_mf_nato_verify_sis_flow_control(2, &sis_fc_valid); // Blocked is valid
    assert(sis_fc_res == 0);
    assert(sis_fc_valid == 1);
    tsfi_mf_nato_verify_sis_flow_control(5, &sis_fc_valid); // 5 is invalid
    assert(sis_fc_valid == 0);
    printf("  [PASS] SIS Flow Control verified.\n");

    // Verify SIS SAP Allocation verifier
    printf("[TEST] Validating NATO SIS SAP Allocation...\n");
    int sis_sap_valid = -1;
    int sis_sap_res = tsfi_mf_nato_verify_sis_sap_allocation(8, &sis_sap_valid); // SAP 8 is valid
    assert(sis_sap_res == 0);
    assert(sis_sap_valid == 1);
    tsfi_mf_nato_verify_sis_sap_allocation(20, &sis_sap_valid); // SAP 20 is invalid
    assert(sis_sap_valid == 0);
    printf("  [PASS] SIS SAP Allocation verified.\n");

    // Verify FEC Code Rate verifier
    printf("[TEST] Validating NATO FEC Code Rate...\n");
    int fec_rate_valid = -1;
    int fec_rate_res = tsfi_mf_nato_verify_fec_code_rate(75, &fec_rate_valid); // 3/4 rate is valid
    assert(fec_rate_res == 0);
    assert(fec_rate_valid == 1);
    tsfi_mf_nato_verify_fec_code_rate(60, &fec_rate_valid); // 60% rate is invalid
    assert(fec_rate_valid == 0);
    printf("  [PASS] FEC Code Rate verified.\n");

    // Verify FEC Block Size verifier
    printf("[TEST] Validating NATO FEC Block Size...\n");
    int fec_size_valid = -1;
    int fec_size_res = tsfi_mf_nato_verify_fec_block_size(512, &fec_size_valid); // 512 bytes is valid
    assert(fec_size_res == 0);
    assert(fec_size_valid == 1);
    tsfi_mf_nato_verify_fec_block_size(300, &fec_size_valid); // 300 bytes is invalid
    assert(fec_size_valid == 0);
    // Verify SIS Primitive Parser
    printf("[TEST] Validating NATO SIS Primitive Parser...\n");
    uint8_t bind_data[] = {0x01, 0x00, 0x01, 0x02};
    int parsed_type = -1;
    int parsed_valid = -1;
    int parse_res = tsfi_mf_nato_parse_sis_primitive(bind_data, sizeof(bind_data), &parsed_type, &parsed_valid);
    assert(parse_res == 0);
    assert(parsed_type == 0x01);
    assert(parsed_valid == 1);

    uint8_t short_unbind[] = {0x02};
    parse_res = tsfi_mf_nato_parse_sis_primitive(short_unbind, sizeof(short_unbind), &parsed_type, &parsed_valid);
    assert(parse_res == 0);
    assert(parsed_type == 0x02);
    assert(parsed_valid == 0); // Need 2 bytes
    printf("  [PASS] SIS Primitive Parser verified.\n");

    // Verify D_PDU Encoder & Checksum
    printf("[TEST] Validating NATO D_PDU Encoder...\n");
    uint8_t payload[] = {'H', 'e', 'l', 'l', 'o'};
    uint8_t frame[64];
    size_t frame_size = 0;
    int encode_res = tsfi_mf_nato_encode_d_pdu(0x00, payload, sizeof(payload), frame, &frame_size);
    assert(encode_res == 0);
    assert(frame_size == 10); // 3 (header) + 5 (payload) + 2 (CRC)
    assert(frame[0] == 0x00);
    assert(frame[1] == 0x00);
    assert(frame[2] == 0x05);
    // Verify CRC CCITT-FALSE for encoded frame:
    // Header + payload is frame[0] to frame[7]
    uint16_t verify_crc = 0xFFFF;
    for (size_t i = 0; i < 8; i++) {
        verify_crc ^= (uint16_t)frame[i] << 8;
        for (int j = 0; j < 8; j++) {
            if (verify_crc & 0x8000) {
                verify_crc = (verify_crc << 1) ^ 0x1021;
            } else {
                verify_crc <<= 1;
            }
        }
    }
    assert(frame[8] == ((verify_crc >> 8) & 0xFF));
    assert(frame[9] == (verify_crc & 0xFF));
    // Verify Selective Repeat ARQ State Machine
    printf("[TEST] Validating NATO Selective Repeat ARQ...\n");
    int next_exp = 5;
    int arq_valid = -1;
    // Receive expected in-order frame (5) -> valid, increments next_expected
    int arq_res = tsfi_mf_nato_arq_update(0, 5, 8, &next_exp, &arq_valid);
    assert(arq_res == 0);
    assert(arq_valid == 1);
    assert(next_exp == 6);
    // Receive out-of-order frame (8) within window [6, 13] -> valid, does not increment next_expected
    tsfi_mf_nato_arq_update(0, 8, 8, &next_exp, &arq_valid);
    assert(arq_valid == 1);
    assert(next_exp == 6);
    // Receive frame (20) outside window -> invalid
    tsfi_mf_nato_arq_update(0, 20, 8, &next_exp, &arq_valid);
    assert(arq_valid == 0);
    printf("  [PASS] Selective Repeat ARQ verified.\n");

    // Verify U_PDU Header Encoder
    printf("[TEST] Validating NATO U_PDU Header Encoder...\n");
    uint8_t hdr_buf[8];
    size_t hdr_size = 0;
    int hdr_res = tsfi_mf_nato_encode_u_pdu_header(3, 4, 7, 1, hdr_buf, &hdr_size);
    assert(hdr_res == 0);
    assert(hdr_size == 2);
    assert(hdr_buf[0] == 0x34); // (3 << 4) | 4
    assert(hdr_buf[1] == 0x87); // 7 | 0x80
    printf("  [PASS] U_PDU Header Encoder verified.\n");

    return 0;
}

