#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L

#include "hathitrust_alto_consolidate.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* hathitrust_alto_consolidate_text(const HtrcAltoPage *page) {
    if (!page || page->token_count == 0) return NULL;
    
    // Estimate size
    size_t cap = 1024;
    char *res = malloc(cap);
    if (!res) return NULL;
    res[0] = '\0';
    size_t len = 0;
    
    int last_vpos = -1;
    for (int i = 0; i < page->token_count; i++) {
        HtrcAltoToken *tok = &page->tokens[i];
        
        // Check for vertical position jumps signifying new lines
        if (last_vpos != -1 && tok->vpos > last_vpos + 10) {
            // Append newline
            if (len + 2 >= cap) {
                cap = cap * 2 + 1024;
                res = realloc(res, cap);
            }
            strcat(res, "\n");
            len++;
        } else if (i > 0) {
            // Append space
            if (len + 2 >= cap) {
                cap = cap * 2 + 1024;
                res = realloc(res, cap);
            }
            strcat(res, " ");
            len++;
        }
        
        size_t tok_len = strlen(tok->content);
        if (len + tok_len + 1 >= cap) {
            cap = cap * 2 + tok_len + 1024;
            res = realloc(res, cap);
        }
        strcat(res, tok->content);
        len += tok_len;
        
        last_vpos = tok->vpos;
    }
    
    return res;
}
