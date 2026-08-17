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
    uint32_t opcode_signature;             // "JESC" (0x4A455343)
    uint16_t proof_objective_deck_ingest;  // Theorem 111: JES Objective Job Card Deck Ingestion
    uint16_t proof_card_stream_carry;      // Theorem 112: Objective Job Carry Input Stream Binding
    uint16_t proof_xcom_symbol_resolve;    // Theorem 113: XCOM Backend Relocatable SKELETON Symbol Resolution
    uint16_t proof_xplsm_conduction_state; // Theorem 114: XPLSM Active-High Execution State Conduction
    uint16_t proof_totient_state_preserve; // Theorem 115: Grounded Totient State Preservation Across Step Boundaries
    char jes_carry_title[64];              // "JES-OBJECTIVE-JOB-CARRY-SKELETON-CORPS"
    uint32_t checksum_rule18;              // 3-term recurrence checksum
} CpmJESCarryBinary;

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
    const char *out_path = (argc > 1) ? argv[1] : "jes_carry_proving.bin";

    CpmJESCarryBinary bin;
    memset(&bin, 0, sizeof(bin));

    bin.magic[0] = 'A';
    bin.magic[1] = 'N';
    bin.magic[2] = 'K';
    bin.magic[3] = 'H';

    bin.load_address = CPM_TPA_BASE;
    bin.entry_point = CPM_TPA_BASE;
    bin.opcode_signature = 0x4A455343; // "JESC"

    bin.proof_objective_deck_ingest = 1;
    bin.proof_card_stream_carry = 1;
    bin.proof_xcom_symbol_resolve = 1;
    bin.proof_xplsm_conduction_state = 1;
    bin.proof_totient_state_preserve = 1;

    snprintf(bin.jes_carry_title, sizeof(bin.jes_carry_title),
             "JES-OBJECTIVE-JOB-CARRY-SKELETON-CORPS");

    bin.checksum_rule18 = compute_rule18_checksum((const uint8_t *)&bin, sizeof(bin) - sizeof(uint32_t));

    FILE *f = fopen(out_path, "wb");
    if (!f) {
        perror("fopen");
        return 1;
    }
    fwrite(&bin, 1, sizeof(bin), f);
    fclose(f);

    printf("Synthesized JES Objective Job Carry Binary '%s' (%zu bytes)\n", out_path, sizeof(bin));
    return 0;
}
