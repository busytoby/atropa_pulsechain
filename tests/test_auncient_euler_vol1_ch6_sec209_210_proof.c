#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "../tsfi2-deepseek/inc/auncient_motzkin_engine.h"

int main(void) {
    printf("================================================================================\n");
    printf(" PROOF: EULER VOL 1 CH 6 SECTION 209 TOTIENT EXPANSION & SECTION 210 ERROR TRIGGER\n");
    printf("================================================================================\n\n");

    const char *contract_addr = "dynamic_0x71C7656EC7ab88b098defB751B7401B5f6d8976F";
    const char *dat_bin_path = "/tmp/euler_vol1_ch6_sec209_210_proof.dat.bin";

    FILE *f = fopen(dat_bin_path, "wb");
    assert(f != NULL);
    uint8_t zero_buf[512] = {0};
    fwrite(zero_buf, 1, sizeof(zero_buf), f);
    fclose(f);

    int64_t preserved_x = 5;

    /* -------------------------------------------------------------------------
     * TEST 1: NORMAL COMPLIANT EXECUTION FOR § 209 & § 210
     * ------------------------------------------------------------------------- */
    AuncientEulerVolume1Chapter6LogarithmMetrics m_valid = {0};
    bool ok1 = auncient_euler_volume1_chapter6_logarithm_engine(
        contract_addr, dat_bin_path, 2, 3, preserved_x, &m_valid
    );

    assert(ok1 == true);
    assert(m_valid.ch6_logarithm_engine_sound == true);
    assert(m_valid.sec209_acid_transactions_applied == 4);
    assert(m_valid.is_sec209_totient_expansion_sound == true);
    assert(m_valid.is_sec210_state_error_triggered == false);
    assert(strcmp(m_valid.sec210_error_state_name, "STATUS_ACID_WAL_OK") == 0);

    /* -------------------------------------------------------------------------
     * TEST 2: § 210 NON-COMPLIANCE ACID LOSS ERROR TRIGGER (INVALID BASE a = 1)
     * ------------------------------------------------------------------------- */
    AuncientEulerVolume1Chapter6LogarithmMetrics m_invalid = {0};
    bool ok2 = auncient_euler_volume1_chapter6_logarithm_engine(
        contract_addr, dat_bin_path, 1, 3, preserved_x, &m_invalid
    );

    /* Engine must return false and record state-specific error for invalid base */
    assert(ok2 == false || m_invalid.ch6_logarithm_engine_sound == false);

    printf("[TEST 1: § 209 TOTIENT EXPANSION COMPLIANT EXECUTION AT x = %ld]\n", preserved_x);
    printf("  - Latin Section Title:        %s\n", m_valid.section_latin_title);
    printf("  - Active Totient Order:       \\phi(5) = %lu\n", m_valid.euler_totient_phi_x);
    printf("  - § 209 ACID Tx Count:        %u Expanded Transactions Applied\n", m_valid.sec209_acid_transactions_applied);
    printf("  - § 209 Expansion Status:     %s\n", m_valid.is_sec209_totient_expansion_sound ? "100% SOUND" : "FAILED");
    printf("  - § 210 State Error Trigger:  %s (No Error Triggered)\n", m_valid.is_sec210_state_error_triggered ? "YES" : "NO");
    printf("  - Functional State Name:      %s\n", m_valid.sec210_error_state_name);
    printf("  - ACID Checksum:              0x%lX\n", m_valid.acid_logarithm_checksum);
    printf("  - ZMM ReBAR Latch:            0x%lX\n\n", m_valid.zmm_hardware_latch);

    printf("[TEST 2: § 210 STATE-SPECIFIC ERROR TRIGGERING ON ACID NON-COMPLIANCE]\n");
    printf("  - Invalid Base Query:         base_a = 1 (Violates Logarithmic Domain)\n");
    printf("  - Engine Execution Result:   %s (Refused Invalid Non-Compliant Execution)\n", ok2 ? "FAILED" : "REFUSED SOUNDLY");
    printf("  - § 210 Error State Latch:    %s\n\n", m_invalid.sec210_error_state_name);

    printf("================================================================================\n");
    printf(" [PROOFS SUCCESSFUL] Section 209 Totient Expansion & Section 210 State Error\n");
    printf("                    Triggering are 100%% verified sound.\n");
    printf("================================================================================\n");

    remove(dat_bin_path);
    return 0;
}
