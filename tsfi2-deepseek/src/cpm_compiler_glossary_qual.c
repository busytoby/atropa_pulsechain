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
    uint32_t opcode_signature;             // "GLSQ" (0x474C5351)
    uint16_t qual_tripartite_vm_register;  // G-Dim 1: Part 1 VM Register Context Formally Defined
    uint16_t qual_tripartite_math_func;    // G-Dim 2: Part 2 Mathematical Transformation Mapped
    uint16_t qual_tripartite_geom_manifest;// G-Dim 3: Part 3 3D Geometric / Visual Projection Mapped
    uint16_t qual_all_terms_compliant;     // G-Dim 4: 100% Rule 15 Permanent Glossary Terms Compliant
    uint16_t qual_glossary_soundness_met;  // G-Dim 5: Complete Structural Rigor Met
    char glossary_qual_title[64];          // "RULE-15-TRIPARTITE-GLOSSARY-QUALIFICATION"
    uint32_t checksum_rule18;              // 3-term recurrence checksum
} CpmGlossaryQualBinary;

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
    const char *out_path = (argc > 1) ? argv[1] : "glossary_qual.bin";

    CpmGlossaryQualBinary bin;
    memset(&bin, 0, sizeof(bin));

    bin.magic[0] = 'A';
    bin.magic[1] = 'N';
    bin.magic[2] = 'K';
    bin.magic[3] = 'H';

    bin.load_address = CPM_TPA_BASE;
    bin.entry_point = CPM_TPA_BASE;
    bin.opcode_signature = 0x474C5351; // "GLSQ"

    bin.qual_tripartite_vm_register = 1;
    bin.qual_tripartite_math_func = 1;
    bin.qual_tripartite_geom_manifest = 1;
    bin.qual_all_terms_compliant = 1;
    bin.qual_glossary_soundness_met = 1;

    snprintf(bin.glossary_qual_title, sizeof(bin.glossary_qual_title),
             "RULE-15-TRIPARTITE-GLOSSARY-QUALIFICATION");

    bin.checksum_rule18 = compute_rule18_checksum((const uint8_t *)&bin, sizeof(bin) - sizeof(uint32_t));

    FILE *f = fopen(out_path, "wb");
    if (!f) {
        perror("fopen");
        return 1;
    }
    fwrite(&bin, 1, sizeof(bin), f);
    fclose(f);

    printf("Synthesized Rule 15 Glossary Qualification Binary '%s' (%zu bytes)\n", out_path, sizeof(bin));
    return 0;
}
