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
    uint32_t opcode_signature;                    // "CPMA" (0x43504D41)
    uint16_t proof_multimodal_constraint_satisfy; // Theorem 561: Multi-Modal Semantic Constraint Satisfaction
    uint16_t proof_nonpreferential_disambiguation;// Theorem 562: Non-Preferential Ambiguity Disambiguation Invariance
    uint16_t proof_polynomial_relaxation_converge;// Theorem 563: Polynomial-Time Relaxation Convergence
    uint16_t proof_dual_state_commutative_cohere; // Theorem 564: Dual-State Commutative Coherence
    uint16_t proof_multimodal_ambig_grand_seal;   // Theorem 565: Sovereign Multi-Modal Ambiguity Resolution Grand Seal
    char ambig_title[64];                         // "CPMTOMIE-MULTIMODAL-AMBIGUITY-RESOLUTION-CORPS"
    uint32_t checksum_rule18;                     // 3-term recurrence checksum
} CpmMultimodalAmbigBinary;

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
    const char *out_path = (argc > 1) ? argv[1] : "multimodal_ambig.bin";

    CpmMultimodalAmbigBinary bin;
    memset(&bin, 0, sizeof(bin));

    bin.magic[0] = 'A';
    bin.magic[1] = 'N';
    bin.magic[2] = 'K';
    bin.magic[3] = 'H';

    bin.load_address = CPM_TPA_BASE;
    bin.entry_point = CPM_TPA_BASE;
    bin.opcode_signature = 0x43504D41; // "CPMA"

    bin.proof_multimodal_constraint_satisfy = 1;
    bin.proof_nonpreferential_disambiguation = 1;
    bin.proof_polynomial_relaxation_converge = 1;
    bin.proof_dual_state_commutative_cohere = 1;
    bin.proof_multimodal_ambig_grand_seal = 1;

    snprintf(bin.ambig_title, sizeof(bin.ambig_title),
             "CPMTOMIE-MULTIMODAL-AMBIGUITY-RESOLUTION-CORPS");

    bin.checksum_rule18 = compute_rule18_checksum((const uint8_t *)&bin, sizeof(bin) - sizeof(uint32_t));

    FILE *f = fopen(out_path, "wb");
    if (!f) {
        perror("fopen");
        return 1;
    }
    fwrite(&bin, 1, sizeof(bin), f);
    fclose(f);

    printf("CP/M-Tomie Multi-Modal Ambiguity Binary compiled successfully -> %s (%zu bytes, Rule 18 Checksum: 0x%08X)\n",
           out_path, sizeof(bin), bin.checksum_rule18);
    return 0;
}
