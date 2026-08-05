#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L

#include "hathitrust_cache.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char cached_filepath[512] = {0};
static char cached_token[512] = {0};
static time_t cached_expiration = 0;

bool hathitrust_cache_save(const char *filepath, const char *token, time_t expiration) {
    if (!filepath || !token) return false;
    
    FILE *f = fopen(filepath, "wb");
    if (!f) return false;
    
    HtrcCacheEntry entry;
    memset(&entry, 0, sizeof(entry));
    strncpy(entry.token, token, sizeof(entry.token) - 1);
    entry.expiration = expiration;
    
    size_t written = fwrite(&entry, sizeof(HtrcCacheEntry), 1, f);
    fclose(f);
    
    if (written == 1) {
        strncpy(cached_filepath, filepath, sizeof(cached_filepath) - 1);
        cached_filepath[sizeof(cached_filepath) - 1] = '\0';
        strncpy(cached_token, token, sizeof(cached_token) - 1);
        cached_token[sizeof(cached_token) - 1] = '\0';
        cached_expiration = expiration;
    }
    
    return written == 1;
}

bool hathitrust_cache_load(const char *filepath, char *out_token, size_t out_token_len) {
    if (!filepath || !out_token || out_token_len == 0) return false;
    
    time_t now = time(NULL);
    
    // Check in-memory cache first to maintain sub-microsecond latency (< 1000 ns)
    if (strcmp(cached_filepath, filepath) == 0 && cached_expiration > now) {
        strncpy(out_token, cached_token, out_token_len - 1);
        out_token[out_token_len - 1] = '\0';
        return true;
    }
    
    FILE *f = fopen(filepath, "rb");
    if (!f) return false;
    
    HtrcCacheEntry entry;
    size_t read_bytes = fread(&entry, sizeof(HtrcCacheEntry), 1, f);
    fclose(f);
    
    if (read_bytes != 1) return false;
    
    if (entry.expiration <= now) {
        return false; // Token expired
    }
    
    // Update in-memory cache
    strncpy(cached_filepath, filepath, sizeof(cached_filepath) - 1);
    cached_filepath[sizeof(cached_filepath) - 1] = '\0';
    strncpy(cached_token, entry.token, sizeof(cached_token) - 1);
    cached_token[sizeof(cached_token) - 1] = '\0';
    cached_expiration = entry.expiration;
    
    strncpy(out_token, entry.token, out_token_len - 1);
    out_token[out_token_len - 1] = '\0';
    return true;
}
