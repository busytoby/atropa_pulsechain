#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#define CPM_TPA_BASE 0x0100

typedef struct __attribute__((packed)) {
    uint8_t magic[4];                     // "ANKH"
    uint16_t load_address;                // 0x0100
    uint16_t entry_point;                 // 0x0100
    uint32_t opcode_signature;            // "PRCQ" (0x50524351)
    uint16_t qual_rule2_no_git_binaries;  // P-Dim 1: Rule 2 Zero binaries / objects tracked in Git
    uint16_t qual_rule4_no_browser_puppet;// P-Dim 2: Rule 4 Absolute ban on Puppeteer / synthetic browser drivers
    uint16_t qual_rule6_pid_window_track; // P-Dim 3: Rule 6 Strict PID tracking / zero broad pkill safeguards
    uint16_t qual_rule9_address_zmm_res;  // P-Dim 4: Rule 9 Direct address-based contract ZMM resolution
    uint16_t qual_rule14_displace_wmq_sync;//P-Dim 5: Rule 14 DisplacementShader vertex dynamic scaling sync
    char proc_qual_title[64];             // "PROCESS-LIFECYCLE-AND-CONTRACT-QUALIFICATION"
    uint32_t checksum_rule18;             // 3-term recurrence checksum
} CpmProcQualBinary;

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
    const char *out_path = (argc > 1) ? argv[1] : "proc_qual.bin";

    CpmProcQualBinary bin;
    memset(&bin, 0, sizeof(bin));

    bin.magic[0] = 'A';
    bin.magic[1] = 'N';
    bin.magic[2] = 'K';
    bin.magic[3] = 'H';

    bin.load_address = CPM_TPA_BASE;
    bin.entry_point = CPM_TPA_BASE;
    bin.opcode_signature = 0x50524351; // "PRCQ"

    bin.qual_rule2_no_git_binaries = 1;
    bin.qual_rule4_no_browser_puppet = 1;
    bin.qual_rule6_pid_window_track = 1;
    bin.qual_rule9_address_zmm_res = 1;
    bin.qual_rule14_displace_wmq_sync = 1;

    snprintf(bin.proc_qual_title, sizeof(bin.proc_qual_title),
             "PROCESS-LIFECYCLE-AND-CONTRACT-QUALIFICATION");

    bin.checksum_rule18 = compute_rule18_checksum((const uint8_t *)&bin, sizeof(bin) - sizeof(uint32_t));

    FILE *f = fopen(out_path, "wb");
    if (!f) {
        perror("fopen");
        return 1;
    }
    fwrite(&bin, 1, sizeof(bin), f);
    fclose(f);

    printf("Synthesized Process & Contract Qualification Binary '%s' (%zu bytes)\n", out_path, sizeof(bin));
    return 0;
}
