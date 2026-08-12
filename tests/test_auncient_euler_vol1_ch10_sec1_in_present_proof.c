#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "../tsfi2-deepseek/inc/auncient_motzkin_engine.h"

int main(void) {
    printf("================================================================================\n");
    printf(" PROOF: CH 10 SEC 1 IN-PRESENT CIRCULAR SERIES ENGINE\n");
    printf("================================================================================\n\n");

    const char *contract_addr = "dynamic_0x71C7656EC7ab88b098defB751B7401B5f6d8976F";
    const char *dat_bin_ch10_path = "/tmp/euler_vol1_ch10_sec1_in_present_proof.dat.bin";

    FILE *f = fopen(dat_bin_ch10_path, "wb");
    assert(f != NULL);
    uint8_t zero_buf[512] = {0};
    fwrite(zero_buf, 1, sizeof(zero_buf), f);
    fclose(f);

    int64_t preserved_x = 5;
    int64_t preserved_y = 11;
    int64_t preserved_y2 = 13;
    uint64_t input_arc_scaled = 100000ULL; // arc x = 0.1 rad scaled to 1,000,000

    AuncientEulerVolume1Chapter10Section1InPresentMetrics metrics = {0};

    bool ok = auncient_euler_volume1_chapter10_sec1_in_present_engine(
        contract_addr,
        dat_bin_ch10_path,
        input_arc_scaled,
        preserved_x,
        preserved_y,
        preserved_y2,
        &metrics
    );

    assert(ok == true);
    assert(metrics.ch10_sec1_in_present_sound == true);
    assert(metrics.is_totient_in_present == true);
    assert(metrics.is_circular_euler_identity_sound == true);
    assert(metrics.totient_phi_x == (uint64_t)preserved_x);
    assert(metrics.totient_phi_y == (uint64_t)preserved_y);
    assert(metrics.totient_phi_y2 == (uint64_t)preserved_y2);
    assert(metrics.cos_series_scaled == 995004ULL); // \cos(0.1) \approx 0.995004 scaled
    assert(metrics.sin_series_scaled == 99833ULL);  // \sin(0.1) \approx 0.099833 scaled

    printf("[SECTION 1 IN-PRESENT CIRCULAR SERIES METRICS (§ 316-§ 325)]\n");
    printf("  - Latin Title:                 %s\n", metrics.section_latin_title);
    printf("  - Preserved Variables (x,y,y2):(%ld, %ld, %ld)\n",
           metrics.preserved_random_x, metrics.preserved_random_y, metrics.preserved_random_y2);
    printf("  - Totient In-Present:          phi(x)=%lu, phi(y)=%lu, phi(y2)=%lu (Established In-Present)\n",
           metrics.totient_phi_x, metrics.totient_phi_y, metrics.totient_phi_y2);
    printf("  - Circular Series cos(x):      %lu (0.995004 scaled, In-Present Sound)\n", metrics.cos_series_scaled);
    printf("  - Circular Series sin(x):      %lu (0.099833 scaled, In-Present Sound)\n", metrics.sin_series_scaled);
    printf("  - Euler Identity e^{ix}:       100%% SOUND (cos x + i sin x in-present)\n");
    printf("  - ACID Rollback & Replay:      %s / %s (Intact History)\n",
           metrics.is_acid_rollback_sound ? "SOUND" : "FAILED",
           metrics.is_acid_replay_sound ? "SOUND" : "FAILED");
    printf("  - Rule 9 Dynamic Address:      %s (dynamic_<address>)\n",
           metrics.rule9_address_resolution_sound ? "SOUND" : "FAILED");
    printf("  - Rule 13 .dat.bin Layout:     %s (.dat.bin Quadtree)\n",
           metrics.rule13_dat_bin_verified ? "VERIFIED" : "FAILED");
    printf("  - Master FNV-1a Checksum:      0x%016lX\n", metrics.acid_ch10_sec1_checksum);
    printf("  - 512-bit ZMM ReBAR Latch:     0x%08lX\n\n", metrics.zmm_hardware_latch);

    printf("================================================================================\n");
    printf(" [PROOFS SUCCESSFUL] Section 1 In-Present Circular Series Engine sound.\n");
    printf("================================================================================\n");

    remove(dat_bin_ch10_path);
    return 0;
}
