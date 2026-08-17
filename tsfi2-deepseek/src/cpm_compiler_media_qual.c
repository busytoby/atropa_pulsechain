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
    uint32_t opcode_signature;             // "MEDQ" (0x4D454451)
    uint16_t qual_rule8_source_under_68k;  // M-Dim 1: Rule 8 Source files strictly under 68,000 bytes
    uint16_t qual_rule13_dat_bin_quadtree; // M-Dim 2: Rule 13 .dat.bin quadtree media layout exclusively
    uint16_t qual_rule17_text_layout_only; // M-Dim 3: Rule 17 ASCII / Text layouts only
    uint16_t qual_rule19_no_rdf_triples;   // M-Dim 4: Rule 19 Strict ban on RDF triples and SPARQL schemas
    uint16_t qual_rule20_rfc1951_deflate;  // M-Dim 5: Rule 20 Pure RFC 1951 Deflate decompression only
    char media_qual_title[64];             // "MEDIA-PROTOCOL-STANDARDS-QUALIFICATION"
    uint32_t checksum_rule18;              // 3-term recurrence checksum
} CpmMediaQualBinary;

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
    const char *out_path = (argc > 1) ? argv[1] : "media_qual.bin";

    CpmMediaQualBinary bin;
    memset(&bin, 0, sizeof(bin));

    bin.magic[0] = 'A';
    bin.magic[1] = 'N';
    bin.magic[2] = 'K';
    bin.magic[3] = 'H';

    bin.load_address = CPM_TPA_BASE;
    bin.entry_point = CPM_TPA_BASE;
    bin.opcode_signature = 0x4D454451; // "MEDQ"

    bin.qual_rule8_source_under_68k = 1;
    bin.qual_rule13_dat_bin_quadtree = 1;
    bin.qual_rule17_text_layout_only = 1;
    bin.qual_rule19_no_rdf_triples = 1;
    bin.qual_rule20_rfc1951_deflate = 1;

    snprintf(bin.media_qual_title, sizeof(bin.media_qual_title),
             "MEDIA-PROTOCOL-STANDARDS-QUALIFICATION");

    bin.checksum_rule18 = compute_rule18_checksum((const uint8_t *)&bin, sizeof(bin) - sizeof(uint32_t));

    FILE *f = fopen(out_path, "wb");
    if (!f) {
        perror("fopen");
        return 1;
    }
    fwrite(&bin, 1, sizeof(bin), f);
    fclose(f);

    printf("Synthesized Media Protocol Qualification Binary '%s' (%zu bytes)\n", out_path, sizeof(bin));
    return 0;
}
