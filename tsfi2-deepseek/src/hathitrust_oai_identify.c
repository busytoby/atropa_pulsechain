#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L

#include "hathitrust_oai_identify.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char* extract_tag_value(const char *xml, const char *tag) {
    char open_tag[64];
    char close_tag[64];
    snprintf(open_tag, sizeof(open_tag), "<%s>", tag);
    snprintf(close_tag, sizeof(close_tag), "</%s>", tag);
    
    const char *start = strstr(xml, open_tag);
    if (!start) return NULL;
    start += strlen(open_tag);
    
    const char *end = strstr(start, close_tag);
    if (!end) return NULL;
    
    size_t len = end - start;
    char *res = malloc(len + 1);
    if (!res) return NULL;
    memcpy(res, start, len);
    res[len] = '\0';
    return res;
}

bool hathitrust_oai_parse_identify(const char *xml, HtrcOaiIdentity *ident) {
    if (!xml || !ident) return false;
    memset(ident, 0, sizeof(HtrcOaiIdentity));
    
    ident->repository_name = extract_tag_value(xml, "repositoryName");
    ident->admin_email = extract_tag_value(xml, "adminEmail");
    ident->granularity = extract_tag_value(xml, "granularity");
    ident->compression = extract_tag_value(xml, "compression");
    
    return (ident->repository_name != NULL);
}

void hathitrust_oai_free_identity(HtrcOaiIdentity *ident) {
    if (!ident) return;
    free(ident->repository_name);
    free(ident->admin_email);
    free(ident->granularity);
    free(ident->compression);
    memset(ident, 0, sizeof(HtrcOaiIdentity));
}
