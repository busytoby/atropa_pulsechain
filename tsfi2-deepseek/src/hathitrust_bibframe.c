#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L

#include "hathitrust_bibframe.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

bool hathitrust_to_bibframe_rdf(
    const char *title,
    const char *creator,
    char *out_rdf,
    size_t max_len
) {
    if (!title || !creator || !out_rdf || max_len == 0) return false;
    
    int written = snprintf(out_rdf, max_len,
                           "<http://example.org/work> <http://id.loc.gov/ontologies/bibframe/title> \"%s\" .\n"
                           "<http://example.org/work> <http://id.loc.gov/ontologies/bibframe/creator> \"%s\" .\n",
                           title, creator);
                           
    return (written > 0 && (size_t)written < max_len);
}
