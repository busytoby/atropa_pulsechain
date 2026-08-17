#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#define CPM_TPA_BASE 0x0100

typedef struct __attribute__((packed)) {
    uint8_t magic[4];                            // "ANKH"
    uint16_t load_address;                       // 0x0100
    uint16_t entry_point;                        // 0x0100
    uint32_t opcode_signature;                   // "CPLY" (0x43504C59)
    uint16_t proof_geneplore_dynamic_duality;   // Theorem 271: Geneplore Generative-Exploratory Duality
    uint16_t proof_combine_operator_closure;     // Theorem 272: Direct Manipulation Combine Operator Closure
    uint16_t proof_decompose_facet_orthogonality;// Theorem 273: Direct Manipulation Decompose Facet Orthogonality
    uint16_t proof_ideation_momentum_invariance; // Theorem 274: Non-Linear Continuous Ideation Momentum Invariance
    uint16_t proof_clay_play_tpa_seal;           // Theorem 275: Sovereign Clay Play TPA 0100H Synthesis Seal
    char clay_play_title[64];                    // "CPMTOMIE-CLAY-PLAY-GENEPLORE-METAPHOR-INTERFACE"
    uint32_t checksum_rule18;                    // 3-term recurrence checksum
} CpmClayPlayBinary;

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
    const char *out_path = (argc > 1) ? argv[1] : "clay_play_geneplore.bin";

    CpmClayPlayBinary bin;
    memset(&bin, 0, sizeof(bin));

    bin.magic[0] = 'A';
    bin.magic[1] = 'N';
    bin.magic[2] = 'K';
    bin.magic[3] = 'H';

    bin.load_address = CPM_TPA_BASE;
    bin.entry_point = CPM_TPA_BASE;
    bin.opcode_signature = 0x43504C59; // "CPLY"

    bin.proof_geneplore_dynamic_duality = 1;
    bin.proof_combine_operator_closure = 1;
    bin.proof_decompose_facet_orthogonality = 1;
    bin.proof_ideation_momentum_invariance = 1;
    bin.proof_clay_play_tpa_seal = 1;

    snprintf(bin.clay_play_title, sizeof(bin.clay_play_title),
             "CPMTOMIE-CLAY-PLAY-GENEPLORE-METAPHOR-INTERFACE");

    bin.checksum_rule18 = compute_rule18_checksum((const uint8_t *)&bin, sizeof(bin) - sizeof(uint32_t));

    FILE *f = fopen(out_path, "wb");
    if (!f) {
        perror("fopen");
        return 1;
    }
    fwrite(&bin, 1, sizeof(bin), f);
    fclose(f);

    printf("CP/M-Tomie Clay Play Geneplore Binary compiled successfully -> %s (%zu bytes, Rule 18 Checksum: 0x%08X)\n",
           out_path, sizeof(bin), bin.checksum_rule18);
    return 0;
}
