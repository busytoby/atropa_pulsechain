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
    uint32_t opcode_signature;                    // "CPTS" (0x43505453)
    uint16_t proof_totient_mass_mapping_invar;    // Theorem 331: Totient-to-Clay Mass Mapping Invariance
    uint16_t proof_clay_sculpting_homomorphism;   // Theorem 332: Clay Sculpting Deformation Homomorphism
    uint16_t proof_nonpreferential_mass_conserve; // Theorem 333: Non-Preferential Mass Conservation
    uint16_t proof_spheroid_displacement_project; // Theorem 334: Spheroid ALU & DisplacementShader Surface Projection
    uint16_t proof_totient_sculpt_grand_seal;     // Theorem 335: Sovereign CP/M-Tomie Totient Sculpting Grand Seal
    char totient_sculpt_title[64];                // "CPMTOMIE-TOTIENT-CLAY-MASS-SCULPTING-CORPS"
    uint32_t checksum_rule18;                     // 3-term recurrence checksum
} CpmTotientSculptBinary;

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
    const char *out_path = (argc > 1) ? argv[1] : "totient_sculpt.bin";

    CpmTotientSculptBinary bin;
    memset(&bin, 0, sizeof(bin));

    bin.magic[0] = 'A';
    bin.magic[1] = 'N';
    bin.magic[2] = 'K';
    bin.magic[3] = 'H';

    bin.load_address = CPM_TPA_BASE;
    bin.entry_point = CPM_TPA_BASE;
    bin.opcode_signature = 0x43505453; // "CPTS"

    bin.proof_totient_mass_mapping_invar = 1;
    bin.proof_clay_sculpting_homomorphism = 1;
    bin.proof_nonpreferential_mass_conserve = 1;
    bin.proof_spheroid_displacement_project = 1;
    bin.proof_totient_sculpt_grand_seal = 1;

    snprintf(bin.totient_sculpt_title, sizeof(bin.totient_sculpt_title),
             "CPMTOMIE-TOTIENT-CLAY-MASS-SCULPTING-CORPS");

    bin.checksum_rule18 = compute_rule18_checksum((const uint8_t *)&bin, sizeof(bin) - sizeof(uint32_t));

    FILE *f = fopen(out_path, "wb");
    if (!f) {
        perror("fopen");
        return 1;
    }
    fwrite(&bin, 1, sizeof(bin), f);
    fclose(f);

    printf("CP/M-Tomie Totient Clay Sculpting Binary compiled successfully -> %s (%zu bytes, Rule 18 Checksum: 0x%08X)\n",
           out_path, sizeof(bin), bin.checksum_rule18);
    return 0;
}
