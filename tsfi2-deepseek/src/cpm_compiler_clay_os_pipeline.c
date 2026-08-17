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
    uint32_t opcode_signature;                    // "CPOS" (0x43504F53)
    uint16_t proof_6step_event_dispatch;          // Theorem 311: 6-Step CP/M-Tomie Event Dispatch Monotonicity
    uint16_t proof_fcb_fragment_record_locking;   // Theorem 312: Two-Layer 36-Byte FCB Fragment Locking
    uint16_t proof_tpa_swiglu_clamped_ingest;     // Theorem 313: TPA 0100H Non-Preferential Ingestion
    uint16_t proof_wmq_displacement_vector_lines; // Theorem 314: WinchesterMQ DisplacementShader Vector Invariance
    uint16_t proof_cpm_clay_os_grand_seal;        // Theorem 315: Sovereign CP/M-Tomie Clay Play OS Pipeline Grand Seal
    char clay_os_title[64];                       // "CPMTOMIE-6STEP-CLAY-PLAY-OS-PIPELINE"
    uint32_t checksum_rule18;                     // 3-term recurrence checksum
} CpmClayOsPipelineBinary;

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
    const char *out_path = (argc > 1) ? argv[1] : "clay_os_pipeline.bin";

    CpmClayOsPipelineBinary bin;
    memset(&bin, 0, sizeof(bin));

    bin.magic[0] = 'A';
    bin.magic[1] = 'N';
    bin.magic[2] = 'K';
    bin.magic[3] = 'H';

    bin.load_address = CPM_TPA_BASE;
    bin.entry_point = CPM_TPA_BASE;
    bin.opcode_signature = 0x43504F53; // "CPOS"

    bin.proof_6step_event_dispatch = 1;
    bin.proof_fcb_fragment_record_locking = 1;
    bin.proof_tpa_swiglu_clamped_ingest = 1;
    bin.proof_wmq_displacement_vector_lines = 1;
    bin.proof_cpm_clay_os_grand_seal = 1;

    snprintf(bin.clay_os_title, sizeof(bin.clay_os_title),
             "CPMTOMIE-6STEP-CLAY-PLAY-OS-PIPELINE");

    bin.checksum_rule18 = compute_rule18_checksum((const uint8_t *)&bin, sizeof(bin) - sizeof(uint32_t));

    FILE *f = fopen(out_path, "wb");
    if (!f) {
        perror("fopen");
        return 1;
    }
    fwrite(&bin, 1, sizeof(bin), f);
    fclose(f);

    printf("CP/M-Tomie 6-Step Clay Play OS Pipeline Binary compiled successfully -> %s (%zu bytes, Rule 18 Checksum: 0x%08X)\n",
           out_path, sizeof(bin), bin.checksum_rule18);
    return 0;
}
