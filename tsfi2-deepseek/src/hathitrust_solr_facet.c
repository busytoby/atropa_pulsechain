#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L

#include "hathitrust_solr_facet.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* hathitrust_solr_compile_facet_query(const char *query, const char *facet_field) {
    if (!query || !facet_field) return NULL;
    
    size_t cap = 1024;
    char *res = malloc(cap);
    if (!res) return NULL;
    
    snprintf(res, cap,
             "/solr/select?q=%s&facet=true&facet.field=%s&wt=json",
             query, facet_field);
             
    return res;
}
