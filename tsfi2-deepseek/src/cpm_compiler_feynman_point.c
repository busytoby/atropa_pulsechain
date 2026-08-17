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
    uint32_t opcode_signature;                    // "CPFP" (0x43504650)
    uint16_t proof_feynman_point_phase_invar;     // Theorem 521: Feynman Point Hexadecimal Phase Invariance
    uint16_t proof_constant_energy_minimum;       // Theorem 522: Constant Phase-Lock Energy Minimum
    uint16_t proof_nonpreferential_orbit_trapping;// Theorem 523: Non-Preferential Orbit Trapping
    uint16_t proof_coaxial_rf_phase_coherence;    // Theorem 524: Coaxial RF Phase Coherence at Feynman Minimum
    uint16_t proof_feynman_point_grand_seal;      // Theorem 525: Sovereign Feynman Point Lyapunov Grand Seal
    char feynman_title[64];                       // "CPMTOMIE-FEYNMAN-POINT-LYAPUNOV-CORPS"
    uint32_t checksum_rule18;                     // 3-term recurrence checksum
} CpmFeynmanPointBinary;

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
    const char *out_path = (argc > 1) ? argv[1] : "feynman_point.bin";

    CpmFeynmanPointBinary bin;
    memset(&bin, 0, sizeof(bin));

    bin.magic[0] = 'A';
    bin.magic[1] = 'N';
    bin.magic[2] = 'K';
    bin.magic[3] = 'H';

    bin.load_address = CPM_TPA_BASE;
    bin.entry_point = CPM_TPA_BASE;
    bin.opcode_signature = 0x43504650; // "CPFP"

    bin.proof_feynman_point_phase_invar = 1;
    bin.proof_constant_energy_minimum = 1;
    bin.proof_nonpreferential_orbit_trapping = 1;
    bin.proof_coaxial_rf_phase_coherence = 1;
    bin.proof_feynman_point_grand_seal = 1;

    snprintf(bin.feynman_title, sizeof(bin.feynman_title),
             "CPMTOMIE-FEYNMAN-POINT-LYAPUNOV-CORPS");

    bin.checksum_rule18 = compute_rule18_checksum((const uint8_t *)&bin, sizeof(bin) - sizeof(uint32_t));

    FILE *f = fopen(out_path, "wb");
    if (!f) {
        perror("fopen");
        return 1;
    }
    fwrite(&bin, 1, sizeof(bin), f);
    fclose(f);

    printf("CP/M-Tomie Feynman Point Binary compiled successfully -> %s (%zu bytes, Rule 18 Checksum: 0x%08X)\n",
           out_path, sizeof(bin), bin.checksum_rule18);
    return 0;
}
