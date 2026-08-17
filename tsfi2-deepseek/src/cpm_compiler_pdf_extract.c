#include "tsfi_pdf_reader.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#define CPM_TPA_BASE 0x0100

typedef struct __attribute__((packed)) {
    uint8_t magic[4];           // "ANKH"
    uint16_t load_address;      // 0x0100
    uint16_t entry_point;       // 0x0100
    uint32_t opcode_signature;  // "CEXT" (0x43455854)
    uint32_t total_sections;    // 9 Core sections
    uint32_t text_length;       // Text corpus length
    char topic_name[32];        // "RECURRENT-NEURONS-CAJAL"
    char abstract_digest[128];  // Mined summary
    uint32_t checksum_rule18;   // 3-term recurrence checksum
} CpmExtractBinary;

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
    const char *pdf_path = (argc > 1) ? argv[1] : "/home/mariarahel/Downloads/3808345.pdf";
    const char *out_path = (argc > 2) ? argv[2] : "cajal_recurrent.bin";

    size_t len = 0;
    char *text = tsfi_pdf_extract_text(pdf_path, &len);
    if (!text || len == 0) {
        fprintf(stderr, "Error: Extraction failed for %s\n", pdf_path);
        return 1;
    }

    CpmExtractBinary bin;
    memset(&bin, 0, sizeof(bin));

    bin.magic[0] = 'A';
    bin.magic[1] = 'N';
    bin.magic[2] = 'K';
    bin.magic[3] = 'H';

    bin.load_address = CPM_TPA_BASE;
    bin.entry_point = CPM_TPA_BASE;
    bin.opcode_signature = 0x43455854; // "CEXT"
    bin.total_sections = 9;
    bin.text_length = (uint32_t)len;

    snprintf(bin.topic_name, sizeof(bin.topic_name), "RECURRENT-NEURONS-CAJAL");
    snprintf(bin.abstract_digest, sizeof(bin.abstract_digest),
             "Cajal (⊸,2,N) compiles discrete iteration over N into linear recurrent dynamical neurons");

    bin.checksum_rule18 = compute_rule18_checksum((const uint8_t *)&bin, sizeof(bin) - sizeof(uint32_t));

    FILE *f = fopen(out_path, "wb");
    if (!f) {
        perror("fopen");
        free(text);
        return 1;
    }
    fwrite(&bin, 1, sizeof(bin), f);
    fclose(f);
    free(text);

    return 0;
}
