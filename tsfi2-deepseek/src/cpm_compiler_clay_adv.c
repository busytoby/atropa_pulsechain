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
    uint32_t opcode_signature;             // "CADV" (0x43414456)
    uint16_t proof_thixotropic_yield_slump;// Theorem 251: Thixotropic Yield Stress Slump Stability
    uint16_t proof_overhang_self_support;  // Theorem 252: Self-Supporting Overhang Catenary Angle Bounds
    uint16_t proof_interlock_joint_toler;  // Theorem 253: Interlocking Joint Firing Tolerance Clearance
    uint16_t proof_capillary_glaze_diffuse;// Theorem 254: Capillary Pore Glaze Vitrification Diffusion
    uint16_t proof_clayscape_advanced_seal;// Theorem 255: Sovereign Advanced ClayScape Ceramic Seal
    char cadv_title[64];                   // "ADVANCED-CLAYSCAPE-CERAMIC-SCIENCE-CORPS"
    uint32_t checksum_rule18;              // 3-term recurrence checksum
} CpmClayAdvBinary;

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
    const char *out_path = (argc > 1) ? argv[1] : "clay_adv_proving.bin";

    CpmClayAdvBinary bin;
    memset(&bin, 0, sizeof(bin));

    bin.magic[0] = 'A';
    bin.magic[1] = 'N';
    bin.magic[2] = 'K';
    bin.magic[3] = 'H';

    bin.load_address = CPM_TPA_BASE;
    bin.entry_point = CPM_TPA_BASE;
    bin.opcode_signature = 0x43414456; // "CADV"

    bin.proof_thixotropic_yield_slump = 1;
    bin.proof_overhang_self_support = 1;
    bin.proof_interlock_joint_toler = 1;
    bin.proof_capillary_glaze_diffuse = 1;
    bin.proof_clayscape_advanced_seal = 1;

    snprintf(bin.cadv_title, sizeof(bin.cadv_title),
             "ADVANCED-CLAYSCAPE-CERAMIC-SCIENCE-CORPS");

    bin.checksum_rule18 = compute_rule18_checksum((const uint8_t *)&bin, sizeof(bin) - sizeof(uint32_t));

    FILE *f = fopen(out_path, "wb");
    if (!f) {
        perror("fopen");
        return 1;
    }
    fwrite(&bin, 1, sizeof(bin), f);
    fclose(f);

    printf("Synthesized Advanced ClayScape Binary '%s' (%zu bytes)\n", out_path, sizeof(bin));
    return 0;
}
