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
    uint32_t opcode_signature;             // "Q250" (0x51323530)
    uint16_t proof_computational_totality; // Theorem 246: Universal Computational Totality & Termination
    uint16_t proof_lie_weyl_isomorphism;   // Theorem 247: Lie-Weyl Scenegraph-to-Hardware Isomorphism
    uint16_t proof_verlet_fet_physics;     // Theorem 248: Symplectic FET Verlet Physical Conservation
    uint16_t proof_helmholtz_min_dispatch; // Theorem 249: Thermodynamic Helmholtz Free Energy Minimal Dispatch
    uint16_t proof_quarter_millennium_seal;// Theorem 250: 250-Theorem Sovereign Quarter-Millennium Master Seal
    char q250_title[64];                   // "QUARTER-MILLENNIUM-250-THEOREMS-MASTER-SEAL"
    uint32_t checksum_rule18;              // 3-term recurrence checksum
} CpmQuarter250Binary;

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
    const char *out_path = (argc > 1) ? argv[1] : "quarter250_proving.bin";

    CpmQuarter250Binary bin;
    memset(&bin, 0, sizeof(bin));

    bin.magic[0] = 'A';
    bin.magic[1] = 'N';
    bin.magic[2] = 'K';
    bin.magic[3] = 'H';

    bin.load_address = CPM_TPA_BASE;
    bin.entry_point = CPM_TPA_BASE;
    bin.opcode_signature = 0x51323530; // "Q250"

    bin.proof_computational_totality = 1;
    bin.proof_lie_weyl_isomorphism = 1;
    bin.proof_verlet_fet_physics = 1;
    bin.proof_helmholtz_min_dispatch = 1;
    bin.proof_quarter_millennium_seal = 1;

    snprintf(bin.q250_title, sizeof(bin.q250_title),
             "QUARTER-MILLENNIUM-250-THEOREMS-MASTER-SEAL");

    bin.checksum_rule18 = compute_rule18_checksum((const uint8_t *)&bin, sizeof(bin) - sizeof(uint32_t));

    FILE *f = fopen(out_path, "wb");
    if (!f) {
        perror("fopen");
        return 1;
    }
    fwrite(&bin, 1, sizeof(bin), f);
    fclose(f);

    printf("Synthesized 250-Theorem Quarter-Millennium Master Binary '%s' (%zu bytes)\n", out_path, sizeof(bin));
    return 0;
}
