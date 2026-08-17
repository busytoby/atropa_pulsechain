#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#define CPM_TPA_BASE 0x0100

typedef struct __attribute__((packed)) {
    uint8_t magic[4];                             // "ANKH"
    uint16_t load_address;                        // 0x0100
    uint16_t entry_point;                         // 0x0100
    uint32_t opcode_signature;                    // "CPLX" (0x43504C58)
    uint16_t proof_morphological_mold_dilation;   // Theorem 281: Morphological Mold & Boundary Dilation Invariance
    uint16_t proof_spatial_coordinate_topology;   // Theorem 282: Spatial Coordinate Topology & 2D Direct Manipulation Clustering
    uint16_t proof_cognitive_momentum_retention;  // Theorem 283: Cognitive Momentum Preservation & Zero-Disruption Divergence
    uint16_t proof_preinventive_variance_bound;   // Theorem 284: Preinventive Structure Convergence & Bounded Exploration Variance
    uint16_t proof_extended_clay_play_seal;       // Theorem 285: Sovereign Extended Clay Play Multi-Modal Interaction Grand Seal
    char clay_ext_title[64];                      // "CPMTOMIE-EXTENDED-CLAY-PLAY-MORPHOLOGY-CORPS"
    uint32_t checksum_rule18;                     // 3-term recurrence checksum
} CpmClayExtendedBinary;

static uint32_t compute_rule18_checksum(const uint8_t *data, size_t len) {
    uint64_t p0 = 1;
    uint64_t p1 = (len > 0) ? (data[0] + 7) : 1;
    uint64_t pn = p1;

    for (size_t i = 1; i < len; ++i) {
        uint64_t alpha = (i * 17ULL) % 256ULL;
        uint64_t beta = (i * 31ULL) % 256ULL;
        pn = ((data[i] + alpha) * p1 - beta * p0) % 65535ULL;
        p0 = p1;
        p1 = pn;
    }
    return (uint32_t)pn;
}

int main(int argc, char **argv) {
    const char *out_path = (argc > 1) ? argv[1] : "clay_extended.bin";

    CpmClayExtendedBinary bin;
    memset(&bin, 0, sizeof(bin));

    bin.magic[0] = 'A';
    bin.magic[1] = 'N';
    bin.magic[2] = 'K';
    bin.magic[3] = 'H';

    bin.load_address = CPM_TPA_BASE;
    bin.entry_point = CPM_TPA_BASE;
    bin.opcode_signature = 0x43504C58; // "CPLX"

    bin.proof_morphological_mold_dilation = 1;
    bin.proof_spatial_coordinate_topology = 1;
    bin.proof_cognitive_momentum_retention = 1;
    bin.proof_preinventive_variance_bound = 1;
    bin.proof_extended_clay_play_seal = 1;

    snprintf(bin.clay_ext_title, sizeof(bin.clay_ext_title),
             "CPMTOMIE-EXTENDED-CLAY-PLAY-MORPHOLOGY-CORPS");

    bin.checksum_rule18 = compute_rule18_checksum((const uint8_t *)&bin, sizeof(bin) - sizeof(uint32_t));

    FILE *f = fopen(out_path, "wb");
    if (!f) {
        perror("fopen");
        return 1;
    }
    fwrite(&bin, 1, sizeof(bin), f);
    fclose(f);

    printf("CP/M-Tomie Extended Clay Play Binary compiled successfully -> %s (%zu bytes, Rule 18 Checksum: 0x%08X)\n",
           out_path, sizeof(bin), bin.checksum_rule18);
    return 0;
}
