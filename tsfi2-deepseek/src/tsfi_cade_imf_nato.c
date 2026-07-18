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





