#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#define CPM_TPA_BASE 0x0100

typedef struct __attribute__((packed)) {
    uint8_t magic[4];                    // "ANKH"
    uint16_t load_address;               // 0x0100
    uint16_t entry_point;                // 0x0100
    uint32_t opcode_signature;           // "CJNR" (0x434A4E52)
    uint16_t proof_affine_embedding;     // Proof 1: Categorical Affine Subspace Embedding
    uint16_t proof_kronecker_tensor;     // Proof 2: Direct Sum / Kronecker Product Commutativity
    uint16_t proof_finite_unrolling;     // Proof 3: Truncated Dynamical Invariance & Bounded Depth
    uint16_t proof_gradient_smoothness;  // Proof 4: Lipschitz Continuous Backpropagation Continuity
    uint16_t proof_scsi_hbridge_interop; // Proof 5: WinchesterMQ SCSI Register Gated Mapping
    uint16_t proof_discrete_motzkin_fit; // Proof 6: Motzkin Field Divisor Field Stability
    char suite_description[64];          // "CAJAL-RECURRENT-NEURON-PROOF-SUITE"
    uint32_t checksum_rule18;            // 3-term recurrence checksum
} CpmCajalNeuronProofBinary;

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
    const char *out_path = (argc > 1) ? argv[1] : "cajal_neuron_suite.bin";

    CpmCajalNeuronProofBinary bin;
    memset(&bin, 0, sizeof(bin));

    bin.magic[0] = 'A';
    bin.magic[1] = 'N';
    bin.magic[2] = 'K';
    bin.magic[3] = 'H';

    bin.load_address = CPM_TPA_BASE;
    bin.entry_point = CPM_TPA_BASE;
    bin.opcode_signature = 0x434A4E52; // "CJNR"

    bin.proof_affine_embedding = 1;
    bin.proof_kronecker_tensor = 1;
    bin.proof_finite_unrolling = 1;
    bin.proof_gradient_smoothness = 1;
    bin.proof_scsi_hbridge_interop = 1;
    bin.proof_discrete_motzkin_fit = 1;

    snprintf(bin.suite_description, sizeof(bin.suite_description),
             "CAJAL-RECURRENT-NEURON-PROOF-SUITE");

    bin.checksum_rule18 = compute_rule18_checksum((const uint8_t *)&bin, sizeof(bin) - sizeof(uint32_t));

    FILE *f = fopen(out_path, "wb");
    if (!f) {
        perror("fopen");
        return 1;
    }
    fwrite(&bin, 1, sizeof(bin), f);
    fclose(f);

    printf("Emitted CP/M-Tomie Neuron Proof Suite '%s' (%zu bytes)\n", out_path, sizeof(bin));
    return 0;
}
