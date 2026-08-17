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
    uint32_t opcode_signature;                    // "CPPP" (0x43505050)
    uint16_t proof_dual_physicality_mapping;      // Theorem 361: Dual Physicality Mapping Invariance
    uint16_t proof_virtual_plasticity_yield;      // Theorem 362: Virtual Plasticity Non-Newtonian Yield Stress
    uint16_t proof_viscoelastic_relaxation_time;  // Theorem 363: Viscoelastic Stress Relaxation Time Bounds
    uint16_t proof_cohesive_surface_tension;      // Theorem 364: Cohesive Boundary Surface Tension Invariance
    uint16_t proof_clay_physicality_seal;         // Theorem 365: Sovereign Physical-Virtual Clay Duality Grand Seal
    char clay_phys_title[64];                     // "CPMTOMIE-CLAY-PHYSICALITY-PLASTICITY-CORPS"
    uint32_t checksum_rule18;                     // 3-term recurrence checksum
} CpmClayPhysBinary;

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
    const char *out_path = (argc > 1) ? argv[1] : "clay_physicality.bin";

    CpmClayPhysBinary bin;
    memset(&bin, 0, sizeof(bin));

    bin.magic[0] = 'A';
    bin.magic[1] = 'N';
    bin.magic[2] = 'K';
    bin.magic[3] = 'H';

    bin.load_address = CPM_TPA_BASE;
    bin.entry_point = CPM_TPA_BASE;
    bin.opcode_signature = 0x43505050; // "CPPP"

    bin.proof_dual_physicality_mapping = 1;
    bin.proof_virtual_plasticity_yield = 1;
    bin.proof_viscoelastic_relaxation_time = 1;
    bin.proof_cohesive_surface_tension = 1;
    bin.proof_clay_physicality_seal = 1;

    snprintf(bin.clay_phys_title, sizeof(bin.clay_phys_title),
             "CPMTOMIE-CLAY-PHYSICALITY-PLASTICITY-CORPS");

    bin.checksum_rule18 = compute_rule18_checksum((const uint8_t *)&bin, sizeof(bin) - sizeof(uint32_t));

    FILE *f = fopen(out_path, "wb");
    if (!f) {
        perror("fopen");
        return 1;
    }
    fwrite(&bin, 1, sizeof(bin), f);
    fclose(f);

    printf("CP/M-Tomie Clay Physicality Binary compiled successfully -> %s (%zu bytes, Rule 18 Checksum: 0x%08X)\n",
           out_path, sizeof(bin), bin.checksum_rule18);
    return 0;
}
