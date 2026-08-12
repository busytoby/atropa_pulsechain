#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "../tsfi2-deepseek/inc/auncient_motzkin_engine.h"

int main(void) {
    printf("================================================================================\n");
    printf(" PROOF: EULER VOL 1 CH 7 SEC 5 FAST CONVERGING LOGARITHM SERIES ENGINE\n");
    printf("================================================================================\n\n");

    const char *contract_addr = "dynamic_0x71C7656EC7ab88b098defB751B7401B5f6d8976F";
    const char *dat_bin_ch7_path = "/tmp/euler_vol1_ch7_sec5_proof.dat.bin";

    FILE *f = fopen(dat_bin_ch7_path, "wb");
    assert(f != NULL);
    uint8_t zero_buf[512] = {0};
    fwrite(zero_buf, 1, sizeof(zero_buf), f);
    fclose(f);

    int64_t preserved_x = 5;
    int64_t preserved_y = 11;

    AuncientEulerVolume1Chapter7Section5FastConvergingLogSeriesMetrics sec5_metrics = {0};
    bool ok = auncient_euler_volume1_chapter7_sec5_fast_converging_log_series_engine(
        contract_addr,
        dat_bin_ch7_path,
        100000, // x = 0.1 => \ln(1.1 / 0.9) = \ln(1.2222...)
        5,      // 5 terms
        preserved_x,
        preserved_y,
        &sec5_metrics
    );

    assert(ok == true);
    assert(sec5_metrics.ch7_sec5_fast_log_engine_sound == true);
    assert(sec5_metrics.is_fast_series_convergence_sound == true);
    assert(sec5_metrics.is_stanag_vfio_wmq_mounted == true);

    printf("[SECTION 5 FAST CONVERGING LOGARITHM SERIES METRICS (§ 240-§ 258)]\n");
    printf("  - Latin Title:                 %s\n", sec5_metrics.section_latin_title);
    printf("  - Input x:                     x = 0.1 (Scaled x = %lu)\n", sec5_metrics.input_x_scaled);
    printf("  - Evaluated \\ln((1+x)/(1-x)):   Scaled \\ln = %lu (Terms = %u)\n",
           sec5_metrics.ln_ratio_scaled, sec5_metrics.fast_terms_computed);
    printf("  - STANAG VFIO WMQ Mounted:     %s (Preserved Pair x = %ld, y = %ld)\n",
           sec5_metrics.is_stanag_vfio_wmq_mounted ? "YES (100% INTACT)" : "NO",
           sec5_metrics.preserved_random_x, sec5_metrics.preserved_random_y);
    printf("  - Fast Log Series WAL:         0x%lX\n", sec5_metrics.sec5_fast_log_wal_checksum);
    printf("  - Master FNV-1a Checksum:      0x%lX\n", sec5_metrics.acid_sec5_log_checksum);
    printf("  - 512-bit ZMM ReBAR Latch:      0x%lX\n\n", sec5_metrics.zmm_hardware_latch);

    printf("================================================================================\n");
    printf(" [PROOFS SUCCESSFUL] Chapter 7 Section 5 Fast Converging Logarithm Series Engine sound.\n");
    printf("================================================================================\n");

    remove(dat_bin_ch7_path);
    return 0;
}
