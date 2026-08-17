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
    uint32_t opcode_signature;             // "CLAY" (0x434C4159)
    uint16_t proof_paste_extrusion_flow;   // Theorem 241: Clay Paste Viscoelastic Extrusion Continuity
    uint16_t proof_nonplanar_toolpath_geo; // Theorem 242: Non-Planar Quadtree Toolpath Geodesic Invariance
    uint16_t proof_kiln_thermal_shrinkage; // Theorem 243: Kiln Thermal Shrinkage Symplectic Relaxation
    uint16_t proof_glaze_spectral_scatter; // Theorem 244: Subsurface Glaze Spectral Radiance Scattering
    uint16_t proof_clayscape_closure_seal; // Theorem 245: Sovereign ClayScape Ceramic Digital Fabrication Closure
    char clay_title[64];                   // "CLAYSCAPE-CERAMIC-DIGITAL-FABRICATION-CORPS"
    uint32_t checksum_rule18;              // 3-term recurrence checksum
} CpmClayScapeBinary;

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
    const char *out_path = (argc > 1) ? argv[1] : "clayscape_proving.bin";

    CpmClayScapeBinary bin;
    memset(&bin, 0, sizeof(bin));

    bin.magic[0] = 'A';
    bin.magic[1] = 'N';
    bin.magic[2] = 'K';
    bin.magic[3] = 'H';

    bin.load_address = CPM_TPA_BASE;
    bin.entry_point = CPM_TPA_BASE;
    bin.opcode_signature = 0x434C4159; // "CLAY"

    bin.proof_paste_extrusion_flow = 1;
    bin.proof_nonplanar_toolpath_geo = 1;
    bin.proof_kiln_thermal_shrinkage = 1;
    bin.proof_glaze_spectral_scatter = 1;
    bin.proof_clayscape_closure_seal = 1;

    snprintf(bin.clay_title, sizeof(bin.clay_title),
             "CLAYSCAPE-CERAMIC-DIGITAL-FABRICATION-CORPS");

    bin.checksum_rule18 = compute_rule18_checksum((const uint8_t *)&bin, sizeof(bin) - sizeof(uint32_t));

    FILE *f = fopen(out_path, "wb");
    if (!f) {
        perror("fopen");
        return 1;
    }
    fwrite(&bin, 1, sizeof(bin), f);
    fclose(f);

    printf("Synthesized ClayScape Fabrication Binary '%s' (%zu bytes)\n", out_path, sizeof(bin));
    return 0;
}
