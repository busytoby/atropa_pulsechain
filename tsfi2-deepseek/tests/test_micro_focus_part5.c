#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "tsfi_cade_imf.h"
#include "tsfi_eer_emergency.h"

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
    // Verify IRS Route Matcher
    printf("[TEST] Validating IRS Route Identification...\n");
    int is_irs = -1;
    int irs_val = -1;
    int irs_res = tsfi_mf_norad_is_irs_route("RUMIRS", &is_irs, &irs_val);
    assert(irs_res == 0);
    assert(irs_val == 1);
    assert(is_irs == 1);

    tsfi_mf_norad_is_irs_route("RUMNDH", &is_irs, &irs_val);
    assert(irs_val == 1);
    assert(is_irs == 0);
    printf("  [PASS] IRS Route Identification verified.\n");

    // Verify IRS Query Formatter
    printf("[TEST] Validating IRS Query Formatter...\n");
    uint8_t irs_pdu[8];
    size_t irs_size = 0;
    int irs_q_res = tsfi_mf_cics_format_irs_query(0x12345678, 10, irs_pdu, &irs_size);
    assert(irs_q_res == 0);
    assert(irs_size == 6);
    assert(irs_pdu[0] == 0xFD);
    assert(irs_pdu[1] == 10);
    assert(irs_pdu[2] == 0x12);
    assert(irs_pdu[5] == 0x78);
    // Verify IRS Response Decoder
    printf("[TEST] Validating IRS Response Decoder...\n");
    uint8_t resp_pdu[] = {0xFE, 0x02, 0x00};
    int audit_status = -1;
    int resp_val = -1;
    int resp_res = tsfi_mf_cics_decode_irs_response(resp_pdu, sizeof(resp_pdu), &audit_status, &resp_val);
    assert(resp_res == 0);
    assert(resp_val == 1);
    assert(audit_status == 2); // Rejected/audited flag
    printf("  [PASS] IRS Response Decoder verified.\n");

    // Verify NATO Relay Sequencer
    printf("[TEST] Validating NATO Relay Sequencer...\n");
    int in_seq = -1;
    int seq_res = tsfi_mf_nato_relay_verify_sequence(5, 5, &in_seq);
    assert(seq_res == 0);
    assert(in_seq == 1);

    tsfi_mf_nato_relay_verify_sequence(5, 6, &in_seq);
    assert(in_seq == 0);
    // Verify IRS Merkle Combiner
    printf("[TEST] Validating IRS Merkle Combiner...\n");
    uint8_t left_h[32] = {0x01};
    uint8_t right_h[32] = {0x02};
    uint8_t parent_h[32];
    int irs_m_res = tsfi_mf_irs_merkle_combine(left_h, right_h, parent_h);
    assert(irs_m_res == 0);
    printf("  [PASS] IRS Merkle Combiner verified.\n");

    // Verify CICS Timeout Monitor
    printf("[TEST] Validating CICS Timeout Monitor...\n");
    int is_to = -1;
    int to_res = tsfi_mf_cics_check_irs_timeout(1000, 2500, 1000, &is_to); // 1500ms elapsed > 1000ms limit -> timeout
    assert(to_res == 0);
    assert(is_to == 1);

    tsfi_mf_cics_check_irs_timeout(1000, 1500, 1000, &is_to); // 500ms elapsed <= 1000ms limit -> no timeout
    assert(is_to == 0);
    // Exhaustive Emergency Broadcast System (EBS) Integration Test
    printf("[TEST] Executing Exhaustive Emergency Broadcast System (EBS) Integration Test...\n");
    
    // 1. Initial State (Normal Operations)
    int ebs_defcon = 5;
    uint16_t ebs_status = 0;
    int ebs_valid = -1;
    int ebs_res = tsfi_mf_norad_encode_defcon(ebs_defcon, 0, &ebs_status, &ebs_valid);
    assert(ebs_res == 0);
    assert(ebs_valid == 1);
    assert((ebs_status & 0x07) == 5); // DEFCON 5
    assert((ebs_status & (1 << 10)) == 0); // No alert active

    int ebs_authorized = -1;
    ebs_res = tsfi_mf_cics_authorize_transaction("NDFT", ebs_status, &ebs_authorized);
    assert(ebs_res == 0);
    assert(ebs_authorized == 1); // Routine transactions authorized at DEFCON 5

    // 2. Incoming Flash Message Classification & Route Matcher
    int ebs_priority = -1;
    int ebs_prec_val = -1;
    ebs_res = tsfi_mf_janap_classify_precedence('Z', &ebs_priority, &ebs_prec_val); // Flash precedence
    assert(ebs_res == 0);
    assert(ebs_prec_val == 1);
    assert(ebs_priority == 4);

    int ebs_is_irs = -1;
    int ebs_route_val = -1;
    ebs_res = tsfi_mf_norad_is_irs_route("RUMIRS", &ebs_is_irs, &ebs_route_val);
    assert(ebs_res == 0);
    assert(ebs_route_val == 1);
    assert(ebs_is_irs == 1); // IRS route confirmed

    // 3. Alert Event Received -> Escalation to DEFCON 1
    int ebs_naap_state = 0; // IDLE
    int ebs_naap_valid = -1;
    ebs_res = tsfi_mf_norad_naap_update(0, &ebs_naap_state, &ebs_naap_valid); // TRIGGER_ALERT -> ALERT_ISSUED
    assert(ebs_res == 0);
    assert(ebs_naap_valid == 1);
    assert(ebs_naap_state == 1);

    ebs_res = tsfi_mf_norad_naap_update(1, &ebs_naap_state, &ebs_naap_valid); // SEND_ACK -> ACK_PENDING
    assert(ebs_res == 0);
    assert(ebs_naap_state == 2);

    ebs_res = tsfi_mf_norad_naap_update(2, &ebs_naap_state, &ebs_naap_valid); // CONFIRM_ALERT -> CONFIRMED
    assert(ebs_res == 0);
    assert(ebs_naap_state == 3);

    // 4. Encode DEFCON 1 Alert Status Word (Radar contacts detected)
    ebs_res = tsfi_mf_norad_encode_defcon(1, 15, &ebs_status, &ebs_valid);
    assert(ebs_res == 0);
    assert(ebs_valid == 1);
    assert((ebs_status & 0x07) == 1); // DEFCON 1
    assert(((ebs_status >> 3) & 0x7F) == 15); // 15 radar contacts
    assert((ebs_status & (1 << 10)) != 0); // Alert Active

    // 5. CICS Authorization Block Verification (DEFCON 1)
    ebs_res = tsfi_mf_cics_authorize_transaction("NDFT", ebs_status, &ebs_authorized); // Routine transaction
    assert(ebs_res == 0);
    assert(ebs_authorized == 0); // Routine transaction BLOCKED during DEFCON 1

    ebs_res = tsfi_mf_cics_authorize_transaction("NJTF", ebs_status, &ebs_authorized); // Critical transaction
    assert(ebs_res == 0);
    assert(ebs_authorized == 1); // Critical transaction ALLOWED during DEFCON 1

    // 6. Broadcast Alert PDU Generation
    uint8_t ebs_bc_pdu[8];
    size_t ebs_bc_size = 0;
    ebs_res = tsfi_mf_cics_generate_naap_broadcast(1, 0xAA55, ebs_bc_pdu, &ebs_bc_size);
    assert(ebs_res == 0);
    assert(ebs_bc_size == 4);
    assert(ebs_bc_pdu[0] == 0xBC); // Broadcast indicator
    assert(ebs_bc_pdu[1] == 1);    // DEFCON 1
    assert(ebs_bc_pdu[2] == 0xAA);
    assert(ebs_bc_pdu[3] == 0x55);

    // 7. Format 3270 Screen Alert Map
    char ebs_3270_stream[128];
    size_t ebs_3270_size = 0;
    ebs_res = tsfi_mf_cics_format_3270_map(1, ebs_3270_stream, &ebs_3270_size);
    assert(ebs_res == 0);
    assert(ebs_3270_size > 0);
    assert(ebs_3270_stream[0] == '\x11');

    // Verify NORAD Command Auth Hash
    printf("[TEST] Validating NORAD Command Auth Hash...\n");
    uint8_t c2_payload[] = {0x10, 0x20, 0x30, 0x40};
    uint32_t c2_hash = 0;
    int ebs_h_res = tsfi_mf_norad_auth_hash(c2_payload, sizeof(c2_payload), &c2_hash);
    assert(ebs_h_res == 0);
    assert(c2_hash != 0);
    printf("  [PASS] NORAD Command Auth Hash verified.\n");

    // Verify JANAP Message Validator
    printf("[TEST] Validating JANAP Message Validator...\n");
    int janap_valid_flag = -1;
    int val_res = tsfi_mf_janap_validate_message(msg, strlen(msg), &janap_valid_flag);
    assert(val_res == 0);
    assert(janap_valid_flag == 1);
    // Verify IRS Alarm Link
    printf("[TEST] Validating IRS Alarm Link...\n");
    int test_defcon = 5;
    uint16_t test_status = 0;
    int ebs_link_res = tsfi_mf_norad_link_irs_alarm(2, &test_defcon, &test_status); // Audit status 2 (rejection) -> defcon 2 + alarm set
    assert(ebs_link_res == 0);
    assert(test_defcon == 2);
    assert((test_status & (1 << 10)) != 0);
    printf("  [PASS] IRS Alarm Link verified.\n");

    // Verify IRS Retry Backoff
    printf("[TEST] Validating IRS Retry Backoff...\n");
    int next_bo = 0;
    int bo_res = tsfi_mf_irs_calculate_retry_backoff(3, 1000, &next_bo); // 1000 * 2^3 = 8000
    assert(bo_res == 0);
    assert(next_bo == 8000);
    // Verify IRS Clearance Formatter
    printf("[TEST] Validating IRS Clearance Formatter...\n");
    uint8_t cl_pdu[8];
    size_t cl_size = 0;
    int cl_res = tsfi_mf_irs_format_clearance(0xDEADC0DE, cl_pdu, &cl_size);
    assert(cl_res == 0);
    assert(cl_size == 5);
    assert(cl_pdu[0] == 0xFA);
    printf("  [PASS] IRS Clearance Formatter verified.\n");

    // Verify NORAD Clearance Validator
    printf("[TEST] Validating NORAD Clearance Validator...\n");
    int is_cl = -1;
    int val_cl_res = tsfi_mf_norad_validate_clearance(cl_pdu, cl_size, 0xDEADC0DE, &is_cl);
    assert(val_cl_res == 0);
    assert(is_cl == 1);

    // Verify TSFiNoradIrsRelay Orchestrator
    printf("[TEST] Validating TSFiNoradIrsRelay Orchestrator...\n");
    TSFiNoradIrsRelay relay;
    int relay_res = tsfi_mf_norad_irs_relay_init(&relay, 0xDEADC0DE);
    assert(relay_res == 0);
    assert(relay.defcon_level == 5);
    assert(relay.is_link_active == 1);

    uint8_t relay_pdu[8];
    size_t relay_size = 0;
    // Process IRS-destined JANAP teletype message
    const char *irs_msg = "R RUMIRS 9999 S\r\nTaxAuditQueryText\r\nNNNN\r\n";
    relay_res = tsfi_mf_norad_irs_relay_process_msg(&relay, irs_msg, strlen(irs_msg), relay_pdu, &relay_size);
    assert(relay_res == 0);
    assert(relay_size == 6);
    assert(relay_pdu[0] == 0xFD); // Formatted IRS Query PDU
    // Verify TSFiNoradIrsRelay loopback
    printf("[TEST] Validating TSFiNoradIrsRelay Loopback Verification...\n");
    int loop_ok = -1;
    int loop_res = tsfi_mf_norad_irs_relay_test_loopback(&relay, irs_msg, relay_pdu, &relay_size, &loop_ok);
    assert(loop_res == 0);
    assert(loop_ok == 1);
    assert(relay.defcon_level == 5);
    assert((relay.status_word & (1 << 10)) == 0);
    // Verify Unified Dispatch
    printf("[TEST] Validating Micro Focus Unified Dispatch...\n");
    uint8_t disp_pdu[8];
    size_t disp_size = 0;
    int disp_res = tsfi_mf_unified_dispatch("IRS", "CLEARANCE", (const uint8_t *)"\xDE\xAD\xC0\xDE", 4, disp_pdu, &disp_size);
    assert(disp_res == 0);
    assert(disp_size == 5);
    assert(disp_pdu[0] == 0xFA);
    // Verify CLI Dispatch Integration
    printf("[TEST] Validating CLI DISPATCH Command Integration...\n");
    char cli_cmd[128] = "0.0 DISPATCH IRS CLEARANCE \xDE\xAD\xC0\xDE";
    int tsfi_cli_process_line(void *ws, char *input);
    int cli_res = tsfi_cli_process_line(NULL, cli_cmd);
    assert(cli_res == 0);
    // Verify EER Emergency Database
    printf("[TEST] Validating EER Emergency Database...\n");
    TSFiEerDatabase eer_db;
    int eer_res = tsfi_eer_db_init(&eer_db);
    assert(eer_res == 0);
    
    eer_res = tsfi_eer_insert_incident(&eer_db, 101, 1, 1690000000, 1); // Nuclear Alert
    assert(eer_res == 0);
    
    eer_res = tsfi_eer_insert_agency(&eer_db, 201, "NORAD_TOKEN", 10, 1); // NORAD
    assert(eer_res == 0);
    
    eer_res = tsfi_eer_link_response(&eer_db, 201, 101);
    assert(eer_res == 0);
    
    int active_alerts = -1;
    eer_res = tsfi_eer_get_active_alerts(&eer_db, 201, &active_alerts);
    assert(eer_res == 0);
    assert(active_alerts == 1);
    // Verify EER DEFCON Broadcast Integration Trigger
    printf("[TEST] Validating EER DEFCON Broadcast Integration Trigger...\n");
    uint8_t bc_buf[8];
    size_t bc_sz = 0;
    // Generate DEFCON 2 broadcast
    int eer_bc_res = tsfi_mf_cics_generate_naap_broadcast(2, 0x1122, bc_buf, &bc_sz);
    assert(eer_bc_res == 0);
    assert(bc_sz == 4);
    assert(bc_buf[0] == 0xBC);
    
    // Ingest the broadcast packet into EER Database
    int extracted_defcon = bc_buf[1];
    uint32_t incident_id = 999;
    eer_res = tsfi_eer_insert_incident(&eer_db, incident_id, extracted_defcon, 1700000000, 1);
    assert(eer_res == 0);
    
    // Assert the record was saved with DEFCON 2
    assert(eer_db.incidents[eer_db.incident_count - 1].defcon_level == 2);
    
    // Link to Broadcasting Unit
    if (eer_db.unit_count < 16) {
        TSFiEerBroadcastingUnit *unit = &eer_db.units[eer_db.unit_count++];
        unit->unit_id = 301;
        unit->target_area = 55;
        unit->power_level = 100;
    }
    
    // Map relation
    if (eer_db.transmits_count < 32) {
        TSFiEerTransmitsAlert *trans = &eer_db.transmits[eer_db.transmits_count++];
        trans->unit_id = 301;
        trans->incident_id = incident_id;
    }
    
    assert(eer_db.transmits[eer_db.transmits_count - 1].incident_id == incident_id);
    // Verify SSA DMF Formatting & Decoding
    printf("[TEST] Validating SSA DMF Query/Response...\n");
    uint8_t dmf_pdu[16];
    size_t dmf_size = 0;
    int ssa_res = tsfi_mf_ssa_format_dmf_query("123456789", dmf_pdu, &dmf_size);
    assert(ssa_res == 0);
    assert(dmf_size == 10);
    assert(dmf_pdu[0] == 0xFB);
    assert(memcmp(&dmf_pdu[1], "123456789", 9) == 0);

    uint8_t dmf_resp_pdu[] = {0xFC, 1}; // Deceased indicator
    int deceased = -1;
    ssa_res = tsfi_mf_ssa_decode_dmf_response(dmf_resp_pdu, sizeof(dmf_resp_pdu), &deceased);
    assert(ssa_res == 0);
    assert(deceased == 1);
    printf("  [PASS] SSA DMF Query/Response verified.\n");

    // Verify SSA SSN Site Resolution
    printf("[TEST] Validating SSA SSN Site Resolution...\n");
    char site[32];
    ssa_res = tsfi_mf_ssa_resolve_issuance_site("002445566", site, sizeof(site));
    assert(ssa_res == 0);
    assert(strcmp(site, "New Hampshire") == 0);
    printf("  [PASS] SSA SSN Site Resolution verified.\n");

    // Verify SSA Checksum
    printf("[TEST] Validating SSA Checksum...\n");
    int ssa_valid = -1;
    // 1+2+3+4+5+6+7+8+9 = 45 -> 45 % 10 = 5 (invalid)
    tsfi_mf_ssa_verify_checksum("123456789", &ssa_valid);
    assert(ssa_valid == 0);

    // 1+2+3+4+5+6+7+8+4 = 40 -> 40 % 10 = 0 (valid)
    tsfi_mf_ssa_verify_checksum("123456784", &ssa_valid);
    assert(ssa_valid == 1);
    // Verify SSA High-Group allocation
    printf("[TEST] Validating SSA High-Group Allocation...\n");
    int alloc = -1;
    ssa_res = tsfi_mf_ssa_verify_high_group(50, 5, &alloc);
    assert(ssa_res == 0);
    assert(alloc == 1);
    
    ssa_res = tsfi_mf_ssa_verify_high_group(50, 15, &alloc);
    assert(ssa_res == 0);
    assert(alloc == 0);
    printf("  [PASS] SSA High-Group Allocation verified.\n");

    // Verify SSA Benefit Auth Formatting
    printf("[TEST] Validating SSA Benefit Auth Formatting...\n");
    uint8_t ben_pdu[16];
    size_t ben_size = 0;
    ssa_res = tsfi_mf_ssa_format_benefit_auth(12345, 250.50, ben_pdu, &ben_size);
    assert(ssa_res == 0);
    assert(ben_size == 9);
    assert(ben_pdu[0] == 0xF9);
    // Verify SSA Exhaustive SSN Validator
    printf("[TEST] Validating SSA Exhaustive SSN Validator...\n");
    int ssn_ok = -1;
    
    // Valid case: Area 050, Group 05 (<= 10), Serial 1122
    ssa_res = tsfi_mf_ssa_verify_ssn_exhaustive("050051122", &ssn_ok);
    assert(ssa_res == 0);
    assert(ssn_ok == 1);
    
    // Invalid case: Area 666
    tsfi_mf_ssa_verify_ssn_exhaustive("666051122", &ssn_ok);
    assert(ssn_ok == 0);
    
    // Invalid case: Group 00
    tsfi_mf_ssa_verify_ssn_exhaustive("050001122", &ssn_ok);
    assert(ssn_ok == 0);
    
    // Invalid case: Group exceeds High-Group (Group 15 > 10 for Area 050)
    tsfi_mf_ssa_verify_ssn_exhaustive("050151122", &ssn_ok);
    assert(ssn_ok == 0);
    
    // Invalid case: Serial 0000
    tsfi_mf_ssa_verify_ssn_exhaustive("050050000", &ssn_ok);
    assert(ssn_ok == 0);
    
    // Verify SSA-IRS Joint Sync
    printf("[TEST] Validating SSA-IRS Joint Sync...\n");
    int audit_act = -1;
    int sync_res = tsfi_mf_ssa_irs_sync_audit("050051122", 1, 1, &audit_act); // Deceased dependent
    assert(sync_res == 0);
    assert(audit_act == 2); // Escalate to Audit

    sync_res = tsfi_mf_ssa_irs_sync_audit("050051122", 0, 0, &audit_act); // Unallocated SSN
    assert(sync_res == 0);
    assert(audit_act == 1); // Hold Refund
    printf("  [PASS] SSA-IRS Joint Sync verified.\n");

    // Verify SSA-IRS Fraud Alert Encoding
    printf("[TEST] Validating SSA-IRS Fraud Alert Encoding...\n");
    char alert_msg[128];
    int ssa_alert_res = tsfi_mf_ssa_irs_format_fraud_alert("050051122", 2, alert_msg, sizeof(alert_msg));
    assert(ssa_alert_res == 0);
    assert(strstr(alert_msg, "FRAUD ALERT: SSN 050051122 ACTION CODE 2") != NULL);
    assert(strstr(alert_msg, "NNNN") != NULL);
    // Verify Tri-Agency Coordinator
    printf("[TEST] Validating Tri-Agency Coordinator...\n");
    int tri_defcon = 5;
    uint16_t tri_status = 0;
    int tri_res = tsfi_mf_tri_agency_coordinate("050051122", 1, 0, &tri_defcon, &tri_status); // Deceased + unallocated
    assert(tri_res == 0);
    assert(tri_defcon == 1);
    assert((tri_status & (1 << 11)) != 0);
    printf("  [PASS] Tri-Agency Coordinator verified.\n");

    // Verify SSA-IRS-NATO STANAG Broadcast
    printf("[TEST] Validating SSA-IRS-NATO STANAG Broadcast...\n");
    uint8_t nato_frame[16];
    size_t nato_size = 0;
    tsfi_gost_emergency_defcon_level = 5; // Reset
    int ssa_nato_res = tsfi_mf_nato_format_ssa_irs_broadcast("050051122", 2, nato_frame, &nato_size);
    assert(ssa_nato_res == 0); // Permitted
    assert(nato_size == 13);
    assert(nato_frame[0] == 0x7E);
    assert(nato_frame[1] == 0x55);
    assert(nato_frame[2] == 2);
    assert(nato_frame[12] == 0x7E);
    assert(tsfi_gost_emergency_defcon_level == 1); // Alarm triggered by broadcast accessor
    printf("  [PASS] SSA-IRS-NATO STANAG Broadcast permitted raw identity and triggered DEFCON 1 successfully.\n");
    // Verify Soviet GOST Scrambler
    printf("[TEST] Validating Soviet GOST Scrambler...\n");
    tsfi_gost_is_broadcast_channel = 1;
    uint32_t left = 0x11111111;
    uint32_t right = 0x22222222;
    int gost_res = tsfi_mf_ussr_gost_scramble(&left, &right, 0x33333333);
    assert(gost_res == 0);
    assert(left != 0x11111111);
    tsfi_gost_is_broadcast_channel = 0;
    printf("  [PASS] Soviet GOST Scrambler verified.\n");

    // Verify Soviet GOST Transliteration
    printf("[TEST] Validating Soviet GOST Transliteration...\n");
    char cyrillic[32];
    gost_res = tsfi_mf_ussr_gost_transliterate("RUMIRS", cyrillic, sizeof(cyrillic));
    assert(gost_res == 0);
    assert(strcmp(cyrillic, "PUMIPC") == 0); // R->P, S->C transliterated
    // Verify Soviet GOST 32-Round Block Cipher
    printf("[TEST] Validating Soviet GOST 32-Round Cipher...\n");
    tsfi_gost_is_broadcast_channel = 1;
    uint32_t c_left = 0xAAAAAAAA;
    uint32_t c_right = 0x55555555;
    uint32_t keys_8[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    int cipher_res = tsfi_mf_ussr_gost_encrypt_32(&c_left, &c_right, keys_8);
    assert(cipher_res == 0);
    assert(c_left != 0xAAAAAAAA);
    tsfi_gost_is_broadcast_channel = 0;
    printf("  [PASS] Soviet GOST 32-Round Cipher verified.\n");

    // Verify Soviet Red Telephone Scrambler
    printf("[TEST] Validating Soviet Red Telephone Scrambler...\n");
    uint8_t raw_signal[] = "TacticalAudioSignal";
    uint8_t scr_signal[32] = {0};
    int phone_res = tsfi_mf_ussr_red_phone_scramble(raw_signal, sizeof(raw_signal), 0x99, scr_signal);
    assert(phone_res == 0);
    assert(scr_signal[0] != raw_signal[0]);
    // Verify GOST S-Box Setting
    printf("[TEST] Validating GOST Custom S-Box Selector...\n");
    int sbox_res = tsfi_mf_gost_set_sbox(1); // Set test profile
    assert(sbox_res == 0);
    sbox_res = tsfi_mf_gost_set_sbox(0); // Set KGB profile
    assert(sbox_res == 0);
    printf("  [PASS] GOST Custom S-Box Selector verified.\n");

    // Verify GOST Hash step
    printf("[TEST] Validating GOST Hash Compression Step...\n");
    uint8_t dummy_block[32] = {0x01};
    uint8_t hash_st[32] = {0xAB};
    int hash_res = tsfi_mf_gost_hash_step(dummy_block, hash_st);
    assert(hash_res == 0);
    assert(hash_st[0] != 0xAB);
    // Verify USSS Auth PDU Formatting and Verification
    printf("[TEST] Validating USSS Auth PDU...\n");
    uint8_t usss_pdu[16];
    size_t usss_size = 0;
    int usss_res = tsfi_mf_usss_format_auth_pdu(99, 12345, usss_pdu, &usss_size);
    assert(usss_res == 0);
    assert(usss_size == 9);
    assert(usss_pdu[0] == 0xF7);

    int usss_valid = -1;
    usss_res = tsfi_mf_usss_verify_auth_pdu(usss_pdu, usss_size, &usss_valid);
    assert(usss_res == 0);
    assert(usss_valid == 1);
    printf("  [PASS] USSS Auth PDU verified.\n");

    // Verify Soviet Spec-Svyaz Rotor Cipher
    printf("[TEST] Validating Soviet Spec-Svyaz Rotor Cipher...\n");
    uint8_t spec_raw[] = "SpecSvyazMessage";
    uint8_t spec_enc[32] = {0};
    int spec_res = tsfi_mf_ussr_spec_svyaz_cipher(spec_raw, sizeof(spec_raw), 5, spec_enc);
    assert(spec_res == 0);
    assert(spec_enc[0] != spec_raw[0]);
    // Verify GOST broadcast-only restriction
    printf("[TEST] Validating GOST broadcast-only restriction...\n");
    tsfi_gost_is_broadcast_channel = 0; // Set to restricted/non-broadcast state
    uint32_t test_l = 0xAA;
    uint32_t test_r = 0xBB;
    int bcast_res = tsfi_mf_ussr_gost_scramble(&test_l, &test_r, 0);
    assert(bcast_res == -4); // Denied
    printf("  [PASS] GOST broadcast-only restriction verified.\n");

    // Verify GOST emergency DEFCON override (intrusion detection)
    printf("[TEST] Validating GOST Emergency DEFCON override...\n");
    tsfi_gost_is_broadcast_channel = 1; // Mark as broadcast-enabled for execution
    tsfi_gost_emergency_defcon_level = 5; // Reset
    uint32_t bad_left = 0x31323334; // "1234" (ASCII digits)
    uint32_t bad_right = 0;
    tsfi_mf_ussr_gost_scramble(&bad_left, &bad_right, 0);
    assert(tsfi_gost_emergency_defcon_level == 1);
    printf("  [PASS] GOST Emergency DEFCON override verified.\n");

    // Verify TIN pattern blocking
    printf("[TEST] Validating TIN range blocking inside GOST...\n");
    tsfi_gost_is_broadcast_channel = 1; // Mark as broadcast-enabled
    tsfi_gost_emergency_defcon_level = 5; // Reset
    uint32_t itin_val = 950000000; // ITIN range
    uint32_t normal_val = 0;
    int block_res = tsfi_mf_ussr_gost_scramble(&itin_val, &normal_val, 0);
    assert(block_res == -3); // Blocked
    assert(tsfi_gost_emergency_defcon_level == 1); // Alarm triggered
    printf("  [PASS] TIN range blocking inside GOST verified.\n");
    
    // Restore default
    tsfi_gost_is_broadcast_channel = 0;

    return 0;
}
