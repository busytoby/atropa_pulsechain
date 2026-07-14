#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include <math.h>
#include <string.h>
#include "../src/tsfi_computel_blue_box.c"

int main(void) {
    printf("[TEST] Initializing Auncient Computel Blue Box SF/MF test suite...\n");

    float buffer[200];

    // 1. Test SF Seizure (2600 Hz)
    generate_sf_seizure(buffer, 200);
    // Spot check sample (index 99) -> should be non-zero
    assert(buffer[99] != 0.0f);
    printf("[TEST] SF 2600 Hz seizure tone verified.\n");

    // 2. Test MF Keypulse 'K' (1100 + 1700 Hz)
    bool ok = generate_mf_tone('K', buffer, 200);
    assert(ok == true);
    assert(buffer[99] != 0.0f);

    // 3. Test MF Start 'S' (1500 + 1700 Hz)
    ok = generate_mf_tone('S', buffer, 200);
    assert(ok == true);
    assert(buffer[99] != 0.0f);

    // 4. Test invalid digit
    ok = generate_mf_tone('Z', buffer, 200);
    assert(ok == false);

    // 5. Test Red Box Payphone Coin Tones
    float coin_buf[2000];
    // Nickel (5 cents) -> 1 beep = 2 * 66 ms = 132 ms * 8000 = 1056 samples
    int samples = generate_red_box_coin_tone(5, coin_buf, 2000);
    assert(samples == 1056);

    // Dime (10 cents) -> 2 beeps = 4 * 66 ms = 264 ms * 8000 = 2112 samples
    float large_coin_buf[3000];
    samples = generate_red_box_coin_tone(10, large_coin_buf, 3000);
    assert(samples == 2112);

    // 6. Test Immutable ZMM Address Storage contained within Blue Box
    const char *addr800 = blue_box_get_immutable_address(800);
    assert(addr800 != NULL);
    assert(strcmp(addr800, "dynamic_0x0000000000000000000000000000000000000800") == 0);

    const char *addr805 = blue_box_get_immutable_address(805);
    assert(addr805 != NULL);
    assert(strcmp(addr805, "dynamic_0x0000000000000000000000000000000000000805") == 0);

    const char *addr_invalid = blue_box_get_immutable_address(999);
    assert(addr_invalid == NULL);

    printf("[SUCCESS] All Computel Blue Box SF/MF, Red Box coin, and immutable address storage tests passed successfully.\n");
    return 0;
}
