#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L

#include "hathitrust_cache.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
    
    return written == 1;
}

bool hathitrust_cache_load(const char *filepath, char *out_token, size_t out_token_len) {
    if (!filepath || !out_token || out_token_len == 0) return false;
    
    FILE *f = fopen(filepath, "rb");
    if (!f) return false;
    
    HtrcCacheEntry entry;
    size_t read_bytes = fread(&entry, sizeof(HtrcCacheEntry), 1, f);
    fclose(f);
    
    if (read_bytes != 1) return false;
    
    // Check expiration against current time
    time_t now = time(NULL);
    if (entry.expiration <= now) {
        return false; // Token expired
    }
    
    strncpy(out_token, entry.token, out_token_len - 1);
    out_token[out_token_len - 1] = '\0';
    return true;
}
