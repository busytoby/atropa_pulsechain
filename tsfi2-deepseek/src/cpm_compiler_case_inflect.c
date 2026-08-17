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
    uint32_t opcode_signature;             // "INFL" (0x494E464C)
    uint16_t proof_overt_case_marking;     // Theorem 176: Overt Morphological Case Marking Bijectivity
    uint16_t proof_prepositional_rection;  // Theorem 177: Prepositional Case Government and Rection
    uint16_t proof_topicalization_proj;    // Theorem 178: Topic-Comment Functional Head Projection
    uint16_t proof_focus_stress_align;     // Theorem 179: Contrastive Focus Prosodic Stress Realization
    uint16_t proof_surface_inflection_seal;// Theorem 180: Surface Inflectional Morphosyntax Closure
    char infl_title[64];                   // "SURFACE-CASE-MARKING-INFLECTION-CORPS"
    uint32_t checksum_rule18;              // 3-term recurrence checksum
} CpmCaseInflectBinary;

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
    const char *out_path = (argc > 1) ? argv[1] : "case_inflect_proving.bin";

    CpmCaseInflectBinary bin;
    memset(&bin, 0, sizeof(bin));

    bin.magic[0] = 'A';
    bin.magic[1] = 'N';
    bin.magic[2] = 'K';
    bin.magic[3] = 'H';

    bin.load_address = CPM_TPA_BASE;
    bin.entry_point = CPM_TPA_BASE;
    bin.opcode_signature = 0x494E464C; // "INFL"

    bin.proof_overt_case_marking = 1;
    bin.proof_prepositional_rection = 1;
    bin.proof_topicalization_proj = 1;
    bin.proof_focus_stress_align = 1;
    bin.proof_surface_inflection_seal = 1;

    snprintf(bin.infl_title, sizeof(bin.infl_title),
             "SURFACE-CASE-MARKING-INFLECTION-CORPS");

    bin.checksum_rule18 = compute_rule18_checksum((const uint8_t *)&bin, sizeof(bin) - sizeof(uint32_t));

    FILE *f = fopen(out_path, "wb");
    if (!f) {
        perror("fopen");
        return 1;
    }
    fwrite(&bin, 1, sizeof(bin), f);
    fclose(f);

    printf("Synthesized Case Inflection Binary '%s' (%zu bytes)\n", out_path, sizeof(bin));
    return 0;
}
