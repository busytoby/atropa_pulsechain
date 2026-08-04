#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L

#include "hathitrust_mets.h"
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

bool hathitrust_mets_parse(const char *xml, HtrcMetsStructure *mets) {
    if (!xml || !mets) return false;
    
    memset(mets, 0, sizeof(HtrcMetsStructure));
    
    // Find OBJID or ID of the volume
    const char *objid_ptr = strstr(xml, "OBJID=\"");
    if (objid_ptr) {
        mets->volume_id = extract_attribute(objid_ptr - 6, "OBJID");
    }
    
    mets->page_capacity = 16;
    mets->pages = malloc(mets->page_capacity * sizeof(HtrcMetsPage));
    mets->page_count = 0;
    
    const char *ptr = xml;
    while ((ptr = strstr(ptr, "<div")) != NULL) {
        // We only care about TYPE="page" divisions
        char *type_val = extract_attribute(ptr, "TYPE");
        if (type_val && strcmp(type_val, "page") == 0) {
            free(type_val);
            
            char *order_val = extract_attribute(ptr, "ORDER");
            char *label_val = extract_attribute(ptr, "LABEL");
            
            // Find file ID inside <fptr> or similar child tags if present
            char *file_id_val = NULL;
            const char *fptr_ptr = strstr(ptr, "<fptr");
            if (fptr_ptr) {
                const char *div_end = strstr(ptr, "</div>");
                if (div_end && fptr_ptr < div_end) {
                    file_id_val = extract_attribute(fptr_ptr, "FILEID");
                }
            }
            
            if (mets->page_count >= mets->page_capacity) {
                mets->page_capacity *= 2;
                mets->pages = realloc(mets->pages, mets->page_capacity * sizeof(HtrcMetsPage));
            }
            
            HtrcMetsPage *page = &mets->pages[mets->page_count];
            page->seq = order_val ? atoi(order_val) : (mets->page_count + 1);
            page->label = label_val ? label_val : strdup("");
            page->file_id = file_id_val ? file_id_val : strdup("");
            
            free(order_val);
            mets->page_count++;
        } else {
            free(type_val);
        }
        
        ptr += 4;
    }
    
    return true;
}

void hathitrust_mets_free(HtrcMetsStructure *mets) {
    if (!mets) return;
    free(mets->volume_id);
    if (mets->pages) {
        for (int i = 0; i < mets->page_count; i++) {
            free(mets->pages[i].label);
            free(mets->pages[i].file_id);
        }
        free(mets->pages);
    }
    memset(mets, 0, sizeof(HtrcMetsStructure));
}
