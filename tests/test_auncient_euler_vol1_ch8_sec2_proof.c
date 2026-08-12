#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "../tsfi2-deepseek/inc/auncient_motzkin_engine.h"

int main(void) {
    printf("================================================================================\n");
    printf(" PROOF: EULER VOL 1 CH 8 SEC 2 SINE INFINITE PRODUCT FORMULA ENGINE\n");
    printf("================================================================================\n\n");

    const char *contract_addr = "dynamic_0x71C7656EC7ab88b098defB751B7401B5f6d8976F";
    const char *dat_bin_ch8_path = "/tmp/euler_vol1_ch8_sec2_product_proof.dat.bin";

    FILE *f = fopen(dat_bin_ch8_path, "wb");
    assert(f != NULL);
    uint8_t zero_buf[512] = {0};
    fwrite(zero_buf, 1, sizeof(zero_buf), f);
    fclose(f);

    int64_t preserved_x = 5;
    int64_t preserved_y = 11;
    uint64_t arc_z = 100000; // z = 0.1 rad scaled by 1,000,000
    uint32_t product_factors = 10; // k = 1 to 10 factors

    AuncientEulerVolume1Chapter8Section2SineInfiniteProductMetrics sec2_metrics = {0};

    bool ok = auncient_euler_volume1_chapter8_sec2_sine_infinite_product_engine(
        contract_addr,
        dat_bin_ch8_path,
        arc_z,
        product_factors,
        preserved_x,
        preserved_y,
        &sec2_metrics
    );

    assert(ok == true);
    assert(sec2_metrics.ch8_sec2_product_engine_sound == true);
    assert(sec2_metrics.is_product_convergence_sound == true);

    printf("[SECTION 2 SINE INFINITE PRODUCT FORMULA METRICS (§ 265-§ 284)]\n");
    printf("  - Latin Title:                 %s\n", sec2_metrics.section_latin_title);
    printf("  - Preserved Random Pair (x,y): (%ld, %ld)\n",
           sec2_metrics.preserved_random_x, sec2_metrics.preserved_random_y);
    printf("  - Input Arc z:                 z = 0.1 rad (Scaled z = %lu)\n", sec2_metrics.input_arc_z_scaled);
    printf("  - Evaluated \\sin(z) Product:   \\sin(0.1) = %lu (0.099833...)\n", sec2_metrics.evaluated_sin_product_scaled);
    printf("  - Product Factors Computed:    %u Factors (§ 265 Infinite Product Formula)\n",
           sec2_metrics.product_factors_computed);
    printf("  - Product Convergence:        %s\n",
           sec2_metrics.is_product_convergence_sound ? "100% VERIFIED SOUND" : "FAILED");
    printf("  - Infinite Product WAL:        0x%lX\n", sec2_metrics.sec2_product_wal_checksum);
    printf("  - Rule 9 Dynamic Address:      %s (dynamic_<address>)\n",
           sec2_metrics.rule9_address_resolution_sound ? "SOUND" : "FAILED");
    printf("  - Rule 13 .dat.bin Layout:     %s (.dat.bin Quadtree)\n",
           sec2_metrics.rule13_dat_bin_verified ? "VERIFIED" : "FAILED");
    printf("  - Master FNV-1a Checksum:      0x%lX\n", sec2_metrics.acid_sec2_product_checksum);
    printf("  - 512-bit ZMM ReBAR Latch:     0x%lX\n\n", sec2_metrics.zmm_hardware_latch);

    printf("================================================================================\n");
    printf(" [PROOFS SUCCESSFUL] Section 2 Sine Infinite Product Formula Engine sound.\n");
    printf("================================================================================\n");

    remove(dat_bin_ch8_path);
    return 0;
}
