#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#define CPM_TPA_BASE 0x0100

typedef struct __attribute__((packed)) {
    uint8_t magic[4];                      // "ANKH"
    uint16_t load_address;                 // 0x0100
    uint16_t entry_point;                  // 0x0100
    uint32_t opcode_signature;             // "LSEM" (0x4C53454D)
    uint16_t proof_lexical_insertion_inv;  // Theorem 156: Inherent Semantic Feature Lexical Insertion Invariance
    uint16_t proof_chomsky_standard_model; // Theorem 157: Chomsky 1965 Standard Generative Transformational Model
    uint16_t proof_experiencer_locative;   // Theorem 158: Experiencer, Patient and Locative Semantic Mapping
    uint16_t proof_surface_passivization;  // Theorem 159: Active-to-Passive Surface Transformation Invariance
    uint16_t proof_deep_case_hierarchy;    // Theorem 160: Deep Case Subjectivization Hierarchy Priority Lattice
    char ling_title[64];                   // "CHOMSKY-FILLMORE-LINGUISTIC-SEMANTICS-CORPS"
    uint32_t checksum_rule18;              // 3-term recurrence checksum
} CpmLingSemanticsBinary;

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
    const char *out_path = (argc > 1) ? argv[1] : "ling_semantics_proving.bin";

    CpmLingSemanticsBinary bin;
    memset(&bin, 0, sizeof(bin));

    bin.magic[0] = 'A';
    bin.magic[1] = 'N';
    bin.magic[2] = 'K';
    bin.magic[3] = 'H';

    bin.load_address = CPM_TPA_BASE;
    bin.entry_point = CPM_TPA_BASE;
    bin.opcode_signature = 0x4C53454D; // "LSEM"

    bin.proof_lexical_insertion_inv = 1;
    bin.proof_chomsky_standard_model = 1;
    bin.proof_experiencer_locative = 1;
    bin.proof_surface_passivization = 1;
    bin.proof_deep_case_hierarchy = 1;

    snprintf(bin.ling_title, sizeof(bin.ling_title),
             "CHOMSKY-FILLMORE-LINGUISTIC-SEMANTICS-CORPS");

    bin.checksum_rule18 = compute_rule18_checksum((const uint8_t *)&bin, sizeof(bin) - sizeof(uint32_t));

    FILE *f = fopen(out_path, "wb");
    if (!f) {
        perror("fopen");
        return 1;
    }
    fwrite(&bin, 1, sizeof(bin), f);
    fclose(f);

    printf("Synthesized Linguistic Semantics Binary '%s' (%zu bytes)\n", out_path, sizeof(bin));
    return 0;
}
