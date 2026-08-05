#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L

#include "hathitrust_mets_range.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* hathitrust_mets_map_range(const char *label, int start_page, int end_page) {
    if (!label || start_page < 0 || end_page < start_page) return NULL;
    
    size_t cap = 2048;
    char *res = malloc(cap);
    if (!res) return NULL;
    
    snprintf(res, cap,
             "{\n"
             "  \"@id\": \"http://example.org/iiif/range/r1\",\n"
             "  \"@type\": \"sc:Range\",\n"
             "  \"label\": \"%s\",\n"
             "  \"canvases\": [\n",
             label);
             
    for (int p = start_page; p <= end_page; p++) {
        char canvas_buf[256];
        snprintf(canvas_buf, sizeof(canvas_buf),
                 "    \"http://example.org/iiif/canvas/p%d\"%s\n",
                 p, (p == end_page) ? "" : ",");
        strncat(res, canvas_buf, cap - strlen(res) - 1);
    }
    
    strncat(res, "  ]\n}", cap - strlen(res) - 1);
    return res;
}
