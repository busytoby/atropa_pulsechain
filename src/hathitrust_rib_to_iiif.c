#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L

#include "hathitrust_rib_to_iiif.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* hathitrust_rib_to_iiif(const char *rib) {
    if (!rib) return NULL;
    
    // We scan for PxrTexture pattern URLs
    // Pattern "PxrTexture" "page_tex_1" "string filename" ["https://..."]
    size_t cap = 4096;
    char *res = malloc(cap);
    if (!res) return NULL;
    
    char *ptr = res;
    size_t remaining = cap;
    
    int n = snprintf(ptr, remaining,
                     "{\n"
                     "  \"@context\": \"http://iiif.io/api/presentation/2/context.json\",\n"
                     "  \"@type\": \"sc:Manifest\",\n"
                     "  \"@id\": \"https://catalog.hathitrust.org/iiif/rib_export/manifest\",\n"
                     "  \"label\": \"RIB Exported Manifest\",\n"
                     "  \"sequences\": [\n"
                     "    {\n"
                     "      \"@type\": \"sc:Sequence\",\n"
                     "      \"canvases\": [\n");
    ptr += n;
    remaining -= n;
    
    const char *scan = rib;
    int canvas_count = 0;
    while ((scan = strstr(scan, "PxrTexture")) != NULL) {
        // Find texture ID / name
        const char *name_ptr = strchr(scan, '"');
        if (name_ptr) name_ptr = strchr(name_ptr + 1, '"'); // Move to target name
        
        // Find filename URL
        const char *file_ptr = strstr(scan, "filename\"");
        if (file_ptr) {
            const char *url_start = strchr(file_ptr, '[');
            if (url_start && *(url_start + 1) == '"') {
                url_start += 2;
                const char *url_end = strchr(url_start, '"');
                if (url_end) {
                    size_t url_len = url_end - url_start;
                    char url[1024];
                    if (url_len < sizeof(url)) {
                        memcpy(url, url_start, url_len);
                        url[url_len] = '\0';
                        
                        n = snprintf(ptr, remaining,
                                     "        {\n"
                                     "          \"@type\": \"sc:Canvas\",\n"
                                     "          \"@id\": \"https://catalog.hathitrust.org/iiif/rib_export/canvas/p%d\",\n"
                                     "          \"label\": \"Page %d\",\n"
                                     "          \"height\": 3000,\n"
                                     "          \"width\": 2000,\n"
                                     "          \"images\": [\n"
                                     "            {\n"
                                     "              \"@type\": \"oa:Annotation\",\n"
                                     "              \"motivation\": \"sc:painting\",\n"
                                     "              \"resource\": {\n"
                                     "                \"@type\": \"dctypes:Image\",\n"
                                     "                \"@id\": \"%s\",\n"
                                     "                \"format\": \"image/jpeg\"\n"
                                     "              },\n"
                                     "              \"on\": \"https://catalog.hathitrust.org/iiif/rib_export/canvas/p%d\"\n"
                                     "            }\n"
                                     "          ]\n"
                                     "        },\n",
                                     canvas_count + 1, canvas_count + 1, url, canvas_count + 1);
                        ptr += n;
                        remaining -= n;
                        canvas_count++;
                    }
                }
            }
        }
        scan += 10;
    }
    
    // Clean trailing comma from last canvas item if present
    if (canvas_count > 0 && *(ptr - 2) == ',') {
        ptr -= 2;
        remaining += 2;
    }
    
    snprintf(ptr, remaining,
             "\n      ]\n"
             "    }\n"
             "  ]\n"
             "}\n");
             
    return res;
}
