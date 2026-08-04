#ifndef HATHITRUST_OAUTH_CALLBACK_H
#define HATHITRUST_OAUTH_CALLBACK_H

#include <stdbool.h>
#include <stddef.h>

/*
 * Evaluates a HathiTrust OAuth callback query string and extracts the verifier token.
 * Parameters:
 *   query_string: The URL query payload.
 *   out_verifier: Output buffer for the extracted oauth_verifier token.
 *   max_len: Size of the output buffer.
 * Returns:
 *   true on success, false if missing or token was truncated.
 */
bool hathitrust_oauth_verify_callback(
    const char *query_string,
    char *out_verifier,
    size_t max_len
);

#endif /* HATHITRUST_OAUTH_CALLBACK_H */
