#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L

#include "hathitrust_pxrmaterial.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* hathitrust_generate_pxrmaterial_rib(
    const char *material_id,
    const char *diffuse_tex,
    const char *specular_tex,
    const char *roughness_tex
) {
    if (!material_id || !diffuse_tex || !specular_tex || !roughness_tex) return NULL;
    
    size_t cap = 2048;
    char *res = malloc(cap);
    if (!res) return NULL;
    
    snprintf(res, cap,
             "  # Bxdf bindings for material %s\n"
             "  Pattern \"PxrTexture\" \"diff_%s\" \"string filename\" [\"%s\"]\n"
             "  Pattern \"PxrTexture\" \"spec_%s\" \"string filename\" [\"%s\"]\n"
             "  Pattern \"PxrTexture\" \"rough_%s\" \"string filename\" [\"%s\"]\n"
             "  Bxdf \"PxrDisney\" \"mat_%s\"\n"
             "  {\n"
             "    \"reference color baseColor\" [\"diff_%s:resultRGB\"]\n"
             "    \"reference float specular\" [\"spec_%s:resultR\"]\n"
             "    \"reference float roughness\" [\"rough_%s:resultR\"]\n"
             "  }\n",
             material_id, material_id, diffuse_tex,
             material_id, specular_tex,
             material_id, roughness_tex,
             material_id, material_id, material_id, material_id);
             
    return res;
}
