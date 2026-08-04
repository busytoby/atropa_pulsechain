#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L

#include "hathitrust_oauth_callback.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

bool hathitrust_oauth_verify_callback(
    const char *query_string,
    char *out_verifier,
    size_t max_len
) {
    if (!query_string || !out_verifier || max_len == 0) return false;
    
    const char *key = "oauth_verifier=";
    const char *pos = strstr(query_string, key);
    if (!pos) return false;
    
    pos += strlen(key);
    size_t idx = 0;
    while (*pos && *pos != '&' && idx < max_len - 1) {
        out_verifier[idx++] = *pos++;
    }
    out_verifier[idx] = '\0';
    
    return (idx > 0);
}
