#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L

#include "hathitrust_marc_jsonld.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* hathitrust_marc_to_jsonld(const HtrcMarcBibliographic *bib) {
    if (!bib) return NULL;
    
    // Allocate space for formatted JSON-LD
    size_t cap = 2048;
    char *res = malloc(cap);
    if (!res) return NULL;
    
    snprintf(res, cap,
             "{\n"
             "  \"@context\": \"http://schema.org\",\n"
             "  \"@type\": \"Book\",\n"
             "  \"name\": \"%s\",\n"
             "  \"author\": {\n"
             "    \"@type\": \"Person\",\n"
             "    \"name\": \"%s\"\n"
             "  },\n"
             "  \"datePublished\": \"%s\",\n"
             "  \"isbn\": \"%s\",\n"
             "  \"identifier\": [\n"
             "    {\n"
             "      \"@type\": \"PropertyValue\",\n"
             "      \"name\": \"OCLC\",\n"
             "      \"value\": \"%s\"\n"
             "    }\n"
             "  ]\n"
             "}\n",
             bib->title ? bib->title : "",
             bib->author ? bib->author : "",
             bib->pub_date ? bib->pub_date : "",
             bib->isbn ? bib->isbn : "",
             bib->oclc ? bib->oclc : "");
             
    return res;
}
