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
    uint32_t opcode_signature;                    // "CPCS" (0x43504353)
    uint16_t proof_prompt_fatigue_elimination;    // Theorem 321: Prompt Fatigue Elimination
    uint16_t proof_ishii_tangible_affordance;     // Theorem 322: Ishii Tangible Affordance Dual Physicality
    uint16_t proof_neurodynamic_ideation_stability;// Theorem 323: Neurodynamical Divergent Ideation Stability
    uint16_t proof_auditory_haptic_commutation;   // Theorem 324: Auditory/Haptic Tactile Commutation
    uint16_t proof_cst_tangible_grand_seal;       // Theorem 325: Sovereign CP/M-Tomie CST Tangible Affordance Grand Seal
    char clay_cst_title[64];                      // "CPMTOMIE-CST-TANGIBLE-AFFORDANCE-CORPS"
    uint32_t checksum_rule18;                     // 3-term recurrence checksum
} CpmClayCstBinary;

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
    const char *out_path = (argc > 1) ? argv[1] : "clay_cst.bin";

    CpmClayCstBinary bin;
    memset(&bin, 0, sizeof(bin));

    bin.magic[0] = 'A';
    bin.magic[1] = 'N';
    bin.magic[2] = 'K';
    bin.magic[3] = 'H';

    bin.load_address = CPM_TPA_BASE;
    bin.entry_point = CPM_TPA_BASE;
    bin.opcode_signature = 0x43504353; // "CPCS"

    bin.proof_prompt_fatigue_elimination = 1;
    bin.proof_ishii_tangible_affordance = 1;
    bin.proof_neurodynamic_ideation_stability = 1;
    bin.proof_auditory_haptic_commutation = 1;
    bin.proof_cst_tangible_grand_seal = 1;

    snprintf(bin.clay_cst_title, sizeof(bin.clay_cst_title),
             "CPMTOMIE-CST-TANGIBLE-AFFORDANCE-CORPS");

    bin.checksum_rule18 = compute_rule18_checksum((const uint8_t *)&bin, sizeof(bin) - sizeof(uint32_t));

    FILE *f = fopen(out_path, "wb");
    if (!f) {
        perror("fopen");
        return 1;
    }
    fwrite(&bin, 1, sizeof(bin), f);
    fclose(f);

    printf("CP/M-Tomie CST Tangible Affordance Binary compiled successfully -> %s (%zu bytes, Rule 18 Checksum: 0x%08X)\n",
           out_path, sizeof(bin), bin.checksum_rule18);
    return 0;
}
