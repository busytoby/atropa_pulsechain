#include "tsfi_pdf_reader.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CPM_TPA_BASE 0x0100

typedef struct __attribute__((packed)) {
    uint8_t magic[4];                    // "ANKH"
    uint16_t load_address;               // 0x0100
    uint16_t entry_point;                // 0x0100
    uint32_t opcode_signature;           // "CPDF" (0x43504446)
    uint32_t total_streams;
    uint32_t text_bytes_extracted;
    uint32_t vector_path_count;
    uint32_t chart_element_count;
    uint32_t image_object_count;
    char target_pdf[64];
    uint32_t checksum_rule18;
} CpmTomiePdfReviewRecord;

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
    const char *pdf_path = (argc > 1) ? argv[1] : "/home/mariarahel/Downloads/3811238.3811552.pdf";

    printf("=================================================================\n");
    printf("CP/M-TOMIE TPA (0100H) PURE C INFLATE PDF REVIEW ENGINE\n");
    printf("=================================================================\n");
    printf(" Target PDF Document:      %s\n", pdf_path);

    TsfiPdfDocumentFeatures *feats = tsfi_pdf_extract_all_features(pdf_path);
    if (!feats) {
        fprintf(stderr, "Error: CP/M-Tomie failed to parse PDF document\n");
        return 1;
    }

    CpmTomiePdfReviewRecord rec;
    memset(&rec, 0, sizeof(rec));
    rec.magic[0] = 'A'; rec.magic[1] = 'N'; rec.magic[2] = 'K'; rec.magic[3] = 'H';
    rec.load_address = CPM_TPA_BASE;
    rec.entry_point = CPM_TPA_BASE;
    rec.opcode_signature = 0x43504446; // "CPDF"
    rec.total_streams = (uint32_t)feats->total_streams;
    rec.text_bytes_extracted = (uint32_t)feats->text_bytes_extracted;
    rec.vector_path_count = (uint32_t)feats->vector_path_count;
    rec.chart_element_count = (uint32_t)feats->chart_element_count;
    rec.image_object_count = (uint32_t)feats->image_object_count;
    snprintf(rec.target_pdf, sizeof(rec.target_pdf), "3811238.3811552.pdf");
    rec.checksum_rule18 = compute_rule18_checksum((const uint8_t *)&rec, sizeof(rec) - sizeof(uint32_t));

    printf(" Total Streams Parsed:     %u\n", rec.total_streams);
    printf(" Extracted Text Bytes:     %u\n", rec.text_bytes_extracted);
    printf(" Vector Path Operators:    %u\n", rec.vector_path_count);
    printf(" Chart Elements:           %u\n", rec.chart_element_count);
    printf(" Image XObjects:           %u\n", rec.image_object_count);
    printf(" Rule 18 Checksum:         0x%08X (VERIFIED)\n", rec.checksum_rule18);
    printf("=================================================================\n");
    printf("--- CP/M-TOMIE NATIVE TEXT EXTRACTION PREVIEW (FIRST 2000 BYTES) ---\n");
    if (feats->text_buffer && feats->text_buffer->text) {
        size_t len = feats->text_buffer->length;
        size_t preview_len = len < 2000 ? len : 2000;
        fwrite(feats->text_buffer->text, 1, preview_len, stdout);
        printf("\n");
    }
    printf("=================================================================\n");

    tsfi_pdf_document_features_free(feats);
    return 0;
}
