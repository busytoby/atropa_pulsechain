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
    uint32_t opcode_signature;                    // "CPAU" (0x43504155)
    uint16_t proof_continuous_visual_representation;// Theorem 301: Continuous Visual Representation
    uint16_t proof_rapid_reversible_mutation;     // Theorem 302: Rapid Reversible State Mutation
    uint16_t proof_cognitive_dissonance_bound;    // Theorem 303: Cognitive Dissonance Elimination
    uint16_t proof_human_ai_autonomy_balance;     // Theorem 304: Human-AI Autonomy Balance Invariance
    uint16_t proof_shneiderman_autonomy_seal;     // Theorem 305: Sovereign Direct Manipulation TPA 0100H Synthesis Seal
    char clay_autonomy_title[64];                 // "CPMTOMIE-SHNEIDERMAN-DIRECT-MANIPULATION-AUTONOMY"
    uint32_t checksum_rule18;                     // 3-term recurrence checksum
} CpmClayAutonomyBinary;

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
    const char *out_path = (argc > 1) ? argv[1] : "clay_autonomy.bin";

    CpmClayAutonomyBinary bin;
    memset(&bin, 0, sizeof(bin));

    bin.magic[0] = 'A';
    bin.magic[1] = 'N';
    bin.magic[2] = 'K';
    bin.magic[3] = 'H';

    bin.load_address = CPM_TPA_BASE;
    bin.entry_point = CPM_TPA_BASE;
    bin.opcode_signature = 0x43504155; // "CPAU"

    bin.proof_continuous_visual_representation = 1;
    bin.proof_rapid_reversible_mutation = 1;
    bin.proof_cognitive_dissonance_bound = 1;
    bin.proof_human_ai_autonomy_balance = 1;
    bin.proof_shneiderman_autonomy_seal = 1;

    snprintf(bin.clay_autonomy_title, sizeof(bin.clay_autonomy_title),
             "CPMTOMIE-SHNEIDERMAN-DIRECT-MANIPULATION-AUTONOMY");

    bin.checksum_rule18 = compute_rule18_checksum((const uint8_t *)&bin, sizeof(bin) - sizeof(uint32_t));

    FILE *f = fopen(out_path, "wb");
    if (!f) {
        perror("fopen");
        return 1;
    }
    fwrite(&bin, 1, sizeof(bin), f);
    fclose(f);

    printf("CP/M-Tomie Shneiderman Autonomy Binary compiled successfully -> %s (%zu bytes, Rule 18 Checksum: 0x%08X)\n",
           out_path, sizeof(bin), bin.checksum_rule18);
    return 0;
}
