#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L

#include "hathitrust_spine.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* hathitrust_generate_book_binding_rib(int page_count) {
    if (page_count <= 0) return NULL;
    
    // Scale spine thickness (approx 0.1 mm per page, normalized to render space units)
    double spine_thickness = page_count * 0.002;
    if (spine_thickness < 0.05) spine_thickness = 0.05;
    
    size_t cap = 2048;
    char *res = malloc(cap);
    if (!res) return NULL;
    
    snprintf(res, cap,
             "  # Book Binding Spine and Covers\n"
             "  AttributeBegin\n"
             "    Attribute \"identifier\" \"name\" [\"book_spine\"]\n"
             "    Bxdf \"PxrDisney\" \"spine_material\" \"color baseColor\" [0.15 0.1 0.05]\n"
             "    # Spine geometry block\n"
             "    Patch \"bilinear\" \"P\" [-%f -1.6 0  %f -1.6 0  -%f 1.6 0  %f 1.6 0]\n"
             "  AttributeEnd\n"
             "  AttributeBegin\n"
             "    Attribute \"identifier\" \"name\" [\"front_cover\"]\n"
             "    Bxdf \"PxrDisney\" \"cover_material\" \"color baseColor\" [0.1 0.2 0.3]\n"
             "    # Front cover geometry offset outside spine\n"
             "    Translate %f 0 0\n"
             "    Patch \"bilinear\" \"P\" [0 -1.6 0  1.1 -1.6 0  0 1.6 0  1.1 1.6 0]\n"
             "  AttributeEnd\n"
             "  AttributeBegin\n"
             "    Attribute \"identifier\" \"name\" [\"back_cover\"]\n"
             "    Bxdf \"PxrDisney\" \"cover_material\" \"color baseColor\" [0.1 0.2 0.3]\n"
             "    # Back cover geometry offset outside spine\n"
             "    Translate -%f 0 0\n"
             "    Patch \"bilinear\" \"P\" [-1.1 -1.6 0  0 -1.6 0  -1.1 1.6 0  0 1.6 0]\n"
             "  AttributeEnd\n",
             spine_thickness / 2.0, spine_thickness / 2.0,
             spine_thickness / 2.0, spine_thickness / 2.0,
             spine_thickness / 2.0,
             spine_thickness / 2.0);
             
    return res;
}
