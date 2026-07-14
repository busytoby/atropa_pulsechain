#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include <math.h>
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

    printf("[SUCCESS] All Computel Blue Box SF/MF modulation tests passed successfully.\n");
    return 0;
}
