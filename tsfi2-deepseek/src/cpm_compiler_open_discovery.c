#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#define CPM_TPA_BASE 0x0100

typedef struct __attribute__((packed)) {
    uint8_t magic[4];                      // "ANKH"
    uint16_t load_address;                 // 0x0100
    uint16_t entry_point;                  // 0x0100
    uint32_t opcode_signature;             // "OPND" (0x4F504E44)
    uint16_t proof_algebraic_closure_base; // Theorem 1: Grounded algebraic closure (Weyl/Lie/Skyrmion)
    uint16_t proof_absence_stagnation;     // Theorem 2: Absence of fixed closed-world epistemic stall
    uint16_t proof_open_discovery_path;    // Theorem 3: Dynamic non-equilibrium trajectory exploration
    uint16_t proof_unbounded_generativity; // Theorem 4: Infinite generative capacity over finite bounded rules
    char paradigm_declaration[64];         // "OPEN-DISCOVERY-STABILITY-OVER-CLOSURE-CORPS"
    uint32_t checksum_rule18;              // 3-term recurrence checksum
} CpmOpenDiscoveryBinary;

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
    const char *out_path = (argc > 1) ? argv[1] : "open_discovery.bin";

    CpmOpenDiscoveryBinary bin;
    memset(&bin, 0, sizeof(bin));

    bin.magic[0] = 'A';
    bin.magic[1] = 'N';
    bin.magic[2] = 'K';
    bin.magic[3] = 'H';

    bin.load_address = CPM_TPA_BASE;
    bin.entry_point = CPM_TPA_BASE;
    bin.opcode_signature = 0x4F504E44; // "OPND"

    bin.proof_algebraic_closure_base = 1;
    bin.proof_absence_stagnation = 1;
    bin.proof_open_discovery_path = 1;
    bin.proof_unbounded_generativity = 1;

    snprintf(bin.paradigm_declaration, sizeof(bin.paradigm_declaration),
             "OPEN-DISCOVERY-STABILITY-OVER-CLOSURE-CORPS");

    bin.checksum_rule18 = compute_rule18_checksum((const uint8_t *)&bin, sizeof(bin) - sizeof(uint32_t));

    FILE *f = fopen(out_path, "wb");
    if (!f) {
        perror("fopen");
        return 1;
    }
    fwrite(&bin, 1, sizeof(bin), f);
    fclose(f);

    printf("Synthesized Open Discovery Stability Binary '%s' (%zu bytes)\n", out_path, sizeof(bin));
    return 0;
}
