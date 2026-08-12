#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "../tsfi2-deepseek/inc/auncient_motzkin_engine.h"

int main(void) {
    printf("=== Auncient HathiTrust Euler Vol 1 Ch 5 (Radicand k == Preserved x) Test ===\n");

    const char *contract_addr = "dynamic_0x71C7656EC7ab88b098defB751B7401B5f6d8976F";
    const char *dat_bin_ch5_path = "/tmp/euler_vol1_ch5_k_x.dat.bin";

    /* Create dummy .dat.bin asset (Rule 13 layout) */
    FILE *f = fopen(dat_bin_ch5_path, "wb");
    assert(f != NULL);
    uint8_t zero_buf[512] = {0};
    fwrite(zero_buf, 1, sizeof(zero_buf), f);
    fclose(f);

    /* Test Chapter 5 Engine with preserved_random_x = 5 (so k == x == 5) */
    int64_t preserved_x = 5;
    AuncientEulerVolume1Chapter5IrrationalSurdMetrics ch5_metrics = {0};
    bool ok = auncient_euler_volume1_chapter5_irrational_surd_engine(
        contract_addr,
        dat_bin_ch5_path,
        3,           /* rational base a */
        2,           /* surd multiplier b */
        preserved_x, /* preserved random x */
        &ch5_metrics
    );

    assert(ok == true);
    assert(ch5_metrics.ch5_surd_engine_sound == true);
    assert(ch5_metrics.is_k_equal_preserved_x_verified == true);
    assert(ch5_metrics.radicand_k == (uint64_t)preserved_x);
    assert(ch5_metrics.rule9_address_resolution_sound == true);
    assert(ch5_metrics.rule13_dat_bin_verified == true);

    printf("[PASS] Radicand k == Preserved x Verification Succeeded:\n");
    printf("       Title: %s\n", ch5_metrics.chapter_latin_title);
    printf("       Preserved x: %ld | Radicand k: %lu (k == x verified)\n",
           ch5_metrics.preserved_random_x, ch5_metrics.radicand_k);
    printf("       Expression: %ld + %ld*\\sqrt{%lu}\n",
           ch5_metrics.rational_base, ch5_metrics.surd_multiplier, ch5_metrics.radicand_k);

    remove(dat_bin_ch5_path);
    printf("=== Radicand Equality Test Passed cleanly ===\n");
    return 0;
}
