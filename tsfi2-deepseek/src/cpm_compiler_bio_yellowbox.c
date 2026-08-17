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
    uint32_t opcode_signature;                    // "CPYB" (0x43505942)
    uint16_t proof_bio_ringtone_encoding;         // Theorem 351: .bio Ringtone Acoustic Packaging
    uint16_t proof_pbx_inband_yellowbox_dialup;   // Theorem 352: PBX In-Band Yellow Box Dialup
    uint16_t proof_soul_saat_asset_commutation;   // Theorem 353: Soul & Saat Asset Commutation
    uint16_t proof_tone_matrix_invertibility;     // Theorem 354: Non-Preferential Tone Matrix Invertibility
    uint16_t proof_bio_yellowbox_telephony_seal;  // Theorem 355: Sovereign .bio Yellow Box Telephony Grand Seal
    char bio_yellowbox_title[64];                 // "CPMTOMIE-BIO-RINGTONE-YELLOWBOX-TELEPHONY"
    uint32_t checksum_rule18;                     // 3-term recurrence checksum
} CpmBioYellowboxBinary;

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
    const char *out_path = (argc > 1) ? argv[1] : "bio_yellowbox.bin";

    CpmBioYellowboxBinary bin;
    memset(&bin, 0, sizeof(bin));

    bin.magic[0] = 'A';
    bin.magic[1] = 'N';
    bin.magic[2] = 'K';
    bin.magic[3] = 'H';

    bin.load_address = CPM_TPA_BASE;
    bin.entry_point = CPM_TPA_BASE;
    bin.opcode_signature = 0x43505942; // "CPYB"

    bin.proof_bio_ringtone_encoding = 1;
    bin.proof_pbx_inband_yellowbox_dialup = 1;
    bin.proof_soul_saat_asset_commutation = 1;
    bin.proof_tone_matrix_invertibility = 1;
    bin.proof_bio_yellowbox_telephony_seal = 1;

    snprintf(bin.bio_yellowbox_title, sizeof(bin.bio_yellowbox_title),
             "CPMTOMIE-BIO-RINGTONE-YELLOWBOX-TELEPHONY");

    bin.checksum_rule18 = compute_rule18_checksum((const uint8_t *)&bin, sizeof(bin) - sizeof(uint32_t));

    FILE *f = fopen(out_path, "wb");
    if (!f) {
        perror("fopen");
        return 1;
    }
    fwrite(&bin, 1, sizeof(bin), f);
    fclose(f);

    printf("CP/M-Tomie .bio Yellow Box Telephony Binary compiled successfully -> %s (%zu bytes, Rule 18 Checksum: 0x%08X)\n",
           out_path, sizeof(bin), bin.checksum_rule18);
    return 0;
}
