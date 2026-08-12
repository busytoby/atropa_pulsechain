#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "../tsfi2-deepseek/inc/auncient_motzkin_engine.h"

int main(void) {
    printf("================================================================================\n");
    printf(" AUNCIENT EULER OPERA OMNIA VOL. 1 - PRESERVED X CONTINUITY VERIFICATION\n");
    printf("================================================================================\n\n");

    const char *contract_addr = "dynamic_0x71C7656EC7ab88b098defB751B7401B5f6d8976F";
    const char *dat_bin_path = "/tmp/euler_vol1_preserved_x.dat.bin";

    FILE *f = fopen(dat_bin_path, "wb");
    assert(f != NULL);
    uint8_t zero_buf[512] = {0};
    fwrite(zero_buf, 1, sizeof(zero_buf), f);
    fclose(f);

    /* Preserved random x value established in earlier ACID pipeline testing */
    int64_t preserved_x = 7;

    /* 1. Chapter 3 ACID Checksum Engine Evaluation with preserved_x = 7 */
    int64_t poly_p[3] = { 1, 2, 1 }; // 1 + 2x + x^2
    int64_t poly_q[3] = { 3, 1, 0 }; // 3 + x
    AuncientEulerVolume1Chapter3AcidChecksumMetrics ch3_metrics = {0};

    bool ch3_ok = auncient_euler_volume1_chapter3_acid_checksum_engine(
        contract_addr,
        dat_bin_path,
        poly_p,
        poly_q,
        preserved_x,
        &ch3_metrics
    );
    assert(ch3_ok == true);

    printf("[STEP 1 - CHAPTER 3 ACID PIPELINE]\n");
    printf("  - Preserved Random Variable x: %ld\n", ch3_metrics.preserved_random_x);
    printf("  - Evaluated P(x = %ld): %ld\n", ch3_metrics.preserved_random_x, ch3_metrics.eval_p_x);
    printf("  - Evaluated Q(x = %ld): %ld\n", ch3_metrics.preserved_random_x, ch3_metrics.eval_q_x);
    printf("  - Evaluated Product R(x = %ld): %ld\n", ch3_metrics.preserved_random_x, ch3_metrics.eval_r_x);
    printf("  - Transactional Checksum: 0x%lX\n\n", ch3_metrics.acid_transactional_checksum);

    /* 2. Chapter 4 Fractional Division Engine Evaluation with preserved_x = 7 */
    AuncientEulerVolume1Chapter4FractionDivMetrics ch4_metrics = {0};
    bool ch4_ok = auncient_euler_volume1_chapter4_fraction_div_engine(
        contract_addr,
        dat_bin_path,
        2, 1, 1, 0, // N1 = 2x, D1 = 1
        1, 0, 1, 0, // N2 = 1,  D2 = 1
        preserved_x,
        &ch4_metrics
    );
    assert(ch4_ok == true);

    printf("[STEP 2 - CHAPTER 4 FRACTIONAL ENGINE]\n");
    printf("  - Preserved Random Variable x: %ld\n", preserved_x);
    printf("  - Solved Quotient Numerator: %ld\n", ch4_metrics.quot_num_x);
    printf("  - Solved Quotient Denominator: %ld\n", ch4_metrics.quot_den_x);
    printf("  - Transactional Checksum: 0x%lX\n\n", ch4_metrics.acid_fractional_checksum);

    /* 3. Chapter 5 Irrational Surd Engine Evaluation with preserved_x = 7 */
    AuncientEulerVolume1Chapter5IrrationalSurdMetrics ch5_metrics = {0};
    bool ch5_ok = auncient_euler_volume1_chapter5_irrational_surd_engine(
        contract_addr,
        dat_bin_path,
        3,           /* rational base a */
        2,           /* surd multiplier b */
        preserved_x, /* preserved random x */
        &ch5_metrics
    );
    assert(ch5_ok == true);

    printf("[STEP 3 - CHAPTER 5 IRRATIONAL SURD ENGINE]\n");
    printf("  - Preserved Random Variable x: %ld\n", ch5_metrics.preserved_random_x);
    printf("  - Radicand k: %lu\n", ch5_metrics.radicand_k);
    printf("  - Radicand Equality Verified (k == x): %s\n",
           ch5_metrics.is_k_equal_preserved_x_verified ? "YES (100% SOUND)" : "NO");
    printf("  - Evaluated Expression: %ld + %ld*\\sqrt{%lu}\n",
           ch5_metrics.rational_base, ch5_metrics.surd_multiplier, ch5_metrics.radicand_k);
    printf("  - ACID Checksum: 0x%lX\n", ch5_metrics.acid_surd_checksum);
    printf("  - ZMM Hardware Latch: 0x%lX\n\n", ch5_metrics.zmm_hardware_latch);

    /* 4. Cross-Chapter Continuity Assertion */
    assert(ch3_metrics.preserved_random_x == ch5_metrics.preserved_random_x);
    assert(ch5_metrics.radicand_k == (uint64_t)ch3_metrics.preserved_random_x);

    printf("================================================================================\n");
    printf(" [SUCCESS] Preserved variable x = %ld cleanly retained across Ch 3, Ch 4, and Ch 5.\n", preserved_x);
    printf("           Radicand k = %lu matches preserved x = %ld with 100%% continuity.\n",
           ch5_metrics.radicand_k, ch3_metrics.preserved_random_x);
    printf("================================================================================\n");

    remove(dat_bin_path);
    return 0;
}
