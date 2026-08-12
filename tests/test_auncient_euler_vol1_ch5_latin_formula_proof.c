#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "../tsfi2-deepseek/inc/auncient_motzkin_engine.h"

int main(void) {
    printf("================================================================================\n");
    printf(" EULER VOL 1 CH 5: LATIN FORMULA SYNTHESIS ENGINE PROOF\n");
    printf("================================================================================\n\n");

    const char *contract_addr = "dynamic_0x71C7656EC7ab88b098defB751B7401B5f6d8976F";
    const char *dat_bin_path = "/tmp/euler_vol1_ch5_latin_formula_proof.dat.bin";

    FILE *f = fopen(dat_bin_path, "wb");
    assert(f != NULL);
    uint8_t zero_buf[512] = {0};
    fwrite(zero_buf, 1, sizeof(zero_buf), f);
    fclose(f);

    int64_t preserved_x = 5;

    /* -------------------------------------------------------------------------
     * PROOF: LATIN FORMULA SYNTHESIS ENGINE FOR CHAPTER 5
     * Synthesizes and validates:
     *   1. Forward Continuity: 'nuncupatur erat ordinatorum est'
     *   2. Reverse Audit:     'ordinatorum erat nuncupatur est'
     *   3. UBI Locators:      'ubi', 'ubi his qui', 'ubi et', 'ubi est'
     * ------------------------------------------------------------------------- */
    AuncientEulerVolume1Chapter5LatinFormulaMetrics form_metrics = {0};
    bool ok = auncient_euler_volume1_chapter5_latin_formula_engine(
        contract_addr,
        dat_bin_path,
        preserved_x,
        &form_metrics
    );

    assert(ok == true);
    assert(form_metrics.ch5_latin_formula_sound == true);
    assert(form_metrics.preserved_random_x == 5);
    assert(form_metrics.radicand_k == 5);
    assert(form_metrics.euler_totient_phi_x == 4);
    assert(form_metrics.is_forward_formula_sound == true);
    assert(form_metrics.is_reverse_formula_sound == true);
    assert(form_metrics.is_ubi_locator_latching_sound == true);

    printf("[LATIN FORMULA SYNTHESIS PROOF AT PRESERVED x = %ld]\n", preserved_x);
    printf("  - Latin Section Title:          %s\n", form_metrics.section_latin_title);
    printf("  - Forward Formula 1:            'nuncupatur erat ordinatorum est' (%s)\n",
           form_metrics.is_forward_formula_sound ? "100% SOUND" : "FAILED");
    printf("  - Reverse Formula 2:            'ordinatorum erat nuncupatur est' (%s)\n",
           form_metrics.is_reverse_formula_sound ? "100% SOUND" : "FAILED");
    printf("  - UBI Address Locators:         ubi, ubi his qui, ubi et, ubi est (%s)\n",
           form_metrics.is_ubi_locator_latching_sound ? "LATCHED & SOUND" : "FAILED");
    printf("  - Master FNV-1a Checksum:       0x%lX\n", form_metrics.master_latin_formula_checksum);
    printf("  - ZMM Hardware Latch:           0x%lX\n\n", form_metrics.zmm_hardware_latch);

    printf("================================================================================\n");
    printf(" [PROOFS SUCCESSFUL] Chapter 5 Latin Formula Synthesis Engine is 100%% sound.\n");
    printf("================================================================================\n");

    remove(dat_bin_path);
    return 0;
}
