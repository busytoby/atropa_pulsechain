#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "../tsfi2-deepseek/inc/auncient_motzkin_engine.h"

int main(void) {
    printf("================================================================================\n");
    printf(" EULER VOL 1 CH 5: COMPOUND SURD HIGHER-DEGREE EXPONENTIATION PROOF\n");
    printf("================================================================================\n\n");

    const char *contract_addr = "dynamic_0x71C7656EC7ab88b098defB751B7401B5f6d8976F";
    const char *dat_bin_path = "/tmp/euler_vol1_ch5_surd_power_proof.dat.bin";

    FILE *f = fopen(dat_bin_path, "wb");
    assert(f != NULL);
    uint8_t zero_buf[512] = {0};
    fwrite(zero_buf, 1, sizeof(zero_buf), f);
    fclose(f);

    int64_t preserved_x = 5;

    /* -------------------------------------------------------------------------
     * PROOF: COMPOUND SURD EXPONENTIATION (a + b\sqrt{k})^n AT PRESERVED x = 5
     * Evaluate 3rd power (n=3) of (2 + \sqrt{5}):
     *   (2 + \sqrt{5})^3 = (2^3 + 3*2*1^2*5) + (3*2^2*1 + 1^3*5)\sqrt{5}
     *                    = (8 + 30) + (12 + 5)\sqrt{5}
     *                    = 38 + 17\sqrt{5}  (A_3 = 38, B_3 = 17)
     * Evaluate 4th power (n=4) of (2 + \sqrt{5}):
     *   (38 + 17\sqrt{5})(2 + \sqrt{5}) = (38*2 + 17*1*5) + (38*1 + 17*2)\sqrt{5}
     *                                   = (76 + 85) + (38 + 34)\sqrt{5}
     *                                   = 161 + 72\sqrt{5} (A_4 = 161, B_4 = 72)
     * ------------------------------------------------------------------------- */
    AuncientEulerVolume1Chapter5HigherDegreeSurdPowerMetrics p3_metrics = {0};
    bool ok_p3 = auncient_euler_volume1_chapter5_higher_degree_surd_power_engine(
        contract_addr,
        dat_bin_path,
        2, 1, 3, /* (2 + 1\sqrt{5})^3 */
        preserved_x,
        &p3_metrics
    );

    assert(ok_p3 == true);
    assert(p3_metrics.ch5_surd_power_engine_sound == true);
    assert(p3_metrics.expanded_rational_part == 38);
    assert(p3_metrics.expanded_surd_part == 17);

    AuncientEulerVolume1Chapter5HigherDegreeSurdPowerMetrics p4_metrics = {0};
    bool ok_p4 = auncient_euler_volume1_chapter5_higher_degree_surd_power_engine(
        contract_addr,
        dat_bin_path,
        2, 1, 4, /* (2 + 1\sqrt{5})^4 */
        preserved_x,
        &p4_metrics
    );

    assert(ok_p4 == true);
    assert(p4_metrics.ch5_surd_power_engine_sound == true);
    assert(p4_metrics.expanded_rational_part == 161);
    assert(p4_metrics.expanded_surd_part == 72);

    printf("[1. COMPOUND SURD 3RD POWER PROOF (n=3) AT x = %ld]\n", preserved_x);
    printf("    Base Surd:                  2 + 1*\\sqrt{5}\n");
    printf("    Exponent Power:             n = 3\n");
    printf("    Expanded Rational Part A3:  %ld\n", p3_metrics.expanded_rational_part);
    printf("    Expanded Surd Part B3:      %ld*\\sqrt{5}\n", p3_metrics.expanded_surd_part);
    printf("    Expanded Result:            38 + 17*\\sqrt{5} (EXACT MATCH!)\n");
    printf("    ACID Checksum:              0x%lX\n\n", p3_metrics.acid_surd_power_checksum);

    printf("[2. COMPOUND SURD 4TH POWER PROOF (n=4) AT x = %ld]\n", preserved_x);
    printf("    Base Surd:                  2 + 1*\\sqrt{5}\n");
    printf("    Exponent Power:             n = 4\n");
    printf("    Expanded Rational Part A4:  %ld\n", p4_metrics.expanded_rational_part);
    printf("    Expanded Surd Part B4:      %ld*\\sqrt{5}\n", p4_metrics.expanded_surd_part);
    printf("    Expanded Result:            161 + 72*\\sqrt{5} (EXACT MATCH!)\n");
    printf("    Radicand k ===== Preserved x: %s (k ===== x = 5)\n", p4_metrics.is_k_equal_preserved_x_verified ? "YES" : "NO");
    printf("    ACID Checksum:              0x%lX\n", p4_metrics.acid_surd_power_checksum);
    printf("    ZMM Hardware Latch:         0x%lX\n\n", p4_metrics.zmm_hardware_latch);

    printf("================================================================================\n");
    printf(" [PROOFS SUCCESSFUL] Compound Surd Higher-Degree Exponentiation is 100%% sound.\n");
    printf("================================================================================\n");

    remove(dat_bin_path);
    return 0;
}
