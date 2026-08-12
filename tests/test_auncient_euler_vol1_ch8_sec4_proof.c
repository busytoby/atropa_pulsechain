#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "../tsfi2-deepseek/inc/auncient_motzkin_engine.h"

int main(void) {
    printf("================================================================================\n");
    printf(" PROOF: CH 8 SEC 4 ARCTANGENT SERIES & PI COMPUTATION ENGINE\n");
    printf("================================================================================\n\n");

    const char *contract_addr = "dynamic_0x71C7656EC7ab88b098defB751B7401B5f6d8976F";
    const char *dat_bin_ch8_path = "/tmp/euler_vol1_ch8_sec4_arctan_proof.dat.bin";

    FILE *f = fopen(dat_bin_ch8_path, "wb");
    assert(f != NULL);
    uint8_t zero_buf[512] = {0};
    fwrite(zero_buf, 1, sizeof(zero_buf), f);
    fclose(f);

    int64_t preserved_x = 5;
    int64_t preserved_y = 11;
    uint64_t tangent_t = 200000; // t = 0.2 scaled by 1,000,000
    uint32_t terms = 10;

    AuncientEulerVolume1Chapter8Section4ArctanPiSeriesMetrics sec4_metrics = {0};

    bool ok = auncient_euler_volume1_chapter8_sec4_arctan_pi_series_engine(
        contract_addr,
        dat_bin_ch8_path,
        tangent_t,
        terms,
        preserved_x,
        preserved_y,
        &sec4_metrics
    );

    assert(ok == true);
    assert(sec4_metrics.ch8_sec4_arctan_engine_sound == true);
    assert(sec4_metrics.is_arctan_series_sound == true);
    assert(sec4_metrics.is_pi_computation_sound == true);

    printf("[SECTION 4 ARCTANGENT SERIES & PI COMPUTATION METRICS (§ 270-§ 285)]\n");
    printf("  - Latin Title:                 %s\n", sec4_metrics.section_latin_title);
    printf("  - Preserved Random Pair (x,y): (%ld, %ld)\n",
           sec4_metrics.preserved_random_x, sec4_metrics.preserved_random_y);
    printf("  - Input Tangent t:             t = 0.2 (Scaled t = %lu)\n", sec4_metrics.input_tangent_t_scaled);
    printf("  - Evaluated \\text{atan}(0.2):    \\text{atan}(0.2) = %lu (0.197395...)\n",
           sec4_metrics.evaluated_arctan_t_scaled);
    printf("  - Evaluated \\pi Value:         \\pi = %lu (3.141593... scaled by 1,000,000)\n",
           sec4_metrics.evaluated_pi_scaled);
    printf("  - Series Terms Computed:       %u Terms (§ 270 Arctan Power Series)\n",
           sec4_metrics.series_terms_computed);
    printf("  - Arctan & Pi Soundness:       %s\n",
           (sec4_metrics.is_arctan_series_sound && sec4_metrics.is_pi_computation_sound)
           ? "100% VERIFIED SOUND" : "FAILED");
    printf("  - Arctan Series WAL Checksum:  0x%lX\n", sec4_metrics.sec4_arctan_wal_checksum);
    printf("  - Rule 9 Dynamic Address:      %s (dynamic_<address>)\n",
           sec4_metrics.rule9_address_resolution_sound ? "SOUND" : "FAILED");
    printf("  - Rule 13 .dat.bin Layout:     %s (.dat.bin Quadtree)\n",
           sec4_metrics.rule13_dat_bin_verified ? "VERIFIED" : "FAILED");
    printf("  - Master FNV-1a Checksum:      0x%lX\n", sec4_metrics.acid_sec4_arctan_checksum);
    printf("  - 512-bit ZMM ReBAR Latch:     0x%lX\n\n", sec4_metrics.zmm_hardware_latch);

    printf("================================================================================\n");
    printf(" [PROOFS SUCCESSFUL] Section 4 Arctangent Series & Pi Computation Engine sound.\n");
    printf("================================================================================\n");

    remove(dat_bin_ch8_path);
    return 0;
}
