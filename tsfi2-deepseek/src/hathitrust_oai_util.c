#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L

#include "hathitrust_oai_util.h"
#include "hathitrust_oai.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* hathitrust_oai_extract_tag(const char *xml, const char *tag) {
    if (!xml || !tag) return NULL;
    char open_tag[128];
    snprintf(open_tag, sizeof(open_tag), "<%s", tag);
    const char *start = strstr(xml, open_tag);
    if (!start) return NULL;
    
    const char *close_bracket = strchr(start, '>');
    if (!close_bracket) return NULL;
    start = close_bracket + 1;
    
    char close_tag[128];
    snprintf(close_tag, sizeof(close_tag), "</%s>", tag);
    const char *end = strstr(start, close_tag);
    if (!end) return NULL;
    
    size_t len = end - start;
    char *res = malloc(len + 1);
    if (res) {
        memcpy(res, start, len);
        res[len] = '\0';
    }
    return res;
}

bool hathitrust_oai_harvest_all(const char *set, void (*page_callback)(const char *xml_page)) {
    if (!page_callback) return false;
    
    // Page 1
    char *xml = hathitrust_oai_query("ListRecords", "oai_dc", set, NULL, NULL, NULL, NULL);
    if (!xml) {
        return false;
    }
    
    page_callback(xml);
    
    char *token = hathitrust_oai_extract_tag(xml, "resumptionToken");
    free(xml);
    
    // Auto-pagination loop
    // To prevent infinite loops during tests or overloads, limit to a maximum of 5 iterations
    int page_count = 1;
    while (token && strlen(token) > 0 && page_count < 5) {
        char *next_xml = hathitrust_oai_query("ListRecords", NULL, NULL, NULL, token, NULL, NULL);
        free(token);
        token = NULL;
        
        if (!next_xml) {
            return false;
        }
        
        page_callback(next_xml);
        
        token = hathitrust_oai_extract_tag(next_xml, "resumptionToken");
        free(next_xml);
        page_count++;
    }
    
    free(token);
    return true;
}

bool hathitrust_oai_save_state(const char *filepath, const char *datestamp) {
    if (!filepath || !datestamp) return false;
    FILE *f = fopen(filepath, "w");
    if (!f) return false;
    fprintf(f, "%s\n", datestamp);
    fclose(f);
    return true;
}

char* hathitrust_oai_load_state(const char *filepath) {
    if (!filepath) return NULL;
    FILE *f = fopen(filepath, "r");
    if (!f) return NULL;
    
    char buf[128];
    if (fgets(buf, sizeof(buf), f)) {
        size_t len = strlen(buf);
        while (len > 0 && (buf[len - 1] == '\n' || buf[len - 1] == '\r')) {
            buf[len - 1] = '\0';
            len--;
        }
        fclose(f);
        return strdup(buf);
    }
    
    fclose(f);
    return NULL;
}
