#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "../tsfi2-deepseek/inc/auncient_motzkin_engine.h"

int main(void) {
    printf("================================================================================\n");
    printf(" PROOF: EULER VOL 1 CH 8 SEC 2 SINE EXPLICIT ITERATION PRODUCT ENGINE\n");
    printf("================================================================================\n\n");

    const char *contract_addr = "dynamic_0x71C7656EC7ab88b098defB751B7401B5f6d8976F";
    const char *dat_bin_ch8_path = "/tmp/euler_vol1_ch8_sec2_explicit_iter_proof.dat.bin";

    FILE *f = fopen(dat_bin_ch8_path, "wb");
    assert(f != NULL);
    uint8_t zero_buf[512] = {0};
    fwrite(zero_buf, 1, sizeof(zero_buf), f);
    fclose(f);

    int64_t preserved_x = 5;
    int64_t preserved_y = 11;
    uint64_t arc_z = 100000; // z = 0.1 rad scaled by 1,000,000

    /* Test convergence across multiple iteration counts: k = 1, k = 5, k = 10 factors */
    uint32_t iterations_to_test[3] = {1, 5, 10};

    for (int i = 0; i < 3; i++) {
        uint32_t factors_cnt = iterations_to_test[i];
        AuncientEulerVolume1Chapter8Section2SineInfiniteProductMetrics metrics = {0};

        bool ok = auncient_euler_volume1_chapter8_sec2_sine_infinite_product_engine(
            contract_addr,
            dat_bin_ch8_path,
            arc_z,
            factors_cnt,
            preserved_x,
            preserved_y,
            &metrics
        );

        assert(ok == true);
        assert(metrics.ch8_sec2_product_engine_sound == true);

        printf("[ITERATION STEP %d: k = 1 to %u FACTORS]\n", i + 1, factors_cnt);
        printf("  - Evaluated \\sin(0.1) Iterated Product: %lu (Scaled by 1,000,000)\n",
               metrics.evaluated_sin_product_scaled);
        printf("  - Factors Iterated:                    %u Factors\n", metrics.product_factors_computed);
        printf("  - Master FNV-1a Checksum:              0x%lX\n\n", metrics.acid_sec2_product_checksum);
    }

    printf("================================================================================\n");
    printf(" [PROOFS SUCCESSFUL] Factor-by-factor iteration of sine product formula sound.\n");
    printf("================================================================================\n");

    remove(dat_bin_ch8_path);
    return 0;
}
