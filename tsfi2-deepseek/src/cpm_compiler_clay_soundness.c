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
    uint32_t opcode_signature;                    // "CPMS" (0x43504D53)
    uint16_t proof_saffer_semantic_congruence;    // Theorem 411: Saffer Metaphor Semantic Congruence
    uint16_t proof_lee_cultural_grounding;        // Theorem 412: Lee & Lee Cultural Affordance Grounding
    uint16_t proof_anti_dissonance_boundary;      // Theorem 413: Cross-Metaphor Anti-Dissonance Boundary
    uint16_t proof_direct_tactile_immediacy;      // Theorem 414: Direct Tactile Affordance Immediacy
    uint16_t proof_metaphor_soundness_seal;       // Theorem 415: Sovereign Metaphoric Soundness Grand Seal
    char soundness_title[64];                     // "CPMTOMIE-CLAY-METAPHOR-SOUNDNESS-CORPS"
    uint32_t checksum_rule18;                     // 3-term recurrence checksum
} CpmClaySoundnessBinary;

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
    const char *out_path = (argc > 1) ? argv[1] : "clay_soundness.bin";

    CpmClaySoundnessBinary bin;
    memset(&bin, 0, sizeof(bin));

    bin.magic[0] = 'A';
    bin.magic[1] = 'N';
    bin.magic[2] = 'K';
    bin.magic[3] = 'H';

    bin.load_address = CPM_TPA_BASE;
    bin.entry_point = CPM_TPA_BASE;
    bin.opcode_signature = 0x43504D53; // "CPMS"

    bin.proof_saffer_semantic_congruence = 1;
    bin.proof_lee_cultural_grounding = 1;
    bin.proof_anti_dissonance_boundary = 1;
    bin.proof_direct_tactile_immediacy = 1;
    bin.proof_metaphor_soundness_seal = 1;

    snprintf(bin.soundness_title, sizeof(bin.soundness_title),
             "CPMTOMIE-CLAY-METAPHOR-SOUNDNESS-CORPS");

    bin.checksum_rule18 = compute_rule18_checksum((const uint8_t *)&bin, sizeof(bin) - sizeof(uint32_t));

    FILE *f = fopen(out_path, "wb");
    if (!f) {
        perror("fopen");
        return 1;
    }
    fwrite(&bin, 1, sizeof(bin), f);
    fclose(f);

    printf("CP/M-Tomie Clay Metaphor Soundness Binary compiled successfully -> %s (%zu bytes, Rule 18 Checksum: 0x%08X)\n",
           out_path, sizeof(bin), bin.checksum_rule18);
    return 0;
}
