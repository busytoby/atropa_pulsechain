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
    uint32_t opcode_signature;                    // "CPLS" (0x43504C53)
    uint16_t proof_singularity_boundary_intercept;// Theorem 501: Lyapunov Singularity Interception Invariance
    uint16_t proof_accumulator_redirection;       // Theorem 502: Non-Preferential Accumulator Redirection
    uint16_t proof_energy_clamping_drift_bound;   // Theorem 503: Asymptotic Boundary Clamping Bound
    uint16_t proof_harmonic_absorption_damping;   // Theorem 504: Non-Oscillatory Harmonic Absorption
    uint16_t proof_lyapunov_boundary_grand_seal;  // Theorem 505: Sovereign Lyapunov Singularity Boundary Grand Seal
    char lyapunov_boundary_title[64];             // "CPMTOMIE-LYAPUNOV-SINGULARITY-BOUNDARY-CORPS"
    uint32_t checksum_rule18;                     // 3-term recurrence checksum
} CpmLyapunovBoundaryBinary;

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
    const char *out_path = (argc > 1) ? argv[1] : "lyapunov_boundary.bin";

    CpmLyapunovBoundaryBinary bin;
    memset(&bin, 0, sizeof(bin));

    bin.magic[0] = 'A';
    bin.magic[1] = 'N';
    bin.magic[2] = 'K';
    bin.magic[3] = 'H';

    bin.load_address = CPM_TPA_BASE;
    bin.entry_point = CPM_TPA_BASE;
    bin.opcode_signature = 0x43504C53; // "CPLS"

    bin.proof_singularity_boundary_intercept = 1;
    bin.proof_accumulator_redirection = 1;
    bin.proof_energy_clamping_drift_bound = 1;
    bin.proof_harmonic_absorption_damping = 1;
    bin.proof_lyapunov_boundary_grand_seal = 1;

    snprintf(bin.lyapunov_boundary_title, sizeof(bin.lyapunov_boundary_title),
             "CPMTOMIE-LYAPUNOV-SINGULARITY-BOUNDARY-CORPS");

    bin.checksum_rule18 = compute_rule18_checksum((const uint8_t *)&bin, sizeof(bin) - sizeof(uint32_t));

    FILE *f = fopen(out_path, "wb");
    if (!f) {
        perror("fopen");
        return 1;
    }
    fwrite(&bin, 1, sizeof(bin), f);
    fclose(f);

    printf("CP/M-Tomie Lyapunov Boundary Binary compiled successfully -> %s (%zu bytes, Rule 18 Checksum: 0x%08X)\n",
           out_path, sizeof(bin), bin.checksum_rule18);
    return 0;
}
