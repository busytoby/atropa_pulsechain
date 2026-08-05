#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L

#include "hathitrust_marc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char* extract_marc_subfield(const char *xml, const char *tag, const char *code) {
    char tag_search[64];
    snprintf(tag_search, sizeof(tag_search), "tag=\"%s\"", tag);
    
    const char *ptr = xml;
    while ((ptr = strstr(ptr, tag_search)) != NULL) {
        // Find end of datafield tag opening
        const char *datafield_start = ptr;
        // Find closing </datafield>
        const char *datafield_end = strstr(datafield_start, "</datafield>");
        if (!datafield_end) return NULL;
        
        char code_search[64];
        snprintf(code_search, sizeof(code_search), "code=\"%s\"", code);
        
        const char *subfield_ptr = strstr(datafield_start, code_search);
        if (subfield_ptr && subfield_ptr < datafield_end) {
            // Find inner text after subfield opening tag
            const char *val_start = strchr(subfield_ptr, '>');
            if (val_start) {
                val_start++;
                const char *val_end = strstr(val_start, "</subfield>");
                if (val_end && val_end < datafield_end) {
                    size_t len = val_end - val_start;
                    char *val = malloc(len + 1);
                    if (val) {
                        memcpy(val, val_start, len);
                        val[len] = '\0';
                        
                        // Clean up trailing slash/comma/dots commonly found in MARC fields
                        if (len > 1 && (val[len - 1] == '/' || val[len - 1] == ',' || val[len - 1] == '.' || val[len - 1] == ';')) {
                            val[len - 1] = '\0';
                            // Trim trailing space
                            size_t new_len = strlen(val);
                            if (new_len > 0 && val[new_len - 1] == ' ') {
                                val[new_len - 1] = '\0';
                            }
                        }
                        
                        return val;
                    }
                }
            }
        }
        ptr += 10;
    }
    
    return NULL;
}

bool hathitrust_marc_transcode(const char *xml, HtrcMarcBibliographic *bib) {
    if (!xml || !bib) return false;
    
    memset(bib, 0, sizeof(HtrcMarcBibliographic));
    
    // Title is 245 subfield a
    bib->title = extract_marc_subfield(xml, "245", "a");
    
    // Author is 100 subfield a
    bib->author = extract_marc_subfield(xml, "100", "a");
    
    // Date is 260 subfield c or 264 subfield c
    bib->pub_date = extract_marc_subfield(xml, "260", "c");
    if (!bib->pub_date) {
        bib->pub_date = extract_marc_subfield(xml, "264", "c");
    }
    
    // ISBN is 020 subfield a
    bib->isbn = extract_marc_subfield(xml, "020", "a");
    
    // OCLC is 035 subfield a
    bib->oclc = extract_marc_subfield(xml, "035", "a");
    
    return true;
}

void hathitrust_marc_free(HtrcMarcBibliographic *bib) {
    if (!bib) return;
    free(bib->title);
    free(bib->author);
    free(bib->pub_date);
    free(bib->isbn);
    free(bib->oclc);
    memset(bib, 0, sizeof(HtrcMarcBibliographic));
}
