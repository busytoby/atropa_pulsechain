#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L

#include "hathitrust_region.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

HtrcStructuralRegionType hathitrust_region_classify(const HtrcAltoPage *page) {
    if (!page || page->token_count == 0) return REGION_UNKNOWN;
    
    // Check for index keywords
    bool has_index_keyword = false;
    for (int i = 0; i < page->token_count; i++) {
        if (strcasecmp(page->tokens[i].content, "index") == 0 ||
            strcasecmp(page->tokens[i].content, "index.") == 0 ||
            strcasecmp(page->tokens[i].content, "table") == 0) {
            has_index_keyword = true;
            break;
        }
    }
    
    if (page->token_count < 15) {
        return REGION_TITLE_PAGE;
    }
    
    if (has_index_keyword && page->token_count > 30) {
        return REGION_INDEX_BACK;
    }
    
    return REGION_CONTENT_BODY;
}
