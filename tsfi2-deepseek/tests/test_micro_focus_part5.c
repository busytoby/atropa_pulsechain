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
    // Verify D_PDU Decoder
    printf("[TEST] Validating NATO D_PDU Decoder...\n");
    uint8_t enc_frame[64];
    size_t enc_size = 0;
    uint8_t payload_in[] = {'P', 'a', 's', 's'};
    tsfi_mf_nato_encode_d_pdu(0x02, payload_in, sizeof(payload_in), enc_frame, &enc_size);
    
    int dec_type = -1;
    uint8_t dec_payload[64];
    size_t dec_pay_size = 0;
    int dec_valid = -1;
    int dec_res = tsfi_mf_nato_decode_d_pdu(enc_frame, enc_size, &dec_type, dec_payload, &dec_pay_size, &dec_valid);
    assert(dec_res == 0);
    assert(dec_valid == 1);
    assert(dec_type == 0x02);
    assert(dec_pay_size == sizeof(payload_in));
    assert(dec_payload[0] == 'P');
    
    // Test corrupt frame
    enc_frame[4] ^= 0xFF; // flip bit in payload
    tsfi_mf_nato_decode_d_pdu(enc_frame, enc_size, &dec_type, dec_payload, &dec_pay_size, &dec_valid);
    assert(dec_valid == 0); // CRC mismatch
    printf("  [PASS] D_PDU Decoder verified.\n");

    // Verify Address Encoder/Decoder
    printf("[TEST] Validating NATO Address Encoder/Decoder...\n");
    uint8_t addr_byte = 0;
    int addr_res = tsfi_mf_nato_encode_address(5, 10, &addr_byte);
    assert(addr_res == 0);
    assert(addr_byte == 0x5A); // (5 << 4) | 10
    
    int node_out = -1;
    int sub_node_out = -1;
    int dec_addr_res = tsfi_mf_nato_decode_address(0x5A, &node_out, &sub_node_out);
    assert(dec_addr_res == 0);
    assert(node_out == 5);
    assert(sub_node_out == 10);
    // Verify EOT Handshake
    printf("[TEST] Validating NATO EOT Handshake...\n");
    uint8_t eot_frame[8];
    size_t eot_size = 0;
    int eot_res = tsfi_mf_nato_encode_eot(2, eot_frame, &eot_size); // reason 2
    assert(eot_res == 0);
    assert(eot_size == 2);
    assert(eot_frame[0] == 0x0F);
    assert(eot_frame[1] == 2);

    int dec_reason = -1;
    int eot_valid = -1;
    eot_res = tsfi_mf_nato_decode_eot(eot_frame, eot_size, &dec_reason, &eot_valid);
    assert(eot_res == 0);
    assert(eot_valid == 1);
    assert(dec_reason == 2);
    printf("  [PASS] EOT Handshake verified.\n");

    // Verify Bind Confirmation
    printf("[TEST] Validating NATO Bind Confirmation...\n");
    uint8_t bind_conf[8];
    size_t bind_conf_size = 0;
    int conf_res = tsfi_mf_nato_generate_bind_confirm(0, 16, bind_conf, &bind_conf_size); // Success, 16 SAPs
    assert(conf_res == 0);
    assert(bind_conf_size == 3);
    assert(bind_conf[0] == 0x81);
    assert(bind_conf[1] == 0);
    assert(bind_conf[2] == 16);
    // Verify Physical Link State Machine
    printf("[TEST] Validating NATO Physical Link State Machine...\n");
    int state = 0; // Disconnected
    int valid = -1;
    int link_res = tsfi_mf_nato_phy_link_update(0, &state, &valid); // CONNECT_REQ
    assert(link_res == 0);
    assert(valid == 1);
    assert(state == 1); // Connecting

    tsfi_mf_nato_phy_link_update(2, &state, &valid); // DISCONNECT_REQ (invalid from Connecting)
    assert(valid == 0);
    assert(state == 1);

    tsfi_mf_nato_phy_link_update(1, &state, &valid); // CONNECT_CONF
    assert(valid == 1);
    assert(state == 2); // Connected
    printf("  [PASS] Physical Link State Machine verified.\n");

    // Verify Segment Bounds Validation
    printf("[TEST] Validating NATO Segment Bounds...\n");
    int seg_valid = -1;
    int bounds_res = tsfi_mf_nato_verify_segment_bounds(0, 100, 300, 0, &seg_valid); // Segment 0..100 of 300 (not last) -> valid
    assert(bounds_res == 0);
    assert(seg_valid == 1);

    tsfi_mf_nato_verify_segment_bounds(200, 100, 300, 1, &seg_valid); // Segment 200..300 of 300 (last) -> valid
    assert(seg_valid == 1);

    tsfi_mf_nato_verify_segment_bounds(200, 100, 300, 0, &seg_valid); // Segment 200..300 of 300 (not last) -> invalid
    assert(seg_valid == 0);
    // Verify Helmholtz Physical Link
    printf("[TEST] Validating NATO Helmholtz Physical Link...\n");
    int epoch = 0; // EPOCH_INIT
    int ev_valid = -1;
    
    // Apply HILBERT_ENCODE -> EPOCH_AVAIL
    int h_res = tsfi_mf_nato_phy_link_evaluate_helmholtz(1, 0.0f, &epoch, &ev_valid);
    assert(h_res == 0);
    assert(ev_valid == 1);
    assert(epoch == 1);
    
    // Apply BANACH_NORM (invalid from EPOCH_AVAIL)
    tsfi_mf_nato_phy_link_evaluate_helmholtz(2, 0.0f, &epoch, &ev_valid);
    assert(ev_valid == 0);
    assert(epoch == 1);
    
    // Apply HILBERT_ENCODE -> EPOCH_FORM
    tsfi_mf_nato_phy_link_evaluate_helmholtz(1, 0.0f, &epoch, &ev_valid);
    assert(ev_valid == 1);
    assert(epoch == 2);
    
    // Apply BANACH_NORM -> EPOCH_POLARIZE
    tsfi_mf_nato_phy_link_evaluate_helmholtz(2, 0.0f, &epoch, &ev_valid);
    assert(ev_valid == 1);
    assert(epoch == 3);
    // Verify HFRCP State Machine
    printf("[TEST] Validating NATO HFRCP State Machine...\n");
    int hf_state = 0; // Idle
    int hf_valid = -1;
    // Tune frequency in valid band -> Tuning
    int hf_res = tsfi_mf_nato_hfrcp_update(0, 14250, &hf_state, &hf_valid); // 14.25 MHz (20m Amateur)
    assert(hf_res == 0);
    assert(hf_valid == 1);
    assert(hf_state == 1);

    // Tune frequency in invalid band -> fails
    tsfi_mf_nato_hfrcp_update(0, 500, &hf_state, &hf_valid); // 500 kHz (out of bounds)
    assert(hf_valid == 0);
    assert(hf_state == 1);

    // Complete tuning -> Listening
    tsfi_mf_nato_hfrcp_update(1, 0, &hf_state, &hf_valid);
    assert(hf_valid == 1);
    assert(hf_state == 3);
    printf("  [PASS] HFRCP State Machine verified.\n");

    // Verify Flow Throttling Tracker
    printf("[TEST] Validating NATO Flow Throttling...\n");
    int is_thr = -1;
    int thr_val = -1;
    int thr_res = tsfi_mf_nato_verify_throttle_limit(50, 40, &is_thr, &thr_val); // queue size 50 >= threshold 40 -> throttled
    assert(thr_res == 0);
    assert(thr_val == 1);
    assert(is_thr == 1);

    tsfi_mf_nato_verify_throttle_limit(30, 40, &is_thr, &thr_val); // queue size 30 < threshold 40 -> not throttled
    assert(thr_val == 1);
    assert(is_thr == 0);
    // Verify Helmholtz HFRCP
    printf("[TEST] Validating NATO Helmholtz HFRCP...\n");
    int hf_epoch = 0; // EPOCH_INIT
    int hf_ev_valid = -1;
    
    // Apply HILBERT_ENCODE -> EPOCH_AVAIL
    int hf_h_res = tsfi_mf_nato_hfrcp_evaluate_helmholtz(1, 0.0f, &hf_epoch, &hf_ev_valid);
    assert(hf_h_res == 0);
    assert(hf_ev_valid == 1);
    assert(hf_epoch == 1);
    
    // Apply RESONANCE (invalid from EPOCH_AVAIL)
    tsfi_mf_nato_hfrcp_evaluate_helmholtz(4, 0.0f, &hf_epoch, &hf_ev_valid);
    assert(hf_ev_valid == 0);
    assert(hf_epoch == 1);
    
    // Apply BANACH_NORM -> EPOCH_FORM
    tsfi_mf_nato_hfrcp_evaluate_helmholtz(2, 0.0f, &hf_epoch, &hf_ev_valid);
    assert(hf_ev_valid == 1);
    assert(hf_epoch == 2);
    // Verify AFSK Modem
    printf("[TEST] Validating NATO AFSK Modem...\n");
    int afsk_valid = -1;
    int afsk_res = tsfi_mf_nato_afsk_verify(875, 300, &afsk_valid); // Standard mark tone at 300 baud -> valid
    assert(afsk_res == 0);
    assert(afsk_valid == 1);

    tsfi_mf_nato_afsk_verify(1000, 300, &afsk_valid); // Invalid tone -> invalid
    assert(afsk_valid == 0);
    printf("  [PASS] AFSK Modem verified.\n");

    // Verify Merkle Combine
    printf("[TEST] Validating NATO Merkle Combine...\n");
    uint32_t parent = 0;
    int m_res = tsfi_mf_nato_merkle_combine(0x12345678, 0x9abcdef0, &parent);
    assert(m_res == 0);
    assert(parent != 0);
    printf("  [PASS] Merkle Combine verified.\n");

    return 0;
}

