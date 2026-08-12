#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "../tsfi2-deepseek/inc/auncient_motzkin_engine.h"

int main(void) {
    printf("================================================================================\n");
    printf(" EULER OPERA OMNIA VOL 1 CH 5: SURD ARITHMETIC & CONJUGATE PROOF\n");
    printf("================================================================================\n\n");

    const char *contract_addr = "dynamic_0x71C7656EC7ab88b098defB751B7401B5f6d8976F";
    const char *dat_bin_ch5_path = "/tmp/euler_vol1_ch5_proof.dat.bin";

    FILE *f = fopen(dat_bin_ch5_path, "wb");
    assert(f != NULL);
    uint8_t zero_buf[512] = {0};
    fwrite(zero_buf, 1, sizeof(zero_buf), f);
    fclose(f);

    int64_t preserved_x = 5;

    /* Surd 1: 3 + 2\sqrt{5} (a1 = 3, b1 = 2) */
    /* Surd 2: 1 + 4\sqrt{5} (a2 = 1, b2 = 4) */
    AuncientEulerVolume1Chapter5SurdArithmeticMetrics metrics = {0};

    bool ok = auncient_euler_volume1_chapter5_surd_arithmetic_engine(
        contract_addr,
        dat_bin_ch5_path,
        3, 2,  /* 3 + 2\sqrt{5} */
        1, 4,  /* 1 + 4\sqrt{5} */
        preserved_x,
        &metrics
    );

    assert(ok == true);
    assert(metrics.ch5_arithmetic_engine_sound == true);
    assert(metrics.is_k_equal_preserved_x_verified == true);
    assert(metrics.is_conjugate_norm_rational == true);

    printf("[SURD ARITHMETIC PROOF AT PRESERVED x = %ld]\n", preserved_x);
    printf("  - Expression 1: %ld + %ld*\\sqrt{%ld}\n", metrics.base1_a, metrics.mult1_b, metrics.preserved_random_x);
    printf("  - Expression 2: %ld + %ld*\\sqrt{%ld}\n\n", metrics.base2_c, metrics.mult2_d, metrics.preserved_random_x);

    printf("[1. ADDITION PROOF]\n");
    printf("  - (3 + 2\\sqrt{5}) + (1 + 4\\sqrt{5}) = %ld + %ld*\\sqrt{5}\n\n",
           metrics.sum_base_a, metrics.sum_mult_b);

    printf("[2. MULTIPLICATION PROOF]\n");
    printf("  - (3 + 2\\sqrt{5}) * (1 + 4\\sqrt{5}) = (3*1 + 2*4*5) + (3*4 + 1*2)\\sqrt{5}\n");
    printf("                                       = (3 + 40) + (12 + 2)\\sqrt{5}\n");
    printf("                                       = %ld + %ld*\\sqrt{5}\n\n",
           metrics.prod_base_a, metrics.prod_mult_b);

    printf("[3. CONJUGATE RATIONALIZATION NORM PROOF]\n");
    printf("  - (3 + 2\\sqrt{5}) * (3 - 2\\sqrt{5}) = 3^2 - 2^2 * 5\n");
    printf("                                       = 9 - 4 * 5 = 9 - 20\n");
    printf("                                       = %ld (STRICTLY RATIONAL INTEGER!)\n\n",
           metrics.conjugate_norm);

    printf("[HARDWARE & RULE COMPLIANCE]\n");
    printf("  - Rule 9 Dynamic Address Sound: %s\n", metrics.rule9_address_resolution_sound ? "YES" : "NO");
    printf("  - Rule 13 .dat.bin Sound: %s\n", metrics.rule13_dat_bin_verified ? "YES" : "NO");
    printf("  - ACID Checksum: 0x%lX | ZMM Hardware Latch: 0x%lX\n",
           metrics.acid_surd_arithmetic_checksum, metrics.zmm_hardware_latch);

    printf("================================================================================\n");
    printf(" [PROOF SUCCESSFUL] Preserved variable x = %ld unlocks complete surd arithmetic,\n", preserved_x);
    printf("                    proving that conjugate multiplication collapses irrational surds\n");
    printf("                    back to strictly rational integer norms (%ld).\n", metrics.conjugate_norm);
    printf("================================================================================\n");

    remove(dat_bin_ch5_path);
    return 0;
}
