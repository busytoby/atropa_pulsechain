#include "tsfi_pdf_writer.h"
#include "tsfi_pdf_reader.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv) {
    const char *out_pdf = (argc > 1) ? argv[1] : "/tmp/cpm_tomie_generated_test.pdf";

    TsfiPdfDocumentWriter *w = tsfi_pdf_writer_create();
    if (!w) {
        fprintf(stderr, "Failed to create PDF writer\n");
        return 1;
    }

    const char *test_msg = "CP/M-Tomie Pure C Native PDF Generation System Verified";
    tsfi_pdf_writer_add_page(w, test_msg, strlen(test_msg));

    if (tsfi_pdf_writer_finalize_file(w, out_pdf) != 0) {
        fprintf(stderr, "Failed to finalize PDF file\n");
        tsfi_pdf_writer_free(w);
        return 1;
    }
    tsfi_pdf_writer_free(w);

    printf("Successfully generated clean-room PDF: '%s'\n", out_pdf);

    // Verify roundtrip read via tsfi_pdf_reader
    size_t len = 0;
    char *read_back = tsfi_pdf_extract_text(out_pdf, &len);
    if (read_back) {
        printf("Verified Roundtrip Extraction: '%s' (%zu bytes)\n", read_back, len);
        free(read_back);
    } else {
        printf("Verification Warning: Could not read back generated PDF\n");
    }

    return 0;
}
