#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "../tsfi2-deepseek/inc/auncient_motzkin_engine.h"

int main(void) {
    printf("================================================================================\n");
    printf(" EULER VOL 1 CH 5: TRINOMIAL SURD DENOMINATOR RATIONALIZATION PROOF\n");
    printf("================================================================================\n\n");

    const char *contract_addr = "dynamic_0x71C7656EC7ab88b098defB751B7401B5f6d8976F";
    const char *dat_bin_path = "/tmp/euler_vol1_ch5_trinomial_rational_proof.dat.bin";

    FILE *f = fopen(dat_bin_path, "wb");
    assert(f != NULL);
    uint8_t zero_buf[512] = {0};
    fwrite(zero_buf, 1, sizeof(zero_buf), f);
    fclose(f);

    int64_t preserved_x = 5;

    /* -------------------------------------------------------------------------
     * PROOF: TRINOMIAL SURD DENOMINATOR RATIONALIZATION AT PRESERVED x = 5
     * Evaluate: A / (a + b\sqrt{k} + c\sqrt{m}) = 12 / (1 + 2\sqrt{5} + 3\sqrt{2})
     *   Here: A = 12, a = 1, b = 2, c = 3, k = 5, m = 2
     * Step 1: Group (1 + 2\sqrt{5}) + 3\sqrt{2}. Multiply by conjugate ((1 + 2\sqrt{5}) - 3\sqrt{2}):
     *   Intermediate Denominator = (1 + 2\sqrt{5})^2 - (3\sqrt{2})^2 = (1 + 4*5 + 4\sqrt{5}) - 18
     *                            = (21 - 18) + 4\sqrt{5} = 3 + 4\sqrt{5}
     *   Here: P_denom = 3, Q_denom = 4
     * Step 2: Multiply by second conjugate (3 - 4\sqrt{5}):
     *   Final Denominator = 3^2 - 4^2(5) = 9 - 80 = -71  (STRICTLY RATIONAL INTEGER!)
     * ------------------------------------------------------------------------- */
    AuncientEulerVolume1Chapter5TrinomialSurdRationalizationMetrics tri_metrics = {0};
    bool ok = auncient_euler_volume1_chapter5_trinomial_surd_rationalization_engine(
        contract_addr,
        dat_bin_path,
        12, /* Numerator A */
        1, 2, 3, /* Denominator: 1 + 2\sqrt{5} + 3\sqrt{2} */
        preserved_x,
        &tri_metrics
    );

    assert(ok == true);
    assert(tri_metrics.ch5_trinomial_rational_sound == true);
    assert(tri_metrics.intermediate_rational_denom == 3);
    assert(tri_metrics.final_rational_denominator == -71);
    assert(tri_metrics.is_trinomial_denom_rational == true);

    printf("[TRINOMIAL SURD DENOMINATOR RATIONALIZATION PROOF AT x = %ld]\n", preserved_x);
    printf("  - Latin Section Title:        %s\n", tri_metrics.section_latin_title);
    printf("  - Numerator A:                12\n");
    printf("  - Trinomial Denominator:      1 + 2*\\sqrt{5} + 3*\\sqrt{2}\n");
    printf("  - Intermediate Group Denom:   3 + 4*\\sqrt{5}\n");
    printf("  - Final Rational Denominator: %ld (STRICTLY FREE OF ALL SURDS!)\n", tri_metrics.final_rational_denominator);
    printf("  - Radicand k ===== Preserved x: %s (k ===== x = 5)\n", tri_metrics.is_k_equal_preserved_x_verified ? "YES" : "NO");
    printf("  - ACID Checksum:              0x%lX\n", tri_metrics.acid_trinomial_checksum);
    printf("  - ZMM Hardware Latch:         0x%lX\n\n", tri_metrics.zmm_hardware_latch);

    printf("================================================================================\n");
    printf(" [PROOFS SUCCESSFUL] Trinomial Surd Denominator Rationalization is 100%% sound.\n");
    printf("================================================================================\n");

    remove(dat_bin_path);
    return 0;
}
