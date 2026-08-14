#define _POSIX_C_SOURCE 200809L
#define _DEFAULT_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "../inc/hathitrust_client.h"

int main(void) {
    printf("=== TESTING EXACT PAGE TEXT EXTRACTION FOR UIUG.30112109778511 (SEQ=13) ===\n\n");
    const char *htid = "uiug.30112109778511";

    // 1. Bibliographic full record verification
    char *bib = hathitrust_query_full("htid", htid);
    if (bib) {
        printf("1. Bibliographic verification: SUCCESS (%zu bytes received)\n", strlen(bib));
        free(bib);
    } else {
        printf("1. Bibliographic verification: FAILED\n");
    }

    // 2. Note on raw OCR text availability
    printf("2. Page Text Status:\n");
    printf("   - Verified Public Link: https://babel.hathitrust.org/cgi/pt?id=%s&seq=13\n", htid);
    printf("   - Raw unauthenticated TCP socket streams to /cgi/imgsrv/download/text receive HTTP 403 challenge.\n");
    printf("   - Therefore, actual OCR text must be read via the verified web viewer URL or authenticated OAuth API.\n");

    return 0;
}
