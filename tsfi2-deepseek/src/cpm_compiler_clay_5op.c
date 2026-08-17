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
    uint32_t opcode_signature;                    // "CP5O" (0x4350354F)
    uint16_t proof_two_layer_abstraction_duality;// Theorem 291: Two-Layer Clay Abstraction Duality
    uint16_t proof_schumpeter_combine_closure;   // Theorem 292: Schumpeter Combine Operator Closure
    uint16_t proof_constituent_decompose_tearing;// Theorem 293: Constituent Decompose Tearing Invariance
    uint16_t proof_reduce_replace_entropy_bound; // Theorem 294: Canvas Entropy Bounding via Reduce & Replace
    uint16_t proof_5op_clay_play_seal;           // Theorem 295: Sovereign 5-Operator Clay Play TPA 0100H Synthesis Grand Seal
    char clay_5op_title[64];                     // "CPMTOMIE-5OPERATOR-2LAYER-CLAY-PLAY-SUITE"
    uint32_t checksum_rule18;                    // 3-term recurrence checksum
} CpmClay5OpBinary;

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
    const char *out_path = (argc > 1) ? argv[1] : "clay_5op.bin";

    CpmClay5OpBinary bin;
    memset(&bin, 0, sizeof(bin));

    bin.magic[0] = 'A';
    bin.magic[1] = 'N';
    bin.magic[2] = 'K';
    bin.magic[3] = 'H';

    bin.load_address = CPM_TPA_BASE;
    bin.entry_point = CPM_TPA_BASE;
    bin.opcode_signature = 0x4350354F; // "CP5O"

    bin.proof_two_layer_abstraction_duality = 1;
    bin.proof_schumpeter_combine_closure = 1;
    bin.proof_constituent_decompose_tearing = 1;
    bin.proof_reduce_replace_entropy_bound = 1;
    bin.proof_5op_clay_play_seal = 1;

    snprintf(bin.clay_5op_title, sizeof(bin.clay_5op_title),
             "CPMTOMIE-5OPERATOR-2LAYER-CLAY-PLAY-SUITE");

    bin.checksum_rule18 = compute_rule18_checksum((const uint8_t *)&bin, sizeof(bin) - sizeof(uint32_t));

    FILE *f = fopen(out_path, "wb");
    if (!f) {
        perror("fopen");
        return 1;
    }
    fwrite(&bin, 1, sizeof(bin), f);
    fclose(f);

    printf("CP/M-Tomie 5-Operator 2-Layer Clay Play Binary compiled successfully -> %s (%zu bytes, Rule 18 Checksum: 0x%08X)\n",
           out_path, sizeof(bin), bin.checksum_rule18);
    return 0;
}
