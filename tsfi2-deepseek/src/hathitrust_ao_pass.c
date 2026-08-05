#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L

#include "hathitrust_ao_pass.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* hathitrust_generate_ao_pass_rib(void) {
    size_t cap = 1024;
    char *res = malloc(cap);
    if (!res) return NULL;
    
    snprintf(res, cap,
             "## RIB Ambient Occlusion Render Pass\n"
             "Option \"searchpath\" \"shader\" [\".:../shaders\"]\n"
             "Display \"ao_pass.exr\" \"openexr\" \"color\"\n"
             "AttributeBegin\n"
             "  Attribute \"visibility\" \"int camera\" [1]\n"
             "  Attribute \"visibility\" \"int indirect\" [1]\n"
             "  Integrator \"PxrOcclusion\" \"ao_integrator\"\n"
             "  {\n"
             "    \"int numSamples\" [64]\n"
             "    \"float maxDistance\" [0.8]\n"
             "    \"string distribution\" [\"cosine\"]\n"
             "  }\n"
             "AttributeEnd\n");
             
    return res;
}
