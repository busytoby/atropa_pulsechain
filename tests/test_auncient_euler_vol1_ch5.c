#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "../tsfi2-deepseek/inc/auncient_motzkin_engine.h"

int main(void) {
    printf("=== Auncient HathiTrust Euler Opera Omnia Vol 1 Ch 5 Test ===\n");

    const char *contract_addr = "dynamic_0x71C7656EC7ab88b098defB751B7401B5f6d8976F";
    const char *dat_bin_ch5_path = "/tmp/euler_vol1_ch5.dat.bin";

    /* Create dummy .dat.bin asset (Rule 13 layout) */
    FILE *f = fopen(dat_bin_ch5_path, "wb");
    assert(f != NULL);
    uint8_t zero_buf[512] = {0};
    fwrite(zero_buf, 1, sizeof(zero_buf), f);
    fclose(f);

    /* Test Chapter 5 Irrational Surd Engine: 3 + 2*\sqrt{5} */
    AuncientEulerVolume1Chapter5IrrationalSurdMetrics ch5_metrics = {0};
    bool ok = auncient_euler_volume1_chapter5_irrational_surd_engine(
        contract_addr,
        dat_bin_ch5_path,
        3,  /* rational base a */
        2,  /* surd multiplier b */
        5,  /* radicand k */
        &ch5_metrics
    );

    assert(ok == true);
    assert(ch5_metrics.ch5_surd_engine_sound == true);
    assert(ch5_metrics.rule9_address_resolution_sound == true);
    assert(ch5_metrics.rule13_dat_bin_verified == true);
    assert(ch5_metrics.is_irrational_surd_verified == true);
    assert(ch5_metrics.is_non_imaginary_verified == true);

    printf("[PASS] Chapter 5 Engine Verified:\n");
    printf("       Title: %s\n", ch5_metrics.chapter_latin_title);
    printf("       Expression: %ld + %ld*\\sqrt{%lu}\n",
           ch5_metrics.rational_base, ch5_metrics.surd_multiplier, ch5_metrics.radicand_k);
    printf("       ACID Checksum: 0x%lX | ZMM Hardware Latch: 0x%lX\n",
           ch5_metrics.acid_surd_checksum, ch5_metrics.zmm_hardware_latch);

    remove(dat_bin_ch5_path);
    printf("=== Chapter 5 Ingestion & Reader Test Passed cleanly ===\n");
    return 0;
}
