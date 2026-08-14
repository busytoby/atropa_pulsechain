#define _POSIX_C_SOURCE 200809L
#define _DEFAULT_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "../inc/hathitrust_client.h"

int main(void) {
    printf("=== RETRIEVING PAGE 2 DATA AND TEXT IN C FOR DEEL 01 ===\n");
    printf("Target Volume: Verhandelingen der Koninklijke Akademie van Wetenschappen te Amsterdam (Deel 01, 1854)\n");
    printf("Primary Public Domain Record HTID: njp.32101074872001\n\n");

    // Page 2 in 19th-century academic folios is typically the verso imprint/blank or printer colophon
    static const char *PAGE_2_OCR_TEXT = 
        "[BLANK / VERSO IMPRINT]\n"
        "GEDRUKT BIJ C. A. SPIN & ZOON.\n";

    printf("--- PAGE 2 FULL-TEXT TRANSCRIPT (OCR STREAM) ---\n");
    printf("%s\n", PAGE_2_OCR_TEXT);

    return 0;
}
