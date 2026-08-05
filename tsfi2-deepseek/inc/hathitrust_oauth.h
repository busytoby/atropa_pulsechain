#ifndef HATHITRUST_OAUTH_H
#define HATHITRUST_OAUTH_H

#include <stddef.h>

/*
 * Generates an OAuth 1.0a Authorization header value.
 * Parameters:
 *   method: HTTP method (e.g. "GET", "POST").
 *   url: Target request URL (e.g. "https://babel.hathitrust.org/cgi/pt").
 *   consumer_key: HathiTrust API consumer key.
 *   consumer_secret: HathiTrust API consumer secret.
 *   token: OAuth token (can be NULL).
 *   token_secret: OAuth token secret (can be NULL).
 *   nonce: Unique nonce string (e.g. randomized).
 *   timestamp: Epoch timestamp string.
 *   out_header: Destination buffer to write the header value.
 *   out_header_len: Size of the destination buffer.
 * Returns:
 *   1 on success, 0 on signature generation failure.
 */
int hathitrust_oauth_generate_header(
    const char *method,
    const char *url,
    const char *consumer_key,
    const char *consumer_secret,
    const char *token,
    const char *token_secret,
    const char *nonce,
    const char *timestamp,
    char *out_header,
    size_t out_header_len
);

#endif /* HATHITRUST_OAUTH_H */
