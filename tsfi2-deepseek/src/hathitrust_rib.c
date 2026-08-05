#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L

#include "hathitrust_rib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* hathitrust_mets_to_rib(const HtrcMetsStructure *mets) {
    if (!mets) return NULL;
    
    size_t cap = 2048 + mets->page_count * 512;
    char *res = malloc(cap);
    if (!res) return NULL;
    
    char *ptr = res;
    size_t remaining = cap;
    
    int n = snprintf(ptr, remaining,
                     "##RenderMan RIB-Structure 1.1\n"
                     "FrameBegin 1\n"
                     "Display \"book_render.tiff\" \"file\" \"rgba\"\n"
                     "Format 1920 1080 1\n"
                     "Projection \"perspective\" \"fov\" [30]\n"
                     "Translate 0 0 5\n"
                     "WorldBegin\n"
                     "  LightSource \"distantlight\" 1 \"intensity\" [1.5] \"from\" [0 10 -10]\n");
                     
    ptr += n;
    remaining -= n;
    
    for (int i = 0; i < mets->page_count; i++) {
        HtrcMetsPage *page = &mets->pages[i];
        
        n = snprintf(ptr, remaining,
                     "  AttributeBegin\n"
                     "    Attribute \"identifier\" \"name\" [\"page_%d\"]\n"
                     "    Translate %d 0 0\n"
                     "    Pattern \"PxrTexture\" \"page_tex_%d\" \"string filename\" [\"https://babel.hathitrust.org/cgi/imgsrv/image?id=%s;seq=%d;size=100\"]\n"
                     "    Bxdf \"PxrDisney\" \"material_%d\" \"reference color baseColor\" [\"page_tex_%d:resultRGB\"]\n"
                     "    SubdivisionMesh \"catmull-clark\" [4] [0 1 2 3] [\"interpolateboundary\"] [0 0] [] [] \"P\" [-1 -1.5 0  1 -1.5 0  1 1.5 0  -1 1.5 0]\n"
                     "  AttributeEnd\n",
                     page->seq,
                     i * 2, // Spacing pages out in space
                     page->seq,
                     mets->volume_id ? mets->volume_id : "unknown",
                     page->seq,
                     page->seq,
                     page->seq);
                     
        ptr += n;
        remaining -= n;
    }
    
    snprintf(ptr, remaining,
             "WorldEnd\n"
             "FrameEnd\n");
             
    return res;
}
