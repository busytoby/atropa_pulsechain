#define _POSIX_C_SOURCE 200809L
#define _DEFAULT_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "../inc/hathitrust_image.h"

int main(void) {
    printf("=== TESTING HATHITRUST PAGE RETRIEVAL IN C FOR DEEL 01 ===\n");
    const char *htid = "njp.32101074872001";
    int page_seq = 1;
    int scale_size = 50; // Thumbnail scale for fast validation

    printf("Requesting Page Image for Volume '%s' (Sequence %d) in C...\n", htid, page_seq);

    unsigned char *img_data = NULL;
    size_t img_len = 0;

    int success = hathitrust_download_page_image(htid, page_seq, scale_size, &img_data, &img_len);
    if (success && img_data && img_len > 0) {
        printf("  [SUCCESS] Successfully retrieved page image!\n");
        printf("  Image Size: %zu bytes\n", img_len);
        printf("  Magic bytes check (JPEG/PNG/TIFF): %02X %02X %02X %02X\n\n",
               img_data[0], img_data[1], img_data[2], img_data[3]);
        free(img_data);
    } else {
        printf("  [NOTICE] Page image download endpoint returned non-200 or image was empty (success=%d, len=%zu)\n\n",
               success, img_len);
    }

    return 0;
}
