#include "tsfi_nsa_tempest.h"
#include <math.h>
#include <string.h>

void tsfi_tempest_init(tsfi_tempest_chamber *chamber, double target_attenuation_db) {
    if (!chamber) return;
    chamber->attenuation_db = target_attenuation_db;
    
    // Logarithmic attenuation calculation:
    // P_out = P_in * 10^(-dB/10)
    // boundary_leakage_mv represents voltage, so using -dB/20:
    chamber->boundary_leakage_mv = 5.0 * pow(10.0, -target_attenuation_db / 20.0);
    
    if (target_attenuation_db >= 80.0) {
        chamber->shield_status = TEMPEST_SHIELD_EXCELLENT;
    } else if (target_attenuation_db >= 40.0) {
        chamber->shield_status = TEMPEST_SHIELD_DEGRADED;
    } else {
        chamber->shield_status = TEMPEST_SHIELD_COMPROMISED;
    }
    
    chamber->red_zone_active = 1;
    chamber->black_zone_active = 1;
}

int tsfi_tempest_verify_shielding(tsfi_tempest_chamber *chamber) {
    if (!chamber) return -1;
    
    // Re-verify leakage voltage threshold
    chamber->boundary_leakage_mv = 5.0 * pow(10.0, -chamber->attenuation_db / 20.0);
    
    if (chamber->attenuation_db >= 80.0) {
        chamber->shield_status = TEMPEST_SHIELD_EXCELLENT;
        return 0; // Fully compliant
    } else if (chamber->attenuation_db >= 40.0) {
        chamber->shield_status = TEMPEST_SHIELD_DEGRADED;
        return 1; // Degraded warning
    } else {
        chamber->shield_status = TEMPEST_SHIELD_COMPROMISED;
        return 2; // Compromised error
    }
}

int tsfi_tempest_route_transaction(tsfi_tempest_chamber *chamber, const uint8_t *red_payload, size_t red_len, uint8_t *black_payload_out, size_t *black_len_out) {
    if (!chamber || !red_payload || !black_payload_out || !black_len_out) return -1;
    
    // Block routing if shield is completely compromised to prevent signal leaks
    if (chamber->shield_status == TEMPEST_SHIELD_COMPROMISED) {
        return -2; // RF compromise lockout
    }
    
    // Perform Auncient translation: scramble/obfuscate Red processing payload into Black storage data
    for (size_t i = 0; i < red_len; i++) {
        // Shift bits based on attenuation factors to simulate high frequency filters
        black_payload_out[i] = red_payload[i] ^ 0xAA;
    }
    *black_len_out = red_len;
    return 0;
}

int tsfi_tempest_route_storage(tsfi_tempest_chamber *chamber, const uint8_t *black_payload, size_t black_len, uint8_t *red_payload_out, size_t *red_len_out) {
    if (!chamber || !black_payload || !red_payload_out || !red_len_out) return -1;
    
    if (chamber->shield_status == TEMPEST_SHIELD_COMPROMISED) {
        return -2; // RF compromise lockout
    }
    
    // Restore Red processing instructions from Black database blocks
    for (size_t i = 0; i < black_len; i++) {
        red_payload_out[i] = black_payload[i] ^ 0xAA;
    }
    *red_len_out = black_len;
    return 0;
}

int tsfi_tempest_scsi_bridge_handshake(tsfi_tempest_chamber *chamber, tsfi_scsi_loopback_bridge *bridge) {
    if (!chamber || !bridge) return -1;
    
    // 1. Lock out if TEMPEST shielding is compromised
    if (chamber->shield_status == TEMPEST_SHIELD_COMPROMISED) {
        return -2; // RF compromise lockout
    }
    
    // 2. Lock out if synthetic automation signatures are detected
    if (bridge->synthetic_flag) {
        return -3; // Automated/synthetic driver block
    }
    
    // 3. Verify keycode matches the Auncient virtual hardware mapping:
    // Keycode 30 (a/A) or 32 (d/D)
    if (bridge->keycode_reg != 30 && bridge->keycode_reg != 32) {
        return -4; // Invalid keycode register mapping
    }
    
    // 4. Drive the SCSI status handshake loop
    switch (bridge->status_reg) {
        case 0: // Idle
            bridge->status_reg = 1; // REQ
            break;
        case 1: // REQ
            bridge->status_reg = 2; // ACK
            break;
        case 2: // ACK
            bridge->status_reg = 3; // DATA_IN
            break;
        case 3: // DATA_IN
            bridge->data_reg = bridge->keycode_reg;
            bridge->status_reg = 4; // DATA_OUT
            break;
        case 4: // DATA_OUT
            bridge->loopback_cnt++;
            bridge->status_reg = 0; // Reset to Idle
            break;
        default:
            bridge->status_reg = 0;
            return -5; // Invalid state register
    }
    
    return 0;
}
