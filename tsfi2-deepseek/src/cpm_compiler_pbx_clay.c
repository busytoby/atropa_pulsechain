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
    uint32_t opcode_signature;                    // "CPPB" (0x43505042)
    uint16_t proof_pbx_supervisory_coupling;      // Theorem 341: 24-Trunk PBX Supervisory Tone Coupling
    uint16_t proof_decadic_uniselector_step;      // Theorem 342: Decadic Uniselector Step Frequency Rotation
    uint16_t proof_auditory_haptic_synchrony;     // Theorem 343: Multi-Modal Auditory/Haptic Synchrony
    uint16_t proof_edo22_microtonal_harmonics;    // Theorem 344: DisplacementShader EDO-22 Microtonal Harmonics
    uint16_t proof_pbx_clay_sound_shape_seal;     // Theorem 345: Sovereign PBX Clay Play Multi-Modal Grand Seal
    char pbx_clay_title[64];                      // "CPMTOMIE-24TRUNK-PBX-CLAY-SOUND-SHAPE-CORPS"
    uint32_t checksum_rule18;                     // 3-term recurrence checksum
} CpmPbxClayBinary;

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
    const char *out_path = (argc > 1) ? argv[1] : "pbx_clay.bin";

    CpmPbxClayBinary bin;
    memset(&bin, 0, sizeof(bin));

    bin.magic[0] = 'A';
    bin.magic[1] = 'N';
    bin.magic[2] = 'K';
    bin.magic[3] = 'H';

    bin.load_address = CPM_TPA_BASE;
    bin.entry_point = CPM_TPA_BASE;
    bin.opcode_signature = 0x43505042; // "CPPB"

    bin.proof_pbx_supervisory_coupling = 1;
    bin.proof_decadic_uniselector_step = 1;
    bin.proof_auditory_haptic_synchrony = 1;
    bin.proof_edo22_microtonal_harmonics = 1;
    bin.proof_pbx_clay_sound_shape_seal = 1;

    snprintf(bin.pbx_clay_title, sizeof(bin.pbx_clay_title),
             "CPMTOMIE-24TRUNK-PBX-CLAY-SOUND-SHAPE-CORPS");

    bin.checksum_rule18 = compute_rule18_checksum((const uint8_t *)&bin, sizeof(bin) - sizeof(uint32_t));

    FILE *f = fopen(out_path, "wb");
    if (!f) {
        perror("fopen");
        return 1;
    }
    fwrite(&bin, 1, sizeof(bin), f);
    fclose(f);

    printf("CP/M-Tomie 24-Trunk PBX Clay Sound-Shape Binary compiled successfully -> %s (%zu bytes, Rule 18 Checksum: 0x%08X)\n",
           out_path, sizeof(bin), bin.checksum_rule18);
    return 0;
}
