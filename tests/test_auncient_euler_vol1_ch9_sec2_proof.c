#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "../tsfi2-deepseek/inc/auncient_motzkin_engine.h"

int main(void) {
    printf("================================================================================\n");
    printf(" PROOF: CH 9 SEC 2 NON-TRANSCENDENTAL DISCRETE PARTITION ENGINE\n");
    printf("================================================================================\n\n");

    const char *contract_addr = "dynamic_0x71C7656EC7ab88b098defB751B7401B5f6d8976F";
    const char *dat_bin_ch9_path = "/tmp/euler_vol1_ch9_sec2_partition_proof.dat.bin";

    FILE *f = fopen(dat_bin_ch9_path, "wb");
    assert(f != NULL);
    uint8_t zero_buf[512] = {0};
    fwrite(zero_buf, 1, sizeof(zero_buf), f);
    fclose(f);

    int64_t preserved_x = 5;
    int64_t preserved_y = 11;

    AuncientEulerVolume1Chapter9Section2DiscretePartitionMetrics sec2_metrics = {0};

    bool ok = auncient_euler_volume1_chapter9_sec2_discrete_partition_engine(
        contract_addr,
        dat_bin_ch9_path,
        preserved_x,
        preserved_y,
        &sec2_metrics
    );

    assert(ok == true);
    assert(sec2_metrics.ch9_sec2_discrete_partition_sound == true);
    assert(sec2_metrics.is_partition_negation_asserted == true);
    assert(sec2_metrics.is_non_transcendental_discrete_sound == true);
    assert(sec2_metrics.discrete_partition_sum == 16ULL);          // P = 5 + 11 = 16
    assert(sec2_metrics.ibis_active_node_partition == 7ULL);       // I_a = (5 * 11) % 16 = 7
    assert(sec2_metrics.sedis_passive_node_partition == 2ULL);      // S_p = (25 + 121) % 16 = 2

    printf("[SECTION 2 DISCRETE PARTITION METRICS (§ 296-§ 305)]\n");
    printf("  - Latin Title:                 %s\n", sec2_metrics.section_latin_title);
    printf("  - Preserved Random Pair (x,y): (%ld, %ld)\n",
           sec2_metrics.preserved_random_x, sec2_metrics.preserved_random_y);
    printf("  - Discrete Partition Sum P:    P = x + y = %lu (Non-Transcendental Discrete Integer)\n",
           sec2_metrics.discrete_partition_sum);
    printf("  - Active Ibis Node I_a:        I_a = (x * y) mod P = %lu (Ibis Active Partition Node)\n",
           sec2_metrics.ibis_active_node_partition);
    printf("  - Passive Sedis Node S_p:      S_p = (x^2 + y^2) mod P = %lu (Sedis Passive Partition Node)\n",
           sec2_metrics.sedis_passive_node_partition);
    printf("  - Discrete Soundness:          %s (No Continuum Transcendental Decay)\n",
           sec2_metrics.is_non_transcendental_discrete_sound ? "100% VERIFIED SOUND" : "FAILED");
    printf("  - Discrete Partition WAL:     0x%lX\n", sec2_metrics.ch9_sec2_wal_checksum);
    printf("  - Rule 9 Dynamic Address:      %s (dynamic_<address>)\n",
           sec2_metrics.rule9_address_resolution_sound ? "SOUND" : "FAILED");
    printf("  - Rule 13 .dat.bin Layout:     %s (.dat.bin Quadtree)\n",
           sec2_metrics.rule13_dat_bin_verified ? "VERIFIED" : "FAILED");
    printf("  - Master FNV-1a Checksum:      0x%lX\n", sec2_metrics.acid_ch9_sec2_checksum);
    printf("  - 512-bit ZMM ReBAR Latch:     0x%lX\n\n", sec2_metrics.zmm_hardware_latch);

    printf("================================================================================\n");
    printf(" [PROOFS SUCCESSFUL] Section 2 Discrete Partition Engine sound.\n");
    printf("================================================================================\n");

    remove(dat_bin_ch9_path);
    return 0;
}
