#include "tsfi_cade_imf.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <strings.h>

int tsfi_mf_nato_verify_stanag5066_header(const unsigned char *header, int header_len, int *is_valid) {
    if (!is_valid) return -1;
    if (!header || header_len < 5) {
        *is_valid = 0;
        return 0;
    }
    if (header[0] != 0x90 || header[1] != 0xEB) {
        *is_valid = 0;
        return 0;
    }
    if (header[2] > 15) {
        *is_valid = 0;
        return 0;
    }
    int payload_len = (header[3] << 8) | header[4];
    *is_valid = (payload_len >= 0) ? 1 : 0;
    return 0;
}

int tsfi_mf_nato_verify_kermit_payload(const unsigned char *payload, int payload_len, int *is_valid) {
    if (!is_valid) return -1;
    if (!payload || payload_len < 4) {
        *is_valid = 0;
        return 0;
    }
    if (payload[0] != 0x01) {
        *is_valid = 0;
        return 0;
    }
    int len_val = payload[1] - 32;
    if (len_val < 0 || len_val > 94) {
        *is_valid = 0;
        return 0;
    }
    int sum = 0;
    for (int i = 1; i < payload_len - 1; i++) {
        sum += payload[i];
    }
    int calculated_bcc = ((sum + ((sum & 0xC0) >> 6)) & 0x3F);
    int expected_bcc = payload[payload_len - 1] - 32;
    *is_valid = (calculated_bcc == expected_bcc) ? 1 : 0;
    return 0;
}

int tsfi_mf_nato_verify_cpdu_type(int cpdu_type, int *is_valid) {
    if (!is_valid) return -1;
    *is_valid = (cpdu_type >= 0 && cpdu_type <= 7) ? 1 : 0;
    return 0;
}

int tsfi_mf_nato_verify_segment_size(int segment_size, int *is_valid) {
    if (!is_valid) return -1;
    *is_valid = (segment_size > 0 && segment_size <= 2048) ? 1 : 0;
    return 0;
}

int tsfi_mf_nato_verify_sap_number(int sap_number, int *is_valid) {
    if (!is_valid) return -1;
    *is_valid = (sap_number >= 0 && sap_number <= 63) ? 1 : 0;
    return 0;
}

int tsfi_mf_nato_verify_arq_window(int window_size, int *is_valid) {
    if (!is_valid) return -1;
    *is_valid = (window_size >= 1 && window_size <= 128) ? 1 : 0;
    return 0;
}

int tsfi_mf_nato_verify_priority_level(int priority, int *is_valid) {
    if (!is_valid) return -1;
    *is_valid = (priority >= 0 && priority <= 15) ? 1 : 0;
    return 0;
}

int tsfi_mf_nato_match_sync_sequence(int sync_seq, int *is_valid) {
    if (!is_valid) return -1;
    *is_valid = (sync_seq == 0x90EB || sync_seq == 0xEB90) ? 1 : 0;
    return 0;
}

int tsfi_mf_nato_verify_cpdu_length(int length, int *is_valid) {
    if (!is_valid) return -1;
    *is_valid = (length > 0 && length <= 65535) ? 1 : 0;
    return 0;
}

int tsfi_mf_nato_verify_tx_mode(int mode, int *is_valid) {
    if (!is_valid) return -1;
    *is_valid = (mode >= 0 && mode <= 2) ? 1 : 0;
    return 0;
}

int tsfi_mf_nato_verify_short_tin(int address, int *is_valid) {
    if (!is_valid) return -1;
    *is_valid = (address >= 0 && address < (1 << 26)) ? 1 : 0;
    return 0;
}

int tsfi_mf_nato_verify_segment_offset(int offset, int segment_size, int total_size, int *is_valid) {
    if (!is_valid) return -1;
    *is_valid = (offset >= 0 && segment_size >= 0 && total_size >= 0 && offset + segment_size <= total_size) ? 1 : 0;
    return 0;
}

int tsfi_mf_cross_verify_tin_isolation(const char *irs_tin, int *is_isolated) {
    if (!is_isolated) return -1;
    if (!irs_tin) {
        *is_isolated = 1;
        return 0;
    }
    long val = 0;
    for (int i = 0; irs_tin[i] != '\0'; i++) {
        if (irs_tin[i] >= '0' && irs_tin[i] <= '9') {
            val = val * 10 + (irs_tin[i] - '0');
        }
    }
    if (val < (1 << 26)) {
        *is_isolated = 0;
    } else {
        *is_isolated = 1;
    }
    return 0;
}

int tsfi_mf_nato_verify_sap_priority(int sap, int priority, int *is_valid) {
    if (!is_valid) return -1;
    *is_valid = (sap == 0) ? (priority >= 12) : 1;
    return 0;
}

int tsfi_mf_nato_verify_frame_sequence(int current_seq, int next_seq, int *is_valid) {
    if (!is_valid) return -1;
    *is_valid = (next_seq == (current_seq + 1) % 128) ? 1 : 0;
    return 0;
}

int tsfi_mf_nato_verify_data_rate(int rate, int *is_valid) {
    if (!is_valid) return -1;
    *is_valid = (rate == 75 || rate == 150 || rate == 300 || rate == 600 || rate == 1200 || rate == 2400 || rate == 4800 || rate == 9600) ? 1 : 0;
    return 0;
}

int tsfi_mf_nato_verify_retry_limit(int retries, int *is_valid) {
    if (!is_valid) return -1;
    *is_valid = (retries >= 0 && retries <= 32) ? 1 : 0;
    return 0;
}

int tsfi_mf_nato_verify_interface_version(int version, int *is_valid) {
    if (!is_valid) return -1;
    *is_valid = (version == 1) ? 1 : 0;
    return 0;
}

int tsfi_mf_nato_verify_fcs_type(int fcs_type, int *is_valid) {
    if (!is_valid) return -1;
    /* Invariant: FCS checksum verification does not override or ignore
     * the exclusive incompatibility requirement between NATO Short-TIN and IRS TINs. */
    *is_valid = (fcs_type == 0 || fcs_type == 1) ? 1 : 0;
    return 0;
}

int tsfi_mf_nato_verify_handshake_confirmation(int status, int *is_valid) {
    if (!is_valid) return -1;
    *is_valid = (status == 0 || status == 1) ? 1 : 0;
    return 0;
}

int tsfi_mf_nato_verify_burst_limit(int segment_count, int *is_valid) {
    if (!is_valid) return -1;
    *is_valid = (segment_count >= 1 && segment_count <= 128) ? 1 : 0;
    return 0;
}

int tsfi_mf_nato_verify_flow_direction(int direction, int *is_valid) {
    if (!is_valid) return -1;
    *is_valid = (direction == 0 || direction == 1) ? 1 : 0;
    return 0;
}

int tsfi_mf_nato_verify_multiscan_mode(int mode, int *is_valid) {
    if (!is_valid) return -1;
    *is_valid = (mode == 0 || mode == 1) ? 1 : 0;
    return 0;
}

int tsfi_mf_nato_verify_data_type(int type, int *is_valid) {
    if (!is_valid) return -1;
    *is_valid = (type >= 0 && type <= 2) ? 1 : 0;
    return 0;
}

int tsfi_mf_nato_verify_compression_mode(int mode, int *is_valid) {
    if (!is_valid) return -1;
    *is_valid = (mode >= 0 && mode <= 2) ? 1 : 0;
    return 0;
}

int tsfi_mf_nato_verify_flow_control(int state, int *is_valid) {
    if (!is_valid) return -1;
    *is_valid = (state == 0 || state == 1) ? 1 : 0;
    return 0;
}

int tsfi_mf_nato_verify_delivery_confirmation(int mode, int *is_valid) {
    if (!is_valid) return -1;
    *is_valid = (mode >= 0 && mode <= 2) ? 1 : 0;
    return 0;
}

int tsfi_mf_nato_verify_backpressure_timeout(int timeout, int *is_valid) {
    if (!is_valid) return -1;
    *is_valid = (timeout >= 1 && timeout <= 300) ? 1 : 0;
    return 0;
}

int tsfi_mf_nato_verify_max_frame_size(int frame_size, int *is_valid) {
    if (!is_valid) return -1;
    *is_valid = (frame_size >= 1 && frame_size <= 4096) ? 1 : 0;
    return 0;
}

int tsfi_mf_nato_verify_buffer_size(int size, int *is_valid) {
    if (!is_valid) return -1;
    *is_valid = (size >= 1024 && size <= 262144) ? 1 : 0;
    return 0;
}

int tsfi_mf_nato_verify_segment_lifetime(int lifetime, int *is_valid) {
    if (!is_valid) return -1;
    *is_valid = (lifetime >= 1 && lifetime <= 3600) ? 1 : 0;
    return 0;
}

int tsfi_mf_nato_verify_rejection_reason(int reason, int *is_valid) {
    if (!is_valid) return -1;
    *is_valid = (reason >= 0 && reason <= 15) ? 1 : 0;
    return 0;
}

int tsfi_mf_nato_verify_packet_delay(int delay, int *is_valid) {
    if (!is_valid) return -1;
    *is_valid = (delay >= 0 && delay <= 120) ? 1 : 0;
    return 0;
}

int tsfi_mf_nato_verify_high_water_mark(int percentage, int *is_valid) {
    if (!is_valid) return -1;
    *is_valid = (percentage >= 50 && percentage <= 95) ? 1 : 0;
    return 0;
}

int tsfi_mf_nato_verify_retransmit_timeout(int timeout, int *is_valid) {
    if (!is_valid) return -1;
    *is_valid = (timeout >= 1 && timeout <= 600) ? 1 : 0;
    return 0;
}

int tsfi_mf_nato_verify_low_water_mark(int percentage, int *is_valid) {
    if (!is_valid) return -1;
    *is_valid = (percentage >= 10 && percentage <= 49) ? 1 : 0;
    return 0;
}

int tsfi_mf_nato_verify_multiscan_frame_count(int count, int *is_valid) {
    if (!is_valid) return -1;
    *is_valid = (count >= 1 && count <= 32) ? 1 : 0;
    return 0;
}

int tsfi_mf_nato_verify_idle_timeout(int timeout, int *is_valid) {
    if (!is_valid) return -1;
    *is_valid = (timeout >= 5 && timeout <= 600) ? 1 : 0;
    return 0;
}

int tsfi_mf_nato_verify_delay_interval(int interval, int *is_valid) {
    if (!is_valid) return -1;
    *is_valid = (interval >= 1 && interval <= 10) ? 1 : 0;
    return 0;
}

int tsfi_mf_nato_verify_low_water_mode(int mode, int *is_valid) {
    if (!is_valid) return -1;
    *is_valid = (mode >= 0 && mode <= 2) ? 1 : 0;
    return 0;
}

int tsfi_mf_nato_verify_adaptation_mode(int mode, int *is_valid) {
    if (!is_valid) return -1;
    *is_valid = (mode >= 0 && mode <= 2) ? 1 : 0;
    return 0;
}

int tsfi_mf_nato_verify_subnetwork_profile(int profile, int *is_valid) {
    if (!is_valid) return -1;
    *is_valid = (profile >= 0 && profile <= 2) ? 1 : 0;
    return 0;
}

int tsfi_mf_nato_verify_retry_delay(int delay_ms, int *is_valid) {
    if (!is_valid) return -1;
    *is_valid = (delay_ms >= 10 && delay_ms <= 600) ? 1 : 0;
    return 0;
}

int tsfi_mf_nato_verify_profile_override(int flag, int *is_valid) {
    if (!is_valid) return -1;
    *is_valid = (flag >= 0 && flag <= 2) ? 1 : 0;
    return 0;
}

int tsfi_mf_nato_verify_backoff_multiplier(int multiplier, int *is_valid) {
    if (!is_valid) return -1;
    *is_valid = (multiplier >= 1 && multiplier <= 4) ? 1 : 0;
    return 0;
}

int tsfi_mf_nato_verify_broadcast_address(int address, int *is_valid) {
    if (!is_valid) return -1;
    *is_valid = (address == 0x3FFFFFF) ? 1 : 0;
    return 0;
}

int tsfi_mf_nato_verify_broadcast_window(int mode, int window_size, int *is_valid) {
    if (!is_valid) return -1;
    *is_valid = ((mode == 0 && window_size == 0) || (mode != 0)) ? 1 : 0;
    return 0;
}

int tsfi_mf_nato_verify_broadcast_backoff_limit(int limit, int *is_valid) {
    if (!is_valid) return -1;
    *is_valid = (limit >= 2 && limit <= 16) ? 1 : 0;
    return 0;
}

int tsfi_mf_nato_verify_broadcast_queue_capacity(int capacity, int *is_valid) {
    if (!is_valid) return -1;
    *is_valid = (capacity >= 10 && capacity <= 1000) ? 1 : 0;
    return 0;
}

int tsfi_mf_nato_verify_backoff_algorithm(int algorithm, int *is_valid) {
    if (!is_valid) return -1;
    *is_valid = (algorithm >= 0 && algorithm <= 2) ? 1 : 0;
    return 0;
}

int tsfi_mf_nato_verify_channel_busy_threshold(int threshold_dbm, int *is_valid) {
    if (!is_valid) return -1;
    *is_valid = (threshold_dbm >= -110 && threshold_dbm <= -70) ? 1 : 0;
    return 0;
}

int tsfi_mf_nato_verify_channel_free_threshold(int threshold_dbm, int *is_valid) {
    if (!is_valid) return -1;
    *is_valid = (threshold_dbm >= -120 && threshold_dbm <= -80) ? 1 : 0;
    return 0;
}

int tsfi_mf_nato_verify_broadcast_repeats(int repeats, int *is_valid) {
    if (!is_valid) return -1;
    *is_valid = (repeats >= 1 && repeats <= 5) ? 1 : 0;
    return 0;
}

int tsfi_mf_nato_verify_collision_clear_wait(int wait_ms, int *is_valid) {
    if (!is_valid) return -1;
    *is_valid = (wait_ms >= 5 && wait_ms <= 120) ? 1 : 0;
    return 0;
}

int tsfi_mf_nato_verify_broadcast_guard_time(int guard_ms, int *is_valid) {
    if (!is_valid) return -1;
    *is_valid = (guard_ms >= 10 && guard_ms <= 500) ? 1 : 0;
    return 0;
}

int tsfi_mf_nato_verify_guard_time_flag(int flag, int *is_valid) {
    if (!is_valid) return -1;
    *is_valid = (flag >= 0 && flag <= 2) ? 1 : 0;
    return 0;
}

int tsfi_mf_nato_verify_broadcast_expiry(int expiry_sec, int *is_valid) {
    if (!is_valid) return -1;
    *is_valid = (expiry_sec >= 1 && expiry_sec <= 1800) ? 1 : 0;
    return 0;
}

int tsfi_mf_nato_verify_busy_detection_timeout(int timeout_sec, int *is_valid) {
    if (!is_valid) return -1;
    *is_valid = (timeout_sec >= 5 && timeout_sec <= 30) ? 1 : 0;
    return 0;
}

int tsfi_mf_nato_verify_channel_quality(int quality, int *is_valid) {
    if (!is_valid) return -1;
    *is_valid = (quality >= 0 && quality <= 100) ? 1 : 0;
    return 0;
}

int tsfi_mf_nato_verify_backoff_jitter(int jitter_pct, int *is_valid) {
    if (!is_valid) return -1;
    *is_valid = (jitter_pct >= 0 && jitter_pct <= 50) ? 1 : 0;
    return 0;
}

int tsfi_mf_nato_verify_guard_band(int spacing_ms, int *is_valid) {
    if (!is_valid) return -1;
    *is_valid = (spacing_ms >= 5 && spacing_ms <= 250) ? 1 : 0;
    return 0;
}

int tsfi_mf_nato_verify_signal_ramp(int ramp_ms, int *is_valid) {
    if (!is_valid) return -1;
    *is_valid = (ramp_ms >= 1 && ramp_ms <= 50) ? 1 : 0;
    return 0;
}

int tsfi_mf_nato_verify_rssi_drop(int drop_db, int *is_valid) {
    if (!is_valid) return -1;
    *is_valid = (drop_db >= 3 && drop_db <= 20) ? 1 : 0;
    return 0;
}

int tsfi_mf_nato_verify_backoff_cap(int cap, int *is_valid) {
    if (!is_valid) return -1;
    *is_valid = (cap >= 4 && cap <= 10) ? 1 : 0;
    return 0;
}

int tsfi_mf_nato_verify_rssi_hysteresis(int hysteresis_db, int *is_valid) {
    if (!is_valid) return -1;
    *is_valid = (hysteresis_db >= 1 && hysteresis_db <= 10) ? 1 : 0;
    return 0;
}

int tsfi_mf_nato_verify_busy_hysteresis_time(int hysteresis_ms, int *is_valid) {
    if (!is_valid) return -1;
    *is_valid = (hysteresis_ms >= 5 && hysteresis_ms <= 100) ? 1 : 0;
    return 0;
}

int tsfi_mf_nato_verify_rssi_drop_time(int drop_ms, int *is_valid) {
    if (!is_valid) return -1;
    *is_valid = (drop_ms >= 1 && drop_ms <= 20) ? 1 : 0;
    return 0;
}

int tsfi_mf_nato_verify_busy_samples(int samples, int *is_valid) {
    if (!is_valid) return -1;
    *is_valid = (samples >= 2 && samples <= 10) ? 1 : 0;
    return 0;
}

int tsfi_mf_nato_verify_rssi_drop_samples(int samples, int *is_valid) {
    if (!is_valid) return -1;
    *is_valid = (samples >= 2 && samples <= 8) ? 1 : 0;
    return 0;
}

int tsfi_mf_nato_verify_backoff_lfsr_size(int bits, int *is_valid) {
    if (!is_valid) return -1;
    *is_valid = (bits >= 8 && bits <= 16) ? 1 : 0;
    return 0;
}

int tsfi_mf_nato_verify_status_interval(int interval_sec, int *is_valid) {
    if (!is_valid) return -1;
    *is_valid = (interval_sec >= 1 && interval_sec <= 60) ? 1 : 0;
    return 0;
}

int tsfi_mf_nato_verify_up_ramp_rate(int rate_db, int *is_valid) {
    if (!is_valid) return -1;
    *is_valid = (rate_db >= 1 && rate_db <= 20) ? 1 : 0;
    return 0;
}

int tsfi_mf_nato_verify_down_ramp_rate(int rate_db, int *is_valid) {
    if (!is_valid) return -1;
    *is_valid = (rate_db >= 1 && rate_db <= 15) ? 1 : 0;
    return 0;
}

int tsfi_mf_nato_verify_backoff_base(int base, int *is_valid) {
    if (!is_valid) return -1;
    *is_valid = (base >= 2 && base <= 4) ? 1 : 0;
    return 0;
}

int tsfi_mf_nato_verify_busy_decay_rate(int rate_db, int *is_valid) {
    if (!is_valid) return -1;
    *is_valid = (rate_db >= 1 && rate_db <= 10) ? 1 : 0;
    return 0;
}

int tsfi_mf_nato_verify_backoff_scale(int scale_ms, int *is_valid) {
    if (!is_valid) return -1;
    *is_valid = (scale_ms >= 1 && scale_ms <= 500) ? 1 : 0;
    return 0;
}

int tsfi_mf_nato_verify_busy_hysteresis_threshold(int threshold_dbm, int *is_valid) {
    if (!is_valid) return -1;
    *is_valid = (threshold_dbm >= -90 && threshold_dbm <= -50) ? 1 : 0;
    return 0;
}

int tsfi_mf_nato_verify_jitter_dist(int dist, int *is_valid) {
    if (!is_valid) return -1;
    *is_valid = (dist == 0 || dist == 1) ? 1 : 0;
    return 0;
}

int tsfi_mf_nato_verify_up_ramp_duration(int duration_ms, int *is_valid) {
    if (!is_valid) return -1;
    *is_valid = (duration_ms >= 1 && duration_ms <= 10) ? 1 : 0;
    return 0;
}

int tsfi_mf_nato_verify_slot_map(int map, int *is_valid) {
    if (!is_valid) return -1;
    *is_valid = (map >= 0 && map <= 2) ? 1 : 0;
    return 0;
}

int tsfi_mf_nato_verify_down_ramp_duration(int duration_ms, int *is_valid) {
    if (!is_valid) return -1;
    *is_valid = (duration_ms >= 1 && duration_ms <= 15) ? 1 : 0;
    return 0;
}

int tsfi_mf_nato_verify_slot_duration(int duration_ms, int *is_valid) {
    if (!is_valid) return -1;
    *is_valid = (duration_ms >= 5 && duration_ms <= 50) ? 1 : 0;
    return 0;
}

int tsfi_mf_nato_verify_up_ramp_samples(int samples, int *is_valid) {
    if (!is_valid) return -1;
    *is_valid = (samples >= 2 && samples <= 6) ? 1 : 0;
    return 0;
}

int tsfi_mf_nato_verify_slot_offset(int offset_ms, int *is_valid) {
    if (!is_valid) return -1;
    *is_valid = (offset_ms >= 0 && offset_ms <= 10) ? 1 : 0;
    return 0;
}

int tsfi_mf_nato_verify_down_ramp_samples(int samples, int *is_valid) {
    if (!is_valid) return -1;
    *is_valid = (samples >= 2 && samples <= 8) ? 1 : 0;
    return 0;
}

int tsfi_mf_nato_verify_slot_method(int method, int *is_valid) {
    if (!is_valid) return -1;
    *is_valid = (method >= 0 && method <= 2) ? 1 : 0;
    return 0;
}

int tsfi_mf_nato_verify_up_ramp_rate_threshold(int rate_db, int *is_valid) {
    if (!is_valid) return -1;
    *is_valid = (rate_db >= 3 && rate_db <= 15) ? 1 : 0;
    return 0;
}

int tsfi_mf_nato_verify_down_ramp_rate_threshold(int rate_db, int *is_valid) {
    if (!is_valid) return -1;
    *is_valid = (rate_db >= 2 && rate_db <= 12) ? 1 : 0;
    return 0;
}

int tsfi_mf_nato_verify_slot_boundary(int boundary_ms, int *is_valid) {
    if (!is_valid) return -1;
    *is_valid = (boundary_ms >= 1 && boundary_ms <= 5) ? 1 : 0;
    return 0;
}

int tsfi_mf_nato_verify_slot_clock_source(int source, int *is_valid) {
    if (!is_valid) return -1;
    *is_valid = (source >= 0 && source <= 2) ? 1 : 0;
    return 0;
}

int tsfi_mf_nato_verify_up_ramp_rate_factor(int factor, int *is_valid) {
    if (!is_valid) return -1;
    *is_valid = (factor >= 1 && factor <= 5) ? 1 : 0;
    return 0;
}

int tsfi_mf_nato_verify_down_ramp_rate_factor(int factor, int *is_valid) {
    if (!is_valid) return -1;
    *is_valid = (factor >= 1 && factor <= 5) ? 1 : 0;
    return 0;
}

int tsfi_mf_nato_verify_slot_method_option(int option, int *is_valid) {
    if (!is_valid) return -1;
    *is_valid = (option >= 0 && option <= 3) ? 1 : 0;
    return 0;
}

int tsfi_mf_nato_verify_slot_boundary_option(int option, int *is_valid) {
    if (!is_valid) return -1;
    *is_valid = (option >= 0 && option <= 2) ? 1 : 0;
    return 0;
}

int tsfi_mf_nato_verify_up_ramp_rate_limit(int limit_db, int *is_valid) {
    if (!is_valid) return -1;
    *is_valid = (limit_db >= 5 && limit_db <= 25) ? 1 : 0;
    return 0;
}

int tsfi_mf_nato_verify_down_ramp_rate_limit(int limit_db, int *is_valid) {
    if (!is_valid) return -1;
    *is_valid = (limit_db >= 3 && limit_db <= 20) ? 1 : 0;
    return 0;
}

int tsfi_mf_nato_verify_slot_offset_boundary(int boundary_ms, int *is_valid) {
    if (!is_valid) return -1;
    *is_valid = (boundary_ms >= 1 && boundary_ms <= 8) ? 1 : 0;
    return 0;
}

int tsfi_mf_nato_verify_slot_boundary_source(int source, int *is_valid) {
    if (!is_valid) return -1;
    *is_valid = (source >= 0 && source <= 2) ? 1 : 0;
    return 0;
}

int tsfi_mf_nato_verify_up_ramp_rate_factor_offset(int offset_db, int *is_valid) {
    if (!is_valid) return -1;
    *is_valid = (offset_db >= 0 && offset_db <= 5) ? 1 : 0;
    return 0;
}

int tsfi_mf_nato_verify_down_ramp_rate_factor_offset(int offset_db, int *is_valid) {
    if (!is_valid) return -1;
    *is_valid = (offset_db >= 0 && offset_db <= 5) ? 1 : 0;
    return 0;
}

int tsfi_mf_nato_verify_slot_boundary_selector(int selector, int *is_valid) {
    if (!is_valid) return -1;
    *is_valid = (selector >= 0 && selector <= 2) ? 1 : 0;
    return 0;
}

int tsfi_mf_nato_verify_slot_boundary_source_offset(int offset_ms, int *is_valid) {
    if (!is_valid) return -1;
    *is_valid = (offset_ms >= 0 && offset_ms <= 12) ? 1 : 0;
    return 0;
}

int tsfi_mf_nato_verify_up_ramp_duration_threshold(int duration_ms, int *is_valid) {
    if (!is_valid) return -1;
    *is_valid = (duration_ms >= 2 && duration_ms <= 15) ? 1 : 0;
    return 0;
}

int tsfi_mf_nato_verify_down_ramp_duration_threshold(int duration_ms, int *is_valid) {
    if (!is_valid) return -1;
    *is_valid = (duration_ms >= 2 && duration_ms <= 20) ? 1 : 0;
    return 0;
}

int tsfi_mf_nato_verify_slot_boundary_source_offset_limit(int limit_ms, int *is_valid) {
    if (!is_valid) return -1;
    *is_valid = (limit_ms >= 1 && limit_ms <= 15) ? 1 : 0;
    return 0;
}

int tsfi_mf_nato_verify_slot_boundary_source_offset_range(int range_ms, int *is_valid) {
    if (!is_valid) return -1;
    *is_valid = (range_ms >= 0 && range_ms <= 20) ? 1 : 0;
    return 0;
}

int tsfi_mf_nato_verify_rssi_quiet_sample_threshold(int samples, int *is_valid) {
    if (!is_valid) return -1;
    *is_valid = (samples >= 1 && samples <= 10) ? 1 : 0;
    return 0;
}

int tsfi_mf_nato_verify_parameter(int param_type, int value, int *is_valid) {
    if (!is_valid) return -1;
    int min_val = 0;
    int max_val = 0;
    switch (param_type) {
#define X(enum_sfx, fn_sfx, min, max) \
        case TSFI_NATO_PARAM_##enum_sfx: min_val = min; max_val = max; break;
        NATO_PARAMETER_TABLE
#undef X
        
    }
    *is_valid = (value >= min_val && value <= max_val) ? 1 : 0;
    return 0;
}

#define X(enum_sfx, fn_sfx, min, max) \
int tsfi_mf_nato_verify_##fn_sfx(int value, int *is_valid) { \
    return tsfi_mf_nato_verify_parameter(TSFI_NATO_PARAM_##enum_sfx, value, is_valid); \
}
NATO_PARAMETER_TABLE
#undef X

int tsfi_mf_nato_verify_sis_flow_control(int state, int *is_valid) {
    if (!is_valid) return -1;
    *is_valid = (state >= 0 && state <= 3) ? 1 : 0;
    return 0;
}

int tsfi_mf_nato_verify_sis_sap_allocation(int sap_id, int *is_valid) {
    if (!is_valid) return -1;
    *is_valid = (sap_id >= 0 && sap_id <= 15) ? 1 : 0;
    return 0;
}

int tsfi_mf_nato_verify_fec_code_rate(int rate_pct, int *is_valid) {
    if (!is_valid) return -1;
    *is_valid = (rate_pct == 50 || rate_pct == 75 || rate_pct == 87) ? 1 : 0;
    return 0;
}

int tsfi_mf_nato_verify_fec_block_size(int size_bytes, int *is_valid) {
    if (!is_valid) return -1;
    *is_valid = (size_bytes == 128 || size_bytes == 256 || size_bytes == 512 || size_bytes == 1024) ? 1 : 0;
    return 0;
}

int tsfi_mf_nato_parse_sis_primitive(const uint8_t *data, size_t size, int *prim_type, int *is_valid) {
    if (!data || !prim_type || !is_valid || size == 0) return -1;
    
    *prim_type = data[0];
    *is_valid = 0;
    
    if (data[0] == 0x01) { // C_Bind
        if (size >= 4) *is_valid = 1;
    } else if (data[0] == 0x02) { // C_Unbind
        if (size >= 2) *is_valid = 1;
    } else if (data[0] == 0x03) { // U_Data
        if (size >= 6) *is_valid = 1;
    }
    
    return 0;
}

int tsfi_mf_nato_encode_d_pdu(int type, const uint8_t *payload, size_t pay_size, uint8_t *out_frame, size_t *out_size) {
    if (!payload || !out_frame || !out_size) return -1;
    
    out_frame[0] = type & 0x0F;
    out_frame[1] = (pay_size >> 8) & 0xFF;
    out_frame[2] = pay_size & 0xFF;
    
    for (size_t i = 0; i < pay_size; i++) {
        out_frame[3 + i] = payload[i];
    }
    
    size_t header_and_pay_len = 3 + pay_size;
    
    // CRC-16 CCITT-FALSE
    uint16_t crc = 0xFFFF;
    for (size_t i = 0; i < header_and_pay_len; i++) {
        crc ^= (uint16_t)out_frame[i] << 8;
        for (int j = 0; j < 8; j++) {
            if (crc & 0x8000) {
                crc = (crc << 1) ^ 0x1021;
            } else {
                crc <<= 1;
            }
        }
    }
    
    out_frame[header_and_pay_len] = (crc >> 8) & 0xFF;
    out_frame[header_and_pay_len + 1] = crc & 0xFF;
    
    *out_size = header_and_pay_len + 2;
    return 0;
}

int tsfi_mf_nato_arq_update(int event, int seq_num, int window_size, int *next_expected, int *is_valid) {
    if (!next_expected || !is_valid) return -1;
    if (seq_num < 0 || seq_num >= 128 || window_size < 1 || window_size > 64) {
        *is_valid = 0;
        return 0;
    }
    
    int limit = (*next_expected + window_size) % 128;
    int in_window = 0;
    if (*next_expected + window_size <= 128) {
        if (seq_num >= *next_expected && seq_num < *next_expected + window_size) {
            in_window = 1;
        }
    } else {
        if (seq_num >= *next_expected || seq_num < limit) {
            in_window = 1;
        }
    }
    
    if (in_window) {
        *is_valid = 1;
        if (event == 0 && seq_num == *next_expected) {
            *next_expected = (*next_expected + 1) % 128;
        }
    } else {
        *is_valid = 0;
    }
    
    return 0;
}

int tsfi_mf_nato_encode_u_pdu_header(int dest_sap, int src_sap, int priority, int is_segment, uint8_t *out_hdr, size_t *out_size) {
    if (!out_hdr || !out_size) return -1;
    if (dest_sap < 0 || dest_sap > 15 || src_sap < 0 || src_sap > 15 || priority < 0 || priority > 15) {
        return -2;
    }
    
    out_hdr[0] = ((dest_sap & 0x0F) << 4) | (src_sap & 0x0F);
    out_hdr[1] = (priority & 0x0F) | (is_segment ? 0x80 : 0x00);
    *out_size = 2;
    return 0;
}

int tsfi_mf_nato_decode_d_pdu(const uint8_t *frame, size_t size, int *type, uint8_t *payload_out, size_t *pay_size_out, int *is_valid) {
    if (!frame || !type || !payload_out || !pay_size_out || !is_valid) return -1;
    
    *is_valid = 0;
    if (size < 5) return 0;
    
    int extracted_type = frame[0] & 0x0F;
    size_t pay_size = (frame[1] << 8) | frame[2];
    
    if (size != 3 + pay_size + 2) return 0;
    
    // Verify CRC CCITT-FALSE
    uint16_t crc = 0xFFFF;
    for (size_t i = 0; i < 3 + pay_size; i++) {
        crc ^= (uint16_t)frame[i] << 8;
        for (int j = 0; j < 8; j++) {
            if (crc & 0x8000) {
                crc = (crc << 1) ^ 0x1021;
            } else {
                crc <<= 1;
            }
        }
    }
    
    uint16_t frame_crc = (frame[3 + pay_size] << 8) | frame[3 + pay_size + 1];
    if (crc == frame_crc) {
        *type = extracted_type;
        *pay_size_out = pay_size;
        for (size_t i = 0; i < pay_size; i++) {
            payload_out[i] = frame[3 + i];
        }
        *is_valid = 1;
    }
    
    return 0;
}

int tsfi_mf_nato_encode_address(int node_addr, int sub_node_addr, uint8_t *out_byte) {
    if (!out_byte) return -1;
    if (node_addr < 0 || node_addr > 7 || sub_node_addr < 0 || sub_node_addr > 15) {
        return -2;
    }
    *out_byte = ((node_addr & 0x07) << 4) | (sub_node_addr & 0x0F);
    return 0;
}

int tsfi_mf_nato_decode_address(uint8_t addr_byte, int *node_addr, int *sub_node_addr) {
    if (!node_addr || !sub_node_addr) return -1;
    *node_addr = (addr_byte >> 4) & 0x07;
    *sub_node_addr = addr_byte & 0x0F;
    return 0;
}

int tsfi_mf_nato_encode_eot(int reason, uint8_t *out_frame, size_t *out_size) {
    if (!out_frame || !out_size) return -1;
    out_frame[0] = 0x0F;
    out_frame[1] = reason & 0xFF;
    *out_size = 2;
    return 0;
}

int tsfi_mf_nato_decode_eot(const uint8_t *frame, size_t size, int *reason, int *is_valid) {
    if (!frame || !reason || !is_valid) return -1;
    *is_valid = 0;
    if (size == 2 && frame[0] == 0x0F) {
        *reason = frame[1];
        *is_valid = 1;
    }
    return 0;
}

int tsfi_mf_nato_generate_bind_confirm(int status, int max_saps, uint8_t *out_pkt, size_t *out_size) {
    if (!out_pkt || !out_size) return -1;
    out_pkt[0] = 0x81;
    out_pkt[1] = status & 0xFF;
    out_pkt[2] = max_saps & 0xFF;
    *out_size = 3;
    return 0;
}

int tsfi_mf_nato_phy_link_update(int event, int *current_state, int *is_valid) {
    if (!current_state || !is_valid) return -1;
    
    *is_valid = 0;
    if (event == 4) { // RESET
        *current_state = 0;
        *is_valid = 1;
        return 0;
    }
    
    switch (*current_state) {
        case 0: // Disconnected
            if (event == 0) { // CONNECT_REQ
                *current_state = 1;
                *is_valid = 1;
            }
            break;
        case 1: // Connecting
            if (event == 1) { // CONNECT_CONF
                *current_state = 2;
                *is_valid = 1;
            }
            break;
        case 2: // Connected
            if (event == 2) { // DISCONNECT_REQ
                *current_state = 3;
                *is_valid = 1;
            }
            break;
        case 3: // Disconnecting
            if (event == 3) { // DISCONNECT_CONF
                *current_state = 0;
                *is_valid = 1;
            }
            break;
    }
    
    return 0;
}

int tsfi_mf_nato_verify_segment_bounds(int offset, int seg_size, int total_size, int is_last, int *is_valid) {
    if (!is_valid) return -1;
    *is_valid = 0;
    if (offset < 0 || seg_size <= 0 || total_size <= 0) return 0;
    if (offset + seg_size > total_size) return 0;
    
    if (is_last) {
        if (offset + seg_size == total_size) {
            *is_valid = 1;
        }
    } else {
        if (offset + seg_size < total_size) {
            *is_valid = 1;
        }
    }
    return 0;
}

int tsfi_mf_nato_phy_link_evaluate_helmholtz(int op_type, float value, int *current_epoch, int *is_valid) {
    (void)value;
    if (!current_epoch || !is_valid) return -1;
    *is_valid = 0;
    
    switch (op_type) {
        case 1: // HILBERT_ENCODE
            if (*current_epoch == 0) { // EPOCH_INIT -> EPOCH_AVAIL
                *current_epoch = 1;
                *is_valid = 1;
            } else if (*current_epoch == 1) { // EPOCH_AVAIL -> EPOCH_FORM
                *current_epoch = 2;
                *is_valid = 1;
            }
            break;
        case 2: // BANACH_NORM
            if (*current_epoch == 2) { // EPOCH_FORM -> EPOCH_POLARIZE
                *current_epoch = 3;
                *is_valid = 1;
            } else if (*current_epoch == 3) { // EPOCH_POLARIZE -> EPOCH_CONJUGATE
                *current_epoch = 4;
                *is_valid = 1;
            }
            break;
        case 3: // DIFFUSION
            if (*current_epoch == 4) { // EPOCH_CONJUGATE -> EPOCH_CONIFY
                *current_epoch = 5;
                *is_valid = 1;
            } else if (*current_epoch == 5) { // EPOCH_CONIFY -> EPOCH_SATURATE
                *current_epoch = 6;
                *is_valid = 1;
            }
            break;
        case 4: // RESONANCE
            if (*current_epoch == 6) { // EPOCH_SATURATE -> EPOCH_IONIZE
                *current_epoch = 7;
                *is_valid = 1;
            } else if (*current_epoch == 7) { // EPOCH_IONIZE -> EPOCH_MAGNETIZE
                *current_epoch = 8;
                *is_valid = 1;
            } else if (*current_epoch == 8) { // EPOCH_MAGNETIZE -> EPOCH_DONE
                *current_epoch = 9;
                *is_valid = 1;
            }
            break;
    }
    
    return 0;
}

int tsfi_mf_nato_hfrcp_update(int action, int frequency_khz, int *current_state, int *is_valid) {
    if (!current_state || !is_valid) return -1;
    *is_valid = 0;
    
    switch (action) {
        case 0: // TUNE_FREQ
            if ((*current_state == 0 || *current_state == 3) && frequency_khz >= 2000 && frequency_khz <= 30000) {
                *current_state = 1; // Tuning
                *is_valid = 1;
            }
            break;
        case 1: // TUNE_COMPLETE
            if (*current_state == 1) {
                *current_state = 3; // Listening
                *is_valid = 1;
            }
            break;
        case 2: // START_TX
            if (*current_state == 3) {
                *current_state = 2; // Transmitting
                *is_valid = 1;
            }
            break;
        case 3: // STOP_TX
            if (*current_state == 2) {
                *current_state = 3; // Listening
                *is_valid = 1;
            }
            break;
        case 4: // GO_IDLE
            if (*current_state == 3) {
                *current_state = 0; // Idle
                *is_valid = 1;
            }
            break;
    }
    return 0;
}

int tsfi_mf_nato_verify_throttle_limit(int queue_size, int threshold, int *is_throttled, int *is_valid) {
    if (!is_throttled || !is_valid) return -1;
    *is_valid = 0;
    if (queue_size < 0 || threshold <= 0) return 0;
    
    *is_valid = 1;
    *is_throttled = (queue_size >= threshold) ? 1 : 0;
    return 0;
}

int tsfi_mf_nato_hfrcp_evaluate_helmholtz(int op_type, float value, int *current_epoch, int *is_valid) {
    (void)value;
    if (!current_epoch || !is_valid) return -1;
    *is_valid = 0;
    
    switch (op_type) {
        case 1: // HILBERT_ENCODE (prepares carrier freq config)
            if (*current_epoch == 0) { // EPOCH_INIT -> EPOCH_AVAIL
                *current_epoch = 1;
                *is_valid = 1;
            }
            break;
        case 2: // BANACH_NORM (tunes antenna impedance match)
            if (*current_epoch == 1) { // EPOCH_AVAIL -> EPOCH_FORM
                *current_epoch = 2;
                *is_valid = 1;
            } else if (*current_epoch == 2) { // EPOCH_FORM -> EPOCH_POLARIZE
                *current_epoch = 3;
                *is_valid = 1;
            }
            break;
        case 3: // DIFFUSION (spreads frequency lock loops)
            if (*current_epoch == 3) { // EPOCH_POLARIZE -> EPOCH_CONJUGATE
                *current_epoch = 4;
                *is_valid = 1;
            } else if (*current_epoch == 4) { // EPOCH_CONJUGATE -> EPOCH_CONIFY
                *current_epoch = 5;
                *is_valid = 1;
            } else if (*current_epoch == 5) { // EPOCH_CONIFY -> EPOCH_SATURATE
                *current_epoch = 6;
                *is_valid = 1;
            }
            break;
        case 4: // RESONANCE (keys power amp lock)
            if (*current_epoch == 6) { // EPOCH_SATURATE -> EPOCH_IONIZE
                *current_epoch = 7;
                *is_valid = 1;
            } else if (*current_epoch == 7) { // EPOCH_IONIZE -> EPOCH_MAGNETIZE
                *current_epoch = 8;
                *is_valid = 1;
            } else if (*current_epoch == 8) { // EPOCH_MAGNETIZE -> EPOCH_DONE
                *current_epoch = 9;
                *is_valid = 1;
            }
            break;
    }
    return 0;
}

int tsfi_mf_nato_afsk_verify(int tone_freq_hz, int baud_rate, int *is_valid) {
    if (!is_valid) return -1;
    *is_valid = 0;
    
    // Check standard STANAG 5066 AFSK tone limits
    if (tone_freq_hz != 875 && tone_freq_hz != 2025 && tone_freq_hz != 1800) {
        return 0;
    }
    
    // Check standard baud rates
    if (baud_rate != 75 && baud_rate != 150 && baud_rate != 300 &&
        baud_rate != 600 && baud_rate != 1200 && baud_rate != 2400) {
        return 0;
    }
    
    *is_valid = 1;
    return 0;
}

int tsfi_mf_nato_merkle_combine(uint32_t left_hash, uint32_t right_hash, uint32_t *parent_hash) {
    if (!parent_hash) return -1;
    *parent_hash = (left_hash * 33) ^ right_hash ^ 0x55555555;
    return 0;
}

int tsfi_mf_nato_drs_update(int rssi, int snr, int *current_rate, int *is_valid) {
    if (!current_rate || !is_valid) return -1;
    *is_valid = 0;
    
    if (rssi < -120 || rssi > 0 || snr < -10 || snr > 40) {
        return 0;
    }
    
    *is_valid = 1;
    if (snr >= 25) {
        *current_rate = 9600;
    } else if (snr >= 15) {
        *current_rate = 4800;
    } else if (snr >= 8) {
        *current_rate = 1200;
    } else if (snr >= 3) {
        *current_rate = 300;
    } else {
        *current_rate = 75;
    }
    return 0;
}

int tsfi_mf_nato_reorder_buffer_insert(int seq_num, int expected_seq, int *is_valid) {
    if (!is_valid) return -1;
    *is_valid = 0;
    
    if (seq_num < 0 || seq_num >= 128 || expected_seq < 0 || expected_seq >= 128) {
        return 0;
    }
    
    // Check if within expected reorder window of 8 frames (modulo 128)
    int diff = (seq_num - expected_seq + 128) % 128;
    if (diff < 8) {
        *is_valid = 1;
    }
    return 0;
}

int tsfi_mf_nato_hmtp_encode_envelope(const char *sender, const char *recipient, uint8_t *out_pkt, size_t *out_size) {
    if (!sender || !recipient || !out_pkt || !out_size) return -1;
    
    size_t s_len = strlen(sender);
    size_t r_len = strlen(recipient);
    
    out_pkt[0] = 0x4D; // "M" prefix for Mail
    memcpy(out_pkt + 1, sender, s_len);
    out_pkt[1 + s_len] = ',';
    memcpy(out_pkt + 1 + s_len + 1, recipient, r_len);
    out_pkt[1 + s_len + 1 + r_len] = ';';
    
    *out_size = 1 + s_len + 1 + r_len + 1;
    return 0;
}

int tsfi_mf_nato_csm_evaluate(int noise_floor_dbm, int carrier_lock_ms, int *status_flags, int *is_valid) {
    if (!status_flags || !is_valid) return -1;
    *is_valid = 0;
    
    if (noise_floor_dbm < -150 || noise_floor_dbm > -50 || carrier_lock_ms < 0) {
        return 0;
    }
    
    *is_valid = 1;
    *status_flags = 0;
    if (noise_floor_dbm < -100) {
        *status_flags |= 0x01; // CHANNEL_QUIET
    }
    if (carrier_lock_ms > 200) {
        *status_flags |= 0x02; // CARRIER_LOCK
    }
    return 0;
}

int tsfi_mf_nato_fragment_payload(const uint8_t *payload, size_t pay_size, size_t mtu, int frag_idx, uint8_t *out_frag, size_t *out_frag_size, int *is_last) {
    if (!payload || !out_frag || !out_frag_size || !is_last) return -1;
    if (pay_size == 0 || mtu == 0) return -2;
    
    size_t offset = (size_t)frag_idx * mtu;
    if (offset >= pay_size) {
        *out_frag_size = 0;
        *is_last = 0;
        return 0;
    }
    
    size_t chunk_size = pay_size - offset;
    if (chunk_size > mtu) {
        chunk_size = mtu;
        *is_last = 0;
    } else {
        *is_last = 1;
    }
    
    memcpy(out_frag, payload + offset, chunk_size);
    *out_frag_size = chunk_size;
    return 0;
}

int tsfi_mf_nato_generate_unbind_confirm(int reason, uint8_t *out_pkt, size_t *out_size) {
    if (!out_pkt || !out_size) return -1;
    out_pkt[0] = 0x82;
    out_pkt[1] = reason & 0xFF;
    *out_size = 2;
    return 0;
}

int tsfi_mf_nato_generate_warning(int warning_code, uint8_t *out_pkt, size_t *out_size) {
    if (!out_pkt || !out_size) return -1;
    out_pkt[0] = 0x8F;
    out_pkt[1] = warning_code & 0xFF;
    *out_size = 2;
    return 0;
}

int tsfi_mf_nato_generate_connect_reject(int reject_reason, uint8_t *out_pkt, size_t *out_size) {
    if (!out_pkt || !out_size) return -1;
    out_pkt[0] = 0x83;
    out_pkt[1] = reject_reason & 0xFF;
    *out_size = 2;
    return 0;
}

int tsfi_mf_nato_encode_d_pdu_type1(int dest_sap, int src_sap, const uint8_t *payload, size_t pay_size, uint8_t *out_frame, size_t *out_size) {
    if (!out_frame || !out_size) return -1;
    if (pay_size > 0 && !payload) return -2;
    
    out_frame[0] = 0x01; // D_PDU Type 1
    out_frame[1] = ((dest_sap & 0x0F) << 4) | (src_sap & 0x0F);
    if (pay_size > 0) {
        memcpy(out_frame + 2, payload, pay_size);
    }
    *out_size = 2 + pay_size;
    return 0;
}

int tsfi_mf_nato_generate_hard_reset(int reset_code, uint8_t *out_pkt, size_t *out_size) {
    if (!out_pkt || !out_size) return -1;
    out_pkt[0] = 0x8C;
    out_pkt[1] = reset_code & 0xFF;
    *out_size = 2;
    return 0;
}

int tsfi_mf_nato_generate_flow_clear(int sap_id, uint8_t *out_pkt, size_t *out_size) {
    if (!out_pkt || !out_size) return -1;
    out_pkt[0] = 0x8E;
    out_pkt[1] = sap_id & 0xFF;
    *out_size = 2;
    return 0;
}

int tsfi_mf_nato_arq_detect_duplicate(int seq_num, const uint8_t *received_mask, int expected_seq, int *is_duplicate, int *is_valid) {
    if (!received_mask || !is_duplicate || !is_valid) return -1;
    *is_valid = 0;
    *is_duplicate = 0;
    
    if (seq_num < 0 || seq_num >= 128 || expected_seq < 0 || expected_seq >= 128) {
        return 0;
    }
    
    int diff = (seq_num - expected_seq + 128) % 128;
    if (diff < 32) {
        *is_valid = 1;
        int byte_idx = diff / 8;
        int bit_idx = diff % 8;
        if (received_mask[byte_idx] & (1 << bit_idx)) {
            *is_duplicate = 1;
        }
    }
    return 0;
}

int tsfi_mf_nato_encode_d_pdu_type2(int dest_sap, int src_sap, int tx_seq, const uint8_t *payload, size_t pay_size, uint8_t *out_frame, size_t *out_size) {
    if (!out_frame || !out_size) return -1;
    if (pay_size > 0 && !payload) return -2;
    
    out_frame[0] = 0x02; // D_PDU Type 2
    out_frame[1] = ((dest_sap & 0x0F) << 4) | (src_sap & 0x0F);
    out_frame[2] = tx_seq & 0xFF;
    if (pay_size > 0) {
        memcpy(out_frame + 3, payload, pay_size);
    }
    *out_size = 3 + pay_size;
    return 0;
}

int tsfi_mf_nato_generate_flow_control(int flow_action, int sap_id, uint8_t *out_pkt, size_t *out_size) {
    if (!out_pkt || !out_size) return -1;
    out_pkt[0] = 0x8D;
    out_pkt[1] = ((flow_action & 0x0F) << 4) | (sap_id & 0x0F);
    *out_size = 2;
    return 0;
}

int tsfi_mf_nato_generate_flow_alert(int alert_reason, uint8_t *out_pkt, size_t *out_size) {
    if (!out_pkt || !out_size) return -1;
    out_pkt[0] = 0x89;
    out_pkt[1] = alert_reason & 0xFF;
    *out_size = 2;
    return 0;
}

int tsfi_mf_nato_encode_d_pdu_type8(int dest_sap, int src_sap, int is_ack, uint8_t *out_frame, size_t *out_size) {
    if (!out_frame || !out_size) return -1;
    out_frame[0] = 0x08; // D_PDU Type 8
    out_frame[1] = ((dest_sap & 0x0F) << 4) | (src_sap & 0x0F);
    out_frame[2] = is_ack ? 1 : 0;
    *out_size = 3;
    return 0;
}

int tsfi_mf_nato_encode_d_pdu_type9(int dest_sap, int src_sap, int ack_seq, const uint8_t *ack_mask, uint8_t *out_frame, size_t *out_size) {
    if (!out_frame || !out_size || !ack_mask) return -1;
    out_frame[0] = 0x09; // D_PDU Type 9
    out_frame[1] = ((dest_sap & 0x0F) << 4) | (src_sap & 0x0F);
    out_frame[2] = ack_seq & 0xFF;
    memcpy(out_frame + 3, ack_mask, 4);
    *out_size = 7;
    return 0;
}

int tsfi_mf_nato_encode_d_pdu_type3(int dest_sap, int src_sap, const uint8_t *payload, size_t pay_size, uint8_t *out_frame, size_t *out_size) {
    if (!out_frame || !out_size) return -1;
    if (pay_size > 0 && !payload) return -2;
    out_frame[0] = 0x03; // D_PDU Type 3
    out_frame[1] = ((dest_sap & 0x0F) << 4) | (src_sap & 0x0F);
    if (pay_size > 0) {
        memcpy(out_frame + 2, payload, pay_size);
    }
    *out_size = 2 + pay_size;
    return 0;
}

int tsfi_mf_norad_encode_janap128(const char *routing_ind, int chan_seq, char classification, char *out_pkt, size_t *out_size) {
    if (!routing_ind || !out_pkt || !out_size) return -1;
    
    size_t ri_len = strlen(routing_ind);
    if (ri_len < 3 || ri_len > 8) return -2;
    if (chan_seq < 0 || chan_seq > 9999) return -3;
    if (classification != 'U' && classification != 'C' && classification != 'S' && classification != 'T') return -4;
    
    int written = sprintf(out_pkt, "R %s %04d %c\r\n", routing_ind, chan_seq, classification);
    if (written < 0) return -5;
    
    *out_size = (size_t)written;
    return 0;
}

int tsfi_mf_norad_encode_defcon(int defcon_level, int radar_contacts, uint16_t *out_status, int *is_valid) {
    if (!out_status || !is_valid) return -1;
    *is_valid = 0;
    
    if (defcon_level < 1 || defcon_level > 5 || radar_contacts < 0 || radar_contacts > 99) {
        return 0;
    }
    
    *is_valid = 1;
    int alert = (defcon_level <= 3) ? 1 : 0;
    *out_status = (defcon_level & 0x07) | ((radar_contacts & 0x7F) << 3) | (alert << 10);
    return 0;
}

int tsfi_mf_norad_naap_update(int event, int *current_state, int *is_valid) {
    if (!current_state || !is_valid) return -1;
    *is_valid = 0;
    
    switch (*current_state) {
        case 0: // IDLE
            if (event == 0) { // TRIGGER_ALERT
                *current_state = 1; // ALERT_ISSUED
                *is_valid = 1;
            }
            break;
        case 1: // ALERT_ISSUED
            if (event == 1) { // SEND_ACK
                *current_state = 2; // ACK_PENDING
                *is_valid = 1;
            }
            break;
        case 2: // ACK_PENDING
            if (event == 2) { // CONFIRM_ALERT
                *current_state = 3; // CONFIRMED
                *is_valid = 1;
            }
            break;
        case 3: // CONFIRMED
            if (event == 3) { // RESET_ALERT
                *current_state = 0; // IDLE
                *is_valid = 1;
            }
            break;
    }
    return 0;
}

int tsfi_mf_norad_detect_eom(const char *msg_buffer, size_t size, int *is_eom_detected) {
    if (!msg_buffer || !is_eom_detected) return -1;
    *is_eom_detected = 0;
    
    // Scan for standard teletype message terminal sequence "NNNN"
    if (size >= 4) {
        for (size_t i = 0; i <= size - 4; i++) {
            if (msg_buffer[i] == 'N' && msg_buffer[i+1] == 'N' &&
                msg_buffer[i+2] == 'N' && msg_buffer[i+3] == 'N') {
                *is_eom_detected = 1;
                break;
            }
        }
    }
    return 0;
}

int tsfi_mf_cics_map_janap_transid(const char *routing_ind, char *out_transid, int *is_valid) {
    if (!routing_ind || !out_transid || !is_valid) return -1;
    *is_valid = 0;
    
    if (strcmp(routing_ind, "RUMJTF") == 0) {
        *is_valid = 1;
        strcpy(out_transid, "NJTF");
    } else if (strcmp(routing_ind, "RUMNDH") == 0) {
        *is_valid = 1;
        strcpy(out_transid, "NNDH");
    } else {
        *is_valid = 1;
        strcpy(out_transid, "NDFT");
    }
    return 0;
}

int tsfi_mf_cics_generate_naap_broadcast(int defcon_level, uint16_t term_id, uint8_t *out_pdu, size_t *out_size) {
    if (!out_pdu || !out_size) return -1;
    out_pdu[0] = 0xBC; // Broadcast PDU marker
    out_pdu[1] = defcon_level & 0xFF;
    out_pdu[2] = (term_id >> 8) & 0xFF;
    out_pdu[3] = term_id & 0xFF;
    *out_size = 4;
    return 0;
}

int tsfi_mf_cics_extract_janap_payload(const char *janap_msg, size_t msg_size, char *out_payload, size_t *out_size) {
    if (!janap_msg || !out_payload || !out_size) return -1;
    
    // Find the end of the first line (header)
    const char *newline = strchr(janap_msg, '\n');
    if (!newline) return -2;
    
    const char *payload_start = newline + 1;
    
    // Find EOM NNNN
    const char *eom = strstr(payload_start, "NNNN");
    if (!eom) return -3;
    
    // Compute length
    size_t len = eom - payload_start;
    
    // Trim trailing carriage return or newline
    while (len > 0 && (payload_start[len - 1] == '\r' || payload_start[len - 1] == '\n')) {
        len--;
    }
    
    if (len >= msg_size) {
        len = msg_size - 1;
    }
    
    memcpy(out_payload, payload_start, len);
    out_payload[len] = '\0';
    *out_size = len;
    return 0;
}

int tsfi_mf_cics_authorize_transaction(const char *transid, uint16_t naap_status, int *is_authorized) {
    if (!transid || !is_authorized) return -1;
    
    int defcon_level = naap_status & 0x07;
    if (defcon_level == 1 || defcon_level == 2) {
        // Only allow critical transactions
        if (strcmp(transid, "NJTF") == 0 || strcmp(transid, "NNDH") == 0) {
            *is_authorized = 1;
        } else {
            *is_authorized = 0;
        }
    } else {
        *is_authorized = 1;
    }
    return 0;
}

int tsfi_mf_cics_format_3270_map(int defcon_level, char *out_stream, size_t *out_size) {
    if (!out_stream || !out_size) return -1;
    int written = sprintf(out_stream, "\x11\x40\x40\x1D\x08*** SYSTEM DEFCON ALERT: %d ***", defcon_level);
    if (written < 0) return -2;
    *out_size = (size_t)written;
    return 0;
}

int tsfi_mf_janap_classify_precedence(char precedence_char, int *priority_level, int *is_valid) {
    if (!priority_level || !is_valid) return -1;
    *is_valid = 0;
    
    switch (precedence_char) {
        case 'Z': // Flash
            *priority_level = 4;
            *is_valid = 1;
            break;
        case 'O': // Immediate
            *priority_level = 3;
            *is_valid = 1;
            break;
        case 'P': // Priority
            *priority_level = 2;
            *is_valid = 1;
            break;
        case 'R': // Routine
            *priority_level = 1;
            *is_valid = 1;
            break;
    }
    return 0;
}

int tsfi_mf_norad_is_irs_route(const char *routing_ind, int *is_irs, int *is_valid) {
    if (!routing_ind || !is_irs || !is_valid) return -1;
    *is_valid = 0;
    *is_irs = 0;
    
    size_t len = strlen(routing_ind);
    if (len < 3 || len > 8) return 0;
    
    *is_valid = 1;
    if (strcmp(routing_ind, "RUMIRS") == 0 || strcmp(routing_ind, "RUMTAX") == 0) {
        *is_irs = 1;
    }
    return 0;
}

int tsfi_mf_cics_format_irs_query(uint32_t tax_record_id, int routing_code, uint8_t *out_pdu, size_t *out_size) {
    if (!out_pdu || !out_size) return -1;
    out_pdu[0] = 0xFD; // IRS Query PDU marker
    out_pdu[1] = routing_code & 0xFF;
    out_pdu[2] = (tax_record_id >> 24) & 0xFF;
    out_pdu[3] = (tax_record_id >> 16) & 0xFF;
    out_pdu[4] = (tax_record_id >> 8) & 0xFF;
    out_pdu[5] = tax_record_id & 0xFF;
    *out_size = 6;
    return 0;
}

int tsfi_mf_cics_decode_irs_response(const uint8_t *in_pdu, size_t pdu_size, int *audit_status, int *is_valid) {
    if (!in_pdu || !audit_status || !is_valid) return -1;
    *is_valid = 0;
    *audit_status = -1;
    
    if (pdu_size < 3 || in_pdu[0] != 0xFE) return 0;
    
    *is_valid = 1;
    *audit_status = in_pdu[1] & 0xFF;
    return 0;
}

int tsfi_mf_nato_relay_verify_sequence(int current_seq, int expected_seq, int *is_in_sequence) {
    if (!is_in_sequence) return -1;
    *is_in_sequence = 0;
    
    if (current_seq == expected_seq) {
        *is_in_sequence = 1;
    }
    return 0;
}

int tsfi_mf_irs_merkle_combine(const uint8_t *left_hash, const uint8_t *right_hash, uint8_t *out_parent_hash) {
    if (!left_hash || !right_hash || !out_parent_hash) return -1;
    uint32_t l, r, p = 0;
    memcpy(&l, left_hash, 4);
    memcpy(&r, right_hash, 4);
    int res = tsfi_mf_nato_merkle_combine(l, r, &p);
    memcpy(out_parent_hash, &p, 4);
    return res;
}

int tsfi_mf_cics_check_irs_timeout(uint32_t send_time_ms, uint32_t current_time_ms, uint32_t timeout_limit_ms, int *is_timeout) {
    if (!is_timeout) return -1;
    *is_timeout = 0;
    if (current_time_ms - send_time_ms > timeout_limit_ms) {
        *is_timeout = 1;
    }
    return 0;
}

int tsfi_mf_norad_auth_hash(const uint8_t *payload, size_t size, uint32_t *out_hash) {
    if (!payload || !out_hash) return -1;
    
    uint32_t hash = 2166136261U;
    for (size_t i = 0; i < size; i++) {
        hash = (hash ^ payload[i]) * 16777619U;
    }
    *out_hash = hash;
    return 0;
}

int tsfi_mf_janap_validate_message(const char *msg_buffer, size_t size, int *is_valid) {
    if (!msg_buffer || !is_valid) return -1;
    (void)size;
    *is_valid = 0;
    
    // Find the end of the header line
    const char *newline = strchr(msg_buffer, '\n');
    if (!newline) return 0;
    
    // Validate header start "R "
    if (newline - msg_buffer < 10) return 0;
    if (msg_buffer[0] != 'R' || msg_buffer[1] != ' ') return 0;
    
    // Scan for EOM marker "NNNN"
    const char *eom = strstr(msg_buffer, "NNNN");
    if (!eom) return 0;
    if (eom < newline) return 0;
    
    *is_valid = 1;
    return 0;
}

int tsfi_mf_norad_link_irs_alarm(int audit_status, int *defcon_level, uint16_t *status_word) {
    if (!defcon_level || !status_word) return -1;
    if (audit_status == 2) {
        if (*defcon_level > 2) {
            *defcon_level = 2;
        }
        *status_word |= (1 << 10); // Trigger local alarm
    }
    return 0;
}

int tsfi_mf_irs_calculate_retry_backoff(int retry_count, int base_backoff_ms, int *next_backoff_ms) {
    if (!next_backoff_ms) return -1;
    int backoff = base_backoff_ms * (1 << retry_count);
    if (backoff > 30000) {
        backoff = 30000;
    }
    *next_backoff_ms = backoff;
    return 0;
}

int tsfi_mf_irs_format_clearance(uint32_t auth_token, uint8_t *out_pdu, size_t *out_size) {
    if (!out_pdu || !out_size) return -1;
    out_pdu[0] = 0xFA; // IRS Clearance PDU marker
    out_pdu[1] = (auth_token >> 24) & 0xFF;
    out_pdu[2] = (auth_token >> 16) & 0xFF;
    out_pdu[3] = (auth_token >> 8) & 0xFF;
    out_pdu[4] = auth_token & 0xFF;
    *out_size = 5;
    return 0;
}

int tsfi_mf_norad_validate_clearance(const uint8_t *in_pdu, size_t pdu_size, uint32_t expected_token, int *is_cleared) {
    if (!in_pdu || !is_cleared) return -1;
    *is_cleared = 0;
    
    if (pdu_size < 5 || in_pdu[0] != 0xFA) return 0;
    
    uint32_t auth_token = ((uint32_t)in_pdu[1] << 24) | 
                           ((uint32_t)in_pdu[2] << 16) | 
                           ((uint32_t)in_pdu[3] << 8) | 
                           in_pdu[4];
    
    if (auth_token == expected_token) {
        *is_cleared = 1;
    }
    return 0;
}

int tsfi_mf_norad_irs_relay_init(TSFiNoradIrsRelay *relay, uint32_t clearance_token) {
    if (!relay) return -1;
    relay->defcon_level = 5;
    relay->status_word = 0;
    relay->naap_state = 0; // IDLE
    relay->expected_clearance_token = clearance_token;
    relay->is_link_active = 1;
    return 0;
}

int tsfi_mf_norad_irs_relay_process_msg(TSFiNoradIrsRelay *relay, const char *msg_buffer, size_t size, uint8_t *out_pdu, size_t *out_size) {
    if (!relay || !msg_buffer || !out_pdu || !out_size) return -1;
    
    int is_valid = 0;
    int res = tsfi_mf_janap_validate_message(msg_buffer, size, &is_valid);
    if (res != 0 || !is_valid) return -2;
    
    // Extract Routing Indicator (assume it's the second field)
    char ri[16];
    if (sscanf(msg_buffer, "R %15s", ri) != 1) return -3;
    
    int is_irs = 0;
    int route_val = 0;
    tsfi_mf_norad_is_irs_route(ri, &is_irs, &route_val);
    if (!is_irs) return -4; // Not destined for IRS
    
    // Extract payload text
    char payload[128];
    size_t pay_size = 0;
    tsfi_mf_cics_extract_janap_payload(msg_buffer, sizeof(payload), payload, &pay_size);
    
    // Compute authentication hash
    uint32_t auth = 0;
    tsfi_mf_norad_auth_hash((const uint8_t *)payload, pay_size, &auth);
    
    // Format IRS Query PDU carrying hash as the tax record ID reference
    return tsfi_mf_cics_format_irs_query(auth, 1, out_pdu, out_size);
}

int tsfi_mf_norad_irs_relay_test_loopback(TSFiNoradIrsRelay *relay, const char *test_msg, uint8_t *out_pdu, size_t *out_pdu_size, int *is_success) {
    if (!relay || !test_msg || !out_pdu || !out_pdu_size || !is_success) return -1;
    *is_success = 0;
    
    // 1. Process query message
    int res = tsfi_mf_norad_irs_relay_process_msg(relay, test_msg, strlen(test_msg), out_pdu, out_pdu_size);
    if (res != 0) return 0;
    
    // 2. Simulate IRS Reject Audit response (audit status 2)
    uint8_t simulated_resp[] = {0xFE, 0x02, 0x00};
    int audit_status = -1;
    int resp_valid = 0;
    res = tsfi_mf_cics_decode_irs_response(simulated_resp, sizeof(simulated_resp), &audit_status, &resp_valid);
    if (res != 0 || !resp_valid) return 0;
    
    // 3. Link alert status to alarm registers
    res = tsfi_mf_norad_link_irs_alarm(audit_status, &relay->defcon_level, &relay->status_word);
    if (res != 0) return 0;
    
    // 4. Issue clearance certificate
    uint8_t cl_pdu[8];
    size_t cl_size = 0;
    res = tsfi_mf_irs_format_clearance(relay->expected_clearance_token, cl_pdu, &cl_size);
    if (res != 0) return 0;
    
    // 5. Validate clearance and unlock
    int is_cleared = 0;
    res = tsfi_mf_norad_validate_clearance(cl_pdu, cl_size, relay->expected_clearance_token, &is_cleared);
    if (res != 0 || !is_cleared) return 0;
    
    // Reset DEFCON back to normal operations
    relay->defcon_level = 5;
    relay->status_word &= ~(1 << 10);
    
    *is_success = 1;
    return 0;
}

int tsfi_mf_ssa_format_dmf_query(const char *ssn, uint8_t *out_pdu, size_t *out_size) {
    if (!ssn || !out_pdu || !out_size) return -1;
    size_t len = strlen(ssn);
    if (len != 9) return -2;
    
    out_pdu[0] = 0xFB; // SSA DMF Query Code
    for (int i = 0; i < 9; i++) {
        if (ssn[i] < '0' || ssn[i] > '9') return -3;
        out_pdu[i + 1] = ssn[i];
    }
    *out_size = 10;
    return 0;
}

int tsfi_mf_ssa_decode_dmf_response(const uint8_t *in_pdu, size_t pdu_size, int *is_deceased) {
    if (!in_pdu || !is_deceased) return -1;
    if (pdu_size < 2 || in_pdu[0] != 0xFC) return -2;
    
    *is_deceased = (in_pdu[1] == 1) ? 1 : 0;
    return 0;
}

int tsfi_mf_ssa_resolve_issuance_site(const char *ssn, char *site_name_out, int max_len) {
    if (!ssn || !site_name_out || max_len < 16) return -1;
    
    // Parse first 3 digits
    int area = 0;
    if (sscanf(ssn, "%3d", &area) != 1) return -2;
    
    if (area >= 1 && area <= 3) {
        strncpy(site_name_out, "New Hampshire", max_len - 1);
    } else if (area >= 4 && area <= 7) {
        strncpy(site_name_out, "Maine", max_len - 1);
    } else if (area >= 8 && area <= 9) {
        strncpy(site_name_out, "Vermont", max_len - 1);
    } else {
        strncpy(site_name_out, "Default Site", max_len - 1);
    }
    site_name_out[max_len - 1] = '\0';
    return 0;
}

int tsfi_mf_ssa_verify_checksum(const char *ssn, int *is_valid) {
    if (!ssn || !is_valid) return -1;
    size_t len = strlen(ssn);
    if (len != 9) {
        *is_valid = 0;
        return 0;
    }
    
    // Simple checksum: sum of all digits modulo 10 must equal 0
    int sum = 0;
    for (int i = 0; i < 9; i++) {
        if (ssn[i] < '0' || ssn[i] > '9') {
            *is_valid = 0;
            return 0;
        }
        sum += (ssn[i] - '0');
    }
    *is_valid = ((sum % 10) == 0) ? 1 : 0;
    return 0;
}

































