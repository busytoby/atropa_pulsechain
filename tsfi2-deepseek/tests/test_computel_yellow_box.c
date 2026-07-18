#include "tsfi_computel_yellow_box.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>

int main(void) {
    printf("[TEST] Initializing Auncient Computel Yellow Box S-Box scrambler tests...\n");

    // Initialize Yellow Box with KGB profile
    yellow_box_init(0x12345678, 0);
    YellowBoxState state = yellow_box_get_state();
    assert(state.scramble_key == 0x12345678);
    assert(state.active_sbox_profile == 0);
    printf("[TEST] Yellow Box successfully initialized with KGB profile.\n");

    // Scramble message block
    uint8_t input[] = "SecretSignalPayload00";
    uint8_t scrambled[32] = {0};
    uint8_t descrambled[32] = {0};
    
    int rc = yellow_box_scramble_signal(input, sizeof(input), scrambled);
    assert(rc == 0);
    assert(memcmp(input, scrambled, sizeof(input)) != 0);
    printf("[TEST] Signal successfully scrambled (mismatch verified).\n");

    // Re-initialize for descrambling (GOST is symmetric under round key ordering adjustments)
    // For one round of scramble, running scramble again acts as descramble since left/right swap back
    rc = yellow_box_scramble_signal(scrambled, sizeof(input), descrambled);
    assert(rc == 0);
    printf("[TEST] Descrambling pass executed.\n");

    // Test Secure Telegram Routing (integrating Yellow Box, S-Box, IRS, NATO, NORAD, and SSA status)
    printf("[TEST] Validating Yellow Box Secure Telegram Routing...\n");
    uint8_t telegram[32] = {0};
    size_t tel_size = 0;
    rc = tsfi_mf_yellow_box_secure_telegram_route("050051122", 1, 0, 1, telegram, &tel_size);
    assert(rc == 0);
    assert(tel_size == 19);
    assert(telegram[0] == 0x7E); // STANAG 5066 framing check
    assert(telegram[1] == 0x55);
    assert(telegram[18] == 0x7E);
    printf("[TEST] Yellow Box Secure Telegram successfully routed and framed (19 bytes).\n");

    // Test Keypad Scrambler
    printf("[TEST] Validating Keypad Scrambler...\n");
    uint8_t scrambled_code = 0;
    rc = tsfi_mf_yellow_box_scramble_keypad(0x41, &scrambled_code);
    assert(rc == 0);
    assert(scrambled_code != 0x41);
    printf("[TEST] Keypad code successfully scrambled.\n");

    // Test GOST CFB stream encryption
    printf("[TEST] Validating GOST CFB encryption...\n");
    uint8_t iv[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    uint8_t stream_in[] = "ContinuousTelemetryStream";
    uint8_t stream_out[32] = {0};
    uint32_t cfb_keys[8] = {9, 10, 11, 12, 13, 14, 15, 16};
    rc = tsfi_mf_gost_encrypt_cfb(iv, stream_in, sizeof(stream_in), cfb_keys, stream_out);
    assert(rc == 0);
    assert(stream_out[0] != stream_in[0]);
    printf("[TEST] GOST CFB stream encryption completed successfully.\n");

    // Test Yellow Box PBX Switchboard routing
    printf("[TEST] Validating Yellow Box PBX Switchboard routing...\n");
    uint32_t ext_out = 0;
    rc = tsfi_mf_yellow_box_pbx_route(500, &ext_out);
    assert(rc == 0);
    assert(ext_out >= 100 && ext_out <= 999);
    printf("[TEST] Yellow Box PBX Switchboard routed extension 500 to secure extension %d.\n", ext_out);

    printf("[SUCCESS] All Yellow Box S-Box scrambler integration tests passed.\n");
    return 0;
}
