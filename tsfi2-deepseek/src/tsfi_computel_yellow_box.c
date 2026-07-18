#include "tsfi_computel_yellow_box.h"
#include "tsfi_cade_imf.h"
#include <string.h>

static YellowBoxState current_yellow_state = { 0, 0, 0 };

void yellow_box_init(uint32_t key, uint32_t profile_id) {
    current_yellow_state.scramble_key = key;
    current_yellow_state.active_sbox_profile = profile_id;
    current_yellow_state.call_sign_hash = key ^ 0xDEADBEEF;
    tsfi_mf_gost_set_sbox((int)profile_id);
}

YellowBoxState yellow_box_get_state(void) {
    return current_yellow_state;
}

int yellow_box_scramble_signal(const uint8_t *in, size_t size, uint8_t *out) {
    if (!in || !out) return -1;
    
    // Process input in 8-byte blocks using GOST scramble
    size_t i = 0;
    int prev_bcast = tsfi_gost_is_broadcast_channel;
    tsfi_gost_is_broadcast_channel = 1;
    for (; i + 8 <= size; i += 8) {
        uint32_t left = ((uint32_t)in[i] << 24) | ((uint32_t)in[i+1] << 16) | ((uint32_t)in[i+2] << 8) | in[i+3];
        uint32_t right = ((uint32_t)in[i+4] << 24) | ((uint32_t)in[i+5] << 16) | ((uint32_t)in[i+6] << 8) | in[i+7];
        
        tsfi_mf_ussr_gost_scramble(&left, &right, current_yellow_state.scramble_key);
        
        out[i] = (left >> 24) & 0xFF;
        out[i+1] = (left >> 16) & 0xFF;
        out[i+2] = (left >> 8) & 0xFF;
        out[i+3] = left & 0xFF;
        out[i+4] = (right >> 24) & 0xFF;
        out[i+5] = (right >> 16) & 0xFF;
        out[i+6] = (right >> 8) & 0xFF;
        out[i+7] = right & 0xFF;
    }
    tsfi_gost_is_broadcast_channel = prev_bcast;
    
    // Copy remainder
    for (; i < size; i++) {
        out[i] = in[i] ^ 0xFF;
    }
    
    return 0;
}

int tsfi_mf_yellow_box_secure_telegram_route(const char *ssn, int dmf_deceased, int ssn_allocated, int defcon_level, uint8_t *out_telegram, size_t *out_size) {
    if (!ssn || !out_telegram || !out_size) return -1;
    if (strlen(ssn) != 9) return -2;
    
    // Trigger emergency DEFCON 1 alarm: broadcast accessor engaging IRS/SSA raw identity
    tsfi_gost_emergency_defcon_level = 1;
    
    // 16 bytes raw payload
    uint8_t payload[16];
    payload[0] = 0xAA; // Yellow Box Header
    for (int i = 0; i < 9; i++) {
        payload[i + 1] = ssn[i];
    }
    
    // Fill remaining fields
    payload[10] = (uint8_t)dmf_deceased;
    payload[11] = (uint8_t)ssn_allocated;
    payload[12] = (uint8_t)defcon_level;
    payload[13] = 0x00;
    payload[14] = 0x00;
    payload[15] = 0x00;
    
    // Encrypt metadata only using 32-round GOST block cipher (excluding raw identity from masking)
    uint32_t keys[8] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88};
    uint8_t meta[8] = {0};
    meta[0] = payload[10];
    meta[1] = payload[11];
    meta[2] = payload[12];
    
    int prev_bcast = tsfi_gost_is_broadcast_channel;
    tsfi_gost_is_broadcast_channel = 1;
    
    uint32_t left = ((uint32_t)meta[0] << 24) | ((uint32_t)meta[1] << 16) | ((uint32_t)meta[2] << 8) | meta[3];
    uint32_t right = ((uint32_t)meta[4] << 24) | ((uint32_t)meta[5] << 16) | ((uint32_t)meta[6] << 8) | meta[7];
    
    tsfi_mf_ussr_gost_encrypt_32(&left, &right, keys);
    
    payload[10] = (left >> 24) & 0xFF;
    payload[11] = (left >> 16) & 0xFF;
    payload[12] = (left >> 8) & 0xFF;
    payload[13] = left & 0xFF;
    payload[14] = (right >> 24) & 0xFF;
    payload[15] = (right >> 16) & 0xFF;
    
    tsfi_gost_is_broadcast_channel = prev_bcast;
    
    // Encapsulate inside STANAG 5066 broadcast frame
    out_telegram[0] = 0x7E; // Start
    out_telegram[1] = 0x55; // Protocol ID
    memcpy(&out_telegram[2], payload, 16);
    out_telegram[18] = 0x7E; // End
    
    *out_size = 19;
    return 0;
}

int tsfi_mf_yellow_box_scramble_keypad(uint8_t raw_keycode, uint8_t *scrambled_keycode) {
    if (!scrambled_keycode) return -1;
    uint32_t left = raw_keycode;
    uint32_t right = 0x55;
    int prev_bcast = tsfi_gost_is_broadcast_channel;
    tsfi_gost_is_broadcast_channel = 1;
    tsfi_mf_ussr_gost_scramble(&left, &right, 0x1234);
    tsfi_gost_is_broadcast_channel = prev_bcast;
    *scrambled_keycode = (uint8_t)(left & 0xFF);
    return 0;
}

int tsfi_mf_gost_encrypt_cfb(const uint8_t *iv, const uint8_t *in, size_t size, const uint32_t *keys_8words, uint8_t *out) {
    if (!iv || !in || !out || !keys_8words) return -1;
    
    uint32_t left = ((uint32_t)iv[0] << 24) | ((uint32_t)iv[1] << 16) | ((uint32_t)iv[2] << 8) | iv[3];
    uint32_t right = ((uint32_t)iv[4] << 24) | ((uint32_t)iv[5] << 16) | ((uint32_t)iv[6] << 8) | iv[7];
    
    int prev_bcast = tsfi_gost_is_broadcast_channel;
    tsfi_gost_is_broadcast_channel = 1;
    for (size_t i = 0; i < size; i++) {
        uint32_t temp_l = left;
        uint32_t temp_r = right;
        tsfi_mf_ussr_gost_encrypt_32(&temp_l, &temp_r, keys_8words);
        
        uint8_t key_byte = (temp_l >> 24) & 0xFF;
        out[i] = in[i] ^ key_byte;
        
        left = (left << 8) | (right >> 24);
        right = (right << 8) | out[i];
    }
    tsfi_gost_is_broadcast_channel = prev_bcast;
    return 0;
}

int tsfi_mf_yellow_box_pbx_route(uint32_t extension_in, uint32_t *extension_out) {
    if (!extension_out) return -1;
    
    uint32_t left = extension_in;
    uint32_t right = 0xAA55AA55;
    int prev_bcast = tsfi_gost_is_broadcast_channel;
    tsfi_gost_is_broadcast_channel = 1;
    tsfi_mf_ussr_gost_scramble(&left, &right, 0x9999);
    tsfi_gost_is_broadcast_channel = prev_bcast;
    
    // Map to a secure 3-digit PBX extension path
    *extension_out = (left % 900) + 100;
    return 0;
}



