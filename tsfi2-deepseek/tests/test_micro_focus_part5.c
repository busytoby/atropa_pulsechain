#include <stdio.h>
#include <assert.h>
#include <string.h>
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
    // Verify DRS State Machine
    printf("[TEST] Validating NATO DRS Adaptation...\n");
    int rate = 0;
    int rate_valid = -1;
    int drs_res = tsfi_mf_nato_drs_update(-70, 20, &rate, &rate_valid); // SNR 20 -> 4800 bps
    assert(drs_res == 0);
    assert(rate_valid == 1);
    assert(rate == 4800);

    tsfi_mf_nato_drs_update(-130, 20, &rate, &rate_valid); // RSSI -130 (invalid) -> fails
    assert(rate_valid == 0);
    printf("  [PASS] DRS Adaptation verified.\n");

    // Verify RX Reorder Buffer
    printf("[TEST] Validating NATO RX Reorder Buffer...\n");
    int reorder_valid = -1;
    int reorder_res = tsfi_mf_nato_reorder_buffer_insert(5, 2, &reorder_valid); // Sequence 5 is within window [2, 9] -> valid
    assert(reorder_res == 0);
    assert(reorder_valid == 1);

    tsfi_mf_nato_reorder_buffer_insert(20, 2, &reorder_valid); // Sequence 20 is outside window [2, 9] -> invalid
    assert(reorder_valid == 0);
    printf("  [PASS] RX Reorder Buffer verified.\n");

    // Verify HMTP Envelope
    printf("[TEST] Validating NATO HMTP Envelope...\n");
    uint8_t mail_pkt[128];
    size_t mail_size = 0;
    int mail_res = tsfi_mf_nato_hmtp_encode_envelope("alice@node.org", "bob@node.org", mail_pkt, &mail_size);
    assert(mail_res == 0);
    assert(mail_size == 1 + 14 + 1 + 12 + 1); // "M" + sender + "," + recipient + ";"
    assert(mail_pkt[0] == 0x4D);
    assert(mail_pkt[mail_size - 1] == ';');
    printf("  [PASS] HMTP Envelope verified.\n");

    // Verify CSM Evaluator
    printf("[TEST] Validating NATO Channel State Monitor...\n");
    int csm_flags = -1;
    int csm_valid = -1;
    int csm_res = tsfi_mf_nato_csm_evaluate(-110, 300, &csm_flags, &csm_valid); // Noise -110 (< -100), carrier lock 300 (> 200) -> quiet & lock
    assert(csm_res == 0);
    assert(csm_valid == 1);
    assert(csm_flags == 0x03);

    tsfi_mf_nato_csm_evaluate(-80, 100, &csm_flags, &csm_valid); // Noise -80, lock 100 -> flags should be 0
    assert(csm_valid == 1);
    assert(csm_flags == 0x00);
    // Verify Payload Fragmenter
    printf("[TEST] Validating NATO Payload Fragmenter...\n");
    uint8_t payload_data[] = {1, 2, 3, 4, 5, 6, 7, 8};
    uint8_t chunk[4];
    size_t chunk_size = 0;
    int is_lst = -1;
    int frag_res = tsfi_mf_nato_fragment_payload(payload_data, sizeof(payload_data), 3, 0, chunk, &chunk_size, &is_lst); // chunk 0 (1,2,3) -> not last
    assert(frag_res == 0);
    assert(chunk_size == 3);
    assert(is_lst == 0);
    assert(chunk[0] == 1);

    tsfi_mf_nato_fragment_payload(payload_data, sizeof(payload_data), 3, 2, chunk, &chunk_size, &is_lst); // chunk 2 (7,8) -> last
    assert(chunk_size == 2);
    assert(is_lst == 1);
    assert(chunk[0] == 7);
    printf("  [PASS] Payload Fragmenter verified.\n");

    // Verify Unbind Confirmation
    printf("[TEST] Validating NATO Unbind Confirmation...\n");
    uint8_t unbind_conf[8];
    size_t unbind_conf_size = 0;
    int unbind_res = tsfi_mf_nato_generate_unbind_confirm(0, unbind_conf, &unbind_conf_size); // Success/Normal close
    assert(unbind_res == 0);
    assert(unbind_conf_size == 2);
    assert(unbind_conf[0] == 0x82);
    assert(unbind_conf[1] == 0);
    // Verify Warning Primitive
    printf("[TEST] Validating NATO Warning Primitive...\n");
    uint8_t warn_pkt[8];
    size_t warn_size = 0;
    int warn_res = tsfi_mf_nato_generate_warning(1, warn_pkt, &warn_size);
    assert(warn_res == 0);
    assert(warn_size == 2);
    assert(warn_pkt[0] == 0x8F);
    assert(warn_pkt[1] == 1);
    printf("  [PASS] Warning Primitive verified.\n");

    // Verify Connect Reject Primitive
    printf("[TEST] Validating NATO Connect Reject Primitive...\n");
    uint8_t reject_pkt[8];
    size_t reject_size = 0;
    int reject_res = tsfi_mf_nato_generate_connect_reject(2, reject_pkt, &reject_size);
    assert(reject_res == 0);
    assert(reject_size == 2);
    assert(reject_pkt[0] == 0x83);
    assert(reject_pkt[1] == 2);
    // Verify D_PDU Type 1
    printf("[TEST] Validating NATO D_PDU Type 1 Encoder...\n");
    uint8_t pdu_payload[] = {0xAA, 0xBB};
    uint8_t dpdu[16];
    size_t dpdu_size = 0;
    int dpdu_res = tsfi_mf_nato_encode_d_pdu_type1(3, 4, pdu_payload, sizeof(pdu_payload), dpdu, &dpdu_size);
    assert(dpdu_res == 0);
    assert(dpdu_size == 4);
    assert(dpdu[0] == 0x01);
    assert(dpdu[1] == 0x34); // (3 << 4) | 4 = 0x34
    assert(dpdu[2] == 0xAA);
    printf("  [PASS] D_PDU Type 1 verified.\n");

    // Verify Hard Reset
    printf("[TEST] Validating NATO Hard Reset Primitive...\n");
    uint8_t reset_pkt[8];
    size_t reset_size = 0;
    int reset_res = tsfi_mf_nato_generate_hard_reset(1, reset_pkt, &reset_size);
    assert(reset_res == 0);
    assert(reset_size == 2);
    assert(reset_pkt[0] == 0x8C);
    assert(reset_pkt[1] == 1);
    // Verify Flow Clear
    printf("[TEST] Validating NATO Flow Clear Primitive...\n");
    uint8_t clear_pkt[8];
    size_t clear_size = 0;
    int clear_res = tsfi_mf_nato_generate_flow_clear(5, clear_pkt, &clear_size);
    assert(clear_res == 0);
    assert(clear_size == 2);
    assert(clear_pkt[0] == 0x8E);
    assert(clear_pkt[1] == 5);
    printf("  [PASS] Flow Clear Primitive verified.\n");

    // Verify Duplicate Detector
    printf("[TEST] Validating NATO ARQ Duplicate Detector...\n");
    uint8_t rec_mask[4] = {0x01, 0x00, 0x00, 0x00}; // Bit 0 set (corresponds to expected_seq)
    int is_dup = -1;
    int dup_valid = -1;
    int dup_res = tsfi_mf_nato_arq_detect_duplicate(10, rec_mask, 10, &is_dup, &dup_valid); // seq 10 == expected 10, offset 0 -> bit 0 checked -> duplicate
    assert(dup_res == 0);
    assert(dup_valid == 1);
    assert(is_dup == 1);

    tsfi_mf_nato_arq_detect_duplicate(11, rec_mask, 10, &is_dup, &dup_valid); // seq 11 == expected 10 + 1 -> bit 1 checked -> not duplicate
    assert(dup_valid == 1);
    assert(is_dup == 0);
    // Verify D_PDU Type 2
    printf("[TEST] Validating NATO D_PDU Type 2 Encoder...\n");
    uint8_t pdu_payload2[] = {0x11, 0x22};
    uint8_t dpdu2[16];
    size_t dpdu2_size = 0;
    int dpdu2_res = tsfi_mf_nato_encode_d_pdu_type2(2, 5, 42, pdu_payload2, sizeof(pdu_payload2), dpdu2, &dpdu2_size);
    assert(dpdu2_res == 0);
    assert(dpdu2_size == 5);
    assert(dpdu2[0] == 0x02);
    assert(dpdu2[1] == 0x25); // (2 << 4) | 5 = 0x25
    assert(dpdu2[2] == 42);   // Tx Seq
    assert(dpdu2[3] == 0x11);
    printf("  [PASS] D_PDU Type 2 verified.\n");

    // Verify Flow Control (Resume/Pause)
    printf("[TEST] Validating NATO Flow Control Primitive...\n");
    uint8_t flow_pkt[8];
    size_t flow_size = 0;
    int flow_res = tsfi_mf_nato_generate_flow_control(1, 3, flow_pkt, &flow_size); // Resume, SAP 3
    assert(flow_res == 0);
    assert(flow_size == 2);
    assert(flow_pkt[0] == 0x8D);
    assert(flow_pkt[1] == 0x13); // (1 << 4) | 3 = 0x13
    // Verify Flow Alert
    printf("[TEST] Validating NATO Flow Alert Primitive...\n");
    uint8_t alert_pkt[8];
    size_t alert_size = 0;
    int alert_res = tsfi_mf_nato_generate_flow_alert(1, alert_pkt, &alert_size);
    assert(alert_res == 0);
    assert(alert_size == 2);
    assert(alert_pkt[0] == 0x89);
    assert(alert_pkt[1] == 1);
    printf("  [PASS] Flow Alert Primitive verified.\n");

    // Verify D_PDU Type 8
    printf("[TEST] Validating NATO D_PDU Type 8 Encoder...\n");
    uint8_t dpdu8[8];
    size_t dpdu8_size = 0;
    int dpdu8_res = tsfi_mf_nato_encode_d_pdu_type8(2, 4, 1, dpdu8, &dpdu8_size); // Destination 2, Source 4, ACK Reset = 1
    assert(dpdu8_res == 0);
    assert(dpdu8_size == 3);
    assert(dpdu8[0] == 0x08);
    assert(dpdu8[1] == 0x24); // (2 << 4) | 4 = 0x24
    assert(dpdu8[2] == 1);
    // Verify D_PDU Type 9
    printf("[TEST] Validating NATO D_PDU Type 9 Encoder...\n");
    uint8_t mask[4] = {0x12, 0x34, 0x56, 0x78};
    uint8_t dpdu9[16];
    size_t dpdu9_size = 0;
    int dpdu9_res = tsfi_mf_nato_encode_d_pdu_type9(1, 3, 5, mask, dpdu9, &dpdu9_size);
    assert(dpdu9_res == 0);
    assert(dpdu9_size == 7);
    assert(dpdu9[0] == 0x09);
    assert(dpdu9[1] == 0x13);
    assert(dpdu9[2] == 5);
    assert(dpdu9[3] == 0x12);
    printf("  [PASS] D_PDU Type 9 verified.\n");

    // Verify D_PDU Type 3
    printf("[TEST] Validating NATO D_PDU Type 3 Encoder...\n");
    uint8_t pdu_payload3[] = {0xDE, 0xAD};
    uint8_t dpdu3[16];
    size_t dpdu3_size = 0;
    int dpdu3_res = tsfi_mf_nato_encode_d_pdu_type3(2, 6, pdu_payload3, sizeof(pdu_payload3), dpdu3, &dpdu3_size);
    assert(dpdu3_res == 0);
    assert(dpdu3_size == 4);
    assert(dpdu3[0] == 0x03);
    assert(dpdu3[1] == 0x26);
    assert(dpdu3[2] == 0xDE);
    // Verify JANAP 128
    printf("[TEST] Validating NORAD JANAP 128 Envelope...\n");
    char janap_pkt[64];
    size_t janap_size = 0;
    int janap_res = tsfi_mf_norad_encode_janap128("RUMJTF", 123, 'S', janap_pkt, &janap_size);
    assert(janap_res == 0);
    assert(janap_size > 0);
    assert(strncmp(janap_pkt, "R RUMJTF 0123 S\r\n", janap_size) == 0);
    printf("  [PASS] JANAP 128 Envelope verified.\n");

    // Verify DEFCON Status
    printf("[TEST] Validating NORAD DEFCON Status Word...\n");
    uint16_t defcon_status = 0;
    int defcon_valid = -1;
    int defcon_res = tsfi_mf_norad_encode_defcon(3, 12, &defcon_status, &defcon_valid); // DEFCON 3, 12 contacts -> alert active (bit 10 set)
    assert(defcon_res == 0);
    assert(defcon_valid == 1);
    assert((defcon_status & 0x07) == 3);
    assert(((defcon_status >> 3) & 0x7F) == 12);
    assert((defcon_status & (1 << 10)) != 0); // Alert set
    // Verify NAAP State Machine
    printf("[TEST] Validating NORAD NAAP State Machine...\n");
    int naap_state = 0; // IDLE
    int naap_valid = -1;
    int naap_res = tsfi_mf_norad_naap_update(0, &naap_state, &naap_valid); // TRIGGER_ALERT
    assert(naap_res == 0);
    assert(naap_valid == 1);
    assert(naap_state == 1); // ALERT_ISSUED
    printf("  [PASS] NAAP State Machine verified.\n");

    // Verify EOM Detector
    printf("[TEST] Validating NORAD JANAP EOM Detector...\n");
    const char *msg = "R RUMJTF 0123 U\r\nHello World\r\nNNNN\r\n";
    int eom_det = -1;
    int eom_res = tsfi_mf_norad_detect_eom(msg, strlen(msg), &eom_det);
    assert(eom_res == 0);
    assert(eom_det == 1);
    // Verify CICS Transid Map
    printf("[TEST] Validating CICS JANAP Transid Mapping...\n");
    char transid[8];
    int trans_val = -1;
    int trans_res = tsfi_mf_cics_map_janap_transid("RUMJTF", transid, &trans_val);
    assert(trans_res == 0);
    assert(trans_val == 1);
    assert(strcmp(transid, "NJTF") == 0);
    printf("  [PASS] CICS JANAP Transid Mapping verified.\n");

    // Verify CICS NAAP Alert Broadcast
    printf("[TEST] Validating CICS NAAP Alert Broadcast...\n");
    uint8_t bc_pkt[8];
    size_t bc_size = 0;
    int bc_res = tsfi_mf_cics_generate_naap_broadcast(2, 0x1234, bc_pkt, &bc_size);
    assert(bc_res == 0);
    assert(bc_size == 4);
    assert(bc_pkt[0] == 0xBC);
    assert(bc_pkt[1] == 2);
    assert(bc_pkt[2] == 0x12);
    assert(bc_pkt[3] == 0x34);
    // Verify JANAP Payload Extractor
    printf("[TEST] Validating CICS JANAP Payload Extractor...\n");
    char extracted_payload[64];
    size_t ext_size = 0;
    int ext_res = tsfi_mf_cics_extract_janap_payload(msg, sizeof(extracted_payload), extracted_payload, &ext_size);
    assert(ext_res == 0);
    assert(ext_size > 0);
    assert(strcmp(extracted_payload, "Hello World") == 0);
    printf("  [PASS] CICS JANAP Payload Extractor verified.\n");

    // Verify Transaction Authorization Exit
    printf("[TEST] Validating CICS NAAP Transaction Authorization...\n");
    int authorized = -1;
    int auth_res = tsfi_mf_cics_authorize_transaction("NJTF", 2, &authorized); // NJTF is critical -> allowed at DEFCON 2
    assert(auth_res == 0);
    assert(authorized == 1);

    tsfi_mf_cics_authorize_transaction("NDFT", 2, &authorized); // NDFT is non-critical -> blocked at DEFCON 2
    assert(authorized == 0);

    tsfi_mf_cics_authorize_transaction("NDFT", 5, &authorized); // Allowed at DEFCON 5
    assert(authorized == 1);
    // Verify 3270 Map Formatter
    printf("[TEST] Validating CICS 3270 Map Formatter...\n");
    char display_buf[128];
    size_t display_size = 0;
    int fmt_res = tsfi_mf_cics_format_3270_map(1, display_buf, &display_size);
    assert(fmt_res == 0);
    assert(display_size > 0);
    assert(display_buf[0] == '\x11');
    assert(display_buf[display_size - 1] == '*');
    printf("  [PASS] CICS 3270 Map Formatter verified.\n");

    // Verify Precedence Classifier
    printf("[TEST] Validating JANAP Precedence Classifier...\n");
    int priority = -1;
    int prec_val = -1;
    int prec_res = tsfi_mf_janap_classify_precedence('Z', &priority, &prec_val); // Flash -> priority 4
    assert(prec_res == 0);
    assert(prec_val == 1);
    assert(priority == 4);

    tsfi_mf_janap_classify_precedence('R', &priority, &prec_val); // Routine -> priority 1
    assert(prec_val == 1);
    assert(priority == 1);

    tsfi_mf_janap_classify_precedence('X', &priority, &prec_val); // Invalid -> invalid
    assert(prec_val == 0);
    printf("  [PASS] JANAP Precedence Classifier verified.\n");

    return 0;
}

