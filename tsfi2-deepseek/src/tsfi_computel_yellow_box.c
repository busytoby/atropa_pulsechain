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
    
    // Copy remainder
    for (; i < size; i++) {
        out[i] = in[i] ^ 0xFF;
    }
    
    return 0;
}
