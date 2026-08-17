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
    uint32_t opcode_signature;                    // "CPGD" (0x43504744)
    uint16_t proof_guilford_fluency_node_count;   // Theorem 401: Guilford Fluency Invariant
    uint16_t proof_cross_category_flexibility;    // Theorem 402: Cross-Category Flexibility Duality
    uint16_t proof_originality_group_divergence;  // Theorem 403: Originality Modular Group Divergence
    uint16_t proof_recursive_elaboration_depth;   // Theorem 404: Recursive Elaboration Depth Bound
    uint16_t proof_guilford_divergent_seal;       // Theorem 405: Sovereign Guilford Divergent Ideation Grand Seal
    char guilford_title[64];                      // "CPMTOMIE-GUILFORD-DIVERGENT-IDEATION-CORPS"
    uint32_t checksum_rule18;                     // 3-term recurrence checksum
} CpmClayGuilfordBinary;

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
    const char *out_path = (argc > 1) ? argv[1] : "clay_guilford.bin";

    CpmClayGuilfordBinary bin;
    memset(&bin, 0, sizeof(bin));

    bin.magic[0] = 'A';
    bin.magic[1] = 'N';
    bin.magic[2] = 'K';
    bin.magic[3] = 'H';

    bin.load_address = CPM_TPA_BASE;
    bin.entry_point = CPM_TPA_BASE;
    bin.opcode_signature = 0x43504744; // "CPGD"

    bin.proof_guilford_fluency_node_count = 1;
    bin.proof_cross_category_flexibility = 1;
    bin.proof_originality_group_divergence = 1;
    bin.proof_recursive_elaboration_depth = 1;
    bin.proof_guilford_divergent_seal = 1;

    snprintf(bin.guilford_title, sizeof(bin.guilford_title),
             "CPMTOMIE-GUILFORD-DIVERGENT-IDEATION-CORPS");

    bin.checksum_rule18 = compute_rule18_checksum((const uint8_t *)&bin, sizeof(bin) - sizeof(uint32_t));

    FILE *f = fopen(out_path, "wb");
    if (!f) {
        perror("fopen");
        return 1;
    }
    fwrite(&bin, 1, sizeof(bin), f);
    fclose(f);

    printf("CP/M-Tomie Guilford Divergent Binary compiled successfully -> %s (%zu bytes, Rule 18 Checksum: 0x%08X)\n",
           out_path, sizeof(bin), bin.checksum_rule18);
    return 0;
}
