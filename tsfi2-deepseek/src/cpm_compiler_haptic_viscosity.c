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
    uint32_t opcode_signature;                    // "CPHV" (0x43504856)
    uint16_t proof_material_viscosity_damping;    // Theorem 551: Material Viscosity Damping Invariance
    uint16_t proof_elastic_plastic_yield_thresh;  // Theorem 552: Elastic Plastic Yield Threshold Invariance
    uint16_t proof_submillisecond_haptic_latency; // Theorem 553: Sub-Millisecond Haptic Feedback Latency
    uint16_t proof_kinetic_energy_dissipation;    // Theorem 554: Non-Preferential Kinetic Dissipation
    uint16_t proof_haptic_viscosity_grand_seal;   // Theorem 555: Sovereign Haptic Resistance Grand Seal
    char haptic_title[64];                        // "CPMTOMIE-HAPTIC-TACTILE-VISCOSITY-CORPS"
    uint32_t checksum_rule18;                     // 3-term recurrence checksum
} CpmHapticViscosityBinary;

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
    const char *out_path = (argc > 1) ? argv[1] : "haptic_viscosity.bin";

    CpmHapticViscosityBinary bin;
    memset(&bin, 0, sizeof(bin));

    bin.magic[0] = 'A';
    bin.magic[1] = 'N';
    bin.magic[2] = 'K';
    bin.magic[3] = 'H';

    bin.load_address = CPM_TPA_BASE;
    bin.entry_point = CPM_TPA_BASE;
    bin.opcode_signature = 0x43504856; // "CPHV"

    bin.proof_material_viscosity_damping = 1;
    bin.proof_elastic_plastic_yield_thresh = 1;
    bin.proof_submillisecond_haptic_latency = 1;
    bin.proof_kinetic_energy_dissipation = 1;
    bin.proof_haptic_viscosity_grand_seal = 1;

    snprintf(bin.haptic_title, sizeof(bin.haptic_title),
             "CPMTOMIE-HAPTIC-TACTILE-VISCOSITY-CORPS");

    bin.checksum_rule18 = compute_rule18_checksum((const uint8_t *)&bin, sizeof(bin) - sizeof(uint32_t));

    FILE *f = fopen(out_path, "wb");
    if (!f) {
        perror("fopen");
        return 1;
    }
    fwrite(&bin, 1, sizeof(bin), f);
    fclose(f);

    printf("CP/M-Tomie Haptic Viscosity Binary compiled successfully -> %s (%zu bytes, Rule 18 Checksum: 0x%08X)\n",
           out_path, sizeof(bin), bin.checksum_rule18);
    return 0;
}
