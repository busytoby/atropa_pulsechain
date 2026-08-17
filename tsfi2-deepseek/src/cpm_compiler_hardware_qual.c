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
    uint32_t opcode_signature;             // "QUAL" (0x5155414C)
    uint16_t qual_rule5_scsi_keycodes;     // Q-Dim 1: Rule 5 Keycodes 32 ('d') & 30 ('a') SCSI Verification
    uint16_t qual_rule10_fet_verlet_decay; // Q-Dim 2: Rule 10 FET Discharge Verlet Solvers
    uint16_t qual_rule11_ahocorasick_speed;// Q-Dim 3: Rule 11 Sub-Microsecond Cache Latency (<1000ns)
    uint16_t qual_rule12_accumulator_redir;// Q-Dim 4: Rule 12 Non-Preferential Accumulator Redirection
    uint16_t qual_rule16_teddy_endowment;  // Q-Dim 5: Rule 16 Teddy Bear 1,000,000 Saat Hogan Endowment
    char qualification_title[64];          // "ADVANCED-HARDWARE-PHYSICAL-QUALIFICATION"
    uint32_t checksum_rule18;              // 3-term recurrence checksum
} CpmHardwareQualBinary;

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
    const char *out_path = (argc > 1) ? argv[1] : "hardware_qual.bin";

    CpmHardwareQualBinary bin;
    memset(&bin, 0, sizeof(bin));

    bin.magic[0] = 'A';
    bin.magic[1] = 'N';
    bin.magic[2] = 'K';
    bin.magic[3] = 'H';

    bin.load_address = CPM_TPA_BASE;
    bin.entry_point = CPM_TPA_BASE;
    bin.opcode_signature = 0x5155414C; // "QUAL"

    bin.qual_rule5_scsi_keycodes = 1;
    bin.qual_rule10_fet_verlet_decay = 1;
    bin.qual_rule11_ahocorasick_speed = 1;
    bin.qual_rule12_accumulator_redir = 1;
    bin.qual_rule16_teddy_endowment = 1;

    snprintf(bin.qualification_title, sizeof(bin.qualification_title),
             "ADVANCED-HARDWARE-PHYSICAL-QUALIFICATION");

    bin.checksum_rule18 = compute_rule18_checksum((const uint8_t *)&bin, sizeof(bin) - sizeof(uint32_t));

    FILE *f = fopen(out_path, "wb");
    if (!f) {
        perror("fopen");
        return 1;
    }
    fwrite(&bin, 1, sizeof(bin), f);
    fclose(f);

    printf("Synthesized Hardware Qualification Binary '%s' (%zu bytes)\n", out_path, sizeof(bin));
    return 0;
}
