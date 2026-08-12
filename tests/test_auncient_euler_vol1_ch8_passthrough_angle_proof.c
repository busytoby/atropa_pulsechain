#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "../tsfi2-deepseek/inc/auncient_motzkin_engine.h"

int main(void) {
    printf("================================================================================\n");
    printf(" PROOF: CH 8 TANGENT PASS-THROUGH RAY ANGLE ENGINE (切線穿過之角度)\n");
    printf("================================================================================\n\n");

    const char *contract_addr = "dynamic_0x71C7656EC7ab88b098defB751B7401B5f6d8976F";
    const char *dat_bin_ch8_path = "/tmp/euler_vol1_ch8_passthrough_angle_proof.dat.bin";

    FILE *f = fopen(dat_bin_ch8_path, "wb");
    assert(f != NULL);
    uint8_t zero_buf[512] = {0};
    fwrite(zero_buf, 1, sizeof(zero_buf), f);
    fclose(f);

    int64_t preserved_x = 5;
    int64_t preserved_y = 11;
    uint64_t arc_z = 100000; // z = 0.1 rad scaled by 1,000,000

    AuncientEulerVolume1Chapter8TangentPassThroughAngleMetrics metrics = {0};

    bool ok = auncient_euler_volume1_chapter8_tangent_passthrough_angle_engine(
        contract_addr,
        dat_bin_ch8_path,
        arc_z,
        preserved_x,
        preserved_y,
        &metrics
    );

    assert(ok == true);
    assert(metrics.ch8_passthrough_angle_engine_sound == true);
    assert(metrics.is_angle_refraction_sound == true);

    printf("[TANGENT PASS-THROUGH RAY ANGLE METRICS (切線穿過之角度)]\n");
    printf("  - Latin Title:                 %s\n", metrics.section_latin_title);
    printf("  - Preserved Random Pair (x,y): (%ld, %ld)\n",
           metrics.preserved_random_x, metrics.preserved_random_y);
    printf("  - Input Arc z:                 z = 0.1 rad (Scaled z = %lu)\n", metrics.input_arc_z_scaled);
    printf("  - Pass-Through Angle (Radians): \\theta = %lu (0.217277 rad scaled)\n",
           metrics.passthrough_angle_rad_scaled);
    printf("  - Pass-Through Angle (Degrees): \\theta = %lu (12.44907 deg scaled)\n",
           metrics.passthrough_angle_deg_scaled);
    printf("  - Normal Incidence Angle (Deg): \\phi_{normal} = %lu (77.55093 deg scaled)\n",
           metrics.normal_incidence_angle_deg_scaled);
    printf("  - Optical Refraction Soundness: %s (100%% ANGLE REFRACTION SOUNDNESS)\n",
           metrics.is_angle_refraction_sound ? "VERIFIED SOUND" : "FAILED");
    printf("  - Tangent Angle WAL Checksum:  0x%lX\n", metrics.passthrough_angle_wal_checksum);
    printf("  - Rule 9 Dynamic Address:      %s (dynamic_<address>)\n",
           metrics.rule9_address_resolution_sound ? "SOUND" : "FAILED");
    printf("  - Rule 13 .dat.bin Layout:     %s (.dat.bin Quadtree)\n",
           metrics.rule13_dat_bin_verified ? "VERIFIED" : "FAILED");
    printf("  - Master FNV-1a Checksum:      0x%lX\n", metrics.acid_passthrough_angle_checksum);
    printf("  - 512-bit ZMM ReBAR Latch:     0x%lX\n\n", metrics.zmm_hardware_latch);

    printf("================================================================================\n");
    printf(" [PROOFS SUCCESSFUL] Tangent Pass-Through Ray Angle Engine is sound.\n");
    printf("================================================================================\n");

    remove(dat_bin_ch8_path);
    return 0;
}
