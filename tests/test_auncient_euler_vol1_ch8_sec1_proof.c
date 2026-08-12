#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "../tsfi2-deepseek/inc/auncient_motzkin_engine.h"

int main(void) {
    printf("================================================================================\n");
    printf(" PROOF: EULER VOL 1 CH 8 SEC 1 SINE & COSINE POWER SERIES ENGINE\n");
    printf("================================================================================\n\n");

    const char *contract_addr = "dynamic_0x71C7656EC7ab88b098defB751B7401B5f6d8976F";
    const char *dat_bin_ch8_path = "/tmp/euler_vol1_ch8_sec1_proof.dat.bin";

    FILE *f = fopen(dat_bin_ch8_path, "wb");
    assert(f != NULL);
    uint8_t zero_buf[512] = {0};
    fwrite(zero_buf, 1, sizeof(zero_buf), f);
    fclose(f);

    int64_t preserved_x = 5;
    int64_t preserved_y = 11;
    uint64_t arc_z = 100000; // z = 0.1 rad scaled by 1,000,000
    uint32_t terms = 5;

    AuncientEulerVolume1Chapter8Section1SineCosineSeriesMetrics sec1_metrics = {0};

    bool ok = auncient_euler_volume1_chapter8_sec1_sine_cosine_series_engine(
        contract_addr,
        dat_bin_ch8_path,
        arc_z,
        terms,
        preserved_x,
        preserved_y,
        &sec1_metrics
    );

    assert(ok == true);
    assert(sec1_metrics.ch8_sec1_trig_engine_sound == true);
    assert(sec1_metrics.is_pythagorean_identity_sound == true);

    printf("[SECTION 1 SINE & COSINE POWER SERIES METRICS (§ 259-§ 262)]\n");
    printf("  - Latin Title:                 %s\n", sec1_metrics.section_latin_title);
    printf("  - Preserved Random Pair (x,y): (%ld, %ld)\n",
           sec1_metrics.preserved_random_x, sec1_metrics.preserved_random_y);
    printf("  - Input Arc z:                 z = 0.1 rad (Scaled z = %lu)\n", sec1_metrics.input_arc_z_scaled);
    printf("  - Evaluated \\sin(z):           \\sin(0.1) = %lu (0.099833...)\n", sec1_metrics.evaluated_sin_z_scaled);
    printf("  - Evaluated \\cos(z):           \\cos(0.1) = %lu (0.995004...)\n", sec1_metrics.evaluated_cos_z_scaled);
    printf("  - Pythagorean Identity:        \\sin^2(z) + \\cos^2(z) = 1 (Scaled = %lu)\n",
           sec1_metrics.pythagorean_identity_scaled);
    printf("  - Series Terms Computed:       %u Terms (§ 262 Infinite Power Series)\n",
           sec1_metrics.series_terms_computed);
    printf("  - Trig Series WAL Checksum:    0x%lX\n", sec1_metrics.sec1_trig_wal_checksum);
    printf("  - Rule 9 Dynamic Address:      %s (dynamic_<address>)\n",
           sec1_metrics.rule9_address_resolution_sound ? "SOUND" : "FAILED");
    printf("  - Rule 13 .dat.bin Layout:     %s (.dat.bin Quadtree)\n",
           sec1_metrics.rule13_dat_bin_verified ? "VERIFIED" : "FAILED");
    printf("  - Master FNV-1a Checksum:      0x%lX\n", sec1_metrics.acid_sec1_trig_checksum);
    printf("  - 512-bit ZMM ReBAR Latch:     0x%lX\n\n", sec1_metrics.zmm_hardware_latch);

    printf("================================================================================\n");
    printf(" [PROOFS SUCCESSFUL] Section 1 Sine & Cosine Power Series Engine sound.\n");
    printf("================================================================================\n");

    remove(dat_bin_ch8_path);
    return 0;
}
