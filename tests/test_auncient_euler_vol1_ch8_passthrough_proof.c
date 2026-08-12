#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "../tsfi2-deepseek/inc/auncient_motzkin_engine.h"

int main(void) {
    printf("================================================================================\n");
    printf(" PROOF: CH 8 TANGENT PASS-THROUGH RAY PERSPECTIVE ENGINE\n");
    printf("================================================================================\n\n");

    const char *contract_addr = "dynamic_0x71C7656EC7ab88b098defB751B7401B5f6d8976F";
    const char *dat_bin_ch8_path = "/tmp/euler_vol1_ch8_passthrough_proof.dat.bin";

    FILE *f = fopen(dat_bin_ch8_path, "wb");
    assert(f != NULL);
    uint8_t zero_buf[512] = {0};
    fwrite(zero_buf, 1, sizeof(zero_buf), f);
    fclose(f);

    int64_t preserved_x = 5;
    int64_t preserved_y = 11;
    uint64_t arc_z = 100000; // z = 0.1 rad scaled by 1,000,000

    AuncientEulerVolume1Chapter8TangentPassThroughRayMetrics metrics = {0};

    bool ok = auncient_euler_volume1_chapter8_tangent_passthrough_ray_engine(
        contract_addr,
        dat_bin_ch8_path,
        arc_z,
        preserved_x,
        preserved_y,
        &metrics
    );

    assert(ok == true);
    assert(metrics.ch8_passthrough_engine_sound == true);
    assert(metrics.is_passthrough_ray_continuity_sound == true);

    printf("[TANGENT PASS-THROUGH RAY PERSPECTIVE METRICS]\n");
    printf("  - Latin Title:                 %s\n", metrics.section_latin_title);
    printf("  - Preserved Random Pair (x,y): (%ld, %ld)\n",
           metrics.preserved_random_x, metrics.preserved_random_y);
    printf("  - Input Arc z:                 z = 0.1 rad (Scaled z = %lu)\n", metrics.input_arc_z_scaled);
    printf("  - Tangent Entry Vector E:      E = X - \\tan \\theta \\cdot Y = %lu (Scaled)\n",
           metrics.passthrough_entry_vector_scaled);
    printf("  - Tangent Exit Vector L:       L = X + \\tan \\theta \\cdot Y = %lu (Scaled)\n",
           metrics.passthrough_exit_vector_scaled);
    printf("  - Conic Ray Throughput I:      I = \\sqrt{E \\cdot L} = %lu (Scaled Ray Intensity)\n",
           metrics.passthrough_ray_intensity_scaled);
    printf("  - Pass-Through Continuity:     %s (100%% OPTICAL CONTINUITY SOUNDNESS)\n",
           metrics.is_passthrough_ray_continuity_sound ? "VERIFIED SOUND" : "FAILED");
    printf("  - Tangent Pass-Through WAL:    0x%lX\n", metrics.passthrough_wal_checksum);
    printf("  - Rule 9 Dynamic Address:      %s (dynamic_<address>)\n",
           metrics.rule9_address_resolution_sound ? "SOUND" : "FAILED");
    printf("  - Rule 13 .dat.bin Layout:     %s (.dat.bin Quadtree)\n",
           metrics.rule13_dat_bin_verified ? "VERIFIED" : "FAILED");
    printf("  - Master FNV-1a Checksum:      0x%lX\n", metrics.acid_passthrough_checksum);
    printf("  - 512-bit ZMM ReBAR Latch:     0x%lX\n\n", metrics.zmm_hardware_latch);

    printf("================================================================================\n");
    printf(" [PROOFS SUCCESSFUL] Tangent Pass-Through Ray Perspective Engine is sound.\n");
    printf("================================================================================\n");

    remove(dat_bin_ch8_path);
    return 0;
}
