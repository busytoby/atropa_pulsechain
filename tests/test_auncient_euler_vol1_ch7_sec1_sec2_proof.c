#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "../tsfi2-deepseek/inc/auncient_motzkin_engine.h"

int main(void) {
    printf("================================================================================\n");
    printf(" PROOF: CH 7 SEC 1 EXPONENTIAL SERIES & SEC 2 EULER'S NUMBER e ENGINES\n");
    printf("================================================================================\n\n");

    const char *contract_addr = "dynamic_0x71C7656EC7ab88b098defB751B7401B5f6d8976F";
    const char *dat_bin_ch7_path = "/tmp/euler_vol1_ch7_sec1_sec2_proof.dat.bin";

    FILE *f = fopen(dat_bin_ch7_path, "wb");
    assert(f != NULL);
    uint8_t zero_buf[512] = {0};
    fwrite(zero_buf, 1, sizeof(zero_buf), f);
    fclose(f);

    int64_t preserved_x = 5;
    int64_t preserved_y = 11;

    /* 1. Section 1 Exponential Series Engine (§ 234-§ 236) */
    AuncientEulerVolume1Chapter7Section1ExponentialSeriesMetrics sec1_metrics = {0};
    bool ok1 = auncient_euler_volume1_chapter7_sec1_exponential_series_engine(
        contract_addr, dat_bin_ch7_path, 2, 100000, 5, preserved_x, preserved_y, &sec1_metrics
    );
    assert(ok1 == true);
    assert(sec1_metrics.ch7_sec1_exp_engine_sound == true);
    assert(sec1_metrics.is_exponential_series_sound == true);
    assert(sec1_metrics.is_x_y_registers_wmq_mounted == true);

    /* 2. Section 2 Euler's Number e Engine (§ 237-§ 238) */
    AuncientEulerVolume1Chapter7Section2EulerNumberEMetrics sec2_metrics = {0};
    bool ok2 = auncient_euler_volume1_chapter7_sec2_euler_number_e_engine(
        contract_addr, dat_bin_ch7_path, 100000, 5, preserved_x, preserved_y, &sec2_metrics
    );
    assert(ok2 == true);
    assert(sec2_metrics.ch7_sec2_e_engine_sound == true);
    assert(sec2_metrics.is_e_series_convergence_sound == true);
    assert(sec2_metrics.is_natural_base_wal_retained == true);

    printf("[SECTION 1 EXPONENTIAL SERIES ENGINE METRICS (§ 234-§ 236)]\n");
    printf("  - Latin Title:                 %s\n", sec1_metrics.section_latin_title);
    printf("  - Base a & Exponent z:         base_a = %lu, z = 0.1 (Scaled z = %lu)\n",
           sec1_metrics.base_a, sec1_metrics.input_z_scaled);
    printf("  - Evaluated a^z:               Scaled a^z = %lu (Terms = %u)\n",
           sec1_metrics.exp_a_z_scaled, sec1_metrics.binomial_terms_computed);
    printf("  - WMQ Mountable (x, y):        %s (x = %ld, y = %ld)\n",
           sec1_metrics.is_x_y_registers_wmq_mounted ? "YES (MOUNTED OVER STANAG VFIO)" : "NO",
           sec1_metrics.preserved_random_x, sec1_metrics.preserved_random_y);
    printf("  - Exponential Series WAL:      0x%lX\n", sec1_metrics.sec1_exp_wal_checksum);
    printf("  - Master FNV-1a Checksum:      0x%lX\n\n", sec1_metrics.acid_sec1_exp_checksum);

    printf("[SECTION 2 EULER'S NUMBER e ENGINE METRICS (§ 237-§ 238)]\n");
    printf("  - Latin Title:                 %s\n", sec2_metrics.section_latin_title);
    printf("  - Scaled Euler Constant e:     e = %lu (2.718281...)\n", sec2_metrics.euler_const_e_scaled);
    printf("  - Evaluated e^z:               Scaled e^z = %lu (z = 0.1, Terms = %u)\n",
           sec2_metrics.exp_e_z_scaled, sec2_metrics.e_series_terms_computed);
    printf("  - Natural Base e WAL Retained: %s (0x%lX)\n",
           sec2_metrics.is_natural_base_wal_retained ? "YES (100% INTACT)" : "NO",
           sec2_metrics.sec2_e_wal_checksum);
    printf("  - Master FNV-1a Checksum:      0x%lX\n\n", sec2_metrics.acid_sec2_e_checksum);

    printf("================================================================================\n");
    printf(" [PROOFS SUCCESSFUL] Chapter 7 Section 1 & Section 2 Engines 100%% sound.\n");
    printf("================================================================================\n");

    remove(dat_bin_ch7_path);
    return 0;
}
