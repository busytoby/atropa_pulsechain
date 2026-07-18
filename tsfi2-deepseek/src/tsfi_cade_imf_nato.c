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

int tsfi_mf_nato_verify_slot_boundary_source_offset_range_limit(int limit_ms, int *is_valid) {
    if (!is_valid) return -1;
    *is_valid = (limit_ms >= 0 && limit_ms <= 30) ? 1 : 0;
    return 0;
}

int tsfi_mf_nato_verify_rssi_quiet_sample_threshold_limit(int limit_samples, int *is_valid) {
    if (!is_valid) return -1;
    *is_valid = (limit_samples >= 2 && limit_samples <= 12) ? 1 : 0;
    return 0;
}

int tsfi_mf_nato_verify_slot_boundary_source_offset_range_limit_boundary(int boundary_ms, int *is_valid) {
    if (!is_valid) return -1;
    *is_valid = (boundary_ms >= 0 && boundary_ms <= 40) ? 1 : 0;
    return 0;
}

int tsfi_mf_nato_verify_rssi_quiet_sample_threshold_limit_margin(int margin_samples, int *is_valid) {
    if (!is_valid) return -1;
    *is_valid = (margin_samples >= 1 && margin_samples <= 6) ? 1 : 0;
    return 0;
}

int tsfi_mf_nato_verify_slot_boundary_source_offset_range_limit_val(int val_ms, int *is_valid) {
    if (!is_valid) return -1;
    *is_valid = (val_ms >= 0 && val_ms <= 50) ? 1 : 0;
    return 0;
}

int tsfi_mf_nato_verify_rssi_quiet_sample_threshold_limit_margin_range(int range_samples, int *is_valid) {
    if (!is_valid) return -1;
    *is_valid = (range_samples >= 1 && range_samples <= 8) ? 1 : 0;
    return 0;
}

int tsfi_mf_nato_verify_slot_boundary_source_offset_range_limit_val_boundary(int boundary_ms, int *is_valid) {
    if (!is_valid) return -1;
    *is_valid = (boundary_ms >= 0 && boundary_ms <= 60) ? 1 : 0;
    return 0;
}

int tsfi_mf_nato_verify_rssi_quiet_sample_threshold_limit_margin_range_option(int option, int *is_valid) {
    if (!is_valid) return -1;
    *is_valid = (option >= 0 && option <= 4) ? 1 : 0;
    return 0;
}

int tsfi_mf_nato_verify_slot_boundary_source_offset_range_limit_val_margin(int margin_ms, int *is_valid) {
    if (!is_valid) return -1;
    *is_valid = (margin_ms >= 0 && margin_ms <= 70) ? 1 : 0;
    return 0;
}

int tsfi_mf_nato_verify_rssi_quiet_sample_threshold_limit_margin_range_val(int val_samples, int *is_valid) {
    if (!is_valid) return -1;
    *is_valid = (val_samples >= 1 && val_samples <= 10) ? 1 : 0;
    return 0;
}

int tsfi_mf_nato_verify_slot_boundary_source_offset_range_limit_val_margin_range(int range_ms, int *is_valid) {
    if (!is_valid) return -1;
    *is_valid = (range_ms >= 0 && range_ms <= 80) ? 1 : 0;
    return 0;
}

int tsfi_mf_nato_verify_rssi_quiet_sample_threshold_limit_margin_range_option_selector(int selector, int *is_valid) {
    if (!is_valid) return -1;
    *is_valid = (selector >= 0 && selector <= 3) ? 1 : 0;
    return 0;
}

int tsfi_mf_nato_verify_slot_boundary_source_offset_range_limit_val_margin_range_option_selector_option(int option, int *is_valid) {
    if (!is_valid) return -1;
    *is_valid = (option >= 0 && option <= 2) ? 1 : 0;
    return 0;
}

int tsfi_mf_nato_verify_rssi_quiet_sample_threshold_limit_margin_range_option_selector_margin(int margin_samples, int *is_valid) {
    if (!is_valid) return -1;
    *is_valid = (margin_samples >= 1 && margin_samples <= 5) ? 1 : 0;
    return 0;
}

int tsfi_mf_nato_verify_slot_boundary_source_offset_range_limit_val_margin_range_option_selector_option_margin(int margin_ms, int *is_valid) {
    if (!is_valid) return -1;
    *is_valid = (margin_ms >= 0 && margin_ms <= 90) ? 1 : 0;
    return 0;
}

int tsfi_mf_nato_verify_rssi_quiet_sample_threshold_limit_margin_range_option_selector_margin_range(int range_samples, int *is_valid) {
    if (!is_valid) return -1;
    *is_valid = (range_samples >= 1 && range_samples <= 7) ? 1 : 0;
    return 0;
}
