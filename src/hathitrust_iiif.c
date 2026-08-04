#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L

#include "hathitrust_iiif.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* hathitrust_mets_to_iiif(const HtrcMetsStructure *mets, const char *title) {
    if (!mets) return NULL;
    
    size_t cap = 4096 + mets->page_count * 1024;
    char *res = malloc(cap);
    if (!res) return NULL;
    
    char *ptr = res;
    size_t remaining = cap;
    
    int n = snprintf(ptr, remaining,
                     "{\n"
                     "  \"@context\": \"http://iiif.io/api/presentation/2/context.json\",\n"
                     "  \"@type\": \"sc:Manifest\",\n"
                     "  \"@id\": \"https://catalog.hathitrust.org/iiif/%s/manifest\",\n"
                     "  \"label\": \"%s\",\n"
                     "  \"sequences\": [\n"
                     "    {\n"
                     "      \"@type\": \"sc:Sequence\",\n"
                     "      \"canvases\": [\n",
                     mets->volume_id ? mets->volume_id : "unknown",
                     title ? title : "Unknown Title");
                     
    ptr += n;
    remaining -= n;
    
    for (int i = 0; i < mets->page_count; i++) {
        HtrcMetsPage *page = &mets->pages[i];
        
        n = snprintf(ptr, remaining,
                     "        {\n"
                     "          \"@type\": \"sc:Canvas\",\n"
                     "          \"@id\": \"https://catalog.hathitrust.org/iiif/%s/canvas/p%d\",\n"
                     "          \"label\": \"%s\",\n"
                     "          \"height\": 3000,\n"
                     "          \"width\": 2000,\n"
                     "          \"images\": [\n"
                     "            {\n"
                     "              \"@type\": \"oa:Annotation\",\n"
                     "              \"motivation\": \"sc:painting\",\n"
                     "              \"resource\": {\n"
                     "                \"@type\": \"dctypes:Image\",\n"
                     "                \"@id\": \"https://babel.hathitrust.org/cgi/imgsrv/image?id=%s;seq=%d;size=100\",\n"
                     "                \"format\": \"image/jpeg\"\n"
                     "              },\n"
                     "              \"on\": \"https://catalog.hathitrust.org/iiif/%s/canvas/p%d\"\n"
                     "            }\n"
                     "          ]\n"
                     "        }%s\n",
                     mets->volume_id ? mets->volume_id : "unknown",
                     page->seq,
                     page->label ? page->label : "",
                     mets->volume_id ? mets->volume_id : "unknown",
                     page->seq,
                     mets->volume_id ? mets->volume_id : "unknown",
                     page->seq,
                     (i == mets->page_count - 1) ? "" : ",");
                     
        ptr += n;
        remaining -= n;
    }
    
    snprintf(ptr, remaining,
             "      ]\n"
             "    }\n"
             "  ]\n"
             "}\n");
             
    return res;
}
