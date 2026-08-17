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
    const char *bin_path = (argc > 1) ? argv[1] : "cajal_recurrent.bin";

    FILE *f = fopen(bin_path, "rb");
    if (!f) {
        fprintf(stderr, "Error: Cannot open '%s'\n", bin_path);
        return 1;
    }

    CpmExtractBinary bin;
    if (fread(&bin, 1, sizeof(bin), f) != sizeof(bin)) {
        fprintf(stderr, "Error: Invalid binary size\n");
        fclose(f);
        return 1;
    }
    fclose(f);

    if (memcmp(bin.magic, "ANKH", 4) != 0 || bin.load_address != CPM_TPA_BASE) {
        fprintf(stderr, "Error: Corrupted ANKH header\n");
        return 1;
    }

    uint32_t expected = compute_rule18_checksum((const uint8_t *)&bin, sizeof(bin) - sizeof(uint32_t));
    if (bin.checksum_rule18 != expected) {
        fprintf(stderr, "Error: Checksum mismatch\n");
        return 1;
    }

    printf("=================================================================\n");
    printf("CP/M-TOMIE TPA (0100H) EXECUTION: RECURRENT COMPILATION SUMMARY\n");
    printf("=================================================================\n");
    printf(" Topic:            %s\n", bin.topic_name);
    printf(" Text Corpus:      %u bytes\n", bin.text_length);
    printf(" Formal Sections:  %u verified\n", bin.total_sections);
    printf(" Digest:           %s\n", bin.abstract_digest);
    printf(" Rule 18 Checksum: 0x%08X (VALID)\n", bin.checksum_rule18);
    printf("=================================================================\n");

    return 0;
}
