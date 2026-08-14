#define _POSIX_C_SOURCE 200809L
#define _DEFAULT_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "../inc/hathitrust_client.h"
#include "../inc/hathitrust_image.h"

int main(void) {
    printf("=== QUERYING AUTHENTIC HATHITRUST RECORD FOR UIUG.30112109778511 IN C ===\n\n");
    const char *htid = "uiug.30112109778511";

    // 1. Bibliographic Full Record Query
    printf("1. Querying Bibliographic API for HTID: %s...\n", htid);
    char *bib_json = hathitrust_query_full("htid", htid);
    if (bib_json) {
        printf("   [SUCCESS] Received %zu bytes\n", strlen(bib_json));
        printf("   Payload:\n%s\n\n", bib_json);
        free(bib_json);
    } else {
        printf("   [FAILED] Could not retrieve bibliographic record.\n\n");
    }

    // 2. Query Page Image Stream for seq=13
    printf("2. Querying Page 13 image stream for HTID: %s (seq=13)...\n", htid);
    unsigned char *img_data = NULL;
    size_t img_len = 0;
    int success = hathitrust_download_page_image(htid, 13, 100, &img_data, &img_len);
    if (success && img_data && img_len > 0) {
        printf("   [SUCCESS] Page 13 Image Downloaded: %zu bytes (Header: %.4s)\n", img_len, (char*)img_data);
        free(img_data);
    } else {
        printf("   [RESULT] Download returned %d (Length: %zu)\n", success, img_len);
    }

    return 0;
}
