#ifndef HATHITRUST_IMAGE_H
#define HATHITRUST_IMAGE_H

#include <stddef.h>

/*
 * Downloads a page image for a public domain HathiTrust volume.
 * Parameters:
 *   htid: HathiTrust Volume ID (e.g. "nyp.33433082257217").
 *   seq: Page sequence number (1-indexed).
 *   size: Scale factor (e.g. 100 for full size, or 50, etc.).
 *   out_img: Output pointer for allocated image bytes.
 *   out_len: Output pointer for image size.
 * Returns:
 *   1 on success, 0 on failure.
 *   The caller is responsible for freeing *out_img on success.
 */
int hathitrust_download_page_image(
    const char *htid,
    int seq,
    int size,
    unsigned char **out_img,
    size_t *out_len
);

#endif /* HATHITRUST_IMAGE_H */
