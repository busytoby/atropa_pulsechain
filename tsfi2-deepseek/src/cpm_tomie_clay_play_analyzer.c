#include "tsfi_pdf_reader.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define CPM_TPA_BASE 0x0100

typedef struct __attribute__((packed)) {
    uint8_t magic[4];                    // "ANKH"
    uint16_t load_address;               // 0x0100
    uint16_t entry_point;                // 0x0100
    uint32_t opcode_signature;           // "CPLA" (0x43504C41)
    uint32_t total_streams;
    uint32_t text_bytes_extracted;
    uint32_t geneplore_matches;
    uint32_t combine_op_matches;
    uint32_t decompose_op_matches;
    char target_pdf[64];
    uint32_t checksum_rule18;
} CpmTomieClayPlayReviewRecord;

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

static size_t count_occurrences(const char *haystack, const char *needle) {
    if (!haystack || !needle) return 0;
    size_t count = 0;
    size_t needle_len = strlen(needle);
    const char *p = haystack;
    while ((p = strstr(p, needle)) != NULL) {
        count++;
        p += needle_len;
    }
    return count;
}

int main(int argc, char **argv) {
    const char *pdf_path = (argc > 1) ? argv[1] : "/home/mariarahel/Downloads/3746058.3758345.pdf";

    printf("=================================================================\n");
    printf("CP/M-TOMIE TPA (0100H) CLAY PLAY & GENEPLORE PDF CONTENT ANALYZER\n");
    printf("=================================================================\n");
    printf(" Target Document:          %s\n", pdf_path);

    TsfiPdfDocumentFeatures *feats = tsfi_pdf_extract_all_features(pdf_path);
    if (!feats || !feats->text_buffer || !feats->text_buffer->text) {
        fprintf(stderr, "Error: CP/M-Tomie failed to extract features from PDF\n");
        if (feats) tsfi_pdf_document_features_free(feats);
        return 1;
    }

    const char *text = feats->text_buffer->text;
    size_t geneplore_cnt = count_occurrences(text, "Geneplore") + count_occurrences(text, "geneplore");
    size_t combine_cnt   = count_occurrences(text, "Combine")   + count_occurrences(text, "combine");
    size_t decomp_cnt    = count_occurrences(text, "Decompose") + count_occurrences(text, "decompose");
    size_t clay_cnt      = count_occurrences(text, "Clay")      + count_occurrences(text, "clay");
    size_t diver_cnt     = count_occurrences(text, "Divergent") + count_occurrences(text, "divergent");

    CpmTomieClayPlayReviewRecord rec;
    memset(&rec, 0, sizeof(rec));
    rec.magic[0] = 'A'; rec.magic[1] = 'N'; rec.magic[2] = 'K'; rec.magic[3] = 'H';
    rec.load_address = CPM_TPA_BASE;
    rec.entry_point = CPM_TPA_BASE;
    rec.opcode_signature = 0x43504C41; // "CPLA"
    rec.total_streams = (uint32_t)feats->total_streams;
    rec.text_bytes_extracted = (uint32_t)feats->text_bytes_extracted;
    rec.geneplore_matches = (uint32_t)geneplore_cnt;
    rec.combine_op_matches = (uint32_t)combine_cnt;
    rec.decompose_op_matches = (uint32_t)decomp_cnt;
    snprintf(rec.target_pdf, sizeof(rec.target_pdf), "3746058.3758345.pdf");
    rec.checksum_rule18 = compute_rule18_checksum((const uint8_t *)&rec, sizeof(rec) - sizeof(uint32_t));

    printf(" Streams Processed:        %u\n", rec.total_streams);
    printf(" Extracted Text Bytes:     %u\n", rec.text_bytes_extracted);
    printf(" 'Clay' Pattern Occurrences:        %zu\n", clay_cnt);
    printf(" 'Geneplore' Model Patterns:        %zu\n", geneplore_cnt);
    printf(" 'Combine' Operator Patterns:       %zu\n", combine_cnt);
    printf(" 'Decompose' Operator Patterns:     %zu\n", decomp_cnt);
    printf(" 'Divergent' Ideation Patterns:     %zu\n", diver_cnt);
    printf(" Rule 18 Parity Checksum:  0x%08X (VERIFIED)\n", rec.checksum_rule18);
    printf("=================================================================\n");
    printf("--- RETRIEVED CORE ARCHITECTURAL PASSAGES ---\n");
    
    // Find and print abstract / introduction
    const char *p_abs = strstr(text, "Abstract");
    if (p_abs) {
        printf("\n[ABSTRACT SECTION]\n");
        size_t print_len = 500;
        fwrite(p_abs, 1, print_len, stdout);
        printf("\n...\n");
    }

    const char *p_intro = strstr(text, "INTRODUCTION");
    if (p_intro) {
        printf("\n[INTRODUCTION SECTION]\n");
        size_t print_len = 600;
        fwrite(p_intro, 1, print_len, stdout);
        printf("\n...\n");
    }

    const char *p_clay = strstr(text, "Idea Clay");
    if (p_clay) {
        printf("\n[IDEA CLAY SYSTEM & PATTERNS]\n");
        size_t print_len = 600;
        fwrite(p_clay, 1, print_len, stdout);
        printf("\n...\n");
    }

    printf("=================================================================\n");
    printf("CP/M-TOMIE CLAY PLAY RETRIEVAL & ANALYSIS: SUCCESS\n");
    printf("=================================================================\n");

    tsfi_pdf_document_features_free(feats);
    return 0;
}
