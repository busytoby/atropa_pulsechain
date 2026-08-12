#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "../tsfi2-deepseek/inc/auncient_motzkin_engine.h"

int main(void) {
    printf("================================================================================\n");
    printf(" EULER VOL 1 CH 5: SURD DIVISION & CONJUGATE RATIONALIZATION PROOF\n");
    printf("================================================================================\n\n");

    const char *contract_addr = "dynamic_0x71C7656EC7ab88b098defB751B7401B5f6d8976F";
    const char *dat_bin_path = "/tmp/euler_vol1_ch5_div_proof.dat.bin";

    FILE *f = fopen(dat_bin_path, "wb");
    assert(f != NULL);
    uint8_t zero_buf[512] = {0};
    fwrite(zero_buf, 1, sizeof(zero_buf), f);
    fclose(f);

    int64_t preserved_x = 5;

    /* -------------------------------------------------------------------------
     * PROOF: SURD DIVISION ENGINE WITH CONJUGATE RATIONALIZATION AT PRESERVED x = 5
     * Evaluate: (3 + 2\sqrt{5}) / (1 + 4\sqrt{5})  with a1=3, b1=2, a2=1, b2=4, x=5
     * Multiply by conjugate (1 - 4\sqrt{5}):
     * Numerator   = (3*1 - 2*4*5) + (2*1 - 3*4)\sqrt{5} = (3 - 40) + (2 - 12)\sqrt{5} = -37 - 10\sqrt{5}
     * Denominator = 1^2 - 4^2(5) = 1 - 80 = -79
     * Result Quotient = (-37 - 10\sqrt{5}) / -79 = (37 + 10\sqrt{5}) / 79
     * ------------------------------------------------------------------------- */
    AuncientEulerVolume1Chapter5SurdDivisionMetrics div_metrics = {0};
    bool ok = auncient_euler_volume1_chapter5_surd_division_engine(
        contract_addr,
        dat_bin_path,
        3, 2, /* Numerator: 3 + 2\sqrt{5} */
        1, 4, /* Denominator: 1 + 4\sqrt{5} */
        preserved_x,
        &div_metrics
    );

    assert(ok == true);
    assert(div_metrics.ch5_surd_div_engine_sound == true);
    assert(div_metrics.quotient_num_base == -37);
    assert(div_metrics.quotient_num_mult == -10);
    assert(div_metrics.quotient_rational_denom == -79);
    assert(div_metrics.is_quotient_denom_rational == true);

    printf("[SURD DIVISION & CONJUGATE RATIONALIZATION PROOF AT x = %ld]\n", preserved_x);
    printf("  - Latin Section Title:        %s\n", div_metrics.section_latin_title);
    printf("  - Numerator Surd:             3 + 2*\\sqrt{5}\n");
    printf("  - Denominator Surd:           1 + 4*\\sqrt{5}\n");
    printf("  - Conjugate Multiplier:       (1 - 4*\\sqrt{5})\n");
    printf("  - Quotient Numerator Base:    %ld\n", div_metrics.quotient_num_base);
    printf("  - Quotient Numerator Mult:    %ld*\\sqrt{5}\n", div_metrics.quotient_num_mult);
    printf("  - Quotient Rational Denom:    %ld (STRICTLY FREE OF SURDS!)\n", div_metrics.quotient_rational_denom);
    printf("  - Final Simplified Quotient:  (%ld + %ld*\\sqrt{5}) / %ld\n",
           -div_metrics.quotient_num_base, -div_metrics.quotient_num_mult, -div_metrics.quotient_rational_denom);
    printf("  - Radicand k == Preserved x:  %s (k == x = 5)\n", div_metrics.is_k_equal_preserved_x_verified ? "YES" : "NO");
    printf("  - ACID Checksum:              0x%lX\n", div_metrics.acid_surd_div_checksum);
    printf("  - ZMM Hardware Latch:         0x%lX\n\n", div_metrics.zmm_hardware_latch);

    printf("================================================================================\n");
    printf(" [PROOFS SUCCESSFUL] Chapter 5 Surd Division & Conjugate Rationalization 100%% sound.\n");
    printf("================================================================================\n");

    remove(dat_bin_path);
    return 0;
}
