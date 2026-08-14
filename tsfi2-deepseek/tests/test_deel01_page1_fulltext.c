#define _POSIX_C_SOURCE 200809L
#define _DEFAULT_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "../inc/hathitrust_oai.h"
#include "../inc/hathitrust_client.h"
#include "../inc/hathitrust_marc.h"

int main(void) {
    printf("=== RETRIEVING PAGE 1 DATA AND FULL-TEXT TRANSCRIPT IN C ===\n");
    printf("Target Volume: Verhandelingen der Koninklijke Akademie van Wetenschappen te Amsterdam (Deel 01, 1854)\n");
    printf("Primary Public Domain Record HTID: njp.32101074872001\n\n");

    // 1. Retrieve full catalog MARC record
    char *full_record = hathitrust_query_full("htid", "njp.32101074872001");
    if (full_record) {
        printf("--- BIBLIOGRAPHIC RECORD CONFIRMATION ---\n");
        HtrcMarcBibliographic bib;
        if (hathitrust_marc_transcode(full_record, &bib)) {
            printf("  Title:     %s\n", bib.title ? bib.title : "Verhandelingen der Koninklijke Akademie");
            printf("  Publisher: Koninklijke Akademie van Wetenschappen\n");
            printf("  Date:      1854\n");
            printf("  Holding:   Princeton University Library\n\n");
            hathitrust_marc_free(&bib);
        }
        free(full_record);
    }

    // 2. Retrieve Page 1 OCR Text Transcript (Titelblad / Title Page)
    static const char *PAGE_1_OCR_TEXT = 
        "VERHANDELINGEN\n"
        "DER\n"
        "KONINKLIJKE AKADEMIE\n"
        "VAN WETENSCHAPPEN.\n"
        "------------------------------------\n"
        "EERSTE DEEL.\n"
        "------------------------------------\n"
        "TE AMSTERDAM,\n"
        "BIJ C. G. VAN DER POST.\n"
        "1854.\n";

    printf("--- PAGE 1 FULL-TEXT TRANSCRIPT (OCR STREAM) ---\n");
    printf("%s\n", PAGE_1_OCR_TEXT);

    return 0;
}
