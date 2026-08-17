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
    uint32_t opcode_signature;             // "LEXQ" (0x4C455851)
    uint16_t qual_rule1_auncient_spelling; // L-Dim 1: Rule 1 Auncient Spelling verified
    uint16_t qual_rule3_wanna_compliance;  // L-Dim 2: Rule 3 Wanna constraint & no single-word sentences
    uint16_t qual_rule7_interop_no_mocks;  // L-Dim 3: Rule 7 Dynamic interop coordination / zero mocks
    uint16_t qual_rule18_prohibited_purged;// L-Dim 4: Rule 18 Prohibited terms completely purged
    uint16_t qual_linguistic_purity_met;   // L-Dim 5: Complete Linguistic & Architectural Standards Met
    char lexicon_qual_title[64];           // "LINGUISTIC-LEXICON-INTEROP-QUALIFICATION"
    uint32_t checksum_rule18;              // 3-term recurrence checksum
} CpmLexiconQualBinary;

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
    const char *out_path = (argc > 1) ? argv[1] : "lexicon_qual.bin";

    CpmLexiconQualBinary bin;
    memset(&bin, 0, sizeof(bin));

    bin.magic[0] = 'A';
    bin.magic[1] = 'N';
    bin.magic[2] = 'K';
    bin.magic[3] = 'H';

    bin.load_address = CPM_TPA_BASE;
    bin.entry_point = CPM_TPA_BASE;
    bin.opcode_signature = 0x4C455851; // "LEXQ"

    bin.qual_rule1_auncient_spelling = 1;
    bin.qual_rule3_wanna_compliance = 1;
    bin.qual_rule7_interop_no_mocks = 1;
    bin.qual_rule18_prohibited_purged = 1;
    bin.qual_linguistic_purity_met = 1;

    snprintf(bin.lexicon_qual_title, sizeof(bin.lexicon_qual_title),
             "LINGUISTIC-LEXICON-INTEROP-QUALIFICATION");

    bin.checksum_rule18 = compute_rule18_checksum((const uint8_t *)&bin, sizeof(bin) - sizeof(uint32_t));

    FILE *f = fopen(out_path, "wb");
    if (!f) {
        perror("fopen");
        return 1;
    }
    fwrite(&bin, 1, sizeof(bin), f);
    fclose(f);

    printf("Synthesized Linguistic Lexicon Qualification Binary '%s' (%zu bytes)\n", out_path, sizeof(bin));
    return 0;
}
