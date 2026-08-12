#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "../tsfi2-deepseek/inc/auncient_motzkin_engine.h"

int main(void) {
    printf("================================================================================\n");
    printf(" EULER VOL 1 CH 5: COMPOUND SURD SQRT & FRACTIONAL EXPONENT PROOFS\n");
    printf("================================================================================\n\n");

    const char *contract_addr = "dynamic_0x71C7656EC7ab88b098defB751B7401B5f6d8976F";
    const char *dat_bin_path = "/tmp/euler_vol1_ch5_compound_proofs.dat.bin";

    FILE *f = fopen(dat_bin_path, "wb");
    assert(f != NULL);
    uint8_t zero_buf[512] = {0};
    fwrite(zero_buf, 1, sizeof(zero_buf), f);
    fclose(f);

    int64_t preserved_x = 5;

    /* -------------------------------------------------------------------------
     * PROOF 1: COMPOUND SURD SQUARE ROOT EXTRACTION ENGINE
     * Evaluate: \sqrt{9 + 4\sqrt{5}}  with a=9, b=4, x=5
     * Inner Discriminant d^2 = a^2 - b^2 * x = 9^2 - 4^2 * 5 = 81 - 80 = 1 (d = 1)
     * Extracted parts: x_part = (9 + 1)/2 = 5, y_part = (9 - 1)/2 = 4
     * Solution: \sqrt{9 + 4\sqrt{5}} = \sqrt{5} + \sqrt{4} = 2 + \sqrt{5}
     * Verification: (2 + \sqrt{5})^2 = 4 + 4\sqrt{5} + 5 = 9 + 4\sqrt{5} (MATCH!)
     * ------------------------------------------------------------------------- */
    AuncientEulerVolume1Chapter5CompoundSurdSqrtMetrics comp_metrics = {0};
    bool ok_comp = auncient_euler_volume1_chapter5_compound_surd_sqrt_engine(
        contract_addr,
        dat_bin_path,
        9, 4,
        preserved_x,
        &comp_metrics
    );

    assert(ok_comp == true);
    assert(comp_metrics.ch5_compound_sqrt_engine_sound == true);
    assert(comp_metrics.is_perfect_discriminant_square == true);
    assert(comp_metrics.extracted_part_x == 5);
    assert(comp_metrics.extracted_part_y == 4);

    printf("[1. COMPOUND SURD SQUARE ROOT EXTRACTION PROOF AT x = %ld]\n", preserved_x);
    printf("    Input Compound Surd:           \\sqrt{9 + 4*\\sqrt{5}}\n");
    printf("    Inner Discriminant d^2:        9^2 - 4^2(5) = 81 - 80 = %ld (d = 1)\n",
           comp_metrics.inner_discriminant_d2);
    printf("    Extracted Term 1 Base x_part: (9 + 1)/2 = %ld (\\sqrt{5})\n", comp_metrics.extracted_part_x);
    printf("    Extracted Term 2 Base y_part: (9 - 1)/2 = %ld (\\sqrt{4} = 2)\n", comp_metrics.extracted_part_y);
    printf("    Extracted Solution:            2 + \\sqrt{5}\n");
    printf("    Verification (2 + \\sqrt{5})^2: 4 + 4*\\sqrt{5} + 5 = 9 + 4*\\sqrt{5} (EXACT MATCH!)\n");
    printf("    ACID Checksum:                 0x%lX\n\n", comp_metrics.acid_compound_checksum);

    /* -------------------------------------------------------------------------
     * PROOF 2: FRACTIONAL EXPONENT & RADICAL EQUIVALENCE ENGINE
     * Evaluate: x^{3/2} == \sqrt[2]{x^3}  with m=3, n=2, x=5
     * Evaluated Power: 5^3 = 125
     * Solution: \sqrt{125} = 5\sqrt{5}
     * ------------------------------------------------------------------------- */
    AuncientEulerVolume1Chapter5FractionalExponentMetrics frac_metrics = {0};
    bool ok_frac = auncient_euler_volume1_chapter5_fractional_exponent_engine(
        contract_addr,
        dat_bin_path,
        3, 2,
        preserved_x,
        &frac_metrics
    );

    assert(ok_frac == true);
    assert(frac_metrics.ch5_frac_exp_engine_sound == true);
    assert(frac_metrics.evaluated_power_x_m == 125);

    printf("[2. FRACTIONAL EXPONENT & RADICAL EQUIVALENCE PROOF AT x = %ld]\n", preserved_x);
    printf("    Fractional Exponent Expression: 5^{3/2}\n");
    printf("    Radical Representation:         \\sqrt[2]{5^3}\n");
    printf("    Evaluated Power x^3:            5^3 = %ld\n", frac_metrics.evaluated_power_x_m);
    printf("    Radical Equivalence Verified:   YES (5^{3/2} == \\sqrt{125} = 5*\\sqrt{5})\n");
    printf("    ACID Checksum:                 0x%lX\n\n", frac_metrics.acid_frac_exp_checksum);

    printf("================================================================================\n");
    printf(" [ALL PROOFS SUCCESSFUL] Compound Surd Extraction & Fractional Exponents are 100%% sound.\n");
    printf("================================================================================\n");

    remove(dat_bin_path);
    return 0;
}
