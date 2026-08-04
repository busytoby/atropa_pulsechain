#ifndef HATHITRUST_CACHE_H
#define HATHITRUST_CACHE_H

#include <stdbool.h>
#include <time.h>

typedef struct {
    char token[512];
    time_t expiration;
} HtrcCacheEntry;

/*
 * Saves a resumption token and its expiration timestamp to a local cache file.
 * Parameters:
 *   filepath: The destination cache file.
 *   token: The resumption token string.
 *   expiration: Expiration epoch timestamp.
 * Returns:
 *   true on success, false on write error.
 */
bool hathitrust_cache_save(const char *filepath, const char *token, time_t expiration);

/*
 * Loads the active resumption token from the cache file if it has not expired.
 * Parameters:
 *   filepath: The cache file path.
 *   out_token: Destination buffer to write the token.
 *   out_token_len: Size of the destination buffer.
 * Returns:
 *   true if a valid, unexpired token was loaded, false otherwise.
 */
bool hathitrust_cache_load(const char *filepath, char *out_token, size_t out_token_len);

#endif /* HATHITRUST_CACHE_H */
