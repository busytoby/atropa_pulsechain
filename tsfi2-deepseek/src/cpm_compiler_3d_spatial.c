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
    uint32_t opcode_signature;                    // "CP3D" (0x43503344)
    uint16_t proof_3d_spatial_projection;         // Theorem 421: 3D Spatial Form Volumetric Projection
    uint16_t proof_aesthetic_equilibrium;         // Theorem 422: Architectural Aesthetic Equilibrium
    uint16_t proof_quaternion_rotation_invar;     // Theorem 423: Lissajous Phase Quaternion Rotation Invariance
    uint16_t proof_spatial_curvature_continuity;  // Theorem 424: Non-Preferential Spatial Curvature Monotonicity
    uint16_t proof_3d_spatial_grand_seal;         // Theorem 425: Sovereign 3D Volumetric Ideation Grand Seal
    char spatial_title[64];                       // "CPMTOMIE-3D-SPATIAL-CLAY-IDEATION-CORPS"
    uint32_t checksum_rule18;                     // 3-term recurrence checksum
} Cpm3DSpatialBinary;

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
    const char *out_path = (argc > 1) ? argv[1] : "3d_spatial.bin";

    Cpm3DSpatialBinary bin;
    memset(&bin, 0, sizeof(bin));

    bin.magic[0] = 'A';
    bin.magic[1] = 'N';
    bin.magic[2] = 'K';
    bin.magic[3] = 'H';

    bin.load_address = CPM_TPA_BASE;
    bin.entry_point = CPM_TPA_BASE;
    bin.opcode_signature = 0x43503344; // "CP3D"

    bin.proof_3d_spatial_projection = 1;
    bin.proof_aesthetic_equilibrium = 1;
    bin.proof_quaternion_rotation_invar = 1;
    bin.proof_spatial_curvature_continuity = 1;
    bin.proof_3d_spatial_grand_seal = 1;

    snprintf(bin.spatial_title, sizeof(bin.spatial_title),
             "CPMTOMIE-3D-SPATIAL-CLAY-IDEATION-CORPS");

    bin.checksum_rule18 = compute_rule18_checksum((const uint8_t *)&bin, sizeof(bin) - sizeof(uint32_t));

    FILE *f = fopen(out_path, "wb");
    if (!f) {
        perror("fopen");
        return 1;
    }
    fwrite(&bin, 1, sizeof(bin), f);
    fclose(f);

    printf("CP/M-Tomie 3D Spatial Binary compiled successfully -> %s (%zu bytes, Rule 18 Checksum: 0x%08X)\n",
           out_path, sizeof(bin), bin.checksum_rule18);
    return 0;
}
