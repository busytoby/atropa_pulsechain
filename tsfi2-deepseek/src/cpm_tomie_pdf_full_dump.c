#include "tsfi_pdf_reader.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
    const char *pdf_path = (argc > 1) ? argv[1] : "/home/mariarahel/Downloads/3811238.3811552.pdf";
    size_t len = 0;
    char *text = tsfi_pdf_extract_text(pdf_path, &len);
    if (!text) {
        fprintf(stderr, "Error extracting text\n");
        return 1;
    }
    FILE *out = fopen("/tmp/cpm_tomie_extracted_paper.txt", "w");
    if (out) {
        fwrite(text, 1, len, out);
        fclose(out);
    }
    free(text);
    return 0;
}
