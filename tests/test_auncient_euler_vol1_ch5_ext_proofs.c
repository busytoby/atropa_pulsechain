#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "../tsfi2-deepseek/inc/auncient_motzkin_engine.h"

int main(void) {
    printf("================================================================================\n");
    printf(" EULER VOL 1 CH 5: DENOMINATOR RATIONALIZATION & FACTOR EXTRACTION PROOFS\n");
    printf("================================================================================\n\n");

    const char *contract_addr = "dynamic_0x71C7656EC7ab88b098defB751B7401B5f6d8976F";
    const char *dat_bin_path = "/tmp/euler_vol1_ch5_ext_proofs.dat.bin";

    FILE *f = fopen(dat_bin_path, "wb");
    assert(f != NULL);
    uint8_t zero_buf[512] = {0};
    fwrite(zero_buf, 1, sizeof(zero_buf), f);
    fclose(f);

    int64_t preserved_x = 5;

    /* -------------------------------------------------------------------------
     * PROOF 1: BINOMIAL DENOMINATOR RATIONALIZATION ENGINE
     * Evaluate: 22 / (3 + 2\sqrt{5})  with A=22, a=3, b=2, x=5
     * Conjugate Product: 22 * (3 - 2\sqrt{5}) / (3^2 - 2^2 * 5)
     *                  = (66 - 44\sqrt{5}) / (9 - 20)
     *                  = (66 - 44\sqrt{5}) / -11
     *                  = -6 + 4\sqrt{5}
     * ------------------------------------------------------------------------- */
    AuncientEulerVolume1Chapter5DenominatorRationalizationMetrics rat_metrics = {0};
    bool ok_rat = auncient_euler_volume1_chapter5_denominator_rationalization_engine(
        contract_addr,
        dat_bin_path,
        22, 3, 2,
        preserved_x,
        &rat_metrics
    );

    assert(ok_rat == true);
    assert(rat_metrics.ch5_rationalization_engine_sound == true);
    assert(rat_metrics.is_denom_strictly_rational == true);
    assert(rat_metrics.rationalized_denom == -11);

    printf("[1. BINOMIAL DENOMINATOR RATIONALIZATION PROOF AT x = %ld]\n", preserved_x);
    printf("    Input Fraction:                22 / (3 + 2*\\sqrt{5})\n");
    printf("    Conjugate Multiplier:          (3 - 2*\\sqrt{5})\n");
    printf("    Rationalized Numerator Base:   %ld\n", rat_metrics.rationalized_num_base);
    printf("    Rationalized Numerator Mult:   %ld*\\sqrt{5}\n", rat_metrics.rationalized_num_mult);
    printf("    Rationalized Denominator:      %ld (STRICTLY FREE OF SURDS!)\n", rat_metrics.rationalized_denom);
    printf("    ACID Checksum:                 0x%lX\n\n", rat_metrics.acid_rationalization_checksum);

    /* -------------------------------------------------------------------------
     * PROOF 2: SURD FACTOR EXTRACTION ENGINE
     * Evaluate: \sqrt{4 * 5} = \sqrt{20}  with c=2, x=5
     * Extracted Form: 2 * \sqrt{5}
     * ------------------------------------------------------------------------- */
    AuncientEulerVolume1Chapter5SurdFactorExtractionMetrics fact_metrics = {0};
    bool ok_fact = auncient_euler_volume1_chapter5_surd_factor_extraction_engine(
        contract_addr,
        dat_bin_path,
        2, /* c = 2, c^2 = 4 */
        preserved_x,
        &fact_metrics
    );

    assert(ok_fact == true);
    assert(fact_metrics.ch5_factor_extraction_sound == true);
    assert(fact_metrics.total_radicand == 20);
    assert(fact_metrics.extracted_surd_multiplier == 2);
    assert(fact_metrics.reduced_radicand_x == 5);

    printf("[2. SURD FACTOR EXTRACTION PROOF AT x = %ld]\n", preserved_x);
    printf("    Total Radicand Input:          \\sqrt{%lu} (\\sqrt{4 * 5})\n", fact_metrics.total_radicand);
    printf("    Extracted Perfect Square Factor c: %ld (c^2 = %ld)\n",
           fact_metrics.perfect_square_factor_c, fact_metrics.perfect_square_factor_c * fact_metrics.perfect_square_factor_c);
    printf("    Extracted Surd Expression:     %ld*\\sqrt{%lu}\n",
           fact_metrics.extracted_surd_multiplier, fact_metrics.reduced_radicand_x);
    printf("    Radicand Reduced Equality:     %s (Reduced Radicand == Preserved x = 5)\n",
           (fact_metrics.reduced_radicand_x == (uint64_t)preserved_x) ? "YES (100% SOUND)" : "NO");
    printf("    ACID Checksum:                 0x%lX\n\n", fact_metrics.acid_factor_checksum);

    printf("================================================================================\n");
    printf(" [ALL PROOFS SUCCESSFUL] Rationalization and Factor Extraction are 100%% sound.\n");
    printf("================================================================================\n");

    remove(dat_bin_path);
    return 0;
}
