#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "../tsfi2-deepseek/inc/auncient_motzkin_engine.h"

int main(void) {
    printf("================================================================================\n");
    printf(" PROOF: CH 9 SEC 1 LOGARITHMIC TERMS POWER SERIES ENGINE\n");
    printf("================================================================================\n\n");

    const char *contract_addr = "dynamic_0x71C7656EC7ab88b098defB751B7401B5f6d8976F";
    const char *dat_bin_ch9_path = "/tmp/euler_vol1_ch9_sec1_log_proof.dat.bin";

    FILE *f = fopen(dat_bin_ch9_path, "wb");
    assert(f != NULL);
    uint8_t zero_buf[512] = {0};
    fwrite(zero_buf, 1, sizeof(zero_buf), f);
    fclose(f);

    int64_t preserved_x = 5;
    int64_t preserved_y = 11;
    uint64_t input_x = 200000; // x = 0.2 scaled by 1,000,000
    uint32_t terms = 10;

    AuncientEulerVolume1Chapter9Section1LogSeriesMetrics sec1_metrics = {0};

    bool ok = auncient_euler_volume1_chapter9_sec1_log_series_engine(
        contract_addr,
        dat_bin_ch9_path,
        input_x,
        terms,
        preserved_x,
        preserved_y,
        &sec1_metrics
    );

    assert(ok == true);
    assert(sec1_metrics.ch9_sec1_log_series_sound == true);
    assert(sec1_metrics.is_log_series_sound == true);
    assert(sec1_metrics.evaluated_log_series_scaled == 182322ULL); // \ln(1.2) \approx 0.18232155...

    printf("[SECTION 1 LOGARITHMIC TERMS POWER SERIES METRICS (§ 286-§ 295)]\n");
    printf("  - Latin Title:                 %s\n", sec1_metrics.section_latin_title);
    printf("  - Preserved Random Pair (x,y): (%ld, %ld)\n",
           sec1_metrics.preserved_random_x, sec1_metrics.preserved_random_y);
    printf("  - Input Scalar x:              x = 0.2 (Scaled x = %lu)\n", sec1_metrics.input_x_scaled);
    printf("  - Evaluated \\ln(1+0.2):        \\ln(1.2) = %lu (0.182322... scaled by 1,000,000)\n",
           sec1_metrics.evaluated_log_series_scaled);
    printf("  - Series Terms Computed:       %u Terms (§ 286 Logarithmic Power Series)\n",
           sec1_metrics.series_terms_computed);
    printf("  - Logarithmic Soundness:       %s\n",
           sec1_metrics.is_log_series_sound ? "100% VERIFIED SOUND" : "FAILED");
    printf("  - Logarithmic WAL Checksum:    0x%lX\n", sec1_metrics.ch9_sec1_wal_checksum);
    printf("  - Rule 9 Dynamic Address:      %s (dynamic_<address>)\n",
           sec1_metrics.rule9_address_resolution_sound ? "SOUND" : "FAILED");
    printf("  - Rule 13 .dat.bin Layout:     %s (.dat.bin Quadtree)\n",
           sec1_metrics.rule13_dat_bin_verified ? "VERIFIED" : "FAILED");
    printf("  - Master FNV-1a Checksum:      0x%lX\n", sec1_metrics.acid_ch9_sec1_checksum);
    printf("  - 512-bit ZMM ReBAR Latch:     0x%lX\n\n", sec1_metrics.zmm_hardware_latch);

    printf("================================================================================\n");
    printf(" [PROOFS SUCCESSFUL] Section 1 Logarithmic Terms Power Series Engine sound.\n");
    printf("================================================================================\n");

    remove(dat_bin_ch9_path);
    return 0;
}
