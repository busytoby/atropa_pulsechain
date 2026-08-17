#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#define CPM_TPA_BASE 0x0100

typedef struct __attribute__((packed)) {
    uint8_t magic[4];                     // "ANKH"
    uint16_t load_address;                // 0x0100
    uint16_t entry_point;                 // 0x0100
    uint32_t opcode_signature;            // "LYST" (0x4C595354)
    uint32_t discrete_initial_energy;     // V(x_0) = 100,000
    int32_t discrete_dissipation_rate;    // Delta V = -12,500
    uint16_t proof_positive_definite;     // Theorem 1: Discrete Positive Definiteness V(x) > 0
    uint16_t proof_monotone_dissipation;  // Theorem 2: Monotone Negative Step Decay Delta V(x) < 0
    uint16_t proof_bmrc_contraction_rate; // Theorem 3: Fixed-point contraction bounded by 7/8
    uint16_t proof_finite_termination;    // Theorem 4: Termination in finite machine steps
    char standard_id[64];                 // "LYAPUNOV-STRATEGY-PROOF-STANDARD-CORPS"
    uint32_t checksum_rule18;             // 3-term recurrence checksum
} CpmLyapunovStdBinary;

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
    const char *out_path = (argc > 1) ? argv[1] : "lyapunov_std.bin";

    CpmLyapunovStdBinary bin;
    memset(&bin, 0, sizeof(bin));

    bin.magic[0] = 'A';
    bin.magic[1] = 'N';
    bin.magic[2] = 'K';
    bin.magic[3] = 'H';

    bin.load_address = CPM_TPA_BASE;
    bin.entry_point = CPM_TPA_BASE;
    bin.opcode_signature = 0x4C595354; // "LYST"

    bin.discrete_initial_energy = 100000;
    bin.discrete_dissipation_rate = -12500;

    bin.proof_positive_definite = 1;
    bin.proof_monotone_dissipation = 1;
    bin.proof_bmrc_contraction_rate = 1;
    bin.proof_finite_termination = 1;

    snprintf(bin.standard_id, sizeof(bin.standard_id),
             "LYAPUNOV-STRATEGY-PROOF-STANDARD-CORPS");

    bin.checksum_rule18 = compute_rule18_checksum((const uint8_t *)&bin, sizeof(bin) - sizeof(uint32_t));

    FILE *f = fopen(out_path, "wb");
    if (!f) {
        perror("fopen");
        return 1;
    }
    fwrite(&bin, 1, sizeof(bin), f);
    fclose(f);

    printf("Synthesized Lyapunov Strategy Standard Binary '%s' (%zu bytes)\n", out_path, sizeof(bin));
    return 0;
}
