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

/*
 * Downloads a page image for an authorized HathiTrust volume using RFC 5849 OAuth.
 * Parameters:
 *   htid: HathiTrust Volume ID.
 *   seq: Page sequence number (1-indexed).
 *   size: Scale factor (e.g. 100 for full size).
 *   consumer_key: OAuth consumer key.
 *   consumer_secret: OAuth consumer secret.
 *   token: OAuth access token (or NULL).
 *   token_secret: OAuth token secret (or NULL).
 *   out_img: Output pointer for allocated image bytes.
 *   out_len: Output pointer for image size.
 * Returns:
 *   1 on success, 0 on failure.
 */
int hathitrust_download_page_image_oauth(
    const char *htid,
    int seq,
    int size,
    const char *consumer_key,
    const char *consumer_secret,
    const char *token,
    const char *token_secret,
    unsigned char **out_img,
    size_t *out_len
);

#endif /* HATHITRUST_IMAGE_H */
