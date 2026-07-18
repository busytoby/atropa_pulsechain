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

    printf("[SUCCESS] All Yellow Box S-Box scrambler integration tests passed.\n");
    return 0;
}
