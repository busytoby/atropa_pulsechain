#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "../tsfi2-deepseek/inc/auncient_motzkin_engine.h"

int main(void) {
    printf("================================================================================\n");
    printf(" PROOF: EULER VOL 1 CH 6 ENGINES WITH DUAL PRESERVED RANDOM NUMBERS (x AND y)\n");
    printf("================================================================================\n\n");

    const char *contract_addr = "dynamic_0x71C7656EC7ab88b098defB751B7401B5f6d8976F";
    const char *dat_bin_path = "/tmp/euler_vol1_ch6_dual_random_proof.dat.bin";

    FILE *f = fopen(dat_bin_path, "wb");
    assert(f != NULL);
    uint8_t zero_buf[512] = {0};
    fwrite(zero_buf, 1, sizeof(zero_buf), f);
    fclose(f);

    int64_t preserved_x = 5;
    int64_t preserved_y = 11;

    /* -------------------------------------------------------------------------
     * PROOF 1: FUNDAMENTAL LOGARITHM ENGINE WITH DUAL RANDOM NUMBERS (x AND y)
     * ------------------------------------------------------------------------- */
    AuncientEulerVolume1Chapter6LogarithmMetrics m_log = {0};
    bool ok1 = auncient_euler_volume1_chapter6_logarithm_engine(
        contract_addr, dat_bin_path, 2, 3, preserved_x, preserved_y, &m_log
    );
    assert(ok1 == true);
    assert(m_log.ch6_logarithm_engine_sound == true);
    assert(m_log.preserved_random_x == 5);
    assert(m_log.preserved_random_y == 11);

    /* -------------------------------------------------------------------------
     * PROOF 2: SECTION 1 LOG LAWS ENGINE WITH DUAL RANDOM NUMBERS (x AND y)
     * ------------------------------------------------------------------------- */
    AuncientEulerVolume1Chapter6Section1LogLawsMetrics m_sec1 = {0};
    bool ok2 = auncient_euler_volume1_chapter6_sec1_log_laws_engine(
        contract_addr, dat_bin_path, 2, 4, 8, 3, preserved_x, preserved_y, &m_sec1
    );
    assert(ok2 == true);
    assert(m_sec1.ch6_sec1_engine_sound == true);
    assert(m_sec1.preserved_random_x == 5);
    assert(m_sec1.preserved_random_y == 11);

    /* -------------------------------------------------------------------------
     * PROOF 3: SECTION 2 BASE CHANGE ENGINE WITH DUAL RANDOM NUMBERS (x AND y)
     * ------------------------------------------------------------------------- */
    AuncientEulerVolume1Chapter6Section2LogBaseChangeMetrics m_sec2 = {0};
    bool ok3 = auncient_euler_volume1_chapter6_sec2_log_base_change_engine(
        contract_addr, dat_bin_path, 2, 4, 16, preserved_x, preserved_y, &m_sec2
    );
    assert(ok3 == true);
    assert(m_sec2.ch6_sec2_engine_sound == true);
    assert(m_sec2.preserved_random_x == 5);
    assert(m_sec2.preserved_random_y == 11);

    printf("[1. FUNDAMENTAL LOGARITHM ENGINE - DUAL RANDOM PAYLOAD]\n");
    printf("    Preserved Random x:              x = %ld\n", m_log.preserved_random_x);
    printf("    Preserved Random y:              y = %ld\n", m_log.preserved_random_y);
    printf("    Logarithm Checksum:              0x%lX\n\n", m_log.acid_logarithm_checksum);

    printf("[2. SECTION 1 LOG LAWS ENGINE - DUAL RANDOM PAYLOAD]\n");
    printf("    Preserved Random x:              x = %ld\n", m_sec1.preserved_random_x);
    printf("    Preserved Random y:              y = %ld\n", m_sec1.preserved_random_y);
    printf("    Sec 1 Checksum:                  0x%lX\n\n", m_sec1.acid_sec1_log_checksum);

    printf("[3. SECTION 2 BASE CHANGE ENGINE - DUAL RANDOM PAYLOAD]\n");
    printf("    Preserved Random x:              x = %ld\n", m_sec2.preserved_random_x);
    printf("    Preserved Random y:              y = %ld\n", m_sec2.preserved_random_y);
    printf("    Sec 2 Checksum:                  0x%lX\n\n", m_sec2.acid_sec2_log_checksum);

    printf("================================================================================\n");
    printf(" [PROOFS SUCCESSFUL] Dual random numbers (x and y) are 100%% integrated and sound\n");
    printf("                    across all Chapter 6 logarithm engines.\n");
    printf("================================================================================\n");

    remove(dat_bin_path);
    return 0;
}
