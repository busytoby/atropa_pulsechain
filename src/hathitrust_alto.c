#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L

#include "hathitrust_alto.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char* extract_attribute(const char *tag, const char *attr) {
    char search[128];
    snprintf(search, sizeof(search), "%s=\"", attr);
    const char *ptr = strstr(tag, search);
    if (!ptr) return NULL;
    ptr += strlen(search);
    const char *end = strchr(ptr, '"');
    if (!end) return NULL;
    size_t len = end - ptr;
    char *val = malloc(len + 1);
    if (val) {
        memcpy(val, ptr, len);
        val[len] = '\0';
    }
    return val;
}

bool hathitrust_alto_parse(const char *xml, HtrcAltoPage *page) {
    if (!xml || !page) return false;
    
    memset(page, 0, sizeof(HtrcAltoPage));
    page->token_capacity = 16;
    page->tokens = malloc(page->token_capacity * sizeof(HtrcAltoToken));
    page->token_count = 0;
    
    const char *ptr = xml;
    while ((ptr = strstr(ptr, "<String")) != NULL) {
        char *content_val = extract_attribute(ptr, "CONTENT");
        char *hpos_val = extract_attribute(ptr, "HPOS");
        char *vpos_val = extract_attribute(ptr, "VPOS");
        char *width_val = extract_attribute(ptr, "WIDTH");
        char *height_val = extract_attribute(ptr, "HEIGHT");
        
        if (content_val) {
            if (page->token_count >= page->token_capacity) {
                page->token_capacity *= 2;
                page->tokens = realloc(page->tokens, page->token_capacity * sizeof(HtrcAltoToken));
            }
            
            HtrcAltoToken *tok = &page->tokens[page->token_count];
            tok->content = content_val;
            tok->hpos = hpos_val ? atoi(hpos_val) : 0;
            tok->vpos = vpos_val ? atoi(vpos_val) : 0;
            tok->width = width_val ? atoi(width_val) : 0;
            tok->height = height_val ? atoi(height_val) : 0;
            
            page->token_count++;
        }
        
        free(hpos_val);
        free(vpos_val);
        free(width_val);
        free(height_val);
        
        ptr += 7;
    }
    
    return true;
}

void hathitrust_alto_free(HtrcAltoPage *page) {
    if (!page) return;
    if (page->tokens) {
        for (int i = 0; i < page->token_count; i++) {
            free(page->tokens[i].content);
        }
        free(page->tokens);
    }
    memset(page, 0, sizeof(HtrcAltoPage));
}
