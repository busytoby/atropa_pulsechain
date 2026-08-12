#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "../tsfi2-deepseek/inc/auncient_motzkin_engine.h"

int main(void) {
    printf("================================================================================\n");
    printf(" EULER VOL 1 CH 5: CUBE ROOT EXTRACTION FROM BINOMIAL SURDS PROOF\n");
    printf("================================================================================\n\n");

    const char *contract_addr = "dynamic_0x71C7656EC7ab88b098defB751B7401B5f6d8976F";
    const char *dat_bin_path = "/tmp/euler_vol1_ch5_cube_root_proof.dat.bin";

    FILE *f = fopen(dat_bin_path, "wb");
    assert(f != NULL);
    uint8_t zero_buf[512] = {0};
    fwrite(zero_buf, 1, sizeof(zero_buf), f);
    fclose(f);

    int64_t preserved_x = 5;

    /* -------------------------------------------------------------------------
     * PROOF: CUBE ROOT EXTRACTION FROM BINOMIAL SURDS AT PRESERVED x = 5
     * Evaluate: \sqrt[3]{38 + 17\sqrt{5}}  with a=38, b=17, k=5
     * Cubing (2 + \sqrt{5})^3:
     *   Real part: x^3 + 3x y^2 k = 2^3 + 3(2)(1^2)(5) = 8 + 30 = 38 (MATCH!)
     *   Surd part: (3x^2 y + y^3 k)\sqrt{5} = (3(4)(1) + 1^3(5))\sqrt{5} = (12 + 5)\sqrt{5} = 17\sqrt{5} (MATCH!)
     * Solution: \sqrt[3]{38 + 17\sqrt{5}} = 2 + \sqrt{5}
     * ------------------------------------------------------------------------- */
    AuncientEulerVolume1Chapter5CubeRootExtractionMetrics cube_metrics = {0};
    bool ok = auncient_euler_volume1_chapter5_cube_root_extraction_engine(
        contract_addr,
        dat_bin_path,
        38, 17, /* Binomial surd input: 38 + 17\sqrt{5} */
        preserved_x,
        &cube_metrics
    );

    assert(ok == true);
    assert(cube_metrics.ch5_cube_root_engine_sound == true);
    assert(cube_metrics.extracted_cube_root_x == 2);
    assert(cube_metrics.extracted_cube_root_y == 1);
    assert(cube_metrics.is_cube_root_exact_verified == true);

    printf("[CUBE ROOT EXTRACTION FROM BINOMIAL SURD PROOF AT x = %ld]\n", preserved_x);
    printf("  - Latin Section Title:        %s\n", cube_metrics.section_latin_title);
    printf("  - Input Binomial Surd:        \\sqrt[3]{38 + 17*\\sqrt{5}}\n");
    printf("  - Radicand k ===== Preserved x: %lu (k ===== x = 5)\n", cube_metrics.radicand_k);
    printf("  - Extracted Rational Term x:  x = %ld\n", cube_metrics.extracted_cube_root_x);
    printf("  - Extracted Surd Term y:      y = %ld (y*\\sqrt{5} = 1*\\sqrt{5})\n", cube_metrics.extracted_cube_root_y);
    printf("  - Extracted Solution:         2 + \\sqrt{5}\n");
    printf("  - Verification (2+\\sqrt{5})^3: 38 + 17*\\sqrt{5} (%s)\n",
           cube_metrics.is_cube_root_exact_verified ? "EXACT MATCH" : "MISMATCH");
    printf("  - ACID Checksum:              0x%lX\n", cube_metrics.acid_cube_root_checksum);
    printf("  - ZMM Hardware Latch:         0x%lX\n\n", cube_metrics.zmm_hardware_latch);

    printf("================================================================================\n");
    printf(" [PROOFS SUCCESSFUL] Chapter 5 Cube Root Extraction from Binomial Surds 100%% sound.\n");
    printf("================================================================================\n");

    remove(dat_bin_path);
    return 0;
}
