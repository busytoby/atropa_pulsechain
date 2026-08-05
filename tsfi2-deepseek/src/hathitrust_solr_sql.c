#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L

#include "hathitrust_solr_sql.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void escape_sql_string(char *dest, const char *src, size_t dest_cap) {
    size_t d = 0;
    for (size_t s = 0; src[s] != '\0' && d < dest_cap - 2; s++) {
        if (src[s] == '\'') {
            dest[d++] = '\'';
            dest[d++] = '\'';
        } else {
            dest[d++] = src[s];
        }
    }
    dest[d] = '\0';
}

char* hathitrust_solr_to_sql(
    const char *htid,
    const char *title,
    const char *author,
    const char *pub_date
) {
    if (!htid || !title || !author || !pub_date) return NULL;
    
    char esc_title[1024];
    char esc_author[1024];
    
    escape_sql_string(esc_title, title, sizeof(esc_title));
    escape_sql_string(esc_author, author, sizeof(esc_author));
    
    size_t cap = 4096;
    char *res = malloc(cap);
    if (!res) return NULL;
    
    snprintf(res, cap,
             "INSERT INTO hathitrust_catalog (htid, title, author, pub_date) "
             "VALUES ('%s', '%s', '%s', '%s');\n",
             htid, esc_title, esc_author, pub_date);
             
    return res;
}
